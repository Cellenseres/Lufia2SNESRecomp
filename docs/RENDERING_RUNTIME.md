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

## Performance requirements for consumer integration

These requirements apply when reconstructing or binding new decomp routines,
changing host bridges, or composing platform overlays. Original-ROM fidelity
remains mandatory. Semantic verification and runtime performance are separate
properties: a verified C function can still be expensive when its bridge is
entered once per guest opcode, or can remain unused in its real caller context.

### Measured regression: Build70 to Build71

The controlled comparison used the same inputs, sound, custom DSP firmware,
widescreen and Azahar New3DS settings at 100 percent CPU and speed. Detail
profiling was disabled for the timing comparison. The tested source baseline
was Main `4dca46d`, with 2,161 selected verified bindings. The later Main
bindings were not part of that console measurement.

| Matched replay interval | Build70 | Build71 |
| --- | ---: | ---: |
| Logo wait, guest CPU mean | 14.75 ms | 4.67 ms |
| Counter wait, guest CPU mean | 195.05 ms | 9.64 ms |
| Same counter-wait interval, whole frame mean | 203.80 ms | 23.52 ms |

Two integration failures caused the regression:

- Several existing, verified `CMP $40 / BEQ` continuations were registered as
  native hooks but omitted from native batching admission. The hooks blocked
  the older route; the remaining native bridge dispatched individual opcodes.
  `Lufia2DecompFramePollIsCounterPair` now supplies the common classification.
  Classification alone grants no permission: both live hooks, exact ROM bytes,
  CPU mode, remaining budget, observers and device boundaries are still checked.
- The owner of generated `snes.c` omitted
  `snesrecomp_platform_patch_apu_catchup`. Registering a native SPC driver and
  idle callback therefore did not accelerate relative APU catch-up: the caller
  still invoked every individual cycle. The DMA/beam source owner now composes
  this optional platform seam exactly once. Main establishes the APU overlay
  before generating that source, so the composition is active on desktop too.
  Native-SPC-off builds retain the ordinary per-cycle caller.

The logo wait also scanned the entire growing hook table at every batch. Its
admission now follows the existing normalized opcode-page chain. The timing
implementation explicitly includes its own declarations, including the 64-bit
deadline return type.

The console candidate completed 3,580 automatic frames, including menus, world
map and battle fixtures. Native clock checks covered 1.5 million states with
each of GCC and Clang sanitizers. APU comparisons checked timer/port events and
PCM against individual stepping; deliberately broken controls were rejected.
The complete 600-frame snapshots before/after APU batching were byte-identical.
These are scoped correctness and emulator results, not proof of hardware 60 FPS.
World-map and battle PPU means remained about 34 and 53 ms in that suite.

### Rules for new work

1. **Preserve semantics in the decomp library.** Write readable gameplay C and
   reuse established helpers. Keep observable flags, widths, bus/MMIO ordering,
   child frames, waits and original quirks. Do not replace a bus-visible MVN
   with `memcpy`, delete NOP waits, or shorten a sound handshake for speed.
   Timing aggregation, direct-memory admission and caching belong to a separate
   consumer/platform optimization with an independently checked contract.
2. **Count host work at its actual frequency.** Mark polling loops, repeated
   continuations, child handoffs, bus callbacks and APU catch-up as hot-path
   risks. Record calls and cost, not just the number of compiled functions.
   A one-opcode native function can leave thousands of expensive dispatcher
   round trips per frame. Prefer a certified run of existing semantic steps
   over repeated host re-entry when the original event boundaries permit it.
3. **Check real admission after adding hooks.** A new hook is also a possible
   observer that disables an existing fast path. Inspect effective hook
   registration, replacements, resets and supported CPU/hardware modes.
   Keep identical wait-site classification in one place. Never whitelist a
   merely similar loop without its ROM/semantic contract, and never bypass
   arbitrary hooks to recover speed. NMI/IRQ callbacks must be checked in
   their real ownership and stack context, not only an ordinary-call fixture.
4. **Check the compiled caller, not only the API.** At the batch checkpoint,
   inspect the generated translation units and linked symbols for every changed
   overlay/binding seam. A native driver, GPU capability or idle callback being
   present does not establish that its caller uses it. Compose a fetched source
   through its existing owner; preserve optional/feature-off builds and reviewed
   source hashes. Do not edit fetched sources or weaken guards to make a build
   accept a changed source.
5. **Avoid repeated host bookkeeping.** Use the maintained hook/page indexes
   rather than whole-registry scans. Index immutable layout/ROM metadata at
   load time. Reuse tiles, palettes, geometry and resources according to their
   actual dependencies; do not hash/copy full memories or search all objects
   per pixel/opcode without measured justification. Preserve invalidation on
   writes, reset, scene changes, palette animation and rotating GPU slots.
   Do not turn an observable bus read into an unchecked raw pointer access.
6. **Keep graphics and audio costs visible.** Rendering should consume prepared
   immutable tile/OBJ/primitive data; declaring GPU output while also generating
   a full CPU RGB frame is not a speedup. CPU fallback must remain exact without
   repeating guest execution or sprite/status side effects. DSP synthesis does
   not eliminate ARM-side SPC timers, port queues or synchronization. Their cost
   may be inside `guest_ms`; a small audio-submit time does not rule it out.
7. **Measure changes in a controlled batch.** Reuse the existing validation
   cadence: run the affected union once, not the entire suite per function.
   For changed hot bindings, compare an appropriate fixed replay or bounded
   production-context benchmark. Record baseline/candidate identities, active
   native/fallback routes, calls, guest/bridge/APU/PPU work, submission, waits
   and whole-frame time with matching compiler and runtime settings. Disabled
   detail fields are unmeasured, not zero cost. Resume PCs are not complete
   function profiles. Reuse proofs only for unchanged relevant inputs.
8. **Gate performance claims on the result.** Keep functional, ABI, actual
   runtime-admission and performance evidence distinct. Do not count newly
   selected functions as proven runtime acceleration. Preserve native guest
   semantics and exact APU/PCM behavior when batching; unchanged pictures alone
   are insufficient. Do not hide a regression by disabling sound, effects,
   widescreen or new bindings, skipping frames or accelerating game time.
   The release goal remains p95 work <= 14 ms and p99 <= 16.7 ms in required
   scenes. An unmeasured or over-budget scene is not a 60-FPS pass. Emulator
   and hardware acceptance remain separate.

Inspect emitted ARM code when a measured kernel suggests expensive division,
copies or missed inlining. Use proven ranges and benchmark the whole result;
neither narrower arithmetic, assembly nor compiler flags guarantee a benefit.
In the Build71 investigation, narrower clock arithmetic reduced CPU time but
barely changed whole-frame time. That experiment was excluded from delivery.

At a shared checkpoint, document which hot paths changed, whether native
admission is actually exercised, the before/after costs, preserved contracts,
and checks run/reused/deferred. Keep remaining performance work explicit without
changing a correct semantic function's verification status to describe speed.
Tests, proof tools, gameplay snapshots and raw captures remain private.

### Raster policy validation intervals

The serial PPU timeline exposes a guest-thread-only read-only epoch. Its HDMA
walker reads source tables and writes PPU registers; guest execution is paused
between interrupt callbacks. The epoch is suspended before each callback,
renewed afterwards and cleared on exit. Reentrant walks reject before doing
device work. GPU workers must not consume this guest-thread certificate.

Battle capture compares its WRAM policy once per admitted interval, after every
IRQ, and on every call outside this timeline. A mismatch retains the existing
partial-frame fallback. The policy snapshot and its pixel dependencies do not
change; this avoids repeating the same comparison for every scanline.

Build72's isolated 120-frame battle comparison against Build71 kept audio,
widescreen, inputs and the simulation unchanged. Without detail profiling,
the measured mean draw time was 37.994 to 34.418 ms; frame time was 54.548 to
50.976 ms (frames 11--120). Guest, upload and draw-command counters matched,
and the complete 324,186-byte saved end state was byte-identical. Serial detail
profiling separately reduced policy checks from about 222 to one per frame.
Both compilers' sanitizer checks covered 452 paired IRQ/timeline scenarios,
including policy mutation, standalone checks and a rejected stale-epoch mutant.
These are scoped emulator results, not hardware timing or a 60-FPS release.
