static const FixedSpanOpcode battle_digits_setup[] = {
    {0xe808u, 2u, 3u, 1u}, {0xe80au, 2u, 3u, 1u}, {0xe80cu, 2u, 3u, 1u}};
static const FixedSpanOpcode battle_digits_hundreds[] = {
    {0xe80eu, 2u, 3u, 0u}, {0xe810u, 1u, 2u, 0u}, {0xe811u, 1u, 2u, 0u},
    {0xe812u, 3u, 3u, 0u}, {0xe815u, 2u, 2u, 0u}, {0xe817u, 1u, 2u, 0u},
    {0xe818u, 2u, 3u, 0u}, {0xe81au, 3u, 6u, 2u}, {0xe81du, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_digits_hundreds_done[] = {{0xe80eu, 2u, 3u, 0u},
                                                              {0xe810u, 1u, 2u, 0u},
                                                              {0xe811u, 1u, 2u, 0u},
                                                              {0xe812u, 3u, 3u, 0u},
                                                              {0xe815u, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_digits_tens[] = {
    {0xe81fu, 1u, 2u, 0u}, {0xe820u, 1u, 2u, 0u}, {0xe821u, 3u, 3u, 0u},
    {0xe824u, 2u, 2u, 0u}, {0xe826u, 1u, 2u, 0u}, {0xe827u, 2u, 3u, 0u},
    {0xe829u, 2u, 5u, 2u}, {0xe82bu, 2u, 3u, 0u}, {0xe82du, 2u, 3u, 0u}};
static const FixedSpanOpcode battle_digits_finish[] = {
    {0xe81fu, 1u, 2u, 0u}, {0xe820u, 1u, 2u, 0u}, {0xe821u, 3u, 3u, 0u},
    {0xe824u, 2u, 3u, 0u}, {0xe82fu, 1u, 2u, 0u}, {0xe830u, 2u, 3u, 0u},
    {0xe832u, 2u, 3u, 1u}, {0xe834u, 1u, 6u, 2u}};

typedef enum BattleDigitsPhase {
    DIGITS_SETUP,
    DIGITS_HUNDREDS,
    DIGITS_HUNDREDS_DONE,
    DIGITS_TENS,
    DIGITS_FINISH
} BattleDigitsPhase;

typedef struct BattleDigitsClock {
    FixedSpanClock span;
    BattleDigitsPhase phase;
    unsigned hundreds_left, tens_left;
    bool inline_return;
} BattleDigitsClock;

static void SetBattleDigitsPhase(BattleDigitsClock *clock, BattleDigitsPhase phase) {
    static const struct {
        const FixedSpanOpcode *timing;
        unsigned count;
    } phases[] = {{battle_digits_setup, 3u},
                  {battle_digits_hundreds, 9u},
                  {battle_digits_hundreds_done, 5u},
                  {battle_digits_tens, 9u},
                  {battle_digits_finish, 7u}};
    clock->phase = phase;
    clock->span.timing = phases[phase].timing;
    clock->span.count =
        phases[phase].count + (phase == DIGITS_FINISH && clock->inline_return);
    clock->span.index = 0u;
}

static void PrepareBattleDigitsAccess(BattleDigitsClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count || clock->span.remaining)
            return;
        BattleDigitsPhase next;
        if (clock->phase == DIGITS_SETUP) {
            next = clock->hundreds_left ? DIGITS_HUNDREDS : DIGITS_HUNDREDS_DONE;
        } else if (clock->phase == DIGITS_HUNDREDS) {
            next = --clock->hundreds_left ? DIGITS_HUNDREDS : DIGITS_HUNDREDS_DONE;
        } else if (clock->phase == DIGITS_HUNDREDS_DONE) {
            next = clock->tens_left ? DIGITS_TENS : DIGITS_FINISH;
        } else {
            assert(clock->phase == DIGITS_TENS);
            next = --clock->tens_left ? DIGITS_TENS : DIGITS_FINISH;
        }
        SetBattleDigitsPhase(clock, next);
    }
}

static uint8_t BattleDigitsRead(void *context, uint32_t address) {
    BattleDigitsClock *clock = context;
    PrepareBattleDigitsAccess(clock);
    return FixedSpanRead(&clock->span, address);
}

static void BattleDigitsWrite(void *context, uint32_t address, uint8_t value) {
    BattleDigitsClock *clock = context;
    PrepareBattleDigitsAccess(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static void BeginBattleDigits(BattleDigitsClock *clock, CpuState *cpu,
                              unsigned hundreds, unsigned tens, bool inline_return) {
    *clock = (BattleDigitsClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x810000u;
    clock->hundreds_left = hundreds;
    clock->tens_left = tens;
    clock->inline_return = inline_return;
    SetBattleDigitsPhase(clock, DIGITS_SETUP);
}

static RecompReturn ActorBridgeTimedBattleDigits(CpuState *cpu) {
    const uint32_t entry = 0x81e808u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned hundreds = cpu->X / 100u;
    const unsigned tens = (cpu->X % 100u) / 10u;
    const unsigned master = 8u * (64u + 28u * (hundreds + tens));
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (cpu->D || cpu->_flag_D || cpu->m_flag != 1u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu && (cpu->DB & 0x7fu) >= 0x40u) ||
        !BattleQuietClockSupported(cpu, entry, 0x81e835u, 2u, false, room))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01e808u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    BattleDigitsClock clock;
    BeginBattleDigits(&clock, cpu, hundreds, tens, false);
    const Lufia2Memory memory = {BattleDigitsRead, BattleDigitsWrite, &clock, NULL,
                                 NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2DecimalDigits3(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e834u);
    assert(clock.phase == DIGITS_FINISH && clock.span.index == clock.span.count &&
           !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
