static const FixedSpanOpcode equipment_inventory_read[] = {
    {0xbd70u, 1u, 4u, 2u},
    {0xbd71u, 2u, 4u, 2u},
    {0xbd73u, 3u, 5u, 2u},
};

static const FixedSpanOpcode equipment_inventory_advance[] = {
    {0xbd8eu, 1u, 5u, 2u},
    {0xbd8fu, 1u, 2u, 0u},
    {0xbd90u, 1u, 2u, 0u},
    {0xbd91u, 3u, 3u, 0u},
};

enum { EQUIPMENT_INVENTORY_PAGE_REMAINING = 0x73u };

static bool EquipmentInventoryClockSupported(const CpuState *cpu, uint32_t entry) {
    const bool read_entry = entry == 0x82bd70u;
    const uint8_t bank = cpu->DB & 0x7fu;
    if ((!read_entry && entry != 0x82bd8eu) || cpu->D || cpu->_flag_D ||
        cpu->m_flag || cpu->x_flag || cpu->S < 0x1f10u ||
        (bank >= 0x40u && cpu->DB != 0x7eu) ||
        (read_entry && cpu->X >= 192u))
        return false;
    const bool page_crossed = read_entry && cpu->X >= EQUIPMENT_INVENTORY_PAGE_REMAINING;
    const unsigned master = 6u * cpu_region_speed(entry) +
        (read_entry ? 54u : 40u) + (page_crossed ? 6u : 0u);
    const unsigned budget = master + 40u;
    if (!g_snes || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= 1364u || (refresh_clock && refresh_clock != cpu->master_cycles))
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(
        cpu, entry, 0x1ffcu, true, fence - budget - 1u, budget);
}

static void ActorBridgeEquipmentInventoryStep(CpuState *cpu, uint32_t entry) {
    if (!EquipmentInventoryClockSupported(cpu, entry))
        return;
    const bool read_entry = entry == 0x82bd70u;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    clock.count = read_entry ? 3u : 4u;
    FixedSpanOpcode timing[4];
    const FixedSpanOpcode *original =
        read_entry ? equipment_inventory_read : equipment_inventory_advance;
    for (unsigned index = 0u; index < clock.count; ++index)
        timing[index] = original[index];
    if (read_entry && cpu->X >= EQUIPMENT_INVENTORY_PAGE_REMAINING)
        ++timing[2].cycles;
    clock.timing = timing;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = read_entry
        ? Lufia2MenuReadEquipmentInventoryEntry(&memory, &state)
        : Lufia2MenuAdvanceEquipmentInventoryIndex(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
