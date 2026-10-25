static const FixedSpanOpcode sound_entry_opcodes[] = {
    {0x9c0fu, 1u, 3u, 1u}, {0x9c10u, 2u, 3u, 0u},
    {0x9c12u, 3u, 3u, 0u}, {0x9c15u, 3u, 3u, 0u}
};

static const FixedSpanOpcode object_continue_opcodes[] = {
    {0xe10fu, 1u, 2u, 0u}, {0xe110u, 3u, 5u, 1u},
    {0xe113u, 1u, 2u, 0u}, {0xe114u, 1u, 2u, 0u},
    {0xe115u, 1u, 2u, 0u}, {0xe116u, 2u, 2u, 0u},
    {0xe118u, 1u, 2u, 0u}, {0xe119u, 3u, 6u, 2u}
};

static bool ContinuationEntrySupported(const CpuState *cpu, uint32_t entry) {
    const uint64_t budget = 320u;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x20000u ||
        cpu->_flag_D || cpu->host_return_valid > 3u ||
        cpu->host_return_valid == 1u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffdu, true, 1363u, budget) ||
        g_snes->vPos >= 262u || g_snes->dma->hdmaPendingInit ||
        (!g_snes->hdmaBeamOff && g_snes->nmiEnabled))
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].hdmaActive)
            return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool ObjectContinuationSupported(const CpuState *cpu) {
    if (cpu->D || !cpu->m_flag || cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return false;
    if (cpu->DB != 0x7eu && cpu->DB != 0x7fu && cpu->Y < 0x8000u)
        return false;
    return ContinuationEntrySupported(cpu, 0x83e10fu);
}

static RecompReturn ActorBridgeContinuationEntry(CpuState *cpu, uint32_t entry,
    ActorWholeFunction run, const FixedSpanOpcode *opcodes, unsigned count) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (entry == 0x83e10fu ? !ObjectContinuationSupported(cpu)
                          : !ContinuationEntrySupported(cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    clock.timing = opcodes;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && clock.index == clock.count && !clock.remaining);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
}
