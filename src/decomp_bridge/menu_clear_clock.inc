#include "lufia2/special_menu.h"

static const FixedSpanOpcode menu_clear_map_header[] = {
    {0xa4ddu, 1u, 3u, 1u},
    {0xa4deu, 2u, 3u, 0u},
    {0xa4e0u, 1u, 3u, 1u},
    {0xa4e1u, 2u, 2u, 0u},
    {0xa4e3u, 1u, 3u, 1u},
    {0xa4e4u, 1u, 4u, 1u},
    {0xa4e5u, 2u, 3u, 0u},
    {0xa4e7u, 3u, 3u, 0u},
};

static const FixedSpanOpcode menu_clear_map_row[] = {
    {0xa4eau, 3u, 7u, 2u},
    {0xa4edu, 3u, 7u, 2u},
    {0xa4f0u, 3u, 7u, 2u},
    {0xa4f3u, 3u, 7u, 2u},
    {0xa4f6u, 1u, 2u, 0u},
    {0xa4f7u, 1u, 2u, 0u},
    {0xa4f8u, 2u, 3u, 0u},
};

static const FixedSpanOpcode menu_clear_map_row_last[] = {
    {0xa4eau, 3u, 7u, 2u},
    {0xa4edu, 3u, 7u, 2u},
    {0xa4f0u, 3u, 7u, 2u},
    {0xa4f3u, 3u, 7u, 2u},
    {0xa4f6u, 1u, 2u, 0u},
    {0xa4f7u, 1u, 2u, 0u},
    {0xa4f8u, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_clear_map_finish[] = {
    {0xa4fau, 1u, 4u, 1u},
    {0xa4fbu, 1u, 4u, 1u},
};

static const FixedSpanOpcode menu_clear_sprite_header[] = {
    {0xa4fdu, 1u, 3u, 1u},
    {0xa4feu, 2u, 3u, 0u},
    {0xa500u, 3u, 3u, 0u},
    {0xa503u, 3u, 3u, 0u},
};

static const FixedSpanOpcode menu_clear_sprite_row[] = {
    {0xa506u, 3u, 3u, 0u},
    {0xa509u, 3u, 7u, 2u},
    {0xa50cu, 1u, 2u, 0u},
    {0xa50du, 1u, 2u, 0u},
    {0xa50eu, 3u, 3u, 0u},
    {0xa511u, 3u, 7u, 2u},
    {0xa514u, 1u, 2u, 0u},
    {0xa515u, 1u, 2u, 0u},
    {0xa516u, 1u, 2u, 0u},
    {0xa517u, 2u, 3u, 0u},
};

static const FixedSpanOpcode menu_clear_sprite_row_last[] = {
    {0xa506u, 3u, 3u, 0u},
    {0xa509u, 3u, 7u, 2u},
    {0xa50cu, 1u, 2u, 0u},
    {0xa50du, 1u, 2u, 0u},
    {0xa50eu, 3u, 3u, 0u},
    {0xa511u, 3u, 7u, 2u},
    {0xa514u, 1u, 2u, 0u},
    {0xa515u, 1u, 2u, 0u},
    {0xa516u, 1u, 2u, 0u},
    {0xa517u, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_clear_sprite_high_header[] = {
    {0xa519u, 3u, 3u, 0u},
    {0xa51cu, 3u, 3u, 0u},
};

static const FixedSpanOpcode menu_clear_sprite_high_word[] = {
    {0xa51fu, 3u, 7u, 2u},
    {0xa522u, 1u, 2u, 0u},
    {0xa523u, 1u, 2u, 0u},
    {0xa524u, 1u, 2u, 0u},
    {0xa525u, 2u, 3u, 0u},
};

static const FixedSpanOpcode menu_clear_sprite_high_word_last[] = {
    {0xa51fu, 3u, 7u, 2u},
    {0xa522u, 1u, 2u, 0u},
    {0xa523u, 1u, 2u, 0u},
    {0xa524u, 1u, 2u, 0u},
    {0xa525u, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_clear_sprite_finish[] = {
    {0xa527u, 1u, 4u, 1u},
};

typedef struct MenuClearPhase {
    uint16_t end;
    uint8_t count, fetch_bytes, data_bytes, internal_cycles;
    const FixedSpanOpcode *timing, *last_timing;
} MenuClearPhase;

static const MenuClearPhase menu_clear_phases[] = {
    {0xa4eau, 8u, 13u, 4u, 7u,
     menu_clear_map_header, NULL},
    {0xa4fau, 7u, 16u, 8u, 11u,
     menu_clear_map_row, menu_clear_map_row_last},
    {0xa4fcu, 2u, 2u, 2u, 4u,
     menu_clear_map_finish, NULL},
    {0xa506u, 4u, 9u, 1u, 2u,
     menu_clear_sprite_header, NULL},
    {0xa519u, 10u, 19u, 4u, 10u,
     menu_clear_sprite_row, menu_clear_sprite_row_last},
    {0xa51fu, 2u, 6u, 0u, 0u,
     menu_clear_sprite_high_header, NULL},
    {0xa527u, 5u, 8u, 2u, 6u,
     menu_clear_sprite_high_word, menu_clear_sprite_high_word_last},
    {0xa528u, 1u, 1u, 1u, 2u,
     menu_clear_sprite_finish, NULL},
};

static unsigned MenuClearPhaseIndex(uint32_t entry) {
    switch (entry) {
    case 0x84a4ddu: return 0u;
    case 0x84a4eau: return 1u;
    case 0x84a4fau: return 2u;
    case 0x84a4fdu: return 3u;
    case 0x84a506u: return 4u;
    case 0x84a519u: return 5u;
    case 0x84a51fu: return 6u;
    case 0x84a527u: return 7u;
    default: return 8u;
    }
}

static void ActorBridgeMenuClearStep(CpuState *cpu, uint32_t entry);

static bool MenuClearHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeMenuClearStep && MenuClearPhaseIndex(pc) < 8u;
}

static bool MenuClearContextSupported(const CpuState *cpu, unsigned phase) {
    if (phase >= 8u || cpu->D || cpu->_flag_D || cpu->S < 0x1f04u ||
        cpu->S > 0x1ffcu || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u))
        return false;
    if (phase == 0u || phase == 3u)
        return true;
    if (cpu->m_flag || cpu->x_flag)
        return false;
    if (phase == 1u)
        return cpu->DB == 0x7eu && cpu->X <= 0x07feu && !(cpu->X & 1u);
    if (phase == 2u)
        return cpu->DB == 0x7eu;
    if (cpu->DB != 0x7eu && (cpu->DB & 0x7fu) >= 0x40u)
        return false;
    if (phase == 4u)
        return cpu->X && cpu->X <= 128u && cpu->Y == (128u - cpu->X) * 4u;
    if (phase == 5u)
        return !cpu->X && cpu->Y == 512u;
    if (phase == 6u)
        return cpu->X && cpu->X <= 16u && cpu->Y == 512u + (16u - cpu->X) * 2u;
    return true;
}

static void ActorBridgeMenuClearStep(CpuState *cpu, uint32_t entry) {
    const unsigned index = MenuClearPhaseIndex(entry);
    if (!MenuClearContextSupported(cpu, index))
        return;
    const MenuClearPhase *phase = menu_clear_phases + index;
    const bool last = phase->last_timing &&
        (index == 1u ? !cpu->X : cpu->X == 1u);
    unsigned budget = phase->fetch_bytes * cpu_region_speed(entry) +
        phase->data_bytes * 8u + phase->internal_cycles * 6u - (last ? 6u : 0u);
    uint64_t refresh, charged;
    snes_refresh_state_get(&refresh, &charged);
    if (refresh >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    if (!charged || refresh + budget >= 1364u)
        budget += 40u;
    if (cpu->master_cycles > UINT64_MAX - budget || !g_snes)
        return;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, true,
            fence - budget - 1u, budget) ||
        !Lufia2DecompFrameRangeAllows(entry, 0x840000u | phase->end,
                                      MenuClearHookOwned))
        return;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x840000u;
    clock.timing = last ? phase->last_timing : phase->timing;
    clock.count = phase->count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SpecialMenuClearStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
