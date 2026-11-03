#include "battle_clock_common.inc"

static const FixedSpanOpcode battle_fill_begin[] = {
    {0xe7d2u, 1u, 3u, 1u}, {0xe7d3u, 2u, 2u, 0u},
    {0xe7d5u, 1u, 3u, 1u}, {0xe7d6u, 1u, 4u, 1u},
    {0xe7d7u, 3u, 5u, 2u}, {0xe7dau, 3u, 4u, 1u},
    {0xe7ddu, 3u, 4u, 1u}};
static const FixedSpanOpcode battle_fill_width[] = {
    {0xe7e0u, 2u, 3u, 0u}, {0xe7e2u, 3u, 5u, 2u},
    {0xe7e5u, 3u, 3u, 0u}, {0xe7e8u, 1u, 2u, 0u},
    {0xe7e9u, 3u, 5u, 2u}};
static const FixedSpanOpcode battle_fill_cell[] = {
    {0xe7ecu, 3u, 7u, 2u}, {0xe7efu, 1u, 2u, 0u},
    {0xe7f0u, 1u, 2u, 0u}, {0xe7f1u, 1u, 2u, 0u},
    {0xe7f2u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_fill_last_cell[] = {
    {0xe7ecu, 3u, 7u, 2u}, {0xe7efu, 1u, 2u, 0u},
    {0xe7f0u, 1u, 2u, 0u}, {0xe7f1u, 1u, 2u, 0u},
    {0xe7f2u, 2u, 2u, 0u}};
static const FixedSpanOpcode battle_fill_row[] = {
    {0xe7f4u, 3u, 5u, 2u}, {0xe7f7u, 1u, 2u, 0u},
    {0xe7f8u, 3u, 3u, 0u}, {0xe7fbu, 1u, 2u, 0u},
    {0xe7fcu, 3u, 5u, 2u}, {0xe7ffu, 2u, 3u, 0u},
    {0xe801u, 3u, 6u, 2u}, {0xe804u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_fill_last_row[] = {
    {0xe7f4u, 3u, 5u, 2u}, {0xe7f7u, 1u, 2u, 0u},
    {0xe7f8u, 3u, 3u, 0u}, {0xe7fbu, 1u, 2u, 0u},
    {0xe7fcu, 3u, 5u, 2u}, {0xe7ffu, 2u, 3u, 0u},
    {0xe801u, 3u, 6u, 2u}, {0xe804u, 2u, 2u, 0u}};
static const FixedSpanOpcode battle_fill_finish[] = {{0xe806u, 1u, 4u, 1u}};

typedef enum BattleFillPhase {
    FILL_BEGIN, FILL_WIDTH, FILL_CELL, FILL_ROW, FILL_FINISH
} BattleFillPhase;

typedef struct BattleFillClock {
    FixedSpanClock span;
    BattleFillPhase phase;
    unsigned columns, cells_left, rows_left;
} BattleFillClock;

static void SetBattleFillPhase(BattleFillClock *clock, BattleFillPhase phase) {
    const FixedSpanOpcode *timing = battle_fill_finish;
    unsigned count = 1u;
    switch (phase) {
    case FILL_BEGIN:
        timing = battle_fill_begin;
        count = 7u;
        break;
    case FILL_WIDTH:
        timing = battle_fill_width;
        count = 5u;
        break;
    case FILL_CELL:
        timing = clock->cells_left == 1u ? battle_fill_last_cell : battle_fill_cell;
        count = 5u;
        break;
    case FILL_ROW:
        timing = clock->rows_left == 1u ? battle_fill_last_row : battle_fill_row;
        count = 8u;
        break;
    case FILL_FINISH:
        break;
    }
    clock->phase = phase;
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static void PrepareBattleFillAccess(BattleFillClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count || clock->span.remaining)
        return;
    BattleFillPhase next;
    switch (clock->phase) {
    case FILL_BEGIN:
        next = FILL_WIDTH;
        break;
    case FILL_WIDTH:
        clock->cells_left = clock->columns;
        next = FILL_CELL;
        break;
    case FILL_CELL:
        next = --clock->cells_left ? FILL_CELL : FILL_ROW;
        break;
    default:
        assert(clock->phase == FILL_ROW);
        next = --clock->rows_left ? FILL_WIDTH : FILL_FINISH;
        break;
    }
    SetBattleFillPhase(clock, next);
}

static uint8_t BattleFillRead(void *context, uint32_t address) {
    BattleFillClock *clock = context;
    PrepareBattleFillAccess(clock);
    return FixedSpanRead(&clock->span, address);
}

static void BattleFillWrite(void *context, uint32_t address, uint8_t value) {
    BattleFillClock *clock = context;
    PrepareBattleFillAccess(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static RecompReturn ActorBridgeTimedBattleFill(CpuState *cpu) {
    const uint32_t entry = 0x81e7d2u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned columns = cpu->ram ? cpu->ram[0x09f2u] : 0u;
    const unsigned rows = cpu->ram ? cpu->ram[0x09f3u] : 0u;
    const unsigned end = cpu->X + (rows ? rows - 1u : 0u) * 64u + columns * 2u;
    const unsigned master = 8u * (128u + rows * (64u + columns * 24u));
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (!columns || !rows || cpu->_flag_D || cpu->m_flag != 1u ||
        !BattleWindowOutputSupported(cpu->X, end) ||
        !BattleQuietClockSupported(cpu, entry, 0x81e808u, 2u, false, room))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01e7d2u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    BattleFillClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.columns = columns;
    clock.rows_left = rows;
    SetBattleFillPhase(&clock, FILL_BEGIN);
    const Lufia2Memory memory = {BattleFillRead, BattleFillWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleFillRect(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e807u);
    assert(clock.phase == FILL_FINISH && clock.span.index == clock.span.count &&
           !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
