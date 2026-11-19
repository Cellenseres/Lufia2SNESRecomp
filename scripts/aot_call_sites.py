#!/usr/bin/env python3
"""Generate native bridge control-flow sites from static AOT metadata."""

import argparse
from bisect import bisect_left
from collections import defaultdict
import json
from pathlib import Path


def direct_calls(manifest):
    for node in manifest["nodes"].values():
        for demand in node["demands"]:
            if demand["kind"] != "direct_call" or demand["resolution"] != "aot_exact":
                continue
            target = demand["target"]
            yield demand["site_pc24"], target["pc24"], target["m"], target["x"]

    # The status-roll C spans hand off at these JSLs.
    random_scale = manifest["nodes"].get("808299:M1X0", {})
    if random_scale.get("disposition") == "aot_eligible":
        yield 0x85d409, 0x808299, 1, 0
        yield 0x85d43b, 0x808299, 1, 0


def entries(manifest, rom):
    modes_by_site = defaultdict(int)
    targets_by_site = {}
    for pc, target, accumulator_width, index_width in direct_calls(manifest):
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
        destination = int.from_bytes(rom[offset + 1:offset + length], "little")
        if opcode == 0x20:
            destination |= bank << 16
        if destination != target or accumulator_width not in (0, 1) or index_width not in (0, 1):
            continue
        targets_by_site[pc] = destination
        width = accumulator_width * 2 + index_width
        modes_by_site[pc] |= 1 << width
    for key, node in manifest["nodes"].items():
        mode = manifest["exit_modes"].get(key)
        pc = node["max_pc24"]
        if mode is None or pc is None or node["disposition"] != "aot_eligible":
            continue
        bank = pc >> 16
        address = pc & 0xffff
        if not 0x80 <= bank <= 0xbf or address < 0x8000:
            continue
        offset = ((bank & 0x7f) << 15) + (address & 0x7fff)
        if offset >= len(rom) or rom[offset] not in (0x60, 0x6b):
            continue
        if mode["m"] not in (0, 1) or mode["x"] not in (0, 1):
            continue
        targets_by_site[pc] = rom[offset]
        width = mode["m"] * 2 + mode["x"]
        modes_by_site[pc] |= 0x10 | (1 << width)
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
    page_bits = 12
    sites = [pc for pc, _, _ in data]
    first_page = (sites[0] >> page_bits) if sites else 0
    last_page = (sites[-1] >> page_bits) if sites else 0
    index_type = "uint16_t" if len(sites) <= 0xffff else "uint32_t"
    offsets = [bisect_left(sites, page << page_bits)
               for page in range(first_page, last_page + 2)]
    lines.append(f"\n#define LUFIA2_AOT_CALL_SITE_PAGE_BITS {page_bits}u\n")
    lines.append(f"#define LUFIA2_AOT_CALL_SITE_FIRST_PAGE 0x{first_page:x}u\n")
    lines.append(f"static const {index_type} lufia2_aot_call_page_offsets[] = {{\n")
    for index in range(0, len(offsets), 12):
        lines.append("    " + ", ".join(f"{value}u" for value in offsets[index:index + 12]) + ",\n")
    lines.append("};\n")
    generated = "".join(lines)
    if not args.output.is_file() or args.output.read_text(encoding="utf-8") != generated:
        args.output.write_text(generated, encoding="utf-8")


if __name__ == "__main__":
    main()
