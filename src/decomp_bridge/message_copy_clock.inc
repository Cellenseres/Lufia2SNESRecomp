enum { MESSAGE_COPY_CLOCK_ROOM = 6144u, MESSAGE_COPY_LENGTH = 37u };

static const FixedSpanOpcode message_copy_setup[] = {
    {0x94e7u, 1u, 3u, 1u}, {0x94e8u, 1u, 3u, 1u},
    {0x94e9u, 2u, 3u, 0u}, {0x94ebu, 1u, 4u, 2u},
    {0x94ecu, 1u, 4u, 2u}, {0x94edu, 1u, 4u, 2u},
    {0x94eeu, 2u, 3u, 0u}, {0x94f0u, 1u, 3u, 1u},
    {0x94f1u, 1u, 4u, 1u}, {0x94f2u, 3u, 3u, 0u},
    {0x94f5u, 3u, 5u, 2u}, {0x94f8u, 3u, 4u, 1u},
    {0x94fbu, 3u, 3u, 0u}
};

static const FixedSpanOpcode message_copy_loop[][5] = {
    {{0x94feu, 3u, 5u, 1u}, {0x9501u, 3u, 4u, 1u},
     {0x9504u, 1u, 2u, 0u}, {0x9505u, 1u, 2u, 0u},
     {0x9506u, 2u, 3u, 0u}},
    {{0x94feu, 3u, 5u, 1u}, {0x9501u, 3u, 4u, 1u},
     {0x9504u, 1u, 2u, 0u}, {0x9505u, 1u, 2u, 0u},
     {0x9506u, 2u, 2u, 0u}}
};

static const FixedSpanOpcode message_copy_restore[] = {
    {0x9508u, 2u, 3u, 0u}, {0x950au, 1u, 5u, 2u},
    {0x950bu, 1u, 5u, 2u}, {0x950cu, 1u, 5u, 2u},
    {0x950du, 1u, 4u, 1u}, {0x950eu, 1u, 4u, 1u}
};

typedef struct MessageCopyClock {
    FixedSpanClock span;
    unsigned copied;
    bool started;
} MessageCopyClock;

static void BeginMessageCopyOpcode(MessageCopyClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count)
            return;
        if (clock->span.timing == message_copy_restore)
            return;
        if (clock->started)
            ++clock->copied;
        clock->started = true;
        clock->span.index = 0u;
        if (clock->copied == MESSAGE_COPY_LENGTH) {
            clock->span.timing = message_copy_restore;
            clock->span.count = sizeof message_copy_restore / sizeof(FixedSpanOpcode);
        } else {
            clock->span.timing = message_copy_loop[
                clock->copied + 1u == MESSAGE_COPY_LENGTH];
            clock->span.count = 5u;
        }
    }
}

static uint8_t MessageCopyRead(void *context, uint32_t address) {
    MessageCopyClock *clock = context;
    BeginMessageCopyOpcode(clock);
    return FixedSpanRead(&clock->span, address);
}

static void MessageCopyWrite(void *context, uint32_t address, uint8_t value) {
    MessageCopyClock *clock = context;
    BeginMessageCopyOpcode(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool MessageCopyClockSupported(const CpuState *cpu) {
    if (cpu->D || cpu->_flag_D || cpu->host_return_valid != 3u ||
        cpu->S < 0x1f20u || cpu->X < 0x8000u || cpu->X > 0xffdbu ||
        (cpu->x_flag && (cpu->X > 0xffu || cpu->Y > 0xffu)) ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, 0x8594e7u)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - MESSAGE_COPY_CLOCK_ROOM ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x8594e7u, 0x1ffbu, true,
            1363u, MESSAGE_COPY_CLOCK_ROOM) ||
        g_snes->vPos < 225u || g_snes->vPos >= 262u ||
        (262u - g_snes->vPos) * 1364u - g_snes->hPos < MESSAGE_COPY_CLOCK_ROOM ||
        !Lufia2DecompFrameRangeQuiet(0x8594e7u, 0x859510u))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedMessageCopy(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (!MessageCopyClockSupported(cpu))
        return ActorBridgeFallback(cpu, &frame, 0x8594e7u);
    frame.host_return_pc24 = (uint32_t)cpu->ram[cpu->S + 3u] << 16 |
        (uint16_t)(((uint16_t)cpu->ram[cpu->S + 2u] << 8 |
                    cpu->ram[cpu->S + 1u]) + 1u);
    MessageCopyClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x850000u;
    clock.span.timing = message_copy_setup;
    clock.span.count = sizeof message_copy_setup / sizeof(FixedSpanOpcode);
    const Lufia2Memory memory = {MessageCopyRead, MessageCopyWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleCopyMessageName(&memory, &state);
    BeginMessageCopyOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x85950fu);
    assert(clock.copied == MESSAGE_COPY_LENGTH &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}
