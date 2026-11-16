#include "lufia2/battle_timed_callbacks.h"

static void ActorBridgeBattleNmiCallback(CpuState *cpu, uint32_t entry);

static uint32_t BattleNmiCallbackHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeBattleNmiCallback)
        return 0u;
    switch (pc) {
    case 0x858e79u: case 0x858e7cu: case 0x858e8eu: return 0x858e97u;
    default: return 0u;
    }
}

static bool BattleNmiCallbackHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return BattleNmiCallbackHookEnd(pc, hook) != 0u;
}

static bool BattleNmiCallbackWindow(const CpuState *cpu) {
#if defined(LUFIA2_HAVE_BEAM_HOLD_QUERY) || defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
    if (cpu != &g_cpu || !Lufia2DecompFrameReturnCpuReady() ||
        !Lufia2DecompNmiHasInterruptOwner() || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x85u || cpu->DB != 0x85u || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->S < 0x1f02u || cpu->S > 0x1ff3u ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        cpu->master_cycles != cpu->coprocessor_master_cycles ||
        cpu->master_cycles > UINT64_MAX - 512u || !g_snes || !g_snes->dma ||
        !g_snes->apu || !cpu->ram || cpu->ram != g_snes->ram ||
        !g_snes->inNmi || g_snes->inIrq || g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        g_snes->beamMasterLast != cpu->master_cycles || g_snes->dma->dmaBusy ||
        g_snes->dma->dmaTimer || g_snes->dma->hdmaPendingInit || snesrecomp_beam_hold_active())
        return false;
    const Cart *cart = g_snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom || cart->romSize <= 0x28e97u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return false;
    for (unsigned i = 0u; i < 8u; ++i)
        if (g_snes->dma->channel[i].dmaActive)
            return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles || deadline - cpu->master_cycles < 512u))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    const unsigned edge = g_snes->vPos < 225u ? 225u : 262u;
    return phase < 1364u && (!current || current == cpu->master_cycles) &&
        512u < (uint64_t)(edge - g_snes->vPos) * 1364u - g_snes->hPos &&
        NmiSpanAvoidsHdma(g_snes, 512u, true) &&
        Lufia2DecompFrameRangeAllows(0x858e79u, 0x858e97u, BattleNmiCallbackHookOwned);
#else
    (void)cpu;
    return false;
#endif
}

static void ActorBridgeBattleNmiCallback(CpuState *cpu, uint32_t entry) {
    if (!BattleNmiCallbackHookEnd(entry, ActorBridgeBattleNmiCallback) ||
        !BattleNmiCallbackWindow(cpu))
        return;
    FixedSpanOpcode timing[LUFIA2_NMI_BATTLE_CALLBACK_OPCODES];
    unsigned count = 0u;
#define CALLBACK_OPCODE(pc, fetched, cycles, data) \
    (timing[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x858e8eu) {
        const unsigned cursor = cpu->ram[cpu->S + 1u] |
            (unsigned)cpu->ram[cpu->S + 2u] << 8;
        if (cursor > 14u || (cursor & 1u))
            return;
        CALLBACK_OPCODE(0x8e8eu, 1u, 5u, 2u);
        CALLBACK_OPCODE(0x8e8fu, 1u, 2u, 0u);
        CALLBACK_OPCODE(0x8e90u, 1u, 2u, 0u);
        CALLBACK_OPCODE(0x8e91u, 3u, 3u, 0u);
        CALLBACK_OPCODE(0x8e94u, 2u, cursor == 14u ? 2u : 3u, 0u);
        if (cursor == 14u)
            CALLBACK_OPCODE(0x8e96u, 1u, 4u, 1u);
    } else {
        const unsigned cursor = entry == 0x858e79u ? 0u : cpu->Y;
        if (cursor > 14u || (cursor & 1u))
            return;
        if (entry == 0x858e79u)
            CALLBACK_OPCODE(0x8e79u, 3u, 3u, 0u);
        CALLBACK_OPCODE(0x8e7cu, 3u, 4u, 1u);
        const bool finished = (cpu->ram[0x12e3u + cursor] & 0x80u) != 0u;
        CALLBACK_OPCODE(0x8e7fu, 2u, finished ? 3u : 2u, 0u);
        if (finished) {
            CALLBACK_OPCODE(0x8e96u, 1u, 4u, 1u);
        } else {
            CALLBACK_OPCODE(0x8e81u, 2u, 3u, 0u);
            CALLBACK_OPCODE(0x8e83u, 1u, 2u, 0u);
            CALLBACK_OPCODE(0x8e84u, 3u, 3u, 0u);
            CALLBACK_OPCODE(0x8e87u, 1u, 2u, 0u);
            CALLBACK_OPCODE(0x8e88u, 2u, 3u, 0u);
            CALLBACK_OPCODE(0x8e8au, 1u, 4u, 2u);
        }
    }
#undef CALLBACK_OPCODE
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleNmiCallbackStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
