#include "lufia2/battle_background_steps.h"

enum {
    BACKGROUND_SPAN_MAX_OPCODES = LUFIA2_BACKGROUND_SPAN_MAX_WORDS * LUFIA2_BACKGROUND_WORD_OPCODES
};
_Static_assert(BACKGROUND_SPAN_MAX_OPCODES <= 32u, "Background span progress mask");

static unsigned BackgroundSpanReserve(const CpuState *cpu, unsigned words) {
    const unsigned fetched = 14u * cpu_region_speed(0x81ba59u);
    unsigned reserve = 40u;
    for (unsigned index = 0u; index < words; ++index)
        reserve += fetched + 68u + (cpu->X + 2u * index >= 0x100u ? 6u : 0u);
    return reserve;
}

static bool TryBackgroundWordSpan(CpuState *cpu, uint32_t entry) {
    if (!BackgroundWordHookOwned(entry, ActorBridgeBackgroundDecodeWord) ||
        cpu != &g_cpu || cpu->PB != 0x81u || cpu->DB != 0x7eu ||
        cpu->m_flag || cpu->x_flag || cpu->X < 2u || cpu->X >= 0x800u || (cpu->X & 1u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !Lufia2DecompFrameRangeAllows(entry, 0x81ba67u, BackgroundWordHookOwned))
        return false;
    unsigned words = Lufia2DecompFrameSpanBudget(entry, 0x81ba67u,
        ActorBridgeBackgroundDecodeWord) / LUFIA2_BACKGROUND_WORD_OPCODES;
    if (words > LUFIA2_BACKGROUND_SPAN_MAX_WORDS)
        words = LUFIA2_BACKGROUND_SPAN_MAX_WORDS;
    const unsigned remaining = (0x800u - cpu->X) / 2u;
    if (words > remaining)
        words = remaining;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (!words || deadline <= cpu->master_cycles)
        return false;
    const uint64_t room = deadline - cpu->master_cycles;
    while (words && BackgroundSpanReserve(cpu, words) > room)
        --words;
    const unsigned reserve = BackgroundSpanReserve(cpu, words);
    if (!words || cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return false;
    FixedSpanOpcode timing[BACKGROUND_SPAN_MAX_OPCODES];
    for (unsigned index = 0u; index < words; ++index) {
        const unsigned offset = index * LUFIA2_BACKGROUND_WORD_OPCODES;
        const unsigned x = cpu->X + 2u * index;
        timing[offset] = (FixedSpanOpcode){0xba59u, 1u, 2u, 0u};
        timing[offset + 1u] = (FixedSpanOpcode){0xba5au, 3u,
            (uint8_t)(5u + (x >= 0x100u)), 2u};
        timing[offset + 2u] = (FixedSpanOpcode){0xba5du, 3u, 7u, 2u};
        timing[offset + 3u] = (FixedSpanOpcode){0xba60u, 1u, 2u, 0u};
        timing[offset + 4u] = (FixedSpanOpcode){0xba61u, 1u, 2u, 0u};
        timing[offset + 5u] = (FixedSpanOpcode){0xba62u, 3u, 3u, 0u};
        timing[offset + 6u] = (FixedSpanOpcode){0xba65u, 2u, x == 0x7feu ? 2u : 3u, 0u};
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 14u, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = words * LUFIA2_BACKGROUND_WORD_OPCODES,
        .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleDecodeBackgroundWordsSpan(&memory, &state, words);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == entry || result.pc == 0x81ba67u));
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
    return true;
}
