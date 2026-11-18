#include "lufia2/event_timer_steps.h"

_Static_assert(LUFIA2_EVENT_TIMER_MAX_OPCODES <= 32u, "Event timer progress mask");

static unsigned PlanEventTimerStep(const CpuState *cpu, uint32_t entry,
                                   FixedSpanOpcode *plan, uint32_t *next) {
    unsigned count = 0u;
    unsigned slot = cpu->X;
#define TIMER_OPCODE(pc, fetched, cycles, data)                                        \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x80cbaeu) {
        TIMER_OPCODE(0xcbaeu, 1u, 3u, 1u);
        TIMER_OPCODE(0xcbafu, 1u, 3u, 1u);
        TIMER_OPCODE(0xcbb0u, 2u, 3u, 0u);
        TIMER_OPCODE(0xcbb2u, 3u, 4u, 1u);
        TIMER_OPCODE(0xcbb5u, 2u, 2u, 0u);
        TIMER_OPCODE(0xcbb7u, 3u, 6u, 2u);
        TIMER_OPCODE(0xcbbau, 3u, 3u, 0u);
        slot = 0u;
    }
    if (entry == 0x80cbaeu || entry == 0x80cbbdu) {
        for (unsigned i = 0u; i < 2u; ++i) {
            const uint8_t timer = cpu->ram[0x1d18cu + slot];
            const bool active = (timer & 0x80u) != 0u;
            TIMER_OPCODE(0xcbbdu, 4u, 5u, 1u);
            TIMER_OPCODE(0xcbc1u, 2u, active ? 2u : 3u, 0u);
            if (active) {
                TIMER_OPCODE(0xcbc3u, 1u, 2u, 0u);
                TIMER_OPCODE(0xcbc4u, 4u, 5u, 1u);
                TIMER_OPCODE(0xcbc8u, 2u, 2u, 0u);
                TIMER_OPCODE(0xcbcau, 2u, timer == 0x81u ? 2u : 3u, 0u);
                if (timer == 0x81u) {
                    *next = 0x80cbccu;
                    return count;
                }
            }
            TIMER_OPCODE(0xcbf0u, 1u, 2u, 0u);
            TIMER_OPCODE(0xcbf1u, 3u, 3u, 0u);
            TIMER_OPCODE(0xcbf4u, 2u, slot == 7u ? 2u : 3u, 0u);
            if (++slot == 8u) {
                *next = 0x80cbf6u;
                return count;
            }
        }
        *next = 0x80cbbdu;
        return count;
    }
    if (entry == 0x80cbf6u) {
        TIMER_OPCODE(0xcbf6u, 3u, 3u, 0u);
        slot = 7u;
    }
    if (entry == 0x80cbf6u || entry == 0x80cbf9u) {
        for (unsigned i = 0u; i < 2u; ++i) {
            const bool active = (cpu->ram[0x1d18cu + slot] & 0x80u) != 0u;
            TIMER_OPCODE(0xcbf9u, 4u, 5u, 1u);
            TIMER_OPCODE(0xcbfdu, 2u, active ? 2u : 3u, 0u);
            if (active) {
                TIMER_OPCODE(0xcbffu, 2u, 2u, 0u);
                TIMER_OPCODE(0xcc01u, 3u, 6u, 2u);
                TIMER_OPCODE(0xcc04u, 2u, 3u, 0u);
                *next = 0x80cc09u;
                return count;
            }
            TIMER_OPCODE(0xcc06u, 1u, 2u, 0u);
            TIMER_OPCODE(0xcc07u, 2u, slot == 0u ? 2u : 3u, 0u);
            if (!slot--) {
                *next = 0x80cc09u;
                return count;
            }
        }
        *next = 0x80cbf9u;
        return count;
    }
    const bool redraw = cpu->ram[(cpu->DB == 0x7fu ? 0x10000u : 0u) + 0x1273u] != 0u;
    TIMER_OPCODE(0xcc09u, 3u, 4u, 1u);
    TIMER_OPCODE(0xcc0cu, 2u, redraw ? 2u : 3u, 0u);
    if (!redraw) {
        TIMER_OPCODE(0xcc23u, 1u, 4u, 1u);
        TIMER_OPCODE(0xcc24u, 1u, 4u, 1u);
    }
    *next = redraw ? 0x80cc0eu : 0x80cc25u;
#undef TIMER_OPCODE
    return count;
}

static void ActorBridgeEventTimerStep(CpuState *cpu, uint32_t entry) {
    const unsigned reserve = 768u;
    if (!EventTimerStepHookOwned(entry, ActorBridgeEventTimerStep) || cpu != &g_cpu ||
        !cpu->ram || cpu->PB != 0x80u || cpu->D || cpu->_flag_D || cpu->m_flag != 1u ||
        cpu->x_flag ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        ((entry == 0x80cbbdu || entry == 0x80cbf9u) && cpu->X >= 8u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffdu, false))
        return;
    const uint32_t fence = entry <= 0x80cbbdu   ? 0x80cbf6u
                           : entry <= 0x80cbf9u ? 0x80cc09u
                                                : 0x80cc25u;
    if (!Lufia2DecompFrameRangeAllows(entry, fence, EventTimerStepHookOwned))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[LUFIA2_EVENT_TIMER_MAX_OPCODES];
    uint32_t next;
    const unsigned count = PlanEventTimerStep(cpu, entry, timing, &next);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeEventTimerStep) <
        count)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned fetched = fence - entry < 32u ? fence - entry : 32u;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, fetched, &fetch);
    FixedSpanClock clock = {.cpu = cpu,
                            .bank = 0x800000u,
                            .timing = timing,
                            .count = count,
                            .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result =
        Lufia2FieldEventTimerStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
