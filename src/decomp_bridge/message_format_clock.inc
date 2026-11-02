enum {
    MESSAGE_FORMAT_CLOCK_ROOM = 40960u,
    MESSAGE_FORMAT_SOURCE_BYTES = 73u,
    MESSAGE_FORMAT_CYCLE_LIMIT = MESSAGE_FORMAT_SOURCE_BYTES * 47u + 36u * 24u + 64u,
    MESSAGE_FORMAT_REFRESH_LIMIT = 32u
};

_Static_assert(MESSAGE_FORMAT_CYCLE_LIMIT * 8u + MESSAGE_FORMAT_REFRESH_LIMIT * 40u
    <= MESSAGE_FORMAT_CLOCK_ROOM, "Message format clock reservation");
_Static_assert((MESSAGE_FORMAT_CYCLE_LIMIT * 8u + 1363u +
    MESSAGE_FORMAT_REFRESH_LIMIT * 40u + 1363u) / 1364u <= MESSAGE_FORMAT_REFRESH_LIMIT,
    "Message format refresh reservation");

static const FixedSpanOpcode message_format_setup[] = {
    {0xc5b5u, 3u, 3u, 0u}, {0xc5b8u, 3u, 4u, 1u},
    {0xc5bbu, 3u, 5u, 1u}
};
static const FixedSpanOpcode message_format_character[] = {
    {0xc5beu, 2u, 2u, 0u}, {0xc5c0u, 2u, 2u, 0u},
    {0xc5c2u, 2u, 2u, 0u}, {0xc5c4u, 2u, 2u, 0u},
    {0xc5c6u, 2u, 3u, 0u}, {0xc5d1u, 4u, 5u, 1u},
    {0xc5d5u, 3u, 6u, 2u}, {0xc5d8u, 1u, 2u, 0u},
    {0xc5d9u, 1u, 2u, 0u}, {0xc5dau, 2u, 3u, 0u},
    {0xc5bbu, 3u, 5u, 1u}
};
static const FixedSpanOpcode message_format_control[] = {
    {0xc5beu, 2u, 2u, 0u}, {0xc5c0u, 2u, 2u, 0u},
    {0xc5c2u, 2u, 2u, 0u}, {0xc5c4u, 2u, 2u, 0u},
    {0xc5c6u, 2u, 2u, 0u}, {0xc5c8u, 4u, 5u, 1u},
    {0xc5ccu, 1u, 2u, 0u}, {0xc5cdu, 1u, 2u, 0u},
    {0xc5ceu, 3u, 5u, 1u}
};
static const FixedSpanOpcode message_format_argument[] = {
    {0xc5d1u, 4u, 5u, 1u}, {0xc5d5u, 3u, 6u, 2u},
    {0xc5d8u, 1u, 2u, 0u}, {0xc5d9u, 1u, 2u, 0u},
    {0xc5dau, 2u, 3u, 0u}, {0xc5bbu, 3u, 5u, 1u}
};
static const FixedSpanOpcode message_format_newline[] = {
    {0xc5beu, 2u, 2u, 0u}, {0xc5c0u, 2u, 2u, 0u},
    {0xc5c2u, 2u, 3u, 0u}, {0xc5e6u, 3u, 4u, 1u}
};
static const FixedSpanOpcode message_format_end[] = {
    {0xc5beu, 2u, 3u, 0u}, {0xc5fau, 3u, 4u, 1u}
};
static const FixedSpanOpcode message_format_line_padding[] = {
    {0xc5e9u, 2u, 2u, 0u}, {0xc5ebu, 2u, 3u, 0u},
    {0xc5dcu, 2u, 2u, 0u}, {0xc5deu, 4u, 5u, 1u},
    {0xc5e2u, 1u, 2u, 0u}, {0xc5e3u, 3u, 6u, 2u},
    {0xc5e6u, 3u, 4u, 1u}
};
static const FixedSpanOpcode message_format_line_done[] = {
    {0xc5e9u, 2u, 2u, 0u}, {0xc5ebu, 2u, 2u, 0u},
    {0xc5edu, 1u, 2u, 0u}, {0xc5eeu, 2u, 3u, 0u},
    {0xc5bbu, 3u, 5u, 1u}
};
static const FixedSpanOpcode message_format_end_padding[] = {
    {0xc5fdu, 2u, 2u, 0u}, {0xc5ffu, 2u, 3u, 0u},
    {0xc5f0u, 2u, 2u, 0u}, {0xc5f2u, 4u, 5u, 1u},
    {0xc5f6u, 1u, 2u, 0u}, {0xc5f7u, 3u, 6u, 2u},
    {0xc5fau, 3u, 4u, 1u}
};
static const FixedSpanOpcode message_format_restore[] = {
    {0xc5fdu, 2u, 2u, 0u}, {0xc5ffu, 2u, 2u, 0u},
    {0xc601u, 1u, 2u, 0u}, {0xc602u, 4u, 5u, 1u},
    {0xc606u, 2u, 3u, 0u}, {0xc608u, 1u, 5u, 2u},
    {0xc609u, 1u, 5u, 2u}, {0xc60au, 1u, 5u, 2u},
    {0xc60bu, 1u, 4u, 1u}, {0xc60cu, 1u, 4u, 1u}
};

static void SetMessageFormatSpan(FixedSpanClock *clock,
    const FixedSpanOpcode *timing, unsigned count) {
    assert(clock->index == clock->count && !clock->remaining);
    clock->timing = timing;
    clock->count = count;
    clock->index = 0u;
}

static uint8_t MessageFormatRead(void *context, uint32_t address) {
    FixedSpanClock *clock = context;
    BeginFixedSpanOpcode(clock);
    const uint16_t source = clock->timing[clock->index].pc;
    const uint8_t value = FixedSpanRead(clock, address);
#define MESSAGE_FORMAT_SPAN(name) \
    SetMessageFormatSpan(clock, message_format_##name, \
        sizeof message_format_##name / sizeof(FixedSpanOpcode))
    if (source == 0xc5bbu) {
        if (!value) {
            MESSAGE_FORMAT_SPAN(end);
        } else if (value == 0x0au) {
            MESSAGE_FORMAT_SPAN(newline);
        } else if (value < 0x10u) {
            MESSAGE_FORMAT_SPAN(control);
        } else {
            MESSAGE_FORMAT_SPAN(character);
        }
    } else if (source == 0xc5ceu) {
        MESSAGE_FORMAT_SPAN(argument);
    } else if (source == 0xc5e6u) {
        if (value == 18u) {
            MESSAGE_FORMAT_SPAN(line_done);
        } else {
            MESSAGE_FORMAT_SPAN(line_padding);
        }
    } else if (source == 0xc5fau) {
        if (value == 36u) {
            MESSAGE_FORMAT_SPAN(restore);
        } else {
            MESSAGE_FORMAT_SPAN(end_padding);
        }
    }
#undef MESSAGE_FORMAT_SPAN
    return value;
}

static bool MessageFormatSourceSupported(const CpuState *cpu) {
    if ((cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->Y < 0x2000u || cpu->Y > 0xf000u - MESSAGE_FORMAT_SOURCE_BYTES)
        return false;
    const uint8_t *source = cpu->ram + (cpu->DB - 0x7eu) * 65536u + cpu->Y;
    unsigned columns = 0u;
    for (unsigned index = 0u; index < MESSAGE_FORMAT_SOURCE_BYTES;) {
        const uint8_t value = source[index++];
        if (!value)
            return true;
        if (value == 0x0au) {
            if (columns > 18u)
                return false;
            columns = 18u;
        } else {
            if (value < 0x10u && ++index > MESSAGE_FORMAT_SOURCE_BYTES)
                return false;
            if (++columns > 36u)
                return false;
        }
    }
    return false;
}

static bool MessageFormatClockSupported(const CpuState *cpu) {
    if (cpu->m_flag != 1u || cpu->x_flag || cpu->host_return_valid > 3u ||
        (cpu->host_return_valid && cpu->host_return_valid != 3u) ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, 0x85c5b5u)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - MESSAGE_FORMAT_CLOCK_ROOM ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x85c5b5u, 0x1ff4u, false,
            1363u, MESSAGE_FORMAT_CLOCK_ROOM) ||
        g_snes->vPos < 225u || g_snes->vPos >= 262u ||
        (262u - g_snes->vPos) * 1364u - g_snes->hPos < MESSAGE_FORMAT_CLOCK_ROOM ||
        !Lufia2DecompFrameRangeQuiet(0x85c5b5u, 0x85c60eu))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles) &&
        MessageFormatSourceSupported(cpu);
}

static RecompReturn ActorBridgeTimedMessageFormat(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (!MessageFormatClockSupported(cpu))
        return ActorBridgeFallback(cpu, &frame, 0x85c5b5u);
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = message_format_setup;
    clock.count = sizeof message_format_setup / sizeof(FixedSpanOpcode);
    const Lufia2Memory memory = {MessageFormatRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleFormatResourceMessage(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x85c60du);
    assert(clock.timing == message_format_restore &&
        clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, NULL, result.pc);
}
