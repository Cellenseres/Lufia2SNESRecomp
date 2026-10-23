enum { RIPPLE_CLOCK_OPCODES = 277u, RIPPLE_CLOCK_BUDGET = 8000u };

static const FixedSpanOpcode ripple_prefix[] = {
    {0xa736u, 1u, 3u, 1u}, {0xa737u, 2u, 2u, 0u},
    {0xa739u, 1u, 3u, 1u}, {0xa73au, 1u, 4u, 1u},
    {0xa73bu, 1u, 2u, 0u}, {0xa73cu, 3u, 4u, 1u},
    {0xa73fu, 1u, 2u, 0u}, {0xa740u, 1u, 2u, 0u},
    {0xa741u, 3u, 4u, 1u}, {0xa744u, 2u, 2u, 0u},
    {0xa746u, 1u, 2u, 0u}, {0xa747u, 1u, 4u, 2u},
    {0xa748u, 3u, 3u, 0u}, {0xa74bu, 2u, 2u, 0u},
    {0xa74du, 2u, 3u, 1u}
};

static const FixedSpanOpcode ripple_row[] = {
    {0xa74fu, 3u, 4u, 1u}, {0xa752u, 1u, 2u, 0u},
    {0xa753u, 3u, 4u, 1u}, {0xa756u, 4u, 5u, 1u},
    {0xa75au, 1u, 2u, 0u}, {0xa75bu, 1u, 2u, 0u},
    {0xa75cu, 2u, 5u, 2u}, {0xa75eu, 2u, 3u, 0u}
};

static const FixedSpanOpcode ripple_suffix[] = {
    {0xa760u, 1u, 5u, 2u}, {0xa761u, 3u, 6u, 2u},
    {0xa764u, 3u, 6u, 2u}, {0xa767u, 2u, 2u, 0u},
    {0xa769u, 3u, 4u, 1u}, {0xa76cu, 1u, 4u, 1u}
};

typedef struct RippleClock {
    CpuState *cpu;
    FixedSpanClock span;
    FixedSpanOpcode row_timing[8];
    unsigned stage, rows, opcodes;
} RippleClock;

static void RippleClockSpan(RippleClock *clock,
                             const FixedSpanOpcode *timing, unsigned count) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->cpu;
    clock->span.bank = 0x850000u;
    clock->span.timing = timing;
    clock->span.count = count;
}

static void RippleClockPrepare(RippleClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count)
        return;
    clock->opcodes += clock->span.count;
    if (!clock->stage) {
        clock->stage = 1u;
        memcpy(clock->row_timing, ripple_row, sizeof ripple_row);
    } else {
        assert(clock->stage == 1u);
        ++clock->rows;
    }
    if (clock->rows == 32u) {
        clock->stage = 2u;
        RippleClockSpan(clock, ripple_suffix, 6u);
    } else {
        clock->row_timing[7].cycles = clock->rows == 31u ? 2u : 3u;
        RippleClockSpan(clock, clock->row_timing, 8u);
    }
}

static uint8_t RippleClockRead(void *context, uint32_t address) {
    RippleClock *clock = context;
    RippleClockPrepare(clock);
    return FixedSpanRead(&clock->span, address);
}

static void RippleClockWrite(void *context, uint32_t address, uint8_t value) {
    RippleClock *clock = context;
    RippleClockPrepare(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static void ActorBridgeRippleRowClock(CpuState *cpu, uint32_t entry) {
    if (!NmiTimerClockSupported(cpu, entry, 277u, RIPPLE_CLOCK_BUDGET))
        return;
    RippleClock clock = {0};
    clock.cpu = cpu;
    RippleClockSpan(&clock, ripple_prefix, 15u);
    const Lufia2Memory memory = {
        RippleClockRead, RippleClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleRippleRow(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x85a76du);
    assert(clock.stage == 2u && clock.rows == 32u &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    clock.opcodes += clock.span.count;
    assert(clock.opcodes == RIPPLE_CLOCK_OPCODES);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, 0u);
    interp_bridge_pre_opcode_redirect(result.pc);
}
