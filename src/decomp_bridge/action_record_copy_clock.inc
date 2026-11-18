#include "lufia2/battle_action_work_steps.h"

enum { ACTION_RECORD_COPY_BYTES = 12u, ACTION_RECORD_COPY_OPCODES = 6u,
    ACTION_RECORD_COPY_MAX_OPCODES =
        ACTION_RECORD_COPY_OPCODES * LUFIA2_BATTLE_ACTION_RECORD_COPY_MAX_BYTES };
_Static_assert(ACTION_RECORD_COPY_MAX_OPCODES <= 32u, "Action record progress mask");

static void ActorBridgeActionRecordCopySpan(CpuState *cpu, uint32_t entry);

static bool ActionRecordCopyHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x85cdbbu || pc == 0x85cde5u) && hook == ActorBridgeActionRecordCopySpan;
}

static void ActorBridgeActionRecordCopySpan(CpuState *cpu, uint32_t entry) {
    if (!ActionRecordCopyHookOwned(entry, ActorBridgeActionRecordCopySpan) ||
        cpu != &g_cpu || cpu->PB != 0x85u || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->D || cpu->_flag_D || cpu->Y >= ACTION_RECORD_COPY_BYTES ||
        !cpu->ram || cpu->S < 0x1f00u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 13u, ActionRecordCopyHookOwned))
        return;
    unsigned bytes = Lufia2DecompFrameSpanBudget(entry, entry + 13u,
        ActorBridgeActionRecordCopySpan) / ACTION_RECORD_COPY_OPCODES;
    if (bytes > LUFIA2_BATTLE_ACTION_RECORD_COPY_MAX_BYTES)
        bytes = LUFIA2_BATTLE_ACTION_RECORD_COPY_MAX_BYTES;
    if (bytes > ACTION_RECORD_COPY_BYTES - cpu->Y)
        bytes = ACTION_RECORD_COPY_BYTES - cpu->Y;
    const unsigned pointer_offset = entry == 0x85cdbbu ? 0xb5u : 0xb8u;
    const uint32_t pointer = cpu->ram[pointer_offset] |
        ((uint32_t)cpu->ram[pointer_offset + 1u] << 8u) |
        ((uint32_t)cpu->ram[pointer_offset + 2u] << 16u);
    const uint32_t destination = pointer + cpu->Y;
    if (destination < 0x7e0000u || destination >= 0x800000u)
        return;
    if (bytes > 0x800000u - destination)
        bytes = 0x800000u - destination;
    const uint32_t pointer_address = 0x7e0000u | pointer_offset;
    // Recheck a changed pointer before it can select a device address.
    if (destination <= pointer_address + 2u && destination + bytes > pointer_address)
        bytes = bytes ? 1u : 0u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (!bytes || (deadline && deadline <= cpu->master_cycles))
        return;
    uint64_t room = 1363u;
    if (deadline && room > deadline - cpu->master_cycles)
        room = deadline - cpu->master_cycles;
    const unsigned iteration = 13u * cpu_region_speed(entry) + 58u;
    const unsigned timed_bytes = room > 40u ? (unsigned)((room - 40u) / iteration) : 0u;
    if (bytes > timed_bytes)
        bytes = timed_bytes;
    const unsigned reserve = bytes * iteration + 40u;
    if (!bytes || cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowWithDeferredHdmaSupported(cpu, entry, reserve,
            0x1fffu, false, true))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[ACTION_RECORD_COPY_MAX_OPCODES];
    for (unsigned index = 0u; index < bytes; ++index) {
        FixedSpanOpcode *step = timing + ACTION_RECORD_COPY_OPCODES * index;
        step[0] = (FixedSpanOpcode){(uint16_t)entry, 4u, 5u, 1u};
        step[1] = (FixedSpanOpcode){(uint16_t)(entry + 4u), 2u, 6u, 4u};
        step[2] = (FixedSpanOpcode){(uint16_t)(entry + 6u), 1u, 2u, 0u};
        step[3] = (FixedSpanOpcode){(uint16_t)(entry + 7u), 1u, 2u, 0u};
        step[4] = (FixedSpanOpcode){(uint16_t)(entry + 8u), 3u, 3u, 0u};
        step[5] = (FixedSpanOpcode){(uint16_t)(entry + 11u), 2u,
            cpu->Y + index + 1u == ACTION_RECORD_COPY_BYTES ? 2u : 3u, 0u};
    }
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, 13u, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u, .timing = timing,
        .count = ACTION_RECORD_COPY_OPCODES * bytes, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result =
        Lufia2BattleCopyActionRecordSpan(&memory, &state, entry, bytes);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == entry || result.pc == entry + 13u));
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
