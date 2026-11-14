enum { SOUND_PAYLOAD_PAIR_MASTER_BUDGET = 576u };

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
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1fffu,
                                          SOUND_PAYLOAD_PAIR_MASTER_BUDGET))
        return false;
    const Cart *cart = g_snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom || cart->romSize < 0x1997u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        Lufia2DecompFrameSpanBudget(entry, entry + 1u,
            ActorBridgeSoundTransferOpcode) < 17u ||
        !Lufia2DecompFrameRangeAllows(entry, 0x809997u, ActorBridgeSoundHookOwned))
        return false;
    const FixedSpanOpcode timing[] = {
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
        {0x9995u, 2u, 2u, 0u}
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
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(17u, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(prefetched.pc);
    return true;
#endif
}
