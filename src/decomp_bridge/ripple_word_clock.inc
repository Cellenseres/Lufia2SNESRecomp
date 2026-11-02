#include "lufia2/battle_ripple_steps.h"
#include "nmi_owner.h"

static void ActorBridgeRippleWordClock(CpuState *cpu, uint32_t entry);

static bool RippleWordClockReady(CpuState *cpu, uint32_t entry) {
    if (entry != 0x85aa61u || cpu->PB != 0x85u || cpu->emulation ||
        cpu->m_flag || !cpu->x_flag || cpu->X >= 0xa8u || (cpu->X & 1u) ||
        !cpu->Y || cpu->Y > 12u || !cpu->ram || !g_snes ||
        g_snes->ram != cpu->ram || g_snes->vPos >= 262u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || !Lufia2DecompFrameReturnCpuReady())
        return false;
    const bool next_run = cpu->Y == 1u;
    const unsigned count = next_run ? 10u : 7u;
    const unsigned fetched = next_run ? 19u : 13u;
    const unsigned master = fetched * cpu_region_speed(entry) + 46u -
        (cpu->X == 0xa6u ? 6u : 0u);
    const unsigned budget = master + 40u;
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    if (g_snes->inNmi) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
        if (!Lufia2DecompNmiHasInterruptOwner() || cpu->DB != 0x85u ||
            cpu->D || cpu->_flag_D || cpu->S < 0x1f00u || cpu->S > 0x1ffbu ||
            !g_snes->dma || g_snes->inIrq || g_snes->hIrqEnabled ||
            g_snes->vIrqEnabled || g_snes->hPos >= 1364u ||
            g_snes->beamMasterLast != cpu->master_cycles ||
            g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
            cpu->ram[cpu->S + 3u] != 0x3bu || cpu->ram[cpu->S + 4u] != 0x8fu ||
            current != cpu->master_cycles)
            return false;
        for (unsigned channel = 0u; channel < 8u; ++channel)
            if (g_snes->dma->channel[channel].dmaActive)
                return false;
        const unsigned next_line = g_snes->vPos < 225u ? 225u : 262u;
        const uint64_t before_edge =
            (uint64_t)(next_line - g_snes->vPos) * 1364u - g_snes->hPos;
        return budget < before_edge && NmiSpanAvoidsHdma(g_snes, budget, true) &&
            Lufia2DecompFrameRangeQuiet(entry + 1u, 0x85aa74u);
#else
        return false;
#endif
    }
    if (Lufia2DecompFrameOwnedSpanBudget(entry, 0x85aa74u,
            ActorBridgeRippleWordClock) < count)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true,
        fence - budget - 1u, budget);
}

static void ActorBridgeRippleWordClock(CpuState *cpu, uint32_t entry) {
    if (!RippleWordClockReady(cpu, entry))
        return;
    FixedSpanOpcode timing[10] = {
        {0xaa61u, 4u, 6u, 2u}, {0xaa65u, 1u, 2u, 0u},
        {0xaa66u, 2u, cpu->Y == 1u ? 2u : 3u, 0u}
    };
    unsigned count = 3u;
    if (cpu->Y == 1u) {
        timing[count++] = (FixedSpanOpcode){0xaa68u, 2u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0xaa6au, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0xaa6bu, 3u, 3u, 0u};
    }
    timing[count++] = (FixedSpanOpcode){0xaa6eu, 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaa6fu, 1u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaa70u, 2u, 2u, 0u};
    timing[count++] = (FixedSpanOpcode){0xaa72u, 2u,
        cpu->X == 0xa6u ? 2u : 3u, 0u};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleRippleWordRowStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
