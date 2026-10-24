static bool SpecialFadeRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x84u && !cpu->emulation && cpu->m_flag == 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D &&
        (cpu->DB == 0x7eu || cpu->DB < 0x40u ||
         (cpu->DB >= 0x80u && cpu->DB < 0xc0u)) &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static bool SpecialFadeTimingSupported(const CardScoreClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (cpu->coprocessor_master_cycles != cpu->master_cycles)
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

static bool PrepareSpecialFadeClock(CardScoreClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    const CpuState *cpu = clock->call.cpu;
    if (!SpecialFadeRegistersSupported(cpu))
        return false;
    const bool entering = pc == 0x84a5b5u || pc == 0x84a5cau;
    if (entering) {
        const uint16_t entry = (uint16_t)pc;
        CardScoreOpcode(clock, entry, 1u, 3u, 1u);
        CardScoreOpcode(clock, entry + 1u, 2u, 3u, 0u);
        CardScoreOpcode(clock, entry + 3u, 3u, 4u, 1u);
        CardScoreOpcode(clock, entry + 6u, 2u, 2u, 0u);
        CardScoreOpcode(clock, entry + 8u, 3u, 4u, 1u);
        CardScoreCall(clock, entry + 11u);
    } else if (pc == 0x84a5c3u || pc == 0x84a5d8u) {
        const uint16_t landing = (uint16_t)pc;
        const bool waiting = cpu->ram[0x0581u] != 0u;
        CardScoreOpcode(clock, landing, 3u, 4u, 1u);
        CardScoreOpcode(clock, landing + 3u, 2u, waiting ? 3u : 2u, 0u);
        if (waiting) {
            CardScoreCall(clock, landing - 3u);
        } else {
            CardScoreOpcode(clock, landing + 5u, 1u, 4u, 1u);
            clock->returning = true;
            clock->next = pc + 6u;
        }
    } else {
        return false;
    }
    return SpecialFadeTimingSupported(clock);
}

static uint8_t SpecialFadeChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareSpecialFadeClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedSpecialFade(CpuState *cpu, uint32_t landing) {
    ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const bool resuming = landing == 0x84a5c3u || landing == 0x84a5d8u;
    if (resuming && cpu->ram && cpu->S < 0x1ffcu) {
        ++frame.entry_s;
        frame.host_return_pc24 = 0x840000u | (uint16_t)((cpu->ram[cpu->S + 2u] |
            (uint16_t)cpu->ram[cpu->S + 3u] << 8u) + 1u);
    }
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || (resuming && cpu->S >= 0x1ffcu) ||
        !PrepareSpecialFadeClock(&clock, landing))
        return ActorBridgeFallback(cpu, &frame, landing);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    if (resuming)
        result = Lufia2SpecialMenuResumeFade(&memory, &state, SpecialFadeChild, &clock, landing);
    else if (landing == 0x84a5b5u)
        result = Lufia2SpecialMenuFadeIn(&memory, &state, SpecialFadeChild, &clock);
    else
        result = Lufia2SpecialMenuFadeOut(&memory, &state, SpecialFadeChild, &clock);
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
