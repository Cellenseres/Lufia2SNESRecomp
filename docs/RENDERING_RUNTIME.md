# Shared rendering and runtime work

The consumer owns Lufia-specific layout decisions. `snesrecomp-platform` owns
portable SNES graphics operations, immutable frame data, resource versions and
presenter admission. Console SDK calls stay in the target adapters.

## Graphics

`snesrecomp::ppu` provides the SDK-free capture, tile, palette, object, Mode 7,
indexed-layout and composition code. `snesrecomp::platform` links it publicly,
so existing desktop consumers retain their target and symbols. A native
presenter reports PPU capabilities separately from ordinary image presentation.
Preparation, submission and cancellation belong to that presenter instance.
Unsupported effects and resource exhaustion must be rejected before submission.

The shared consumer helpers under `src` prepare menu, battle, intro and world
graphics without including a console SDK. Layout metadata and source-region
indices avoid repeated object searches. Row caches check the actual register,
VRAM, palette and qualified-object dependencies. Moving sprites, palette
animation and raster writes cannot be treated as an unchanged picture merely
because the scene identifier is unchanged.

`cmake/Lufia2PpuHost.cmake` and `cmake/Lufia2PpuOffload.cmake` are optional
consumer integrations. Adding the portable helpers does not select a native GPU
backend or bypass the ordinary desktop renderer. A port must explicitly enable
the appropriate generator and supply a backend with the required capabilities.
Borrowed PPU views are synchronous; deferred jobs own their recorded data.

RAM-resident layout preloading is explicit. Desktop layout editing retains its
periodic file refresh, and resetting a menu permits another refresh immediately.
`LUFIA2_ASSET_ROOT` optionally changes the asset root; without it, executable-
directory and current-directory lookup retain their existing behavior.

## Guest timing

The frame-counter poll optimization belongs to the host bridge, not the
semantic decompilation. It accepts only the reviewed CMP/BEQ byte sequences,
native hooks, matching CPU context, unchanged counter and supported hardware
state. The dispatcher supplies the remaining instruction budget. IRQ, HDMA,
observers, tracing and unsupported device states retain the ordinary path.
Clock aggregation ends at the original instruction that crosses a refresh,
beam or APU-flush boundary; the real handler still runs there. No guest wait,
audio handshake or simulated instruction budget is discarded.

The optional APU-program idle callback certifies the distance to its next
deferred access. The platform further limits the span at port writes, sample
generation, timer counter changes and interpreted opcodes. A missing callback
retains per-cycle native stepping. SPC execution and DSP synthesis are not
disabled by this optimization.

Older interpreted savestates can resume the native SPC driver only at its
verified empty-loop boundary with an exact uploaded-code/data signature.
Native state chunks take precedence; unknown driver states remain interpreted.
Joining the loop does not restart the driver or reset ARAM, timers or clocks.

## Integration validation

The combined integration was checked with GCC and Clang sanitizer runs over
the poll timing, dispatch budget, hook replacement and exact memory ranges;
portable PPU ownership, cache, trace, window and Mode 7 comparisons; battle and
menu pixel references; and SPC admission/lifecycle checks against the original
interpreter. Tests, ROMs, gameplay snapshots and proof tools remain private.
These checks establish the covered correctness contracts, not console frame
rates. Console adapters and hardware performance acceptance remain separate.
