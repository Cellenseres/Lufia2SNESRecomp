#include "lufia2_battle_widescreen.h"

void Lufia2BattleWidescreenHandoff(
    Lufia2VideoHandoff *handoff,
    const Lufia2BattleState *battle,
    Lufia2VideoLayout previous_layout) {
    if (battle && battle->active != (previous_layout == LUFIA2_VIDEO_BATTLE))
        Lufia2VideoHandoffReset(handoff);
}

/* The Battle callback owns the scene. Require the live BG tile layout used by
 * the margin asset. Subscreen layers can change while that picture is visible:
 * a Battle frame has $212D=$15 with the same main layers and tile layout. */
static bool PictureReady(const Ppu *ppu) {
    return ppu && !PPU_forcedBlank(ppu) && PPU_brightness(ppu) > 0u &&
        PPU_mode(ppu) == 1u &&
        ppu->screenEnabled[0] == 0x1fu &&
        ppu->bgTileAdr == 0x0142u &&
        (ppu->bgXsc[0] & 3u) == 0u &&
        (ppu->bgXsc[1] & 3u) == 0u;
}

bool Lufia2BattleWidescreenMargin(
    const Lufia2BattleState *battle,
    const Ppu *ppu,
    uint8_t *background_id) {
    if (!battle || !battle->active || !battle->background_valid || !battle->display_ready ||
        !background_id || !PictureReady(ppu))
        return false;
    *background_id = battle->background_id;
    return true;
}
