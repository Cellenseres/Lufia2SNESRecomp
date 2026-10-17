#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lufia2/field.h"
#include "lufia2/item.h"
#include "lufia2/battle.h"
#include "lufia2/actor.h"
#include "lufia2/world_map.h"
#include "lufia2/menu.h"
#include "lufia2/ancient_cave.h"

static unsigned checks;
static const uint32_t guard_entries[] = {0x81f114u, 0x81f15bu, 0x81ebf4u, 0x85948cu,
                                         0x85ce21u, 0x85cf68u, 0x85d025u, 0x85d94fu,
                                         0x85d9f0u, 0x85937du, 0x85cfabu, 0x85d080u};
static const unsigned width_modes[] = {0u, 1u, 2u, 0u, 2u, 0u, 0u, 0u, 0u, 0u, 3u, 0u};
static unsigned allowed_bank(unsigned w, unsigned b) {
    (void)w;
    (void)b;
    return 1u;
}
static uint8_t guard_read(void *context, uint32_t address) {
    (void)context;
    fprintf(stderr, "guard read %06X\n", address);
    exit(1);
}
static void guard_write(void *context, uint32_t address, uint8_t value) {
    (void)context;
    (void)value;
    fprintf(stderr, "guard write %06X\n", address);
    exit(1);
}
#ifndef RUNTIME_ABI
static uint8_t guard_child(void *x, Lufia2CpuState *n, uint32_t t, uint32_t s,
                           uint8_t f) {
    (void)x;
    (void)n;
    (void)t;
    (void)s;
    (void)f;
    exit(1);
}
static Lufia2ExecutionResult Guard1(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2InventoryConsumeRequestedItem(m, n, guard_child, NULL);
}
static Lufia2ExecutionResult Guard2(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2BattleResetTargetMotion(m, n, guard_child, NULL);
}
static Lufia2ExecutionResult Guard3(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2BattleRefreshInactiveEnemySprites(m, n, guard_child, NULL);
}
static Lufia2ExecutionResult Guard4(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2BattlePrepareAlternateAction(m, n, guard_child, NULL);
}
static Lufia2ExecutionResult Guard5(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2BattleBuildActionAdjustments(m, n, guard_child, NULL);
}
static Lufia2ExecutionResult Guard6(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2BattleCalculateActionBias(m, n, guard_child, NULL);
}
static Lufia2ExecutionResult Guard7(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2BattleStoreActionResult(m, n, guard_child, NULL);
}
#endif
static int check_guard(unsigned which, Lufia2CpuState state, unsigned emulation) {
    const Lufia2Memory memory = {guard_read, guard_write, NULL, NULL, NULL};
#ifdef RUNTIME_ABI
    static const RuntimeBridge bridges[] = {
        Lufia2DecompBridge_81F114, Lufia2DecompBridge_81F15B,
        Lufia2DecompBridge_81EBF4, Lufia2DecompBridge_85948C,
        Lufia2DecompBridge_85CE21, Lufia2DecompBridge_85CF68,
        Lufia2DecompBridge_85D025, Lufia2DecompBridge_85D94F,
        Lufia2DecompBridge_85D9F0, Lufia2DecompBridge_85937D,
        Lufia2DecompBridge_85CFAB, Lufia2DecompBridge_85D080};
    const uint32_t entry = guard_entries[which];
    RuntimeCase test = {0};
    test.memory = &memory;
    test.start = state.stack;
    test.entry = entry;
    test.guard_only = 1u;
    RuntimeCase *previous = runtime_case;
    runtime_case = &test;
    CpuState cpu = {0};
    runtime_load(&cpu, &state);
    cpu.emulation = (uint8_t)emulation;
    CpuState before = cpu;
    RecompReturn result = bridges[which](&cpu);
    if (result != RECOMP_RETURN_LLE_UNWIND_BASE || test.stub != 1u || test.kind != 3u ||
        test.target != entry || test.source != entry || test.restore != state.stack ||
        memcmp(&before, &cpu, sizeof cpu)) {
        fprintf(
            stderr,
            "guard entry%06X PB%02X DB%02X S%04X DP%04X M%uX%u E%u target%06X source%06X\n",
            entry, state.program_bank, state.data_bank, state.stack, state.direct_page,
            state.accumulator_is_8_bit, state.index_is_8_bit, emulation, test.target,
            test.source);
        return 1;
    }
    runtime_case = previous;
#else
    (void)emulation;
    Lufia2CpuState before = state;
    before.resume_pc = guard_entries[which];
    static Lufia2ExecutionResult (*const functions[])(const Lufia2Memory *,
                                                      Lufia2CpuState *) = {
        Lufia2InventoryRemoveQuantity,
        Guard1,
        Guard2,
        Guard3,
        Guard4,
        Guard5,
        Guard6,
        Guard7,
        Lufia2BattleInvalidateTurnQueueActor,
        Lufia2BattleRemoveTurnQueueActor,
        Lufia2BattleAccumulateActionAdjustment,
        Lufia2BattleReadActorBiasFlag};
    Lufia2ExecutionResult result = functions[which](&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != guard_entries[which] ||
        memcmp(&before, &state, sizeof state))
        return 1;
#endif
    ++checks;
    return 0;
}
int main(void) {
    for (unsigned which = 0u; which < 12u; ++which) {
        Lufia2CpuState valid = {0};
        valid.program_bank = (uint8_t)(guard_entries[which] >> 16);
        valid.data_bank = 0x7eu;
        valid.accumulator_is_8_bit = width_modes[which] == 3u ? 0u : 1u;
        valid.stack = 0x1f80u;
        valid.accumulator = 0x1234u;
        valid.x = 0x5678u;
        valid.y = 0x9abcu;
        valid.carry = valid.irq_disable = 1u;
        unsigned lower = 0x1f00u;
        unsigned upper = 0x1ffcu;
        for (unsigned stack = 0u; stack < 65536u; ++stack)
            if (stack < lower || stack > upper) {
                Lufia2CpuState state = valid;
                state.stack = (uint16_t)stack;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        for (unsigned page = 1u; page < 65536u; ++page) {
            Lufia2CpuState state = valid;
            state.direct_page = (uint16_t)page;
            if (check_guard(which, state, 0u))
                return 1;
        }
        for (unsigned bank = 0u; bank < 256u; ++bank) {
            Lufia2CpuState state = valid;
            if (bank != valid.program_bank) {
                state.program_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
            if (!allowed_bank(which, bank)) {
                state = valid;
                state.data_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        }
        for (unsigned flags = 0u; flags < 8u; ++flags)
            if (flags >= 4u ||
                (width_modes[which] != 2u &&
                 ((flags & 2u) || (width_modes[which] == 0u && !(flags & 1u)) ||
                  (width_modes[which] == 3u && (flags & 1u))))) {
                Lufia2CpuState state = valid;
                state.accumulator_is_8_bit = flags & 1u;
                state.index_is_8_bit = (flags >> 1u) & 1u;
                state.decimal = (flags >> 2u) & 1u;
                if (check_guard(which, state, 0u))
                    return 1;
            }
#ifdef RUNTIME_ABI
        for (unsigned bank = 0u; bank < 256u; ++bank) {
            if (!allowed_bank(which, bank))
                continue;
            for (unsigned width = 0u; width < (width_modes[which] == 2u   ? 4u
                                               : width_modes[which] == 1u ? 2u
                                                                          : 1u);
                 ++width) {
                Lufia2CpuState state = valid;
                state.data_bank = (uint8_t)bank;
                state.accumulator_is_8_bit = width_modes[which] == 3u ? 0u
                                             : !width_modes[which]    ? 1u
                                                                      : width & 1u;
                state.index_is_8_bit =
                    width_modes[which] == 2u ? (width >> 1) & 1u : 0u;
                if (check_guard(which, state, 1u))
                    return 1;
            }
        }
#endif
    }
#ifndef RUNTIME_ABI
    const Lufia2Memory memory = {guard_read, guard_write, NULL, NULL, NULL};
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x81u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x81f15bu;
        Lufia2ExecutionResult result =
            Lufia2InventoryConsumeRequestedItem(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x81f15bu ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x81u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x81ebf4u;
        Lufia2ExecutionResult result =
            Lufia2BattleResetTargetMotion(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x81ebf4u ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x85u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x85948cu;
        Lufia2ExecutionResult result =
            Lufia2BattleRefreshInactiveEnemySprites(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x85948cu ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x85u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x85ce21u;
        Lufia2ExecutionResult result =
            Lufia2BattlePrepareAlternateAction(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x85ce21u ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x85u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x85cf68u;
        Lufia2ExecutionResult result =
            Lufia2BattleBuildActionAdjustments(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x85cf68u ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x85u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x85d025u;
        Lufia2ExecutionResult result =
            Lufia2BattleCalculateActionBias(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x85d025u ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
    {
        Lufia2CpuState n = {0};
        n.program_bank = 0x85u;
        n.data_bank = 0x7eu;
        n.stack = 0x1f80u;
        n.accumulator_is_8_bit = 1u;
        Lufia2CpuState expected = n;
        expected.resume_pc = 0x85d94fu;
        Lufia2ExecutionResult result =
            Lufia2BattleStoreActionResult(&memory, &n, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x85d94fu ||
            memcmp(&expected, &n, sizeof n))
            return 1;
        ++checks;
    }
#endif
    printf("PASS%u unpaired field-entry pre-access guards\n", checks);
    return 0;
}

int g_interp_apu_driving;
