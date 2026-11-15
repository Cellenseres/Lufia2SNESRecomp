#include "actor_record_offsets_clock.h"

enum { ACTOR_RECORD_OFFSETS_CLOCK_RESERVE = 320u };

static void ActorBridgeActorRecordOffsets(CpuState *cpu, uint32_t entry);

bool Lufia2DecompActorRecordOffsetsHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x83ab4fu && hook == ActorBridgeActorRecordOffsets;
}

static bool ActorRecordOffsetsHooksAllow(void) {
    return Lufia2DecompFrameRangeAllows(0x83ab4fu, 0x83ab61u,
        Lufia2DecompActorRecordOffsetsHookOwned);
}

/* Paired calls and the original RTL keep their existing retirement owners. */
static void ActorBridgeActorRecordOffsets(CpuState *cpu, uint32_t entry) {
    if (cpu != &g_cpu || entry != 0x83ab4fu || cpu->m_flag != 1u ||
        cpu->D || cpu->_flag_D || cpu->S < 0x1f04u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        Lufia2DecompFramePairedCallReady(cpu, entry) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - ACTOR_RECORD_OFFSETS_CLOCK_RESERVE ||
        !g_snes || !g_snes->cart)
        return;
    const Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < 0x1ab61u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (g_snes->vPos >= 262u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false,
            fence - ACTOR_RECORD_OFFSETS_CLOCK_RESERVE - 1u,
            ACTOR_RECORD_OFFSETS_CLOCK_RESERVE))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles ||
        !Lufia2DecompFrameRangeAllows(entry, 0x83ab60u,
            Lufia2DecompActorRecordOffsetsHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, 0x83ab60u,
            ActorBridgeActorRecordOffsets) < 9u)
        return;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u,
        .timing = actor_record_offset_timing, .count = 9u};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2ActorSetRecordOffsets(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x83ab60u);
    (void)result;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == 9u && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(9u, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(0x83ab60u);
}
