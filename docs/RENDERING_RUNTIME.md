## World-session branch continuation - 2026-11-05

Ordinary branches into `86:925C` do not offer a call binding. The checked
interpreter boundary now resumes the existing semantic world-session loop
without a fabricated guest call frame. The standalone session and continuation
share one game body; this adds no algorithm or canonical entry. The consumer
supplies separate fetch/internal/data clocks and preserves original child calls.

Admission requires the actual global CPU, original saved stack, DB/DP/widths,
binary mode, synchronized beam/refresh, device/deadline room and a quiet indexed
parent span. Pending interrupts, host interrupt context, nested interpreter
owners, explicit stop PCs, tracing and global/foreign observers retain their
original route. Child boundaries recheck admission. Active owner handoffs,
deadlines, consumable SKIP1 returns and actual nonlocal town exits retain their
stack, landing and APU ownership. Unknown hooks are never cleared or ignored.

The longest parent prefix is 288 SlowROM master clocks: a non-centered row takes
the longer branch, including the repeated-frame BRA. At most 40 refresh clocks
raise it to 328. The existing sound-window policy reserves 384 and uses the real
1024 HDMA fence. Visible active/pending HDMA permits hPos at most 639; the separate
800 cap also stays before the shortest scanline boundary. Original HDMA, waits,
MMIO and sound work remain enabled.

The shared checkpoint includes 4,109 ROM/guard cases per compiler, 5,546 actual
admission/refresh cases, independent branch/freshness/landing controls and six
CMake availability plus four composition checks. Agent Release and API-off actor
compilation pass. The generated target contains one interpreter owner with one
world seam; actual linked production actor/interpreter objects preserve full
state, extended APU clocks, PCM, picture and native APU idle counts across 800
paired frames. Additional 720 observer and 360 controlled SKIP1 frames retain the
same contracts. Child RTS model cases do not substitute for these runtime proofs.

Instrumented admission witnesses report 211/135/29 native continuations in the
360/332/108-frame world, entry and exit replays. Actual production profiles
retire fewer original opcodes; unchanged foreign/global controls refuse the seam.
Six alternating 4,320-frame same-core cost pairs remove opcode instrumentation,
keep DSP synthesis/draining and omit sample hashing in both arms. Their final
median candidate/reference ratio is 0.995; individual ratios span 0.982 to 1.053.
An earlier exploratory sample looked faster. The final spread establishes no
reliable speedup or whole-renderer/FPS acceptance. Performance remains unproven.
Full unrelated semantic regression and other-platform builds are deferred.

Private receipt: runtime-frontier/world-continuation-current/ACCEPTANCE.json.


## Effect-slot counter admission

The existing effect-slot callback now also admits the original two-op countdown
at `81:8AB4/8B25` when the preceding cursor phase crossed a device boundary.
Read-only timing rows, narrow NZ transfer and the indexed registry avoid another
dispatcher or whole-state copy. Existing full slot batches remain eligible;
unknown hooks, observers and insufficient clock windows retain original stepping.

Actual production objects remove 8,492 battle and 6,900 Strong opcodes, preserving
full guest/device state, APU clocks, PCM and picture. Foreign/global witnesses
remain identical. Three alternating uninstrumented battle pairs measured
reference 1325/1328/1278 ms and candidate 1205/1218/1200 ms for 3,491 guest frames
plus PCM draining. Drawing/setup are excluded. These local samples establish
neither a general speedup nor hardware FPS. No new canonical function is counted.
Private proof: `runtime-frontier/effect-completion-current/ACCEPTANCE.json`.

# Effect slot-start integration — 2026-11-03

The effect engine reuses its existing semantic slot-start body at `81:8A8C`
and `81:8ACF`. These bounded 4/6/23-opcode continuations preserve original MMIO,
stack and per-op clocks. They add no canonical decomp function or algorithm.
Indexed ownership, remaining batch budget and hardware/observer guards keep
foreign hooks and certified waits intact. Existing preparation admissions and
all measured raw contexts outside the new spans are unchanged.

Hardware words must retain their paired consumer bus service. Reading the
multiply result as two independent hardware bytes preserved PCM here but
incorrectly doubled one legacy pacing credit. Full-state comparison detected
this; the accepted path uses `Lufia2DecompFrameReturnReadWord` and preserves
both original bus bytes. Do not normalize or ignore a pacing-counter mismatch.

Original opcode counts decrease by 7,046 in the 984-frame battle segment and
3,662 in the 792-frame Strong segment, with full guest/device/APU/PCM equality.
Actual Release-object replay, foreign/global hook controls and the shared
MSVC/Clang ROM/ABI checkpoint pass. This measures interpreter removal; it does
not establish a causal wall-time speedup or hardware 60 FPS. Private receipt:
`.agent/decomp/runtime-frontier/effect-slot-begin/ACCEPTANCE.json`.

# Effect-dispatch admission and observers — 2026-11-03

The existing `$81:8A95` and `$81:8B08` preparation spans now reserve their
actual FastROM cost, including the unchanged refresh allowance. SlowROM bounds
remain 288/278 master clocks; FastROM bounds are 256/250. Both code and the
admitted bank-$95 stream use the same ROM access speed.

Admission also checks the existing indexed hook ranges. Only the exact owned
entry callbacks may be crossed; foreign hooks and global observers retain
original execution. No new opcode hook, binding or semantic API is installed.

A normal-input 3,491-frame replay raises admitted preparations from 2,565 to
2,654 out of 3,269 attempts. Its final 984-frame action segment interprets
254,049 rather than 254,850 opcodes. Guest state, clocks, PCM and native APU
idle accounting match. With foreign middle-of-span hooks, all 3,269 callbacks
match the original-instruction control; the old source skipped 2,565.

Three alternating production-interpreter cost pairs measured frame simulation
and PCM drain/hash, excluding drawing and setup: reference 1495.028/1201.301/
1428.700 ms, candidate 1295.315/1198.049/1273.294 ms. Variance prevents an
attributed speedup or FPS claim. Standalone decomp semantics remain unchanged.
Private evidence: `.agent/decomp/runtime-frontier/effect-dispatch-banks/`.

# Filled battle windows — 2026-11-03

The missing original `$81:E38F..E3AD` draws the `$87B7` window with fill tile
`$2150`. It shares `FilledWindow` with `$E3AE/$E3CD`, retaining each original
child frame. This adds one original variant rather than another algorithm.

The same batch fixes five window ADC operations that formerly used binary
addition when the decimal flag was set. Original decimal behavior now uses the
existing CPU arithmetic helper; no waits, bus accesses or original quirks change.
Strict MSVC/Clang each pass 6,144 semantic cases across the three variants,
including live DP and both decimal states. Each also passes 8,192 timing cases
(1,637 admitted, 6,555 declined) and 144 refusal guards. CPU, full WRAM, ordered
fetch/data accesses, per-opcode clocks and return frames match the ROM.

The consumer times the existing rectangle filler and composes the three window
variants from shared child clock models. Forty-four current generated/linked
caller pairs preserve full CPU, WRAM, hardware and clocks, including mutating
parent/child hooks. Unknown hooks retain their original execution; independently
safe children can still run natively. Accepted calls reduce interpreted opcodes
157->2 for filling and 388->2 for a filled window. No new opcode hooks, table
scans, guest-time shortcuts or APU bypasses are introduced.

Three 3,000-call samples per entry, with detail profiling off, reduce median
filler cost168.568->57.526ms and window costs429.163->150.187ms,
436.832->152.897ms and428.962->153.456ms. These are constructed caller costs;
ordinary frequency of the newly discovered variant remains unproven.
Matching180 menu/180 field/600 boot frames preserve CPU/WRAM/hardware,
nonempty PCM and native idle work. Windows Release,14 availability gates,
six producer gates and two optional compilation modes pass. Optional compilation
is reused on unchanged actor/header inputs after the semantic-only ADC fix.
No full regression or fresh public CLI export is claimed. All proofs stay local.

# Second-battler stat reader — 2026-11-03

The previously missing original `$85:C07C..C098` reader follows the second
battler pointer at DP `$C1`. It shares the existing first-battler stat body,
including byte fields `$0E/$BC`, full word reads and original child frames.
This adds an original entry, not an independent new algorithm.

MSVC and Clang each pass 8,448 original-ROM semantic cases plus six no-access
guards, covering both readers and the existing first-reader JSR caller.
Each compiler also passes 4,096 native timing cases and 26 refusal guards.
CPU, full WRAM, ordered bus accesses and per-opcode clocks match exactly.

Sixteen current generated/linked caller pairs preserve full CPU, WRAM,
hardware and clocks. Accepted calls reduce interpreted opcodes from 28–30
to 2; unsafe banks, DP, decimal mode, mirrors, hooks and deadlines retain
original execution. Three 5,000-call samples with profiling off reduce the
median constructed caller cost from 63.960 to 36.468 ms. Ordinary gameplay
frequency remains unproven; this is not a whole-game performance claim.

Matching 180 menu, 180 field and 600 boot frames preserve full state, nonempty
PCM and native APU-idle work. Windows Release, eleven availability gates,
six producer gates and two optional compilation modes pass. Existing semantic
and timed cases share this checkpoint; no new full regression is claimed.
Tests and detailed proof receipts remain local.

The shared frame comparison passes180menu/180field/600boot frames with
identical CPU/fullWRAM/hardware/PCM/nativeAPUidle work. Boot host costs varied
across three matched600-frame runs:221.789->230.819ms,280.848->257.354ms and
221.324->216.724ms CPU/APU time. Two repetitions resolve an initial timing
uncertainty; they are not additional unique coverage or a whole-game speed claim.

# Post-return leaf integration — 2026-11-03

The original `$82:F46C`, `$82:F6BC` and `$85:EB5F` bodies now have verified
consumer bindings. Immutable timing phases preserve fetched/data bus order,
page-cross penalties, current interpreter store clocks and real RTS/RTL frames.
Native admission remains limited to synchronized quiet WRAM/VBlank spans;
unsafe banks, DP, hooks, devices and deadlines keep original execution.
Scratch comparison admits only stable DP0 WRAM, where its actual semantic body
naturally takes the original first-match path. Other paths remain in the ROM.

Forty-three matched real production pairs preserve CPU/full WRAM/hardware and
clocks. Raw instructions13/11/8 become2. Three samples of5,000 calls per entry,
with detailed profiling disabled, improve medians63.335->45.762ms,
63.727->51.283ms and46.989->39.093ms. No ordinary-call-frequency or FPS claim.
The scoped expected-return context adds these three targets, including RTL,
without a new opcode hook, whole-table search, context copy or APU/wait bypass.
The existing two-byte quiet-span admission remains equivalent and its affected
ROM/clock cases are rerun together. The extended maximum is59 guest cycles;
59*8+40 refresh master clocks fits the unchanged512-clock reservation.

# Fixed cursor preset integration — 2026-11-03

The missing `$82:A00F` preset is reconstructed in the standalone decomp and
bound through a quiet-WRAM/VBlank consumer span. Its seven setup/call opcodes
and RTS retain fetch/data order, guest/master clocks and the actual frame.
Unsafe contexts resume the original entry with the original bank. The original
cursor child still uses its existing integration; its legacy clock debt is
outside this parent checkpoint and is not claimed fixed or certified.

Twelve matched real production pairs preserve complete CPU/WRAM/hardware and
clocks; accepted paths reduce raw interpreted instructions10 to2. With detail
profiling off, three 5,000-call samples improve median54.987 to48.207ms. These
are constructed same-bank caller costs, not a measured organic hotspot or FPS.
The producer adds this target to the existing scoped return context, with no
new opcode hook, whole-table scan, context copy, wait/APU bypass or budget change.

# Small menu leaf integration — 2026-11-03

The new `$80:8AF7` and `$82:F4D7` bodies demonstrate why native selection and
fewer interpreted opcodes alone are insufficient. Initial matched 5,000-call
costs regressed from 42.917 to 49.418 ms and 36.188 to 45.672 ms. Those results
were rejected. The avoidable work was a rewritten-return dispatch for an
ordinary paired RTS.

The generated caller now exposes its true expected return PC through a scoped
scalar for these two targets. It retains the existing caller expression,
including inline-argument lookup; that extra binary lookup is restricted to
these targets. Other targets do not acquire a new lookup or return policy.
The consumer compares the actual timed RTS target with that expected PC.
Matching balanced returns use the normal host return; unknown or different
frames keep the existing unwind. Reading stack bytes alone would not establish
the host caller's expected target, so that shortcut is not used.

Current three-sample medians are 38.706 to 28.864 ms and 33.015 to 26.407 ms,
with complete state/clocks equal. Thirty-five linked original pairs and matched
600-menu/600-field/1,200-boot hardware/PCM/native-idle replays pass. Admission
reserves the original clocks/refresh and rejects aliases, MMIO, hooks, device
edges and unsafe frames. No opcode hook or whole-table scan is added; source admission avoids
buffer copies. Certified wait batches and the generated APU idle seam remain enabled.
These scoped gains do not prove ordinary frequency or general FPS improvement.

# Caller-width menu text — 2026-11-02

The new original `$80:8866` alternate entry shares the semantic string VM.
Its consumer has immutable setup/character/end timing phases and no per-opcode
host handoffs. A scan of at most thirty-two WRAM bytes admits only plain text
that fits the requested width. Separate indexed hook ranges cover the entry,
VM and glyph writer. Unsafe inputs and complex controls use the original path.
No wait, MMIO, APU cycle or sound handshake is removed.

Sixteen linked original-interpreter pairs, five default-fallback pairs and
600-menu/600-field/1,200-boot hardware/PCM replays pass. Matched 1,000-call
median is 108.667 ms interpreted versus 44.622 ms native, complete state equal.
This deliberately uses the original byte-interpreter VM as oracle: the older
AOT/native VM route undercharges this caller's original timing and is not a
valid clock reference. That pre-existing broader timing debt is not rewritten
in this checkpoint. Repeated whole-menu costs overlap and establish no general
speedup; ordinary entry frequency remains unmeasured. Certified wait batching
and the generated native APU idle interface remain intact.

# Resource formatter integration — 2026-11-02

The newly reconstructed `$85:C5B5` continuation uses immutable timing phases,
selected from actual source/counter reads, with the original inherited register
frame and nonlocal RTL return. Its conservative admission scans at most 73 input
bytes without copies; it requires plain non-aliasing WRAM, safe stack, quiet
VBlank, sufficient device/deadline room, and indexed hook/observer checks.
There are no new opcode hooks, per-opcode host dispatches or whole-table scans.
The existing paired caller carries its readiness and APU progress-flush context.

Fourteen linked production pairs preserve CPU/WRAM/hardware/clocks. Three matched
1,000-call samples preserve complete final state: median 108.713 ms interpreted,
42.373 ms native. The unsafe branches retain fallback. Existing copier/context
gates and 600-menu/600-field/1,200-boot hardware/PCM replays also pass. The actual
SPC idle seam and certified 1,024-opcode wait batches remain enabled.
These results establish a scoped integration and synthetic cost improvement;
ordinary formatter frequency and whole-game FPS remain unmeasured.

# Inactive HDMA native-wait admission - 2026-10-28

## Partial effect-slot timer admission - 2026-10-28

The matched battle refusal profile shows valid CPU/slot contexts: all observed
refusals at81:8A7E/8AAA/8AC1/8B1B come from the certified time window. The
original full slot can exceed that window while its two/six-opcode timer
header still fits. The consumer then runs the shared original header and
continues at the real advance boundary. All previous full admissions remain;
advance timing is unchanged. No original opcode,store,wait or sound work is
removed. The existing four hook sites and14-opcode minimum reservation stay.
New prefix admissions additionally require a quiet internal opcode range;
no observer or foreign mid-header hook is bypassed. Older semantic APIs keep
the previous full-step fallback. Lookups remain on normalized hook-page lists.

Shared validation:11280 full/header ROM/clock states,1128 decimal semantic
states,763840 windows,12existing semantic guards,128consumer refusals and10
new API/observer refusals pass. The legacy-only build passes6768/752states
and the same old guards. Two independent wrong-landing/omitted-observer
controls are detected. StrictMSVC/Clang,API gate current/legacy/draft,
WindowsRelease and actual actor symbol/generated caller checks pass. Ten
parent/step bodies are text-identical to the previous verified revision;
prior whole-parent evidence is reused only for those unchanged bodies.
Full unrelated suites/platform rebuilds are deferred.

In1200matched production frames,all CPU/WRAM/device/HDMA records,five full
hardware snapshots and670463fresh PCM samples are identical. Script headers
add4918native prefixes;actor headers add10743. All47543previous full native
uses remain. Attempts and advance counts are unchanged. Raw498167->465081;
only the timer-header PCs decrease. Wait9423216attempts/1944batches and
APU1838713calls/20758459skipped cycles match. One fixed-affinity uninstrumented
core/audio pair measured7.904854->7.74617s with matching final state and PCM.
This is no statistical speedup,whole-game or60FPS claim. Private receipt:
.agent/decomp/runtime-frontier/effect-slot-headers/ACCEPTANCE.json.

## Bounded battle-effect cleanup - 2026-10-28

The cleanup at81:8BFB uses the original ordered WMDATA clear through a bounded
semantic service. The consumer admits up to32 iterations only within the live
opcode budget,deadline,refresh and beam/HDMA window. The actual maximum in the
matched fixture is23bytes. Every STZ,DEX and BNE keeps its original cycles and
bus behavior. Every port write advances the bridge epoch; the final write's
one-based opcode index preserves quiescence progress beyond a32-bit mask.
No shared clock representation,guest speed,MMIO,wait or sound work changes.

The same hook site remains registered. Quiet-range and normalized-page owner
checks exclude observers and changed hooks; no full-registry scan is added.
Older semantic APIs retain the unchanged four-byte consumer. The CMake gate
accepts the new API,selects that legacy fallback and rejects a draft parent.
Actual generated12-opcode minimum admission remains; larger spans additionally
obey the interpreter's remaining opcode budget. Source/API presence alone is
not the acceptance evidence: actual actor symbols and production uses are
checked below.

Shared checks:78336 public ROM cases,1320 actual timed-hook cases,3055360
window cases,55refusals and256 complete2048-byte clears/1572864 original ops.
StrictMSVC/Clang,legacy consumer compile andWindowsRelease pass. Independent
wrong-MMIO and omitted-deadline controls are detected. Full unrelated suites
are deferred; this is the changed semantic/consumer batch's combined state.

The1200-frame matched production sequence has identical CPU/WRAM/device/HDMA
records,five full hardware snapshots and670463 fresh PCM samples. Actual
native clear bytes13961->21509; attempts14108->5253,bodies3493->2186,max4->23.
Raw opcodes520811->498167;only81:8BFB/8BFE/8BFF decrease,by7548 each.
Wait9423216attempts/1944batches andAPU1838713calls/20758459skipped cycles match.
One fixed-affinity uninstrumented core/audio pair measured8.2274298->7.6039187s
with identical final state/PCM. A single pair does not establish statistical
speedup,whole-game performance or60FPS. Private receipt:
.agent/decomp/runtime-frontier/effect-clear-span/ACCEPTANCE.json.

## Battle timer scan continuation - 2026-10-28

The consumer uses the shared original timer-controller entry85:8F1B and
handler return85:8F3C. Each admitted phase ends at the real handler target
with its8F3B frame preserved,or before original RTS8F49. The NMI interpreter
retains handler execution,return ownership and event boundaries. A96-opcode
reservation bounds a phase; stack,CPU,IRQ,DMA,refresh and physical HDMA/beam
windows remain guarded. Existing8F1F/8F3E and wave/ripple hooks remain fallback
paths. API availability is gated against the verified85:8DC5 parent.

8192 ROM timing plans,8192 actual hook spans,2162 guard/admission cases and
6676 service cases pass. Two timing/landing faults are detected; strictMSVC,
Clang,WindowsRelease and optional-NMI-off compile pass. Generated interpreter
ownership and actual actor references are checked. No unrelated full suite
was rerun. The optional-off guard does not change the tested active body.

The1200-frame matched production replay has identical CPU,WRAM,device and
HDMA records,five full hardware snapshots and670463 fresh PCM samples.
The new continuation admits74 of316 observed uses; the entry hook has no use
in this fixture because its existing parent already executes the prefix.
Actual raw opcodes521216->520811. Wait9423216attempts/1944batches and
APU1838713calls/20758459skipped cycles are unchanged. One fixed-affinity
uninstrumented core/audio pair measured12.5560712->10.1360654s with identical
final state and PCM. This single noisy pair establishes no statistical
speedup,FPS target or whole-game performance claim. Receipt:
.agent/decomp/runtime-frontier/timer-controller-entry/ACCEPTANCE.json.

No new canonical metadata/binding count is claimed. An independent whole-call
consumer ABI for85:8F1B remains separate work; these are certified NMI phases
of the existing parent,not an always-refused replacement binding.

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

### Bounded NMI callback phases

The existing secondary and scroll resumes at `80:8653`/`80:865A` now accept
the actual M1X0 caller as well as their original M1X1 semantic contract.
Both share the existing C phase worker and stop before an original JSR.
Other entry modes retain their contracts. No canonical function, algorithm
or hook PC is added. The consumer handles M1X0 only; legacy M1X1 aggregate
timing and observer behavior remain separate from this checkpoint.

Static two/four-opcode plans preserve LDA/BEQ cycles and both callback exits.
Admission executes real interrupt ownership, remaining-budget, pending-event,
observer, exact indexed-hook, clock, stack, ROM, deadline, IRQ/DMA/HDMA and
1024/1364 fence predicates. The common read-only window preserves the existing
restore admission, including its original overflow reserve. Immutable bank80
opcode fetches update the original bus latches directly inside that admitted
scope. WRAM reads and every guest instruction/device clock remain on their
original path. Unknown hooks/observers retain interpretation. The actual
private interpreter wrapper's two interrupt callers use stop-on-RTI with
no yield or stop list; other stop/yield APIs cannot own this
span. This is checked against the generated, compiled caller.

The shared checkpoint covers 524,288 original-ROM callback cases and original
cycle/bus bounds across both index widths, DB values, flags and stack limits.
The matrix and static timing tables remain identical across the consumer-only
fetch refinement. Production predicates pass 28,672 owner, 218 context/plan
and 5,186 old/new restore-admission assertions. Joint entry/inner observers
retain 10,796/8,780 full register, clock, beam and APU-clock snapshots and equal
profiles. Three real present-callback variants match original interpretation
before JSR, including bus latches; their WRAM overrides exist only in private
fixtures. Windows actual compile/link and strict GCC/Clang semantic/consumer
translation pass. No fresh full regression or CLI build is claimed.

In the matched 5,399-frame replay, 3,879 secondary and 150 partial scroll spans
replace 15,816 original fetches: 736,334 -> 720,518. Explicit bridge fallbacks
stay 7,166. Only the four callback opcode rows differ. Existing 2,457 restore
spans and their 39,312 replaced opcodes remain unchanged. CPU, full WRAM,
hardware/image, rolling PCM/count, APU clock, native wait totals and bus credits
match the prior restore build and current production control. The observed
maximum callback span is 116 master clocks; natural admission uses the 1364
fence. The 1024 fence has predicate/bound coverage, not a separate HDMA replay.

The actual generated APU caller also retains identical work: 8,089,011 native
idle calls, 5,140,317 native skipped spans covering 93,602,742 idle cycles,
and 3,131,707 native program-cycle calls. No interpreter idle spans appear.
This proves use of the idle interface, separately from equal guest timing.

Initial cost pairs were slower and triggered isolation and fetch-path work.
Isolation reversed direction and overlapped; no causal regression was proven.
Final production pairs measure reference 0.5171/0.4894 and candidate
0.5020/0.5049 CPU/APU ms/frame (medians 0.50323/0.50344). With both main threads
on logical CPU0, wall medians are 0.42511/0.42825 and active-thread cycle
medians 1,004,131/996,726 per frame; paired directions remain mixed.
These are distinct integration and cost results, with no stable speedup,
regression or whole-game/FPS performance acceptance claimed. The native
version avoids a source-derived 31,632 general-ROM callback reads present
in the initial native implementation. Private proof and initial measurements:
`nmi-callback-clocks-current/ACCEPTANCE.json` under `.agent/decomp/runtime-frontier`.

### Native NMI frame restoration with 16-bit indexes

The actual battle caller reaches `80:869B` in M0X0. The previous restore API
and hook admitted only M0X1. The semantic restore API now accepts both index
widths and retains the original overlapping counter updates, reset branch,
register/status pulls and exact pre-RTI boundary. The display/clock API keeps
its prior width contract. No algorithm, canonical binding or hook PC is added.

The consumer uses the existing restore hook for M0X0 only. Its actual generated
interrupt caller grants ownership only with sixteen remaining instructions,
no pending interrupt/wait, quiescent mode or observer. Production range checks
retain exact callback identity and indexed foreign-hook detection. The old
M0X1 route is unchanged. Its older aggregate timing/observer policy is outside
this checkpoint; the new per-opcode proof must not be generalized to that route.

Static twelve/sixteen-opcode tables preserve original 57/68 CPU-cycle paths.
They reserve at most 504/592 master clocks including 80 refresh clocks, strictly
before the 1024/1364 HDMA/scanline fence. Active HDMA channels are permitted
inside that fence; pending initialization and active DMA are rejected. IRQ,
deadline, clock, stack, ROM and reset-combination checks remain conservative.
The word read at `$4210` uses the production atomic MMIO-word primitive; the
reset-enabled path keeps its original byte read. RTI and a requested reset
execute on their original route. Guest waits and sound handshakes are retained.

The shared checkpoint passes 262,144 original restore cases across both index
widths and reset branches, 245,760 source-table cycle bounds, 4,096 unchanged
display cases and 2,048 interpreted RTI rejoins. Actual production predicates
pass 28,672 owner and 52 context/range/fence assertions, preserving old owner
decisions. Foreign entry/inner pairs retain 5,399/4,390 complete register,
clock, beam and APU-clock observations and equal profiles. A matched private
WRAM reset-control override exercises 118 real atomic-word native restores
against original interpretation over 1,200 frames; it is not a production patch.

In the matched 5,399-frame battle, 2,457 of 4,390 M0X0 restore attempts execute
natively. Raw fetches fall 775,646 -> 736,334 (39,312); explicit bridge fallbacks
remain 7,166. All profile rows outside those sixteen NMI opcodes stay equal.
CPU, full WRAM, hardware/image, rolling PCM/count, APU clocks, native wait totals
and bus credits match the prior queue build and current production control.
The observed maximum span is 488 master clocks. Remaining restores retain
their fallback; this is not an exhaustive census of each rejection reason.

Two alternating profiling-OFF pairs measure reference 0.4853/0.5107 and
candidate 0.4701/0.4933 CPU/APU ms/frame (medians 0.4980/0.4817). This short
battle measurement does not establish whole-game speedup or FPS acceptance.
Windows production compile/link and strict GCC/Clang translation of the actual
semantic and consumer files pass. Full regression and a fresh CLI build were
deferred; neither is reported as passing on this changed library revision.
Private evidence: `nmi-restore-clock-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`.

### Empty battle VRAM-queue rows

The existing queue-search callback now uses a shorter bound when its known
WRAM length word is zero and HDMA initialization is not pending. It checks
only that word, with the original M0X0, DB97/7E, DP0, row, stack, deadline,
refresh, clock and indexed-hook conditions. Empty rows use a static two/three
opcode table instead of copying and zeroing eleven timing entries. The bound
is five/eight fetched bytes at actual ROM speed, 22 data/internal master clocks,
and 80 refresh clocks. Occupied or pending-init rows retain the original
mutable timing and 250-clock reservation. RTL still executes on its original
route. No standalone algorithm, canonical binding or hook is added.

The shared checkpoint covers 6,528 original empty phases and their original
RTL rejoins, full CPU/WRAM/ordered accesses, and 13,056 cycle/bus bounds from
the actual source table. The unchanged standalone queue/BRK contracts retain
their prior evidence. The current Windows application builds and links;
GCC/Clang also translate the changed fragment against real SDK/API headers
with strict warnings. This is not a new whole-platform or resource-CLI run.

Actual production predicates pass 337,876 assertions, including old-admission
preservation and the pending-HDMA legacy route. Foreign entry/inner observer
pairs retain 5,829/8,653 complete register, clock, beam and APU-clock snapshots.
An entry observer allows later non-overlapping continuation spans; its observed
PC profile stays equal. An inner observer blocks all queue spans and the full
profile stays equal. No observer is ignored to manufacture native admission.

In the matched 5,399-frame battle, native queue spans rise from 7,202 to 7,679;
5,324 use the static empty path. Raw fetches fall 776,828 -> 775,646 (1,182).
Explicit bridge fallbacks stay 7,166; prior row, window and marker intake stays
equal. Guest registers, WRAM, hardware/image, rolling PCM and count, APU clocks,
native idle totals and bus credits match both prior baseline and new production
control. All observed spans stay before the 1364 fence; 1024 has predicate
arithmetic coverage only. No full-table scan or guest behavior is removed.

Two alternating profiling-OFF cost pairs are mixed: reference 0.4826/0.5375,
candidate 0.4950/0.4922 CPU/APU ms/frame (medians 0.5101/0.4936). No stable
whole-game speedup or FPS acceptance is established. Full regression and
unchanged standalone/resource-CLI runs were deferred. Private evidence:
`queue-free-budget-current/ACCEPTANCE.json` under `.agent/decomp/runtime-frontier`.

### Bounded target-marker records

The existing target selector now exposes its `81:D60E` marker-record loop as
an internal native span. One to three records share the complete parent's
original C helpers; a selected record stops before its original JSR. No wait,
MMIO operation, child behavior or canonical binding is removed or added.

The consumer admits only M1X0, PB81/DB7E, DP0 and the original twelve-record
grid with its matching countdown. It uses real CPU, return, APU-clock,
deadline, IRQ/DMA/HDMA, refresh and indexed-hook predicates. Static timing
tables preserve both record branches, terminal branches and the pre-JSR exit;
no per-span timing arrays are copied. The selected grid cannot cross the
absolute-Y operand page. Each span reserves at most 29 CPU cycles per record
at eight master cycles plus 80 refresh clocks, strictly before its 1024/1364
beam fence. Foreign entry, inner or global observers keep their original route.

The shared checkpoint passes 116,672 original-ROM/guard cases, including full
marker parents, actual child return/unwind paths and unchanged name spans.
9,888 cases independently check original cycles. The actual production
predicates pass 90,528 assertions; eight capability conditions pass. Foreign
entry and inner-hook pairs each retain 5,712 observations, matching profiles
and guest/audio endpoints. Naturally admitted spans use the 1364 fence;
1024 has predicate arithmetic coverage, not a separate enabled-HDMA replay.

In the matched 5,399-frame battle, 2,208 spans replace 39,280 original fetches
(816,108 -> 776,828). Explicit bridge fallbacks remain 7,166. Prior row spans
remain 615 calls/1,957 cells; full whole-window/row/end-piece calls stay 5/5/234.
CPU, WRAM, hardware, final image, rolling PCM and sample count, APU clock words,
native APU idle totals and bus credits match the prior accepted row baseline
and the new production-object control. Each span checks original cycles and
its master-clock/beam bounds. The final generated actor object contains and
actually executes the guarded callback.

Two profiling-disabled, alternating battle cost pairs measure CPU/APU ms/frame:
reference 0.4936/0.5446, candidate 0.5071/0.5159 (medians 0.5191/0.5115).
The pairs move in opposite directions; this establishes no stable whole-game
speedup or FPS result. Strict MSVC/GCC/Clang and C++ API checks pass. Unchanged
resource-CLI code/dependencies reuse the prior 680-resource evidence; full
regression was deferred. Private evidence: `target-marker-spans-current/ACCEPTANCE.json`
under `.agent/decomp/runtime-frontier`.

### Bounded battle-frame row cells

The existing `81:E553` row-fill loop now has a bounded native consumer route.
Each span executes at most six original cells (30 opcodes), then resumes at
`81:E553` or `81:E55D`. The portable body shares the complete row's original
loop; its unbounded entry remains unbounded. No canonical binding or algorithm
is added. The feature is enabled only with the public API, implementation and
verified M0X0 parent contract present; the actual actor object registers it.

Admission uses the production CPU, return, deadline, IRQ/DMA/HDMA, refresh and
indexed hook predicates. It requires WRAM and rejects protected destinations,
counter aliases and cross-bank output. It reserves the original 22 CPU cycles
per continuing cell (21 for a terminating cell), bus clocks and refresh room
strictly before the applicable 1024/1364-master-cycle fence. Existing static
five-opcode timing tables are reused without per-span table copies.

Whole-window and complete-row batches recognize only the exact registered
cell callback. Foreign replacements, inner hooks and global observers retain
their original routes. The existing quiet-clock wrapper still supplies NULL
ownership, preserving the admission of other batches and certified waits.

One shared checkpoint passes 105,760 original-ROM/guard cases and 25,215
production-predicate assertions. In the 5,399-frame battle replay, 615 native
spans execute 1,957 cells: 9,785 fewer original interpreter fetches
(825,893 -> 816,108). Existing full native calls remain 5 whole windows,
5 complete rows and 234 end-pieces; explicit bridge fallbacks remain 7,166.
CPU, WRAM, hardware, final image, rolling PCM, sample count, APU clock words,
native APU idle totals and bus credits match the production-object control.
Every admitted span also checks cycles, master-clock bounds and beam fences.
Naturally observed spans use the 1364 fence; the 1024 fence has predicate
arithmetic coverage, not a separate enabled-HDMA gameplay replay.

Two actual foreign-hook comparisons each preserve all 2,604 observations,
full profiles and guest/audio endpoints. Strict standalone/compiler and C++
checks pass. Resource CLI sources and dependencies are unchanged; the prior
680-resource checks are reused, not claimed as fresh runs. Full regression
was deferred under the shared-checkpoint cadence. Timing evidence is scoped
to matched, profiling-disabled production-object battle runs; no stable
whole-game speedup or FPS acceptance is claimed. Private evidence:
`row-spans-current/ACCEPTANCE.json` under `.agent/decomp/runtime-frontier`.

### Complete native functions: original AOT envelopes

The reached `83:A6DF` actor reset and `86:E709` world object-indexing entries
now retain original AOT entry/return envelopes around their verified portable
bodies. Previously the whole-function bridge charged zero CPU cycles at these
calls. The original envelopes charge 94/91 actor cycles for X16/X8 and 34 world
cycles, snapshot the coprocessor clock and retain entry/block deadlines,
tailcall adoption, paired returns and nonlocal return handling.

`scripts/native_leaf_entries.py` regenerates only these four M1 variants from
the supported ROM and pinned emitter. It preserves the original instruction
body as the compiled fallback. Native admission checks the actual M/X mode
against the generated variant before calling the portable body. The C seam
rejects emulation, decimal arithmetic and active tracing/diagnostic policy.
Actor admission additionally requires its direct-page operand in WRAM and
the known stack page. Open-bus-sensitive operands keep the original AOT body;
this does not certify those cold contexts against literal interpreter timing.
The direct-page penalty remains at the operand read after PHX. Word operands
and stores retain the platform's original word-access MMIO credit instead of
counting two byte transactions. Long stores preserve cross-bank carry.

CMake applies this composition only with the ROM and all required selected
entries verified. Source properties and configuration-specific definitions are carried
to the generated copies; the fetched SDK and standalone library remain intact.
Reference-only and ROM-unavailable configurations retain their existing route.
No hooks, waits or APU handshakes are removed or added. Interpreter pre-opcode
hooks remain distinct: the original AOT control also omits these inner hooks.

The shared consumer checkpoint covers 130 paired synthetic CPU/ABI cases,
including real scheduler deadline expiry, inherited returns, word bus credits,
rejected modes, diagnostic policy and stack/operand limits. Deliberately
incoherent compiled-mode cases prove rejection, not valid game execution.
Unchanged original-AOT reference fixtures are reused by source/binary identity.
Three additional comparisons exercise the actual optional profiler: both PCs
are attributed to decomp time and Boolean success is not a scheduler handoff.
This scope measures the guard/body; AOT entry/return time stays outside it.

Matched normal boot (1,200 frames) and battle (5,399 frames) replays admit all
135 boot actor resets, 46 battle resets and 800 world-index calls. Final CPU,
WRAM, hardware and image hashes, rolling PCM hash, sample count, exact APU clock
words, native idle totals and bus-credit totals match the original AOT control.
Raw interpreter fetches are 2,378/829,698 and explicit bridge fallbacks 22/7,379
in both arms. These normal runs omit the exploratory witness's two foreign
interpreter observers; older counts are not a matched optimization baseline.

Two alternating production-object cost pairs disable profiling and compare
identical inputs. Median CPU/APU time in ms/frame is 0.3170 -> 0.3125 for boot
and 0.3522 -> 0.3555 for battle. Mixed small changes establish no stable speedup.
No whole-game, per-frame visual, hardware or 60-FPS acceptance is implied.
Semantic and binding counts are unchanged: these are two existing algorithms.
Private evidence: `linear-leaf-integration-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`.

### World linear helpers: original envelopes

The same composition now covers `86:973E` (display disable, M1) and `86:AE05`
(block-pointer arithmetic, M0), retaining their 17/46-cycle original AOT
entry/return envelopes and complete compiled cold paths. The generation step
composes several leaves in one bank without overwriting earlier replacements.
Default native admission requires D=0; display writes additionally require
DB=86 and X16. Other contexts retain their original AOT operations.

The block-pointer callback preserves every original word transaction. Its
consumer also retains the emitter's deferred raw-P representation after the
last LDA while preserving the correct separate arithmetic flag mirrors.
This is an ABI adaptation; the standalone semantic function is unchanged.
The shared actor word writer was extracted without changing its algorithm.
No hook ownership, wait batching, APU handshake or guest delay was removed.

The shared checkpoint covers 184 paired generated-entry CPU/ABI contexts plus
10 invalid-mode/diagnostic-policy comparisons and four optional-profiler pairs.
It compares raw P and individual flag mirrors, deadlines, inherited returns,
WRAM, hardware and word bus credits. Invalid compiled modes prove rejection,
not valid gameplay. Default Windows Release/CLI and trace/reverse helper
compilation pass; the latter is not a complete debug frontend build.
The reference/no-ROM early guard remains unchanged.

Boot (1,200 frames) and battle (5,399 frames) match the AOT controls in final
CPU/WRAM/hardware/image state, PCM, APU clock words, native idle totals and bus
credits. All 883 display calls and 623 block-pointer calls observed in battle
admit the C body. The existing 46 actor resets and 800 object-index calls also
remain native. The boot fixture does not call the two new leaf compositions.

Two alternating production-object cost pairs with profiling off give median
CPU/APU ms/frame AOT/native of 0.3143/0.3083 for boot and 0.3822/0.3634 for
battle. These short comparisons establish no general speedup or FPS result;
the controls use correct AOT clocks, not the former zero-clock bridge.
The standalone library is byte-identical; its semantic verification is reused,
without another full decomp suite. No new algorithm or canonical binding is
counted. Evidence: `world-leaf-integration-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`. Branching arrival was still open at this historical checkpoint; see below.

### Branching world-arrival helpers: exact native boundaries

The generated moving/stopped arrival entries (`86:9AB1`, `86:9AB8`, M1/X16)
now use the existing shared semantic C body through optional original block
boundaries. The consumer preserves each block's Watchdog refresh, deadline,
coprocessor snapshot and cycle charge, including taken branches. A partial
execution unwinds at the exact pending ROM PC through the original owner;
normal returns retain the original paired/tail return epilogues. No additional
hooks or opcode dispatcher crossings are introduced. Complete original AOT
bodies remain compiled for declined contexts. Existing four linear helpers
continue to compose into the same generated banks.

Admission requires the production CPU/RAM, native mode, decimal clear, M1/X16,
PB/DB=86, D=0, the known stack range and inactive tracing/diagnostic policy.
Unsupported contexts retain the original route. Raw P can remain deferred after
DEC/CMP in emitted AOT; the consumer preserves that representation independently
of the correct flag mirrors. Standalone semantics remain original-ROM faithful.

The shared checkpoint passes 102,400 ROM differential cases, 384 no-access
guards, two rejected mutation controls, 472 actual generated-entry ABI pairs
and eight real optional-profiler pairs. Deadline cases execute the production
Watchdog and master-deadline predicate. Read-only accessors observe the pending
unwind PC/owner before the outer interpreter publishes its resume cursor.
Moving-entry ABI cases use a redirected production caller seam, not natural
moving gameplay. Incoherent compiled modes prove rejection, not ROM fidelity.

Normal boot1200/battle5399 replays match final CPU/WRAM/hardware/image state,
PCM, APU clocks, native idle totals and bus credits. All 558 naturally observed
stopped-arrival calls complete in C; moving-arrival intake is zero in these
fixtures. Existing actor/index/display/block-pointer counts stay 46/800/883/623
in battle. Raw interpreter/fallback counters are disabled in these controls;
zero counters therefore do not demonstrate zero interpretation.

Two alternating production-object cost pairs per scene, profiling off, give
median CPU/APU ms/frame AOT/native boot 0.4285/0.4006 and battle 0.6722/0.6374.
Individual pairs disagree; these noisy short measurements establish no stable
speedup or FPS claim. Boot does not reach the new arrival entries. Controls
use correct original AOT clocks, not the former zero-clock bridge.

Default Release and CLI pass, as do strict standalone MSVC/GCC/Clang builds,
C++ callback linkage and resource decoding. Trace/reverse checks compile only
the changed helper, not complete debug frontends. No full decomp regression was
repeated. No canonical binding or algorithm count changes. Private evidence:
`arrival-boundary-integration-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`. Direct semantic-inline parents and remaining
hot fallback paths require their own timing/utilization evidence.

### Current native-use diagnostic after arrival integration

A fresh counter-versus-production-object control at `1f78caa` matches CPU,
WRAM, hardware, final image, PCM/sample count, all APU clock words, native idle
and bus credits in menu600, field600, boot1200 and battle5399. Actual core fetch
and 2,033 explicit bridge counters install no PC hooks or observers.

| Fixture | Raw opcode fetches | Explicit bridge fallbacks |
| --- | ---: | ---: |
| Menu, 600 frames | 3,058 | 3 |
| Field, 600 frames | 0 | 1 |
| Boot, 1,200 frames | 2,378 | 22 |
| Battle, 5,399 frames | 830,142 | 7,389 |

These are fixture counts, not a whole-game native percentage. Aliases and some
timed continuations are not explicit bridge counters. A fallback can continue
through a native partial span; `85:ECDB` accounts for 5,829 battle fallbacks
without proving 5,829 wholly interpreted calls. Counter overhead excludes a
causal runtime or FPS claim. Counts from earlier clock revisions are historical,
not an identical scheduling baseline for attributing this change.

The 213 battle division fallback entries are M0/X16, DB7E; the existing timed
wrapper requires M1. The 272 multiplication fallback entries are M1/X16,
DB81/85; their detailed quiet-window rejection needs targeted predicate
observation. These routines already have semantic C. Preserve original entry
widths, hardware fences, observer ownership and APU synchronization when
investigating admission. Private evidence:
`native-usage-after-arrival-current/ANALYSIS.json` under
`.agent/decomp/runtime-frontier`.

### Native division: incoming accumulator width

`80:8378` saves P before REP establishes its working widths and restores P
before returning. Its existing semantic C supports both incoming accumulator
widths; the consumer now admits M0 as well as M1 through the same timed body.
The change removes only the M1 restriction and allows the wide entry in the
existing window predicate. X16, D=0, binary arithmetic, original hook ownership,
return frames, guest clocks, beam/deadline reserves and APU behavior stay gated.
The actual quiet range is 216 bytes, below the production 256-byte limit.

24,576 original-ROM cases compare CPU, full WRAM and ordered accesses across
M0/M1, X0/X1, flags and operand extremes including a zero divisor. The consumer
still requires X16. In the matched battle fixture only 12 of 213 M0 division
calls pass all remaining predicates; 198 remain outside VBlank and three lack
end-of-VBlank room. A separate admission diagnostic passes 168 assertions
against actual production predicates, including global-observer rejection.
Entry/middle hook registration was not separately replayed here; the original
range predicate and registration ownership logic are unchanged.

The 5,399-frame candidate and admission-control replays match final CPU/WRAM,
hardware/image, PCM, APU clocks, native idle and bus credits. Raw fetches fall
830,142 -> 829,058; explicit fallbacks fall 7,389 -> 7,377. Twelve M0 divisions
complete natively. These are fixture counts, not whole-game coverage or speed.

Two alternating production-object cost pairs with profiling disabled match
endpoints. CPU/APU ms/frame reference/candidate is 0.4556/0.5929 in the first
pair and 0.5608/0.5476 in the second (medians 0.5082/0.5702). The mixed/noisy
result establishes neither a stable speedup nor a performance PASS or FPS
claim. The static fallback count and semantic/native correctness are distinct.

Default Release and actual generated M0 caller/link selection pass. Standalone
sources/archive are unchanged; their other compiler gates and full decomp suite
were not repeated. No canonical algorithm or binding is added. Private evidence:
`divide-width-integration-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`.

### Decompressor phases: foreign hook ownership

The control, symbol, flagged-literal and copy phase consumer could skip foreign
inner or global observers. A reproduced menu case reaches 496 branch-hook
visits in the original instruction control but only 140 in the former native
handler, even though game endpoints match. Hook fidelity therefore needs its
own evidence alongside CPU/audio/image comparisons.

The phase admission now checks every covered original instruction range using
the actual indexed hook registry. The flagged literal may inline only the exact
advance callback at `80:8F14` or its `80:8FB8` destination-bank mirror. The two
semantic APIs share `StreamContinueFlagged`; its original clock stages remain
unchanged. Replaced callbacks and foreign/global observers retain original
execution. No delays, MMIO/APU work, guest opcode or timing operation is removed.
The largest checked range is 34 bytes, within the production 256-byte limit;
no whole-table search or CPU-state copy is added to the production hot path.

Eight paired 600-frame menu modes compare normal execution, five inner or
replaced-entry hook sites, a global observer and an admission diagnostic.
Observer counts now match the original instruction control: the reproduced
branch has all 496 visits. CPU/WRAM/hardware/image, PCM, APU clocks, native idle
and bus-credit endpoints also match. Fifty real registry assertions cover
both destination-bank mirrors, rollover ranges, global observers and exact
advance-callback replacement/restoration. The 7F mirror is predicate/registry
coverage, not a separately observed 7F resource replay. Unrelated hooks and
whole-game observer behavior are outside this proof.

Without foreign hooks the fixture retains the same 3,058 raw opcode fetches.
Two alternating production-object cost pairs, profiling disabled, give CPU/APU
ms/frame reference/candidate of 0.4225/0.4353 and 0.4204/0.4063 (medians
0.42145/0.42080). Endpoints match; mixed timings prove no stable speedup or FPS.
Default Release and actual bridge compile/link selection pass. The standalone
archive and all semantic/clock bodies are unchanged; full decomp/compiler
regressions were not repeated. No new algorithm or binding is counted.
Private evidence: `decompress-ownership-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`.

### Remaining battle-window admission frontier

A read-only first-rejection diagnostic over the matched 5,399-frame battle
fixture records 99 whole-window entry attempts (5 accepted, 19 rejected for
return ownership, 72 outside VBlank, 3 with insufficient remaining VBlank),
188 row attempts (5 accepted, 181 outside VBlank, 2 with insufficient remaining
VBlank), and 383 end-piece attempts (23 accepted, 360 outside VBlank).
These are wrapper admission counts; inlined child work is not counted again.
The diagnostic preserves short-circuit predicate order and installs no new
hooks. CPU/WRAM/hardware/image, PCM, APU clocks, idle and bus credits match the
current production-object control; its raw counters are disabled. The counter
arm records 829,058 actual interpreter fetches. No timing improvement follows
from this diagnostic.

The end-piece timing plan contains 15 original opcodes totaling 65 CPU cycles,
including RTS, while its existing admission reserves 128 cycles before the
master-clock/refresh allowance. This is a source inventory, not a certified
new reservation or permission to cross scanline/HDMA/interrupt boundaries.
The next experiment must prove an end-piece bound against actual bus speeds,
refresh, beam fences, callback ownership and the linked return path. Row and
whole-window timing are separate contracts. No admission or algorithm changed
at this diagnostic checkpoint. Private evidence:
`battle-window-admission-current/REVIEW.json` under
`.agent/decomp/runtime-frontier`.

### Visible-line battle-window end-pieces

The existing `81:E570` end-piece can now use its unchanged timed C body during
visible lines when its complete original sequence fits before the next beam
fence. The plan has 15 opcodes and 65 CPU cycles including RTS. Legal source
ROM, destination WRAM and stack transactions cost at most eight master clocks
per cycle (496 master clocks for the explicit bus/internal plan). Admission
reserves 600: 65*8 plus the existing conservative refresh allowance. Execution
must end strictly before H=1024 when beam HDMA is enabled, or H=1364 otherwise.
Pending HDMA initialization, DMA/IRQ/NMI state, deadline, return ownership,
refresh state, output bounds and the actual indexed hook predicate remain
checked. The range is 35 bytes. Row loops, whole windows and VBlank routing
retain their existing paths and reservations.

24,576 fresh original-ROM cases compare CPU, WRAM, ordered accesses and the
59 body cycles before the six-cycle RTS. The matrix includes both WRAM banks,
flags, source/output extremes and direct pages, including states rejected by
the consumer. In the matched battle fixture 211 additional end-pieces complete
natively; per-call traces record 65 CPU cycles, at most 600 master clocks and
no fence or scanline crossing. These observed calls use the beam-HDMA-off
1364 fence. The 1024 fence is covered by actual predicate arithmetic tests,
not a separately observed natural HDMA-enabled scene. Hardware/FPS acceptance
is not implied.

4,431 production-predicate assertions cover mode, stack, return width,
RAM/clock state, pending HDMA, IRQs, global observers and both fence edges.
Paired foreign entry/middle hooks reach all 403 visits at each site, with
identical profiles and guest endpoints. Normal and predicate-diagnostic
5,399-frame replays preserve CPU/WRAM/hardware/final-image state, rolling PCM,
APU clocks, native idle and bus credits. End-piece admissions rise 23 -> 234;
whole/row admissions remain 5/5. Raw fetches fall 829,058 -> 825,893 and
explicit fallbacks 7,377 -> 7,166 in this fixture.

Two alternating production-object cost pairs with profiling off match
endpoints. CPU/APU ms/frame reference/candidate is 0.56035/0.40818 and
0.38527/0.39504 (medians 0.47281/0.40161). The mixed timings establish no
stable speedup or FPS claim. Incremental Release and actual caller/compile/link
selection pass. Standalone C/archive and original clock/bus operations are
unchanged; the full decomp/compiler suite was not repeated. No new canonical
algorithm or binding is counted. Private evidence:
`window-ends-beam-current/ACCEPTANCE.json` under
`.agent/decomp/runtime-frontier`.

### Remaining reached paths: measured scope

At the following checkpoint, three independently regenerated AOT controls
replace one world helper at a time. Their first observed entries have identical
CPU/WRAM inputs and matching data outputs. The current bridges charge zero CPU
cycles; AOT charges 17 for `86:973E`, 46 for `86:AE05` and 15 for `86:9AB8`.
These first-call observations preceded the two linear-helper fixes above.
At that audit, branching arrival still required path-specific timing. The
subsequent boundary checkpoint above resolves the two emitted entry routes. Evidence: `hot-world-usage-current/INVESTIGATION.json`.

Explicit bridge fallback counts also include partially native execution. In the
matched battle diagnostic, target-name clearing admits 19,207 native spans;
9,430 other invocations reject at a later budget/refresh check. The effect-slot
shared state gate passes 208,021 times; this is not a completed-span counter.
The diagnostic retains predicate order and matches endpoint CPU/WRAM/hardware,
PCM, image, APU clocks and bus credits. It establishes neither speedup nor that
remaining safety guards should be widened. Evidence: `hot-world-usage-current/GATES.json`.

### Battle upload queue admission

The existing `85:ECDB`/`85:ECDE` timed queue hooks now check the production
opcode-page hook registry. Only their exact callbacks at those two addresses
belong to the span. Foreign, replaced and global observers retain individual
execution; no hook is added or ignored to enable admission. A matched replay
exposed 4,439 skipped foreign callbacks before this correction: the reference
and corrected paths both deliver all 8,653 callbacks.

The guard also accepts the observed DB7E context alongside DB97. Both address
the same WRAM queue at `1A8F..1AEE`; logical bus accesses and regional timing
remain unchanged. Existing CPU, deadline, DMA/HDMA, interrupt, refresh and APU
synchronization restrictions remain. The span stops before the original full-
queue BRK or free-row RTL. Standalone semantics and binding counts are unchanged.

In a fixed 5,399-frame battle replay, DB7E admits 2,759 additional timed phases
and DB97 retains 4,439. Actual interpreter fetches fall from 845,400 to 831,147;
explicit bridge fallbacks remain 7,409, since a fallback may subsequently use
a native partial path. These are execution counts, not native completion rates.
Four observer configurations match CPU/WRAM/hardware/PCM/final-image hashes,
sample counts, exact APU clock words and native idle counts. Another 25,132
checks exercise real admission predicates and registered hook identities;
4,352 original-ROM phase comparisons cover both banks and all queue rows,
including 128 final-row BRK boundaries, CPU state and ordered data accesses.

Four alternating production-source timing pairs use identical inputs and no
profiling counters. Median guest CPU/APU time is 0.3674 versus 0.3656 ms/frame;
paired changes range from -4.38 to +5.01 percent. This tiny median difference
does not establish a reliable speedup. Drawing medians are 3.2653 versus
3.2443 ms/frame; drawing is unchanged. This headless Windows probe excludes
frontend/presentation and is not whole-game, hardware or 60-FPS acceptance.
The queue API-off compile and actual-source production link pass. Unchanged
semantic/CLI checks are reused within their recorded scope; full regression
was deferred. Private receipt: `native-usage-audit-current/queue/ACCEPTANCE.json`
under `.agent/decomp/runtime-frontier`.

### Observer ownership in earlier fixed spans

A second matched 5,399-frame replay exposes observer loss in the existing
actor fine-position, background-word, battle-input and NMI-timer spans:

| Observer site | Individual stepping | Previous native path | Corrected path |
| --- | ---: | ---: | ---: |
| `83:A748` | 6 | 2 | 6 |
| `81:BA5A` | 1,023 | 237 | 1,023 |
| `85:EC9D` | 4,388 | 1,726 | 4,388 |
| `85:8F3F` | 9,848 | 4 | 9,848 |

The guards now consult the actual indexed hook registry and accept only exact
owned callback identities. The timer caller retains its certified header and
advance callbacks. Input admission checks separate executed intervals: the
pause wait at `85:ECB7/B9/BB` and the skipped resume entry remain outside the
initial span. An initial envelope covering that gap blocked input batching;
that version was rejected before acceptance. No existing wait is whitelisted
merely because it lies between two executed regions.

All 13,296 missing callbacks are restored. The normal replay retains exactly
831,147 interpreter fetches, 7,409 explicit fallbacks and the same native-idle
count. CPU/WRAM/hardware/PCM/sample/image hashes and exact APU clock words match.
Another 600-frame field comparison matches but reaches none of these observer
sites; it does not establish field-hotpath timing. Forty-four checks exercise
real registry replacement, inner/global observers and outside-span hooks.
The timer-step predicate additionally uses a parent-derived CPU clone; this
is admission evidence, not an independently observed native timer-step call.

The production-source actor links with the existing generated callers. Timer-
step-off, NMI-support-off and affected-API-off compilations pass. Three matched
alternating production-source pairs, profiling disabled, measure CPU/APU medians
0.3544 versus 0.3581 ms/frame and drawing 3.1378 versus 3.1778 ms/frame. Paired
CPU changes range from -18.48 to +5.70 percent; unchanged drawing also varies.
These measurements do not establish a stable speedup or whole-game regression
bound. This is a scoped observer-correctness fix, not performance/FPS acceptance.
Standalone semantics, clock callbacks and generated dispatch are unchanged;
full semantic/all-platform/CLI regression was not repeated. Swirl-rim and wave-
fill observer sites had zero hits and remain unaccepted follow-up work.
Private receipt: `.agent/decomp/runtime-frontier/fixed-span-ownership-current/ACCEPTANCE.json`.

### Swirl and wave-fill observer coverage

The original 664-frame tutorial replay reaches the swirl-rim hook at `84:8C91`.
A separate 60-frame synthetic battle state reaches wave fill: only RAM timer
bytes `1B17=1,1B18=1,1B19=4` are seeded after rollback load. The original NMI
dispatcher reads selector four at `85:9E05`; its stored `AEEA` and original RTS
enter `85:AEEB`. There is no direct host jump or overridden owner predicate.
This is a valid synthetic timer contract, not a new gameplay capture.

Without range ownership, foreign callbacks at `84:8C95` occur only 403 of
1,024 times; at `85:AEF7`, only two of 176. Both existing timed guards now
accept only their exact own entry callback in the executed interval. Corrected
observer runs retain every original call and matching CPU/WRAM/hardware/PCM/
sample/image hashes, exact APU clock words and native-idle counts. Sixteen
registry checks run in actual callback contexts: swirl outside NMI and wave
inside NMI. Wrong callback identities, the same callback at a different PC,
inner/global observers and overlong ranges are rejected; terminal PCs stay
outside each span.

Without observers, native admissions remain 621 swirl rows and 174 wave words;
raw fetches remain 111,300 and 10,851 in these respective fixtures. Existing
clock/decomp semantics are unchanged. Production-source link, affected-API-off
and NMI-support-off compilations pass. Three alternating cost pairs per scene,
profiling disabled, measure CPU/APU medians 0.4635 -> 0.4609 ms/frame (tutorial)
and 0.4793 -> 0.4624 ms/frame (synthetic timer). Unchanged drawing also varies;
these short headless Windows measurements do not establish a speedup, whole-
game performance or hardware/FPS acceptance. Full semantic/CLI/all-platform
regression was not repeated for these admission-only changes. Private receipt:
`.agent/decomp/runtime-frontier/effect-span-ownership-current/ACCEPTANCE.json`.

### Startup and decompressor-reference observers

A 1,200-frame original-path boot control exposes one skipped callback in the
boot prefix and one in the initial sound-transfer prefix. A 600-frame menu
control exposes another 108 skipped callbacks across reached reference-header,
long-reference and copy-setup phases. The reference conservatively rejects
only these native prefixes and removes only the six reference prehooks; actual
production predicates remain in the current/candidate arms.

Boot and sound/object prefixes now check their executed descriptor intervals.
The shared hook helper keeps noncontiguous branch regions separate; it does
not cover skipped addresses or change existing fixed-span clock services.
Reference phases check their exact own hook identities, selected bank mirror
and conditional rollover region. Subsequent flag/copy hooks remain outside
these intervals. Normal interpreter counts remain 2,330 for boot and 3,058
for menu, with unchanged explicit fallbacks and native-idle counts. All 110
missing callbacks are restored; CPU/WRAM/hardware/PCM/sample/image hashes and
exact APU clock words match original-path controls.

Sixty-six actual-registry checks cover reached boot/sound/DB7E phases plus
explicit synthetic object, DB7F and rollover contracts. The object prefix was
not reached by these replays; its sound-derived CPU clone proves admission,
not newly observed gameplay. Branch-gap/end positives preserve downstream
phases and waits. Default production-source link and boot-off, continuation-
off and affected-APIs-off compiles pass. Three alternating production-source
cost pairs per scene, profiling disabled: CPU/APU medians 0.3040 -> 0.3054 ms
for boot and 0.2773 -> 0.2809 ms for menu. Drawing also varies. These are
scoped correctness observations, not speedup, whole-game or hardware/FPS
acceptance. Standalone semantics and the original clock source are byte-
identical; full semantic/CLI/all-platform regression was not repeated.
Private receipt: `.agent/decomp/runtime-frontier/startup-span-ownership-current/ACCEPTANCE.json`.

### Observed name-entry admission boundary

A 2,820-frame new-game replay reaches one native save-menu call, but no native
`82:EBF2` binding attempt or shared name-entry body call. The interpreter still
reaches that address in DB83. Merely allowing DB83 in a private guard experiment
does not change admission, state, PCM or the raw execution profile. Keep that
experiment out of production; reaching an address through interpreter flow is
not proof that the generated native entry is offered there.

Sharing the existing name-entry body removes 3,718 COFF code-section bytes from
the two affected production modules. It neither admits another context nor
establishes faster execution. Any future continuation must independently prove
the actual caller, return ownership, clock/bus behavior and preserved wait/APU
batching. Do not add a hook solely to manufacture a cold native hit.

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


## Battle target-name continuations - 2026-10-28

The original clear loops at81:D58D/D7C5/D8CC now share the existing selector's
C algorithm through bounded continuations. Up to eight indexed stores retain
all original opcode fetches, writes, flags and per-opcode device/APU timing.
Admission shrinks the span to fit its real deadline and ends before HDMA or
the next scanline. Observer/interior-hook checks use the maintained page index.
Three capability-gated hooks require verified81:D4E0 and the matching API/body;
these phases add no canonical function or binding count.

One shared checkpoint covers18,432 original spans and next-opcode rejoins,
99 refusal cases,491,040 raster/deadline cases,57 portable API cases and eight
capability gates. Strict MSVC/Clang C11 and the reserved Windows Release pass.
The original-ROM test caught and corrected an X16 indexed-store timing error.
A matched1,200-frame battle preserves CPU/WRAM/device/HDMA records, five full
hardware snapshots and670,463 fresh PCM samples. Two sites admit1,938 spans
covering10,537 stores; the third cancel site has ROM/ABI coverage only.
Interpreted opcodes fall616,104->584,493. Existing wait/APU counts are identical.

Three alternating fixed-affinity pairs without profiling give core-plus-audio
medians7.9710139->8.2796385 seconds, with substantial variation. This is evidence
of reduced interpretation, not an established performance improvement; no FPS
claim is made. Unrelated full regression and other-platform builds are deferred.
Private evidence: runtime-frontier/target-name-spans/ACCEPTANCE.json.


## NMI work with HDMA enabled - 2026-10-28

The consumer now distinguishes HDMA feature enable from its next physical
transfer. NMI ownership can persist into visible scanlines after a long
handler. Finite timer/ripple phases may run when their complete reservation
ends before HDMA or the next line; complete ripple/primary-wave builders
with HDMA enabled remain confined to the remaining VBlank. Existing owner,
stack, interrupt, DMA, refresh and synchronized beam/APU guards remain.
No hook, semantic routine, clock descriptor or guest behavior changed.

The shared checkpoint passes38,784 original spans and rejoins,512 original
RTI suffixes,780 portable timer cases,211 refusal cases and5,003,152 window
cases. All2,455,496 previously allowed window cases remain allowed; an unsafe
window mutant is rejected. Strict C11 and reserved Windows Release pass.
A1,200-frame battle preserves all captured CPU/WRAM/device/HDMA records,
five hardware snapshots and670,463 fresh PCM samples. Interpreted opcodes
fall584,493->521,216. Actual use includes20 whole ripple builders,7 whole
primary-wave builders and6,932 shorter phase admissions. Wait/APU counts
are identical. A single fixed-affinity uninstrumented core/audio pair measures
9.497871->8.0837394 seconds, with identical end state and PCM. This single
sample is not statistical speedup acceptance or a frontend/console FPS claim.
Unrelated standalone suites and full regression were deferred.
Private evidence: runtime-frontier/battle-nmi-admission/ACCEPTANCE.json.


## Owned ripple word rows - 2026-11-02

The optional85:AA61 hook shares the existing ripple word algorithm. Admission
preserves NMI ownership,caller stack,DB/DP,widths,refresh,device/HDMA edges and
observer/interior-hook guards. Normal calls retain the existing owned budget.
The common NMI owner remains a four-argument leaf;packed observation flags
avoid a fifth stack argument. Native observers are still refused by the
unchanged word readiness quiet-range check. Certified wait/APU batches remain.

The shared checkpoint covers8192 semantic rows,2048 full-parent states,
42504 ordinary and22176 NMI timed states,47 guards and201335808 owner/composed
guard cases. Actual generated callers,linked code,legacy headers and gates
pass. Unchanged semantic/timing-body proofs were reused;no full suite repeated.

Two matched1200-frame synthetic encounter fixtures compare CPU/WRAM,hardware
snapshots,HDMA byte order and fresh PCM. The word case executes20242 native
rows and reduces430234 decoded instructions to283737;667405 PCM match.
The old battle control keeps386157 decoded instructions and670463 PCM.
Wait/APU counts match. These fixtures are not whole-game coverage.

After removing shared stack overhead,two order-balanced uninstrumented pairs
per input measure5.5251613->5.0961861 and5.3883316->5.1979264 seconds for the
word fixture. Controls measure7.5870777->7.3683267 and7.3248524->7.1689015.
All end states/PCM match. Earlier variants showed substantial host drift;
these small samples establish no statistical/global speedup or frontend FPS.
The control also improves;the whole difference cannot be assigned to rows.
Private evidence:runtime-frontier/word-ripple-row/ACCEPTANCE.json.

## Original idle scene returns — 2026-11-02

Scene selectors8/9 in86:AF0A call the original single-RTS entries86:AFB0/
AFB1. Their verified bindings reuse the existing timed idle-return owner;
no hooks,wait admission,APU seams or new timing framework are added. The
portable header capability preserves builds against the previous API.
The shared owner now synchronizes flag mirrors without copying unchanged
registers back. This relies on all9 callbacks being immutable original RTS
functions;the standalone immutability check guards that precondition.

36864 original-ROM/timed cases compare CPU,WRAM,ordered fetch/data bus and
clock/refresh/beam state;4608 portable immutability/bank checks and774
admission checks pass. A deliberate portable CPU mutation is rejected.
The actual generated Bank86 callers select both bridges,and Agent Windows
Release links successfully. Prior-header and frame-return-off actor builds
pass. The latter exposed an existing card-table HDMA registration guard;
registration now requires the same frame-return feature as its implementation.

The matched native microfixture executes1.8million calls per sample with
zero fallbacks. Reference samples take0.099–0.147s;candidate0.098–0.120s.
Noise and overlapping distributions do not establish a speedup. These are
bounded host-call measurements,not gameplay/FPS acceptance. A larger full
differential-fixture timing also includes interpreter/seeding work and ran
concurrently with compilation;it is not accepted as performance evidence.
Guest RTS fetches,stack reads,timing and original return semantics remain.
No full decomp regression was rerun for these empty endpoints;the shared
affected union and optional integration builds cover the changed contract.

## Party member status and vital changes — 2026-11-02

Eight previously uncatalogued original routines in 81:F639–F722 now use
shared status and vital-value C algorithms. Their consumer executes each
accepted call through the existing FixedSpanClock, with immutable plans for
18 original paths. Planning performs bounded RAM/ROM peeks, with no timing
table search, allocation or per-call plan copy. No PC hooks are added, and
certified wait/APU batching and its generated callers are unchanged.

Admission requires native M1X0, a paired frame, ordinary RAM records, a
matching LoROM cartridge, quiet code ranges and sufficient uninterrupted
clock room. Status paths accept either decimal flag. Vital paths currently
admit binary arithmetic; decimal execution retains the original fallback,
while the standalone C implementation preserves decimal behavior. The
prologue's original Y truncation is also applied when choosing the timing
path. An early timing fixture caught its omission at HP case 72.

65536 standalone original-ROM cases compare CPU, full WRAM and ordered bus/
MMIO, plus 40 refusal cases. The shared consumer proof covers 32768 calls
across fast/slow ROM, low-WRAM mirrors, both WRAM banks and cross-bank RTL
returns, plus 784 read-only admission cases. All pass. The 4552 mixed-frame
return dispatches preserve the original RTS paths; they are not entry
fallbacks. Every accepted fixture call executes the semantic C body.

These paths have no direct far-JSL byte references in the inspected ROM;
this does not exclude indirect invocation. Current gameplay frequency and
before/after gameplay cost remain unmeasured. No game-wide speedup or FPS
claim follows from these entries or fixture admission counts. The affected
semantic/ABI union replaces another full regression for this checkpoint.

## Field developer controls and Cave diagnostics - 2026-11-02

Seven previously missing original routines now have verified C implementations
and consumer bindings: 8E:B0D4/B134/B13B/B150/B23F/B275 and 85:84C6.
Existing B142/B149 reuse the same input algorithm and now also admit X8.
The controller retains its five pushed child calls; the Cave map retains
256 cells, its stair marker, eight values and the bounded link count.

Consumer timing reuses FixedSpanClock. Immutable timing segments repeat for
map cells, links and sprites without copying an opcode timeline or scanning
the binding table. Each accepted parent remains inside C between actual
child boundaries; it does not redispatch each original opcode. No PC hooks
were added. Admission checks CPU mode, stack, RAM mappings, observer-free
code ranges, DMA/IRQ state, synchronized clocks and remaining device budget.
Every child return is checked again before continuing. Unsupported contexts
retain the original execution route. Binary timing admission is separate
from the standalone renderer's original decimal arithmetic support.

The shared checkpoint covers 41728 original-ROM leaf cases, 1024 controller
cases, 35 standalone child-context/unwind cases and 26 no-access refusals.
Separate consumer checks compare CPU, full WRAM, ordered fetch/data phases,
open bus and guest clocks across 50448 leaf calls and 8192 controller calls.
1024 composed controller runs include 125 actual native sprite-grid calls;
45 changed child contexts and 846 read-only admission cases also pass.
The divider retains its MMIO sequence, JSR and both original RTS visits.

These are diagnostic paths with no measured gameplay invocation frequency.
The fixture establishes native admission and exact guest costs; it does not
establish a before/after whole-frame speedup or hardware FPS result. Prior
certified wait/APU batching remains owned by its existing source composition.
Full regression is deferred; this checkpoint runs the affected union once
and rechecks focused failures without repeating unrelated green suites.

## Alternate ripple setup and native admission - 2026-11-02

Three missing original routines are now reconstructed and bound: 81:BACB
selects one of four ripple paths, 85:A76E initializes the alternate scroll
table, and 85:A7B1 builds its split scroll values. The selector reads its
original ROM table; unknown targets remain exact original continuations.
The table builder preserves the WRAM read at 7E:210D, ordered word writes,
carry between shift and addition, and original decimal behavior.

Descriptor construction is shared with existing 85:A701. Its consumer now
executes the real 85:A736 child in the same timed C call, removing the one
nested host dispatch from an accepted initialization. Both selector branches
are checked with their actual native initializers. Shared timed RTL handling
also replaces identical return implementations in party and diagnostic code.

Inlining checks the indexed hook-page registry. Only the exact registered
ripple clock callbacks at A736/A74F/A760 are accepted as certified owners;
foreign callbacks, misplaced owned callbacks and observers retain fallback.
The strict quiet-range API keeps its original behavior. No hook is added or
removed, and the existing NMI ripple and counter-wait batches remain intact.

A prior diagnostic fixture used a permissive quiet-range stub and missed
the production predicate's 256-byte limit. Its 8E:B0D4..B288 interval now
uses two bounded ranges. Before this correction the guard always refused
native execution, despite selected bindings and passing semantic fixtures.
The replacement proof executes the actual production range implementation,
with registered clock identities and explicit foreign-hook negative cases.

The shared checkpoint passes 21504 portable ROM comparisons and 27 no-access
refusals. The consumer passes 32768 timed calls, including 128 actual composed
initializers, 18 child-return contexts and 376 read-only admission cases.
7936 unknown-target handoffs preserve malformed/unrecognized selector values;
they are not initial-entry fallbacks. Production range checks additionally
pass 24576 indexed-versus-linear comparisons and 802 admission scenarios.
The shared RTL/range changes pass 32768 existing party and 58640 diagnostic
timed cases, preserving their CPU, WRAM, fetch/data phases and guest clocks.

Consumer timing currently admits binary arithmetic in synchronized ordinary
contexts; unsupported modes, NMI/IRQ execution, observers and device deadlines
retain original execution. The standalone routines preserve their declared
ROM contracts. No current gameplay frequency, before/after whole-frame cost
or FPS result is established by these synthetic checks. Full regression is
deferred in favor of the affected shared union and incremental Release build.

## Paired-call timing for fixed battle messages — 2026-11-02

The existing `$85:94E7` copier transfers 37 bytes through the original WRAM
port. Its old consumer bridge omitted the callee's guest clocks: a real
`$81:AD96` caller executed eleven caller cycles instead of the original
680 caller-plus-callee cycles. The replacement accounts for all 205 callee
opcodes (669 cycles), preserves every original data/MMIO access and RTL,
and uses the existing semantic C without duplicating its algorithm.

The ordinary paired AOT route did not populate the hook-only readiness
flag. A separate read-only context now records the current CPU, target and
original waiting/stopped/NMI/IRQ state. It is tracked only for this copier
or nested calls within an active record, and restored before propagation.
Other native bindings retain their existing readiness and batching rules.
This context is not permission to skip timing, observer or hardware checks.

Native copying currently requires a ROM source without index wrap, a paired
three-byte frame and a quiet vblank interval. The 6,144-master-clock reserve
exceeds the conservative 669*8 + five 40-clock refresh charges = 5,552 bound.
Unsupported and feature-off contexts use the original interpreter. No extra opcode
hooks, shortened waits, deleted MMIO or altered sound handshakes are used.
The certified 1,024-opcode wait budget and native APU-idle caller remain.

Strict MSVC/Clang checks cover 16,384 timed inputs per compiler and 46
refusals. Another 4,096 cases cover context masks, identities, nesting and
restoration. Real generated and linked callers reduce interpreted opcodes
from 207 to two while preserving CPU/WRAM/hardware/refresh state. Foreign
hooks and insufficient deadlines retain all 207 original opcodes. Feature-off,
older-library-header and four CMake activation gates pass.

Matched counter-off replays preserve CPU/WRAM/hardware/PCM/APU idle across
600 menu, 600 field and 1,200 boot frames. Neutrality-checked counters find
zero copier calls in these scenes. This is not an ordinary battle hit-rate
or game-wide speedup claim. Single paired CPU/APU times are noisy; no FPS
acceptance is inferred. The new `$81:AD96/$81:AEEA` semantic entries still
need their own timed parent integration. This prerequisite adds no new
decompiled algorithm or canonical native binding.

## Fixed-message parent timing and paired APU context — 2026-11-02

The verified AD96/AEEA message routines use immutable bounded own-opcode spans
between child calls. The existing copier is composed only after all its timing,
stack, source, hook and hardware guards. Unknown/unsafe returns resume the exact
original landing; real frame yields retain interpreter continuation ownership.
The bridge does not add per-opcode hooks or claim that every later frame runs
natively. No new semantic algorithm is counted for this consumer checkpoint.

The scoped paired caller must also propagate and restore the interpreter's
`auto_quiescent` context. Merely recording interrupt readiness loses the original
APU progress-flush partition. A real production comparison caught a 7e-15 APU
remainder difference despite matching CPU, RAM and total clocks. The corrected
producer restores both contexts before every return or unwind. Tracking remains
limited to copier/message targets and nested calls inside those scopes; other
AOT calls do not save or copy these contexts. Certified wait/APU batches remain.

Eight production pairs and shared menu/field/boot replays pass with bit-identical
serialized hardware and PCM. Matched hot-path timing and organic battle use are
separate evidence; this checkpoint claims neither a battle hit rate nor 60 FPS.


## Progress ownership at timed native returns

The staged-command integration exposed two consumer errors despite matching
CPU, WRAM and guest clocks. RTS/RTL timing reused a read-progress output for
each fetched/stack byte; a later quiet read erased an earlier changed-read
notification. Accumulate progress across the complete original instruction.

A timed child also consumes its progress checkpoints while executing. When
an interpreted caller resumes, update its saved write/dynamic epochs for the
certified battle-call class. Otherwise it processes the child's old progress
again at a later instruction. These two errors changed APU flush grouping and
the serialized fractional catch-up value in six mixed fallback/native cases.
Accumulating return progress and transferring checkpoint ownership restores
bit-identical hardware state; no floating tolerance or APU work suppression is
used. The instruction clocks, guest bus accesses and relative APU phase remain.

The generated interpreter must receive the same feature definition and call
classification as the native body, including return-PC capture. A direct unit
call cannot prove this: the first actual caller retained all266 original
window opcodes until producer feature propagation was corrected. The accepted
checkpoint checks actual generated/linked callers, unknown and mutating hooks,
21 availability cases and missing producer anchors. Guard-only CMake changes
reuse a completed build only after identical generated source is established.

### Sound-poll admission audit � 2026-11-03

Sound poll runs now certify all registered hooks across the read/branch span.
Foreign callbacks and observers retain individual opcode dispatch. Certification
uses the existing page index and runs only after hardware admission succeeds.
The wrapper no longer evaluates the same initial hardware predicate twice.

Visible-line sound admission retains its HDMA fence whenever any channel is
active or initialization is pending. With neither, the normal scanline fence
applies; beam clocks, refresh, IRQ deadlines and APU handshakes still execute.
Counter-wait certification and its opcode budget remain unchanged.

Both compilers passed 30,720 original-interpreter timing/state cases, 49,152
admission combinations and 120 hook cases. Deliberately bypassing hook or HDMA
checks failed. A linked 600-frame replay preserved 221,621 foreign-hook calls,
CPU, WRAM, hardware state, PCM and native APU idle work. Release and optional
feature modes compiled. No standalone routine or new binding was added.

Matched uninstrumented timing samples varied; they do not establish a rendered
speedup or an FPS result. Headless boot measurements cover a different path.
Private source hashes, cost samples and proof scope are recorded in
`.agent/decomp/runtime-frontier/sound-admission-audit/ACCEPTANCE.json`.

### Measured swirl admission checkpoint � 2026-11-03

A local original-field-command battle replay identified late-scanline refusals
in the existing Mode7 transition. The register-only advance now reserves its
20 opcode bytes, 48 internal clocks and 40 refresh clocks, with the original
half-table and final-row branch costs. MMIO preparation and storage retain
their previous budgets. No routine, adapter API or binding was added.

Partial spans reject interior hooks. The preferred complete row certifies its
existing three callback positions, so those hooks still permit full batching;
foreign callbacks and observers retain their original route. Both compilers
passed 24,576 ROM cases and rejoins, 32 complete 224-row table compositions,
120 context refusals, 108 temporal checks and 15 hook cases. The previous hook
policy and fixed-budget negative controls failed the new checks. Windows
Release and the full-row-only feature configuration compiled.

Matched 900-frame production replays preserved CPU, WRAM, hardware, PCM and
native APU idle work. A foreign-hook comparison retained all 7,168 callbacks.
Passive admission counts kept the complete row at 4,461 native calls; the
partial advance increased from 1,283 to 1,608 and individual interpreter
opcodes fell from 237,023 to 233,447. Three alternating rendered comparisons
with the uninstrumented production interpreter measured CPU/APU medians of
404.462 and 400.440 ms per 900 frames. These scoped samples are not an FPS
or whole-game performance result. Detailed provenance is private at
`.agent/decomp/runtime-frontier/battle-frontier/ACCEPTANCE.json`.

## Effect-slot hook ownership - 2026-11-03

The existing four effect-slot phases now certify every executed header and
advance span against the indexed pre-opcode hook registry. Only their four
exact native callback keys are owned. Foreign callbacks and observers retain
original execution; independently safe header/cursor prefixes remain usable.
The old full path could bypass an interior callback. A matched 900-frame battle
replay now preserves all 53,956 foreign calls; the old consumer retained 8,593.
CPU, WRAM, hardware, clocks, PCM and native APU idle state match literal stepping.

Admission derives the header/advance cost from actual ROM speed, timer state
and counter branch. Refresh reserve is computed once, before indexed ownership
lookup. The duplicate advance retry and refresh preflight were removed. This
changes consumer admission only: no standalone semantics, API, binding or new
algorithm. Strict MSVC and Clang each pass 11,280 ROM cases, 1,128 decimal cases,
96 hook cases and 12,439,680 independently derived timing windows, plus refusal
guards. The isolated Windows Release and prefix-disabled compilation pass.

In the same battle fixture, raw interpreted opcodes fall from 233,447 to
229,077. Three alternating uninstrumented CPU/APU pairs measured
438.078/403.961, 395.924/404.049 and 410.719/399.722 ms per 900 frames.
These mixed, noisy costs establish no reliable general speedup or FPS claim.
No full regression or public CLI export was rerun for this local consumer fix.
Private receipt: `.agent/decomp/runtime-frontier/effect-slot-admission/ACCEPTANCE.json`.

## Width-expanded menu semantics - 2026-11-03

The existing clear-tilemap and hide-sprite C bodies at 84:A4DD/A4FD now preserve
all four original entry widths. Canonical entries stay M1X0; dynamic exits
reflect PHP/PLP restoration. Each compiler passes 512 original-ROM cases and
six untouched refusals. This is an expanded semantic contract, not a new
algorithm or binding. Both existing bindings remain selected and verified.

Do not widen their existing untimed consumer just because those cases pass.
An actual matched 480-frame F0 casino comparison rejected that experiment:
WRAM matched, but CPU/hardware/PCM/native APU idle state diverged. The consumer
width change was removed. With canonical admission retained, matched 480-frame
F0 and F4 replays preserve all of those states. Additional live widths require
clocked loop boundaries rather than a whole-routine admission override.
Private receipt: `.agent/decomp/runtime-frontier/special-menu-widths/ACCEPTANCE.json`.

## Clocked menu-clear continuations - 2026-11-03

The clear-tilemap and hide-sprite bodies now share their existing C helpers
with eight original continuation phases. Timed interpreter hooks consume one
admitted phase, retaining original RTS handling. Their existing canonical
whole-call wrappers keep the previous width restriction. No whole-call
ANY_WIDTH override, new ROM algorithm or binding is introduced.

Admission checks exact indexed hook ownership across each executed byte range,
including operands. Unknown hooks and observers retain original execution.
Static timing tables include the runtime's indexed 16-bit write charge, with
no per-iteration table copies. Refresh, beam/device limits, DMA/IRQ/NMI and
APU/deadline coherence are checked before execution. The feature gate requires
both verified whole-routine contracts and the new phase API/source; reference
and unsupported configurations keep the existing path.

Each strict MSVC/Clang clock proof passes 3,072 positive states, 282 guards and
6,912 independent original-ROM-cost windows. The source-composition proof also
checks 300,288 intermediate CPU/data boundaries per compiler. The incorrect
indexed-write timing control fails. The exact target definition, compiled
callback/semantic reference and feature-off compilation were inspected; all
25 availability mutations pass.

Matched 480-frame runs for all eight casino selectors preserve CPU, full WRAM,
hardware, PCM and native APU idle state. In F0/F4, foreign callbacks remain
1,152/2,304 respectively. Passive actual-decision counts show 977/1,954 native
phases and raw interpreter opcodes 52,665->45,551 / 18,429->4,200. Each raw-opcode
reduction equals the new phases' retired opcode count in these fixtures.

Three alternating uninstrumented pairs per selector give median scoped CPU/APU
costs 180.601->176.533 ms (F0) and 116.706->113.912 ms (F4), per 480 frames.
F0 paired results vary. These measurements establish no whole-game or FPS claim.
No full decomp regression or public CLI export was repeated for this checkpoint.
Private receipt: `.agent/decomp/runtime-frontier/menu-clear-phases/ACCEPTANCE.json`.


## Special-menu fade entry widths - 2026-11-03

The existing fade consumer now admits original M0X0 headers at 84:A5B5/A5CA.
Original PHP/SEP/PLP, frame waits and M1X0 child/continuation guards remain.
No new hook, adapter, binding, lookup or timing model is added. The canonical
entry metadata stays M1X0; restored exit width is correctly dynamic.

The shared private MSVC ROM/ABI checkpoint covers both entry widths, child
unwinds/context changes, saved-status continuations and admission guards.
Current isolated Windows Release and matched generated/linked callers pass.
Two 480-frame selector F0/F4 replays retain CPU/WRAM/hardware/PCM/native APU
idle. Each moves one M0 fade header from fallback to native: raw opcode counts
45551 -> 45545 and 4200 -> 4194. Existing waits/resumptions remain admitted.
Single CPU/APU samples are recorded in the private acceptance receipt; six
fewer opcodes do not justify attributing the observed timing variance to this
change. No whole-game performance or FPS claim; no full regression repeated.
Receipt: .agent/decomp/runtime-frontier/special-fade-widths/ACCEPTANCE.json.


## Menu input entry widths and hooks - 2026-11-03

The existing 84:9F1A input consumer admits the widths normalized by its original
PHP/SEP/REP/PLP header. Its verified semantic body and timing table are unchanged;
no function, binding, adapter API or hook is added. The other three table-round
roots remain wide-only. Input admission now checks the existing indexed range
API, preserving foreign hooks and observers across the complete original body.

The shared private MSVC checkpoint covers 4096 original-ROM/timed states across
all four widths, 352 current admission cases and nine sibling-width refusals.
Current isolated Windows Release and actual linked callers pass. Matched F0/F1
480-frame replays retain CPU/WRAM/hardware/PCM/native APU idle. F0 native/fallback
calls and raw opcodes are unchanged. F1 adds 274 native M1 calls: raw input
opcodes fall 21291 -> 78; every other raw PC count remains identical (7845).
One boundary-limited M1 call still declines normally.

An independent original-interpreter control confirms all 318 interior hook
callbacks. The old consumer skipped 43 through its M0 batch. A rejected draft
incorrectly used FrameReturnCpuReady, whose blocked default only has meaning
inside pre-opcode-hook scope. It is absent from the final ordinary-call guard.
Applicable hardware, interrupt, deadline and refresh checks remain intact.

Three alternating matched F1 CPU/APU samples are recorded in the private
receipt. Their median falls 234.0997 -> 208.7560 ms/480 frames, but one pair is
substantially slower and host variance is large. This establishes fewer
interpreted operations, not stable whole-game speedup or FPS. No full regression
or public export repeated. Proofs remain private under
.agent/decomp/runtime-frontier/input-command-widths/ACCEPTANCE.json.


## Standalone decimal paths, existing consumer admission - 2026-11-03

Six existing semantic routines complete D1 behavior without new bindings,
wrappers, hooks, timing models or host guards. The binary seed path remains
ordinary C. Direct D1 consumer admission stays excluded: source correctness
alone does not certify its decimal opcode clocks or caller ownership.
Current production-feature ABI checks confirm384 native D0 calls and384
unchanged D1 refusals. Return metadata peeks are separately checked.
Matched600boot/960menu frame pairs with old/new semantic bodies retain
CPU/WRAM/hardware/PCM/native APU idle. Scoped CPU/APU costs are recorded
in the private decimal-semantic-paths acceptance receipt; no speedup or FPS
claim. Standalone proofs cover9792 states each with MSVC/Clang and include
a failing binary-seed mutant. No full regression or public export repeated.


## Timed display-transition polls - 2026-11-04

Three original LDA-absolute/branch loops (`83:85AF`, `8E:B2BD`, `8E:B2E6`)
reuse the existing bounded timed poll runner. The only new hook sites are their
three disjoint loop entries. No new timing engine, idle skipping, global table
scan, canonical binding or gameplay algorithm is introduced. Original byte
reads, flags, branches, guest clocks, refresh, device deadlines and APU work
remain. Range ownership rejects foreign interior hooks and global observers;
existing certified counter waits keep their hooks and batching.

The shared checkpoint passes 99,081 ROM/guard cases per MSVC and Clang, three
independent opcode mutation controls, actual old-header consumer compilation
and unmangled C++ public linkage. Actual generated/linked producer objects and
passive native-counter arms agree in matched 540-frame transition and 1,580-frame
cave replays. Full guest/device state, extended APU clocks, nonempty PCM,
native APU idle steps and final pixels match. Only a live-checked host RAM
pointer is normalized. The transition uses 57,463 plus 56,428 native iterations;
raw opcodes fall 250,398 to 22,616, with every other raw context unchanged.
The third loop has zero observed admission in these scenes; its original-ROM
semantic coverage is separate from runtime adoption.

Actual foreign-branch and global-hook controls retain all 125,129 and 250,258
witnesses, respectively, and admit zero native iterations. Three alternating
matched CPU/APU/PCM samples for 540 frames measure 241.0303/253.8792/250.7930 ms
before and 155.6850/163.1315/168.0961 ms after. Setup, drawing and detailed
profiling are excluded. This small Windows sample establishes scoped native
adoption and lower measured cost, not whole-game speedup or console FPS.
Full regression, other-platform builds and public CLI export are deferred;
build definitions, source selection and canonical metadata are unchanged.
Private receipt: `.agent/decomp/runtime-frontier/display-transition-polls-current/ACCEPTANCE.json`.


## Shared actor contact geometry - 2026-11-05

Legacy field-trigger scanning and the complete actor-contact routine share one
private original overlap body at `83:B8DF`. No public ABI, native binding, hook,
timing engine, source list or admission predicate changes. Both call paths keep
their original flags, ordered reads, scratch writes and guest call/return frames.

The shared checkpoint reuses unchanged MSVC/Clang proofs of 65,536 ROM-fragment
states each, including a 32,768-state legacy binary-equivalence subset. A changed
original load opcode fails. The resumed isolated Windows dependency build and
actual producer relink pass. Matched 3,298-frame field and 1,580-frame cave arms
retain full guest/device state, extended APU clock, PCM, native idle, raw opcode
contexts and picture. Passive counters match the real producer and report
12,710 complete-contact calls in the cave; field/legacy calls are zero there.
No native-admission claim is made for an unexercised path.

COFF code in the two affected modules decreases by 1,600 bytes. Initial unpinned
samples were slower; controlled same-core wall times vary in both directions
(507.3723/479.1349/489.1375 ms before; 493.8704/492.0267/480.9239 ms after).
Thread CPU values are quantized; this small sample establishes neither a stable
speedup nor a repeatable slowdown, and gives no console FPS or whole-game claim.
No full regression, other-platform rebuild or CLI export was repeated for this
source-only sharing. Private receipt:
`.agent/decomp/runtime-frontier/actor-contact-sharing-current/ACCEPTANCE.json`.


## Late display-poll admission - 2026-11-05

Only the three existing display-transition polls may start through hPos 1264;
sound and counter polls retain 1240. New positions require the actual global
CPU, vPos below 262 and a current refresh clock with phase below 1364. The
original M1 absolute load and same-page branch cost at most 54 master clocks;
the actual refresh implementation adds at most 40. The resulting 1358 endpoint
precedes the shortest 1360-clock scanline. The 128-clock deadline reserve,
IRQ/NMI, DMA, HDMA, beam, APU and exact indexed hook checks remain mandatory.

MSVC and Clang each pass 1,158 actual admission cases, 103 registry cases and
5,456 actual refresh cases. Removing refresh freshness fails the negative
control. Matched 204/540-frame production replays retire 4,406/4,400 fewer raw
opcodes, with 2,203/2,200 extra native iterations. Full state, extended APU
clock, PCM, picture and native idle agree; 1,580/3,298-frame controls are
unchanged. Foreign and global observers remain exercised with identical opcode
profiles. Passive native-counter arms also match their unobserved arms.

Three alternating same-core pairs measure 204-frame wall times of
88.50/85.64/87.25 ms before versus 83.75/83.74/91.14 ms after, and 540-frame
times of 183.96/172.35/181.51 ms versus 170.14/171.13/180.80 ms. Thread CPU
timing is coarse; the short scene varies both ways. These scoped CPU/APU/PCM
measurements do not establish a general speedup or whole-game FPS. Drawing
and setup are excluded.

The isolated agent Release and optional-API-off compilation pass. Original
transition semantics and their 99,081-case MSVC/Clang ROM proof are unchanged
and reused; no semantic entry, API or binding is added. Unrelated full suites
are deferred. Raw-profile hrv0 is a hardcoded sentinel, not a sampled caller
flag; passive production-hook observation measured host-return-valid 1.
Private receipt: `.agent/decomp/runtime-frontier/late-transition-window-current/ACCEPTANCE.json`.


### Sound transfer single-opcode admission

The single-opcode caller retains APU clock coherence and uses the existing
hardware window predicate when HDMA beam tracking is enabled. That predicate
still fences active or pending visible-line HDMA, IRQ/NMI, DMA, refresh,
stack bounds and the frame deadline. The separate relaxed scanline route
continues to require the beam-off flag. Global observers retain the previous
route in the newly admitted contexts. No original sound opcode or handshake
semantics change.

The scoped checkpoint compares all 58 sound-transfer opcode boundaries with
both MSVC and Clang: 9,948 admitted states, 4,900 refusals and 1,450 no-access
guards per compiler. Original-ROM CPU, WRAM, ordered accesses and opcode clocks
agree. Actual SDK hook replays preserve 733 foreign-hook and 1,212 global
observer calls. The 1,800-frame cold replay preserves complete saved state,
extended APU clocks, picture, native APU idle count and 972,311 PCM samples;
raw interpreter calls decrease from 42,641 to 33,629. Native single-opcode
execution rises from zero to 9,012 steps in that same replay.

Three serial alternating 600-frame comparisons with profiling disabled have
candidate/reference runtime ratios 0.947, 1.036 and 0.995. This is not a reliable
speedup or whole-game FPS result. Unchanged standalone semantics and unrelated
regressions reuse earlier evidence; no full regression or owner build was run.


### Boot NMI wait and certified logo batches

The internal `80:8084/8086` phase uses the original standalone boot NMI-bank
read and branch through a scoped continuation. Native admission retains
M1X1/DP0, coherent APU clocks, stack, IRQ/NMI, DMA/HDMA, refresh, deadline and
observer checks. Multi-operation retirement also requires the actual owned-span
budget to admit at least 16 opcodes. The existing counter-poll classification
and APU idle seams are unchanged.

The logo wait kernel runs before opcode callbacks. Its two hook slots accept
only the exact certified callback identity and PC. Compatibility is maintained
when the SDK registry adds, replaces or clears hooks; rejected full-table
insertions do not alter it. The hot wait reads two cached blocker flags.
Installation also preserves pre-existing foreign callbacks and global observers.
Foreign callbacks retain their original route. This prevents the native hooks
from blocking the already certified wait batches or adding repeated ownership
calls to the hot path.

The shared checkpoint passes 16,384 original-ROM step states and 1,273 guards
per MSVC/Clang, plus the affected full boot matrix. Timed integration passes
1,024 cases, 56 guards, four ownership cases and 18 instruction budgets per
compiler. Actual SDK registry checks cover 16 mutations and five identities,
including mirror keys, clear-all, replacement, restoration and table capacity.
API-off composition passes; disabled logo-wait generation adds no cache dependency.

In the paired 1,800-frame cold replay, 780 native calls retire 917 instructions;
raw interpreter calls decrease from 33,629 to 32,712. Complete saved state
(after one validated host pointer is normalized),
extended clocks, picture, 972,311 PCM samples/hash, 165,698 native APU idle
steps, counter metrics and 3,074,869 logo wait pairs agree. Separate foreign
and global observer replays preserve 1,492,459 and 593 callback invocations.

Three separate-image 600-frame timing pairs have median candidate/reference
ratio 1.0068. A registration-only reference toggle in one linked image gives
five alternating ratios from 0.8996 to 1.0335, median 1.0133, with identical
saved state, clocks, PCM and wait counts. These measurements distinguish
integration cost from image layout; they do not establish a reliable speedup
or whole-game FPS result. No original wait, sound handshake or guest work is
removed. Unrelated full regression and other platform integration were deferred.
