"""Produce a consumer-only fast 4bpp renderer with an exact reference path."""
import argparse
import hashlib
import json
from pathlib import Path
from runtime_perf_overlay import definitions
from runtime_ppu_colour import transform as transform_colour
from runtime_ppu_retained import transform as transform_retained
from runtime_ppu_menu_colour import transform as transform_menu_colour


def replace(text, old, new, count=1):
    if text.count(old) != count:
        raise ValueError('PPU host anchor mismatch: ' + old)
    return text.replace(old, new)


def transform_elastic(text):
    opening = 'static void PpuMergeElasticBackground('
    end_anchor = '\nstatic void PpuMergeStretchedBackground('
    if text.count(opening) != 1 or text.count(end_anchor) != 1:
        raise ValueError('Exactly one original elastic renderer required')
    begin = text.index(opening)
    end = text.index(end_anchor, begin)
    original = text[begin:end]
    reference = replace(original, opening, opening.replace('(', '_Reference('))
    host = replace(original, opening, opening.replace('(', '_Host('))
    host = replace(host, '    for (int x = d0; x < d1; x++) {', '''    if (d0 >= d1) continue;
    SnesRecompPpuResampleStep sampling = snesrecomp_ppu_resample_begin(
        (unsigned)sw, (unsigned)dw, (unsigned)(d0 - dst0));
    for (int x = d0; x < d1; x++, snesrecomp_ppu_resample_next(&sampling)) {''')
    host = replace(host, 'int sx = src0 + ((2 * (x - dst0) + 1) * sw) / (2 * dw);',
        'int sx = src0 + (int)sampling.index;')
    wrapper = '''static void PpuMergeElasticBackground(Ppu *ppu, PpuPixelPrioBufs *dstbuf,
    const PpuPixelPrioBufs *layerbuf, const PpuWsElasticSeg *segs, int nseg) {
  if (lufia2_ppu_host_costs_enabled)
    PpuMergeElasticBackground_Host(ppu, dstbuf, layerbuf, segs, nseg);
  else
    PpuMergeElasticBackground_Reference(ppu, dstbuf, layerbuf, segs, nseg);
}
'''
    return text[:begin] + reference + '\n' + host + '\n' + wrapper + text[end:]


def unpack_background(fast):
    fast = replace(fast,
        'pixel = (bits >> i) & 1 | (bits >> (7 + i)) & 2 | (bits >> (14 + i)) & 4 | (bits >> (21 + i)) & 8;',
        'pixel = (bits >> (4 * (i))) & 15u;')
    fast = replace(fast,
        'pixel = (bits >> (7 - i)) & 1 | (bits >> (14 - i)) & 2 | (bits >> (21 - i)) & 4 | (bits >> (28 - i)) & 8;',
        'pixel = (bits >> (4 * (7 - (i)))) & 15u;')
    fast = replace(fast, '(bits & (0x01010101 << i))', 'pixel')
    fast = replace(fast, '(bits & (0x80808080 >> i))', 'pixel')
    fast = replace(fast, 'addr[0] | addr[8] << 16)',
        'Lufia2PpuDecode4bpp((uint32)addr[0] | (uint32)addr[8] << 16))')
    fast = replace(fast, 'bits >>= (x & 7)', 'bits >>= 4 * (x & 7)')
    fast = replace(fast, 'bits <<= (x & 7)', 'bits <<= 4 * (x & 7)')
    fast = replace(fast, 'bits >>= 1', 'bits >>= 4', 2)
    fast = replace(fast, 'bits <<= 1', 'bits <<= 4', 2)
    return fast


def record_sprites(original):
    original = replace(original, '  Lufia2BattleEffectsBeginSprites();',
        '  if (lufia2_ppu_obj_capture) Lufia2PpuObjCaptureLine(ppu, line);\n'
        '  Lufia2BattleEffectsBeginSprites();')
    original = replace(original, '            for (int px = px_left; px < px_right; px++, dst++) {', '''            if (lufia2_ppu_obj_capture) {
              int begin = px_left, end = px_right;
              if (ws_clip_left && begin < -(col + x)) begin = -(col + x);
              if (begin > end) begin = end;
              int overlay_begin = begin, overlay_end = begin;
              if (capture_slot) {
                overlay_begin = IntMax(begin, obj_capture->x0 - (col + x));
                overlay_end = IntMin(end, obj_capture->x1 - (col + x));
                if (overlay_begin > end) overlay_begin = end;
                if (overlay_end < overlay_begin) overlay_end = overlay_begin;
              }
              Lufia2PpuObjCaptureRun(ppu, line, col + x,
                  ((objAdr + usedTile * 16) & 0x7fff) / 16, row & 7,
                  paletteBase, (oam1 & 0x3000) >> 12, !!(oam1 & 0x4000), slot,
                  begin, end, overlay_begin, overlay_end,
                  (capture_slot ? SNESRECOMP_OBJ_RUN_OVERLAY : 0) |
                  (obj_remove ? SNESRECOMP_OBJ_RUN_REMOVE_OVERLAY : 0) |
                  (battle_effect_sprite ? SNESRECOMP_OBJ_RUN_EFFECT : 0),
                  Lufia2MenuUiSpriteOwner(slot));
            }
            for (int px = px_left; px < px_right; px++, dst++) {''')
    original = replace(original,
        '  return tilesFound != 0 || battle_margin_sprites;',
        '  if (lufia2_ppu_obj_capture) Lufia2PpuObjCaptureEndLine(ppu, line, battle_margin_sprites);\n'
        '  return tilesFound != 0 || battle_margin_sprites;')
    return original


def qualify_sprite_effects(fast):
    # The evaluator cannot execute guest code between selection and tile fetch.
    # Keep the selected OAM order and reuse its read-only effect classification.
    fast = replace(fast, '  uint8_t foundSprites[128];',
        '  uint8_t foundSprites[128];\n  bool foundEffects[128];')
    fast = replace(fast, '      if (Lufia2BattleEffectsSprite(ppu, index >> 1) ||',
        '      const bool effect = Lufia2BattleEffectsSprite(ppu, index >> 1);\n'
        '      if (effect ||')
    fast = replace(fast, '          foundSprites[spritesFound - 1] = index;',
        '          foundSprites[spritesFound - 1] = index;\n'
        '          foundEffects[spritesFound - 1] = effect;')
    fast = replace(fast, '            const bool battle_effect_sprite = Lufia2BattleEffectsSprite(ppu, slot);',
        '            const bool battle_effect_sprite = foundEffects[i - 1];')
    return fast


def transform_sprites(text):
    name = 'ppu_evaluateSprites'
    matches = [(match, end) for match, end in definitions(text) if match['name'] == name]
    if len(matches) != 1:
        raise ValueError('Exactly one original sprite evaluator required')
    match, end = matches[0]
    original = record_sprites(text[match.start():end])
    reference = replace(original, name + '(', name + '_Reference(')
    fast = replace(original, name + '(', name + '_Host(')
    fast = replace(fast, '  for(int i = 0; i < 128; i++) {', '''  static SnesRecompObjRangeCache range_cache;
  const uint32_t *range_row = snesrecomp_obj_range_row(&range_cache,
      ppu->oam, ppu->highOam, PPU_objSize(ppu), PPU_objInterlace(ppu),
      (unsigned)line);
  const unsigned range_start = index >> 1;
  unsigned range_begin = range_start;
  bool range_wrapped = false;
  for (;;) {
    unsigned range_slot = snesrecomp_obj_range_next(range_row, range_begin);
    if (range_slot == 128) {
      if (range_wrapped || !range_start) break;
      range_wrapped = true;
      range_begin = 0;
      continue;
    }
    if (range_wrapped && range_slot >= range_start) break;
    index = (uint8_t)(range_slot * 2u);
    range_begin = range_slot + 1u;''')
    fast = replace(fast, 'uint32 plane = addr[0] | addr[8] << 16;',
        'uint32 plane = Lufia2PpuDecode4bpp((uint32)addr[0] | (uint32)addr[8] << 16);')
    fast = replace(fast, 'uint32 bits = plane >> shift;', 'uint32 bits = plane >> (4 * shift);')
    fast = replace(fast, 'int pixel = (bits >> 0) & 1 | (bits >> 7) & 2 |\n                    (bits >> 14) & 4 | (bits >> 21) & 8;',
        'int pixel = bits & 15u;')
    fast = qualify_sprite_effects(fast)
    wrapper = '''static bool ppu_evaluateSprites(Ppu *ppu, int line) {
  if (lufia2_ppu_host_costs_enabled)
    return ppu_evaluateSprites_Host(ppu, line);
  return ppu_evaluateSprites_Reference(ppu, line);
}'''
    return text[:match.start()] + reference + '\n' + fast + '\n' + wrapper + text[end:]


def transform_overlays(text):
    text=replace(text,'static void PpuClearOverlayRenderLine(Ppu *ppu, uint y) {\n',
        'static void PpuClearOverlayRenderLine(Ppu *ppu, uint y) {\n'
        '  Lufia2MenuUiBeforePixelLine(ppu);\n'
        '  if (lufia2_ppu_host_costs_enabled && Lufia2MenuUiPrimitiveCaptureActive(ppu)) return;\n')
    name = 'PpuWriteOverlayRenderLine'
    matches = [(m, end) for m, end in definitions(text) if m['name'] == name]
    if len(matches) != 1:
        raise ValueError('Exactly one original overlay writer required')
    match, end = matches[0]
    original = text[match.start():end]
    reference = replace(original, name + '(', name + '_Reference(')
    host = replace(original, name + '(', name + '_Host(')
    host = replace(host, '''  for (int x = x0; x < x1; x++)
    dst[x + texture_extra] =
        PpuOverlayColor(ppu, src[x + kPpuExtraLeftRight]);''', '''  static Lufia2PpuPaletteCache palette_cache;
  const uint32 *rgb = Lufia2PpuPaletteRgb(ppu, &palette_cache);
  for (int x = x0; x < x1; x++) {
    unsigned index = src[x + kPpuExtraLeftRight] & 255u;
    dst[x + texture_extra] = index ? rgb[index] | 0xff000000u : 0;
  }''')
    wrapper = '''static void PpuWriteOverlayRenderLine(Ppu *ppu,
    PpuOverlaySource source, uint y) {
  if (lufia2_ppu_host_costs_enabled)
    if (Lufia2MenuUiPrimitiveCaptureActive(ppu)) return;
  if (lufia2_ppu_host_costs_enabled)
    PpuWriteOverlayRenderLine_Host(ppu, source, y);
  else
    PpuWriteOverlayRenderLine_Reference(ppu, source, y);
}
'''
    text = text[:match.start()] + reference + '\n' + host + '\n' + wrapper + text[end:]
    name = 'PpuFinishBackgroundOverlay'
    matches = [(m, end) for m, end in definitions(text) if m['name'] == name]
    if len(matches) != 1:
        raise ValueError('Exactly one original overlay merger required')
    match, end = matches[0]
    original = text[match.start():end]
    reference = replace(original, name + '(', name + '_Reference(')
    host = replace(original, name + '(', name + '_Host(')
    host = replace(host, '''  for (size_t i = begin; i < end; i++) {
    int x = (int)i - kPpuExtraLeftRight;
    if (remove && x >= capture->x0 && x < capture->x1)
      continue;
    if (src[i] > dst[i])
      dst[i] = src[i];
  }''', '''  size_t first_end = end, second_begin = end;
  if (remove && capture->x0 < capture->x1) {
    int left = capture->x0 + kPpuExtraLeftRight;
    int right = capture->x1 + kPpuExtraLeftRight;
    if (left < (int)begin) left = (int)begin;
    if (right > (int)end) right = (int)end;
    if (left < right) {
      first_end = (size_t)left;
      second_begin = (size_t)right;
    }
  }
  for (size_t i = begin; i < first_end; ++i)
    if (src[i] > dst[i]) dst[i] = src[i];
  for (size_t i = second_begin; i < end; ++i)
    if (src[i] > dst[i]) dst[i] = src[i];''')
    wrapper = '''static void PpuFinishBackgroundOverlay(Ppu *ppu, uint y,
    bool sub, uint layer, PpuPixelPrioBufs *layerbuf) {
  if (lufia2_ppu_host_costs_enabled)
    PpuFinishBackgroundOverlay_Host(ppu, y, sub, layer, layerbuf);
  else
    PpuFinishBackgroundOverlay_Reference(ppu, y, sub, layer, layerbuf);
}
'''
    output = '#include "lufia2_ppu_palette.h"\n' + text[:match.start()] + reference + '\n' + host + '\n' + wrapper + text[end:]
    # The reference's unsigned bitplanes must not shift a promoted signed int.
    return output.replace('addr[0] | addr[8] << 16',
        '(uint32)addr[0] | (uint32)addr[8] << 16').replace(
        '0x01010101 << i', '0x01010101u << i')


def transform_layer_cache(text):
    # The arming token is published only after the overlay buffer was cleared.
    # This avoids assuming that arbitrary callers supply a zero destination.
    text = '#include "lufia2_ppu_layer_cache.h"\n' + text
    text = replace(text, 'void ppu_reset(Ppu* ppu) {', '''static Lufia2PpuLayerCache lufia2_layer_cache;
static const Ppu *lufia2_layer_armed_ppu;
static PpuPixelPrioBufs *lufia2_layer_armed_buffer;

void ppu_reset(Ppu* ppu) {
  lufia2_layer_cache.memory_valid = lufia2_layer_cache.checked = false;
  lufia2_layer_armed_ppu = NULL;
  lufia2_layer_armed_buffer = NULL;''')
    text = replace(text, '  ppu->renderPitch = (uint)pitch;', '''  Lufia2PpuLayerCacheBegin(&lufia2_layer_cache);
  lufia2_layer_armed_ppu = NULL;
  lufia2_layer_armed_buffer = NULL;
  ppu->renderPitch = (uint)pitch;''')
    text = replace(text, 'void ppu_runLine(Ppu* ppu, int line) {', '''void ppu_runLine(Ppu* ppu, int line) {
  if (!line) Lufia2PpuLayerCacheBegin(&lufia2_layer_cache);''')
    text = replace(text, '''  return &ppu->overlayBuffers[source];
}''', '''  lufia2_layer_armed_ppu = ppu;
  lufia2_layer_armed_buffer = &ppu->overlayBuffers[source];
  return &ppu->overlayBuffers[source];
}''')
    helper = '''static bool Lufia2PpuLayerCacheEligible(Ppu *ppu,
    PpuPixelPrioBufs *dst, unsigned y, bool sub, unsigned layer, bool mosaic) {
  bool armed = lufia2_layer_armed_ppu == ppu &&
      lufia2_layer_armed_buffer == dst;
  lufia2_layer_armed_ppu = NULL;
  lufia2_layer_armed_buffer = NULL;
#if SNESRECOMP_TRACE
  /* Keep per-line debug window callbacks in their reference order. */
  return false;
#endif
  if (!armed || !lufia2_ppu_host_costs_enabled || sub || !y || y > 224 ||
      (layer != 0 && layer != 2) || PPU_mode(ppu) != 1 || mosaic ||
      PPU_bigTiles(ppu, layer) || ppu->renderVram ||
      ppu->widescreenLineEnhancer || ppu->mode7Hd.pixels ||
      ppu->wsMode2CaptureLayer || Lufia2BattleEffectsActive(ppu) ||
      WsShadowLayerActive(layer) || !(ppu->wsLayerClamp & (1u << layer)) ||
      ((ppu->wsLayerMirror | ppu->wsLayerRepeat) & (1u << layer)) ||
      ppu->wsMarginGapL[layer] || ppu->wsMarginGapR[layer] ||
      PpuWidescreenLayerRepeatBandActive(ppu, layer, y) ||
      PpuWidescreenLayerStretchBandActive(ppu, layer, y) ||
      PpuWidescreenLayerWorldBandAt(ppu, layer, y) >= 0 ||
      PpuWidescreenLayerElasticBandAt(ppu, layer, y) >= 0) return false;
  for (unsigned i = 0; i < kPpuWsAnchorBands; ++i)
    if (y >= ppu->wsAnchorY0[i][layer] && y < ppu->wsAnchorY1[i][layer])
      return false;
  return true;
}
'''
    name = 'PpuDrawBackground_4bpp_policy'
    matches = [(m, end) for m, end in definitions(text) if m['name'] == name]
    if len(matches) != 1:
        raise ValueError('Exactly one original 4bpp policy required')
    at = matches[0][0].start()
    text = text[:at] + helper + text[at:]
    for bits in [4, 2]:
        name = f'PpuDrawBackground_{bits}bpp_policy'
        matches = [(m, end) for m, end in definitions(text) if m['name'] == name]
        if len(matches) != 1:
            raise ValueError('Exactly one original background policy required')
        match, end = matches[0]
        original = text[match.start():end]
        # Recursion on rare battle policies remains inside the uncached core.
        core = original.replace(name + '(', name + '_Uncached(')
        wrapper = f'''static void {name}(Ppu *ppu, PpuPixelPrioBufs *dstbuf,
    uint y, bool sub, uint layer, PpuZbufType zhi, PpuZbufType zlo, bool mosaic) {{
  bool eligible = Lufia2PpuLayerCacheEligible(ppu, dstbuf, y, sub, layer, mosaic);
  if (eligible && Lufia2PpuLayerCacheRead(&lufia2_layer_cache, ppu, layer, y,
          zhi, zlo, g_snes_ppu_dbg_layer_mask, dstbuf)) return;
  {name}_Uncached(ppu, dstbuf, y, sub, layer, zhi, zlo, mosaic);
  if (eligible) Lufia2PpuLayerCacheStore(&lufia2_layer_cache, layer, y, dstbuf);
}}
'''
        text = text[:match.start()] + core + '\n' + wrapper + text[end:]
    return text


def transform_background_cache(text):
    text = '#include "lufia2_ppu_background_cache.h"\n' + text
    text = replace(text, 'static Lufia2PpuLayerCache lufia2_layer_cache;',
        'static Lufia2PpuBackgroundCache lufia2_background_cache;\nstatic const Ppu *lufia2_background_armed_ppu;\nstatic bool lufia2_background_eligible, lufia2_background_rgb_reused;\nstatic Lufia2PpuLayerCache lufia2_layer_cache;')
    text = replace(text, '  lufia2_layer_cache.memory_valid = lufia2_layer_cache.checked = false;',
        '  lufia2_background_cache.memory_valid = lufia2_background_cache.checked = false;\n  lufia2_background_armed_ppu = NULL;\n  lufia2_layer_cache.memory_valid = lufia2_layer_cache.checked = false;')
    text = replace(text, '  Lufia2PpuLayerCacheBegin(&lufia2_layer_cache);',
        '  Lufia2PpuBackgroundCacheBegin(&lufia2_background_cache);\n  Lufia2PpuLayerCacheBegin(&lufia2_layer_cache);')
    text = replace(text, '  if (!line) Lufia2PpuLayerCacheBegin(&lufia2_layer_cache);',
        '  if (!line) { Lufia2PpuBackgroundCacheBegin(&lufia2_background_cache); Lufia2PpuLayerCacheBegin(&lufia2_layer_cache); }')
    text = replace(text, 'static void PpuDrawBackgrounds(Ppu *ppu, int y, bool sub) {',
        'static void PpuDrawBackgrounds_Uncached(Ppu *ppu, int y, bool sub) {')
    wrapper = r"""static bool Lufia2PpuBackgroundCacheEligible(Ppu *ppu, int y, bool sub) {
  if (!lufia2_ppu_host_costs_enabled || lufia2_background_armed_ppu != ppu || sub ||
      y < 1 || y > 224 || PPU_mode(ppu) != 1 || ppu->renderVram ||
      ppu->widescreenLineEnhancer || ppu->mode7Hd.pixels || ppu->wsMode2CaptureLayer ||
      ppu->extraLeftCur != ppu->extraLeftRight ||
      ppu->extraRightCur != ppu->extraLeftRight ||
      Lufia2BattleEffectsActive(ppu)) return false;
#if SNESRECOMP_TRACE
  return false;
#endif
  for (unsigned bg = 0; bg < 4; ++bg) if (WsShadowLayerActive(bg)) return false;
  if ((ppu->wsLayerClamp & 5u) != 5u || ((ppu->wsLayerMirror | ppu->wsLayerRepeat) & 5u) ||
      ppu->wsHudSplitHeight) return false;
  for (unsigned bg = 0; bg <= 2; bg += 2) {
    if (ppu->wsMarginGapL[bg] || ppu->wsMarginGapR[bg] ||
        PpuWidescreenLayerRepeatBandActive(ppu, bg, y) ||
        PpuWidescreenLayerStretchBandActive(ppu, bg, y) ||
        PpuWidescreenLayerWorldBandAt(ppu, bg, y) >= 0 ||
        PpuWidescreenLayerElasticBandAt(ppu, bg, y) >= 0) return false;
    for (unsigned slot = 0; slot < kPpuWsAnchorBands; ++slot)
      if (y >= ppu->wsAnchorY0[slot][bg] && y < ppu->wsAnchorY1[slot][bg]) return false;
  }
  int elastic = PpuWidescreenLayerElasticBandAt(ppu, 1, y);
  if (elastic >= 0 && ppu->wsElasticNSeg[elastic][1] > kPpuWsElasticSegs) return false;

  for (unsigned bg = 0; bg < 4; ++bg) {
    if (bg == 0 || bg == 2) {
      const PpuOverlayCapture *c = ppu->overlayCaptures + bg;
      if (!PpuOverlayActiveOnLine(ppu, (PpuOverlaySource)bg, y - 1) ||
          c->x0 != 0 || c->x1 != 256 || c->y0 != 0 || c->y1 != 224 ||
          c->flags != kPpuOverlayFlag_RemoveFromGame) return false;
    } else if (PpuOverlayActiveOnLine(ppu, (PpuOverlaySource)bg, y - 1)) return false;
  }
  return true;
}
static void PpuDrawBackgrounds(Ppu *ppu, int y, bool sub) {
  bool eligible = Lufia2PpuBackgroundCacheEligible(ppu, y, sub);
  lufia2_background_eligible = eligible;
  lufia2_background_rgb_reused = false;
  lufia2_background_armed_ppu = NULL;
  if (eligible && Lufia2PpuBackgroundCacheRead(&lufia2_background_cache, ppu, y, g_snes_ppu_dbg_layer_mask)) {
    lufia2_layer_armed_ppu = NULL; lufia2_layer_armed_buffer = NULL;
    if (Lufia2MenuUiCachedIndexRow(ppu, y,
        lufia2_background_cache.planes[0][y-1]+kPpuExtraLeftRight,
        lufia2_background_cache.planes[1][y-1]+kPpuExtraLeftRight,
        lufia2_background_cache.background[y-1]+kPpuExtraLeftRight)) return;
    Lufia2PpuBackgroundCacheRestore(&lufia2_background_cache, ppu, y);
    /* Current palette and live menu ownership still run in original order.
     * Sprite evaluation and guest-visible status have already run normally. */
    lufia2_background_rgb_reused = Lufia2PpuBackgroundRgbRead(&lufia2_background_cache, ppu, y);
    if (lufia2_background_rgb_reused) Lufia2PpuBackgroundRgbRestorePlane(&lufia2_background_cache, ppu, y, 0, 0);
    else if (!Lufia2PpuBackgroundCaptureRead(&lufia2_background_cache, ppu, y, 0, 0)) {
      PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Bg1, y);
      Lufia2PpuBackgroundCaptureStore(&lufia2_background_cache, ppu, y, 0, 0);
    }
    Lufia2MenuUiPlane(ppu, 0, y);
    if (lufia2_background_rgb_reused) Lufia2PpuBackgroundRgbRestorePlane(&lufia2_background_cache, ppu, y, 1, 2);
    else if (!Lufia2PpuBackgroundCaptureRead(&lufia2_background_cache, ppu, y, 1, 2)) {
      PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Bg3, y);
      Lufia2PpuBackgroundCaptureStore(&lufia2_background_cache, ppu, y, 1, 2);
    }
    Lufia2MenuUiPlane(ppu, 2, y);
    return;
  }
  PpuDrawBackgrounds_Uncached(ppu, y, sub);
  if (eligible) {
    Lufia2PpuBackgroundCacheStore(&lufia2_background_cache, ppu, y);
    if (!Lufia2MenuUiPrimitiveCaptureActive(ppu)) {
      Lufia2PpuBackgroundCaptureStore(&lufia2_background_cache, ppu, y, 0, 0);
      Lufia2PpuBackgroundCaptureStore(&lufia2_background_cache, ppu, y, 1, 2);
    }
  }
}

"""
    anchor = 'static void PpuDrawBattleMarginLine(Ppu *ppu, unsigned y);'
    text = replace(text, anchor, wrapper + anchor)
    text = replace(text, '  ClearBackdrop(ppu, &ppu->bgBuffers[0]);\n\n  // Render main screen',
        '  ClearBackdrop(ppu, &ppu->bgBuffers[0]);\n  lufia2_background_armed_ppu = ppu;\n\n  // Render main screen')
    text = replace(text, '  PpuDrawBackgrounds(ppu, y, false);\n  if (ppu->widescreenLineEnhancer',
        '  PpuDrawBackgrounds(ppu, y, false);\n  if (Lufia2MenuUiPrimitiveCaptureActive(ppu)) {\n    Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);\n    return;\n  }\n  if (lufia2_background_rgb_reused) {\n    Lufia2PpuBackgroundRgbRestoreMain(&lufia2_background_cache, ppu, y);\n    Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);\n    return;\n  }\n  if (ppu->widescreenLineEnhancer')
    text = replace(text, '  PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);\n',
        '  PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);\n  if (lufia2_background_eligible) Lufia2PpuBackgroundRgbStore(&lufia2_background_cache, ppu, y);\n')
    return text


def transform_menu_background(text):
    anchor='  // A game may clamp the world at a room boundary while still asking the\n'
    text=replace(text,anchor,'''  /* The menu's isolated background pass replaces this complete RGB row.
   * Keep both screen indices, primitive capture, and guest OBJ qualification;
   * do not colour-convert pixels which Line will immediately overwrite. */
  if (lufia2_ppu_host_costs_enabled && Lufia2MenuUiBackgroundRedrawn(ppu, y)) {
    PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);
    Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);
    return;
  }

'''+anchor)
    text=transform_menu_colour(text)
    callback='Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);'
    if callback not in text:
        raise ValueError('Known complete menu callback changed')
    text=text.replace(callback,
        '(lufia2_ppu_host_costs_enabled ? Lufia2MenuUiReplayLine : Lufia2MenuUiLine)'
        '(ppu, y, PpuDrawMenuOverlayLine);')
    return text


def transform(text):
    name = 'PpuDrawBackground_4bpp'
    matches = [(match, end) for match, end in definitions(text) if match['name'] == name]
    if len(matches) != 1:
        raise ValueError('Exactly one original 4bpp renderer required')
    match, end = matches[0]
    original = text[match.start():end]
    fast = unpack_background(replace(original, name + '(', name + '_Host('))
    shadow = unpack_background(replace(original, name + '(', name + '_HostShadow('))
    macro = '#define VIEWPORT_ALLOWED(i) \\\n  PpuViewportAllows(ppu, layer, \\\n      (int)(dstz + (i) - dstbuf->data - kPpuExtraLeftRight), \\\n      (int)(dstz + (i) - dstbuf->data - kPpuExtraLeftRight) + \\\n          ws_bias[windex])'
    fast = replace(fast, macro, '#define VIEWPORT_ALLOWED(i) true')
    before = '''    uint x = win.edges[windex] + ppu->hScroll[layer] + ws_bias[windex];
    uint w = win.edges[windex + 1] - win.edges[windex];
    int ws_sx = win.edges[windex];
    PpuZbufType *dstz = dstbuf->data + win.edges[windex] + kPpuExtraLeftRight;'''
    after = '''    int left = win.edges[windex], right = win.edges[windex + 1];
    if (!Lufia2PpuClipViewport(ppu->widescreenLineEnhancer != NULL, layer,
            ppu->wsViewportInsetL[layer], ppu->wsViewportInsetR[layer],
            ws_bias[windex], &left, &right)) continue;
    uint x = left + ppu->hScroll[layer] + ws_bias[windex];
    uint w = right - left;
    int ws_sx = left;
    PpuZbufType *dstz = dstbuf->data + left + kPpuExtraLeftRight;'''
    fast = replace(fast, before, after)
    # Shadow lookup keys use the beginning of the original tile span. Moving
    # that beginning through a source inset could choose another key. Keep
    # the original span and clipping for this policy, with only row unpacking.
    opening = fast.index('{') + 1
    fast = fast[:opening] + '''
  if (ppu->widescreenLineEnhancer && WsShadowLayerActive(layer)) {
    PpuDrawBackground_4bpp_HostShadow(ppu, dstbuf, y, sub, layer, zhi, zlo);
    return;
  }
''' + fast[opening:]
    reference = replace(original, name + '(', name + '_Reference(')
    wrapper = '''static void PpuDrawBackground_4bpp(Ppu *ppu, PpuPixelPrioBufs *dstbuf,
    uint y, bool sub, uint layer, PpuZbufType zhi, PpuZbufType zlo) {
  if (lufia2_ppu_host_costs_enabled)
    PpuDrawBackground_4bpp_Host(ppu, dstbuf, y, sub, layer, zhi, zlo);
  else
    PpuDrawBackground_4bpp_Reference(ppu, dstbuf, y, sub, layer, zhi, zlo);
}'''
    output = '#include "lufia2_ppu_host.h"\n' + text[:match.start()] + reference + '\n' + shadow + '\n' + fast + '\n' + wrapper + text[end:]
    return transform_menu_background(transform_retained(transform_colour(transform_background_cache(transform_layer_cache(transform_overlays(transform_sprites(transform_elastic(output))))))))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    source = args.input.read_text()
    output = transform(source)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text() != output:
        args.output.write_text(output)
    args.output.with_suffix('.json').write_text(json.dumps({
        'input_sha256': hashlib.sha256(source.encode()).hexdigest(),
        'output_sha256': hashlib.sha256(output.encode()).hexdigest(),
        'original_4bpp_reference_retained': True,
        'original_elastic_reference_retained': True,
        'original_sprite_reference_retained': True,
        'original_overlay_reference_retained': True,
        'overlay_palette_checked_per_write': True,
        'removed_overlay_interval_skipped': True,
        'reference_bitplane_shifts_unsigned': True,
        'isolated_mode1_background_rows_cached': True,
        'qualified_mode1_background_composition_reused': True,
        'raw_menu_rgb_rows_keyed_by_live_palette_and_captured_obj': True,
        'discarded_primary_menu_rgb_conversion_skipped': True,
        'fixed_colour_composition_prepared_per_span': True,
        'bg_only_host_clones_skip_unused_sprite_selection': True,
        'retained_battle_rows_keep_guest_qualification_and_live_ui': True,
        'guest_and_owner_callbacks_remain_live': True,
        'layer_cache_excludes_palette_and_oam': True,
        'sprite_vertical_candidates_cached': True,
        'shadow_span_reference_mapping_retained': True,
        'exact_pixel_centre_incremental_sampling': True}, indent=2))


if __name__ == '__main__':
    main()
