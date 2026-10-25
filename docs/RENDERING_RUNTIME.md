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

### Battle scene decisions and native HUD bounds

An ordinary BG2 row needs no new visibility decision when its registers exactly
match the already saved scene and no title rows await processing. Title and
party rows are admitted first. The GPU consumer does not read the discarded
index scratch; subsequent RGB replay clears and regenerates its own indices.
Palette, OBJ and VRAM changes still reach the normal captured graphics path.

The GPU capture must also express the software renderer's game-specific layer
bounds. The battle HUD at tilemap `0x5c00`, character base `0x4000`, remains
native-width on BG2. Export zero margins for that layer, including restored HUD
scenes; retain authored BG1 margins and the other layers' existing policy.
Otherwise wrapped BG2 tiles can appear over the widescreen background.

The fixed 120-frame ARM comparison reduced mean draw time from 34.418 to
28.916 ms and total frame time from 50.976 to 45.473 ms before the bounds fix
(frames 11--120, normal helper pipeline, detail off, audio/widescreen on).
All 119 GPU jobs and five PICA readbacks were byte-identical for the decision
optimization. The bounds fix then removed the observed CPU/GPU margin mismatch:
all five unscaled PICA pictures matched software pixels, 383,040 pixels total.
Only qualified BG2 bounds and their hashes changed across the 119 jobs.
Complete saved state stayed identical to both GPU and software references.
GCC/Clang sanitizer checks covered 65,536 paired decisions and 102,400 viewport
cases each; a missing pending-title guard was deliberately detected.
These measurements remain scoped to that replay and are still above budget.

### Effect actor preparation

Effect actor ordering may be cached only during a certified read-only raster
interval or while an owned replay policy is bound. Keep the original group and
actor order. The dependent vertical candidate table also tracks OBJ size and
interlace. Uncertified calls retain the original live scan; IRQ boundaries,
policy rebinding, initialization and pixel restart invalidate the relevant
cache. GPU jobs never retain these borrowed actor pointers.

The same replay reduced mean draw time from 28.927 to 26.830 ms and total frame
time from 45.474 to 43.376 ms. All 119 GPU jobs, five PICA readbacks and saved
state matched the preceding reference. Each GCC/Clang sanitizer configuration,
with and without the timeline feature, passed 20,480 renderer pairs including
live writes, rebound policy addresses, signed coordinate edges, changing VRAM,
OBJ size and interlace. Missing interval and row-key invalidation mutants failed
as intended. A separate quiet-battle sprite-worker experiment gained only about
0.05 ms in this replay and was not adopted. No console 60-FPS claim follows.

Qualified battle RGB rows certify their actual OBJ pixels and effect ownership
before reuse. Actor/OAM metadata changes alone must not erase that certificate.
Keep first-admission and script-mode invalidation, and retain the existing
palette, VRAM, register, layout and qualified-OBJ checks. Separately rendered
HUD source rows still retain their conservative metadata invalidation.
The fixed replay changed mean draw time from 26.830 to 25.816 ms and frame time
from 43.376 to 42.362 ms; mean row hits rose from 2.618 to 7.855. All 119 jobs,
five PICA readbacks and saved state matched. Both sanitizer builds passed
24,580 dependency cases; removed ownership-mask and script-mode guards failed
negative controls. These are internal emulator timings, still above budget.

### Bounded native counter waits

Verified counter waits may batch up to 1024 original opcodes per host dispatch,
bounded by the caller's remaining step budget. Admission, original ROM bytes,
counter equality, observer rejection, interrupt/DMA guards and epoch-overflow
checks remain mandatory. Refresh reserve scales with the admitted budget;
each device and APU boundary still executes at its original opcode. This is
host scheduling, not a change to the semantic decomp or the game's wait time.

Edge distances outside the bounded span are rejected before division. Within
that span, a 32-bit quotient is sufficient; absolute guest clocks remain 64-bit.
The r55 ARM output eliminates the two 64-bit division sites in this clocked
path. The separate bus-accounted fallback retains its conservative limits.

The fixed battle replay reduced mean guest time from 15.828 to 13.803 ms and
frame time from 42.362 to 40.410 ms. The world-start replay reduced guest time
from 8.930 to 6.797 ms; pacing absorbed the gain in its total frame time.
Battle, world-start and the 600-frame boot replay each matched their complete
reference end state. The boot replay does not establish a whole-frame speedup.
Both GCC/Clang sanitizer runs passed 1,500,000 original event-ordered states
and 46,184,128 bounded-division cases. Missing refresh, epoch, APU-edge and
distance guards were detected by negative controls. Unchanged semantic tests
are reused; no new full decompilation regression or hardware timing is claimed.

### Preparing only consumed battle data

Consecutive rows from the same immutable raster source reuse its adjusted
registers and layer bounds. A changed opaque HUD state, restored scene source
or window-policy boundary still builds and compares the complete band. This
avoids copying and comparing the same 412-byte band on every unchanged row.

CPU OBJ pixels and their ownership mask are materialized only before an index
consumer, a pending HUD replay or an admitted RGB row. Register and palette
histories still advance on every row. The synchronous input callback is local
to that row, runs at most once and never escapes into a GPU job. Existing eager
callers keep their original interface. Sprite selection and guest status flags
are not repeated; the fallback capture remains unchanged.

The fixed battle replay reduced mean draw time from 25.811 to 24.405 ms with
interval reuse, then to 23.078 ms with demanded inputs. Total frame time changed
from 40.410 to 37.672 ms. Each stage retained all 119 serialized jobs, five
PICA readbacks, the blank-frame output and complete saved state byte-for-byte.
Both GCC/Clang sanitizer runs passed 4096 actual raster-builder comparisons and
65,536 eager/lazy/legacy HUD cases, including poisoned unused OBJ scratch.
Missing source, HUD coverage, window, index-input and pending-input guards,
and duplicate preparation, were rejected by negative controls. These are still
over-budget internal emulator results, not a finished performance release.
