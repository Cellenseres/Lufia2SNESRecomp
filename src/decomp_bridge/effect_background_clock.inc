#include "lufia2/battle_effect_background_steps.h"

static void ActorBridgeEffectBackground(CpuState *cpu, uint32_t entry);

static uint32_t EffectBackgroundEnd(uint32_t pc) {
    switch (pc) {
    case 0x818b69u: return 0x818b6eu;
    case 0x818b6eu: return 0x818b7au;
    case 0x818b95u: return 0x818b9au;
    default: return 0u;
    }
}

static bool EffectBackgroundHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return EffectBackgroundEnd(pc) && hook == ActorBridgeEffectBackground;
}

static void ActorBridgeEffectBackground(CpuState *cpu, uint32_t entry) {
    const uint32_t end = EffectBackgroundEnd(entry);
    const bool destination = entry == 0x818b6eu;
    const unsigned count = destination ? LUFIA2_EFFECT_BACKGROUND_DESTINATION_OPCODES
        : LUFIA2_EFFECT_BACKGROUND_HEADER_OPCODES;
    const unsigned reserve = destination ? 256u : 128u;
    if (!end || cpu != &g_cpu || cpu->PB != 0x81u ||
        (destination && cpu->S < 0x1f02u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffcu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, EffectBackgroundHookOwned) ||
        Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeEffectBackground) < count)
        return;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return;
    const bool paired = entry == 0x818b69u &&
        cpu->master_cycles <= UINT64_MAX - 192u &&
        NativeOpcodeWindowSupported(cpu, entry, 192u, 0x1ffcu, true) &&
        Lufia2DecompFrameRangeAllows(entry, 0x818b9au, EffectBackgroundHookOwned) &&
        Lufia2DecompFrameOwnedSpanBudget(entry, 0x818b9au,
            ActorBridgeEffectBackground) >= 2u * LUFIA2_EFFECT_BACKGROUND_HEADER_OPCODES;
    const uint64_t start = cpu->master_cycles;
    FixedSpanOpcode timing[LUFIA2_EFFECT_BACKGROUND_DESTINATION_OPCODES];
    if (destination) {
        timing[0] = (FixedSpanOpcode){0x8b6eu, 2u, 3u, 0u};
        timing[1] = (FixedSpanOpcode){0x8b70u, 3u, 3u, 0u};
        timing[2] = (FixedSpanOpcode){0x8b73u, 1u, 2u, 0u};
        timing[3] = (FixedSpanOpcode){0x8b74u, 1u, 2u, 0u};
        timing[4] = (FixedSpanOpcode){0x8b75u, 4u, 6u, 2u};
        timing[5] = (FixedSpanOpcode){0x8b79u, 1u, 4u, 2u};
    } else {
        timing[0] = (FixedSpanOpcode){(uint16_t)entry, 3u,
            (uint8_t)(4u + !cpu->m_flag), (uint8_t)(1u + !cpu->m_flag)};
        timing[1] = (FixedSpanOpcode){(uint16_t)(entry + 3u), 2u, 2u, 0u};
    }
    Lufia2DecompRomOpcodeSpan fetch[2];
    const bool first_fetch = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, end - entry, &fetch[0]);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u,
        .timing = timing, .count = count, .opcode_span = first_fetch ? &fetch[0] : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result = destination
        ? Lufia2BattleEffectPrepareBackgroundDestination(&memory, &state)
        : Lufia2BattleEffectBackgroundStep(&memory, &state, entry);
    if (!destination)
        timing[1].cycles = result.pc == end ? 2u : 3u;
    if (paired && result.pc == 0x818b95u) {
        BeginFixedSpanOpcode(&clock);
        const bool second_fetch = Lufia2DecompPrepareRomOpcodeSpan(cpu, 0x818b95u, 5u, &fetch[1]);
        clock.opcode_span = second_fetch ? &fetch[1] : NULL;
        timing[2] = (FixedSpanOpcode){0x8b95u, 3u,
            (uint8_t)(4u + !cpu->m_flag), (uint8_t)(1u + !cpu->m_flag)};
        timing[3] = (FixedSpanOpcode){0x8b98u, 2u, 2u, 0u};
        clock.count = 2u * LUFIA2_EFFECT_BACKGROUND_HEADER_OPCODES;
        result = Lufia2BattleEffectBackgroundStep(&memory, &state, 0x818b95u);
        timing[3].cycles = result.pc == 0x818b9au ? 2u : 3u;
    }
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= (paired ? 192u : reserve));
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
