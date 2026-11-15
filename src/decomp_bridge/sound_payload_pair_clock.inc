enum {
    SOUND_PAYLOAD_PAIR_MASTER_BUDGET = 576u,
    SOUND_PAYLOAD_HANDSHAKE_MASTER_BUDGET = 768u
};

static bool SoundPayloadWindowSupported(const CpuState *cpu, uint32_t entry,
                                        unsigned budget) {
    if (ActorBridgeSoundWindowSupported(cpu, entry, 0x1fffu, budget))
        return true;
#if defined(LUFIA2_HAVE_BEAM_HOLD_QUERY) || defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
    /* Per-opcode clocks still walk the beam. Only ordinary line boundaries
     * may occur here; HDMA, VBlank and field transitions keep the old route. */
    if (!g_snes || !g_snes->apu || !g_snes->dma || !g_snes->hdmaBeamOff ||
        g_snes->dma->hdmaPendingInit || g_snes->vPos >= 262u ||
        g_snes->vPos == 224u || g_snes->vPos == 261u ||
        !budget || budget >= SOUND_TRANSFER_SCANLINE_CLOCKS ||
        cpu->master_cycles > UINT64_MAX - budget || snesrecomp_beam_hold_active())
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= SOUND_TRANSFER_SCANLINE_CLOCKS || current != cpu->master_cycles)
        return false;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true,
        SOUND_TRANSFER_SCANLINE_CLOCKS - 1u, budget);
#else
    return false;
#endif
}

static bool ActorBridgeSoundPayloadPair(CpuState *cpu, uint32_t entry) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    (void)cpu;
    (void)entry;
    return false;
#else
    if (cpu != &g_cpu || entry != 0x809969u || cpu->PB != 0x80u ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        cpu->Y > 0xfffdu || !SoundSourceMemory(cpu->DB, cpu->Y) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - SOUND_PAYLOAD_PAIR_MASTER_BUDGET ||
        !SoundPayloadWindowSupported(cpu, entry, SOUND_PAYLOAD_PAIR_MASTER_BUDGET))
        return false;
    const unsigned opcode_budget = Lufia2DecompFrameSpanBudget(
        entry, entry + 1u, ActorBridgeSoundTransferOpcode);
    const Cart *cart = g_snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom || cart->romSize < 0x1997u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        opcode_budget < 17u ||
        !Lufia2DecompFrameRangeAllows(entry, 0x809997u, ActorBridgeSoundHookOwned))
        return false;
    bool handshake = false;
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLLS) && \
    defined(LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS)
    handshake = opcode_budget >= 27u && cart->romSize >= 0x19aeu &&
        cpu->master_cycles <= UINT64_MAX - SOUND_PAYLOAD_HANDSHAKE_MASTER_BUDGET &&
        SoundPayloadWindowSupported(cpu, entry, SOUND_PAYLOAD_HANDSHAKE_MASTER_BUDGET) &&
        Lufia2DecompFrameRangeAllows(0x809997u, 0x8099aeu, ActorBridgeSoundHookOwned);
#endif
    FixedSpanOpcode timing[] = {
        {0x9969u, 2u, 3u, 1u}, {0x996bu, 4u, 5u, 1u},
        {0x996fu, 2u, 3u, 1u}, {0x9971u, 4u, 5u, 1u},
        {0x9975u, 2u, 3u, 1u}, {0x9977u, 2u, 2u, 0u},
        {0x9979u, 4u, 5u, 1u},
        {0x997du, 3u, (uint8_t)(4u + (cpu->Y >= 0x100u)), 1u},
        {0x9980u, 2u, 3u, 1u}, {0x9982u, 1u, 2u, 0u},
        {0x9983u, 2u, 3u, 0u},
        {0x9988u, 3u, (uint8_t)(4u + (cpu->Y + 1u >= 0x100u)), 1u},
        {0x998bu, 2u, 3u, 1u}, {0x998du, 1u, 2u, 0u},
        {0x998eu, 2u, 3u, 0u}, {0x9993u, 2u, 3u, 1u},
        {0x9995u, 2u, 2u, 0u},
        {0x9997u, 4u, 5u, 1u}, {0x999bu, 2u, 2u, 0u},
        {0x999du, 2u, 5u, 2u}, {0x999fu, 2u, 5u, 2u},
        {0x99a1u, 3u, 3u, 0u}, {0x99a4u, 1u, 2u, 0u},
        {0x99a5u, 1u, 2u, 0u}, {0x99a6u, 2u, 2u, 0u},
        {0x99a8u, 2u, 2u, 0u}, {0x99aau, 4u, 5u, 1u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = 17u};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult sent = Lufia2SoundUploadStep(&memory, &state, entry);
    assert(sent.pc == 0x80997du);
    const Lufia2ExecutionResult prefetched = Lufia2SoundUploadStep(
        &memory, &state, sent.pc);
    BeginFixedSpanOpcode(&clock);
    assert(prefetched.pc == 0x809997u && clock.index == 17u && !clock.remaining);
    uint32_t next = prefetched.pc;
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLLS) && \
    defined(LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS)
    if (handshake) {
        clock.count = 18u;
        const Lufia2ExecutionResult compared = Lufia2SoundPollStep(
            &memory, &state, next);
        assert(compared.pc == 0x80999bu && clock.index == 18u);
        timing[18].cycles += !state.zero;
        clock.count = 19u;
        BeginFixedSpanOpcode(&clock);
        next = Lufia2SoundPollBranchTarget(compared.pc, state.zero, state.overflow);
        if (next == 0x80999du) {
            const bool more = state.x >= 3u;
            timing[24].cycles += more;
            clock.count = more ? 25u : 27u;
            next = Lufia2SoundUploadStep(&memory, &state, next).pc;
            BeginFixedSpanOpcode(&clock);
            assert(next == (more ? 0x809969u : 0x8099aeu));
        }
        assert(clock.index == clock.count && !clock.remaining);
    }
#else
    (void)handshake;
#endif
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(next);
    return true;
#endif
}
