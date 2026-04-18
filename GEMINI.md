# GW2TacO - Guild Wars 2 Tactical Overlay

## Project Overview
GW2TacO is an overlay designed for the MMORPG *Guild Wars 2*. It acts as a transparent window over the game client to provide players with tactical guidance, timers, and specialized tracking features without injecting code into or modifying the game itself. The project relies on the Guild Wars 2 "Mumble Link" API, which exposes real-time player positioning, map, and camera information directly to external applications.

## Technical Stack & Architecture
- **Language**: C++23
- **Build System**: CMake (cross-platform configuration, though currently Windows/x64 focused)
- **Toolchain**: Designed for MSVC, utilizing the latest C++23 Module capabilities (`.ixx` files).
- **Graphics/Rendering (`src/core2`)**: Direct3D 11 backend to handle 3D overlays, markers, and trails effectively within the 3D space aligned with the game's camera.
- **Custom UI Toolkit (`src/white_board`)**: A bespoke user interface library featuring CSS-like properties (`UI.css`), styling managers, window systems, buttons, text boxes, and more.

## Core Features
1. **Positional Overlay & Markers (`src/marker_data.cpp`, `src/gw2_pois.cpp`, `POIs/*.xml`)**:
   Provides in-game navigational cues such as paths, markers for collections, jump puzzle guides, and boss locations.
2. **Timers (`src/map_timer.cpp`, `src/locational_timer.cpp`, `maptimer.xml`)**:
   Tracks server-wide map meta events and provides location-specific timers.
3. **Mumble Link Integration (`src/mumble_link.cpp`)**:
   Reads shared memory to synchronize the overlay with the player's exact coordinates, map, and world state.
4. **Game Modes & Tracking Elements**:
   - **WvW (World vs World)** (`src/wvw.cpp`): Tactical information and tracking.
   - **Instanced Content** (`src/dungeon_progress.cpp`, `src/raid_progress.cpp`): Dungeon and Raid progress tracking.
   - **Trading Post Tracking** (`src/tp_tracker.cpp`).
5. **External API & Software Hooks**:
   - GW2 Web API bindings (`src/gw2_api.cpp`).
   - TeamSpeak 3 Integration (`src/ts3_connection.cpp`, `src/ts3_control.cpp`).

## Agent Instructions (AI Guidelines)
When contributing to or modifying this repository, observe the following rules:

- **Build System**: This is a CMake C++ project. All build dependencies, file groups, and property management must happen via `CMakeLists.txt`.
- **C++23 Modules**: The project heavily uses C++23 modules. 
  - When adding an `import` statement, **place it below all `#include` statements**.
  - Keep `import` statements **sorted alphabetically**.
- **Code Style**: Strictly adhere to the Google C++ Style Guide. Consistent formatting and modern C++ practices apply. Ensure any additions follow existing naming and architectural patterns. Use C++ modules instead of headers where possible.
- **Testing**: Use the **GoogleTest** framework for unit tests.

