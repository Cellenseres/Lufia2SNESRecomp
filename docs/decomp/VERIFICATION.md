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
