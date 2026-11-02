static const FixedSpanOpcode preview_initializer_prefix[] = {
    {0x9c9bu, 2u, 3u, 0u}, {0x9c9du, 2u, 3u, 0u},
    {0x9c9fu, 2u, 2u, 0u}, {0x9ca1u, 1u, 3u, 1u},
    {0x9ca2u, 1u, 4u, 1u}, {0x9ca3u, 2u, 2u, 0u},
    {0x9ca5u, 3u, 4u, 1u}, {0x9ca8u, 3u, 4u, 1u},
    {0x9cabu, 3u, 4u, 1u}, {0x9caeu, 3u, 4u, 1u},
    {0x9cb1u, 3u, 4u, 1u}, {0x9cb4u, 4u, 8u, 3u}
};

static const FixedSpanOpcode world_initializer_prefix[] = {
    {0x920eu, 1u, 3u, 1u}, {0x920fu, 1u, 3u, 1u},
    {0x9210u, 2u, 3u, 0u}, {0x9212u, 2u, 3u, 0u},
    {0x9214u, 2u, 2u, 0u}, {0x9216u, 1u, 3u, 1u},
    {0x9217u, 1u, 4u, 1u}, {0x9218u, 3u, 4u, 1u},
    {0x921bu, 3u, 6u, 2u}
};

typedef struct SessionInitializerClock {
    FixedSpanClock span;
    uint32_t child;
} SessionInitializerClock;

static uint8_t SessionInitializerChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    SessionInitializerClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert((site == 0x849cb4u && target == 0x81c2fbu && frame_size == 3u) ||
        (site == 0x86921bu && target == 0x86cdf5u && frame_size == 2u));
    state->program_bank = (uint8_t)(target >> 16);
    clock->child = target;
    return 0u;
}

static RecompReturn ActorBridgeSessionInitializer(CpuState *cpu, uint32_t entry,
    ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const bool preview = entry == 0x849c9bu;
    const uint32_t end = preview ? 0x849cb8u : 0x86921eu;
    const unsigned fence = g_snes && !g_snes->hdmaBeamOff &&
        g_snes->vPos < 225u ? 1024u : 1364u;
    if (cpu->D || cpu->_flag_D || cpu->x_flag || cpu->m_flag != 1u ||
        cpu->host_return_valid != 3u || cpu->S < 0x1f20u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu, false,
            fence - 1u, 512u) || g_snes->vPos >= 262u ||
        fence - g_snes->hPos <= 512u ||
        !Lufia2DecompFrameRangeQuiet(entry, end))
        return ActorBridgeFallback(cpu, &frame, entry);
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return ActorBridgeFallback(cpu, &frame, entry);
    SessionInitializerClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = entry & 0xff0000u;
    clock.span.timing = preview ? preview_initializer_prefix : world_initializer_prefix;
    clock.span.count = preview ? sizeof preview_initializer_prefix / sizeof(FixedSpanOpcode)
        : sizeof world_initializer_prefix / sizeof(FixedSpanOpcode);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state,
        SessionInitializerChild, &clock);
    assert(result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND && clock.child);
    (void)result;
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    return interp_tier_dispatch_tail(cpu, clock.child,
        preview ? 0x849cb4u : 0x86921bu, frame.entry_s, frame.hrv);
}
