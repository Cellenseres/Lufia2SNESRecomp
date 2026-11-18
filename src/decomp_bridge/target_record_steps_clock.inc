#include "lufia2/battle_target_record_steps.h"

_Static_assert(LUFIA2_BATTLE_TARGET_RECORD_MAX_OPCODES <= 32u, "Target record progress mask");

static const FixedSpanOpcode target_record_start[] = {
    {0xd4e5u, 1u, 4u, 1u},
    {0xd4e6u, 1u, 4u, 2u},
    {0xd4e7u, 2u, 3u, 1u},
    {0xd4e9u, 1u, 3u, 1u},
    {0xd4eau, 2u, 3u, 0u},
    {0xd4ecu, 3u, 3u, 0u},
    {0xd4efu, 3u, 3u, 0u},
};

static const FixedSpanOpcode target_record_clear[] = {
    {0xd4f2u, 3u, 7u, 2u},
    {0xd4f5u, 1u, 2u, 0u},
    {0xd4f6u, 1u, 2u, 0u},
    {0xd4f7u, 1u, 2u, 0u},
    {0xd4f8u, 1u, 2u, 0u},
    {0xd4f9u, 1u, 2u, 0u},
    {0xd4fau, 2u, 3u, 0u},
};

static const FixedSpanOpcode target_record_party_start[] = {
    {0xd4fcu, 3u, 3u, 0u},
    {0xd4ffu, 1u, 2u, 0u},
};

static const FixedSpanOpcode target_record_party_present[] = {
    {0xd500u, 2u, 3u, 0u},
    {0xd502u, 4u, 6u, 2u},
    {0xd506u, 3u, 7u, 2u},
    {0xd509u, 3u, 5u, 2u},
    {0xd50cu, 2u, 3u, 0u},
    {0xd514u, 2u, 3u, 0u},
    {0xd516u, 1u, 2u, 0u},
    {0xd517u, 1u, 2u, 0u},
    {0xd518u, 1u, 2u, 0u},
    {0xd519u, 1u, 2u, 0u},
    {0xd51au, 1u, 2u, 0u},
    {0xd51bu, 1u, 2u, 0u},
    {0xd51cu, 3u, 3u, 0u},
    {0xd51fu, 2u, 3u, 0u},
};

static const FixedSpanOpcode target_record_party_absent[] = {
    {0xd500u, 2u, 3u, 0u},
    {0xd502u, 4u, 6u, 2u},
    {0xd506u, 3u, 7u, 2u},
    {0xd509u, 3u, 5u, 2u},
    {0xd50cu, 2u, 2u, 0u},
    {0xd50eu, 3u, 3u, 0u},
    {0xd511u, 3u, 7u, 2u},
    {0xd514u, 2u, 3u, 0u},
    {0xd516u, 1u, 2u, 0u},
    {0xd517u, 1u, 2u, 0u},
    {0xd518u, 1u, 2u, 0u},
    {0xd519u, 1u, 2u, 0u},
    {0xd51au, 1u, 2u, 0u},
    {0xd51bu, 1u, 2u, 0u},
    {0xd51cu, 3u, 3u, 0u},
    {0xd51fu, 2u, 3u, 0u},
};

static const FixedSpanOpcode target_record_enemy_start[] = {
    {0xd521u, 2u, 2u, 0u},
    {0xd523u, 3u, 4u, 1u},
    {0xd526u, 1u, 2u, 0u},
    {0xd527u, 3u, 3u, 0u},
    {0xd52au, 1u, 2u, 0u},
    {0xd52bu, 1u, 3u, 1u},
    {0xd52cu, 1u, 4u, 2u},
    {0xd52du, 1u, 4u, 2u},
};

static const FixedSpanOpcode target_record_enemy_save[] = {
    {0xd52bu, 1u, 3u, 1u},
    {0xd52cu, 1u, 4u, 2u},
    {0xd52du, 1u, 4u, 2u},
};

static const FixedSpanOpcode target_record_enemy_finish[] = {
    {0xd531u, 1u, 5u, 2u},
    {0xd532u, 1u, 5u, 2u},
    {0xd533u, 2u, 2u, 0u},
    {0xd535u, 3u, 6u, 1u},
    {0xd538u, 2u, 3u, 1u},
    {0xd53au, 3u, 6u, 1u},
    {0xd53du, 2u, 3u, 1u},
    {0xd53fu, 3u, 6u, 1u},
    {0xd542u, 1u, 4u, 1u},
    {0xd543u, 1u, 2u, 0u},
    {0xd544u, 1u, 2u, 0u},
    {0xd545u, 1u, 2u, 0u},
    {0xd546u, 1u, 2u, 0u},
    {0xd547u, 1u, 2u, 0u},
    {0xd548u, 1u, 2u, 0u},
    {0xd549u, 2u, 2u, 0u},
    {0xd54bu, 2u, 2u, 0u},
};

static uint16_t TargetRecordRamWord(const CpuState *cpu, unsigned address) {
    const unsigned offset = (cpu->DB == 0x7fu ? 0x10000u : 0u) + address;
    return cpu->ram[offset] | (uint16_t)cpu->ram[offset + 1u] << 8;
}

static unsigned PlanTargetRecordStep(const CpuState *cpu, uint32_t entry,
    FixedSpanOpcode *plan, uint32_t *next) {
    unsigned count = 0u;
#define TARGET_OPCODES(rows) do { \
    memcpy(plan + count, rows, sizeof rows); \
    count += sizeof rows / sizeof rows[0]; \
} while (0)
    if (entry == 0x81d4e5u || entry == 0x81d4f2u) {
        unsigned left = cpu->A;
        if (entry == 0x81d4e5u) {
            TARGET_OPCODES(target_record_start);
            left = 12u;
        }
        for (unsigned record = 0u; record < 2u; ++record) {
            TARGET_OPCODES(target_record_clear);
            if (!--left) {
                plan[count - 1u].cycles = 2u;
                *next = 0x81d4fcu;
                return count;
            }
        }
        *next = 0x81d4f2u;
        return count;
    }
    if (entry == 0x81d4fcu || entry == 0x81d500u) {
        unsigned index = cpu->X;
        if (entry == 0x81d4fcu) {
            TARGET_OPCODES(target_record_party_start);
            index = 0u;
        }
        if (TargetRecordRamWord(cpu, 0xa64u + index)) {
            TARGET_OPCODES(target_record_party_present);
        } else {
            TARGET_OPCODES(target_record_party_absent);
        }
        if (index == 6u)
            plan[count - 1u].cycles = 2u;
        *next = index == 6u ? 0x81d521u : 0x81d500u;
        return count;
    }
    if (entry == 0x81d521u) {
        TARGET_OPCODES(target_record_enemy_start);
    } else if (entry == 0x81d52bu) {
        TARGET_OPCODES(target_record_enemy_save);
    } else {
        TARGET_OPCODES(target_record_enemy_finish);
        if (cpu->ram[cpu->S + 5u] == 5u) {
            *next = 0x81d54du;
            return count;
        }
        plan[count - 1u].cycles = 3u;
        TARGET_OPCODES(target_record_enemy_save);
    }
    *next = 0x81d52eu;
#undef TARGET_OPCODES
    return count;
}

static void ActorBridgeTargetRecordStep(CpuState *cpu, uint32_t entry) {
    const bool wide = entry == 0x81d4f2u || entry == 0x81d4fcu;
    const unsigned limit = entry == 0x81d531u ? 0x1ffau
                         : entry == 0x81d4e5u ? 0x1ffeu : 0x1fffu;
    const unsigned reserve = 768u;
    if (!TargetRecordStepHookOwned(entry, ActorBridgeTargetRecordStep) ||
        cpu != &g_cpu || !cpu->ram || cpu->PB != 0x81u || cpu->D || cpu->_flag_D ||
        cpu->x_flag || cpu->m_flag > 1u ||
        (entry != 0x81d500u && cpu->m_flag != !wide) ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        cpu->S < 0x1f00u || cpu->S > limit ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, limit, true))
        return;
    if (entry == 0x81d4f2u && (!cpu->A || cpu->A > 12u || cpu->X % 4u ||
            cpu->X / 4u + cpu->A != 12u))
        return;
    if (entry == 0x81d500u && (cpu->X > 6u || cpu->X % 2u || cpu->Y != cpu->X * 2u))
        return;
    if (entry == 0x81d531u) {
        const unsigned saved_y = cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8;
        const unsigned saved_x = cpu->ram[cpu->S + 3u] | (unsigned)cpu->ram[cpu->S + 4u] << 8;
        const unsigned saved_a = cpu->ram[cpu->S + 5u];
        if (saved_a >= 6u || saved_y != saved_a || saved_x != saved_a * 4u)
            return;
        if (saved_a != 5u &&
            !Lufia2DecompFrameRangeAllows(0x81d52bu, 0x81d52eu, TargetRecordStepHookOwned))
            return;
    }
    const uint32_t fence = entry <= 0x81d4f2u ? 0x81d4fcu
                         : entry <= 0x81d500u ? 0x81d521u
                         : entry <= 0x81d52bu ? 0x81d52eu : 0x81d54du;
    if (!Lufia2DecompFrameRangeAllows(entry, fence, TargetRecordStepHookOwned))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles)
        return;
    FixedSpanOpcode timing[LUFIA2_BATTLE_TARGET_RECORD_MAX_OPCODES];
    uint32_t next;
    const unsigned count = PlanTargetRecordStep(cpu, entry, timing, &next);
    if (Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeTargetRecordStep) < count)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned fetched = fence - entry < 32u ? fence - entry : 32u;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, fetched, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleTargetRecordStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
