#ifndef LUFIA2_BATTLE_H
#define LUFIA2_BATTLE_H

#include <stdbool.h>
#include <stdint.h>

enum {
    LUFIA2_BATTLE_BACKGROUND_MIN = 0x00,
    LUFIA2_BATTLE_BACKGROUND_MAX = 0x18,
    LUFIA2_WRAM_BATTLE_LAYOUT_HINT = 0x09AA,
    LUFIA2_WRAM_BATTLE_BACKGROUND_LOADED = 0x11E1,
};

typedef struct Lufia2BattleState {
    bool layout_hint;
    bool background_valid;
    uint8_t background_id;
} Lufia2BattleState;

Lufia2BattleState Lufia2BattleInspect(const uint8_t *wram);

#endif
