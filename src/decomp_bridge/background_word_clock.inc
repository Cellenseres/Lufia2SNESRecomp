#include "lufia2/battle_background_steps.h"

enum { BACKGROUND_WORD_BUDGET = 320u };

static bool BackgroundWordClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x81ba59u || cpu->m_flag || cpu->x_flag ||
        cpu->DB != 0x7eu || cpu->D || cpu->_flag_D ||
        cpu->X < 2u || cpu->X >= 0x0800u || (cpu->X & 1u) ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - BACKGROUND_WORD_BUDGET || !g_snes)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true,
            fence - BACKGROUND_WORD_BUDGET - 1u, BACKGROUND_WORD_BUDGET))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static void ActorBridgeBackgroundDecodeWord(CpuState *cpu, uint32_t entry) {
    if (!BackgroundWordClockSupported(cpu, entry))
        return;
    const FixedSpanOpcode timing[] = {
        {0xba59u, 1u, 2u, 0u},
        {0xba5au, 3u, (uint8_t)(5u + (cpu->X >= 0x100u)), 2u},
        {0xba5du, 3u, 7u, 2u}, {0xba60u, 1u, 2u, 0u},
        {0xba61u, 1u, 2u, 0u}, {0xba62u, 3u, 3u, 0u},
        {0xba65u, 2u, (uint8_t)(cpu->X == 0x07feu ? 2u : 3u), 0u}
    };
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = 7u;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleDecodeBackgroundWordStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == 7u && !clock.remaining);
    assert(result.pc == 0x81ba59u || result.pc == 0x81ba67u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(7u, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
