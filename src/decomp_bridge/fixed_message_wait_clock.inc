enum { FIXED_MESSAGE_CLOCK_ROOM = 256u };

static const FixedSpanOpcode fixed_message_first_message[] = {
    {0xad96u, 3u, 3u, 0u}, {0xad99u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_first_display[] = {
    {0xad9du, 3u, 3u, 0u}, {0xada0u, 3u, 5u, 2u},
    {0xada3u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_first_wait[] = {
    {0xada7u, 2u, 2u, 0u}, {0xada9u, 1u, 3u, 1u},
    {0xadaau, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_first_input[] = {
    {0xadaeu, 2u, 2u, 0u}, {0xadb0u, 4u, 5u, 1u},
    {0xadb4u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_first_done[] = {
    {0xadb8u, 1u, 4u, 1u}, {0xadb9u, 1u, 2u, 0u},
    {0xadbau, 2u, 2u, 0u}, {0xadbcu, 2u, 3u, 0u}
};

static const FixedSpanOpcode fixed_message_first_repeat[] = {
    {0xadb8u, 1u, 4u, 1u}, {0xadb9u, 1u, 2u, 0u},
    {0xadbau, 2u, 3u, 0u}, {0xadd5u, 1u, 3u, 1u},
    {0xadd6u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_repeat_input[] = {
    {0xaddau, 2u, 2u, 0u}, {0xaddcu, 4u, 5u, 1u},
    {0xade0u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_repeat_wait[] = {
    {0xade4u, 1u, 4u, 1u}, {0xade5u, 1u, 2u, 0u},
    {0xade6u, 2u, 3u, 0u}, {0xadd5u, 1u, 3u, 1u},
    {0xadd6u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_repeat_done[] = {
    {0xade4u, 1u, 4u, 1u}, {0xade5u, 1u, 2u, 0u},
    {0xade6u, 2u, 2u, 0u}, {0xade8u, 3u, 3u, 0u},
    {0xadebu, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_second_display[] = {
    {0xadefu, 3u, 3u, 0u}, {0xadf2u, 3u, 5u, 2u},
    {0xadf5u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_second_wait[] = {
    {0xadf9u, 2u, 2u, 0u}, {0xadfbu, 1u, 3u, 1u},
    {0xadfcu, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_second_input[] = {
    {0xae00u, 2u, 2u, 0u}, {0xae02u, 4u, 5u, 1u},
    {0xae06u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_second_repeat[] = {
    {0xae0au, 1u, 4u, 1u}, {0xae0bu, 1u, 2u, 0u},
    {0xae0cu, 2u, 3u, 0u}, {0xadfbu, 1u, 3u, 1u},
    {0xadfcu, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_second_done[] = {
    {0xae0au, 1u, 4u, 1u}, {0xae0bu, 1u, 2u, 0u},
    {0xae0cu, 2u, 2u, 0u}
};

static const FixedSpanOpcode fixed_message_single_message[] = {
    {0xaeeau, 3u, 3u, 0u}, {0xaeedu, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_single_display[] = {
    {0xaef1u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_single_wait[] = {
    {0xaef5u, 2u, 2u, 0u}, {0xaef7u, 1u, 3u, 1u},
    {0xaef8u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_single_input[] = {
    {0xaefcu, 2u, 2u, 0u}, {0xaefeu, 4u, 5u, 1u},
    {0xaf02u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_single_repeat[] = {
    {0xaf06u, 1u, 4u, 1u}, {0xaf07u, 1u, 2u, 0u},
    {0xaf08u, 2u, 3u, 0u}, {0xaef7u, 1u, 3u, 1u},
    {0xaef8u, 4u, 8u, 3u}
};

static const FixedSpanOpcode fixed_message_single_done[] = {
    {0xaf06u, 1u, 4u, 1u}, {0xaf07u, 1u, 2u, 0u},
    {0xaf08u, 2u, 2u, 0u}, {0xaf0au, 2u, 3u, 0u}
};

typedef struct FixedMessageWaitClock {
    FixedSpanClock span;
    ActorPushedCall call;
    uint32_t entry;
    uint32_t return_pc;
} FixedMessageWaitClock;

static bool FixedMessageClockSupported(const CpuState *cpu, uint32_t entry) {
    const unsigned fence = g_snes && !g_snes->hdmaBeamOff &&
        g_snes->vPos < 225u ? 1024u : 1364u;
    if (cpu->D || cpu->_flag_D || cpu->DB != 0x97u || cpu->x_flag ||
        cpu->m_flag != 1u || cpu->host_return_valid != 3u || cpu->S < 0x1f20u ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, entry)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIXED_MESSAGE_CLOCK_ROOM ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x810000u, 0x1ffbu, false,
            fence - 1u, FIXED_MESSAGE_CLOCK_ROOM) || g_snes->vPos >= 262u ||
        fence - g_snes->hPos < FIXED_MESSAGE_CLOCK_ROOM)
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles);
}

static bool PrepareFixedMessageClock(FixedMessageWaitClock *clock, uint32_t landing) {
    CpuState *cpu = clock->call.cpu;
    if (!FixedMessageClockSupported(cpu, clock->entry))
        return false;
    const bool expires = cpu->ram[cpu->S + 1u] == 1u;
    const FixedSpanOpcode *timing = NULL;
    unsigned count = 0u;
    clock->return_pc = 0u;
#define FIXED_MESSAGE_PLAN(name) \
    timing = fixed_message_##name; \
    count = sizeof fixed_message_##name / sizeof(FixedSpanOpcode)
    switch (landing) {
    case 0x81ad96u: FIXED_MESSAGE_PLAN(first_message); break;
    case 0x81ad9du: FIXED_MESSAGE_PLAN(first_display); break;
    case 0x81ada7u: FIXED_MESSAGE_PLAN(first_wait); break;
    case 0x81adaeu: FIXED_MESSAGE_PLAN(first_input); break;
    case 0x81adb8u:
        if (expires) {
            FIXED_MESSAGE_PLAN(first_done);
            clock->return_pc = 0x81ae0eu;
        } else {
            FIXED_MESSAGE_PLAN(first_repeat);
        }
        break;
    case 0x81addau: FIXED_MESSAGE_PLAN(repeat_input); break;
    case 0x81ade4u:
        if (expires) {
            FIXED_MESSAGE_PLAN(repeat_done);
        } else {
            FIXED_MESSAGE_PLAN(repeat_wait);
        }
        break;
    case 0x81adefu: FIXED_MESSAGE_PLAN(second_display); break;
    case 0x81adf9u: FIXED_MESSAGE_PLAN(second_wait); break;
    case 0x81ae00u: FIXED_MESSAGE_PLAN(second_input); break;
    case 0x81ae0au:
        if (expires) {
            FIXED_MESSAGE_PLAN(second_done);
            clock->return_pc = 0x81ae0eu;
        } else {
            FIXED_MESSAGE_PLAN(second_repeat);
        }
        break;
    case 0x81aeeau: FIXED_MESSAGE_PLAN(single_message); break;
    case 0x81aef1u: FIXED_MESSAGE_PLAN(single_display); break;
    case 0x81aef5u: FIXED_MESSAGE_PLAN(single_wait); break;
    case 0x81aefcu: FIXED_MESSAGE_PLAN(single_input); break;
    case 0x81af06u:
        if (expires) {
            FIXED_MESSAGE_PLAN(single_done);
            clock->return_pc = 0x81af36u;
        } else {
            FIXED_MESSAGE_PLAN(single_repeat);
        }
        break;
    default: return false;
    }
#undef FIXED_MESSAGE_PLAN
    uint32_t first = landing, end = clock->return_pc ? clock->return_pc + 1u : landing;
    for (unsigned i = 0u; i < count; ++i) {
        const uint32_t pc = 0x810000u | timing[i].pc;
        if (pc < first) first = pc;
        if (pc + timing[i].fetched > end) end = pc + timing[i].fetched;
    }
    if (!Lufia2DecompFrameRangeQuiet(first, end))
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x810000u;
    clock->span.timing = timing;
    clock->span.count = count;
    return true;
}

static uint8_t FixedMessageChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    FixedMessageWaitClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(frame_size == 3u &&
        (0x810000u | clock->span.timing[clock->span.count - 1u].pc) == site);
    /* Inherit readiness only across the accepted parent span. */
    bool copied = false;
    if (target == 0x8594e7u) {
        CpuState *cpu = clock->call.cpu;
        ActorBridgeStore(cpu, state);
        cpu->PB = 0x85u;
        if (MessageCopyClockSupported(cpu, true)) {
            const RecompReturn result = RunTimedMessageCopy(cpu);
            assert(result == RECOMP_RETURN_NORMAL);
            (void)result;
            ActorBridgeLoad(cpu, state);
            copied = true;
        }
    }
    if (!copied &&
        !ActorBridgeEffectEngineChild(&clock->call, state, target, site, frame_size))
        return 0u;
    if (PrepareFixedMessageClock(clock, site + 4u))
        return 1u;
    const RecompReturn result = interp_tier_dispatch_tail(clock->call.cpu,
        site + 4u, site, clock->call.frame.entry_s, clock->call.frame.hrv);
    clock->call.unwound = (RecompReturn)((int)result + 1);
    return 0u;
}

static RecompReturn ActorBridgeFixedMessageWait(CpuState *cpu, uint32_t entry,
    ActorSceneOwnersFunction run) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    FixedMessageWaitClock clock = {0};
    clock.entry = entry;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareFixedMessageClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    frame.host_return_pc24 = (uint32_t)cpu->ram[cpu->S + 3u] << 16 |
        (uint16_t)(((uint16_t)cpu->ram[cpu->S + 2u] << 8 |
                    cpu->ram[cpu->S + 1u]) + 1u);
    clock.call.frame = frame;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, FixedMessageChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        unsigned end = clock.span.index;
        while (end < clock.span.count &&
            (0x810000u | clock.span.timing[end].pc) != result.pc)
            ++end;
        assert(end < clock.span.count);
        clock.span.count = end;
    }
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    assert(clock.return_pc == result.pc);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}
