static const FixedSpanOpcode battle_clear_begin[] = {
    {0xe7a6u, 1u, 3u, 1u}, {0xe7a7u, 2u, 2u, 0u},
    {0xe7a9u, 1u, 3u, 1u}, {0xe7aau, 1u, 4u, 1u},
    {0xe7abu, 3u, 5u, 2u}
};
static const FixedSpanOpcode battle_clear_width[] = {
    {0xe7aeu, 3u, 4u, 1u}
};
static const FixedSpanOpcode battle_clear_cell[] = {
    {0xe7b1u, 3u, 6u, 1u}, {0xe7b4u, 3u, 6u, 1u},
    {0xe7b7u, 1u, 2u, 0u}, {0xe7b8u, 1u, 2u, 0u},
    {0xe7b9u, 1u, 2u, 0u}, {0xe7bau, 2u, 3u, 0u}
};
static const FixedSpanOpcode battle_clear_last_cell[] = {
    {0xe7b1u, 3u, 6u, 1u}, {0xe7b4u, 3u, 6u, 1u},
    {0xe7b7u, 1u, 2u, 0u}, {0xe7b8u, 1u, 2u, 0u},
    {0xe7b9u, 1u, 2u, 0u}, {0xe7bau, 2u, 2u, 0u}
};
static const FixedSpanOpcode battle_clear_row[] = {
    {0xe7bcu, 2u, 3u, 0u}, {0xe7beu, 3u, 5u, 2u},
    {0xe7c1u, 1u, 2u, 0u}, {0xe7c2u, 3u, 3u, 0u},
    {0xe7c5u, 1u, 2u, 0u}, {0xe7c6u, 3u, 5u, 2u},
    {0xe7c9u, 2u, 3u, 0u}, {0xe7cbu, 3u, 6u, 2u},
    {0xe7ceu, 2u, 3u, 0u}
};
static const FixedSpanOpcode battle_clear_last_row[] = {
    {0xe7bcu, 2u, 3u, 0u}, {0xe7beu, 3u, 5u, 2u},
    {0xe7c1u, 1u, 2u, 0u}, {0xe7c2u, 3u, 3u, 0u},
    {0xe7c5u, 1u, 2u, 0u}, {0xe7c6u, 3u, 5u, 2u},
    {0xe7c9u, 2u, 3u, 0u}, {0xe7cbu, 3u, 6u, 2u},
    {0xe7ceu, 2u, 2u, 0u}
};
static const FixedSpanOpcode battle_clear_finish[] = {
    {0xe7d0u, 1u, 4u, 1u}
};

typedef enum BattleClearPhase {
    CLEAR_BEGIN, CLEAR_WIDTH, CLEAR_CELL, CLEAR_ROW, CLEAR_FINISH
} BattleClearPhase;

typedef struct BattleClearClock {
    FixedSpanClock span;
    BattleClearPhase phase;
    unsigned columns, cells_left, rows_left;
} BattleClearClock;

static void SetBattleClearPhase(BattleClearClock *clock, BattleClearPhase phase) {
    const FixedSpanOpcode *timing = battle_clear_finish;
    unsigned count = 1u;
    if (phase == CLEAR_BEGIN) {
        timing = battle_clear_begin;
        count = 5u;
    } else if (phase == CLEAR_WIDTH) {
        timing = battle_clear_width;
    } else if (phase == CLEAR_CELL) {
        timing = clock->cells_left == 1u ? battle_clear_last_cell : battle_clear_cell;
        count = 6u;
    } else if (phase == CLEAR_ROW) {
        timing = clock->rows_left == 1u ? battle_clear_last_row : battle_clear_row;
        count = 9u;
    }
    clock->phase = phase;
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static void PrepareBattleClearAccess(BattleClearClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count || clock->span.remaining)
        return;
    BattleClearPhase next;
    if (clock->phase == CLEAR_BEGIN) {
        next = CLEAR_WIDTH;
    } else if (clock->phase == CLEAR_WIDTH) {
        clock->cells_left = clock->columns;
        next = CLEAR_CELL;
    } else if (clock->phase == CLEAR_CELL) {
        next = --clock->cells_left ? CLEAR_CELL : CLEAR_ROW;
    } else {
        assert(clock->phase == CLEAR_ROW);
        next = --clock->rows_left ? CLEAR_WIDTH : CLEAR_FINISH;
    }
    SetBattleClearPhase(clock, next);
}

static uint8_t BattleClearRead(void *context, uint32_t address) {
    BattleClearClock *clock = context;
    PrepareBattleClearAccess(clock);
    return FixedSpanRead(&clock->span, address);
}

static void BattleClearWrite(void *context, uint32_t address, uint8_t value) {
    BattleClearClock *clock = context;
    PrepareBattleClearAccess(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool BattleClearOutputSupported(const CpuState *cpu,
    unsigned columns, unsigned rows) {
    const unsigned end = cpu->X + (rows - 1u) * 64u + columns * 2u;
    return end <= 0x10000u &&
        (end <= 0x09f2u || cpu->X >= 0x09f6u) &&
        (end <= 0x1efcu || cpu->X >= 0x2000u);
}

static RecompReturn ActorBridgeTimedBattleClear(CpuState *cpu) {
    const uint32_t entry = 0x81e7a6u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned columns = cpu->ram && cpu->ram[0x09f2u]
        ? cpu->ram[0x09f2u] : 256u;
    const unsigned rows = cpu->ram && cpu->ram[0x09f3u]
        ? cpu->ram[0x09f3u] : 256u;
    const unsigned master = 8u * (27u + rows * (36u + columns * 22u));
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (!cpu->ram || cpu->_flag_D || cpu->m_flag != 1u ||
        !BattleClearOutputSupported(cpu, columns, rows) ||
        !QuietSpanClockSupported(cpu, entry, 0x81e7d2u, 2u, false, room))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01e7a6u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    BattleClearClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.columns = columns;
    clock.rows_left = rows;
    SetBattleClearPhase(&clock, CLEAR_BEGIN);
    const Lufia2Memory memory = {
        BattleClearRead, BattleClearWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleClearRect(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e7d1u);
    assert(clock.phase == CLEAR_FINISH && clock.span.index == clock.span.count &&
        !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
