package main

import (
	"bytes"
	"context"
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"
	"time"
)

func TestNormalizePath(t *testing.T) {
	tests := []struct {
		name string
		in   string
		want string
	}{
		{
			name: "unix shm path",
			in:   "/dev/shm/MumbleLink",
			want: `Z:\dev\shm\MumbleLink`,
		},
		{
			name: "unix path with surrounding whitespace",
			in:   "  /dev/shm/custom.link  ",
			want: `Z:\dev\shm\custom.link`,
		},
		{
			name: "windows path unchanged",
			in:   `Z:\dev\shm\MumbleLink`,
			want: `Z:\dev\shm\MumbleLink`,
		},
		{
			name: "windows path with forward slashes",
			in:   "C:/Users/player/MumbleLink",
			want: `C:\Users\player\MumbleLink`,
		},
		{
			name: "empty string",
			in:   "   ",
			want: "",
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			if got := normalizePath(tc.in); got != tc.want {
				t.Fatalf("normalizePath(%q) = %q, want %q", tc.in, got, tc.want)
			}
		})
	}
}

func TestReadUint32(t *testing.T) {
	var buf [mumbleSize]byte
	binary.LittleEndian.PutUint32(buf[offsetTick:offsetTick+4], 0x12345678)
	binary.LittleEndian.PutUint32(buf[offsetProcessId:offsetProcessId+4], 4242)

	if got := readUint32(buf[:], offsetTick); got != 0x12345678 {
		t.Fatalf("readUint32(offsetTick) = %#x, want %#x", got, uint32(0x12345678))
	}
	if got := readUint32(buf[:], offsetProcessId); got != 4242 {
		t.Fatalf("readUint32(offsetProcessId) = %d, want %d", got, 4242)
	}
}

func TestWriteDestMappedMemory(t *testing.T) {
	src := make([]byte, mumbleSize)
	for i := range src {
		src[i] = byte(i % 251)
	}
	dest := make([]byte, mumbleSize)

	if err := writeDest(dest, nil, src); err != nil {
		t.Fatalf("writeDest(mapped) unexpected error: %v", err)
	}
	if !bytes.Equal(dest, src) {
		t.Fatal("writeDest(mapped) did not copy source bytes identically")
	}
}

func TestWriteDestFallbackFile(t *testing.T) {
	path := filepath.Join(t.TempDir(), "MumbleLink")
	f, err := os.OpenFile(path, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0666)
	if err != nil {
		t.Fatalf("OpenFile failed: %v", err)
	}
	defer f.Close()

	// Advance file offset to verify writeDest always writes at offset 0.
	if _, err := f.Write([]byte("prefix")); err != nil {
		t.Fatalf("Write failed: %v", err)
	}

	src := make([]byte, mumbleSize)
	binary.LittleEndian.PutUint32(src[offsetTick:offsetTick+4], 99)
	binary.LittleEndian.PutUint32(src[offsetProcessId:offsetProcessId+4], 1337)

	if err := writeDest(nil, f, src); err != nil {
		t.Fatalf("writeDest(file) unexpected error: %v", err)
	}

	got, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("ReadFile failed: %v", err)
	}
	if !bytes.Equal(got, src) {
		t.Fatalf("file contents mismatch (len=%d, want=%d)", len(got), len(src))
	}
}

func TestReadConsistentSnapshot(t *testing.T) {
	src := make([]byte, mumbleSize)
	binary.LittleEndian.PutUint32(src[offsetTick:offsetTick+4], 777)
	binary.LittleEndian.PutUint32(src[offsetProcessId:offsetProcessId+4], 8888)
	src[mumbleSize-1] = 0xAB

	var dst [mumbleSize]byte
	tick, ok := readConsistentSnapshot(src, &dst)
	if !ok {
		t.Fatal("readConsistentSnapshot returned ok=false on stable buffer")
	}
	if tick != 777 {
		t.Fatalf("readConsistentSnapshot tick = %d, want 777", tick)
	}
	if !bytes.Equal(dst[:], src) {
		t.Fatal("snapshot bytes do not match source buffer")
	}
}

func TestSleepContext(t *testing.T) {
	t.Run("completes when duration elapses", func(t *testing.T) {
		ctx := context.Background()
		if !sleepContext(ctx, 5*time.Millisecond) {
			t.Fatal("sleepContext returned false, want true")
		}
	})

	t.Run("returns false when already cancelled", func(t *testing.T) {
		ctx, cancel := context.WithCancel(context.Background())
		cancel()
		if sleepContext(ctx, 1*time.Second) {
			t.Fatal("sleepContext returned true for cancelled context")
		}
	})

	t.Run("wakes early on cancellation", func(t *testing.T) {
		ctx, cancel := context.WithCancel(context.Background())
		go func() {
			time.Sleep(10 * time.Millisecond)
			cancel()
		}()
		start := time.Now()
		if sleepContext(ctx, 2*time.Second) {
			t.Fatal("sleepContext returned true after cancellation")
		}
		if elapsed := time.Since(start); elapsed >= 500*time.Millisecond {
			t.Fatalf("sleepContext took too long to cancel: %v", elapsed)
		}
	})
}
