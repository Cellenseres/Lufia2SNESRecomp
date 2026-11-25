#!/usr/bin/env python3
"""Generate native continuations for the ROM code reachable from known entries and tables."""

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re

from cfg65816 import CONDITIONAL, IMM_M, IMM_X, LoRom, OPCODES, OPERAND_LEN

CODE_BANKS = (0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x8e)
MODES = ((0, 0), (0, 1), (1, 0), (1, 1))
INLINE_TABLE_CALL = 0x828028
# JMP ($0008) reads its handler from this ROM table.
POINTER_TABLES = {0x82bade: 0x82bb66}
REGION_INSTRUCTIONS = 160

CONTROL = {0x20, 0x22, 0x60, 0x6b, 0x6c, 0x7c, 0xdc, 0xfc, 0x44, 0x54}
JUMPS = {0x80, 0x82, 0x4c, 0x5c}
REGISTERS = set(bytes.fromhex("0a1a2a3a4a6a88c8cae8 183858 78b8d8f8ea 8a98a8aa9bbbeb 1b3b5b7b9aba"))
STACK = set(bytes.fromhex("080b282b484b5a687a8babdafaf4"))
MODIFY = set(bytes.fromhex("04140c1c c6d6cede e6f6eefe 06160e1e 26362e3e 46564e5e 66766e7e"))
INDIRECT_MODES = {0x01, 0x03, 0x07, 0x11, 0x12, 0x13, 0x17}
UNSUPPORTED = {0x00, 0x02, 0x40, 0x42, 0x62, 0xcb, 0xd4, 0xdb, 0xfb}
PUSHED_LOADS = {0xa9: 0x48, 0xa2: 0xda, 0xa0: 0x5a}
CHANGING = {"INC", "DEC", "INX", "DEX", "INY", "DEY", "ADC", "SBC", "TSB", "TRB",
            "ASL", "LSR", "ROL", "ROR"}


def canonical(pc):
    return pc | 0x800000 if pc & 0xffff >= 0x8000 and pc >> 16 < 0x40 else pc


class Code:
    def __init__(self, rom, manifest, functions):
        self.rom = rom
        self.entries = {canonical(node["key"]["pc24"]) for node in manifest["nodes"].values()}
        self.entries |= {canonical(int(function.address.replace(":", ""), 16)) for function in functions}
        self.states = set()
        self.indirect = set()
        self.pushed = set()

    def word(self, pc):
        low = self.rom.byte(pc)
        high = self.rom.byte((pc & 0xff0000) | ((pc + 1) & 0xffff))
        return None if low is None or high is None else low | high << 8

    def decode(self, pc, m, x):
        opcode = self.rom.byte(pc)
        name, mode = OPCODES[opcode]
        size = (1 if m else 2) if mode == IMM_M else (1 if x else 2) if mode == IMM_X else OPERAND_LEN[mode]
        if (pc & 0xffff) + size >= 0x10000:
            return None
        operand = sum(self.rom.byte(pc + 1 + i) << 8 * i for i in range(size))
        return opcode, name, operand, size + 1

    def successors(self, pc, m, x):
        decoded = self.decode(pc, m, x)
        if decoded is None:
            return
        opcode, name, operand, length = decoded
        bank = pc & 0xff0000
        following = bank | ((pc + length) & 0xffff)
        # Code pushes its own return address with PEA or an immediate load in front of a jump.
        if opcode == 0xf4 or (length == 3 and PUSHED_LOADS.get(opcode) == self.rom.byte(following)):
            self.pushed.add(bank | ((operand + 1) & 0xffff))
        if name in CONDITIONAL or opcode == 0x80:
            yield bank | ((pc + 2 + ((operand ^ 0x80) - 0x80)) & 0xffff), m, x
            if opcode == 0x80:
                return
        elif opcode == 0x82:
            yield bank | ((pc + 3 + operand) & 0xffff), m, x
            return
        elif opcode in (0x4c, 0x5c):
            yield bank | operand if opcode == 0x4c else canonical(operand), m, x
            return
        elif opcode in (0x20, 0x22):
            target = bank | operand if opcode == 0x20 else canonical(operand)
            # Recorded return widths are incomplete, so every combination stays possible.
            for mode in MODES:
                yield target, *mode
                if target != INLINE_TABLE_CALL:
                    yield following, *mode
            return
        elif opcode in (0x6c, 0x7c, 0xdc, 0xfc):
            self.indirect.add(pc)
            if opcode == 0xfc:
                for mode in MODES:
                    yield following, *mode
            return
        elif name in ("RTS", "RTL", "RTI", "STP", "COP", "BRK"):
            return
        if opcode == 0x28:
            for mode in MODES:
                yield following, *mode
        elif opcode == 0xc2:
            yield following, 0 if operand & 0x20 else m, 0 if operand & 0x10 else x
        elif opcode == 0xe2:
            yield following, 1 if operand & 0x20 else m, 1 if operand & 0x10 else x
        else:
            yield following, m, x

    def walk(self, entries):
        work = [(entry, *mode) for entry in entries for mode in MODES]
        while work:
            state = work.pop()
            pc = state[0]
            if state in self.states or pc >> 16 not in CODE_BANKS or pc & 0xffff < 0x8000:
                continue
            self.states.add(state)
            work.extend(self.successors(*state))

    def table(self, address):
        bank, base = address & 0xff0000, address & 0xffff
        limit, found = 0x10000, False
        for index in range(256):
            entry = base + 2 * index
            if entry >= limit or entry >= 0xfffe:
                break
            target = self.word(bank | entry)
            if target < 0x8000:
                # Tables indexed from a later entry start inside preceding bytes.
                if found or index >= 4:
                    break
                continue
            found = True
            if target > entry:
                limit = min(limit, target)
            yield bank | target

    def explore(self):
        known, entries = set(), self.entries
        while entries:
            self.walk(entries)
            known |= entries
            entries = {pc for pc in self.pushed if pc & 0xffff >= 0x8000}
            for pc in self.indirect:
                operand = self.word(pc + 1)
                if self.rom.byte(pc) in (0x7c, 0xfc) and operand >= 0x8000:
                    entries.update(self.table((pc & 0xff0000) | operand))
                elif pc in POINTER_TABLES:
                    entries.update(self.table(POINTER_TABLES[pc]))
            for pc in {pc for pc, _, _ in self.states}:
                if self.rom.byte(pc) == 0x20 and (pc & 0xff0000) | self.word(pc + 1) == INLINE_TABLE_CALL:
                    entries.update(self.table(pc + 3))
            entries -= known


def step_name(opcode, wide=None):
    name, mode = OPCODES[opcode]
    parts = {"(dp)": "DpInd", "(dp,x)": "DpIndX", "(dp),y": "DpIndY", "[dp]": "DpLong",
             "[dp],y": "DpLongY", "(sr),y": "SrIndY", "(abs)": "AbsInd", "(abs,x)": "AbsIndX",
             "[abs]": "AbsLongInd", "dp,x": "DpX", "dp,y": "DpY", "abs,x": "AbsX",
             "abs,y": "AbsY", "long,x": "LongX", "imm_m": "", "imm_x": "", "imp": "", "imm8": ""}
    suffix = parts.get(mode, mode.capitalize())
    return f"Step{name.capitalize()}{suffix}" + ("" if wide is None else "Word" if wide else "Byte")


def core(opcode):
    name, mode = OPCODES[opcode]
    if opcode in CONTROL:
        return "ContinuationControl"
    if opcode in (0xc2, 0xe2):
        return "ContinuationStatus"
    if name in CONDITIONAL:
        return "ContinuationBranch"
    if opcode in JUMPS:
        return "ContinuationJump"
    if opcode in REGISTERS:
        return "ContinuationRegister"
    if opcode in STACK:
        return "ContinuationStack"
    if opcode in MODIFY:
        return "ContinuationModify"
    if opcode & 0x1f in INDIRECT_MODES:
        return "ContinuationIndirect"
    return "ContinuationMemory"


def bridge_addresses(roots):
    """Addresses named in hand-written bridges own their entry and must start a span."""
    addresses = set()
    for root in roots:
        files = [root] if root.is_file() else sorted(root.iterdir())
        for path in files:
            if path.suffix in (".c", ".h", ".inc", ".cmake"):
                for value in re.findall(r"0x(8[0-9a-fA-F]{5})u?\b", path.read_text(encoding="utf-8")):
                    addresses.add(int(value, 16))
                for value in re.findall(r"0x(0[0-9a-fA-F]{5})u?\b", path.read_text(encoding="utf-8")):
                    addresses.add(0x800000 | int(value, 16))
    return addresses


class Emitter:
    def __init__(self, code, owned):
        self.code = code
        self.modes = defaultdict(set)
        for pc, m, x in code.states:
            if code.rom.byte(pc) not in UNSUPPORTED and code.decode(pc, m, x):
                self.modes[pc].add((m, x))
        self.boundaries = set(owned) | {pc for pc in self.modes if code.rom.byte(pc) in CONTROL}
        self.used = set()
        self.find_loops()

    def forms(self, pc):
        """Distinct decodes of one address: (wide or None, operand, length)."""
        opcode = self.code.rom.byte(pc)
        mode = OPCODES[opcode][1]
        result = {}
        for m, x in self.modes[pc]:
            _, _, operand, length = self.code.decode(pc, m, x)
            wide = None
            if mode in (IMM_M, IMM_X):
                wide = not (m if mode == IMM_M else x)
            result[wide] = (operand, length)
        return opcode, sorted(result.items(), key=lambda item: item[0] is True)

    def find_loops(self):
        """Loop heads start a span; polling loops stay visible instruction by instruction."""
        addresses = sorted(self.modes)
        for pc in addresses:
            opcode, forms = self.forms(pc)
            if OPCODES[opcode][0] not in CONDITIONAL and opcode not in (0x80, 0x82, 0x4c):
                continue
            operand = forms[0][1][0]
            target = self.target(pc, opcode, operand)
            if target > pc:
                continue
            self.boundaries.add(target)
            body = [address for address in addresses if target <= address <= pc] if pc - target <= 256 else []
            names = {OPCODES[self.code.rom.byte(address)][0] for address in body}
            if body and not names & CHANGING and not any(
                    name.startswith(("ST", "PH", "PE", "JS", "MV")) for name in names):
                self.boundaries |= set(body)

    @staticmethod
    def target(pc, opcode, operand):
        bank = pc & 0xff0000
        if opcode == 0x4c:
            return bank | operand
        if opcode == 0x82:
            return bank | ((pc + 3 + operand) & 0xffff)
        return bank | ((pc + 2 + ((operand ^ 0x80) - 0x80)) & 0xffff)

    def regions(self):
        current = []
        for pc in sorted(self.modes):
            ends = current and (self.code.rom.byte(current[-1]) in CONTROL | JUMPS)
            if current and (pc >> 16 != current[-1] >> 16 or pc - current[-1] > 256 or
                            (ends and len(current) >= REGION_INSTRUCTIONS) or
                            len(current) >= 4 * REGION_INSTRUCTIONS):
                yield current
                current = []
            current.append(pc)
        if current:
            yield current

    def goto(self, pc, target, members):
        if target > pc and target in members and target not in self.boundaries:
            return f"goto L_{target & 0xffff:04x};"
        return "return;"

    def statement(self, pc, opcode, wide, operand, length, members):
        self.used.add((opcode, wide))
        call = f"{step_name(opcode, wide)}(run, 0x{pc & 0xffff:04x}u, 0x{operand:x}u)"
        following = (pc & 0xff0000) | ((pc + length) & 0xffff)
        if opcode in CONTROL or opcode == 0x5c:
            return f"(void){call};\n    return;", None
        if opcode in JUMPS:
            jump = self.goto(pc, self.target(pc, opcode, operand), members)
            if jump == "return;":
                return f"(void){call};\n    return;", None
            return f"if ({call} == CONTINUATION_STOP)\n        return;\n    {jump}", None
        after = self.goto(pc, following, members)
        if OPCODES[opcode][0] in CONDITIONAL:
            taken = self.goto(pc, self.target(pc, opcode, operand), members)
            if taken == "return;" and after == "return;":
                return f"(void){call};\n    return;", None
            if taken == "return;":
                return f"if ({call} != CONTINUATION_NEXT)\n        return;", after
            if after == "return;":
                return f"if ({call} != CONTINUATION_TAKEN)\n        return;\n    {taken}", None
            return (f"switch ({call}) {{\n    case CONTINUATION_TAKEN: {taken}\n"
                    f"    case CONTINUATION_STOP: return;\n    default: break;\n    }}"), after
        if after == "return;":
            return f"(void){call};\n    return;", None
        return f"if ({call} == CONTINUATION_STOP)\n        return;", after

    def region(self, members):
        first, member_set = members[0], set(members)
        lines = [f"static void Continue{first:06X}(Continuation *run, uint16_t address) {{",
                 "    switch (address) {"]
        lines += [f"    case 0x{pc & 0xffff:04x}u: goto L_{pc & 0xffff:04x};" for pc in members]
        lines += ["    default: return;", "    }"]
        for index, pc in enumerate(members):
            opcode, forms = self.forms(pc)
            upcoming = members[index + 1] if index + 1 < len(members) else None
            lines.append(f"L_{pc & 0xffff:04x}:")
            wide, (operand, length) = forms[0]
            if len(forms) == 2:
                flag = "mf" if OPCODES[opcode][1] == IMM_M else "xf"
                wide_operand, wide_length = forms[1][1]
                text, after = self.statement(pc, opcode, True, wide_operand, wide_length, member_set)
                body = [text] + ([after] if after else [])
                lines.append(f"    if (!run->registers->{flag}) {{\n    " + "\n    ".join(body) + "\n    }")
            text, after = self.statement(pc, opcode, wide, operand, length, member_set)
            lines.append("    " + text)
            if after and after != f"goto L_{(upcoming or 0) & 0xffff:04x};":
                lines.append("    " + after)
        lines += ["}", ""]
        return "\n".join(lines)

    def generate(self):
        regions = list(self.regions())
        bodies = [self.region(members) for members in regions]
        lines = []
        for opcode, wide in sorted(self.used, key=lambda item: (item[0], item[1] is True)):
            name = step_name(opcode, wide)
            if wide is None:
                lines.append(f"CONTINUATION_STEP({name}, {core(opcode)}, 0x{opcode:02x}u)")
            else:
                lines.append(f"CONTINUATION_IMMEDIATE({name}, 0x{opcode:02x}u, {str(wide).lower()})")
        lines += [""] + bodies
        lines.append("static const ContinuationRange native_continuation_ranges[] = {")
        for members in regions:
            lines.append(f"    {{0x{members[0]:06x}u, 0x{members[-1]:06x}u, Continue{members[0]:06X}}},")
        lines += ["};", ""]
        return "\n".join(lines)


def main():
    from decomp_manifest import load_functions

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("rom", type=Path)
    parser.add_argument("functions", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("bridges", type=Path, nargs="+", help="hand-written sources and directories")
    args = parser.parse_args()
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    code = Code(LoRom(args.rom.read_bytes()), manifest, load_functions(args.functions))
    code.explore()
    generated = Emitter(code, bridge_addresses(args.bridges)).generate()
    if not args.output.is_file() or args.output.read_text(encoding="utf-8") != generated:
        args.output.write_text(generated, encoding="utf-8")


if __name__ == "__main__":
    main()
