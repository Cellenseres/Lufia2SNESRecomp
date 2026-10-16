## Sloped world windows — 2026-10-16

Ten complete entries:86:C831/C843/C854/C928/CA06/CA2F/CA5E/CA83/CAB2/CAD7.
They count edge rows, initialize scanlines, rasterize four signed slopes and
build upper/lower HDMA bands. Four directional leaves share one implementation.
Zero count still draws256 rows; zero complement angle and arithmetic overflow
retain M0 returns, while row exhaustion retains M1. CA06 accepts both widths,
restores its saved Y, and keeps the original indexed JSR frame and read order.
Header pointers, clipping, carries, wrapping and data-bank behavior remain exact.
The original C5F8 caller and its CB21 clipping subtree remain interpreted.
Operand-byte false entries from raw scans were excluded; no duplicates added.

One combined checkpoint passes45056 direct and22528 actual production-ABI
CPU/full128KiB WRAM/ordered-bus/child-snapshot cases,8 unwinds and20 altered
child returns per path,91 semantic plus140 additional consumer guards,15 child
redirects and14 in-memory ROM fault controls. Consumer contracts require
PB86/DB7F/DP0/D0/X16 andS1F20..1FFC with the documented entry M width.
Formatting afterward preserves exact C token streams; proofs are reused.

Three affected engine transitions0C/0E/0F complete in860/983/1021 frames with
2772 script steps. Across11028 frames per run, including7444 ordinary frames,
all PPU and pixel hashes match without alignment gaps. All ten new entries
execute natively, with no original entries at those addresses on the enabled
path. Ordinary CPU/WRAM/audio also match; scene/closure496 CPU/WRAM and1138
rawPCM hashes differ. Existing HLE clock limitations remain; no full-cycle or
whole-runtime audio equivalence is claimed. Existing circle timing eligibility
is retained on both sides. No synthetic delays or duplicate frames were added.

Read-only bank86 profiling of these three transitions measures5332021 original
instructions in the reference versus1136470 with the new bindings:4195551 fewer
(78.69%). This is scoped interpreter work, not whole-game coverage or an FPS
benchmark. GCC/Clang semantic and actual resource CLI checks, MSVC W4/WX proofs,
metadata and isolated WindowsRelease pass.1514 verified/1503 selected/752 WRAM,
462 portable C units;40 actual generated wrappers for ten new bindings.
Full regression is deferred for this revision. Private evidence is under
world-sloped-windows-next; ROM, unrelated WIP and tools remain uncommitted.

## Circular windows and scene sprite setup - 2026-10-16

Six complete contracts are reconstructed:86:C358/C40A/C444/C4A8 andE84F/E8B5.
Circular window construction keeps its zero-radius RTL, saved data bank, exact
headers/pointers and both row directions. Byte count0 still means256 rows.
The extent table retains PHP/SEP30, X/Y truncation, carried midpoint arithmetic
and status restoration. Lower-half field56 controls the original row stepping.
E84F keeps sixteen OAM attribute/high-field updates, both eight-record groups
and scene parameters. E8B5 keeps ADC6 carry across all eight records;byte4 is
untouched. ExistingE5E2/E5ED/E5FA/E609 packers are reused,not duplicated or recounted.

Three nonoverlapping WRAM spans are catalogued:1467 index,1EC2..1F21 records,
and neutral1219. The7F:DE00 extent table aliases existing actor/object storage;
no overlapping table allocation is invented. The original1467 counter naming
in world_map_objects has457 identical normalRelease executable/data/unwind and
relocation sections;debug/checksum sections are excluded from that comparison.

The shared12-root checkpoint covers six new contracts plusC2D9 and five resource
scene callers.41472 direct/20736 actual production-ABI cases compare CPU,all128KiB
WRAM,ordered operand/MMIO accesses and child snapshots. Each path adds480 child
unwinds and1200 changed returns.112 semantic plus167 additional consumer guards
pass;the ABI log repeats112 semantic guards.675 redirects cover17 distinct sites
and45 root/site occurrences. Fourteen in-memory ROM faults are detected.
The four affected ABI entries are rechecked after the consumer timing guard:
14336 cases and the ABI edge union pass;6400 unchanged ABI positives and all
41472 unchanged direct cases are reused. No identical semantic proof restart.

Consumer eligibility preserves two timing-sensitive preparation calls inB63B.
Exact live frames identify C2D9'sC314 far return,layer0 and itsB678/B67F caller
returns. C358 and its three row/table children retain the original path only
inside those frames. Stack peeks use the known WRAM buffer without guest bus
accesses. Twelve actual fallback frames and152 near misses prove the guard
without guest CPU/WRAM/bus mutation. E84F also rejects stacks below1F24 because
the original record stores reach1F21. Other tested contexts remain native.
Semantic C,software waits and original quirks are unchanged;no artificial cycles,
duplicate presentation frames or production shadow interpreter are introduced.

An initial runtime comparison exposed a real presentation difference at0E:
one fewer preparation frame also shifted later pictures. The bounded original
timing path above removes that regression. A separate sequential probe failed
during field closure before13 on both native/reference paths (vector00862D,
resumeD2C704). That experiment is retained as initial evidence. The final three
scenes therefore start from an exact saved runtime baseline rather than inherit
that unrelated carry-over. This does not claim arbitrary scene chaining or all
real-story preconditions. The in-process rollback API excludes ROM data;its
CPU/master-clock/fullWRAM/PPU fingerprint is checked before and after restoration.
DSP output-ring delivery is restored as well. No snapshot file or ROM export.

The final nine scene pairs all complete without failure:17094 frames per run,
including7444 ordinary and9650 scene/closure frames. All PPU/register/memory and
pixel fingerprints match. Frames for0C/0E/0F/10/11/12/13/14/15 are860/983/1021/
771/771/771/771/771/771;all7332 script steps match. Only the twelve explicitly
excluded preparation entries use original circle code on the enabled path.
Ordinary7444 CPU/WRAM/PPU/pixel/PCM hashes match. Scene/closure2750 CPU/WRAM and
604 rawPCM hashes differ,so whole-runtime CPU/audio/cycle parity is not claimed.
The pre-existing HLE cycle-accounting limitation remains separately documented.

Read-only original-instruction profiling of those nine scenes measures bank86
at9899677 reference versus7957355 enabled instructions:1942322 fewer (19.62%).
This is a scoped interpreter-work reduction,not whole-game coverage or an FPS
benchmark. Remaining hot blocks lie mainly in86:C8xx..CBxx and guide next work.

Strict MSVC,GCC/Clang semantic builds,actual resource CLI targets/tests,metadata,
isolated WindowsRelease and the incremental consumer rebuild pass.1504 verified,
zero drafts,1493 selected bindings,752 WRAM locations,461 portable C units
(460 sources plus CLI). Generation emits24 actual wrappers for six new entries.
Full regression is deferred on this revision;no new full-suite pass is claimed.

Private evidence:world-window-shapes-next/checks.json,timing-abi-checks.json,
timing-guards.log,counter-name-equivalence.json,compilers.json,runtime/comparison.json,
runtime/interpretation-profile.json andACCEPTANCE_RECEIPT.json. Initial failed
probe/logs remain separately named *-initial. ROM/foreign WIP/private tools stay
outside the commits. No push.

## Remaining world scene callers and window records - 2026-10-16

Fourteen complete contracts are reconstructed and bound:86:B15A/B59E/B63B,
BB5C/BE30/C04E,B19A,BBC4/BE72/BE7A/BEC4 andC321/C336/C2D9. All nineteen distinct
nonempty authored scene callbacks inAEB1's23-slot table now have semantic entry
contracts. Two empty RTS entries,the0000 target and theC00F alias remain exactly
as authored; this does not claim all descendants or a whole native scene VM.
Unknown descendants includingC358/E84F remain explicit original services.

B19A shares the existing BBE6/C08C loader. Actor group setup shares the existing
AFB2/AFE3 implementation with its original anchor coordinates. The transition
retains both scripts, both window submissions, fade, resource290, palette MVN
and object startup. BEC4 keeps its actualM0X0 entry/exit;BE7A guardsM0 returns
inside itsM1 parent. Positions, animation writes and exact child frames remain
observable. No software waits, duplicate stores or original quirks are removed.

C321 reads the hardware multiply result after the original register writes.
C336 copies all six original record bytes. C2D9 preserves four parameter reads,
the saved layer byte, buffer selection, farC358 child and selector increment.
Inspection ofC358/C4A8/C40A/C444 identifies circular-mask centerX/Y and radius;
the remaining field keeps its neutral name. No inferred rectangle replacement.
Existing1E50 records and battle/field scratch aliases are reused, without new
overlapping WRAM catalog entries.

The shared18-root checkpoint includes four affected prior callers.17920 direct
and8960 production-ABI cases compare CPU/full128KiB WRAM/ordered operand-MMIO
accesses and child snapshots. Each path also passes656 child unwinds and1640
changed-return contexts.178 semantic and252 additional consumer guards pass;
the ABI log includes the repeated178 semantic guards.1230 redirect probes cover
all82 distinct call sites. Fourteen in-memory ROM faults are detected.

The first ABI pass exposed2048 BEC4 fallbacks:its wrapper selected theM1 helper.
The corrected M0X0 adapter passes all2048 affected cases withzero fallback.
One fault control exposed correlated synthetic window-buffer inputs; independent
first-buffer parity covers the missing branch. Only1024 direct/512 ABI window
cases are repeated.16896 direct/6400 ABI positive cases and unchanged edge inputs
are reused, rather than restarting all checks. The final local-name correction
has identical137 COFF code/data/debug/unwind sections and relocation bytes;
all414 symbol records match after normalizing the one private helper name.

Strict MSVC,GCC/Clang semantic builds,actual resource CLI targets/tests,metadata
and isolated Windows Release pass. Final GCC/Clang rebuilds cover the naming
correction; unchanged CLI dependencies/results are reused.1498 verified entries,
zero drafts,1487 selected bindings,749 WRAM locations and459 portable C units
(458 sources plus CLI). Generation emits56 actual wrappers for14 new entries.
Full1965-job regression is deferred on this revision; it is not reported green.

All six original production scenes complete withzero failure. Each run records
13651 frames:7444 ordinary and6207 scene/closure frames. All PPU/register/memory/
pixel fingerprints match. Scene selectors0C/0D/0E/0F/10/16 take860/431/983/1021/
771/701 frames on both paths; script steps are800/420/962/1010/760/690 on both.
All fourteen new entry addresses execute, withno original entry for those roots
on the enabled path. No visible animation frame or script step is omitted.

Runtime equality remains scoped:ordinary7444 CPU/WRAM/PPU/pixel/PCM fingerprints
match. Across scenes/closures3804 CPU/WRAM and1104 rawPCM fingerprints differ.
Those samples are recorded, not newly localized individually. The prior
scene-resource timing review already establishes that the HLE bridge lacks
original instruction-cycle accounting and can shift unchanged sound-upload
progress. These unchanged timing contracts are not re-proven per entry; no
whole-runtime CPU/audio/cycle equivalence is claimed. No timing compensation,
semantic patch or shadow interpreter is introduced.

Private evidence:world-remaining-scenes-next/checks.json,name-equivalence.json,
compilers.json,compiler-final.json,generated-witnesses.json,runtime/comparison.json
andACCEPTANCE_RECEIPT.json. ROM remains at its original path; no ROM/private
tooling/foreign WIP included. No push.

## Scene resources, palette and actor groups - 2026-10-16

Fourteen complete bank86 entry contracts are reconstructed and bound: AFB2,
AFE3, BF70/BFF5/C002/C00F/C041, C2B3, C582, 966C, BB8C, BF99, BBE6 and C08C.
Five resource-scene variants share their actual BF7A tail, rather than duplicate
algorithms. Group scenes reuse actor/track/frame-script children. Palette
components saturate in original red/blue/green order; count zero retains256
iterations. C2B3 preserves incoming SBC carry. Window records preserve aliased
table writes and ADC carry. Palette MVN retains PHB/PLB and all register results.
Graphics upload reads the decoder's returned DP fields and preserves DMA/NMI
register order. Unknown children, including E84F, remain explicit services.

The14-root checkpoint passes20480 direct and10240 production-ABI original-ROM
cases, comparing CPU, all128KiB WRAM, ordered operand/MMIO data accesses and
child-entry snapshots. Each path adds408 child unwinds and1020 changed returned
contexts.137 semantic plus196 additional consumer guards pass; the ABI log also
repeats137 semantic guards.765 production redirect probes cover23 distinct
sites and51 root/site occurrences. No supported-entry fallback. Sixteen in-memory
ROM faults are detected. Unchanged positive binaries/sources are sealed and
reused for the subsequent edge/fault checkpoint, rather than rerun identically.
First four positive profiles per parent compose canonical new children and the
existing decompressor against actual original child instructions. Other child
bodies use explicit paired contracts; their internals are not claimed here.

Strict MSVC, GCC/Clang semantic builds, actual resource CLI targets/tests,
metadata and isolated Windows Release pass.1484 verified contracts, zero drafts,
1473 bindings,749 WRAM locations,458 portable C units (457 sources plus CLI).
Generation emits56 actual wrappers for the14 new entries. Only one new memory
span is added:1E50..1EC1 records. Existing7F field/actor storage reused for scene
window/palette work is documented as aliases, without overlapping new entries.

Three production scenes complete without failure, exercising eight new entry
addresses. Ordinary7444-frame CPU/WRAM/PPU/pixel/PCM fingerprints match exactly.
Selector0A takes448 frames on both paths and matches all688 recorded frames,
including menu closure. Selector0B takes379 frames on both paths; PPU/pictures
match all619 frames, while CPU/WRAM and rawPCM hashes differ in618/230 frames.
Selector14 takes771 native versus772 reference frames. Both execute760 script
steps. The reference has one additional black graphics-preparation picture;
after aligning that sample, all remaining PPU/register/memory/pixel states and
pictures match. No animation step is removed. Totals are9762/9763 frames.

Native integration is not cycle-exact. A focused read-only snapshot pair keeps
its8134-frame prefixes identical to the completed runs. CPU/full WRAM match at
0A completion and the following field frame, but clocks differ by14. The next
field frame reaches80:998B versus80:997D in the unchanged sound-payload sender;
A/Y/P/N and scratch0060 reflect asynchronous upload progress. Generated HLE
wrappers/generic bridges do not account the original instruction stream and
use plain, unpaced bus accessors. Clock/loader differences are therefore an
existing consumer timing limitation, not a claim of full runtime CPU/audio
parity. No invented cycles/waits or semantic fixes were added to conceal them.

One shared final-state checkpoint covers the semantic and consumer commits;
intermediate commits are not independently tested. Full1965 regression is
deferred on this local additive batch; no old CPU/ABI helper body changed.
Fixture:tests/decomp_verify/lufia2_world_scene_resources_verify.c. Private
recipes/results:world-scene-callbacks-next/build_direct.cmd,build_engine_abi.cmd,
positive-receipt.json,checks.json,compilers.json,runtime/comparison.json and
TIMING_REVIEW.md. No ROM/private tools/foreign WIP included. No push.

## World frame scripts and scene actor setup - 2026-10-16

Five complete bank86 contracts are reconstructed and bound: C1D2 runs the
scene frame script, C246 processes timed/default callbacks, C284/C2AD start
five scene actors/the individual actor, and D385 applies the incoming fade
control. D385 shares the existing D381/D37D fade writes. C1EC remains an
internal loop, with no speculative standalone binding. Original child waits,
conditional updates, plane suppression, carry/brightness exits, frame-counter
wrap and exact child continuations are preserved.

C246 reads 16-bit duration operands and masks their low byte. It retains
16-bit cumulative wrap and the timed callback followed by the default callback.
Original hand-pushed C271/C282 frames precede the indirect target reads. The
callbacks land at C272/C283. Unknown callback bodies remain child services;
returned unsupported contexts hand off at the original landing. Long host
slices retain exact C1EC/C24D continuations instead of fast-forwarding waits.
Metadata documents 1224/1226 reuse without overlapping existing memory entries.

The combined seven-root checkpoint includes affected D381/D37D:25600 direct
and12800 production-ABI original-ROM cases, CPU/full128KiB WRAM, ordered operand
data accesses/MMIO and child-entry snapshots. Each path adds344 child unwinds
and860 changed-child-return cases across repeated frames.49 semantic guards
and98 additional consumer guards pass (ABI also repeats the49 semantic cases).
330 production redirect probes cover all22 distinct child sites; zero supported
entry fallback. Twelve in-memory ROM fault controls are caught. Timed records
include zero duration, matches/misses and257/258-record cumulative wrap.
The fixture changes only heap test data at86:F000; the ROM file stays intact.
Canonical C246/C2AD compose with actual original callbacks/animation children;
unknown frame-update internals are explicit paired contracts. Test stops match
the repeated child ordinal and stack, not just a recurring PC.

Strict MSVC, GCC/Clang semantic builds, promoted metadata and isolated Windows
Release pass. Actual GCC/Clang resource CLI targets/tests pass. After the final
scene readability cleanup, only semantic targets are rebuilt; unchanged CLI
and resource-format dependencies reuse their recorded gate. Generation selects
1459 bindings and emits20 actual wrappers for the five new entries. Metadata:
1470 verified, zero drafts,748 WRAM locations,456 portable C units (455 library
sources plus CLI). No unrelated shared CPU/ABI helpers changed.

The production-runtime native/reference comparison denies only these five new
bindings. All8296 recorded CPU-register/flag/resume, WRAM, PPU, pixel andPCM
fingerprints match:7444 ordinary frames,240 menu-closing frames and612 real
world-scene frames. Sixteen additional early-scene CPU/WRAM snapshots also
match. Both runs finish without failure. Native entries rise from596 at the
prior checkpoint to1784:593 C246 calls now invoke593 native AF71 callbacks,
plus the C1D2 and D385 entries. The reference observer counts denied attempts
separately; its1191 attempted entries include595 denied new entries. These are
scenario observations, not whole-game coverage or complete VM/cycle parity.
The script uses the authentic83:B471 field-text JSL and original BA39/BA69
route, with five bytes in unused in-memory cart padding; no ROM file is edited.

One shared final-state checkpoint covers both commits. Full1965 regression
is deferred on this revision; no intermediate-commit or deferred-test pass
claim. Fixture:tests/decomp_verify/lufia2_world_frame_script_verify.c. Private
recipes/results:world-frame-script-next/build_direct.cmd,build_engine_abi.cmd,
checks.json,compilers.json,compilers-semantic-final.json,runtime/comparison.json.
No ROM, private tools or foreign WIP included. No push.

## World scene sequences and motion callbacks - 2026-10-16

Ten complete bank86 entry contracts now have verified native bindings: AEB1,
D330,9934,AF3A,AF88,AF90,AF98,AFA0,AFA8 and AF71. D330 and9934 expose shared
tails of existing D32D/992B; they do not duplicate those algorithms. Six motion
variants share one implementation. A357 and the new callbacks share the exact
position-copy operation. AEB1 retains every 8-bit selector alias, all128 indirect
table reads, original saves/restores, scene fields and child frames. Unknown
callbacks and the C1D2 frame-script owner remain original child services.

The combined checkpoint covers ten new and three affected roots:29696 direct
and14848 production-ABI original-ROM cases, comparing CPU, full128KiB WRAM,
ordered data accesses/MMIO and child-entry states. Each path adds304 unwind
and620 changed-child-return cases.128 semantic and180 additional consumer guard
cases pass; the ABI log also repeats the128 semantic guards.570 production
redirect probes cover16 distinct call sites and38 root/site occurrences, with
zero supported-entry fallback. Eleven in-memory ROM fault controls are caught.
Canonical D330/9934 are composed with their actual original nested children;
CDF5,D37D,D3A5,E0B9,E709,A2C6 also execute original instructions where valid.
Other children use explicit paired contracts, not claims about their internals.

MSVC W4/WX, GCC/Clang semantic builds, actual resource CLI targets/tests,
metadata validation and isolated Windows Release pass. Generation selects1454
bindings and emits four real dispatch witnesses for each new entry. Metadata
contains1465 verified contracts, zero drafts,748 WRAM locations and455 portable
C units (454 library sources plus CLI). Address1230 aliases the existing122E
span rather than creating an overlapping memory entry.

A private production-runtime replay compares all ten new bindings enabled versus
denied. All7444 ordinary frames have identical CPU/WRAM,PPU,pixel andPCM hashes.
An authentic field-text JSL invokes the original BA39/BA69 world-scene path with
selector2 and an in-memory script in unused cart padding; no ROM file is changed.
Both variants complete in612 frames. The native run replaces596 actual entries
(AEB1,AF3A,D330 and593 pose refreshes); AF71 still executes593 original times
under the original C1D2 owner. All852 closing-menu/scenario frames have identical
PPU,pixel andPCM hashes. One scenario CPU/WRAM fingerprint differs; exact cycle
or whole-scene CPU/WRAM equality is not claimed. Against the older recovery
instrumented replay, all7444 pictures/PPU hashes also match, but5908 CPU/WRAM
and5822 rawPCM hashes differ; that older harness is not a timing oracle for this
revision. Seven diagnostic snapshots differ only in inactive stack bytes.

One shared final-state checkpoint covers both commits, not independent tests of
intermediate revisions. Full1965 regression is deferred on this revision: the
13-root ROM/ABI union, compiler/CLI gates, full Release and runtime comparison
cover this local world-scene batch. No unrelated semantic/ABI helpers changed.
Fixture:tests/decomp_verify/lufia2_world_scene_sequence_verify.c. Private build
recipes/results:world-scene-sequence-next/build_direct.cmd,build_engine_abi.cmd,
checks.json,compilers.json,runtime/comparison.json. No ROM, tools or foreign WIP
is included. No push.

## Scene sequence and world-owner callbacks - 2026-10-16

BA39 callbacks BA69,BAC9 and BB94 now carry their complete original tails
through the optional80:9CB8 child services. Scene/map/text reloads reuse existing
children. BB94 preserves original A/X/Y/DB/P saves,operand-based world selection,
child carry,field flags and restoration order. Uncertain86:AEB1/8E:B2EE/world
owner internals remain explicit original services;no invented child semantics.
All seven regular scene callbacks have reconstructed bodies with services.
BA67 self-loop,BAED reset and out-of-table targets retain exact original
continuations. No new metadata roots/bindings or whole-VM closure claim.

Combined new+affected checkpoint:8192 direct and8192 productionABI states,
CPU/full128KiBWRAM/ordered data bus and all child-entry snapshots. Eight fixture
suites cover32 call occurrences and27 distinct sites;the world alternatives
share five sites and differ at their selected world-call site.256 unwinds and512
changed-child M/X/DP/decimal returns per path pass.480 production redirect
probes (27 distinct sites),15 direct/18 ABI guards,512 legacy index/flag states
and13 in-memory ROM fault controls pass,zero supported entry fallback.
Tests execute canonical80:A368 against its actual original instructions;
BF0B also runs actual instructions with explicitly synthetic counter changes.
Other child internals are paired contracts. Saved Y/DB,carry and overflow are
deliberately clobbered by new opaque children to exercise caller restoration.

MSVC W4/WX,full isolated Windows Release,GCC/Clang semantic targets/metadata
pass. CLI/resource dependency hashes remain unchanged and reuse recorded gates.
Recovery proof is reused from text-scene-display-next:its engine,body,shared
child helper and production bridge are byte-identical;its route returns before
the modified scene dispatcher. No shared mutable state is introduced. The same
combined-final-state checkpoint covers both planned commits;intermediate
commits are not independently tested. Full1965 and live7444 replay deferred
current revision,not reported as passing.1455 verified/1444 bindings/747 WRAM
locations/454 portable units (453src plus CLI). No ROM/tools/foreign WIP staged.
Fixture:tests/decomp_verify/lufia2_text_scene_owner_verify.c. Private reproduction
text-scene-owner-next/build_direct.cmd andbuild_engine_abi.cmd; supported ROM
plus --unwind,--returns,--guards;direct --indices and --fault are also available.
Receipt:text-scene-owner-next/ACCEPTANCE_RECEIPT.json. No push.

## Scene display callback tails - 2026-10-16

The optional80:9CB8 services path now completes BA39 callbacks BA97,BAA2 and
BAF1 (both operand branches). Original fixed-graphics uploads,force-blank/NMI
switches,two HDMA stream setups,scroll and speaker updates retain their order.
All eleven child sites preserve pushed frames and exact continuations. Frame
waits remain real original children. Existing verified graphics/upload children
are reused through the consumer; no copied algorithms or new standalone roots.
The shared index read,near frame and return/state test also serve the unchanged
no-service API. Other BA39 callbacks remain explicit original prefixes.
The child-call guard is shared with recovery without changing its contract.

One combined final-state checkpoint:4096 direct and4096 productionABI scene
states compare original CPU/full128KiBWRAM/ordered data bus (including MMIO)
and every child-entry CPU/frame snapshot. Additional88 child unwinds and176
changed-child M/X/DP/decimal returns per path cover all eleven call sites.
165 production redirect probes cover all sites with zero supported fallback.
15 direct and18 ABI guards pass.512 old-API cases cover every byte index and
both field-state flags,including out-of-table targets and8-bit shift aliases.
Six in-memory ROM faults are detected. The wait callback executes actual BF0B;
other child internals use explicit paired contracts,not new internal proofs.

Affected recovery rerun:512 direct/256 ABI complete states,424 unwinds each,
112 changed returns each,16 fade stalls each,15/18 guards and105 redirects pass.
MSVC W4/WX,full isolated Windows Release,GCC/Clang semantic targets/metadata
pass. CLI/resource linked-input hashes remain identical to recorded green
gates;those checks are reused. Full1965 and live7444-frame replay are deferred
on this revision. The preceding recovery replay is historical evidence,not a
new-source timing/frame acceptance claim. No original waits are optimized away.
1455 verified/1444 bindings/747 WRAM names/454 portable units (453src plus CLI).
The complete BA39 owner remains partial;three complete callback slices do not
promote the other scene operations or imply complete VM closure/native hits.
Fixture:tests/decomp_verify/lufia2_text_scene_display_verify.c. Reproduction:
text-scene-display-next/build_direct.cmd andbuild_engine_abi.cmd; supported ROM
argument plus --unwind,--returns,--guards;direct also --indices and --fault.
Private receipt:text-scene-display-next/ACCEPTANCE_RECEIPT.json. No push.

## Recovery text command and child services - 2026-10-16

80:A2E1 now composes its original seven child sites through the optional
Lufia2TextEngineStepWithServices API and existing 80:9CB8 production binding.
No new standalone binding or function promotion. The legacy no-service API
retains its boundary contract. Canonical party recovery and gold spending are
reused. Original C1/81 fade requests, bounded literal acknowledgement polls,
100 loop waits and one final wait preserve CPU, frames and ordered bus access.
Child unwinds retain live runtime state; cross-bank handoffs forward PB.
All nominal non-BRK text targets are now recognized when services are supplied:
199 unique targets /201 of205 table routes. Recognition does not prove full
VM closure; partial handlers and original asynchronous continuations remain.

Combined original-ROM checkpoint:512 direct and256 production-ABI recovery
states;424 child-unwind cases per path,112 changed-child contexts per path,
16 stalled-fade cases per path,15 direct and18 ABI guards. Each complete case
compares CPU/full128KiBWRAM/ordered data bus and106 child-entry snapshots.
105 production redirect probes cover all seven sites;zero supported fallback.
10240 affected gold-text states and128 legacy cross-bank cases pass. Nine
in-memory ROM faults and one missing-PB-forwarding source fault are detected.
The frame-wait child executes actual original instructions in both paths;
audio/background child internals use paired explicit contracts in this fixture.

Real production-object integration:7444 ordinary frames match the accepted
Gold checkpoint in every CPU/WRAM,PPU,PCM and pixel fingerprint;1495 original
text CPU/full-WRAM pairs pass. A synthetic script in unused in-memory ROM
padding, entered through actual83:B471 JSL, then exercises real NMI/APU recovery.
Native and original configurations both finish after357 frames:eligible party
healed,10000->9999 gold,recovery counter incremented,100+1 waits,no failures.
The native entry yields at its first asynchronous child;the original tail
completes. This is not proof that all101 frames now remain native, nor a cycle
or audio parity claim for this injected sequence. The ROM file is unchanged.

MSVC W4/WX,isolated Windows Release,GCC/Clang semantic targets and metadata
checks pass. Linked CLI/resource sources match the previously recorded green
gates and reuse those results. Full1965 regression is deferred on this revision.
One combined final-state checkpoint covers the semantic and consumer commits;
intermediate commits were not tested separately.1455 verified/1444 selected
bindings/747 WRAM locations/453 portable units (452 semantic C plus CLI).
Fixture:tests/decomp_verify/lufia2_text_recovery_verify.c. Private reproduction:
text-recovery-caller-next/build_direct.cmd andbuild_engine_abi.cmd;run the
binaries with the supported ROM and --unwind,--stalls,--returns,--guards;
the ABI fixture also accepts --legacy-banks. No ROM/private tools committed.
Receipt:text-recovery-caller-next/ACCEPTANCE_RECEIPT.json.

## Gold arithmetic and scene reload caller - 2026-10-15

Windows Release passes for this combined source.7444-frame replay has1495
original text CPU/fullWRAM pairs (1472returns/23boundaries),all green. All7444
pixel hashes,PPU hashes and43 images equal prior0dda828. Seven whole-frame
CPU/WRAM hashes differ only in the unchanged8293C8/8293CA wait PC and,four
frames,its NMI-saved PC low byte at1FEC. Exact fingerprint reconstruction
explains all seven;two also have direct prior full-WRAM byte comparisons.
Two current runs reproduce every CPU/PPU/PCM fingerprint.146 raw PCM hashes
differ from prior;audio/cycle timing parity remains open. No new-entry
whole-run hit rate is claimed because observations reset before menus.
Detailed evidence:gold-scene-callers-next/runtime-acceptance.json.

80:BF12 GoldAdd and80:BF43 GoldSpend share the existing text arithmetic.
No duplicate algorithm. Preserve the original two-part cap test and the
subtraction/undo writes when spending fails, plus PHP/PLP and near frames.
New public entries support PB80,M1X0,DP0,binary,stack1F20..1FFC and return
atBF42/BF6E. The legacy text wrapper retains its previous behavior.
80:A350 SceneReloadAndFadeIn sets DB80,writes FF separately to7FD0C2/C3,
and calls canonical83:85DC,83:AFCD,80:A368 via exact far/far/near frames.
All child returns/unwinds remain explicit. No unsafe inline text reload.
The two reload bytes have a neutral name; further meaning is unproven.

Shared original-ROM CPU/full128KiBWRAM/ordered-data-bus checkpoint:
173056 direct and35072 productionABI/fullVM cases. Covers both gold roots,
scene parent and affected text add/subtract routes. Each direct gold entry
covers all65536 amounts plus16384 structured balance/cap/borrow/context
states. Additional192 child unwinds per path cover all three scene sites;
43 guards each comprise42 public contexts and one missing-service check.
45 production redirect/nonlocal-return probes span all three child sites.
Full child-entry CPU snapshots compared. Two scene children are opaque
contract fixtures; third oracle executes original text-reset instructions.
No new proof of opaque child internals or full-game completeness claimed.
Ten in-memory ROM fault controls detected;zero supported entry fallback.
MSVC W4/WX fixtures and GCC/Clang semantic targets/metadata pass. Unchanged
linked CLI/resource inputs reuse the earlier recorded CLI gates. Full1965
regression deferred. One final-source checkpoint spans the planned commits;
intermediate commits are not independently tested.

Catalogue83:858B is a suspect D8 table read into following code bytes,not
an invented native entry. Its actual original dispatch edge stays visible.
Receipt:gold-scene-callers-next/ACCEPTANCE_RECEIPT.json.

## Cave entry, party recovery and scene callbacks - 2026-10-15

81:F5BD PartyRestoreVitals restores both values per eligible party member in
original order and increments the saturating recovery counter. Existing
F5ED/F60B share the extended loop; their original single-field behavior stays
verified. No duplicate restoration algorithms.84:878A AncientCaveEnter
carries blue inventory/equipment, saves the84:8B20 ranges bytewise viaWMDATA
and calls the already verified84:8888 reset through its exact near frame.
Both new public entries have complete child/boundary contracts and bindings.
Internal8862/886D helpers are documented through878A, not new public bindings.
New WRAM names: recovery count0B59,Cave entry count0B6A and neutralE75D bytes.

TextABD0 composes canonical Cave entry/reset/exit and preserves unknown child
frames. Unreachable post-Cave tails were pruned; if a child can later finish,
ABDD/ABE8 remain explicit original continuations. TextBA39 reads the actual
indirect table and preserves all256 byte operands, including8-bit shift
aliases. BB70 color-effect callback completes natively. BA67 self-loop,
BAED reset and unknown fade/frame/map children retain exact original targets
and live frames. Both text entries remain partial; no fullVMclosure claim.

15360 direct original-ROM CPU/full128KiB WRAM/ordered-bus cases:4096 each
new recovery and affectedF5ED/F60B,1024 each Cave entry/ABD0/BA39.4608 production
ABI/fullVM cases:1024 each three party entries,256 each Cave entry/ABD0,
1024 scene dispatch. Actual text-handler visits required; all byte operands,
nonzero/high-bit parameters and both post-effect continuations covered.
Additional64 child-unwind states each path:15424 direct/edge,4672 ABI/edge.
253 guards each:22 new public-entry contexts,28 internal contexts,203 other
actual ROM opcode routes. Zero supported entry fallback; intentional text
child handoffs are retained. Full child-entry CPU snapshots and15 production
redirect probes verify arguments, frames and nonlocal returns. Twelve in-memory
ROM fault controls detected, including both RTL kinds and saturation/masks.

Shared checkpoint composes unchanged four public groups fromdirect-final/
abi-final with final ABD0/BA39 focused runs after fixture expansion. ABD0
pruned-tail checks supersede its earlier runs; related library/compiler
checks rerun incrementally. No claim that every intermediate source or planned
commit was independently checked. The earlier PB handoff defect was fixed;
ambiguous80:C0B7 stops require the independentBA44 near-return marker.

MSVC direct/productionABI fixtures and GCC/Clang warnings-as-errors libraries
and metadata PASS. Consumer manifest selects1441 verified bindings without
drafts. Unchanged CLI/resource-format linked inputs match preceding green
CLI checks. Full Windows GUI/Rust regeneration and new gameplay replay are
DEFERRED to a larger integration checkpoint; latest full Release/replay is
the preceding0dda828 consumer revision. Full1965-job regression also deferred
current source. None is reported passing on this revision.

1452 verified entries /1441 bindings /746 WRAM names /451 portable units
(450 semantic sources+CLI). Text198unique/200of205 nominal routes recognized;
remaining9DD4,A2E1,BF08,BF09,BF0A include four originalBRK targets. Prefix
recognition is not dependency closure or a measured runtime hit rate.
Foreign WIP/ROM intact; no owner build/windows changes. Receipt:
text-cave-callbacks-next/ACCEPTANCE_RECEIPT.json. No push.

## Encounter callers and text rendering - 2026-10-15

81:8351 FieldGenerateEncounter now selects map/Cave encounter records through
the existing81:839F generator. It preserves original P/DB/fullA/X/Y, multiply
MMIO and saved frames. Unsupported contexts and slots>=32 hand off before any
changes; the original81:835E BRK remains intact. No duplicate generator, RNG,
sprite or field-record algorithms. The complete child-service contract is
verified and bound. New textAB51/BC55 compose those canonical children; ACA7
can finish through the native encounter chain. Existing BC98 placement body
is exported internally unchanged, with affected opcode68 verified throughVM.

TextB7FE centers measured windows with original Mode7 writes. Text9EC8 selects
font page3 and shares existing glyph/window completion via internal PRINT
flow. No second renderer. Original carry, lookup misses and row-underflow
behavior remain literal; no original bug was fixed in semantic C.

28,672 direct original-ROM CPU/full128KiB WRAM/ordered-bus states:8,192 selector
cases cover all256 map values and32 slots;4,096 each AB51,BC55,B7FE,9EC8,ACA7.
14,336 productionABI/fullVM cases:8,192 selector and1,024 each six text groups,
including affected68. Zero supported fallback. Actual original handler visits
required; child-entry fullCPU snapshots check restored-state-hidden arguments.
Additional128 child-unwind cases on each path and128 direct empty-window
prefixes. Totals28,928 direct/edge and14,464 ABI/edge.308 guards each fixture:
238 selector guards (224 invalid slots+14 contexts),70 internal contexts.
Production callback machinery includes15 redirect probes. Eight in-memory ROM
fault controls detected, including return opcode and restored child arguments.
Open-window extended-glyph rows and zero-width centering prefixes covered;
empty full-window underflow is retained, not collapsed or silently repaired.

GCC/Clang warnings-as-errors semantic libraries+metadata PASS. Unchanged CLI
linked inputs hash-match the preceding green CLI proof. Isolated Windows
Release PASS; initial registry-denied MSBuild was rerun with appropriate host
access, reusing generated analysis. No owner build/windows changes.
Shared7444-frame cold/intro/field/menu replay:1495 original text CPU/fullWRAM
pairs pass (1472 returns/23 boundaries). All CPU/WRAM,PPU andPCM frame hashes,
all7444 pixel hashes and43 images identical to preceding native replay. New
text routes were not visited. Selector hits are not claimed: its reported
observer resets before menus. Synthetic ROM/ABI cases cover the new paths.
This is not whole-game interpreter/cycle/audio equivalence.
Full1965-job regression deferred on current sources, not reported green.

1450 verified entries /1439 bindings /743 WRAM names /449 portable units
(448 semantic sources+CLI). Text196unique/198of205 nominal routes recognized,
seven unique remain:9DD4,A2E1,ABD0,BA39,BF08,BF09,BF0A. Four are originalBRK
targets. Recognition includes earlier partial prefixes, not fullVMclosure.
One combined checkpoint covers both planned semantic commits and consumer
binding/gitlink; intermediate commits are not independently tested. Receipt:
text-encounter-callers-next/ACCEPTANCE_RECEIPT.json. Foreign WIP and ROM intact.

## Shared actor visibility and sprite callers - 2026-10-15

80:BF6F FieldFindWindowActor reuses the existing lookup body. 8E:BAF6
FieldRevealActor and8E:BB17 FieldHideActor share one state implementation with
EventOpHideActor. Event $60/$61 keep their previous exact continuation; show
still stops at8E:BB12 BEFORE the81:8351 frame. The public reveal function has
the complete child-service contract throughBB16. 81:8351 generates a field
encounter record, NOT graphics; its inner81:839F is already verified.
No cloned lookup, party offsets, allocation or map-occupancy implementations.

TextA8F8 replaces actor sprites through existing canonical children; ACA7
reveals/marks the encounter actor and retains exact81:8351 handoffs. The
original lookup-miss behavior and stale-slot use remain literal.

28,672 direct ROM CPU/full128KiB WRAM/ordered-bus states across3 roots,2 text
routes and2 affected event routes. 7,168 production-ABI/full-VM states, zero
positive fallback; all four VM groups actually visit their original handlers.
Additional128 child-unwind and256 wide/stack-alias slot states on each path:
29,056 direct and7,552 ABI/edge cases total. 70 context guards plus203 other-ROM
route guards on each fixture; ABI fixture covers42 production root guards,
with28 internal guards and203 default-dispatch checks. Callback redirect probes
cover the production service machinery. Eleven in-memory ROM controls detected,
including all three RTL kinds and child actor-argument corruption. The latter
keeps final CPU/WRAM/bus identical but fails the full child-entry CPU snapshot.

MSVC isolated full Windows Release and GCC/Clang semantic libraries+metadata
PASS. Unchanged CLI/resource-format inputs hash-match the prior green tests;
reuse CLI checks. Full1965 regression deferred. Shared7444-frame replay passes
all1495 direct ROM text-body CPU/fullWRAM pairs (1472 returns,23 boundaries).
Text9DFA runs32 times andA8F8 twice in that replay. All43 pictures and allPPU
hashes match previous native replay. CombinedCPU/WRAM raw hashes match7442
frames; at7175/7385 only PC differs between CMP8293C8 andBEQ8293CA in the same
unchanged menu polling loop. Isolating BF6F original restores both hashes.
Focused byte snapshots prove equal WRAM; exact FNV inversion/reconstruction
proves all other recorded CPU fields match. Raw2 differences remain recorded.
PCM differs4399 frames, first2594; no cycle/audio parity claimed. Focus replays
were justified by these concrete differences, not routine repeated regression.

1449 verified entries /1438 bindings /743 WRAM names /448 portable units
(447 semantic sources +CLI). Text192unique/194of205 routes,11 unique remain;
native prefix recognition is not full opcode dependency closure.

## Text script/field caller batch - 2026-10-15

Nine internal routes: 9DFA,9E83,9F10,A041,AC6A,BBEB,BDBB,BDCA,BD99.
Typing-tone/pause and packed script repeats retain byte overflow, bank crossings,
original fetch/previous-byte frames, and flag effects. Choice-table setup enters
the existing native window path. A041 reaches A3C6 with M0: preserve that exact
boundary rather than reuse the M1-only goto reader. Shared helper unchanged.
Field callers reuse canonical collected-choice, header lookup, area transition,
edge tracing and layer publishing. Unknown children retain their original frames.
BD99 keeps the title/menu checkpoint and stops before the original reload tail.
AC6A/BDBB retain explicit unknown child boundaries; no complete UI claim.

36,864 direct ROM CPU/full128KiB WRAM cases; 9,216 production ABI/full-engine
cases, zero fallback. Full ordered data-bus comparison for eight routes;
BDBB compares CPU/full WRAM, all writes and MMIO order. Its unchanged canonical
tracer performs a host read-only WRAM preflight, excluded from read-bus identity.
65,658 pre-access/unknown-handler guards; nine in-memory ROM faults detected.
All result groups pass. Accepted results compose unchanged groups from staged
checkpoint runs; failed fixture setup or changed dependency reruns only affected
groups. Pause/repeat/window fixtures now contain valid terminal text; initial
unbounded measurement and ambiguous repeated-stop states were fixture issues.

MSVC Release semantic library + GCC/Clang semantic library and metadata PASS.
Unchanged CLI/resource-format linked inputs hash-match the prior green tests;
reuse those CLI checks. A final generated pause-counter name adds one otherwise
unused macro; expanded C tokens are identical (naming-identity.json), so no
repeated semantic proofs/compiler builds for that alias. Full GUI/full1965 and
new gameplay replay deferred; prior7444-frame replay remains earlier evidence.

1446 verified entries /1435 bindings /743 WRAM names /446 portable units
(445 semantic sources +CLI). No new public promotion or binding. Text recognition
190 unique /192 of205 routes,13 unique remain; not complete VM dependency closure.

# Auxiliary actor slots and object-sprite far entry - 2026-10-15

Two complete guarded entries now prepare auxiliary actor slots and call the
existing object-sprite builder through its original far wrapper. The text
object-actor command uses those same entries. Slot setup shares the existing
record-offset and fine-position routines. Palette copying, sprite tables and
object appearance remain in the already verified83F6CA implementation.
An experimental duplicate of that existing entry was removed before testing.

The far wrapper consumes the actual near return bytes. If original writes
change that return, it hands back at the resulting original address with the
correct stack. Eight direct and eight production-ABI cases prove both low- and
high-byte changes at two stack positions; a control suppressing that handoff
fails immediately. Bank-zero sprite writes remain bank-zero in the reused
builder, separately from DB-relative slot setup.

The shared checkpoint passes16384 direct ROM states and4096 production-ABI/ROM
states across the two new roots, existing sprite builder and text caller. The
text engine retains384 exact child boundaries, with no fallback.46 direct and
15 production guards preserve rejected states. Five ROM mutation controls cover
sprite values, banks, operand bias and both RTL return kinds. Existing actor,
graphics and text-field source is exact outside the additions. The initial
palette/near-entry candidate was already verified; no duplicate API/body/binding
or new guard is added to that existing routine.

MSVC Windows Release, GCC/Clang standalone, CLI and metadata checks pass once
on the combined source. A fresh production replay checks the combined recent
item/equipment, text-menu and actor batches. All1495 actual whole-text calls
match original CPU/full WRAM, with1439 returns and56 boundaries. All7444 frame
CPU/WRAM and PPU/image hashes match the preceding native replay, and all43
pictures match. The newly supported map-change command is reached once; other
new entries retain their synthetic proof scope.4787 PCM hashes differ beginning
at frame2656, so cycle/audio parity is not claimed. Cold boot reaches all seven
expected scene states. A new full1965-job regression remains deferred.

Totals:1446 verified entries,1435 native bindings,742 WRAM locations and445
portable C units. Text entry recognition is181 unique handlers/183 of205 routes;
22 unique handlers remain. Recognition includes guarded continuations and
native prefixes, not complete VM dependency closure or measured ROM coverage.

# Text menu and scene handoffs - 2026-10-15

Seven additional script entries now use native caller code: map-change request,
field menus, two item-reward pages and scripted battle. Existing fade, menu
selection, field-value counting and encounter routines are reused. Their actual
child frames and target banks remain intact. Menu selection keeps its82803D
consumer checkpoint, including the complete CPU state at that instruction.

One map-change handler completes natively. The other six caller prefixes retain
explicit original continuations at open fade, menu or encounter children. Their
post-menu tails remain original code and are not claimed as verified functions.
An initial reconstruction included unreachable tails; those were removed before
acceptance. In particular the sensitive full8385DC map reload is not inlined.
No public entry, binding or complete-menu implementation is newly promoted.

The shared checkpoint passes28672 direct original-ROM cases,7168 complete
text-engine/production-ABI cases,6144 live child boundaries and65627 untouched
unknown/unsupported guards. CPU/full WRAM, ordered data bus and reached menu
checkpoints match. Seven ROM mutations are detected. A separate omission control
removes only the menu checkpoint: CPU, WRAM and40 bus events still match while
the hook-presence assertion fails, proving that the observer is meaningful.

Existing engine source is exact after removing the added dispatch block;
read-only bridge instrumentation reverses exactly to production. Names and
proven WRAM constants were cleaned up with checked equivalent normalized tokens.
After pruning unproved tails, the affected ROM/ABI cases and semantic compiler
targets were checked again. Unchanged CLI/resource-format inputs reuse their
passing GCC/Clang CLI checks. MSVC, GCC and Clang semantic-library checks pass.
A fresh whole desktop build/full1965 regression is deferred: production bridge,
public ABI, bindings and CFG roots are unchanged. The preceding Windows Release
and runtime replay remain evidence for their respective earlier revisions.

Totals remain1444 verified entries,1433 bindings and742 WRAM locations;444
portable C units. Text entry recognition is180 unique handlers/182 of205 routes,
with23 unique handlers remaining. Recognition includes explicit native prefixes
and continuations; it is not complete opcode dependency closure or runtime load.

# Item, equipment and party restoration callers - 2026-10-15

Eight complete guarded ROM routines now handle party-status clearing, requested
item removal, possession flags, packed inventory subtraction and equipment-count
nibbles. Four text commands share these routines and the existing party restore
and equipment-slot helpers. The previously verified82FAA0 quantity-increase
routine is reused; its experimental duplicate was removed before integration.

Possession bits,96 inventory words and packed equipment-count nibbles remain
separate formats. Original missing-item return asymmetry, wrapping subtraction,
neighbor-nibble pollution on underflow, saturation flags and empty equipment
counting stay literal. Equipment addresses are reread through(dp2A),Y for each
access. Four byte shifts retain four separate read/modify/write bus operations.
The new fixed-byte direct-page shift leaves every previous CPU helper unchanged.

The guarded M0/X16 item routines require binary arithmetic, DP0 and bounded
stacks; quantity subtraction accepts even inventory offsets below192. Equipment
clearing accepts member IDs0..6 after low-byte masking. Party-status clearing
uses the original four record pointers, M1/X16 and unrestricted DP/DB. Rejected
states retain the original entry; nested callers retain their actual frames.

One shared checkpoint passes139264 original-ROM CPU/full-WRAM/ordered-bus cases,
including32768 cases for the reused quantity-increase routine and4096 byte-shift
cases. Production bridges/full text engine pass13312 additional ROM cases with
no fallback.153 direct and62 production-ABI guards pass, as do256 exact nested
continuations and12 injected ROM-fault controls. An initial guard expectation
omitted resume_pc; correcting the fixture required no semantic change. Private
ABI observation initially selected an older helper header; pinning its intended
header restored exact semantic-bus comparisons without changing production code.

GCC and Clang warning-clean standalone builds and resource-CLI tests pass.
Generated metadata lists were refreshed; only that outstanding documentation
check was repeated. The consumer receipt records the isolated Windows Release
checkpoint. Existing helper tokens and read-only bridge-observation identity
are checked. Formatting and comment placement preserve compiler tokens.

Totals:1444 verified entries,1433 native bindings,742 WRAM locations and443
portable C units. Text recognition is173 unique handlers/175 of205 routes,
with30 unique handlers remaining. Recognition includes exact continuations;
it is not full VM dependency closure or measured runtime coverage. A new full
1965-job regression is deferred; the prior7444-frame replay remains evidence
for its earlier combined revision, not a newly run whole-game comparison.

# Scene rendering and tile-text callers - 2026-10-15

Eight additional internal text handlers use existing native scene-pair lookup,
object copying, layer/attribute rendering and text-buffer clearing. Three share
the original rectangle setup. The two tile-text writers retain their ROM-backed
ASCII-indexed tables, byte arithmetic, separate row writes and16-bit cursor wrap.
Original NMI-register addressing, upload requests and frame waits stay literal.
Unknown upload/action children retain their actual targets and live frames.

C093 already exists as Lufia2SceneReadActorPair; the new text adapter reuses it
and the existing BFAA callback. No coordinate lookup, tile copier or renderer is
duplicated. Removing the added helper/dispatch block reproduces the preceding
scene-command/engine sources exactly. A4096-character host quantum hands back
at B137/B1C2 without changing original state or terminating the guest loop.

A shared checkpoint passes32768 direct original-ROM cases and8192 full
text-engine/production-ABI cases, including3584 exact child continuations and
no fallback. Three additional direct and three ABI cases prove exact65536-record
header and4096-character continuations.65640 pre-access guards and eight injected
ROM faults pass. CPU/full WRAM/ordered data-bus events match. An FF-ID fixture
initially read unseeded bytes after its sentinel and produced zero-sized copy
loops; valid trailing record data fixed the fixture. Original FF comparison
precedence remains unchanged. Formatting preserves C tokens.

The shared MSVC semantic-library, GCC/Clang standalone, CLI and metadata checks
pass. A fresh7444-frame production replay covers the combined three text batches:
1495 actual text-body calls match original CPU/full WRAM, with1438 returns and
57 boundaries. CPU/WRAM and PPU/image hashes match the prior native replay on
all7444 frames; all43 pictures match.2475 PCM hashes differ beginning at4970;
no cycle/audio parity is claimed. Actual newly recorded commands are A29C once
and B07823 times. This route does not reach the eight render/tile handlers;
their evidence remains the synthetic ROM/ABI cases above. The probe links the
fresh semantic library with unchanged isolated consumer/core objects, without
rebuilding the entire desktop UI.

Compiler and broader runtime results are recorded in the consumer acceptance
receipt. A new whole desktop Release/full1965 regression is deferred: generated
CFGs, metadata, bindings and production bridges are unchanged. Necessary MSVC
semantic-library and production-ABI checks share this checkpoint. No individual
intermediate-commit or reused whole-game result is claimed as a new full run.
Totals remain1436 verified entries,1425 bindings and742 WRAM locations, with
441 portable C units. Text recognition is169 unique handlers/171 of205 routes;
34 unique handlers remain. Recognition includes explicit continuations and
is not whole-VM completeness or current runtime coverage.

# Text exits, audio callers and party rebuild - 2026-10-15

Eleven more internal text handlers use C: six audio command callers, text exit,
nonlocal field restart, window-state toggle, screen-effect reset and party actor
rebuild. Existing native driver/immediate-sound helpers are reused. APU replies,
song-loader hooks, music fade hooks and unknown scene children keep their exact
original targets and pushed frames. Handler recognition includes these explicit
continuations; it does not claim every audio child or continuation tail is native.
No music root is inlined with a null consumer checkpoint.

The original C11C actor-release body is shared with window closure. Reversing
its name/return-argument change and removing the new dispatch block reproduces
the previous complete engine source exactly. The existing prompt still supplies
9CA5. Restart keeps the original S1FFF reset before JML83ACEF, without popping
old text frames. Screen reset keeps its original long word writes and widths.

The combined checkpoint passes45056 direct original-ROM states and11264
complete text-engine/production-ABI states, including7452 exact continuations
and no fallback. CPU/full WRAM/ordered data bus match.65679 pre-access guard
states and six injected ROM faults pass. The eleven direct groups passed first;
the harness selection was then narrowed to avoid routing unrelated existing
party commands through the unknown-handler check. Only the open guards were
rerun. No production semantic fix was required. Formatting preserves C tokens.

Windows Release, GCC/Clang standalone, CLI and metadata checks pass once on
the combined source. Full1965 and a fresh whole-game replay remain deferred;
no result from the previous source is presented as a run of this revision.
No public entries/bindings are added:1436 verified entries,1425 bindings and
742 WRAM locations, with439 portable C units. Text recognition is161 unique
handlers/163 of205 routes;42 unique handlers remain. These are dispatch counts,
not complete-game coverage or runtime hit rates.

# Party and inventory text commands - 2026-10-15

Twelve additional internal text handlers execute in portable C: party membership,
spell learning, member markers and percentage stat gains, actor visibility,
display offsets and object locations, plus flag/item/gold/object-bit conditions.
The existing lookup, spell-record, multiplication, stat and occupancy routines
are shared. Signed-byte widening and script jumps have one internal implementation.
Unknown party sprite children retain their original targets and live call frames.

The complete 81:F723/F74F menu-party routines are verified and selected through
two production bridges. Their four menu slots remain distinct from the text
callers' five field slots. Empty-before-duplicate precedence, rejection codes,
zero-count underflow, repeated count writes and byte-index truncation are kept.
The semantic entry accepts PB81/M1 and stack1F10..1FFC; direct page, data bank,
index width and decimal state remain observable. The consumer conservatively
selects native M1/X16/binary contexts; unsupported states retain its fallback.

One shared checkpoint passes 73728 direct original-ROM cases, including 16384
additional membership edge cases, and 14336 production-ABI/ROM cases. CPU,
full WRAM and ordered data-bus events match. The ABI cases include 1024 exact
child continuations and no fallback. There are 65706 pre-access guard cases;
six deliberate ROM faults are detected. Complete text callers use sufficient
stack for native handler entry; smaller direct-handler stacks are guarded.
The initial ABI observer stopped before an unchanged computed-jump table read
in unsupported shallow-stack cases; its caller fixtures were separated from
the direct guard proofs. No game-semantic correction was required.

Formatting preserves C tokens, joining adjacent fixture string literals.
Compiler/CLI checks belong to this combined source checkpoint; intermediate
commits are not claimed independently tested. The historical 1965-job full
regression and a fresh whole-game replay are deferred; the preceding replay
is not claimed on this revision. Totals:1436 verified entries,1425 bindings,
742 WRAM locations and438 portable C units. Text recognition reaches150 unique
handlers/152 of205 routes;53 unique handlers remain. These counts do not measure
whole-game completeness or runtime hit rates.

# Speaker, scene-cell and relative-position commands - 2026-10-15

Fourteen additional internal text handlers are reconstructed: seven speaker
presets, operand-selected speaker and speaker refresh, three scene-cell
callers and two relative actor-position commands. The existing speaker
updater is shared with wait-for-button; its143 operation tokens are unchanged.
Speaker4/5 retain their original preset order. Signed offsets preserve the
original byte flags while widening A, then add to16-bit fine coordinates.

Scene cells use the existing80:C05C and80:BFAA C implementations. Successful
and missing lookups preserve their exact original behavior. A65536-record
search limit hands off at80:BFBC with all caller, status and bank frames
still present. No decoder, actor lookup or positioning routine is duplicated.
Native commands require their proven PB80/M1/X16/DP0/binary/stack context;
other contexts retain the original handler. No public entry/binding is added.

The shared checkpoint passes61440 actorROM cases,1024 affected eventBC
cases,12288 sceneROM cases,three exact search-budget continuations and15360
full-engine/productionABI cases. Those include2048 originalC2A1 window
continuations.65731 actor and65575 scene guards leave unsupported states
untouched. Three ROM faults are detected. Windows Release and GCC/Clang
standalone/CLI/metadata pass once on this combined batch. Speaker formatting
was checked by complete C-token identity; no redundant semantic rerun follows.

A7444-frame replay compares1495 actual text-body calls with the ROM:
1421 returns and74 boundaries match CPU/full WRAM. Field callers share this
body; their canonical return9CB7 is compared at the original9DB2 RTL.
Dispatcher9D3B visits use the native handled-opcode count before stopping;
the initial first-visit checker was corrected against an isolated captured
state. Game semantics needed no fix. The corrected observer produces exactly
the same frame hashes as the earlier entry-only observer.

All pictures and PPU hashes match the preceding native replay. Two combined
CPU/WRAM frame hashes and4380PCM hashes differ; no cycle/audio equality is
claimed. Full1965 remains an earlier baseline and is deferred here. Counts
remain1434 verified entries,1423 bindings and742 WRAM locations, now433 C
units. Direct text recognition reaches102 unique handlers/104 of205 routes;
101 unique handlers remain. Recognition is not whole-VM completeness.

# Actor placement and animation controls - 2026-10-15

Four more internal text handlers now run in C:2F/A71B places an actor,
B6/AC8A writes its presentation flags, AF/ACBD sets the secondary timer
and flag0736bit7, and B0/ACE2 clears both animation-control high bits.
The first uses the existing filteredBF6F lookup; the other three use
the existing rawBF92 lookup. Their different missing/excluded behavior
and absolute-versus-long addressing remain literal.

The already reconstructed80:A74D position routine is shared with eventBC.
Its extracted body has409 identical C tokens after substituting the caller's
return address. There is one implementation of fine/cell positioning,
occupancy removal/restoration, script restart and per-actor flag changes.
The new text caller and existing event caller preserve their actual frames.

One affected checkpoint covers53248 directROM actor states across13
handlers,1024 complete eventBC cases,4096 full text-engine/productionABI
cases,65705 unknown/context guards and three detected ROM faults. The
direct groups passed first; only malformed synthetic event banks and an
unterminated list fixture needed correction. Retained passing groups were
not rerun. No semantic implementation change followed those proofs.
Windows Release and GCC/Clang standalone/CLI/metadata pass together.

Full1965 and a new whole-game replay are deferred for a broader integration
checkpoint. The preceding7444-frame replay belongs to the previous revision;
it is not claimed as a run of this new revision. All new and affected
boundaries are checked directly against original CPU/full WRAM/ordered bus.
Counts remain1434 verified entries,1423 bindings,742 locations and432 C
units. Direct text recognition is88 unique handlers across90/205 routes,
with115 unique handlers remaining; this is not whole-VM completeness.

## Dialogue actor commands - 2026-10-15

Nine additional internal handlers cover ten dialogue opcode routes inside
80:9CB8: actor script requests, facing writes, object-record bindings,
conditional actor starts, individual/aggregate motion waits, temporary
script refresh and single/all actor script restarts. The complete actor
lookup and script-pointer helpers are shared with existing reconstructed
code. No duplicate public entry or runtime binding was added.

The native boundary is PB80, M1/X16, DP0, binary arithmetic and
S1F10..1FFC. The preceding state-command module gains the same bank/stack
guards while retaining its proven decimal arithmetic. Other contexts
continue through the original handler. Missing actors deliberately retain
the original stale-slot behavior; ignored carry results are not repaired.
Original call frames, script-byte wrapping, first-match exclusion mask,
script selector restoration and9D00/9DB0 continuations remain observable.

The shared checkpoint covers36864 direct original-ROM actor states,
10240 complete engine/production-ABI cases,65653 actor guards and three
detected ROM faults. Affected state-command coverage passes184320 ROM
states and65705 guards. Seven unchanged actor groups retain their first
passing proof; only the two script reload groups needed a CPU-bank fix
and focused reruns. Full1965 is the older baseline and is deferred here.
Windows Release and GCC/Clang standalone/CLI/metadata gates pass together.

A7444-frame input-driven replay verifies eight actual command calls
against original CPU/full WRAM. PPU state and all sampled pictures are
identical to the preceding native baseline. One combinedCPU/WRAM hash
and3765PCM hashes differ; this is not cycle/audio equality. The diagnostic
interpreter PC is restored after each live check. Counts remain1434
verified entries,1423 bindings and742 WRAM locations, now432 C units.
The text table recognizes84 unique handlers across86/205 routes;
119 unique handlers remain outside direct dispatch. These are route
counts, not whole-game functionality or complete VM verification.

Private evidence: `.agent/decomp/text-actor-command-next/`.

## Event and menu-buffer caller contexts - 2026-10-15

Three existing entries gain observed caller contexts:83:C0EF/83:C0FA accept
X8 and S1FFD;82:F660 accepts DB83 and other original data-bank reads. The
indexed/BRK83:F0BC sibling retains its old width/stack restrictions. ROM bytes
confirm the two leaves contain no index operations and the buffer source is
absolute0100..11D7 followed by long copies. No algorithm or binding is added.

Combined actual-source validation passes12288 ROM/ABI cases,9216 guards and
two faults (wrong latch operation and the obsolete buffer-bank restriction).
The copy fixture initially stopped at its20000-instruction ceiling; only that
entry was retried with a sufficient limit. Leaf native-entry assertions were
checked separately; the passing copy cases were retained. Build recipes and
fixture limits were corrected without changing game semantics.

Production6814-frame replay has1029 native position probes,35 native latch
calls and one native copy; none reaches its own interpreted entry. The copy
replaces23297 interpreter instructions in this trace.31 early-menu pictures
are shifted to existing reference images one/two frames later; all match nearby
reference content. The sampled confirm picture differs by phase. CPU/WRAM,
PPU and PCM frame hashes differ, so timing/audio parity is explicitly unproven.
Current Windows Release and standalone GCC/Clang/CLI/metadata pass. Full1965
remains the older baseline under the owner's affected-batch cadence.
Private evidence: field-event-admission-next, including retained DIFF results.

## Combined frame-wait contracts - 2026-10-15

Five additional contracts at83:900C,8E:B550,86:8B48,86:974F and86:9748 share
one original-ROM verifier: tests/decomp_verify/lufia2_frame_wait_verify.c.
Its16756 cases compare CPU state, full WRAM and ordered data-bus events across
all M/X modes, direct-page/saved-stack aliases, saved-status bit changes,
high-byte counter changes, poll limits, zero-count wraps and child unwinds.
There are16696 returns,48 exact continuations and12 explicit child unwinds.
1276 unsupported-bank/null-child guards precede every bus access. Four injected
faults cover the poll/count budgets, saved status and original-ROM input.

The complete bounded execution contracts are verified; all five remain unbound.
No wait is optimized away or scheduled on the host by semantic C. Original
clock/NMI delivery and existing consumer wait patches are untouched. Archive
symbols and metadata are unique for1434 functions;1423 bindings remain.
Current GCC/Clang standalone/tool/CLI/metadata and Windows Release gates pass.
Full1965 remains an older baseline; it was not repeated for unused additive
APIs whose existing callers and runtime bindings are unchanged.

The manifest generator also preserves unchanged output files rather than
deleting/recreating every CFG. Five generator checks confirm byte-identical
normal/reference output, stable timestamps, stale-only cleanup and isolated
updates. An actual4.669-second incremental Windows build preserves all nine
effective CFGs and54 compiled ROM-bank objects. This measures that build only,
not a general project speedup. Runtime selection and generated bytes are intact.

## Combined field entry-width validation - 2026-10-15

Four existing entries gain real caller modes:83:D3F7 and8E:B63B accept X8,
84:8193 accepts its normalized four entry widths, and83:D350 uses its verified
X8 prefix with an exact83:D38D interpreter continuation. The original F9D4
width/stack quirk remains intact. No new functions or bindings are claimed.

The combined actual-source ABI batch passes458752 original-ROM states and
53376 semantic/runtime guard states. D350 includes4685 exact handoffs and
28083 returns. Existing direct mode proofs are reused for unchanged semantic
bodies; eight prior width faults remain detected and two new primary-action
fault controls detect a missing handoff and an in-memory ROM-prefix change.
Only affected failed guard fixtures were retried; seven passing jobs were
retained unchanged. The failures were link-recipe/expected-count issues.

Standalone GCC13/Clang18 library/tool/CLI/metadata gates and normal Windows
Release pass. Actual production6814-frame input replay matches every prior
three-mode native CPU/full-WRAM, PPU, PCM and pixel hash, with456 native D350,
46 graphics-upload and421 item-region entries and zero own interpreted hits.
D350 now directly includes secondary installation, so its separate entry has
zero dispatch hits. The existing47-frame fade offset versus original admission
is retained; native dispatch is not claimed cycle/audio equivalent to ROM.

Full1965 is the prior baseline, not a regression run on this revision. Under
the owner's updated cadence, these localized entry changes share affected
checks and do not trigger another complete regression. The next broad full
run must replace legacy unsupported-mode expectations for D350/8193 and the
three action/region guard fixtures with their new positive/negative coverage.
Private evidence: field-width-public-next/checks.json and runtime-comparison.json.

## Standalone verification evidence correction - 2026-10-15

The local field-admission and formatting standalone repeat receipts were
invalid: CRLF shell scripts continued after failed commands and their final
success text masked the failures. The earlier CI-repair compiler runs were
valid. The semantic full1965 and Windows proofs are unaffected.

Both standalone gates have now been rerun on the current formatted source
with GCC13 and Clang18. Configure, strict library/tool build, resource CLI
CTest and metadata validation each return a checked zero exit status. The
replacement invokes WSL commands directly, without generated shell scripts.
The original receipts remain historical evidence, superseded for these
standalone claims by the corrected run. No remote CI result is claimed.

Compiler build directories are reused for incremental checks. The owner's
updated cadence is defined in AGENTS.md: related changes share one affected-test
checkpoint and may produce several atomic commits. Full ROM regression is
reserved for substantial decompilation batches, broad shared changes or evidence
of wider regression, rather than each small slice or commit. A private timing
wrapper records each original verifier job without changing its arguments,
timeout, result or test selection. Reports identify the combined tested revision
and distinguish checks run, reused and deferred.

## Field helper formatting - 2026-10-15

Field animation declarations and guard wrapping match the formatter. Exact
C tokens and preprocessing directives are unchanged from full1965. GCC13
and Clang18 standalone429-unit/tool builds plus nine CLI methods pass on
the formatted source. The semantic regression was retained rather than
repeated for whitespace. No bindings, CPU behavior or timing changed.

## Field caller admission and shared animation

Real field callers enter83:8682 and83:AEB5 atS1FFD. Both are two-byte JSR
frames with return bytes at1FFE/1FFF; the previous1FFC ceiling unnecessarily
selected the interpreter. These two existing bindings now admit that frame.
All other width/bank/DP/decimal/stack guards remain. JSL ceilings stay unchanged.

The old animation continuation and complete eight-slot function now share one
worker. The continuation stops before the child JSR, retaining the original
PHX/stack and CPU state. Unsupported legacy M0 entry hands off before access;
its verified ROM contract isM1. No function, binding or WRAM count increases.

140288 direct and140288 ABI slot states include both X widths,256 banks,
all256 slot byte values, all158 supported stack values, both child sites and
legacy boundaries.132096 ABI calls select native with15 redirects.8192 colour
states per direct/ABI suite execute5460 original child returns,45 redirects.
11904 unsupported guards and six detected faults/three baselines pass.
22 actual CPU/WRAM entry captures each replay directly and through the ABI.

Current production passes3570 intro/field frames with zero own interpreted
entries for both routines. CPU/fullWRAM, PPU/picture andPCM hashes match the
private ROM-verified native candidate. All pictures match the old admission
baseline. CPU/stack andPCM timing differ from that baseline; native dispatch
is not cycle exact, and original/native timing or audio equality is not claimed.
Original wait/startup timing owners remain unbound.

Normal Windows Release, strict429-unit C11, GCC13/Clang18 standalone library
and tool builds plus nine ROM-free CLI methods pass. Full1965 passes. Four
stale admission fixtures are replaced by five actual current-source jobs;
1588 unchanged-contract legacy object graphs relink the current library.
Repaired fade,18 battle,6title,15default,7object,7geometry and7title-motion
fixtures rebuild.194 historical fixture sources remain unavailable; this is
not a complete clean-source historical rebuild.1429 metadata names/addresses,
1423 bindings and both animation API archive definitions are unique.

## Standalone CLI/CI repair - 2026-10-15

Both Linux compiler jobs failed because battle_frame_setup.c omitted stddef.h
for NULL; field_update.c had the same masked omission. The combined Linux gate
also caught the new TitleObjectStep NULL call. All three files now own
its standard header. No original-ROM semantics or runtime bindings changed.

The combined current library and resource CLI build with GCC13.3 and Clang18.1
under Wall/Wextra/Wpedantic/Werror, without forced verifier includes. Both
CTest runs pass nine ROM-free CLI methods covering valid decoding, overlap,
malformed input, declared bounds, exit status and unsupported ROM rejection.
CI runs CTest with no-tests=error. Consumer builds gain no test/Python dependency.

The preceding full1964 semantic regression remains green; it was not repeated
for owning-header/build/test changes. Counts remain1429 verified/1423 selected
bindings/742 WRAM locations/429 semantic C units. Remote CI was not triggered.
Future semantic checkpoints also require these independent standalone gates.

## Native title projection and particle animation

86:83F2 projects a title object and selects its depth; 86:856F advances
its 24 particle slots. Both public entries reuse the existing workers.
The old internal callers retain their unlimited depth search. The new
projection boundary returns at 8502 or hands off exactly at 84F4 after 4096
comparisons. The live CPU/stack/bus state is retained. Animation restores
its live saved status;its exit widths can change through stack aliases.
No private budget accessor or copied ROM tables enter the public API.

Seven actual-source jobs cover 156,672 original-ROM states and 156,672 native
adapter states, all 256 data banks, all 65,536 object/pool offsets and the full
supported 1F20..1FFD stack range. CPU/full WRAM/MMIO and ordered reads/writes
agree. Two additional 4096-comparison probes use synthetic table responses
with original ROM instructions;they are not real-gameplay evidence.
The private proof adds 8192 smaller-budget cases and 905 exact continuations.
There are 27,648 unique adapter guards and 131,588 C++ guards. Nine C, two bridge
and two memory-only ROM faults are detected with six positive baselines.
Changed return targets are checked through known and unknown destinations.

All 1964 regression jobs, strict C11 across 429 units and normal Release pass.
Four actual generated-table original/native pairs agree at literal caller
waits. Boot/reset/intro/real audio passes 4680 frames plus 90 replay frames.
Projection is observed natively with zero own interpreted instructions.
Animation may have zero boot hits because its native parent inlines it.
Inlining also reduces child-binding hit counts;binding counts alone do
not measure whole-game completeness or speedup.

The regression relinks 1592 hashed historical object graphs and rebuilds
the repaired fade, 18 battle, 6 prior title, 15 default-record, 7 title-object and
7 geometry fixtures. 194 historical fixture-source paths remain unavailable;
no complete clean-source historical rebuild is claimed. 1429 verified
function addresses/names and 1423 bindings are unique; each function has
exactly one archive definition. Six intentional timing/startup owners
remain verified without runtime bindings.

## Native title geometry entry boundaries

Four original entry contracts now reuse existing semantic implementations:
86:8503 applies a product sign, 86:85C2 spawns a title particle, and
80:8450/8486 calculate sine/cosine through the original ROM table. The sign
operation and trig save/calculate/restore phases are shared once. Existing
pushed-call APIs retain their original frames; no ROM data is embedded.
The particle-pool pointer at 15BE is catalogued. The 85EB fallthrough is
not counted as another function.

Sign correction enters/exits M0X0. Particle spawning enters M1X0 and
restores live saved status. Trig supports all four width combinations;
M1X0 is its canonical metadata entry and exits are dynamic. The original
ROM accesses for angles 180..255 remain; no input clamping is introduced.
Binary arithmetic/DP0 and original data-bank addressing are preserved.

Seven actual-source jobs cover 705,536 original-ROM states and 705,536
actual adapter states, plus 32,768 original JSL/RTL caller compositions.
CPU/full WRAM/MMIO and ordered bus accesses agree. All 24 free particle
positions, exhaustion/inactive cases, saved-status aliases and eleven
altered return targets are covered. There are 24,576 consumer guards and
263,170 C++ guards. Seven C, four bridge and six memory-only ROM faults
are detected, with four positive replacement/proxy baselines.
All 1,427 metadata addresses and function names are unique; each has
exactly one definition in the current archive. All 1,421 bindings are unique.

All 1,957 regression jobs, strict C11 across 429 units and normal Release
pass. Eight real generated-table original/native pairs agree at literal
JSR/JSL caller waits, including mixed widths. Boot/reset/intro/real audio
passes over 4,680 frames and ninety snapshot replay frames. Sign/trig
entries are observed natively, with zero own interpreted instructions.
Spawning can have zero normal boot hits because the native title-object
parent already contains it internally. Binding count alone proves no
additional speedup; no whole-game or original/native timing claim is made.

The regression relinks 1,592 hashed historical object graphs with the
current library and rebuilds the repaired fade, eighteen battle, six prior
title, fifteen default-record and seven title-object fixtures. There remain
194 absent historical fixture-source paths. No complete clean-source
historical rebuild is claimed. The six intentional timing/startup owners
remain verified without runtime bindings.

## Native title object activation

Three additional original entries are verified and selected natively:
86:8513 selects the depth-dependent sprite, 86:8530 sets object speed,
and 86:866A activates an object through its three original child calls.
Existing sprite/speed implementations are reused. The sprite-record
pointer is catalogued in WRAM. No ROM tables are embedded or duplicated.

All three enter with M1X0, DP0 and binary arithmetic. Saved live status
can change the speed helper's and activation parent's exit widths;
both exits are dynamic. Return-frame and status aliases, all data banks,
child unwinds and the original projection child remain explicit.

Verification covers 214,020 original-ROM states and 214,020 actual
adapter states, CPU/full WRAM/MMIO and ordered own-span bus accesses.
These include 256 original-projection/native-child compositions and all
four final child-return width combinations. Sixteen dedicated return/status
aliases, sixty parent redirects, 13,824 leaf guards and 197,382 C++ guards
pass. Four C, three bridge, three memory-only ROM and two reverted-core
faults are detected. All seven new jobs and the full 1,950-job matrix pass.
Strict C11 covers all 429 units and the normal Release build passes.

Six production original/native pairs agree at the literal caller wait.
Each new entry executes eighteen times over 4,680 boot/reset/intro/audio
frames with zero own interpreted instructions; ninety snapshot replays
agree. Original projection remains an original child; it is not promoted
by these tests. No original/native timing or whole-game coverage is claimed.

The regression links 1,592 hashed historical reference/adapter object
graphs with the current library. The repaired fade fixture, eighteen battle,
six prior title and fifteen default-record fixtures are rebuilt from
accepted sources. There remain 194 absent historical fixture-source paths;
no complete clean-source historical rebuild is claimed.

## Word shift bus order

The shared absolute-word LSR and ROR helpers now write the high byte before
the low byte, matching original 65816 read-modify-write bus order. Reads,
final values and CPU flags are unchanged. This corrects semantic fidelity
in title signed products and system multiplication; no game behavior is
intentionally patched and no native entry or binding is added by this fix.

All 786,432 original-ROM comparisons cover every word value, both carry
inputs and every data bank, including exact ordered reads and writes.
Reverting either helper is detected. The combined title checkpoint also
passes all 1,950 regression jobs, strict C11 across 429 units, normal
Release, six production pairs and 4,680 boot/reset/intro/audio frames
with ninety snapshot replays. Historical fixture-source limits remain;
this is not a claim of complete source rebuild or game coverage.

## Default-record bank mirrors

The existing native entries 81:EC35/EC56/ED35 now accept the original low-WRAM
mirror banks 00..3F/80..BF as well as their previously supported DB7E. Their
portable C and child-call ownership remain unchanged. The consumer shares
the existing system-bank predicate; the remaining 127 banks keep the fallback.

The boot caller actually enters with DB83. Over 4,680 boot/reset/new-session
frames, interpreted instructions in this family fall from 18,696 to 9. Those
nine are the original ED30/ED34/EC40 continuations, three per initialization.
No scheduler, clock or child-fallback policy is changed to suppress them.

Verification: 29,670 original-ROM states plus 29,670 actual adapter states across
all 129 admitted banks; 42,615 redirect probes; 69,120 unsupported-state guards.
CPU/full WRAM, MMIO and own-span bus ordering agree. Forty-two production
original/native pairs agree at the literal caller wait with actual generated
dispatch and children. Real audio and ninety snapshot replay frames pass.
Two bridge-policy and three memory-only ROM faults are detected. Normal
Release and the full 1,943-job regression pass. No functions or bindings added.

The semantic library is unchanged from the accepted title checkpoint.
Six historical guard fixtures are rebuilt for the newly admitted DB00;
the historical fixture-source availability limitation remains. No clean
rebuild of all historical fixtures, whole-game coverage or timing equality
is claimed.

## Native title record and tile helpers

Three original title entries now have portable C and native selection:
`$86:8637` finds the first active sprite record, `$86:8655` copies nineteen
ROM-backed bytes, and `$86:8740` copies word rows with the original stride.
The existing particle-stride helper is shared; the earlier title C remains
unchanged. The seventy-five thirteen-byte records and the bank-relative
current-object pointer are catalogued in WRAM. No ROM data is embedded.

The first two entries require M1X0, DP0 and binary arithmetic. The row
copier is reached with M0X0, supports all four width combinations, and
restores status/data bank from the live stack. Its exit width is dynamic.
Aliases between source, destination, counters and return frames retain
their original effects. Zero dimensions retain the original loops. The
4,096-word host budget hands off exactly before the next original load.

Verification covers 279,616 original-ROM states and 279,616 actual adapter
states, CPU/full WRAM/ordered bus, 35,328 consumer guards and 394,750 C++
guard calls. Twenty-five altered return targets and 157 exact budget
boundaries pass. Seven C, four bridge and eight memory-only ROM faults
are detected. All six new jobs, all 1,934 shared jobs, strict C11 across
429 units and normal Release pass. Actual generated selections execute
all three entries natively during 4,680 boot/reset/intro/audio frames and
ninety snapshot replays, with zero own interpreted instructions.

The historical regression run retains 1,592 hashed reference/adapter
object graphs relinked against the current library; 194 old fixture
source paths are absent. The repaired fade fixture and eighteen recent
battle fixtures are rebuilt from accepted sources. No clean rebuild of
all historical fixture sources is claimed. The boot sample establishes
neither whole-game coverage nor original/native timing or cycle equality.

## Shared battle session exit

The DB97 `$84:9B3E` adapter uses the existing owned-child helper and
nonlocal return policy. The restored outer stack must survive return;
ordinary dispatch would restore the wrong inner frame. The actual SDK
owner and a deliberately wrong policy distinguish these behaviors.

All 127 other previously admitted data-bank profiles keep the legacy
preview contract. Guards, all thirteen child sites, missing entries,
unwinds, redirects and actual-source fault controls are covered. The
original frame-wait boundary remains interpreter-owned. There is no new
`$81:8855` binding, duplicate body or whole-game performance claim.

The combined batch passes 18 actual-library jobs, all 1,928 shared jobs,
strict C11, C++ guards and normal Release. Two production prefix pairs
match CPU/full WRAM at `$85:EC98`; the native parents execute zero own
interpreted instructions there. Three boot/reset/session runs retain
logos, intro, real audio and ninety snapshot replay hashes. Controlled
child completion tests do not establish complete battle gameplay/timing.

Historical verifier sources are partly absent from this workspace. Their
hashed reference and adapter objects are retained and relinked against the
current semantic library. The eighteen new fixtures compile the current
public semantic and bridge sources. This is an incremental regression run;
no clean-source rebuild of those historical fixtures is claimed.

The first full run found one obsolete palette-fade fixture guard. It
expected M1X1 rejection after an earlier verified extension admitted that
mode. A separate fixture retains both M0 guards and compares 16,384 ROM
states across both supported X widths. Reinstating the old C restriction
fails the new positive matrix. The entire 1,928-job matrix was rerun with
the repaired fixture; the failed first run remains archived. No palette
semantics or runtime selection changed in this batch.

## Battle entry byte-clear correction

The decomp retains the original SEP20 at `$81:8828`. Its absence cleared
two additional bytes in C. The 1,024-state original-ROM full-entry comparison
and a fault control detect the discrepancy. CPU, full WRAM and ordered bus
agree with the correction, including the old entry API. The combined
session-exit integration passes all 1,928 shared jobs and normal Release.
No original-game patch, new entry or runtime binding is introduced.

## Save-flag consumer data banks

`$80:905F` reads save flags with long addressing; its native address resolver
also has no DB-relative access. The adapter now accepts every data bank
while retaining its original M1X0, DP0, native-mode, decimal, PB and stack
guards. The original semantic C and function/binding counts are unchanged.

65,536 ROM states, 65,536 actual-adapter states and 65,536 native-resolver
compositions cover all 256 data banks and all 256 file-index bytes. CPU,
full WRAM, SRAM and ordered bus agree. The adapter's other guards pass
6,144 cases. Restoring the old DB restriction fails the actual-source proof
and reproduces the diagnostic save caller's 29 interpreted instructions.

Three production diagnostic caller pairs match CPU/full WRAM at the same
original wait edge; their own native instruction fallbacks are zero. The
reference save fixture executes one original CMP to align the wait edge;
no PC masking or cycle equality is claimed. Full 1,910 jobs, strict C11,
normal Release and 4,680 boot/reset/intro/audio/replay frames pass. A field
helper formatting cleanup preserves every C token. No full diagnostic
gameplay or whole-game performance claim is made.

## Field menu/window service integration

Native `$83:83A0` retains the three menu children and original nonreturn
stack-reset tail. `$80:C21A` exposes the existing window-timer operation
without copying its body. `$80:9C72` delegates its original children and
reuses the text VM. Both index widths include the actual M1X1 field caller.
Child lookups use live widths; legacy contexts retain their original APIs.
Original word TRB byte ordering and the prompt's comparison read are kept.

The combined batch passes 30 actual-library jobs, the full 1,907-job suite,
strict portable C11, C++ consumers and original-ROM CPU/full-WRAM/ordered
bus fixtures. Three ROM/five actual-C/seven adapter/lookup faults are caught.
Five production caller pairs and boot/reset/intro/audio/replay checks pass.
The menu fixture ends at the original frame wait; full menu gameplay and
cycle equality are not claimed. Only the independent C21A timer adds a
binding. Raw guard evidence does not narrow existing legacy entry profiles.

## Original prompt-counter bus read

The text prompt INC/CMP sequence reads the elapsed byte twice in the ROM.
The decomp retains the comparison read. Original-ROM CPU/full-WRAM/ordered
bus fixtures detect removal of that read. This is a literal reconstruction
correction, with no original-game patch or new binding.

# Semantic Decomp Verification

A function is `verified` only when its complete stated entry and exit contract
has passed comparison against the supported original ROM. The comparison covers
CPU state and WRAM, plus ordered bus effects where the function exposes them.
Partial implementations remain `draft`; unknown child behavior uses an explicit
continuation boundary.

ROM differential harnesses, mutation checks, fixtures, seeds, and temporary
selftests are maintained privately. They are not part of the public source tree
or build configuration. The ROM must never be committed or redistributed.

Public checks include the Decomp metadata validator
(`python scripts/metadata_index.py --check`), generated Consumer binding coverage,
and normal Release builds with Decomp enabled and disabled. A successful build
does not by itself promote a function to `verified`.

`src/decomp_bridge/actor_bridge.c` also contains battle and field bridges.
A later change can split these by subsystem, keeping the runtime ABI covered
by the existing tests.

## Intro startup runtime regression

Cold-boot comparisons isolate the native `$80:8117` selection introduced by
`64004ed`: both logos appear, then the screen remains forced blank while the
guest waits at `$80:9A0F/$80:9A13`. Interpreting only `$80:8117` restores the
intro; switching the other startup bindings alone does not.

The semantic C produces the original CPU, WRAM, VRAM and palette state.
The failure is in runtime bootstrap: the APU's guest-clock mapping is invalid
until the first scheduled port write, and `apu_runToGuestCycle` consequently
does not advance it. Interpreted startup happens to supply relative SPC time;
native initialization exposes the missing mapping. At the same return PC,
the original reaches IPL-ready `$AA/$BB`, while the failing native path still
has zero output ports. Matching the CPU clock counters alone does not fix it.

The runtime now anchors the fresh APU timeline before the first guest
instruction. It writes no ports, skips no SPC instructions and inserts no
delay. Native `$80:8117` selection is restored; its semantic C is unchanged.
Cold and hardware-reset hooks reset the frame adapter, retaining monotonic
CPU clocks on reset. Loaded execution states keep their existing time mapping.

The normal Release builds. Private production-object tests cover three
1,500-frame starts (cold, reset, new session), both logos, visible intro and
real audio. They observe zero interpreted `$80:8117..8210` instructions.
Ninety snapshot-replay frames match CPU, WRAM, PPU, APU and picture hashes.
Removing only the clock initialization reproduces both logos followed by the
missing intro and silent audio. An additional 1,500-frame original-video
control passes with the corrected runtime. The headless reset fixture supplies
the legacy SpcPlayer reset hook; the actual SPC700 and DSP execute normally.
No SRAM files or manual gameplay are involved. These are runtime regression
checks, not a claim that native and interpreted startup have identical cycles.

## Save validation integration

Native `$80:9037` and `$80:90E5` preserve the four-file validation pass,
original checksum comparison, invalid-file clearing and saved CPU status.
17,408 direct and 17,408 production-bridge cases compare CPU, complete
WRAM/SRAM and ordered bus accesses, including all sixteen invalid-file
masks. Original-child and native-child compositions reuse existing save
implementations. Boundary, guard, redirect, C++ and fault-control checks pass.

The joint 1,820-job suite, strict portable C11 and normal Release pass.
Three 1,500-frame production boots (cold, hardware reset, new session)
reach both logos and the intro with real audio; neither save validation
nor video initialization executes original interpreted instructions.
Ninety snapshot replay frames match CPU/WRAM/PPU/APU/picture hashes.
The headless reset fixture retains the legacy SpcPlayer init hook described
above. Synthetic SRAM is confined to the tests; no real save files or
manual gameplay are involved. Cycle-identical execution is not claimed.

## Boot controller integration

Verified `$80:8035/$80:8281` remain unbound because original JML arrivals
retain interpreter ownership. Native `$80:80D3` preserves the exact RAM clear.
Portable contracts, adapter models, child/stack/bus tests and fault controls
pass. The shared 1,836-job regression, strict portable C11 and Release pass.
Three production-object cold/reset/new-session boots show both logos, intro
and real audio with zero original RAM-clear/save-validation/video instructions.
The original startup owner remains active; ninety snapshot replay hashes match.
The legacy SpcPlayer reset fixture remains the same as the earlier boot probe.

## Field publication integration

Native `$80:FBCE/$83:8E4C` preserve attribute masking, first-layer indexing,
zero-count underflow, blanking, layer publication and original frame waits.
CPU/full-WRAM/ordered-bus comparisons, original-child compositions, all-site
handoffs, production ABI, guards and negative controls pass in the shared
1,836-job regression. Live field/NMI timing is not claimed by synthetic waits.

## World sound caller integration

Native `$86:D378` reuses the existing sound-command child while preserving
the distinct JSL child and RTS caller frames. Field/world direct and actual
bridge matrices each pass 10,496 states, with 384 original-child compositions
and all five call-site boundaries. Integrated-source and bridge-frame faults
are detected. Full 1,836-job regression, C++ consumers, strict C11 and normal
Release pass; original APU waits remain explicit test services.

## Diagnostic menu controller integration

Native `$85:8045/$85:8322/$85:8415` preserve all 62 child sites and original
JSR/JSL/RTS/RTL contracts. The message routine accepts the event caller's
16-bit text pointer. Missing native children retain their pushed frames and
transfer to the owning interpreter through a diagnostics-specific adapter;
the original `$85:851C` wait remains unbound. A successful load explicitly
publishes its bank and original stack reset before the field-session tail.

15,360 direct states, 7,168 actual-adapter states, all 379 own instructions,
1,984 missing-entry handoffs, boundaries, redirects, guards, 988 compositions
and 40,960 C++ guard calls pass. Direct matrices compare CPU/full WRAM and
ordered bus; save cases also compare SRAM. Recursive native-child tests
compare ordered MMIO. Eight ROM/eight actual-C/five adapter faults are caught.
All 1,859 shared jobs, strict C11 and Release pass. Three production-object
original/native prefixes match CPU/full WRAM at the original wait; three
boot/reset/session runs retain logos, intro, audio and ninety replay hashes.
Synthetic button/frame/save services do not claim complete menu gameplay or
cycle equality. No real save files are accessed by these fixtures.

In the save-prefix fixture, the existing `$80:905F` guard rejects caller DB85
because its supported consumer profile is DB7E. The new parent executes 15
instructions natively, then preserves the original continuation at `$85:8348`
for the remaining 29. This is an explicit existing-callee fallback; full native
save-menu execution is not claimed.

## Field/event caller service integration

Existing `$80:CBAE` and `$83:81C6` adapters select the complete redraw and
trigger service paths when their original child profile is available.
The lookup uses the child's M1X0 profile. Other existing entry contexts
retain the legacy APIs; both trigger APIs share one semantic core.
The scoped owner helper publishes continuation banks and transfers missing
children with their original pushed frames. Existing VM handoffs remain
unchanged, including the PB85/DB91/X8743 diagnostic-message call.

20,480 direct, 20,480 owner-helper and 20,480 canonical-adapter states,
32,768 legacy regressions, 512 actual VM message handoffs, all sixteen
child sites, 1,024 compositions and 18,432 C++ guards pass. Seven ROM,
five actual-C and five adapter faults are detected. All 1,877 shared jobs,
strict portable C11 and Release pass. Three production caller pairs retain
CPU/full WRAM with zero interpreted native-parent instructions. Boot,
reset, intro, audio and ninety snapshot replay hashes remain green.
Explicit child services and synthetic states are not complete gameplay
or cycle-equality evidence. No new function or binding count is claimed.

## Field reload runtime regression

In-game bisect identified the complete `$83:85DC` native binding as the cause
of intermittent black field scenes after save selection or closing a menu.
The consumer temporarily used `Lufia2DecompBridge_85DC`, which reconstructs the
original setup and hands off at `$83:8637`. After the bridge repair and successful
gameplay confirmation, the complete `Lufia2DecompBridge_8385DC` is selected again.

Those tests compare the parent with explicit contracts for eleven children.
They did not reproduce the actual nested runtime dispatch, NMI or scheduler
sequence, so their green result alone did not establish runtime correctness.

The failing gameplay trace identifies the upload call at `$83:865D` to
`$80:8285`: the camera leaves X8 active, for which the upload has no native
dispatch entry. A nested call interpreter consumes its own deadline and returns
`SKIP_1` with the child still active (`S=$1FF0`, expected `$1FFA`, resume
`$80:880B`). The parent removes one unwind level and reports a normal return,
before reaching the NMI re-enable. Missing native reload children now transfer
their already-pushed guest frame to the owning interpreter. The parent preserves
the transfer's return code rather than interpreting it as a child return.

The reload ABI suite additionally checks 704 missing-entry handoffs against the
original ROM through each child entry, including CPU, guest stack, WRAM and four
transfer return codes. Replaying the recorded upload deadline against the old
bridge fails this suite. Gameplay confirmation covers loading from save selection
and repeated returns from the game menu. The corrected trace contains nine
reloads without false normal returns and fifteen completed fade-ins. Each fade-in
clears `$0581` after eight frames; the final brightness reaches `$0F`.
The full verifier passes all 518 independent jobs with this repair, including
the existing 3,456 reload ABI cases and the additional 704 handoff cases.

## Battle wave table integration

The feature repair selects `$85:ADE1`, `$85:AE68`, `$85:AEEB` and `$85:AA3D`
through standalone replacements. These leaf bridges require native M1X0 in
bank `$85`, accept both decimal modes and preserve the two-byte JSR frame.
The return frame must lie in the bank-zero WRAM mirror, including any extra
byte required for a paired host return. Rejected entries transfer unchanged to
the owning interpreter before any memory access.

Private original-ROM comparisons cover 4,352 ABI cases across three host-return
modes and 60 rejected entries. Mutation checks detect a wrong return-frame
size, lost overflow and an omitted program-bank guard. The three wave builders
additionally pass 196,608 comparisons spanning every 16-bit phase value. The
integrated Release build and all 518 existing verification jobs pass. These
checks do not establish interrupt scheduling or rendered PPU behavior.

The repaired feature remains isolated: 384 standalone replacements are selected
and 77 unverified feature drafts retain static recomp selection. A complete
entry/exit and child-call contract, ROM comparisons and consumer ABI evidence
are required before selecting each further draft.

## Menu palette integration

The five fixed palette loaders `$86:90C0`, `$86:90D3`, `$86:90E6`, `$86:90F9`
and `$86:910C` preserve the original bank `$9F` MVN transfers and RTL boundary.
Their bridges accept either M width, X16 and both decimal modes in bank `$86`.
Entry guards require native mode and a WRAM return frame before reading it.

The ABI oracle checks actual native selection as well as resulting behavior:
8,960 native calls, 1,600 unchanged fallbacks and 60 explicit rejected entries
pass across three host-return modes. Focused cases include palette output
overlapping saved stack bytes. Wrong RTL size, overflow and program-bank guards
are detected by mutations. The earlier motion repair passed all 518 jobs and
the Release build; the combined nine-binding snapshot also passes all 518 jobs and Release.
Its first run timed out in object-update-abi during a concurrent large matrix;
an unchanged rerun without that matrix passes within the existing timeout.


## World perspective plane integration

The complete `$86:A894` parent is selected for its original bank-86 caller
context: M1X0, DP0, PB/DB86 and S in `$1F00..$1FFC`. The semantic entry and
bridge reject other contexts before any memory access. Its four row builders
share the parent implementation; their independent entry contracts remain
draft. The parent matrix covers 262,144 original-ROM comparisons over all byte
angles/tilts, both calculation branches and decimal modes. Consumer checks
assert 16,384 actual native executions and 24 unchanged entry handoffs. Seven
mutations of frame size, overflow and guards are detected. No consumer hook
lies within the plane or its inlined children.

The ten-binding checkpoint, including the shared width-aware arithmetic and
ordered word reads, passes all 518 independent jobs and Release. Selection lists
376 standalone replacements and 85 draft candidate fallbacks. Independent CPU
adapter tests pass 1,048,576 interpreter comparisons; word-order properties pass
458,752 cases with each of MSVC and clang. Interrupt scheduling and rendered
PPU timing are outside these plane-table comparisons.


## Arithmetic, angle and clear-routine integration

Six further leaves are selected: menu multiply `$82:8000`, world division
`$86:A5A9`, battle sine/cosine `$85:DE2A`/`$85:DE1E`, and the world sprite and
slot-flag clears `$86:E650`/`$86:E640`. Each bridge requires native mode, the
routine bank and S at most `$1FFC`; the slot clear also requires M1X0. The
others accept any widths, D and DB, as their ROM code saves and restores P.

Dedicated original-ROM profiles add 16,384 multiply, 32,768 division, 16,384
cases per angle lookup and 4,096 per clear routine. They cover edge operands,
carry and decimal input, every width pair, several data banks, and stacks or
direct pages overlapping the work tables. Every supported case runs natively
and matches; unsupported entries fall back with unchanged CPU state. Five bridge
mutations per routine, plus a width-guard mutation for the slot clear, are
caught. No consumer hook lies inside any of the six routines.

## Menu item integration

The menu item index `$82:88A0` and position `$82:88CB` are selected for the
caller stack band `$1F00..$1FFC`, where the inlined multiply's return frame
cannot alias its work words; the position also requires M1. Other entries keep
the original code. 8,192-case original-ROM profiles per routine add aliasing
item indices, data banks, widths, carry and decimal input. Every supported case
runs natively and matches, fallbacks leave the CPU unchanged, and frame,
overflow, bank, stack, emulation, band and width mutations are caught.


## Copied-member totals integration

`$81:F481` is verified through RTL for native mode, PB81, DP0 and the caller
stack `$1F00..$1FFC`. Other contexts retain the original entry before any
memory access. It accepts all M/X widths, data banks and decimal modes.
The scratch record is a forward byte copy; overlapping records therefore keep
the original write order. The absolute `$C1` reload is independent of DP.
The shared `$81:F4ED` calculation retains its checkpoint at `$81:F576`, before
its RTS. Neither the derived-stat event at `$81:F4E2` nor an extra event is
introduced. Three named copy phases replace instruction-by-instruction loops.

21,568 differential cases cover the general seeds, a 16,384-case matrix
of records, scratch/stack overlap, widths and banks, and 4,096 subscriber cases.
5,581 cases execute the actual native bridge against the ROM; 15,987 unsupported entries are
unchanged handoffs. Every supported comparison checks the entire CPU state,
WRAM and ordered writes, both at the stat checkpoint and at the final RTL.
The subscriber changes scratch totals and the output record receives them.
The real host event dispatcher is used in the consumer ABI test. Fifteen
additional guard tests cover three host-return modes. Eight bridge mutations
(frame, overflow, PB, stack bounds, emulation, DP and missing checkpoint) are
detected. Tests exercise data subscribers that preserve execution state.

Nineteen of the 95 feature additions are verified; 76 remain draft. All 518
integrated jobs and the Release build pass with 385 standalone replacements.
The first run hit three 120-second timeouts; the complete rerun with two jobs
passes the unchanged plan and time limits. The private verifier CMake module
accepts LUFIA2_DECOMP_VERIFY_JOBS (1-16, default 4). This is an isolated repair
checkpoint, not a main rollout.


## Scene tracks, view origin and battle velocities

Four complete return contracts are verified: `$86:94D4` steps the five scene
tracks, `$86:A791` calculates the screen origin, `$85:DD63` calculates signed
angle velocities, and `$81:A598` transfers these velocities to an effect slot.
The first two and the effect routine return through RTS; the angle routine
returns through RTL. The bridges retain the original return-frame sizes.

All four require native mode, the original program bank, DP0 and a caller
stack no higher than `$1FFC`. Tracks accept either accumulator width and X16;
the others require M8/X16. Tracks and view origin additionally require a data
bank that maps the low work-RAM window. The velocity routine requires
`S=$1F00..$1FFC`; its effect caller requires `$1F03..$1FFC`, reserving the
three-byte child frame. Unsupported entries hand off before memory access.
All decimal modes are preserved. Velocity calculation uses the already
verified sine, cosine and multiplication semantics and their original frames.

The matrices exercise all 256 angles, 16 speed edges, timer/script endings,
coordinate edges, banks, widths and decimal modes. Effect slots also overlap
scratch bytes and caller return frames; forward staging and final writes keep
the ROM ordering. CPU, all WRAM, ordered writes and hardware reads are compared
through the actual native bridges. Guarded cases are unchanged entry handoffs,
not original-ROM execution claims.

- scenetracks: 33,856 cases, 33,616 native, 240 unchanged handoffs
- sceneview: 33,856 cases, 12,928 native, 20,928 unchanged handoffs
- velocity: 37,952 cases, 34,064 native, 3,888 unchanged handoffs
- effectvelocity: 33,856 cases, 33,552 native, 304 unchanged handoffs

81 extra guard cases cover three host-return modes. All 35 deliberately broken
bridges are detected: return-frame size, overflow, entry bank, stack bounds,
DP, emulation, width and work-RAM bank checks. Bodies use typed values, named
work fields and arithmetic helpers; CPU-state operations retain observable
flags and call frames. The review found no AI markers in these bodies.

23 of 95 feature routines are verified, with 72 draft. All 518 independent
integrated jobs and the Release build pass with 389 standalone replacements.
Every new binding has a generated dispatch call. Main and the normal build
remain unchanged; this is an isolated local checkpoint.


## World object visibility and its original list caller

`$86:E295` is verified through RTS for native mode, PB86, M0X0, DP0,
S no higher than `$1FFC`, and a data bank mapping low work RAM. The record
start X must be at most `$1FF1`, and list start Y at most `$1FD2`. These limits
keep all record reads and both output fields in RAM. Record, list, camera,
counter and caller-frame aliases preserve their original order and rewritten
RTS destinations. Entries outside this contract retain the original entry.

`$86:E287` is verified through RTS for the original updater's caller:
native PB86/M0X0/DP0, work-RAM DB, X=`$1469`, Y=`$124F`, S=`$1F00..$1FFC`,
and 1..21 objects in DP `$22`. The ROM updater initializes exactly 21 objects;
binary and decimal pointer advances keep these records clear of child frames.
A zero counter wraps on its first DEC; it is not an empty list. Unsupported
counts retain the original code. This contract closes the whole loop, including
its child JSR/RTS frames, rather than using a synthetic iteration cap.

The consumer reads the two counter bytes in explicit low/high order before
materializing a caller frame. Nine count-guard cases check exactly these two
RAM reads, their byte values and the resulting open-bus high byte. All CPU
registers/status and WRAM remain unchanged. The other 57 guard cases require
no memory reads or writes. All 66 cases cover three host-return modes; no
comparison was relaxed. General ROM comparisons still check CPU, all WRAM,
ordered writes and hardware reads. The list matrix includes banks 00/7E/7F/86.

- visibleobject: 66,624 cases, 40,272 native, 26,352 entry handoffs
- visiblelist: 33,856 cases, 32,768 native, 1,088 entry handoffs

All 25 broken bridges are detected: return frame, overflow, PB, stack, DP,
emulation, widths, work-RAM bank, record/list bounds or starts, and count
bounds. The bodies retain readable box calculations and the original forward
loop, with CPU helpers only for observable flags and frames. No AI markers
were found. Parent ProjectObjects and UpdateObjects screens have no hard
mismatches but remain draft, with budgets and explicit child handoffs.

25 of 95 feature routines are verified, with 70 draft. All 518 independent
integrated jobs and the Release build pass with 391 standalone replacements.
Both new bindings have generated dispatch calls. Main and the normal build
remain unchanged; this is an isolated local checkpoint.


## World perspective row builders

The four sign variants at $86:A9B0, AA5B, AB0E and ABC1 are verified
through RTS for their original plane caller: native PB86/M0X0/DP0/DB86,
Y=$0382 or $01C1, and 1..112 rows in DP $26. The consumer accepts stack
pointers up to $1FFC, including the parent's child frame at $1EFC.
Unsupported entries retain original execution before any writes.

Each row preserves reciprocal reads, multiplier accesses, forward output
order, decimal arithmetic, counter RMW order, flags and caller-frame aliases.
The 16-bit read at $4217 includes the joypad byte at $4218; its full value
is retained in X. The source uses shared, readable calculations for the
four quadrants and memory callbacks for observable hardware accesses.

The actual bridges pass 200960 cases: 196608 native ROM comparisons and
4352 entry handoffs. Their matrices cover factors, angles, subtraction steps,
row counts, both bands, decimal mode and overlapping return frames.
132 guard fixtures cover three host-return modes. Count guards check the
exact two low/high RAM reads and resulting open-bus byte; CPU registers,
status and WRAM remain unchanged. Other guards require no accesses.
All 48 deliberately broken bridges are detected. The complete parent plane
matrix still passes all 262144 ROM comparisons after the row contracts.

29 of 95 feature routines are verified, with 66 draft. All 518 independent
integrated jobs and the Release build pass with 395 standalone replacements.
All four bindings have generated dispatch calls. The current row library also
passes 16384 native parent ABI cases and 24 parent guards. Main and the normal
build remain unchanged; this is an isolated local checkpoint.


## Finite product, ripple and packed field attributes

$86:A583 is verified through RTS for M8/X16, native PB86 and S<=1FFC,
with any DP/DB. The body retains both hardware products, scratch aliases,
decimal ADC and the first product in X. $85:A736 is verified through RTS
for M8/X16, native PB85, S<=1FFC and a low-WRAM byte at uint16(DP+$33).
It fills exactly 32 ripple bytes, preserving X, bank restoration, flags,
phase arithmetic and stack aliases, including wrapped stacks.

$80:ED0E is verified through RTL for native PB80, DP0, S=1F00..1FFC,
any entry widths/DB and a nonzero rounded dimension product. It leaves
M8/X16. A readable four-cell group loop replaces the instruction-shaped
body. It retains multiplier accesses, packed-byte and field scratch writes,
pointer carries, attribute merge order, the original decimal rounding and
final registers/flags. Zero count means 65536 groups in the original;
that entry retains original execution before any writes. The count guard
reads layout/width/height RAM only. The bridge checks source flow before
capturing return bytes; supported output cannot reach the 1Fxx caller frame.
Three zero-count guard fixtures check those exact reads, values and open-bus
byte. Other guards require no accesses. CPU registers/status and WRAM stay
unchanged on rejection. The verifier raises only this root's reference budget
to 2097152 instructions to cover valid large maps; no comparison is relaxed.

The three actual bridges pass 107200 cases: 90279 native ROM comparisons,
16921 entry handoffs, 51 extra guard cases and 27 detected bridge/source
mutants. Field matrices cover 4096 dimension/target/width/decimal states,
including 255x255 cells, plus all 256 packed bytes against four old attribute
patterns (1024 cases). An initially ineffective bad-shift control now uses
a populated large map and fails correctly. Native counts are measured by
actual dispatch, not inferred from semantic pass totals.

Low-stack regression found a genuine shared wave defect: at SP0, the PHA
used to select bank85 targets ROM, so PLB may read another bank. The memory
view now follows the bank actually pulled; the wave pattern uses its original
absolute indexed address rather than a fixed long bank85 address. Three
existing wave builders and RippleRow are corrected. The four existing wave
bindings pass 6400 actual-ABI cases (5632 native,768 handoffs), including
2048 low-stack cases, plus 60 guard fixtures. Seven controls restoring either
bad bank assumption fail against the ROM. Normal-stack behavior is preserved.

32 of 95 feature routines are verified, with 63 draft. All 518 independent
integrated jobs and the Release build pass with 398 standalone replacements.
The three new bindings have generated dispatch calls. Main and the normal
build remain unchanged; this is an isolated local checkpoint.


## Battle tile copying and world motion

$81:BCCC is verified through RTL for M8/X16, binary arithmetic, DP0,
S=1F00..1FFC and a DB exposing the hardware multiplier (00..3F or80..BF).
The native bridge also requires native mode and PB81. The low product byte
of the two factors gives the row count; zero means256 rows. The two planes
are copied in their original forward byte order, including overlapping
source and output buffers. No source bytes are cached ahead of writes.

Before execution, six RAM reads inspect position, factors and target base.
Let n be the row count, p the position word and b the target base. The first
target is uint16(b+(((p+(p&F8))&FF)<<6)); the last primary output byte is
first+(n-1)*64+floor((n-1+(p&7))/8)*512+63. The complete native contract
requires first>=2000 and last<=FFFF. This keeps outputs separate from the
work bytes and return frame; the second plane may extend into bank7F RAM.
Unsupported spans retain original execution before writes. Nine guard
fixtures check the exact six reads and final open-bus target high byte;
27 CPU guard fixtures require no accesses. Rejections preserve registers,
status and WRAM. Supported execution captures the return frame after the
body, whose outputs cannot touch that frame.

ROM callers at81:BCA3 and85:EC3A establish the ordinary tile-buffer layout.
The proof also covers zero256-row counts, boundary targets and overlapping
planes with explicitly byte-distinct payloads. An earlier all-zero payload
did not detect a deliberately reordered plane read; the strengthened
payload now detects that control. No comparison was weakened.

$86:A417 and$86:995B are verified through RTS for M8/X16, DP0 and a DB
mapping low work RAM. Their native bridges require native mode and PB86.
StepOffsets accepts S=1E00..1FFC; ScrollAdvance accepts S=1E02..1FFC so its
nested JSR reaches the supported child stack. The original product calls,
quadrant handling, decimal arithmetic, fractional offsets and nested stack
writes/reads remain observable. The minimum stack keeps child frames away
from direct-page scratch and world-map globals.

Actual-library bridge verification passes74944 cases:71602 native ROM
comparisons and3342 original-entry handoffs. The blitter contributes7232
cases; each world routine contributes33856, including all256 angles and
distance, bank, decimal and stack boundaries. All84 additional guard
fixtures and29 deliberately broken source/bridge controls are detected.

35 of95 feature routines are verified, with60 draft. All518 independent
integrated jobs and the Release build pass with401 standalone replacements.
All three new bindings have generated dispatch calls. Main and the normal
build remain unchanged; this is an isolated local checkpoint.


## Six complete field, sprite and circle roots

$80:C195 now uses a readable fixed-range flag loop. It marks slots 5 through
39, preserving the other bits and the accumulator high byte; X and final
comparison flags match the ROM. M8/X16, any DP/DB, RTL. Native PB80 and
S=1F00..1FFC keep the caller frame separate from the output bytes.

$83:F9D0 now expresses cell-pointer arithmetic directly: twice the sum of
column and the hardware row product, then the selected map's table base.
Both original JSR/RTS frames and the saved relative offset remain observable.
M8/X16, any DP/DB, S=1F04..1FFC, RTL; native PB83. Decimal arithmetic,
pointer carry into the next long-address bank, direct-page wrapping and
scratch reads overlapping nested frames are covered. An initial rewrite
omitted N/Z export; the corrected LeaveSum version passes all comparisons.

$86:8E6B now uses a readable 1000-byte clear loop, retaining each individual
bus write. Final X=15C0, Y=0, Z=1, N=0; accumulator, carry and overflow stay
unchanged. M8/X16, any DP/DB, RTL. Native PB86 and S=1F00..1FFC exclude
return-frame overlap with the clear range. The matrices include all 256
banks and four distinct old-byte patterns, including nonzero boundaries.

$85:8F4A is verified through RTS for M8, either index width and any DP/DB.
Native PB85 and S=1F00..1FFC keep the caller frame outside the 88-bit random
register. Every 16-bit direct-page value and each individual random bit,
with zero/all-one patterns, are checked. High-byte-first word writes,
rotation carry, direct-page-derived accumulator high byte and final flags
are retained. The existing readable implementation needed no rewrite.

$85:B26D is verified through RTS for DP0, DB7E, S=1F00..1FFC and any entry
widths/status. It computes the midpoint half-width table and restores the
saved status. $85:B208 is verified through RTS for M8/X16, DP0, any DB and
S=1F08..1FFC; this also supports its nested width child. Both native entries
require PB85. Every radius in binary/decimal modes is covered, with entry
width combinations, high register edges, previous-radius comparisons,
rebuild/no-change paths, saved X/DB and the original table write sequence.
The fixed scratch and stack contract keeps output and saved frames disjoint.

The six actual-library bridges pass 191104 cases: 189946 native ROM
comparisons and 1158 original-entry handoffs. All 108 additional guard
fixtures preserve CPU/WRAM and perform no accesses. Seventy deliberately
broken bridge/source controls are detected, including output byte order,
hidden accumulator bytes, decimal arithmetic, saved X and the circle rebuild
condition. Native dispatch counts are measured separately from semantic passes.

41 of 95 feature routines are verified, with 54 draft. All 518 independent
integrated jobs and the Release build pass with 407 standalone replacements.
All six new bindings have generated dispatch calls. Main and the normal
build remain unchanged; this is an isolated local checkpoint.


## Image rows and shared RAM copy

$86:9009 copies 256 bytes, $86:906A copies 128 bytes and advances both
pointers, and $86:8FF6 copies two rows separated by a $200-byte target
stride. The 128-byte routine previously used binary arithmetic for two
ADC operations even when the entry decimal flag was set. A valid RAM-stub
state (index77, seed1142049) exposes the source-pointer mismatch at WRAM
$08; final CPU flags alone miss it. Both increments now use decimal-aware
arithmetic. Source and target reads are explicitly sequenced, retaining
the original bus order rather than depending on C argument evaluation.

The row contract is X16, DP=0, DB in a first-bank WRAM mirror, prepared
MVN/RTS bytes at $057D/$0580, and S=$1F04..$1FFC. The 256-byte row needs
M8 and target=$2000..$FF00; the 128-byte row accepts either accumulator
width and target=$2000..$FF80. The two-row block accepts either width,
S=$1F08..$1FFC and target=$2000..$FD00. All three retain decimal mode,
saved frames, operand-bank writes, forward overlap behavior and RTS state.
Native bridges additionally require non-emulation mode and PB=$86.
Unsupported entry states hand off before any writes or CPU changes.

$00:057D identifies the RAM MVN/RTS kernel prepared by the original game.
Its complete buffer contract accepts X16, either accumulator width, any
DP/DB, S=$1F00..$1FFC, code in low-RAM banks, destination=$7E/$7F and
Y >= $2000 with Y+A <= $FFFF. A remains a 16-bit byte counter even in M8.
The byte loop retains forward copies and the original bank-operand reads;
it leaves A=$FFFF, advanced X/Y, destination DB and unchanged status flags.
The bounded output keeps live RAM code, scratch and caller frames intact.
Other states keep the original entry. Real field-map loading calls it in
PB=$83 and menu copying calls it in PB=$86.

The consumer binding uses dispatch_addresses for those two execution banks,
with one canonical semantic function and no duplicate alias implementations.
The bridge's fallback and RTS PC inherit PB. Manifest validation preserves
the verified-status gate and rejects conflicting, duplicate, malformed or
undeclared aliases. Eleven portable manifest tests pass; the configured
SNESRecomp emitter separately produces all eight bank/width forwarding shims.

The four actual-library ABI proofs pass 49408 cases: 28240 native ROM
comparisons and 21168 unchanged original-entry handoffs. Profiles cover
decimal flags, register widths, banks, pointer boundaries, overlapping
payloads and counts up to 57344 bytes. All 132 rejection fixtures verify
CPU/WRAM, read sequence, read values and open bus. Twenty-eight deliberately
broken source/bridge controls are caught, including decimal arithmetic,
copy counts, destination bank, strides and return frames.

45 of 95 feature routines are verified, with 50 draft. All 518 independent
integrated jobs and the Release build pass with 411 canonical standalone
replacements. The RAM kernel has actual dispatch in both PB83 and PB86;
all four copy roots have generated calls. Main and the normal build remain
unchanged; this is an isolated local checkpoint.


## Queued uploads, palette components and visible-object ordering

Six complete feature roots are selected: $80:87A7/$87FC, $81:B3F8/$B396,
$82:80A5 and $86:E686. The palette proofs cover every colour word and all
32 component values against all 256 hardware factors, including decimal
fade arithmetic. Tile filling covers every destination offset. Sorting is
stable, descending and unsigned, with the original object references retained;
the native list contract is bounded at 21 entries before any writes.

Queued NMI uploads retain the eight scroll-register writes, the four listed
requests, channel selection, request acknowledgement, tilemap DMA and saved
status. A reverse DMA can replace the listed upload's two-byte return frame.
The old reconstruction discarded those bytes and continued its normal parent.
The original instead transfers to the altered return address with the parent's
saved P still on the stack. The implementation now reads the real frame,
preserves the temporary comparison carry and dispatches that transfer. Original
ROM seed1520000 takes 92 instructions to $80:8638; before repair, C incorrectly
returned at $80:87FB and emitted 27 rather than 24 bus events. No binding of
this feature routine was enabled before this counterexample was repaired.

The actual-library ABI matrices pass 223616 cases:
210208 native comparisons and 13408 unchanged entry fallbacks, with
144 rejection fixtures and 37 broken source/bridge controls caught. A separate
reverse-DMA model writes the child frame at MDMAEN identically for C and ROM:
10240 exact native transfer comparisons cover all four slots, all four request kinds, six channel bits,
five return targets including wraparound, four DB mirrors, four stack bounds,
all entry M/X combinations and both decimal modes. CPU, all WRAM and ordered
bus events remain mandatory; the transfer PC and boundary flow are additionally
required. Five broken return/carry/dispatch controls are caught. The verifier's
default partial label for intentional control transfers is retained; ordinary
routine matrices must still have zero partial results. This models DMA's
external memory effect; it does not claim cycle-accurate PPU/DMA verification.

51 of 95 feature routines are verified, with 44 draft. All 518 independent
integrated jobs and the Release build pass with 417 canonical standalone
replacements. All six roots have actual generated dispatch calls. This is
an isolated local checkpoint; main and the normal build remain unchanged.


## Slide corrections, battle colour tables and world sprites

Nine further roots are selected: menu slide corrections $82:8AD8/$8AE9,
battle colour loaders $85:8AAF/$8AF4/$8B22 and world sprite builders
$86:E555/$E5BB/$E479/$E4E7. The sprite tile and attribute words are now read
in separate statements, so the low/high bus order is fixed by C rather than by
argument evaluation. No other behaviour changed.

The corrections cover every byte error and span pair with both index widths
and safe cursor destinations. The colour loaders cover nonzero payloads, all
data banks, direct pages, status and stacks. The sprite builders cover real
object records, mirrored poses, coordinates, attributes, tiles, decimal mode,
every sprite counter and both high bits. Native contracts: corrections M8,
DP0, hardware DB, Y <= $0800, S $1F00..$1FFC; colours M8/X16, S $1F02..$1FFC,
$8AAF also a hardware DB; sprites M16/X16, DP0, first-bank WRAM DB, counter
at most 126 (pair) or 127 and objects $1000..$1E00. Read-only preflight
checks keep their original read order before any fallback.

The actual-library ABI matrices pass 886336 cases: 877289 native
comparisons and 9047 unchanged entry fallbacks, with 264 rejection
fixtures and 54 broken source/bridge controls caught. The shared production
bridges repeat the same matrices with identical counts, and 20 further
controls against them are caught. CPU, all WRAM and ordered bus events
remain mandatory.

60 of 95 feature routines are verified, with 35 draft. The integrated 518-job
verification and Release build must pass before the staged selection is a
green checkpoint. Main and the normal build remain unchanged.


## Effect stream opcodes and battle tile ids

Four further roots are selected: the effect opcodes $81:A40B (add a stream
word to a slot field) and $81:953F (repeat counter), and the battle tile row
$85:9790 with its grid $85:972E. Their bodies were already exact; no source
change was needed.

The opcodes run from the dispatcher's JSR (abs,X) with M8/X16. Their profile
covers decimal mode, eight direct pages including pages that place the
stream pointer on the scratch word or wrap bank zero, sixteen slot indices
including the scratch word, the caller frame and the bank-7F carry, stream
pointers in WRAM, ROM and the bank-zero mirror, and repeat counts 0, 1, 2
and 255. The tile profile covers counter wrap, row offsets crossing banks
or landing on the caller frame, eight data banks and four stacks. The grid's
stores start at $2816 and cannot reach the bank-zero stack.

The actual-library ABI matrices pass 102656 cases: 101952 native
comparisons and 704 unchanged entry fallbacks. Frame, overflow,
emulation, bank, stack and width bridge mutations and 5 source
mutations are caught (41 controls). The shared production bridges
repeat the matrices with identical counts, and ten further controls against
them are caught. No consumer hook lies inside these routines.

64 of 95 feature routines are verified, with 31 draft. The integrated 518-job
verification and Release build must pass before the staged selection is a
green checkpoint. Main and the normal build remain unchanged.


## Battle actor sprite and world animations

Three further roots are selected: the battle actor sprite $81:8EEA and the
world animation start $86:E0B9 and step $86:E11F. The sprite and start
bodies were already exact. The step inlines the start and the frame-record
lookup with their JSR frames. Its object writes use DP,X addressing; with a
non-zero direct page they can land on that nested frame, and the original
then returns elsewhere. Seed 2299999 (DP $0A86, S $1F00) shows this: the
original runs away while the old C returned normally. The step now requires
DP0 and S $1F00..$1FFC before any access; other entries keep the original.

The sprite profile covers all 64 records, every sprite kind, screen edges on
both axes, list cursors on the stack and wrapping the bank, four direct
pages, data banks and stacks, decimal mode and carry; both the drawn and the
skipped exit occur. The animation profile covers all 22 objects and
out-of-table indices, ROM and RAM tables, every timer class, loop and
follow-up endings, and direct pages and stacks inside and outside the
contract.

The actual-library ABI matrices pass 101568 cases: 80400 native
comparisons and 21168 unchanged entry fallbacks. 29 bridge,
guard and profile-wide source controls are caught, including the nested
frame witness. The shared production bridges repeat the matrices with
identical counts, and nine further controls against them are caught. No
consumer hook lies inside these routines.

67 of 95 feature routines are verified, with 28 draft. The integrated 518-job
verification and Release build must pass before the staged selection is a
green checkpoint. Main and the normal build remain unchanged.


## Battle sprite lists, OAM groups, drift and slot palettes

The six roots $81:8E92, $85:8B4B/$8BC0/$8C27/$894A and $86:911F
have complete native contracts. Unsupported entries resume the original at
the unchanged entry, before any CPU update or memory write. The three OAM
passes require M8/X16, DP0 and S $1F00..$1FFC. Variable output spans must
fit in $7E:2000..$FFFF, protecting DP scratch, return frames and the bank
boundary. Drift requires M8/X16 and binary arithmetic with that stack band.
Slot palettes require X16, DP0 and counts 1..7; the consumer protects the
same stack band. Larger counts can overwrite the RAM move stub.

The actor-list root keeps the actual saved record index and child return
address. Sprite writes can rewrite either. Rewritten child returns are
exported immediately after the original RTS, with its actual CPU, WRAM and
bus effects. All 189 transfer cases match the original landing after RTS;
they are complete abnormal exits, not unimplemented child prefixes. Four
additional saved-index witnesses also match the actual-library ABI. The
longest original run takes 335746 instructions; only those four witnesses
use a 2097152-step cap instead of the default 262144. No comparison is
relaxed. The saved-index mutation is detected by two of those witnesses.

The six actual-library ABI matrices cover 102784 cases: 57421 native
comparisons and 45363 unchanged-entry fallbacks. An additional 141
entry-guard cases cover widths, emulation, program bank, decimal mode,
direct page, both stack bounds, OAM span boundaries and invalid slot counts.
They check CPU and WRAM preservation and exact permitted read-only preflight
accesses. Eleven effective source/bridge controls are caught. Historical
ineffective guard-removal controls and diagnostic crashes are retained in
the receipt but excluded from that count; source controls were repeated with
explicit selection-error diagnostics. Removing one redundant guard alone
correctly leaves the second guard effective. No recomp hook is swallowed.

73 of 95 feature routines are verified, with 22 draft. All 518 independent
verification jobs and the Release build pass with 439 standalone native
replacements. All six roots have actual generated dispatch entries. This is
an isolated local checkpoint; main and the normal build are unchanged.


## Complete world-object parent passes

$86:E430, $E3D2, $E3AB, $E2D2 and $E1B9 now have complete native
contracts. Each accepts its documented fixed-record domain and otherwise
hands off at the unchanged entry without CPU changes or memory writes.
All nested drawing, sorting, projection and slot-user passes finish natively
inside that domain. The per-frame root owns the whole sequence, including
sprite clearing and shared pattern allocation; no wait loop is bypassed.

The five ABI matrices contain 25920 cases (6072 native comparisons and
19848 unchanged-entry fallbacks), with no mismatch, partial completion
or inconclusive original run. The 4096-state families cover coordinate
edges, tilted/plain views, shared and distinct patterns, inactive unknown
kinds, flag combinations, both RAM banks, ROM-bank aliases, stack limits
and invalid counters. CPU, all WRAM and ordered bus effects are compared.
150 explicit guards additionally verify the live host-return states with
zero bus accesses. Twelve source/bridge controls detect altered user counts,
sprite counts, list/projection strides, camera position, wrong root return
frames and damaged last-user guards. Only clean diagnostic exits count.

The $C0-indexed last-user table uses direct-page bank zero, while $1365 is
absolute in DB. A damaged last-user pointer can overwrite a nested return
frame; it is rejected before writes. Update restricts DB to $86/$06 because
its pool tables are ROM absolute reads, while its inactive records need no
kind validation. The header records individual stack and payload contracts.
All executed children stay in bank $86, which has no consumer hook points.

The image parents $86:9022/$8F6F and field parent $80:F821 also preserve
the original accumulator, indexes and flags at child-entry handoffs. Twelve
before/after ROM witnesses detect the old mistakes and match the repairs.
They remain draft: exact partial handoff is not a whole-function proof.

78 of 95 feature additions are verified and 17 remain draft. All 518
independent jobs and the Release build pass with 444 native replacements.
All five roots have actual generated dispatch and wrapper entries. This is
an isolated local checkpoint; main and the normal build are unchanged.


## Complete battle render parents

$85:8A39 (frame setup), $85:8C98 (party sprites) and $85:8D2E (party
tilemap) now have complete native contracts. The shared guard checks the
fixed records' dimensions and output spans. Setup checks all used children
before its first clear, so a rejected entry has no partial effects. Party
roots require M8/X16, binary arithmetic, DP0 and S $1F00..$1FFC; setup
requires S >= $1F10 and a genuine MMIO-bank mirror. Active party dimensions
are 1..16 in each byte; inactive records are ignored. Sprite output fits
$7E:2000..$FFFF; tile output stays $7E:2800..$3FFF, outside all nested frames.
The complete passes keep the original port writes, palette streams, sprite
counting, mirrored strips, tile arithmetic, data-bank saves and CPU flags.

The three ABI matrices cover 53184 cases: 22032 native comparisons and
31152 unchanged-entry fallbacks. They include every 1..16 column/row
combination, six active records, mirrored strips, both render modes, mode
overrides, held grids, unused invalid records, stack boundaries and invalid
widths/DP/decimal/banks/output spans. There is no mismatch, partial completion
or inconclusive original run. Another 81 entry guards check all live
host-return states with zero bus accesses. Eight source/bridge controls
detect wrong mirrors, bottom tile rows, clear extent, root return frames
and weakened dimension bounds. No clean comparison is discarded.

Names now identify the proven block-size word and bound, party enable field,
mode override, tile-grid hold, WRAM port setup and color-byte streams. Shared
preflight belongs to the internal battle header. These paths execute children
only in banks $85/$81's palette and render bodies; none crosses a hook point.
A setup witness takes 5347 original instructions, all replaced by the native
parent and children. This is an instruction count, not a speedup estimate.

81 of 95 feature additions are verified and 14 remain draft. All518
independent jobs and Release pass with447 native replacements. All three
roots have actual generated dispatch and wrapper entries. This is an
isolated local checkpoint; main and the normal build are unchanged.


## Menu tile parents and original frame waits

$82:838F, $82:8069 and $82:80CA implement their complete parent contracts.
The tile work is native; the required child callback executes the original
$82:93C2 frame wait on the already-pushed JSR frame. A returned child restores
the exact parent tail, including the rectangle's REP $20. An unwound child
propagates its live CPU and unwind depth without a stale state restore.
No native loop assumes a completed frame or clears an upload request.

All three require PB $82, X16, DP0, binary arithmetic and S $1F04..$1FFC.
The rectangle additionally requires M16 and each dimension in 1..32.
Missing children and unsupported entries hand off unchanged before writes.
The old partial entry bodies are private drawing helpers; public names and
headers describe the complete callback APIs. The consumer keeps the existing
original wait/yield service, including its generated M1X0 dispatch entry.

19200 full original-ROM comparisons and 19200 preview ABI comparisons cover
all 1..32 rectangle shapes, widths, flags, positions, palette values, counter
wrap, DB aliases and rejected dimensions/stack/DP/decimal states. Another
87 live-state entry guards have zero bus accesses. Each independent ABI run
also checks 15 redirects/unwinds:135 total. Eight deliberate source/bridge
errors are detected cleanly. The symmetric counter increment is a synthetic
environment for differential execution, not a full NMI or gameplay test.

84/95 additions are verified,11 remain draft. All518 independent jobs and
Release pass with450 native replacements. The three roots have actual dispatch
and wrapper entries; their exact production bridges pass against the native
archive. This is a local checkpoint; main and the normal build are unchanged.


## Complete menu screen and image-upload parents

$86:8DD7, $82:8044, $86:9022 and $86:8F6F now implement complete parent
contracts using required original frame-wait children. The screen's wait is
$86:8B48; uploads use $82:93C2. Tile clears, OAM clear, image block moves and
transfer-register/request writes remain native in their original order.
Returned uploads restore the encoded bank and frames before the parent's
RTL. Unwinds preserve the live runtime state. No native loop skips frames,
clears a request or claims completion of hardware work.

Screen/queue entries require M8/X16, DP0, binary arithmetic and S $1F04..$1FFC.
Image parents require X16, DP0, binary arithmetic, S $1F10..$1FFC, a low-WRAM
alias DB or $7E, and the original MVN/RTS stub. The set also requires M8,
1..7 list entries with indices <=85. Missing children or unsupported entries
hand off before writes. Original children remain responsible for timing.

32768 complete-ROM cases and 32768 preview ABI cases include every DB bank,
all flags, counter wrap, 1..7 list lengths, byte indices 0..85, RAM-stub faults,
width/DP/decimal/stack bounds, transfer sizes/addresses and rejected counts.
The ABI matrix has20768 native comparisons and12000 unchanged-entry fallbacks,
120 redirects/unwinds and105 zero-access CPU guards. Nine deliberate errors
are detected cleanly. CPU, all WRAM and ordered writes/MMIO reads are compared.
Counter changes are symmetric fixtures, not a fullNMI timing or gameplay test.

Full-parent/ABI tests found two flaws missed by prefixes: restoring frames
without restoring PB after upload, and treating the grid's RTL as RTS. Both
are repaired and covered by effective source/bridge controls. Public names
now use complete callback signatures; partial bodies are private helpers.

88/95 additions are verified,7 draft. All518 independent jobs and Release
pass with454 native replacements. All four roots have generated dispatch and
wrapper entries. The exact production bridges also pass the native archive.
This is a local checkpoint; main and the normal build remain unchanged.


## Cursor slide and slide count

$82:8AFA counts the slide steps; every sixteenth step it runs the original
$86:8B55 sprite frame (animation, OAM rebuild and the $86:8B48 wait) through
its JSL frame at $82:8B02. $82:89FA moves the cursor slot along the major
axis with the original error corrections and calls $82:8AFA through its real
JSR sites at $82:8A98 and $82:8AD3, so a bound count runs natively inside the
slide while the frame service stays original.

The slide's entry contract is checked before any write: PB82, M8/X16, DP0,
binary arithmetic, S $1F12..$1FFC, a register/low-WRAM DB mirror, slots below
48 and a major span of at most 127. When a frame will occur, every active
sprite's timer must outlive the slide, and each descriptor must lie in ROM or
$7E:2000+ without wrapping its bank, with at most 128 OAM pieces in total. The
sprite service then cannot rewrite the pending frames or the saved registers.

88192 production-preview ABI cases: 77189 native and 11003
unchanged-entry fallbacks. The selection oracle was written separately from
the ROM services and agrees in every case. The slide's count child runs the
native count in these matrices: about 5.6 million composed count calls in the
whole-ROM cases match. Edge cases cover spans 127/128, the timer threshold,
128/129 pieces, descriptors at $1FFF/$2000 and at the bank end, unused bad
descriptors and slots 47/48. 54 zero-access CPU guards pass. Eleven
deliberate errors (wait target/frame, return frames, count site, span, timer,
piece, descriptor, slot and stack bounds) fail with clear diagnostics. Dropping
the Y refresh after the count is not detectable: both children preserve Y.

Two random-WRAM count seeds (143, 356) stay inconclusive: the original
$86:8B55 child exceeds the trace cap on random sprite data on both sides.
The harness previously left a child interpreter current after such an abort,
which corrupted all later cases; it now restores the outer reference.

90 of 95 feature routines are verified, with 5 draft. All 518 independent
verification jobs and Release pass with 456 native replacements. Both roots
have generated dispatch and wrapper entries. This checkpoint is local; main
and the normal build remain unchanged.


## NMI pad service and battle-effect frame yield

$80:8703 uploads requested OAM/palette buffers and polls the automatic pad read
before updating held/pressed buttons and repeat timers. Native calls require
PB80, M8, DP0, S1F00..1FFC and hardware DB; either index width is restored
with caller P. The consumer refuses LLE driving before any bus access because
that context has no read-side beam progression. The two unchanged core
functions confirm 25350 timer/beam states without new auto-read initiation
(at most 66 reads), plus 76050 states with automatic initiation at the VBlank
edge (at most 109 reads); 128 LLE reads
leave timer and beam unchanged. This is a read-side core witness, not a full
NMI scheduling or gameplay test. Repeat operand reads are sequenced in C.

$81:9169 is now named Lufia2BattleEffectYield: it saves the next stream pointer,
reloads the slot timer, PLX-discards the opcode JSR frame and RTS-returns to the
dispatcher exit at $81:8C59. It does not resume at the next opcode. The bridge
follows the generated non-local return routing: compiled ancestor, interpreter
owner, direct paired bounce, or dispatch. Native entries require PB81,
M8/X16, DP0 and S1F00..1FFA; decimal and DB are preserved as in the ROM.

Actual-library comparisons: 8192 complete effect body/ABI cases and 8192 NMI
cases (4663 native, 3529 unchanged entry fallbacks), no mismatch/partial or
budget-limited NMI case. Forty-eight zero-bus CPU/context guards pass; nine
effective preview errors fail cleanly. The generic NMI fixture still has 32
cases with a static busy bit and no modeled progression; this generic screen
is not the proof for this routine. The explicit per-side polling model and
unchanged-core witness establish the polling contract.

An additional 1536 directed effect body/ABI cases cover saved-register,
opcode/dispatcher-frame and stream-pointer aliases plus long-index bank
crossings at both stack edges. All match the original. The same run adds
1536 NMI cases (870 native,666 entry fallbacks), with no mismatch or cap.

92/95 additions are verified, with 3 draft and 458 native replacements.
Actual production bridges, nine canonical controls, all 518 independent
verification jobs and Release pass. Both generated dispatches are present.
The checkpoint is local; main and the normal build remain unchanged.

## Field region edges and grid-menu cursor

$80:F821 now reconstructs the complete cell-edge walk, unpack, fold and
PLB/PLP/RTL tail. It preserves the original actor mask, byte swaps, scratch
words, nested call frames and write order. A read-only replay selects finite
walks inside a checked WRAM window. The original code handles rejected walks;
no native timeout changes their behavior. PB80, DP0, binary arithmetic,
low-WRAM DB and S1F09..1FFC are required; both caller widths are restored.
Selected layers are 0/2/4/6. Walk and fold tables end below7F:C000 so they
cannot overwrite packed input, metadata or the protected caller stack.
Overlapping fold planes, including odd first-plane pointers, retain live reads.

Actual-library/production-bridge comparison covers 21821 synthetic field
cases:20981 native returns,559 finite fallbacks,281 original budget caps on
rejected paths. Another12267 layer/alias cases give11120 native returns,
382 finite fallbacks and765 rejected original caps. None of the accepted
native cases mismatches or exhausts the original budget. All29 directed
contracts pass. Synthetic fixtures contain no copied ROM map data.

$82:8720 now reconstructs cursor movement, signed wrap/limit checks, button
priority, action codes and original sound children. It requires PB82,
M8/X16, DP0, binary arithmetic, low-WRAM DB, S1F02..1FFC and X<=255.
The sound callback is required; unwinds retain the child's live CPU/frame.
65536 actual-library/production-bridge cases give32768 native and32768
unchanged entry fallbacks, without mismatch, partial or budget-limited cases.
An independently formulated selection predicate agrees in every case;
15 child redirect/unwind probes check the production dispatch ABI.

822 distinct CPU/context guards reject with zero reads/writes in the body
and production bridge (emulation is a bridge-only guard). Seven canonical
semantic errors and two production guard errors are caught. CPU, all WRAM,
ordered writes and hardware reads remain mandatory comparison fields.

$82:8B08 remains draft and unbound. Its repaired four-argument API retains
original frame/text children, but38 synthetic original text-service cases
still exhaust the default budget. Longer diagnostics reach an APU handshake;
this is unresolved fixture/service evidence, not a passing comparison.

94/95 additions have passed their individual proofs;1 remains draft.
The intended460 native replacements require the complete518-job integration
and Release build before this becomes a green checkpoint.

## Complete menu polling caller ($82:8B08)

All95 feature caller contracts are complete; current status is in metadata.
The menu polling caller preserves original sound, string and frame children via
its required child callback, including their stack frames and live CPU on unwind.
Its native entry requires PB82, M8/X16, DP0, binary arithmetic, low-WRAM DB,
S$1F04..$1FFC and a cursor-slot-minus-five byte index. Unsupported states keep
the original entry; changed child/return state transfers at its actual boundary.

Original-ROM/production-ABI matrix:65536 cases with64-instruction original-child
cuts,4778 complete returns,23894 live transfers and36864 entry fallbacks;
zero mismatch, partial or inconclusive. Earlier/later cuts cover4096 cases each:
16 instructions (193 returns,3903 transfers) and4096 instructions (4087 returns,
9 transfers). Without cuts,65536 cases give28634 returns and36864 fallbacks;
38 original children exhaust the synthetic trace budget. Each of those38 is
separately compared at a65536-instruction original-child transfer, preserving
CPU, full WRAM and ordered bus events. A transfer is not a completed child.
This proves the complete polling caller/service contract, not termination or
complete reconstruction of the original text/sprite children. Three semantic
error controls and1230 CPU/context guard cases independently exercise the proof.

## Animated cursor movement ($82:89FA)

`tests/decomp_verify/lufia2_menu_cursor_animation_verify.c` accepts the local
US ROM path and optionally a128KiB WRAM capture path. Without a capture it
constructs four animated sprite slots through original `$86:8CDA/8CF5` code.
No preview file, game process or SRAM is required. It checks4352 ordinary
returns, five changed-child context transfers, two bounded-loop transfers
and two child unwinds against `interp816`; CPU/full WRAM, ordered write
fingerprints and exact MMIO order must match. The clock callback advances
on alternate `$0040` reads inside the original wait, not by omitting waits.
The separate production-bridge fixture exercises the same observed caller
contract and fifteen rewritten-return/nonlocal-return dispatch probes.

The three existing menu callers `$82:9246/92F9/9971` retain their widths,
stack limits and literal DB-relative accesses. Their expanded DB admission
has a shared24576-state original-ROM/ABI proof covering all data banks,
including real DB83 calls. Counting broader caller contexts as new functions
or new runtime bindings would be incorrect.

## Text state and palette commands

`tests/decomp_verify/lufia2_text_state_commands_verify.c` exercises13 new
internal handlers reached through80:9CB8. Its184320 original-ROM states
include all65536 byte pairs for subtraction in both binary and decimal
modes, bank-crossing operands, all data banks, palette index wrapping and
Mode7 MMIO. It compares CPU, full WRAM and exact ordered data-bus events.
Another13312 complete text-engine/production-ABI states cover dispatcher
integration and saved-status returns.65627 unsupported/unknown handler
contexts leave CPU and memory untouched. Three ROM mutations are detected.

The command module accepts M1/X16, DP0; relocated scratch can overlap guest
return frames and stays in original execution. A late guard review added
that explicit boundary, so affected positive/ABI/compiler checks were
rerun on the final source instead of reusing the earlier module revision.
Windows Release and GCC/Clang standalone/CLI checks are green. Full1965
remains a prior baseline; unchanged unrelated suites were deferred.

The7444-frame original-input replay has identical pictures/PPU state.
Both new fade handlers also pass original-ROM comparisons at real calls.
PCM hashes differ from frame3580, and the combined CPU/WRAM hash differs
on one frame. The observer-free run reproduces these hashes exactly.
These results do not establish cycle or audio-sample equality.

## Text field-effect callers

`tests/decomp_verify/lufia2_text_field_commands_verify.c` covers thirteen
internal commands through the existing text engine. The shared checkpoint
compares 53248 direct ROM states, 13312 full-engine/production-ABI states,
three direct and three engine search-budget continuations, and 65705 guards.
CPU, all WRAM and ordered data-bus events are checked. Three in-memory ROM
mutations prove sensitivity to spawn type, coordinate stores and action ID.
Windows Release and standalone GCC/Clang/CLI pass on the combined source.
A formatting-only pass preserves every C token. No new runtime replay or
full 1965-job regression is claimed for this checkpoint.

Owner builds use `build/windows`. Agent integration builds must use
`build/windows-decomp-agent` with independent generated sources, objects and
outputs. Never run two builds or configurations in the same build directory.
A missing object during concurrent compilation can be transient; inspect
the active builds before deleting artifacts or changing linker settings.

## Scene-control text commands

`tests/decomp_verify/lufia2_text_scene_control_verify.c` covers23 new internal
routes: object parameters/search/waits, actor facing, animation requests,
camera/layer controls and color interpolation. One combined checkpoint passes
94208 direct ROM states,23552 full-engine/production-ABI states, two direct
and two engine search-budget continuations,65835 guards and six ROM faults.
CPU, full WRAM and ordered data-bus events match. Existing script initialization
and coordinate scaling are shared via internal exports, without body changes.

The first18 direct groups were retained after correcting missing terminators
in synthetic attribute lists. Only the remaining five groups ran afterward.
New semantic-source formatting preserves C tokens; the fixture comparison
joins adjacent C string literals. Windows Release in the independent agent
build and GCC/Clang standalone/CLI checks pass once on the combined state.

The7444-frame replay checks1495 actual text-body calls against ROM:1421
returns and74 boundaries match CPU/full WRAM. It reaches B92C twice and B93D
once. PPU/image hashes and43 pictures equal the prior native replay. Two
CPU/WRAM frame hashes and4787 PCM hashes differ; no cycle/audio equality is
claimed. Full1965-job regression remains deferred. Planned semantic commits
share this final combined checkpoint, without separate intermediate retests.
