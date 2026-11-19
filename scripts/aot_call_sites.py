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


def rom_call_target(rom, pc):
    bank = pc >> 16
    address = pc & 0xffff
    if not 0x80 <= bank <= 0xbf or address < 0x8000:
        return None
    offset = ((bank & 0x7f) << 15) + (address & 0x7fff)
    opcode = rom[offset] if offset < len(rom) else 0
    if opcode not in (0x20, 0x22):
        return None
    length = 3 if opcode == 0x20 else 4
    if address + length > 0x10000 or offset + length > len(rom):
        return None
    destination = int.from_bytes(rom[offset + 1:offset + length], "little")
    return destination | bank << 16 if opcode == 0x20 else destination


def selected_functions(metadata_path, coverage_path):
    from decomp_manifest import load_functions

    width_masks = {"M0X0": 1, "M0X1": 2, "M1X0": 4, "M1X1": 8}
    coverage = json.loads(coverage_path.read_text(encoding="utf-8"))
    selected = {row["address"] for row in coverage["functions"]
                if row["selection"] == "selected" and row["status"] == "verified"}
    return {int(function.address.replace(":", ""), 16): width_masks[function.entry_mx]
            for function in load_functions(metadata_path)
            if function.address in selected and function.entry_mx in width_masks}


def entries(manifest, rom, semantic_targets=None):
    modes_by_site = defaultdict(int)
    targets_by_site = {}
    for pc, target, accumulator_width, index_width in direct_calls(manifest):
        if accumulator_width not in (0, 1) or index_width not in (0, 1):
            continue
        if rom_call_target(rom, pc) != target:
            continue
        targets_by_site[pc] = target
        modes_by_site[pc] |= 1 << (accumulator_width * 2 + index_width)

    native_targets = defaultdict(int)
    code_banks = set()
    for node in manifest["nodes"].values():
        key = node["key"]
        bank = key["pc24"] >> 16
        if 0x80 <= bank <= 0xbf:
            code_banks.add(bank)
        if node["disposition"] == "aot_eligible" and key["m"] in (0, 1) and key["x"] in (0, 1):
            native_targets[key["pc24"]] |= 1 << (key["m"] * 2 + key["x"])

    for pc, widths in (semantic_targets or {}).items():
        native_targets[pc] |= widths
        if 0x80 <= pc >> 16 <= 0xbf:
            code_banks.add(pc >> 16)

    # C continuations can reach ROM call sites omitted from the static CFG.
    for bank in code_banks:
        for address in range(0x8000, 0x10000):
            pc = (bank << 16) | address
            target = rom_call_target(rom, pc)
            modes = native_targets.get(target, 0)
            if modes:
                targets_by_site[pc] = target
                modes_by_site[pc] |= modes

    return [(pc, targets_by_site[pc], modes) for pc, modes in sorted(modes_by_site.items())]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--decomp-functions", type=Path)
    parser.add_argument("--decomp-coverage", type=Path)
    args = parser.parse_args()
    if bool(args.decomp_functions) != bool(args.decomp_coverage):
        parser.error("The decomp function metadata and coverage report are required together")
    semantic_targets = (selected_functions(args.decomp_functions, args.decomp_coverage)
                        if args.decomp_functions else None)
    data = entries(json.loads(args.manifest.read_text(encoding="utf-8")),
                   args.rom.read_bytes(), semantic_targets)
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
