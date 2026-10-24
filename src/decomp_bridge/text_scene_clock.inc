enum { TEXT_SCENE_CLOCK_CAPACITY = 48u };

typedef struct TextSceneClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TEXT_SCENE_CLOCK_CAPACITY];
    ActorPushedCall call;
    bool complete;
} TextSceneClock;

static void TextSceneOpcode(TextSceneClock *clock, unsigned pc,
                             unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < TEXT_SCENE_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool TextSceneDataByte(const CpuState *cpu, uint16_t address, uint8_t *value) {
    if (ActorBridgeSystemBank(cpu->DB) || cpu->DB == 0x7eu || cpu->DB == 0x7fu) {
        const unsigned offset = cpu->DB == 0x7fu ? 0x10000u : 0u;
        *value = cpu->ram[offset + address];
        return true;
    }
    if (cpu->DB < 0xc0u || cpu->DB >= 0xd0u)
        return false;
    const unsigned offset = (cpu->DB & 0x7fu) * 0x8000u + (address & 0x7fffu);
    if (offset >= g_snes->cart->romSize)
        return false;
    *value = g_snes->cart->rom[offset];
    return true;
}

static bool TextSceneWindow(const TextSceneClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    unsigned budget = 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x800000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x800000u, 0x1ffcu, true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareTextSceneClock(TextSceneClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->complete = false;
    if (!cpu->ram || !g_snes || !g_snes->cart ||
        g_snes->cart->type != CART_LOROM || !g_snes->cart->rom ||
        cpu->PB != 0x80u || cpu->emulation || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->D || cpu->_flag_D || cpu->S < 0x1f10u || cpu->S > 0x1ffcu ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u))
        return false;
    uint8_t reload;
    if (!TextSceneDataByte(cpu, 0x05b3u, &reload))
        return false;
    switch (pc) {
    case 0x80bc3du:
        TextSceneOpcode(clock, 0xbc3du, 3u, 4u, 1u);
        TextSceneOpcode(clock, 0xbc40u, 2u, 2u, 0u);
        TextSceneOpcode(clock, 0xbc42u, 2u, reload & 0x10u ? 3u : 2u, 0u);
        TextSceneOpcode(clock, reload & 0x10u ? 0xbc4cu : 0xbc44u, 3u, 6u, 2u);
        break;
    case 0x80bc47u:
        TextSceneOpcode(clock, 0xbc47u, 2u, 5u, 2u);
        TextSceneOpcode(clock, 0xbc49u, 3u, 4u, 0u);
        clock->complete = true;
        break;
    case 0x80bc4fu:
        TextSceneOpcode(clock, 0xbc4fu, 3u, 6u, 2u);
        break;
    case 0x80bc52u:
        TextSceneOpcode(clock, 0xbc52u, 3u, 4u, 0u);
        clock->complete = true;
        break;
    case 0x80bcbcu:
        TextSceneOpcode(clock, 0xbcbcu, 3u, 6u, 2u);
        break;
    case 0x80bcbfu:
        TextSceneOpcode(clock, 0xbcbfu, 3u, 4u, 1u);
        TextSceneOpcode(clock, 0xbcc2u, 2u, 2u, 0u);
        TextSceneOpcode(clock, 0xbcc4u, 2u, cpu->A & 2u ? 2u : 3u, 0u);
        if (!(cpu->A & 2u)) {
            TextSceneOpcode(clock, 0xbce1u, 3u, 4u, 0u);
            clock->complete = true;
            break;
        }
        TextSceneOpcode(clock, 0xbcc6u, 2u, 3u, 0u);
        TextSceneOpcode(clock, 0xbcc8u, 1u, 4u, 2u);
        TextSceneOpcode(clock, 0xbcc9u, 1u, 3u, 1u);
        TextSceneOpcode(clock, 0xbccau, 3u, 3u, 0u);
        TextSceneOpcode(clock, 0xbccdu, 3u, 3u, 0u);
        TextSceneOpcode(clock, 0xbcd0u, 3u, 3u, 0u);
        for (unsigned byte = 0u; byte < 32u; ++byte)
            TextSceneOpcode(clock, 0xbcd3u, 3u, 7u, 2u);
        TextSceneOpcode(clock, 0xbcd6u, 2u, 3u, 0u);
        TextSceneOpcode(clock, 0xbcd8u, 2u, 2u, 0u);
        TextSceneOpcode(clock, 0xbcdau, 2u, 3u, 1u);
        TextSceneOpcode(clock, 0xbcdcu, 3u, 6u, 2u);
        break;
    case 0x80bcdfu:
        TextSceneOpcode(clock, 0xbcdfu, 1u, 4u, 1u);
        TextSceneOpcode(clock, 0xbce0u, 1u, 5u, 2u);
        TextSceneOpcode(clock, 0xbce1u, 3u, 4u, 0u);
        clock->complete = true;
        break;
    default:
        return false;
    }
    return TextSceneWindow(clock);
}

static uint8_t TextSceneChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TextSceneClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTextSceneClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeTimedTextScene(CpuState *cpu, uint32_t entry,
                                              ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TextSceneClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTextSceneClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, TextSceneChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        BeginFixedSpanOpcode(&clock.span);
        assert(result.pc == 0x809d00u && clock.complete &&
            clock.span.index == clock.span.count && !clock.span.remaining);
    }
    const uint32_t source = clock.complete
        ? 0x800000u | clock.timing[clock.span.count - 1u].pc : result.pc;
    return interp_tier_dispatch_tail(cpu, result.pc, source, frame.entry_s, frame.hrv);
}
