static const FixedSpanOpcode status_digits_begin[] = {
    {0xe2c8u, 1u, 4u, 2u}, {0xe2c9u, 2u, 3u, 1u}, {0xe2cbu, 3u, 6u, 1u},
    {0xe2ceu, 1u, 2u, 0u}, {0xe2cfu, 3u, 6u, 1u}, {0xe2d2u, 2u, 4u, 2u},
    {0xe2d4u, 3u, 3u, 0u}, {0xe2d7u, 2u, 3u, 0u}, {0xe2dcu, 3u, 6u, 2u}};
static const FixedSpanOpcode status_digits_clamped[] = {
    {0xe2c8u, 1u, 4u, 2u}, {0xe2c9u, 2u, 3u, 1u}, {0xe2cbu, 3u, 6u, 1u},
    {0xe2ceu, 1u, 2u, 0u}, {0xe2cfu, 3u, 6u, 1u}, {0xe2d2u, 2u, 4u, 2u},
    {0xe2d4u, 3u, 3u, 0u}, {0xe2d7u, 2u, 2u, 0u}, {0xe2d9u, 3u, 3u, 0u},
    {0xe2dcu, 3u, 6u, 2u}};
static const FixedSpanOpcode status_digits_hundreds[] = {
    {0xe2dfu, 1u, 2u, 0u}, {0xe2e0u, 3u, 6u, 1u}, {0xe2e3u, 3u, 6u, 1u},
    {0xe2e6u, 2u, 3u, 1u}, {0xe2e8u, 2u, 3u, 0u}, {0xe2f0u, 1u, 2u, 0u},
    {0xe2f1u, 2u, 2u, 0u}, {0xe2f3u, 3u, 6u, 1u}, {0xe2f6u, 2u, 3u, 1u},
    {0xe2f8u, 1u, 2u, 0u}, {0xe2f9u, 2u, 2u, 0u}, {0xe2fbu, 3u, 6u, 1u},
    {0xe2feu, 2u, 3u, 1u}, {0xe300u, 1u, 2u, 0u}, {0xe301u, 2u, 2u, 0u},
    {0xe303u, 3u, 6u, 1u}, {0xe306u, 1u, 5u, 2u}};
static const FixedSpanOpcode status_digits_tens[] = {
    {0xe2dfu, 1u, 2u, 0u}, {0xe2e0u, 3u, 6u, 1u}, {0xe2e3u, 3u, 6u, 1u},
    {0xe2e6u, 2u, 3u, 1u}, {0xe2e8u, 2u, 2u, 0u}, {0xe2eau, 2u, 3u, 1u},
    {0xe2ecu, 2u, 3u, 0u}, {0xe2f8u, 1u, 2u, 0u}, {0xe2f9u, 2u, 2u, 0u},
    {0xe2fbu, 3u, 6u, 1u}, {0xe2feu, 2u, 3u, 1u}, {0xe300u, 1u, 2u, 0u},
    {0xe301u, 2u, 2u, 0u}, {0xe303u, 3u, 6u, 1u}, {0xe306u, 1u, 5u, 2u}};
static const FixedSpanOpcode status_digits_ones[] = {
    {0xe2dfu, 1u, 2u, 0u}, {0xe2e0u, 3u, 6u, 1u}, {0xe2e3u, 3u, 6u, 1u},
    {0xe2e6u, 2u, 3u, 1u}, {0xe2e8u, 2u, 2u, 0u}, {0xe2eau, 2u, 3u, 1u},
    {0xe2ecu, 2u, 2u, 0u}, {0xe2eeu, 2u, 3u, 0u}, {0xe2feu, 2u, 3u, 1u},
    {0xe300u, 1u, 2u, 0u}, {0xe301u, 2u, 2u, 0u}, {0xe303u, 3u, 6u, 1u},
    {0xe306u, 1u, 5u, 2u}};

typedef enum StatusDigitsPhase {
    STATUS_DIGITS_BEGIN,
    STATUS_DIGITS_CHILD,
    STATUS_DIGITS_END
} StatusDigitsPhase;
typedef struct StatusDigitsClock {
    FixedSpanClock span;
    BattleDigitsClock digits;
    StatusDigitsPhase phase;
    unsigned value;
} StatusDigitsClock;

static FixedSpanClock *PrepareStatusDigitsAccess(StatusDigitsClock *clock) {
    if (clock->phase == STATUS_DIGITS_BEGIN) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count || clock->span.remaining)
            return &clock->span;
        BeginBattleDigits(&clock->digits, clock->span.cpu, clock->value / 100u,
                          (clock->value % 100u) / 10u, true);
        clock->phase = STATUS_DIGITS_CHILD;
    }
    if (clock->phase == STATUS_DIGITS_CHILD) {
        BeginFixedSpanOpcode(&clock->digits.span);
        if (clock->digits.phase != DIGITS_FINISH ||
            clock->digits.span.index != clock->digits.span.count ||
            clock->digits.span.remaining) {
            PrepareBattleDigitsAccess(&clock->digits);
            return &clock->digits.span;
        }
        clock->phase = STATUS_DIGITS_END;
        clock->span.index = 0u;
        clock->span.timing = clock->value >= 100u  ? status_digits_hundreds
                             : clock->value >= 10u ? status_digits_tens
                                                   : status_digits_ones;
        clock->span.count = clock->value >= 100u  ? 17u
                            : clock->value >= 10u ? 15u
                                                  : 13u;
    }
    return &clock->span;
}

static uint8_t StatusDigitsRead(void *context, uint32_t address) {
    return FixedSpanRead(PrepareStatusDigitsAccess(context), address);
}

static void StatusDigitsWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanWrite(PrepareStatusDigitsAccess(context), address, value);
}

static void BeginStatusDigits(StatusDigitsClock *clock, CpuState *cpu, unsigned input) {
    *clock = (StatusDigitsClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x810000u;
    clock->span.timing = input >= 1000u ? status_digits_clamped : status_digits_begin;
    clock->span.count = input >= 1000u ? 10u : 9u;
    clock->value = input >= 1000u ? 999u : input;
}

static RecompReturn ActorBridgeTimedStatusDigits(CpuState *cpu) {
    const uint32_t entry = 0x81e2c8u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (!cpu->ram || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->m_flag != 1u)
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01e2c8u : entry);
    const unsigned input = cpu->ram[0x11u] | (unsigned)cpu->ram[0x12u] << 8;
    const unsigned value = input >= 1000u ? 999u : input;
    const unsigned master = 8u * (192u + 28u * (value / 100u + (value % 100u) / 10u));
    const unsigned room = master + 40u * (master / 1324u + 2u);
    const unsigned end = cpu->Y + 9u;
    if (end > 0x10000u || !(end <= 0x11u || cpu->Y >= 0x17u) ||
        !(end <= 0xb2u || cpu->Y >= 0xb5u) || !(end <= 0x1efcu || cpu->Y >= 0x2000u) ||
        !Lufia2DecompFrameRangeQuiet(0x81e808u, 0x81e835u) ||
        !BattleQuietClockSupported(cpu, entry, 0x81e308u, 2u, false, room))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01e2c8u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    StatusDigitsClock clock;
    BeginStatusDigits(&clock, cpu, input);
    const Lufia2Memory memory = {StatusDigitsRead, StatusDigitsWrite, &clock, NULL,
                                 NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleStatusDigits(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e307u);
    assert(clock.phase == STATUS_DIGITS_END && clock.span.index == clock.span.count &&
           !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
