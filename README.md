# CTR Crowd Control

Play **Crash Team Racing (PS1, 1999)** with [Crowd Control](https://crowdcontrol.live/).

This uses the native PC port of CTR, [ctr-native](https://github.com/CTR-tools/ctr-native), with a Crowd Control client integrated into the engine itself. A TCP connection is opened between the executable and the Crowd Control desktop app to send effects.

A huge thanks to [CDRomatron](https://github.com/CDRomatron) (the original creator of CTR Crowd Control) and [Faradise](https://github.com/FRDS) (who took over the project from CDRomatron) for being so helpful and willing to answer questions! Links to their CTR Crowd Control projects are in the Credits below.

- For steps to play, see the Quick Start guide below.
- For technical info / further development, see [docs/CROWD_CONTROL.md](docs/CROWD_CONTROL.md).

## Quick Start: Playing with Crowd Control

Read this if you've downloaded a release build and want to get up and running.

1. **Set up the game:** Unzip the release. Create a folder named `assets` next to the executable, then in that folder, add your **NTSC-U** copy of Crash Team Racing named `ctr-u.bin`:
   ```
   CTR-Crowd-Control/
     ctr_native.exe
     assets/
       ctr-u.bin
   ```

2. **Load the pack:** Install the [Crowd Control desktop app](https://crowdcontrol.live/) and the [Crowd Control SDK](https://github.com/WarpWorld/CrowdControl.SDK/releases), then point the SDK at `pack/CrashTeamRacingNativePack.cs` (included in the release) and load it as a local pack.

3. **Launch:** With the desktop app running and the pack loaded, run:
   ```
   ctr_native.exe --crowd-control
   ```
   The game connects to the app automatically. **That's it!**

*If running Crowd Control on a non-standard host or port, see [docs/CROWD_CONTROL.md](docs/CROWD_CONTROL.md) for notes on `--crowd-host` and `--crowd-port` flags.*

Majority of the README below this point is the original `ctr-native` README with a few notes added about CrowdControl structure, and updated Credits.

## Philosophy

- **No byte budget.** Game source lives in `game/` as our own copies. Edit freely.
- **No PSX toolchain.** Targets Windows and Linux with SDL3. No MIPS compiler needed.
- **Clean platform layer.** `main.c` owns process startup; host details stay in `platform/native_*`.
- **No build system nonsense.** Just `build.bat` / `build.sh`.
- **Fully static build.** Single executable, zero dependencies. SDL3 is compiled from vendored source and linked statically.
- **Crowd Control as ordinary game code.** As much as possible, effects are typed with named structs and enums that the decomp provides, never hardcoded PS1 RAM addresses.

## Directory Layout

```
ctr-crowd-control/
  main.c              Entrypoint and native platform boundary
  platform/           Native-owned audio, input, memcard, CD, and PSX facade glue
  crowd/              Crowd Control transport, effect runtime, and effect handlers
  include/crowd/      Crowd Control public headers, including the effect table (crowd_effects.h)
  pack/               Crowd Control desktop app effect pack (C#)
  tools/crowdcontrol/   Dev harness and pack/header sync checker
  build-msvc.bat      Windows build (MSVC x86)
  build.bat           Windows build (MinGW i686)
  build.sh            Linux build
  CMakePresets.json   Shared CLion/command-line CMake configurations
  README.md           This file
  game/               Our copies of all decompiled game source (943 files)
    game_unity.h      Ordered unity include chain for all game source files
  include/            Project headers (structs, globals, declarations, platform facade)
  externals/
    SDL/              SDL3 source (static build)
```

## Building from source

### Windows

The recommended native Windows toolchain is MSVC x86:

1. Install Visual Studio 2022 or Visual Studio Build Tools 2022.
2. Select the **Desktop development with C++** workload and a current Windows SDK.
3. Ensure CMake 3.20 or newer is on `PATH` (standalone or the Visual Studio C++ CMake tools component).
4. Run `build-msvc.bat`, or select the `windows-msvc-x86` CMake preset in CLion.

The existing MinGW i686 build remains supported:

1. Install [MSYS2](https://www.msys2.org/).
2. In an MSYS2 terminal:
   ```
   pacman -Syu
   pacman -S --needed git mingw-w64-i686-gcc mingw-w64-i686-cmake mingw-w64-i686-make
   ```
   If the update asks you to close the terminal, reopen MSYS2 and run the install command.
3. Add `C:\msys64\mingw32\bin` to your system PATH
4. Open a new Command Prompt or PowerShell and run `build.bat`.

That's it. SDL3 is compiled from vendored source -- no separate install needed.

### Linux (Debian/Ubuntu)

```
sudo apt install gcc-multilib
sudo apt install libx11-dev libxext-dev libgl1-mesa-dev libasound2-dev libudev-dev libdbus-1-dev
```

### Build commands

```
build-msvc.bat       # Windows, MSVC x86 (recommended)
build.bat            # Windows, MinGW i686
chmod +x build.sh
./build.sh           # Linux
```

The shared CMake presets can also be used directly or selected as CLion CMake profiles:

```
cmake --preset windows-msvc-x86
cmake --build --preset windows-msvc-x86-debug
ctest --preset windows-msvc-x86-debug
```

`ctest` runs `tools/crowdcontrol/gen_pack.py --check`, which fails the build if [`pack/CrashTeamRacingNativePack.cs`](pack/CrashTeamRacingNativePack.cs) has drifted from [`include/crowd/crowd_effects.h`](include/crowd/crowd_effects.h).
See [docs/CROWD_CONTROL.md](docs/CROWD_CONTROL.md) if you're adding or changing an effect.

First build compiles SDL3 from source. This is cached as a static library in the selected build directory.

Output:

- MSVC: `build-msvc-x86/Release/ctr_native.exe`
- MinGW: `build/ctr_native.exe`
- Linux: `build/ctr_native`

The Crowd Control integration builds in by default. If this isn't desired, add the flag `-DCTR_CROWD_CONTROL=OFF`.
Note that Crowd Control code never affects the PSX-matching build path.

### Clean build

```
rmdir /s /q build    # Windows: delete cached libraries
build.bat            # Windows: rebuild everything

rmdir /s /q build-msvc-x86
build-msvc.bat       # Windows MSVC: rebuild everything

rm -rf build/        # Linux: delete cached libraries
./build.sh           # Linux: rebuild everything
```

## Running

### Normal Setup

If you downloaded a release build, you only need two things for normal play:

1. The game executable:
   - `ctr_native.exe` on Windows
   - `ctr_native` on Linux
2. Your own NTSC-U retail CTR disc image, named (put in directory called `assets`):
   - `assets/ctr-u.bin`

Example:

```
CTR-Crowd-Control/
  ctr_native.exe
  assets/
    ctr-u.bin
```

Then run `ctr_native.exe` (add `--crowd-control` to enable the Crowd Control integration).

The disc image must be the common single-track raw PSX BIN layout: MODE2/2352 sectors, with the data track starting at byte 0. A cooked 2048-byte `.iso` does not preserve the XA/STR sector data needed for audio and video playback.

For development builds run from `build/`, put the same `assets/ctr-u.bin` next to the source tree:

```
ctr-crowd-control/
  build/
    ctr_native.exe
  assets/
    ctr-u.bin
```

### Extracted Asset Override

You do not need extracted assets for normal play.

Extracted files are still supported for development, modding, and debugging. If present, they override files from `ctr-u.bin`.

Extracted-asset override structure:

```
CTR-Crowd-Control/
  ctr_native.exe
  assets/
    BIGFILE.BIG
    SOUNDS/KART.HWL
    TEST.STR
    XA/
      ENG.XNF
      ENG/EXTRA/S00.XA ... S05.XA
      ENG/GAME/S00.XA ... S20.XA
      MUSIC/S00.XA ... S01.XA
```

The full extracted asset list is:

- `BIGFILE.BIG`
- `SOUNDS/KART.HWL`
- `TEST.STR`
- `XA/ENG.XNF`
- `XA/ENG/EXTRA/S00.XA` through `S05.XA`
- `XA/ENG/GAME/S00.XA` through `S20.XA`
- `XA/MUSIC/S00.XA` through `S01.XA`

## Bug Replays

Internal builds can record a small bug report folder. See `docs/REPLAYS.md`.

## Architecture

```
main.c (entrypoint)
  |
  +-- platform/native_* (platform shell, audio, input, memcard, CD, renderer, PSX facade glue)
  |
  +-- crowd/ (Crowd Control transport, effect runtime, effect handlers)
  |
  +-- game/game_unity.h
        |
        +-- game/ (all decompiled game source)
              |
              +-- include/ (headers: structs, globals, declarations)
```

- `CTR_NATIVE` is defined for native host/platform-specific code
- `CTR_CROWD_CONTROL` is defined when the Crowd Control integration is built in (default)
- First-party native code targets portable C17 with compiler extensions disabled
- The default build uses 32-bit mode while remaining PSX address-shaped data and host-pointer contracts are audited. GPU primitive links are bridged through 24-bit native tokens; see `docs/MEMORY_MODEL.md`.

## Roadmap

- Clean up `game/` copies strip byte budget hacks and route platform-specific code through `CTR_NATIVE`
- Keep reducing 32-bit host-pointer assumptions in PSX-shaped data, and keep pruning inherited compatibility code now owned in `include/` and `platform/`.

## Credits

- [CTR-tools/ctr-native](https://github.com/CTR-tools/ctr-native) — the native port this fork adds Crowd Control support to
- [CTR-ModSDK](https://github.com/CTR-tools/CTR-ModSDK) — the decompilation project `ctr-native` is built on
- [PsyCross](https://github.com/OpenDriver2/PsyCross) — original PS1 compatibility code from which parts of CTR Native's owned platform layer and PsyQ facade headers are derived
- [SDL3](https://github.com/libsdl-org/SDL) — cross-platform multimedia
- [Crowd Control](https://crowdcontrol.live/) — the platform and SDK this integration targets
- [CDRomatron/CrashTeamRacing-CrowdControl](https://github.com/CDRomatron/CrashTeamRacing-CrowdControl) — the original Crowd Control for CTR
- [FRDS/CrashTeamRacing-CrowdControl](https://github.com/FRDS/CrashTeamRacing-CrowdControl) — a fork of the original Crowd Control for CTR
- Crash Team Racing is a trademark of Sony Computer Entertainment / Naughty Dog
