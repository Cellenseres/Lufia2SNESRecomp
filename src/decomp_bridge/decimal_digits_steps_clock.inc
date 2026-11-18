#include "lufia2/decimal_digits_steps.h"

enum { DECIMAL_DIGITS_MAX_SPAN = 3u + LUFIA2_DECIMAL_DIGITS_STEP_MAX_OPCODES };
_Static_assert(DECIMAL_DIGITS_MAX_SPAN <= 32u, "Decimal digit progress mask");

static unsigned PrepareDecimalDigitClock(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *timing, uint32_t *next) {
    const FixedSpanOpcode *source;
    unsigned count;
    if (entry == 0x81e808u) {
        source = battle_digits_setup;
        count = 3u;
        *next = 0x81e80eu;
    } else if (entry == 0x81e80eu) {
        const bool counted = cpu->X >= 100u;
        source = counted ? battle_digits_hundreds : battle_digits_hundreds_done;
        count = counted ? 9u : 5u;
        *next = counted ? 0x81e80eu : 0x81e81fu;
    } else if (entry == 0x81e81fu) {
        const bool counted = cpu->X >= 10u;
        source = counted ? battle_digits_tens : battle_digits_finish;
        count = counted ? 9u : 4u;
        *next = counted ? 0x81e81fu : 0x81e82fu;
    } else {
        source = battle_digits_finish + 4u;
        count = 3u;
        *next = 0x81e834u;
    }
    memcpy(timing, source, count * sizeof timing[0]);
    return count;
}

static void ActorBridgeDecimalDigitsStep(CpuState *cpu, uint32_t entry) {
    const bool byte_accumulator = entry == 0x81e808u || entry == 0x81e80eu;
    if (!DecimalDigitsHookOwned(entry, ActorBridgeDecimalDigitsStep) || cpu != &g_cpu ||
        cpu->PB != 0x81u || cpu->D || cpu->_flag_D || cpu->x_flag ||
        cpu->m_flag != byte_accumulator ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu && (cpu->DB & 0x7fu) >= 0x40u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    FixedSpanOpcode timing[DECIMAL_DIGITS_MAX_SPAN];
    uint32_t next;
    unsigned count = PrepareDecimalDigitClock(cpu, entry, timing, &next);
    const unsigned first_count = count;
    unsigned phases = entry == 0x81e808u ? 2u : 1u;
    unsigned reserve = 0u;
    uint32_t fence = 0u;
    for (; phases; --phases) {
        const bool combined = phases == 2u;
        reserve = combined ? 352u : entry == 0x81e808u || entry == 0x81e82fu ? 128u : 256u;
        fence = entry == 0x81e808u ? (combined ? 0x81e81fu : 0x81e80eu)
            : entry == 0x81e80eu ? 0x81e81fu : entry == 0x81e81fu ? 0x81e82fu : 0x81e834u;
        count = first_count;
        if (combined)
            count += PrepareDecimalDigitClock(cpu, 0x81e80eu, timing + count, &next);
        else
            (void)PrepareDecimalDigitClock(cpu, entry, timing, &next);
        if (cpu->master_cycles > UINT64_MAX - reserve ||
            !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true) ||
            !Lufia2DecompFrameRangeAllows(entry, fence, DecimalDigitsHookOwned))
            continue;
        const unsigned budget = combined
            ? Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeDecimalDigitsStep)
            : Lufia2DecompFrameSpanBudget(entry, fence, ActorBridgeDecimalDigitsStep);
        if (budget >= count)
            break;
    }
    if (!phases)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry,
        (unsigned)(fence - entry), &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    Lufia2ExecutionResult result = Lufia2DecimalDigitsStep(&memory, &state, entry);
    if (phases == 2u)
        result = Lufia2DecimalDigitsStep(&memory, &state, result.pc);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
