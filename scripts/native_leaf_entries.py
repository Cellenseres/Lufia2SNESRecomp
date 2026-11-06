"""Retain original AOT envelopes around verified semantic leaf bodies."""
import argparse
import hashlib
from pathlib import Path
import re
import sys

ROM_SHA256 = '7c34ecb16c10f551120ed7b86cfbc947042f479b52ee74bb3c40e92fdd192b3a'
LEAVES = (
    (0x83, 0xa6df, 0xa71c, 'Lufia2ActorResetAotBody', (94, 91), 1),
    (0x86, 0xe709, 0xe71e, 'Lufia2WorldIndexAotBody', (34, 34), 1),
    (0x86, 0x973e, 0x9748, 'Lufia2WorldDisableDisplayAotBody', (17, 17), 1),
    (0x86, 0xae05, 0xae1e, 'Lufia2WorldBlockPointerAotBody', (46, 46), 0),
)
ARRIVALS = (
    (0x9ab1, 'Lufia2WorldMovingArrivalAotBody'),
    (0x9ab8, 'Lufia2WorldStoppedArrivalAotBody'),
)


def semantic_body(original, helper, cycles, index_width, accumulator_width):
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
            + f'    if (cpu->m_flag != {accumulator_width}u || cpu->x_flag != {index_width}u ||\n'
            + f'        !{helper}(cpu)) {{\n' + guarded + '\n    }\n'
            + returning + tail)


def arrival_body(original, helper):
    label = re.search(r'^  L_[0-9A-F]+_M1X0:', original, re.MULTILINE)
    tail = '    { uint16 _ret_s = cpu->S;'
    if not label or original.count(tail) != 2:
        raise ValueError('Original arrival control-flow envelope changed')
    returning = iter(('NativeArrivalReturn_9AD8', 'NativeArrivalReturn_9ADA'))
    body = original[label.start():]
    body = re.sub(re.escape(tail), lambda match: '  ' + next(returning) + ':\n' + tail, body)
    entry = (f'    extern uint32 {helper}(CpuState *cpu);\n'
             + '    const uint32 _arrival_exit = cpu->m_flag == 1u && cpu->x_flag == 0u\n'
             + f'        ? {helper}(cpu) : 0u;\n'
             + '    if (_arrival_exit == 0x869ad8u) goto NativeArrivalReturn_9AD8;\n'
             + '    if (_arrival_exit == 0x869adau) goto NativeArrivalReturn_9ADA;\n'
             + '    if (_arrival_exit) {\n'
             + '        RecompStackPopYield();\n'
             + '        return interp_bridge_lle_yield_unwind(cpu, _arrival_exit);\n'
             + '    }\n')
    return original[:label.start()] + entry + body


def prepare(rom_path, sdk, generated, output, arrival_boundaries=False):
    rom = rom_path.read_bytes()
    if hashlib.sha256(rom).hexdigest() != ROM_SHA256:
        raise ValueError('Expected the supported clean headerless US ROM')
    sys.path.insert(0, str(sdk / 'recompiler'))
    from v2.emit_function import emit_function
    output.mkdir(parents=True, exist_ok=True)
    sources = {leaf[0]: (generated / f'bank{leaf[0]:02x}_v2.c').read_text(encoding='utf-8')
               for leaf in LEAVES}
    for bank, start, end, helper, cycles, accumulator_width in LEAVES:
        source = sources[bank]
        bridge = f'Lufia2DecompBridge_{bank:02X}{start:04X}'
        for index_width in (0, 1):
            name = f'bank_{bank:02X}_{start:04X}_M{accumulator_width}X{index_width}'
            pattern = r'RecompReturn ' + name + r'\(CpuState \*cpu\) \{[\s\S]*?\n\}'
            matches = list(re.finditer(pattern, source))
            if len(matches) != 1 or bridge not in matches[0][0]:
                raise ValueError(f'Expected the selected decomp forwarding entry {name}')
            original = emit_function(rom, bank, start, accumulator_width, index_width,
                                     end=end, func_name=f'bank_{bank:02X}_{start:04X}')
            wrapped = semantic_body(original, helper, cycles[index_width], index_width,
                                    accumulator_width)
            source, count = re.subn(pattern, lambda match: wrapped, source)
            if count != 1:
                raise ValueError(f'Ambiguous generated leaf {name}')
        sources[bank] = source
    if arrival_boundaries:
        source = sources[0x86]
        for start, helper in ARRIVALS:
            name = f'bank_86_{start:04X}_M1X0'
            pattern = r'RecompReturn ' + name + r'\(CpuState \*cpu\) \{[\s\S]*?\n\}'
            matches = list(re.finditer(pattern, source))
            if len(matches) != 1 or f'Lufia2DecompBridge_86{start:04X}' not in matches[0][0]:
                raise ValueError(f'Expected the selected arrival forwarding entry {name}')
            original = emit_function(rom, 0x86, start, 1, 0,
                                     end=0x9adb, func_name=f'bank_86_{start:04X}')
            wrapped = arrival_body(original, helper)
            source, count = re.subn(pattern, lambda match: wrapped, source)
            if count != 1:
                raise ValueError(f'Ambiguous generated arrival {name}')
        sources[0x86] = source
    for bank, source in sources.items():
        target = output / f'bank{bank:02x}_v2.c'
        if not target.exists() or target.read_text(encoding='utf-8') != source:
            target.write_text(source, encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--generated', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--arrival-boundaries', action='store_true')
    args = parser.parse_args()
    prepare(args.rom, args.sdk, args.generated, args.out, args.arrival_boundaries)


if __name__ == '__main__':
    main()
