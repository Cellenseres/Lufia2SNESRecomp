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
