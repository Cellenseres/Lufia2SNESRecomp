#include "lufia2/battle_window_steps.h"

static void ActorBridgeBattleWindowStep(CpuState *cpu, uint32_t entry) {
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
