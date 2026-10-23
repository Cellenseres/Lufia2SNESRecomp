#include "lufia2/battle_nmi_timer_steps.h"

enum { BATTLE_TIMER_HEADER_BUDGET = 288u, BATTLE_TIMER_ADVANCE_BUDGET = 160u };

static bool BattleTimerClockSupported(const CpuState *cpu, uint32_t entry) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    if ((entry != 0x858f1fu && entry != 0x858f3eu) ||
        cpu->PB != 0x85u || cpu->DB != 0x85u || cpu->D || cpu->emulation ||
        cpu->_flag_D || cpu->m_flag != 1u || cpu->x_flag != 1u ||
        cpu->X > 0x38u || (cpu->X & 7u) || cpu->Y > 0xffu ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || !cpu->ram ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffdu ||
        !Lufia2DecompNmiHasInterruptOwner() ||
        !Lufia2DecompFrameReturnCpuReady())
        return false;
    const uint64_t budget = entry == 0x858f1fu
        ? BATTLE_TIMER_HEADER_BUDGET : BATTLE_TIMER_ADVANCE_BUDGET;
    if (cpu->master_cycles > UINT64_MAX - budget ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || !g_snes->dma || g_snes->ram != cpu->ram ||
        !g_snes->inNmi || g_snes->inIrq || !g_snes->hdmaBeamOff ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        cpu->ram[cpu->S + 1u] != 0x78u || cpu->ram[cpu->S + 2u] != 0x8eu)
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
    return budget < before_edge;
#else
    (void)cpu;
    (void)entry;
    return false;
#endif
}

static void ActorBridgeBattleTimerStep(CpuState *cpu, uint32_t entry) {
    if (!BattleTimerClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[10];
    unsigned count = 0u;
    bool expired = false;
    if (entry == 0x858f1fu) {
        const bool active = cpu->ram[0x1b17u + cpu->X] != 0u;
        timing[count++] = (FixedSpanOpcode){0x8f1fu, 3u, 4u, 1u};
        timing[count++] = (FixedSpanOpcode){0x8f22u, 2u,
            (uint8_t)(active ? 2u : 3u), 0u};
        if (active) {
            expired = cpu->ram[0x1b18u + cpu->X] == 1u;
            timing[count++] = (FixedSpanOpcode){0x8f24u, 3u, 7u, 2u};
            timing[count++] = (FixedSpanOpcode){0x8f27u, 2u,
                (uint8_t)(expired ? 2u : 3u), 0u};
        }
    }
    if (!expired) {
        timing[count++] = (FixedSpanOpcode){0x8f3eu, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x8f3fu, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x8f40u, 2u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x8f42u, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x8f43u, 2u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x8f45u, 2u,
            (uint8_t)(cpu->X == 0x38u ? 2u : 3u), 0u};
    }
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x850000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleTimerStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.pc == 0x858f1fu || result.pc == 0x858f29u || result.pc == 0x858f47u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
