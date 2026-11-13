enum {
    FIELD_TICK_CLOCK_RESERVE = 256u,
    FIELD_TICK_EFFECT_FLAGS = 0x1261u,
    FIELD_TICK_PALETTE_FLAGS = 0x1262u,
    FIELD_TICK_WINDOW_TIMER = 0x1d0c1u,
    FIELD_TICK_TEXT_STATE = 0x099bu
};

static const FixedSpanOpcode field_tick_effect_call[] = {
    {0x9c72u, 4u, 8u, 3u}
};

static const FixedSpanOpcode field_tick_timer_call[] = {
    {0x9c76u, 3u, 6u, 2u}
};

static const FixedSpanOpcode field_tick_idle_text[] = {
    {0x9c79u, 3u, 4u, 1u}, {0x9c7cu, 2u, 2u, 0u},
    {0x9c7eu, 2u, 3u, 0u}, {0x9cb2u, 3u, 4u, 1u},
    {0x9cb5u, 2u, 2u, 0u}
};

_Static_assert(sizeof field_tick_effect_call / sizeof *field_tick_effect_call +
    sizeof field_tick_timer_call / sizeof *field_tick_timer_call +
    sizeof field_tick_idle_text / sizeof *field_tick_idle_text + 1u ==
    LUFIA2_FIELD_TICK_OPCODES, "review field caller opcode budget");


typedef struct FieldTickClock {
    FixedSpanClock span;
    ActorPushedCall call;
    uint8_t data_bank;
    uint8_t index_width;
} FieldTickClock;

static bool FieldTickClockSupported(const CpuState *cpu) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x80u || !ActorBridgeSystemBank(cpu->DB) ||
        cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu || g_interp_apu_driving ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x809c72u) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIELD_TICK_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x809c72u, 0x1ffcu,
            false, 1363u, FIELD_TICK_CLOCK_RESERVE) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x1cb8u ||
        !Lufia2DecompFrameRangeQuiet(0x809c72u, 0x809cb8u))
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < FIELD_TICK_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareFieldTickClock(FieldTickClock *clock, uint32_t landing) {
    CpuState *cpu = clock->call.cpu;
    if (!FieldTickClockSupported(cpu) || cpu->S != clock->call.frame.entry_s ||
        cpu->DB != clock->data_bank || cpu->x_flag != clock->index_width)
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x800000u;
    if (landing == 0x809c72u) {
        /* Only JSL $83:8073 with inactive child services is certified here. */
        if (cpu->host_return_valid != 3u ||
            Lufia2DecompFramePairedReturnPc(cpu, 0x809c72u) != 0x838077u ||
            (cpu->ram[FIELD_TICK_EFFECT_FLAGS] & 0xb7u) ||
            (cpu->ram[FIELD_TICK_PALETTE_FLAGS] & 1u) || cpu->ram[FIELD_TICK_WINDOW_TIMER] == 1u)
            return false;
        clock->span.timing = field_tick_effect_call;
        clock->span.count = sizeof field_tick_effect_call / sizeof *field_tick_effect_call;
    } else if (landing == 0x809c76u) {
        if (cpu->host_return_valid != 3u || cpu->ram[FIELD_TICK_WINDOW_TIMER] == 1u)
            return false;
        clock->span.timing = field_tick_timer_call;
        clock->span.count = sizeof field_tick_timer_call / sizeof *field_tick_timer_call;
    } else if (landing == 0x809c79u) {
        if (cpu->host_return_valid != 2u || (cpu->ram[FIELD_TICK_TEXT_STATE] & 0x8au))
            return false;
        clock->span.timing = field_tick_idle_text;
        clock->span.count = sizeof field_tick_idle_text / sizeof *field_tick_idle_text;
    } else {
        return false;
    }
    return true;
}

static uint8_t FieldTickChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    FieldTickClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert((site == 0x809c72u && target == 0x848000u && frame_size == 3u) ||
        (site == 0x809c76u && target == 0x80c21au && frame_size == 2u));
    Lufia2DecompFramePairedAccountSpan(clock->call.cpu, 0x809c72u,
        clock->span.count, clock->span.progress_mask);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareFieldTickClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedFieldTick(CpuState *cpu) {
    FieldTickClock clock = {0};
    clock.call = (ActorPushedCall){cpu,
        {cpu->S, cpu->host_return_valid, UINT32_MAX}, RECOMP_RETURN_NORMAL};
    clock.data_bank = cpu->DB;
    clock.index_width = cpu->x_flag;
    if (!PrepareFieldTickClock(&clock, 0x809c72u)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x9c72u;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    clock.call.frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, 0x809c72u);
    const Lufia2Memory memory = {
        FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldEventTickWithServices(
        &memory, &state, FieldTickChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x809cb7u);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    bool return_progress = false;
    const RecompReturn returned = ReturnTimedLongCallWithProgress(
        cpu, &clock.call.frame, result.pc, &return_progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x809c72u,
        clock.span.count + 1u,
        clock.span.progress_mask | (unsigned)return_progress << clock.span.count);
    return returned;
}
