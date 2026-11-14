enum { ITEM_REGION_RECORD_LIMIT = 8u };

typedef struct ItemRegionPlan {
    FixedSpanOpcode opcodes[LUFIA2_FIELD_ITEM_REGION_OPCODE_BUDGET - 2u];
    unsigned count;
    uint32_t landing;
} ItemRegionPlan;

static void AppendItemRegionOpcode(ItemRegionPlan *plan, unsigned pc,
    unsigned fetched, unsigned cycles, unsigned data) {
    assert(plan->count < sizeof plan->opcodes / sizeof *plan->opcodes);
    plan->opcodes[plan->count++] = (FixedSpanOpcode){pc, fetched, cycles, data};
}

static unsigned ItemRegionIndexedReadCycles(unsigned offset, unsigned field) {
    return 4u + (offset + field >= 0x100u);
}

static bool PlanItemRegionRectangle(ItemRegionPlan *plan, const uint8_t *record,
    unsigned x, unsigned y, unsigned record_offset, bool alternate) {
    const unsigned pc = alternate ? 0xb680u : 0xb65au;
    const unsigned offset = alternate ? 9u : 5u;
#define RECT_OPCODE(delta, fetched, cycles, data) \
    AppendItemRegionOpcode(plan, pc + delta, fetched, cycles, data)
    RECT_OPCODE(0u, 2u, 3u, 1u);
    RECT_OPCODE(2u, 3u, ItemRegionIndexedReadCycles(record_offset, offset), 1u);
    RECT_OPCODE(5u, 2u, x < record[offset] ? 3u : 2u, 0u);
    if (x < record[offset])
        return false;
    RECT_OPCODE(7u, 3u, ItemRegionIndexedReadCycles(record_offset, offset + 2u), 1u);
    RECT_OPCODE(10u, 2u, x >= record[offset + 2u] ? 3u : 2u, 0u);
    if (x >= record[offset + 2u])
        return false;
    RECT_OPCODE(12u, 2u, 3u, 1u);
    RECT_OPCODE(14u, 3u, ItemRegionIndexedReadCycles(record_offset, offset + 1u), 1u);
    RECT_OPCODE(17u, 2u, y < record[offset + 1u] ? 3u : 2u, 0u);
    if (y < record[offset + 1u])
        return false;
    RECT_OPCODE(19u, 3u, ItemRegionIndexedReadCycles(record_offset, offset + 3u), 1u);
    RECT_OPCODE(22u, 2u, y >= record[offset + 3u] ? 3u : 2u, 0u);
#undef RECT_OPCODE
    return y < record[offset + 3u];
}

static bool PlanItemRegion(const CpuState *cpu, ItemRegionPlan *plan) {
    const uint8_t *ram = cpu->ram;
    const unsigned x = ram[0x6bau], y = ram[0x6e2u];
    unsigned offset = ram[0xf002u] | (unsigned)ram[0xf003u] << 8;
    unsigned item = 0xffu;
    const uint8_t *selected = NULL;
    *plan = (ItemRegionPlan){0};
#define ITEM_OPCODE(pc, fetched, cycles, data) \
    AppendItemRegionOpcode(plan, pc, fetched, cycles, data)
    ITEM_OPCODE(0xb63bu, 1u, 3u, 1u);
    ITEM_OPCODE(0xb63cu, 3u, 4u, 1u);
    ITEM_OPCODE(0xb63fu, 2u, 3u, 1u);
    ITEM_OPCODE(0xb641u, 3u, 4u, 1u);
    ITEM_OPCODE(0xb644u, 2u, 3u, 1u);
    ITEM_OPCODE(0xb646u, 2u, 2u, 0u);
    ITEM_OPCODE(0xb648u, 1u, 3u, 1u);
    ITEM_OPCODE(0xb649u, 1u, 4u, 1u);
    ITEM_OPCODE(0xb64au, 2u, 3u, 0u);
    ITEM_OPCODE(0xb64cu, 3u, 5u, 2u);
    ITEM_OPCODE(0xb64fu, 2u, 2u, 0u);
    ITEM_OPCODE(0xb651u, 2u, 3u, 1u);
    for (unsigned inspected = 0u; inspected < ITEM_REGION_RECORD_LIMIT; ++inspected) {
        /* Keep previewed records within one WRAM bank. */
        if (offset > 0xff2u)
            return false;
        const uint8_t *record = ram + 0xf000u + offset;
        ITEM_OPCODE(0xb653u, 3u, ItemRegionIndexedReadCycles(offset, 0u), 1u);
        ITEM_OPCODE(0xb656u, 2u, 2u, 0u);
        ITEM_OPCODE(0xb658u, 2u, record[0] == 0xffu ? 3u : 2u, 0u);
        if (record[0] == 0xffu)
            break;
        bool alternate = false;
        bool found = PlanItemRegionRectangle(plan, record, x, y, offset, false);
        if (!found) {
            ITEM_OPCODE(0xb679u, 3u, ItemRegionIndexedReadCycles(offset, 9u), 1u);
            ITEM_OPCODE(0xb67cu, 2u, 2u, 0u);
            ITEM_OPCODE(0xb67eu, 2u, record[9] == 0xffu ? 3u : 2u, 0u);
            if (record[9] != 0xffu) {
                alternate = true;
                found = PlanItemRegionRectangle(plan, record, x, y, offset, true);
            }
        }
        if (found) {
            const unsigned pc = alternate ? 0xb698u : 0xb672u;
            ITEM_OPCODE(pc, 3u, ItemRegionIndexedReadCycles(offset, 13u), 1u);
            ITEM_OPCODE(pc + 3u, 2u, 3u, 1u);
            ITEM_OPCODE(pc + 5u, 2u, 3u, 0u);
            selected = record;
            item = record[13];
            break;
        }
        ITEM_OPCODE(0xb69fu, 2u, 3u, 0u);
        ITEM_OPCODE(0xb6a1u, 1u, 2u, 0u);
        ITEM_OPCODE(0xb6a2u, 1u, 2u, 0u);
        ITEM_OPCODE(0xb6a3u, 3u, 3u, 0u);
        ITEM_OPCODE(0xb6a6u, 1u, 2u, 0u);
        ITEM_OPCODE(0xb6a7u, 2u, 3u, 0u);
        ITEM_OPCODE(0xb6a9u, 2u, 3u, 0u);
        offset += 15u;
        if (inspected + 1u == ITEM_REGION_RECORD_LIMIT) {
            plan->landing = 0x8eb653u;
            return true;
        }
    }
    ITEM_OPCODE(0xb6abu, 2u, 3u, 1u);
    ITEM_OPCODE(0xb6adu, 2u, 2u, 0u);
    ITEM_OPCODE(0xb6afu, 1u, 2u, 0u);
    ITEM_OPCODE(0xb6b0u, 2u, item == 0xffu ? 2u : 3u, 0u);
    if (item == 0xffu) {
        ITEM_OPCODE(0xb6b2u, 3u, 4u, 0u);
    } else {
        const unsigned facing = ram[0x1d09du];
        ITEM_OPCODE(0xb6b5u, 4u, 5u, 1u);
        ITEM_OPCODE(0xb6b9u, 2u, 2u, 0u);
        ITEM_OPCODE(0xb6bbu, 2u, facing == 0u ? 3u : 2u, 0u);
        bool accepted;
        assert(selected);
        if (facing == 0u) {
            ITEM_OPCODE(0xb6c4u, 2u, 3u, 1u);
            ITEM_OPCODE(0xb6c6u, 3u, ItemRegionIndexedReadCycles(offset, 2u), 1u);
            accepted = y >= selected[2];
            ITEM_OPCODE(0xb6c9u, 2u, accepted ? 2u : 3u, 0u);
            if (accepted)
                ITEM_OPCODE(0xb6cbu, 2u, 3u, 0u);
        } else {
            ITEM_OPCODE(0xb6bdu, 2u, 2u, 0u);
            ITEM_OPCODE(0xb6bfu, 2u, facing == 1u ? 3u : 2u, 0u);
            if (facing == 1u) {
                ITEM_OPCODE(0xb6cdu, 3u, ItemRegionIndexedReadCycles(offset, 2u), 1u);
                ITEM_OPCODE(0xb6d0u, 2u, 3u, 1u);
                accepted = selected[2] >= y;
                ITEM_OPCODE(0xb6d2u, 2u, accepted ? 2u : 3u, 0u);
            } else {
                ITEM_OPCODE(0xb6c1u, 1u, 2u, 0u);
                ITEM_OPCODE(0xb6c2u, 2u, 3u, 0u);
                accepted = false;
            }
        }
        if (accepted) {
            ITEM_OPCODE(0xb6d4u, 3u, ItemRegionIndexedReadCycles(offset, 0u), 1u);
            ITEM_OPCODE(0xb6d7u, 4u, 5u, 1u);
            ITEM_OPCODE(0xb6dbu, 2u, 2u, 0u);
            ITEM_OPCODE(0xb6ddu, 3u, 3u, 0u);
            plan->landing = 0x8eb6e0u;
            return true;
        }
    }
    ITEM_OPCODE(0xb738u, 1u, 4u, 1u);
#undef ITEM_OPCODE
    plan->landing = 0x8eb739u;
    return true;
}

static bool ItemRegionWindowSupported(const CpuState *cpu, unsigned reserve) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x8eu || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->host_return_valid != 3u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x8eb63bu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x8eb63bu, 0x1ffcu, false, 1363u, reserve) ||
        !g_snes->apu || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x73739u ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit || g_snes->vPos >= 262u)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedItemRegion(CpuState *cpu) {
    ItemRegionPlan plan;
    const uint32_t return_pc = Lufia2DecompFramePairedReturnPc(cpu, 0x8eb63bu);
    if (!ItemRegionWindowSupported(cpu, 0u) ||
        (return_pc != 0x83c217u && return_pc != 0x83bd62u) ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) !=
            (uint16_t)(return_pc - 1u) || cpu->ram[cpu->S + 3u] != 0x83u ||
        !Lufia2DecompFrameRangeQuiet(0x8eb63bu, 0x8eb6e0u) ||
        !Lufia2DecompFrameRangeQuiet(0x8eb738u, 0x8eb73au) ||
        !PlanItemRegion(cpu, &plan) ||
        !ItemRegionWindowSupported(cpu, FieldMapClockReserve(plan.opcodes, plan.count))) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
        return ActorBridgeFallback(cpu, &frame, 0x8eb63bu);
    }
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, return_pc};
    PairedSpanClock clock = BeginPairedSpanClock(cpu, 0x8eb63bu, plan.opcodes, plan.count);
    const Lufia2Memory memory = {PairedSpanRead, PairedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldItemRegionProbeStep(
        &memory, &state, 0x8eb63bu, ITEM_REGION_RECORD_LIMIT);
    PreparePairedSpan(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x8eb739u);
    bool progress = false;
    const RecompReturn returned = ReturnTimedLongCallWithProgress(
        cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x8eb63bu, 1u, progress);
    return returned;
}
