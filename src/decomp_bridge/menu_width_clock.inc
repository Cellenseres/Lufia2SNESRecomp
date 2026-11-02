enum {
    MENU_WIDTH_CLOCK_ROOM = 24576u,
    MENU_WIDTH_RAW_TILE = 0x0578u,
    MENU_WIDTH_CHARACTER_LIMIT = 31u,
    MENU_WIDTH_CYCLE_LIMIT = 88u + MENU_WIDTH_CHARACTER_LIMIT * 84u + 81u + 6u,
    MENU_WIDTH_REFRESH_LIMIT = 20u
};

_Static_assert(MENU_WIDTH_CYCLE_LIMIT * 8u + MENU_WIDTH_REFRESH_LIMIT * 40u
    <= MENU_WIDTH_CLOCK_ROOM, "Menu width clock reservation");
_Static_assert((MENU_WIDTH_CYCLE_LIMIT * 8u + 1363u +
    MENU_WIDTH_REFRESH_LIMIT * 40u + 1363u) / 1364u <= MENU_WIDTH_REFRESH_LIMIT,
    "Menu width refresh reservation");

static const FixedSpanOpcode menu_width_setup[] = {
    {0x8866u, 1u, 3u, 1u},
    {0x8867u, 2u, 3u, 0u},
    {0x8869u, 2u, 3u, 0u},
    {0x886bu, 1u, 4u, 2u},
    {0x886cu, 3u, 4u, 1u},
    {0x886fu, 3u, 4u, 1u},
    {0x8872u, 3u, 6u, 2u},
    {0x888cu, 1u, 3u, 1u},
    {0x888du, 2u, 3u, 0u},
    {0x888fu, 2u, 3u, 0u},
    {0x8891u, 2u, 2u, 0u},
    {0x8893u, 1u, 3u, 1u},
    {0x8894u, 1u, 4u, 1u},
    {0x8895u, 2u, 4u, 2u},
    {0x8897u, 2u, 3u, 0u},
    {0x8899u, 1u, 2u, 0u},
    {0x889au, 1u, 2u, 0u},
    {0x889bu, 3u, 3u, 0u},
    {0x889eu, 3u, 5u, 2u},
    {0x88a1u, 3u, 5u, 2u},
    {0x88a4u, 1u, 2u, 0u},
    {0x88a5u, 2u, 3u, 0u},
    {0x88a7u, 3u, 3u, 0u},
    {0x88aau, 3u, 5u, 2u},
    {0x88adu, 2u, 6u, 4u},
};

static const FixedSpanOpcode menu_width_character[] = {
    {0x88afu, 1u, 2u, 0u},
    {0x88b0u, 2u, 2u, 0u},
    {0x88b2u, 2u, 2u, 0u},
    {0x88b4u, 3u, 6u, 2u},
    {0x88dau, 3u, 6u, 2u},
    {0x8db3u, 1u, 3u, 0u},
    {0x8db4u, 3u, 4u, 1u},
    {0x8db7u, 2u, 2u, 0u},
    {0x8db9u, 1u, 3u, 0u},
    {0x8dbau, 2u, 2u, 0u},
    {0x8dbcu, 2u, 2u, 0u},
    {0x8dbeu, 3u, 6u, 1u},
    {0x8dc1u, 3u, 4u, 1u},
    {0x8dc4u, 3u, 6u, 1u},
    {0x8dc7u, 1u, 2u, 0u},
    {0x8dc8u, 1u, 2u, 0u},
    {0x8dc9u, 1u, 6u, 2u},
    {0x88ddu, 3u, 6u, 2u},
    {0x88e0u, 2u, 3u, 0u},
    {0x88e8u, 1u, 6u, 2u},
    {0x88b7u, 2u, 3u, 0u},
    {0x88adu, 2u, 6u, 4u},
};

static const FixedSpanOpcode menu_width_end[] = {
    {0x88afu, 1u, 2u, 0u},
    {0x88b0u, 2u, 2u, 0u},
    {0x88b2u, 2u, 3u, 0u},
    {0x88b9u, 3u, 5u, 2u},
    {0x88bcu, 3u, 6u, 2u},
    {0x88c8u, 1u, 3u, 0u},
    {0x88c9u, 3u, 4u, 1u},
    {0x88ccu, 2u, 3u, 0u},
    {0x88d7u, 1u, 3u, 0u},
    {0x88d8u, 1u, 2u, 0u},
    {0x88d9u, 1u, 6u, 2u},
    {0x88bfu, 1u, 3u, 0u},
    {0x88c0u, 2u, 2u, 0u},
    {0x88c2u, 1u, 3u, 0u},
    {0x88c3u, 1u, 2u, 0u},
    {0x88c4u, 1u, 2u, 0u},
    {0x88c5u, 3u, 6u, 2u},
    {0x88e9u, 3u, 5u, 2u},
    {0x88ecu, 1u, 4u, 1u},
    {0x88edu, 1u, 6u, 2u},
    {0x8875u, 1u, 5u, 2u},
    {0x8876u, 1u, 4u, 1u},
};


static uint8_t MenuWidthRead(void *context, uint32_t address) {
    FixedSpanClock *clock = context;
    BeginFixedSpanOpcode(clock);
    const uint16_t source = clock->timing[clock->index].pc;
    const uint8_t value = FixedSpanRead(clock, address);
    if (source == 0x88adu && !clock->remaining) {
        assert(clock->index == clock->count);
        clock->timing = value ? menu_width_character : menu_width_end;
        clock->count = value
            ? sizeof menu_width_character / sizeof(FixedSpanOpcode)
            : sizeof menu_width_end / sizeof(FixedSpanOpcode);
        clock->index = 0u;
    }
    return value;
}

static bool MenuWidthTextSupported(const CpuState *cpu) {
    const unsigned width = (uint8_t)cpu->A;
    const unsigned bank = cpu->ram[0x5fu];
    if (!width || width > MENU_WIDTH_CHARACTER_LIMIT + 1u ||
        (bank != 0x7eu && bank != 0x7fu) ||
        cpu->Y < (bank == 0x7eu ? 0x8000u : 0x2000u) || cpu->Y > 0xefe0u ||
        cpu->X < 0x4040u || cpu->X > 0x7000u ||
        cpu->ram[MENU_WIDTH_RAW_TILE])
        return false;
    const uint8_t *source = cpu->ram + (bank - 0x7eu) * 65536u + cpu->Y;
    for (unsigned column = 0u; column < width; ++column) {
        const uint8_t character = source[column];
        if (!character)
            return true;
        if (character < 0x10u || character >= 0xccu)
            return false;
    }
    return false;
}

static bool MenuWidthClockSupported(const CpuState *cpu) {
    if (cpu->D || cpu->DB != 0x7eu || cpu->_flag_D ||
        cpu->host_return_valid > 3u ||
        (cpu->host_return_valid && cpu->host_return_valid != 3u) ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, 0x808866u)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - MENU_WIDTH_CLOCK_ROOM ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x808866u, 0x1ffcu, true,
            1363u, MENU_WIDTH_CLOCK_ROOM) ||
        g_snes->vPos < 225u || g_snes->vPos >= 262u ||
        (262u - g_snes->vPos) * 1364u - g_snes->hPos < MENU_WIDTH_CLOCK_ROOM ||
        !Lufia2DecompFrameRangeQuiet(0x808866u, 0x808878u) ||
        !Lufia2DecompFrameRangeQuiet(0x80888cu, 0x8088eeu) ||
        !Lufia2DecompFrameRangeQuiet(0x808db3u, 0x808dcau))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles) &&
        MenuWidthTextSupported(cpu);
}

static RecompReturn ActorBridgeTimedMenuWidth(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (!MenuWidthClockSupported(cpu))
        return ActorBridgeFallback(cpu, &frame, 0x808866u);
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x800000u;
    clock.timing = menu_width_setup;
    clock.count = sizeof menu_width_setup / sizeof(FixedSpanOpcode);
    const Lufia2Memory memory = {MenuWidthRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2MenuDrawStringWithWidth(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x808877u);
    assert(clock.timing == menu_width_end &&
        clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}


