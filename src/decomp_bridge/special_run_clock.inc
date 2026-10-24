typedef struct SpecialRunPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} SpecialRunPhase;

static const FixedSpanOpcode special_run_a689[] = {
    {0xa689u, 2u, 3u, 0u},
    {0xa68bu, 2u, 3u, 0u},
    {0xa68du, 2u, 2u, 0u},
    {0xa68fu, 2u, 3u, 1u},
    {0xa691u, 2u, 2u, 0u},
    {0xa693u, 3u, 4u, 1u},
    {0xa696u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a699[] = {
    {0xa699u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a69c[] = {
    {0xa69cu, 2u, 3u, 1u},
    {0xa69eu, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6a2[] = {
    {0xa6a2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6a5[] = {
    {0xa6a5u, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6a7[] = {
    {0xa6a7u, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6ab[] = {
    {0xa6abu, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6ae[] = {
    {0xa6aeu, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6b0[] = {
    {0xa6b0u, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6b4[] = {
    {0xa6b4u, 2u, 3u, 1u},
    {0xa6b6u, 2u, 5u, 2u},
    {0xa6b8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6bb[] = {
    {0xa6bbu, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6bd[] = {
    {0xa6bdu, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6c1[] = {
    {0xa6c1u, 2u, 2u, 0u},
    {0xa6c3u, 2u, 3u, 1u},
    {0xa6c5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6c8[] = {
    {0xa6c8u, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6ca[] = {
    {0xa6cau, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6ce[] = {
    {0xa6ceu, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6d1[] = {
    {0xa6d1u, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6d3[] = {
    {0xa6d3u, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6d7[] = {
    {0xa6d7u, 2u, 2u, 0u},
    {0xa6d9u, 2u, 3u, 1u},
    {0xa6dbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6de[] = {
    {0xa6deu, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6e0[] = {
    {0xa6e0u, 2u, 2u, 0u},
};

static const FixedSpanOpcode special_run_a6e4[] = {
    {0xa6e4u, 2u, 2u, 0u},
    {0xa6e6u, 2u, 3u, 1u},
    {0xa6e8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6eb[] = {
    {0xa6ebu, 2u, 3u, 0u},
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6ed[] = {
    {0xa6edu, 2u, 2u, 0u},
    {0xa6efu, 2u, 3u, 1u},
    {0xa6f1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6f4[] = {
    {0xa6f4u, 2u, 3u, 0u},
    {0xa6f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6f9[] = {
    {0xa6f9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode special_run_a6fc[] = {
    {0xa6fcu, 2u, 2u, 0u},
    {0xa6feu, 2u, 3u, 1u},
    {0xa700u, 2u, 2u, 0u},
    {0xa702u, 2u, 3u, 1u},
};

static const SpecialRunPhase special_run_phases[] = {
    {0xa689u, 0x0000u, 0x0000u, 1u, sizeof special_run_a689 / sizeof special_run_a689[0], special_run_a689},
    {0xa699u, 0x0000u, 0x0000u, 1u, sizeof special_run_a699 / sizeof special_run_a699[0], special_run_a699},
    {0xa69cu, 0xa6a0u, 0x0000u, 1u, sizeof special_run_a69c / sizeof special_run_a69c[0], special_run_a69c},
    {0xa6a2u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6a2 / sizeof special_run_a6a2[0], special_run_a6a2},
    {0xa6a5u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6a5 / sizeof special_run_a6a5[0], special_run_a6a5},
    {0xa6a7u, 0xa6a9u, 0x0000u, 1u, sizeof special_run_a6a7 / sizeof special_run_a6a7[0], special_run_a6a7},
    {0xa6abu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6ab / sizeof special_run_a6ab[0], special_run_a6ab},
    {0xa6aeu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6ae / sizeof special_run_a6ae[0], special_run_a6ae},
    {0xa6b0u, 0xa6b2u, 0x0000u, 1u, sizeof special_run_a6b0 / sizeof special_run_a6b0[0], special_run_a6b0},
    {0xa6b4u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6b4 / sizeof special_run_a6b4[0], special_run_a6b4},
    {0xa6bbu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6bb / sizeof special_run_a6bb[0], special_run_a6bb},
    {0xa6bdu, 0xa6bfu, 0x0000u, 1u, sizeof special_run_a6bd / sizeof special_run_a6bd[0], special_run_a6bd},
    {0xa6c1u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6c1 / sizeof special_run_a6c1[0], special_run_a6c1},
    {0xa6c8u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6c8 / sizeof special_run_a6c8[0], special_run_a6c8},
    {0xa6cau, 0xa6ccu, 0x0000u, 1u, sizeof special_run_a6ca / sizeof special_run_a6ca[0], special_run_a6ca},
    {0xa6ceu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6ce / sizeof special_run_a6ce[0], special_run_a6ce},
    {0xa6d1u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6d1 / sizeof special_run_a6d1[0], special_run_a6d1},
    {0xa6d3u, 0xa6d5u, 0x0000u, 1u, sizeof special_run_a6d3 / sizeof special_run_a6d3[0], special_run_a6d3},
    {0xa6d7u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6d7 / sizeof special_run_a6d7[0], special_run_a6d7},
    {0xa6deu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6de / sizeof special_run_a6de[0], special_run_a6de},
    {0xa6e0u, 0xa6e2u, 0x0000u, 1u, sizeof special_run_a6e0 / sizeof special_run_a6e0[0], special_run_a6e0},
    {0xa6e4u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6e4 / sizeof special_run_a6e4[0], special_run_a6e4},
    {0xa6ebu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6eb / sizeof special_run_a6eb[0], special_run_a6eb},
    {0xa6edu, 0x0000u, 0x0000u, 1u, sizeof special_run_a6ed / sizeof special_run_a6ed[0], special_run_a6ed},
    {0xa6f4u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6f4 / sizeof special_run_a6f4[0], special_run_a6f4},
    {0xa6f9u, 0x0000u, 0x0000u, 1u, sizeof special_run_a6f9 / sizeof special_run_a6f9[0], special_run_a6f9},
    {0xa6fcu, 0x0000u, 0xa704u, 1u, sizeof special_run_a6fc / sizeof special_run_a6fc[0], special_run_a6fc},
};

static const SpecialRunPhase *FindSpecialRunPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof special_run_phases / sizeof special_run_phases[0]; ++i)
        if (special_run_phases[i].pc == pc)
            return special_run_phases + i;
    return NULL;
}

static void AppendSpecialRunPhase(PaletteEntryClock *clock, uint16_t pc) {
    const SpecialRunPhase *phase = FindSpecialRunPhase(pc);
    assert(phase);
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        PaletteEntryOpcode(clock, op->pc, op->fetched, op->cycles, op->data_bytes);
    }
    if (phase->return_pc)
        clock->return_pc = 0x840000u | phase->return_pc;
    if (!phase->branch)
        return;
    static const uint16_t branches[] = {
        0xa6a0u, 0xa6a9u, 0xa6b2u, 0xa6bfu, 0xa6ccu, 0xa6d5u, 0xa6e2u
    };
    static const uint16_t targets[] = {
        0xa6a7u, 0xa6b0u, 0xa6bdu, 0xa6cau, 0xa6d3u, 0xa6e0u, 0xa6edu
    };
    for (unsigned i = 0u; i < sizeof branches / sizeof branches[0]; ++i) {
        if (phase->branch != branches[i])
            continue;
        const bool next = clock->call.cpu->ram[0x30u] != 0xf0u + i;
        PaletteEntryOpcode(clock, phase->branch, 2u, next ? 3u : 2u, 0u);
        AppendSpecialRunPhase(clock, next ? targets[i] : phase->branch + 2u);
        return;
    }
    assert(false);
}

static bool PrepareSpecialRun(PaletteEntryClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->return_pc = 0u;
    if ((pc >> 16) != 0x84u || cpu->PB != 0x84u || !cpu->ram ||
        !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x22705u || cpu->emulation || cpu->m_flag != 1u ||
        cpu->x_flag || cpu->D || cpu->_flag_D ||
        (cpu->DB != 0u && cpu->DB != 0x80u && cpu->DB != 0x7eu) ||
        cpu->S < 0x1f10u || cpu->S > 0x1ffcu ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || !FindSpecialRunPhase((uint16_t)pc))
        return false;
    AppendSpecialRunPhase(clock, (uint16_t)pc);
    return PaletteEntryWindow(clock);
}

static uint8_t SpecialRunChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    PaletteEntryClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareSpecialRun(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeSpecialRun(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    PaletteEntryClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if ((cpu->host_return_valid != 0u && cpu->host_return_valid != 3u) ||
        !PrepareSpecialRun(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, SpecialRunChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        BeginFixedSpanOpcode(&clock.span);
        assert(result.pc == 0x84a704u && clock.return_pc == result.pc &&
            clock.span.index == clock.span.count && !clock.span.remaining);
    }
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
        frame.entry_s, frame.hrv);
}
