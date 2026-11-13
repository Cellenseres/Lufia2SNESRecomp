enum { FIELD_CHILD_CLOCK_RESERVE = 512u };

static const FixedSpanOpcode field_effect_idle_timing[] = {
    {0x8000u, 3u, 4u, 1u}, {0x8003u, 2u, 2u, 0u},
    {0x8005u, 2u, 3u, 0u}, {0x800du, 2u, 2u, 0u},
    {0x800fu, 2u, 3u, 0u}, {0x8017u, 2u, 2u, 0u},
    {0x8019u, 2u, 3u, 0u}, {0x8021u, 2u, 2u, 0u},
    {0x8023u, 2u, 3u, 0u}, {0x8048u, 2u, 2u, 0u},
    {0x804au, 2u, 3u, 0u}, {0x809bu, 3u, 4u, 1u},
    {0x809eu, 2u, 2u, 0u}, {0x80a0u, 2u, 3u, 0u}
};

static const FixedSpanOpcode field_timer_idle_timing[] = {
    {0xc21au, 4u, 5u, 1u}, {0xc21eu, 2u, 2u, 0u},
    {0xc220u, 2u, 3u, 0u}
};

static const FixedSpanOpcode field_timer_countdown_timing[] = {
    {0xc21au, 4u, 5u, 1u}, {0xc21eu, 2u, 2u, 0u},
    {0xc220u, 2u, 2u, 0u}, {0xc222u, 1u, 2u, 0u},
    {0xc223u, 4u, 5u, 1u}, {0xc227u, 2u, 3u, 0u}
};

_Static_assert(sizeof field_effect_idle_timing / sizeof *field_effect_idle_timing +
    1u == LUFIA2_FIELD_EFFECT_OPCODES, "review field effect opcode budget");
_Static_assert(sizeof field_timer_countdown_timing / sizeof *field_timer_countdown_timing +
    1u == LUFIA2_FIELD_TIMER_MAX_OPCODES, "review field timer opcode budget");

static bool FieldChildClockSupported(const CpuState *cpu, uint32_t entry,
    uint32_t return_source, uint32_t caller_return, uint8_t frame_size) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != entry >> 16 || !ActorBridgeSystemBank(cpu->DB) ||
        cpu->m_flag != 1u || cpu->x_flag > 1u || cpu->host_return_valid != frame_size ||
        cpu->S < 0x1f20u || cpu->S > (uint16_t)(0x1fffu - frame_size) ||
        g_interp_apu_driving || !Lufia2DecompFramePairedCallReady(cpu, entry) ||
        Lufia2DecompFramePairedReturnPc(cpu, entry) != caller_return ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIELD_CHILD_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu - frame_size,
            false, 1363u, FIELD_CHILD_CLOCK_RESERVE) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM ||
        !Lufia2DecompFrameRangeQuiet(entry, return_source + 1u))
        return false;
    const uint32_t last = ((entry >> 16) & 0x7fu) * 0x8000u +
        (return_source & 0x7fffu);
    if (last >= g_snes->cart->romSize)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < FIELD_CHILD_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn RunTimedFieldChild(CpuState *cpu, uint32_t entry,
    uint32_t return_source, uint8_t frame_size, const FixedSpanOpcode *timing,
    unsigned count, ActorSceneOwnersFunction run) {
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, entry)};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == return_source);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    const RecompReturn returned = frame_size == 3u
        ? ReturnTimedLongCall(cpu, &frame, return_source)
        : ReturnTimedMenuCall(cpu, &frame, return_source);
    Lufia2DecompFramePairedAccountSpan(cpu, entry, count + 1u, clock.progress_mask);
    assert(cpu->master_cycles - start <= FIELD_CHILD_CLOCK_RESERVE);
    return returned;
}

static RecompReturn ActorBridgeTimedFieldEffects(CpuState *cpu) {
    if (!FieldChildClockSupported(cpu, 0x848000u, 0x8480bfu, 0x809c76u, 3u) ||
        (cpu->ram[0x1261u] & 0xb7u) || (cpu->ram[0x1262u] & 1u)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x8000u;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    return RunTimedFieldChild(cpu, 0x848000u, 0x8480bfu, 3u,
        field_effect_idle_timing, sizeof field_effect_idle_timing / sizeof *field_effect_idle_timing,
        Lufia2FieldUpdateScreenEffects);
}

static RecompReturn ActorBridgeTimedFieldTimer(CpuState *cpu) {
    if (!FieldChildClockSupported(cpu, 0x80c21au, 0x80c23cu, 0x809c79u, 2u) ||
        cpu->ram[0x1d0c1u] == 1u) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0xc21au;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const bool idle = cpu->ram[0x1d0c1u] == 0xffu;
    return RunTimedFieldChild(cpu, 0x80c21au, 0x80c23cu, 2u,
        idle ? field_timer_idle_timing : field_timer_countdown_timing,
        idle ? sizeof field_timer_idle_timing / sizeof *field_timer_idle_timing :
            sizeof field_timer_countdown_timing / sizeof *field_timer_countdown_timing,
        Lufia2FieldTextWindowTimer);
}
