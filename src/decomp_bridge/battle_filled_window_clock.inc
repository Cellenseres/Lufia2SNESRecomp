#define FILLED_WINDOW_BEGIN(pc)                                                        \
    {{pc, 1u, 4u, 2u}, {pc + 1u, 2u, 3u, 0u}, {pc + 3u, 1u, 2u, 0u},               \
     {pc + 4u, 1u, 2u, 0u}, {pc + 5u, 3u, 3u, 0u}, {pc + 8u, 1u, 2u, 0u},          \
     {pc + 9u, 3u, 3u, 0u}, {pc + 12u, 3u, 5u, 2u}, {pc + 15u, 2u, 3u, 0u},        \
     {pc + 17u, 3u, 6u, 2u}}
#define FILLED_WINDOW_MIDDLE(pc)                                                       \
    {{pc + 20u, 3u, 3u, 0u}, {pc + 23u, 3u, 5u, 2u},                                \
     {pc + 26u, 1u, 5u, 2u}, {pc + 27u, 3u, 6u, 2u}}

static const FixedSpanOpcode filled_window_begin[][10] = {
    FILLED_WINDOW_BEGIN(0xe38fu), FILLED_WINDOW_BEGIN(0xe3aeu),
    FILLED_WINDOW_BEGIN(0xe3cdu)};
static const FixedSpanOpcode filled_window_middle[][4] = {
    FILLED_WINDOW_MIDDLE(0xe38fu), FILLED_WINDOW_MIDDLE(0xe3aeu),
    FILLED_WINDOW_MIDDLE(0xe3cdu)};
#undef FILLED_WINDOW_BEGIN
#undef FILLED_WINDOW_MIDDLE

static const FixedSpanOpcode filled_window_fill_return[] = {{0xe807u, 1u, 6u, 2u}};
static const FixedSpanOpcode filled_window_frame_return[] = {{0xe541u, 1u, 6u, 2u}};

typedef enum FilledWindowPhase {
    FILLED_WINDOW_BEGIN,
    FILLED_WINDOW_FILL,
    FILLED_WINDOW_FILL_RETURN,
    FILLED_WINDOW_MIDDLE,
    FILLED_WINDOW_FRAME,
    FILLED_WINDOW_FRAME_RETURN
} FilledWindowPhase;

typedef struct FilledWindowClock {
    FixedSpanClock span;
    BattleFillClock fill;
    BattleWindowClock window;
    FilledWindowPhase phase;
    unsigned kind;
} FilledWindowClock;

static void SetFilledWindowPlan(FilledWindowClock *clock, FilledWindowPhase phase,
                                 const FixedSpanOpcode *timing, unsigned count) {
    clock->phase = phase;
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static FixedSpanClock *PrepareFilledWindowAccess(FilledWindowClock *clock) {
    for (;;) {
        if (clock->phase == FILLED_WINDOW_FILL) {
            BeginFixedSpanOpcode(&clock->fill.span);
            if (clock->fill.phase != FILL_FINISH ||
                clock->fill.span.index != clock->fill.span.count ||
                clock->fill.span.remaining) {
                PrepareBattleFillAccess(&clock->fill);
                return &clock->fill.span;
            }
            SetFilledWindowPlan(clock, FILLED_WINDOW_FILL_RETURN,
                                filled_window_fill_return, 1u);
        } else if (clock->phase == FILLED_WINDOW_FRAME) {
            BeginFixedSpanOpcode(&clock->window.span);
            if (clock->window.phase != WINDOW_FINISH ||
                clock->window.span.index != clock->window.span.count ||
                clock->window.span.remaining) {
                PrepareBattleWindowAccess(&clock->window);
                return &clock->window.span;
            }
            SetFilledWindowPlan(clock, FILLED_WINDOW_FRAME_RETURN,
                                filled_window_frame_return, 1u);
        }
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count || clock->span.remaining)
            return &clock->span;
        switch (clock->phase) {
        case FILLED_WINDOW_BEGIN:
            clock->phase = FILLED_WINDOW_FILL;
            SetBattleFillPhase(&clock->fill, FILL_BEGIN);
            break;
        case FILLED_WINDOW_FILL_RETURN:
            SetFilledWindowPlan(clock, FILLED_WINDOW_MIDDLE,
                                filled_window_middle[clock->kind], 4u);
            break;
        default:
            assert(clock->phase == FILLED_WINDOW_MIDDLE);
            clock->phase = FILLED_WINDOW_FRAME;
            SetBattleWindowPhase(&clock->window, WINDOW_SETUP);
            break;
        }
    }
}

static uint8_t FilledWindowRead(void *context, uint32_t address) {
    return FixedSpanRead(PrepareFilledWindowAccess(context), address);
}

static void FilledWindowWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanWrite(PrepareFilledWindowAccess(context), address, value);
}

static RecompReturn ActorBridgeTimedFilledWindow(CpuState *cpu, unsigned kind) {
    static const uint32_t entries[] = {0x81e38fu, 0x81e3aeu, 0x81e3cdu};
    const uint32_t entry = entries[kind];
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned columns = cpu->ram ? cpu->ram[0x09f2u] : 0u;
    const unsigned rows = cpu->ram ? cpu->ram[0x09f3u] : 0u;
    const unsigned end = cpu->X + (rows + 1u) * 64u + 2u * (columns + 2u);
    const unsigned cycles = 768u + rows * (160u + columns * 24u) + columns * 48u;
    const unsigned master = cycles * 8u;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (cpu->DB != 0x7eu || !columns || !rows || cpu->_flag_D ||
        cpu->m_flag != 1u || !BattleWindowOutputSupported(cpu->X, end) ||
        !BattleQuietClockSupported(cpu, entry, entry + 31u, 3u, false, room) ||
        !Lufia2DecompFrameRangeQuiet(0x81e7d2u, 0x81e808u) ||
        !Lufia2DecompFrameRangeQuiet(0x81e503u, 0x81e593u))
        return ActorBridgeFallback(cpu, &frame,
                                   cpu->PB == 1u ? entry & 0x7fffffu : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    FilledWindowClock clock = {0};
    clock.kind = kind;
    clock.span.cpu = clock.fill.span.cpu = clock.window.span.cpu = cpu;
    clock.span.bank = clock.fill.span.bank = clock.window.span.bank = 0x810000u;
    clock.fill.columns = clock.window.fill_count = columns;
    clock.fill.rows_left = clock.window.rows_left = rows;
    clock.window.whole_window = true;
    SetFilledWindowPlan(&clock, FILLED_WINDOW_BEGIN, filled_window_begin[kind], 10u);
    const Lufia2Memory memory = {FilledWindowRead, FilledWindowWrite, &clock, NULL,
                                 NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        kind == 0u   ? Lufia2BattleDrawBlankWindow(&memory, &state)
        : kind == 1u ? Lufia2BattleWindowE3AE(&memory, &state)
                     : Lufia2BattleWindowE3CD(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == entry + 30u);
    assert(clock.phase == FILLED_WINDOW_FRAME_RETURN &&
           clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}
