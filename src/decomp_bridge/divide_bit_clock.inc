#include "lufia2/math_steps.h"

enum { DIVIDE_SPAN_OPCODES = LUFIA2_DIVIDE_SPAN_MAX_BITS * LUFIA2_DIVIDE_BIT_MAX_OPCODES };
_Static_assert(DIVIDE_SPAN_OPCODES <= 32u, "Division progress mask");

typedef struct DivideBitClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[DIVIDE_SPAN_OPCODES];
    Lufia2CpuState *state;
    BattleDividePhase phase;
    unsigned bit;
    unsigned end_bit;
} DivideBitClock;

static void AppendDivideBitPhase(DivideBitClock *clock, BattleDividePhase phase) {
    BattleDivideClock plan = {.bit = clock->bit};
    SetBattleDividePhase(&plan, phase);
    assert(clock->span.count + plan.span.count <= DIVIDE_SPAN_OPCODES);
    for (unsigned i = 0u; i < plan.span.count; ++i)
        clock->timing[clock->span.count++] = plan.timing[i];
    clock->phase = phase;
}

static void PrepareDivideBitAccess(DivideBitClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index != clock->span.count || clock->span.remaining)
            return;
        BattleDividePhase next;
        if (clock->phase == DIVIDE_SHIFT)
            next = clock->state->carry ? DIVIDE_SUBTRACT_CARRY : DIVIDE_COMPARE;
        else if (clock->phase == DIVIDE_COMPARE)
            next = clock->state->carry ? DIVIDE_SUBTRACT_COMPARE : DIVIDE_SKIP;
        else {
            assert(clock->phase == DIVIDE_SKIP ||
                clock->phase == DIVIDE_SUBTRACT_CARRY ||
                clock->phase == DIVIDE_SUBTRACT_COMPARE);
            if (++clock->bit == clock->end_bit)
                return;
            next = DIVIDE_SHIFT;
        }
        AppendDivideBitPhase(clock, next);
    }
}

static uint8_t DivideBitRead(void *context, uint32_t address) {
    DivideBitClock *clock = context;
    PrepareDivideBitAccess(clock);
    return FixedSpanRead(&clock->span, address);
}

static void DivideBitWrite(void *context, uint32_t address, uint8_t value) {
    DivideBitClock *clock = context;
    PrepareDivideBitAccess(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static void ActorBridgeDivideBitSpan(CpuState *cpu, uint32_t entry) {
    if (!DivideBitHookOwned(entry, ActorBridgeDivideBitSpan) || cpu != &g_cpu ||
        cpu->PB != 0x80u || cpu->m_flag || cpu->D || cpu->_flag_D ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const unsigned first_bit = (entry - 0x80837eu) / 13u;
    unsigned bits = first_bit < 15u ? LUFIA2_DIVIDE_SPAN_MAX_BITS : 1u;
    for (; bits; --bits) {
        const unsigned reserve = bits == 2u ? 480u : 264u;
        const uint32_t end = entry + bits * 13u;
        if (cpu->master_cycles <= UINT64_MAX - reserve &&
            NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true) &&
            Lufia2DecompFrameRangeAllows(entry, end, DivideBitHookOwned) &&
            Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeDivideBitSpan) >=
                bits * LUFIA2_DIVIDE_BIT_MAX_OPCODES)
            break;
    }
    if (!bits)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, bits * 13u, &fetch);
    DivideBitClock clock = {.span = {.cpu = cpu, .bank = 0x800000u,
        .opcode_span = prepared ? &fetch : NULL}, .state = &state,
        .bit = first_bit, .end_bit = first_bit + bits};
    clock.span.timing = clock.timing;
    AppendDivideBitPhase(&clock, DIVIDE_SHIFT);
    const Lufia2Memory memory = {DivideBitRead, DivideBitWrite, &clock, NULL, NULL};
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2DivideBitSpan(&memory, &state, entry, bits);
    PrepareDivideBitAccess(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry + bits * 13u);
    assert(clock.bit == clock.end_bit && clock.span.index == clock.span.count &&
        !clock.span.remaining);
    assert(cpu->master_cycles - start <= (bits == 2u ? 480u : 264u));
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
