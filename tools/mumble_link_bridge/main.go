// Package main implements a lightweight MumbleLink bridge running under Wine/Proton.
// It synchronizes Guild Wars 2's Windows named shared memory ("MumbleLink")
// to Linux's POSIX shared memory (/dev/shm/MumbleLink), enabling native Linux
// applications like GW2TacO to access game position and camera data.
package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

const (
	mumbleSize                     = 5460
	fileMapRead                    = 0x0004
	processQueryLimitedInformation = 0x1000
	stillActive                    = 259

	offsetTick      = 4
	offsetContext   = 1108
	offsetProcessId = offsetContext + 80 // 1188
)

var (
	kernel32               = syscall.NewLazyDLL("kernel32.dll")
	procOpenFileMappingW   = kernel32.NewProc("OpenFileMappingW")
	procMapViewOfFile      = kernel32.NewProc("MapViewOfFile")
	procUnmapViewOfFile    = kernel32.NewProc("UnmapViewOfFile")
	procOpenProcess        = kernel32.NewProc("OpenProcess")
	procGetExitCodeProcess = kernel32.NewProc("GetExitCodeProcess")
)

// normalizePath converts Unix or Windows paths into a valid Wine path pointing to the Linux host.
func normalizePath(p string) string {
	p = strings.TrimSpace(p)
	if strings.HasPrefix(p, "/") {
		// e.g. /dev/shm/MumbleLink -> Z:\dev\shm\MumbleLink
		return `Z:` + filepath.FromSlash(p)
	}
	return filepath.FromSlash(p)
}

// isProcessAlive checks if the Guild Wars 2 process is still running.
func isProcessAlive(pid uint32) bool {
	if pid == 0 {
		return true
	}
	r1, _, _ := procOpenProcess.Call(uintptr(processQueryLimitedInformation), 0, uintptr(pid))
	if r1 == 0 {
		return false
	}
	hProc := syscall.Handle(r1)
	defer syscall.CloseHandle(hProc)

	var exitCode uint32
	r2, _, _ := procGetExitCodeProcess.Call(uintptr(hProc), uintptr(unsafe.Pointer(&exitCode)))
	if r2 == 0 {
		return false
	}
	return exitCode == stillActive
}

func main() {
	defaultDest := os.Getenv("GW2_MUMBLE_PATH")
	if defaultDest == "" {
		defaultDest = os.Getenv("MUMBLE_LINK_PATH")
	}
	if defaultDest == "" {
		defaultDest = `Z:\dev\shm\MumbleLink`
	}

	destFlag := flag.String("dest", defaultDest, "Destination path on Linux filesystem (accessible via Wine drive)")
	nameFlag := flag.String("name", "MumbleLink", "Source Win32 memory map name created by Guild Wars 2")
	rateFlag := flag.Int("rate", 8, "Update poll interval in milliseconds when active (~125 Hz)")
	idleRateFlag := flag.Int("idle-rate", 100, "Update poll interval in milliseconds when idle (~10 Hz)")
	quietFlag := flag.Bool("quiet", false, "Suppress informational logging")
	verboseFlag := flag.Bool("verbose", false, "Print verbose telemetry updates")
	flag.Parse()

	targetPath := normalizePath(*destFlag)
	if !*quietFlag {
		fmt.Printf("[MumbleLink Bridge] Initializing...\n")
		fmt.Printf("[MumbleLink Bridge] Source Memory Map: %s\n", *nameFlag)
		fmt.Printf("[MumbleLink Bridge] Target Path:       %s\n", targetPath)
	}

	// Ensure destination directory exists (e.g. Z:\dev\shm)
	dir := filepath.Dir(targetPath)
	if err := os.MkdirAll(dir, 0755); err != nil && !*quietFlag {
		fmt.Printf("[MumbleLink Bridge] Warning: Could not create directory %s: %v\n", dir, err)
	}

	// Open or create the target file
	destFile, err := os.OpenFile(targetPath, os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		fmt.Printf("[MumbleLink Bridge] Error opening target file %s: %v\n", targetPath, err)
		os.Exit(1)
	}
	defer destFile.Close()

	if err := destFile.Truncate(mumbleSize); err != nil && !*quietFlag {
		fmt.Printf("[MumbleLink Bridge] Warning: Truncate failed on %s: %v\n", targetPath, err)
	}

	// Attempt to memory-map the target file for zero-copy performance
	var destBytes []byte
	var hDestMap syscall.Handle
	var destPtr uintptr

	hDestMap, err = syscall.CreateFileMapping(syscall.Handle(destFile.Fd()), nil, syscall.PAGE_READWRITE, 0, mumbleSize, nil)
	if err == nil && hDestMap != 0 {
		destPtr, err = syscall.MapViewOfFile(hDestMap, syscall.FILE_MAP_WRITE, 0, 0, mumbleSize)
		if err == nil && destPtr != 0 {
			defer syscall.UnmapViewOfFile(destPtr)
			destBytes = unsafe.Slice((*byte)(unsafe.Pointer(destPtr)), mumbleSize)
			if !*quietFlag {
				fmt.Printf("[MumbleLink Bridge] Destination mapped directly into memory via tmpfs.\n")
			}
		}
	}
	if destBytes == nil && !*quietFlag {
		fmt.Printf("[MumbleLink Bridge] Note: Memory mapping destination unsupported; falling back to file writes.\n")
	}
	if hDestMap != 0 {
		defer syscall.CloseHandle(hDestMap)
	}

	// Handle graceful shutdown on Ctrl+C / SIGINT
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-sigChan
		if !*quietFlag {
			fmt.Printf("\n[MumbleLink Bridge] Shutting down cleanly...\n")
		}
		if destPtr != 0 {
			_ = syscall.UnmapViewOfFile(destPtr)
		}
		if hDestMap != 0 {
			_ = syscall.CloseHandle(hDestMap)
		}
		destFile.Close()
		os.Exit(0)
	}()

	rateDuration := time.Duration(*rateFlag) * time.Millisecond
	idleDuration := time.Duration(*idleRateFlag) * time.Millisecond
	nameUtf16, err := syscall.UTF16PtrFromString(*nameFlag)
	if err != nil {
		fmt.Printf("[MumbleLink Bridge] Invalid source name: %v\n", err)
		os.Exit(1)
	}

	for {
		if !*quietFlag {
			fmt.Printf("[MumbleLink Bridge] Waiting for Guild Wars 2 (%s)...\n", *nameFlag)
		}

		var hSrc syscall.Handle
		for {
			r1, _, _ := procOpenFileMappingW.Call(uintptr(fileMapRead), 0, uintptr(unsafe.Pointer(nameUtf16)))
			if r1 != 0 {
				hSrc = syscall.Handle(r1)
				break
			}
			time.Sleep(1 * time.Second)
		}

		srcPtr, _, errMap := procMapViewOfFile.Call(uintptr(hSrc), uintptr(fileMapRead), 0, 0, mumbleSize)
		if srcPtr == 0 {
			if !*quietFlag {
				fmt.Printf("[MumbleLink Bridge] Failed to map view of source memory: %v. Retrying in 1s...\n", errMap)
			}
			syscall.CloseHandle(hSrc)
			time.Sleep(1 * time.Second)
			continue
		}

		srcBytes := unsafe.Slice((*byte)(unsafe.Pointer(srcPtr)), mumbleSize)
		if !*quietFlag {
			fmt.Printf("[MumbleLink Bridge] Connected to Guild Wars 2! Synchronizing data to %s...\n", targetPath)
		}

		var lastTick uint32
		var lastPid uint32
		idleCounter := 0
		checkProcessCounter := 0
		connected := true

		for connected {
			curTick := *(*uint32)(unsafe.Pointer(srcPtr + offsetTick))

			if curTick != lastTick {
				if destBytes != nil {
					copy(destBytes, srcBytes)
				} else {
					_, _ = destFile.WriteAt(srcBytes, 0)
				}

				if *verboseFlag && curTick%100 == 0 {
					pid := *(*uint32)(unsafe.Pointer(srcPtr + offsetProcessId))
					fmt.Printf("[MumbleLink Bridge] Tick: %d | PID: %d\n", curTick, pid)
				}

				lastTick = curTick
				idleCounter = 0
				time.Sleep(rateDuration)
			} else {
				idleCounter++
				if idleCounter > 60 { // ~500ms without tick change
					checkProcessCounter++
					if checkProcessCounter >= 10 { // check every ~1 second of idle
						checkProcessCounter = 0
						pid := *(*uint32)(unsafe.Pointer(srcPtr + offsetProcessId))
						if pid != 0 {
							lastPid = pid
						}
						if lastPid != 0 && !isProcessAlive(lastPid) {
							if !*quietFlag {
								fmt.Printf("[MumbleLink Bridge] Guild Wars 2 process (PID %d) terminated.\n", lastPid)
							}
							connected = false
							break
						}
					}
					time.Sleep(idleDuration)
				} else {
					time.Sleep(rateDuration)
				}
			}
		}

		// Cleanup source mapping and prepare for reconnection
		procUnmapViewOfFile.Call(srcPtr)
		syscall.CloseHandle(hSrc)
	}
}
