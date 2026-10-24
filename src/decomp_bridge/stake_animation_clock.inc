enum { STAKE_ANIMATION_CLOCK_CAPACITY = 160u };

typedef struct StakeAnimationClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[STAKE_ANIMATION_CLOCK_CAPACITY];
    ActorPushedCall call;
    bool returning;
} StakeAnimationClock;

static void StakeAnimationOpcode(StakeAnimationClock *clock, uint16_t pc,
                                  unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < STAKE_ANIMATION_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool StakeAnimationRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x84u && !cpu->emulation && cpu->m_flag <= 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D && cpu->DB == 0x7eu &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static bool StakeAnimationTimingSupported(const StakeAnimationClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = clock->returning
        ? cpu_region_speed(0x84c26eu) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x840000u, 0x1ffcu, true, 1363u, budget))
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

static bool PrepareStakeAnimationClock(StakeAnimationClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    const CpuState *cpu = clock->call.cpu;
    if (!StakeAnimationRegistersSupported(cpu))
        return false;
    if (pc == 0x84c243u) {
        if (cpu->m_flag)
            return false;
        StakeAnimationOpcode(clock, 0xc243u, 3u, 3u, 0u);
        StakeAnimationOpcode(clock, 0xc246u, 2u, 4u, 2u);
    } else if (pc == 0x84c268u) {
        if (!cpu->m_flag)
            return false;
        const unsigned remaining = cpu->ram[0u] | (unsigned)cpu->ram[1u] << 8u;
        if (remaining > 23u)
            return false;
        StakeAnimationOpcode(clock, 0xc268u, 2u, 3u, 0u);
        StakeAnimationOpcode(clock, 0xc26au, 2u, 7u, 4u);
        StakeAnimationOpcode(clock, 0xc26cu, 2u, remaining ? 3u : 2u, 0u);
        clock->returning = remaining == 0u;
    } else {
        return false;
    }
    if (!clock->returning) {
        StakeAnimationOpcode(clock, 0xc248u, 3u, 3u, 0u);
        StakeAnimationOpcode(clock, 0xc24bu, 3u, 3u, 0u);
        for (unsigned sprite = 0u; sprite < 12u; ++sprite) {
            StakeAnimationOpcode(clock, 0xc24eu, 3u, 5u, 2u);
            StakeAnimationOpcode(clock, 0xc251u, 1u, 2u, 0u);
            StakeAnimationOpcode(clock, 0xc252u, 3u, 3u, 0u);
            StakeAnimationOpcode(clock, 0xc255u, 3u, 7u, 2u);
            for (unsigned step = 0u; step < 5u; ++step)
                StakeAnimationOpcode(clock, 0xc258u + step, 1u, 2u, 0u);
            StakeAnimationOpcode(clock, 0xc25du, 2u, sprite < 11u ? 3u : 2u, 0u);
        }
        StakeAnimationOpcode(clock, 0xc25fu, 2u, 3u, 0u);
        StakeAnimationOpcode(clock, 0xc261u, 2u, 2u, 0u);
        StakeAnimationOpcode(clock, 0xc263u, 2u, 3u, 1u);
        StakeAnimationOpcode(clock, 0xc265u, 3u, 6u, 2u);
    }
    return StakeAnimationTimingSupported(clock);
}

static uint8_t StakeAnimationChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    StakeAnimationClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareStakeAnimationClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedStakeAnimation(CpuState *cpu, uint32_t landing) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    StakeAnimationClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareStakeAnimationClock(&clock, landing))
        return ActorBridgeFallback(cpu, &frame, landing);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = landing == 0x84c243u
        ? Lufia2CardGameAnimateStakeSprites(&memory, &state, StakeAnimationChild, &clock)
        : Lufia2CardGameResumeStakeAnimation(&memory, &state, StakeAnimationChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && result.pc == 0x84c26eu &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
