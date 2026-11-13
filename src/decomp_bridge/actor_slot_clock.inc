enum { ACTOR_SLOT_PHASE_CLOCK_RESERVE = 512u, ACTOR_SLOT_PHASE_MAX_OPCODES = 15u };

static void ActorBridgeActorSlotPhase(CpuState *cpu, uint32_t entry);

static bool ActorSlotPhaseHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x83bba5u || pc == 0x83bbd1u) && hook == ActorBridgeActorSlotPhase;
}

static unsigned PlanActorSlotPhaseClocks(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define SLOT_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    const unsigned slot = cpu->ram[0xa7u];
    if (entry == 0x83bbd1u) {
        SLOT_OPCODE(0xbbd1u, 2u, 3u, 1u);
        SLOT_OPCODE(0xbbd3u, 1u, 2u, 0u);
        SLOT_OPCODE(0xbbd4u, 2u, 3u, 1u);
        SLOT_OPCODE(0xbbd6u, 2u, 2u, 0u);
        SLOT_OPCODE(0xbbd8u, 2u, slot == 39u ? 2u : 3u, 0u);
        return count;
    }
    const uint8_t state = cpu->ram[0x0622u + slot];
    SLOT_OPCODE(0xbba5u, 2u, cpu->x_flag ? 3u : 4u, cpu->x_flag ? 1u : 2u);
    SLOT_OPCODE(0xbba7u, 3u, 4u, 1u);
    SLOT_OPCODE(0xbbaau, 2u, 2u, 0u);
    SLOT_OPCODE(0xbbacu, 2u, state & 4u ? 3u : 2u, 0u);
    if (state & 4u)
        return count;
    SLOT_OPCODE(0xbbaeu, 2u, 2u, 0u);
    SLOT_OPCODE(0xbbb0u, 2u, state & 0x18u ? 3u : 2u, 0u);
    if (!(state & 0x18u)) {
        SLOT_OPCODE(0xbbb2u, 2u, 2u, 0u);
        SLOT_OPCODE(0xbbb4u, 2u, state & 1u ? 3u : 2u, 0u);
        if (state & 1u)
            return count;
        SLOT_OPCODE(0xbbb6u, 1u, 2u, 0u);
        SLOT_OPCODE(0xbbb7u, 2u, slot ? 3u : 2u, 0u);
        if (!slot)
            return count;
    }
    const bool blocked = (cpu->ram[0x1d0a1u] & 0x2cu) != 0u;
    SLOT_OPCODE(0xbbbeu, 4u, 5u, 1u);
    SLOT_OPCODE(0xbbc2u, 2u, 2u, 0u);
    SLOT_OPCODE(0xbbc4u, 2u, blocked ? 2u : 3u, 0u);
    if (blocked) {
        SLOT_OPCODE(0xbbc6u, 1u, 2u, 0u);
        SLOT_OPCODE(0xbbc7u, 2u, slot ? 3u : 2u, 0u);
    }
#undef SLOT_OPCODE
    return count;
}

static bool ActorSlotPhaseClockSupported(const CpuState *cpu, uint32_t entry) {
    if ((entry != 0x83bba5u && entry != 0x83bbd1u) || cpu != &g_cpu ||
        !cpu->ram || cpu->emulation || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u ||
        cpu->x_flag > 1u || cpu->ram[0xa7u] >= 40u ||
        (!cpu->x_flag && cpu->ram[0xa8u]) || g_interp_bridge_pc_hook ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        !g_snes || !g_snes->dma || !Lufia2DecompFrameReturnCpuReady() || !g_snes->apu ||
        !g_snes->cart || !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0x1bbdau || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - ACTOR_SLOT_PHASE_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu,
            false, 1363u, ACTOR_SLOT_PHASE_CLOCK_RESERVE) ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < ACTOR_SLOT_PHASE_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void ActorBridgeActorSlotPhase(CpuState *cpu, uint32_t entry) {
    if (!ActorSlotPhaseClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[ACTOR_SLOT_PHASE_MAX_OPCODES];
    const unsigned count = PlanActorSlotPhaseClocks(cpu, entry, timing);
    const uint32_t end = entry == 0x83bba5u ? 0x83bbd1u : 0x83bbdau;
    if (Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeActorSlotPhase) < count)
        return;
    const uint64_t start = cpu->master_cycles;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x83bba5u
        ? Lufia2ActorSelectSlotUpdate(&memory, &state)
        : Lufia2ActorAdvanceSlot(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= ACTOR_SLOT_PHASE_CLOCK_RESERVE);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
