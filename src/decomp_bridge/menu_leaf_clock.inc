enum {
    MENU_LEAF_CLOCK_ROOM = 512u,
    MENU_LEAF_CYCLE_LIMIT = 43u
};

_Static_assert(MENU_LEAF_CYCLE_LIMIT * 8u + 40u <= MENU_LEAF_CLOCK_ROOM,
    "Menu leaf clock reservation");

static const FixedSpanOpcode menu_pointer_offset[] = {
    {0x8af7u, 2u, 3u, 0u}, {0x8af9u, 2u, 7u, 5u},
    {0x8afbu, 2u, 4u, 2u}, {0x8afdu, 1u, 2u, 0u},
    {0x8afeu, 1u, 2u, 0u}, {0x8affu, 2u, 6u, 4u},
    {0x8b01u, 1u, 2u, 0u}, {0x8b02u, 2u, 4u, 2u},
    {0x8b04u, 2u, 4u, 2u}, {0x8b06u, 2u, 3u, 0u}
};
static const FixedSpanOpcode name_selection_decrement[] = {
    {0xf4d7u, 2u, 2u, 0u}, {0xf4d9u, 3u, 6u, 2u}
};
static const FixedSpanOpcode name_selection_increment[] = {
    {0xf4d7u, 2u, 3u, 0u}, {0xf4e5u, 3u, 6u, 2u},
    {0xf4e8u, 3u, 4u, 1u}
};
static const FixedSpanOpcode name_selection_decrement_done[] = {
    {0xf4dcu, 2u, 3u, 0u}, {0xf4f2u, 1u, 2u, 0u}
};
static const FixedSpanOpcode name_selection_decrement_wrap[] = {
    {0xf4dcu, 2u, 2u, 0u}, {0xf4deu, 2u, 2u, 0u},
    {0xf4e0u, 3u, 4u, 1u}, {0xf4e3u, 2u, 3u, 0u},
    {0xf4f2u, 1u, 2u, 0u}
};
static const FixedSpanOpcode name_selection_increment_done[] = {
    {0xf4ebu, 2u, 2u, 0u}, {0xf4edu, 2u, 3u, 0u},
    {0xf4f2u, 1u, 2u, 0u}
};
static const FixedSpanOpcode name_selection_increment_wrap[] = {
    {0xf4ebu, 2u, 2u, 0u}, {0xf4edu, 2u, 2u, 0u},
    {0xf4efu, 3u, 4u, 1u}, {0xf4f2u, 1u, 2u, 0u}
};

static void SetNameSelectionSpan(FixedSpanClock *clock,
    const FixedSpanOpcode *timing, unsigned count) {
    assert(clock->index == clock->count && !clock->remaining);
    clock->timing = timing;
    clock->count = count;
    clock->index = 0u;
}

static uint8_t NameSelectionRead(void *context, uint32_t address) {
    FixedSpanClock *clock = context;
    BeginFixedSpanOpcode(clock);
    const uint16_t source = clock->timing[clock->index].pc;
    const uint8_t value = FixedSpanRead(clock, address);
    if (source == 0xf4e8u) {
        if (value == 3u)
            SetNameSelectionSpan(clock, name_selection_increment_wrap,
                sizeof name_selection_increment_wrap / sizeof(FixedSpanOpcode));
        else
            SetNameSelectionSpan(clock, name_selection_increment_done,
                sizeof name_selection_increment_done / sizeof(FixedSpanOpcode));
    }
    return value;
}

static void NameSelectionWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanClock *clock = context;
    BeginFixedSpanOpcode(clock);
    const uint16_t source = clock->timing[clock->index].pc;
    FixedSpanWrite(clock, address, value);
    if (source == 0xf4d9u) {
        if (value & 0x80u)
            SetNameSelectionSpan(clock, name_selection_decrement_wrap,
                sizeof name_selection_decrement_wrap / sizeof(FixedSpanOpcode));
        else
            SetNameSelectionSpan(clock, name_selection_decrement_done,
                sizeof name_selection_decrement_done / sizeof(FixedSpanOpcode));
    }
}

static bool MenuPointerOffsetSourceSupported(const CpuState *cpu) {
    if (cpu->D)
        return false;
    uint32_t source = cpu->ram[0x5du] | (uint32_t)cpu->ram[0x5eu] << 8 |
        (uint32_t)cpu->ram[0x5fu] << 16;
    source = (source + cpu->Y) & 0xffffffu;
    if ((source >> 16 != 0x7eu && source >> 16 != 0x7fu) ||
        (source & 0xffffu) < 0x2000u || (source & 0xffffu) > 0xfffeu)
        return false;
    const uint8_t *value = cpu->ram + (source & 0x1ffffu);
    const unsigned pointer = value[0] | (unsigned)value[1] << 8;
    return pointer >= 0x2000u && pointer <= 0xfffeu;
}

static bool QuietSpanClockSupported(const CpuState *cpu, uint32_t entry,
    uint32_t end, uint8_t frame_size, bool wide_accumulator, unsigned room) {
    if (cpu->x_flag || (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        (cpu->host_return_valid && cpu->host_return_valid != frame_size) ||
        (!Lufia2DecompFrameReturnCpuReady() &&
         !Lufia2DecompFramePairedCallReady(cpu, entry)) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - room ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu - frame_size,
            wide_accumulator,
            1363u, room) ||
        g_snes->vPos < 225u || g_snes->vPos >= 262u ||
        (262u - g_snes->vPos) * 1364u - g_snes->hPos < room ||
        !Lufia2DecompFrameRangeQuiet(entry, end))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return phase < 1364u && (!charged || charged == cpu->master_cycles);
}

static bool QuietLeafClockSupported(const CpuState *cpu, uint32_t entry,
    uint32_t end, uint8_t frame_size, bool wide_accumulator) {
    return QuietSpanClockSupported(cpu, entry, end, frame_size, wide_accumulator,
        MENU_LEAF_CLOCK_ROOM);
}

static bool MenuLeafClockSupported(const CpuState *cpu, uint32_t entry,
    uint32_t end) {
    return QuietLeafClockSupported(cpu, entry, end, 2u, false);
}

static RecompReturn ActorBridgeTimedMenuLeaf(CpuState *cpu, bool pointer_offset) {
    const uint32_t entry = pointer_offset ? 0x808af7u : 0x82f4d7u;
    const uint32_t last = pointer_offset ? 0x808b08u : 0x82f4f3u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const uint32_t fallback = cpu->PB == ((entry >> 16) ^ 0x80u)
        ? entry & 0x7fffffu : entry;
    if (!MenuLeafClockSupported(cpu, entry, last + 1u) ||
        (pointer_offset && !MenuPointerOffsetSourceSupported(cpu)))
        return ActorBridgeFallback(cpu, &frame, fallback);
    if (frame.hrv == 2u)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    if (pointer_offset) {
        clock.timing = menu_pointer_offset;
        clock.count = sizeof menu_pointer_offset / sizeof(FixedSpanOpcode);
    } else if (cpu->_flag_C) {
        clock.timing = name_selection_decrement;
        clock.count = sizeof name_selection_decrement / sizeof(FixedSpanOpcode);
    } else {
        clock.timing = name_selection_increment;
        clock.count = sizeof name_selection_increment / sizeof(FixedSpanOpcode);
    }
    const Lufia2Memory memory = {
        pointer_offset ? FixedSpanRead : NameSelectionRead,
        pointer_offset ? FixedSpanWrite : NameSelectionWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = pointer_offset
        ? Lufia2MenuResolvePointerOffset(&memory, &state)
        : Lufia2MenuCycleNameEntrySelection(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == last);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

#if defined(LUFIA2_HAVE_NATIVE_FIXED_CURSOR)
static const FixedSpanOpcode fixed_cursor_setup[] = {
    {0xa00fu, 3u, 3u, 0u}, {0xa012u, 2u, 4u, 2u},
    {0xa014u, 3u, 3u, 0u}, {0xa017u, 2u, 4u, 2u},
    {0xa019u, 3u, 3u, 0u}, {0xa01cu, 3u, 3u, 0u},
    {0xa01fu, 3u, 6u, 2u}
};

typedef struct FixedCursorClock {
    FixedSpanClock span;
    ActorPushedCall call;
} FixedCursorClock;

static uint8_t FixedCursorChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    FixedCursorClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(target == 0x8289c4u && site == 0x82a01fu && frame_size == 2u);
    return ActorBridgePushedChild(&clock->call, state, target, site, frame_size);
}

static RecompReturn ActorBridgeTimedFixedCursor(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const uint32_t entry = 0x82a00fu;
    const uint32_t fallback = cpu->PB == 2u ? 0x02a00fu : entry;
    if (cpu->D || cpu->_flag_D ||
        !MenuLeafClockSupported(cpu, entry, 0x82a023u))
        return ActorBridgeFallback(cpu, &frame, fallback);
    if (frame.hrv == 2u)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    FixedCursorClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    clock.span.cpu = cpu;
    clock.span.bank = 0x820000u;
    clock.span.timing = fixed_cursor_setup;
    clock.span.count = sizeof fixed_cursor_setup / sizeof(FixedSpanOpcode);
    const Lufia2Memory memory = {
        FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuCreateFixedCursorPair(&memory, &state, FixedCursorChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x82a022u);
    ActorBridgeStore(cpu, &state);
    if (!MenuLeafClockSupported(cpu, entry, 0x82a023u))
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
#endif
