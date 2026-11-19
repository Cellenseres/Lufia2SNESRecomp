#!/usr/bin/env python3
"""Generate native bridge call sites from exact static AOT demands."""

import argparse
from collections import defaultdict
import json
from pathlib import Path


def entries(manifest, rom):
    modes_by_site = defaultdict(int)
    targets_by_site = {}
    for node in manifest["nodes"].values():
        for demand in node["demands"]:
            if demand["kind"] != "direct_call" or demand["resolution"] != "aot_exact":
                continue
            pc = demand["site_pc24"]
            bank = pc >> 16
            address = pc & 0xffff
            if not 0x80 <= bank <= 0xbf or address < 0x8000:
                continue
            offset = ((bank & 0x7f) << 15) + (address & 0x7fff)
            opcode = rom[offset] if offset < len(rom) else 0
            if opcode not in (0x20, 0x22):
                continue
            length = 3 if opcode == 0x20 else 4
            if address + length > 0x10000 or offset + length > len(rom):
                continue
            target = demand["target"]
            destination = int.from_bytes(rom[offset + 1:offset + length], "little")
            if opcode == 0x20:
                destination |= bank << 16
            if destination != target["pc24"] or target["m"] not in (0, 1) or target["x"] not in (0, 1):
                continue
            targets_by_site[pc] = destination
            width = target["m"] * 2 + target["x"]
            modes_by_site[pc] |= 1 << width
    return [(pc, targets_by_site[pc], modes) for pc, modes in sorted(modes_by_site.items())]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    data = entries(json.loads(args.manifest.read_text(encoding="utf-8")), args.rom.read_bytes())
    lines = ["static const struct Lufia2AotCallSite { uint32_t pc, target_and_widths; } lufia2_aot_call_sites[] = {\n"]
    for pc, target, modes in data:
        lines.append(f"    {{0x{pc:06x}u, 0x{modes:02x}{target:06x}u}},\n")
    lines.append("    {0u, 0u}\n};\n")
    generated = "".join(lines)
    if not args.output.is_file() or args.output.read_text(encoding="utf-8") != generated:
        args.output.write_text(generated, encoding="utf-8")


if __name__ == "__main__":
    main()
