#pragma once

#include "lufia2_battle.h"
#include "lufia2_video_handoff.h"
#include "lufia2_video_policy.h"
#include "snes/ppu.h"

/* Discard the previous scene's pixels on a semantic Battle ownership change.
 * The previous layout is only a cache tag; it never identifies Battle. */
void Lufia2BattleWidescreenHandoff(
    Lufia2VideoHandoff *handoff,
    const Lufia2BattleState *battle,
    Lufia2VideoLayout previous_layout);

/* Pure presentation guard over callback ownership, loaded background, display
 * shadow and live brightness. Battle PPU layout varies during the fight. */
bool Lufia2BattleWidescreenMargin(
    const Lufia2BattleState *battle,
    const Ppu *ppu,
    uint8_t *background_id);
