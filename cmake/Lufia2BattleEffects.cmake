include_guard(GLOBAL)

# Patch the build-tree PPU copy.
function(lufia2_ppu_battle_effects source_var)
    set(_source "${${source_var}}")
    set(_anchor [=[  uint8_t padding = ppu->wsLayerMirror | ppu->wsLayerRepeat;]=])
    set(_hook [=[  /* Keep the battle HUD at native width. */
  if (layer == 1 && Lufia2BattleEffectsActive(ppu) &&
      PPU_bgTilemapAdr(ppu, 1) == 0x5c00 &&
      PPU_bgTileAdr(ppu, 1) == 0x4000 &&
      (ppu->extraLeftCur || ppu->extraRightCur)) {
    uint16_t left = ppu->extraLeftCur;
    uint16_t right = ppu->extraRightCur;
    ppu->extraLeftCur = ppu->extraRightCur = 0;
    PpuDrawBackground_4bpp_policy(ppu, dstbuf, y, sub, layer, zhi, zlo, mosaic);
    ppu->extraLeftCur = left;
    ppu->extraRightCur = right;
    return;
  }
  if (layer == 0 && Lufia2BattleEffectsActive(ppu)) {
    if (!IS_SCREEN_ENABLED(ppu, sub, layer))
      return;
    PpuPixelPrioBufs background;
    ClearBackdrop(ppu, &background);
    if (PPU_bigTiles(ppu, layer))
      PpuDrawBackgroundBig(ppu, &background, y, sub, layer, 4, zhi, zlo, mosaic);
    else if (mosaic)
      PpuDrawBackground_4bpp_mosaic(ppu, &background, y, sub, layer, zhi, zlo);
    else
      PpuDrawBackground_4bpp(ppu, &background, y, sub, layer, zhi, zlo);
    PpuWindows window;
    if (IS_SCREEN_WINDOWED(ppu, sub, layer))
      PpuWindows_Calc(&window, ppu, layer, y);
    else
      PpuWindows_Clear(&window, ppu, layer, y);
    for (unsigned band = 0; band < window.nr; ++band) {
      if (window.bits & (1u << band))
        continue;
      Lufia2BattleEffectsMargin(ppu, &background, y, sub,
          window.edges[band], window.edges[band + 1], zlo, mosaic);
    }
    for (int x = -ppu->extraLeftCur; x < 256 + ppu->extraRightCur; ++x) {
      unsigned at = x + kPpuExtraLeftRight;
      if (background.data[at] > dstbuf->data[at])
        dstbuf->data[at] = background.data[at];
    }
    return;
  }
]=])
    # Both policies share this anchor; patch only 4-bpp.
    string(FIND "${_source}" "static void PpuDrawBackground_4bpp_policy(" _start)
    string(FIND "${_source}" "static void PpuDrawBackground_2bpp_mosaic(" _end)
    math(EXPR _length "${_end} - ${_start}")
    if(_start LESS 0 OR _length LESS 1)
        message(FATAL_ERROR "Battle background PPU boundary changed")
    endif()
    string(SUBSTRING "${_source}" 0 ${_start} _prefix)
    string(SUBSTRING "${_source}" ${_start} ${_length} _body)
    string(SUBSTRING "${_source}" ${_end} -1 _suffix)
    _lufia2_count_literal(_count "${_body}" "${_anchor}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Battle background PPU seam changed")
    endif()
    string(REPLACE "${_anchor}" "${_hook}${_anchor}" _body "${_body}")
    set(_source "${_prefix}${_body}${_suffix}")
    string(FIND "${_source}" "static NOINLINE void PpuDrawWholeLine(Ppu *ppu, uint y) {" _start)
    string(FIND "${_source}" "static bool PpuHdWindowCondition(" _end)
    math(EXPR _length "${_end} - ${_start}")
    if(_start LESS 0 OR _length LESS 1)
        message(FATAL_ERROR "Battle colour PPU boundary changed")
    endif()
    string(SUBSTRING "${_source}" 0 ${_start} _prefix)
    string(SUBSTRING "${_source}" ${_start} ${_length} _body)
    string(SUBSTRING "${_source}" ${_end} -1 _suffix)
    foreach(_needle IN ITEMS
            "  PpuClearOverlayRenderLine(ppu, y);"
            "ppu->cgram[pixel & 0xff]"
            "ppu->cgram[ppu->bgBuffers[1].data[i] & 0xff]")
        _lufia2_count_literal(_count "${_body}" "${_needle}")
        if(_needle STREQUAL "ppu->cgram[pixel & 0xff]")
            set(_expected 2)
            set(_replacement "Lufia2BattleEffectsColour(ppu, pixel, i, false)")
        elseif(_needle STREQUAL "ppu->cgram[ppu->bgBuffers[1].data[i] & 0xff]")
            set(_expected 1)
            set(_replacement "Lufia2BattleEffectsColour(ppu, ppu->bgBuffers[1].data[i], i, true)")
        else()
            set(_expected 1)
            set(_replacement "${_needle}\n  Lufia2BattleEffectsBeginLine(ppu);")
        endif()
        if(NOT _count EQUAL _expected)
            message(FATAL_ERROR "Battle colour PPU seam changed: ${_needle}")
        endif()
        string(REPLACE "${_needle}" "${_replacement}" _body "${_body}")
    endforeach()
    set(_source "${_prefix}${_body}${_suffix}")
    set(_old "static inline uint8 PpuMosaicAt(Ppu *ppu, int i) {")
    _lufia2_count_literal(_count "${_source}" "${_old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Battle mosaic PPU seam changed")
    endif()
    string(REPLACE "${_old}" "static inline int PpuMosaicAt(Ppu *ppu, int i) {\n  if (Lufia2BattleEffectsActive(ppu))\n    return Lufia2BattleEffectsMosaic(i, PPU_mosaicSize(ppu));" _source "${_source}")
    set(_old "if ((ppu->bgBuffers[1].data[i] & 0xff) != 0)")
    _lufia2_count_literal(_count "${_source}" "${_old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Battle subscreen opacity seam changed")
    endif()
    string(REPLACE "${_old}" "if ((ppu->bgBuffers[1].data[i] & 0xff) != 0 ||\n                  Lufia2BattleEffectsOpaque(ppu->bgBuffers[1].data[i], i, true))" _source "${_source}")
    # Keep wrapped sprites outside the native viewport.
    set(_old "    int width = win.edges[windex + 1] - left;")
    set(_new [=[    int width = win.edges[windex + 1] - left;
    if (Lufia2BattleEffectsActive(ppu)) {
      int right = IntMin(left + width, 256);
      left = IntMax(left, 0);
      width = right - left;
      if (width <= 0)
        continue;
    }]=])
    _lufia2_count_literal(_count "${_source}" "${_old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Battle OBJ viewport seam changed")
    endif()
    string(REPLACE "${_old}" "${_new}" _source "${_source}")
    set(_old [=[  PpuWidescreenAdjustPinnedWindowEdges(win->edges[0], window_right, &w1l,
                                       &w1r, &w2l, &w2r);]=])
    set(_new [=[  bool battle_windows = Lufia2BattleEffectsWindows(
      ppu, layer, win->edges[0], window_right,
      &w1l, &w1r, &w2l, &w2r, &winflags);
  if (!battle_windows)
    PpuWidescreenAdjustPinnedWindowEdges(win->edges[0], window_right, &w1l,
                                         &w1r, &w2l, &w2r);]=])
    _lufia2_count_literal(_count "${_source}" "${_old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Battle colour window seam changed")
    endif()
    string(REPLACE "${_old}" "${_new}" _source "${_source}")
    # Battle windows must not expand twice.
    set(_old "  if (ppu->wsWindowExpandLayers & (1u << layer)) {")
    set(_new "  if (!battle_windows && (ppu->wsWindowExpandLayers & (1u << layer))) {")
    _lufia2_count_literal(_count "${_source}" "${_old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Battle window expansion seam changed")
    endif()
    string(REPLACE "${_old}" "${_new}" _source "${_source}")
    set(${source_var} "#include \"lufia2_battle_effects.h\"\n${_source}" PARENT_SCOPE)
endfunction()
