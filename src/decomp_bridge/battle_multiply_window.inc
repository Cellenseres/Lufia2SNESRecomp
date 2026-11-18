typedef struct BattleMultiplyWindowClock {
    FixedSpanClock span;
    uint32_t word_address;
    uint16_t word;
    bool read_high, write_high;
} BattleMultiplyWindowClock;
// Keep register word transactions through the model's byte callbacks.
static void CompleteMultiplyWordByte(BattleMultiplyWindowClock *clock, uint32_t address) {
    clock->span.data_master += cpu_region_speed(address);
    if (!--clock->span.remaining)
        FinishFixedSpanOpcode(&clock->span);
}

static uint8_t BattleMultiplyWindowRead(void *context, uint32_t address) {
    BattleMultiplyWindowClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    if (clock->read_high) {
        assert(address == clock->word_address + 1u && clock->span.remaining == 1u);
        clock->read_high = false;
        CompleteMultiplyWordByte(clock, address);
        return (uint8_t)(clock->word >> 8u);
    }
    if ((uint16_t)address == 0x4216u && clock->span.remaining == 2u) {
        bool progress = false;
        clock->word = Lufia2DecompFrameReturnReadWord(
            clock->span.cpu, address, address + 1u, &progress);
        clock->word_address = address;
        clock->read_high = true;
        clock->span.progress |= progress;
        CompleteMultiplyWordByte(clock, address);
        return (uint8_t)clock->word;
    }
    return FixedSpanRead(&clock->span, address);
}

static void BattleMultiplyWindowWrite(void *context, uint32_t address, uint8_t value) {
    BattleMultiplyWindowClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    if (clock->write_high) {
        assert(address == clock->word_address + 1u && clock->span.remaining == 1u);
        bool progress = false;
        Lufia2DecompFrameReturnWriteWord(clock->span.cpu, clock->word_address, address,
            clock->word | ((uint16_t)value << 8u), &progress);
        clock->span.progress |= progress;
        clock->write_high = false;
        CompleteMultiplyWordByte(clock, address);
        return;
    }
    if ((uint16_t)address == 0x4202u && clock->span.remaining == 2u) {
        clock->word = value;
        clock->word_address = address;
        clock->write_high = true;
        CompleteMultiplyWordByte(clock, address);
        return;
    }
    FixedSpanWrite(&clock->span, address, value);
}

static void ActorBridgeBattleMultiplyWindow(CpuState *cpu, uint32_t entry) {
    enum { MULTIPLY_WINDOW_BUDGET = 1104u };
    const unsigned count = sizeof battle_multiply_steps / sizeof battle_multiply_steps[0] - 1u;
    if (!BattleMultiplyHookOwned(entry, ActorBridgeBattleMultiplyWindow) || cpu != &g_cpu ||
        cpu->PB != 0x80u || cpu->D || cpu->_flag_D || cpu->m_flag != 1u || cpu->x_flag ||
        (cpu->DB & 0x7fu) >= 0x40u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !NativeOpcodeWindowSupported(cpu, entry, MULTIPLY_WINDOW_BUDGET, 0x1fffu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, 0x808377u, BattleMultiplyHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, 0x808377u, ActorBridgeBattleMultiplyWindow) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 32u, &fetch);
    BattleMultiplyWindowClock clock = {0};
    BeginBattleMultiply(&clock.span, cpu, false);
    clock.span.opcode_span = prepared ? &fetch : NULL;
    const Lufia2Memory memory = {
        BattleMultiplyWindowRead, BattleMultiplyWindowWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2Multiply16By8(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x808377u);
    assert(clock.span.index == count && !clock.span.remaining &&
        !clock.read_high && !clock.write_high);
    assert(cpu->master_cycles - start <= MULTIPLY_WINDOW_BUDGET);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
