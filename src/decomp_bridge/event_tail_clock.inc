enum { EVENT_TAIL_CLOCK_CAPACITY = 16u };

typedef struct EventScriptCursor {
    uint16_t position;
    uint8_t bank;
    uint8_t running_bank;
} EventScriptCursor;

typedef struct EventTailClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[EVENT_TAIL_CLOCK_CAPACITY];
    ActorPushedCall call;
    bool complete;
} EventTailClock;

static void EventTailOpcode(EventTailClock *clock, uint16_t pc,
                             unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < EVENT_TAIL_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool EventScriptByte(const CpuState *cpu, const EventScriptCursor *cursor,
                             uint8_t *value) {
    if (cursor->position < 0x8000u)
        return false;
    if (cursor->bank == 0x7eu || cursor->bank == 0x7fu) {
        *value = cpu->ram[((unsigned)cursor->bank & 1u) * 0x10000u + cursor->position];
        return true;
    }
    if (cursor->bank < 0x80u || cursor->bank >= 0xd0u ||
        !g_snes || !g_snes->cart || !g_snes->cart->rom)
        return false;
    const uint32_t offset = ((uint32_t)cursor->bank & 0x7fu) * 0x8000u +
        (cursor->position & 0x7fffu);
    if (offset >= g_snes->cart->romSize)
        return false;
    *value = g_snes->cart->rom[offset];
    return true;
}

static bool EventTailRegistersSupported(const CpuState *cpu) {
    if (!cpu->ram || !g_snes || !g_snes->cart ||
        g_snes->cart->type != CART_LOROM || !g_snes->cart->rom ||
        cpu->PB != 0x80u || cpu->emulation || cpu->m_flag > 1u ||
        cpu->x_flag || cpu->D || cpu->_flag_D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return false;
    const EventScriptCursor cursor = {cpu->Y, cpu->DB, cpu->ram[0x1d199u]};
    uint8_t value;
    return EventScriptByte(cpu, &cursor, &value);
}

static bool EventTailTimingSupported(const EventTailClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x800000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x800000u, 0x1ffcu, true, 1363u, budget))
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

static bool PrepareEventTailClock(EventTailClock *clock, uint32_t pc) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->complete = false;
    const CpuState *cpu = clock->call.cpu;
    const bool word = pc == 0x80d2a5u || pc == 0x80d2adu;
    if (!EventTailRegistersSupported(cpu) || cpu->m_flag == word)
        return false;
    switch (pc) {
    case 0x80d2bbu:
    case 0x80d2e1u:
    case 0x80d2eeu:
    case 0x80d2b5u:
        EventTailOpcode(clock, (uint16_t)pc, 3u, 6u, 2u);
        break;
    case 0x80d2beu:
        EventTailOpcode(clock, 0xd2beu, 4u, 8u, 3u);
        break;
    case 0x80d2c2u:
    case 0x80d2b8u:
        EventTailOpcode(clock, (uint16_t)pc, 3u, 4u, 0u);
        clock->complete = true;
        break;
    case 0x80d2a5u:
        EventTailOpcode(clock, 0xd2a5u, 1u, 2u, 0u);
        EventTailOpcode(clock, 0xd2a6u, 4u, 6u, 2u);
        EventTailOpcode(clock, 0xd2aau, 3u, 6u, 2u);
        break;
    case 0x80d2adu:
        EventTailOpcode(clock, 0xd2adu, 2u, 3u, 0u);
        EventTailOpcode(clock, 0xd2afu, 3u, 4u, 0u);
        clock->complete = true;
        break;
    case 0x80d2e4u:
    case 0x80d2f1u: {
        const bool skip = ((cpu->A & 1u) != 0u) == (pc == 0x80d2f1u);
        EventTailOpcode(clock, (uint16_t)pc, 2u, 2u, 0u);
        EventTailOpcode(clock, (uint16_t)(pc + 2u), 2u, skip ? 3u : 2u, 0u);
        EventTailOpcode(clock, (uint16_t)(pc + (skip ? 7u : 4u)), 3u, 4u, 0u);
        EventTailOpcode(clock, skip ? 0xd2b2u : 0xd2a2u, 3u, 6u, 2u);
        break;
    }
    default:
        return false;
    }
    return EventTailTimingSupported(clock);
}

static uint8_t EventTailChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    EventTailClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareEventTailClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedEventTail(CpuState *cpu, uint32_t entry,
                                              ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    EventTailClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareEventTailClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, EventTailChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.pc == 0x80cc35u) {
        BeginFixedSpanOpcode(&clock.span);
        assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && clock.complete &&
            clock.span.index == clock.span.count && !clock.span.remaining);
    }
    const uint32_t source = clock.complete
        ? 0x800000u | clock.timing[clock.span.count - 1u].pc : result.pc;
    return interp_tier_dispatch_tail(cpu, result.pc, source, frame.entry_s, frame.hrv);
}
