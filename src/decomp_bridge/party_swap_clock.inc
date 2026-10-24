enum {
    PARTY_SWAP_OPCODE_COUNT = 84u,
    PARTY_SWAP_CLOCK_BUDGET = 4096u
};

typedef struct PartySwapClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[PARTY_SWAP_OPCODE_COUNT];
} PartySwapClock;

static bool PartySwapClockSupported(const CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D || cpu->_flag_D ||
        !(cpu->DB == 0x7eu || (cpu->DB & 0x7fu) < 0x40u) ||
        cpu->host_return_valid != 2u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - PARTY_SWAP_CLOCK_BUDGET)
        return false;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, 0x82b6efu, 0x1ffcu, false,
                                             1363u, PARTY_SWAP_CLOCK_BUDGET))
        return false;
    if (g_snes->vPos < 225u) {
        const unsigned until_vblank =
            (225u - g_snes->vPos) * 1364u - g_snes->hPos;
        if (!g_snes->hdmaBeamOff || until_vblank < PARTY_SWAP_CLOCK_BUDGET)
            return false;
    } else if (g_snes->vPos > 257u) {
        return false;
    }
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static void PreparePartySwapClock(PartySwapClock *clock, CpuState *cpu) {
    static const FixedSpanOpcode setup[] = {
        {0xb6efu, 2u, 3u, 0u}, {0xb6f1u, 3u, 4u, 1u},
        {0xb6f4u, 1u, 2u, 0u}, {0xb6f5u, 1u, 2u, 0u},
        {0xb6f6u, 3u, 4u, 1u}, {0xb6f9u, 1u, 2u, 0u},
        {0xb6fau, 1u, 2u, 0u}, {0xb6fbu, 2u, 3u, 0u},
        {0xb6fdu, 3u, 5u, 2u}, {0xb700u, 2u, 4u, 2u},
        {0xb702u, 3u, 5u, 2u}, {0xb705u, 3u, 6u, 2u},
        {0xb708u, 2u, 4u, 2u}, {0xb70au, 3u, 6u, 2u},
        {0xb70du, 2u, 3u, 0u}, {0xb70fu, 3u, 4u, 1u},
        {0xb712u, 3u, 4u, 1u}
    };
    static const uint16_t columns[] = {
        0x0a7bu, 0x1268u, 0x1298u, 0x1238u, 0x12c8u, 0x12f8u,
        0x1328u, 0x1358u, 0x1448u, 0x1478u, 0x1208u
    };
    const unsigned left = cpu->ram[0x14afu];
    const unsigned right = cpu->ram[0x14b0u];
    for (unsigned i = 0u; i < 17u; ++i)
        clock->timing[i] = setup[i];
    clock->timing[8].cycles += ((left << 1u) & 0xffu) >= 0x80u;
    clock->timing[10].cycles += ((right << 1u) & 0xffu) >= 0x80u;
    clock->timing[11].cycles += ((left << 1u) & 0xffu) >= 0x80u;
    clock->timing[13].cycles += ((right << 1u) & 0xffu) >= 0x80u;
    for (unsigned column = 0u; column < 11u; ++column) {
        const uint16_t pc = (uint16_t)(0xb715u + column * 14u);
        const unsigned index = 17u + column * 6u;
        const unsigned low = columns[column] & 0xffu;
        clock->timing[index] = (FixedSpanOpcode){pc, 3u,
            (uint8_t)(4u + (low + left >= 0x100u)), 1u};
        clock->timing[index + 1u] = (FixedSpanOpcode){pc + 3u, 1u, 3u, 0u};
        clock->timing[index + 2u] = (FixedSpanOpcode){pc + 4u, 3u,
            (uint8_t)(4u + (low + right >= 0x100u)), 1u};
        clock->timing[index + 3u] = (FixedSpanOpcode){pc + 7u, 3u,
            (uint8_t)(5u + (low + left >= 0x100u)), 1u};
        clock->timing[index + 4u] = (FixedSpanOpcode){pc + 10u, 1u, 3u, 0u};
        clock->timing[index + 5u] = (FixedSpanOpcode){pc + 11u, 3u,
            (uint8_t)(5u + (low + right >= 0x100u)), 1u};
    }
    clock->timing[83] = (FixedSpanOpcode){0xb7afu, 2u, 3u, 0u};
    clock->span.cpu = cpu;
    clock->span.bank = 0x820000u;
    clock->span.timing = clock->timing;
    clock->span.count = PARTY_SWAP_OPCODE_COUNT;
}

static RecompReturn ReturnTimedMenuCall(CpuState *cpu, const ActorBridgeFrame *frame, uint32_t source) {
    const uint16_t stack = cpu->S;
    bool progress = false;
    (void)Lufia2DecompFrameReturnReadByte(cpu, source, &progress);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const uint16_t low = Lufia2DecompFrameReturnReadByte(cpu, cpu->S, &progress);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const uint16_t high = Lufia2DecompFrameReturnReadByte(cpu, cpu->S, &progress);
    Lufia2DecompFrameReturnAccountOpcode(cpu, 6u,
        cpu_region_speed(source) + 2u * 8u + 3u * 6u, progress);
    const uint32_t target = ((uint32_t)cpu->PB << 16) |
        (uint16_t)((high << 8 | low) + 1u);
    return ActorBridgeResolveReturn(cpu, frame, 2u, source, stack, target);
}

static RecompReturn RunPartySwap(CpuState *cpu, const ActorBridgeFrame *frame) {
    PartySwapClock clock = {0};
    PreparePartySwapClock(&clock, cpu);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2MenuSwapPartyMembers(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x82b7b1u);
    assert(clock.span.index == PARTY_SWAP_OPCODE_COUNT && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, frame, result.pc);
}
