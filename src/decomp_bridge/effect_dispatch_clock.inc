#include "lufia2/battle_effect_slot_steps.h"

static void ActorBridgeEffectDispatchPrepare(CpuState *cpu, uint32_t entry);

static bool EffectDispatchHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeEffectDispatchPrepare &&
        (pc == 0x818a95u || pc == 0x818b08u);
}

static unsigned EffectDispatchPrepareBudget(uint32_t entry) {
    const bool actor = entry == 0x818b08u;
    const unsigned rom_bytes = actor ? 14u : 16u;
    const unsigned wram_and_internal = actor ? 126u : 120u;
    /* Code and stream share FastROM timing. */
    return rom_bytes * cpu_region_speed(entry) + wram_and_internal + 40u;
}

static bool EffectDispatchPrepareClockSupported(const CpuState *cpu,
                                                 uint32_t entry) {
    if ((entry != 0x818a95u && entry != 0x818b08u) ||
        cpu->PB != 0x81u || (cpu->DB != 0x7eu && cpu->DB != 0x81u &&
        cpu->DB != 0x95u) || cpu->m_flag != 1u ||
        cpu->x_flag || cpu->D || cpu->_flag_D || !cpu->ram ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        cpu->ram[0xc5u] != 0x95u || !(cpu->ram[0xc4u] & 0x80u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes)
        return false;
    const unsigned budget = EffectDispatchPrepareBudget(entry);
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
            fence - budget - 1u, budget))
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && clock == cpu->master_cycles &&
        Lufia2DecompFrameRangeAllows(entry,
            entry + (entry == 0x818b08u ? 13u : 15u), EffectDispatchHookOwned);
}

static void ActorBridgeEffectDispatchPrepare(CpuState *cpu, uint32_t entry) {
    if (!EffectDispatchPrepareClockSupported(cpu, entry))
        return;
    const FixedSpanOpcode script[] = {
        {0x8a95u, 2u, 6u, 4u}, {0x8a97u, 2u, 3u, 0u},
        {0x8a99u, 3u, 3u, 0u}, {0x8a9cu, 1u, 2u, 0u},
        {0x8a9du, 1u, 2u, 0u}, {0x8a9eu, 2u, 7u, 4u},
        {0x8aa0u, 2u, 3u, 0u}, {0x8aa2u, 1u, 3u, 1u},
        {0x8aa3u, 1u, 4u, 1u}
    };
    const FixedSpanOpcode actor[] = {
        {0x8b08u, 1u, 2u, 0u}, {0x8b09u, 2u, 6u, 4u},
        {0x8b0bu, 2u, 3u, 0u}, {0x8b0du, 1u, 2u, 0u},
        {0x8b0eu, 1u, 2u, 0u}, {0x8b0fu, 2u, 7u, 4u},
        {0x8b11u, 2u, 3u, 0u}, {0x8b13u, 1u, 3u, 1u},
        {0x8b14u, 1u, 4u, 1u}
    };
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = entry == 0x818b08u ? actor : script;
    clock.count = 9u;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleEffectDispatchPrepareStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == 9u && !clock.remaining);
    assert(result.pc == 0x818aa4u || result.pc == 0x818b15u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(9u, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
