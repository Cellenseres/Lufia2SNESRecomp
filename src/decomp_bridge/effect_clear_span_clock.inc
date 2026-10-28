#include "lufia2/battle_effect_slot_steps.h"

static void ActorBridgeEffectClearBytes(CpuState *cpu, uint32_t entry);

static unsigned EffectClearSpanSize(CpuState *cpu, uint32_t entry) {
    if (entry != 0x818bfbu || cpu->PB != 0x81u ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->DB != 0x97u ||
        cpu->D || cpu->_flag_D || cpu->emulation || !cpu->X || cpu->X > 0x800u ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() || !g_snes || g_snes->vPos >= 262u ||
        g_snes->ramAdr != 0x3800u - cpu->X ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false, 1363u, 1u) ||
        !Lufia2DecompFrameRangeQuiet(entry + 1u, entry + 6u))
        return 0u;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return 0u;
    const unsigned opcodes = Lufia2DecompFrameSpanBudget(
        entry, entry + 6u, ActorBridgeEffectClearBytes);
    unsigned limit = opcodes / 3u;
    if (!opcodes)
        limit = 4u;
    if (limit > LUFIA2_BATTLE_EFFECT_CLEAR_SPAN_MAX_BYTES)
        limit = LUFIA2_BATTLE_EFFECT_CLEAR_SPAN_MAX_BYTES;
    if (limit > cpu->X)
        limit = cpu->X;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u &&
        g_snes->hPos < 1024u ? 1024u : 1364u;
    uint64_t room = fence - g_snes->hPos - 1u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline) {
        if (deadline <= cpu->master_cycles)
            return 0u;
        if (room > deadline - cpu->master_cycles)
            room = deadline - cpu->master_cycles;
    }
    if (room > UINT64_MAX - cpu->master_cycles)
        room = UINT64_MAX - cpu->master_cycles;
    const unsigned iteration = 6u * cpu_region_speed(entry) + 20u;
    unsigned bytes = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (bytes > limit)
        bytes = limit;
    if (limit == cpu->X && bytes < limit &&
        limit * iteration + 34u <= room)
        bytes = limit;
    return bytes;
}

static void ActorBridgeEffectClearBytes(CpuState *cpu, uint32_t entry) {
    const unsigned bytes = EffectClearSpanSize(cpu, entry);
    if (!bytes)
        return;
    FixedSpanOpcode timing[3u * LUFIA2_BATTLE_EFFECT_CLEAR_SPAN_MAX_BYTES];
    for (unsigned index = 0u; index < bytes; ++index) {
        timing[3u * index] = (FixedSpanOpcode){0x8bfbu, 3u, 4u, 1u};
        timing[3u * index + 1u] = (FixedSpanOpcode){0x8bfeu, 1u, 2u, 0u};
        timing[3u * index + 2u] = (FixedSpanOpcode){0x8bffu, 2u,
            (uint8_t)(cpu->X == index + 1u ? 2u : 3u), 0u};
    }
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x810000u;
    clock.timing = timing;
    clock.count = 3u * bytes;
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleEffectClearByteSpan(&memory, &state, bytes);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.pc == 0x818bfbu || result.pc == 0x818c01u);
    ActorBridgeStore(cpu, &state);
    /* Every port write advances the bridge epoch. */
    Lufia2DecompFrameReturnCompleteSpan(clock.count, clock.count - 2u);
    interp_bridge_pre_opcode_redirect(result.pc);
}
