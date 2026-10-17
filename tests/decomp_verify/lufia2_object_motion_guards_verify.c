#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lufia2/field.h"
#include "lufia2/actor.h"

static unsigned checks;
static const uint32_t guard_entries[] = {0x83e1adu, 0x83e1b9u, 0x83ee9du, 0x83eea9u,
                                         0x83eed8u, 0x83ed63u, 0x83ee0eu};
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
static int check_guard(unsigned which, Lufia2CpuState state, unsigned emulation) {
    const Lufia2Memory memory = {guard_read, guard_write, NULL, NULL, NULL};
#ifdef RUNTIME_ABI
    static const RuntimeBridge bridges[] = {
        Lufia2DecompBridge_83E1AD, Lufia2DecompBridge_83E1B9, Lufia2DecompBridge_83EE9D,
        Lufia2DecompBridge_83EEA9, Lufia2DecompBridge_83EED8, Lufia2DecompBridge_83ED63,
        Lufia2DecompBridge_83EE0E};
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
        Lufia2ObjectAddDisplayOffsetX, Lufia2ObjectAddDisplayOffsetY,
        Lufia2ObjectAddFinePositionX,  Lufia2ObjectAddFinePositionY,
        Lufia2ObjectReadRandomOffset,  Lufia2ObjectReadSubcommand,
        Lufia2ObjectCopyAnimationState};
    Lufia2ExecutionResult result = functions[which](&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != guard_entries[which] ||
        memcmp(&before, &state, sizeof state))
        return 1;
#endif
    ++checks;
    return 0;
}
int main(void) {
    for (unsigned which = 0u; which < 7u; ++which) {
        Lufia2CpuState valid = {0};
        valid.program_bank = 0x83u;
        valid.data_bank = which < 4u ? 0xffu : 0x7eu;
        valid.accumulator_is_8_bit = which >= 4u;
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
            if (which >= 4u && (bank & 0x7fu) >= 0x40u && bank != 0x7eu) {
                state = valid;
                state.data_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        }
        for (unsigned flags = 0u; flags < 8u; ++flags)
            if (flags >= 4u || (flags & 1u) != (unsigned)(which >= 4u)) {
                Lufia2CpuState state = valid;
                state.accumulator_is_8_bit = flags & 1u;
                state.index_is_8_bit = (flags >> 1u) & 1u;
                state.decimal = (flags >> 2u) & 1u;
                if (check_guard(which, state, 0u))
                    return 1;
            }
#ifdef RUNTIME_ABI
        for (unsigned bank = 0u; bank < 256u; ++bank) {
            if (which >= 4u && (bank & 0x7fu) >= 0x40u && bank != 0x7eu)
                continue;
            for (unsigned width = 0u; width < 2u; ++width) {
                Lufia2CpuState state = valid;
                state.data_bank = (uint8_t)bank;
                state.index_is_8_bit = width & 1u;
                if (check_guard(which, state, 1u))
                    return 1;
            }
        }
#endif
    }
    printf("PASS%u unpaired field-entry pre-access guards\n", checks);
    return 0;
}

int g_interp_apu_driving;
