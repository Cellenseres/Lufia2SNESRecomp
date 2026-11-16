#include "lufia2/battle_target_name_steps.h"

static void ActorBridgeTargetCursorPhase(CpuState *cpu, uint32_t entry);

static uint32_t TargetCursorPhaseEnd(uint32_t pc) {
    switch (pc) {
    case 0x81d5b7u: return 0x81d5e3u;
    case 0x81d5e7u: return 0x81d603u;
    case 0x81d603u: return 0x81d60eu;
    default: return 0u;
    }
}

static bool TargetCursorHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return TargetCursorPhaseEnd(pc) && hook == ActorBridgeTargetCursorPhase;
}

static void ActorBridgeTargetCursorPhase(CpuState *cpu, uint32_t entry) {
    const uint32_t end = TargetCursorPhaseEnd(entry);
    const unsigned reserve = entry == 0x81d5b7u ? 768u : entry == 0x81d5e7u ? 512u : 256u;
    if (!end || cpu != &g_cpu || cpu->PB != 0x81u || cpu->DB != 0x7eu ||
        cpu->D || cpu->_flag_D || cpu->m_flag != 1u || cpu->x_flag ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffcu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, end, TargetCursorHookOwned))
        return;
    if (entry == 0x81d5b7u &&
        ((cpu->A & 0xff00u) || cpu->ram[0x26u] > 5u || g_snes->cart->romSize <= 0x29ed3u))
        return;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return;
    const unsigned budget = Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeTargetCursorPhase);
    FixedSpanOpcode timing[24];
    unsigned count = 0u;
#define CURSOR_OPCODE(pc, fetched, cycles, data) \
    (timing[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    if (entry == 0x81d5b7u) {
        const bool clamped = cpu->ram[0x4abeu] >= 152u;
        CURSOR_OPCODE(0xd5b7u, 1u, 4u, 2u);
        CURSOR_OPCODE(0xd5b8u, 2u, 3u, 1u);
        CURSOR_OPCODE(0xd5bau, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5bbu, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5bcu, 2u, 3u, 0u);
        CURSOR_OPCODE(0xd5beu, 4u, 6u, 2u);
        CURSOR_OPCODE(0xd5c2u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5c3u, 3u, 5u, 2u);
        CURSOR_OPCODE(0xd5c6u, 3u, 3u, 0u);
        CURSOR_OPCODE(0xd5c9u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5cau, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5cbu, 3u, 3u, 0u);
        CURSOR_OPCODE(0xd5ceu, 2u, clamped ? 2u : 3u, 0u);
        if (clamped)
            CURSOR_OPCODE(0xd5d0u, 3u, 3u, 0u);
        CURSOR_OPCODE(0xd5d3u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5d4u, 3u, 3u, 0u);
        CURSOR_OPCODE(0xd5d7u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5d8u, 2u, 3u, 0u);
        CURSOR_OPCODE(0xd5dau, 2u, 2u, 0u);
        CURSOR_OPCODE(0xd5dcu, 2u, 3u, 1u);
        CURSOR_OPCODE(0xd5deu, 2u, 2u, 0u);
        CURSOR_OPCODE(0xd5e0u, 3u, 4u, 1u);
    } else if (entry == 0x81d5e7u) {
        const bool clamped = cpu->ram[0x4abeu] >= 152u;
        CURSOR_OPCODE(0xd5e7u, 3u, 4u, 1u);
        CURSOR_OPCODE(0xd5eau, 2u, 2u, 0u);
        CURSOR_OPCODE(0xd5ecu, 2u, clamped ? 2u : 3u, 0u);
        if (clamped)
            CURSOR_OPCODE(0xd5eeu, 2u, 2u, 0u);
        CURSOR_OPCODE(0xd5f0u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5f1u, 3u, 4u, 1u);
        CURSOR_OPCODE(0xd5f4u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5f5u, 3u, 4u, 1u);
        CURSOR_OPCODE(0xd5f8u, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5f9u, 2u, 2u, 0u);
        CURSOR_OPCODE(0xd5fbu, 3u, 4u, 1u);
        CURSOR_OPCODE(0xd5feu, 1u, 2u, 0u);
        CURSOR_OPCODE(0xd5ffu, 3u, 4u, 1u);
        CURSOR_OPCODE(0xd602u, 1u, 5u, 2u);
    } else {
        const bool hidden = cpu->ram[0x28u] != 0u;
        CURSOR_OPCODE(0xd603u, 2u, 3u, 1u);
        CURSOR_OPCODE(0xd605u, 2u, hidden ? 3u : 2u, 0u);
        if (!hidden) {
            CURSOR_OPCODE(0xd607u, 3u, 3u, 0u);
            CURSOR_OPCODE(0xd60au, 2u, 2u, 0u);
            CURSOR_OPCODE(0xd60cu, 2u, 3u, 1u);
        }
    }
#undef CURSOR_OPCODE
    if (budget < count)
        return;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleTargetCursorStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
