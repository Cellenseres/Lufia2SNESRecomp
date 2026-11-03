static const FixedSpanOpcode staged_command_begin[] = {
    {0xd081u, 2u, 3u, 0u}, {0xd083u, 1u, 2u, 0u}, {0xd084u, 3u, 5u, 2u},
    {0xd087u, 1u, 2u, 0u}, {0xd088u, 4u, 6u, 2u}, {0xd08cu, 1u, 2u, 0u},
};

static const FixedSpanOpcode staged_command_begin_end[] = {
    {0xd090u, 1u, 2u, 0u},
    {0xd091u, 2u, 3u, 0u},
    {0xd093u, 3u, 6u, 2u},
};

static const FixedSpanOpcode staged_command_cancel[] = {
    {0xd096u, 2u, 2u, 0u},
    {0xd098u, 3u, 3u, 0u},
};

static const FixedSpanOpcode staged_command_status[] = {
    {0xd096u, 2u, 3u, 0u}, {0xd09bu, 2u, 3u, 0u}, {0xd09du, 4u, 6u, 2u},
    {0xd0a1u, 1u, 2u, 0u}, {0xd0a2u, 1u, 2u, 0u}, {0xd0a3u, 2u, 3u, 0u},
    {0xd0a5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode staged_command_window[] = {
    {0xd0a8u, 2u, 3u, 0u}, {0xd0aau, 4u, 6u, 2u}, {0xd0aeu, 1u, 2u, 0u},
    {0xd0afu, 2u, 3u, 0u}, {0xd0b1u, 2u, 2u, 0u}, {0xd0b3u, 3u, 4u, 1u},
    {0xd0b6u, 2u, 2u, 0u}, {0xd0b8u, 3u, 4u, 1u}, {0xd0bbu, 3u, 3u, 0u},
    {0xd0beu, 3u, 5u, 2u}, {0xd0c1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode staged_command_end[] = {
    {0xd0c4u, 1u, 2u, 0u},
};

typedef struct StagedCommandClock {
    FixedSpanClock span;
    FixedSpanOpcode party_read;
    unsigned child;
    unsigned record;
} StagedCommandClock;

static void SetStagedCommandPlan(StagedCommandClock *clock,
                                 const FixedSpanOpcode *timing, unsigned count) {
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
    clock->span.remaining = 0u;
}

#define STAGED_COMMAND_PLAN(clock, name)                                               \
    SetStagedCommandPlan(clock, staged_command_##name,                                 \
                         sizeof staged_command_##name / sizeof *staged_command_##name)

static FixedSpanClock *PrepareStagedCommandAccess(StagedCommandClock *clock,
                                                  uint32_t address) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index == clock->span.count && !clock->span.remaining) {
        if (clock->span.timing == staged_command_begin) {
            clock->party_read = (FixedSpanOpcode){
                0xd08du, 3u, (uint8_t)(5u + ((address >> 8) != 0x7e15u)), 2u};
            SetStagedCommandPlan(clock, &clock->party_read, 1u);
        } else {
            assert(clock->span.timing == &clock->party_read);
            STAGED_COMMAND_PLAN(clock, begin_end);
        }
    }
    return &clock->span;
}

static uint8_t StagedCommandRead(void *context, uint32_t address) {
    return FixedSpanRead(PrepareStagedCommandAccess(context, address), address);
}

static void StagedCommandWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanWrite(PrepareStagedCommandAccess(context, address), address, value);
}

static uint8_t StagedCommandChild(void *context, Lufia2CpuState *state, uint32_t target,
                                  uint32_t site, uint8_t frame) {
    StagedCommandClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(frame == 2u && clock->span.index == clock->span.count &&
           !clock->span.remaining);
    if (clock->child == 0u) {
        assert(site == 0x81d093u && target == 0x81d12eu);
        FinishStatusChild(&clock->span, state, 0xd12eu);
        if (state->carry)
            STAGED_COMMAND_PLAN(clock, cancel);
        else
            STAGED_COMMAND_PLAN(clock, status);
    } else if (clock->child == 1u) {
        assert(site == 0x81d0a5u && target == 0x81e1b5u);
        StatusDetailClock status;
        BeginStatusDetail(&status, clock->span.cpu, clock->record);
        const Lufia2Memory memory = {StatusDetailRead, StatusDetailWrite, &status, NULL,
                                     NULL};
        const Lufia2ExecutionResult result = Lufia2BattleDrawPartyStatusDetail(
            &memory, state, StatusDetailChild, &status);
        BeginFixedSpanOpcode(&status.span);
        assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e2aeu &&
               status.child == 6u && status.span.index == status.span.count &&
               !status.span.remaining);
        FinishStatusChild(&status.span, state, 0xe2aeu);
        STAGED_COMMAND_PLAN(clock, window);
    } else {
        assert(clock->child == 2u && site == 0x81d0c1u && target == 0x81e503u);
        BattleWindowClock window = {0};
        window.span.cpu = clock->span.cpu;
        window.span.bank = 0x810000u;
        window.whole_window = true;
        window.fill_count = 13u;
        window.rows_left = 4u;
        SetBattleWindowPhase(&window, WINDOW_SETUP);
        const Lufia2Memory memory = {BattleWindowRead, BattleWindowWrite, &window, NULL,
                                     NULL};
        const Lufia2ExecutionResult result = Lufia2BattleWindow(&memory, state);
        BeginFixedSpanOpcode(&window.span);
        assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81e541u &&
               window.span.index == window.span.count && !window.span.remaining);
        FinishStatusChild(&window.span, state, 0xe541u);
        STAGED_COMMAND_PLAN(clock, end);
    }
    ++clock->child;
    return 1u;
}

static unsigned StatusDigitCycleLimit(unsigned value) {
    if (value >= 1000u)
        value = 999u;
    return 192u + 28u * (value / 100u + (value % 100u) / 10u);
}

static unsigned StagedCommandCycleLimit(const CpuState *cpu, unsigned record) {
    const uint8_t *party = cpu->ram + record;
    const unsigned level = party[0xeu];
    const unsigned hp = party[0x11u] | (unsigned)party[0x12u] << 8;
    const unsigned mp = party[0x13u] | (unsigned)party[0x14u] << 8;
    const unsigned gauge =
        BATTLE_MULTIPLY_CYCLE_LIMIT + BATTLE_DIVIDE_CYCLE_LIMIT + 264u;
    return 128u + 1520u + 700u + 64u + 28u * (level / 100u + (level % 100u) / 10u) +
           StatusDigitCycleLimit(hp) + StatusDigitCycleLimit(mp) + 3u * gauge;
}

static RecompReturn ActorBridgeTimedStagedCommand(CpuState *cpu) {
    const uint32_t entry = 0x81d081u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    unsigned record = 0u;
    if (!cpu->ram || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->m_flag != 1u)
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01d081u : entry);
    const unsigned slot = cpu->ram[0x1be8u] | (unsigned)cpu->ram[0x1be9u] << 8;
    if (!cpu->_flag_C && !BattleStatusRecordSupported(cpu, 2u * slot, &record))
        return ActorBridgeFallback(cpu, &frame, entry);
    const unsigned cycles = cpu->_flag_C ? 128u : StagedCommandCycleLimit(cpu, record);
    const unsigned master = 8u * cycles;
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (!BattleQuietClockSupported(cpu, entry, 0x81d0c6u, 2u, false, room) ||
        !Lufia2DecompFrameRangeQuiet(0x81d12eu, 0x81d12fu) ||
        (!cpu->_flag_C && (!Lufia2DecompFrameRangeQuiet(0x81e1b5u, 0x81e2afu) ||
                           !Lufia2DecompFrameRangeQuiet(0x81e2afu, 0x81e38fu) ||
                           !Lufia2DecompFrameRangeQuiet(0x81e503u, 0x81e593u) ||
                           !Lufia2DecompFrameRangeQuiet(0x81e808u, 0x81e835u) ||
                           !Lufia2DecompFrameRangeQuiet(0x80834cu, 0x808378u) ||
                           !Lufia2DecompFrameRangeQuiet(0x808378u, 0x808450u))))
        return ActorBridgeFallback(cpu, &frame, cpu->PB == 1u ? 0x01d081u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    StagedCommandClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.record = record;
    STAGED_COMMAND_PLAN(&clock, begin);
    const Lufia2Memory memory = {StagedCommandRead, StagedCommandWrite, &clock, NULL,
                                 NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleRefreshStagedCommand(&memory, &state, StagedCommandChild, &clock);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        assert(result.pc == 0x81cc93u && clock.child == 1u);
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s,
                                         frame.hrv);
    }
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81d0c5u &&
           clock.child == 3u);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

#undef STAGED_COMMAND_PLAN
