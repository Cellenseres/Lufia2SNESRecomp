static const FixedSpanOpcode equipment_inventory_read[] = {
    {0xbd70u, 1u, 4u, 2u},
    {0xbd71u, 2u, 4u, 2u},
    {0xbd73u, 3u, 5u, 2u},
    {0xbd76u, 2u, 2u, 0u},
};

static const FixedSpanOpcode equipment_inventory_select[] = {
    {0xbd78u, 3u, 8u, 4u},
    {0xbd7bu, 3u, 5u, 2u},
};

static const FixedSpanOpcode equipment_inventory_classify[] = {
    {0xbd82u, 3u, 3u, 0u},
    {0xbd85u, 2u, 2u, 0u},
};

static const FixedSpanOpcode equipment_inventory_advance[] = {
    {0xbd8eu, 1u, 5u, 2u},
    {0xbd8fu, 1u, 2u, 0u},
    {0xbd90u, 1u, 2u, 0u},
    {0xbd91u, 3u, 3u, 0u},
    {0xbd94u, 2u, 2u, 0u},
};

typedef struct EquipmentInventoryPhase {
    uint32_t entry;
    const FixedSpanOpcode *timing;
    unsigned count;
    uint16_t branch_target;
    Lufia2ExecutionResult (*run)(const Lufia2Memory *, Lufia2CpuState *);
} EquipmentInventoryPhase;

static const EquipmentInventoryPhase equipment_inventory_phases[] = {
    {0x82bd70u, equipment_inventory_read, 4u, 0xbd8eu,
        Lufia2MenuInspectEquipmentInventoryEntry},
    {0x82bd78u, equipment_inventory_select, 2u, 0u,
        Lufia2MenuSelectEquipmentInventoryRecord},
    {0x82bd82u, equipment_inventory_classify, 2u, 0xbd8eu,
        Lufia2MenuClassifyEquipmentInventoryRecord},
    {0x82bd8eu, equipment_inventory_advance, 5u, 0xbd70u,
        Lufia2MenuContinueEquipmentInventory},
};

enum { EQUIPMENT_INVENTORY_PAGE_REMAINING = 0x73u };

static bool EquipmentInventoryClockSupported(
    const CpuState *cpu, const EquipmentInventoryPhase *phase) {
    const bool read_entry = phase->entry == 0x82bd70u;
    const uint8_t bank = cpu->DB & 0x7fu;
    if (cpu->D || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f10u || (bank >= 0x40u && cpu->DB != 0x7eu) ||
        (read_entry && cpu->X >= 192u))
        return false;
    unsigned master = phase->branch_target ? 6u : 0u;
    for (unsigned index = 0u; index < phase->count; ++index) {
        const FixedSpanOpcode *timing = phase->timing + index;
        master += timing->fetched * cpu_region_speed(phase->entry) +
            timing->data_bytes * 8u +
            (timing->cycles - timing->fetched - timing->data_bytes) * 6u;
    }
    if (read_entry && cpu->X >= EQUIPMENT_INVENTORY_PAGE_REMAINING)
        master += 6u;
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
        cpu, phase->entry, 0x1ffcu, true, fence - budget - 1u, budget);
}

static void ActorBridgeEquipmentInventoryStep(CpuState *cpu, uint32_t entry) {
    const EquipmentInventoryPhase *phase = NULL;
    for (unsigned index = 0u; index < 4u; ++index)
        if (equipment_inventory_phases[index].entry == entry)
            phase = equipment_inventory_phases + index;
    if (!phase || !EquipmentInventoryClockSupported(cpu, phase))
        return;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    clock.count = phase->count;
    FixedSpanOpcode timing[5];
    for (unsigned index = 0u; index < clock.count; ++index)
        timing[index] = phase->timing[index];
    if (entry == 0x82bd70u && cpu->X >= EQUIPMENT_INVENTORY_PAGE_REMAINING)
        ++timing[2].cycles;
    clock.timing = timing;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = phase->run(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    if (phase->branch_target && (uint16_t)result.pc == phase->branch_target)
        ++timing[clock.count - 1u].cycles;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
