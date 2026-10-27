typedef struct PanelInputCall {
    ActorPushedCall call;
    PanelInputClock clock;
    unsigned input_frames;
} PanelInputCall;

static uint8_t PanelInputChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    PanelInputCall *panel = context;
    PanelInputClock *clock = &panel->clock;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    assert(landing == clock->next);
    if (site == 0x84ee9eu)
        clock->stop_before_input = ++panel->input_frames == 4096u;
    if (!ActorBridgePushedChild(&panel->call, state, target, site, frame_size))
        return 0u;
    if (PreparePanelInputClock(clock, panel->call.cpu, landing))
        return 1u;
    panel->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        panel->call.cpu, landing, landing, panel->call.frame.entry_s,
        panel->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgePanelInput(CpuState *cpu, uint32_t entry) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    PanelInputCall panel;
    panel.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    panel.clock.stop_before_input = false;
    panel.input_frames = 0u;
    if (!PreparePanelInputClock(&panel.clock, cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite,
        &panel.clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (entry) {
    case 0x84ee4bu:
        result = Lufia2CardTableRunInputPanel(&memory, &state, PanelInputChild, &panel);
        break;
    case 0x84ef4eu:
        result = Lufia2CardTableLoadPanelResource(&memory, &state, PanelInputChild, &panel);
        break;
    case 0x84ef88u:
        result = Lufia2CardTableAppendPanelCommand(&memory, &state);
        break;
    default:
        result = Lufia2CardTableMatchPanelSequence(&memory, &state, PanelInputChild, &panel);
        break;
    }
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)panel.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&panel.clock.span);
    assert(panel.clock.span.index == panel.clock.span.count &&
        !panel.clock.span.remaining && result.pc == panel.clock.next);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

static RecompReturn ActorBridgePanelCopy(CpuState *cpu, uint32_t entry) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    PanelCopyClock clock;
    if (!PreparePanelCopyClock(&clock, cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const uint32_t next = RunPanelCopySpan(&clock, entry);
    return interp_tier_dispatch_tail(cpu, next, next, frame.entry_s, frame.hrv);
}
