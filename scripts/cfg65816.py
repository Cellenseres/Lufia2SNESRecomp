#!/usr/bin/env python3
"""Mode-tracking static 65816 control-flow walker for contract research.

This module decodes code from a LoROM image and explores every path from an
entry with an explicit entry M/X. It tracks what a contract needs, not what a
function computes:

- M/X per path (REP/SEP, PHP/PLP pairs, PLP of a pushed constant, XCE),
- the stack delta relative to the entry frame and pushed-byte provenance,
- DB and DP (PHK/PLB, PEA/PLB, LDA #/PHA/PLB, MVN/MVP, TCD, PHD/PLD, TDC),
- DB-, DP- and stack-relative data accesses made while DB or DP is still the
  caller's value (the caller invariants a native replacement depends on),
- absolute/long MMIO operands, WRAM long operands,
- direct children with their entry M/X at every call site and exit outcomes,
- every return site and every place analysis cannot continue (dynamic
  targets, RAM code, reads of the caller frame, unknown widths).

It never executes code and never emits ROM bytes other than decoded operand
values. Results are research evidence, not verification: every claim still
needs an original-ROM differential probe.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass, field
from typing import Callable, Optional

# Addressing modes: name -> operand length (negative = width-dependent).
IMP, ACC, IMM_M, IMM_X, IMM8 = "imp", "acc", "imm_m", "imm_x", "imm8"
DP, DPX, DPY, DPIND, DPINDX, DPINDY = "dp", "dp,x", "dp,y", "(dp)", "(dp,x)", "(dp),y"
DPLONG, DPLONGY, SR, SRINDY = "[dp]", "[dp],y", "sr", "(sr),y"
ABS, ABSX, ABSY, ABSIND, ABSINDX, ABSLONGIND = (
    "abs", "abs,x", "abs,y", "(abs)", "(abs,x)", "[abs]")
LONG, LONGX, REL, RELL, MOVE, PEA_M, PEI_M = (
    "long", "long,x", "rel", "rell", "move", "pea", "pei")

OPERAND_LEN = {
    IMP: 0, ACC: 0, IMM8: 1, DP: 1, DPX: 1, DPY: 1, DPIND: 1, DPINDX: 1,
    DPINDY: 1, DPLONG: 1, DPLONGY: 1, SR: 1, SRINDY: 1, ABS: 2, ABSX: 2,
    ABSY: 2, ABSIND: 2, ABSINDX: 2, ABSLONGIND: 2, LONG: 3, LONGX: 3, REL: 1,
    RELL: 2, MOVE: 2, PEA_M: 2, PEI_M: 1,
}

DP_MODES = {DP, DPX, DPY, DPIND, DPINDX, DPINDY, DPLONG, DPLONGY, PEI_M}
DB_MODES = {ABS, ABSX, ABSY, DPIND, DPINDX, DPINDY}
SR_MODES = {SR, SRINDY}


def _build_opcodes() -> list[tuple[str, str]]:
    table: list[Optional[tuple[str, str]]] = [None] * 256
    alu = ["ORA", "AND", "EOR", "ADC", "STA", "LDA", "CMP", "SBC"]
    even = {1: DPINDX, 3: SR, 5: DP, 7: DPLONG, 9: IMM_M, 0xD: ABS, 0xF: LONG}
    odd = {1: DPINDY, 2: DPIND, 3: SRINDY, 5: DPX, 7: DPLONGY, 9: ABSY,
           0xD: ABSX, 0xF: LONGX}
    for i, name in enumerate(alu):
        for col, mode in even.items():
            table[(i * 2) << 4 | col] = (name, mode)
        for col, mode in odd.items():
            table[(i * 2 + 1) << 4 | col] = (name, mode)
    rest = """
    00 BRK imm8   02 COP imm8   04 TSB dp     06 ASL dp     08 PHP imp
    0A ASL acc    0B PHD imp    0C TSB abs    0E ASL abs
    10 BPL rel    14 TRB dp     16 ASL dp,x   18 CLC imp    1A INC acc
    1B TCS imp    1C TRB abs    1E ASL abs,x
    20 JSR abs    22 JSL long   24 BIT dp     26 ROL dp     28 PLP imp
    2A ROL acc    2B PLD imp    2C BIT abs    2E ROL abs
    30 BMI rel    34 BIT dp,x   36 ROL dp,x   38 SEC imp    3A DEC acc
    3B TSC imp    3C BIT abs,x  3E ROL abs,x
    40 RTI imp    42 WDM imm8   44 MVP move   46 LSR dp     48 PHA imp
    4A LSR acc    4B PHK imp    4C JMP abs    4E LSR abs
    50 BVC rel    54 MVN move   56 LSR dp,x   58 CLI imp    5A PHY imp
    5B TCD imp    5C JML long   5E LSR abs,x
    60 RTS imp    62 PER rell   64 STZ dp     66 ROR dp     68 PLA imp
    6A ROR acc    6B RTL imp    6C JMP (abs)  6E ROR abs
    70 BVS rel    74 STZ dp,x   76 ROR dp,x   78 SEI imp    7A PLY imp
    7B TDC imp    7C JMP (abs,x) 7E ROR abs,x
    80 BRA rel    82 BRL rell   84 STY dp     86 STX dp     88 DEY imp
    89 BIT imm_m  8A TXA imp    8B PHB imp    8C STY abs    8E STX abs
    90 BCC rel    94 STY dp,x   96 STX dp,y   98 TYA imp    9A TXS imp
    9B TXY imp    9C STZ abs    9E STZ abs,x
    A0 LDY imm_x  A2 LDX imm_x  A4 LDY dp     A6 LDX dp     A8 TAY imp
    AA TAX imp    AB PLB imp    AC LDY abs    AE LDX abs
    B0 BCS rel    B4 LDY dp,x   B6 LDX dp,y   B8 CLV imp    BA TSX imp
    BB TYX imp    BC LDY abs,x  BE LDX abs,y
    C0 CPY imm_x  C2 REP imm8   C4 CPY dp     C6 DEC dp     C8 INY imp
    CA DEX imp    CB WAI imp    CC CPY abs    CE DEC abs
    D0 BNE rel    D4 PEI pei    D6 DEC dp,x   D8 CLD imp    DA PHX imp
    DB STP imp    DC JML [abs]  DE DEC abs,x
    E0 CPX imm_x  E2 SEP imm8   E4 CPX dp     E6 INC dp     E8 INX imp
    EA NOP imp    EB XBA imp    EC CPX abs    EE INC abs
    F0 BEQ rel    F4 PEA pea    F6 INC dp,x   F8 SED imp    FA PLX imp
    FB XCE imp    FC JSR (abs,x) FE INC abs,x
    """
    tokens = rest.split()
    for i in range(0, len(tokens), 3):
        table[int(tokens[i], 16)] = (tokens[i + 1], tokens[i + 2])
    missing = [i for i, entry in enumerate(table) if entry is None]
    if missing:  # pragma: no cover - table construction invariant
        raise AssertionError(f"undefined opcodes: {missing}")
    return table  # type: ignore[return-value]


OPCODES = _build_opcodes()

A_WRITERS = {"ORA", "AND", "EOR", "ADC", "SBC", "LDA", "TXA", "TYA"}
ACC_RMW = {"ASL", "ROL", "LSR", "ROR", "INC", "DEC"}
STORES = {"STA", "STX", "STY", "STZ"}
CONDITIONAL = {"BPL", "BMI", "BVC", "BVS", "BCC", "BCS", "BNE", "BEQ"}
TERMINAL = {"BRK", "COP", "STP", "WAI", "RTI"}


def is_mmio(offset: int) -> bool:
    return (0x2100 <= offset <= 0x21FF or 0x4016 <= offset <= 0x4017 or
            0x4200 <= offset <= 0x437F)


def is_system_bank(bank: int) -> bool:
    return bank <= 0x3F or 0x80 <= bank <= 0xBF


def is_ram_code(address: int) -> bool:
    bank, offset = address >> 16, address & 0xFFFF
    return bank in (0x7E, 0x7F) or (is_system_bank(bank) and offset < 0x2000)


def fmt(address: int) -> str:
    return f"${address >> 16:02X}:{address & 0xFFFF:04X}"


def fmt_mx(m: Optional[int], x: Optional[int]) -> str:
    def one(v: Optional[int]) -> str:
        return "?" if v is None else str(v)
    return f"M{one(m)}/X{one(x)}"


class LoRom:
    """Read-only LoROM view. Addresses are 24-bit CPU addresses."""

    def __init__(self, data: bytes):
        self.data = data

    def offset(self, address: int) -> Optional[int]:
        bank, offset = (address >> 16) & 0x7F, address & 0xFFFF
        if offset < 0x8000:
            return None
        rom_offset = bank * 0x8000 + (offset - 0x8000)
        return rom_offset if rom_offset < len(self.data) else None

    def byte(self, address: int) -> Optional[int]:
        rom_offset = self.offset(address)
        return None if rom_offset is None else self.data[rom_offset]

    def cpu_address(self, rom_offset: int, fast: bool = True) -> int:
        bank = rom_offset // 0x8000 | (0x80 if fast else 0)
        return bank << 16 | (0x8000 + rom_offset % 0x8000)


@dataclass(frozen=True)
class Insn:
    pc: int
    opcode: int
    name: str
    mode: str
    operand: int
    length: int

    def operand_address(self) -> Optional[int]:
        """Static data address the operand names (bank of DB omitted)."""
        if self.mode in (ABS, ABSX, ABSY, ABSIND, ABSINDX, ABSLONGIND):
            return self.operand
        if self.mode in (LONG, LONGX):
            return self.operand
        return None

    def text(self) -> str:
        m = self.mode
        if m in (IMP, ACC):
            return self.name
        if m == REL:
            return f"{self.name} {fmt(self.branch_target())}"
        if m == RELL:
            return f"{self.name} {fmt(self.branch_target())}"
        if m == MOVE:
            return f"{self.name} ${self.operand & 0xFF:02X},${self.operand >> 8:02X}"
        width = OPERAND_LEN.get(m, 0)
        if m in (IMM_M, IMM_X):
            width = self.length - 1
        digits = {1: 2, 2: 4, 3: 6}.get(width, 2)
        value = f"${self.operand:0{digits}X}"
        pattern = {
            IMM_M: "#{}", IMM_X: "#{}", IMM8: "#{}", DP: "{}", DPX: "{},x",
            DPY: "{},y", DPIND: "({})", DPINDX: "({},x)", DPINDY: "({}),y",
            DPLONG: "[{}]", DPLONGY: "[{}],y", SR: "{},s", SRINDY: "({},s),y",
            ABS: "{}", ABSX: "{},x", ABSY: "{},y", ABSIND: "({})",
            ABSINDX: "({},x)", ABSLONGIND: "[{}]", LONG: "{}", LONGX: "{},x",
            PEA_M: "{}", PEI_M: "({})",
        }[m]
        return f"{self.name} {pattern.format(value)}"

    def branch_target(self) -> int:
        bank = self.pc & 0xFF0000
        if self.mode == REL:
            rel = self.operand - 0x100 if self.operand & 0x80 else self.operand
            return bank | ((self.pc + 2 + rel) & 0xFFFF)
        rel = self.operand - 0x10000 if self.operand & 0x8000 else self.operand
        return bank | ((self.pc + 3 + rel) & 0xFFFF)


class WidthUnknown(Exception):
    pass


def decode(rom: LoRom, pc: int, m: Optional[int], x: Optional[int]) -> Optional[Insn]:
    """Decode one instruction. Returns None outside ROM; raises WidthUnknown."""
    opcode = rom.byte(pc)
    if opcode is None:
        return None
    name, mode = OPCODES[opcode]
    if mode == IMM_M:
        if m is None:
            raise WidthUnknown(f"{name} immediate needs M")
        size = 1 if m else 2
    elif mode == IMM_X:
        if x is None:
            raise WidthUnknown(f"{name} immediate needs X")
        size = 1 if x else 2
    else:
        size = OPERAND_LEN[mode]
    operand = 0
    for i in range(size):
        address = (pc & 0xFF0000) | ((pc + 1 + i) & 0xFFFF)
        value = rom.byte(address)
        if value is None:
            return None
        operand |= value << (8 * i)
    return Insn(pc, opcode, name, mode, operand, 1 + size)


# Abstract values. A register value, a DB value and a DP value are one of:
#   int                  known constant
#   ("entry",)           unchanged from function entry (caller-provided)
#   None                 unknown
ENTRY = ("entry",)


@dataclass(frozen=True)
class State:
    m: Optional[int]
    x: Optional[int]
    sp: Optional[int]            # bytes relative to entry frame, pushes negative
    db: object
    dp: object
    a_lo: object = None
    a_hi: object = None
    stack: tuple = ()            # pushed bytes, last = top; each an abstract byte
    wram: tuple = ()             # ((offset, byte), ...) known stores to watched WRAM

    def push(self, *items: object) -> "State":
        """Push bytes in CPU order (the first item is written first)."""
        sp = None if self.sp is None else self.sp - len(items)
        stack = (self.stack + tuple(items))[-64:]
        return State(self.m, self.x, sp, self.db, self.dp, self.a_lo, self.a_hi, stack,
                     self.wram)

    def pop(self, count: int) -> tuple["State", list, bool]:
        """Pop bytes. Returns (state, popped top-first, crossed_entry_frame)."""
        popped = []
        stack = list(self.stack)
        tracked = 0 if self.sp is None else -self.sp
        crossed = self.sp is not None and self.sp + count > 0
        for _ in range(count):
            if stack and tracked > 0:
                popped.append(stack.pop())
            else:
                popped.append(None)
            tracked -= 1
        sp = None if self.sp is None else self.sp + count
        return (State(self.m, self.x, sp, self.db, self.dp, self.a_lo, self.a_hi,
                      tuple(stack), self.wram), popped, crossed)

    def with_(self, **changes) -> "State":
        values = {
            "m": self.m, "x": self.x, "sp": self.sp, "db": self.db, "dp": self.dp,
            "a_lo": self.a_lo, "a_hi": self.a_hi, "stack": self.stack,
            "wram": self.wram,
        }
        values.update(changes)
        return State(**values)


@dataclass(frozen=True)
class Outcome:
    kind: str              # "RTS", "RTL"
    pc: int
    m: Optional[int]
    x: Optional[int]
    sp: Optional[int]
    db: object
    dp: object


@dataclass
class CallSite:
    site: int
    target: Optional[int]
    kind: str              # JSR/JSL/JMP/JML/JSR(abs,x)/...
    m: Optional[int]
    x: Optional[int]
    db: object
    dp: object
    note: str = ""


@dataclass
class Summary:
    address: int
    m: Optional[int]
    x: Optional[int]
    outcomes: set = field(default_factory=set)
    stops: list = field(default_factory=list)            # (pc, reason)
    calls: list = field(default_factory=list)            # CallSite
    tail_jumps: list = field(default_factory=list)       # CallSite
    db_uses: dict = field(default_factory=dict)          # pc -> text, DB still caller's
    dp_uses: dict = field(default_factory=dict)          # pc -> text, DP still caller's
    sr_uses: dict = field(default_factory=dict)          # pc -> text
    frame_reads: dict = field(default_factory=dict)      # pc -> text
    mmio: dict = field(default_factory=dict)             # pc -> text
    wram_long: dict = field(default_factory=dict)        # pc -> text
    mode_changes: dict = field(default_factory=dict)     # pc -> text
    instructions: set = field(default_factory=set)
    modes: dict = field(default_factory=dict)            # pc -> {(m, x)}
    states: int = 0
    truncated: bool = False
    recursive: bool = False

    @property
    def frame_access(self) -> bool:
        return bool(self.frame_reads)


def _byte_value(item: object) -> Optional[int]:
    return item if isinstance(item, int) else None


class Analyzer:
    """Explores functions from (address, entry M, entry X) with memoisation."""

    def __init__(self, rom: LoRom, *, state_limit: int = 20000,
                 dynamic_bindings: Optional[dict[int, int]] = None,
                 assume_dynamic_preserves: bool = False,
                 stop_at: Optional[set[int]] = None,
                 ram_models: Optional[dict[int, Callable]] = None,
                 watched_wram: Optional[set[int]] = None):
        self.rom = rom
        self.state_limit = state_limit
        self.dynamic_bindings = dict(dynamic_bindings or {})
        self.assume_dynamic_preserves = assume_dynamic_preserves
        self.stop_at = set(stop_at or ())
        self.ram_models = dict(ram_models or {})
        self.watched_wram = set(watched_wram or ())
        self.summaries: dict[tuple, Summary] = {}
        self._active: set[tuple] = set()

    def analyze(self, address: int, m: Optional[int], x: Optional[int]) -> Summary:
        key = (address, m, x)
        if key in self.summaries:
            return self.summaries[key]
        if key in self._active:
            summary = Summary(address, m, x, recursive=True)
            summary.stops.append((address, "recursive entry"))
            return summary
        self._active.add(key)
        try:
            summary = self._walk(address, m, x)
        finally:
            self._active.discard(key)
        self.summaries[key] = summary
        return summary

    # ------------------------------------------------------------------ walk
    def _walk(self, entry: int, m: Optional[int], x: Optional[int]) -> Summary:
        summary = Summary(entry, m, x)
        start = State(m, x, 0, ENTRY, ENTRY)
        work: list[tuple[int, State]] = [(entry, start)]
        seen: set[tuple[int, State]] = set()
        per_pc: dict[int, int] = {}
        while work:
            pc, state = work.pop()
            if (pc, state) in seen:
                continue
            if len(seen) >= self.state_limit:
                summary.truncated = True
                summary.stops.append((pc, "state limit"))
                break
            per_pc[pc] = per_pc.get(pc, 0) + 1
            if per_pc[pc] > 96:
                summary.truncated = True
                summary.stops.append((pc, "too many distinct states at this PC"))
                continue
            seen.add((pc, state))
            if pc in self.stop_at and pc != entry:
                summary.stops.append((pc, "requested continuation boundary"))
                continue
            for succ in self._step(summary, pc, state):
                work.append(succ)
        summary.states = len(seen)
        return summary

    def _step(self, summary: Summary, pc: int, st: State) -> list[tuple[int, State]]:
        try:
            insn = decode(self.rom, pc, st.m, st.x)
        except WidthUnknown as error:
            summary.stops.append((pc, f"unknown width: {error}"))
            return []
        if insn is None:
            summary.stops.append((pc, "left ROM"))
            return []
        summary.instructions.add(pc)
        summary.modes.setdefault(pc, set()).add((st.m, st.x))
        name, mode = insn.name, insn.mode
        nxt = (pc & 0xFF0000) | ((pc + insn.length) & 0xFFFF)
        self._record_access(summary, insn, st)

        if name in TERMINAL:
            summary.stops.append((pc, f"terminal {name}"))
            return []
        if name == "XCE":
            summary.stops.append((pc, "XCE (emulation switch)"))
            return []

        # Mode changes.
        if name in ("REP", "SEP"):
            m, x = st.m, st.x
            setting = 1 if name == "SEP" else 0
            if insn.operand & 0x20:
                m = setting
            if insn.operand & 0x10:
                x = setting
            if (m, x) != (st.m, st.x) or insn.operand & 0x30:
                summary.mode_changes[pc] = insn.text()
            return [(nxt, st.with_(m=m, x=x))]
        if name == "PHP":
            return [(nxt, st.push(("P", st.m, st.x)))]
        if name == "PLP":
            st2, popped, crossed = st.pop(1)
            self._note_frame(summary, insn, crossed)
            item = popped[0]
            if isinstance(item, tuple) and item[0] == "P":
                m, x = item[1], item[2]
            elif isinstance(item, int):
                m, x = (item >> 5) & 1, (item >> 4) & 1
            else:
                m = x = None
            summary.mode_changes[pc] = f"PLP -> {fmt_mx(m, x)}"
            return [(nxt, st2.with_(m=m, x=x))]

        # Register-to-stack and stack-to-register transfers.
        if name == "PHA":
            if st.m is None:
                summary.stops.append((pc, "PHA with unknown M"))
                return []
            return [(nxt, st.push(st.a_lo) if st.m else st.push(st.a_hi, st.a_lo))]
        if name == "PLA":
            if st.m is None:
                summary.stops.append((pc, "PLA with unknown M"))
                return []
            st2, popped, crossed = st.pop(1 if st.m else 2)
            self._note_frame(summary, insn, crossed)
            if st.m:
                return [(nxt, st2.with_(a_lo=popped[0]))]
            return [(nxt, st2.with_(a_lo=popped[0], a_hi=popped[1]))]
        if name in ("PHX", "PHY", "PLX", "PLY"):
            if st.x is None:
                summary.stops.append((pc, f"{name} with unknown X"))
                return []
            width = 1 if st.x else 2
            if name.startswith("PH"):
                return [(nxt, st.push(*([None] * width)))]
            st2, _, crossed = st.pop(width)
            self._note_frame(summary, insn, crossed)
            return [(nxt, st2)]
        if name == "PHB":
            return [(nxt, st.push(("DB", st.db)))]
        if name == "PHK":
            return [(nxt, st.push(pc >> 16))]
        if name == "PHD":
            return [(nxt, st.push(("DH", st.dp), ("DL", st.dp)))]
        if name == "PEA":
            return [(nxt, st.push(insn.operand >> 8, insn.operand & 0xFF))]
        if name == "PEI":
            return [(nxt, st.push(None, None))]
        if name == "PER":
            value = insn.branch_target() & 0xFFFF
            return [(nxt, st.push(value >> 8, value & 0xFF))]
        if name == "PLB":
            st2, popped, crossed = st.pop(1)
            self._note_frame(summary, insn, crossed)
            item = popped[0]
            if isinstance(item, tuple) and item[0] == "DB":
                db = item[1]
            elif isinstance(item, int):
                db = item
            else:
                db = None
            return [(nxt, st2.with_(db=db))]
        if name == "PLD":
            st2, popped, crossed = st.pop(2)
            self._note_frame(summary, insn, crossed)
            lo, hi = popped
            if isinstance(lo, int) and isinstance(hi, int):
                dp = hi << 8 | lo
            elif (isinstance(lo, tuple) and isinstance(hi, tuple) and lo[0] == "DL"
                  and hi[0] == "DH" and lo[1] == hi[1]):
                dp = lo[1]
            else:
                dp = None
            return [(nxt, st2.with_(dp=dp))]
        if name == "TCD":
            lo, hi = st.a_lo, st.a_hi
            if isinstance(lo, int) and isinstance(hi, int):
                dp = hi << 8 | lo
            elif (isinstance(lo, tuple) and isinstance(hi, tuple) and lo[0] == "DL"
                  and hi[0] == "DH" and lo[1] == hi[1]):
                dp = lo[1]
            else:
                dp = None
            return [(nxt, st.with_(dp=dp))]
        if name == "TDC":
            return [(nxt, st.with_(a_lo=("DL", st.dp), a_hi=("DH", st.dp)))]
        if name in ("TCS", "TXS"):
            summary.stops.append((pc, f"{name} replaces the stack pointer"))
            return []
        if name == "TSC":
            return [(nxt, st.with_(a_lo=None, a_hi=None))]
        if name == "XBA":
            return [(nxt, st.with_(a_lo=st.a_hi, a_hi=st.a_lo))]
        if name in ("MVN", "MVP"):
            return [(nxt, st.with_(db=insn.operand & 0xFF, a_lo=0xFF, a_hi=0xFF))]

        # Accumulator values.
        if name == "LDA" and mode == IMM_M:
            if st.m:
                return [(nxt, st.with_(a_lo=insn.operand & 0xFF))]
            return [(nxt, st.with_(a_lo=insn.operand & 0xFF, a_hi=insn.operand >> 8))]
        if name in A_WRITERS or (name in ACC_RMW and mode == ACC):
            if st.m:
                return [(nxt, st.with_(a_lo=None))]
            return [(nxt, st.with_(a_lo=None, a_hi=None))]

        # Control flow.
        if name in CONDITIONAL:
            return [(insn.branch_target(), st), (nxt, st)]
        if name in ("BRA", "BRL"):
            return [(insn.branch_target(), st)]
        if name == "JMP" and mode == ABS:
            return [((pc & 0xFF0000) | insn.operand, st)]
        if name == "JML" and mode == LONG:
            target = insn.operand
            if is_ram_code(target):
                return self._dynamic(summary, insn, st, nxt, target, "JML to RAM code",
                                     is_call=False)
            summary.tail_jumps.append(CallSite(pc, target, "JML", st.m, st.x, st.db, st.dp))
            return [(target, st)]
        if name in ("JMP", "JML"):
            return self._dynamic(summary, insn, st, nxt, None, insn.text(), is_call=False)
        if name == "JSR" and mode == ABSINDX:
            return self._dynamic(summary, insn, st, nxt, None, insn.text(), is_call=True)
        if name in ("JSR", "JSL"):
            target = ((pc & 0xFF0000) | insn.operand) if name == "JSR" else insn.operand
            if is_ram_code(target):
                return self._dynamic(summary, insn, st, nxt, target,
                                     f"{name} to RAM code {fmt(target)}", is_call=True)
            return self._call(summary, insn, st, nxt, target, name)
        if name in ("RTS", "RTL"):
            summary.outcomes.add(Outcome(name, pc, st.m, st.x, st.sp, st.db, st.dp))
            return []
        if name in STORES and self.watched_wram:
            return [(nxt, self._watch_store(insn, st))]
        return [(nxt, st)]

    def _watch_store(self, insn: Insn, st: State) -> State:
        """Track 8-bit constant stores into watched low-WRAM bytes."""
        if insn.mode == ABS:
            # The caller's DB counts as a WRAM-mirroring bank here; the
            # summary's db_uses lists the store, so the assumption is visible.
            bank = st.db if isinstance(st.db, int) else 0 if st.db == ENTRY else None
            if bank is None or not is_system_bank(bank) or insn.operand >= 0x2000:
                offset = None
            else:
                offset = insn.operand
        elif insn.mode == LONG and (insn.operand >> 16 == 0x7E or (
                is_system_bank(insn.operand >> 16) and insn.operand & 0xFFFF < 0x2000)):
            offset = insn.operand & 0xFFFF
        else:
            offset = None
        known = dict(st.wram)
        if offset is None:
            # An indexed store, or one through an unresolved DB, may reach a
            # watched byte: forget everything rather than keep a stale value.
            if insn.mode in (ABSX, ABSY, LONGX, DPINDY, DPLONGY) or (
                    insn.mode == ABS and insn.operand in self.watched_wram):
                return st.with_(wram=())
            return st
        width = 1 if (insn.name == "STA" or insn.name == "STZ") and st.m else \
            1 if insn.name in ("STX", "STY") and st.x else 2
        if insn.name == "STZ":
            values = [0, 0]
        elif insn.name == "STA":
            values = [st.a_lo, st.a_hi]
        else:
            values = [None, None]
        for i in range(width):
            if offset + i in self.watched_wram:
                value = values[i]
                if isinstance(value, int):
                    known[offset + i] = value
                else:
                    known.pop(offset + i, None)
        return st.with_(wram=tuple(sorted(known.items())))

    # ----------------------------------------------------------- transfers
    def _call(self, summary: Summary, insn: Insn, st: State, nxt: int,
              target: int, kind: str) -> list[tuple[int, State]]:
        site = CallSite(insn.pc, target, kind, st.m, st.x, st.db, st.dp)
        summary.calls.append(site)
        child = self.analyze(target, st.m, st.x)
        expected = "RTS" if kind == "JSR" else "RTL"
        if child.frame_access:
            site.note = "child reads its caller frame (inline data or frame surgery)"
            summary.stops.append((insn.pc, f"{kind} {fmt(target)}: {site.note}"))
            return []
        results = []
        if not child.outcomes:
            site.note = "child has no statically reachable return"
            summary.stops.append((insn.pc, f"{kind} {fmt(target)}: {site.note}"))
        for outcome in sorted(child.outcomes, key=_outcome_key):
            if outcome.kind != expected:
                summary.stops.append(
                    (insn.pc, f"{kind} {fmt(target)} returns with {outcome.kind} at "
                              f"{fmt(outcome.pc)}"))
                continue
            if outcome.sp is None or outcome.sp != 0:
                summary.stops.append(
                    (insn.pc, f"{kind} {fmt(target)} returns with stack delta "
                              f"{outcome.sp} at {fmt(outcome.pc)}"))
                continue
            db = st.db if outcome.db == ENTRY else outcome.db
            dp = st.dp if outcome.dp == ENTRY else outcome.dp
            results.append((nxt, st.with_(m=outcome.m, x=outcome.x, db=db, dp=dp,
                                          a_lo=None, a_hi=None)))
        return results

    def _dynamic(self, summary: Summary, insn: Insn, st: State, nxt: int,
                 vector: Optional[int], reason: str, *, is_call: bool
                 ) -> list[tuple[int, State]]:
        kind = insn.text()
        site = CallSite(insn.pc, vector, kind, st.m, st.x, st.db, st.dp, reason)
        (summary.calls if is_call else summary.tail_jumps).append(site)
        model = self.ram_models.get((vector or 0) & 0xFFFF) if vector is not None \
            and is_ram_code(vector) else None
        if model is not None and is_call:
            result = model(insn, st)
            site.note = f"{reason}; modelled RAM stub: {result[1]}"
            if result[0] is None:
                summary.stops.append((insn.pc, site.note))
                return []
            return [(nxt, result[0])]
        bound = self.dynamic_bindings.get(insn.pc)
        if bound is None and vector is not None:
            bound = self.dynamic_bindings.get(vector)
        if bound is not None:
            site.note = f"{reason}; bound by the caller to {fmt(bound)}"
            if not is_call:
                return [(bound, st)]
            call_kind = "JSL" if insn.name == "JSL" else "JSR"
            return self._call(summary, insn, st, nxt, bound, call_kind)
        if is_call and self.assume_dynamic_preserves:
            site.note = f"{reason}; ASSUMED to preserve M/X, DB, DP and S"
            summary.stops.append((insn.pc, site.note))
            return [(nxt, st.with_(a_lo=None, a_hi=None))]
        summary.stops.append((insn.pc, f"dynamic transfer: {reason}"))
        return []

    # -------------------------------------------------------------- effects
    def _note_frame(self, summary: Summary, insn: Insn, crossed: bool) -> None:
        if crossed:
            summary.frame_reads[insn.pc] = insn.text()

    def _record_access(self, summary: Summary, insn: Insn, st: State) -> None:
        name, mode = insn.name, insn.mode
        control = name in ("JMP", "JML", "JSR", "JSL", "PEA", "PER")
        if mode in SR_MODES:
            summary.sr_uses[insn.pc] = insn.text()
            if st.sp is not None and insn.operand > -st.sp:
                summary.frame_reads[insn.pc] = insn.text() + " (caller frame)"
        if mode in DP_MODES and st.dp == ENTRY:
            summary.dp_uses[insn.pc] = insn.text()
        if mode in DB_MODES and not control and st.db == ENTRY:
            summary.db_uses[insn.pc] = insn.text()
        if mode in (ABS, ABSX, ABSY) and not control:
            if is_mmio(insn.operand):
                bank = st.db if isinstance(st.db, int) else None
                if bank is None or is_system_bank(bank):
                    qual = "" if bank is not None else " (if DB is a system bank)"
                    summary.mmio[insn.pc] = insn.text() + qual
        if mode in (LONG, LONGX) and not control:
            bank, offset = insn.operand >> 16, insn.operand & 0xFFFF
            if is_system_bank(bank) and is_mmio(offset):
                summary.mmio[insn.pc] = insn.text()
            elif bank in (0x7E, 0x7F) or (is_system_bank(bank) and offset < 0x2000):
                summary.wram_long[insn.pc] = insn.text()


def _outcome_key(outcome: Outcome) -> tuple:
    return (outcome.pc, outcome.kind, str(outcome.m), str(outcome.x),
            str(outcome.sp), str(outcome.db), str(outcome.dp))


def describe_value(value: object) -> str:
    if value == ENTRY:
        return "caller's"
    if isinstance(value, int):
        return f"${value:02X}" if value <= 0xFF else f"${value:04X}"
    return "unknown"


# ---------------------------------------------------------------- scanning
def scan_references(rom: LoRom, target: int) -> list[tuple[int, str]]:
    """Byte-pattern candidates for direct JSL/JML/JSR/JMP to `target`.

    Candidates are not decoded in context: data and misaligned hits are
    possible. They exist so no child ABI is inferred from a single caller
    without looking at the others.
    """
    lo, hi, bank = target & 0xFF, (target >> 8) & 0xFF, target >> 16
    banks = {bank, bank ^ 0x80}
    hits: list[tuple[int, str]] = []
    data = rom.data
    for opcode, kind in ((0x22, "JSL"), (0x5C, "JML")):
        for b in banks:
            needle = bytes((opcode, lo, hi, b))
            start = data.find(needle)
            while start >= 0:
                hits.append((rom.cpu_address(start), kind))
                start = data.find(needle, start + 1)
    bank_lo = (bank & 0x7F) * 0x8000
    bank_hi = min(bank_lo + 0x8000, len(data))
    if target & 0xFFFF >= 0x8000:
        for opcode, kind in ((0x20, "JSR"), (0x4C, "JMP")):
            needle = bytes((opcode, lo, hi))
            start = data.find(needle, bank_lo, bank_hi)
            while start >= 0:
                hits.append((rom.cpu_address(start), kind))
                start = data.find(needle, start + 1, bank_hi)
    return sorted(hits)


def scan_ram_writers(rom: LoRom, first: int, last: int) -> list[tuple[int, str]]:
    """Byte-pattern candidates for stores into low WRAM `first..last`."""
    hits: list[tuple[int, str]] = []
    data = rom.data
    ops_abs = {0x8D: "STA", 0x8E: "STX", 0x8C: "STY", 0x9C: "STZ",
               0x9D: "STA ,x", 0x99: "STA ,y", 0x9E: "STZ ,x"}
    for address in range(first, last + 1):
        lo, hi = address & 0xFF, address >> 8
        for opcode, kind in ops_abs.items():
            needle = bytes((opcode, lo, hi))
            start = data.find(needle)
            while start >= 0:
                hits.append((rom.cpu_address(start), f"{kind} ${address:04X}"))
                start = data.find(needle, start + 1)
        for bank in (0x00, 0x7E, 0x80):
            for opcode, kind in ((0x8F, "STA long"), (0x9F, "STA long,x")):
                needle = bytes((opcode, lo, hi, bank))
                start = data.find(needle)
                while start >= 0:
                    hits.append((rom.cpu_address(start),
                                 f"{kind} ${bank:02X}:{address:04X}"))
                    start = data.find(needle, start + 1)
    return sorted(set(hits))


def parse_address(text: str) -> int:
    cleaned = text.replace("$", "").replace(":", "")
    value = int(cleaned, 16)
    if not 0 <= value <= 0xFFFFFF:
        raise ValueError(f"address out of range: {text}")
    return value


if __name__ == "__main__":  # pragma: no cover
    sys.exit("cfg65816 is a library; use ancient_cave_contracts.py")
