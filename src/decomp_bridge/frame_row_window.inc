static unsigned BattleRowWindowSpanSize(const CpuState *cpu, uint32_t entry) {
    if (!Lufia2DecompOpcodeWindowReady())
        return 0u;
    const unsigned legacy = BattleRowSpanSize(cpu, entry);
    if (legacy)
        return legacy;
    if (entry != 0x81e553u || cpu != &g_cpu || cpu->PB != 0x81u ||
        cpu->m_flag || cpu->x_flag || cpu->D || cpu->_flag_D ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !NativeOpcodeWindowSupported(cpu, entry, 208u, 0x1fffu, true))
        return 0u;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles) ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 10u, BattleFrameRowHookOwned))
        return 0u;
    const unsigned budget = Lufia2DecompFrameSpanBudget(entry, entry + 10u,
        ActorBridgeBattleFrameCells);
    if (budget < 5u)
        return 0u;
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    const uint16_t counter = cpu->ram[bank + 0x0a3cu] |
        (uint16_t)cpu->ram[bank + 0x0a3du] << 8;
    unsigned cells = counter > 0x8000u ? 1u : (unsigned)counter + 1u;
    if (cells > 6u)
        cells = 6u;
    if (cells > budget / 5u)
        cells = budget / 5u;
    for (; cells; --cells) {
        const unsigned room = 164u * cells + 48u;
        if (cpu->master_cycles <= UINT64_MAX - room &&
            NativeOpcodeWindowSupported(cpu, entry, room, 0x1fffu, true) &&
            BattleWindowOutputSupported((unsigned)cpu->Y + 2u,
                (unsigned)cpu->Y + 2u + 2u * cells))
            return cells;
    }
    return 0u;
}
