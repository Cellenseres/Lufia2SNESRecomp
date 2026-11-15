static bool ActorBridgeSoundPayloadProgress(CpuState *cpu, uint32_t entry) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    (void)cpu;
    (void)entry;
    return false;
#else
    const bool more = cpu->X >= 3u;
    const FixedSpanOpcode timing[] = {
        {0x999du, 2u, 5u, 2u}, {0x999fu, 2u, 5u, 2u},
        {0x99a1u, 3u, 3u, 0u}, {0x99a4u, 1u, 2u, 0u},
        {0x99a5u, 1u, 2u, 0u}, {0x99a6u, 2u, (uint8_t)(2u + more), 0u},
        {0x99a8u, 2u, 2u, 0u}, {0x99aau, 4u, 5u, 1u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = more ? 6u : 8u};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SoundUploadStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.pc == (more ? 0x809969u : 0x8099aeu) &&
           clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
    return true;
#endif
}
