#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lufia2/field.h"

static unsigned checks;
static const uint32_t guard_entries[] = {0x83acefu, 0x83aeedu, 0x83af54u};
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
static uint8_t guard_child(void *context, Lufia2CpuState *cpu, uint32_t target,
                           uint32_t site, uint8_t frame) {
    (void)context;
    (void)cpu;
    (void)site;
    (void)frame;
    fprintf(stderr, "guard child %06X\n", target);
    exit(1);
}
#endif
static int check_guard(unsigned which, Lufia2CpuState state, unsigned emulation) {
    const Lufia2Memory memory = {guard_read, guard_write, NULL, NULL, NULL};
#ifdef RUNTIME_ABI
    static const RuntimeBridge bridges[] = {
        Lufia2DecompBridge_83ACEF, Lufia2DecompBridge_83AEED, Lufia2DecompBridge_83AF54,
        Lufia2DecompBridge_83ACB7, Lufia2DecompBridge_83AD23};
    const uint32_t entry = which < 3u    ? guard_entries[which]
                           : which == 3u ? 0x83acb7u
                                         : 0x83ad23u;
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
        memcmp(&before, &cpu, sizeof cpu))
        return 1;
    runtime_case = previous;
#else
    (void)emulation;
    Lufia2CpuState before = state;
    before.resume_pc = guard_entries[which];
    Lufia2ExecutionResult result =
        which == 0u ? Lufia2FieldSelectSessionSetup(&memory, &state, guard_child, NULL)
        : which == 1u ? Lufia2FieldAdvancePaletteCycles(&memory, &state)
                      : Lufia2FieldStepWaveTable(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != guard_entries[which] ||
        memcmp(&before, &state, sizeof state))
        return 1;
#endif
    ++checks;
    return 0;
}
int main(void) {
    for (unsigned which = 0u; which < 3u; ++which) {
        Lufia2CpuState valid = {0};
        valid.program_bank = 0x83u;
        valid.data_bank = which == 0u ? 0x86u : 0x7fu;
        valid.accumulator_is_8_bit = 1u;
        valid.stack = 0x1f80u;
        valid.accumulator = 0x1234u;
        valid.x = 0x5678u;
        valid.y = 0x9abcu;
        valid.carry = valid.irq_disable = 1u;
        unsigned lower = which == 0u ? 0x1f10u : 0x1f00u;
        unsigned upper = which == 0u ? 0x1fffu : 0x1ffcu;
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
            if (bank != 0x83u) {
                state.program_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
            if (which && bank != 0x7fu) {
                state = valid;
                state.data_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        }
        for (unsigned flags = 0u; flags < 8u; ++flags)
            if (which == 0u ? flags >= 4u : flags != 1u) {
                Lufia2CpuState state = valid;
                state.accumulator_is_8_bit = flags & 1u;
                state.index_is_8_bit = (flags >> 1u) & 1u;
                state.decimal = (flags >> 2u) & 1u;
                if (check_guard(which, state, 0u))
                    return 1;
            }
#ifdef RUNTIME_ABI
        for (unsigned bank = 0u; bank < (which == 0u ? 256u : 1u); ++bank)
            for (unsigned width = 0u; width < (which == 0u ? 4u : 1u); ++width) {
                Lufia2CpuState state = valid;
                if (!which) {
                    state.data_bank = (uint8_t)bank;
                    state.accumulator_is_8_bit = width & 1u;
                    state.index_is_8_bit = (width >> 1u) & 1u;
                }
                if (check_guard(which, state, 1u))
                    return 1;
            }
        if (!which) {
            if (check_guard(3u, valid, 0u) || check_guard(4u, valid, 0u))
                return 1;
        }
#else
        if (!which) {
            const Lufia2Memory memory = {guard_read, guard_write, NULL, NULL, NULL};
            Lufia2CpuState before = valid;
            before.resume_pc = 0x83acefu;
            Lufia2ExecutionResult result =
                Lufia2FieldSelectSessionSetup(&memory, &valid, NULL, NULL);
            if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x83acefu ||
                memcmp(&before, &valid, sizeof valid))
                return 1;
        }
#endif
    }
    printf("PASS%u unpaired field-entry pre-access guards\n", checks);
    return 0;
}

int g_interp_apu_driving;
