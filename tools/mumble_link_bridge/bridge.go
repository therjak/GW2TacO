package main

import (
	"context"
	"encoding/binary"
	"os"
	"strings"
	"time"
)

const (
	mumbleSize = 5460

	offsetTick      = 4
	offsetContext   = 1108
	offsetProcessId = offsetContext + 80 // 1188

	idleThreshold    = 500 * time.Millisecond
	procCheckPeriod  = 1 * time.Second
	maxSnapshotTries = 3
)

// normalizePath converts Unix or Windows paths into a valid Wine path pointing to the Linux host.
func normalizePath(p string) string {
	p = strings.TrimSpace(p)
	winPath := strings.ReplaceAll(p, "/", `\`)
	if strings.HasPrefix(p, "/") {
		// e.g. /dev/shm/MumbleLink -> Z:\dev\shm\MumbleLink
		return `Z:` + winPath
	}
	return winPath
}

// sleepContext waits for duration d or until ctx is cancelled.
// It returns true if the timer elapsed, or false if ctx was cancelled.
func sleepContext(ctx context.Context, d time.Duration) bool {
	timer := time.NewTimer(d)
	defer timer.Stop()
	select {
	case <-ctx.Done():
		return false
	case <-timer.C:
		return true
	}
}

// readUint32 reads a little-endian uint32 at the given byte offset.
func readUint32(buf []byte, offset int) uint32 {
	return binary.LittleEndian.Uint32(buf[offset : offset+4])
}

// writeDest writes a full MumbleLink frame to the mapped memory or fallback file.
func writeDest(destBytes []byte, destFile *os.File, data []byte) error {
	if destBytes != nil {
		copy(destBytes, data)
		return nil
	}
	_, err := destFile.WriteAt(data, 0)
	return err
}

// readConsistentSnapshot copies srcBytes into dst, verifying ui_tick did not
// change mid-copy so torn frames are not propagated.
func readConsistentSnapshot(srcBytes []byte, dst *[mumbleSize]byte) (uint32, bool) {
	for range maxSnapshotTries {
		tickBefore := readUint32(srcBytes, offsetTick)
		copy(dst[:], srcBytes)
		tickAfter := readUint32(srcBytes, offsetTick)
		if tickBefore == tickAfter {
			return tickAfter, true
		}
	}
	return 0, false
}
