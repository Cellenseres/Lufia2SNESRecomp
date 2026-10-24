enum { PALETTE_ENTRY_CLOCK_CAPACITY = 544u };

typedef struct PaletteEntryClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[PALETTE_ENTRY_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t return_pc;
} PaletteEntryClock;

static void PaletteEntryOpcode(PaletteEntryClock *clock, unsigned pc,
                                unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < PALETTE_ENTRY_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static void PaletteEntryMove(PaletteEntryClock *clock, unsigned pc, unsigned length) {
    for (unsigned byte = 0u; byte < length; ++byte)
        PaletteEntryOpcode(clock, pc, 3u, 7u, 2u);
}

static bool PaletteEntryWindow(const PaletteEntryClock *clock) {
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

static bool PreparePaletteEntry(PaletteEntryClock *clock, uint32_t pc) {
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
        (cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        (bank != 0x840000u || (cpu->DB != 0u && cpu->DB != 0x80u && cpu->DB != 0x7eu)))
        return false;
    const bool word = pc == 0x84a569u || pc == 0x84a565u || pc == 0x84b5b9u ||
        pc == 0x84b5cbu || pc == 0x84b5ceu || pc == 0x84b5d1u ||
        pc == 0x84b5d4u || pc == 0x84b5d7u;
    if (cpu->m_flag == word)
        return false;
    switch (pc) {
    case 0x84a529u:
        PaletteEntryOpcode(clock, 0xa529u, 1u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa52au, 1u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa52bu, 1u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xa52cu, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa52eu, 1u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xa52fu, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa532u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa535u, 3u, 3u, 0u);
        PaletteEntryMove(clock, 0xa538u, 256u);
        PaletteEntryOpcode(clock, 0xa53bu, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa53eu, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa541u, 3u, 3u, 0u);
        PaletteEntryMove(clock, 0xa544u, 256u);
        PaletteEntryOpcode(clock, 0xa547u, 1u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa548u, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa54au, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xa54cu, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xa54eu, 3u, 6u, 2u);
        break;
    case 0x84a551u:
        PaletteEntryOpcode(clock, 0xa551u, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa553u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa556u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa558u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa55bu, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa55du, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa560u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa562u, 3u, 6u, 2u);
        break;
    case 0x84a565u: {
        const bool narrow = (cpu->ram[cpu->S + 1u] & 0x10u) != 0u;
        PaletteEntryOpcode(clock, 0xa565u, 1u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa566u, 1u, narrow ? 4u : 5u, narrow ? 1u : 2u);
        PaletteEntryOpcode(clock, 0xa567u, 1u, narrow ? 4u : 5u, narrow ? 1u : 2u);
        clock->return_pc = 0x84a568u;
        break;
    }
    case 0x84b57fu:
        PaletteEntryOpcode(clock, 0xb57fu, 1u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb580u, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb582u, 1u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb583u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb586u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb589u, 3u, 3u, 0u);
        PaletteEntryMove(clock, 0xb58cu, 256u);
        PaletteEntryOpcode(clock, 0xb58fu, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb592u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb595u, 3u, 3u, 0u);
        PaletteEntryMove(clock, 0xb598u, 256u);
        PaletteEntryOpcode(clock, 0xb59bu, 1u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xb59cu, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb59eu, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xb5a0u, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb5a2u, 3u, 6u, 2u);
        break;
    case 0x84b5a5u:
        PaletteEntryOpcode(clock, 0xb5a5u, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5a7u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5aau, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xb5acu, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5afu, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xb5b1u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5b4u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xb5b6u, 3u, 6u, 2u);
        break;
    case 0x84b5b9u:
        PaletteEntryOpcode(clock, 0xb5b9u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5bcu, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xb5beu, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5c1u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xb5c3u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5c6u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xb5c8u, 3u, 6u, 2u);
        break;
    case 0x84b5cbu: case 0x84b5ceu: case 0x84b5d1u: case 0x84b5d4u:
    case 0x84b5f5u:
        PaletteEntryOpcode(clock, (uint16_t)pc, 3u, 6u, 2u);
        break;
    case 0x84b5d7u:
        PaletteEntryOpcode(clock, 0xb5d7u, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5d9u, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xb5dbu, 3u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xb5deu, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xb5e0u, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb5e2u, 3u, 6u, 2u);
        break;
    case 0x84b5e5u:
        PaletteEntryOpcode(clock, 0xb5e5u, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xb5e7u, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb5e9u, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb5ebu, 3u, 6u, 2u);
        break;
    case 0x84b5eeu:
        PaletteEntryOpcode(clock, 0xb5eeu, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xb5f0u, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xb5f2u, 3u, 6u, 2u);
        break;
    case 0x84b5f8u:
        PaletteEntryOpcode(clock, 0xb5f8u, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5fau, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xb5fdu, 4u, 6u, 2u);
        PaletteEntryOpcode(clock, 0xb601u, 1u, 4u, 1u);
        clock->return_pc = 0x84b602u;
        break;
    case 0x84a569u:
        PaletteEntryOpcode(clock, 0xa569u, 1u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa56au, 1u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa56bu, 1u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xa56cu, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa56eu, 2u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa570u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa573u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa575u, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xa577u, 2u, 3u, 1u);
        PaletteEntryOpcode(clock, 0xa579u, 4u, 8u, 3u);
        break;
    case 0x84a57du:
        PaletteEntryOpcode(clock, 0xa57du, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xa57fu, 3u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa582u, 3u, 3u, 0u);
        PaletteEntryOpcode(clock, 0xa585u, 3u, 5u, 2u);
        PaletteEntryOpcode(clock, 0xa588u, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xa58au, 3u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa58du, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa58fu, 3u, 5u, 2u);
        PaletteEntryOpcode(clock, 0xa592u, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xa594u, 3u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa597u, 2u, 4u, 2u);
        PaletteEntryOpcode(clock, 0xa599u, 3u, 5u, 2u);
        PaletteEntryOpcode(clock, 0xa59cu, 2u, 2u, 0u);
        PaletteEntryOpcode(clock, 0xa59eu, 3u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa5a1u, 3u, 6u, 2u);
        break;
    case 0x84a5a4u:
        PaletteEntryOpcode(clock, 0xa5a4u, 1u, 4u, 1u);
        PaletteEntryOpcode(clock, 0xa5a5u, 1u, 5u, 2u);
        PaletteEntryOpcode(clock, 0xa5a6u, 1u, 5u, 2u);
        clock->return_pc = 0x84a5a7u;
        break;
    default:
        return false;
    }
    return PaletteEntryWindow(clock);
}

static uint8_t PaletteEntryChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    PaletteEntryClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PreparePaletteEntry(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing,
        clock->call.frame.entry_s, clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgePaletteEntry(CpuState *cpu, uint32_t entry,
                                            ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    PaletteEntryClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PreparePaletteEntry(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, PaletteEntryChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.return_pc == result.pc && clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
