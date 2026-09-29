#include "lufia2_battle_widescreen.h"

void Lufia2BattleWidescreenHandoff(
    Lufia2VideoHandoff *handoff,
    const Lufia2BattleState *battle,
    Lufia2VideoLayout previous_layout) {
    if (battle && battle->active != (previous_layout == LUFIA2_VIDEO_BATTLE))
        Lufia2VideoHandoffReset(handoff);
}

/* The callback owns Battle throughout its visible effects. Its tile and layer
 * registers change within a fight, so they cannot gate the margin each frame.
 * $0583 and the live PPU still suppress setup, black transitions and fades. */
static bool DisplayVisible(const Ppu *ppu) {
    return ppu && !PPU_forcedBlank(ppu) && PPU_brightness(ppu) > 0u;
}

bool Lufia2BattleWidescreenMargin(
    const Lufia2BattleState *battle,
    const Ppu *ppu,
    uint8_t *background_id) {
    if (!battle || !battle->active || !battle->background_valid || !battle->display_ready ||
        !background_id || !DisplayVisible(ppu))
        return false;
    *background_id = battle->background_id;
    return true;
}
