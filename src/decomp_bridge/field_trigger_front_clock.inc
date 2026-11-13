enum { FIELD_TRIGGER_FRONT_MAX_OPCODES = 20u };

static void ActorBridgeFieldTriggerFront(CpuState *cpu, uint32_t entry);

static bool FieldTriggerFrontHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x8381c6u || pc == 0x8381cfu || pc == 0x8381dbu || pc == 0x8381fau) &&
        hook == ActorBridgeFieldTriggerFront;
}

static unsigned FieldTriggerFrontReserve(uint32_t entry) {
    return entry == 0x8381dbu ? 416u : entry == 0x8381fau ? 576u : 192u;
}

static unsigned PlanFieldTriggerFrontClocks(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define FRONT_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x8381c6u) {
        FRONT_OPCODE(0x81c6u, 1u, 3u, 1u);
        FRONT_OPCODE(0x81c7u, 2u, 3u, 0u);
        FRONT_OPCODE(0x81c9u, 2u, 3u, 0u);
        return count;
    }
    if (entry == 0x8381cfu) {
        const bool contact = (cpu->ram[0x09a7u] & 1u) != 0u;
        FRONT_OPCODE(0x81cfu, 3u, 4u, 1u);
        FRONT_OPCODE(0x81d2u, 2u, 2u, 0u);
        FRONT_OPCODE(0x81d4u, 2u, contact ? 2u : 3u, 0u);
        if (contact)
            FRONT_OPCODE(0x81d6u, 2u, 3u, 0u);
        return count;
    }
    const uint8_t control = cpu->ram[0x1d0a1u];
    if (entry == 0x8381dbu) {
        FRONT_OPCODE(0x81dbu, 2u, 3u, 0u);
        FRONT_OPCODE(0x81ddu, 2u, cpu->_flag_Z ? 2u : 3u, 0u);
        if (!cpu->_flag_Z)
            return count;
        FRONT_OPCODE(0x81dfu, 4u, 5u, 1u);
        FRONT_OPCODE(0x81e3u, 2u, 2u, 0u);
        FRONT_OPCODE(0x81e5u, 2u, control & 0x3cu ? 3u : 2u, 0u);
        if (control & 0x3cu)
            return count;
        FRONT_OPCODE(0x81e7u, 1u, 3u, 1u);
        FRONT_OPCODE(0x81e8u, 3u, 4u, 1u);
        FRONT_OPCODE(0x81ebu, 2u, 3u, 1u);
        FRONT_OPCODE(0x81edu, 3u, 4u, 1u);
        FRONT_OPCODE(0x81f0u, 2u, 3u, 1u);
        FRONT_OPCODE(0x81f2u, 2u, 2u, 0u);
        FRONT_OPCODE(0x81f4u, 1u, 3u, 1u);
        FRONT_OPCODE(0x81f5u, 1u, 4u, 1u);
        return count;
    }
    const uint8_t actor = cpu->ram[0x0622u];
    FRONT_OPCODE(0x81fau, 3u, 4u, 1u);
    FRONT_OPCODE(0x81fdu, 2u, 2u, 0u);
    FRONT_OPCODE(0x81ffu, 2u, actor & 0x40u ? 2u : 3u, 0u);
    if (!(actor & 0x40u))
        return count;
    FRONT_OPCODE(0x8201u, 2u, 2u, 0u);
    FRONT_OPCODE(0x8203u, 2u, actor & 0x80u ? 3u : 2u, 0u);
    if (actor & 0x80u)
        return count;
    FRONT_OPCODE(0x8205u, 4u, 5u, 1u);
    FRONT_OPCODE(0x8209u, 2u, 2u, 0u);
    FRONT_OPCODE(0x820bu, 2u, control & 4u ? 3u : 2u, 0u);
    if (!(control & 4u)) {
        FRONT_OPCODE(0x820du, 2u, 2u, 0u);
        FRONT_OPCODE(0x820fu, 2u, control & 0x38u ? 3u : 2u, 0u);
        if (control & 0x38u)
            return count;
        const bool busy = (cpu->ram[0x09a8u] & 8u) != 0u;
        FRONT_OPCODE(0x8211u, 3u, 4u, 1u);
        FRONT_OPCODE(0x8214u, 2u, 2u, 0u);
        FRONT_OPCODE(0x8216u, 2u, busy ? 3u : 2u, 0u);
        if (busy)
            return count;
    }
    const bool blocked = (cpu->ram[0x05b5u] & 2u) != 0u;
    FRONT_OPCODE(0x8218u, 3u, 4u, 1u);
    FRONT_OPCODE(0x821bu, 2u, 2u, 0u);
    FRONT_OPCODE(0x821du, 2u, blocked ? 3u : 2u, 0u);
    if (blocked)
        return count;
    const bool pending = cpu->ram[0x1d0a3u] != 0u;
    FRONT_OPCODE(0x821fu, 4u, 5u, 1u);
    FRONT_OPCODE(0x8223u, 2u, pending ? 2u : 3u, 0u);
    if (pending) {
        FRONT_OPCODE(0x8225u, 2u, 2u, 0u);
        FRONT_OPCODE(0x8227u, 3u, 6u, 2u);
    }
#undef FRONT_OPCODE
    return count;
}

static bool FieldTriggerFrontClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x8381c6u && entry != 0x8381cfu &&
        entry != 0x8381dbu && entry != 0x8381fau)
        return false;
    const unsigned reserve = FieldTriggerFrontReserve(entry);
    const unsigned stack_limit = entry == 0x8381c6u ? 0x1ffdu : 0x1ffcu;
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        (entry != 0x8381c6u && cpu->x_flag != (entry == 0x8381dbu)) ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        cpu->S < 0x1f20u || cpu->S > stack_limit || g_interp_bridge_pc_hook ||
        !g_snes || !g_snes->dma || !g_snes->apu || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0x1823bu || g_snes->vPos >= 262u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, stack_limit, false, 1363u, reserve) ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void ActorBridgeFieldTriggerFront(CpuState *cpu, uint32_t entry) {
    if (!FieldTriggerFrontClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[FIELD_TRIGGER_FRONT_MAX_OPCODES];
    const unsigned count = PlanFieldTriggerFrontClocks(cpu, entry, timing);
    const uint32_t end = entry == 0x8381c6u ? 0x8381cbu : entry == 0x8381cfu
        ? 0x8381d8u : entry == 0x8381dbu ? 0x8381f6u : 0x83822au;
    if (Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeFieldTriggerFront) < count)
        return;
    const uint64_t start = cpu->master_cycles;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x8381c6u
        ? Lufia2FieldPrepareTriggerChecks(&memory, &state) : entry == 0x8381cfu
        ? Lufia2FieldSelectContact(&memory, &state) : entry == 0x8381dbu
        ? Lufia2FieldPrepareContactScan(&memory, &state)
        : Lufia2FieldPreparePendingEvent(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= FieldTriggerFrontReserve(entry));
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
