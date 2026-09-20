#include "lufia2_battle.h"

Lufia2BattleState Lufia2BattleInspect(const uint8_t *wram) {
    Lufia2BattleState state = {false, false, 0};
    if (!wram)
        return state;
    state.layout_hint = wram[LUFIA2_WRAM_BATTLE_LAYOUT_HINT] != 0u;
    state.background_id = wram[LUFIA2_WRAM_BATTLE_BACKGROUND_LOADED];
    state.background_valid =
        state.background_id >= LUFIA2_BATTLE_BACKGROUND_MIN &&
        state.background_id <= LUFIA2_BATTLE_BACKGROUND_MAX;
    return state;
}
