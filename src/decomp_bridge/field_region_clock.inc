enum { FIELD_REGION_MAX_OPCODES = 19u };

static void ActorBridgeFieldRegionPhase(CpuState *cpu, uint32_t entry);

static bool FieldRegionHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x83823bu || pc == 0x83826bu || pc == 0x838285u) &&
        hook == ActorBridgeFieldRegionPhase;
}

static unsigned PlanFieldRegionClocks(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define REGION_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x838285u) {
        const uint8_t value = (uint8_t)cpu->A;
        REGION_OPCODE(0x8285u, 2u, 2u, 0u);
        REGION_OPCODE(0x8287u, 2u, value == 0x0bu ? 2u : 3u, 0u);
        if (value == 0x0bu)
            return count;
        REGION_OPCODE(0x828eu, 2u, 2u, 0u);
        REGION_OPCODE(0x8290u, 2u, value == 0x0cu ? 2u : 3u, 0u);
        if (value == 0x0cu)
            return count;
        REGION_OPCODE(0x8297u, 2u, 2u, 0u);
        REGION_OPCODE(0x8299u, 2u, value == 0x0du ? 2u : 3u, 0u);
        return count;
    }
    const uint8_t actor = cpu->ram[0x0622u], control = cpu->ram[0x1d0a1u];
    if (entry == 0x83826bu) {
        REGION_OPCODE(0x826bu, 3u, 4u, 1u);
        REGION_OPCODE(0x826eu, 2u, 2u, 0u);
        REGION_OPCODE(0x8270u, 2u, actor & 1u ? 3u : 2u, 0u);
        if (actor & 1u)
            return count;
        REGION_OPCODE(0x8272u, 4u, 5u, 1u);
        REGION_OPCODE(0x8276u, 2u, 2u, 0u);
        REGION_OPCODE(0x8278u, 2u, control & 4u ? 3u : 2u, 0u);
        return count;
    }
    const bool enabled = (cpu->ram[0x05b5u] & 0x10u) != 0u;
    REGION_OPCODE(0x823bu, 3u, 4u, 1u);
    REGION_OPCODE(0x823eu, 2u, 2u, 0u);
    REGION_OPCODE(0x8240u, 2u, enabled ? 2u : 3u, 0u);
    if (!enabled)
        return count;
    REGION_OPCODE(0x8242u, 3u, 4u, 1u);
    REGION_OPCODE(0x8245u, 2u, 2u, 0u);
    REGION_OPCODE(0x8247u, 2u, actor & 0x80u ? 3u : 2u, 0u);
    if (actor & 0x80u)
        return count;
    REGION_OPCODE(0x8249u, 4u, 5u, 1u);
    REGION_OPCODE(0x824du, 2u, 2u, 0u);
    REGION_OPCODE(0x824fu, 2u, control & 8u ? 3u : 2u, 0u);
    if (control & 8u)
        return count;
    REGION_OPCODE(0x8251u, 2u, 2u, 0u);
    REGION_OPCODE(0x8253u, 3u, 6u, 2u);
    REGION_OPCODE(0x8256u, 3u, 4u, 1u);
    REGION_OPCODE(0x8259u, 2u, 3u, 1u);
    REGION_OPCODE(0x825bu, 3u, 4u, 1u);
    REGION_OPCODE(0x825eu, 2u, 3u, 1u);
    REGION_OPCODE(0x8260u, 1u, 3u, 1u);
    REGION_OPCODE(0x8261u, 2u, 2u, 0u);
    REGION_OPCODE(0x8263u, 1u, 3u, 1u);
    REGION_OPCODE(0x8264u, 1u, 4u, 1u);
#undef REGION_OPCODE
    return count;
}

static bool FieldRegionClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x83823bu && entry != 0x83826bu && entry != 0x838285u)
        return false;
    const unsigned reserve = entry == 0x83823bu ? 576u : 256u;
    const uint8_t bank = entry == 0x83826bu ? 0x7eu : 0x83u;
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != bank || cpu->m_flag != 1u || cpu->x_flag ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu || g_interp_bridge_pc_hook ||
        !g_snes || !g_snes->dma || !g_snes->apu || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0x1829eu || g_snes->vPos >= 262u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, reserve) ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void ActorBridgeFieldRegionPhase(CpuState *cpu, uint32_t entry) {
    if (!FieldRegionClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[FIELD_REGION_MAX_OPCODES];
    const unsigned count = PlanFieldRegionClocks(cpu, entry, timing);
    const uint32_t end = entry == 0x83823bu ? 0x838265u
        : entry == 0x83826bu ? 0x83827au : 0x83829eu;
    if (Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeFieldRegionPhase) < count)
        return;
    const uint64_t start = cpu->master_cycles;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x83823bu
        ? Lufia2FieldPrepareRegionProbe(&memory, &state)
        : entry == 0x83826bu ? Lufia2FieldSelectRegionScript(&memory, &state)
        : Lufia2FieldSelectRegionScene(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= (entry == 0x83823bu ? 576u : 256u));
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
