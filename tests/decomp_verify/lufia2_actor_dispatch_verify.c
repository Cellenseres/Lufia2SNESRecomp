#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interp816.h"
#include "lufia2/actor_frontend.h"
#include "snes_function_verify.h"

enum {
    LUFIA2_ROM_SIZE = 0x280000,
    PRIMARY_DISPATCH_PC = 0x83c83c,
    SECONDARY_DISPATCH_PC = 0x83d59a,
    TEST_STACK = 0x1ff0,
    TEST_SCRIPT = 0x3000,
    PRIMARY_CASES_PER_OPCODE = 8,
    SECONDARY_CASES_PER_OPCODE = 8,
    KNOWN_HANDLER_VARIANTS = 4,
    ACTION_CORE_VARIANTS = 16,
    MOVEMENT_HELPER_CASES = 1024,
    FIXED_ACTION_HANDLER_CASES = 128,
    OPERAND_ACTION_HANDLER_CASES = 1024,
    LEADER_RADIUS_CASES = 4096,
    LEADER_STEP_CASES = ACTION_CORE_VARIANTS * 3 * 16,
    RANDOM_SCALE_CASES = 4096,
    RANDOM_TIMER_CASES = 1024,
    GENERIC_HANDLER_CASES = 1024,
};

typedef struct NativeMemory {
    SnesVerifyBus *bus;
} NativeMemory;

static uint8_t NativeRead(void *opaque, uint32_t address) {
    NativeMemory *memory = (NativeMemory *)opaque;
    return SnesVerifyBusRead(memory->bus, address);
}

static void NativeWrite(void *opaque, uint32_t address, uint8_t value) {
    NativeMemory *memory = (NativeMemory *)opaque;
    SnesVerifyBusWrite(memory->bus, address, value);
}

static size_t LoRomOffset(uint32_t address) {
    const uint8_t bank = (uint8_t)(address >> 16);
    const uint16_t offset = (uint16_t)address;
    return ((size_t)(bank & 0x7fu) << 15) | (offset & 0x7fffu);
}

static uint8_t Rom8(const SnesVerifyBus *bus, uint32_t address) {
    const size_t offset = LoRomOffset(address);
    return offset < bus->rom_size ? bus->rom[offset] : 0xffu;
}

static uint16_t Rom16(const SnesVerifyBus *bus, uint32_t address) {
    const uint8_t lo = Rom8(bus, address);
    const uint32_t hi_address =
        (address & 0xff0000u) | (uint16_t)((uint16_t)address + 1u);
    return (uint16_t)(lo | ((uint16_t)Rom8(bus, hi_address) << 8));
}

static uint32_t PrimaryHandler(const SnesVerifyBus *bus, uint8_t opcode) {
    const uint8_t index = (uint8_t)(opcode << 1);
    const uint16_t target = Rom16(bus, 0x83d467u + index);
    return 0x830000u | target;
}

static uint32_t SecondaryHandler(const SnesVerifyBus *bus, uint8_t opcode) {
    const uint16_t first = Rom16(
        bus, 0x83df17u + (uint16_t)((opcode >> 4) * 2u));
    uint16_t table = 0;

    if (first == 0xd5d4u)
        table = 0xdf37u;
    else if (first == 0xd5e0u)
        table = 0xdf57u;
    else if (first == 0xd5ecu)
        table = 0xdf77u;
    else
        return 0x830000u | first;

    return 0x830000u |
           Rom16(bus, 0x830000u |
               (uint16_t)(table + ((opcode & 0x0fu) * 2u)));
}

static bool Poke16(SnesVerifyBus *bus, uint32_t address, uint16_t value) {
    return SnesVerifyBusPoke(bus, address, (uint8_t)value) &&
           SnesVerifyBusPoke(
               bus, address + 1u, (uint8_t)(value >> 8));
}

static void InitInterp(
    Interp816 *cpu,
    uint32_t pc,
    const Lufia2ActorFrontendCpu *input) {
    cpu->a = input->accumulator;
    cpu->x = input->x;
    cpu->y = input->y;
    cpu->sp = input->stack;
    cpu->pc = (uint16_t)pc;
    cpu->dp = input->direct_page;
    cpu->k = input->program_bank;
    cpu->db = input->data_bank;
    cpu->c = input->carry != 0;
    cpu->z = input->zero != 0;
    cpu->n = input->negative != 0;
    cpu->v = input->overflow != 0;
    cpu->d = input->decimal != 0;
    cpu->i = input->irq_disable != 0;
    cpu->mf = input->accumulator_is_8_bit != 0;
    cpu->xf = input->index_is_8_bit != 0;
    cpu->e = false;
    cpu->irqWanted = false;
    cpu->nmiWanted = false;
    cpu->waiting = false;
    cpu->stopped = false;
    interp816_set_brk_hook_enabled(cpu, false);
}

static bool SameState(
    const Lufia2ActorFrontendCpu *native, const Interp816 *reference) {
    return native->accumulator == reference->a &&
           native->x == reference->x &&
           native->y == reference->y &&
           native->stack == reference->sp &&
           native->direct_page == reference->dp &&
           native->data_bank == reference->db &&
           native->program_bank == reference->k &&
           native->carry == (uint8_t)reference->c &&
           native->zero == (uint8_t)reference->z &&
           native->negative == (uint8_t)reference->n &&
           native->overflow == (uint8_t)reference->v &&
           native->decimal == (uint8_t)reference->d &&
           native->irq_disable == (uint8_t)reference->i &&
           native->accumulator_is_8_bit == (uint8_t)reference->mf &&
           native->index_is_8_bit == (uint8_t)reference->xf;
}

static bool SeedPrimary(
    SnesVerifyBus *bus,
    unsigned variant,
    uint8_t opcode,
    Lufia2ActorFrontendCpu *input) {
    const uint8_t slot =
        (uint8_t)((variant * 7u + opcode) % 40u);
    const uint16_t record = (uint16_t)(slot * 3u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(input, 0, sizeof(*input));
    input->accumulator = (uint16_t)(0xa500u | opcode);
    input->x = slot;
    input->y = (uint16_t)(variant * 11u);
    input->stack = TEST_STACK;
    input->direct_page = dp;
    input->data_bank = 0x83;
    input->program_bank = 0x83;
    input->carry = variant & 1u;
    input->zero = (variant >> 1) & 1u;
    input->negative = (variant >> 2) & 1u;
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 1;

    return SnesVerifyBusPoke(
               bus, 0x830736u + slot, (uint8_t)(variant * 19u)) &&
           Poke16(bus, dp + 0x00abu, record) &&
           Poke16(bus, dp + 0x002au,
               (uint16_t)(0x5a00u | variant)) &&
           Poke16(bus, 0x7fe506u + record, TEST_SCRIPT) &&
           SnesVerifyBusPoke(
               bus, 0x7fe508u + record, 0x7eu) &&
           SnesVerifyBusPoke(
               bus, 0x7e0000u + TEST_SCRIPT, opcode);
}

static bool SeedSecondary(
    SnesVerifyBus *bus,
    unsigned variant,
    uint8_t opcode,
    Lufia2ActorFrontendCpu *input) {
    const uint8_t slot = (variant & 3u) == 0 ? 0u :
        (uint8_t)((variant * 5u + opcode) % 39u + 1u);
    const uint16_t record = (uint16_t)(slot * 3u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(input, 0, sizeof(*input));
    input->accumulator =
        (uint16_t)(0x3c00u | (opcode ^ 0x5au));
    input->x = slot;
    input->y = (uint16_t)(variant * 13u);
    input->stack = TEST_STACK;
    input->direct_page = dp;
    input->data_bank = 0x83;
    input->program_bank = 0x83;
    input->carry = variant & 1u;
    input->zero = (variant >> 1) & 1u;
    input->negative = (variant >> 2) & 1u;
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = (variant & 1u) ? 0u : 1u;

    return SnesVerifyBusPoke(bus, dp + 0x00a7u, slot) &&
           SnesVerifyBusPoke(bus, dp + 0x00a8u, 0) &&
           Poke16(bus, dp + 0x00abu, record) &&
           SnesVerifyBusPoke(bus, dp + 0x0047u,
               (variant & 2u) ? 0xc0u : 0x40u) &&
           SnesVerifyBusPoke(bus, dp + 0x004bu,
               (variant & 4u) ? 0xc0u : 0x80u) &&
           Poke16(bus, 0x7fe3eeu + record, TEST_SCRIPT) &&
           SnesVerifyBusPoke(
               bus, 0x7fe3f0u + record, 0x7eu) &&
           SnesVerifyBusPoke(
               bus, 0x7e0000u + TEST_SCRIPT, opcode);
}

static bool RunPrimaryCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t opcode,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorScriptDispatchResult result;
    uint32_t target;
    unsigned instructions = 0;

    if (!SeedPrimary(bus, variant, opcode, &input))
        return false;
    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetTrace(bus);
    result = Lufia2ActorPrimaryScriptDispatch(&memory, &native);
    target = PrimaryHandler(bus, opcode);
    if (result.opcode != opcode || result.handler_pc != target) {
        fprintf(stderr,
            "FAIL primary case %u: opcode=%02X handler=%06X/%06X\n",
            case_index, opcode, result.handler_pc, target);
        return false;
    }

    {
        uint8_t *native_wram =
            (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
        int stop;
        if (!native_wram)
            return false;
        memcpy(
            native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
        memcpy(
            bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
        InitInterp(reference, PRIMARY_DISPATCH_PC, &input);
        SnesVerifyBusResetTrace(bus);
        stop = SnesVerifyRunUntil(
            reference, &target, 1, 96, &instructions);
        if (stop != 0 ||
            !SameState(&native, reference) ||
            memcmp(
                native_wram, bus->wram,
                SNES_VERIFY_WRAM_SIZE) != 0) {
            fprintf(stderr,
                "FAIL primary case %u: opcode=%02X stop=%d "
                "insns=%u A=%04X/%04X X=%04X/%04X "
                "Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
                "M=%u/%u Xf=%u/%u\n",
                case_index, opcode, stop, instructions,
                native.accumulator, reference->a,
                native.x, reference->x,
                native.y, reference->y,
                native.stack, reference->sp,
                native.data_bank, reference->db,
                native.accumulator_is_8_bit, reference->mf,
                native.index_is_8_bit, reference->xf);
            free(native_wram);
            return false;
        }
        free(native_wram);
    }
    return true;
}

static bool RunSecondaryCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t opcode,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorScriptDispatchResult result;
    uint32_t target;
    unsigned instructions = 0;

    if (!SeedSecondary(bus, variant, opcode, &input))
        return false;
    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetTrace(bus);
    result =
        Lufia2ActorSecondaryScriptDispatch(&memory, &native);
    target = SecondaryHandler(bus, opcode);
    if (result.opcode != opcode || result.handler_pc != target) {
        fprintf(stderr,
            "FAIL secondary case %u: opcode=%02X "
            "handler=%06X/%06X\n",
            case_index, opcode, result.handler_pc, target);
        return false;
    }

    {
        uint8_t *native_wram =
            (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
        int stop;
        if (!native_wram)
            return false;
        memcpy(
            native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
        memcpy(
            bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
        InitInterp(reference, SECONDARY_DISPATCH_PC, &input);
        SnesVerifyBusResetTrace(bus);
        stop = SnesVerifyRunUntil(
            reference, &target, 1, 128, &instructions);
        if (stop != 0 ||
            !SameState(&native, reference) ||
            memcmp(
                native_wram, bus->wram,
                SNES_VERIFY_WRAM_SIZE) != 0) {
            fprintf(stderr,
                "FAIL secondary case %u: opcode=%02X stop=%d "
                "insns=%u A=%04X/%04X X=%04X/%04X "
                "Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
                "M=%u/%u Xf=%u/%u\n",
                case_index, opcode, stop, instructions,
                native.accumulator, reference->a,
                native.x, reference->x,
                native.y, reference->y,
                native.stack, reference->sp,
                native.data_bank, reference->db,
                native.accumulator_is_8_bit, reference->mf,
                native.index_is_8_bit, reference->xf);
            free(native_wram);
            return false;
        }
        free(native_wram);
    }
    return true;
}


static bool SeedKnownPrimaryHandler(
    SnesVerifyBus *bus,
    unsigned variant,
    uint16_t y,
    Lufia2ActorFrontendCpu *input) {
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(input, 0, sizeof(*input));
    input->accumulator =
        (uint16_t)(0x6d00u | ((variant * 53u) & 0xffu));
    input->x = (uint16_t)(variant * 17u);
    input->y = y;
    input->stack = TEST_STACK;
    input->direct_page = dp;
    input->data_bank = 0x7e;
    input->program_bank = 0x83;
    input->carry = variant & 1u;
    input->zero = (variant >> 1) & 1u;
    input->negative = (variant >> 1) & 1u;
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    return Poke16(
        bus, dp + 0x002au,
        (uint16_t)(0x4100u + variant * 7u));
}

static bool CompareKnownPrimaryBoundary(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    const Lufia2ActorFrontendCpu *input,
    uint32_t start_pc,
    uint32_t redispatch_pc,
    uint32_t stop_pc,
    uint32_t handler_pc,
    Lufia2ActorPrimaryScriptStepFlow expected_flow,
    uint8_t expected_opcode,
    unsigned case_index,
    const char *name) {
    Lufia2ActorFrontendCpu native = *input;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryScriptStepResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    int stop;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(bus);
    result = Lufia2ActorPrimaryScriptExecuteKnownHandler(
        &memory, &native, handler_pc);

    if (result.flow != expected_flow ||
        result.handler_pc != stop_pc ||
        (expected_flow == LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED &&
         result.opcode != expected_opcode)) {
        fprintf(stderr,
            "FAIL %s case %u: flow=%u/%u opcode=%02X/%02X "
            "target=%06X/%06X\n",
            name, case_index,
            (unsigned)result.flow, (unsigned)expected_flow,
            result.opcode, expected_opcode,
            result.handler_pc, stop_pc);
        return false;
    }

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, start_pc, input);
    if (redispatch_pc != 0) {
        unsigned to_redispatch = 0;
        stop = SnesVerifyRunUntil(
            reference, &redispatch_pc, 1, 200000, &to_redispatch);
        instructions += to_redispatch;
        if (stop != 0) {
            fprintf(stderr,
                "FAIL %s case %u: did not reach redispatch %06X "
                "stop=%d insns=%u\n",
                name, case_index, redispatch_pc, stop, instructions);
            free(native_wram);
            return false;
        }
    }
    {
        unsigned to_target = 0;
        stop = SnesVerifyRunUntil(
            reference, &stop_pc, 1, 200000, &to_target);
        instructions += to_target;
    }

    if (stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL %s case %u: stop=%d A=%04X/%04X "
            "X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X M=%u/%u Xf=%u/%u V=%u/%u\n",
            name, case_index, stop,
            native.accumulator, reference->a,
            native.x, reference->x,
            native.y, reference->y,
            native.stack, reference->sp,
            native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.overflow, reference->v);
        free(native_wram);
        return false;
    }

    free(native_wram);
    return true;
}

static bool RunCommitTailCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t value,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t record =
        (uint16_t)(((variant * 0x31u + value) & 0xffu) * 2u);
    const uint16_t cursor =
        (uint16_t)(0x2100u + ((unsigned)value << 4) + variant);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    if (!SeedKnownPrimaryHandler(bus, variant, cursor, &input) ||
        !Poke16(bus, dp + 0x00abu, record))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83c8c7u, 0, 0x83c8d2u, 0x83c8c7u,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2,
        0, case_index, "C8C7");
}

static bool RunJumpHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t next_cursor =
        (uint16_t)(0x5000u + ((unsigned)next_opcode << 3) + variant);
    const uint16_t operand =
        (uint16_t)(next_cursor - 0xa1d4u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint32_t stop_pc = PrimaryHandler(bus, next_opcode);

    if (!SeedKnownPrimaryHandler(
            bus, variant, TEST_SCRIPT, &input) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 1u),
            (uint8_t)operand) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 2u),
            (uint8_t)(operand >> 8)) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | next_cursor, next_opcode) ||
        !Poke16(bus, dp + 0x00a7u, (uint16_t)(variant + 1u)))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83d2b4u, 0x83c85au, stop_pc, 0x83d2b4u,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, "D2B4");
}

static bool RunMaskHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    uint32_t handler_pc,
    unsigned case_index,
    const char *name) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(((unsigned)next_opcode + variant * 11u) % 40u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint8_t initial_mask =
        (uint8_t)(0xa5u ^ next_opcode ^ (variant * 29u));
    const uint32_t stop_pc = PrimaryHandler(bus, next_opcode);

    if (!SeedKnownPrimaryHandler(
            bus, variant, TEST_SCRIPT, &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 1u),
            next_opcode) ||
        !SnesVerifyBusPoke(
            bus, 0x7fe57eu + slot, initial_mask))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, 0x83c85cu, stop_pc, handler_pc,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, name);
}


static bool RunJumpTailAliasCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t pointer =
        (uint16_t)(0x6200u |
            ((unsigned)next_opcode + variant * 17u));
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint32_t stop_pc = PrimaryHandler(bus, next_opcode);

    if (!SeedKnownPrimaryHandler(
            bus, variant, TEST_SCRIPT, &input) ||
        !SnesVerifyBusPoke(
            bus, dp + 0x002bu, (uint8_t)(pointer >> 8)) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | pointer, next_opcode))
        return false;

    input.accumulator =
        (uint16_t)(0xa500u | (uint8_t)pointer);

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83d2bdu, 0x83c85au, stop_pc, 0x83d2bdu,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, "D2BD");
}

static bool RunCoordinateHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    uint32_t handler_pc,
    uint16_t coordinate_base,
    unsigned case_index,
    const char *name) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(8u +
            (((unsigned)next_opcode + variant * 7u) % 20u));
    const uint8_t radius = (uint8_t)(2u + (variant & 1u));
    const bool take_jump = (variant & 2u) == 0;
    const uint8_t target_coordinate =
        (uint8_t)(0x60u + (next_opcode & 0x0fu));
    const uint8_t actor_coordinate =
        take_jump ? target_coordinate :
        (uint8_t)(target_coordinate - 0x30u);
    const uint16_t jump_cursor =
        (uint16_t)(0x6800u +
            ((unsigned)next_opcode << 3) + variant);
    const uint16_t jump_operand =
        (uint16_t)(jump_cursor - 0xa1d4u);
    const uint32_t stop_pc = PrimaryHandler(bus, next_opcode);
    const uint32_t redispatch_pc =
        take_jump ? 0x83c85au : 0x83c85cu;
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    if (!SeedKnownPrimaryHandler(
            bus, variant, TEST_SCRIPT, &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 1u),
            radius) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 2u),
            (uint8_t)jump_operand) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 3u),
            (uint8_t)(jump_operand >> 8)) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 4u),
            next_opcode) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | jump_cursor, next_opcode) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | coordinate_base,
            target_coordinate) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u |
                (uint16_t)(coordinate_base + slot),
            actor_coordinate))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, redispatch_pc, stop_pc, handler_pc,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, name);
}

static bool RunD14DCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t value,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(8u + ((unsigned)value % 24u));
    const uint16_t record =
        (uint16_t)(((unsigned)value % 40u) * 3u);
    const uint8_t indirect_index =
        (uint8_t)(0x40u + (value & 0x1fu));
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint32_t stop_pc = 0x83c8d2u;
    const Lufia2ActorPrimaryScriptStepFlow flow =
        LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2;

    if (!SeedKnownPrimaryHandler(
            bus, variant,
            (uint16_t)(TEST_SCRIPT + value), &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !Poke16(bus, dp + 0x00abu, record) ||
        !SnesVerifyBusPoke(
            bus, 0x7fe5a6u + slot, indirect_index))
        return false;

    if (variant == 0u) {
        if (!SnesVerifyBusPoke(bus, 0x7e09a1u, 0x01u) ||
            !SnesVerifyBusPoke(bus, 0x7e09a6u, 0x00u))
            return false;
    } else {
        if (!SnesVerifyBusPoke(bus, 0x7e09a1u, 0x80u) ||
            !SnesVerifyBusPoke(
                bus, 0x7e09a6u,
                variant == 1u ? 0x01u : 0x00u))
            return false;
    }

    if (!SnesVerifyBusPoke(
            bus, 0x7e09a1u + indirect_index,
            variant == 2u ? 0x80u : 0x01u))
        return false;

    if (variant == 3u) {
        /*
         * Force D350 through its complete secondary-script install path.
         * This keeps D14D's child case deterministic while still verifying
         * the real JSL/RTL stack boundary and D16A..D173 post-call tail.
         */
        if (!SnesVerifyBusPoke(bus, 0x7e0622u + slot, 0x08u) ||
            !SnesVerifyBusPoke(bus, 0x7fe4deu, 0x5au))
            return false;
    }

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83d14du, 0, stop_pc, 0x83d14du,
        flow, 0, case_index, "D14D");
}


static bool SeedActionCoreCase(
    SnesVerifyBus *bus,
    unsigned variant,
    uint8_t action,
    Lufia2ActorFrontendCpu *input) {
    static const uint8_t probe_values[4] = {0x00u, 0x01u, 0x07u, 0x02u};
    const uint16_t dp = (variant & 2u) ? 0x0020u : 0;
    const uint8_t slot =
        (uint8_t)(8u + ((action + variant * 3u) % 24u));
    const uint16_t record = (uint16_t)(slot * 3u);
    const unsigned mode = (variant >> 2) & 3u;
    uint8_t x_coordinate = 0x30u;
    uint8_t y_coordinate = 0x40u;
    uint8_t flags = 0;

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(input, 0, sizeof(*input));

    input->accumulator =
        (uint16_t)(0xa500u | action);
    input->x = (uint16_t)(0x1200u | slot);
    input->y = (uint16_t)(0x4300u | variant);
    input->stack = TEST_STACK;
    input->direct_page = dp;
    input->data_bank = 0x7e;
    input->program_bank = 0x83;
    input->carry = variant & 1u;
    input->zero = (variant >> 1) & 1u;
    input->negative = (variant >> 2) & 1u;
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit =
        (mode == 1u && action < 4u) ? 0u : (uint8_t)(variant & 1u);
    if (input->index_is_8_bit) {
        input->x &= 0x00ffu;
        input->y &= 0x00ffu;
    }

    if (mode == 2u)
        flags = 0x08u;
    else if (mode == 3u)
        flags = 0x20u;

    if (action < 4u && mode < 2u) {
        const bool carry_path = mode == 1u;
        switch (action) {
        case 0:
            y_coordinate = carry_path ? 5u : 0u;
            if (!SnesVerifyBusPoke(
                    bus, 0x7fe61eu + slot,
                    carry_path ? 1u : 0u))
                return false;
            break;
        case 1:
            y_coordinate = 5u;
            if (!SnesVerifyBusPoke(
                    bus, 0x7fe66eu + slot,
                    carry_path ? 10u : 3u))
                return false;
            break;
        case 2:
            x_coordinate = carry_path ? 5u : 0u;
            if (!SnesVerifyBusPoke(
                    bus, 0x7fe5f6u + slot,
                    carry_path ? 1u : 0u))
                return false;
            break;
        case 3:
            x_coordinate = 5u;
            if (!SnesVerifyBusPoke(
                    bus, 0x7fe646u + slot,
                    carry_path ? 10u : 3u))
                return false;
            break;
        default:
            break;
        }
    }

    if (!Poke16(bus, dp + 0x00a7u, slot) ||
        !Poke16(bus, dp + 0x00abu, record) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0736u + slot,
            (uint8_t)(0xf0u | (variant & 0x0fu))) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0622u + slot, flags) ||
        !SnesVerifyBusPoke(
            bus, 0x7e06bau + slot, x_coordinate) ||
        !SnesVerifyBusPoke(
            bus, 0x7e06e2u + slot, y_coordinate) ||
        !Poke16(
            bus, 0x7fe3eeu + record,
            (uint16_t)(0x1111u + variant)) ||
        !SnesVerifyBusPoke(
            bus, 0x7fe3f0u + record, 0x55u))
        return false;

    if (action < 4u && mode == 1u) {
        const uint8_t direction = Rom8(bus, 0x83c1b0u + action);
        const uint8_t width = 8u;
        const uint16_t tile = (uint16_t)(0x0030u + (variant & 3u));
        const uint16_t attribute_base = 0x1000u;
        uint8_t probe_x = x_coordinate;
        uint8_t probe_y = y_coordinate;
        uint16_t cell_offset;

        switch (direction) {
        case 0:
            ++probe_y;
            break;
        case 2:
            --probe_x;
            break;
        case 4:
            --probe_y;
            break;
        case 6:
            ++probe_x;
            break;
        default:
            return false;
        }

        cell_offset =
            (uint16_t)(2u * ((uint16_t)probe_x +
                (uint16_t)probe_y * width));

        if (!SnesVerifyBusPoke(bus, 0x0005b9u, width) ||
            !Poke16(bus, 0x0005aau, 0) ||
            !Poke16(bus, 0x7fd008u, 0) ||
            !Poke16(bus, 0x7f0000u + cell_offset, tile) ||
            !Poke16(bus, 0x7fd03eu, attribute_base) ||
            !SnesVerifyBusPoke(
                bus, 0x7f0000u + attribute_base + tile,
                probe_values[variant & 3u]))
            return false;
    }

    return true;
}

static bool RunActionCoreCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t action,
    unsigned case_index) {
    const uint32_t stop_pc = 0x83d3aeu;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryActionFlow flow;
    uint8_t *native_wram;
    unsigned instructions = 0;
    int stop;

    if (!SeedActionCoreCase(bus, variant, action, &input))
        return false;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetTrace(bus);
    flow = Lufia2ActorPrimaryActionCore(&memory, &native);
    if (flow != LUFIA2_ACTOR_PRIMARY_ACTION_RETURN_D3AE) {
        fprintf(stderr,
            "FAIL D350 case %u: action=%02X unexpected flow=%u\n",
            case_index, action, (unsigned)flow);
        return false;
    }

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83d350u, &input);
    SnesVerifyBusResetTrace(bus);
    stop = SnesVerifyRunUntil(
        reference, &stop_pc, 1, 512, &instructions);

    if (stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL D350 case %u: action=%02X variant=%u "
            "flow=%u stop=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X DB=%02X/%02X M=%u/%u Xf=%u/%u\n",
            case_index, action, variant, (unsigned)flow,
            stop, instructions,
            native.accumulator, reference->a,
            native.x, reference->x,
            native.y, reference->y,
            native.stack, reference->sp,
            native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf);
        free(native_wram);
        return false;
    }

    free(native_wram);
    return true;
}

static bool SeedMapProbeCase(
    SnesVerifyBus *bus,
    unsigned case_index,
    Lufia2ActorFrontendCpu *input,
    uint8_t *value_out) {
    const uint16_t dp = (case_index & 1u) ? 0x0020u : 0;
    const uint8_t x_coordinate =
        (uint8_t)((case_index * 5u + 3u) & 0x1fu);
    const uint8_t y_coordinate =
        (uint8_t)((case_index * 7u + 1u) & 0x1fu);
    const uint8_t width =
        (uint8_t)(16u + ((case_index >> 5) & 0x0fu));
    /*
     * Keep the synthetic layer index inside the normal small table window.
     * 0x36 aliases $7F:D03E ($7F:D008 + 0x36), which is also the
     * attribute-base pointer used later by FB71. That alias is legal memory
     * behavior but makes the independently precomputed expected value depend
     * on write order instead of testing the map-probe semantics.
     */
    const uint16_t layer_index =
        (uint16_t)((case_index * 2u) & 0x001eu);
    const uint16_t base_offset =
        (uint16_t)(0x0800u + ((case_index & 0x0fu) * 0x20u));
    const uint16_t cell_offset =
        (uint16_t)(2u * ((uint16_t)x_coordinate +
            (uint16_t)y_coordinate * width));
    const uint16_t resolved_offset =
        (uint16_t)(cell_offset + base_offset);
    const uint16_t tile =
        (uint16_t)((case_index * 13u + 0x21u) & 0x03ffu);
    const uint16_t attribute_base = 0x3000u;
    const uint8_t value =
        (uint8_t)(case_index ^ (case_index >> 4) ^ 0xa5u);

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(input, 0, sizeof(*input));

    input->accumulator =
        (uint16_t)(0xb600u | (case_index & 0xffu));
    input->x = (uint16_t)(0x2200u | (case_index & 0xffu));
    input->y = (uint16_t)(0x3300u | (case_index & 0xffu));
    input->stack = TEST_STACK;
    input->direct_page = dp;
    input->data_bank = 0x7e;
    input->program_bank = 0x83;
    input->carry = case_index & 1u;
    input->zero = (case_index >> 1) & 1u;
    input->negative = (case_index >> 2) & 1u;
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;

    *value_out = value;

    return SnesVerifyBusPoke(
               bus, dp + 0x008fu, x_coordinate) &&
           SnesVerifyBusPoke(
               bus, dp + 0x0091u, y_coordinate) &&
           SnesVerifyBusPoke(bus, 0x0005b9u, width) &&
           Poke16(bus, 0x0005aau, layer_index) &&
           Poke16(bus, 0x7fd008u + layer_index, base_offset) &&
           Poke16(bus, 0x7f0000u + resolved_offset, tile) &&
           Poke16(bus, 0x7fd03eu, attribute_base) &&
           SnesVerifyBusPoke(
               bus, 0x7f0000u + attribute_base + tile, value);
}

static bool RunMovementStepCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index) {
    static const uint8_t direction[4] = {0u, 2u, 4u, 6u};
    static const uint32_t stops[4] = {
        0x83fb24u, 0x83fb27u, 0x83fb2au, 0x83fb2du};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    uint8_t *native_wram;
    uint32_t target;
    unsigned instructions = 0;
    int stop;

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(&input, 0, sizeof(input));
    input.accumulator =
        (uint16_t)(0xc500u | direction[case_index & 3u]);
    input.x = (uint16_t)(0x3400u | (case_index & 0xffu));
    input.y = (uint16_t)(0x4500u | (case_index & 0xffu));
    input.stack = TEST_STACK;
    input.direct_page = (case_index & 4u) ? 0x0020u : 0;
    input.data_bank = 0x7e;
    input.program_bank = 0x83;
    input.carry = case_index & 1u;
    input.zero = (case_index >> 1) & 1u;
    input.negative = (case_index >> 2) & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = (case_index >> 3) & 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    if (!SnesVerifyBusPoke(
            bus, input.direct_page + 0x008fu,
            (uint8_t)(0x40u + (case_index & 0x1fu))) ||
        !SnesVerifyBusPoke(
            bus, input.direct_page + 0x0091u,
            (uint8_t)(0x60u + ((case_index >> 2) & 0x1fu))))
        return false;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    target = Lufia2ActorMovementStep(&memory, &native);
    if (target == 0)
        return false;

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83fb12u, &input);
    stop = SnesVerifyRunUntil(
        reference, stops, 4, 64, &instructions);

    if (stop < 0 || stops[stop] != target ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL FB12 case %u: target=%06X stop=%d "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X\n",
            case_index, target, stop,
            native.accumulator, reference->a,
            native.x, reference->x,
            native.y, reference->y,
            native.stack, reference->sp);
        free(native_wram);
        return false;
    }

    free(native_wram);
    return true;
}

static bool RunMapOffsetCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index) {
    const uint32_t stop_pc = 0x83f9edu;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    uint8_t ignored;
    uint8_t *native_wram;
    unsigned instructions = 0;
    int stop;

    if (!SeedMapProbeCase(bus, case_index, &input, &ignored))
        return false;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    Lufia2ActorResolveMapCellOffset(&memory, &native);

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83f9d4u, &input);
    stop = SnesVerifyRunUntil(
        reference, &stop_pc, 1, 128, &instructions);

    if (stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL F9D4 case %u: stop=%d "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X M=%u/%u Xf=%u/%u\n",
            case_index, stop,
            native.accumulator, reference->a,
            native.x, reference->x,
            native.y, reference->y,
            native.stack, reference->sp,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf);
        free(native_wram);
        return false;
    }

    free(native_wram);
    return true;
}

static bool RunMapValueCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index) {
    const uint32_t stop_pc = 0x83fb8au;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    uint8_t expected_value;
    uint8_t *native_wram;
    unsigned instructions = 0;
    int stop;

    if (!SeedMapProbeCase(
            bus, case_index, &input, &expected_value))
        return false;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    Lufia2ActorReadMapCellValue(&memory, &native);
    if ((uint8_t)native.accumulator != expected_value) {
        fprintf(stderr,
            "FAIL FB71 case %u: result=%02X expected=%02X\n",
            case_index, (uint8_t)native.accumulator, expected_value);
        return false;
    }

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83fb71u, &input);
    stop = SnesVerifyRunUntil(
        reference, &stop_pc, 1, 192, &instructions);

    if (stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL FB71 case %u: stop=%d "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X M=%u/%u Xf=%u/%u\n",
            case_index, stop,
            native.accumulator, reference->a,
            native.x, reference->x,
            native.y, reference->y,
            native.stack, reference->sp,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf);
        free(native_wram);
        return false;
    }

    free(native_wram);
    return true;
}


static bool RunFixedActionHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index) {
    static const uint32_t handlers[8] = {
        0x83c867u, 0x83c86bu, 0x83c86fu, 0x83c873u,
        0x83c87cu, 0x83c880u, 0x83c884u, 0x83c888u,
    };
    static const uint8_t actions[8] = {
        0x00u, 0x01u, 0x02u, 0x03u,
        0x81u, 0x82u, 0x83u, 0x84u,
    };
    const unsigned handler_index =
        case_index / ACTION_CORE_VARIANTS;
    const unsigned variant =
        case_index % ACTION_CORE_VARIANTS;
    const uint32_t handler_pc = handlers[handler_index];
    const uint8_t action = actions[handler_index];
    Lufia2ActorFrontendCpu input;
    const uint32_t stop_pc = 0x83c8d2u;

    if (!SeedActionCoreCase(bus, variant, action, &input))
        return false;

    input.data_bank = 0x7eu;
    input.program_bank = 0x83u;
    input.index_is_8_bit = 0;
    input.x = (uint16_t)(0x1200u | ((variant * 3u + action) & 0xffu));
    input.y = (uint16_t)(TEST_SCRIPT + (variant * 3u));

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, 0, stop_pc, handler_pc,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2,
        0, case_index, "C867-action");
}

static bool RunOperandActionHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index) {
    const uint8_t action = (uint8_t)(case_index >> 2);
    const unsigned variant = case_index & 3u;
    Lufia2ActorFrontendCpu input;
    const uint32_t stop_pc = 0x83c8d2u;

    if (!SeedActionCoreCase(bus, variant, action, &input))
        return false;

    input.data_bank = 0x7eu;
    input.program_bank = 0x83u;
    input.index_is_8_bit = 0;
    input.x = (uint16_t)(0x1400u | ((case_index * 5u) & 0xffu));
    input.y = TEST_SCRIPT;

    if (!SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 1u),
            action))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83c877u, 0, stop_pc, 0x83c877u,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2,
        0, case_index, "C877-action");
}

static bool RunInstallScriptHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t operand,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot = (uint16_t)(8u + ((unsigned)operand % 24u));
    const uint16_t record =
        (uint16_t)((((unsigned)operand + variant * 7u) % 40u) * 3u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    if (!SeedKnownPrimaryHandler(
            bus, variant, (uint16_t)(TEST_SCRIPT + variant), &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !Poke16(bus, dp + 0x00abu, record) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + variant + 1u),
            operand) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0622u + slot,
            (uint8_t)(operand ^ (variant * 0x21u))))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83c891u, 0, 0x83c8d2u, 0x83c891u,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2,
        0, case_index, "C891");
}

static bool RunTimerStoreHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(((unsigned)next_opcode + variant * 13u) % 40u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint8_t operand =
        (uint8_t)(0x3cu ^ next_opcode ^ (variant * 0x45u));
    /* Variant 3 crosses from bank $7E into $7F. */
    const uint16_t cursor =
        variant == 3u ? 0xffffu : (uint16_t)(TEST_SCRIPT + variant);
    const uint32_t stop_pc = PrimaryHandler(bus, next_opcode);

    if (!SeedKnownPrimaryHandler(bus, variant, cursor, &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !SnesVerifyBusPoke(bus, 0x7e0000u + cursor + 1u, operand) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(cursor + 2u), next_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7fe4deu + slot, (uint8_t)~operand))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83c8eeu, 0x83c85cu, stop_pc, 0x83c8eeu,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, "C8EE");
}

static bool RunFlagBitHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    uint32_t handler_pc,
    unsigned case_index,
    const char *name) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(((unsigned)next_opcode + variant * 11u) % 40u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint8_t flags =
        (uint8_t)(next_opcode ^ (variant * 0x5bu));
    const uint32_t stop_pc = PrimaryHandler(bus, next_opcode);

    if (!SeedKnownPrimaryHandler(
            bus, variant, (uint16_t)(TEST_SCRIPT + variant), &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + variant + 1u),
            next_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7e0736u + slot, flags))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, 0x83c85cu, stop_pc, handler_pc,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, name);
}

static bool RunMapFlagHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t cell_flags,
    unsigned case_index) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(8u + (((unsigned)cell_flags + variant) % 24u));
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint8_t x_coordinate =
        (uint8_t)((cell_flags * 3u + variant) & 0x1fu);
    const uint8_t y_coordinate =
        (uint8_t)((cell_flags * 5u + (variant << 2)) & 0x1fu);
    const uint8_t width = (uint8_t)(0x20u + (variant << 3));
    const uint16_t layer_index = (uint16_t)((variant & 2u) * 2u);
    const uint16_t base_offset = (uint16_t)(0x0400u + variant * 0x40u);
    const uint16_t cell_offset =
        (uint16_t)(2u * ((uint16_t)x_coordinate +
            (uint16_t)y_coordinate * width) + base_offset);
    const uint8_t next_opcode =
        (uint8_t)(cell_flags ^ (variant * 0x35u));
    const bool flagged = (cell_flags & 0x30u) == 0x30u;
    const uint32_t stop_pc =
        flagged ? 0x83c8d2u : PrimaryHandler(bus, next_opcode);

    if (!SeedKnownPrimaryHandler(
            bus, variant, (uint16_t)(TEST_SCRIPT + variant), &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + variant + 1u),
            next_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7e06bau + slot, x_coordinate) ||
        !SnesVerifyBusPoke(bus, 0x7e06e2u + slot, y_coordinate) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0736u + slot, (uint8_t)(cell_flags * 7u)) ||
        !SnesVerifyBusPoke(bus, 0x0005b9u, width) ||
        !Poke16(bus, 0x0005aau, layer_index) ||
        !Poke16(bus, 0x7fd008u + layer_index, base_offset) ||
        !Poke16(bus, 0x7f0000u + cell_offset,
            (uint16_t)(0x0100u * cell_flags + variant)))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83cbb7u, flagged ? 0 : 0x83c85cu, stop_pc, 0x83cbb7u,
        flagged ? LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2 :
                  LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        flagged ? 0 : next_opcode, case_index, "CBB7");
}

static bool LeaderAxisWithinRadius(
    uint8_t actor, uint8_t leader, uint8_t radius) {
    const uint8_t low = (uint8_t)(actor - radius - 1u);
    const uint8_t high =
        (uint8_t)(low + (uint8_t)(radius << 1) + 1u);

    return low < leader && high >= leader;
}

static bool RunLeaderRadiusHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index) {
    static const uint8_t radii[4] = {0u, 1u, 3u, 0x90u};
    static const uint8_t actor_bases[4] = {0x02u, 0x40u, 0xfdu, 0x80u};
    const unsigned variant = case_index & 3u;
    const uint8_t radius = radii[(case_index >> 8) & 3u];
    const uint8_t actor_base = actor_bases[(case_index >> 10) & 3u];
    const uint8_t actor_x =
        (uint8_t)(actor_base + ((case_index >> 2) & 1u));
    const uint8_t actor_y =
        (uint8_t)(actor_base - ((case_index >> 3) & 1u));
    const uint8_t leader_x =
        (uint8_t)(actor_x + ((case_index >> 4) & 7u) - 4u);
    const uint8_t leader_y =
        (uint8_t)(actor_y + (((case_index >> 7) & 1u) ? 5u : 0u) +
            ((case_index >> 5) & 3u) - 2u);
    const uint16_t slot = (uint16_t)(8u + (case_index % 24u));
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint16_t script = (uint16_t)(TEST_SCRIPT + variant);
    const uint8_t skip_opcode = (uint8_t)(case_index * 37u);
    const uint8_t jump_opcode = (uint8_t)(skip_opcode ^ 0x80u);
    const uint16_t jump_cursor =
        (uint16_t)(0x6800u + ((case_index & 0xffu) << 3));
    const uint16_t jump_operand = (uint16_t)(jump_cursor - 0xa1d4u);
    const bool within =
        LeaderAxisWithinRadius(actor_x, leader_x, radius) &&
        LeaderAxisWithinRadius(actor_y, leader_y, radius);
    const uint8_t next_opcode = within ? skip_opcode : jump_opcode;
    Lufia2ActorFrontendCpu input;

    if (!SeedKnownPrimaryHandler(bus, variant, script, &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(script + 1u), radius) ||
        !Poke16(bus, 0x7e0000u | (uint16_t)(script + 2u), jump_operand) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(script + 4u), skip_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7e0000u | jump_cursor, jump_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7e06bau, leader_x) ||
        !SnesVerifyBusPoke(bus, 0x7e06e2u, leader_y) ||
        !SnesVerifyBusPoke(bus, 0x7e06bau + slot, actor_x) ||
        !SnesVerifyBusPoke(bus, 0x7e06e2u + slot, actor_y))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        0x83cbe1u, within ? 0x83c85cu : 0x83c85au,
        PrimaryHandler(bus, next_opcode), 0x83cbe1u,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, "CBE1");
}

static bool RunLeaderEqualHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned variant,
    uint8_t next_opcode,
    uint32_t handler_pc,
    uint16_t coordinate_base,
    unsigned case_index,
    const char *name) {
    Lufia2ActorFrontendCpu input;
    const uint16_t slot =
        (uint16_t)(8u + (((unsigned)next_opcode + variant * 7u) % 20u));
    const bool equal = (variant & 2u) == 0;
    const uint8_t leader = (uint8_t)(0x50u + (next_opcode & 0x1fu));
    const uint8_t actor =
        equal ? leader : (uint8_t)(leader + 1u + (next_opcode >> 5));
    const uint16_t jump_cursor =
        (uint16_t)(0x6800u + ((unsigned)next_opcode << 3) + variant);
    const uint16_t jump_operand = (uint16_t)(jump_cursor - 0xa1d4u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;

    if (!SeedKnownPrimaryHandler(bus, variant, TEST_SCRIPT, &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !Poke16(bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 1u),
            jump_operand) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(TEST_SCRIPT + 3u), next_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7e0000u | jump_cursor, next_opcode) ||
        !SnesVerifyBusPoke(bus, 0x7e0000u | coordinate_base, leader) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(coordinate_base + slot), actor))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, equal ? 0x83c85au : 0x83c85cu,
        PrimaryHandler(bus, next_opcode), handler_pc,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, name);
}

static bool RunLeaderStepHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    bool x_axis) {
    const unsigned variant = case_index % ACTION_CORE_VARIANTS;
    const unsigned outcome = (case_index / ACTION_CORE_VARIANTS) % 3u;
    const uint8_t next_opcode =
        (uint8_t)((case_index / (ACTION_CORE_VARIANTS * 3u)) * 0x25u +
            variant);
    const uint8_t ahead_action = x_axis ? 0x02u : 0x00u;
    const uint8_t behind_action = x_axis ? 0x03u : 0x01u;
    const uint16_t axis_base = x_axis ? 0x06bau : 0x06e2u;
    const uint32_t handler_pc = x_axis ? 0x83cc41u : 0x83cc63u;
    uint8_t action = outcome == 1u ? behind_action : ahead_action;
    Lufia2ActorFrontendCpu input;
    uint8_t actor;
    uint8_t leader;

    if (!SeedActionCoreCase(bus, variant, action, &input))
        return false;

    /* Same slot as SeedActionCoreCase. */
    actor = bus->wram[(uint16_t)(axis_base + 8u +
        ((action + variant * 3u) % 24u))];
    if (outcome == 0u && actor != 0)
        leader = (uint8_t)(actor - 1u);
    else if (outcome == 1u)
        leader = (uint8_t)(actor + 1u);
    else
        leader = actor;

    input.data_bank = 0x7eu;
    input.program_bank = 0x83u;
    input.index_is_8_bit = 0;
    input.x = (uint16_t)(0x1500u | ((case_index * 3u) & 0xffu));
    input.y = (uint16_t)(TEST_SCRIPT + variant * 3u);

    if (!SnesVerifyBusPoke(bus, 0x7e0000u | axis_base, leader) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(input.y + 1u), next_opcode))
        return false;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, 0x83c85cu, PrimaryHandler(bus, next_opcode),
        handler_pc, LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
        next_opcode, case_index, x_axis ? "CC41" : "CC63");
}

enum {
    RANDOM_TABLE = 0x0521,
    RANDOM_INDEX = 0x0559,
    RANDOM_TABLE_SIZE = 0x37,
};

static void SeedRandomTable(
    SnesVerifyBus *bus, unsigned seed, uint8_t index) {
    for (unsigned i = 0; i < RANDOM_TABLE_SIZE; ++i)
        bus->wram[RANDOM_TABLE + i] =
            (uint8_t)((seed * 0x9du) ^ (i * 0x3bu) ^ (seed >> 3));
    bus->wram[RANDOM_INDEX] = index;
}

/* Independent model of $80:8299/$80:82C7 and $80:832D. */
static uint16_t ExpectedRandom(
    const uint8_t *wram, uint16_t accumulator, bool byte_only) {
    uint8_t table[RANDOM_TABLE_SIZE];
    uint8_t index = (uint8_t)(wram[RANDOM_INDEX] + 1u);

    memcpy(table, wram + RANDOM_TABLE, sizeof(table));
    if (index >= RANDOM_TABLE_SIZE) {
        for (unsigned i = 0; i < 24u; ++i)
            table[i] ^= table[i + 31u];
        for (unsigned i = 24u; i < RANDOM_TABLE_SIZE; ++i)
            table[i] ^= table[i - 24u];
        index = 0;
    }
    if (byte_only)
        return (uint16_t)((accumulator & 0xff00u) | table[index]);
    return (uint16_t)(((unsigned)(uint8_t)accumulator * table[index]) >> 8);
}

static bool RunRandomCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    bool byte_only) {
    static const uint8_t banks[4] = {0x7eu, 0x83u, 0x00u, 0x80u};
    const uint32_t stop_pc = byte_only ? 0x8082e6u : 0x8082c6u;
    const char *name = byte_only ? "82C7" : "8299";
    const uint8_t index_seed = (uint8_t)(case_index & 0x3fu);
    const uint8_t index = index_seed == 0x3fu ? 0xffu : index_seed;
    const unsigned flags = (case_index >> 8) & 0x0fu;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    uint16_t expected;
    uint8_t *native_wram;
    unsigned instructions = 0;
    int stop;

    memset(bus->wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)(case_index * 0x9e37u + 0x1234u);
    input.x = (uint16_t)(0x5a00u | ((case_index * 7u) & 0xffu));
    input.y = (uint16_t)(0xa500u | ((case_index * 13u) & 0xffu));
    input.stack = TEST_STACK;
    input.direct_page = (case_index & 0x10u) ? 0x0020u : 0;
    input.data_bank = banks[(case_index >> 4) & 3u];
    input.program_bank = 0x80u;
    input.carry = flags & 1u;
    input.overflow = (flags >> 1) & 1u;
    input.decimal = (flags >> 2) & 1u;
    input.irq_disable = (flags >> 3) & 1u;
    input.zero = (case_index >> 5) & 1u;
    input.negative = (case_index >> 3) & 1u;
    input.accumulator_is_8_bit = (case_index >> 6) & 1u;
    input.index_is_8_bit = (case_index >> 7) & 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }
    SeedRandomTable(bus, case_index, index);
    expected = ExpectedRandom(bus->wram, input.accumulator, byte_only);

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    if (byte_only)
        Lufia2RandomByte(&memory, &native);
    else
        Lufia2RandomScale(&memory, &native);
    if (native.accumulator != expected) {
        fprintf(stderr,
            "FAIL %s case %u: result=%04X expected=%04X\n",
            name, case_index, native.accumulator, expected);
        return false;
    }

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, byte_only ? 0x8082c7u : 0x808299u, &input);
    stop = SnesVerifyRunUntil(
        reference, &stop_pc, 1, 1024, &instructions);

    if (stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL %s case %u: stop=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X DB=%02X/%02X M=%u/%u Xf=%u/%u "
            "N=%u/%u Z=%u/%u C=%u/%u V=%u/%u\n",
            name, case_index, stop, instructions,
            native.accumulator, reference->a,
            native.x, reference->x,
            native.y, reference->y,
            native.stack, reference->sp,
            native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.negative, reference->n,
            native.zero, reference->z,
            native.carry, reference->c,
            native.overflow, reference->v);
        free(native_wram);
        return false;
    }

    free(native_wram);
    return true;
}

static bool RunRandomTimerHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    bool scaled) {
    const unsigned variant = case_index & 3u;
    const uint8_t range = (uint8_t)(case_index * 0x1du + 0x40u);
    const uint8_t base = (uint8_t)((case_index >> 2) * 0x0bu);
    const uint8_t index = (uint8_t)((case_index >> 2) % 0x3au);
    const uint16_t slot = (uint16_t)(8u + (case_index % 24u));
    const uint16_t record = (uint16_t)((case_index % 40u) * 3u);
    const uint16_t dp = (variant & 1u) ? 0x0020u : 0;
    const uint16_t script = (uint16_t)(TEST_SCRIPT + variant);
    const uint32_t handler_pc = scaled ? 0x83c8d4u : 0x83c8afu;
    Lufia2ActorFrontendCpu input;

    if (!SeedKnownPrimaryHandler(bus, variant, script, &input) ||
        !Poke16(bus, dp + 0x00a7u, slot) ||
        !Poke16(bus, dp + 0x00abu, record) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(script + 1u),
            scaled ? base : range) ||
        !SnesVerifyBusPoke(
            bus, 0x7e0000u | (uint16_t)(script + 2u),
            scaled ? range : base) ||
        !SnesVerifyBusPoke(
            bus, 0x7fe3c6u + slot, (uint8_t)~case_index))
        return false;

    SeedRandomTable(bus, case_index ^ 0x5au, index);
    input.overflow = (case_index >> 3) & 1u;
    input.irq_disable = (case_index >> 4) & 1u;

    return CompareKnownPrimaryBoundary(
        bus, reference, initial, &input,
        handler_pc, 0, 0x83c8d2u, handler_pc,
        LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2,
        0, case_index, scaled ? "C8D4" : "C8AF");
}


typedef struct GenericHandlerStats {
    unsigned redispatched;
    unsigned committed;
    unsigned boundaries;
} GenericHandlerStats;

static uint8_t g_generic_seed[SNES_VERIFY_WRAM_SIZE];

/*
 * Broad seeded state for handlers built from leader, facing, $47, RNG
 * and D350 pieces. The native flow picks the reference stop PC; state
 * and WRAM must then match exactly.
 */
static bool RunGenericHandlerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    uint32_t handler_pc,
    unsigned case_index,
    const char *name,
    bool low_script,
    GenericHandlerStats *stats) {
    static const int8_t deltas[8] = {0, 1, -1, 2, -2, 5, -9, 0x40};
    const unsigned variant = case_index & 15u;
    const uint8_t action = (uint8_t)((case_index >> 4) & 3u);
    const uint16_t slot =
        (uint16_t)(8u + ((action + variant * 3u) % 24u));
    const uint16_t dp = (variant & 2u) ? 0x0020u : 0;
    /* Low scripts also run with DB=$00, so $42xx is MMIO. */
    const uint16_t script = (uint16_t)(
        (low_script ? 0x1800u : TEST_SCRIPT) + (case_index & 7u));
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryScriptStepResult result;
    uint8_t actor_x;
    uint8_t actor_y;

    if (!SeedActionCoreCase(bus, variant, action, &input))
        return false;

    input.data_bank = 0x7eu;
    input.program_bank = 0x83u;
    input.index_is_8_bit = 0;
    input.x = (uint16_t)(0x1600u | (case_index & 0xffu));
    input.y = script;
    input.overflow = (case_index >> 9) & 1u;

    actor_x = bus->wram[0x06bau + slot];
    actor_y = bus->wram[0x06e2u + slot];
    bus->wram[0x06bau] =
        (uint8_t)(actor_x + deltas[(case_index >> 6) & 7u]);
    bus->wram[0x06e2u] =
        (uint8_t)(actor_y + deltas[(case_index >> 3) & 7u]);
    bus->wram[0x0692u + slot] = (uint8_t)(((case_index >> 7) & 3u) * 2u);
    bus->wram[dp + 0x47u] = (uint8_t)(case_index * 0x35u);
    bus->wram[0x10000u + 0xe5a6u + slot] =
        (uint8_t)(actor_x + deltas[(case_index >> 2) & 7u]);
    bus->wram[0x10000u + 0xe5ceu + slot] =
        (uint8_t)(actor_y + deltas[(case_index >> 5) & 7u]);
    for (unsigned i = 1; i < 8u; ++i)
        bus->wram[(uint16_t)(script + i)] =
            (uint8_t)((case_index * 0x3du) ^ (i * 0x47u));
    SeedRandomTable(bus, case_index * 3u, (uint8_t)(case_index % 0x3au));

    if (low_script) {
        input.data_bank = ((case_index >> 8) & 1u) ? 0x00u : 0x7eu;
        /* Garbage facings and directions: exact LLE boundaries. */
        if ((case_index & 7u) == 7u) {
            bus->wram[0x0692u] = (uint8_t)(case_index * 0x61u + 1u);
            bus->wram[0x0692u + slot] = (uint8_t)(case_index * 0x2du + 1u);
        }
        /* D03F reads it as a direction 0/2/4/6. */
        bus->wram[(uint16_t)(script + 1u)] =
            ((case_index & 0x80u) &&
             (handler_pc != 0x83d03fu || (case_index & 7u) == 7u))
            ? (uint8_t)(bus->wram[(uint16_t)(script + 1u)] & 0xfeu)
            : (uint8_t)(((case_index >> 5) & 3u) * 2u);
        bus->wram[(uint16_t)(script + 2u)] =
            (uint8_t)((case_index >> 2) % 7u);
        bus->wram[0x0692u] = (uint8_t)(((case_index >> 4) & 3u) * 2u);
        bus->wram[0x1291u + slot] = (uint8_t)(case_index * 0x59u);
        bus->wram[0x09a1u] = (uint8_t)(case_index * 0x21u);
        bus->wram[0x10000u + 0xe57eu + slot] =
            (uint8_t)(case_index * 0x13u);
        /* One record value; D210's operand sits at -1..+2. */
        memset(bus->wram + 0x10000u + 0xe5a6u,
            (uint8_t)(case_index * 0x1du), 0x100u);
        bus->wram[(uint16_t)(script + 3u)] = (uint8_t)(
            case_index * 0x1du + ((case_index >> 8) & 3u) - 1u);

        /* $A9 record, $7F:DB4C slot pick, $05D2 slot states. */
        const uint16_t record = (uint16_t)((case_index * 3u) & 0x3fu);
        const uint8_t leader_x = bus->wram[0x06bau];
        const uint8_t leader_y = bus->wram[0x06e2u];
        bus->wram[dp + 0xa9u] = (uint8_t)record;
        bus->wram[dp + 0xaau] = 0;
        bus->wram[0x10000u + 0xdb4cu + record] =
            (uint8_t)((case_index >> 3) % 40u);
        for (unsigned i = 0; i < 40u; ++i) {
            static const uint8_t states[4] = {0xffu, 0x80u, 0x7fu, 0xc3u};
            bus->wram[0x05d2u + i] = states[(i + case_index) & 3u];
            bus->wram[0x06bau + i] = (uint8_t)(
                actor_x + deltas[(i * 5u + case_index) & 7u] / 2);
            bus->wram[0x06e2u + i] = (uint8_t)(
                actor_y + deltas[(i * 3u + (case_index >> 2)) & 7u] / 2);
        }
        /* Only the actor in range: CFB9 must skip itself. */
        if (case_index & 0x200u) {
            for (unsigned i = 0; i < 40u; ++i)
                bus->wram[0x06bau + i] = (uint8_t)(actor_x + 0x40u);
        }
        bus->wram[0x06bau + slot] = actor_x;
        bus->wram[0x06e2u + slot] = actor_y;
        bus->wram[0x06bau] = leader_x;
        bus->wram[0x06e2u] = leader_y;

        /*
         * Collision map ($7E:4000 + x + y * width), height bits
         * ($7F:0001 + cell * 2), actor size and wander bounds.
         */
        bus->wram[0x05b9u] = (uint8_t)(0x20u + ((case_index >> 4) & 0x10u));
        bus->wram[0x05aau] = 0;
        bus->wram[0x05abu] = 0;
        bus->wram[0x10000u + 0xd008u] = 0;
        bus->wram[0x10000u + 0xd009u] = 0;
        for (unsigned i = 0; i < 0x5000u; ++i) {
            const unsigned h = (i * 0x9du) ^ (i >> 5) ^ case_index;
            bus->wram[0x4000u + i] =
                (h & 0x0cu) == 0 ? (uint8_t)(h * 0x3bu) : 0u;
        }
        for (unsigned i = 0; i < 0x4000u; i += 2u)
            bus->wram[0x10001u + i] =
                (((i >> 6) ^ case_index) & 0x1cu) == 0
                    ? (uint8_t)((i + case_index) << 6) : 0x00u;
        bus->wram[dp + 0x90u] = 0x5au;
        bus->wram[dp + 0x92u] = 0xa5u;
        bus->wram[0x10000u + 0xe216u + slot] =
            (uint8_t)(1u + ((case_index >> 3) & 1u));
        bus->wram[0x10000u + 0xe5f6u + slot] =
            (uint8_t)(actor_x - (case_index & 3u));
        bus->wram[0x10000u + 0xe61eu + slot] =
            (uint8_t)(actor_y - ((case_index >> 2) & 3u));
        bus->wram[0x10000u + 0xe646u + slot] =
            (uint8_t)(actor_x + ((case_index >> 7) & 3u));
        bus->wram[0x10000u + 0xe66eu + slot] =
            (uint8_t)(actor_y + ((case_index >> 8) & 3u));

        /* Reset, script-table, event and fine-position inputs. */
        bus->wram[0x070au + slot] = (uint8_t)((case_index * 0x13u) & 0x3fu);
        bus->wram[0x09a6u] = (uint8_t)(case_index * 0x55u);
        bus->wram[0x0622u] = (uint8_t)(case_index * 0x37u);
        bus->wram[0x09a7u] = (uint8_t)(case_index >> 2);
        bus->wram[0x1724u] = (uint8_t)((case_index * 7u) & 0x3fu);
        bus->wram[0x1725u] = 0;
        bus->wram[0x10000u + 0xd0a1u] =
            (case_index & 0x40u) ? 0x40u : 0x00u;
        for (unsigned i = 0; i < 0x400u; ++i)
            bus->wram[0x10000u + 0xdc8cu + i] =
                (uint8_t)((i * 0x3bu) ^ (case_index * 0x11u));

        /* Operand point on the actor for CF8C's arrival path. */
        if (handler_pc == 0x83cf8cu && (case_index & 0x0cu) == 0x0cu) {
            bus->wram[(uint16_t)(script + 1u)] = actor_x;
            bus->wram[(uint16_t)(script + 2u)] =
                (uint8_t)(actor_y + ((case_index >> 4) & 1u));
        }

        /*
         * $7E:F000 point lists for D0AA, strides $0F and 4. D0AA keeps
         * walking from slot+stride after C9C5 clobbers X, so the rest
         * of the page reads as terminators, like unused list space.
         */
        memset(bus->wram + 0xf000u, 0xff, 0x1000u);
        {
            static const struct { uint16_t head; uint16_t list;
                                  uint8_t stride; } lists[2] = {
                {0x0002u, 0x0100u, 0x0fu}, {0x0026u, 0x0300u, 0x04u}};
            for (unsigned l = 0; l < 2u; ++l) {
                const unsigned count = (case_index >> (l * 2u)) % 5u;
                uint16_t at = lists[l].list;
                bus->wram[0xf000u + lists[l].head] = (uint8_t)at;
                bus->wram[0xf001u + lists[l].head] = (uint8_t)(at >> 8);
                for (unsigned e = 0; e < count; ++e) {
                    bus->wram[0xf000u + at] = (uint8_t)e;
                    bus->wram[0xf001u + at] = (uint8_t)(
                        actor_x + deltas[(e + case_index) & 7u]);
                    bus->wram[0xf002u + at] = (uint8_t)(
                        actor_y + deltas[(e * 3u + case_index) & 7u]);
                    at = (uint16_t)(at + lists[l].stride);
                }
                bus->wram[0xf000u + at] = 0xffu;
            }
        }
    }

    memcpy(g_generic_seed, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2ActorPrimaryScriptExecuteKnownHandler(
        &memory, &native, handler_pc);
    memcpy(bus->wram, g_generic_seed, SNES_VERIFY_WRAM_SIZE);

    if (result.flow == LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED) {
        ++stats->redispatched;
        return CompareKnownPrimaryBoundary(
            bus, reference, initial, &input,
            handler_pc, 0x83c85cu, result.handler_pc, handler_pc,
            LUFIA2_ACTOR_PRIMARY_SCRIPT_REDISPATCHED,
            result.opcode, case_index, name);
    }
    if (result.flow == LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2) {
        ++stats->committed;
        return CompareKnownPrimaryBoundary(
            bus, reference, initial, &input,
            handler_pc, 0, 0x83c8d2u, handler_pc,
            LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2,
            0, case_index, name);
    }
    /* Boundary: ROM must reach resume_pc with the same state. */
    {
        Lufia2ActorFrontendCpu boundary = input;
        uint8_t *boundary_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
        unsigned instructions = 0;
        bool reached = false;

        if (!boundary_wram)
            return false;
        memcpy(bus->wram, g_generic_seed, SNES_VERIFY_WRAM_SIZE);
        memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
        (void)Lufia2ActorPrimaryScriptExecuteKnownHandler(
            &memory, &boundary, handler_pc);
        memcpy(boundary_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
        memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
        InitInterp(reference, handler_pc, &input);
        while (instructions < 4000000u) {
            if (SnesVerifyPc24(reference) == result.handler_pc &&
                reference->sp == boundary.stack) {
                reached = true;
                break;
            }
            interp816_runOpcode(reference);
            ++instructions;
        }
        if (!reached || boundary.resume_pc != result.handler_pc ||
            !SameState(&boundary, reference) ||
            memcmp(boundary_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
            fprintf(stderr,
                "FAIL %s case %u: boundary %06X reached=%d "
                "A=%04X/%04X X=%04X/%04X S=%04X/%04X\n",
                name, case_index, result.handler_pc, reached ? 1 : 0,
                boundary.accumulator, reference->a,
                boundary.x, reference->x, boundary.stack, reference->sp);
            free(boundary_wram);
            return false;
        }
        free(boundary_wram);
        ++stats->boundaries;
        return true;
    }
}

typedef struct WholeFunction {
    const char *name;
    uint32_t entry;
    uint32_t exit;
    void (*native)(const Lufia2ActorFrontendMemory *,
                   Lufia2ActorFrontendCpu *);
} WholeFunction;

static const WholeFunction kWholeFunctions[] = {
    {"83:FA3F", 0x83fa3fu, 0x83fa80u, Lufia2ActorMarkMapOccupancy},
    {"83:D416", 0x83d416u, 0x83d436u, Lufia2ActorLoadPrimaryScript},
    {"83:A746", 0x83a746u, 0x83a76cu, Lufia2ActorSyncFinePosition},
    {"84:8766", 0x848766u, 0x848774u, Lufia2QueueDeferredSound},
    {"83:FACB", 0x83facbu, 0x83faf3u, Lufia2ActorAddDisplayOffset},
    {"83:FA81", 0x83fa81u, 0x83facau, Lufia2ActorMoveFinePosition},
    {"83:C947", 0x83c947u, 0x83c989u, Lufia2ActorPrimaryReset},
    {"83:CB65", 0x83cb65u, 0x83cb70u, Lufia2ActorClearSlotLinks},
    {"83:CA68", 0x83ca68u, 0x83ca92u, Lufia2ActorBlockedEvent},
};
enum {
    WHOLE_FUNCTION_COUNT =
        sizeof(kWholeFunctions) / sizeof(kWholeFunctions[0]),
    WHOLE_FUNCTION_CASES = 4096,
};

static uint64_t g_whole_rng = UINT64_C(0x2545f4914f6cdd1d);

static uint32_t WholeRandom(void) {
    g_whole_rng ^= g_whole_rng << 13;
    g_whole_rng ^= g_whole_rng >> 7;
    g_whole_rng ^= g_whole_rng << 17;
    return (uint32_t)(g_whole_rng >> 32);
}

/* Random WRAM and entry state, ROM run to the RTS/RTL. */
static bool RunWholeFunctionCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    const WholeFunction *function,
    unsigned case_index) {
    static const uint16_t dps[5] = {
        0x0000u, 0x0020u, 0x0080u, 0x0400u, 0x0a00u};
    static const uint8_t banks[5] = {0x00u, 0x7eu, 0x7fu, 0x80u, 0x83u};
    const uint16_t dp = dps[WholeRandom() % 5u];
    const uint8_t slot = (uint8_t)(WholeRandom() % 40u);
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    uint8_t *native_wram;
    unsigned instructions = 0;
    int stop;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = WholeRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    Poke16(bus, dp + 0x00a7u, slot);
    Poke16(bus, dp + 0x00a9u, (uint16_t)(slot * 2u));
    Poke16(bus, dp + 0x00abu, (uint16_t)(slot * 3u));
    Poke16(bus, dp + 0x002au, (uint16_t)(0x1800u + (WholeRandom() & 0xffu)));
    Poke16(bus, 0x1724u, (uint16_t)(WholeRandom() & 0x3fu));

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)WholeRandom();
    input.x = (uint16_t)WholeRandom();
    input.y = (uint16_t)(0x1800u + (WholeRandom() & 0xffu));
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[WholeRandom() % 5u];
    input.program_bank = (uint8_t)(function->entry >> 16);
    input.carry = WholeRandom() & 1u;
    input.zero = WholeRandom() & 1u;
    input.negative = WholeRandom() & 1u;
    input.overflow = WholeRandom() & 1u;
    input.irq_disable = WholeRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = 0;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    function->native(&memory, &native);

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, function->entry, &input);
    stop = SnesVerifyRunUntil(
        reference, &function->exit, 1, 4096, &instructions);

    if (stop != 0 || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL whole %s case %u: stop=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X M=%u/%u V=%u/%u C=%u/%u\n",
            function->name, case_index, stop, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.overflow, reference->v, native.carry, reference->c);
        free(native_wram);
        return false;
    }
    free(native_wram);
    return true;
}

enum { RESUME_EXACT_CASES = 2048 };

/* D350 with DP >= $0100 and FB12 with a bad index. */
static bool RunResumeExactCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    bool fb12) {
    static const uint16_t dps[3] = {0x0400u, 0x0a00u, 0x0100u};
    const uint16_t dp = dps[WholeRandom() % 3u];
    const uint8_t slot = (uint8_t)(WholeRandom() % 40u);
    const uint32_t entry = fb12 ? 0x83fb12u : 0x83d350u;
    const uint32_t resume = fb12 ? 0x83fb17u : 0x83d370u;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool unknown;
    int stop;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = WholeRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    Poke16(bus, dp + 0x00a7u, slot);
    bus->wram[0x0622u + slot] &= (uint8_t)~0x28u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)WholeRandom();
    if (fb12) {
        uint8_t direction;
        do {
            direction = (uint8_t)WholeRandom();
        } while (!(direction & 1u) && direction < 8u);
        input.accumulator =
            (uint16_t)((input.accumulator & 0xff00u) | direction);
    } else {
        input.accumulator =
            (uint16_t)((input.accumulator & 0xff00u) | (WholeRandom() & 3u));
    }
    input.x = (uint16_t)WholeRandom();
    input.y = (uint16_t)WholeRandom();
    input.stack = 0x1ff0u;
    input.direct_page = fb12 ? (uint16_t)(WholeRandom() & 0x00e0u) : dp;
    input.data_bank = 0x7eu;
    input.program_bank = 0x83u;
    input.carry = WholeRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = fb12 ? (uint8_t)(WholeRandom() & 1u) : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    unknown = fb12
        ? Lufia2ActorMovementStep(&memory, &native) == 0
        : Lufia2ActorPrimaryActionCore(&memory, &native) ==
              LUFIA2_ACTOR_PRIMARY_ACTION_UNKNOWN_D370_TARGET;

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, entry, &input);
    stop = SnesVerifyRunUntil(reference, &resume, 1, 256, &instructions);

    if (!unknown || native.resume_pc != resume || stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL resume %s case %u: unknown=%d resume=%06X stop=%d "
            "A=%04X/%04X X=%04X/%04X S=%04X/%04X\n",
            fb12 ? "FB17" : "D370", case_index, unknown ? 1 : 0,
            native.resume_pc, stop, native.accumulator, reference->a,
            native.x, reference->x, native.stack, reference->sp);
        free(native_wram);
        return false;
    }
    free(native_wram);
    return true;
}

typedef struct PrimaryUpdateStats {
    unsigned returned_c83b;
    unsigned returned_c8d3;
    unsigned boundary_handler;
    unsigned boundary_internal;
    unsigned dispatches;
} PrimaryUpdateStats;

enum { PRIMARY_UPDATE_CASES = 16384 };

/* Whole $83:C7F8 against the ROM, random machine and scripts. */
static bool RunPrimaryUpdateCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    PrimaryUpdateStats *stats) {
    static const uint16_t dps[4] = {0x0000u, 0x0020u, 0x0000u, 0x0400u};
    static const uint8_t banks[4] = {0x00u, 0x7eu, 0x80u, 0x83u};
    const uint16_t dp = dps[WholeRandom() & 3u];
    const uint8_t slot = (uint8_t)(WholeRandom() % 40u);
    const uint16_t record = (uint16_t)(slot * 3u);
    const uint16_t script = (uint16_t)(0x1800u + (WholeRandom() & 0x3ffu));
    const uint32_t exits[2] = {0x83c83bu, 0x83c8d3u};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    unsigned dispatches = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = WholeRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    /* Half the scripts favour opcodes that redispatch. */
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i) {
        static const uint8_t chain[] = {
            0x0c, 0x0d, 0x0e, 0x1a, 0x21, 0x23, 0x24, 0x25, 0x26, 0x27,
            0x2a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x39,
            0x3a, 0x3d, 0x40, 0x42};
        const unsigned roll = WholeRandom() % 100u;
        if (case_index & 1u)
            bus->wram[i] = roll < 95u
                ? chain[WholeRandom() % sizeof(chain)]
                : (uint8_t)(WholeRandom() % 0x50u);
        else
            bus->wram[i] = roll < 85u
                ? (uint8_t)(WholeRandom() % 0x50u) : (uint8_t)WholeRandom();
    }
    Poke16(bus, dp + 0x00a7u, slot);
    Poke16(bus, dp + 0x00a9u, (uint16_t)(slot * 2u));
    Poke16(bus, dp + 0x00abu, record);
    Poke16(bus, 0x7fe506u + record, script);
    bus->wram[0x1e508u + record] = (WholeRandom() & 3u) ? 0x7eu : 0x00u;
    if ((WholeRandom() & 7u) != 0)
        bus->wram[0x0622u + slot] &= 0x7fu;
    if (WholeRandom() & 3u)
        bus->wram[0x1291u + slot] &= 0xf8u;
    bus->wram[0x1e3c6u + slot] = (uint8_t)(WholeRandom() % 3u);
    Poke16(bus, 0x1724u, (uint16_t)(WholeRandom() & 0x3fu));

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)WholeRandom();
    input.x = (uint16_t)WholeRandom();
    input.y = (uint16_t)WholeRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[WholeRandom() & 3u];
    input.program_bank = 0x83u;
    input.carry = WholeRandom() & 1u;
    input.zero = WholeRandom() & 1u;
    input.negative = WholeRandom() & 1u;
    input.overflow = WholeRandom() & 1u;
    input.irq_disable = WholeRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = (WholeRandom() & 3u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2ActorPrimaryUpdate(&memory, &native);

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83c7f8u, &input);
    while (instructions < 4000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
            if (pc == exits[0] || pc == exits[1]) {
                stopped = pc == result.pc;
                break;
            }
        } else if (dispatches == result.dispatches && pc == result.pc &&
                   reference->sp == native.stack) {
            stopped = true;
            break;
        }
        if (pc == 0x83c864u)
            ++dispatches;
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL C7F8 case %u: flow=%u pc=%06X/%06X disp=%u/%u "
            "insns=%u A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X DB=%02X/%02X M=%u/%u Xf=%u/%u wram@%05X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), result.dispatches, dispatches,
            instructions, native.accumulator, reference->a,
            native.x, reference->x, native.y, reference->y,
            native.stack, reference->sp, native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf, (unsigned)diff);
        free(native_wram);
        return false;
    }
    free(native_wram);

    stats->dispatches += result.dispatches;
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
        if (result.pc == exits[0])
            ++stats->returned_c83b;
        else
            ++stats->returned_c8d3;
    } else {
        ++stats->boundary_handler;
    }
    return true;
}

enum { ACTION_CORE_X8_CASES = 8192 };

typedef struct ActionCoreX8Stats {
    unsigned returned;
    unsigned installed;
    unsigned boundary;
} ActionCoreX8Stats;

static uint64_t g_x8_rng = UINT64_C(0x5851f42d4c957f2d);

static uint32_t X8Random(void) {
    g_x8_rng ^= g_x8_rng << 13;
    g_x8_rng ^= g_x8_rng >> 7;
    g_x8_rng ^= g_x8_rng << 17;
    return (uint32_t)(g_x8_rng >> 32);
}

/* D350 as C1B4 calls it: X8, JSL from $83:C237. */
static bool RunActionCoreX8Case(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    ActionCoreX8Stats *stats) {
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x00u, 0x7eu};
    static const uint32_t limits[4] = {
        0x1e61eu, 0x1e66eu, 0x1e5f6u, 0x1e646u};
    const uint16_t dp = (case_index & 7u) == 7u ? 0x0020u : 0x0000u;
    const uint8_t slot =
        (X8Random() & 3u) ? 0u : (uint8_t)(X8Random() % 40u);
    const uint8_t direction = (uint8_t)(X8Random() & 3u);
    const uint8_t action = (X8Random() & 3u)
        ? direction : (uint8_t)(direction + 0x18u);
    const uint8_t coordinate = (uint8_t)(X8Random() % 12u);
    const uint32_t site = 0x83c23bu;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryActionFlow flow;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = X8Random();
        memcpy(bus->wram + i, &word, 4);
    }
    Poke16(bus, dp + 0x00a7u, slot);
    if (X8Random() & 3u)
        bus->wram[0x0622u + slot] &= (uint8_t)~0x28u;
    /* Tile near the per-actor range limit. */
    bus->wram[(direction < 2u ? 0x06e2u : 0x06bau) + slot] =
        (X8Random() & 7u) ? coordinate : 0u;
    bus->wram[limits[direction] + slot] =
        (uint8_t)(coordinate + (X8Random() % 5u) - 2u);
    /* JSL $83:D350 at $83:C237. */
    bus->wram[0x1fe1u] = 0x3au;
    bus->wram[0x1fe2u] = 0xc2u;
    bus->wram[0x1fe3u] = 0x83u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)((X8Random() << 8) | action);
    input.x = (uint16_t)(X8Random() & 0xffu);
    input.y = (uint16_t)(X8Random() & 0xffu);
    input.stack = 0x1fe0u;
    input.direct_page = dp;
    input.data_bank = banks[X8Random() & 3u];
    input.program_bank = 0x83u;
    input.carry = X8Random() & 1u;
    input.zero = action == 0u;
    input.negative = 0;
    input.overflow = X8Random() & 1u;
    input.irq_disable = X8Random() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = 1;

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    flow = Lufia2ActorPrimaryActionCore(&memory, &native);
    if (flow == LUFIA2_ACTOR_PRIMARY_ACTION_RETURN_D3AE)
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83d350u, &input);
    while (instructions < 4096u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (flow == LUFIA2_ACTOR_PRIMARY_ACTION_RETURN_D3AE
                ? pc == site
                : pc == native.resume_pc && reference->sp == native.stack) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }

    /* The only X8 handoff is $83:D38D. */
    if (flow != LUFIA2_ACTOR_PRIMARY_ACTION_RETURN_D3AE &&
        native.resume_pc != 0x83d38du)
        stopped = false;
    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL D350 X8 case %u: action=%02X flow=%u pc=%06X/%06X "
            "insns=%u A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X DB=%02X/%02X C=%u/%u wram@%05X\n",
            case_index, action, (unsigned)flow, native.resume_pc,
            SnesVerifyPc24(reference), instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db, native.carry, reference->c,
            (unsigned)diff);
        free(native_wram);
        return false;
    }
    free(native_wram);

    if (flow != LUFIA2_ACTOR_PRIMARY_ACTION_RETURN_D3AE)
        ++stats->boundary;
    else if ((initial[0x0622u + slot] & 0x80u) !=
             (bus->wram[0x0622u + slot] & 0x80u))
        ++stats->installed;
    else
        ++stats->returned;
    return true;
}

enum { PLAYER_STANDARD_CASES = 16384 };

typedef struct PlayerStandardStats {
    unsigned early;
    unsigned walked;
    unsigned boundary[11];
} PlayerStandardStats;

static const uint32_t kPlayerBoundaries[10] = {
    0x83c1ccu, 0x83c1d7u, 0x83c1dcu, 0x8ebbd1u, 0x8eb6d4u, 0x83d38du,
    0x83d370u, 0x83fbc2u, 0x83ba1cu, 0x83fb17u};

static uint64_t g_player_rng = UINT64_C(0x14057b7ef767814f);

static uint32_t PlayerRandom(void) {
    g_player_rng ^= g_player_rng << 13;
    g_player_rng ^= g_player_rng >> 7;
    g_player_rng ^= g_player_rng << 17;
    return (uint32_t)(g_player_rng >> 32);
}

/* Door rectangles around the leader at $7E:F010. */
static void SeedPlayerDoors(SnesVerifyBus *bus, uint8_t px, uint8_t py) {
    const unsigned count = PlayerRandom() % 4u;
    uint32_t entry = 0xf010u;

    Poke16(bus, 0x7ef002u, 0x0010u);
    for (unsigned i = 0; i < count; ++i, entry += 15u) {
        for (unsigned r = 0; r < 2u; ++r) {
            const uint32_t rect = entry + 5u + 4u * r;
            bus->wram[rect + 0u] = (uint8_t)(px + 2u - PlayerRandom() % 5u);
            bus->wram[rect + 1u] = (uint8_t)(py + 2u - PlayerRandom() % 5u);
            bus->wram[rect + 2u] = (uint8_t)(px + PlayerRandom() % 5u - 1u);
            bus->wram[rect + 3u] = (uint8_t)(py + PlayerRandom() % 5u - 1u);
        }
        bus->wram[entry] = (uint8_t)(PlayerRandom() % 0xffu);
        bus->wram[entry + 2u] = (uint8_t)(py + PlayerRandom() % 5u - 2u);
        if (PlayerRandom() & 1u)
            bus->wram[entry + 9u] = 0xffu;
        bus->wram[entry + 13u] =
            (PlayerRandom() & 3u) ? (uint8_t)PlayerRandom() : 0xffu;
    }
    bus->wram[entry] = 0xffu;
}

/* Whole $83:C1B4 from the BBF3 JSR against the ROM. */
static bool RunPlayerStandardCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    PlayerStandardStats *stats) {
    static const uint16_t dps[8] = {
        0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x83u, 0x83u, 0x83u, 0x00u, 0x80u, 0x7eu};
    static const uint8_t pads[8] = {1, 2, 4, 8, 1, 2, 4, 8};
    const uint16_t dp = dps[PlayerRandom() & 7u];
    const uint8_t px = (uint8_t)(4u + PlayerRandom() % 8u);
    const uint8_t py = (uint8_t)(4u + PlayerRandom() % 8u);
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = PlayerRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    /* Controller, debug flag and leader. */
    if (PlayerRandom() & 7u)
        bus->wram[0x099bu] &= 0x7fu;
    bus->wram[dp + 0x46u] &= (PlayerRandom() & 3u) ? 0x5fu : 0xffu;
    bus->wram[dp + 0x47u] = (uint8_t)(
        (PlayerRandom() & 0xe0u & ((PlayerRandom() & 3u) ? 0xdfu : 0xffu)) |
        ((PlayerRandom() & 7u) ? pads[PlayerRandom() & 7u]
                               : (uint8_t)(PlayerRandom() & 0x0fu)));
    if (PlayerRandom() & 1u)
        bus->wram[0x057cu] = 0;
    Poke16(bus, dp + 0xa7u,
        (PlayerRandom() & 7u) ? 0u : (uint16_t)(PlayerRandom() % 40u));
    bus->wram[0x06bau] = px;
    bus->wram[0x06e2u] = py;
    if (PlayerRandom() & 7u)
        bus->wram[0x0692u] = (uint8_t)((PlayerRandom() & 3u) * 2u);
    /* Other actors near the leader. */
    for (unsigned slot = 8; slot < 40u; ++slot) {
        bus->wram[0x06bau + slot] = (uint8_t)(px + PlayerRandom() % 7u - 3u);
        bus->wram[0x06e2u + slot] = (uint8_t)(py + PlayerRandom() % 7u - 3u);
        if (PlayerRandom() & 3u)
            bus->wram[0x0622u + slot] &= 0xfbu;
        bus->wram[0x1e216u + slot] = (uint8_t)(1u + (PlayerRandom() & 1u));
        if ((PlayerRandom() & 7u) == 0)
            bus->wram[0x05d2u + slot] = 0x70u;
    }
    /* Map: width, sparse collision, values biased to 6. */
    bus->wram[0x05b9u] = (uint8_t)(0x20u + (PlayerRandom() & 0x10u));
    Poke16(bus, 0x05aau, 0);
    Poke16(bus, 0x7fd008u, 0);
    Poke16(bus, 0x7fd03eu, 0x8000u);
    for (unsigned i = 0; i < 0x2000u; ++i)
        if (PlayerRandom() & 1u)
            bus->wram[0x4000u + i] &= 0x31u;
    for (unsigned i = 0; i < 0x400u; ++i) {
        const unsigned roll = PlayerRandom() % 6u;
        bus->wram[0x18000u + i] = roll < 2u ? 6u : roll == 2u ? 7u :
            roll == 3u ? 0u : (uint8_t)PlayerRandom();
    }
    SeedPlayerDoors(bus, px, py);
    /* JSR $C1B4 at $83:BC22. */
    bus->wram[0x1ff1u] = 0x24u;
    bus->wram[0x1ff2u] = 0xbcu;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)PlayerRandom();
    input.x = (uint16_t)PlayerRandom();
    input.y = (uint16_t)PlayerRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[PlayerRandom() & 7u];
    input.program_bank = 0x83u;
    input.carry = PlayerRandom() & 1u;
    input.zero = PlayerRandom() & 1u;
    input.negative = PlayerRandom() & 1u;
    input.overflow = PlayerRandom() & 1u;
    input.irq_disable = PlayerRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = (PlayerRandom() & 3u) ? 0u : 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2PlayerSlotStandardUpdate(&memory, &native);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83c1b4u, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == 0x83bc25u
                : pc == result.pc && reference->sp == native.stack) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL C1B4 case %u: flow=%u pc=%06X/%06X insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X PB=%02X/%02X Xf=%u/%u C=%u/%u Z=%u/%u "
            "N=%u/%u V=%u/%u wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db, native.program_bank,
            reference->k, native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, native.overflow, reference->v,
            (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);

    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
        if (result.pc == 0x83c1e2u)
            ++stats->early;
        else
            ++stats->walked;
    } else {
        unsigned k = 0;
        while (k < 10u && kPlayerBoundaries[k] != result.pc)
            ++k;
        ++stats->boundary[k];
    }
    return true;
}

enum { ACTOR_SLOTS_CASES = 2048 };

typedef struct ActorSlotsStats {
    unsigned returned;
    unsigned unterminated;
    unsigned boundary;
} ActorSlotsStats;

typedef struct SlotsChildContext {
    SnesVerifyBus *bus;
    Interp816 *interp;
    bool stuck;
    bool bad_site;
} SlotsChildContext;

static void InterpToNative(
    const Interp816 *reference, Lufia2ActorFrontendCpu *cpu) {
    cpu->accumulator = reference->a;
    cpu->x = reference->x;
    cpu->y = reference->y;
    cpu->stack = reference->sp;
    cpu->direct_page = reference->dp;
    cpu->data_bank = reference->db;
    cpu->program_bank = reference->k;
    cpu->carry = reference->c;
    cpu->zero = reference->z;
    cpu->negative = reference->n;
    cpu->overflow = reference->v;
    cpu->decimal = reference->d;
    cpu->irq_disable = reference->i;
    cpu->accumulator_is_8_bit = reference->mf;
    cpu->index_is_8_bit = reference->xf;
}

/* JSR child in interp816 until its RTS. */
static uint8_t RunSlotChild(
    void *opaque,
    Lufia2ActorFrontendCpu *cpu,
    uint32_t target,
    uint32_t site) {
    SlotsChildContext *context = (SlotsChildContext *)opaque;
    const uint16_t entry_s = cpu->stack;
    const uint16_t pushed = (uint16_t)(site + 2u);
    const uint32_t back = (site & 0xff0000u) | (uint16_t)(site + 3u);

    /* site must hold JSR target. */
    if (Rom8(context->bus, site) != 0x20u ||
        (Rom8(context->bus, site + 1u) |
         ((uint32_t)Rom8(context->bus, site + 2u) << 8)) !=
            (target & 0xffffu)) {
        context->stuck = true;
        context->bad_site = true;
        return 0;
    }
    SnesVerifyBusWrite(context->bus, cpu->stack, (uint8_t)(pushed >> 8));
    cpu->stack = (uint16_t)(cpu->stack - 1u);
    SnesVerifyBusWrite(context->bus, cpu->stack, (uint8_t)pushed);
    cpu->stack = (uint16_t)(cpu->stack - 1u);
    cpu->program_bank = (uint8_t)(target >> 16);
    InitInterp(context->interp, target, cpu);
    for (unsigned n = 0; n < 1000000u; ++n) {
        if (SnesVerifyPc24(context->interp) == back &&
            context->interp->sp == entry_s) {
            InterpToNative(context->interp, cpu);
            return 1;
        }
        interp816_runOpcode(context->interp);
    }
    context->stuck = true;
    return 0;
}

/* Multiplier latches survive a run; replay them. */
typedef struct BusRegisters {
    uint8_t multiply_a;
    uint8_t multiply_b;
    uint16_t multiply_result;
    uint16_t divide_a;
    uint16_t divide_result;
    uint8_t m7_latch;
    uint16_t m7_a;
    uint8_t m7_b;
} BusRegisters;

static BusRegisters SaveBusRegisters(const SnesVerifyBus *bus) {
    BusRegisters saved;
    saved.multiply_a = bus->multiply_a;
    saved.multiply_b = bus->multiply_b;
    saved.multiply_result = bus->multiply_result;
    saved.divide_a = bus->divide_a;
    saved.divide_result = bus->divide_result;
    saved.m7_latch = bus->m7_latch;
    saved.m7_a = bus->m7_a;
    saved.m7_b = bus->m7_b;
    return saved;
}

static void RestoreBusRegisters(
    SnesVerifyBus *bus, const BusRegisters *saved) {
    bus->multiply_a = saved->multiply_a;
    bus->multiply_b = saved->multiply_b;
    bus->multiply_result = saved->multiply_result;
    bus->divide_a = saved->divide_a;
    bus->divide_result = saved->divide_result;
    bus->m7_latch = saved->m7_latch;
    bus->m7_a = saved->m7_a;
    bus->m7_b = saved->m7_b;
}

static uint64_t g_slots_rng = UINT64_C(0x3c6ef372fe94f82b);

static uint32_t SlotsRandom(void) {
    g_slots_rng ^= g_slots_rng << 13;
    g_slots_rng ^= g_slots_rng >> 7;
    g_slots_rng ^= g_slots_rng << 17;
    return (uint32_t)(g_slots_rng >> 32);
}

/* Whole $83:BB93 from the field-loop JSL; children in interp816. */
static bool RunActorSlotsCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    ActorSlotsStats *stats) {
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x83u, 0x00u};
    static const uint8_t ops[] = {
        0xf0, 0xf0, 0x00, 0x43, 0xfa, 0x13, 0x22, 0x92, 0xe9, 0xd0,
        0x0c, 0x0d, 0x0e, 0x21, 0x23, 0x30, 0x31, 0x3a, 0x3d, 0x40};
    const uint16_t dp = (case_index & 7u) == 7u ? 0x0020u : 0x0000u;
    SlotsChildContext child = {bus, reference, false, false};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    unsigned visits = 0;
    BusRegisters registers;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = SlotsRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i)
        bus->wram[i] = ops[SlotsRandom() % sizeof(ops)];
    for (unsigned slot = 0; slot < 40u; ++slot) {
        const uint16_t record = (uint16_t)(slot * 3u);
        /* Most slots idle, some run both VMs. */
        if (SlotsRandom() % 3u)
            bus->wram[0x0622u + slot] |= 0x04u;
        Poke16(bus, 0x7fe3eeu + record,
            (uint16_t)(0x1800u + (SlotsRandom() & 0x3ffu)));
        bus->wram[0x1e3f0u + record] = (SlotsRandom() & 1u) ? 0x7eu : 0x00u;
        Poke16(bus, 0x7fe506u + record,
            (uint16_t)(0x1800u + (SlotsRandom() & 0x3ffu)));
        bus->wram[0x1e508u + record] = (SlotsRandom() & 1u) ? 0x7eu : 0x00u;
        bus->wram[0x1e4deu + slot] = (uint8_t)(1u << (SlotsRandom() & 3u));
        bus->wram[0x1e216u + slot] = (uint8_t)(1u + (SlotsRandom() & 1u));
    }
    bus->wram[0x05b9u] = 0x20u;
    Poke16(bus, 0x05aau, 0);
    Poke16(bus, 0x7fd008u, 0);
    for (unsigned i = 0; i < 0x2000u; ++i)
        if (SlotsRandom() & 1u)
            bus->wram[0x4000u + i] = 0;
    if (SlotsRandom() & 1u)
        bus->wram[0x1d0feu] = 0;
    if (SlotsRandom() & 3u)
        bus->wram[0x09a1u] = 0xffu;
    /* JSL $83:BB93 at $83:807D. */
    bus->wram[0x1ff1u] = 0x80u;
    bus->wram[0x1ff2u] = 0x80u;
    bus->wram[0x1ff3u] = 0x83u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)SlotsRandom();
    input.x = (uint16_t)(SlotsRandom() & 0xffu);
    input.y = (uint16_t)(SlotsRandom() & 0xffu);
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[SlotsRandom() & 3u];
    input.program_bank = 0x83u;
    input.carry = SlotsRandom() & 1u;
    input.zero = SlotsRandom() & 1u;
    input.negative = SlotsRandom() & 1u;
    input.overflow = SlotsRandom() & 1u;
    input.irq_disable = SlotsRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = (SlotsRandom() & 7u) ? 1u : 0u;
    if (!input.index_is_8_bit)
        input.x = (uint16_t)SlotsRandom();

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    registers = SaveBusRegisters(bus);
    native = input;
    result = Lufia2UpdateActorSlots(&memory, &native, RunSlotChild, &child);
    if (child.bad_site) {
        fprintf(stderr, "FAIL BB93 case %u: child site mismatch\n",
            case_index);
        return false;
    }
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_CHILD_UNWOUND) {
        ++stats->unterminated;
        return true;
    }
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    RestoreBusRegisters(bus, &registers);
    InitInterp(reference, 0x83bb93u, &input);
    while (instructions < 100000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == 0x838081u
                : pc == 0x83bba1u && ++visits == result.dispatches) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL BB93 case %u: stop=%d insns=%u A=%04X/%04X "
            "X=%04X/%04X Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u wram@%05X %02X/%02X\n",
            case_index, stopped ? 1 : 0, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { FIELD_TRIGGER_CASES = 16384 };

typedef struct FieldTriggerStats {
    unsigned returned;
    unsigned boundary[6];
} FieldTriggerStats;

static const uint32_t kFieldBoundaries[5] = {
    0x80cbccu, 0x80cc0eu, 0x83b96du, 0x838225u, 0x838251u};

static uint64_t g_field_rng = UINT64_C(0x6a09e667f3bcc909);

static uint32_t FieldRandom(void) {
    g_field_rng ^= g_field_rng << 13;
    g_field_rng ^= g_field_rng >> 7;
    g_field_rng ^= g_field_rng << 17;
    return (uint32_t)(g_field_rng >> 32);
}

/* Whole $83:81C6 from the field-loop JSR against the ROM. */
static bool RunFieldTriggerCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    FieldTriggerStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x83u, 0x83u, 0x83u, 0x00u, 0x80u, 0x7eu};
    const uint16_t dp = dps[FieldRandom() & 7u];
    const uint8_t px = (uint8_t)(4u + FieldRandom() % 8u);
    const uint8_t py = (uint8_t)(4u + FieldRandom() % 8u);
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = FieldRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    /* Event timers: idle, counting, or expiring. */
    for (unsigned t = 0; t < 8u; ++t) {
        const unsigned roll = FieldRandom() % 16u;
        bus->wram[0x1d18cu + t] = roll < 10u ? (uint8_t)(FieldRandom() & 0x7fu)
            : roll < 15u || (FieldRandom() & 3u) ? (uint8_t)(0x82u + (FieldRandom() % 0x7du))
                         : 0x81u;
    }
    /* Idle gates mostly open. */
    if (FieldRandom() & 3u)
        bus->wram[0x09a7u] |= 0x01u;
    if (FieldRandom() & 7u) {
        bus->wram[0x09a8u] &= 0xf7u;
        bus->wram[0x0622u] &= 0x77u;
        bus->wram[0x05b7u] &= 0xf8u;
        bus->wram[0x05b5u] &= 0x5du;
        bus->wram[0x17aau] = 0;
        bus->wram[0x099bu] &= 0x7fu;
        for (unsigned i = 0; i < 8u; ++i)
            if (FieldRandom() % 16u)
                bus->wram[0x1d057u + i] &= 0x7fu;
    }
    /* One closed gate at a time. */
    if ((FieldRandom() % 6u) == 0) {
        static const uint16_t gate[5] = {
            0x09a8u, 0x0622u, 0x05b7u, 0x05b5u, 0x17aau};
        static const uint8_t bits[5] = {0x08u, 0x88u, 0x07u, 0xa2u, 0xffu};
        const unsigned g = FieldRandom() % 5u;
        uint8_t bit = (uint8_t)(1u << (FieldRandom() & 7u));
        while (!(bit & bits[g]))
            bit = (uint8_t)(1u << (FieldRandom() & 7u));
        bus->wram[gate[g]] |= bit;
    }
    if (FieldRandom() & 3u)
        bus->wram[0x1d0a1u] = 0;
    if (FieldRandom() & 3u)
        bus->wram[0x05b5u] &= 0xefu;
    if (FieldRandom() & 1u)
        bus->wram[0x057cu] = 0;
    /* Leader, touch actors and edge cells. */
    bus->wram[0x06bau] = px;
    bus->wram[0x06e2u] = py;
    for (unsigned slot = 8; slot < 40u; ++slot) {
        bus->wram[0x06bau + slot] = (uint8_t)(px + FieldRandom() % 5u - 2u);
        bus->wram[0x06e2u + slot] = (uint8_t)(py + FieldRandom() % 5u - 2u);
        if (FieldRandom() & 3u) {
            bus->wram[0x0622u + slot] &= 0x7bu;
            bus->wram[0x0736u + slot] &= 0xebu;
        }
        bus->wram[0x1e216u + slot] = (uint8_t)(1u + (FieldRandom() & 1u));
    }
    bus->wram[0x05b9u] = (uint8_t)(0x20u + (FieldRandom() & 0x10u));
    Poke16(bus, 0x05aau, 0);
    Poke16(bus, 0x7fd008u, 0);
    for (unsigned i = 0; i < 0x2000u; ++i)
        if (FieldRandom() & 1u)
            bus->wram[0x4000u + i] &= 0x31u;
    /* JSR $81C6 at $83:808C. */
    bus->wram[0x1ff1u] = 0x8eu;
    bus->wram[0x1ff2u] = 0x80u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)FieldRandom();
    input.x = (uint16_t)FieldRandom();
    input.y = (uint16_t)FieldRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[FieldRandom() & 7u];
    input.program_bank = 0x83u;
    input.carry = FieldRandom() & 1u;
    input.zero = FieldRandom() & 1u;
    input.negative = FieldRandom() & 1u;
    input.overflow = FieldRandom() & 1u;
    input.irq_disable = FieldRandom() & 1u;
    input.accumulator_is_8_bit = (FieldRandom() & 7u) ? 1u : 0u;
    input.index_is_8_bit = (FieldRandom() & 7u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2FieldTriggerUpdate(&memory, &native);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x8381c6u, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == 0x83808fu
                : pc == result.pc && reference->sp == native.stack) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL 81C6 case %u: flow=%u pc=%06X/%06X insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X PB=%02X/%02X M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u "
            "N=%u/%u V=%u/%u wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db, native.program_bank,
            reference->k, native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, native.overflow, reference->v,
            (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);

    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
        ++stats->returned;
    } else {
        unsigned k = 0;
        while (k < 5u && kFieldBoundaries[k] != result.pc)
            ++k;
        ++stats->boundary[k];
    }
    return true;
}

enum { OBJECT_SLOTS_CASES = 16384 };

typedef struct ObjectSlotsStats {
    unsigned returned;
    unsigned boundary;
    unsigned dispatches;
} ObjectSlotsStats;

static uint64_t g_object_rng = UINT64_C(0xbb67ae8584caa73b);

static uint32_t ObjectRandom(void) {
    g_object_rng ^= g_object_rng << 13;
    g_object_rng ^= g_object_rng >> 7;
    g_object_rng ^= g_object_rng << 17;
    return (uint32_t)(g_object_rng >> 32);
}

/* Whole $83:E03E from the field-loop JSR against the ROM. */
static bool RunObjectSlotsCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    ObjectSlotsStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[8] = {
        0x83u, 0x83u, 0x83u, 0x83u, 0x83u, 0x00u, 0x80u, 0x7eu};
    const uint16_t dp = dps[ObjectRandom() & 7u];
    unsigned visits = 0;
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = ObjectRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    static const uint8_t ops[] = {
        0x41, 0x43, 0x4f, 0xf6, 0xf6, 0xf2, 0xf2, 0x1a, 0x1a, 0x80,
        0xf6, 0xf2, 0x1a, 0x00, 0x33, 0x9a, 0x2f, 0x2e, 0x1d, 0x12,
        0x13, 0x22, 0x23, 0x8d, 0x72, 0x29, 0x2b, 0xc0, 0xc3, 0x84,
        0x89, 0xea, 0xf8, 0xf9, 0xe2, 0x82, 0x83, 0xeb, 0xec, 0xed,
        0x35, 0x2c, 0x25, 0xfd, 0x8f, 0x01, 0x80, 0x53, 0x28, 0xd0,
        0xfb, 0xfc, 0xe3, 0xf7, 0x20, 0xf1, 0x19, 0x8e, 0x26, 0xa0,
        0x60, 0x85, 0x8a, 0xee};
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i)
        bus->wram[i] = (ObjectRandom() % 100u) < 85u
            ? ops[ObjectRandom() % sizeof(ops)] : (uint8_t)ObjectRandom();
    for (unsigned slot = 0; slot < 32u; ++slot) {
        const uint16_t record = (uint16_t)(slot * 3u);

        if (ObjectRandom() & 1u)
            bus->wram[0x064au + slot] &= 0x7fu;
        /* Script waits expire in about a quarter of the slots. */
        bus->wram[0x1dfaeu + slot] = (ObjectRandom() & 3u)
            ? (uint8_t)(2u + ObjectRandom() % 200u) : 1u;
        Poke16(bus, 0x7fdeeeu + record,
            (uint16_t)(0x1800u + (ObjectRandom() & 0x3ffu)));
        bus->wram[0x1def0u + record] = (ObjectRandom() & 3u) ? 0x7eu : 0x00u;
        Poke16(bus, 0x7fdfceu + record,
            (uint16_t)(0x1800u + (ObjectRandom() & 0x3ffu)));
        bus->wram[0x1dfd0u + record] = 0x7eu;
        bus->wram[0x1e08eu + slot] = (uint8_t)(ObjectRandom() % 4u);
        if (ObjectRandom() & 1u)
            bus->wram[0x1e386u + slot] =
                (uint8_t)((ObjectRandom() & 0xf0u) | 1u);
        if (ObjectRandom() & 1u)
            bus->wram[0x1daecu + slot] = 0x1fu;
        bus->wram[0x1e23eu + slot] &= 0x0fu;
    }
    if (ObjectRandom() & 3u)
        bus->wram[0x09a7u] |= 0x01u;
    /* JSR $E03E at $83:8081. */
    bus->wram[0x1ff1u] = 0x83u;
    bus->wram[0x1ff2u] = 0x80u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)ObjectRandom();
    input.x = (uint16_t)ObjectRandom();
    input.y = (uint16_t)ObjectRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[ObjectRandom() & 7u];
    input.program_bank = 0x83u;
    input.carry = ObjectRandom() & 1u;
    input.zero = ObjectRandom() & 1u;
    input.negative = ObjectRandom() & 1u;
    input.overflow = ObjectRandom() & 1u;
    input.irq_disable = ObjectRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = (ObjectRandom() & 7u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2ObjectSlotsUpdate(&memory, &native);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83e03eu, &input);
    while (instructions < 64000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == 0x838084u
                : visits == result.dispatches && pc == result.pc &&
                      reference->sp == native.stack) {
            stopped = true;
            break;
        }
        if (pc == 0x83e10fu)
            ++visits;
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL E03E case %u: flow=%u pc=%06X/%06X insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u V=%u/%u "
            "wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, native.overflow, reference->v,
            (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);
    stats->dispatches += result.dispatches;
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { FIELD_NMI_CASES = 8192 };

typedef struct FieldNmiStats {
    unsigned returned;
    unsigned mmio_writes;
} FieldNmiStats;

static uint64_t g_nmi_rng = UINT64_C(0x3c6ef372fe94f82a);

static uint32_t NmiRandom(void) {
    g_nmi_rng ^= g_nmi_rng << 13;
    g_nmi_rng ^= g_nmi_rng >> 7;
    g_nmi_rng ^= g_nmi_rng << 17;
    return (uint32_t)(g_nmi_rng >> 32);
}

/* Whole $83:9FA9 from the $00:0067 JSL; MMIO order compared. */
static bool RunFieldNmiCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    FieldNmiStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x80u, 0x80u, 0x00u, 0x7eu};
    const uint16_t dp = dps[NmiRandom() & 7u];
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    SnesVerifyBusEvent *native_mmio;
    size_t native_count;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;
    bool same_mmio;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    /* Upload queues: mostly empty, a few entries. */
    for (unsigned i = 0; i < 8u; ++i) {
        if (NmiRandom() & 3u)
            Poke16(bus, 0x1236u + 2u * i + 0x10u, 0);
        if (NmiRandom() & 3u)
            Poke16(bus, 0x1236u + 2u * i, 0);
        if (NmiRandom() & 3u)
            Poke16(bus, 0x05c2u + 2u * i, 0);
    }
    if (NmiRandom() & 1u) {
        const uint8_t count = (uint8_t)(2u * (1u + NmiRandom() % 4u));
        bus->wram[0x1d4f8u] = count;
        for (unsigned i = 2; i <= count; i += 2)
            bus->wram[0x1d538u + i] = (uint8_t)(1u + NmiRandom() % 3u);
    } else {
        bus->wram[0x1d4f8u] = 0;
    }
    /* JSL $83:9FA9 at $00:0067. */
    bus->wram[0x1ff1u] = 0x69u;
    bus->wram[0x1ff2u] = 0x00u;
    bus->wram[0x1ff3u] = 0x00u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x83u;
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = (NmiRandom() & 3u) ? 1u : 0u;
    input.index_is_8_bit = (NmiRandom() & 3u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetMmio(bus);
    (void)Lufia2FieldNmiUploads(&memory, &native);
    native.stack = (uint16_t)(native.stack + 3u);       /* RTL */
    native.program_bank = 0x00u;
    native_count = bus->mmio_count;
    native_mmio = (SnesVerifyBusEvent *)malloc(
        sizeof(SnesVerifyBusEvent) * (native_count ? native_count : 1u));
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_mmio || !native_wram) {
        free(native_mmio);
        free(native_wram);
        return false;
    }
    memcpy(native_mmio, bus->mmio, sizeof(SnesVerifyBusEvent) * native_count);
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    SnesVerifyBusResetMmio(bus);
    InitInterp(reference, 0x839fa9u, &input);
    while (instructions < 400000u) {
        if (SnesVerifyPc24(reference) == 0x00006au) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }
    same_mmio = native_count == bus->mmio_count && !bus->mmio_overflow &&
        memcmp(native_mmio, bus->mmio,
            sizeof(SnesVerifyBusEvent) * native_count) == 0;

    if (!stopped || !same_mmio || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL 9FA9 case %u: stop=%d mmio=%u/%u same=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u wram@%05X\n",
            case_index, stopped ? 1 : 0, (unsigned)native_count,
            (unsigned)bus->mmio_count, same_mmio ? 1 : 0, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, (unsigned)diff);
        free(native_mmio);
        free(native_wram);
        return false;
    }
    free(native_mmio);
    free(native_wram);
    ++stats->returned;
    stats->mmio_writes += (unsigned)native_count;
    return true;
}

enum { FIELD_SCROLL_CASES = 16384 };

typedef struct FieldScrollStats {
    unsigned returned;
    unsigned boundary[2];
    unsigned mmio_writes;
} FieldScrollStats;

static const uint32_t kScrollBoundaries[2] = {0x8ebd77u, 0x8ebdd7u};

/* Camera layers: modes 0-4, some disabled or unknown. */
static void SeedFieldScroll(SnesVerifyBus *bus) {
    static const uint8_t modes[16] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 0x80u, 0x85u, 0x05u};

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    for (unsigned layer = 0; layer < 6u; layer += 2u) {
        const uint16_t x = (NmiRandom() & 7u)
            ? (uint16_t)((NmiRandom() & 0x07f0u) |
                  ((NmiRandom() & 3u) ? (1u + NmiRandom() % 15u) : 0u))
            : (NmiRandom() & 1u) ? (uint16_t)(0xeff0u | (NmiRandom() & 0x0fu))
            : (uint16_t)NmiRandom();
        const uint16_t y = (NmiRandom() & 7u)
            ? (uint16_t)((NmiRandom() & 0x07f0u) |
                  ((NmiRandom() & 3u) ? (1u + NmiRandom() % 15u) : 0u))
            : (NmiRandom() & 1u) ? (uint16_t)(0xeff0u | (NmiRandom() & 0x0fu))
            : (uint16_t)NmiRandom();

        bus->wram[0x1d020u + layer] = modes[NmiRandom() & 15u];
        Poke16(bus, 0x121eu + layer, x);
        Poke16(bus, 0x1226u + layer, y);
        if (NmiRandom() & 1u) {
            Poke16(bus, 0x7fd0ceu + layer, x);
            Poke16(bus, 0x7fd0d6u + layer, y);
        } else {
            Poke16(bus, 0x7fd0ceu + layer,
                (uint16_t)(x + NmiRandom() % 64u - 32u));
            Poke16(bus, 0x7fd0d6u + layer,
                (uint16_t)(y + NmiRandom() % 64u - 32u));
        }
        Poke16(bus, 0x7fd0deu + layer, (uint16_t)(NmiRandom() % 9u));
        Poke16(bus, 0x7fd0e6u + layer, (uint16_t)(NmiRandom() % 9u));
        /* Map size in cells; $0100 divides by zero. */
        Poke16(bus, 0x7fd010u + layer, (NmiRandom() & 7u)
            ? (uint16_t)(NmiRandom() & 0x7fu) : 0x0100u);
        Poke16(bus, 0x7fd018u + layer, (NmiRandom() & 7u)
            ? (uint16_t)(NmiRandom() & 0x7fu) : 0x0100u);
        if (NmiRandom() & 1u) {
            Poke16(bus, 0x05a4u, x);
            Poke16(bus, 0x05a6u, y);
            Poke16(bus, 0x7fd08bu, x);
            Poke16(bus, 0x7fd08du, y);
        }
    }
    if (NmiRandom() & 1u)
        bus->wram[0x1261u] &= 0xf7u;
    if (NmiRandom() & 1u)
        Poke16(bus, 0x05a8u, 0);
    else
        Poke16(bus, 0x05a8u, (uint16_t)(NmiRandom() % 9u));
    /* JSL $8E:BD77 at $83:8084. */
    bus->wram[0x1ff1u] = 0x87u;
    bus->wram[0x1ff2u] = 0x80u;
    bus->wram[0x1ff3u] = 0x83u;
}

/* Whole $8E:BD77 from the field loop's JSL. */
static bool RunFieldScrollCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    FieldScrollStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x7eu};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    BusRegisters registers;
    SnesVerifyBusEvent *native_mmio;
    size_t native_count;
    uint8_t *native_wram;
    unsigned instructions = 0;
    unsigned visits = 0;
    bool stopped = false;
    bool same_mmio;

    SeedFieldScroll(bus);
    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x8eu;
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = 1u;
    input.index_is_8_bit = (NmiRandom() & 15u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    registers = SaveBusRegisters(bus);
    native = input;
    SnesVerifyBusResetMmio(bus);
    result = Lufia2FieldScrollUpdate(&memory, &native);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */
        native.program_bank = 0x83u;
    }
    native_count = bus->mmio_count;
    native_mmio = (SnesVerifyBusEvent *)malloc(
        sizeof(SnesVerifyBusEvent) * (native_count ? native_count : 1u));
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_mmio || !native_wram) {
        free(native_mmio);
        free(native_wram);
        return false;
    }
    memcpy(native_mmio, bus->mmio, sizeof(SnesVerifyBusEvent) * native_count);
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    RestoreBusRegisters(bus, &registers);
    SnesVerifyBusResetMmio(bus);

    InitInterp(reference, 0x8ebd77u, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == 0x838088u
                : visits == result.dispatches && pc == result.pc &&
                      reference->sp == native.stack) {
            stopped = true;
            break;
        }
        if (pc == 0x8ebdd7u)
            ++visits;
        interp816_runOpcode(reference);
        ++instructions;
    }
    same_mmio = native_count == bus->mmio_count && !bus->mmio_overflow &&
        memcmp(native_mmio, bus->mmio,
            sizeof(SnesVerifyBusEvent) * native_count) == 0;

    if (!stopped || !same_mmio || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL BD77 case %u: flow=%u pc=%06X/%06X mmio=%u/%u "
            "same=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u V=%u/%u "
            "wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), (unsigned)native_count,
            (unsigned)bus->mmio_count, same_mmio ? 1 : 0, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, native.overflow, reference->v,
            (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_mmio);
        free(native_wram);
        return false;
    }
    free(native_mmio);
    free(native_wram);
    stats->mmio_writes += (unsigned)native_count;
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
        ++stats->returned;
    } else {
        for (unsigned i = 0; i < 2u; ++i)
            if (result.pc == kScrollBoundaries[i])
                ++stats->boundary[i];
    }
    return true;
}

enum { FIELD_CHILD_CASES = 16384 };

typedef struct FieldChildStats {
    unsigned returned;
    unsigned boundary;
} FieldChildStats;

typedef Lufia2ActorPrimaryUpdateResult (*FieldChildFunction)(
    const Lufia2ActorFrontendMemory *memory, Lufia2ActorFrontendCpu *cpu);

/* Idle gates mostly open; animation slots mostly off. */
static void SeedFieldChild(SnesVerifyBus *bus, uint16_t return_word) {
    static const uint16_t gates[6] = {
        0x09a8u, 0x0622u, 0x05b7u, 0x05b5u, 0x17aau, 0x099bu};
    static const uint8_t masks[6] = {0x08u, 0x88u, 0x07u, 0xa2u, 0xffu, 0x80u};

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    for (unsigned i = 0; i < 6u; ++i)
        if (NmiRandom() & 7u)
            bus->wram[gates[i]] &= (uint8_t)~masks[i];
    for (unsigned i = 0; i < 8u; ++i)
        if (NmiRandom() & 7u)
            bus->wram[0x1d057u + i] &= 0x7fu;
    bus->wram[0x1ff1u] = (uint8_t)return_word;
    bus->wram[0x1ff2u] = (uint8_t)(return_word >> 8);
}

/* Whole field-loop child from its JSR in $83:8000. */
static bool RunFieldChildCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    uint32_t entry,
    uint16_t return_word,
    FieldChildFunction function,
    FieldChildStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x00u};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;

    SeedFieldChild(bus, return_word);
    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x83u;
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = 1u;
    input.index_is_8_bit = (NmiRandom() & 7u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = function(&memory, &native);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, entry, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == (0x830000u | (uint16_t)(return_word + 1u))
                : pc == result.pc && reference->sp == native.stack) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL %06X case %u: flow=%u pc=%06X/%06X insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u wram@%05X %02X/%02X\n",
            entry, case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { FIELD_COLOUR_CASES = 16384 };

typedef struct FieldColourStats {
    unsigned returned;
    unsigned boundary;
    unsigned mmio_writes;
} FieldColourStats;

/* Palette cycle slots and a live wave table. */
static void SeedFieldColour(SnesVerifyBus *bus) {
    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    bus->wram[0x09a9u] &= 0xf8u;
    if (NmiRandom() & 3u)
        bus->wram[0x09a9u] |= 0x01u;
    if (!(NmiRandom() & 7u))
        bus->wram[0x09a9u] |= 0x02u;
    if (!(NmiRandom() & 7u))
        bus->wram[0x09a9u] |= 0x04u;
    if (NmiRandom() & 1u)
        bus->wram[0x1280u] = 1u;
    bus->wram[0x1d0f7u] = (NmiRandom() & 7u) ? (uint8_t)(1u + NmiRandom() % 6u) : 0u;
    for (unsigned i = 0; i < 12u; i += 2u)
        bus->wram[0x1ed01u + i] = (uint8_t)(1u + NmiRandom() % 3u);
    bus->wram[0x1d0cau] = (NmiRandom() & 3u) ? 0xc0u : 0xffu;
    bus->wram[0x1d0c9u] = (uint8_t)(NmiRandom() & 0x7eu);
    if (NmiRandom() & 1u) {
        const uint32_t row = 0x0c000u + bus->wram[0x1d0c9u];

        bus->wram[row + 1u] = (uint8_t)(bus->wram[0x1d0c8u] + 1u);
        if (NmiRandom() & 1u)
            bus->wram[row + 2u] = 0xffu;
    }
    /* JSR $83:AEB5 at $83:8070. */
    bus->wram[0x1ff1u] = 0x72u;
    bus->wram[0x1ff2u] = 0x80u;
}

/* Whole $83:AEB5; MMIO order compared. */
static bool RunFieldColourCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    FieldColourStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x00u};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    SnesVerifyBusEvent *native_mmio;
    size_t native_count;
    uint8_t *native_wram;
    unsigned instructions = 0;
    unsigned visits = 0;
    bool stopped = false;
    bool same_mmio;

    SeedFieldColour(bus);
    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x83u;
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = 1u;
    input.index_is_8_bit = (NmiRandom() & 7u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetMmio(bus);
    result = Lufia2FieldColourEffects(&memory, &native);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */
    native_count = bus->mmio_count;
    native_mmio = (SnesVerifyBusEvent *)malloc(
        sizeof(SnesVerifyBusEvent) * (native_count ? native_count : 1u));
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_mmio || !native_wram) {
        free(native_mmio);
        free(native_wram);
        return false;
    }
    memcpy(native_mmio, bus->mmio, sizeof(SnesVerifyBusEvent) * native_count);
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetMmio(bus);

    InitInterp(reference, 0x83aeb5u, &input);
    while (instructions < 4000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED
                ? pc == 0x838073u
                : visits == result.dispatches && pc == result.pc &&
                      reference->sp == native.stack) {
            stopped = true;
            break;
        }
        if (pc == 0x83af11u)
            ++visits;
        interp816_runOpcode(reference);
        ++instructions;
    }
    same_mmio = native_count == bus->mmio_count && !bus->mmio_overflow &&
        memcmp(native_mmio, bus->mmio,
            sizeof(SnesVerifyBusEvent) * native_count) == 0;

    if (!stopped || !same_mmio || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL AEB5 case %u: flow=%u pc=%06X/%06X mmio=%u/%u same=%d "
            "insns=%u A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X C=%u/%u Z=%u/%u N=%u/%u wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), (unsigned)native_count,
            (unsigned)bus->mmio_count, same_mmio ? 1 : 0, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_mmio);
        free(native_wram);
        return false;
    }
    free(native_mmio);
    free(native_wram);
    stats->mmio_writes += (unsigned)native_count;
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { SECONDARY_UPDATE_CASES = 16384 };

/* Whole $83:D508 against the ROM, random machine and scripts. */
static bool RunSecondaryUpdateCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    PrimaryUpdateStats *stats) {
    static const uint16_t dps[4] = {0x0000u, 0x0020u, 0x0000u, 0x0400u};
    static const uint8_t banks[4] = {0x00u, 0x7eu, 0x80u, 0x83u};
    static const uint8_t known[] = {
        0xf0, 0xf0, 0xf0, 0xf0, 0xf1, 0xf9, 0xfa, 0xfb, 0xfc, 0xfe,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe9, 0xea, 0xec,
        0xed, 0xee, 0xef, 0xd0, 0xd1, 0xd5, 0xd6, 0xd7, 0x13, 0x22,
        0x31, 0x55, 0x74, 0x92, 0x93, 0xa1, 0xb2, 0x00, 0x43, 0x01,
        0x83, 0xc1, 0xc2, 0xf2, 0xf5, 0xf6, 0xf7, 0xf8, 0xfd, 0xff,
        0xeb, 0xd4};
    const uint16_t dp = dps[WholeRandom() & 3u];
    const uint8_t slot = (uint8_t)(WholeRandom() % 40u);
    const uint16_t record = (uint16_t)(slot * 3u);
    const uint16_t script = (uint16_t)(0x1800u + (WholeRandom() & 0x3ffu));
    const uint32_t exits[2] = {0x83d599u, 0x83d60eu};
    Lufia2ActorFrontendCpu input;
    Lufia2ActorFrontendCpu native;
    NativeMemory native_context = {bus};
    Lufia2ActorFrontendMemory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ActorPrimaryUpdateResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    unsigned dispatches = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = WholeRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    for (uint16_t i = 0x1800u; i < 0x1f00u; ++i)
        bus->wram[i] = (WholeRandom() % 100u) < 80u
            ? known[WholeRandom() % sizeof(known)] : (uint8_t)WholeRandom();
    /* Sparse collision map, walk state near real values. */
    for (unsigned i = 0; i < 0x5000u; ++i)
        if (WholeRandom() & 3u)
            bus->wram[0x4000u + i] = 0;
    bus->wram[0x05b9u] = (uint8_t)(0x20u + (WholeRandom() & 0x10u));
    Poke16(bus, 0x05aau, 0);
    Poke16(bus, 0x7fd008u, 0);
    Poke16(bus, dp + 0x00a7u, slot);
    Poke16(bus, dp + 0x00a9u, (uint16_t)(slot * 2u));
    Poke16(bus, dp + 0x00abu, record);
    Poke16(bus, 0x7fe3eeu + record, script);
    bus->wram[0x1e3f0u + record] = (WholeRandom() & 3u) ? 0x7eu : 0x00u;
    if (WholeRandom() & 3u)
        bus->wram[0x0622u + slot] |= 0x80u;
    if (WholeRandom() & 1u)
        bus->wram[0x0622u + slot] &= (uint8_t)~0x0au;
    if (WholeRandom() & 1u)
        bus->wram[0x0692u + slot] = (uint8_t)((WholeRandom() & 3u) * 2u);
    if (WholeRandom() & 1u)
        bus->wram[0x1e48eu + slot] = (uint8_t)(WholeRandom() & 0x0fu);
    bus->wram[0x1e4deu + slot] = (uint8_t)(1u << (WholeRandom() & 3u));
    if ((WholeRandom() & 3u) == 0) {
        bus->wram[0x070au + slot] = 3;
        bus->wram[0x09a6u] &= 0xfeu;
    }
    if (WholeRandom() & 1u)
        bus->wram[0x1e4b6u + slot] = (uint8_t)(WholeRandom() & 0x0fu);
    if (WholeRandom() & 1u)
        bus->wram[0x1e216u + slot] = (uint8_t)(1u + (WholeRandom() & 1u));
    if (WholeRandom() & 1u)
        bus->wram[0x1dc8cu + slot * 2u] = 0;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)WholeRandom();
    input.x = (uint16_t)WholeRandom();
    input.y = (uint16_t)WholeRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dp;
    input.data_bank = banks[WholeRandom() & 3u];
    input.program_bank = 0x83u;
    input.carry = WholeRandom() & 1u;
    input.zero = WholeRandom() & 1u;
    input.negative = WholeRandom() & 1u;
    input.overflow = WholeRandom() & 1u;
    input.irq_disable = WholeRandom() & 1u;
    input.accumulator_is_8_bit = 1;
    input.index_is_8_bit = (WholeRandom() & 3u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2ActorSecondaryUpdate(&memory, &native);

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83d508u, &input);
    while (instructions < 4000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
            if (pc == exits[0] || pc == exits[1]) {
                stopped = pc == result.pc;
                break;
            }
        } else if (dispatches == result.dispatches && pc == result.pc &&
                   reference->sp == native.stack) {
            stopped = true;
            break;
        }
        if (pc == 0x83d5d1u)
            ++dispatches;
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL D508 case %u: flow=%u pc=%06X/%06X disp=%u/%u "
            "insns=%u A=%04X/%04X X=%04X/%04X Y=%04X/%04X "
            "S=%04X/%04X DB=%02X/%02X M=%u/%u Xf=%u/%u C=%u/%u "
            "V=%u/%u wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, result.pc,
            SnesVerifyPc24(reference), result.dispatches, dispatches,
            instructions, native.accumulator, reference->a,
            native.x, reference->x, native.y, reference->y,
            native.stack, reference->sp, native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.overflow, reference->v,
            (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);

    stats->dispatches += result.dispatches;
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_RETURNED) {
        if (result.pc == exits[0])
            ++stats->returned_c83b;
        else
            ++stats->returned_c8d3;
    } else {
        ++stats->boundary_handler;
    }
    return true;
}

static const struct {
    uint32_t pc;
    const char *name;
    bool low_script;
} kGenericHandlers[] = {
    {0x83d320u, "D320", false}, {0x83d340u, "D340", false},
    {0x83d125u, "D125", false}, {0x83d132u, "D132", false},
    {0x83ccf0u, "CCF0", false}, {0x83cd0du, "CD0D", false},
    {0x83cd2eu, "CD2E", false}, {0x83cd32u, "CD32", false},
    {0x83cd4fu, "CD4F", false}, {0x83cd92u, "CD92", false},
    {0x83ce73u, "CE73", false}, {0x83cab9u, "CAB9", false},
    {0x83d176u, "D176", true}, {0x83d188u, "D188", true},
    {0x83d196u, "D196", true}, {0x83d1c1u, "D1C1", true},
    {0x83d1d0u, "D1D0", true}, {0x83d1e6u, "D1E6", true},
    {0x83d210u, "D210", true}, {0x83d293u, "D293", true},
    {0x83d2e6u, "D2E6", true}, {0x83d2f6u, "D2F6", true},
    {0x83d30bu, "D30B", true},
    {0x83c98au, "C98A", true}, {0x83ccd7u, "CCD7", true},
    {0x83d01eu, "D01E", true}, {0x83cf6eu, "CF6E", true},
    {0x83cf8cu, "CF8C", true}, {0x83cfb9u, "CFB9", true},
    {0x83d112u, "D112", true}, {0x83d09au, "D09A", true},
    {0x83ca19u, "CA19", true},
    {0x83cda5u, "CDA5", true}, {0x83ce7du, "CE7D", true},
    {0x83cf1au, "CF1A", true}, {0x83d03fu, "D03F", true},
    {0x83c918u, "C918", true}, {0x83cad3u, "CAD3", true},
    {0x83cbb1u, "CBB1", true}, {0x83ca29u, "CA29", true},
    {0x83d135u, "D135", true}, {0x83d1b5u, "D1B5", true},
    {0x83d1feu, "D1FE", true}, {0x83d207u, "D207", true},
};
enum {
    GENERIC_HANDLER_COUNT =
        sizeof(kGenericHandlers) / sizeof(kGenericHandlers[0]),
};

int interp816_opcode_hook(uint32_t address) {
    (void)address;
    return 0;
}

int main(int argc, char **argv) {
    const char *rom_path;
    uint8_t *rom = NULL;
    uint8_t *initial = NULL;
    size_t rom_size = 0;
    SnesVerifyBus bus;
    Interp816 *reference = NULL;
    unsigned primary_passed = 0;
    unsigned secondary_passed = 0;
    unsigned commit_tail_passed = 0;
    unsigned jump_handler_passed = 0;
    unsigned mask_or_passed = 0;
    unsigned mask_and_passed = 0;
    unsigned jump_tail_alias_passed = 0;
    unsigned coordinate_x_passed = 0;
    unsigned coordinate_y_passed = 0;
    unsigned d14d_passed = 0;
    unsigned action_core_passed = 0;
    unsigned action_core_x8_passed = 0;
    ActionCoreX8Stats action_core_x8 = {0, 0, 0};
    unsigned player_standard_passed = 0;
    PlayerStandardStats player_standard;
    unsigned actor_slots_passed = 0;
    ActorSlotsStats actor_slots = {0, 0, 0};
    unsigned field_trigger_passed = 0;
    FieldTriggerStats field_trigger;
    unsigned object_slots_passed = 0;
    ObjectSlotsStats object_slots = {0, 0, 0};
    unsigned field_nmi_passed = 0;
    FieldNmiStats field_nmi = {0, 0};
    unsigned field_scroll_passed = 0;
    FieldScrollStats field_scroll;
    unsigned field_idle_passed = 0;
    FieldChildStats field_idle = {0, 0};
    unsigned field_ticks_passed = 0;
    FieldChildStats field_ticks = {0, 0};
    unsigned field_colour_passed = 0;
    FieldColourStats field_colour = {0, 0, 0};
    unsigned movement_step_passed = 0;
    unsigned map_offset_passed = 0;
    unsigned map_value_passed = 0;
    unsigned fixed_action_handler_passed = 0;
    unsigned operand_action_handler_passed = 0;
    unsigned install_script_passed = 0;
    unsigned timer_store_passed = 0;
    unsigned flag_set_passed = 0;
    unsigned flag_clear_passed = 0;
    unsigned map_flag_passed = 0;
    unsigned leader_radius_passed = 0;
    unsigned leader_equal_x_passed = 0;
    unsigned leader_equal_y_passed = 0;
    unsigned leader_step_x_passed = 0;
    unsigned leader_step_y_passed = 0;
    unsigned random_scale_passed = 0;
    unsigned random_timer_passed = 0;
    unsigned random_timer_scaled_passed = 0;
    unsigned random_byte_passed = 0;
    unsigned whole_passed[WHOLE_FUNCTION_COUNT] = {0};
    unsigned primary_update_passed = 0;
    unsigned secondary_update_passed = 0;
    PrimaryUpdateStats secondary_update_stats;
    unsigned resume_passed[2] = {0, 0};
    PrimaryUpdateStats primary_update_stats;
    unsigned generic_passed[GENERIC_HANDLER_COUNT] = {0};
    GenericHandlerStats generic_stats[GENERIC_HANDLER_COUNT];
    unsigned failed = 0;
    unsigned case_index = 0;
    bool bus_initialized = false;

    memset(generic_stats, 0, sizeof(generic_stats));
    memset(&primary_update_stats, 0, sizeof(primary_update_stats));
    memset(&secondary_update_stats, 0, sizeof(secondary_update_stats));

    if (argc < 2) {
        fprintf(stderr, "usage: %s <lufia2.sfc>\n", argv[0]);
        return 2;
    }
    rom_path = argv[1];

    printf("Lufia II actor script-dispatch verifier\n");
    printf("=======================================\n\n");

    if (!SnesVerifyLoadFile(rom_path, &rom, &rom_size) ||
        rom_size != LUFIA2_ROM_SIZE ||
        !SnesVerifyBusInit(&bus, rom, rom_size)) {
        fprintf(stderr,
            "ERROR: reference ROM/bus initialization failed\n");
        failed = 1;
        goto done;
    }
    bus_initialized = true;
    initial = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    reference =
        interp816_init(&bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!initial || !reference) {
        fprintf(stderr, "ERROR: verifier allocation failed\n");
        failed = 1;
        goto done;
    }

    for (unsigned opcode = 0;
         opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < PRIMARY_CASES_PER_OPCODE && failed < 20;
             ++variant, ++case_index) {
            if (RunPrimaryCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, case_index))
                ++primary_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned opcode = 0;
         opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < SECONDARY_CASES_PER_OPCODE && failed < 20;
             ++variant, ++case_index) {
            if (RunSecondaryCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, case_index))
                ++secondary_passed;
            else
                ++failed;
        }
    }


    case_index = 0;
    for (unsigned value = 0; value < 256 && failed < 20; ++value) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunCommitTailCase(
                    &bus, reference, initial, variant,
                    (uint8_t)value, case_index))
                ++commit_tail_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned opcode = 0; opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunJumpHandlerCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, case_index))
                ++jump_handler_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned opcode = 0; opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunMaskHandlerCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, 0x83d2c4u,
                    case_index, "D2C4"))
                ++mask_or_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned opcode = 0; opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunMaskHandlerCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, 0x83d2d5u,
                    case_index, "D2D5"))
                ++mask_and_passed;
            else
                ++failed;
        }
    }


    case_index = 0;
    for (unsigned opcode = 0; opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunJumpTailAliasCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, case_index))
                ++jump_tail_alias_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned opcode = 0; opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunCoordinateHandlerCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, 0x83cc85u, 0x06bau,
                    case_index, "CC85"))
                ++coordinate_x_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned opcode = 0; opcode < 256 && failed < 20; ++opcode) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunCoordinateHandlerCase(
                    &bus, reference, initial, variant,
                    (uint8_t)opcode, 0x83cca3u, 0x06e2u,
                    case_index, "CCA3"))
                ++coordinate_y_passed;
            else
                ++failed;
        }
    }

    case_index = 0;
    for (unsigned value = 0; value < 256 && failed < 20; ++value) {
        for (unsigned variant = 0;
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunD14DCase(
                    &bus, reference, initial, variant,
                    (uint8_t)value, case_index))
                ++d14d_passed;
            else
                ++failed;
        }
    }


    case_index = 0;
    for (unsigned action = 0; action < 256 && failed < 20; ++action) {
        for (unsigned variant = 0;
             variant < ACTION_CORE_VARIANTS && failed < 20;
             ++variant, ++case_index) {
            if (RunActionCoreCase(
                    &bus, reference, initial, variant,
                    (uint8_t)action, case_index))
                ++action_core_passed;
            else
                ++failed;
        }
    }
    for (unsigned i = 0; i < ACTION_CORE_X8_CASES && failed < 20; ++i) {
        if (RunActionCoreX8Case(&bus, reference, initial, i, &action_core_x8))
            ++action_core_x8_passed;
        else
            ++failed;
    }
    memset(&player_standard, 0, sizeof(player_standard));
    for (unsigned i = 0; i < PLAYER_STANDARD_CASES && failed < 20; ++i) {
        if (RunPlayerStandardCase(
                &bus, reference, initial, i, &player_standard))
            ++player_standard_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < ACTOR_SLOTS_CASES && failed < 20; ++i) {
        if (RunActorSlotsCase(&bus, reference, initial, i, &actor_slots))
            ++actor_slots_passed;
        else
            ++failed;
    }
    memset(&field_trigger, 0, sizeof(field_trigger));
    for (unsigned i = 0; i < FIELD_TRIGGER_CASES && failed < 20; ++i) {
        if (RunFieldTriggerCase(&bus, reference, initial, i, &field_trigger))
            ++field_trigger_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < OBJECT_SLOTS_CASES && failed < 20; ++i) {
        if (RunObjectSlotsCase(&bus, reference, initial, i, &object_slots))
            ++object_slots_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < FIELD_NMI_CASES && failed < 20; ++i) {
        if (RunFieldNmiCase(&bus, reference, initial, i, &field_nmi))
            ++field_nmi_passed;
        else
            ++failed;
    }
    memset(&field_scroll, 0, sizeof(field_scroll));
    for (unsigned i = 0; i < FIELD_SCROLL_CASES && failed < 20; ++i) {
        if (RunFieldScrollCase(&bus, reference, initial, i, &field_scroll))
            ++field_scroll_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < FIELD_CHILD_CASES && failed < 20; ++i) {
        if (RunFieldChildCase(&bus, reference, initial, i, 0x8380cdu,
                0x80bdu, Lufia2FieldIdleTest, &field_idle))
            ++field_idle_passed;
        else
            ++failed;
        if (RunFieldChildCase(&bus, reference, initial, i, 0x838682u,
                0x807cu, Lufia2FieldAnimationTicks, &field_ticks))
            ++field_ticks_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < FIELD_COLOUR_CASES && failed < 20; ++i) {
        if (RunFieldColourCase(&bus, reference, initial, i, &field_colour))
            ++field_colour_passed;
        else
            ++failed;
    }
    if (field_scroll.returned < FIELD_SCROLL_CASES / 8u) {
        fprintf(stderr, "FAIL BD77 too few returned cases: %u\n",
            field_scroll.returned);
        ++failed;
    }
    if (actor_slots.returned < ACTOR_SLOTS_CASES / 2u) {
        fprintf(stderr, "FAIL BB93 too few terminated cases: %u\n",
            actor_slots.returned);
        ++failed;
    }


    case_index = 0;
    for (unsigned i = 0;
         i < MOVEMENT_HELPER_CASES && failed < 20;
         ++i, ++case_index) {
        if (RunMovementStepCase(
                &bus, reference, initial, i))
            ++movement_step_passed;
        else
            ++failed;
    }

    case_index = 0;
    for (unsigned i = 0;
         i < MOVEMENT_HELPER_CASES && failed < 20;
         ++i, ++case_index) {
        if (RunMapOffsetCase(
                &bus, reference, initial, i))
            ++map_offset_passed;
        else
            ++failed;
    }

    case_index = 0;
    for (unsigned i = 0;
         i < MOVEMENT_HELPER_CASES && failed < 20;
         ++i, ++case_index) {
        if (RunMapValueCase(
                &bus, reference, initial, i))
            ++map_value_passed;
        else
            ++failed;
    }


    case_index = 0;
    for (unsigned i = 0;
         i < FIXED_ACTION_HANDLER_CASES && failed < 20;
         ++i, ++case_index) {
        if (RunFixedActionHandlerCase(
                &bus, reference, initial, i))
            ++fixed_action_handler_passed;
        else
            ++failed;
    }

    case_index = 0;
    for (unsigned i = 0;
         i < OPERAND_ACTION_HANDLER_CASES && failed < 20;
         ++i, ++case_index) {
        if (RunOperandActionHandlerCase(
                &bus, reference, initial, i))
            ++operand_action_handler_passed;
        else
            ++failed;
    }

#define RUN_VALUE_CASES(counter, call)                                   \
    case_index = 0;                                                      \
    for (unsigned value = 0; value < 256 && failed < 20; ++value) {      \
        for (unsigned variant = 0;                                       \
             variant < KNOWN_HANDLER_VARIANTS && failed < 20;            \
             ++variant, ++case_index) {                                  \
            if (call)                                                    \
                ++counter;                                               \
            else                                                         \
                ++failed;                                                \
        }                                                                \
    }

    RUN_VALUE_CASES(install_script_passed,
        RunInstallScriptHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, case_index))
    RUN_VALUE_CASES(timer_store_passed,
        RunTimerStoreHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, case_index))
    RUN_VALUE_CASES(flag_set_passed,
        RunFlagBitHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, 0x83c8fcu, case_index, "C8FC"))
    RUN_VALUE_CASES(flag_clear_passed,
        RunFlagBitHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, 0x83c90au, case_index, "C90A"))
    RUN_VALUE_CASES(map_flag_passed,
        RunMapFlagHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, case_index))
    RUN_VALUE_CASES(leader_equal_x_passed,
        RunLeaderEqualHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, 0x83cc1bu, 0x06bau, case_index, "CC1B"))
    RUN_VALUE_CASES(leader_equal_y_passed,
        RunLeaderEqualHandlerCase(&bus, reference, initial, variant,
            (uint8_t)value, 0x83cc2eu, 0x06e2u, case_index, "CC2E"))
#undef RUN_VALUE_CASES

    for (case_index = 0;
         case_index < LEADER_RADIUS_CASES && failed < 20; ++case_index) {
        if (RunLeaderRadiusHandlerCase(
                &bus, reference, initial, case_index))
            ++leader_radius_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < LEADER_STEP_CASES && failed < 20; ++case_index) {
        if (RunLeaderStepHandlerCase(
                &bus, reference, initial, case_index, true))
            ++leader_step_x_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < LEADER_STEP_CASES && failed < 20; ++case_index) {
        if (RunLeaderStepHandlerCase(
                &bus, reference, initial, case_index, false))
            ++leader_step_y_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < RANDOM_SCALE_CASES && failed < 20; ++case_index) {
        if (RunRandomCase(&bus, reference, initial, case_index, false))
            ++random_scale_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < RANDOM_TIMER_CASES && failed < 20; ++case_index) {
        if (RunRandomTimerHandlerCase(
                &bus, reference, initial, case_index, false))
            ++random_timer_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < RANDOM_TIMER_CASES && failed < 20; ++case_index) {
        if (RunRandomTimerHandlerCase(
                &bus, reference, initial, case_index, true))
            ++random_timer_scaled_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < RANDOM_SCALE_CASES && failed < 20; ++case_index) {
        if (RunRandomCase(&bus, reference, initial, case_index, true))
            ++random_byte_passed;
        else
            ++failed;
    }

    for (case_index = 0;
         case_index < SECONDARY_UPDATE_CASES && failed < 20; ++case_index) {
        if (RunSecondaryUpdateCase(
                &bus, reference, initial, case_index,
                &secondary_update_stats))
            ++secondary_update_passed;
        else
            ++failed;
    }

    for (unsigned k = 0; k < 2u && failed < 20; ++k) {
        for (case_index = 0;
             case_index < RESUME_EXACT_CASES && failed < 20; ++case_index) {
            if (RunResumeExactCase(
                    &bus, reference, initial, case_index, k == 1u))
                ++resume_passed[k];
            else
                ++failed;
        }
    }

    for (case_index = 0;
         case_index < PRIMARY_UPDATE_CASES && failed < 20; ++case_index) {
        if (RunPrimaryUpdateCase(
                &bus, reference, initial, case_index,
                &primary_update_stats))
            ++primary_update_passed;
        else
            ++failed;
    }

    for (size_t f = 0; f < WHOLE_FUNCTION_COUNT && failed < 20; ++f) {
        for (case_index = 0;
             case_index < WHOLE_FUNCTION_CASES && failed < 20;
             ++case_index) {
            if (RunWholeFunctionCase(
                    &bus, reference, initial, &kWholeFunctions[f],
                    case_index))
                ++whole_passed[f];
            else
                ++failed;
        }
    }

    for (size_t h = 0; h < GENERIC_HANDLER_COUNT && failed < 20; ++h) {
        for (case_index = 0;
             case_index < GENERIC_HANDLER_CASES && failed < 20;
             ++case_index) {
            if (RunGenericHandlerCase(
                    &bus, reference, initial,
                    kGenericHandlers[h].pc, case_index,
                    kGenericHandlers[h].name,
                    kGenericHandlers[h].low_script, &generic_stats[h]))
                ++generic_passed[h];
            else
                ++failed;
        }
    }

    printf("$83:C83C primary dispatch cases passed:   %u / %u\n",
        primary_passed, 256u * PRIMARY_CASES_PER_OPCODE);
    printf("$83:D59A secondary dispatch cases passed: %u / %u\n",
        secondary_passed, 256u * SECONDARY_CASES_PER_OPCODE);
    printf("$83:C8C7 cursor-commit cases passed:       %u / %u\n",
        commit_tail_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:D2B4 script-jump cases passed:         %u / %u\n",
        jump_handler_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:D2C4 mask-OR cases passed:             %u / %u\n",
        mask_or_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:D2D5 mask-AND cases passed:            %u / %u\n",
        mask_and_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:D2BD alias-tail cases passed:          %u / %u\n",
        jump_tail_alias_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:CC85 X-range cases passed:             %u / %u\n",
        coordinate_x_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:CCA3 Y-range cases passed:             %u / %u\n",
        coordinate_y_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:D14D conditional cases passed:         %u / %u\n",
        d14d_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:D350 action-core cases passed:         %u / %u\n",
        action_core_passed, 256u * ACTION_CORE_VARIANTS);
    printf("$83:D350 X8 (C1B4) cases passed:           %u / %u "
        "(return %u, install %u, LLE boundary %u)\n",
        action_core_x8_passed, ACTION_CORE_X8_CASES,
        action_core_x8.returned, action_core_x8.installed,
        action_core_x8.boundary);
    printf("$83:C1B4 whole-function cases passed:      %u / %u "
        "(RTS C1E2 %u, RTS C245 %u; LLE C1CC %u, C1D7 %u, C1DC %u, "
        "8E:BBD1 %u, 8E:B6D4 %u, D38D %u, D370 %u, FBC2 %u, BA1C %u, "
        "FB17 %u, other %u)\n",
        player_standard_passed, PLAYER_STANDARD_CASES,
        player_standard.early, player_standard.walked,
        player_standard.boundary[0], player_standard.boundary[1],
        player_standard.boundary[2], player_standard.boundary[3],
        player_standard.boundary[4], player_standard.boundary[5],
        player_standard.boundary[6], player_standard.boundary[7],
        player_standard.boundary[8], player_standard.boundary[9],
        player_standard.boundary[10]);
    printf("$83:BB93 whole-function cases passed:      %u / %u "
        "(RTL %u, LLE BBA1 %u, child never returned %u)\n",
        actor_slots_passed, ACTOR_SLOTS_CASES, actor_slots.returned,
        actor_slots.boundary, actor_slots.unterminated);
    printf("$83:81C6 whole-function cases passed:      %u / %u "
        "(RTS 829F %u; LLE 80:CBCC %u, 80:CC0E %u, B96D %u, 8225 %u, "
        "8251 %u, other %u)\n",
        field_trigger_passed, FIELD_TRIGGER_CASES, field_trigger.returned,
        field_trigger.boundary[0], field_trigger.boundary[1],
        field_trigger.boundary[2], field_trigger.boundary[3],
        field_trigger.boundary[4], field_trigger.boundary[5]);
    printf("$83:E03E whole-function cases passed:      %u / %u "
        "(RTS E0FB %u, LLE boundary %u, object VM dispatches %u)\n",
        object_slots_passed, OBJECT_SLOTS_CASES, object_slots.returned,
        object_slots.boundary, object_slots.dispatches);
    printf("$83:9FA9 whole-function cases passed:      %u / %u "
        "(RTL %u, MMIO writes compared %u)\n",
        field_nmi_passed, FIELD_NMI_CASES, field_nmi.returned,
        field_nmi.mmio_writes);
    printf("$83:80CD whole-function cases passed:      %u / %u "
        "(RTS %u, LLE entry %u)\n",
        field_idle_passed, FIELD_CHILD_CASES, field_idle.returned,
        field_idle.boundary);
    printf("$83:8682 whole-function cases passed:      %u / %u "
        "(RTS %u, LLE 86D6/86DB %u)\n",
        field_ticks_passed, FIELD_CHILD_CASES, field_ticks.returned,
        field_ticks.boundary);
    printf("$83:AEB5 whole-function cases passed:      %u / %u "
        "(RTS %u, LLE %u, MMIO writes compared %u)\n",
        field_colour_passed, FIELD_COLOUR_CASES, field_colour.returned,
        field_colour.boundary, field_colour.mmio_writes);
    printf("$8E:BD77 whole-function cases passed:      %u / %u "
        "(RTL %u, LLE BD77 %u, BDD7 %u, MMIO writes compared %u)\n",
        field_scroll_passed, FIELD_SCROLL_CASES, field_scroll.returned,
        field_scroll.boundary[0], field_scroll.boundary[1],
        field_scroll.mmio_writes);
    printf("$83:FB12 movement-step cases passed:       %u / %u\n",
        movement_step_passed, MOVEMENT_HELPER_CASES);
    printf("$83:F9D4 map-offset cases passed:          %u / %u\n",
        map_offset_passed, MOVEMENT_HELPER_CASES);
    printf("$83:FB71 map-value cases passed:           %u / %u\n",
        map_value_passed, MOVEMENT_HELPER_CASES);
    printf("primary fixed-action handler cases:       %u / %u\n",
        fixed_action_handler_passed, FIXED_ACTION_HANDLER_CASES);
    printf("$83:C877 operand-action cases passed:      %u / %u\n",
        operand_action_handler_passed, OPERAND_ACTION_HANDLER_CASES);
    printf("$83:C891 install-script cases passed:      %u / %u\n",
        install_script_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:C8EE timer-store cases passed:         %u / %u\n",
        timer_store_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:C8FC flag-set cases passed:            %u / %u\n",
        flag_set_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:C90A flag-clear cases passed:          %u / %u\n",
        flag_clear_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:CBB7 map-flag cases passed:            %u / %u\n",
        map_flag_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:CBE1 leader-radius cases passed:       %u / %u\n",
        leader_radius_passed, LEADER_RADIUS_CASES);
    printf("$83:CC1B leader-X-equal cases passed:      %u / %u\n",
        leader_equal_x_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:CC2E leader-Y-equal cases passed:      %u / %u\n",
        leader_equal_y_passed, 256u * KNOWN_HANDLER_VARIANTS);
    printf("$83:CC41 leader-X-step cases passed:       %u / %u\n",
        leader_step_x_passed, LEADER_STEP_CASES);
    printf("$83:CC63 leader-Y-step cases passed:       %u / %u\n",
        leader_step_y_passed, LEADER_STEP_CASES);
    printf("$80:8299 random-scale cases passed:        %u / %u\n",
        random_scale_passed, RANDOM_SCALE_CASES);
    printf("$83:C8AF random-timer cases passed:        %u / %u\n",
        random_timer_passed, RANDOM_TIMER_CASES);
    printf("$83:C8D4 random-timer x8 cases passed:     %u / %u\n",
        random_timer_scaled_passed, RANDOM_TIMER_CASES);
    printf("$80:82C7 random-byte cases passed:         %u / %u\n",
        random_byte_passed, RANDOM_SCALE_CASES);
    printf("$83:D508 whole-function cases passed:      %u / %u"
           " (RTS D599 %u, RTS D60E %u, LLE boundary %u, dispatches %u)\n",
        secondary_update_passed, SECONDARY_UPDATE_CASES,
        secondary_update_stats.returned_c83b,
        secondary_update_stats.returned_c8d3,
        secondary_update_stats.boundary_handler,
        secondary_update_stats.dispatches);
    printf("$83:D370 resume-exact cases passed:        %u / %u\n",
        resume_passed[0], RESUME_EXACT_CASES);
    printf("$83:FB17 resume-exact cases passed:        %u / %u\n",
        resume_passed[1], RESUME_EXACT_CASES);
    printf("$83:C7F8 whole-function cases passed:      %u / %u"
           " (RTS C83B %u, RTS C8D3 %u, LLE boundary %u, dispatches %u)\n",
        primary_update_passed, PRIMARY_UPDATE_CASES,
        primary_update_stats.returned_c83b,
        primary_update_stats.returned_c8d3,
        primary_update_passed - primary_update_stats.returned_c83b -
            primary_update_stats.returned_c8d3,
        primary_update_stats.dispatches);
    for (size_t f = 0; f < WHOLE_FUNCTION_COUNT; ++f)
        printf("$%s whole-function cases passed:      %u / %u\n",
            kWholeFunctions[f].name, whole_passed[f],
            WHOLE_FUNCTION_CASES);
    for (size_t h = 0; h < GENERIC_HANDLER_COUNT; ++h)
        printf("$83:%s seeded cases passed:             %u / %u"
               " (redispatch %u, commit %u, boundary %u)\n",
            kGenericHandlers[h].name, generic_passed[h],
            GENERIC_HANDLER_CASES, generic_stats[h].redispatched,
            generic_stats[h].committed, generic_stats[h].boundaries);
    printf("failures: %u\n", failed);
    printf(failed
        ? "RESULT: FAIL - actor dispatch mismatch found\n"
        : "RESULT: PASS - actor dispatch prefixes matched original ROM\n");

done:
    if (reference)
        interp816_free(reference);
    free(initial);
    if (bus_initialized)
        SnesVerifyBusDestroy(&bus);
    free(rom);
    return failed ? 1 : 0;
}
