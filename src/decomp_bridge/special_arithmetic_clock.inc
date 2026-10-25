static const FixedSpanOpcode special_arithmetic_opcodes[] = {
    {0xa3fcu, 1u, 3u, 1u},
    {0xa3fdu, 2u, 3u, 0u},
    {0xa3ffu, 3u, 5u, 2u},
    {0xa402u, 4u, 6u, 2u},
    {0xa406u, 2u, 3u, 0u},
    {0xa408u, 2u, 2u, 0u},
    {0xa40au, 4u, 5u, 1u},
    {0xa40eu, 1u, 2u, 0u},
    {0xa40fu, 1u, 2u, 0u},
    {0xa410u, 1u, 2u, 0u},
    {0xa411u, 1u, 2u, 0u},
    {0xa412u, 1u, 2u, 0u},
    {0xa413u, 1u, 2u, 0u},
    {0xa414u, 1u, 2u, 0u},
    {0xa415u, 1u, 2u, 0u},
    {0xa416u, 2u, 3u, 0u},
    {0xa418u, 4u, 6u, 2u},
    {0xa41cu, 3u, 5u, 2u},
    {0xa41fu, 1u, 4u, 1u},
    {0xae43u, 4u, 6u, 2u},
    {0xae47u, 2u, 3u, 0u},
    {0xae49u, 2u, 2u, 0u},
    {0xae4bu, 4u, 5u, 1u},
    {0xae4fu, 1u, 2u, 0u},
    {0xae50u, 1u, 2u, 0u},
    {0xae51u, 1u, 2u, 0u},
    {0xae52u, 1u, 2u, 0u},
    {0xae53u, 1u, 2u, 0u},
    {0xae54u, 1u, 2u, 0u},
    {0xae55u, 1u, 2u, 0u},
    {0xae56u, 1u, 2u, 0u},
    {0xae57u, 2u, 3u, 0u},
    {0xae59u, 4u, 6u, 2u},
    {0xc31bu, 4u, 6u, 2u},
    {0xc31fu, 2u, 3u, 0u},
    {0xc321u, 2u, 2u, 0u},
    {0xc323u, 4u, 5u, 1u},
    {0xc327u, 1u, 2u, 0u},
    {0xc328u, 1u, 2u, 0u},
    {0xc329u, 1u, 2u, 0u},
    {0xc32au, 1u, 2u, 0u},
    {0xc32bu, 1u, 2u, 0u},
    {0xc32cu, 1u, 2u, 0u},
    {0xc32du, 1u, 2u, 0u},
    {0xc32eu, 1u, 2u, 0u},
    {0xc32fu, 2u, 3u, 0u},
    {0xc331u, 4u, 6u, 2u},
    {0xc335u, 2u, 4u, 2u},
    {0xc337u, 2u, 3u, 0u},
    {0xc339u, 4u, 5u, 1u},
    {0xc33du, 2u, 2u, 0u},
    {0xc33fu, 4u, 5u, 1u},
    {0xc343u, 1u, 2u, 0u},
    {0xc344u, 1u, 2u, 0u},
    {0xc345u, 1u, 2u, 0u},
    {0xc346u, 1u, 2u, 0u},
    {0xc347u, 2u, 3u, 0u},
    {0xc349u, 4u, 6u, 2u},
    {0xc34du, 2u, 4u, 2u},
    {0xceb3u, 2u, 4u, 2u},
    {0xceb5u, 3u, 3u, 0u},
    {0xceb8u, 2u, 2u, 0u},
    {0xcebau, 3u, 3u, 0u},
    {0xcebdu, 2u, 7u, 4u},
    {0xcebfu, 1u, 3u, 0u},
    {0xcec0u, 2u, 4u, 2u},
    {0xcec2u, 3u, 5u, 2u},
    {0xcec5u, 2u, 4u, 2u},
    {0xcec7u, 1u, 2u, 0u},
    {0xcec8u, 1u, 3u, 1u},
    {0xcec9u, 2u, 3u, 0u},
    {0xcecbu, 2u, 2u, 0u},
    {0xcecdu, 1u, 3u, 1u},
    {0xceceu, 1u, 4u, 1u},
    {0xcecfu, 2u, 3u, 0u},
    {0xced1u, 1u, 2u, 0u},
    {0xced2u, 3u, 5u, 2u},
    {0xced5u, 1u, 2u, 0u},
    {0xced6u, 1u, 2u, 0u},
    {0xced7u, 1u, 2u, 0u},
    {0xced8u, 1u, 2u, 0u},
    {0xced9u, 3u, 5u, 2u},
    {0xcedcu, 1u, 4u, 1u},
    {0xceddu, 1u, 2u, 0u},
    {0xcedeu, 1u, 2u, 0u},
    {0xcedfu, 1u, 3u, 0u},
    {0xcee0u, 3u, 3u, 0u},
};

static const unsigned special_arithmetic_ranges[][2] = {
    {0u, 19u},
    {19u, 33u},
    {33u, 59u},
    {59u, 87u},
};

typedef struct SpecialArithmeticClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[40];
} SpecialArithmeticClock;

static bool PrepareSpecialArithmeticClock(SpecialArithmeticClock *clock,
    CpuState *cpu, uint32_t entry) {
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x28000u ||
        cpu->PB != 0x84u || cpu->emulation || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        cpu->host_return_valid < 2u || cpu->host_return_valid > 3u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned root;
    switch (entry) {
    case 0x84a3fcu: root = 0u; break;
    case 0x84ae43u: root = 1u; break;
    case 0x84c31bu: root = 2u; break;
    case 0x84ceb3u: root = 3u; break;
    default: return false;
    }
    if (root && cpu->m_flag) return false;
    if (root >= 2u && cpu->D) return false;
    if (root == 3u && cpu->x_flag) return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    for (unsigned i = special_arithmetic_ranges[root][0]; i < special_arithmetic_ranges[root][1]; ++i) {
        FixedSpanOpcode op = special_arithmetic_opcodes[i];
        if (root == 3u && !(cpu->A & 0x80u)) {
            if (op.pc == 0xceb8u) ++op.cycles;
            if (op.pc == 0xcebau || op.pc == 0xcebdu) continue;
        }
        assert(clock->span.count < sizeof clock->timing / sizeof clock->timing[0]);
        clock->timing[clock->span.count++] = op;
    }
    unsigned budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffdu, true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeSpecialArithmetic(CpuState *cpu, uint32_t entry) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    SpecialArithmeticClock clock = {0};
    if (!PrepareSpecialArithmeticClock(&clock, cpu, entry)) return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span,
        SpecialArithmeticWait, &clock.span};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (entry) {
    case 0x84a3fcu: result = Lufia2SpecialMenuDividePayoutStep(&memory, &state); break;
    case 0x84ae43u: result = Lufia2SpecialMenuDivideScore(&memory, &state); break;
    case 0x84c31bu: result = Lufia2CardGameSplitStakeTens(&memory, &state); break;
    default: result = Lufia2BoardGameScaleDraw(&memory, &state); break;
    }
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
