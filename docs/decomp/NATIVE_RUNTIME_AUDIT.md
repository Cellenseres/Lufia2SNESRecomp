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
