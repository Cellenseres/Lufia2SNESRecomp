#include "lufia2/battle_timed_callbacks.h"

enum { BATTLE_TIMER_CALL_CLOCK_CAPACITY = 96u };

typedef struct BattleTimerCallClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[BATTLE_TIMER_CALL_CLOCK_CAPACITY];
    uint32_t next;
    bool child;
    bool returning;
    unsigned visited;
} BattleTimerCallClock;

static void BattleTimerCallOpcode(BattleTimerCallClock *clock, uint16_t pc,
    unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < BATTLE_TIMER_CALL_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static bool PrepareBattleTimerCallClock(BattleTimerCallClock *clock,
                                        CpuState *cpu, uint32_t entry) {
    if (!cpu->ram || cpu->PB != 0x85u || cpu->DB != 0x85u || cpu->D ||
        cpu->emulation || cpu->_flag_D || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        (entry != 0x858f1bu && entry != 0x858f3cu))
        return false;
    unsigned slot = entry == 0x858f1bu ? 0u : (uint8_t)cpu->X;
    if (slot > 0x38u || (slot & 7u))
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x850000u;
    clock->span.timing = clock->timing;
    clock->child = false;
    clock->returning = false;
    BattleTimerCallOpcode(clock, (uint16_t)entry, 2u, 3u, 0u);
    if (entry == 0x858f1bu) {
        clock->visited = 0u;
        BattleTimerCallOpcode(clock, 0x8f1du, 2u, 2u, 0u);
    } else {
        BattleTimerCallOpcode(clock, 0x8f3eu, 1u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f3fu, 1u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f40u, 2u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f42u, 1u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f43u, 2u, 2u, 0u);
        slot += 8u;
        BattleTimerCallOpcode(clock, 0x8f45u, 2u, slot == 0x40u ? 2u : 3u, 0u);
        if (slot != 0x40u && clock->visited == 8u) {
            clock->next = 0x858f1fu;
            return true;
        }
    }
    for (; slot < 0x40u; slot += 8u) {
        ++clock->visited;
        const bool active = cpu->ram[0x1b17u + slot] != 0u;
        const bool expired = active && cpu->ram[0x1b18u + slot] == 1u;
        BattleTimerCallOpcode(clock, 0x8f1fu, 3u, 4u, 1u);
        BattleTimerCallOpcode(clock, 0x8f22u, 2u, active ? 2u : 3u, 0u);
        if (active) {
            BattleTimerCallOpcode(clock, 0x8f24u, 3u, 7u, 2u);
            BattleTimerCallOpcode(clock, 0x8f27u, 2u, expired ? 2u : 3u, 0u);
        }
        if (expired) {
            const unsigned selector = (uint8_t)(cpu->ram[0x1b19u + slot] << 1);
            BattleTimerCallOpcode(clock, 0x8f29u, 3u, 4u, 1u);
            BattleTimerCallOpcode(clock, 0x8f2cu, 1u, 2u, 0u);
            BattleTimerCallOpcode(clock, 0x8f2du, 1u, 2u, 0u);
            BattleTimerCallOpcode(clock, 0x8f2eu, 2u, 3u, 0u);
            BattleTimerCallOpcode(clock, 0x8f30u, 3u,
                5u + ((0x05u + selector) > 0xffu), 2u);
            BattleTimerCallOpcode(clock, 0x8f33u, 3u, 5u, 2u);
            BattleTimerCallOpcode(clock, 0x8f36u, 1u, 4u, 2u);
            BattleTimerCallOpcode(clock, 0x8f37u, 2u, 3u, 0u);
            BattleTimerCallOpcode(clock, 0x8f39u, 2u, 3u, 0u);
            BattleTimerCallOpcode(clock, 0x8f3bu, 1u, 6u, 2u);
            clock->child = true;
            clock->next = 0x858f3cu;
            return true;
        }
        BattleTimerCallOpcode(clock, 0x8f3eu, 1u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f3fu, 1u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f40u, 2u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f42u, 1u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f43u, 2u, 2u, 0u);
        BattleTimerCallOpcode(clock, 0x8f45u, 2u, slot == 0x38u ? 2u : 3u, 0u);
        if (slot != 0x38u && clock->visited == 8u) {
            clock->next = 0x858f1fu;
            return true;
        }
    }
    BattleTimerCallOpcode(clock, 0x8f47u, 2u, 3u, 0u);
    clock->returning = true;
    clock->next = 0x858f49u;
    return true;
}

static bool BattleTimerCallWindowSupported(const BattleTimerCallClock *clock) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    const CpuState *cpu = clock->span.cpu;
    if (!Lufia2DecompFrameReturnCpuReady() ||
        !Lufia2DecompNmiHasInterruptOwner() ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) || cpu->S < 0x1f20u || cpu->S > 0x1ff8u ||
        cpu->master_cycles != cpu->coprocessor_master_cycles ||
        !g_snes || !g_snes->dma || g_snes->ram != cpu->ram ||
        !g_snes->inNmi || g_snes->inIrq ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        cpu->ram[cpu->S + 1u] != 0x78u || cpu->ram[cpu->S + 2u] != 0x8eu)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    uint64_t budget = clock->returning
        ? cpu_region_speed(0x858f49u) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned index = 0u; index < clock->span.count; ++index) {
        const FixedSpanOpcode *op = clock->timing + index;
        budget += op->fetched * cpu_region_speed(0x850000u | op->pc) +
            op->data_bytes * 8u +
            (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || current != cpu->master_cycles)
        return false;
    const unsigned edge = g_snes->vPos < 225u ? 225u : 262u;
    return budget < (uint64_t)(edge - g_snes->vPos) * 1364u - g_snes->hPos &&
        NmiSpanAvoidsHdma(g_snes, budget, true);
#else
    (void)clock;
    return false;
#endif
}

static uint8_t BattleTimerCallChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    BattleTimerCallClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->child && site == 0x858f3bu && frame_size == 2u &&
        clock->span.index == clock->span.count && !clock->span.remaining);
    state->resume_pc = target;
    return 0u;
}

static void ActorBridgeBattleTimerCall(CpuState *cpu, uint32_t entry) {
    BattleTimerCallClock clock = {0};
    if (!PrepareBattleTimerCallClock(&clock, cpu, entry) ||
        !BattleTimerCallWindowSupported(&clock))
        return;
    const Lufia2Memory memory = {
        FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x858f1bu
        ? Lufia2BattleRunTimedCallbacks(&memory, &state, BattleTimerCallChild, &clock)
        : Lufia2BattleResumeTimedCallbacks(&memory, &state, BattleTimerCallChild, &clock);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    const uint32_t next = result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND
        ? state.resume_pc : result.pc;
    assert((clock.child && result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) ||
        (clock.returning && next == 0x858f49u));
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(next);
}
