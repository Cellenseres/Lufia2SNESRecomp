static void ActorBridgeNmiScrollFront(CpuState *cpu, uint32_t entry);

static uint32_t NmiScrollFrontHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeNmiScrollFront)
        return 0u;
    switch (pc & 0x7fffffu) {
    case 0x0087a7u: return (pc & 0xff0000u) | 0x87b2u;
    case 0x0087c4u: case 0x0087cfu: case 0x0087dau: case 0x0087e5u:
        return pc + 4u;
    default: return 0u;
    }
}

static bool NmiScrollFrontHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return NmiScrollFrontHookEnd(pc, hook) != 0u;
}

static void ActorBridgeNmiScrollFront(CpuState *cpu, uint32_t entry) {
    const uint32_t end = NmiScrollFrontHookEnd(entry, ActorBridgeNmiScrollFront);
    const bool setup = entry == 0x8087a7u;
    if (!end || entry < 0x800000u || cpu != &g_cpu ||
        (!setup && (cpu->m_flag != 1u || cpu->x_flag)) ||
        !NmiClockWindow(cpu, cpu->x_flag != 0u, entry, end, 256u, 256u, true,
            NmiScrollFrontHookOwned))
        return;
    FixedSpanOpcode timing[LUFIA2_NMI_SCROLL_SETUP_OPCODES];
    unsigned count;
    if (setup) {
        static const FixedSpanOpcode preparation[] = {
            {0x87a7u, 1u, 3u, 1u}, {0x87a8u, 2u, 3u, 0u},
            {0x87aau, 2u, 3u, 0u}, {0x87acu, 3u, 3u, 0u},
            {0x87afu, 3u, 3u, 0u}
        };
        memcpy(timing, preparation, sizeof preparation);
        count = LUFIA2_NMI_SCROLL_SETUP_OPCODES;
    } else {
        const unsigned request = (entry - 0x8087c4u) / 11u;
        timing[0] = (FixedSpanOpcode){(uint16_t)entry, 2u, 3u, 1u};
        timing[1] = (FixedSpanOpcode){(uint16_t)(entry + 2u), 2u,
            cpu->ram[0x75u + request] ? 2u : 3u, 0u};
        count = LUFIA2_NMI_DMA_SELECT_OPCODES;
    }
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = count};
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = setup
        ? Lufia2NmiPrepareScrollWrites(&memory, &state)
        : Lufia2NmiSelectListedDma(&memory, &state, entry);
    NmiClockBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
