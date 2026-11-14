static bool ActorBridgeSoundHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
#if defined(LUFIA2_HAVE_NATIVE_SOUND_SOURCE_CLOCK)
    if (SoundSourceHookOwned(pc, hook))
        return true;
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_TRANSFER_OPCODES)
    if (hook == ActorBridgeSoundTransferOpcode)
        return pc >> 16 == 0x80u && ActorBridgeSoundOpcode((uint16_t)pc) != NULL;
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS)
    if (hook == ActorBridgeSoundUpload) {
        for (unsigned i = 0u; i < sizeof sound_upload_entries / sizeof *sound_upload_entries; ++i)
            if (pc == sound_upload_entries[i])
                return true;
    }
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLLS)
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLL_RUNS)
    if (hook == ActorBridgeSoundPollRun) {
#else
    if (hook == ActorBridgeSoundPoll) {
#endif
        for (unsigned i = 0u; i < sizeof sound_poll_entries / sizeof *sound_poll_entries; ++i)
            if (pc == sound_poll_entries[i])
                return true;
    }
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS)
    if (hook == ActorBridgeSoundBranch) {
        for (unsigned i = 0u; i < sizeof sound_branch_entries / sizeof *sound_branch_entries; ++i)
            if (pc == sound_branch_entries[i])
                return true;
    }
    if (hook == ActorBridgeBootSound) {
        for (unsigned i = 0u; i < sizeof boot_sound_entries / sizeof *boot_sound_entries; ++i)
            if (pc == boot_sound_entries[i])
                return true;
    }
#endif
    return false;
}

#if defined(LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS)
static bool ActorBridgeSoundGroupSupported(const CpuState *cpu, uint32_t entry) {
    if (cpu != &g_cpu || !Lufia2DecompFrameReturnCpuReady())
        return false;
    uint32_t finish;
    switch (entry) {
    case 0x809969u: finish = 0x80997du; break;
    case 0x80997du: case 0x809988u: case 0x80998bu:
        finish = 0x809997u; break;
    case 0x80999du: finish = 0x8099aeu; break;
    case 0x809c23u: finish = 0x809c36u; break;
    case 0x809c2bu: finish = 0x809c30u; break;
    case 0x809c35u: case 0x809c36u: case 0x809c3bu:
        finish = 0x809c40u; break;
    case 0x809c49u: finish = 0x809c69u; break;
    default: return false;
    }
#if defined(LUFIA2_HAVE_NATIVE_SOUND_TRANSFER_OPCODES)
    const InterpPreOpcodeHook owner = ActorBridgeSoundTransferOpcode;
#else
    InterpPreOpcodeHook owner = ActorBridgeSoundUpload;
#if defined(LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS)
    if (entry >= 0x809c00u)
        owner = ActorBridgeBootSound;
#endif
#endif
    /* Every supported group fits within 32 original instructions. */
    return Lufia2DecompFrameOwnedSpanBudget(entry, entry + 1u, owner) >= 32u &&
        Lufia2DecompFrameRangeAllows(entry, finish, ActorBridgeSoundHookOwned);
}
#endif

static void ActorBridgeInstallSoundHook(uint32_t pc, InterpPreOpcodeHook hook) {
    if (Lufia2DecompFrameRangeAllows(pc, pc + 1u, ActorBridgeSoundHookOwned))
        interp_bridge_set_pre_opcode_hook(pc, hook);
}
