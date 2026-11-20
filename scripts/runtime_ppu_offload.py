"""Keep timeline work authoritative while exposing a pure pixel replay."""
import argparse
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--input', required=True)
parser.add_argument('--output', required=True)
args = parser.parse_args()
source = Path(args.input).read_text()
anchor = '    if (ppu->renderFlags & kPpuRenderFlags_NewRenderer) {\n      PPU_T0; PpuDrawWholeLine(ppu, line); PPU_ACC(g_ppu_sec_line_ms);'
if source.count(anchor) != 1:
    raise SystemExit('PPU pixel boundary changed or ambiguous')
source = source.replace(anchor, '    if (Lufia2PpuOffloadRecordLine(ppu, (unsigned)line)) return;\n' + anchor)
write_anchor = 'void ppu_write(Ppu* ppu, uint8_t adr, uint8_t val) {\n'
if source.count(write_anchor) != 1:
    raise SystemExit('PPU memory-write boundary changed or ambiguous')
source = source.replace(write_anchor, write_anchor +
    '  if (lufia2_ppu_offload_recording) Lufia2PpuOffloadBeforeMemoryWrite(ppu, adr);\n')


def once(text, old, new):
    if text.count(old) != 1:
        raise SystemExit('Deferred sprite anchor changed or ambiguous: ' + old[:60])
    return text.replace(old, new)


def function_end(text, start):
    """Index after the closing brace, skipping comments and strings."""
    depth, i = 0, text.index('{', start)
    while i < len(text):
        if text.startswith('/*', i):
            i = text.index('*/', i) + 2
            continue
        if text.startswith('//', i):
            i = text.index('\n', i)
            continue
        if text[i] in '"\'':
            quote, i = text[i], i + 1
            while text[i] != quote:
                i += 2 if text[i] == '\\' else 1
        elif text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise SystemExit('Unbalanced host sprite evaluator')


# Sprite rows on the helper core: every decision stays here, only the
# tile fetch and pixel writes move into recorded slivers.
host_start = source.index('static bool ppu_evaluateSprites_Host(Ppu* ppu, int line) {')
host_end = function_end(source, host_start)
deferred = source[host_start:host_end]
deferred = once(deferred, 'ppu_evaluateSprites_Host(', 'ppu_evaluateSprites_Defer(')
deferred = once(deferred,
    '            const uint16 *addr = &PpuRenderVram(ppu)[(objAdr + usedTile * 16 + (row & 0x7)) & 0x7fff];\n'
    '            uint32 plane = Lufia2PpuDecode4bpp((uint32)addr[0] | (uint32)addr[8] << 16);\n',
    '            const unsigned vram_index = (objAdr + usedTile * 16 + (row & 0x7)) & 0x7fff;\n')
deferred = once(deferred,
    '            PpuZbufType *dst = ppu->objBuffer.data + col + x + px_left + kPpuExtraLeftRight;\n', '')
loop_open = '            for (int px = px_left; px < px_right; px++, dst++) {\n'
loop_close = '                dst[0] = z + pixel;\n            }\n'
if deferred.count(loop_open) != 1 or deferred.count(loop_close) != 1:
    raise SystemExit('Deferred sprite pixel loop changed or ambiguous')
loop_start = deferred.index(loop_open)
loop_end = deferred.index(loop_close, loop_start) + len(loop_close)
deferred = deferred[:loop_start] + \
    '            /* Admitted lines have no OBJ overlay slot. */\n' \
    '            Lufia2PpuObjDeferSliver(col + x, vram_index, z, (oam1 & 0x4000) != 0, ws_clip_left, battle_effect_sprite);\n' + \
    deferred[loop_end:]
deferred = once(deferred, '  Lufia2BattleEffectsBeginSprites();',
    '  Lufia2BattleEffectsBeginSprites();\n  Lufia2PpuObjDeferBeginSprites();')
deferred = once(deferred, 'Lufia2BattleEffectsSpriteMargins(ppu, PpuRenderVram(ppu), line)',
    'Lufia2PpuObjDeferBattleMargins(ppu, PpuRenderVram(ppu), line)')

# The same evaluator once more for the helper core, reading a snapshot with
# the PPU's field names. Capture lines never reach the helper.
worker = once(deferred, 'static bool ppu_evaluateSprites_Defer(Ppu* ppu, int line) {',
    'bool Lufia2PpuObjEvalLine(Lufia2PpuObjEval *ppu, int line) {')
worker = once(worker, '  if (lufia2_ppu_obj_capture) Lufia2PpuObjCaptureLine(ppu, line);\n', '')
worker = once(worker, '  Lufia2BattleEffectsBeginSprites();\n  Lufia2PpuObjDeferBeginSprites();',
    '  Lufia2PpuObjEvalBeginSprites();')
worker = once(worker, 'Lufia2BattleEffectsSprite(ppu, index >> 1)', 'Lufia2PpuObjEvalEffect(ppu, index >> 1)')
capture_start = worker.index('            PpuOverlayCapture *obj_capture =')
capture_end = worker.index('            /* Admitted lines have no OBJ overlay slot. */')
worker = worker[:capture_start] + worker[capture_end:]
worker = once(worker, 'Lufia2PpuObjDeferSliver(', 'Lufia2PpuObjEvalSliver(')
worker = once(worker, 'snesrecomp_obj_range_row(&range_cache,', 'Lufia2PpuObjEvalRangeRow(&range_cache,')
worker = once(worker,
    '  bool battle_margin_sprites = Lufia2PpuObjDeferBattleMargins(ppu, PpuRenderVram(ppu), line);\n'
    '  if (lufia2_ppu_obj_capture) Lufia2PpuObjCaptureEndLine(ppu, line, battle_margin_sprites);\n',
    '  bool battle_margin_sprites = ppu->marginSprites;\n')
helpers = []
for name in ('PpuWidescreenHudOamSlot', 'PpuAdjustWidescreenHudOamX', 'PpuDecodeOamX',
             'PpuWidescreenOamLeftHintAllows'):
    worker = worker.replace(name + '(ppu', name + '_Eval(ppu')
    start = source.rindex('\nstatic ', 0, source.index(name + '(Ppu ')) + 1
    text = source[start:function_end(source, start)]
    text = once(text, name + '(Ppu *ppu', name + '_Eval(const Lufia2PpuObjEval *ppu')
    text = text.replace('PpuWidescreenHudOamSlot(ppu', 'PpuWidescreenHudOamSlot_Eval(ppu')
    text = text.replace('Lufia2BattleEffectsSprite(ppu, index >> 1)', 'Lufia2PpuObjEvalEffect(ppu, index >> 1)')
    helpers.append(text)
worker = '\n\n'.join(helpers) + '\n\n' + worker

deferred += '''

/* Admits a line to the helper, or ends deferral before serial rows. */
static void Lufia2PpuObjDeferLine(Ppu *ppu, int line) {
  const PpuOverlayCapture *capture = &ppu->overlayCaptures[kPpuOverlaySource_Obj];
  if (line >= 0 && !lufia2_ppu_obj_capture && !ppu->renderVram &&
      !(PpuOverlayActiveOnLine(ppu, kPpuOverlaySource_Obj, line) && capture->oamCount) &&
      Lufia2PpuObjDeferLineFits((unsigned)line)) {
    size_t begin, end;
    PpuActiveSpan(ppu, &begin, &end);
    Lufia2PpuObjDeferBeginLine((unsigned)line, (unsigned)begin, (unsigned)end, PpuRenderVram(ppu));
#if defined(LUFIA2_OBJ_DEFER_CHECK)
    ClearBackdrop(ppu, &ppu->objBuffer);
#endif
    return;
  }
  Lufia2PpuObjDeferStop();
  ClearBackdrop(ppu, &ppu->objBuffer);
}'''
source = source[:host_end] + '\n\n' + deferred + '\n\n' + worker + source[host_end:]
source = once(source,
    '  if (lufia2_ppu_host_costs_enabled)\n    return ppu_evaluateSprites_Host(ppu, line);\n',
    '  if (lufia2_ppu_host_costs_enabled && lufia2_ppu_obj_defer_ppu == ppu) {\n'
    '#if defined(LUFIA2_OBJ_DEFER_CHECK)\n'
    '    ppu_evaluateSprites_Defer(ppu, line);\n'
    '    return ppu_evaluateSprites_Host(ppu, line);\n'
    '#else\n'
    '    if (Lufia2PpuObjDeferQueueing()) {\n'
    '      Lufia2BattleEffectsBeginSprites();\n'
    '      Lufia2PpuObjDeferQueue(ppu, (unsigned)line,\n'
    '          Lufia2PpuObjDeferBattleMargins(ppu, PpuRenderVram(ppu), line));\n'
    '      return false;\n'
    '    }\n'
    '    return ppu_evaluateSprites_Defer(ppu, line);\n'
    '#endif\n'
    '  }\n'
    '  if (lufia2_ppu_host_costs_enabled)\n    return ppu_evaluateSprites_Host(ppu, line);\n')
source = once(source,
    '    // evaluate sprites\n    ClearBackdrop(ppu, &ppu->objBuffer);\n',
    '    // evaluate sprites\n'
    '    if (lufia2_ppu_obj_defer_ppu == ppu) Lufia2PpuObjDeferLine(ppu, line - 1);\n'
    '    else ClearBackdrop(ppu, &ppu->objBuffer);\n')
source = once(source, '    case 0x3e: {\n      uint8_t val = 0x1; // ppu1 version (4 bit)\n',
    '    case 0x3e: {\n      Lufia2PpuObjDeferSyncFlags(ppu);\n      uint8_t val = 0x1; // ppu1 version (4 bit)\n')
source = '#include "lufia2_ppu_obj_defer.h"\nstatic void Lufia2PpuObjDeferLine(Ppu *ppu, int line);\n' + source

# Isolated party cards consume only the authentic centre. Clip work AFTER
# window classification; changing viewport margins first would change colour
# windows inside that centre. Every surviving span retains its original bit.
window_type_end = source.index('} PpuWindows;') + len('} PpuWindows;')
source = source[:window_type_end] + r'''

static void Lufia2PpuWindowsNativeSource(PpuWindows *win,const Ppu *ppu) {
  if (!(ppu->renderFlags & LUFIA2_PPU_RENDER_NATIVE_SOURCE)) return;
  unsigned count=0;
  uint8_t bits=0;
  for (unsigned i=0;i<win->nr;++i) {
    const int left=IntMax(win->edges[i],0);
    const int right=IntMin(win->edges[i+1],256);
    if (left>=right) continue;
    win->edges[count]=(int16_t)left;
    win->edges[count+1]=(int16_t)right;
    bits|=(uint8_t)(((win->bits>>i)&1u)<<count);
    ++count;
  }
  if (count) {win->nr=(uint8_t)count;win->bits=bits;}
}
''' + source[window_type_end:]
source = once(source, '  win->bits = 0;\n}',
    '  win->bits = 0;\n  Lufia2PpuWindowsNativeSource(win,ppu);\n}')
source = once(source,
    '  debug_server_on_ppu_window(y, (int)layer, win->edges, win->nr, win->bits);\n}',
    '  debug_server_on_ppu_window(y, (int)layer, win->edges, win->nr, win->bits);\n'
    '  Lufia2PpuWindowsNativeSource(win,ppu);\n}')
source = once(source, '  static const uint8 kCwBitsMod[8] = {',
    '  Lufia2PpuWindowsNativeSource(&cwin,ppu);\n  static const uint8 kCwBitsMod[8] = {')
source = once(source,
    '    uint32 left = cwin.edges[windex] + kPpuExtraLeftRight, right = cwin.edges[windex + 1] + kPpuExtraLeftRight;\n',
    '    uint32 left = cwin.edges[windex] + kPpuExtraLeftRight, right = cwin.edges[windex + 1] + kPpuExtraLeftRight;\n'
    '    if (ppu->renderFlags & LUFIA2_PPU_RENDER_NATIVE_SOURCE)\n'
    '      dst = dst_org + left - kPpuExtraLeftRight + ppu->extraLeftRight;\n')

begin = source.index('    // Cache the brightness computation\n')
end = source.index('    // evaluate sprites\n', begin)
brightness = source[begin:end]
source = '#include "lufia2_ppu_offload.h"\n' + source
source += '\nvoid Lufia2PpuReplayPixelLine(Ppu *ppu, unsigned line) {\n' + brightness + '    PpuDrawWholeLine(ppu, line);\n}\n'
source += '\nvoid Lufia2PpuReplayMenuIndexLine(Ppu *ppu,unsigned line) {\n' + \
    '    ClearBackdrop(ppu,&ppu->bgBuffers[0]);\n' + \
    '    lufia2_background_armed_ppu=NULL;\n' + \
    '    PpuDrawBackgrounds_Uncached(ppu,line,false);\n}\n'
source += '''
void Lufia2PpuReplayBattleSourceLine(Ppu *ppu,unsigned line) {
    /* The private direct-card source has no OBJ/capture consumer. Keep the
     * qualified OBJ row and every scalar intact so its already-private scene
     * can be scoped to this BG-only draw without another full Ppu copy. */
    bool scoped=(ppu->renderFlags&LUFIA2_PPU_RENDER_NATIVE_SOURCE) &&
        lufia2_ppu_host_costs_enabled &&
        !((ppu->screenEnabled[0]|ppu->screenEnabled[1])&~6u) &&
        !ppu->wsMode2CaptureLayer && !ppu->mode7Hd.pixels && !ppu->widescreenLineEnhancer;
    for(unsigned i=0;scoped && i<kPpuOverlaySource_Count;++i)
        if(ppu->overlayRenderBuffer[i] || ppu->overlayCaptures[i].flags)scoped=false;
    if(!scoped){PpuDrawBattleMarginLine(ppu,line);return;}
    const bool sprites=ppu->lineHasSprites;
    ppu->lineHasSprites=false;
    PpuDrawWholeLine(ppu,line);
    ppu->lineHasSprites=sprites;
}
'''

source += '\nstatic void Lufia2PpuReplayBattleIndexLineFull(Ppu *ppu,unsigned line) {\n    Lufia2BattleEffectsBeginIndexLine();\n    ClearBackdrop(ppu,&ppu->bgBuffers[0]);\n    lufia2_background_armed_ppu=NULL;\n    PpuDrawBackgrounds(ppu,line,false);\n    Lufia2BattleEffectsEndIndexLine();\n}\n\n/* The HUD decision consumes only centre priority indices. Preserve the\n * complete viewport whenever a host mapping or capture can affect that centre.\n * Guest OBJ qualification already ran; no pixels are removed from GPU work. */\nvoid Lufia2PpuReplayBattleIndexLine(Ppu *ppu,unsigned line) {\n    bool simple=!((ppu->wsLayerMirror|ppu->wsLayerRepeat)&7u) && !ppu->wsHudSplitHeight;\n    for(unsigned bg=0;simple && bg<3;++bg) {\n        if(ppu->wsMarginGapL[bg] || ppu->wsMarginGapR[bg] ||\n            PpuWidescreenLayerRepeatBandActive(ppu,bg,line) ||\n            PpuWidescreenLayerStretchBandActive(ppu,bg,line) ||\n            PpuWidescreenLayerWorldBandAt(ppu,bg,line)>=0 ||\n            PpuWidescreenLayerElasticBandAt(ppu,bg,line)>=0) simple=false;\n        for(unsigned band=0;simple && band<kPpuWsAnchorBands;++band)\n            if(line>=ppu->wsAnchorY0[band][bg] && line<ppu->wsAnchorY1[band][bg])simple=false;\n    }\n    for(unsigned i=0;simple && i<kPpuOverlaySource_Count;++i)\n        if(ppu->overlayRenderBuffer[i] || ppu->overlayCaptures[i].flags)simple=false;\n    if(!simple){Lufia2PpuReplayBattleIndexLineFull(ppu,line);return;}\n    const unsigned left=ppu->extraLeftCur,right=ppu->extraRightCur;\n    ppu->extraLeftCur=ppu->extraRightCur=0;\n    Lufia2BattleEffectsBeginIndexLine();ClearBackdrop(ppu,&ppu->bgBuffers[0]);\n    lufia2_background_armed_ppu=NULL;PpuDrawBackgrounds(ppu,line,false);\n    Lufia2BattleEffectsEndIndexLine();\n    ppu->extraLeftCur=(uint16_t)left;ppu->extraRightCur=(uint16_t)right;\n}\n'
source += '\nvoid Lufia2PpuQualifyBattleSourceLine(Ppu *ppu, unsigned line) {\n' + \
    '    ClearBackdrop(ppu, &ppu->objBuffer);\n' + \
    '    ppu->lineHasSprites = false;\n' + \
    '    if ((ppu->screenEnabled[0] | ppu->screenEnabled[1]) & 16u)\n' + \
    '        ppu->lineHasSprites = ppu_evaluateSprites(ppu, line);\n}\n'
source += '\n#include "lufia2_ppu_shadow.inc"\n'
output = Path(args.output)
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(source)
