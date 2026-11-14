enum { FIXED_OAM_CLEAR_OPCODES = 838u, FIXED_OAM_CLOCK_CHUNK = 32u };

static FixedSpanOpcode FixedOamClearOpcode(unsigned index) {
    static const FixedSpanOpcode prefix[] = {
        {0xb5a3u, 2u, 3u, 0u}, {0xb5a5u, 3u, 3u, 0u},
        {0xb5a8u, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode high_table[] = {
        {0xb5abu, 3u, 7u, 2u}, {0xb5aeu, 1u, 2u, 0u},
        {0xb5afu, 1u, 2u, 0u}, {0xb5b0u, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode low_table[] = {
        {0xb5b8u, 3u, 7u, 2u}, {0xb5bbu, 1u, 2u, 0u},
        {0xb5bcu, 1u, 2u, 0u}, {0xb5bdu, 1u, 2u, 0u},
        {0xb5beu, 1u, 2u, 0u}, {0xb5bfu, 2u, 3u, 0u}
    };
    assert(index < FIXED_OAM_CLEAR_OPCODES);
    if (index < 3u)
        return prefix[index];
    if (index < 67u) {
        FixedSpanOpcode opcode = high_table[(index - 3u) % 4u];
        if (index == 66u)
            --opcode.cycles;
        return opcode;
    }
    if (index == 67u)
        return (FixedSpanOpcode){0xb5b2u, 3u, 3u, 0u};
    if (index == 68u)
        return (FixedSpanOpcode){0xb5b5u, 3u, 3u, 0u};
    if (index < 837u) {
        FixedSpanOpcode opcode = low_table[(index - 69u) % 6u];
        if (index == 836u)
            --opcode.cycles;
        return opcode;
    }
    return (FixedSpanOpcode){0xb5c1u, 2u, 3u, 0u};
}

_Static_assert(FIXED_OAM_CLEAR_OPCODES + 2u == LUFIA2_BATTLE_FIXED_OAM_OPCODE_BUDGET,
    "reserve the original JSR and RTS");

typedef struct FixedOamClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[FIXED_OAM_CLOCK_CHUNK];
    unsigned offset;
} FixedOamClock;

static void PrepareFixedOamClock(FixedOamClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count)
            return;
        if (clock->span.count)
            Lufia2DecompFramePairedAccountSpan(clock->span.cpu, 0x81b5a3u,
                clock->span.count, clock->span.progress_mask);
        clock->offset += clock->span.count;
        const unsigned remaining = FIXED_OAM_CLEAR_OPCODES - clock->offset;
        const unsigned count = remaining < FIXED_OAM_CLOCK_CHUNK
            ? remaining : FIXED_OAM_CLOCK_CHUNK;
        for (unsigned index = 0u; index < count; ++index)
            clock->timing[index] = FixedOamClearOpcode(clock->offset + index);
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x810000u,
            .timing = clock->timing, .count = count};
        if (!count)
            return;
    }
}

static uint8_t FixedOamRead(void *context, uint32_t address) {
    FixedOamClock *clock = context;
    PrepareFixedOamClock(clock);
    return FixedSpanRead(&clock->span, address);
}

static void FixedOamWrite(void *context, uint32_t address, uint8_t value) {
    FixedOamClock *clock = context;
    PrepareFixedOamClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static RecompReturn ActorBridgeTimedFixedOamClear(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0x81b5c9u};
    const BattleLeafCost cost = {2550u, 1281u, 290u};
    const unsigned reserve = BattleLeafClockRoom(0x81b5a3u, cost);
    if (rtl_apu_extended_frame_timing() || cpu != &g_cpu || !cpu->ram ||
        cpu->emulation || cpu->m_flag != 1u || cpu->x_flag || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x81u || cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        cpu->host_return_valid != 2u ||
        !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu) ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x81b5a3u) ||
        Lufia2DecompFramePairedReturnPc(cpu, 0x81b5a3u) != frame.host_return_pc24 ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) != 0xb5c8u ||
        !g_snes || !g_snes->apu || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0xd5c3u ||
        (g_snes->vPos < 225u && !g_snes->hdmaBeamOff) || g_snes->dma->hdmaPendingInit ||
        !BattleClockAllows(cpu, 0x81b5a3u, 0x81b5c4u, 2u, false, reserve, false, NULL))
        return ActorBridgeFallback(cpu, &frame, 0x81b5a3u);
    FixedOamClock clock = {.span = {.cpu = cpu}};
    const Lufia2Memory memory = {FixedOamRead, FixedOamWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleHideOam(&memory, &state);
    PrepareFixedOamClock(&clock);
    assert(clock.offset == FIXED_OAM_CLEAR_OPCODES && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81b5c3u);
    ActorBridgeStore(cpu, &state);
    bool progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(
        cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x81b5a3u, 1u, progress);
    return returned;
}
