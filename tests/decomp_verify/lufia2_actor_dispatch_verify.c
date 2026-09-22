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
    cpu->v = false;
    cpu->d = false;
    cpu->i = false;
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
            reference, &redispatch_pc, 1, 128, &to_redispatch);
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
            reference, &stop_pc, 1, 128, &to_target);
        instructions += to_target;
    }

    if (stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL %s case %u: stop=%d A=%04X/%04X "
            "X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "DB=%02X/%02X M=%u/%u Xf=%u/%u\n",
            name, case_index, stop,
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
    const bool needs_child = variant == 3u;
    const uint32_t stop_pc =
        needs_child ? 0x83d166u : 0x83c8d2u;
    const Lufia2ActorPrimaryScriptStepFlow flow =
        needs_child
            ? LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_D166
            : LUFIA2_ACTOR_PRIMARY_SCRIPT_CONTINUE_C8D2;

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
    const uint16_t layer_index =
        (uint16_t)((case_index * 2u) & 0x003eu);
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
    unsigned movement_step_passed = 0;
    unsigned map_offset_passed = 0;
    unsigned map_value_passed = 0;
    unsigned failed = 0;
    unsigned case_index = 0;
    bool bus_initialized = false;

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
    printf("$83:FB12 movement-step cases passed:       %u / %u\n",
        movement_step_passed, MOVEMENT_HELPER_CASES);
    printf("$83:F9D4 map-offset cases passed:          %u / %u\n",
        map_offset_passed, MOVEMENT_HELPER_CASES);
    printf("$83:FB71 map-value cases passed:           %u / %u\n",
        map_value_passed, MOVEMENT_HELPER_CASES);
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
