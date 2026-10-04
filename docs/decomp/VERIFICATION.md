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
