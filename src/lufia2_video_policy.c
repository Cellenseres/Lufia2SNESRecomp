#include "lufia2_video_policy.h"

Lufia2VideoLayout Lufia2SelectVideoLayout(
    const Ppu *ppu,
    bool widescreen_requested) {
    if (!widescreen_requested)
        return LUFIA2_VIDEO_NATIVE;
    if (!ppu)
        return LUFIA2_VIDEO_CENTERED;
    /* The guest blanks the screen for room and scene transitions. The
       registers in flight say nothing about the scene, so the caller
       holds its previous decision instead. */
    if (PPU_forcedBlank(ppu))
        return LUFIA2_VIDEO_BLANK;

    /* Save selection, name entry and the matching menu family put their
       repeating backdrop on BG2, windows on BG1 and text on BG3. */
    const bool pattern_menu =
        PPU_mode(ppu) == 1 &&
        ppu->screenEnabled[0] == 0x1f &&
        ppu->screenEnabled[1] == 0 &&
        ppu->bgTileAdr == 0x6644 &&
        (ppu->bgXsc[1] & 3) == 0 &&
        ppu->hScroll[1] == 0 && ppu->vScroll[1] == 0;
    if (pattern_menu)
        return LUFIA2_VIDEO_PATTERN_MENU;

    /* Lufia's overworld is the game's Mode 7 BG1+OBJ scene. Its 128x128 world
       map is large enough to provide real pixels on both sides. */
    const bool world_map =
        PPU_mode(ppu) == 7 &&
        ppu->screenEnabled[0] == 0x11 &&
        ppu->screenEnabled[1] == 0;
    if (world_map)
        return LUFIA2_VIDEO_WORLD_MAP;

    /* Regular maps use paired BG1/BG2 cameras over a streamed 32x32-tile VRAM
       ring. The game-specific map resolver supplies the wider margins from
       the complete processed map in WRAM. */
    const uint8_t main_layers = ppu->screenEnabled[0];
    const bool mode_1 = PPU_mode(ppu) == 1;
    const bool known_layers =
        main_layers == 0x1f && ppu->screenEnabled[1] == 0;
    const bool known_tiles =
        ppu->bgTileAdr == 0x1144 &&
        (ppu->bgXsc[0] & 3) == 0 && (ppu->bgXsc[1] & 3) == 0;
    const bool paired_camera =
        ppu->hScroll[0] == ppu->hScroll[1] &&
        ppu->vScroll[0] == ppu->vScroll[1];
    if (mode_1 && known_layers && known_tiles && paired_camera)
        return LUFIA2_VIDEO_REGULAR_MAP;

    return LUFIA2_VIDEO_CENTERED;
}

const char *Lufia2VideoLayoutName(Lufia2VideoLayout layout) {
    switch (layout) {
    case LUFIA2_VIDEO_WORLD_MAP:
        return "Mode 7 world map";
    case LUFIA2_VIDEO_REGULAR_MAP:
        return "regular map";
    case LUFIA2_VIDEO_MAP_LOADING:
        return "map loading (centered)";
    case LUFIA2_VIDEO_BLANK:
        return "forced blank";
    case LUFIA2_VIDEO_PATTERN_MENU:
        return "patterned menu";
    case LUFIA2_VIDEO_CENTERED:
        return "centered 4:3 fallback";
    case LUFIA2_VIDEO_NATIVE:
        return "native 4:3";
    case LUFIA2_VIDEO_LAYOUT_COUNT:
    default:
        return "unknown";
    }
}
