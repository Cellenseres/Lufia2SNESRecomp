# Inactive HDMA native-wait admission - 2026-10-28

Counter waits previously rejected hdmaBeamOff=false even with no active or
pending HDMA channels. The consumer now checks actual channel ownership: MDMA,
active HDMA and pending HDMA remain excluded when applicable. Terminated but
still enabled HDMA channels remain excluded because frame initialization can
reload them. An inactive channel cannot arm itself during read-only polling.
No semantic decomp,binding,hook,clock arithmetic or1024-opcode limit changes.
The same bounded eight-channel pass serves both certified idle routes; there
is no added full function-table search,copy,allocation or cache invalidation.

524288 channel combinations pass with no state mutation; missing-active and
missing-pending guard controls are detected. Reversing only this admission
change restores the exact previous clock source. Existing event/refresh/APU,
observer and deadline certificates still bound every admitted original opcode.
Two1000-frame production sequences match every CPU/WRAM/device record,ten
complete hardware snapshots,544492/556258 fresh PCM samples and ordered HDMA
hashes. Field wait attempts drop9135472->26274,with18917 admitted batches;
menu attempts9096966->7574286,with3157 batches. APU calls/skipped cycles match
1387515/16867917(field) and1422214/17229394(menu); active menu HDMA stays guarded.

Three alternating fixed-affinity pairs use uninstrumented Windows production
objects,equal inputs,normal audio consumption and no artifact export. The1000-frame
headless field core+APU replay median falls6.5359357->0.2676739seconds. This is
neither graphical frame rate nor an ARM/console60FPS claim. Reserved Windows
Release passes. No standalone source changes or unrelated full regression;
tests,captures and diagnostic counters stay private.

# Dekompressor copy admission - 2026-10-28

The consumer reserves96 master cycles for an ordinary original MVN byte and192
for the final byte plus restore tail,instead of384 for both. Worst slow-ROM
instruction costs are52/146 plus40 refresh reserve. Original bus order,MVN
repetition,flags,stack and per-opcode device synchronization are unchanged.
No new semantic function,binding or hook is introduced.

Actual original-ROM ABI proof:8192 copy cases and next-instruction rejoins,
8 full mixed decompressions,4 overlapping-copy compositions,175 guard refusals,
and576 final raster/deadline cases(256 admit,320 refuse). The1000-frame
production replay matches every frame state,five complete hardware snapshots
and556258 fresh PCM samples. Actual raw interpreter instructions drop15814->15444;
MVN808F66 drops749->451. Wait/APU counters remain identical. Three alternating
fixed-affinity pairs use uninstrumented production objects with equal inputs:
reference/candidate medians6.1703576/6.2077594seconds. No runtime speedup or60FPS
claim follows. Reserved Windows Release passes; no unrelated full regression.
Standalone semantic code is unchanged. Tests/captures remain private.

# Capsule consumer checkpoint - 2026-10-28

Five original routines at82:FBA7/A233/A1D8/C170/C0AC are selected after complete
semantic verification. The standalone retains original stack/status restoration,
ordered inventory transfers, capsule availability and feeding child boundaries.
No new opcode hooks or changes to original waits are installed.

The shared original proof covers2560 direct+2560 composed cases,65 child sites
and unwinds,325 changed returns,35 unsupported entries,1024 backup edges and
four negative controls. Three strict compilers pass. Actual consumer ABI covers
3840 new cases and585 changed returns: roots0..3 reuse3072+360 unchanged cases;
the final feeding fixture supplies768+225, including four debug-feed sites.
Affected older menu bindings pass7680 cases+810 returns. Backup adds1536 timed
calls of588 original opcodes,30 refusals and640 HDMA admission combinations.
The final timing-plan replay admits32178 spans and refuses3150 unsafe contexts.

Two natural1000-frame sequences match every CPU/WRAM/device record, ten complete
hardware snapshots and556258/552971 fresh PCM samples. All five new bodies run
across those inputs. Feeding raw opcode counts fall747->146; selection25->20.
A separate identical active-HDMA input produces22 matching ordered transfers,
including source/value/master/beam timestamps, equal states and556401 PCM.
The consumer permits only WRAM-sourced,PPU-targeted HDMA in the proven backup
and initial feeding prefix. Per-opcode clock synchronization keeps HDMA live;
other MMIO phases retain their existing raster fences. No transfer is skipped.

Five alternating fixed-affinity cost pairs use uninstrumented production
objects,artifact export off and PCM consumption on. Reference/native medians
are6.4241173/6.4800721seconds,with noisy overlapping ranges; no speedup or60FPS
claim follows. Wait attempts/admissions match9096966/0 in the feeding replay;
APU calls/skipped cycles match1422214/17229394. This fixture does not establish
wait-batch throughput. Generated and linked APU idle seams remain present.
Eight capability gates,feature-off compilation and reserved Windows Release
pass. Tests and captures stay private. Unchanged original semantic proofs are
reused by source hash; no unrelated full regression is claimed for this batch.

# Main-menu consumer checkpoint - 2026-10-28

Nine original options, capsule and scenario-item routines are reconstructed;
the tenth entry exposes the existing scenario-item lookup. Their original-ROM
contracts pass5120 direct and5120 composed cases,90 child sites/unwinds,
450 changed returns,76 refusals,512 selectors and four negative controls.
Actual bridge ABI passes7680 cases and810 changed-return/unwind cases.

Production comparison exposed an existing unclocked82:8028 inline dispatcher.
The consumer now accounts its original16 opcodes under bounded admission;
3072 additional CPU/WRAM/ordered timed-bus cases pass. Internal dispatch spans
refuse active menu subscribers; the existing callback-aware compatibility route
is retained, with no new full timing proof for that legacy route. Standalone
semantics and original waits remain unchanged. No new opcode hooks are installed.

The1000-frame production replay has equal CPU/WRAM/device state records,
five byte-identical full hardware snapshots and552971 fresh PCM samples.
Reference/native raw opcodes in the new owner ranges are490/422. Both arms use
the common dispatcher correction; only the ten new bindings differ. Wait attempts
match at9103326; neither arm admits a wait batch in this replay. This proves
state equivalence here, not wait throughput or native execution of every path.

Three alternating fixed-affinity cost pairs use uninstrumented normal production
objects with artifact export off and PCM consumption on. Reference/native medians
are8.370539/8.360311seconds, too close for a meaningful speedup claim. No ARM or
hardware60FPS claim follows. Seven capability gates, strict compilers and the
reserved Windows Release pass. Tests and captures remain private; no unrelated
full regression is claimed. Prior rejected private samples are excluded.

## Member-menu source selection and continuations — 2026-10-27

Verified member status and spell-selection routines now have selected consumer
bridges at 82:9E67, 82:B2FE and 82:B370. The main-menu inline dispatch preserves
paired caller ownership when entering the two controllers. Child calls still
use original pushed frames, including regular long-call return ownership.

Twenty semantic prefix contracts expose the original operations between child
calls. Nineteen hooks are installed; a redundant backwards-branch hook is
omitted. Prefixes execute neither child JSR/JSL nor parent RTS. Their clocked
bus callbacks retain original fetches, data ordering and per-opcode accounting,
including the scheduler's progress bitmask. Admission bounds WRAM/ROM accesses
before the next HDMA/line or vertical event and device deadline. It checks each
executed opcode range through existing page chains; only the current exact
owned entry can be bypassed. Foreign hooks and observers still refuse batching.
No certified wait classification or APU synchronization seam is weakened.

Shared ROM/ABI verification and the final reserved Windows Release pass. The
actual nineteen-hook registry preserves all 252 other hooks in 353 checks.
Nine sampled hot wait PCs retain their hash-lookup lengths. In the natural
600-frame menu replay, new C operations replace 45 interpreted opcodes, leaving
66 original opcodes at unsafe boundaries. All frame-state hash records, fresh
PCM bytes and five complete hardware snapshots match. Wait counters match;
this menu fixture has zero admitted idle batches in both arms, so it does not
establish general wait acceleration. Both actual compiled APU idle calls and
the normal-source link are checked separately.

Three alternating fixed-affinity replay pairs have reference/native median
5.052/5.113 seconds and mean 4.931/5.733 seconds with private counters and PCM
consumption retained. Artifact export is disabled for these timing runs.
The sample is noisy and concurrent owner builds may contribute contention;
there is no demonstrated speedup or whole-game, ARM or 60-FPS acceptance.
The change reconstructs and admits original menu work; selection alone remains
insufficient performance evidence. See docs/decomp/VERIFICATION.md for scopes.

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

### Input-panel copy and child phases

The panel consumer repeats a six-opcode copy pattern using nine timing slots,
with at most 128 words per span. Existing page indexes certify the live EF68/
EF7C owner, reject foreign hooks and honor the 1,024-opcode budget, devices,
refresh and deadlines. The exact final writing opcode supplies quiescence
progress. Child-boundary plans retain palette MVN fetches and the 4,096-frame
input continuation. No wait classification or APU caller is replaced.

The 48 original-caller comparisons preserve CPU/WRAM, non-APU return devices,
complete natural frame-end devices and fresh PCM. Sixteen immediate APU states
remain differently aggregated until regular synchronization. Wait attempts and
admissions match. Native admission and compiled/linked APU callers are checked;
the extended-frame contexts exercise absolute catch-up, not the relative seam.

Matched desktop median copy intervals improve from 3,268 to 1,050 us and from
3,564 to 1,186 us. Whole-panel measurements remain noisy, and short resource/
append services do not show a consistent gain. These are caller intervals,
with counters and PCM consumption, not frame or hardware acceptance. Detailed
scope and costs are recorded in the verification document.

A private rollback comparison must also restore the complete extended APU
frame clock and equivalent audio consumer-ring history. An older guest blob
alone produced zero new PCM in a fresh process and was rejected. Restore every
relevant clock component; never treat silent output as sound verification.

### Card-column continuation admission

The original column algorithms also expose exact EA30/EAB4 loop boundaries.
The consumer chooses whole sprite or symbol-row iterations that fit before the
next frame/device boundary and deadline. CPU mode, stack ownership, DMA, IRQ,
NMI, refresh and APU synchronization remain guarded. The normal standalone
algorithms retain their original loops and bus behavior.

Admission uses the maintained opcode-page chains, checks the live owner and
rejects other hooks anywhere inside the interval. It does not scan the entire
hook registry. A bounded local timing array replaces per-opcode host re-entry;
no full memory or metadata copy is needed. Guest writes and opcode timing still
run in original order. The last writing opcode supplies the exact quiescence
progress index, including spans longer than 32 instructions. Existing wait-pair
classification and its 1,024-opcode budget are unchanged.

A rollback restore reinstalls hooks. Reference controls must be applied after
loading the state, and actual native counts must prove which arm ran. An initial
same-arm comparison was rejected for this reason. The final 224 distinct-arm
pairs match CPU, full WRAM, serialized devices and new PCM; wait batching is
unchanged. Actual generated/linked callers and both APU catch-up seams are checked.

The final eight-repeat desktop comparison uses matched sound, inputs and builds,
rotating old-build, hooks-removed and native arms. Hooks-removed/native median
caller intervals are 9.309/8.793 ms (DEB9), 97.096/90.612 ms (F4C6) and
15.788/14.560 ms (F648). Individual paired groups vary, including slower groups.
These are bounded caller intervals spanning guest work, not per-frame timings,
a whole-game result, an ARM result or hardware 60-FPS acceptance.

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

### Native opcode fetches and shared frame-clock conversion

Native sound checkpoints and fixed-span bridges may fold one to four discarded
opcode bytes through the existing LoROM fetch certificate. Admission excludes
observers, per-access timing, unsupported mappers, incomplete ROM storage and
bank-crossing fetches. A refusal leaves the CPU and cartridge latches untouched
for the ordinary byte callbacks. An accepted fetch supplies the original last
byte and cartridge bus address and charges every original transfer. Data
reads/writes, progress masks, opcode clock boundaries, APU work and refresh
accounting remain on their existing paths. This belongs to the consumer bridge,
not to the standalone semantic decompilation.

Sound opcode metadata builds a compact index once from its constant table,
keeping the first definition and the original linear fallback for extra pages.
Native CPU checkpoints execute on the game thread. The poll loop publishes
changed registers at its branch checkpoint and normalizes the first entry,
including a stale packed P byte; it omits only subsequent identity copies.
The verified semantic read/branch routines still execute for every iteration. Sound and fixed-span comparisons cover complete CPU
state, final bus latches, ordered data accesses, progress and clock accounting;
observer, latch, bank-boundary and transfer-count negative controls are required.

The platform APU-clock overlay is independent of interpreted SPC event batching.
Native-SPC consumers apply it after any DSP runtime overlay. Both the normal
guest-clock expression and the extended-frame header use the same exact reduced
integer conversion. Applying only the header does not optimize the normal clock.
Per-source compiler options and include order must survive this composition.

Controlled ARM boot comparisons reduced mean guest work from 108.316 to
97.989 ms with clock integration, then to 92.617 ms with native fetches and
metadata lookup. All three runs produced the same complete 45-frame saved
state. The fixed battle replay reduced guest work from 13.449 to 12.801 ms
with the native fetch changes. These improvements do not resolve the remaining
transition and battle budget violations, and do not establish hardware 60 FPS.

### Sharing background samples across screens

An edge sample may query main and sub screen together. Enable bits and windows
are resolved independently; tile address calculation and pixel decoding are
shared. The result mask identifies the opaque screens, and priority is meaningful
only when that mask is nonzero. The existing single-screen API keeps its output
and refusal contract, including transparent-pixel priority. This is a portable
PPU operation, not a console-specific rendering rule.

Qualified OBJ validation uses aligned word loads only when the actual row is
four-byte aligned. Halfword-aligned storage and odd row pitches retain the
unaligned-safe copy path. No cast may introduce an unaligned load or violate
aliasing rules. Accepted sprite effect ownership is reused between selection
and reverse tile fetching within one evaluator call; guest execution cannot
intervene, and OAM order and sprite-limit status remain unchanged.

The shared sampling and OBJ-load changes reduced warm battle draw time from
22.959 to 21.721 ms with identical inputs. Five native PICA images matched the
original CPU RGB reference, and the complete saved state was unchanged. This
is still an over-budget internal result. Additional phase scopes identify HUD,
frame-freeze, source and art work; sampled scopes must not be added to inclusive
parent times, and unmeasured zero fields are not evidence that work is absent.

A second matched ARM batch with IPO enabled reduced boot guest work from
86.548 to 85.202 ms by omitting identity poll stores, then to 84.581 ms with
the metadata index. Battle guest/frame costs were effectively unchanged in
that batch. Each GCC/Clang sanitizer comparison exercised 65,536 actual
semantic sound/frame-poll runs; negative controls detected missing initial
P normalization and missing publication of changed branch state. The full
3580-frame scene suite retained all ten per-frame guest/control counters.
IPO is a private console build experiment, not a change to game semantics or
a standalone-decomp requirement. These figures remain above the frame budget.


### Bounded hook lookup and short APU requests

The opcode registry retains its normalized keys, page membership and original
slot chains. A fixed 1024-slot index maps to at most 512 registered callbacks;
it adds 2048 bytes and performs no allocation. Replacement and reset update the
same registry contract, and a full table still rejects additional entries.
Neither lookup nor a cache hit grants permission to bypass another hook.

One-cycle APU requests use the existing exact cycle routine directly. Longer
requests retain idle-span discovery and its event boundaries. Ports, timers,
native program actions, DSP work and the return budget are not skipped.

Matched ARM boot work changed from 84.553 to 81.947 ms with the hook index,
then to 81.484 ms with direct single-cycle stepping. Warm battle guest costs
were effectively unchanged between the latter two variants (12.310/12.303 ms).
The complete boot and battle saved states and all ten per-frame guest/control
columns over the 3580-frame suite matched. Suite timings are excluded because
another build ran concurrently. Registry checks cover 262788 setter/lookup
comparisons per sanitizer compiler, including 512 colliding entries. APU tests
compare ordered actions, state and PCM with original cycle stepping; missing
cycle work and wrong callback-slot negative controls are detected. The changed
APU keeps full ASan/UBSan; only the unchanged upstream DSP oracle excludes its
known negative-shift UB in that harness. These are incremental host savings,
not a 60-FPS result or a solution to the remaining splash sound-upload cost.
## Build workspace and retention

Keep current implementation code, firmware integration and required build
scripts in the task's Git worktree. Numbered `.agent` experiments must never
hold the only copy. Save these changes on the private port branch before
removing their scratch copies. Port-only changes remain outside `main` until
separately authorized.

Reuse a stable workspace and incremental build directories. Retain the current
candidate and only references still needed for active comparisons. Before
deleting an older experiment, consolidate unique needed sources and reusable
checks, replace its path dependencies, and verify the retained inputs. Compact
historical evidence with revisions and hashes rather than keeping duplicate
source trees and compiler outputs indefinitely. Record reclaimed space.

Cleanup must verify absolute paths and affect only this task's obsolete files.
Preserve active or foreign builds, protected paths, user saves, toolchains and
needed assets. Never copy a ROM into snapshots, evidence archives or packages.


### Item and spell information windows — regression repair, 2026-10-26

The retained-renderer import d2e388e dropped the b7953a4 generated capture
renderer include and reverted c436fb1's live BG3 HDMA descriptor check to
shared F7..FB scratch. The old fixed include remained in the tree, but the
compiled caller no longer used it. A source file containing the fix therefore
did not establish that the current renderer applied it.

The common generated PPU now resolves its own main-OBJ window and clears only
hidden capture spans before menu composition. Both ordinary and retained early
returns use that same preparation; subscreen pixels, math and visible OBJ stay
unchanged. Native-index capture retains its existing window/material policy.
The retained generator moves the common call to its five complete/early-return
seams without duplicating the primary call. No additional render pass, PPU
clone, guest operation, hook or timing/batching admission is introduced.

Menu text again follows the live enabled direct BG3VOFS descriptor at 7E:80C0,
not scratch reused for WOBJSEL. The description's live direct WOBJSEL table at
7E:81C0 identifies the information view. Its glyphs can cross tile boundaries;
the consumer moves the original rendered text area intact and does not revive
inventory-row TEXT_REGION bindings from the same window ID. Existing inventory
geometry applies again after the information view closes. The structure cache
includes description activation and viewport position; indexes and row caches
remain enabled. Neither the ROM text nor original menu semantics are rewritten.

One affected checkpoint passes 208 native/wide/window/subscreen/layout cases,
28 HDMA descriptor guards and eight common-renderer/host-patch-off controls.
The reported hidden OBJ fixture goes from 131 opaque pixels to zero. Fast
colour admission is exercised separately; its original isolated reference and
the common renderer agree. These synthetic PPU cases model battle effects as
inactive and are not whole-game semantic or PCM evidence.

The final production-object replay covers 822 paired frames: 203 each for
native, wide default and wide shipped-layout item views, plus 213 spell-view
frames. CPU, WRAM, PPU-register/VRAM/OAM/CGRAM/DMA and freshly rendered PCM hashes
match before/after throughout. The native picture is unchanged. Three complete
RGB frames of the default wide centre equal native rendering, including the
description text. Selected entry and steady information frames were inspected.
The private spell fixture initially navigated right to STATUS; its corrected
SPELL route goes down. The three unchanged item arms were reused after that
fixture-only change, not represented as newly executed routes.

Actual production render-path observations, in microseconds (before/after
median among active information frames), are:

| Profile | Guest | PPU and menu | PCM rendering |
| --- | ---: | ---: | ---: |
| Wide default, 27 information frames | 391.8 / 411.5 | 1378.5 / 1384.4 | 5.9 / 6.2 |
| Wide shipped layout, 27 information frames | 390.6 / 411.4 | 1375.7 / 1375.0 | 5.7 / 6.1 |
| Spell layout, 9 information frames | 474.4 / 425.7 | 2097.1 / 1907.5 | 7.3 / 6.9 |

These are one matched replay per arm, with brief selected-frame fixture logging,
not statistically robust acceleration measurements. Guest binding selection,
wait/APU batching and timing sources are unchanged. The ordinary release uses
the common renderer; the optional fast colour path was checked independently.
A separate 48-pair warm PPU-only observation preserves cache hits (zero new
colour admissions during that warm interval); its prior admission control is
positive. Submission/waiting and hardware frame rates are not measured. No
60-FPS or console-performance promotion follows from this presentation repair.

Windows Release and the separately linked production replay pass. Unchanged
semantic, codec and whole-game regressions are not repeated. Tests, images,
fixtures and detailed source/output hashes stay private in the task worktree's
build/menu-info directory. The task branch is codex/menu-info-regression;
main, the semantic/platform pins and owner build/windows are untouched.

### Item/spell list scrolling — regression repair, 2026-10-26

Main 9cce334 integrates the information-window repair above.

The same d2e388e import also removed d9133c4's list-scrolling repair. Inventory
glyphs traverse twelve-pixel screen bands; deriving independently movable text
objects from eight-pixel tile rows splits names during the original animation.
List collection again follows the live viewport at WRAM 1532, with six fixed
screen bands, for either item or two-column spell lists. Description windows
retain their separate intact text region and suppress inventory bindings.

The complete twelve-pixel capture height is stored in each list text object.
Thus occupancy, source ownership, RGB composition, retained blit dirty rows,
primitive reuse and indexed claims inherit the same extent without additional
per-pixel work or separate flag caches. Existing eight-pixel layout destinations
retain their four-pixel scroll gap. Object IDs and the layout file format stay
unchanged. The structure key includes viewport position and active row HDMA.
No semantic decompilation, runtime binding, guest clock or APU seam changed.

One shared checkpoint covers 1,278 paired production frames: 317 each for native,
wide default and wide shipped-layout Items, and 327 for shipped-layout Spells.
The route scrolls down and back up, then opens/closes information. CPU, full
WRAM, PPU registers/VRAM/OAM/CGRAM, DMA and freshly rendered PCM hashes match
throughout. Native and default-wide pictures are unchanged; all 207 captured
default-wide RGB centres match native. Corrected authored-layout output changes
30 Item and 12 Spell frames. Selected before/after scroll images were inspected.
The replay also checks admitted retained blits against RGB composition and
admitted indexed list claims for their full twelve-pixel height. No GPU or
console execution is implied by these CPU-side plan checks.

Matched scrolling-interval medians (guest / PPU+menu / PCM microseconds):

| Profile | Before | After |
| --- | ---: | ---: |
| Wide default | 688.7 / 1959.8 / 15.0 | 704.3 / 1957.4 / 15.0 |
| Wide shipped layout | 597.2 / 1659.2 / 12.6 | 601.1 / 1649.9 / 12.7 |
| Spell shipped layout | 607.8 / 2320.3 / 13.7 | 618.7 / 2310.9 / 13.5 |

These are single matched replays, not a performance promotion. Submission and
waiting are unmeasured. Windows Release passes; unrelated semantic suites were
not repeated. Private sources, replay logs and hashes are in build/menu-scroll.
The initial private linker glob picked up obsolete split-bank objects; the
fixture now uses exactly the actual production link's input objects. The native
centre check compares RGB, since native and overlay alpha conventions differ.
The first two completed profiles were reused after correcting an overly strict
fixture expectation that even the unedited layout must change its picture.


## Effect slots after HDMA - 2026-10-28

Four existing effect-slot phases (81:8A7E/8AAA/8AC1/8B1B) now admit entry
after the scanline's HDMA event when their complete reserved span ends before
the line boundary. Previously the 1024-clock fence also rejected these late
entries. Entry before HDMA still requires the entire span to finish before
HDMA. Existing deadlines, refresh reserve, interrupt, DMA and CPU guards remain.
No new hook, binding, semantic function, opcode timing or guest speed changed.

The shared original-ROM checkpoint passes 9,216 spans and next-opcode rejoins,
246 preaccess guards, 5,760 prior temporal cases and 6,480 post-HDMA cases.
The old condition is rejected by the new admission negative control. A matched
1,200-frame synthetic actor-contact battle preserves every captured frame
state, five complete hardware snapshots and 670,463 fresh PCM samples.
Interpreted opcodes fall from 662,932 to 616,104; accepted slot-hook bodies
increase from 42,287 to 47,543. Wait batching and APU call counts are identical.

Three alternating fixed-affinity pairs without opcode/hook/APU profiling give
median core-plus-audio times of 8.2273365 seconds before and 8.3382124 after.
Results vary; this checkpoint establishes less interpretation, not a speedup
or frontend/console FPS. The uninstrumented binaries also match CPU, WRAM,
hardware and PCM. Reserved Windows Release and strict C11 checks pass.
Unchanged semantic algorithms and capability gates are reused; unrelated full
regression is deferred. Private evidence: runtime-frontier/effect-post-hdma.
