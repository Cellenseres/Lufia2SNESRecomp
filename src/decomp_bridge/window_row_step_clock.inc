#include "lufia2/battle_window_steps.h"

static void ActorBridgeBattleWindowEnd(CpuState *cpu, uint32_t entry) {
    const bool finish = entry == 0x81e53eu;
    const uint32_t end = finish ? 0x81e541u : 0x81e53bu;
    const unsigned count = finish ? 2u : 4u;
    const unsigned reserve = 160u;
    if (cpu->m_flag || cpu->x_flag || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1ffeu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, BattleWindowStepHookOwned) ||
        Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeBattleWindowStep) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    static const uint8_t increments[] = {0xe8u, 0xe8u, 0xe8u, 0xe8u};
    static const uint8_t restore_bank[] = {0xe2u, 0x20u, 0xabu};
    const unsigned offset = (uint16_t)entry;
    const Cart *cart = g_snes->cart;
    if (cart->romSize < offset + end - entry ||
        memcmp(cart->rom + offset, finish ? restore_bank : increments, end - entry))
        return;

    const FixedSpanOpcode *timing = finish ? battle_window_finish : battle_window_bottom_call;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing, .count = count};
    if (finish) {
        cpu->DB = FixedSpanRead(&clock, ++cpu->S);
        cpu->m_flag = 1u;
        cpu->_flag_N = (cpu->DB & 0x80u) != 0u;
        cpu->_flag_Z = cpu->DB == 0u;
    } else {
        BeginFixedSpanOpcode(&clock);
        cpu->X = (uint16_t)(cpu->X + 4u);
        cpu->_flag_N = (cpu->X & 0x8000u) != 0u;
        cpu->_flag_Z = cpu->X == 0u;
    }
    assert(clock.index == clock.count && !clock.remaining);
    cpu_mirrors_to_p(cpu);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(end);
}

static void ActorBridgeBattleWindowStep(CpuState *cpu, uint32_t entry) {
    if (entry == 0x81e537u || entry == 0x81e53eu) {
        ActorBridgeBattleWindowEnd(cpu, entry);
        return;
    }
    const bool begin = entry == 0x81e503u;
    if (!BattleWindowStepHookOwned(entry, ActorBridgeBattleWindowStep) || cpu != &g_cpu ||
        cpu->PB != 0x81u || cpu->m_flag != begin || cpu->x_flag || cpu->D || cpu->_flag_D ||
        (!begin && cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const unsigned reserve = begin ? 456u : entry == 0x81e542u ? 272u
        : entry == 0x81e55du ? 280u : entry == 0x81e527u ? 216u : 128u;
    const uint32_t end = begin ? 0x81e524u : entry == 0x81e542u ? 0x81e553u
        : entry == 0x81e55du ? 0x81e56fu : 0x81e537u;
    if (cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true) ||
        !Lufia2DecompFrameRangeAllows(entry, end, BattleWindowStepHookOwned))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    const FixedSpanOpcode *plan;
    unsigned count;
    uint32_t stop;
    if (begin) {
        plan = battle_window_setup;
        count = LUFIA2_WINDOW_BEGIN_OPCODES;
        stop = end;
    } else if (entry == 0x81e542u || entry == 0x81e55du) {
        const bool head = entry == 0x81e542u;
        const unsigned output = (unsigned)cpu->Y + (head ? 0u : 2u);
        if (cpu->X < 0x8000u || cpu->X > 0xfffau ||
            !BattleWindowOutputSupported(output, output + 2u))
            return;
        plan = head ? battle_window_row_head : battle_window_row_tail;
        count = head ? LUFIA2_WINDOW_ROW_HEAD_OPCODES : LUFIA2_WINDOW_ROW_TAIL_OPCODES;
        stop = end;
    } else {
        const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
        const uint16_t rows = cpu->ram[bank + 0x09feu] |
            (uint16_t)cpu->ram[bank + 0x09ffu] << 8;
        const bool more = rows && rows <= 0x8000u;
        plan = more ? battle_window_top_more : battle_window_top_last;
        count = entry == 0x81e527u ? 8u : 2u;
        if (count == 2u)
            plan += 6u;
        stop = more ? 0x81e532u : 0x81e537u;
    }
    if (Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeBattleWindowStep) < count)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned bytes = begin ? 32u : entry == 0x81e542u ? 17u
        : entry == 0x81e55du ? 18u : entry == 0x81e527u ? 11u : 5u;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, bytes, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = plan,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleWindowStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == stop);
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
