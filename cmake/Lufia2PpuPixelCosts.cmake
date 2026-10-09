include_guard(GLOBAL)

# Consumer overlay; retain the immutable pinned core and original guest state.
function(lufia2_ppu_pixel_replace text_var old new)
    string(REPLACE "${old}" "" _removed "${${text_var}}")
    string(LENGTH "${${text_var}}" _before)
    string(LENGTH "${_removed}" _after)
    string(LENGTH "${old}" _length)
    math(EXPR _delta "${_before} - ${_after}")
    if(NOT _delta EQUAL _length)
        message(FATAL_ERROR "PPU pixel optimization anchor must match once: ${old}")
    endif()
    string(REPLACE "${old}" "${new}" _text "${${text_var}}")
    set(${text_var} "${_text}" PARENT_SCOPE)
endfunction()

function(lufia2_ppu_pixel_costs source_var)
    if(NOT LUFIA2_ENABLE_PPU_PIXEL_CACHE)
        return()
    endif()
    set(_text "${${source_var}}")
    string(FIND "${_text}" "Lufia2BattleEffectsBeginLine(ppu);" _art)
    if(NOT _art EQUAL -1)
        lufia2_ppu_pixel_replace(_text
            "  Lufia2BattleEffectsBeginLine(ppu);\n  if (PPU_forcedBlank(ppu)) {"
            "  Lufia2BattleEffectsBeginLine(ppu);\n  const bool l2_art = Lufia2BattleEffectsActive(ppu);\n  if (PPU_forcedBlank(ppu)) {")
        # BeginLine clears the art arrays; only an active BattleEffectsMargin
        # can fill them. Inactive lines therefore use the original CGRAM.
        lufia2_ppu_pixel_replace(_text
            "uint32 color = Lufia2BattleEffectsColour(ppu, pixel, i, false);"
            "uint32 color = l2_art ? Lufia2BattleEffectsColour(ppu, pixel, i, false) : ppu->cgram[pixel & 255u];")
        lufia2_ppu_pixel_replace(_text
            "uint32 color = Lufia2BattleEffectsColour(ppu, pixel, i, false), color2;"
            "uint32 color = l2_art ? Lufia2BattleEffectsColour(ppu, pixel, i, false) : ppu->cgram[pixel & 255u], color2;")
        lufia2_ppu_pixel_replace(_text
            "Lufia2BattleEffectsOpaque(ppu->bgBuffers[1].data[i], i, true))"
            "(l2_art && Lufia2BattleEffectsOpaque(ppu->bgBuffers[1].data[i], i, true)))")
        lufia2_ppu_pixel_replace(_text
            "color2 = Lufia2BattleEffectsColour(ppu, ppu->bgBuffers[1].data[i], i, true), color_map = half_color_map;"
            "color2 = l2_art ? Lufia2BattleEffectsColour(ppu, ppu->bgBuffers[1].data[i], i, true) : ppu->cgram[ppu->bgBuffers[1].data[i] & 255u], color_map = half_color_map;")
    else()
        lufia2_ppu_pixel_replace(_text
            "  uint32 windex = 0;\n  do {"
            "  const bool l2_art = false;\n  uint32 windex = 0;\n  do {")
    endif()
        # Only the no-math path uses the table. Compare the full palette per
        # scanline after HDMA/background callbacks, so line fades stay exact.
        lufia2_ppu_pixel_replace(_text
            "  uint32 windex = 0;\n  do {"
            "  static Lufia2PpuPaletteCache l2_palette_cache;\n  const uint32 *l2_rgb = NULL;\n  bool l2_rgb_ready = false;\n  uint32 windex = 0;\n  do {")
        lufia2_ppu_pixel_replace(_text
            "      // Math is disabled (or has no effect), so can avoid the per-pixel maths check\n      uint32 i = left;"
            [=[      // Math is disabled (or has no effect), so can avoid the per-pixel maths check
      if (!l2_art && clip_color_mask && !l2_rgb_ready) {
        l2_rgb = Lufia2PpuPaletteRgb(ppu, &l2_palette_cache);
        l2_rgb_ready = true;
      }
      uint32 i = left;]=])
    if(NOT _art EQUAL -1)
        lufia2_ppu_pixel_replace(_text
            "          uint32 color = l2_art ? Lufia2BattleEffectsColour(ppu, pixel, i, false) : ppu->cgram[pixel & 255u];"
            [=[          if (!l2_art) {
            dst[0] = clip_color_mask ? l2_rgb[pixel & 255u] : 0;
            continue;
          }
          uint32 color = Lufia2BattleEffectsColour(ppu, pixel, i, false);]=])
    else()
        lufia2_ppu_pixel_replace(_text
            [=[          uint32 color = ppu->cgram[pixel & 0xff];
          dst[0] = ppu->brightnessMult[color & clip_color_mask] << 16 |
            ppu->brightnessMult[(color >> 5) & clip_color_mask] << 8 |
            ppu->brightnessMult[(color >> 10) & clip_color_mask];]=]
            "          dst[0] = clip_color_mask ? l2_rgb[pixel & 255u] : 0;")
    endif()
    set(${source_var} "#include \"lufia2_ppu_palette.h\"\n${_text}" PARENT_SCOPE)
endfunction()
