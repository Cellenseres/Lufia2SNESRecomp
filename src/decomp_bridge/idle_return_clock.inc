static bool IdleReturnWindowSupported(const CpuState *cpu, uint32_t entry) {
    const unsigned budget = cpu_region_speed(entry) + 2u * 8u + 3u * 6u + 40u;
    if (cpu->host_return_valid != 2u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffdu, true, 1363u, budget))
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

static RecompReturn ActorBridgeTimedIdleReturn(CpuState *cpu, uint32_t entry,
    Lufia2ExecutionResult (*run)(const Lufia2Memory *, Lufia2CpuState *)) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (!IdleReturnWindowSupported(cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == entry);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, entry);
}
