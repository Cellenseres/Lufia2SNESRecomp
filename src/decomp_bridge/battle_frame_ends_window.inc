static void ActorBridgeBattleFrameEndsWindow(CpuState *cpu, uint32_t entry) {
    const unsigned count = 14u, reserve = 640u;
    const uint32_t end = 0x81e592u;
    if (!BattleFrameEndsHookOwned(entry, ActorBridgeBattleFrameEndsWindow) ||
        cpu != &g_cpu || cpu->PB != 0x81u || cpu->D || cpu->_flag_D ||
        cpu->m_flag || cpu->x_flag || !cpu->ram ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->X < 0x8000u || cpu->X > 0xfffcu ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const uint8_t *ram = cpu->ram + (cpu->DB == 0x7fu ? 0x10000u : 0u);
    const unsigned width = ram[0x09fau] | (unsigned)ram[0x09fbu] << 8;
    if (!BattleWindowOutputSupported(cpu->Y, cpu->Y + 2u * width + 6u) ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffdu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, BattleFrameEndsHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeBattleFrameEndsWindow) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 32u, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = battle_window_sides,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleFrameEnds(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == end);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
