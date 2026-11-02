#include "window_hdma_plans.inc"

static bool WindowHdmaClockSupported(const CpuState *cpu, uint32_t entry) {
    const unsigned budget = 2048u;
    if (cpu->D || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->host_return_valid != 3u || cpu->S < 0x1f20u ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, budget) ||
        !Lufia2DecompFrameRangeQuiet(entry, entry == 0x85aa7fu ? 0x85aac3u : 0x85aadcu))
        return false;
    const unsigned line = g_snes->vPos;
    if (line < 225u) {
        if (!g_snes->hdmaBeamOff || (225u - line) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (line >= 262u || (262u - line) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeWindowHdma(CpuState *cpu, uint32_t entry,
    ActorWholeFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (!WindowHdmaClockSupported(cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    if (entry == 0x85aa7fu) {
        clock.timing = window_hdma_initialize;
        clock.count = sizeof window_hdma_initialize / sizeof window_hdma_initialize[0];
    } else {
        clock.timing = window_hdma_restore;
        clock.count = sizeof window_hdma_restore / sizeof window_hdma_restore[0];
    }
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}
