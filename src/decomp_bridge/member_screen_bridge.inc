typedef struct MemberScreenCall {
    ActorPushedCall call;
    MemberScreenClock clock;
} MemberScreenCall;

static uint8_t MemberScreenBridgeChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    MemberScreenCall *screen = context;
    MemberScreenClock *clock = &screen->clock;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    assert(landing == clock->next);
    if (!ActorBridgePushedChild(&screen->call, state, target, site, frame_size))
        return 0u;
    if (PrepareMemberScreenClock(clock, screen->call.cpu, landing))
        return 1u;
    screen->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        screen->call.cpu, landing, landing, screen->call.frame.entry_s,
        screen->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeMemberScreen(CpuState *cpu, uint32_t entry) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    MemberScreenCall screen;
    screen.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u ||
        !PrepareMemberScreenClock(&screen.clock, cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite,
        &screen.clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    if (entry == 0x829e67u)
        result = Lufia2MenuBuildMemberStatus(&memory, &state,
            MemberScreenBridgeChild, &screen);
    else if (entry == 0x82b2feu)
        result = Lufia2MenuRunMemberStatus(&memory, &state,
            MemberScreenBridgeChild, &screen);
    else
        result = Lufia2MenuChooseSpellMember(&memory, &state,
            MemberScreenBridgeChild, &screen);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)screen.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&screen.clock.span);
    assert(screen.clock.span.index == screen.clock.span.count &&
        !screen.clock.span.remaining && result.pc == screen.clock.next);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

static RecompReturn ActorBridgeMemberDispatch(CpuState *cpu,
    const ActorBridgeFrame *frame) {
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuDispatchMainSelection(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        if ((result.pc == 0x82b2feu || result.pc == 0x82b370u) &&
            cpu->S == frame->entry_s && cpu_dispatch_has_entry(cpu, result.pc)) {
            cpu->host_return_valid = frame->hrv;
            return ActorBridgeMemberScreen(cpu, result.pc);
        }
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame->entry_s, frame->hrv);
    }
    return ActorBridgeReturn(cpu, frame, 2u, result.pc);
}

static void ActorBridgeMemberScreenPhase(CpuState *cpu, uint32_t entry) {
    MemberScreenClock clock;
    if (!PrepareMemberScreenClockMode(&clock, cpu, entry, true))
        return;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuRunMemberScreenPhase(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == clock.next &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
