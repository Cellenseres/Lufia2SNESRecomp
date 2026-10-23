#include "lufia2/battle_transition_steps.h"

enum { SWIRL_RIM_CLOCK_BUDGET = 532u };

static bool SwirlRimClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x848c91u || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D ||
        cpu->X > 0xffu || cpu->Y >= 0x40u || (cpu->Y & 1u) ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        cpu->S < 0x1f10u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - SWIRL_RIM_CLOCK_BUDGET || !g_snes)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false,
            fence - SWIRL_RIM_CLOCK_BUDGET - 1u, SWIRL_RIM_CLOCK_BUDGET))
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    return refresh_phase < 1364u &&
        (!refresh_clock || refresh_clock == cpu->master_cycles);
}

static void ActorBridgeSwirlRimStep(CpuState *cpu, uint32_t entry) {
    if (!SwirlRimClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[] = {
        {0x8c91u, 4u, 5u, 1u}, {0x8c95u, 4u, 5u, 1u},
        {0x8c99u, 2u, 3u, 0u}, {0x8c9bu, 4u, 6u, 2u},
        {0x8c9fu, 1u, 2u, 0u}, {0x8ca0u, 4u, 6u, 2u},
        {0x8ca4u, 3u, 3u, 0u}, {0x8ca7u, 3u, 3u, 0u},
        {0x8caau, 3u, 7u, 2u}, {0x8cadu, 1u, 2u, 0u},
        {0x8caeu, 1u, 2u, 0u}, {0x8cafu, 3u, 3u, 0u},
        {0x8cb2u, 3u, 3u, 0u}, {0x8cb5u, 1u, 2u, 0u},
        {0x8cb6u, 2u, 3u, 0u}, {0x8cb8u, 1u, 2u, 0u},
        {0x8cb9u, 1u, 2u, 0u}, {0x8cbau, 3u, 3u, 0u},
        {0x8cbdu, 2u, 3u, 0u}
    };
    if (cpu->Y == 0x3eu)
        timing[18].cycles = 2u;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x840000u;
    clock.timing = timing;
    clock.count = 19u;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleSwirlRimRowStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
