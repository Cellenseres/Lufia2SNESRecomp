enum { FIELD_IDLE_CLOCK_RESERVE = 2048u, FIELD_IDLE_BODY_MAX_OPCODES = 58u };

_Static_assert(FIELD_IDLE_BODY_MAX_OPCODES + 2u <= LUFIA2_FIELD_IDLE_OPCODE_BUDGET,
    "review field idle and original caller budget");

static unsigned PlanFieldIdleClocks(const uint8_t *ram, FixedSpanOpcode *plan) {
    static const uint16_t addresses[] = {0x09a8u, 0x0622u, 0x05b7u, 0x05b5u, 0x17aau, 0x099bu};
    static const uint8_t masks[] = {8u, 0x88u, 7u, 0xa2u, 0xffu, 0x80u};
    static const uint16_t loads[] = {0x80cdu, 0x80d4u, 0x80dbu, 0x80e2u, 0x80e9u, 0x80eeu};
    static const uint16_t tests[] = {0x80d0u, 0x80d7u, 0x80deu, 0x80e5u, 0u, 0x80f1u};
    static const uint16_t branches[] = {0x80d2u, 0x80d9u, 0x80e0u, 0x80e7u, 0x80ecu, 0x80f3u};
    unsigned count = 0u;
#define IDLE_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    for (unsigned gate = 0u; gate < 6u; ++gate) {
        const bool busy = (ram[addresses[gate]] & masks[gate]) != 0u;
        IDLE_OPCODE(loads[gate], 3u, 4u, 1u);
        if (tests[gate])
            IDLE_OPCODE(tests[gate], 2u, 2u, 0u);
        IDLE_OPCODE(branches[gate], 2u, busy ? 3u : 2u, 0u);
        if (busy)
            return count;
    }
    IDLE_OPCODE(0x80f5u, 2u, 2u, 0u);
    for (unsigned slot = 0u; slot < 8u; ++slot) {
        const bool busy = (ram[0x1d057u + slot] & 0x80u) != 0u;
        IDLE_OPCODE(0x80f7u, 4u, 5u, 1u);
        IDLE_OPCODE(0x80fbu, 2u, busy ? 3u : 2u, 0u);
        if (busy)
            return count;
        IDLE_OPCODE(0x80fdu, 1u, 2u, 0u);
        IDLE_OPCODE(0x80feu, 2u, 2u, 0u);
        IDLE_OPCODE(0x8100u, 2u, slot == 7u ? 2u : 3u, 0u);
    }
#undef IDLE_OPCODE
    assert(count <= FIELD_IDLE_BODY_MAX_OPCODES);
    return count;
}

static bool FieldIdleClockSupported(const CpuState *cpu) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || !ActorBridgeSystemBank(cpu->DB) ||
        cpu->m_flag != 1u || cpu->x_flag != 1u || cpu->host_return_valid != 2u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu || g_interp_apu_driving ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x8380cdu) ||
        Lufia2DecompFramePairedReturnPc(cpu, 0x8380cdu) == UINT32_MAX ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIELD_IDLE_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x8380cdu, 0x1ffdu,
            false, 1363u, FIELD_IDLE_CLOCK_RESERVE) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x18102u ||
        !Lufia2DecompFrameRangeQuiet(0x8380cdu, 0x838103u))
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < FIELD_IDLE_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedFieldIdle(CpuState *cpu) {
    if (!FieldIdleClockSupported(cpu)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x80cdu;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, 0x8380cdu)};
    FixedSpanOpcode plan[FIELD_IDLE_BODY_MAX_OPCODES];
    const unsigned count = PlanFieldIdleClocks(cpu->ram, plan);
    PairedSpanClock clock = BeginPairedSpanClock(cpu, 0x8380cdu, plan, count);
    const Lufia2Memory memory = {PairedSpanRead, PairedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldIdleTest(&memory, &state);
    PreparePairedSpan(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x838102u);
    assert(clock.offset == count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    bool return_progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(cpu, &frame, result.pc, &return_progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x8380cdu, 1u, return_progress);
    assert(cpu->master_cycles - start <= FIELD_IDLE_CLOCK_RESERVE);
    return returned;
}
