#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lufia2_battle.h"
#include "snes/ppu.h"

typedef enum Lufia2BattleWidescreenPhase {
    LUFIA2_BATTLE_WIDESCREEN_IDLE = 0,
    LUFIA2_BATTLE_WIDESCREEN_DARK,
    LUFIA2_BATTLE_WIDESCREEN_ACTIVE,
    LUFIA2_BATTLE_WIDESCREEN_EXIT_DARK,
} Lufia2BattleWidescreenPhase;

typedef struct Lufia2BattleWidescreen {
    Lufia2BattleWidescreenPhase phase;
    uint8_t background_id;
} Lufia2BattleWidescreen;

void Lufia2BattleWidescreenReset(Lufia2BattleWidescreen *battle_wide);

void Lufia2BattleWidescreenObserve(
    Lufia2BattleWidescreen *battle_wide,
    const Lufia2BattleState *battle,
    const Ppu *ppu,
    const uint8_t *raster_rows,
    size_t raster_stride,
    unsigned raster_lines,
    bool raster_valid);

bool Lufia2BattleWidescreenMargin(
    const Lufia2BattleWidescreen *battle_wide,
    const Ppu *ppu,
    uint8_t *background_id);

