enum { WORLD_PALETTE_CLOCK_CAPACITY = 544u };

typedef struct WorldPaletteClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[WORLD_PALETTE_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t return_pc;
} WorldPaletteClock;

static void WorldPaletteOpcode(WorldPaletteClock *clock, unsigned pc,
                                unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < WORLD_PALETTE_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static void WorldPaletteMove(WorldPaletteClock *clock, unsigned pc, unsigned length) {
    for (unsigned byte = 0u; byte < length; ++byte)
        WorldPaletteOpcode(clock, pc, 3u, 7u, 2u);
}

static bool WorldPaletteWindow(const WorldPaletteClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    unsigned budget = clock->return_pc ? cpu_region_speed(clock->return_pc) + 34u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, clock->span.bank, 0x1ffcu,
                                             true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareWorldPalette(WorldPaletteClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    const uint32_t bank = pc & 0xff0000u;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = bank;
    clock->span.timing = clock->timing;
    clock->return_pc = 0u;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        cpu->PB != bank >> 16 || cpu->emulation || cpu->m_flag > 1u ||
        cpu->x_flag || cpu->D || cpu->_flag_D || cpu->S < 0x1f10u || cpu->S > 0x1ffcu ||
        cpu->host_return_valid != 2u ||
        (bank != 0x860000u || cpu->DB != 0x86u))
        return false;
    if (!cpu->m_flag || bank != 0x860000u)
        return false;
    switch (pc) {
    case 0x86b2f7u:
        WorldPaletteOpcode(clock, 0xb2f7u, 3u, 6u, 2u);
        break;
    case 0x86b2fau:
        WorldPaletteOpcode(clock, 0xb2fau, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb2fdu, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb2ffu, 3u, 6u, 2u);
        break;
    case 0x86b302u:
        WorldPaletteOpcode(clock, 0xb302u, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb304u, 2u, 3u, 1u);
        WorldPaletteOpcode(clock, 0xb306u, 3u, 6u, 2u);
        break;
    case 0x86b309u:
        WorldPaletteOpcode(clock, 0xb309u, 2u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb30bu, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb30eu, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb311u, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb314u, 1u, 3u, 1u);
        WorldPaletteMove(clock, 0xb315u, 224u);
        WorldPaletteOpcode(clock, 0xb318u, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb31bu, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb31eu, 3u, 3u, 0u);
        WorldPaletteMove(clock, 0xb321u, 256u);
        WorldPaletteOpcode(clock, 0xb324u, 1u, 4u, 1u);
        WorldPaletteOpcode(clock, 0xb325u, 2u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb327u, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb32au, 3u, 5u, 2u);
        WorldPaletteOpcode(clock, 0xb32du, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb32fu, 3u, 4u, 1u);
        WorldPaletteOpcode(clock, 0xb332u, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb334u, 3u, 4u, 1u);
        WorldPaletteOpcode(clock, 0xb337u, 3u, 3u, 0u);
        WorldPaletteOpcode(clock, 0xb33au, 3u, 5u, 2u);
        WorldPaletteOpcode(clock, 0xb33du, 3u, 6u, 2u);
        break;
    case 0x86b340u:
        WorldPaletteOpcode(clock, 0xb340u, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb342u, 3u, 4u, 1u);
        WorldPaletteOpcode(clock, 0xb345u, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb347u, 3u, 4u, 1u);
        WorldPaletteOpcode(clock, 0xb34au, 2u, 2u, 0u);
        WorldPaletteOpcode(clock, 0xb34cu, 3u, 6u, 2u);
        break;
    case 0x86b34fu:
        clock->return_pc = pc;
        break;
    default:
        return false;
    }
    return WorldPaletteWindow(clock);
}

static uint8_t WorldPaletteChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    WorldPaletteClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareWorldPalette(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeWorldPalette(CpuState *cpu, uint32_t entry,
                                            ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    WorldPaletteClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareWorldPalette(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, WorldPaletteChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.return_pc == result.pc && clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
