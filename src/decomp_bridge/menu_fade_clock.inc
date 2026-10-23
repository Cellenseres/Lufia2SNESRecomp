static bool MenuFadeClockSupported(const CpuState *cpu, uint32_t entry) {
    if ((entry != 0x868b2bu && entry != 0x868b41u) || cpu->D ||
            cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
            (cpu->DB != 0x7eu && (cpu->DB & 0x7fu) >= 0x40u) ||
            !Lufia2DecompFrameReturnCpuReady())
        return false;
    const unsigned budget = 6u * cpu_region_speed(entry) + 70u;
    if (!g_snes || cpu->coprocessor_master_cycles != cpu->master_cycles ||
            cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= 1364u ||
            (refresh_clock && refresh_clock != cpu->master_cycles))
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (budget >= fence)
        return false;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu, false,
        fence - budget - 1u, budget);
}

static void ActorBridgeMenuFadeContinuation(CpuState *cpu, uint32_t entry) {
    if (!MenuFadeClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[] = {
        {(uint16_t)entry, 3u, 4u, 1u},
        {(uint16_t)(entry + 3u), 2u, 2u, 0u},
        {(uint16_t)(entry + 5u), 1u, 4u, 1u},
    };
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x860000u;
    clock.timing = timing;
    clock.count = 3u;
    const Lufia2Memory memory = {FixedSpanRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuContinueDisplayFade(&memory, &state, entry);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    if (result.pc == entry - 4u) {
        clock.count = 2u;
        timing[1].cycles = 3u;
    } else {
        assert(result.pc == entry + 6u);
    }
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
