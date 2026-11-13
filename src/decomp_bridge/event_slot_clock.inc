enum {
    EVENT_SLOT_CLOCK_RESERVE = 4096u,
    EVENT_SLOT_BODY_MAX_OPCODES = 117u,
    EVENT_SLOT_PROGRESS_SPAN = 32u
};

_Static_assert(EVENT_SLOT_BODY_MAX_OPCODES + 2u <= LUFIA2_EVENT_SLOT_OPCODE_BUDGET,
    "review event timer and original caller budget");

static unsigned PlanEventSlotClocks(const uint8_t timers[8], FixedSpanOpcode *plan) {
    for (unsigned slot = 0u; slot < 8u; ++slot)
        if (timers[slot] == 0x81u)
            return 0u;
    unsigned count = 0u;
#define EVENT_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    EVENT_OPCODE(0xcbaeu, 1u, 3u, 1u);
    EVENT_OPCODE(0xcbafu, 1u, 3u, 1u);
    EVENT_OPCODE(0xcbb0u, 2u, 3u, 0u);
    EVENT_OPCODE(0xcbb2u, 3u, 4u, 1u);
    EVENT_OPCODE(0xcbb5u, 2u, 2u, 0u);
    EVENT_OPCODE(0xcbb7u, 3u, 6u, 2u);
    EVENT_OPCODE(0xcbbau, 3u, 3u, 0u);
    for (unsigned slot = 0u; slot < 8u; ++slot) {
        const bool active = (timers[slot] & 0x80u) != 0u;
        EVENT_OPCODE(0xcbbdu, 4u, 5u, 1u);
        EVENT_OPCODE(0xcbc1u, 2u, active ? 2u : 3u, 0u);
        if (active) {
            EVENT_OPCODE(0xcbc3u, 1u, 2u, 0u);
            EVENT_OPCODE(0xcbc4u, 4u, 5u, 1u);
            EVENT_OPCODE(0xcbc8u, 2u, 2u, 0u);
            EVENT_OPCODE(0xcbcau, 2u, 3u, 0u);
        }
        EVENT_OPCODE(0xcbf0u, 1u, 2u, 0u);
        EVENT_OPCODE(0xcbf1u, 3u, 3u, 0u);
        EVENT_OPCODE(0xcbf4u, 2u, slot == 7u ? 2u : 3u, 0u);
    }
    EVENT_OPCODE(0xcbf6u, 3u, 3u, 0u);
    for (unsigned slot = 8u; slot-- > 0u;) {
        const bool active = timers[slot] > 0x81u;
        EVENT_OPCODE(0xcbf9u, 4u, 5u, 1u);
        EVENT_OPCODE(0xcbfdu, 2u, active ? 2u : 3u, 0u);
        if (active) {
            EVENT_OPCODE(0xcbffu, 2u, 2u, 0u);
            EVENT_OPCODE(0xcc01u, 3u, 6u, 2u);
            EVENT_OPCODE(0xcc04u, 2u, 3u, 0u);
            break;
        }
        EVENT_OPCODE(0xcc06u, 1u, 2u, 0u);
        EVENT_OPCODE(0xcc07u, 2u, slot == 0u ? 2u : 3u, 0u);
    }
    EVENT_OPCODE(0xcc09u, 3u, 4u, 1u);
    EVENT_OPCODE(0xcc0cu, 2u, 3u, 0u);
    EVENT_OPCODE(0xcc23u, 1u, 4u, 1u);
    EVENT_OPCODE(0xcc24u, 1u, 4u, 1u);
#undef EVENT_OPCODE
    assert(count <= EVENT_SLOT_BODY_MAX_OPCODES);
    return count;
}

typedef struct EventSlotClock {
    FixedSpanClock span;
    const FixedSpanOpcode *plan;
    unsigned count;
    unsigned offset;
} EventSlotClock;

static void PrepareEventSlotSpan(EventSlotClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || !clock->span.count)
            return;
        Lufia2DecompFramePairedAccountSpan(clock->span.cpu, 0x80cbaeu,
            clock->span.count, clock->span.progress_mask);
        clock->offset += clock->span.count;
        const unsigned remaining = clock->count - clock->offset;
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x800000u,
            .timing = clock->plan + clock->offset,
            .count = remaining < EVENT_SLOT_PROGRESS_SPAN ? remaining : EVENT_SLOT_PROGRESS_SPAN};
    }
}

static uint8_t EventSlotRead(void *context, uint32_t address) {
    EventSlotClock *clock = context;
    PrepareEventSlotSpan(clock);
    return FixedSpanRead(&clock->span, address);
}

static void EventSlotWrite(void *context, uint32_t address, uint8_t value) {
    EventSlotClock *clock = context;
    PrepareEventSlotSpan(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool EventSlotClockSupported(const CpuState *cpu) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x80u || !ActorBridgeSystemBank(cpu->DB) ||
        cpu->m_flag != 1u || cpu->x_flag > 1u || cpu->host_return_valid != 3u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu || g_interp_apu_driving ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x80cbaeu) ||
        Lufia2DecompFramePairedReturnPc(cpu, 0x80cbaeu) == UINT32_MAX ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - EVENT_SLOT_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x80cbaeu, 0x1ffcu,
            false, 1363u, EVENT_SLOT_CLOCK_RESERVE) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x4c25u ||
        !Lufia2DecompFrameRangeQuiet(0x80cbaeu, 0x80cc26u))
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < EVENT_SLOT_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedEventSlots(CpuState *cpu) {
    FixedSpanOpcode plan[EVENT_SLOT_BODY_MAX_OPCODES];
    const unsigned count = EventSlotClockSupported(cpu)
        ? PlanEventSlotClocks(cpu->ram + 0x1d18cu, plan) : 0u;
    if (!count) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0xcbaeu;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, 0x80cbaeu)};
    EventSlotClock clock = {.span = {.cpu = cpu, .bank = 0x800000u,
        .timing = plan, .count = EVENT_SLOT_PROGRESS_SPAN}, .plan = plan, .count = count};
    const Lufia2Memory memory = {EventSlotRead, EventSlotWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldEventTimerTick(&memory, &state);
    PrepareEventSlotSpan(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x80cc25u);
    assert(clock.offset == count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    const RecompReturn returned = ReturnTimedLongCall(cpu, &frame, 0x80cc25u);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x80cbaeu, 1u, 0u);
    assert(cpu->master_cycles - start <= EVENT_SLOT_CLOCK_RESERVE);
    return returned;
}
