static void InstallDecompressHook(uint32_t pc, InterpPreOpcodeHook hook,
    bool (*owned)(uint32_t, InterpPreOpcodeHook)) {
    if (Lufia2DecompFrameRangeAllows(pc, pc + 1u, owned))
        interp_bridge_set_pre_opcode_hook(pc, hook);
}

static bool DecompressClockWindowSupported(const CpuState *cpu, uint32_t entry,
    unsigned budget, bool advance) {
    if (!g_snes || !budget)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        (!advance || g_snes->hPos <= 1024u) ? 1024u : 1364u;
    if (budget < fence && ActorBridgeTimedStepSupportedWindow(cpu, entry,
            0x1ffbu, true, fence - budget - 1u, budget))
        return true;
    return NativeOpcodeWindowSupported(cpu, entry, budget, 0x1ffbu, true);
}
