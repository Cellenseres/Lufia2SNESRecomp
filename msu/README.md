# MSU-1 pack folder

Docs only. No ROM, patch or music data is committed here — the `.gitignore`
next to this file makes sure of it.

## Using a pack

MSU-1 is optional. With no pack you get the original SPC soundtrack, which is
the normal case, not an error.

Drop an unpacked Lufia II MSU-1 pack here:

```text
Windows/Linux/macOS   <folder holding Lufia2Recomp>/msu/
PlayStation Vita      ux0:data/Lufia2Recomp/msu/
```

A pack is a set of `<name>-<N>.pcm` tracks, usually with a `<name>.msu` beside
them. The name does not matter — whatever base most tracks share wins. On
desktop the folder is created on first run.

Tracks are 44.1 kHz signed 16-bit stereo PCM with the usual eight-byte header
(`MSU1` plus a 32-bit loop point in stereo frames). Headerless raw PCM works too.

Track number is the game's song id, so any pack following the common Lufia II
numbering fits. A missing `-<N>.pcm` just means the SPC plays that one song.

## Settings

`config.ini`, section `[Sound]`:

| Key | Values | What it does |
|---|---|---|
| `Msu1` | `auto` (default), `on`, `off` | `auto`/`on` use a pack if there is one, `off` always plays the SPC music. |
| `Msu1Dir` | path | Use this folder instead of the default. Empty = default. |

## Where the code is

The MSU-1 chip belongs to the shared SNESRecomp runtime
(`runner/src/snes/msu1.c`). This project has no MSU-1 core of its own and must
never grow one. The folder and settings policy lives in
`lib/snesrecomp-platform/src/msu_pack.c`; the driver that turns a song id into
a track is `src/lufia2_msu_driver.c`. No ROM is patched.

## Provenance

Packs are third-party. Read the credits of whichever one you download — this
repository ships none of it.
