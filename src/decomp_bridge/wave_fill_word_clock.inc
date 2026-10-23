#include "lufia2/battle_wave_fill_steps.h"

enum { WAVE_FILL_WORD_BUDGET = 448u, WAVE_FILL_WORD_OPCODES = 15u };

static bool WaveFillWordClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x85aef5u || cpu->PB != 0x85u || cpu->DB != 0x85u ||
        cpu->D || cpu->emulation || cpu->_flag_D || cpu->m_flag != 1u ||
        cpu->x_flag || cpu->X >= 0x0160u || (cpu->X & 1u) ||
        cpu->Y >= 0x0040u || (cpu->Y & 1u) ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || !cpu->ram ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->master_cycles > UINT64_MAX - WAVE_FILL_WORD_BUDGET ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || !g_snes->dma || g_snes->ram != cpu->ram ||
        g_snes->beamMasterLast != cpu->master_cycles)
        return false;
    uint64_t phase, refresh_clock;
    snes_refresh_state_get(&phase, &refresh_clock);
    if (phase >= 1364u || refresh_clock != cpu->master_cycles)
        return false;
    if (!g_snes->inNmi) {
        const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
            ? 1024u : 1364u;
        return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu,
            false, fence - WAVE_FILL_WORD_BUDGET - 1u, WAVE_FILL_WORD_BUDGET);
    }
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    if (!Lufia2DecompNmiHasInterruptOwner() || g_snes->inIrq ||
        !g_snes->hdmaBeamOff || g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffau ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        cpu->ram[cpu->S + 4u] != 0x3bu || cpu->ram[cpu->S + 5u] != 0x8fu)
        return false;
    for (unsigned channel = 0; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
    const uint64_t before_edge =
        (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
    return WAVE_FILL_WORD_BUDGET < before_edge;
#else
    return false;
#endif
}

static void ActorBridgeWaveFillWord(CpuState *cpu, uint32_t entry) {
    if (!WaveFillWordClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[15] = {
        {0xaef5u, 2u, 3u, 0u}, {0xaef7u, 3u, 5u, 2u},
        {0xaefau, 1u, 2u, 0u}, {0xaefbu, 3u, 5u, 2u},
        {0xaefeu, 4u, 6u, 2u}, {0xaf02u, 2u, 3u, 0u},
        {0xaf04u, 1u, 2u, 0u}, {0xaf05u, 1u, 2u, 0u},
        {0xaf06u, 1u, 2u, 0u}, {0xaf07u, 1u, 2u, 0u},
        {0xaf08u, 3u, 3u, 0u},
        {0xaf0bu, 2u, (uint8_t)(cpu->Y == 0x003eu ? 2u : 3u), 0u}
    };
    unsigned count = 12u;
    if (cpu->Y == 0x003eu)
        timing[count++] = (FixedSpanOpcode){0xaf0du, 3u, 3u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaf10u, 3u, 3u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaf13u, 2u,
        (uint8_t)(cpu->X == 0x015eu ? 2u : 3u), 0u};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleWaveFillWordStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.pc == 0x85aef5u || result.pc == 0x85af15u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
