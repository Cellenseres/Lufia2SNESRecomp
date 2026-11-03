enum { BATTLE_GAUGE_CYCLE_LIMIT = 1024u };
_Static_assert(BATTLE_MULTIPLY_CYCLE_LIMIT + BATTLE_DIVIDE_CYCLE_LIMIT + 264u <=
                   BATTLE_GAUGE_CYCLE_LIMIT,
               "Gauge clock reservation");

static const FixedSpanOpcode battle_gauge_label[] = {
    {0xe2afu, 2u, 3u, 1u}, {0xe2b1u, 3u, 6u, 1u}, {0xe2b4u, 1u, 2u, 0u},
    {0xe2b5u, 3u, 6u, 1u}, {0xe2b8u, 2u, 2u, 0u}, {0xe2bau, 3u, 6u, 1u},
    {0xe2bdu, 3u, 6u, 1u}, {0xe2c0u, 1u, 2u, 0u}, {0xe2c1u, 1u, 2u, 0u},
    {0xe2c2u, 1u, 2u, 0u}, {0xe2c3u, 1u, 2u, 0u}, {0xe2c4u, 1u, 2u, 0u},
    {0xe2c5u, 1u, 2u, 0u}, {0xe2c6u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_gauge_begin[] = {
    {0xe308u, 2u, 3u, 0u}, {0xe30au, 2u, 4u, 2u}, {0xe30cu, 2u, 4u, 2u}};
static const FixedSpanOpcode battle_gauge_zero[] = {{0xe30eu, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_gauge_multiply_call[] = {
    {0xe30eu, 2u, 2u, 0u}, {0xe310u, 2u, 3u, 0u}, {0xe312u, 2u, 2u, 0u},
    {0xe314u, 2u, 3u, 1u}, {0xe316u, 1u, 3u, 1u}, {0xe317u, 1u, 4u, 1u},
    {0xe318u, 4u, 8u, 3u}};
static const FixedSpanOpcode battle_gauge_divide_call[] = {
    {0xe31cu, 2u, 2u, 0u}, {0xe31eu, 1u, 3u, 1u}, {0xe31fu, 1u, 4u, 1u},
    {0xe320u, 2u, 3u, 0u}, {0xe322u, 2u, 4u, 2u}, {0xe324u, 2u, 4u, 2u},
    {0xe326u, 2u, 4u, 2u}, {0xe328u, 2u, 4u, 2u}, {0xe32au, 4u, 8u, 3u}};
static const FixedSpanOpcode battle_gauge_remainder_zero[] = {{0xe32eu, 3u, 3u, 0u},
                                                              {0xe331u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_gauge_remainder[] = {
    {0xe32eu, 3u, 3u, 0u}, {0xe331u, 2u, 2u, 0u}, {0xe333u, 2u, 4u, 2u}};
static const FixedSpanOpcode battle_gauge_round[] = {{0xe335u, 2u, 2u, 0u},
                                                     {0xe337u, 2u, 7u, 4u}};
static const FixedSpanOpcode battle_gauge_unrounded[] = {{0xe335u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_gauge_part_read[] = {{0xe339u, 2u, 3u, 0u},
                                                         {0xe33bu, 2u, 3u, 1u}};
static const FixedSpanOpcode battle_gauge_left[] = {
    {0xe345u, 1u, 2u, 0u}, {0xe346u, 2u, 3u, 1u}, {0xe348u, 3u, 6u, 1u},
    {0xe34bu, 2u, 3u, 1u}, {0xe34du, 1u, 2u, 0u}, {0xe34eu, 2u, 3u, 1u},
    {0xe350u, 2u, 3u, 1u}};
static const FixedSpanOpcode battle_gauge_middle_left[] = {
    {0xe35au, 1u, 2u, 0u}, {0xe35bu, 2u, 3u, 1u}, {0xe35du, 2u, 2u, 0u},
    {0xe35fu, 3u, 6u, 1u}, {0xe362u, 2u, 3u, 1u}, {0xe364u, 1u, 2u, 0u},
    {0xe365u, 2u, 3u, 1u}, {0xe367u, 2u, 3u, 1u}};
static const FixedSpanOpcode battle_gauge_middle_right[] = {
    {0xe371u, 1u, 2u, 0u}, {0xe372u, 2u, 3u, 1u}, {0xe374u, 2u, 2u, 0u},
    {0xe376u, 3u, 6u, 1u}, {0xe379u, 2u, 3u, 1u}, {0xe37bu, 1u, 2u, 0u},
    {0xe37cu, 2u, 3u, 1u}, {0xe37eu, 2u, 3u, 1u}};
static const FixedSpanOpcode battle_gauge_right[] = {{0xe386u, 1u, 2u, 0u},
                                                     {0xe387u, 2u, 3u, 1u},
                                                     {0xe389u, 2u, 2u, 0u},
                                                     {0xe38bu, 3u, 6u, 1u}};

typedef enum BattleGaugePhase {
    GAUGE_LABEL,
    GAUGE_BEGIN,
    GAUGE_ZERO,
    GAUGE_MULTIPLY_CALL,
    GAUGE_MULTIPLY,
    GAUGE_DIVIDE_CALL,
    GAUGE_DIVIDE,
    GAUGE_REMAINDER_ZERO,
    GAUGE_REMAINDER,
    GAUGE_ROUND,
    GAUGE_UNROUNDED,
    GAUGE_PART_READ,
    GAUGE_PART_CAP,
    GAUGE_PART_BODY
} BattleGaugePhase;

typedef struct BattleGaugeClock {
    FixedSpanClock span, multiply;
    BattleDivideClock divide;
    FixedSpanOpcode cap[4];
    Lufia2CpuState *state;
    BattleGaugePhase phase;
    unsigned part;
    uint8_t remaining;
} BattleGaugeClock;

static void SetBattleGaugePhase(BattleGaugeClock *clock, BattleGaugePhase phase) {
    const FixedSpanOpcode *timing;
    unsigned count;
    switch (phase) {
    case GAUGE_LABEL:
        timing = battle_gauge_label;
        count = 14u;
        break;
    case GAUGE_BEGIN:
        timing = battle_gauge_begin;
        count = 3u;
        break;
    case GAUGE_ZERO:
        timing = battle_gauge_zero;
        count = 1u;
        break;
    case GAUGE_MULTIPLY_CALL:
        timing = battle_gauge_multiply_call;
        count = 7u;
        break;
    case GAUGE_DIVIDE_CALL:
        timing = battle_gauge_divide_call;
        count = 9u;
        break;
    case GAUGE_REMAINDER_ZERO:
        timing = battle_gauge_remainder_zero;
        count = 2u;
        break;
    case GAUGE_REMAINDER:
        timing = battle_gauge_remainder;
        count = 3u;
        break;
    case GAUGE_ROUND:
        timing = battle_gauge_round;
        count = 2u;
        break;
    case GAUGE_UNROUNDED:
        timing = battle_gauge_unrounded;
        count = 1u;
        break;
    case GAUGE_PART_READ:
        timing = battle_gauge_part_read;
        count = 2u;
        break;
    case GAUGE_PART_CAP: {
        static const uint16_t sites[] = {0xe33du, 0xe352u, 0xe369u, 0xe380u};
        const uint16_t pc = sites[clock->part];
        const unsigned cap = clock->part == 0u || clock->part == 3u ? 5u : 8u;
        const bool capped = clock->remaining > cap;
        clock->cap[0] = (FixedSpanOpcode){pc, 2u, 2u, 0u};
        clock->cap[1] =
            (FixedSpanOpcode){(uint16_t)(pc + 2u), 2u, capped ? 2u : 3u, 0u};
        count = 2u;
        if (capped)
            clock->cap[count++] = (FixedSpanOpcode){(uint16_t)(pc + 4u), 2u, 2u, 0u};
        if (clock->part < 3u)
            clock->cap[count++] = (FixedSpanOpcode){(uint16_t)(pc + 6u), 2u, 3u, 1u};
        timing = clock->cap;
        break;
    }
    default: {
        assert(phase == GAUGE_PART_BODY && clock->part < 4u);
        static const struct {
            const FixedSpanOpcode *timing;
            unsigned count;
        } parts[] = {{battle_gauge_left, 7u},
                     {battle_gauge_middle_left, 8u},
                     {battle_gauge_middle_right, 8u},
                     {battle_gauge_right, 4u}};
        timing = parts[clock->part].timing;
        count = parts[clock->part].count;
        break;
    }
    }
    clock->phase = phase;
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static FixedSpanClock *PrepareBattleGaugeAccess(BattleGaugeClock *clock) {
    for (;;) {
        if (clock->phase == GAUGE_MULTIPLY) {
            BeginFixedSpanOpcode(&clock->multiply);
            if (clock->multiply.index != clock->multiply.count ||
                clock->multiply.remaining)
                return &clock->multiply;
            SetBattleGaugePhase(clock, GAUGE_DIVIDE_CALL);
        } else if (clock->phase == GAUGE_DIVIDE) {
            BeginFixedSpanOpcode(&clock->divide.span);
            if (clock->divide.phase != DIVIDE_FINISH ||
                clock->divide.span.index != clock->divide.span.count ||
                clock->divide.span.remaining) {
                PrepareBattleDivideAccess(&clock->divide);
                return &clock->divide.span;
            }
            SetBattleGaugePhase(clock, clock->state->accumulator
                                           ? GAUGE_REMAINDER
                                           : GAUGE_REMAINDER_ZERO);
        }
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count || clock->span.remaining)
            return &clock->span;
        BattleGaugePhase next;
        switch (clock->phase) {
        case GAUGE_LABEL:
            next = GAUGE_BEGIN;
            break;
        case GAUGE_BEGIN:
            next = clock->state->zero ? GAUGE_ZERO : GAUGE_MULTIPLY_CALL;
            break;
        case GAUGE_ZERO:
        case GAUGE_REMAINDER_ZERO:
        case GAUGE_ROUND:
        case GAUGE_UNROUNDED:
            next = GAUGE_PART_READ;
            break;
        case GAUGE_MULTIPLY_CALL:
            BeginBattleMultiply(&clock->multiply, clock->span.cpu, true);
            clock->phase = GAUGE_MULTIPLY;
            continue;
        case GAUGE_DIVIDE_CALL:
            BeginBattleDivide(&clock->divide, clock->span.cpu, clock->state, true);
            clock->phase = GAUGE_DIVIDE;
            continue;
        case GAUGE_REMAINDER:
            next = clock->state->zero ? GAUGE_ROUND : GAUGE_UNROUNDED;
            break;
        case GAUGE_PART_READ:
            next = GAUGE_PART_CAP;
            break;
        case GAUGE_PART_CAP:
            next = GAUGE_PART_BODY;
            break;
        default:
            assert(clock->phase == GAUGE_PART_BODY && clock->part < 3u);
            ++clock->part;
            next = GAUGE_PART_CAP;
            break;
        }
        SetBattleGaugePhase(clock, next);
    }
}

static uint8_t BattleGaugeRead(void *context, uint32_t address) {
    BattleGaugeClock *clock = context;
    FixedSpanClock *span = PrepareBattleGaugeAccess(clock);
    const uint8_t value = FixedSpanRead(span, address);
    if (clock->phase == GAUGE_PART_READ && address == 0x4eu)
        clock->remaining = value;
    return value;
}

static void BattleGaugeWrite(void *context, uint32_t address, uint8_t value) {
    BattleGaugeClock *clock = context;
    FixedSpanClock *span = PrepareBattleGaugeAccess(clock);
    FixedSpanWrite(span, address, value);
    if (clock->phase == GAUGE_PART_BODY && address == 0x4eu)
        clock->remaining = value;
}

static RecompReturn ActorBridgeTimedBattleGauge(CpuState *cpu, bool label) {
    const uint32_t entry = label ? 0x81e2afu : 0x81e308u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned master = BATTLE_GAUGE_CYCLE_LIMIT * 8u;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    const unsigned start = cpu->Y + (label ? 0u : 10u),
                   end = cpu->Y + (label ? 11u : 17u);
    if (!cpu->ram || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->m_flag != 1u ||
        (label && cpu->Y < 6u) || end > 0x10000u || !(end <= 0x11u || start >= 0x17u) ||
        !(end <= 0x4eu || start >= 0x54u) || !(end <= 0x1ef8u || start >= 0x2000u) ||
        !Lufia2DecompFrameRangeQuiet(0x80834cu, 0x808378u) ||
        !Lufia2DecompFrameRangeQuiet(0x808378u, 0x808450u) ||
        !BattleQuietClockSupported(cpu, entry, 0x81e38fu, 2u, false, room))
        return ActorBridgeFallback(cpu, &frame,
                                   cpu->PB == 1u ? entry & 0x7fffffu : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    BattleGaugeClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.state = &state;
    SetBattleGaugePhase(&clock, label ? GAUGE_LABEL : GAUGE_BEGIN);
    const Lufia2Memory memory = {BattleGaugeRead, BattleGaugeWrite, &clock, NULL, NULL};
    const Lufia2ExecutionResult result =
        label ? Lufia2BattleStatusGauge(&memory, &state)
              : Lufia2BattleFillStatusGauge(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e38eu);
    assert(clock.phase == GAUGE_PART_BODY && clock.part == 3u &&
           clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
