# GW2TacO - Guild Wars 2 Tactical Overlay

GW2TacO is a tactical overlay for *Guild Wars 2*. It acts as a transparent window over the game client to provide players with tactical guidance, navigational markers, timers, and specialized tracking features without injecting code into or modifying the game client. It communicates with the game using the Guild Wars 2 Mumble Link API to read real-time player position, map context, and camera orientation.

---

## Building on Linux

GW2TacO supports native Linux compilation using a modern C++23 toolchain with **Clang**, **Ninja**, and **CMake**, leveraging Wayland and Vulkan for overlay presentation.

### 1. Prerequisites & Dependencies

The build requires a compiler and build toolchain with C++23 Named Modules support:

* **CMake** >= 3.30
* **Ninja** (`ninja-build` — required for CMake C++23 module dependency scanning)
* **Clang / LLVM** (Clang 18+ with `clang++`, `clang-scan-deps`, and `llvm-ar`)
* **Git** and **pkg-config**

#### Distribution Packages

* **Debian / Ubuntu**:
  ```bash
  sudo apt update
  sudo apt install clang llvm ninja-build cmake pkg-config git \
      libwayland-dev wayland-protocols libxkbcommon-dev libcurl4-openssl-dev
  ```

* **Arch Linux / Manjaro**:
  ```bash
  sudo pacman -S clang llvm ninja cmake pkgconf git \
      wayland wayland-protocols libxkbcommon curl
  ```

* **Fedora**:
  ```bash
  sudo dnf install clang llvm ninja-build cmake pkgconf-pkg-config git \
      wayland-devel wayland-protocols-devel libxkbcommon-devel libcurl-devel
  ```

> [!NOTE]
> Additional dependencies (Vulkan Headers, Volk, Vulkan Memory Allocator, nlohmann_json, GoogleTest, and DirectXShaderCompiler Linux binaries) are fetched automatically via CMake's `FetchContent`.

---

### 2. Debug Build

A Debug build includes debugging symbols, disables compiler optimizations, and defines `_DEBUG` for runtime assertions and logging.

```bash
# 1. Configure the build with Ninja and Clang
cmake -B build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang

# 2. Build the project
cmake --build build-debug -j$(nproc)
```

The compiled binaries will be output to `x64/Debug/`:
* Executable: `x64/Debug/GW2TacO`
* Tests: `x64/Debug/gw2_tactical_test` and `x64/Debug/Base_Tests`
* Shared libraries: `x64/Debug/libdxcompiler.so` (copied post-build)

---

### 3. Release Build

A Release build enables compiler optimizations (`-O3`), activates Link-Time Optimization (`INTERPROCEDURAL_OPTIMIZATION_RELEASE`), and defines `NDEBUG`.

```bash
# 1. Configure the build for Release
cmake -B build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang

# 2. Build the project
cmake --build build-release -j$(nproc)
```

The optimized binary will be placed at `x64/Release/GW2TacO`.

---

### 4. Running Unit Tests

GW2TacO uses GoogleTest. Run tests across the build directory using CTest:

```bash
ctest --test-dir build-debug --output-on-failure
```

Or execute individual test binaries directly:
```bash
./x64/Debug/gw2_tactical_test
./x64/Debug/Base_Tests
```

---

### 5. Running GW2TacO on Linux

On Wayland compositors, launch the executable with the Vulkan backend enabled:

```bash
./x64/Release/GW2TacO -vulkan
# Or for the debug build:
./x64/Debug/GW2TacO -vulkan
```

If running steam under flatpak access to /dev/shm must be granted:
flatpak override --user --filesystem=/dev/shm com.valvesoftware.Steam

---

## Building on Windows

* **Compiler**: Visual Studio 2022 (MSVC v143 or newer) with C++23 standard support.
* Open the repository folder directly in Visual Studio via CMake integration or generate solution files:
  ```powershell
  cmake -B build -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Release
  ```
