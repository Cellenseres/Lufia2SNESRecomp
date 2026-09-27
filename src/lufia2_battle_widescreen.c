#include "lufia2_battle_widescreen.h"

void Lufia2BattleWidescreenHandoff(
    Lufia2VideoHandoff *handoff,
    const Lufia2BattleState *battle,
    Lufia2VideoLayout previous_layout) {
    if (battle && battle->active != (previous_layout == LUFIA2_VIDEO_BATTLE))
        Lufia2VideoHandoffReset(handoff);
}

/* These registers describe the composition supported by the static margin
 * assets. They never establish Battle identity. Check each picture, including
 * fades, resource setup and menus that temporarily change the composition. */
static bool PictureReady(const Ppu *ppu) {
    return ppu && !PPU_forcedBlank(ppu) && PPU_brightness(ppu) > 0u &&
        PPU_mode(ppu) == 1u &&
        ppu->screenEnabled[0] == 0x1fu &&
        ppu->screenEnabled[1] == 0u &&
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
