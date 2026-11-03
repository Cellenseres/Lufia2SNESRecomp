#include "battle_clock_common.inc"

static const FixedSpanOpcode second_stat_begin[] = {
    {0xc07cu, 1u, 3u, 1u}, {0xc07du, 2u, 3u, 0u},
    {0xc07fu, 1u, 4u, 2u}, {0xc080u, 1u, 4u, 2u},
    {0xc081u, 3u, 6u, 2u}, {0xc099u, 1u, 3u, 1u},
    {0xc09au, 2u, 3u, 0u}, {0xc09cu, 1u, 4u, 2u},
    {0xc09du, 3u, 3u, 0u}, {0xc0a0u, 1u, 2u, 0u},
    {0xc0a1u, 1u, 2u, 0u}, {0xc0a2u, 4u, 6u, 2u},
    {0xc0a6u, 1u, 2u, 0u}, {0xc0a7u, 1u, 5u, 2u},
    {0xc0a8u, 1u, 4u, 1u}, {0xc0a9u, 1u, 6u, 2u},
    {0xc084u, 1u, 2u, 0u}};
static const FixedSpanOpcode second_stat_ip[] = {
    {0xc087u, 3u, 3u, 0u}, {0xc08au, 2u, 3u, 0u},
    {0xc091u, 3u, 3u, 0u}, {0xc094u, 1u, 2u, 0u},
    {0xc095u, 1u, 5u, 2u}, {0xc096u, 1u, 5u, 2u},
    {0xc097u, 1u, 4u, 1u}};
static const FixedSpanOpcode second_stat_level[] = {
    {0xc087u, 3u, 3u, 0u}, {0xc08au, 2u, 2u, 0u},
    {0xc08cu, 3u, 3u, 0u}, {0xc08fu, 2u, 2u, 0u},
    {0xc091u, 3u, 3u, 0u}, {0xc094u, 1u, 2u, 0u},
    {0xc095u, 1u, 5u, 2u}, {0xc096u, 1u, 5u, 2u},
    {0xc097u, 1u, 4u, 1u}};
static const FixedSpanOpcode second_stat_word[] = {
    {0xc087u, 3u, 3u, 0u}, {0xc08au, 2u, 2u, 0u},
    {0xc08cu, 3u, 3u, 0u}, {0xc08fu, 2u, 3u, 0u},
    {0xc094u, 1u, 2u, 0u}, {0xc095u, 1u, 5u, 2u},
    {0xc096u, 1u, 5u, 2u}, {0xc097u, 1u, 4u, 1u}};

typedef struct SecondStatClock {
    FixedSpanClock span;
    FixedSpanOpcode load;
    const Lufia2CpuState *state;
    uint8_t pointer_low;
} SecondStatClock;

static FixedSpanClock *PrepareSecondStatAccess(SecondStatClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index == clock->span.count && !clock->span.remaining) {
        if (clock->span.timing == second_stat_begin) {
            clock->load = (FixedSpanOpcode){0xc085u, 2u, 6u, 4u};
            clock->span.timing = &clock->load;
            clock->span.count = 1u;
        } else {
            assert(clock->span.timing == &clock->load);
            const unsigned offset = clock->state->y;
            clock->span.timing = offset == 0xbcu ? second_stat_ip
                                 : offset == 0x0eu ? second_stat_level
                                                   : second_stat_word;
            clock->span.count = offset == 0xbcu ? 7u : offset == 0x0eu ? 9u : 8u;
        }
        clock->span.index = 0u;
    }
    return &clock->span;
}

static uint8_t SecondStatRead(void *context, uint32_t address) {
    SecondStatClock *clock = context;
    FixedSpanClock *span = PrepareSecondStatAccess(clock);
    const uint8_t value = FixedSpanRead(span, address);
    if (span->timing == &clock->load) {
        if (span->remaining == 3u)
            clock->pointer_low = value;
        else if (span->remaining == 2u) {
            const unsigned pointer = (unsigned)value << 8 | clock->pointer_low;
            clock->load.cycles = (uint8_t)(6u +
                ((pointer >> 8) != ((pointer + clock->state->y) >> 8)));
        }
    }
    return value;
}

static void SecondStatWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanWrite(PrepareSecondStatAccess(context), address, value);
}

static RecompReturn ActorBridgeTimedSecondBattlerStat(CpuState *cpu) {
    const uint32_t entry = 0x85c07cu;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (!cpu->ram || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->S > 0x1ffcu ||
        !BattleQuietClockSupported(cpu, entry, 0x85c0aau, 2u, false, 1104u))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 5u ? 0x05c07cu : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    SecondStatClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x850000u;
    clock.span.timing = second_stat_begin;
    clock.span.count = sizeof second_stat_begin / sizeof *second_stat_begin;
    clock.state = &state;
    const Lufia2Memory memory = {SecondStatRead, SecondStatWrite, &clock, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2BattleReadSecondBattlerStat(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x85c098u);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
