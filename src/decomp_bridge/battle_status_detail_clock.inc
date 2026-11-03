enum { BATTLE_STATUS_CYCLE_LIMIT = 5600u };
_Static_assert(700u + 344u + 2u * 696u + 3u * BATTLE_GAUGE_CYCLE_LIMIT <=
                   BATTLE_STATUS_CYCLE_LIMIT,
               "Status detail clock reservation");

static const FixedSpanOpcode status_detail_begin[] = {
    {0xe1b5u, 1u, 3u, 1u}, {0xe1b6u, 1u, 4u, 2u}, {0xe1b7u, 2u, 3u, 0u},
    {0xe1b9u, 4u, 6u, 2u}, {0xe1bdu, 1u, 4u, 2u}, {0xe1beu, 2u, 3u, 0u},
    {0xe1c0u, 3u, 5u, 2u}, {0xe1c3u, 1u, 2u, 0u}, {0xe1c4u, 1u, 5u, 2u},
    {0xe1c5u, 1u, 4u, 2u}, {0xe1c6u, 1u, 4u, 2u}, {0xe1c7u, 2u, 2u, 0u},
    {0xe1c9u, 1u, 3u, 1u}, {0xe1cau, 1u, 4u, 1u}, {0xe1cbu, 2u, 3u, 0u},
    {0xe1cdu, 2u, 5u, 2u}, {0xe1cfu, 1u, 2u, 0u}, {0xe1d0u, 3u, 5u, 2u},
    {0xe1d3u, 1u, 2u, 0u}, {0xe1d4u, 2u, 3u, 0u}, {0xe1d6u, 2u, 2u, 0u},
    {0xe1d8u, 2u, 3u, 1u},
};

static const FixedSpanOpcode status_detail_symbols[] = {
    {0xe1dau, 1u, 2u, 0u}, {0xe1dbu, 3u, 6u, 1u}, {0xe1deu, 2u, 2u, 0u},
    {0xe1e0u, 3u, 6u, 1u}, {0xe1e3u, 1u, 2u, 0u}, {0xe1e4u, 1u, 2u, 0u},
    {0xe1e5u, 1u, 2u, 0u}, {0xe1e6u, 2u, 5u, 2u}, {0xe1e8u, 2u, 3u, 0u},
};

static const FixedSpanOpcode status_detail_symbols_last[] = {
    {0xe1dau, 1u, 2u, 0u}, {0xe1dbu, 3u, 6u, 1u}, {0xe1deu, 2u, 2u, 0u},
    {0xe1e0u, 3u, 6u, 1u}, {0xe1e3u, 1u, 2u, 0u}, {0xe1e4u, 1u, 2u, 0u},
    {0xe1e5u, 1u, 2u, 0u}, {0xe1e6u, 2u, 5u, 2u}, {0xe1e8u, 2u, 2u, 0u},
};

static const FixedSpanOpcode status_detail_level_call[] = {
    {0xe1eau, 1u, 5u, 2u}, {0xe1ebu, 1u, 5u, 2u}, {0xe1ecu, 3u, 5u, 1u},
    {0xe1efu, 1u, 4u, 2u}, {0xe1f0u, 2u, 3u, 0u}, {0xe1f2u, 3u, 3u, 0u},
    {0xe1f5u, 1u, 2u, 0u}, {0xe1f6u, 2u, 3u, 0u}, {0xe1f8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode status_detail_level_prefix[] = {
    {0xe1fbu, 2u, 2u, 0u}, {0xe1fdu, 3u, 6u, 1u}, {0xe200u, 1u, 2u, 0u},
    {0xe201u, 3u, 6u, 1u}, {0xe204u, 2u, 3u, 1u},
};

static const FixedSpanOpcode status_detail_level_single[] = {
    {0xe206u, 2u, 3u, 0u},
};

static const FixedSpanOpcode status_detail_level_double[] = {
    {0xe206u, 2u, 2u, 0u},
    {0xe208u, 1u, 2u, 0u},
    {0xe209u, 2u, 2u, 0u},
    {0xe20bu, 3u, 6u, 1u},
};

static const FixedSpanOpcode status_detail_level_end[] = {
    {0xe20eu, 2u, 3u, 1u}, {0xe210u, 1u, 2u, 0u}, {0xe211u, 2u, 2u, 0u},
    {0xe213u, 3u, 6u, 1u}, {0xe216u, 1u, 5u, 2u}, {0xe217u, 2u, 3u, 0u},
    {0xe219u, 1u, 2u, 0u}, {0xe21au, 1u, 2u, 0u}, {0xe21bu, 3u, 3u, 0u},
    {0xe21eu, 1u, 2u, 0u}, {0xe21fu, 3u, 6u, 2u}, {0xe222u, 2u, 4u, 2u},
    {0xe224u, 3u, 6u, 2u}, {0xe227u, 2u, 4u, 2u}, {0xe229u, 2u, 3u, 0u},
    {0xe22bu, 2u, 2u, 0u}, {0xe22du, 2u, 3u, 1u}, {0xe22fu, 2u, 2u, 0u},
    {0xe231u, 2u, 3u, 1u}, {0xe233u, 1u, 4u, 2u}, {0xe234u, 1u, 4u, 2u},
    {0xe235u, 3u, 6u, 2u},
};

static const FixedSpanOpcode status_detail_hp_gauge[] = {
    {0xe238u, 3u, 6u, 2u},
};

static const FixedSpanOpcode status_detail_mp_digits[] = {
    {0xe23bu, 1u, 5u, 2u}, {0xe23cu, 1u, 5u, 2u}, {0xe23du, 2u, 3u, 0u},
    {0xe23fu, 1u, 2u, 0u}, {0xe240u, 1u, 2u, 0u}, {0xe241u, 3u, 3u, 0u},
    {0xe244u, 1u, 2u, 0u}, {0xe245u, 3u, 6u, 2u}, {0xe248u, 2u, 4u, 2u},
    {0xe24au, 3u, 6u, 2u}, {0xe24du, 2u, 4u, 2u}, {0xe24fu, 2u, 3u, 0u},
    {0xe251u, 2u, 2u, 0u}, {0xe253u, 2u, 3u, 1u}, {0xe255u, 2u, 2u, 0u},
    {0xe257u, 2u, 3u, 1u}, {0xe259u, 1u, 4u, 2u}, {0xe25au, 1u, 4u, 2u},
    {0xe25bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode status_detail_mp_gauge[] = {
    {0xe25eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode status_detail_ip_gauge[] = {
    {0xe261u, 1u, 5u, 2u}, {0xe262u, 1u, 5u, 2u}, {0xe263u, 2u, 3u, 0u},
    {0xe265u, 1u, 2u, 0u}, {0xe266u, 1u, 2u, 0u}, {0xe267u, 3u, 3u, 0u},
    {0xe26au, 1u, 2u, 0u}, {0xe26bu, 2u, 3u, 0u}, {0xe26du, 2u, 2u, 0u},
    {0xe26fu, 3u, 6u, 1u}, {0xe272u, 1u, 2u, 0u}, {0xe273u, 3u, 6u, 1u},
    {0xe276u, 2u, 2u, 0u}, {0xe278u, 3u, 6u, 1u}, {0xe27bu, 1u, 2u, 0u},
    {0xe27cu, 3u, 6u, 1u}, {0xe27fu, 3u, 5u, 1u}, {0xe282u, 2u, 3u, 1u},
    {0xe284u, 2u, 3u, 1u}, {0xe286u, 2u, 2u, 0u}, {0xe288u, 2u, 3u, 1u},
    {0xe28au, 2u, 3u, 1u}, {0xe28cu, 2u, 2u, 0u}, {0xe28eu, 2u, 3u, 1u},
    {0xe290u, 2u, 3u, 0u}, {0xe292u, 2u, 5u, 2u}, {0xe294u, 1u, 2u, 0u},
    {0xe295u, 4u, 6u, 2u}, {0xe299u, 1u, 2u, 0u}, {0xe29au, 2u, 3u, 0u},
    {0xe29cu, 1u, 2u, 0u}, {0xe29du, 4u, 5u, 1u}, {0xe2a1u, 1u, 2u, 0u},
    {0xe2a2u, 4u, 5u, 1u}, {0xe2a6u, 3u, 6u, 1u}, {0xe2a9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode status_detail_end[] = {
    {0xe2acu, 1u, 5u, 2u},
    {0xe2adu, 1u, 4u, 1u},
};

typedef struct StatusDetailClock {
    FixedSpanClock span;
    unsigned symbols_left;
    unsigned child;
    bool level_has_tens;
} StatusDetailClock;

static void SetStatusDetailPlan(StatusDetailClock *clock, const FixedSpanOpcode *timing,
                                unsigned count) {
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
    clock->span.remaining = 0u;
}

#define STATUS_DETAIL_PLAN(clock, name)                                                \
    SetStatusDetailPlan(clock, status_detail_##name,                                   \
                        sizeof status_detail_##name / sizeof *status_detail_##name)

static FixedSpanClock *PrepareStatusDetailAccess(StatusDetailClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    while (clock->span.index == clock->span.count && !clock->span.remaining) {
        if (clock->span.timing == status_detail_begin) {
            clock->symbols_left = 5u;
            STATUS_DETAIL_PLAN(clock, symbols);
        } else if (clock->span.timing == status_detail_symbols ||
                   clock->span.timing == status_detail_symbols_last) {
            if (--clock->symbols_left == 0u)
                STATUS_DETAIL_PLAN(clock, level_call);
            else if (clock->symbols_left == 1u)
                STATUS_DETAIL_PLAN(clock, symbols_last);
            else
                STATUS_DETAIL_PLAN(clock, symbols);
        } else if (clock->span.timing == status_detail_level_prefix) {
            if (clock->level_has_tens)
                STATUS_DETAIL_PLAN(clock, level_double);
            else
                STATUS_DETAIL_PLAN(clock, level_single);
        } else if (clock->span.timing == status_detail_level_single ||
                   clock->span.timing == status_detail_level_double) {
            STATUS_DETAIL_PLAN(clock, level_end);
        } else {
            assert(false);
            break;
        }
        BeginFixedSpanOpcode(&clock->span);
    }
    return &clock->span;
}

static uint8_t StatusDetailRead(void *context, uint32_t address) {
    return FixedSpanRead(PrepareStatusDetailAccess(context), address);
}

static void StatusDetailWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanWrite(PrepareStatusDetailAccess(context), address, value);
}

static void FinishStatusChild(FixedSpanClock *span, Lufia2CpuState *state,
                              uint16_t return_site) {
    const FixedSpanOpcode rts = {return_site, 1u, 6u, 2u};
    *span = (FixedSpanClock){
        .cpu = span->cpu, .bank = 0x810000u, .timing = &rts, .count = 1u};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, span, NULL, NULL};
    (void)memory.read_byte(memory.context, ++state->stack);
    (void)memory.read_byte(memory.context, ++state->stack);
    BeginFixedSpanOpcode(span);
    assert(span->index == span->count && !span->remaining);
}

static uint8_t StatusDetailChild(void *context, Lufia2CpuState *state, uint32_t target,
                                 uint32_t site, uint8_t frame) {
    StatusDetailClock *clock = context;
    static const uint16_t sites[] = {0xe1f8u, 0xe235u, 0xe238u,
                                     0xe25bu, 0xe25eu, 0xe2a9u};
    BeginFixedSpanOpcode(&clock->span);
    assert(frame == 2u && clock->child < 6u &&
           site == (0x810000u | sites[clock->child]));
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    Lufia2ExecutionResult result;
    if (target == 0x81e808u) {
        BattleDigitsClock digits;
        BeginBattleDigits(&digits, clock->span.cpu, state->x / 100u,
                          (state->x % 100u) / 10u, true);
        const Lufia2Memory memory = {BattleDigitsRead, BattleDigitsWrite, &digits, NULL,
                                     NULL};
        result = Lufia2DecimalDigits3(&memory, state);
        (void)memory.read_byte(memory.context, ++state->stack);
        (void)memory.read_byte(memory.context, ++state->stack);
        BeginFixedSpanOpcode(&digits.span);
        assert(digits.phase == DIGITS_FINISH &&
               digits.span.index == digits.span.count && !digits.span.remaining);
    } else if (target == 0x81e2c8u) {
        const unsigned value =
            clock->span.cpu->ram[0x11u] | (unsigned)clock->span.cpu->ram[0x12u] << 8;
        StatusDigitsClock digits;
        BeginStatusDigits(&digits, clock->span.cpu, value);
        const Lufia2Memory memory = {StatusDigitsRead, StatusDigitsWrite, &digits, NULL,
                                     NULL};
        result = Lufia2BattleStatusDigits(&memory, state);
        BeginFixedSpanOpcode(&digits.span);
        assert(digits.phase == STATUS_DIGITS_END &&
               digits.span.index == digits.span.count && !digits.span.remaining);
        FinishStatusChild(&digits.span, state, 0xe307u);
    } else {
        assert(target == 0x81e308u);
        BattleGaugeClock gauge = {0};
        gauge.span.cpu = clock->span.cpu;
        gauge.span.bank = 0x810000u;
        gauge.state = state;
        SetBattleGaugePhase(&gauge, GAUGE_BEGIN);
        const Lufia2Memory memory = {BattleGaugeRead, BattleGaugeWrite, &gauge, NULL,
                                     NULL};
        result = Lufia2BattleFillStatusGauge(&memory, state);
        BeginFixedSpanOpcode(&gauge.span);
        assert(gauge.phase == GAUGE_PART_BODY && gauge.part == 3u &&
               gauge.span.index == gauge.span.count && !gauge.span.remaining);
        FinishStatusChild(&gauge.span, state, 0xe38eu);
    }
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    switch (clock->child++) {
    case 0u:
        STATUS_DETAIL_PLAN(clock, level_prefix);
        break;
    case 1u:
        STATUS_DETAIL_PLAN(clock, hp_gauge);
        break;
    case 2u:
        STATUS_DETAIL_PLAN(clock, mp_digits);
        break;
    case 3u:
        STATUS_DETAIL_PLAN(clock, mp_gauge);
        break;
    case 4u:
        STATUS_DETAIL_PLAN(clock, ip_gauge);
        break;
    default:
        STATUS_DETAIL_PLAN(clock, end);
        break;
    }
    return 1u;
}

static bool BattleStatusRecordSupported(const CpuState *cpu, unsigned slot,
                                        unsigned *record) {
    if (slot > 6u || (slot & 1u))
        return false;
    const unsigned pointer = 0xa64u + slot;
    *record = cpu->ram[pointer] | (unsigned)cpu->ram[pointer + 1u] << 8;
    return *record >= 0x200u && *record <= 0xff43u &&
           (*record + 0xbdu <= 0x1ee0u || *record >= 0x2000u) &&
           (*record + 0xbdu <= 0x2c00u || *record >= 0x2f00u);
}

static void BeginStatusDetail(StatusDetailClock *clock, CpuState *cpu,
                              unsigned record) {
    *clock = (StatusDetailClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x810000u;
    clock->level_has_tens = cpu->ram[record + 0xeu] % 100u >= 10u;
    STATUS_DETAIL_PLAN(clock, begin);
}

static RecompReturn ActorBridgeTimedStatusDetail(CpuState *cpu) {
    const uint32_t entry = 0x81e1b5u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const unsigned master = 8u * BATTLE_STATUS_CYCLE_LIMIT;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (!cpu->ram || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->m_flag != 1u ||
        cpu->X > 6u || (cpu->X & 1u) ||
        !BattleQuietClockSupported(cpu, entry, 0x81e2afu, 2u, false, room) ||
        !Lufia2DecompFrameRangeQuiet(0x81e2afu, 0x81e38fu) ||
        !Lufia2DecompFrameRangeQuiet(0x81e808u, 0x81e835u) ||
        !Lufia2DecompFrameRangeQuiet(0x80834cu, 0x808378u) ||
        !Lufia2DecompFrameRangeQuiet(0x808378u, 0x808450u))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01e1b5u : entry);
    unsigned record;
    if (!BattleStatusRecordSupported(cpu, cpu->X, &record))
        return ActorBridgeFallback(cpu, &frame, entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    StatusDetailClock clock;
    BeginStatusDetail(&clock, cpu, record);
    const Lufia2Memory memory = {StatusDetailRead, StatusDetailWrite, &clock, NULL,
                                 NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleDrawPartyStatusDetail(&memory, &state, StatusDetailChild, &clock);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e2aeu);
    assert(clock.child == 6u && clock.span.index == clock.span.count &&
           !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

#undef STATUS_DETAIL_PLAN
