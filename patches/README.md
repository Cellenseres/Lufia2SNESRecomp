# Maintained native game patches

Host-native replacements for interpreted hot paths, plus their contract tests.
Maintained input, not generated output: nothing is discovered or activated
automatically.

Every patch is off by default, has its own CMake option, and is pinned to the
reviewed core SHA256. A core or platform update stops configuration until the
contract is reviewed again. The fetched core and the generated game C are never
rewritten in place; `cmake/Lufia2NativePatches.cmake` adds one guarded seam to a
build-tree copy of the bridge.

## Patches

`LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD` — batches repeated equal/taken CMP/BEQ
pairs at `$83:900E`, `$86:8B4E`, `$85:EC96`, `$84:8D4F` and `$86:9752`, all with
ROM bytes `C5 40 F0 FC`. One pair and the deadline-crossing sequence stay with
the interpreter, so the resume PC and boundary decision are unchanged.

`LUFIA2_ENABLE_NATIVE_WAIT` — the earlier single-instruction payload for
`$83:900E` and `$83:9010`, kept for diagnosis.

`LUFIA2_ENABLE_ACTOR_EARLY_RETURN` — retires the 19-opcode idle prefix of
`$83:C7F8` and the actor-zero 17-opcode path of `$83:D508`. The dispatcher
beyond `$83:D59A` and every state-changing path stay interpreter-owned.

`LUFIA2_ENABLE_DMA_HOST_FASTFORWARD` — collapses the blocking `$420B` drain
loop, which advances no PPU, APU, beam, CPU or guest clock. Odd timers keep the
upstream path.

`LUFIA2_ENABLE_DMA_DIRECT_SOURCE_READ` — resolves WRAM and mapped LoROM
directly for A-to-B source reads. MMIO, SRAM, B-to-A and special carts keep
`snes_read()`.

`LUFIA2_ENABLE_QUIESCENCE_INDEX` — a candidate filter in front of the bridge's
64-slot state ring, keyed on PC and the read/write epochs. Collisions only add
candidates; the full comparison is unchanged. Desktop only, because the Vita
bridge overlay already filters the same ring.

## Guards

Entry requires the outer auto-quiescent scheduler with an active future
deadline, the documented CPU mode, exact ROM bytes, no pending or active
interrupt, no WAI/STP and no observer at the entry PC. Reads go through the
existing interpreter callback in hardware order. Clocks advance by the exact
retired total. Anything unrecognised uses the original interpreter.

## Runtime switches

An opted-in build defaults to the native path. These restore the original path
in the same binary; there is no automatic A/B switching. Invalid values select
the original path and print a diagnostic.

| Variable | Values |
|---|---|
| `LUFIA2_FRAME_WAIT_FASTFORWARD` | `0` |
| `LUFIA2_NATIVE_WAIT` | `0`, `1` |
| `LUFIA2_ACTOR_EARLY_RETURN` | `0` |
| `LUFIA2_ACTOR_D508_EARLY_RETURN` | `0` |
| `LUFIA2_QUIESCENCE_INDEX` | `0`, `1`, `auto` |

## Contract tests

`LUFIA2_ENABLE_PATCH_TESTS` links these tests and their command-line entries;
`LUFIA2_ENABLE_PATCH_REPORTING` turns on the periodic scheduler report. Both are
off by default and independent, so a measurement build can report without
carrying test code. The patches themselves are always built.

The tests are desktop-only: the Vita host has no command line to invoke them.
Reporting works on both.

Each test compares the patch against the actual linked `interp816_runOpcode` or
core DMA. They verify the instruction and mapping contracts only, not gameplay.
Host tools, invoked by flag and never run during play:
`--frame-wait-fastforward-selftest`, `--native-wait-selftest`,
`--actor-early-return-selftest`, `--actor-d508-early-return-selftest`,
`--dma-host-fastforward-selftest`, `--quiescence-index-selftest`.

## Source map

- `frame_wait.h`, `actor_early_return.h`, `actor_d508_early_return.h`,
  `dma_host_fastforward.h`, `quiescence_index.h`: the payloads.
- `native_patches.c/.h`: activation, counters, scheduler timing.
- `tests/`: one contract test per patch, named after it.
- `../cmake/Lufia2NativePatches.cmake`: the bridge seam.
- `../cmake/Lufia2QuiescenceIndex.cmake`, `../cmake/Lufia2DmaHostFastForward.cmake`:
  the pinned core overlays.
