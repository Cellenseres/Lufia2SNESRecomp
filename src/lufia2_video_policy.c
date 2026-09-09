#include "lufia2_video_policy.h"

enum {
    LUFIA2_INTRO_MAP = 0x02u,
    LUFIA2_INTRO_WAIT_A = 0x869752u,
    LUFIA2_INTRO_WAIT_B = 0x869754u,
    RAW_INIDISP = 0,
    RAW_BGMODE = 4,
    RAW_SETINI = 13,
    RAW_SCREEN_ENABLE = 58,
};

bool Lufia2IntroMode7Candidate(const Lufia2VideoObservation *observation) {
    if (!observation || observation->runtime_map != LUFIA2_INTRO_MAP)
        return false;
    return observation->resume_pc == LUFIA2_INTRO_WAIT_A ||
           observation->resume_pc == LUFIA2_INTRO_WAIT_B;
}

Lufia2IntroMode7RasterDetail Lufia2InspectIntroMode7Raster(
    const uint8_t *rows, size_t stride, unsigned line_count,
    bool history_valid) {
    bool saw_visible = false;
    bool saw_bg1 = false;
    Lufia2IntroMode7RasterDetail detail = {
        LUFIA2_INTRO_RASTER_UNKNOWN,
        LUFIA2_INTRO_REJECT_INVALID_HISTORY,
        0,
        0, 0, 0, 0, 0,
    };

    if (!history_valid || !rows || stride <= RAW_SCREEN_ENABLE + 1u ||
        !line_count)
        return detail;

    for (unsigned y = 0; y < line_count; y++) {
        const uint8_t *r = rows + (size_t)y * stride;

        /* Forced blank and brightness zero are both guaranteed black. They
         * may carry the outgoing scene's dormant register state. */
        if ((r[RAW_INIDISP] & 0x80u) || !(r[RAW_INIDISP] & 0x0fu))
            continue;
        saw_visible = true;
        saw_bg1 = saw_bg1 || (r[RAW_SCREEN_ENABLE] & 0x01u) != 0u;

        detail.line = y;
        detail.inidisp = r[RAW_INIDISP];
        detail.bgmode = r[RAW_BGMODE];
        detail.setini = r[RAW_SETINI];
        detail.main_enable = r[RAW_SCREEN_ENABLE];
        detail.sub_enable = r[RAW_SCREEN_ENABLE + 1u];
        if ((r[RAW_BGMODE] & 7u) != 7u)
            detail.reason = LUFIA2_INTRO_REJECT_MIXED_MODE;
        else if ((r[RAW_SCREEN_ENABLE] & ~0x11u) != 0u)
            detail.reason = LUFIA2_INTRO_REJECT_FOREIGN_MAIN_LAYER;
        else if ((r[RAW_SCREEN_ENABLE + 1u] & ~0x10u) != 0u)
            detail.reason = LUFIA2_INTRO_REJECT_SUBSCREEN;
        else if ((r[RAW_SETINI] & 0x40u) != 0u)
            detail.reason = LUFIA2_INTRO_REJECT_EXTBG;
        else
            continue;

        detail.classification = LUFIA2_INTRO_RASTER_REJECTED;
        return detail;
    }

    detail.reason = LUFIA2_INTRO_REJECT_NONE;
    if (!saw_visible) {
        detail.classification = LUFIA2_INTRO_RASTER_BLANK;
    } else if (saw_bg1) {
        detail.classification = LUFIA2_INTRO_RASTER_MODE7;
    } else {
        detail.classification = LUFIA2_INTRO_RASTER_REJECTED;
        detail.reason = LUFIA2_INTRO_REJECT_NO_BG1;
    }
    return detail;
}

Lufia2IntroMode7Raster Lufia2ClassifyIntroMode7Raster(
    const uint8_t *rows, size_t stride, unsigned line_count,
    bool history_valid) {
    return Lufia2InspectIntroMode7Raster(
        rows, stride, line_count, history_valid).classification;
}

const char *Lufia2IntroMode7RejectReasonName(
    Lufia2IntroMode7RejectReason reason) {
    switch (reason) {
    case LUFIA2_INTRO_REJECT_INVALID_HISTORY:
        return "invalid history";
    case LUFIA2_INTRO_REJECT_MIXED_MODE:
        return "mixed PPU mode";
    case LUFIA2_INTRO_REJECT_FOREIGN_MAIN_LAYER:
        return "foreign main-screen layer";
    case LUFIA2_INTRO_REJECT_SUBSCREEN:
        return "foreign subscreen layer";
    case LUFIA2_INTRO_REJECT_EXTBG:
        return "ExtBG enabled";
    case LUFIA2_INTRO_REJECT_NO_BG1:
        return "no visible BG1 line";
    case LUFIA2_INTRO_REJECT_NONE:
    default:
        return "none";
    }
}

Lufia2VideoLayout Lufia2SelectVideoLayoutObserved(
    const Ppu *ppu,
    bool widescreen_requested,
    const Lufia2VideoObservation *observation) {
    if (!widescreen_requested)
        return LUFIA2_VIDEO_NATIVE;

    /* The flyover programs Mode 7 through the scanline/HDMA walk, after this
       policy runs. Its map/wait pair identifies the sequence; only a black
       transition or a completed Mode 7 raster whose visible layers are a
       subset of BG1+OBJ may widen it. */
    if (Lufia2IntroMode7Candidate(observation)) {
        if (observation->intro_raster == LUFIA2_INTRO_RASTER_BLANK ||
            observation->intro_raster == LUFIA2_INTRO_RASTER_MODE7)
            return LUFIA2_VIDEO_INTRO_MODE7;
        return LUFIA2_VIDEO_CENTERED;
    }

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

Lufia2VideoLayout Lufia2SelectVideoLayout(
    const Ppu *ppu, bool widescreen_requested) {
    return Lufia2SelectVideoLayoutObserved(
        ppu, widescreen_requested, NULL);
}

const char *Lufia2VideoLayoutName(Lufia2VideoLayout layout) {
    switch (layout) {
    case LUFIA2_VIDEO_WORLD_MAP:
        return "Mode 7 world map";
    case LUFIA2_VIDEO_INTRO_MODE7:
        return "Intro Mode 7 flyover";
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
