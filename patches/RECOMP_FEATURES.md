# Lufia II Recomp features

Game-specific features appear in the launcher's **Settings → Lufia II Recomp
Features** category. Choices are saved when the launcher closes or launches
the game. **Restore Defaults** restores their defaults. Direct ROM launches
and `SkipLauncher` also read these choices.

| Setting | Default | Configuration |
| --- | --- | --- |
| Fix Sound menu corruption (Level 0 bug) | On | `[Lufia2Features] FixSoundMenu = 1` |

## Sound menu corruption

The supported US ROM's CONFIG menu dispatches left/right adjustments from
`$82:BF3F`. Its left table at `$82:BF52` selects `$82:B3AD` for the MUSIC row:
the last pointer overlaps the following instruction and reads bytes `AD B3`.
The right table correctly selects `$82:BFAD`, which toggles `$0B54` bit zero
and calls the original sound-mode routine at `$80:9623`.

`$82:B3AD` belongs to party manipulation, explaining the unrelated menu and
party corruption. The patch corrects the resolved pointer to `$BFAD` at
`$82:803D`, before the shared dispatcher stores it. The original handler,
sound update, redraw and timing continue normally. Both targets have the
same N/Z result, so all flags, widths and stack effects are retained.

The correction requires the exact supported dispatcher/table bytes, native
M16/X16, DP0, the CONFIG left-table pointer `$BF51`, row offset 7 and the
erroneous target `$B3AD`. Other rows, directions and callers are unchanged.
FastROM and slow-ROM mirrors are supported. Disabling the setting leaves
the original incorrect dispatch available in the same executable.

This is a consumer patch in `patches/lufia2_sound_menu.c`; the ROM file and
standalone semantic decompilation retain original behavior. It prevents new
corruption and does not restore party data already corrupted in a save.

The launcher addition uses a maintained CMake overlay on the fetched UI.
Neither the fetched UI nor Platform submodule sources are edited.
