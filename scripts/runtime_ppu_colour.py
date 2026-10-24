"""Retain the scalar PPU path; prepare fixed-colour work once per span."""


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise ValueError('PPU colour anchor mismatch: ' + old)
    return text.replace(old, new)


def transform(text):
    text = '#include "snesrecomp_platform/snes_ppu_colour_table.h"\n' + text
    text = replace_once(text, 'static void PpuDrawBattleMarginLine(Ppu *ppu, unsigned y);',
        '''static NOINLINE const SnesRecompPpuColourTable *Lufia2PpuFixedColourTable(
    Ppu *ppu, bool clip, uint16_t fixed) {
  static SnesRecompPpuColourTable tables[8];
  unsigned flags = clip | (PPU_subtractColor(ppu) << 1) | (PPU_halfColor(ppu) << 2);
  SnesRecompPpuColourTable *table = &tables[flags];
  snesrecomp_ppu_colour_table_prepare(table, ppu->cgram,
      ppu->brightnessMult, ppu->brightnessMultHalf, fixed,
      clip, PPU_subtractColor(ppu), PPU_halfColor(ppu));
  return table;
}
static void PpuDrawBattleMarginLine(Ppu *ppu, unsigned y);''')
    text = replace_once(text,
        '      if (!l2_art && clip_color_mask && !l2_rgb_ready) {',
        '      if ((!l2_art || lufia2_ppu_host_costs_enabled) && clip_color_mask && !l2_rgb_ready) {')
    text = replace_once(text,
        '''          if (!l2_art) {
            dst[0] = clip_color_mask ? l2_rgb[pixel & 255u] : 0;''',
        '''          if (!l2_art || (lufia2_ppu_host_costs_enabled && clip_color_mask &&
              ((pixel & 0x0fffu) != 0 || !Lufia2BattleEffectsOpaque(pixel, i, false)))) {
            dst[0] = clip_color_mask ? l2_rgb[pixel & 255u] : 0;''')
    text = replace_once(text,
        '      uint8 *half_color_map = PPU_halfColor(ppu) ? ppu->brightnessMultHalf : ppu->brightnessMult;',
        '''      const SnesRecompPpuColourTable *l2_fixed = NULL;
      if (lufia2_ppu_host_costs_enabled && !PPU_addSubscreen(ppu) && right - left >= 64)
        l2_fixed = Lufia2PpuFixedColourTable(ppu, !clip_color_mask, fixed_color);
      uint8 *half_color_map = PPU_halfColor(ppu) ? ppu->brightnessMultHalf : ppu->brightnessMult;''')
    text = replace_once(text,
        '          if (!l2_art && !(math_enabled_cur & (1u << main_layer))) {',
        '''          if (l2_fixed) {
            const bool math = (math_enabled_cur & (1u << main_layer)) != 0;
            if (!l2_art && !math && !clip_color_mask) {
              dst[0] = 0;
              continue;
            }
            if (!l2_art || (pixel & 0x0fffu) != 0 ||
                !Lufia2BattleEffectsOpaque(pixel, i, false)) {
              dst[0] = l2_fixed->rgb[math][pixel & 255u];
            } else {
              const uint16_t art = Lufia2BattleEffectsColour(ppu, pixel, i, false);
              dst[0] = snesrecomp_ppu_colour_table_rgb5(l2_fixed, art, math);
            }
            continue;
          }
          if (!l2_art && !(math_enabled_cur & (1u << main_layer))) {''')
    text = replace_once(text,
        '          if (!l2_art && !(math_enabled_cur & (1u << main_layer))) {',
        '''          if ((!l2_art || (lufia2_ppu_host_costs_enabled && clip_color_mask &&
              ((pixel & 0x0fffu) != 0 || !Lufia2BattleEffectsOpaque(pixel, i, false)))) &&
              !(math_enabled_cur & (1u << main_layer))) {''')
    for row in ('y', 'y - 1'):
        text = replace_once(text,
            f'  ppu->lineHasSprites = ppu_evaluateSprites(ppu, {row});\n  PpuDrawWholeLine(ppu, y);',
            f'''  /* These callbacks render host clones, after guest OBJ qualification.
   * An isolated BG view cannot consume OBJ pixels or publish guest flags. */
  ppu->lineHasSprites = false;
  if (!lufia2_ppu_host_costs_enabled ||
      ((ppu->screenEnabled[0] | ppu->screenEnabled[1]) & 16u) ||
      ppu->overlayRenderBuffer[kPpuOverlaySource_Obj])
    ppu->lineHasSprites = ppu_evaluateSprites(ppu, {row});
  PpuDrawWholeLine(ppu, y);''')
    return text
