enum { BATTLE_MULTIPLY_CYCLE_LIMIT = 128u, BATTLE_DIVIDE_CYCLE_LIMIT = 512u };

static const FixedSpanOpcode battle_multiply_steps[] = {
    {0x834cu, 1u, 3u, 1u}, {0x834du, 1u, 4u, 2u}, {0x834eu, 1u, 3u, 1u},
    {0x834fu, 2u, 3u, 0u}, {0x8351u, 2u, 3u, 1u}, {0x8353u, 2u, 3u, 1u},
    {0x8355u, 3u, 4u, 1u}, {0x8358u, 2u, 3u, 1u}, {0x835au, 3u, 4u, 1u},
    {0x835du, 2u, 3u, 1u}, {0x835fu, 1u, 3u, 0u}, {0x8360u, 2u, 3u, 1u},
    {0x8362u, 2u, 3u, 0u}, {0x8364u, 3u, 5u, 2u}, {0x8367u, 3u, 5u, 2u},
    {0x836au, 2u, 4u, 2u}, {0x836cu, 2u, 4u, 2u}, {0x836eu, 1u, 2u, 0u},
    {0x836fu, 3u, 5u, 2u}, {0x8372u, 2u, 4u, 2u}, {0x8374u, 1u, 4u, 1u},
    {0x8375u, 1u, 5u, 2u}, {0x8376u, 1u, 4u, 1u}, {0x8377u, 1u, 6u, 3u}};

static void BeginBattleMultiply(FixedSpanClock *clock, CpuState *cpu,
                                bool inline_return) {
    *clock = (FixedSpanClock){0};
    clock->cpu = cpu;
    clock->bank = 0x800000u;
    clock->timing = battle_multiply_steps;
    clock->count =
        sizeof battle_multiply_steps / sizeof(FixedSpanOpcode) - !inline_return;
}

static RecompReturn ActorBridgeTimedBattleMultiply(CpuState *cpu) {
    const uint32_t entry = 0x80834cu;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned master = BATTLE_MULTIPLY_CYCLE_LIMIT * 8u;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (cpu->D || cpu->_flag_D || cpu->m_flag != 1u || (cpu->DB & 0x7fu) >= 0x40u ||
        !BattleQuietClockSupported(cpu, entry, 0x808378u, 3u, false, room))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 0u ? 0x00834cu : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    FixedSpanClock clock;
    BeginBattleMultiply(&clock, cpu, false);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2Multiply16By8(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x808377u);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}

typedef enum BattleDividePhase {
    DIVIDE_BEGIN,
    DIVIDE_SHIFT,
    DIVIDE_COMPARE,
    DIVIDE_SUBTRACT_CARRY,
    DIVIDE_SUBTRACT_COMPARE,
    DIVIDE_SKIP,
    DIVIDE_FINISH
} BattleDividePhase;

typedef struct BattleDivideClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[3];
    Lufia2CpuState *state;
    BattleDividePhase phase;
    unsigned bit;
    bool inline_return;
} BattleDivideClock;

static void SetBattleDividePhase(BattleDivideClock *clock, BattleDividePhase phase) {
    const uint16_t base = (uint16_t)(0x837eu + clock->bit * 13u);
    FixedSpanOpcode *plan = clock->timing;
    unsigned count;
    switch (phase) {
    case DIVIDE_BEGIN:
        plan[0] = (FixedSpanOpcode){0x8378u, 1u, 3u, 1u};
        plan[1] = (FixedSpanOpcode){0x8379u, 2u, 3u, 0u};
        plan[2] = (FixedSpanOpcode){0x837bu, 3u, 3u, 0u};
        count = 3u;
        break;
    case DIVIDE_SHIFT:
        plan[0] = (FixedSpanOpcode){base, 2u, 7u, 4u};
        plan[1] = (FixedSpanOpcode){base + 2u, 1u, 2u, 0u};
        count = 2u;
        break;
    case DIVIDE_COMPARE:
        plan[0] = (FixedSpanOpcode){base + 3u, 2u, 2u, 0u};
        plan[1] = (FixedSpanOpcode){base + 5u, 2u, 4u, 2u};
        count = 2u;
        break;
    case DIVIDE_SUBTRACT_CARRY:
    case DIVIDE_SUBTRACT_COMPARE:
        plan[0] =
            (FixedSpanOpcode){phase == DIVIDE_SUBTRACT_CARRY ? base + 3u : base + 7u,
                              2u, phase == DIVIDE_SUBTRACT_CARRY ? 3u : 2u, 0u};
        plan[1] = (FixedSpanOpcode){base + 9u, 2u, 4u, 2u};
        plan[2] = (FixedSpanOpcode){base + 11u, 2u, 7u, 4u};
        count = 3u;
        break;
    case DIVIDE_SKIP:
        plan[0] = (FixedSpanOpcode){base + 7u, 2u, 3u, 0u};
        count = 1u;
        break;
    default:
        assert(phase == DIVIDE_FINISH);
        plan[0] = (FixedSpanOpcode){0x844eu, 1u, 4u, 1u};
        plan[1] = (FixedSpanOpcode){0x844fu, 1u, 6u, 3u};
        count = clock->inline_return ? 2u : 1u;
        break;
    }
    clock->phase = phase;
    clock->span.timing = clock->timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static void PrepareBattleDivideAccess(BattleDivideClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count || clock->span.remaining)
            return;
        BattleDividePhase next;
        if (clock->phase == DIVIDE_BEGIN) {
            next = DIVIDE_SHIFT;
        } else if (clock->phase == DIVIDE_SHIFT) {
            next = clock->state->carry ? DIVIDE_SUBTRACT_CARRY : DIVIDE_COMPARE;
        } else if (clock->phase == DIVIDE_COMPARE) {
            next = clock->state->carry ? DIVIDE_SUBTRACT_COMPARE : DIVIDE_SKIP;
        } else {
            assert(clock->phase == DIVIDE_SKIP ||
                   clock->phase == DIVIDE_SUBTRACT_CARRY ||
                   clock->phase == DIVIDE_SUBTRACT_COMPARE);
            next = ++clock->bit == 16u ? DIVIDE_FINISH : DIVIDE_SHIFT;
        }
        SetBattleDividePhase(clock, next);
    }
}

static uint8_t BattleDivideRead(void *context, uint32_t address) {
    BattleDivideClock *clock = context;
    PrepareBattleDivideAccess(clock);
    return FixedSpanRead(&clock->span, address);
}

static void BattleDivideWrite(void *context, uint32_t address, uint8_t value) {
    BattleDivideClock *clock = context;
    PrepareBattleDivideAccess(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static void BeginBattleDivide(BattleDivideClock *clock, CpuState *cpu,
                              Lufia2CpuState *state, bool inline_return) {
    *clock = (BattleDivideClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x800000u;
    clock->state = state;
    clock->inline_return = inline_return;
    SetBattleDividePhase(clock, DIVIDE_BEGIN);
}

static RecompReturn ActorBridgeTimedBattleDivide(CpuState *cpu) {
    const uint32_t entry = 0x808378u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned master = BATTLE_DIVIDE_CYCLE_LIMIT * 8u;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (cpu->D || cpu->_flag_D ||
        !BattleQuietClockSupported(cpu, entry, 0x808450u, 3u, true, room))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 0u ? 0x008378u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    BattleDivideClock clock;
    BeginBattleDivide(&clock, cpu, &state, false);
    const Lufia2Memory memory = {BattleDivideRead, BattleDivideWrite, &clock, NULL,
                                 NULL};
    const Lufia2ExecutionResult result = Lufia2Divide16(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x80844fu);
    assert(clock.phase == DIVIDE_FINISH && clock.span.index == clock.span.count &&
           !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}
