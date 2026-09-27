#include "lufia2_battle.h"

Lufia2BattleState Lufia2BattleInspect(const uint8_t *wram) {
    Lufia2BattleState state = {LUFIA2_NMI_UNKNOWN, false, false, 0, false};
    if (!wram)
        return state;
    /* $80:80E4-$80:80EC builds JSL target; RTS at $67-$6B.
     * $80:864C only calls it while $6A (target bank) is nonzero.
     * Installers clear that bank before replacing the target. Validate the
     * whole stub so a disabled, torn or unknown callback cannot claim Battle. */
    const uint8_t *stub = wram + LUFIA2_WRAM_SCENE_NMI;
    if (stub[0] == 0x22u && stub[4] == 0x60u) {
        const uint32_t target = stub[1] | ((uint32_t)stub[2] << 8) |
                                ((uint32_t)stub[3] << 16);
        switch (target) {
        case 0x839fa9u: state.nmi_scene = LUFIA2_NMI_FIELD; break;
        case 0x858dc5u: state.nmi_scene = LUFIA2_NMI_BATTLE; break;
        case 0x86cef6u: state.nmi_scene = LUFIA2_NMI_WORLD_MAP; break;
        case 0x82939cu: state.nmi_scene = LUFIA2_NMI_MENU; break;
        case 0x8092a4u: state.nmi_scene = LUFIA2_NMI_INTRO; break;
        default: break;
        }
    }
    state.active = state.nmi_scene == LUFIA2_NMI_BATTLE;
    /* $81:81EE copies the encounter background; $81:B9C8 indexes its
     * resource table. This byte is reused in other scenes, not a scene flag. */
    state.background_id = wram[LUFIA2_WRAM_BATTLE_BACKGROUND_ID];
    state.background_valid =
        state.background_id >= LUFIA2_BATTLE_BACKGROUND_MIN &&
        state.background_id <= LUFIA2_BATTLE_BACKGROUND_MAX;
    /* Setup blanks this shadow before publishing the Battle callback. The
     * live PPU can still carry the outgoing picture until the next NMI. */
    const uint8_t display = wram[LUFIA2_WRAM_DISPLAY_SHADOW];
    state.display_ready = !(display & 0x80u) && (display & 0x0fu) != 0u;
    return state;
}
