//go:build windows

// Package main implements a lightweight MumbleLink bridge running under Wine/Proton.
// It synchronizes Guild Wars 2's Windows named shared memory ("MumbleLink")
// to Linux's POSIX shared memory (/dev/shm/MumbleLink), enabling native Linux
// applications like GW2TacO to access game position and camera data.
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"
	"time"
	"unsafe"
)

const (
	processQueryLimitedInformation = 0x1000
	stillActive                    = 259
)

var (
	kernel32               = syscall.NewLazyDLL("kernel32.dll")
	procOpenProcess        = kernel32.NewProc("OpenProcess")
	procGetExitCodeProcess = kernel32.NewProc("GetExitCodeProcess")
)

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

	if *rateFlag <= 0 {
		fmt.Printf("[MumbleLink Bridge] Error: -rate must be > 0 (got %d)\n", *rateFlag)
		os.Exit(1)
	}
	if *idleRateFlag <= 0 {
		fmt.Printf("[MumbleLink Bridge] Error: -idle-rate must be > 0 (got %d)\n", *idleRateFlag)
		os.Exit(1)
	}

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

	// Open or create the target file, truncating any stale data from previous runs
	destFile, err := os.OpenFile(targetPath, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
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

	var zeroBuf [mumbleSize]byte
	if err := writeDest(destBytes, destFile, zeroBuf[:]); err != nil && !*quietFlag {
		fmt.Printf("[MumbleLink Bridge] Warning: Failed to clear %s: %v\n", targetPath, err)
	}

	// Handle graceful shutdown on Ctrl+C / SIGTERM without racing with active writes
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	rateDuration := time.Duration(*rateFlag) * time.Millisecond
	idleDuration := time.Duration(*idleRateFlag) * time.Millisecond
	nameUtf16, err := syscall.UTF16PtrFromString(*nameFlag)
	if err != nil {
		fmt.Printf("[MumbleLink Bridge] Invalid source name: %v\n", err)
		os.Exit(1)
	}

	var snapshot [mumbleSize]byte

	for ctx.Err() == nil {
		if !*quietFlag {
			fmt.Printf("[MumbleLink Bridge] Waiting for Guild Wars 2 (%s)...\n", *nameFlag)
		}

		// Guild Wars 2 opens (rather than creates) the named MumbleLink mapping,
		// so the bridge must create it via CreateFileMappingW. If another process
		// already created it, CreateFileMapping returns a handle to the existing
		// mapping with ERROR_ALREADY_EXISTS, which is also valid.
		hSrc, errCreate := syscall.CreateFileMapping(
			syscall.InvalidHandle,
			nil,
			syscall.PAGE_READWRITE,
			0,
			mumbleSize,
			nameUtf16,
		)
		if hSrc == 0 {
			if !*quietFlag {
				fmt.Printf("[MumbleLink Bridge] Failed to create/open source mapping %s: %v. Retrying in 1s...\n", *nameFlag, errCreate)
			}
			if !sleepContext(ctx, 1*time.Second) {
				break
			}
			continue
		}

		srcPtr, errMap := syscall.MapViewOfFile(hSrc, syscall.FILE_MAP_READ, 0, 0, mumbleSize)
		if errMap != nil || srcPtr == 0 {
			if !*quietFlag {
				fmt.Printf("[MumbleLink Bridge] Failed to map view of source memory: %v. Retrying in 1s...\n", errMap)
			}
			_ = syscall.CloseHandle(hSrc)
			if !sleepContext(ctx, 1*time.Second) {
				break
			}
			continue
		}

		srcBytes := unsafe.Slice((*byte)(unsafe.Pointer(srcPtr)), mumbleSize)
		if !*quietFlag {
			fmt.Printf("[MumbleLink Bridge] Connected to Guild Wars 2! Synchronizing data to %s...\n", targetPath)
		}

		var lastTick uint32
		var lastPid uint32
		var writeErrLogged bool
		now := time.Now()
		lastTickTime := now
		lastProcCheck := now

		for ctx.Err() == nil {
			curTick := readUint32(srcBytes, offsetTick)

			if curTick != lastTick {
				snapTick, ok := readConsistentSnapshot(srcBytes, &snapshot)
				if ok && snapTick != lastTick {
					if err := writeDest(destBytes, destFile, snapshot[:]); err != nil {
						if !writeErrLogged && !*quietFlag {
							fmt.Printf("[MumbleLink Bridge] Warning: Write failed on %s: %v\n", targetPath, err)
							writeErrLogged = true
						}
					} else {
						writeErrLogged = false
					}

					if *verboseFlag && snapTick%100 == 0 {
						pid := readUint32(snapshot[:], offsetProcessId)
						fmt.Printf("[MumbleLink Bridge] Tick: %d | PID: %d\n", snapTick, pid)
					}

					lastTick = snapTick
					lastTickTime = time.Now()
				}
				if !sleepContext(ctx, rateDuration) {
					break
				}
			} else {
				now = time.Now()
				if now.Sub(lastTickTime) >= idleThreshold {
					if now.Sub(lastProcCheck) >= procCheckPeriod {
						lastProcCheck = now
						pid := readUint32(srcBytes, offsetProcessId)
						if pid != 0 {
							lastPid = pid
						}
						if lastPid != 0 && !isProcessAlive(lastPid) {
							if !*quietFlag {
								fmt.Printf("[MumbleLink Bridge] Guild Wars 2 process (PID %d) terminated.\n", lastPid)
							}
							break
						}
					}
					if !sleepContext(ctx, idleDuration) {
						break
					}
				} else {
					if !sleepContext(ctx, rateDuration) {
						break
					}
				}
			}
		}

		// Cleanup source mapping and clear stale destination data before reconnecting
		_ = syscall.UnmapViewOfFile(srcPtr)
		_ = syscall.CloseHandle(hSrc)
		_ = writeDest(destBytes, destFile, zeroBuf[:])
	}

	if !*quietFlag {
		fmt.Printf("\n[MumbleLink Bridge] Shutting down cleanly...\n")
	}
}
