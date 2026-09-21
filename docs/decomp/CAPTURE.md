# Decomp research capture

This desktop-only development build records real interpreter call states for
decompilation work. It is separate from normal release builds and uses the
original/reference path for existing decomp replacements.

## Build

```powershell
cmake --preset windows-decomp-capture
cmake --build --preset windows-decomp-capture
```

Run:

```text
build/windows-decomp-capture/Release/Lufia2Recomp.exe
```

Press `Ctrl+Shift+F10` to start recording, play through the scene to inspect,
then press the same shortcut to stop. Recording also stops after 900 frames.

The resulting `captures/session-...` directory contains the summary, call
entry/exit states, WRAM snapshots and bus traces. Send the whole directory
(or a zip of it) when a decomp target needs real gameplay evidence.

Default tracked function entries are:

```text
83:C7F8
83:D508
83:C1B4
80:E365
80:E566
```

Override them without recompiling:

```powershell
$env:SNESRECOMP_DECOMP_CAPTURE_TARGETS="83C1B4,8383E0,8383EB"
```

Values are comma/semicolon separated 24-bit hexadecimal PCs. Up to 16 targets
can be tracked in one session.
