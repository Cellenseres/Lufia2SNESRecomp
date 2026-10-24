enum { MENU_RECTANGLE_CLOCK_CAPACITY = 6500u };

typedef struct MenuRectangleClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[MENU_RECTANGLE_CLOCK_CAPACITY];
} MenuRectangleClock;

static void RectangleOpcode(MenuRectangleClock *clock, uint16_t pc,
                             unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < MENU_RECTANGLE_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool PrepareMenuRectangleClock(MenuRectangleClock *clock, CpuState *cpu) {
    const unsigned columns = cpu->X >> 8u;
    const unsigned rows = cpu->X & 0xffu;
    if (cpu->m_flag || cpu->x_flag || cpu->_flag_D || cpu->D ||
        cpu->host_return_valid != 2u || cpu->S > 0x1ffcu ||
        (cpu->A & 1u) || cpu->A >= 0x0800u || !columns || !rows ||
        (cpu->A & 63u) / 2u + columns > 32u || cpu->A / 64u + rows > 32u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x820000u;
    clock->span.timing = clock->timing;
    RectangleOpcode(clock, 0x83b5u, 2u, 4u, 2u);
    RectangleOpcode(clock, 0x83b7u, 2u, 4u, 2u);
    RectangleOpcode(clock, 0x83b9u, 2u, 4u, 2u);
    RectangleOpcode(clock, 0x83bbu, 1u, 2u, 0u);
    RectangleOpcode(clock, 0x83bcu, 2u, 3u, 0u);
    RectangleOpcode(clock, 0x83beu, 2u, 3u, 1u);
    RectangleOpcode(clock, 0x83c0u, 1u, 3u, 0u);
    RectangleOpcode(clock, 0x83c1u, 2u, 3u, 1u);
    RectangleOpcode(clock, 0x83c3u, 2u, 3u, 0u);
    RectangleOpcode(clock, 0x83c5u, 1u, 3u, 1u);
    RectangleOpcode(clock, 0x83c6u, 2u, 3u, 0u);
    RectangleOpcode(clock, 0x83c8u, 2u, 2u, 0u);
    RectangleOpcode(clock, 0x83cau, 1u, 3u, 1u);
    RectangleOpcode(clock, 0x83cbu, 1u, 4u, 1u);
    RectangleOpcode(clock, 0x83ccu, 2u, 3u, 0u);
    for (unsigned row = 0u; row < rows; ++row) {
        RectangleOpcode(clock, 0x83ceu, 2u, 4u, 2u);
        RectangleOpcode(clock, 0x83d0u, 2u, 4u, 2u);
        for (unsigned column = 0u; column < columns; ++column) {
            RectangleOpcode(clock, 0x83d2u, 3u, 7u, 2u);
            RectangleOpcode(clock, 0x83d5u, 3u, 7u, 2u);
            RectangleOpcode(clock, 0x83d8u, 1u, 2u, 0u);
            RectangleOpcode(clock, 0x83d9u, 1u, 2u, 0u);
            RectangleOpcode(clock, 0x83dau, 1u, 2u, 0u);
            RectangleOpcode(clock, 0x83dbu, 2u, column + 1u < columns ? 3u : 2u, 0u);
        }
        RectangleOpcode(clock, 0x83ddu, 2u, 4u, 2u);
        RectangleOpcode(clock, 0x83dfu, 1u, 2u, 0u);
        RectangleOpcode(clock, 0x83e0u, 3u, 3u, 0u);
        RectangleOpcode(clock, 0x83e3u, 2u, 4u, 2u);
        RectangleOpcode(clock, 0x83e5u, 2u, 7u, 4u);
        RectangleOpcode(clock, 0x83e7u, 2u, row + 1u < rows ? 3u : 2u, 0u);
    }
    RectangleOpcode(clock, 0x83e9u, 1u, 4u, 1u);
    unsigned budget = cpu_region_speed(0x8283eau) + 2u * 8u + 3u * 6u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x820000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x8283b5u, 0x1ffcu, true, 1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        if (!g_snes->hdmaBeamOff ||
            (225u - g_snes->vPos) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedMenuRectangle(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    MenuRectangleClock clock = {0};
    if (!PrepareMenuRectangleClock(&clock, cpu))
        return ActorBridgeFallback(cpu, &frame, 0x8283b5u);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2MenuClearLayerRectangle(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x8283eau &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
