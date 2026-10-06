include_guard(GLOBAL)

# Keep every generated-PPU seam strict while sharing its replacement contract.
function(_lufia2_replace_battle_seam text_var old new contract)
    _lufia2_count_literal(_count "${${text_var}}" "${old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "${contract}")
    endif()
    string(REPLACE "${old}" "${new}" _patched "${${text_var}}")
    set(${text_var} "${_patched}" PARENT_SCOPE)
endfunction()

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
  if (layer == 0 && Lufia2BattleEffectsBackground(ppu)) {
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
    set(_old [=[static void PpuDrawBackground_2bpp_policy(Ppu *ppu, PpuPixelPrioBufs *dstbuf,
                                          uint y, bool sub,
                                          uint layer, PpuZbufType zhi,
                                          PpuZbufType zlo, bool mosaic) {]=])
    set(_new [=[static void PpuDrawBackground_2bpp_policy(Ppu *ppu, PpuPixelPrioBufs *dstbuf,
                                          uint y, bool sub,
                                          uint layer, PpuZbufType zhi,
                                          PpuZbufType zlo, bool mosaic) {
  /* Effect planes share one continuous tilemap. */
  if (Lufia2BattleEffectsPlane(ppu, layer) && ppu->wsBg3WidenY != 1) {
    uint8_t saved = ppu->wsBg3WidenY;
    ppu->wsBg3WidenY = 1;
    PpuDrawBackground_2bpp_policy(ppu, dstbuf, y, sub, layer, zhi, zlo, mosaic);
    ppu->wsBg3WidenY = saved;
    return;
  }]=])
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle effect plane PPU seam changed")
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
    # Only effect sprites may enter the margins.
    set(_old "    int width = win.edges[windex + 1] - left;")
    set(_new [=[    int width = win.edges[windex + 1] - left;
    if (Lufia2BattleEffectsActive(ppu)) {
      for (int x = left; x < left + width; ++x) {
        if (!Lufia2BattleEffectsSpriteVisible(ppu, x))
          continue;
        unsigned at = x + kPpuExtraLeftRight;
        if (clear_backdrop || ppu->objBuffer.data[at] > ppu->bgBuffers[sub].data[at])
          ppu->bgBuffers[sub].data[at] = ppu->objBuffer.data[at];
      }
      continue;
    }]=])
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle OBJ viewport seam changed")
    set(_old "static bool ppu_evaluateSprites(Ppu* ppu, int line) {")
    set(_new "${_old}\n  Lufia2BattleEffectsBeginSprites();")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle sprite evaluation seam changed")
    set(_old "x >= 256 && ppu->wsOamRightHintStrict")
    set(_new "${_old} && !Lufia2BattleEffectsSprite(ppu, index >> 1)")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle sprite coordinate seam changed")
    set(_old [=[      if (PpuWidescreenOamLeftHintAllows(ppu, index, x, spriteSize,
                                          left_extra)) {]=])
    set(_new [=[      if (Lufia2BattleEffectsSprite(ppu, index >> 1) ||
          PpuWidescreenOamLeftHintAllows(ppu, index, x, spriteSize, left_extra)) {]=])
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle sprite admission seam changed")
    set(_old "            int slot = index >> 1;")
    set(_new "${_old}\n            const bool battle_effect_sprite = Lufia2BattleEffectsSprite(ppu, slot);")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle sprite slot seam changed")
    set(_old "ws_edge_clip_on && ppu->wsOamLeftHintStrict &&")
    set(_new "ws_edge_clip_on && !battle_effect_sprite && ppu->wsOamLeftHintStrict &&")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle sprite edge seam changed")
    set(_old "                dst[0] = z + pixel;")
    set(_new "                Lufia2BattleEffectsSpritePixel(col + x + px, battle_effect_sprite);\n${_old}")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle sprite ownership seam changed")
    set(_old "*overlay = z + pixel;")
    _lufia2_count_literal(_count "${_source}" "${_old}")
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Menu sprite ownership seam changed")
    endif()
    string(REPLACE "${_old}" "${_old}\n                    Lufia2MenuUiSpritePixel(screen_x, slot);" _source "${_source}")
    set(_old [=[  PpuWidescreenAdjustPinnedWindowEdges(win->edges[0], window_right, &w1l,
                                       &w1r, &w2l, &w2r);]=])
    set(_new [=[  bool battle_windows = Lufia2BattleEffectsWindows(
      ppu, layer, win->edges[0], window_right,
      &w1l, &w1r, &w2l, &w2r, &winflags);
  if (!battle_windows)
    PpuWidescreenAdjustPinnedWindowEdges(win->edges[0], window_right, &w1l,
                                         &w1r, &w2l, &w2r);]=])
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle colour window seam changed")
    # Battle windows must not expand twice.
    set(_old "  if (ppu->wsWindowExpandLayers & (1u << layer)) {")
    set(_new "  if (!battle_windows && (ppu->wsWindowExpandLayers & (1u << layer))) {")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle window expansion seam changed")
    set(_old "  return tilesFound != 0;")
    set(_new "  bool battle_margin_sprites = Lufia2BattleEffectsSpriteMargins(ppu, PpuRenderVram(ppu), line);\n  return tilesFound != 0 || battle_margin_sprites;")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle actor margin seam changed")
    set(_old "static NOINLINE void PpuDrawWholeLine(Ppu *ppu, uint y) {")
    set(_new "static void PpuDrawBattleMarginLine(Ppu *ppu, unsigned y);\nstatic void PpuDrawMenuOverlayLine(Ppu *ppu, unsigned y);\n${_old}")
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle HUD renderer boundary changed")
    set(_old [=[  PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);
}]=])
    set(_new [=[  PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);
  Lufia2BattleEffectsHudLine(ppu, y, PpuDrawBattleMarginLine);
  Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);
}

static void PpuDrawBattleMarginLine(Ppu *ppu, unsigned y) {
  ClearBackdrop(ppu, &ppu->objBuffer);
  ppu->lineHasSprites = ppu_evaluateSprites(ppu, y);
  PpuDrawWholeLine(ppu, y);
}

static void PpuDrawMenuOverlayLine(Ppu *ppu, unsigned y) {
  ClearBackdrop(ppu, &ppu->objBuffer);
  ppu->lineHasSprites = ppu_evaluateSprites(ppu, y - 1);
  PpuDrawWholeLine(ppu, y);
}]=])
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Battle HUD side seam changed")
    set(_old [=[  if (!sub)
    PpuWriteOverlayRenderLine(ppu, source, y);]=])
    set(_new [=[  if (!sub) {
    PpuWriteOverlayRenderLine(ppu, source, y);
    Lufia2MenuUiPlane(ppu, layer, y);
  }]=])
    _lufia2_replace_battle_seam(_source "${_old}" "${_new}"
        "Menu main-plane seam changed")
    set(${source_var} "#include \"lufia2_battle_effects.h\"\n#include \"lufia2_menu_ui.h\"\n${_source}" PARENT_SCOPE)
endfunction()
