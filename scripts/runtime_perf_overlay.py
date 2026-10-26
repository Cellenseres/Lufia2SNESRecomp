"""Add optional host profiling wrappers to immutable consumer source copies."""
import argparse
import hashlib
import json
import re
from pathlib import Path


def code_mask(text):
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def definitions(text):
    masked = code_mask(text)
    pattern = r'^(?P<prefix>[A-Za-z_][A-Za-z0-9_* \t]*?)\b(?P<name>[A-Za-z_]\w*)\s*\((?P<params>[^{};]*)\)\s*\{'
    for match in re.finditer(pattern, masked, re.MULTILINE):
        depth, end = 1, match.end()
        while depth and end < len(masked):
            depth += (masked[end] == '{') - (masked[end] == '}')
            end += 1
        if depth:
            raise ValueError('Unbalanced function: ' + match['name'])
        yield match, end


def transform(text, kind, bindings=None):
    groups = {
        'apu': {name: ('LUFIA2_PERF_APU', False) for name in (
            'apu_runToGuestCycle', 'apu_runUntilPortQueueEmpty',
            'apu_waitForTransferReady', 'apu_finishHleTransfer')},
        'snes': {'snes_catchupApu': ('LUFIA2_PERF_APU', False)},
        'dsp': {'dsp_cycle': ('LUFIA2_PERF_DSP_SAMPLE', True)},
        'spc': {'spc_runOpcode': ('LUFIA2_PERF_SPC_SAMPLE', True)},
        'native_spc': {'RunDriver': ('LUFIA2_PERF_NATIVE_SPC', False),
                       'DriverUploaded': ('LUFIA2_PERF_NATIVE_SPC_SIGNATURE', False)},
        'interp': {'interp816_runOpcode': ('LUFIA2_PERF_INTERP_SAMPLE', False)},
        'interp_bridge': {'bridge_bus_read': ('LUFIA2_PERF_NATIVE_BUS_SAMPLE', True),
                   'Lufia2DecompNmiAccountCycles': ('LUFIA2_PERF_NATIVE_CLOCK_SAMPLE', True)},
        'spc_semantic': {'Lufia2SpcDriverRun': ('LUFIA2_PERF_NATIVE_SPC_BODY_SAMPLE', True)},
        'offload': {'Lufia2PpuOffloadRecordLine': ('LUFIA2_PERF_PPU_RECORD', False)},
        'ppu': {
            'ppu_runLine': ('LUFIA2_PERF_PPU_RUN_LINE', False),
            'PpuDrawWholeLine': ('LUFIA2_PERF_PPU_LINE', False),
            'PpuDrawBackgrounds': ('LUFIA2_PERF_PPU_BG', False),
            'PpuDrawSprites': ('LUFIA2_PERF_PPU_SPR', False),
            'ppu_evaluateSprites': ('LUFIA2_PERF_PPU_EVAL', False)},
        'rtl': {'SimpleHdma_DoLine': ('LUFIA2_PERF_HDMA', False)},
        'map': {
            'Lufia2PrepareMapWidescreen': ('LUFIA2_PERF_MAP_PREPARE', False),
            'Lufia2FinalizeMapWidescreen': ('LUFIA2_PERF_MAP_FINALIZE', False)},
        'shadow': {'WsShadowFrame': ('LUFIA2_PERF_SHADOW', False)},
        'intro': {'Lufia2IntroWidescreenPaint': ('LUFIA2_PERF_INTRO', False)},
        'menu': {'Lufia2MenuUiCompose': ('LUFIA2_PERF_UI', False)},
        'battle': {'Lufia2BattleUiCompose': ('LUFIA2_PERF_UI', False),
                   'Lufia2BattleUiFinish': ('LUFIA2_PERF_BATTLE_POST', False)},
        'battle_job': {
            'Lufia2BattlePpuJobFreeze': ('LUFIA2_PERF_BATTLE_FREEZE', False),
            'PrepareInternal': ('LUFIA2_PERF_BATTLE_JOB', False),
            'RecordGpuScene': ('LUFIA2_PERF_BATTLE_SCENE', False),
            'FreezeJobFrame': ('LUFIA2_PERF_JOB_FREEZE_FRAME', False),
            'CompactJobArt': ('LUFIA2_PERF_JOB_COMPACT', False),
            'SampleJobEdge': ('LUFIA2_PERF_JOB_EDGE_SAMPLE', True),
            'RecordedObjects': ('LUFIA2_PERF_JOB_OBJECTS', False),
            'RecordedRegisters': ('LUFIA2_PERF_JOB_REGISTERS', False),
            'SourceRange': ('LUFIA2_PERF_JOB_SOURCES', True)},
        'battle_effects': {
            'Lufia2BattleEffectsPixelPolicyMatches': ('LUFIA2_PERF_BATTLE_POLICY', False),
            'Lufia2BattleEffectsGpuArtRow': ('LUFIA2_PERF_JOB_ART_ROW', False),
            'Lufia2BattleEffectsGpuWindows': ('LUFIA2_PERF_JOB_WINDOWS', False),
            'Lufia2BattleEffectsGpuHudLine': ('LUFIA2_PERF_BATTLE_HUD', False),
            'Lufia2BattleEffectsGpuHudLineWithInputs': ('LUFIA2_PERF_BATTLE_HUD', False),
            'DrawHudSource': ('LUFIA2_PERF_HUD_SOURCE', False),
            'CachedHudScanline': ('LUFIA2_PERF_HUD_SCAN', False),
            'CachedHudGeometry': ('LUFIA2_PERF_HUD_GEOMETRY', False),
            'TitleLine': ('LUFIA2_PERF_HUD_TITLE', False),
            'PartyLine': ('LUFIA2_PERF_HUD_PARTY', False),
            'DrawHudOverlay': ('LUFIA2_PERF_BATTLE_OVERLAY', False),
            'Lufia2BattleEffectsGpuArtPalette': ('LUFIA2_PERF_BATTLE_ART', False)},
        'bridge': {},
        'runtime': {'Lufia2RunInterrupt': ('LUFIA2_PERF_RUNTIME_NMI', False),
                    'Lufia2RunToBoundary': ('LUFIA2_PERF_RUNTIME_DISPATCH', False)},
        'audio': {},
    }
    counters = {
        'audio_trace_on_cpu_port_read': 'LUFIA2_PERF_PORT_READ',
        'audio_trace_on_cpu_port_write': 'LUFIA2_PERF_PORT_WRITE',
        'ActorBridgeFallback': 'LUFIA2_PERF_FALLBACK',
    }
    wanted = dict(groups[kind])
    if kind == 'bridge':
        frame_steps = ('ActorBridgeFrameReturn', 'ActorBridgeSelectResume',
                       'ActorBridgePrepareCursorPoll', 'ActorBridgeFrameSaveStep',
                       'ActorBridgeMenuRectangleStep', 'ActorBridgeFramePollStep')
        sound_steps = ('ActorBridgeSoundPoll', 'ActorBridgeSoundUpload',
                       'ActorBridgeSoundBranch', 'ActorBridgeBootSound',
                       'ActorBridgeSoundPollStep', 'ActorBridgeSoundPollRun',
                       'ActorBridgeSoundTransferOpcode')
        present = {m['name'] for m, _ in definitions(text)}
        for name in present.intersection(frame_steps):
            wanted[name] = ('LUFIA2_PERF_NATIVE_FRAME_STEP', False)
        for name in present.intersection(sound_steps):
            wanted[name] = ('LUFIA2_PERF_NATIVE_SOUND_STEP', False)
    found, edits = [], []
    for match, end in definitions(text):
        name = match['name']
        counter = counters.get(name) if kind in ('audio', 'bridge') else None
        pc = 0
        if kind == 'bridge' and name.startswith('Lufia2DecompBridge_'):
            pc = (bindings or {}).get(name)
            if pc is None:
                suffix = name.removeprefix('Lufia2DecompBridge_')
                if len(suffix) == 6:
                    pc = int(suffix, 16)
                else:
                    # Legacy unbound adapters have no published PC mapping.
                    pc = 0
            group, sampled = 'LUFIA2_PERF_DECOMP', False
        elif name in wanted:
            group, sampled = wanted[name]
        elif counter:
            group, sampled = None, False
        else:
            continue
        params = text[match.start('params'):match.end('params')]
        args = []
        if params.strip() not in ('', 'void'):
            for param in params.split(','):
                if '(' in param or '[' in param:
                    raise ValueError('Unsupported wrapper parameter: ' + param)
                args.append(re.search(r'(\w+)\s*$', param)[1])
        signature = text[match.start():match.end()-1].rstrip()
        implementation = name + '_PerfBody'
        body = text[match.start():end]
        offset = match.start('name') - match.start()
        body = body[:offset] + implementation + body[offset + len(name):]
        call = implementation + '(' + ', '.join(args) + ')'
        is_void = re.sub(r'\b(static|inline|NOINLINE)\b', '', match['prefix']).strip() == 'void'
        ret_type = re.sub(r'\b(static|inline|NOINLINE)\b', '', match['prefix']).strip()
        run = call + ';' if is_void else ret_type + ' result = ' + call + ';'
        result = '' if is_void else '\n    return result;'
        passthrough = call + '; return;' if is_void else 'return ' + call + ';'
        if counter:
            wrapper = f'{signature} {{\n    Lufia2PerfCount({counter});\n    {run}{result}\n}}'
        else:
            enter = f'Lufia2PerfEnterSample(&scope, {group});' if sampled else \
                f'Lufia2PerfEnter(&scope, {group}, ' + (('entry' if kind == 'bridge' else ('vector_pc' if name == 'Lufia2RunInterrupt' else 'entry_pc')) if name in wanted and kind in ('bridge', 'runtime') else f'0x{pc:06x}u') + ');'
            extra = ''
            if kind == 'native_spc' and name == 'RunDriver':
                extra = '\n    if ((unsigned)s_driver.wait < 8u) Lufia2PerfCount(LUFIA2_PERF_SPC_WAIT_BASE + (unsigned)s_driver.wait);'
            if kind == 'bridge' and name.startswith('Lufia2DecompBridge_'):
                extra = '\n    if (result != RECOMP_RETURN_NORMAL) Lufia2PerfCount(LUFIA2_PERF_HANDOFF);'
            probe = '\n    Lufia2DecompFramePollProbe(cpu, entry);' if kind == 'bridge' and name == 'ActorBridgeFramePollStep' else ''
            wrapper = f'''{signature} {{
    if (!lufia2_perf_enabled) {{ {passthrough} }}{probe}
    Lufia2PerfScope scope;
    {enter}
    {run}
    Lufia2PerfLeave(&scope);{extra}{result}
}}'''
        edits.append((match.start(), end, body + '\n\n' + wrapper))
        found.append(name)
    missing = set(wanted) - set(found)
    if missing or not found:
        raise ValueError(f'{kind}: missing profiler anchors: {sorted(missing)}')
    if kind == 'audio' and not set(counters).intersection(found):
        raise ValueError('Missing audio port counters')
    for start, end, replacement in reversed(edits):
        text = text[:start] + replacement + text[end:]
    prefix = '#define LUFIA2_NATIVE_IDLE_DIAGNOSTICS 1\n' if kind == 'interp_bridge' else ''
    return prefix + '#include "lufia2_perf.h"\n' + text, found


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--kind', required=True)
    parser.add_argument('--bindings', type=Path)
    args = parser.parse_args()
    bindings = {}
    if args.bindings:
        text = args.bindings.read_text()
        for address, symbol in re.findall(r'address\s*=\s*"([0-9A-Fa-f:]+)"\s*\nbridge\s*=\s*"(\w+)"', text):
            bindings[symbol] = int(address.replace(':', ''), 16)
    original = args.input.read_text()
    if args.kind == 'interp_bridge':
        # Expand this one maintained timing include only in the private
        # diagnostic copy, so its original per-opcode boundary can be sampled.
        pattern = r'^#include "([^"\n]+/src/decomp_bridge/nmi_cycles[.]inc)"$'
        matches = list(re.finditer(pattern, original, re.MULTILINE))
        if len(matches) != 1:
            raise ValueError('Expected one reviewed native clock include')
        match = matches[0]
        original = original[:match.start()] + Path(match[1]).read_text() + original[match.end():]
    output, names = transform(original, args.kind, bindings)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text() != output:
        args.output.write_text(output)
    receipt = {'input': str(args.input), 'input_sha256': hashlib.sha256(args.input.read_bytes()).hexdigest(),
               'output_sha256': hashlib.sha256(args.output.read_bytes()).hexdigest(), 'wrapped': names}
    args.output.with_suffix('.json').write_text(json.dumps(receipt, indent=2))
    print(f'Profiler {args.kind}: {len(names)} wrappers')


if __name__ == '__main__':
    main()
