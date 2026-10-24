typedef struct MenuReplayPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} MenuReplayPhase;

static const FixedSpanOpcode menu_replay_ab23[] = {
    {0xab23u, 2u, 3u, 0u},
    {0xab25u, 3u, 3u, 0u},
    {0xab28u, 3u, 5u, 2u},
    {0xab2bu, 3u, 5u, 2u},
    {0xab2eu, 2u, 2u, 0u},
    {0xab30u, 2u, 3u, 1u},
    {0xab32u, 3u, 3u, 0u},
    {0xab35u, 2u, 4u, 2u},
    {0xab37u, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_ab3a[] = {
    {0xab3au, 3u, 5u, 2u},
    {0xab3du, 3u, 5u, 2u},
    {0xab40u, 3u, 5u, 2u},
    {0xab43u, 3u, 5u, 2u},
    {0xab46u, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_ab49[] = {
    {0xab49u, 2u, 2u, 0u},
    {0xab4bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_ab4e[] = {
    {0xab4eu, 1u, 3u, 1u},
    {0xab4fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_ab52[] = {
    {0xab52u, 3u, 4u, 1u},
    {0xab55u, 2u, 3u, 1u},
    {0xab57u, 1u, 4u, 1u},
    {0xab58u, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_replay_ab5c[] = {
    {0xab5cu, 2u, 3u, 1u},
    {0xab5eu, 2u, 3u, 0u},
};

static const FixedSpanOpcode menu_replay_ab5e[] = {
    {0xab5eu, 2u, 3u, 0u},
};

static const FixedSpanOpcode menu_replay_caf1[] = {
    {0xcaf1u, 1u, 3u, 1u},
    {0xcaf2u, 2u, 3u, 0u},
    {0xcaf4u, 3u, 3u, 0u},
    {0xcaf7u, 3u, 5u, 2u},
    {0xcafau, 3u, 3u, 0u},
    {0xcafdu, 3u, 5u, 2u},
    {0xcb00u, 2u, 2u, 0u},
    {0xcb02u, 2u, 3u, 1u},
    {0xcb04u, 2u, 2u, 0u},
    {0xcb06u, 2u, 3u, 1u},
    {0xcb08u, 2u, 5u, 2u},
};

static const FixedSpanOpcode menu_replay_cb08[] = {
    {0xcb08u, 2u, 5u, 2u},
};

static const FixedSpanOpcode menu_replay_cb0c[] = {
    {0xcb0cu, 2u, 2u, 0u},
    {0xcb0eu, 2u, 3u, 1u},
    {0xcb10u, 2u, 3u, 0u},
    {0xcb12u, 3u, 5u, 2u},
    {0xcb15u, 1u, 2u, 0u},
    {0xcb16u, 3u, 3u, 0u},
};

static const FixedSpanOpcode menu_replay_cb1b[] = {
    {0xcb1bu, 3u, 3u, 0u},
    {0xcb1eu, 3u, 5u, 2u},
    {0xcb21u, 2u, 3u, 0u},
    {0xcb23u, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_cb1e[] = {
    {0xcb1eu, 3u, 5u, 2u},
    {0xcb21u, 2u, 3u, 0u},
    {0xcb23u, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_cb23[] = {
    {0xcb23u, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_cb26[] = {
    {0xcb26u, 3u, 6u, 2u},
};

static const FixedSpanOpcode menu_replay_cb29[] = {
    {0xcb29u, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_replay_cb2d[] = {
    {0xcb2du, 3u, 3u, 0u},
    {0xcb30u, 3u, 5u, 2u},
    {0xcb33u, 2u, 3u, 1u},
};

static const FixedSpanOpcode menu_replay_cb37[] = {
    {0xcb37u, 2u, 2u, 0u},
    {0xcb39u, 2u, 3u, 0u},
    {0xcb5au, 2u, 3u, 1u},
    {0xcb5cu, 3u, 3u, 0u},
    {0xcb5fu, 3u, 5u, 2u},
    {0xcb62u, 1u, 4u, 1u},
};

static const FixedSpanOpcode menu_replay_cb3b[] = {
    {0xcb3bu, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_replay_cb3f[] = {
    {0xcb3fu, 1u, 3u, 0u},
    {0xcb40u, 2u, 2u, 0u},
};

static const FixedSpanOpcode menu_replay_cb44[] = {
    {0xcb44u, 3u, 3u, 0u},
    {0xcb47u, 2u, 3u, 1u},
    {0xcb49u, 1u, 2u, 0u},
    {0xcb4au, 2u, 2u, 0u},
    {0xcb4cu, 2u, 3u, 1u},
};

static const FixedSpanOpcode menu_replay_cb50[] = {
    {0xcb50u, 3u, 3u, 0u},
    {0xcb53u, 3u, 5u, 2u},
    {0xcb56u, 2u, 3u, 0u},
    {0xcb08u, 2u, 5u, 2u},
};

static const FixedSpanOpcode menu_replay_cb53[] = {
    {0xcb53u, 3u, 5u, 2u},
    {0xcb56u, 2u, 3u, 0u},
    {0xcb08u, 2u, 5u, 2u},
};

static const FixedSpanOpcode menu_replay_cb58[] = {
    {0xcb58u, 2u, 2u, 0u},
    {0xcb5au, 2u, 3u, 1u},
    {0xcb5cu, 3u, 3u, 0u},
    {0xcb5fu, 3u, 5u, 2u},
    {0xcb62u, 1u, 4u, 1u},
};

static const MenuReplayPhase menu_replay_phases[] = {
    {0xab23u, 0x0000u, 0x0000u, 0u, sizeof menu_replay_ab23 / sizeof menu_replay_ab23[0], menu_replay_ab23},
    {0xab3au, 0x0000u, 0x0000u, 1u, sizeof menu_replay_ab3a / sizeof menu_replay_ab3a[0], menu_replay_ab3a},
    {0xab49u, 0x0000u, 0x0000u, 1u, sizeof menu_replay_ab49 / sizeof menu_replay_ab49[0], menu_replay_ab49},
    {0xab4eu, 0x0000u, 0x0000u, 1u, sizeof menu_replay_ab4e / sizeof menu_replay_ab4e[0], menu_replay_ab4e},
    {0xab52u, 0xab5au, 0x0000u, 1u, sizeof menu_replay_ab52 / sizeof menu_replay_ab52[0], menu_replay_ab52},
    {0xab5cu, 0x0000u, 0xab60u, 1u, sizeof menu_replay_ab5c / sizeof menu_replay_ab5c[0], menu_replay_ab5c},
    {0xab5eu, 0x0000u, 0xab60u, 1u, sizeof menu_replay_ab5e / sizeof menu_replay_ab5e[0], menu_replay_ab5e},
    {0xcaf1u, 0xcb0au, 0x0000u, 0u, sizeof menu_replay_caf1 / sizeof menu_replay_caf1[0], menu_replay_caf1},
    {0xcb08u, 0xcb0au, 0x0000u, 1u, sizeof menu_replay_cb08 / sizeof menu_replay_cb08[0], menu_replay_cb08},
    {0xcb0cu, 0xcb19u, 0x0000u, 1u, sizeof menu_replay_cb0c / sizeof menu_replay_cb0c[0], menu_replay_cb0c},
    {0xcb1bu, 0x0000u, 0x0000u, 0u, sizeof menu_replay_cb1b / sizeof menu_replay_cb1b[0], menu_replay_cb1b},
    {0xcb1eu, 0x0000u, 0x0000u, 0u, sizeof menu_replay_cb1e / sizeof menu_replay_cb1e[0], menu_replay_cb1e},
    {0xcb23u, 0x0000u, 0x0000u, 1u, sizeof menu_replay_cb23 / sizeof menu_replay_cb23[0], menu_replay_cb23},
    {0xcb26u, 0x0000u, 0x0000u, 1u, sizeof menu_replay_cb26 / sizeof menu_replay_cb26[0], menu_replay_cb26},
    {0xcb29u, 0xcb2bu, 0x0000u, 1u, sizeof menu_replay_cb29 / sizeof menu_replay_cb29[0], menu_replay_cb29},
    {0xcb2du, 0xcb35u, 0x0000u, 1u, sizeof menu_replay_cb2d / sizeof menu_replay_cb2d[0], menu_replay_cb2d},
    {0xcb37u, 0x0000u, 0xcb63u, 1u, sizeof menu_replay_cb37 / sizeof menu_replay_cb37[0], menu_replay_cb37},
    {0xcb3bu, 0xcb3du, 0x0000u, 1u, sizeof menu_replay_cb3b / sizeof menu_replay_cb3b[0], menu_replay_cb3b},
    {0xcb3fu, 0xcb42u, 0x0000u, 1u, sizeof menu_replay_cb3f / sizeof menu_replay_cb3f[0], menu_replay_cb3f},
    {0xcb44u, 0xcb4eu, 0x0000u, 1u, sizeof menu_replay_cb44 / sizeof menu_replay_cb44[0], menu_replay_cb44},
    {0xcb50u, 0xcb0au, 0x0000u, 1u, sizeof menu_replay_cb50 / sizeof menu_replay_cb50[0], menu_replay_cb50},
    {0xcb53u, 0xcb0au, 0x0000u, 1u, sizeof menu_replay_cb53 / sizeof menu_replay_cb53[0], menu_replay_cb53},
    {0xcb58u, 0x0000u, 0xcb63u, 1u, sizeof menu_replay_cb58 / sizeof menu_replay_cb58[0], menu_replay_cb58},
};

static const MenuReplayPhase *FindMenuReplayPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof menu_replay_phases / sizeof menu_replay_phases[0]; ++i)
        if (menu_replay_phases[i].pc == pc)
            return menu_replay_phases + i;
    return NULL;
}

static uint16_t MenuReplayWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static void AppendMenuReplayPhase(CardScoreClock *clock, uint16_t pc,
                                  unsigned unused) {
    (void)unused;
    const MenuReplayPhase *phase = FindMenuReplayPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    assert(phase);
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        CardScoreOpcode(clock, op->pc, op->fetched, op->cycles, op->data_bytes);
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch)
        return;
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xab5au:
        taken = cpu->ram[(uint16_t)(cpu->S + 1u)] == 0u;
        target = taken ? 0xab5eu : 0xab5cu;
        break;
    case 0xcb0au:
        taken = pc == 0xcaf1u || !((uint8_t)(cpu->ram[2u] - 1u) & 0x80u);
        target = taken ? 0xcb23u : 0xcb0cu;
        break;
    case 0xcb19u:
        taken = (uint16_t)(MenuReplayWord(cpu, 0x2feu) + 1u) != 0x3508u;
        target = taken ? 0xcb1eu : 0xcb1bu;
        break;
    case 0xcb2bu:
        taken = (uint8_t)cpu->A != 1u;
        target = taken ? 0xcb3bu : 0xcb2du;
        break;
    case 0xcb35u:
        taken = cpu->ram[0u] != 0u;
        target = taken ? 0xcb58u : 0xcb37u;
        break;
    case 0xcb3du:
        taken = (uint8_t)cpu->A == 2u;
        target = taken ? 0xcb58u : 0xcb3fu;
        break;
    case 0xcb42u:
        taken = !(cpu->A >> 8 & 12u);
        target = taken ? 0xcb08u : 0xcb44u;
        break;
    case 0xcb4eu:
        taken = !((cpu->ram[0u] + 1u) & 1u);
        target = taken ? 0xcb53u : 0xcb50u;
        break;
    default:
        assert(false);
        return;
    }
    CardScoreOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    AppendMenuReplayPhase(clock, target, 0u);
}

static bool PrepareMenuReplay(CardScoreClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        (pc >> 16) != 0x84u || !CardScoreRegistersSupported(cpu))
        return false;
    const MenuReplayPhase *phase = FindMenuReplayPhase((uint16_t)pc);
    if (!phase || (pc != 0x84caf1u && cpu->m_flag != phase->byte_mode))
        return false;
    AppendMenuReplayPhase(clock, (uint16_t)pc, cpu->X);
    return CardScoreWindowSupported(clock);
}

static uint8_t MenuReplayChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareMenuReplay(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeMenuReplay(CpuState *cpu, uint32_t entry,
                                             ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareMenuReplay(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, MenuReplayChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
