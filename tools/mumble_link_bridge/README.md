# GW2TacO MumbleLink Bridge for Linux

A lightweight, zero-overhead bridge running inside Guild Wars 2's Wine/Proton prefix. It synchronizes Guild Wars 2's Windows named shared memory (`MumbleLink`) to Linux's POSIX shared memory (`/dev/shm/MumbleLink`), allowing the native Linux build of **GW2TacO** to track player and camera coordinates in real time.

---

## Why is this needed?

- **Guild Wars 2** runs inside Wine / Proton and writes positional telemetry to a Windows named shared memory map (`CreateFileMappingW(..., L"MumbleLink")`).
- **Wine / Proton** manages named shared memory internally inside `wineserver` and does not automatically mirror it to Linux's `/dev/shm`.
- **GW2TacO (Native Linux)** runs directly on Wayland / Linux and reads positional data from `/dev/shm/MumbleLink`.
- **MumbleLink Bridge** runs inside the same Wine/Proton environment as Guild Wars 2, reads the Windows `MumbleLink` memory, and writes it directly to `Z:\dev\shm\MumbleLink` (the Linux host's `/dev/shm/MumbleLink`).

---

## Building

Because this bridge is written in pure Go without CGO dependencies, it can be cross-compiled for Windows directly on Linux:

```bash
cd tools/mumble_link_bridge
./build.sh
```

Or manually:
```bash
GOOS=windows GOARCH=amd64 go build -ldflags="-s -w" -o mumble_link_bridge.exe .
```

This generates `mumble_link_bridge.exe`.

---

## How to Run with Guild Wars 2

### Option 1: Steam (Proton) Launch Options (Recommended)

1. Copy `mumble_link_bridge.exe` to a permanent location (e.g. your GW2 folder or TacO folder).
2. Right-click **Guild Wars 2** in Steam -> **Properties...** -> **General** -> **Launch Options**.
3. Set the launch options to start the bridge in the background before launching the game:
   ```bash
   wine /path/to/mumble_link_bridge.exe & %command%
   ```
   *(Replace `/path/to/mumble_link_bridge.exe` with the actual path on your system).*

Alternatively, with **Protontricks**:
```bash
protontricks-launch --appid 1284210 /path/to/mumble_link_bridge.exe
```

### Option 2: Lutris

1. Open Lutris, right-click **Guild Wars 2** -> **Configure**.
2. Go to **System options** -> scroll down to **Pre-launch script** or **Command line arguments**.
3. You can run the bridge as a companion process inside the game prefix, or launch it via:
   ```bash
   lutris-wrapper ... wine /path/to/mumble_link_bridge.exe
   ```

### Option 3: Heroic Games Launcher / Bottles

Run `mumble_link_bridge.exe` within the same Wine prefix / Bottle as Guild Wars 2 using the "Run executable (.exe)" button or custom pre-launch script.

---

## Configuration & Flags

The bridge works out of the box with default settings, but supports the following command-line flags and environment variables:

| Flag | Default | Environment Variable | Description |
| :--- | :--- | :--- | :--- |
| `-dest` | `Z:\dev\shm\MumbleLink` | `GW2_MUMBLE_PATH` / `MUMBLE_LINK_PATH` | Path to the Linux shared memory file |
| `-name` | `MumbleLink` | - | Win32 memory map name created by Guild Wars 2 |
| `-rate` | `8` | - | Polling interval in ms when active (~125 Hz sync) |
| `-idle-rate` | `100` | - | Polling interval in ms when game is idle/loading |
| `-verbose` | `false` | - | Print periodic tick updates and game PID |
| `-quiet` | `false` | - | Suppress console output (clean background execution) |

---

## Features

- **Zero Overhead**: Direct memory copying with adaptive sleeping (~120 Hz when active, ~10 Hz when idle) uses < 0.1% CPU.
- **Auto Reconnect**: Detects when Guild Wars 2 exits and automatically waits for the next launch without needing to be restarted.
- **Graceful Shutdown**: Handles `SIGINT` / `Ctrl+C` cleanly unmapping views and closing handles.
- **Pure Go**: No external runtime or C compiler needed to build.
