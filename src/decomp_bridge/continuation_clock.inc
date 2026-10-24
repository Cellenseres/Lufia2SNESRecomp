enum { CONTINUATION_CLOCK_CAPACITY = 48u };

typedef struct ContinuationClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[CONTINUATION_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t entry;
    bool returning;
} ContinuationClock;

static void ContinuationOpcode(ContinuationClock *clock, unsigned pc,
                                unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < CONTINUATION_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool ContinuationWindow(const ContinuationClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    unsigned budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, clock->entry, 0x1ffcu, true, 1363u, budget) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM || !g_snes->cart->rom ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void PrepareCursorPosition(ContinuationClock *clock, bool vertical) {
    const CpuState *cpu = clock->call.cpu;
    const uint8_t *ram = cpu->ram;
    const unsigned move = vertical ? 0x8a81u : 0x8ab2u;
    const unsigned crossing = cpu->Y >= 24u;
    const uint8_t x = (uint8_t)(ram[0x1388u + cpu->Y] +
        (!vertical ? ram[0xe5u] : 0u));
    const uint8_t y = (uint8_t)(ram[0x13e8u + cpu->Y] +
        (vertical ? ram[0xe6u] : 0u));
    const bool first_equal = vertical ? y == ram[0xe0u] : x == ram[0xdfu];
    clock->returning = first_equal && (vertical ? x == ram[0xdfu] : y == ram[0xe0u]);
    ContinuationOpcode(clock, move, 3u, 4u + (vertical && crossing), 1u);
    ContinuationOpcode(clock, move + 3u, 1u, 2u, 0u);
    ContinuationOpcode(clock, move + 4u, 2u, 3u, 1u);
    ContinuationOpcode(clock, move + 6u, 3u, 6u, 1u);
    if (vertical) {
        ContinuationOpcode(clock, move + 9u, 2u, 3u, 1u);
    } else {
        ContinuationOpcode(clock, move + 9u, 2u, 3u, 1u);
        ContinuationOpcode(clock, move + 11u, 3u, 4u + crossing, 1u);
        ContinuationOpcode(clock, move + 14u, 2u, 3u, 1u);
        ContinuationOpcode(clock, move + 16u, 3u, 4u, 1u);
        ContinuationOpcode(clock, move + 19u, 2u, 3u, 1u);
    }
    const unsigned branch = move + (vertical ? 11u : 21u);
    ContinuationOpcode(clock, branch, 2u, first_equal ? 2u : 3u, 0u);
    if (first_equal) {
        ContinuationOpcode(clock, branch + 2u, 3u, 4u + (!vertical && crossing), 1u);
        ContinuationOpcode(clock, branch + 5u, 2u, 3u, 1u);
        ContinuationOpcode(clock, branch + 7u, 2u, clock->returning ? 2u : 3u, 0u);
        if (clock->returning)
            ContinuationOpcode(clock, branch + 9u, 3u, 3u, 0u);
    }
    if (!clock->returning)
        ContinuationOpcode(clock, vertical ? 0x8a98u : 0x8ad3u, 3u, 6u, 2u);
}

static void PrepareCursorContinuation(ContinuationClock *clock, uint32_t landing) {
    const uint8_t *ram = clock->call.cpu->ram;
    const bool vertical = landing == 0x828a9bu;
    const unsigned error = vertical ? 0xe4u : 0xe3u;
    const bool correction = ram[error] < ram[vertical ? 0xe3u : 0xe4u];
    const unsigned pc = vertical ? 0x8a75u : 0x8aa6u;
    ContinuationOpcode(clock, (uint16_t)landing, 2u, 3u, 0u);
    ContinuationOpcode(clock, pc, 2u, 3u, 1u);
    ContinuationOpcode(clock, pc + 2u, 1u, 2u, 0u);
    ContinuationOpcode(clock, pc + 3u, 2u, 3u, 1u);
    ContinuationOpcode(clock, pc + 5u, 2u, 3u, 1u);
    ContinuationOpcode(clock, pc + 7u, 2u, correction ? 2u : 3u, 0u);
    if (correction) {
        ContinuationOpcode(clock, pc + 9u, 3u, 6u, 2u);
        return;
    }
    PrepareCursorPosition(clock, vertical);
}

static bool PrepareContinuationClock(ContinuationClock *clock, uint32_t landing) {
    CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = clock->entry & 0xff0000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    if (!cpu->ram || cpu->host_return_valid != 2u || cpu->D || cpu->_flag_D ||
        cpu->S < 0x1f20u || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        cpu->PB != clock->entry >> 16)
        return false;
    if (clock->entry == 0x869748u) {
        if (landing == 0x86974bu) {
            clock->returning = cpu->m_flag ? (uint8_t)cpu->A == 1u : cpu->A == 1u;
            ContinuationOpcode(clock, 0x974bu, 1u, 2u, 0u);
            ContinuationOpcode(clock, 0x974cu, 2u, clock->returning ? 2u : 3u, 0u);
        } else if (landing != clock->entry) {
            return false;
        }
        if (!clock->returning)
            ContinuationOpcode(clock, 0x9748u, 3u, 6u, 2u);
    } else {
        if ((landing != clock->entry && landing != (clock->entry == 0x828a9bu ? 0x828a81u : 0x828ab2u)) || !cpu->m_flag || cpu->x_flag ||
            cpu->Y >= 48u || !ActorBridgeSystemBank(cpu->DB))
            return false;
        if (landing == clock->entry)
            PrepareCursorContinuation(clock, landing);
        else
            PrepareCursorPosition(clock, clock->entry == 0x828a9bu);
    }
    return ContinuationWindow(clock);
}

static uint8_t ContinuationChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    ContinuationClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareContinuationClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedContinuation(CpuState *cpu, uint32_t entry,
                                                  ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    ContinuationClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    clock.entry = entry;
    if (!PrepareContinuationClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, ContinuationChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        unsigned count = 0u;
        while (count < clock.span.count && clock.timing[count].pc != (uint16_t)result.pc)
            ++count;
        clock.span.count = count;
    }
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    assert(clock.returning && result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (entry == 0x869748u ? 0x86974eu : 0x828b07u));
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
