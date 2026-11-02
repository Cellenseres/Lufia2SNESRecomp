#include "lufia2/menu_rectangle.h"

static unsigned RectangleColumnSpanSize(CpuState *cpu, uint32_t entry) {
    if (entry != 0x8283d2u || cpu->PB != 0x82u || cpu->DB != 0x7eu ||
        cpu->m_flag || cpu->x_flag || cpu->D || cpu->_flag_D ||
        !cpu->ram || !cpu->Y || cpu->Y > 32u || (cpu->X & 1u) ||
        (unsigned)cpu->X + 2u * cpu->Y > 0x800u ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes ||
        g_snes->ram != cpu->ram || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true, 1363u, 1u))
        return 0u;
    const unsigned opcodes = Lufia2DecompFrameOwnedSpanBudget(
        entry, 0x8283ddu, ActorBridgeMenuRectangleStep);
    unsigned limit = opcodes / 6u;
    if (limit > cpu->Y)
        limit = cpu->Y;
    if (limit > LUFIA2_MENU_RECTANGLE_SPAN_MAX_COLUMNS)
        limit = LUFIA2_MENU_RECTANGLE_SPAN_MAX_COLUMNS;
    if (!limit)
        return 0u;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return 0u;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    uint64_t room = fence - g_snes->hPos - 1u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline <= cpu->master_cycles)
        return 0u;
    if (room > deadline - cpu->master_cycles)
        room = deadline - cpu->master_cycles;
    if (room > UINT64_MAX - cpu->master_cycles)
        room = UINT64_MAX - cpu->master_cycles;
    const unsigned column_clocks = 11u * cpu_region_speed(entry) + 80u;
    unsigned columns = 0u;
    for (unsigned count = 1u; count <= limit; ++count) {
        const unsigned raw = count * column_clocks - (count == cpu->Y ? 6u : 0u);
        const uint64_t refresh = charged ? (phase + raw) / 1364u * 40u
                                         : 40u * (raw / 1364u + 1u);
        if (raw + refresh > room)
            break;
        columns = count;
    }
    return columns;
}

static bool RunRectangleColumnSpan(CpuState *cpu, uint32_t entry) {
    const unsigned columns = RectangleColumnSpanSize(cpu, entry);
    if (!columns)
        return false;
    FixedSpanOpcode timing[6u * LUFIA2_MENU_RECTANGLE_SPAN_MAX_COLUMNS];
    for (unsigned column = 0u; column < columns; ++column) {
        FixedSpanOpcode *cell = timing + 6u * column;
        cell[0] = (FixedSpanOpcode){0x83d2u, 3u, 7u, 2u};
        cell[1] = (FixedSpanOpcode){0x83d5u, 3u, 7u, 2u};
        cell[2] = (FixedSpanOpcode){0x83d8u, 1u, 2u, 0u};
        cell[3] = (FixedSpanOpcode){0x83d9u, 1u, 2u, 0u};
        cell[4] = (FixedSpanOpcode){0x83dau, 1u, 2u, 0u};
        cell[5] = (FixedSpanOpcode){0x83dbu, 2u,
            (uint8_t)(column + 1u == cpu->Y ? 2u : 3u), 0u};
    }
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x820000u;
    clock.timing = timing;
    clock.count = 6u * columns;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuClearLayerRectangleColumns(&memory, &state, columns);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    /* Every WRAM write advances the bridge epoch. */
    Lufia2DecompFrameReturnCompleteSpan(clock.count, clock.count - 4u);
    interp_bridge_pre_opcode_redirect(result.pc);
    return true;
}
