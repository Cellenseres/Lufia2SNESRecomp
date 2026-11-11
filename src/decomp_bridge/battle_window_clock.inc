#include "battle_clock_common.inc"

#if defined(LUFIA2_HAVE_NATIVE_BATTLE_ROW_SPANS)
static void ActorBridgeBattleFrameCells(CpuState *cpu, uint32_t entry);
#endif

static bool BattleFrameRowHookOwned(uint32_t pc,
    void (*hook)(CpuState *, uint32_t)) {
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_ROW_SPANS)
    return pc == 0x81e553u && hook == ActorBridgeBattleFrameCells;
#else
    (void)pc;
    (void)hook;
    return false;
#endif
}

static const FixedSpanOpcode battle_window_setup[] = {
    {0xe503u, 1u, 3u, 1u}, {0xe504u, 2u, 2u, 0u}, {0xe506u, 1u, 3u, 1u},
    {0xe507u, 1u, 4u, 1u}, {0xe508u, 3u, 4u, 1u}, {0xe50bu, 1u, 2u, 0u},
    {0xe50cu, 3u, 4u, 1u}, {0xe50fu, 3u, 4u, 1u}, {0xe512u, 3u, 4u, 1u},
    {0xe515u, 3u, 4u, 1u}, {0xe518u, 3u, 4u, 1u}, {0xe51bu, 3u, 5u, 2u},
    {0xe51eu, 1u, 2u, 0u}, {0xe51fu, 3u, 5u, 2u}, {0xe522u, 2u, 3u, 0u},
    {0xe524u, 3u, 6u, 2u}};
static const FixedSpanOpcode battle_window_row_head[] = {{0xe542u, 3u, 5u, 2u},
                                                         {0xe545u, 3u, 5u, 2u},
                                                         {0xe548u, 4u, 6u, 2u},
                                                         {0xe54cu, 3u, 7u, 2u},
                                                         {0xe54fu, 4u, 6u, 2u}};
static const FixedSpanOpcode battle_window_cell_more[] = {{0xe553u, 3u, 7u, 2u},
                                                          {0xe556u, 1u, 2u, 0u},
                                                          {0xe557u, 1u, 2u, 0u},
                                                          {0xe558u, 3u, 8u, 4u},
                                                          {0xe55bu, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_window_cell_last[] = {{0xe553u, 3u, 7u, 2u},
                                                          {0xe556u, 1u, 2u, 0u},
                                                          {0xe557u, 1u, 2u, 0u},
                                                          {0xe558u, 3u, 8u, 4u},
                                                          {0xe55bu, 2u, 2u, 0u}};
static const FixedSpanOpcode battle_window_row_tail[] = {
    {0xe55du, 4u, 6u, 2u}, {0xe561u, 3u, 7u, 2u}, {0xe564u, 3u, 5u, 2u},
    {0xe567u, 1u, 2u, 0u}, {0xe568u, 3u, 3u, 0u}, {0xe56bu, 1u, 2u, 0u},
    {0xe56cu, 3u, 5u, 2u}, {0xe56fu, 1u, 6u, 2u}};
static const FixedSpanOpcode battle_window_sides[] = {
    {0xe570u, 4u, 6u, 2u}, {0xe574u, 3u, 7u, 2u}, {0xe577u, 1u, 2u, 0u},
    {0xe578u, 1u, 2u, 0u}, {0xe579u, 3u, 5u, 2u}, {0xe57cu, 3u, 5u, 2u},
    {0xe57fu, 1u, 2u, 0u}, {0xe580u, 4u, 6u, 2u}, {0xe584u, 3u, 7u, 2u},
    {0xe587u, 3u, 5u, 2u}, {0xe58au, 1u, 2u, 0u}, {0xe58bu, 3u, 3u, 0u},
    {0xe58eu, 1u, 2u, 0u}, {0xe58fu, 3u, 5u, 2u}, {0xe592u, 1u, 6u, 2u}};
static const FixedSpanOpcode battle_window_top_more[] = {
    {0xe527u, 1u, 2u, 0u}, {0xe528u, 1u, 2u, 0u}, {0xe529u, 1u, 2u, 0u},
    {0xe52au, 1u, 2u, 0u}, {0xe52bu, 1u, 2u, 0u}, {0xe52cu, 1u, 2u, 0u},
    {0xe52du, 3u, 8u, 4u}, {0xe530u, 2u, 2u, 0u}};
static const FixedSpanOpcode battle_window_top_last[] = {
    {0xe527u, 1u, 2u, 0u}, {0xe528u, 1u, 2u, 0u}, {0xe529u, 1u, 2u, 0u},
    {0xe52au, 1u, 2u, 0u}, {0xe52bu, 1u, 2u, 0u}, {0xe52cu, 1u, 2u, 0u},
    {0xe52du, 3u, 8u, 4u}, {0xe530u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_window_middle_more[] = {
    {0xe535u, 2u, 3u, 0u}, {0xe52du, 3u, 8u, 4u}, {0xe530u, 2u, 2u, 0u}};
static const FixedSpanOpcode battle_window_middle_last[] = {
    {0xe535u, 2u, 3u, 0u}, {0xe52du, 3u, 8u, 4u}, {0xe530u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_window_middle_call[] = {{0xe532u, 3u, 6u, 2u}};
static const FixedSpanOpcode battle_window_bottom_call[] = {{0xe537u, 1u, 2u, 0u},
                                                            {0xe538u, 1u, 2u, 0u},
                                                            {0xe539u, 1u, 2u, 0u},
                                                            {0xe53au, 1u, 2u, 0u},
                                                            {0xe53bu, 3u, 6u, 2u}};
static const FixedSpanOpcode battle_window_finish[] = {{0xe53eu, 2u, 3u, 0u},
                                                       {0xe540u, 1u, 4u, 1u}};

typedef enum BattleWindowPhase {
    WINDOW_SETUP,
    WINDOW_ROW_HEAD,
    WINDOW_CELL,
    WINDOW_ROW_TAIL,
    WINDOW_SIDES,
    WINDOW_AFTER_TOP,
    WINDOW_AFTER_MIDDLE,
    WINDOW_MIDDLE_CALL,
    WINDOW_BOTTOM_CALL,
    WINDOW_FINISH
} BattleWindowPhase;

typedef struct BattleWindowClock {
    FixedSpanClock span;
    BattleWindowPhase phase;
    unsigned fill_count, cells_left, rows_left;
    bool whole_window, bottom;
} BattleWindowClock;

static void SetBattleWindowPhase(BattleWindowClock *clock, BattleWindowPhase phase) {
    const FixedSpanOpcode *timing;
    unsigned count;
    switch (phase) {
    case WINDOW_SETUP:
        timing = battle_window_setup;
        count = 16u;
        break;
    case WINDOW_ROW_HEAD:
        timing = battle_window_row_head;
        count = 5u;
        break;
    case WINDOW_CELL:
        timing =
            clock->cells_left == 1u ? battle_window_cell_last : battle_window_cell_more;
        count = 5u;
        break;
    case WINDOW_ROW_TAIL:
        timing = battle_window_row_tail;
        count = clock->whole_window ? 8u : 7u;
        break;
    case WINDOW_SIDES:
        timing = battle_window_sides;
        count = clock->whole_window ? 15u : 14u;
        break;
    case WINDOW_AFTER_TOP:
        timing = clock->rows_left ? battle_window_top_more : battle_window_top_last;
        count = 8u;
        break;
    case WINDOW_AFTER_MIDDLE:
        timing =
            clock->rows_left ? battle_window_middle_more : battle_window_middle_last;
        count = 3u;
        break;
    case WINDOW_MIDDLE_CALL:
        timing = battle_window_middle_call;
        count = 1u;
        break;
    case WINDOW_BOTTOM_CALL:
        timing = battle_window_bottom_call;
        count = 5u;
        break;
    default:
        assert(phase == WINDOW_FINISH);
        timing = battle_window_finish;
        count = 2u;
        break;
    }
    clock->phase = phase;
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static void PrepareBattleWindowAccess(BattleWindowClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count || clock->span.remaining)
        return;
    BattleWindowPhase next;
    switch (clock->phase) {
    case WINDOW_SETUP:
        next = WINDOW_ROW_HEAD;
        break;
    case WINDOW_ROW_HEAD:
        clock->cells_left = clock->fill_count;
        next = WINDOW_CELL;
        break;
    case WINDOW_CELL:
        next = --clock->cells_left ? WINDOW_CELL : WINDOW_ROW_TAIL;
        break;
    case WINDOW_ROW_TAIL:
        next = clock->bottom ? WINDOW_FINISH : WINDOW_AFTER_TOP;
        break;
    case WINDOW_SIDES:
        --clock->rows_left;
        next = WINDOW_AFTER_MIDDLE;
        break;
    case WINDOW_AFTER_TOP:
    case WINDOW_AFTER_MIDDLE:
        next = clock->rows_left ? WINDOW_MIDDLE_CALL : WINDOW_BOTTOM_CALL;
        break;
    case WINDOW_MIDDLE_CALL:
        next = WINDOW_SIDES;
        break;
    default:
        assert(clock->phase == WINDOW_BOTTOM_CALL);
        clock->bottom = true;
        next = WINDOW_ROW_HEAD;
        break;
    }
    SetBattleWindowPhase(clock, next);
}

static uint8_t BattleWindowRead(void *context, uint32_t address) {
    BattleWindowClock *clock = context;
    PrepareBattleWindowAccess(clock);
    return FixedSpanRead(&clock->span, address);
}

static void BattleWindowWrite(void *context, uint32_t address, uint8_t value) {
    BattleWindowClock *clock = context;
    PrepareBattleWindowAccess(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool BattleWindowOutputSupported(unsigned start, unsigned end) {
    return end <= 0x10000u && (end <= 0x09f2u || start >= 0x0a00u) &&
           (end <= 0x0a3cu || start >= 0x0a3eu) && (end <= 0x1efcu || start >= 0x2000u);
}

static bool BattleWindowEndsBeamSupported(const CpuState *cpu, unsigned room) {
    const uint32_t entry = 0x81e570u;
    if (cpu->m_flag || cpu->x_flag ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u) ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, entry)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - room ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffdu, true,
            1363u, room) || g_snes->vPos >= 225u ||
        g_snes->dma->hdmaPendingInit ||
        !Lufia2DecompFrameRangeQuiet(entry, 0x81e593u))
        return false;
    const unsigned fence = g_snes->hdmaBeamOff ? 1364u : 1024u;
    if (room >= fence || g_snes->hPos >= fence - room)
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedBattleWindow(CpuState *cpu, unsigned kind) {
    static const uint32_t entries[] = {0x81e503u, 0x81e542u, 0x81e570u};
    static const uint32_t returns[] = {0x81e541u, 0x81e56fu, 0x81e592u};
    const uint32_t entry = entries[kind];
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (!cpu->ram || cpu->_flag_D || cpu->m_flag != (kind == 0u) ||
        (kind && cpu->DB != 0x7eu && cpu->DB != 0x7fu))
        return ActorBridgeFallback(cpu, &frame,
                                   cpu->PB == 1u ? entry & 0x7fffffu : entry);
    const unsigned bank = kind && cpu->DB == 0x7fu ? 0x10000u : 0u;
    const uint8_t *ram = cpu->ram + bank;
    const unsigned width = kind ? (ram[0x09fau] | (unsigned)ram[0x09fbu] << 8)
                                : (cpu->ram[0x09f2u] ? cpu->ram[0x09f2u] : 256u) - 1u;
    const unsigned fill_count = width > 0x8000u ? 1u : width + 1u;
    const unsigned rows = kind ? 0u : cpu->ram[0x09f3u];
    const unsigned start = kind ? cpu->Y : cpu->X;
    const unsigned end =
        kind == 2u ? start + 2u * width + 6u
                   : start + (kind ? 0u : (rows + 1u) * 64u) + 2u * (fill_count + 2u);
    const unsigned source =
        kind ? cpu->X : cpu->ram[0x09f4u] | (unsigned)cpu->ram[0x09f5u] << 8;
    const bool visible_ends = kind == 2u && g_snes && g_snes->vPos < 225u;
    const unsigned cycles = visible_ends ? 65u
                            : kind == 2u   ? 128u
                            : kind == 1u ? 128u + 24u * fill_count
                                         : 512u + 48u * fill_count + 96u * rows;
    const unsigned master = cycles * 8u;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    const uint32_t range_end = kind ? returns[kind] + 1u : 0x81e593u;
    if (source < 0x8000u ||
        source > (kind == 2u ? 0xfffcu
                  : kind     ? 0xfffau
                             : 0xfff0u) ||
        !BattleWindowOutputSupported(start, end) ||
        !(visible_ends ? BattleWindowEndsBeamSupported(cpu, room)
                       : BattleQuietClockAllows(cpu, entry, range_end, 2u,
                           kind != 0u, room, BattleFrameRowHookOwned)))
        return ActorBridgeFallback(cpu, &frame,
                                   cpu->PB == 1u ? entry & 0x7fffffu : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    BattleWindowClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.whole_window = kind == 0u;
    clock.fill_count = fill_count;
    clock.rows_left = rows;
    SetBattleWindowPhase(&clock, kind == 0u   ? WINDOW_SETUP
                                 : kind == 1u ? WINDOW_ROW_HEAD
                                              : WINDOW_SIDES);
    const Lufia2Memory memory = {BattleWindowRead, BattleWindowWrite, &clock, NULL,
                                 NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        kind == 0u   ? Lufia2BattleWindow(&memory, &state)
        : kind == 1u ? Lufia2BattleFrameRow(&memory, &state)
                     : Lufia2BattleFrameEnds(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == returns[kind]);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

#if defined(LUFIA2_HAVE_NATIVE_BATTLE_ROW_SPANS)
typedef struct BattleRowSpanClock {
    FixedSpanClock span;
    uint16_t counter;
    unsigned cells_left, opcodes, progress_mask;
} BattleRowSpanClock;

static void SetBattleRowCellClock(BattleRowSpanClock *clock) {
    CpuState *cpu = clock->span.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x810000u;
    clock->span.timing = ((uint16_t)(clock->counter - 1u) & 0x8000u)
        ? battle_window_cell_last : battle_window_cell_more;
    clock->span.count = 5u;
}

static void PrepareBattleRowCell(BattleRowSpanClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count)
        return;
    clock->progress_mask |= clock->span.progress_mask << clock->opcodes;
    clock->opcodes += 5u;
    --clock->counter;
    if (--clock->cells_left)
        SetBattleRowCellClock(clock);
}

static uint8_t BattleRowSpanRead(void *context, uint32_t address) {
    BattleRowSpanClock *clock = context;
    PrepareBattleRowCell(clock);
    return FixedSpanRead(&clock->span, address);
}

static void BattleRowSpanWrite(void *context, uint32_t address, uint8_t value) {
    BattleRowSpanClock *clock = context;
    PrepareBattleRowCell(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static unsigned BattleRowSpanSize(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x81e553u || (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->m_flag || cpu->x_flag ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true,
            1363u, 1u) || g_snes->vPos >= 262u ||
        g_snes->dma->hdmaPendingInit ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 10u,
            BattleFrameRowHookOwned))
        return 0u;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return 0u;
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    const uint16_t counter = cpu->ram[bank + 0x0a3cu] |
        (uint16_t)cpu->ram[bank + 0x0a3du] << 8;
    unsigned cells = counter > 0x8000u ? 1u : (unsigned)counter + 1u;
    if (cells > 6u)
        cells = 6u;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    while (cells) {
        const unsigned master = 22u * cells * 8u;
        const unsigned room = master + 40u * (master / 1324u + 2u);
        if (g_snes->hPos < fence && room < fence - g_snes->hPos &&
            cpu->master_cycles <= UINT64_MAX - room &&
            deadline - cpu->master_cycles >= room &&
            BattleWindowOutputSupported((unsigned)cpu->Y + 2u,
                (unsigned)cpu->Y + 2u + 2u * cells))
            return cells;
        --cells;
    }
    return 0u;
}

static void ActorBridgeBattleFrameCells(CpuState *cpu, uint32_t entry) {
    const unsigned cells = BattleRowSpanSize(cpu, entry);
    if (!cells)
        return;
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    BattleRowSpanClock clock = {0};
    clock.span.cpu = cpu;
    clock.counter = cpu->ram[bank + 0x0a3cu] |
        (uint16_t)cpu->ram[bank + 0x0a3du] << 8;
    clock.cells_left = cells;
    SetBattleRowCellClock(&clock);
    const Lufia2Memory memory = {
        BattleRowSpanRead, BattleRowSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleFillFrameRowSpan(&memory, &state, entry, cells);
    while (clock.cells_left)
        PrepareBattleRowCell(&clock);
    assert(!clock.span.remaining && clock.opcodes == 5u * cells);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
