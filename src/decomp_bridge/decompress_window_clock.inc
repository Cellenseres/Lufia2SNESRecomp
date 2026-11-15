#include "decompress_window_clock.h"

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
#if defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_WINDOW_CONTEXT) && \
    (defined(LUFIA2_HAVE_BEAM_HOLD_QUERY) || defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)) && \
    !defined(SNES_COSIM) && !defined(SNESRECOMP_INTERP_PROFILE) && \
    !(defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    if (cpu != &g_cpu || !Lufia2DecompDecompressWindowReady() ||
        budget >= 1364u || !g_snes->apu || !g_snes->dma ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos == 224u || g_snes->vPos == 261u ||
        snesrecomp_beam_hold_active())
        return false;
    const Cart *cart = g_snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return false;
    /* Every original opcode still publishes its clocks and device accesses. */
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu, true,
        1363u, budget);
#else
    return false;
#endif
}
