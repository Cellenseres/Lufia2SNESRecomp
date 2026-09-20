#include "lufia2_battle_widescreen.h"

enum {
    RAW_INIDISP = 0,
    BATTLE_TILE_ADDRESS = 0x0142,
};

static bool Visible(const Ppu *ppu) {
    return ppu && !PPU_forcedBlank(ppu) && PPU_brightness(ppu) > 0u;
}

static bool CompletedFrameWasBlack(
    const uint8_t *rows, size_t stride, unsigned lines, bool valid) {
    if (!valid || !rows || stride <= RAW_INIDISP || !lines)
        return false;

    for (unsigned y = 0; y < lines; y++) {
        const uint8_t inidisp = rows[(size_t)y * stride + RAW_INIDISP];
        if (!(inidisp & 0x80u) && (inidisp & 0x0fu))
            return false;
    }
    return true;
}

static bool PictureReady(const Ppu *ppu) {
    return Visible(ppu) &&
        PPU_mode(ppu) == 1u &&
        ppu->screenEnabled[0] == 0x1fu &&
        ppu->screenEnabled[1] == 0u &&
        ppu->bgTileAdr == BATTLE_TILE_ADDRESS &&
        (ppu->bgXsc[0] & 3u) == 0u &&
        (ppu->bgXsc[1] & 3u) == 0u;
}

void Lufia2BattleWidescreenReset(Lufia2BattleWidescreen *battle_wide) {
    if (!battle_wide)
        return;
    battle_wide->phase = LUFIA2_BATTLE_WIDESCREEN_IDLE;
    battle_wide->background_id = 0xffu;
}

static void Activate(
    Lufia2BattleWidescreen *battle_wide,
    const Lufia2BattleState *battle) {
    battle_wide->phase = LUFIA2_BATTLE_WIDESCREEN_ACTIVE;
    if (battle->background_valid)
        battle_wide->background_id = battle->background_id;
}

void Lufia2BattleWidescreenObserve(
    Lufia2BattleWidescreen *battle_wide,
    const Lufia2BattleState *battle,
    const Ppu *ppu,
    const uint8_t *raster_rows,
    size_t raster_stride,
    unsigned raster_lines,
    bool raster_valid) {
    if (!battle_wide || !battle || !ppu)
        return;

    const bool visible = Visible(ppu);
    const bool previous_black = CompletedFrameWasBlack(
        raster_rows, raster_stride, raster_lines, raster_valid);

    switch (battle_wide->phase) {
    case LUFIA2_BATTLE_WIDESCREEN_IDLE:
        if (previous_black)
            battle_wide->phase = LUFIA2_BATTLE_WIDESCREEN_DARK;
        break;

    case LUFIA2_BATTLE_WIDESCREEN_DARK:
        if (!visible)
            break;
        if (battle->background_valid && PictureReady(ppu))
            Activate(battle_wide, battle);
        else
            Lufia2BattleWidescreenReset(battle_wide);
        break;

    case LUFIA2_BATTLE_WIDESCREEN_ACTIVE:
        if (!visible || previous_black) {
            battle_wide->phase = LUFIA2_BATTLE_WIDESCREEN_EXIT_DARK;
        } else if (battle->background_valid) {
            battle_wide->background_id = battle->background_id;
        }
        break;

    case LUFIA2_BATTLE_WIDESCREEN_EXIT_DARK:
        if (!visible)
            break;
        if (PictureReady(ppu))
            Activate(battle_wide, battle);
        else
            Lufia2BattleWidescreenReset(battle_wide);
        break;
    }
}

bool Lufia2BattleWidescreenMargin(
    const Lufia2BattleWidescreen *battle_wide,
    const Ppu *ppu,
    uint8_t *background_id) {
    if (!battle_wide || !background_id || !Visible(ppu) ||
        battle_wide->phase != LUFIA2_BATTLE_WIDESCREEN_ACTIVE ||
        battle_wide->background_id > LUFIA2_BATTLE_BACKGROUND_MAX) {
        return false;
    }
    *background_id = battle_wide->background_id;
    return true;
}
