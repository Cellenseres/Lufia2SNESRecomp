#include "lufia2/battle_effect_slot_steps.h"

enum { EFFECT_CLEAR_STEP_BUDGET = 352u };

static bool EffectClearClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x818bfbu || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->DB != 0x97u || cpu->D || cpu->_flag_D || cpu->emulation ||
        !cpu->X || cpu->X > 0x800u ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes ||
        g_snes->ramAdr != 0x3800u - cpu->X ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - EFFECT_CLEAR_STEP_BUDGET)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
            fence - EFFECT_CLEAR_STEP_BUDGET - 1u, EFFECT_CLEAR_STEP_BUDGET))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static void ActorBridgeEffectClearBytes(CpuState *cpu, uint32_t entry) {
    if (!EffectClearClockSupported(cpu, entry))
        return;
    const unsigned bytes = cpu->X < 4u ? cpu->X : 4u;
    FixedSpanOpcode timing[12];
    unsigned count = 0u;
    for (unsigned byte = 0u; byte < bytes; ++byte) {
        timing[count++] = (FixedSpanOpcode){0x8bfbu, 3u, 4u, 1u};
        timing[count++] = (FixedSpanOpcode){0x8bfeu, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x8bffu, 2u,
            (uint8_t)(cpu->X == byte + 1u ? 2u : 3u), 0u};
    }
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = count;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleEffectClearBytesStep(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.pc == 0x818bfbu || result.pc == 0x818c01u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
