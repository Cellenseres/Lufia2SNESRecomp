enum { SWIRL_HDMA_ROW_BUDGET = 514u };

static bool SwirlHdmaClockSupported(const CpuState *cpu, uint32_t entry) {
    const bool first_half = cpu->Y < 0xe0u && !(cpu->Y & 1u);
    const bool second_half = cpu->Y >= 0xe1u && cpu->Y < 0x1c1u && (cpu->Y & 1u);
    if (entry != 0x848cddu || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->X > 0xffu ||
        (!first_half && !second_half) ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - SWIRL_HDMA_ROW_BUDGET || !g_snes)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
            fence - SWIRL_HDMA_ROW_BUDGET - 1u, SWIRL_HDMA_ROW_BUDGET))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static void ActorBridgeSwirlHdmaRow(CpuState *cpu, uint32_t entry) {
    if (!SwirlHdmaClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[19] = {
        {0x8cddu, 4u, 5u, 1u}, {0x8ce1u, 4u, 5u, 1u},
        {0x8ce5u, 2u, 3u, 0u}, {0x8ce7u, 4u, 6u, 2u},
        {0x8cebu, 1u, 2u, 0u}, {0x8cecu, 4u, 6u, 2u},
        {0x8cf0u, 3u, 7u, 2u}, {0x8cf3u, 1u, 2u, 0u},
        {0x8cf4u, 1u, 2u, 0u}, {0x8cf5u, 3u, 3u, 0u},
        {0x8cf8u, 1u, 2u, 0u}, {0x8cf9u, 2u, 3u, 0u},
        {0x8cfbu, 1u, 2u, 0u}, {0x8cfcu, 1u, 2u, 0u},
        {0x8cfdu, 3u, 3u, 0u}, {0x8d00u, 2u, 3u, 0u}
    };
    unsigned count = 16u;
    if (cpu->Y == 0xdeu) {
        timing[15].cycles = 2u;
        timing[count++] = (FixedSpanOpcode){0x8d02u, 1u, 2u, 0u};
    }
    timing[count++] = (FixedSpanOpcode){0x8d03u, 3u, 3u, 0u};
    timing[count++] = (FixedSpanOpcode){0x8d06u, 2u,
        (uint8_t)(cpu->Y == 0x1bfu ? 2u : 3u), 0u};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x840000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleSwirlHdmaRowStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.pc == 0x848cddu || result.pc == 0x848d08u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
