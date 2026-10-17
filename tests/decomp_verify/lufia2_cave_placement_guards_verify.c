#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "lufia2/ancient_cave.h"

static unsigned checks;
static const uint32_t guard_entries[] = {0x8399c8u, 0x8399d3u, 0x839a1cu,
                                         0x839a6bu, 0x839a9fu, 0x839abcu,
                                         0x839e1bu, 0x839e11u, 0x839de4u};
static const unsigned work_banks[] = {0u, 1u, 1u, 1u, 1u, 1u, 0u, 0u, 0u};
static const unsigned allows_byte_index[] = {1u, 0u, 0u, 0u, 0u, 0u, 1u, 1u, 1u};
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
        Lufia2DecompBridge_8399C8, Lufia2DecompBridge_8399D3,
        Lufia2DecompBridge_839A1C, Lufia2DecompBridge_839A6B,
        Lufia2DecompBridge_839A9F, Lufia2DecompBridge_839ABC,
        Lufia2DecompBridge_839E1B, Lufia2DecompBridge_839E11,
        Lufia2DecompBridge_839DE4};
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
        Lufia2CaveResolveTilePosition,  Lufia2CaveProbeObjectPlacement,
        Lufia2CavePlaceRandomObject,    Lufia2CaveProbeNearbyChest,
        Lufia2CaveProbeChestAtPosition, Lufia2CaveTryPlaceRoomChest,
        Lufia2CaveDrawRandomBelow,      Lufia2CaveDrawRandomIndex,
        Lufia2CaveDrawRandomMean};
    Lufia2ExecutionResult result = functions[which](&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != guard_entries[which] ||
        memcmp(&before, &state, sizeof state))
        return 1;
#endif
    ++checks;
    return 0;
}
int main(void) {
    for (unsigned which = 0u; which < 9u; ++which) {
        Lufia2CpuState valid = {0};
        valid.program_bank = 0x83u;
        valid.data_bank = work_banks[which] ? 0x7fu : 0xffu;
        valid.accumulator_is_8_bit = 1u;
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
            if (work_banks[which] && bank != 0x7fu) {
                state = valid;
                state.data_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        }
        for (unsigned flags = 0u; flags < 8u; ++flags)
            if (flags >= 4u || !(flags & 1u) ||
                (!allows_byte_index[which] && (flags & 2u))) {
                Lufia2CpuState state = valid;
                state.accumulator_is_8_bit = flags & 1u;
                state.index_is_8_bit = (flags >> 1u) & 1u;
                state.decimal = (flags >> 2u) & 1u;
                if (check_guard(which, state, 0u))
                    return 1;
            }
#ifdef RUNTIME_ABI
        for (unsigned bank = 0u; bank < 256u; ++bank) {
            if (work_banks[which] && bank != 0x7fu)
                continue;
            for (unsigned width = 0u; width < (allows_byte_index[which] ? 2u : 1u);
                 ++width) {
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
