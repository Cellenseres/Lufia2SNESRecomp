static void ActorBridgeNmiScrollPair(CpuState *cpu, uint32_t entry);

static bool NmiScrollPairHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x8087b2u && hook == ActorBridgeNmiScrollPair;
}

static void PrepareNmiScrollPairClock(const CpuState *cpu, FixedSpanOpcode *timing) {
    static const FixedSpanOpcode pair[] = {
        {0x87b2u, 3u, 4u, 1u}, {0x87b5u, 2u, 4u, 1u},
        {0x87b7u, 3u, 4u, 1u}, {0x87bau, 2u, 4u, 1u},
        {0x87bcu, 1u, 2u, 0u}, {0x87bdu, 1u, 2u, 0u},
        {0x87beu, 1u, 2u, 0u}, {0x87bfu, 3u, 3u, 0u},
        {0x87c2u, 2u, 3u, 0u}
    };
    memcpy(timing, pair, sizeof pair);
    if (cpu->Y == 14u)
        --timing[8].cycles;
}

static bool NmiScrollPairClockSupported(const CpuState *cpu) {
    return cpu == &g_cpu && g_snes && g_snes->apu && g_snes->vPos < 262u &&
        cpu->m_flag == 1u &&
        cpu->X >= 0x210du && cpu->X <= 0x2114u &&
        cpu->Y == 2u * (cpu->X - 0x210du) &&
        NmiClockWindow(cpu, false, 0x8087b2u, 0x8087c4u, 320u, 320u,
            NmiScrollPairHookOwned);
}

static void ActorBridgeNmiScrollPair(CpuState *cpu, uint32_t entry) {
    if (entry != 0x8087b2u || !NmiScrollPairClockSupported(cpu))
        return;
    FixedSpanOpcode timing[LUFIA2_NMI_SCROLL_PAIR_OPCODES];
    PrepareNmiScrollPairClock(cpu, timing);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u, .timing = timing,
        .count = LUFIA2_NMI_SCROLL_PAIR_OPCODES};
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2NmiWriteScrollPair(&memory, &state);
    NmiClockBeginOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == 0x8087b2u || result.pc == 0x8087c4u));
    assert(cpu->master_cycles - start <= 320u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
