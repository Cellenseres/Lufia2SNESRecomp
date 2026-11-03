static void ActorBridgeEffectBeginSlot(CpuState *cpu, uint32_t entry);

typedef struct EffectBeginSlotClock {
    FixedSpanClock span;
    uint16_t product;
} EffectBeginSlotClock;

static uint8_t EffectBeginSlotRead(void *context, uint32_t address) {
    EffectBeginSlotClock *clock = context;
    FixedSpanClock *span = &clock->span;
    BeginFixedSpanOpcode(span);
    assert(span->index < span->count);
    if (span->timing[span->index].pc != 0x8ae5u)
        return FixedSpanRead(span, address);
    bool progress = false;
    uint8_t value;
    if (span->remaining == 2u) {
        assert(address == 0x4216u);
        clock->product = Lufia2DecompFrameReturnReadWord(
            span->cpu, address, address + 1u, &progress);
        value = (uint8_t)clock->product;
    } else {
        assert(span->remaining == 1u && address == 0x4217u);
        value = (uint8_t)(clock->product >> 8);
    }
    span->data_master += cpu_region_speed(address);
    span->progress |= progress;
    if (!--span->remaining)
        FinishFixedSpanOpcode(span);
    return value;
}

static void EffectBeginSlotWrite(void *context, uint32_t address, uint8_t value) {
    EffectBeginSlotClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
}

static unsigned EffectBeginSlotOpcodes(bool actor, bool motion) {
    return actor ? (motion ? 23u : 6u) : 4u;
}

static bool EffectBeginSlotHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeEffectBeginSlot &&
        (pc == 0x818a8cu || pc == 0x818acfu);
}

static unsigned EffectBeginSlotBudget(uint32_t entry, bool motion) {
    const unsigned fetch = cpu_region_speed(entry);
    if (entry == 0x818a8cu)
        return 9u * fetch + 108u;
    return motion ? 57u * fetch + 326u : 14u * fetch + 128u;
}

static bool EffectBeginSlotSupported(const CpuState *cpu, uint32_t entry,
                                    bool actor, bool motion) {
    if ((entry != 0x818a8cu && entry != 0x818acfu) ||
        cpu->PB != 0x81u || cpu->DB != 0x7eu || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || !cpu->ram ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || g_snes->ram != cpu->ram)
        return false;
    const unsigned base = actor ? 0x54b7u : 0x4f8fu;
    const unsigned stride = actor ? 0x2du : 0x2cu;
    if (cpu->Y < base || (cpu->Y - base) % stride ||
        (cpu->Y - base) / stride >= (actor ? 64u : 30u))
        return false;
    const unsigned budget = EffectBeginSlotBudget(entry, motion);
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
            fence - budget - 1u, budget))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    const uint32_t end = actor ? 0x818b08u : 0x818a95u;
    return phase < 1364u && clock == cpu->master_cycles &&
        Lufia2DecompFrameRangeAllows(entry, end, EffectBeginSlotHookOwned) &&
        Lufia2DecompFrameSpanBudget(entry,
            end, ActorBridgeEffectBeginSlot) >=
            EffectBeginSlotOpcodes(actor, motion);
}

static void ActorBridgeEffectBeginSlot(CpuState *cpu, uint32_t entry) {
    const bool actor = entry == 0x818acfu;
    const bool motion = actor && cpu->ram &&
        cpu->Y <= 0xffffu - 0x25u && cpu->ram[cpu->Y + 0x25u] < 0x80u;
    if (!EffectBeginSlotSupported(cpu, entry, actor, motion))
        return;
    static const FixedSpanOpcode script[] = {
        {0x8a8cu, 1u, 3u, 1u}, {0x8a8du, 3u, 6u, 2u},
        {0x8a90u, 2u, 4u, 2u}, {0x8a92u, 3u, 5u, 2u}
    };
    static const FixedSpanOpcode stationary[] = {
        {0x8acfu, 1u, 3u, 1u}, {0x8ad0u, 3u, 5u, 1u},
        {0x8ad3u, 2u, 3u, 0u}, {0x8b00u, 3u, 6u, 2u},
        {0x8b03u, 2u, 4u, 2u}, {0x8b05u, 3u, 5u, 2u}
    };
    static const FixedSpanOpcode moving[] = {
        {0x8acfu, 1u, 3u, 1u}, {0x8ad0u, 3u, 5u, 1u},
        {0x8ad3u, 2u, 2u, 0u}, {0x8ad5u, 4u, 5u, 1u},
        {0x8ad9u, 2u, 2u, 0u}, {0x8adbu, 4u, 5u, 1u},
        {0x8adfu, 2u, 3u, 0u}, {0x8ae1u, 3u, 3u, 0u},
        {0x8ae4u, 1u, 2u, 0u}, {0x8ae5u, 4u, 6u, 2u},
        {0x8ae9u, 1u, 2u, 0u}, {0x8aeau, 3u, 6u, 2u},
        {0x8aedu, 1u, 2u, 0u}, {0x8aeeu, 3u, 6u, 2u},
        {0x8af1u, 3u, 7u, 2u}, {0x8af4u, 3u, 6u, 2u},
        {0x8af7u, 1u, 2u, 0u}, {0x8af8u, 3u, 6u, 2u},
        {0x8afbu, 3u, 7u, 2u}, {0x8afeu, 2u, 3u, 0u},
        {0x8b00u, 3u, 6u, 2u}, {0x8b03u, 2u, 4u, 2u},
        {0x8b05u, 3u, 5u, 2u}
    };
    EffectBeginSlotClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.span.timing = !actor ? script : motion ? moving : stationary;
    clock.span.count = EffectBeginSlotOpcodes(actor, motion);
    const Lufia2Memory memory = {
        EffectBeginSlotRead, EffectBeginSlotWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleEffectBeginSlotStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    assert(result.pc == (actor ? 0x818b08u : 0x818a95u));
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
