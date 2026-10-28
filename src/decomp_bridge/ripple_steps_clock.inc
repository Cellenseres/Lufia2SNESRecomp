#include "lufia2/battle_ripple_steps.h"

static unsigned RippleStepBudget(uint32_t entry) {
    return entry == 0x85a736u ? 448u : 272u;
}

static bool RippleStepClockSupported(const CpuState *cpu, uint32_t entry) {
    if ((entry != 0x85a736u && entry != 0x85a74fu && entry != 0x85a760u) ||
        cpu->PB != 0x85u || cpu->D || cpu->_flag_D ||
        cpu->m_flag != 1u || cpu->x_flag || !cpu->ram ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        (entry != 0x85a736u && cpu->DB != 0x85u) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - RippleStepBudget(entry))
        return false;
    if (entry == 0x85a74fu &&
        (cpu->X >= 32u || cpu->Y >= 64u || !cpu->ram[0x33u] ||
         cpu->ram[0x33u] > 32u))
        return false;
    const unsigned budget = RippleStepBudget(entry);
    if (g_snes->inNmi) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
        const unsigned frame_offset = entry == 0x85a736u ? 1u : 4u;
        if (!Lufia2DecompNmiHasInterruptOwner() || cpu->DB != 0x85u ||
            cpu->emulation || cpu->S < 0x1f00u || cpu->S > 0x1ffau ||
            !g_snes->dma || g_snes->ram != cpu->ram || g_snes->inIrq ||
            g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
            g_snes->hPos >= 1364u ||
            g_snes->beamMasterLast != cpu->master_cycles ||
            g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
            cpu->ram[cpu->S + frame_offset] != 0x3bu ||
            cpu->ram[cpu->S + frame_offset + 1u] != 0x8fu)
            return false;
        for (unsigned channel = 0u; channel < 8u; ++channel)
            if (g_snes->dma->channel[channel].dmaActive)
                return false;
        uint64_t phase, clock;
        snes_refresh_state_get(&phase, &clock);
        if (phase >= 1364u || clock != cpu->master_cycles)
            return false;
        const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
        const uint64_t before_edge =
            (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
        return budget < before_edge && NmiSpanAvoidsHdma(g_snes, budget, true);
#else
        return false;
#endif
    }
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry,
            entry == 0x85a760u ? 0x1ffcu : 0x1fffu,
            entry == 0x85a736u, fence - budget - 1u, budget))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && clock == cpu->master_cycles;
}

static void ActorBridgeRippleStepClock(CpuState *cpu, uint32_t entry) {
    if (!RippleStepClockSupported(cpu, entry))
        return;
    static const FixedSpanOpcode prepare[] = {
        {0xa736u, 1u, 3u, 1u}, {0xa737u, 2u, 2u, 0u},
        {0xa739u, 1u, 3u, 1u}, {0xa73au, 1u, 4u, 1u},
        {0xa73bu, 1u, 2u, 0u}, {0xa73cu, 3u, 4u, 1u},
        {0xa73fu, 1u, 2u, 0u}, {0xa740u, 1u, 2u, 0u},
        {0xa741u, 3u, 4u, 1u}, {0xa744u, 2u, 2u, 0u},
        {0xa746u, 1u, 2u, 0u}, {0xa747u, 1u, 4u, 2u},
        {0xa748u, 3u, 3u, 0u}, {0xa74bu, 2u, 2u, 0u},
        {0xa74du, 2u, 3u, 1u}
    };
    FixedSpanOpcode row[] = {
        {0xa74fu, 3u, 4u, 1u}, {0xa752u, 1u, 2u, 0u},
        {0xa753u, 3u, 4u, 1u}, {0xa756u, 4u, 5u, 1u},
        {0xa75au, 1u, 2u, 0u}, {0xa75bu, 1u, 2u, 0u},
        {0xa75cu, 2u, 5u, 2u}, {0xa75eu, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode finish[] = {
        {0xa760u, 1u, 5u, 2u}, {0xa761u, 3u, 6u, 2u},
        {0xa764u, 3u, 6u, 2u}, {0xa767u, 2u, 2u, 0u},
        {0xa769u, 3u, 4u, 1u}, {0xa76cu, 1u, 4u, 1u}
    };
    row[7].cycles = cpu->ram[0x33u] == 1u ? 2u : 3u;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = entry == 0x85a736u ? prepare
        : entry == 0x85a74fu ? row : finish;
    clock.count = entry == 0x85a736u ? 15u : entry == 0x85a74fu ? 8u : 6u;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x85a736u
        ? Lufia2BattleRipplePrepareStep(&memory, &state)
        : entry == 0x85a74fu ? Lufia2BattleRippleRowStep(&memory, &state)
        : Lufia2BattleRippleFinishStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
