# Lufia II Recompiled

A native recompilation of **Lufia II: Rise of the Sinistrals** using
[`snesrecomp`](https://github.com/mstan/snesrecomp).

The game boots from the original US ROM and supports the intro, title screen,
normal gameplay, battles, audio, input, saves and the desktop launcher. Video
output can use SDL or OpenGL 3.3.

## ROM

The ROM is not included. Use the clean, headerless US release:

```text
Size:   2621440 bytes
SHA1:   A89931C1F29B161B8BE717DFAB4A4ADB54B42B84
SHA256: 7C34ECB16C10F551120ED7B86CFBC947042F479B52EE74BB3C40E92FDD192B3A
```

Place it in the repository root as `lufia2.sfc`.

## Build

### Windows

Requirements:

- Windows 10 or later
- Visual Studio 2022 with Desktop development with C++
- CMake 3.24 or later
- Python 3

Build from the command line:

```powershell
git submodule update --init --recursive
cmake --preset windows-release
cmake --build --preset windows-release
```

The executable is written to:

```text
build\windows\Release\Lufia2Recomp.exe
```

You can also open `lufia2.sln` and build the Release configuration in Visual
Studio.

### Linux (including the Steam Deck)

Requirements: CMake 3.24 or later, Python 3, GCC or Clang, and the SDL3 build
dependencies. On Debian or Ubuntu:

```bash
sudo apt install build-essential cmake ninja-build pkg-config python3 \
  libgl1-mesa-dev libegl1-mesa-dev libx11-dev libxext-dev libxrandr-dev \
  libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxkbcommon-dev \
  libwayland-dev wayland-protocols libdecor-0-dev libdrm-dev libgbm-dev \
  libasound2-dev libpulse-dev libudev-dev libdbus-1-dev
```

```bash
git submodule update --init --recursive
cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
```

The executable is written to `build/linux/Lufia2Recomp`, with `libSDL3.so.0`
staged beside it and an `$ORIGIN` runpath, so the directory can be copied to a
Steam Deck as it is. SteamOS is immutable: build elsewhere and copy it over,
keeping the target glibc no older than the build machine's.

### Build options

Everything experimental or diagnostic is off by default, so a normal build is
silent and carries no measurement code. The native patches themselves are
production code and are always built; only their instrumentation switches:

| Option | Adds |
|---|---|
| `LUFIA2_ENABLE_RUNTIME_LOG` | map, MSU, widescreen and periodic state tracing |
| `LUFIA2_ENABLE_PATCH_REPORTING` | patch counters and scheduler time every 120 boundaries |
| `LUFIA2_ENABLE_PATCH_TESTS` | the contract selftests and their command-line entries |
| `LUFIA2_ENABLE_PERF_AUDIT` | the finite Vita performance audit |

Errors are always reported.

## Run

Start `Lufia2Recomp.exe` to open the launcher, or pass a ROM directly:

```powershell
.\build\windows\Release\Lufia2Recomp.exe "D:\ROMs\lufia2.sfc"
```

On Linux it is `./Lufia2Recomp` in the build directory, same arguments.

Save data is stored in `saves\save.srm` next to the executable.

## Repository

- `recomp/` contains the control-flow information used during recompilation.
- `src/` contains the game host and runtime code.
- `lib/snesrecomp-platform/` is the shared renderer and host submodule.

## Renderer

The launcher offers SDL accelerated, SDL software and OpenGL 3.3. OpenGL can
also use GLSL shader presets. If it cannot start, the game falls back to SDL for
that run.

See [ISSUES.md](ISSUES.md) for the current limitations.
