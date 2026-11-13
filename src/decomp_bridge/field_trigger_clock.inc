enum { FIELD_TRIGGER_CLOCK_RESERVE = 512u };

static unsigned PlanFieldTriggerClocks(const uint8_t *ram, unsigned stage,
    FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define TRIGGER_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (!stage) {
        TRIGGER_OPCODE(0x81c6u, 1u, 3u, 1u);
        TRIGGER_OPCODE(0x81c7u, 2u, 3u, 0u);
        TRIGGER_OPCODE(0x81c9u, 2u, 3u, 0u);
        TRIGGER_OPCODE(0x81cbu, 4u, 8u, 3u);
        return count;
    }
    if (stage == 1u) {
        const bool contact = (ram[0x09a7u] & 1u) != 0u;
        TRIGGER_OPCODE(0x81cfu, 3u, 4u, 1u);
        TRIGGER_OPCODE(0x81d2u, 2u, 2u, 0u);
        TRIGGER_OPCODE(0x81d4u, 2u, contact ? 2u : 3u, 0u);
        if (contact) {
            TRIGGER_OPCODE(0x81d6u, 2u, 3u, 0u);
            TRIGGER_OPCODE(0x81d8u, 3u, 6u, 2u);
            return count;
        }
    } else {
        assert(stage == 2u);
        TRIGGER_OPCODE(0x81dbu, 2u, 3u, 0u);
        TRIGGER_OPCODE(0x81ddu, 2u, 3u, 0u);
    }
    const bool pending = (ram[0x0622u] & 0x40u) != 0u;
    TRIGGER_OPCODE(0x81fau, 3u, 4u, 1u);
    TRIGGER_OPCODE(0x81fdu, 2u, 2u, 0u);
    TRIGGER_OPCODE(0x81ffu, 2u, pending ? 2u : 3u, 0u);
    if (pending) {
        TRIGGER_OPCODE(0x8201u, 2u, 2u, 0u);
        TRIGGER_OPCODE(0x8203u, 2u, 3u, 0u);
    }
    const bool regions = (ram[0x05b5u] & 0x10u) != 0u;
    TRIGGER_OPCODE(0x823bu, 3u, 4u, 1u);
    TRIGGER_OPCODE(0x823eu, 2u, 2u, 0u);
    TRIGGER_OPCODE(0x8240u, 2u, regions ? 2u : 3u, 0u);
    if (regions) {
        TRIGGER_OPCODE(0x8242u, 3u, 4u, 1u);
        TRIGGER_OPCODE(0x8245u, 2u, 2u, 0u);
        TRIGGER_OPCODE(0x8247u, 2u, 3u, 0u);
    }
    TRIGGER_OPCODE(0x829eu, 1u, 4u, 1u);
#undef TRIGGER_OPCODE
    return count;
}

typedef struct FieldTriggerClock {
    FixedSpanClock span;
    ActorPushedCall call;
    FixedSpanOpcode plan[LUFIA2_FIELD_TRIGGER_OWN_OPCODES];
    unsigned own_opcodes;
} FieldTriggerClock;

static bool FieldTriggerRangeQuiet(void) {
#if defined(LUFIA2_HAVE_NATIVE_FIELD_REGION_PHASES)
    return Lufia2DecompFrameRangeAllows(0x8381c6u, 0x8382a0u, FieldRegionHookOwned);
#else
    return Lufia2DecompFrameRangeQuiet(0x8381c6u, 0x8382a0u);
#endif
}

static bool FieldTriggerClockSupported(const CpuState *cpu, uint16_t stack,
    uint8_t return_frame) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->host_return_valid != return_frame || cpu->S != stack ||
        stack < 0x1f20u || stack > 0x1ffdu || !(cpu->ram[0x0622u] & 0x80u) ||
        g_interp_apu_driving || !Lufia2DecompFramePairedCallReady(cpu, 0x8381c6u) ||
        Lufia2DecompFramePairedReturnPc(cpu, 0x8381c6u) != 0x83808fu ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIELD_TRIGGER_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x8381c6u, 0x1ffdu,
            false, 1363u, FIELD_TRIGGER_CLOCK_RESERVE) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x1829fu ||
        !FieldTriggerRangeQuiet())
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < FIELD_TRIGGER_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void BeginFieldTriggerStage(FieldTriggerClock *clock, unsigned stage) {
    const unsigned count = PlanFieldTriggerClocks(clock->call.cpu->ram, stage, clock->plan);
    clock->span = (FixedSpanClock){.cpu = clock->call.cpu, .bank = 0x830000u,
        .timing = clock->plan, .count = count};
    clock->own_opcodes += count;
    assert(clock->own_opcodes + 1u <= LUFIA2_FIELD_TRIGGER_OWN_OPCODES);
}

static uint8_t FieldTriggerChildClock(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    FieldTriggerClock *clock = context;
    const bool timer = target == 0x80cbaeu && site == 0x8381cbu && frame_size == 3u;
    const bool idle = target == 0x8380cdu && site == 0x8381d8u && frame_size == 2u;
    assert(timer || idle);
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    Lufia2DecompFramePairedAccountSpan(clock->call.cpu, 0x8381c6u,
        clock->span.count, clock->span.progress_mask);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = site + frame_size + 1u;
    const uint16_t stack = (uint16_t)(clock->call.frame.entry_s - 1u);
    if (!FieldTriggerClockSupported(clock->call.cpu, stack, frame_size)) {
        clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
            clock->call.cpu, landing, landing,
            clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
        return 0u;
    }
    BeginFieldTriggerStage(clock, timer ? 1u : 2u);
    return 1u;
}

static RecompReturn ActorBridgeTimedFieldTrigger(CpuState *cpu) {
    if (!FieldTriggerClockSupported(cpu, cpu->S, 2u)) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x81c6u;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    for (unsigned slot = 0u; slot < 8u; ++slot)
        if (cpu->ram[0x1d18cu + slot] == 0x81u) {
            const uint32_t entry = (uint32_t)cpu->PB << 16 | 0x81c6u;
            return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
        }
    FieldTriggerClock clock = {0};
    clock.call.cpu = cpu;
    clock.call.frame = (ActorBridgeFrame){cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, 0x8381c6u)};
    clock.call.unwound = RECOMP_RETURN_NORMAL;
    BeginFieldTriggerStage(&clock, 0u);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldTriggerUpdateWithServices(
        &memory, &state, FieldTriggerChildClock, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x83829fu);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    bool return_progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(
        cpu, &clock.call.frame, result.pc, &return_progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x8381c6u,
        clock.span.count + 1u,
        clock.span.progress_mask | (unsigned)return_progress << clock.span.count);
    return returned;
}
