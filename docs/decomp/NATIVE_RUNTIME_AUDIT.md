# Native execution and runtime fallbacks

This audit covers root `8b3b2d7` and decomp `b80e8d3`, using the existing
5,399-frame battle replay. It measures actual production predicates and
callbacks, rather than inferring execution from selected bindings.

## What the counters mean

The replay contains 690,218 interpreted opcodes and 7,778 explicit outer
fallback calls. Every one of the 17 distinct fallback entries already has
verified semantic C. An outer fallback can continue through native internal
spans; its count does not mean that its entire body remains interpreted.

The bridge-entry counter records 32,773 calls, but its instrumentation excludes
wrappers with four-digit names. It is not a complete native-call count, and
entry does not prove completion. The raw opcode census hard-codes host-return
context to zero; that field cannot establish actual admission conditions.

The catalogue has 2,305 own native bindings among 2,568 known entries, or
89.76 percent of that inventory. This is neither whole-game decompilation
coverage nor a percentage of runtime work. Internal native spans, original
aliases, unreferenced fragments and dynamic instruction frequency differ.

## Existing native work behind the largest residual loops

Additional counters surround the unchanged production predicates and completed
callbacks. They are enabled only during the measured replay interval.

| Internal entry | Attempts | Native spans | Native work |
| --- | ---: | ---: | ---: |
| `81:D58D`, target-name fill | 27,982 | 18,715 | 111,437 bytes |
| `81:D7C5`, target-name fill | 565 | 376 | 2,243 bytes |
| `81:D8CC`, target-name fill | 143 | 100 | 597 bytes |
| `81:8A7E`, effect-script slot | 43,110 | 41,266 | 377,836 opcodes |
| `81:8AC1`, effect-actor slot | 91,968 | 88,276 | 805,220 opcodes |
| `81:8AAA`, script cursor | 7,289 | 4,361 | 31,098 opcodes |
| `81:8B1B`, actor cursor | 15,274 | 9,007 | 64,054 opcodes |
| `81:8AB4`, script counter | 4,823 | 2,928 | 5,856 opcodes |
| `81:8B25`, actor counter | 10,268 | 6,267 | 12,534 opcodes |

The effect-slot spans alone execute 1,296,598 original opcodes natively.
The three name-fill spans execute another 342,831. These are partial measured
totals, not the denominator of a whole-game native execution percentage.

All 9,499 name-fill rejections occur because no permitted chunk fits the
remaining beam/deadline window. None reaches a rejection for bank, width,
page, return context, clock synchronization, refresh state or inner hooks in
this replay. The largest remaining name loop is therefore not missing C.

Every recorded rejection of the larger effect-slot predicate occurs in its
clock-window check, rather than context/grid or hook ownership. Some of these
calls still admit an existing smaller native span: 14,808 header-only spans
and 5,896 cursor-only spans. Primary rejection and final fallback counts must
not be conflated. This audit does not attribute each rejected clock window
to a specific beam, DMA, interrupt or scheduler condition.

Preserve these event boundaries. Removing admission checks or adding duplicate
semantic bodies is not justified by a residual raw opcode count.

## Explicit outer fallbacks

| Entry or group | Calls | Interpretation |
| --- | ---: | --- |
| `85:ECDB`, VRAM queue | 5,829 | Existing partial native queue hooks remain available |
| `80:8703`, NMI uploads | 624 | Existing clocked native consumer has conservative admission |
| `80:834C`, multiplication | 277 | Verified body and clocked consumer already exist |
| `80:8378`, division | 203 | Verified body and clocked consumer already exist |
| Ten battle frame/gauge entries | 836 | Existing verified bodies; entry counts do not establish rejection reasons |
| `83:A746`, `86:974F`, `80:8E9D` | 9 | Six, two and one calls respectively |

The queue accounts for 74.94 percent of explicit fallback calls. This does
not establish 74.94 percent of interpreter time. Existing native queue uptake
must be included before assessing any replacement.

## Next useful boundary investigation

Original ROM decoding and the measured raw census identify command-menu child
rejoins as a separate remaining opportunity:

| Boundary | Raw opcodes | Original operation |
| --- | ---: | --- |
| `81:CBB2..CBB8` | 12,447 | Restore tile index, compare, choose next record |
| `81:CBF1..CBFF` | 8,298 | Evaluate the special input condition before sound calls |
| `81:CC07..CC1A` | 8,292 | Test button masks and route to redraw or a sound call |
| `81:CC1A..CC2E` | 24 | Produce the selected command result after sound |

These operations already exist in the complete standalone parent. A consumer
extension would reuse them, preserve renderer/sound/frame-input children,
and require original-ROM boundary, clock, observer and actual-caller proofs.
The first boundary is only three opcodes; an extra dispatch may cost more
than it saves. Prefer a coherent operation over one hook per instruction.
Counts establish opportunity, not a predicted speedup.

## Evidence and limits

The instrumented replay exactly matches the accepted candidate's CPU, full
WRAM, hardware, image, PCM, sample count, APU clocks, idle steps, bus credits,
all eight APU invocation counters, previous command-tile uptake and complete
raw PC/mode/bank/page/stack census. No production source or binding changed.

Private evidence is under
`.agent/decomp/runtime-frontier/admission-audit-current`, including
`ACCEPTANCE.json`, `ANALYSIS.json`, original boundary decoding and input hashes.
Diagnostic sources and executables remain local. Existing unchanged semantic
proofs were reused; no new full regression, CLI build, performance benchmark
or whole-game acceptance is claimed.

## Rejected command-input consumer

The next evaluation shared the existing parent's input tests with bounded
continuations at `81:CBF1` and `81:CC07`. It preserved original sound children,
redraw, flags, ordered reads and CPU modes. A combined checkpoint passed
458,752 original-ROM cases, 14,336 parent comparisons and three guards.
Production integration, 224 actual admission checks, 112 actual generated
caller cases, eight capability cases and strict C/C++ compiler checks passed.

The measured replay admitted 1,042 first-phase and 148 later-phase spans,
removing 13,380 raw opcodes. Complete guest/audio/clock endpoints matched;
all 5,532 observer snapshots and the outside-span raw census were unchanged.

Performance did not meet the acceptance bar. Two alternating isolated
feature pairs measured CPU/APU medians of 0.527896 versus 0.536213 ms/frame,
about 1.58 percent higher with the candidate. Two alternating comparisons
against the previous complete production binary measured 0.519166 versus
0.555663 ms/frame. Both comparisons were slower in every pair. The full-before
difference is not attributed exclusively to the new hooks: other shared
build differences can contribute. The unchanged raw census rules out an
observed loss of existing interpreter batching in this replay, but does not
explain the additional host cost.

The consumer is therefore **not adopted on main**. The semantic and consumer
implementation remains on private `codex/command-input-evaluation-20261112`
branches: child `8e0e78d`, superproject `b5c9763`. Neither was pushed. Main
retains its previous production source and binding selection. The agent's
evaluation binary is not a rebuilt main release.

Private evidence is under `command-input-current`, with status
`REJECTED_COMMAND_INPUT_COST`, exact tested source/object snapshots and source
resolution. This experiment adds no canonical function or algorithm coverage.
Future work should favor larger coherent native operations over more short
per-opcode hook boundaries, and retain separate semantic and cost gates.

## Complete selected-wrapper census

The main agent cache was rebuilt after rejecting the input experiment.
Diagnostic instrumentation now maps wrapper names through the actual selected
binding report, including four-digit names and the player-update source unit.
All 2,305 selected wrapper definitions are covered; three unselected legacy
definitions are excluded. This instrumentation is local, not a runtime change.

In the same 5,399-frame replay it observes 472 distinct wrappers and 116,569
entries. The former six-digit-only counter recorded 32,773 entries, missing
83,796 entries through four-digit wrappers. Its original subset agrees exactly
with the new census, including guest register and stack contexts.

| Entry | Wrapper entries | Normal returns | Other returns | Same-entry fallback |
| --- | ---: | ---: | ---: | ---: |
| `81:B705`, append OAM sprites | 13,314 | 13,314 | 0 | 0 |
| `81:BE58`, tile renderer | 7,342 | 7,342 | 0 | 0 |
| `85:ECDB`, queue slot | 5,961 | 132 | 5,829 | 5,829 |
| `80:8703`, NMI uploads | 5,399 | 4,775 | 624 | 624 |
| `85:EC81`, frame input | 4,389 | 0 | 4,389 | 0 |
| `83:BBF3`, player update | 104 | 104 | 0 | 0 |
| `81:CB77`, command-menu parent | 6 | 0 | 6 | 0 |

There are 95,055 normal and 21,514 other wrapper returns, with maximum
wrapper nesting ten. Every one of the 7,778 explicit fallback events matches
the active wrapper's canonical entry. The remaining 13,736 other returns are
not additional guard fallbacks; they include cooperative child/frame handoffs.

No raw opcode is observed while a measured wrapper's C frame remains active:
interpreter work is resumed after those frames unwind. This explains why
attributing interpreter work to the nearest active native wrapper produces
zero in this replay. It does not prove that every logical parent executes
entirely natively. A normal wrapper return, a native child call and a native
internal hook are different observations. Inline semantic helpers and internal
hooks are not additional entries in this wrapper census.

The complete raw census remains exactly 690,218 opcodes and the explicit
fallback census remains 7,778. CPU/full WRAM/hardware/image/PCM, sample count,
APU clocks and all eight APU counters, idle steps and bus credits match the
accepted replay. No new semantic body, binding or performance gain is claimed.
Unobserved wrappers are not thereby unused or unimplemented elsewhere.

Evidence is local under `native-census-current`. Future investigation should
measure timing and bus contracts of the existing high-frequency native leaves
before duplicating them or adding more short continuation hooks. Entry counts
alone provide neither host cost nor original guest-clock fidelity.

## Confirmed leaf timing gap

Automatic entry/exit snapshots from the same replay now expose a consumer
timing gap in `81:B705` and `81:BE58`. Seven captured calls match the original
ROM's CPU registers, flags, stack and complete WRAM, but debit zero CPU and
master cycles. Original callee costs, including RTS, are 158, 614, 1,191,
1,462, 1,469 and 1,925 CPU cycles for the six OAM calls, and 153 for the tile
call. The original JSR costs another six cycles; it does not pay the callee's
variable cost. Generated forwarders, dispatch and interpreter return handling
provide no compensating leaf charge on these measured paths.

The actual tile call is the original JSR at `81:CBAF`, returning to `81:CBB2`.
OAM samples come from `81:B5E3`, `81:B61C`, `81:B62F` and `81:B656`. This
finding concerns these recorded paths, not every caller or the whole game.
It does not establish the cause of a visible animation or movement symptom.

A shared original-ROM checkpoint covers 1,024 OAM states and 12,289 tile
states, comparing CPU/full WRAM and complete instruction, CPU-cycle and bus
ledgers. Compact cost models match all 13,313 cases and all seven live
snapshots. OAM cost depends on count and packed-attribute alignment; tile
cost depends on dimensions and atlas wrapping, including original carry and
16-bit overflow. Encoded zero dimensions execute 256 iterations. The models
describe timing of existing C algorithms; they add no function coverage.

This investigation alone established neither a runtime fix nor performance
acceptance. It required a coherent complete-leaf consumer with admission,
refresh, hardware, APU/audio, open-bus and generated-caller proofs. Comparing
against the zero-debit path would not establish original clock fidelity.
The subsequent consumer checkpoint below supplies those scoped proofs.

Local evidence: `native-leaf-clocks-current/ACCEPTANCE.json`, live call-site
records and original-ROM ledgers. The capture replay preserves the complete
previous wrapper/raw census and guest/image/audio/APU endpoints. No full
regression or unrelated build was repeated for this audit.

## Clocked native battle leaves

The two entry bridges now use the unchanged standalone routines with complete
original-call timing. Compact OAM and tile cost models replace per-opcode host
handoffs; they preserve the original counters, packing alignment, atlas carry,
bus speeds and RTS. The actual generated caller now establishes their paired
return context. No opcode hook, canonical entry or game algorithm is added.

Admission checks CPU/RAM identity, original widths/bank/page, source/output/
stack separation, indexed hook ownership, quiet VBlank, deadlines, interrupts,
DMA and refresh. The absolute APU timeline must actually be active. Cold or
legacy relative APU contexts retain the original route: an early aggregate
prototype changed floating catch-up rounding there and was rejected. Unknown
in-range hooks, unsupported geometry and late windows also use the clocked
original interpreter, rather than the previous untimed native bridge.

The combined checkpoint passes 13,313 original cost-model cases and 98 actual
production caller/hardware/guard/observer cases. These include empty and packed
OAM returns, the OAM limit, tile carry, high destinations and encoded zero
width. The normal Windows agent target compiles and links successfully.
Standalone semantic source and its verified entry count are unchanged.

In the rendered 5,399-frame replay, all 13,314 OAM and 7,342 tile attempts
match the clocked-original control. The candidate admits 1,042 OAM and 1,008
tile calls; the remaining calls retain original fallback. Raw opcodes decrease
from 5,666,439 to 5,193,130. CPU, full WRAM, hardware, image, PCM, sample count,
APU clocks, native idle and bus credits remain bit-exact.

APU invocation counts legitimately differ: 31,589 cycles move from individual
native cycles into native idle batches, with 100,164 fewer idle callbacks.
Skipped plus individual native cycles remain exactly 97,043,965. No interpreted
skip batches appear. These host-work counters are reported separately from
the unchanged complete APU state and PCM.

Two alternating cost pairs use uninstrumented production actor, caller,
interpreter and APU objects, with host image submission excluded only from
this isolated measurement. CPU/APU run means are 0.984738 versus 0.743277 and
0.703049 versus 0.662567 ms/frame. Both pairs favor native execution, but the
large host variation limits generalization. This establishes no whole-game
FPS or universal speedup claim. Complete hardware matches between the equally
isolated arms; rendered acceptance is the separate replay above.

Private evidence is under `native-leaf-consumer-current/ACCEPTANCE.json`.
Historical census sources retain their exact original bytes and hashes;
their older measurements are not relabeled as tests of this consumer. Other
entry bridges and inline semantic helpers still need their own timing audits.


## Battle leaves before the next NMI

A read-only rejection census identified the previous VBlank restriction as
the main remaining barrier for these two leaves. The normal software-renderer
path already disables beam-driven HDMA and walks those tables itself. The
consumer now admits pure-WRAM leaf calls in that existing configuration during
visible scanlines, provided the complete original cost and refresh reserve fit
before scanline 225. VBlank calls must still finish before frame wrap. It never
disables HDMA to gain admission. Active beam HDMA, unknown observers, interrupts,
unsafe return contexts, legacy APU timing and late calls retain original execution.
Other consumers keep their previous VBlank-only predicate.

The shared checkpoint passes 158 actual hardware cases, 56 guard/observer cases
and 25,200 old/new VBlank predicate comparisons. Maximum OAM counts and encoded
zero tile dimensions are included. The unchanged original cost models reuse
their 13,313 accepted ROM cases after an exact C-body comparison.

Every one of 5,399 rendered frames matches the original control for CPU, complete
WRAM, image, cumulative PCM and APU clocks. Final hardware, native waits and bus
credits also match. Native OAM calls increase from 1,042 to 8,609 of 13,314;
native tile calls increase from 1,008 to 7,302 of 7,342. The remaining calls use
original fallback. Raw opcodes fall from 5,193,130 to 1,803,407, versus 5,666,439
with both leaves interpreted. All 1,017,996 opcodes outside these leaves remain
identical in the detailed counter census. Native APU cycles remain 97,043,965;
249,730 more cycles use idle batches than in the original control, with 819,196
fewer idle callbacks. These invocation counters are separate from state equality.

Two alternating, unprofiled production-object cost pairs compare the retained
VBlank-only executable with this consumer on identical inputs. CPU/APU means
are 0.734265 versus 0.475761 and 0.874849 versus 0.472724 ms/frame. Host image
submission is excluded only in these isolated measurements; rendered correctness
is checked separately. The two-run median of means improves by 41.1% in this
workload, with substantial reference variation. This is no whole-game FPS claim.
The normal Windows agent executable also compiles and links successfully.

Local evidence: `leaf-visible-window-current/ACCEPTANCE.json`. No standalone
semantic body, canonical entry, dispatch binding or opcode hook was added.
