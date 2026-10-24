enum { WORLD_WINDOW_CLOCK_CAPACITY = 24u };

typedef struct WorldWindowClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[WORLD_WINDOW_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} WorldWindowClock;

static void WorldWindowOpcode(WorldWindowClock *clock, uint16_t pc,
                               unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < WORLD_WINDOW_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static void WorldWindowCall(WorldWindowClock *clock, uint16_t pc) {
    WorldWindowOpcode(clock, pc, 3u, 6u, 2u);
}

static bool WorldWindowRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x86u && !cpu->emulation && cpu->m_flag == 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D && cpu->DB == 0x86u &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static bool WorldWindowTimingSupported(const WorldWindowClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!WorldWindowRegistersSupported(cpu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = clock->returning
        ? cpu_region_speed(clock->next) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x860000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x860000u, 0x1ffcu, true, 1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        if (!g_snes->hdmaBeamOff ||
            (225u - g_snes->vPos) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareWorldWindowClock(WorldWindowClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x860000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!WorldWindowRegistersSupported(clock->call.cpu))
        return false;
    switch (pc) {
    case 0x86b0bau:
    case 0x86b0c8u:
    case 0x86b0cfu:
    case 0x86b0d6u:
    case 0x86b0ddu:
    case 0x86b0e4u:
        WorldWindowOpcode(clock, (uint16_t)pc, 3u, 3u, 0u);
        WorldWindowOpcode(clock, (uint16_t)(pc + 3u), 2u, 2u, 0u);
        if (pc != 0x86b0bau)
            WorldWindowOpcode(clock, (uint16_t)(pc + 5u), 2u, 3u, 0u);
        WorldWindowCall(clock, 0xb0bfu);
        break;
    case 0x86b0c2u:
        WorldWindowOpcode(clock, 0xb0c2u, 2u, 2u, 0u);
        WorldWindowCall(clock, 0xb0c4u);
        break;
    case 0x86b0ebu:
        WorldWindowOpcode(clock, 0xb0ebu, 3u, 3u, 0u);
        WorldWindowOpcode(clock, 0xb0eeu, 3u, 3u, 0u);
        WorldWindowOpcode(clock, 0xb0f1u, 2u, 5u, 2u);
        WorldWindowOpcode(clock, 0xb0f3u, 3u, 3u, 0u);
        WorldWindowOpcode(clock, 0xb0f6u, 2u, 5u, 2u);
        WorldWindowOpcode(clock, 0xb0f8u, 2u, 2u, 0u);
        WorldWindowOpcode(clock, 0xb0fau, 2u, 4u, 1u);
        WorldWindowOpcode(clock, 0xb0fcu, 2u, 2u, 0u);
        WorldWindowOpcode(clock, 0xb0feu, 2u, 4u, 1u);
        WorldWindowOpcode(clock, 0xb100u, 2u, 2u, 0u);
        WorldWindowCall(clock, 0xb102u);
        break;
    case 0x86b264u:
    case 0x86b272u:
    case 0x86b277u:
        WorldWindowOpcode(clock, (uint16_t)pc, 3u, 3u, 0u);
        if (pc != 0x86b264u)
            WorldWindowOpcode(clock, (uint16_t)(pc + 3u), 2u, 3u, 0u);
        WorldWindowOpcode(clock, 0xb267u, 2u, 2u, 0u);
        WorldWindowCall(clock, 0xb269u);
        break;
    case 0x86b26cu:
        WorldWindowOpcode(clock, 0xb26cu, 2u, 2u, 0u);
        WorldWindowCall(clock, 0xb26eu);
        break;
    case 0x86b2bbu:
        WorldWindowOpcode(clock, 0xb2bbu, 2u, 3u, 1u);
        WorldWindowOpcode(clock, 0xb2bdu, 3u, 3u, 0u);
        WorldWindowCall(clock, 0xb2c0u);
        break;
    case 0x86b2c3u:
    case 0x86b2c9u:
    case 0x86b2cfu:
    case 0x86b2d5u:
    case 0x86b2dbu:
        WorldWindowOpcode(clock, (uint16_t)pc, 3u, 3u, 0u);
        WorldWindowCall(clock, (uint16_t)(pc + 3u));
        break;
    case 0x86b2e1u:
        WorldWindowOpcode(clock, 0xb2e1u, 2u, 2u, 0u);
        WorldWindowCall(clock, 0xb2e3u);
        break;
    case 0x86bfebu:
    case 0x86c02cu:
        WorldWindowOpcode(clock, (uint16_t)pc, 3u, 6u, 2u);
        WorldWindowOpcode(clock, (uint16_t)(pc + 3u), 3u, 6u, 2u);
        WorldWindowCall(clock, (uint16_t)(pc + 6u));
        break;
    case 0x86c036u:
        WorldWindowOpcode(clock, 0xc036u, 2u, 2u, 0u);
        WorldWindowOpcode(clock, 0xc038u, 4u, 8u, 3u);
        break;
    case 0x86c03cu:
        WorldWindowOpcode(clock, 0xc03cu, 4u, 8u, 3u);
        break;
    case 0x86b0c7u:
    case 0x86b105u:
    case 0x86b271u:
    case 0x86b2e6u:
    case 0x86bff4u:
    case 0x86c035u:
    case 0x86c040u:
        clock->returning = true;
        break;
    default:
        return false;
    }
    return WorldWindowTimingSupported(clock);
}

static uint8_t WorldWindowChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    WorldWindowClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareWorldWindowClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedWorldWindow(CpuState *cpu, uint32_t entry,
                                               ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    WorldWindowClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareWorldWindowClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, WorldWindowChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
