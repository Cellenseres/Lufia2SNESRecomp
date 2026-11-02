#include "split_ripple_plans.inc"

typedef struct SplitRippleClock {
    FixedSpanClock span;
    ActorPushedCall call;
    FixedSpanOpcode child_timing;
    uint16_t target;
    bool selector;
} SplitRippleClock;

static bool InitialRippleOwnedHook(uint32_t pc, void (*hook)(CpuState *, uint32_t)) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    if (pc == 0x85a736u && hook == ActorBridgeRippleRowClock)
        return true;
#elif defined(LUFIA2_HAVE_NATIVE_RIPPLE_STEPS)
    if (pc == 0x85a736u && hook == ActorBridgeRippleStepClock)
        return true;
#endif
#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_STEPS)
    if ((pc == 0x85a74fu || pc == 0x85a760u) && hook == ActorBridgeRippleStepClock)
        return true;
#endif
    (void)pc; (void)hook;
    return false;
}

static bool SplitRippleWindow(const CpuState *cpu, uint32_t entry,
    unsigned budget, bool wide) {
    if (cpu->S < 0x1f20u || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, wide, 1363u, budget) ||
        !Lufia2DecompFrameRangeQuiet(0x81bacbu, 0x81bae8u) ||
        !Lufia2DecompFrameRangeQuiet(0x85a76eu, 0x85a804u) ||
        !Lufia2DecompFrameRangeAllows(0x85a701u, 0x85a76eu, InitialRippleOwnedHook))
        return false;
    const unsigned line = g_snes->vPos;
    if (line < 225u) {
        if (!g_snes->hdmaBeamOff || (225u - line) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (line >= 262u || (262u - line) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareSplitRipple(SplitRippleClock *clock, CpuState *cpu,
    uint32_t entry, unsigned operation) {
    if (cpu->D || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->host_return_valid != (operation == 1u || operation == 3u ? 3u : 2u) ||
        !SplitRippleWindow(cpu, entry,
            operation == 3u ? 16000u : operation == 1u ? 8192u : 2048u, false) ||
        (operation != 0u &&
         !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u)))
        return false;
    *clock = (SplitRippleClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = entry & 0xff0000u;
    if (operation == 0u) {
        clock->span.timing = split_ripple_table;
        clock->span.count = sizeof split_ripple_table / sizeof split_ripple_table[0];
    } else if (operation == 1u) {
        clock->span.timing = split_ripple_initialize;
        clock->span.count = sizeof split_ripple_initialize / sizeof split_ripple_initialize[0];
    } else if (operation == 2u) {
        clock->span.timing = split_ripple_select;
        clock->span.count = sizeof split_ripple_select / sizeof split_ripple_select[0];
        clock->selector = true;
    } else {
        clock->span.timing = split_ripple_initialize_original;
        clock->span.count = sizeof split_ripple_initialize_original /
            sizeof split_ripple_initialize_original[0];
    }
    return true;
}

static uint8_t SplitRippleRead(void *context, uint32_t address) {
    SplitRippleClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    const bool table = clock->selector && clock->span.timing == split_ripple_select &&
        clock->span.index == clock->span.count - 1u;
    const bool low = clock->span.remaining == 2u;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (table) {
        if (low) clock->target = value;
        else clock->target |= (uint16_t)value << 8;
    }
    return value;
}

static void SplitRippleWrite(void *context, uint32_t address, uint8_t value) {
    SplitRippleClock *clock = context;
    if (clock->selector && clock->span.timing == split_ripple_select &&
        clock->span.index == clock->span.count && !clock->span.remaining) {
        assert(clock->target == 0xbaddu || clock->target == 0xbae2u);
        clock->child_timing = (FixedSpanOpcode){clock->target, 4u, 8u, 3u};
        clock->span.timing = &clock->child_timing;
        clock->span.index = 0u;
        clock->span.count = 1u;
    }
    FixedSpanWrite(&clock->span, address, value);
}

static uint8_t SplitRippleChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    SplitRippleClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (state->accumulator_is_8_bit && !state->index_is_8_bit &&
        !state->direct_page && !state->decimal &&
        SplitRippleWindow(clock->call.cpu, landing, 128u, false))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static uint8_t InitialRippleRowChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    SplitRippleClock *clock = context;
    assert(target == 0x85a736u && site == 0x85a708u && frame_size == 2u);
    (void)target; (void)site; (void)frame_size;
    const Lufia2Memory memory = {SplitRippleRead, SplitRippleWrite, clock, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2BattleRippleRow(&memory, state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x85a76du);
    (void)result;
    const uint16_t low = SplitRippleRead(clock, ++state->stack);
    const uint16_t high = SplitRippleRead(clock, ++state->stack);
    assert((high << 8 | low) == 0xa70au);
    (void)low; (void)high;
    return 1u;
}

static RecompReturn ActorBridgeSplitRipple(CpuState *cpu, uint32_t entry,
    unsigned operation, ActorWholeFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    SplitRippleClock clock;
    if (!PrepareSplitRipple(&clock, cpu, entry, operation))
        return ActorBridgeFallback(cpu, &frame, entry);
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    const Lufia2Memory memory = {SplitRippleRead, SplitRippleWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = operation == 2u ?
        Lufia2BattleSelectRippleMode(&memory, &state, SplitRippleChild, &clock) :
        operation == 3u ? Lufia2BattleInitializeRipple(
            &memory, &state, InitialRippleRowChild, &clock) :
        run(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    if (operation == 1u || operation == 3u)
        return ReturnTimedLongCall(cpu, &frame, result.pc);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
