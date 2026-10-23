enum {
    INVENTORY_SEARCH_FIRST = 0x0a8du,
    INVENTORY_SEARCH_BYTES = 192u,
    INVENTORY_SEARCH_BUDGET = 13200u
};

typedef struct InventorySearchClock {
    CpuState *cpu;
    FixedSpanOpcode read_timing;
    FixedSpanClock read_clock;
    unsigned offset;
    unsigned opcodes;
    uint16_t word;
    bool high_byte;
} InventorySearchClock;

static void InventorySearchOpcode(InventorySearchClock *search,
                                  uint16_t pc, unsigned fetched, unsigned cycles) {
    const FixedSpanOpcode timing = {pc, (uint8_t)fetched, (uint8_t)cycles, 0u};
    FixedSpanClock clock = {0};
    clock.cpu = search->cpu;
    clock.bank = 0x820000u;
    clock.timing = &timing;
    clock.count = 1u;
    BeginFixedSpanOpcode(&clock);
    assert(!clock.progress_mask);
    ++search->opcodes;
}

static uint8_t InventorySearchRead(void *context, uint32_t address) {
    InventorySearchClock *search = context;
    assert(address == (((uint32_t)search->cpu->DB << 16) |
        (INVENTORY_SEARCH_FIRST + search->offset + (search->high_byte ? 1u : 0u))));
    if (!search->high_byte) {
        if (!search->offset)
            InventorySearchOpcode(search, 0xfb94u, 3u, 3u);
        search->read_timing = (FixedSpanOpcode){0xfb97u, 3u,
            (uint8_t)(5u + (search->offset >= 115u)), 2u};
        search->read_clock = (FixedSpanClock){0};
        search->read_clock.cpu = search->cpu;
        search->read_clock.bank = 0x820000u;
        search->read_clock.timing = &search->read_timing;
        search->read_clock.count = 1u;
    }
    const uint8_t value = FixedSpanRead(&search->read_clock, address);
    if (!search->high_byte) {
        search->word = value;
        search->high_byte = true;
        return value;
    }
    search->word |= (uint16_t)value << 8;
    search->high_byte = false;
    assert(!search->read_clock.progress_mask);
    ++search->opcodes;
    InventorySearchOpcode(search, 0xfb9au, 2u, search->word ? 2u : 3u);
    if (!search->word) {
        InventorySearchOpcode(search, 0xfba5u, 1u, 2u);
    } else {
        InventorySearchOpcode(search, 0xfb9cu, 1u, 2u);
        InventorySearchOpcode(search, 0xfb9du, 1u, 2u);
        InventorySearchOpcode(search, 0xfb9eu, 3u, 3u);
        InventorySearchOpcode(search, 0xfba1u, 2u,
            search->offset + 2u == INVENTORY_SEARCH_BYTES ? 2u : 3u);
        if (search->offset + 2u == INVENTORY_SEARCH_BYTES)
            InventorySearchOpcode(search, 0xfba3u, 1u, 2u);
    }
    search->offset += 2u;
    return value;
}

static bool InventorySearchClockSupported(const CpuState *cpu, uint32_t entry) {
    if (!Lufia2DecompFrameReturnCpuReady() ||
        entry != 0x82fb94u || cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->_flag_D || !(cpu->DB == 0x7eu || (cpu->DB & 0x7fu) < 0x40u) ||
        cpu->S < 0x1f10u || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - INVENTORY_SEARCH_BUDGET)
        return false;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, true,
                                             1363u, INVENTORY_SEARCH_BUDGET))
        return false;
    if (g_snes->vPos < 225u) {
        const unsigned until_vblank =
            (225u - g_snes->vPos) * 1364u - g_snes->hPos;
        if (!g_snes->hdmaBeamOff || until_vblank < INVENTORY_SEARCH_BUDGET)
            return false;
    } else if (g_snes->vPos > 249u) {
        return false;
    }
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    return refresh_phase < 1364u &&
        (!refresh_clock || refresh_clock == cpu->master_cycles);
}

static void ActorBridgeInventorySearch(CpuState *cpu, uint32_t entry) {
    if (!InventorySearchClockSupported(cpu, entry))
        return;
    InventorySearchClock clock = {0};
    clock.cpu = cpu;
    const Lufia2Memory memory = {InventorySearchRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2InventoryFindEmptySlot(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    assert(result.pc == 0x82fba4u || result.pc == 0x82fba6u);
    assert(!clock.high_byte && clock.offset <= INVENTORY_SEARCH_BYTES);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, 0u);
    interp_bridge_pre_opcode_redirect(result.pc);
}
