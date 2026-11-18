static void ActorBridgeScreenFadeRequest(CpuState *cpu, uint32_t entry);

static uint32_t ScreenFadeRequestHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc & 0x7fffffu) == 0x0086c1u && hook == ActorBridgeScreenFadeRequest
        ? pc + 5u : 0u;
}

static bool ScreenFadeRequestHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return ScreenFadeRequestHookEnd(pc, hook) != 0u;
}

static void ActorBridgeScreenFadeRequest(CpuState *cpu, uint32_t entry) {
    const uint32_t end = ScreenFadeRequestHookEnd(entry, ActorBridgeScreenFadeRequest);
    if (!end || entry < 0x800000u || cpu != &g_cpu || cpu->m_flag != 1u ||
        !NmiClockWindowWithDeferredHdma(cpu, cpu->x_flag != 0u, entry, end, 192u, 192u,
            true, ScreenFadeRequestHookOwned, true))
        return;
    const FixedSpanOpcode timing[] = {
        {0x86c1u, 3u, 4u, 1u},
        {0x86c4u, 2u, (uint8_t)(cpu->ram[0x0581u] & 0x80u ? 2u : 3u), 0u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = LUFIA2_NMI_FADE_REQUEST_OPCODES};
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2ScreenFadeCheckRequest(&memory, &state);
    NmiClockBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == LUFIA2_NMI_FADE_REQUEST_OPCODES && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(LUFIA2_NMI_FADE_REQUEST_OPCODES, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
