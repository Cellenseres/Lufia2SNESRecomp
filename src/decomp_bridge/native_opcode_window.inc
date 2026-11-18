#include "native_opcode_window.h"

#if defined(LUFIA2_HAVE_NATIVE_SAVE_ACTION_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_FIELD_SCENE_OBJECT_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_SCENE_SLOT_RESET_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_GAUGE_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_DIVIDE_BIT_SPANS) || \
    defined(LUFIA2_HAVE_NATIVE_VRAM_QUEUE_PHASES) || \
    defined(LUFIA2_HAVE_NATIVE_BACKGROUND_WORD_SPANS) || \
    defined(LUFIA2_HAVE_NATIVE_TURN_DISPLAY_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_FILL_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_BACKGROUND_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_COMMAND_LOOP) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_DISPLAY_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_TILEMAP_SPANS) || \
    defined(LUFIA2_HAVE_NATIVE_ACTION_COPY_SPANS) || \
    defined(LUFIA2_HAVE_NATIVE_EFFECT_CLEAR_WINDOW) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_RESULT_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_MARKER_WINDOW) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_INPUT_WINDOW) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_INPUT) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_SELECTION) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CURSOR) || \
    defined(LUFIA2_HAVE_NATIVE_PLAYER_CONTINUATIONS) || \
    defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_REFERENCE_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_DISPATCH_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_FRAME_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_ACTION_CLEAR_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_NAME_STEPS) || \
    defined(LUFIA2_HAVE_NATIVE_FRAME_DECISIONS)
static bool NativeOpcodeWindowSupported(const CpuState *cpu, uint32_t entry,
    unsigned budget, unsigned stack_limit, bool wide_accumulator) {
    if (!g_snes || !budget)
        return false;
#if defined(LUFIA2_HAVE_NATIVE_OPCODE_WINDOW_CONTEXT) && \
    (defined(LUFIA2_HAVE_BEAM_HOLD_QUERY) || defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)) && \
    !defined(SNES_COSIM) && !defined(SNESRECOMP_INTERP_PROFILE) && \
    !(defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    if (cpu != &g_cpu || !Lufia2DecompOpcodeWindowReady() ||
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
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, stack_limit,
        wide_accumulator, 1363u, budget);
#else
    (void)cpu;
    (void)entry;
    (void)stack_limit;
    (void)wide_accumulator;
    return false;
#endif
}
#endif
