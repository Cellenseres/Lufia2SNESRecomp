static bool PrepareCardAnimationClock(CardScoreClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    const CpuState *cpu = clock->call.cpu;
    if (!CardScoreRegistersSupported(cpu))
        return false;
    const bool word = pc == 0x84b11eu || pc == 0x84b121u ||
        pc == 0x84b124u || pc == 0x84b12cu || pc == 0x84b12fu ||
        pc == 0x84b13du || pc == 0x84b14eu || pc == 0x84b16cu ||
        pc == 0x84b22bu;
    if (cpu->m_flag == word)
        return false;
    switch (pc) {
    case 0x84b11eu: CardScoreCall(clock, 0xb11eu); break;
    case 0x84b121u: CardScoreCall(clock, 0xb121u); break;
    case 0x84b124u:
        CardScoreOpcode(clock, 0xb124u, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xb127u, 2u, 4u, 2u);
        CardScoreCall(clock, 0xb129u);
        break;
    case 0x84b12cu: CardScoreCall(clock, 0xb12cu); break;
    case 0x84b12fu:
        CardScoreOpcode(clock, 0xb12fu, 2u, 3u, 0u);
        CardScoreOpcode(clock, 0xb131u, 2u, 2u, 0u);
        CardScoreOpcode(clock, 0xb133u, 2u, 3u, 1u);
        CardScoreOpcode(clock, 0xb135u, 2u, 3u, 1u);
        CardScoreCall(clock, 0xb137u);
        break;
    case 0x84b13au:
        CardScoreOpcode(clock, 0xb13au, 2u, 3u, 0u);
        clock->returning = true;
        clock->next = 0x84b13cu;
        break;
    case 0x84b13du:
        CardScoreOpcode(clock, 0xb13du, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xb13fu, 1u, 3u, 0u);
        for (unsigned offset = 0u; offset < 3u; ++offset)
            CardScoreOpcode(clock, 0xb140u + offset, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xb143u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xb145u, 1u, 2u, 0u);
        CardScoreOpcode(clock, 0xb146u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xb148u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xb14au, 1u, 2u, 0u);
        CardScoreCall(clock, 0xb14bu);
        break;
    case 0x84b14eu:
        CardScoreOpcode(clock, 0xb14eu, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xb150u, 2u, 3u, 0u);
        CardScoreOpcode(clock, 0xb152u, 2u, 2u, 0u);
        CardScoreCall(clock, 0xb154u);
        break;
    case 0x84b157u: CardScoreCall(clock, 0xb157u); break;
    case 0x84b15au:
        CardScoreOpcode(clock, 0xb15au, 2u, 2u, 0u);
        CardScoreOpcode(clock, 0xb15cu, 2u, 3u, 1u);
        CardScoreCall(clock, 0xb15eu);
        break;
    case 0x84b161u: {
        const bool more = ((cpu->ram[4u] - 1u) & 0x80u) == 0u;
        CardScoreOpcode(clock, 0xb161u, 2u, 5u, 2u);
        CardScoreOpcode(clock, 0xb163u, 2u, more ? 3u : 2u, 0u);
        if (more) {
            CardScoreCall(clock, 0xb157u);
        } else {
            CardScoreOpcode(clock, 0xb165u, 2u, 3u, 0u);
            CardScoreOpcode(clock, 0xb167u, 2u, 4u, 2u);
            CardScoreCall(clock, 0xb169u);
        }
        break;
    }
    case 0x84b16cu:
        clock->returning = true;
        break;
    case 0x84b22bu:
        CardScoreOpcode(clock, 0xb22bu, 2u, 3u, 0u);
        CardScoreOpcode(clock, 0xb22du, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xb230u, 2u, 4u, 2u);
        CardScoreOpcode(clock, 0xb232u, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xb235u, 2u, 4u, 2u);
        CardScoreCall(clock, 0xb237u);
        break;
    case 0x84b23au:
        CardScoreOpcode(clock, 0xb23au, 2u, 2u, 0u);
        CardScoreOpcode(clock, 0xb23cu, 2u, 3u, 1u);
        CardScoreCall(clock, 0xb23eu);
        break;
    case 0x84b241u: {
        const bool more = cpu->ram[4u] != 1u;
        CardScoreOpcode(clock, 0xb241u, 2u, 5u, 2u);
        CardScoreOpcode(clock, 0xb243u, 2u, more ? 3u : 2u, 0u);
        if (more) {
            CardScoreCall(clock, 0xb237u);
        } else {
            CardScoreOpcode(clock, 0xb245u, 2u, 2u, 0u);
            CardScoreCall(clock, 0xb247u);
        }
        break;
    }
    case 0x84b24au:
        CardScoreOpcode(clock, 0xb24au, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xb24du, 2u, 4u, 2u);
        CardScoreCall(clock, 0xb24fu);
        break;
    case 0x84b252u:
        CardScoreOpcode(clock, 0xb252u, 2u, 2u, 0u);
        CardScoreOpcode(clock, 0xb254u, 2u, 3u, 1u);
        CardScoreCall(clock, 0xb256u);
        break;
    case 0x84b259u: CardScoreCall(clock, 0xb259u); break;
    case 0x84b25cu: CardScoreCall(clock, 0xb25cu); break;
    case 0x84b25fu:
        CardScoreOpcode(clock, 0xb25fu, 3u, 3u, 0u);
        CardScoreOpcode(clock, 0xb262u, 2u, 4u, 2u);
        CardScoreCall(clock, 0xb264u);
        break;
    case 0x84b267u:
        CardScoreOpcode(clock, 0xb267u, 2u, 2u, 0u);
        CardScoreOpcode(clock, 0xb269u, 2u, 3u, 1u);
        CardScoreCall(clock, 0xb26bu);
        break;
    case 0x84b26eu: CardScoreCall(clock, 0xb26eu); break;
    case 0x84b271u:
        CardScoreOpcode(clock, 0xb271u, 2u, 3u, 0u);
        clock->returning = true;
        clock->next = 0x84b273u;
        break;
    default:
        return false;
    }
    return CardScoreWindowSupported(clock);
}

static uint8_t CardAnimationChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareCardAnimationClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedCardAnimation(CpuState *cpu, uint32_t landing) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareCardAnimationClock(&clock, landing))
        return ActorBridgeFallback(cpu, &frame, landing);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (landing) {
    case 0x84b11eu:
        result = Lufia2CardGameRevealCard(&memory, &state, CardAnimationChild, &clock);
        break;
    case 0x84b13du:
        result = Lufia2CardGameAnimateReposition(&memory, &state, CardAnimationChild, &clock);
        break;
    case 0x84b161u:
        result = Lufia2CardGameResumeReposition(&memory, &state, CardAnimationChild, &clock);
        break;
    case 0x84b22bu:
        result = Lufia2CardGameTurnCardSprites(&memory, &state, CardAnimationChild, &clock);
        break;
    default:
        result = Lufia2CardGameResumeCardTurn(&memory, &state, CardAnimationChild, &clock, landing);
        break;
    }
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
