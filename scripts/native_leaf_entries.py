"""Retain original AOT envelopes around two verified semantic leaf bodies."""
import argparse
import hashlib
from pathlib import Path
import re
import sys

ROM_SHA256 = '7c34ecb16c10f551120ed7b86cfbc947042f479b52ee74bb3c40e92fdd192b3a'
LEAVES = (
    (0x83, 0xa6df, 0xa71c, 'Lufia2ActorResetAotBody', (94, 91)),
    (0x86, 0xe709, 0xe71e, 'Lufia2WorldIndexAotBody', (34, 34)),
)


def semantic_body(original, helper, cycles, index_width):
    charge = f'    cpu->master_cycles += {cycles} * (g_memsel ? 6 : 8);\n'
    returning = '    { uint16 _ret_s = cpu->S;'
    if original.count(charge) != 1 or original.count(returning) != 1:
        raise ValueError('Original linear leaf envelope changed')
    prefix, body = original.split(charge)
    operations, tail = body.split(returning)
    if re.search(r'\bgoto\b|\bcpu_dispatch\w*\s*\(', operations):
        raise ValueError('A leaf acquired another control-flow boundary')
    guarded = '\n'.join('  ' + line if line else line for line in operations.splitlines())
    return (prefix + charge + f'    extern int {helper}(CpuState *cpu);\n'
            + f'    if (cpu->m_flag != 1u || cpu->x_flag != {index_width}u ||\n'
            + f'        !{helper}(cpu)) {{\n' + guarded + '\n    }\n'
            + returning + tail)


def prepare(rom_path, sdk, generated, output):
    rom = rom_path.read_bytes()
    if hashlib.sha256(rom).hexdigest() != ROM_SHA256:
        raise ValueError('Expected the supported clean headerless US ROM')
    sys.path.insert(0, str(sdk / 'recompiler'))
    from v2.emit_function import emit_function
    output.mkdir(parents=True, exist_ok=True)
    for bank, start, end, helper, cycles in LEAVES:
        source = (generated / f'bank{bank:02x}_v2.c').read_text(encoding='utf-8')
        bridge = f'Lufia2DecompBridge_{bank:02X}{start:04X}'
        for index_width in (0, 1):
            name = f'bank_{bank:02X}_{start:04X}_M1X{index_width}'
            pattern = r'RecompReturn ' + name + r'\(CpuState \*cpu\) \{[\s\S]*?\n\}'
            matches = list(re.finditer(pattern, source))
            if len(matches) != 1 or bridge not in matches[0][0]:
                raise ValueError(f'Expected the selected decomp forwarding entry {name}')
            original = emit_function(rom, bank, start, 1, index_width,
                                     end=end, func_name=f'bank_{bank:02X}_{start:04X}')
            wrapped = semantic_body(original, helper, cycles[index_width], index_width)
            source, count = re.subn(pattern, lambda match: wrapped, source)
            if count != 1:
                raise ValueError(f'Ambiguous generated leaf {name}')
        target = output / f'bank{bank:02x}_v2.c'
        if not target.exists() or target.read_text(encoding='utf-8') != source:
            target.write_text(source, encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--generated', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    prepare(args.rom, args.sdk, args.generated, args.out)


if __name__ == '__main__':
    main()
