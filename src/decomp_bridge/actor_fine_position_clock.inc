enum { ACTOR_FINE_CLOCK_BUDGET = 600u };

typedef struct ActorFineClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[22];
} ActorFineClock;

static uint8_t ActorFineRead(void *context, uint32_t address) {
    ActorFineClock *clock = context;
    const bool actor_slot = clock->span.index == 0u;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (actor_slot) {
        assert(address == 0xa7u);
        clock->timing[2].cycles = (uint8_t)(4u + (value >= 0x46u));
        clock->timing[11].cycles = (uint8_t)(4u + (value >= 0x1eu));
    }
    return value;
}

static void ActorFineWrite(void *context, uint32_t address, uint8_t value) {
    ActorFineClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
}

static bool ActorFineClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x83a746u || cpu->m_flag != 1u || cpu->x_flag != 1u ||
        cpu->DB != 0x83u || cpu->X > 0xffu || cpu->Y > 0xffu || cpu->D || cpu->_flag_D ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 3u) ||
        cpu->S < 0x1f10u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - ACTOR_FINE_CLOCK_BUDGET)
        return false;
    if (!g_snes)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false,
            fence - ACTOR_FINE_CLOCK_BUDGET - 1u, ACTOR_FINE_CLOCK_BUDGET))
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    return refresh_phase < 1364u &&
        (!refresh_clock || refresh_clock == cpu->master_cycles);
}

static void ActorBridgeActorFinePosition(CpuState *cpu, uint32_t entry) {
    if (!ActorFineClockSupported(cpu, entry))
        return;
    ActorFineClock clock = {0};
    static const FixedSpanOpcode timing[] = {
        {0xa746u, 2u, 3u, 1u}, {0xa748u, 1u, 2u, 0u},
        {0xa749u, 3u, 4u, 1u}, {0xa74cu, 2u, 3u, 0u},
        {0xa74eu, 1u, 2u, 0u}, {0xa74fu, 1u, 2u, 0u},
        {0xa750u, 1u, 2u, 0u}, {0xa751u, 1u, 2u, 0u},
        {0xa752u, 1u, 4u, 2u}, {0xa753u, 2u, 3u, 0u},
        {0xa755u, 1u, 2u, 0u}, {0xa756u, 3u, 4u, 1u},
        {0xa759u, 2u, 3u, 0u}, {0xa75bu, 1u, 2u, 0u},
        {0xa75cu, 1u, 2u, 0u}, {0xa75du, 1u, 2u, 0u},
        {0xa75eu, 1u, 2u, 0u}, {0xa75fu, 2u, 3u, 1u},
        {0xa761u, 4u, 6u, 2u}, {0xa765u, 1u, 5u, 2u},
        {0xa766u, 4u, 6u, 2u}, {0xa76au, 2u, 3u, 0u}
    };
    for (unsigned i = 0u; i < 22u; ++i)
        clock.timing[i] = timing[i];
    clock.span.cpu = cpu;
    clock.span.bank = 0x830000u;
    clock.span.timing = clock.timing;
    clock.span.count = 22u;
    const Lufia2Memory memory = {ActorFineRead, ActorFineWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ActorSyncFinePosition(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == 22u && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(22u, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(0x83a76cu);
}
