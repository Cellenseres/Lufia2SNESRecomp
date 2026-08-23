# Lufia II Recompiled

A native recompilation of **Lufia II: Rise of the Sinistrals** using
[`snesrecomp`](https://github.com/mstan/snesrecomp).

The game boots from the original US ROM and supports the intro, title screen,
normal gameplay, battles, audio, input, saves and the desktop launcher.

## ROM

The ROM is not included. Use the clean, headerless US release:

```text
Size:   2621440 bytes
SHA1:   A89931C1F29B161B8BE717DFAB4A4ADB54B42B84
SHA256: 7C34ECB16C10F551120ED7B86CFBC947042F479B52EE74BB3C40E92FDD192B3A
```

Place it in the repository root as `lufia2.sfc`.

## Build

Requirements:

- Windows 10 or later
- Visual Studio 2022 with Desktop development with C++
- CMake 3.24 or later
- Python 3

Build from the command line:

```powershell
cmake --preset windows-release
cmake --build --preset windows-release
```

The executable is written to:

```text
build\windows\Release\Lufia2Recomp.exe
```

You can also open `lufia2.sln` and build the Release configuration in Visual
Studio.

## Run

Start `Lufia2Recomp.exe` to open the launcher, or pass a ROM directly:

```powershell
.\build\windows\Release\Lufia2Recomp.exe "D:\ROMs\lufia2.sfc"
```

Save data is stored in `saves\save.srm` next to the executable.

## Repository

- `recomp/` contains the control-flow information used during recompilation.
- `src/` contains the game host and runtime code.

See [ISSUES.md](ISSUES.md) for the current limitations.
