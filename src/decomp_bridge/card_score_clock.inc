enum { CARD_SCORE_CLOCK_CAPACITY = 256u };

typedef struct CardScoreClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[CARD_SCORE_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} CardScoreClock;

static void CardScoreOpcode(CardScoreClock *clock, uint16_t pc,
                            unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < CARD_SCORE_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static void CardScoreCall(CardScoreClock *clock, uint16_t pc) {
    CardScoreOpcode(clock, pc, 3u, 6u, 2u);
}

static bool CardScoreRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x84u && !cpu->emulation && cpu->m_flag <= 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D && cpu->DB == 0x7eu &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static bool CardScoreWindowSupported(const CardScoreClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!CardScoreRegistersSupported(cpu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = clock->returning
        ? cpu_region_speed(clock->next) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x840000u, 0x1ffcu, true,
                                             1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        const unsigned until_vblank =
            (225u - g_snes->vPos) * 1364u - g_snes->hPos;
        if (!g_snes->hdmaBeamOff || until_vblank <= budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void CardScoreLabelRow(CardScoreClock *clock, uint16_t pc) {
    for (unsigned column = 0u; column < 6u; ++column) {
        CardScoreOpcode(clock, pc, 3u, 7u, 2u);
        for (unsigned step = 3u; step < 7u; ++step)
            CardScoreOpcode(clock, pc + step, 1u, 2u, 0u);
        CardScoreOpcode(clock, pc + 7u, 2u, column < 5u ? 3u : 2u, 0u);
    }
}

static void CardScoreLabels(CardScoreClock *clock) {
    CardScoreOpcode(clock, 0xb54fu, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xb552u, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xb555u, 3u, 3u, 0u);
    CardScoreLabelRow(clock, 0xb558u);
    CardScoreOpcode(clock, 0xb561u, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xb564u, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xb567u, 3u, 3u, 0u);
    CardScoreLabelRow(clock, 0xb56au);
    CardScoreOpcode(clock, 0xb573u, 2u, 3u, 0u);
    CardScoreOpcode(clock, 0xb575u, 2u, 2u, 0u);
    CardScoreOpcode(clock, 0xb577u, 2u, 3u, 1u);
    CardScoreCall(clock, 0xb579u);
}

static void CardScoreFrame(CardScoreClock *clock) {
    CardScoreOpcode(clock, 0xadc0u, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xadc3u, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xadc6u, 3u, 3u, 0u);
    CardScoreOpcode(clock, 0xadc9u, 2u, 4u, 2u);
    for (unsigned row = 0u; row < 4u; ++row) {
        CardScoreOpcode(clock, 0xadcbu, 3u, 3u, 0u);
        CardScoreLabelRow(clock, 0xadceu);
        for (unsigned step = 0u; step < 3u; ++step)
            CardScoreOpcode(clock, 0xadd7u + step, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xaddau, 3u, 3u, 0u);
        for (unsigned step = 0u; step < 3u; ++step)
            CardScoreOpcode(clock, 0xadddu + step, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xade0u, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xade3u, 2u, 7u, 4u);
        CardScoreOpcode(clock, 0xade5u, 2u, row < 3u ? 3u : 2u, 0u);
    }
    CardScoreOpcode(clock, 0xade7u, 2u, 3u, 0u);
    CardScoreOpcode(clock, 0xade9u, 2u, 2u, 0u);
    CardScoreOpcode(clock, 0xadebu, 2u, 3u, 1u);
    CardScoreCall(clock, 0xadedu);
}

static bool PrepareCardScoreClock(CardScoreClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!CardScoreRegistersSupported(clock->call.cpu))
        return false;
    switch (pc) {
    case 0x84b54fu: CardScoreLabels(clock); break;
    case 0x84adc0u: CardScoreFrame(clock); break;
    case 0x84adf3u: CardScoreCall(clock, 0xadf3u); break;
    case 0x84adf6u:
        CardScoreOpcode(clock, 0xadf6u, 2u, 4u, 2u);
        CardScoreCall(clock, 0xadf8u);
        break;
    case 0x84adfbu:
        CardScoreOpcode(clock, 0xadfbu, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xadfcu, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xadffu, 3u, 3u, 0u);
        CardScoreCall(clock, 0xae02u);
        break;
    case 0x84ae05u:
        CardScoreOpcode(clock, 0xae05u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xae07u, 1u, 2u, 0u);
        CardScoreCall(clock, 0xae08u);
        break;
    case 0x84ae0bu:
        CardScoreOpcode(clock, 0xae0bu, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae0cu, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae0du, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xae0fu, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae10u, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae11u, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae12u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xae14u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xae16u, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae17u, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae18u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xae1au, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xae1bu, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xae1eu, 3u, 3u, 0u);
        CardScoreCall(clock, 0xae21u);
        break;
    case 0x84ae24u:
        CardScoreOpcode(clock, 0xae24u, 2u, 3u, 0u);
        CardScoreOpcode(clock, 0xae26u, 2u, 2u, 0u);
        CardScoreOpcode(clock, 0xae28u, 2u, 3u, 1u);
        CardScoreCall(clock, 0xae2au);
        break;
    case 0x84b57cu:
    case 0x84adf0u:
    case 0x84ae2du:
        CardScoreOpcode(clock, (uint16_t)pc, 2u, 3u, 0u);
        clock->returning = true;
        clock->next = pc + 2u;
        break;
    default:
        return false;
    }
    return CardScoreWindowSupported(clock);
}

static uint8_t CardScoreChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareCardScoreClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedCardScore(CpuState *cpu, uint32_t entry,
                                             ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->m_flag || cpu->host_return_valid != 2u || !PrepareCardScoreClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, CardScoreChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
