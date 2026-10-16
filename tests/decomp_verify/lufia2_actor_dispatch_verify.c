#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interp816.h"
#include "cpu_state.h"
#include "lufia2/decomp.h"
#include "snes_function_verify.h"
#include "lufia2_level_up_seed.h"
#include "lufia2_inventory_seed.h"
#include "text/text_internal.h"
#include "core/cpu_internal.h"

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

typedef enum SmallBridgeFlow {
    SMALL_BRIDGE_NONE,
    SMALL_BRIDGE_DISPATCH,
    SMALL_BRIDGE_TAIL,
    SMALL_BRIDGE_REWRITTEN,
    SMALL_BRIDGE_UNEXPECTED_CHILD,
} SmallBridgeFlow;

static SnesVerifyBus *g_bridge_bus;
static struct {
    SmallBridgeFlow flow;
    uint32_t target;
    uint32_t site;
    uint16_t restore_s;
    uint16_t entry_s;
    uint8_t hrv;
    unsigned calls;
} g_small_bridge;

uint8 cpu_read8(CpuState *cpu, uint8 bank, uint16 address) {
    const uint8 value = SnesVerifyBusRead(
        g_bridge_bus, ((uint32_t)bank << 16) | address);
    cpu->open_bus = value;
    return value;
}

void cpu_write8(CpuState *cpu, uint8 bank, uint16 address, uint8 value) {
    SnesVerifyBusWrite(g_bridge_bus, ((uint32_t)bank << 16) | address, value);
    cpu->open_bus = value;
}

int cpu_dispatch_has_entry(CpuState *cpu, uint32 pc24) {
    (void)cpu;
    (void)pc24;
    return 0;
}

RecompReturn cpu_dispatch_pc_from(
    CpuState *cpu, uint32 pc24, uint16 restore_s, uint32 site) {
    (void)cpu;
    ++g_small_bridge.calls;
    g_small_bridge.flow = SMALL_BRIDGE_DISPATCH;
    g_small_bridge.target = pc24;
    g_small_bridge.site = site;
    g_small_bridge.restore_s = restore_s;
    return (RecompReturn)111;
}

RecompReturn interp_tier_dispatch_rewritten_return(
    CpuState *cpu, uint32 pc24, uint32 site) {
    (void)cpu;
    ++g_small_bridge.calls;
    g_small_bridge.flow = SMALL_BRIDGE_REWRITTEN;
    g_small_bridge.target = pc24;
    g_small_bridge.site = site;
    return (RecompReturn)111;
}

RecompReturn interp_tier_dispatch_tail(
    CpuState *cpu, uint32 pc24, uint32 site, uint16 entry_s, uint8 hrv) {
    (void)cpu;
    ++g_small_bridge.calls;
    g_small_bridge.flow = SMALL_BRIDGE_TAIL;
    g_small_bridge.target = pc24;
    g_small_bridge.site = site;
    g_small_bridge.entry_s = entry_s;
    g_small_bridge.hrv = hrv;
    return (RecompReturn)111;
}

RecompReturn cpu_dispatch_call_pc(CpuState *cpu, uint32 pc24, uint32 site) {
    (void)cpu;
    (void)pc24;
    (void)site;
    ++g_small_bridge.calls;
    g_small_bridge.flow = SMALL_BRIDGE_UNEXPECTED_CHILD;
    return (RecompReturn)111;
}

RecompReturn cpu_dispatch_call_pc_pushed(
    CpuState *cpu, uint32 pc24, uint32 site, uint8 frame_size,
    uint32 *return_pc24) {
    (void)cpu;
    (void)pc24;
    (void)site;
    (void)frame_size;
    (void)return_pc24;
    ++g_small_bridge.calls;
    g_small_bridge.flow = SMALL_BRIDGE_UNEXPECTED_CHILD;
    return (RecompReturn)111;
}

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

/* ROM for seeds that aim at list contents. */
static const SnesVerifyBus *g_seed_rom;

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
    const Lufia2CpuState *input) {
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
    const Lufia2CpuState *native, const Interp816 *reference) {
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
    Lufia2CpuState *input) {
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
    Lufia2CpuState *input) {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState *input) {
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
    const Lufia2CpuState *input,
    uint32_t start_pc,
    uint32_t redispatch_pc,
    uint32_t stop_pc,
    uint32_t handler_pc,
    Lufia2ActorPrimaryScriptStepFlow expected_flow,
    uint8_t expected_opcode,
    unsigned case_index,
    const char *name) {
    Lufia2CpuState native = *input;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState *input) {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState *input,
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;

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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;

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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
        Lufia2CpuState boundary = input;
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
    void (*native)(const Lufia2Memory *,
                   Lufia2CpuState *);
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    uint32_t resume = 0x83d370u;
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    if (fb12) {
        const uint32_t jump = 0x83fb17u;
        const uint16_t index = (uint8_t)input.accumulator;
        const uint16_t target =
            (uint16_t)(Rom8(bus, 0x83fb1au + index) |
                       ((uint16_t)Rom8(bus, 0x83fb1bu + index) << 8));
        resume = 0x830000u | target;
        stop = SnesVerifyRunUntil(reference, &jump, 1, 256, &instructions);
        if (stop == 0) {
            interp816_runOpcode(reference);
            ++instructions;
            stop = SnesVerifyPc24(reference) == resume ? 0 : -1;
        }
    } else {
        stop = SnesVerifyRunUntil(reference, &resume, 1, 256, &instructions);
    }

    if (!unknown || native.resume_pc != resume || stop != 0 ||
        !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
                "FAIL resume %s case %u: unknown=%d resume=%06X stop=%d "
                "A=%04X/%04X X=%04X/%04X S=%04X/%04X\n",
                fb12 ? "FB12 jump" : "D370", case_index, unknown ? 1 : 0,
                native.resume_pc, stop, native.accumulator, reference->a, native.x,
                reference->x, native.stack, reference->sp);
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
        if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83c1b4u, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
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

    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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
    const Interp816 *reference, Lufia2CpuState *cpu) {
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
    Lufia2CpuState *cpu,
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
    uint32_t wm_address;
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
    saved.wm_address = bus->wm_address;
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
    bus->wm_address = saved->wm_address;
}

static bool SameBusRegisters(
    const SnesVerifyBus *bus, const BusRegisters *expected) {
    return bus->multiply_a == expected->multiply_a &&
        bus->multiply_b == expected->multiply_b &&
        bus->multiply_result == expected->multiply_result &&
        bus->divide_a == expected->divide_a &&
        bus->divide_result == expected->divide_result &&
        bus->m7_latch == expected->m7_latch &&
        bus->m7_a == expected->m7_a && bus->m7_b == expected->m7_b &&
        bus->wm_address == expected->wm_address;
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        ++stats->unterminated;
        return true;
    }
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
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
        if (result.flow == LUFIA2_EXECUTION_RETURNED
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
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
    0x80cc3fu, 0x80cc0eu, 0x83b96du, 0x838225u, 0x838251u};

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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    uint8_t *native_wram;
    BusRegisters registers;
    unsigned instructions = 0;
    unsigned visits = 0;
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
    registers = SaveBusRegisters(bus);
    native = input;
    result = Lufia2FieldTriggerUpdate(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    RestoreBusRegisters(bus, &registers);

    InitInterp(reference, 0x8381c6u, &input);
    /* Random event scripts can run long list searches. */
    while (instructions < 40000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == 0x83808fu
                : pc == result.pc && reference->sp == native.stack &&
                  visits++ == result.dispatches) {
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

    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
        0x60, 0x85, 0x8a, 0xee, 0x14, 0x14, 0x14};
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
        /* Op $14: steps due, zoom near zero, limits as they come. */
        if (ObjectRandom() & 1u) {
            const uint32_t pick = ObjectRandom();

            bus->wram[0x15f9u + slot] =
                (uint8_t)((bus->wram[0x15d9u + slot] & 0x0fu) - 1u);
            bus->wram[0x1659u + slot] =
                (uint8_t)((bus->wram[0x1639u + slot] & 0x0fu) - 1u);
            bus->wram[0x16b9u + slot] =
                (uint8_t)((bus->wram[0x1699u + slot] & 0x0fu) - 1u);
            if (pick & 1u)
                bus->wram[0x1579u + slot] &= 0x03u;
            if (pick & 2u)
                bus->wram[0x15b9u + slot] &= 0xf7u;
        }
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

    const SnesVerifyBus hardware_start = *bus;
    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2ObjectSlotsUpdate(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */

    const SnesVerifyBus hardware_end = *bus;
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    *bus = hardware_start;
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x83e03eu, &input);
    while (instructions < 64000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
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
        hardware_end.multiply_a != bus->multiply_a ||
        hardware_end.multiply_b != bus->multiply_b ||
        hardware_end.multiply_result != bus->multiply_result ||
        hardware_end.divide_a != bus->divide_a ||
        hardware_end.divide_result != bus->divide_result ||
        hardware_end.m7_latch != bus->m7_latch ||
        hardware_end.m7_a != bus->m7_a || hardware_end.m7_b != bus->m7_b ||
        hardware_end.wm_address != bus->wm_address ||
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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
        if (result.flow == LUFIA2_EXECUTION_RETURNED
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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

typedef Lufia2ExecutionResult (*FieldChildFunction)(
    const Lufia2Memory *memory, Lufia2CpuState *cpu);

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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, entry, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
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
        if (result.flow == LUFIA2_EXECUTION_RETURNED
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

/* Script mode: actor waits ($33), window closes. */
static void SeedTextScript(uint8_t *wram) {
    const uint16_t text = (NmiRandom() & 3u)
        ? (uint16_t)(0x8000u + (NmiRandom() & 0x7ff0u))
        : (uint16_t)(0x2000u + (NmiRandom() & 0x3fffu));
    const uint8_t actor = (uint8_t)(NmiRandom() & 0x3fu);

    wram[0x099bu] &= 0xfeu;
    wram[0x09b9u] = 0x7eu;
    wram[0x09b7u] = (uint8_t)text;
    wram[0x09b8u] = (uint8_t)(text >> 8);
    if (NmiRandom() & 7u)
        wram[text] = 0x33u;
    wram[(uint16_t)(text + 3u)] =
        (NmiRandom() & 1u) ? 0x33u : (uint8_t)NmiRandom();
    wram[0x1269u] = (NmiRandom() & 3u) ? actor : (uint8_t)NmiRandom();
    wram[0x0622u + actor] &= (NmiRandom() & 1u) ? 0x77u : 0xffu;
    if (NmiRandom() & 1u)
        wram[0x099cu] &= 0xfeu;
    {
        static const uint8_t ops[64] = {
            0x33u, 0x03u, 0x37u, 0x3cu, 0x00u, 0x42u, 0x68u, 0x05u,
            0x06u, 0x0fu, 0x15u, 0x1au, 0x1bu, 0x38u, 0x1du, 0x2eu,
            0x3eu, 0x3fu, 0x4fu, 0x5fu, 0x27u, 0x2au, 0x50u, 0xc1u,
            0x5au, 0x8au, 0xc5u, 0xb5u, 0x57u, 0x71u, 0x76u, 0xaau,
            0xccu, 0x60u, 0x1eu, 0x09u, 0x22u, 0x26u, 0x95u, 0x94u,
            0x96u, 0x47u, 0x49u, 0x4au, 0x52u, 0x7cu, 0x7du, 0x7eu,
            0x7fu, 0x80u, 0xcbu, 0x74u, 0x0cu, 0x0du, 0x0eu, 0x0bu,
            0x69u, 0x01u, 0x1cu, 0x41u, 0x00u, 0x37u, 0x4bu, 0x68u};
        const uint16_t back = (uint16_t)(0x8000u + (NmiRandom() & 0x7ff0u));

        if (NmiRandom() & 1u)
            wram[text] = ops[NmiRandom() & 63u];
        if (NmiRandom() & 1u)
            wram[(uint16_t)(text + 1u)] = (uint8_t)(NmiRandom() & 0x0fu);
        if (!(NmiRandom() & 3u)) {
            /* An actor id at a $2E range bound, present in $05FA. */
            static const uint8_t kBounds[4] = {0x0fu, 0x10u, 0x4fu, 0x50u};
            const uint8_t id = kBounds[NmiRandom() & 3u];

            wram[(uint16_t)(text + 1u)] = id;
            wram[0x05fau + NmiRandom() % 0x28u] = id;
            wram[0x105fau + NmiRandom() % 0x28u] = id;
        }
        if (NmiRandom() & 1u) {
            /* Choice ($0B): 1-4 rows, table in $7E; speaker ($01) listed. */
            const uint16_t table = (uint16_t)(0x8000u + (NmiRandom() & 0x7ff0u));

            wram[0x126eu] = (uint8_t)(1u + (NmiRandom() & 3u));
            wram[0x126au] = (uint8_t)(NmiRandom() % 5u);
            wram[0x126du] = 0x7eu;
            wram[0x126bu] = (uint8_t)table;
            wram[0x126cu] = (uint8_t)(table >> 8);
            const uint32_t speaker = NmiRandom() % 0x28u;

            wram[0x09acu] = speaker < 4u ? (uint8_t)(0xfeu + (speaker & 1u))
                                         : wram[0x05fau + speaker];
        }
        if (NmiRandom() & 1u) {
            /* $38: seconds near the argument; $41: view at its target. */
            const uint32_t pick = NmiRandom();
            const uint16_t view = (uint16_t)((pick & 3u) * 2u);

            wram[0x125fu] = (uint8_t)((pick >> 2) & 0x0fu);
            wram[0x0042u] = (uint8_t)(0x3au + ((pick >> 6) & 3u));
            wram[(uint16_t)(text + 2u)] = (pick & 0x100u) ? 0xffu : wram[(uint16_t)(text + 2u)];
            wram[0x05aau] = (uint8_t)view;
            wram[0x05abu] = 0;
            if (pick & 0x200u) {
                wram[0x1d08bu] = wram[0x121eu + view];
                wram[0x1d08cu] = wram[0x121fu + view];
                wram[0x1d08du] = wram[0x1226u + view];
                wram[0x1d08eu] = (pick & 0x400u) ? wram[0x1227u + view]
                                                 : (uint8_t)~wram[0x1227u + view];
            }
        }
        wram[0x1254u] = (NmiRandom() & 1u) ? 0x7eu : 0x00u;
        wram[0x1252u] = (uint8_t)back;
        wram[0x1253u] = (uint8_t)(back >> 8);
        wram[back] = ops[NmiRandom() & 63u];
        if (NmiRandom() & 1u)
            for (unsigned i = 0; i < 5u; ++i)
                wram[0x0622u + i] &= 0xf7u;
        if (NmiRandom() & 1u)
            wram[0x0581u] = 0;
        if (NmiRandom() & 1u)
            wram[0x0b62u] = 0;
    }
}

/* Effects mostly idle; timer and text gates mixed. */
static void SeedFieldTick(SnesVerifyBus *bus) {
    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    if (NmiRandom() & 1u)
        bus->wram[0x1261u] &= 0x48u;
    if (NmiRandom() & 3u)
        bus->wram[0x1262u] &= 0xfeu;
    bus->wram[0x1d0fau] = (uint8_t)(NmiRandom() % 0x70u * 2u);
    bus->wram[0x1d0fbu] = 0;
    for (unsigned i = 0; i < 3u; ++i)
        if (NmiRandom() & 1u)
            bus->wram[0x1274u + 2u * i] = bus->wram[0x1275u + 2u * i] = 0;
    bus->wram[0x1d0c1u] = (NmiRandom() & 1u)
        ? 0xffu : (uint8_t)(1u + NmiRandom() % 3u);
    if (NmiRandom() & 3u)
        bus->wram[0x099bu] &= 0x75u;
    if (NmiRandom() & 1u)
        bus->wram[0x1265u] = 0;
    bus->wram[0x1266u] =
        (uint8_t)(bus->wram[0x1265u] - 1u - (NmiRandom() & 1u));
    if (NmiRandom() & 1u)
        SeedTextScript(bus->wram);
    bus->wram[0x09b2u] = (uint8_t)(0x20u + (NmiRandom() & 0x3fu));
    bus->wram[0x109b2u] = (uint8_t)(0x20u + (NmiRandom() & 0x3fu));
    /* JSL $80:9C72 at $83:8073. */
    bus->wram[0x1ff1u] = 0x76u;
    bus->wram[0x1ff2u] = 0x80u;
    bus->wram[0x1ff3u] = 0x83u;
}

/* Whole $80:9C72 from the field loop's JSL. */
static bool RunFieldTickCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    FieldChildStats *stats) {
    const uint32_t entry = 0x809c72u;
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x00u};
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    uint8_t *native_wram;
    static SnesVerifyBusEvent native_mmio[SNES_VERIFY_MAX_MMIO];
    size_t native_count;
    unsigned instructions = 0;
    unsigned visits = 0;
    bool stopped = false;

    SeedFieldTick(bus);
    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x80u;
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

    const uint32_t initial_wm_address = bus->wm_address;
    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetMmio(bus);
    result = Lufia2FieldEventTick(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */
        native.program_bank = 0x83u;
    }
    native_count = bus->mmio_count;
    memcpy(native_mmio, bus->mmio, sizeof(SnesVerifyBusEvent) * native_count);
    SnesVerifyBusResetMmio(bus);
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    const uint32_t native_wm_address = bus->wm_address;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    bus->wm_address = initial_wm_address;

    InitInterp(reference, entry, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == 0x838077u
                : pc == result.pc && reference->sp == native.stack &&
                  visits++ == result.dispatches) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }

    if (native_count != bus->mmio_count || bus->mmio_overflow ||
        memcmp(native_mmio, bus->mmio,
            sizeof(SnesVerifyBusEvent) * native_count) != 0)
        stopped = false;
    if (!stopped || bus->wm_address != native_wm_address || !SameState(&native, reference) ||
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { BATTLE_NMI_CASES = 16384 };

typedef struct BattleNmiStats {
    unsigned returned;
    unsigned boundary;
    unsigned mmio_writes;
} BattleNmiStats;

/* Whole $85:8DC5 from the $00:0067 JSL; MMIO order compared. */
static bool RunBattleNmiCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    BattleNmiStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x80u, 0x80u, 0x00u, 0x7eu};
    const uint16_t dp = dps[NmiRandom() & 7u];
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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
    /* VRAM queue mostly idle; timers mostly on handler 1. */
    for (unsigned i = 0; i < 16u; ++i)
        if (NmiRandom() & 3u)
            Poke16(bus, 0x1a8fu + 6u * i, 0);
    for (unsigned slot = 0; slot < 8u; ++slot) {
        const uint16_t base = (uint16_t)(0x1b17u + 8u * slot);

        if (NmiRandom() & 1u)
            bus->wram[base] = 0;
        bus->wram[base + 1u] = (uint8_t)(1u + (NmiRandom() & 1u));
        bus->wram[base + 2u] = (NmiRandom() & 7u)
            ? 1u : (uint8_t)(NmiRandom() & 0x0fu);
    }
    if (NmiRandom() & 1u)
        bus->wram[0x1b23u] = 0;
    if (NmiRandom() & 7u)
        bus->wram[0x12e3u] |= 0x80u;
    if (NmiRandom() & 1u)
        bus->wram[0x1268u] = 0;
    /* JSL $85:8DC5 at $00:0067. */
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
    input.program_bank = 0x85u;
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
    result = Lufia2BattleNmiUploads(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */
        native.program_bank = 0x00u;
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

    SnesVerifyBusResetMmio(bus);
    InitInterp(reference, 0x858dc5u, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == 0x00006au
                : pc == result.pc && reference->sp == native.stack) {
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
            "FAIL 8DC5 case %u: pc=%06X stop=%d mmio=%u/%u same=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u wram@%05X\n",
            case_index, result.pc, stopped ? 1 : 0, (unsigned)native_count,
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    stats->mmio_writes += (unsigned)native_count;
    return true;
}

enum { FIELD_SPRITES_CASES = 16384 };

typedef struct FieldSpritesStats {
    unsigned returned;
    unsigned boundary;
    unsigned visible;
} FieldSpritesStats;

/* Field actors around the camera, mostly small sprites. */
static void SeedFieldSprites(uint8_t *wram, uint32_t (*random)(void)) {
    uint16_t camera_x;
    uint16_t camera_y;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = random();
        memcpy(wram + i, &word, 4);
    }
    wram[0x1d0b0u] = (random() & 3u) ? (uint8_t)(0x10u + (random() & 0x3fu))
                                     : 0xffu;
    camera_x = (uint16_t)(random() & 0x07ffu);
    camera_y = (uint16_t)(random() & 0x07ffu);
    if (random() & 1u) {
        wram[0x1261u] &= 0xbfu;
        wram[0x1220u] = (uint8_t)camera_x;
        wram[0x1221u] = (uint8_t)(camera_x >> 8);
        wram[0x1228u] = (uint8_t)camera_y;
        wram[0x1229u] = (uint8_t)(camera_y >> 8);
    } else {
        wram[0x1261u] |= 0x40u;
        wram[0x1d0eeu] = (uint8_t)camera_x;
        wram[0x1d0efu] = (uint8_t)(camera_x >> 8);
        wram[0x1d0f0u] = (uint8_t)camera_y;
        wram[0x1d0f1u] = (uint8_t)(camera_y >> 8);
    }
    for (unsigned i = 0; i < 0x48u; ++i) {
        const uint16_t x = (uint16_t)(camera_x - 0x40u + (random() % 0x180u));
        const uint16_t y = (uint16_t)(camera_y - 0x30u + (random() % 0x140u));

        if (random() & 3u)
            wram[0x0622u + i] &= 0xfbu;
        if (random() & 3u)
            wram[0x1e316u + i] &= 0x7fu;
        wram[0x1ddaeu + 2u * i] = (uint8_t)x;
        wram[0x1ddafu + 2u * i] = (uint8_t)(x >> 8);
        wram[0x1de3eu + 2u * i] = (uint8_t)y;
        wram[0x1de3fu + 2u * i] = (uint8_t)(y >> 8);
        if (random() & 15u)
            wram[0x1e216u + i] = (uint8_t)(random() & 3u);
        wram[0x1e37eu + i] = (random() & 1u)
            ? 0xffu : (uint8_t)(random() % 0x20u);
        if (random() & 1u)
            wram[0x1471u + i] = wram[0x066au + i];
    }
    wram[0x0732u] = (uint8_t)((random() % 10u) * 2u);
}

/* Whole $83:A21A from the field loop's JSL. */
static bool RunFieldSpritesCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    FieldSpritesStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x83u, 0x80u, 0x7eu};
    const uint32_t entry = 0x83a21au;
    const uint32_t exit = 0x83808cu;
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    SnesVerifyBusEvent *native_mmio;
    size_t native_count;
    uint8_t *native_wram;
    BusRegisters registers;
    unsigned instructions = 0;
    unsigned visits = 0;
    bool stopped = false;
    bool same_mmio;

    SeedFieldSprites(bus->wram, NmiRandom);
    /* JSL $83:A21A at $83:8088. */
    bus->wram[0x1ff1u] = 0x8bu;
    bus->wram[0x1ff2u] = 0x80u;
    bus->wram[0x1ff3u] = 0x83u;

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
    input.accumulator_is_8_bit = (NmiRandom() & 15u) ? 1u : 0u;
    input.index_is_8_bit = (NmiRandom() & 15u) ? 1u : 0u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    registers = SaveBusRegisters(bus);
    native = input;
    SnesVerifyBusResetMmio(bus);
    result = Lufia2FieldActorSprites(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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
    InitInterp(reference, entry, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == exit
                : pc == result.pc && reference->sp == native.stack &&
                  visits == result.dispatches) {
            stopped = true;
            break;
        }
        if (pc == 0x83a461u)
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
            "FAIL %06X case %u: pc=%06X stop=%d mmio=%u/%u same=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u V=%u/%u "
            "wram@%05X %02X/%02X\n",
            entry, case_index, result.pc, stopped ? 1 : 0,
            (unsigned)native_count, (unsigned)bus->mmio_count,
            same_mmio ? 1 : 0, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    stats->visible += bus->wram[0x1d0b0u] != 0xffu ? bus->wram[0x1d0b0u] : 0;
    return true;
}

enum { WORLD_MAP_NMI_CASES = 16384 };

typedef struct WorldMapNmiStats {
    unsigned returned;
    unsigned tiles;
    unsigned cycles;
    unsigned mmio_writes;
} WorldMapNmiStats;

/* Small upload records and palette cycles, flags mixed. */
static void SeedWorldMapNmi(uint8_t *wram, uint32_t (*random)(void)) {
    static const uint16_t flags[9] = {
        0x11d9u, 0x1710u, 0x1711u, 0x11deu, 0x11dau, 0x11ddu, 0x11dfu,
        0x11e1u, 0x11d8u};

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = random();
        memcpy(wram + i, &word, 4);
    }
    for (unsigned i = 0; i < 9u; ++i)
        if (random() & 1u)
            wram[flags[i]] = 0;
    if (random() & 1u) {
        wram[0x1702u] = 0;
        wram[0x1703u] = 0;
    }
    wram[0x1365u] = (random() & 7u) ? (uint8_t)(1u + (random() & 3u)) : 0;
    for (unsigned i = 0; i < 4u; ++i) {
        const uint16_t record = (uint16_t)(0x0200u + (random() & 0x0dffu));

        wram[0x1367u + 2u * i] = (uint8_t)record;
        wram[0x1368u + 2u * i] = (uint8_t)(record >> 8);
        if (!(random() & 7u)) {
            wram[0x13a7u + 2u * i] = 0;
            wram[0x13a8u + 2u * i] = 0;
        }
        wram[record] = (uint8_t)((random() & 0x80u) | (random() % 3u));
        wram[(uint16_t)(record + 1u)] = (uint8_t)(random() & 0x3fu);
        wram[(uint16_t)(record + 2u)] = (uint8_t)(1u + (random() & 3u));
    }
    wram[0x16e7u] = (random() & 3u) ? (uint8_t)(random() & 3u) : 0;
    for (unsigned i = 0; i < 3u; ++i) {
        const uint16_t base = (uint16_t)(0x16e8u + 5u * i);
        const uint8_t length = (uint8_t)(1u + (random() & 15u));

        wram[base + 1u] = length;
        wram[base + 3u] = (uint8_t)(random() % (length + 1u));
        wram[base + 2u] = (uint8_t)(random() & 3u);
    }
}

/* Whole $86:CEF6 from the $00:0067 JSL; MMIO order compared. */
static bool RunWorldMapNmiCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    WorldMapNmiStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x80u, 0x86u, 0x00u, 0x7eu};
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    SnesVerifyBusEvent *native_mmio;
    size_t native_count;
    uint8_t *native_wram;
    BusRegisters registers;
    unsigned instructions = 0;
    bool stopped = false;
    bool same_mmio;

    SeedWorldMapNmi(bus->wram, NmiRandom);
    if (bus->wram[0x11d9u] && bus->wram[0x1365u])
        ++stats->tiles;
    if (bus->wram[0x16e7u])
        ++stats->cycles;
    /* JSL $86:CEF6 at $00:0067. */
    bus->wram[0x1ff1u] = 0x69u;
    bus->wram[0x1ff2u] = 0x00u;
    bus->wram[0x1ff3u] = 0x00u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x86u;
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
    registers = SaveBusRegisters(bus);
    native = input;
    SnesVerifyBusResetMmio(bus);
    result = Lufia2WorldMapNmiUploads(&memory, &native);
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
    RestoreBusRegisters(bus, &registers);

    SnesVerifyBusResetMmio(bus);
    InitInterp(reference, 0x86cef6u, &input);
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

    if (result.flow != LUFIA2_EXECUTION_RETURNED || !stopped ||
        !same_mmio || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        size_t event = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        while (event < native_count && event < bus->mmio_count &&
               native_mmio[event].address == bus->mmio[event].address &&
               native_mmio[event].value == bus->mmio[event].value)
            ++event;
        fprintf(stderr,
            "FAIL CEF6 case %u: stop=%d mmio=%u/%u first diff %u "
            "(%06X=%02X / %06X=%02X) insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u wram@%05X\n",
            case_index, stopped ? 1 : 0, (unsigned)native_count,
            (unsigned)bus->mmio_count, (unsigned)event,
            event < native_count ? native_mmio[event].address : 0u,
            event < native_count ? native_mmio[event].value : 0u,
            event < bus->mmio_count ? bus->mmio[event].address : 0u,
            event < bus->mmio_count ? bus->mmio[event].value : 0u,
            instructions,
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

enum { VRAM_SLOT_CASES = 16384 };

/* Whole $85:ECDB from the JSL at $85:9633. */
static bool RunVramSlotCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    unsigned *boundaries) {
    static const uint8_t banks[4] = {0x85u, 0x85u, 0x00u, 0x7eu};
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    uint8_t *native_wram;
    unsigned instructions = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    for (unsigned slot = 0; slot < 16u; ++slot)
        if (!(NmiRandom() % 12u))
            Poke16(bus, 0x1a8fu + 6u * slot, 0);
    bus->wram[0x1ff1u] = 0x36u;
    bus->wram[0x1ff2u] = 0x96u;
    bus->wram[0x1ff3u] = 0x85u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = (NmiRandom() & 3u) ? 0 : 0x0400u;
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x85u;
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = NmiRandom() & 1u;
    input.index_is_8_bit = (NmiRandom() & 7u) ? 0u : 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2BattleVramQueueSlot(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */
        native.program_bank = 0x85u;
    }
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);

    InitInterp(reference, 0x85ecdbu, &input);
    while (instructions < 10000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == 0x859637u
                : pc == result.pc && reference->sp == native.stack) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }
    if (!stopped || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        fprintf(stderr,
            "FAIL ECDB case %u: flow=%u pc=%06X stop=%d "
            "X=%04X/%04X Y=%04X/%04X Z=%u/%u N=%u/%u\n",
            case_index, (unsigned)result.flow, result.pc, stopped ? 1 : 0,
            native.x, reference->x, native.y, reference->y,
            native.zero, reference->z, native.negative, reference->n);
        free(native_wram);
        return false;
    }
    free(native_wram);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        ++*boundaries;
    return true;
}

enum { WORLD_MAP_EDGE_CASES = 16384 };

typedef struct WorldMapEdgeStats {
    unsigned returned;
    unsigned boundary;
    unsigned columns;
    unsigned rows;
} WorldMapEdgeStats;

/* Whole $86:99BF from the JSR at $86:9285. */
static bool RunWorldMapEdgeCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    WorldMapEdgeStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x86u, 0x86u, 0x7eu, 0x00u};
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    uint8_t *native_wram;
    uint32_t native_wm_address;
    unsigned instructions = 0;
    bool stopped = false;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    /* Camera usually still or one step away. */
    if (NmiRandom() & 1u)
        bus->wram[0x11f6u] = bus->wram[0x11f2u];
    else if (NmiRandom() & 1u)
        bus->wram[0x11f6u] = (uint8_t)(bus->wram[0x11f2u] +
            ((NmiRandom() & 1u) ? 1u : 0xffu));
    if (NmiRandom() & 1u)
        bus->wram[0x11f7u] = bus->wram[0x11f4u];
    else if (NmiRandom() & 1u)
        bus->wram[0x11f7u] = (uint8_t)(bus->wram[0x11f4u] +
            ((NmiRandom() & 1u) ? 1u : 0xffu));
    if (bus->wram[0x11f2u] != bus->wram[0x11f6u])
        ++stats->columns;
    if (bus->wram[0x11f4u] != bus->wram[0x11f7u])
        ++stats->rows;
    bus->wram[0x1ff1u] = 0x87u;
    bus->wram[0x1ff2u] = 0x92u;

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = 0x86u;
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = (NmiRandom() & 15u) ? 1u : 0u;
    input.index_is_8_bit = (NmiRandom() & 15u) ? 0u : 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    SnesVerifyBusResetMmio(bus);
    bus->wm_address = 0;
    result = Lufia2WorldMapStreamEdges(&memory, &native);
    native_wm_address = bus->wm_address;
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        native.stack = (uint16_t)(native.stack + 2u);   /* RTS */
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    bus->wm_address = 0;

    InitInterp(reference, 0x8699bfu, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == 0x869288u
                : pc == result.pc && reference->sp == native.stack) {
            stopped = true;
            break;
        }
        interp816_runOpcode(reference);
        ++instructions;
    }
    if (!stopped || bus->mmio_count || !SameState(&native, reference) ||
        bus->wm_address != native_wm_address ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL 99BF case %u: flow=%u stop=%d mmio=%u insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
            "M=%u/%u C=%u/%u Z=%u/%u N=%u/%u V=%u/%u wram@%05X %02X/%02X\n",
            case_index, (unsigned)result.flow, stopped ? 1 : 0,
            (unsigned)bus->mmio_count, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
            native.accumulator_is_8_bit, reference->mf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, native.overflow, reference->v,
            (unsigned)diff,
            diff < SNES_VERIFY_WRAM_SIZE ? native_wram[diff] : 0,
            diff < SNES_VERIFY_WRAM_SIZE ? bus->wram[diff] : 0);
        free(native_wram);
        return false;
    }
    free(native_wram);
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { SMALL_CASES = 16384 };

typedef Lufia2ExecutionResult (*SmallFunction)(
    const Lufia2Memory *memory, Lufia2CpuState *cpu);

extern RecompReturn Lufia2DecompBridge_F0A2(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8378(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8CDA(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_8CF5(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_F194(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_F3F4(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_F404(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_F87F(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_CE23(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_810E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C2FD(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_CD83(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9F6F(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_D721(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_E49E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_EFC5(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_82F0A2(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_D749(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_A2E3(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_950E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_D07B(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_E297(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_CD1F(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_E5E1(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_DCF4(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_DCC1(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_B2C5(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_9CB2(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_A918(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_ACDB(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_ED8E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_EE94(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C129(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_DFA2(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C652(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C784(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C56E(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C305(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_C23D(CpuState *cpu);
extern RecompReturn Lufia2DecompBridge_FB1F(CpuState *cpu);

typedef struct SmallTarget {
    const char *name;
    uint32_t entry;
    SmallFunction run;
    uint8_t frame;
    uint32_t call_return;               /* last byte of the call */
    void (*seed)(uint8_t *wram, uint16_t dp);
    void (*registers)(Lufia2CpuState *input);  /* optional */
    RecompReturn (*bridge)(CpuState *cpu);     /* direct ROM-entry replacement */
    uint8_t bridge_width; /* 1 any, 2 M0X0, 3 M?X0, 4 M1X0 */
} SmallTarget;

typedef struct SmallStats {
    unsigned returned;
    unsigned boundary;
    unsigned level_up_results[3];
    unsigned s19_returns[3];
    unsigned level_99_capped;
    unsigned text_windows[4];
    unsigned window_join[2];
    unsigned window_tail[4];
    unsigned window_lookup[2];
    unsigned condition_family[15];
    unsigned condition_compare[8];
    unsigned condition_result[2];
    unsigned condition_join[2];
    unsigned possession_exit[3];
    unsigned condition_record_bus[2]; /* WRAM/CPU MMIO */
    unsigned condition_search[2];
    unsigned condition_goto[2][2]; /* true/false selector, skip/take */
} SmallStats;

static void SeedMenuRequest(uint8_t *wram, uint16_t dp) {
    (void)dp;
    if (NmiRandom() & 7u)
        wram[0x05b5u] &= 0xbfu;
    if (NmiRandom() & 1u)
        wram[0x09a7u] &= 0xfdu;
}

static void SeedTakeButtons(uint8_t *wram, uint16_t dp) {
    if (NmiRandom() & 3u)
        wram[dp + 0x46u] &= (uint8_t)NmiRandom();
}

static void SeedStatusRequests(uint8_t *wram, uint16_t dp) {
    (void)dp;
    if (NmiRandom() & 1u)
        wram[0x09a8u] &= 0xf7u;
    if (NmiRandom() & 3u)
        wram[0x05b7u] &= (uint8_t)(0xf8u | NmiRandom());
}

static void SeedFieldReload(uint8_t *wram, uint16_t dp) {
    (void)wram;
    (void)dp;
}

/* Battle script of known opcodes, forward jumps, then an end. */
static void SeedBattleScript(uint8_t *wram, uint16_t dp) {
    static const uint8_t ops[52] = {
        0x03u, 0x05u, 0x06u, 0x07u, 0x0au, 0x0bu, 0x0cu, 0x0du,
        0x0eu, 0x16u, 0x17u, 0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu,
        0x1fu, 0x20u, 0x42u, 0x43u, 0x0cu, 0x06u, 0x0fu, 0x21u,
        0x04u, 0x08u, 0x09u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u,
        0x28u, 0x29u, 0x2au, 0x2bu, 0x2eu, 0x2fu, 0x30u, 0x35u,
        0x36u, 0x3eu, 0x41u, 0x56u, 0x10u, 0x11u, 0x12u, 0x22u,
        0x37u, 0x3cu, 0x4du, 0x13u};
    static const uint8_t sizes[0x57] = {
        [0x10] = 5, [0x11] = 3, [0x12] = 5, [0x13] = 3, [0x22] = 6,
        [0x37] = 1, [0x3c] = 1, [0x4d] = 1,
        [0x03] = 3, [0x04] = 3, [0x05] = 4, [0x06] = 6, [0x07] = 6,
        [0x08] = 6, [0x09] = 6, [0x0a] = 6, [0x0b] = 6, [0x0c] = 4,
        [0x0d] = 5, [0x0e] = 5, [0x0f] = 5, [0x16] = 5, [0x17] = 5,
        [0x18] = 5, [0x19] = 3, [0x1a] = 3, [0x1b] = 3, [0x1c] = 2,
        [0x1f] = 3, [0x20] = 3, [0x21] = 6, [0x23] = 3, [0x24] = 5,
        [0x25] = 3, [0x26] = 3, [0x27] = 3, [0x28] = 1, [0x29] = 1,
        [0x2a] = 1, [0x2b] = 3, [0x2e] = 1, [0x2f] = 2, [0x30] = 2,
        [0x35] = 2, [0x36] = 2, [0x3e] = 2, [0x41] = 1, [0x42] = 3,
        [0x43] = 1, [0x56] = 3};
    const uint16_t base = (uint16_t)(0x2000u + (NmiRandom() & 0x1ff0u));
    unsigned at = 0;

    for (unsigned i = 0; i < 128u; ++i)
        wram[base + i] = (uint8_t)NmiRandom();
    for (;;) {
        const uint8_t op = ops[NmiRandom() % 52u];
        const unsigned size = sizes[op];

        if (at + size > 120u)
            break;
        wram[base + at] = op;
        if (op >= 0x03u && op <= 0x0bu) {
            const uint16_t jump = (NmiRandom() & 7u) ? 120u : (uint16_t)(NmiRandom() % 120u);

            wram[base + at + size - 2u] = (uint8_t)jump;
            wram[base + at + size - 1u] = (uint8_t)(jump >> 8);
        }
        if (op >= 0x06u && op <= 0x0bu && (NmiRandom() & 1u)) {
            const uint8_t n = (uint8_t)(NmiRandom() & 0x7fu);
            const uint16_t value = (uint16_t)(NmiRandom() & 0x7fffu);
            const uint16_t near = (uint16_t)(value + NmiRandom() % 3u - 1u);

            wram[base + at + 1u] = n;
            wram[base + at + 2u] = (uint8_t)value;
            wram[base + at + 3u] = (uint8_t)(value >> 8);
            wram[0x1f40eu + 2u * n] = (uint8_t)near;
            wram[0x1f40fu + 2u * n] = (uint8_t)(near >> 8);
        }
        if (op == 0x42u)
            wram[base + at + 2u] = 0;
        at += size;
    }
    while (at < 120u)
        wram[base + at++] = 0x00u;
    wram[base + 120u] = (NmiRandom() & 1u) ? 0x00u : 0x4fu;
    wram[(uint16_t)(dp + 0xbbu)] = (uint8_t)base;
    wram[(uint16_t)(dp + 0xbcu)] = (uint8_t)(base >> 8);
    wram[(uint16_t)(dp + 0xbdu)] = 0x7eu;
    wram[0x0a42u] = (uint8_t)base;
    wram[0x0a43u] = (uint8_t)(base >> 8);
    wram[0x0a44u] = 0x7eu;
    for (unsigned i = 0x0a64u; i < 0x0c00u; i += 2u)
        wram[i + 1u] &= 0x07u;
    if (NmiRandom() & 1u)
        wram[0x1f462u] = wram[0x1f463u] = 0;
}

/* Intro state mostly valid, timers at the edges. */
static void SeedIntroNmi(uint8_t *wram, uint16_t dp) {
    static const uint8_t timers[8] = {
        0x00u, 0x01u, 0x1fu, 0x20u, 0x77u, 0x78u, 0x80u, 0xffu};

    wram[(uint16_t)(dp + 0x50u)] = (NmiRandom() & 7u)
        ? (uint8_t)(NmiRandom() % 7u) : (uint8_t)NmiRandom();
    if (NmiRandom() & 1u)
        wram[(uint16_t)(dp + 0x4eu)] = timers[NmiRandom() & 7u];
}

/* HDMA tables, window rows and upload flags in range. */
static void SeedNmiTables(uint8_t *wram, uint16_t dp) {
    const uint16_t source = (uint16_t)(0x2000u + (NmiRandom() & 0x0ff0u));
    const uint16_t target = (uint16_t)(0x4000u + (NmiRandom() & 0x0ff0u));
    const unsigned rows = NmiRandom() & 7u;

    wram[(uint16_t)(dp + 0xf2u)] = (uint8_t)NmiRandom();
    wram[(uint16_t)(dp + 0xf3u)] = (NmiRandom() & 7u)
        ? (uint8_t)((NmiRandom() & 7u) << 4) : (uint8_t)NmiRandom();
    wram[(uint16_t)(dp + 0xf4u)] = (uint8_t)source;
    wram[(uint16_t)(dp + 0xf5u)] = (uint8_t)(source >> 8);
    wram[(uint16_t)(dp + 0xf6u)] = 0x7eu;
    wram[(uint16_t)(dp + 0xf7u)] = (uint8_t)target;
    wram[(uint16_t)(dp + 0xf8u)] = (uint8_t)(target >> 8);
    wram[(uint16_t)(dp + 0xf9u)] = 0x7eu;
    for (unsigned i = 0; i < rows; ++i)
        wram[source + 3u * i] |= 0x01u;
    wram[source + 3u * rows] = 0;
    wram[0x1530u] = (uint8_t)(1u + (NmiRandom() & 7u));
    wram[0x1531u] = (NmiRandom() & 15u) ? 0 : (uint8_t)(NmiRandom() & 1u);
    wram[0x1539u] = (uint8_t)(NmiRandom() & 7u);
    wram[0x153bu] = (uint8_t)(NmiRandom() & 7u);
    for (unsigned t = 0; t < 3u; ++t) {
        const uint16_t table = (uint16_t)(0x80c0u + 0x100u * t);
        const unsigned count = NmiRandom() & 7u;

        for (unsigned i = 0; i < count; ++i)
            wram[table + 3u * i] |= 0x01u;
        wram[table + 3u * count] = 0;
    }
    if (NmiRandom() & 3u)
        wram[0x1565u] &= 0x07u;
    if (NmiRandom() & 3u)
        wram[0x1566u] = (uint8_t)(1u << (NmiRandom() % 5u));
    if (NmiRandom() & 3u)
        wram[0x1568u] &= (uint8_t)~0x03u;
}

static void SeedMenuNmi(uint8_t *wram, uint16_t dp) {
    SeedNmiTables(wram, dp);
    if (!(NmiRandom() & 3u))
        memset(wram + 0x11e8u, 0, 0x20u);
    if (NmiRandom() & 3u)
        wram[0x1565u] = 0;
    if (NmiRandom() & 3u)
        wram[0x1566u] = 0;
    if (NmiRandom() & 3u)
        wram[0x1567u] = 0;
}

static void SeedMenuButtons(uint8_t *wram, uint16_t dp) {
    (void)wram;
    (void)dp;
}

static void SeedMenuWindow(uint8_t *wram, uint16_t dp) {
    (void)dp;
    if (NmiRandom() & 1u)
        wram[0x156au] = 0;
}

static void SeedMenuBlink(uint8_t *wram, uint16_t dp) {
    (void)dp;
    if (NmiRandom() & 1u)
        wram[0x1554u] = 0x1fu;
}

/* Four $7E:F000 rectangle lists around ($8F, $91). */
static void SeedFieldRects(uint8_t *wram, uint16_t dp) {
    static const uint16_t heads[4] = {0x0002u, 0x000au, 0x000cu, 0x0006u};
    static const uint8_t strides[4] = {0x0fu, 0x05u, 0x05u, 0x09u};
    const uint8_t px = wram[(uint16_t)(dp + 0x8fu)];
    const uint8_t py = wram[(uint16_t)(dp + 0x91u)];

    for (unsigned list = 0; list < 4u; ++list) {
        const uint16_t base = (uint16_t)(0x0100u + 0x0200u * list);
        const unsigned count = NmiRandom() % 5u;
        uint16_t at = base;

        wram[0xf000u + heads[list]] = (uint8_t)base;
        wram[0xf001u + heads[list]] = (uint8_t)(base >> 8);
        for (unsigned i = 0; i < count; ++i) {
            const uint8_t x0 = (uint8_t)(px - (NmiRandom() & 7u));
            const uint8_t y0 = (uint8_t)(py - (NmiRandom() & 7u));

            wram[0xf000u + at] = (uint8_t)(NmiRandom() % 0xffu);
            wram[0xf001u + at] = (NmiRandom() & 3u) ? x0 : (uint8_t)NmiRandom();
            wram[0xf002u + at] = (NmiRandom() & 3u) ? y0 : (uint8_t)NmiRandom();
            wram[0xf003u + at] = (uint8_t)(px + (NmiRandom() & 7u));
            wram[0xf004u + at] = (uint8_t)(py + (NmiRandom() & 7u));
            if (NmiRandom() & 3u)
                wram[0xf004u + at] = (uint8_t)(py + 1u);
            at = (uint16_t)(at + strides[list]);
        }
        wram[0xf000u + at] = 0xffu;
    }
    /* Current stair id, often the first entry's. */
    if (NmiRandom() & 1u)
        wram[0x1d0bfu] = (uint8_t)((NmiRandom() & 0x80u) |
            (wram[0xf100u] & 0x7fu));
}

static void SeedScreenFade(uint8_t *wram, uint16_t dp) {
    (void)dp;
    if (NmiRandom() & 1u)
        wram[0x0581u] |= 0x80u;
}

/* Region 0 is the world map; these cells sit inside its entries. */
static void SeedWorldRegion(uint8_t *wram, uint16_t dp) {
    static const uint8_t cells[8][2] = {
        {36, 151}, {59, 188}, {37, 149}, {44, 224},
        {47, 197}, {49, 194}, {40, 176}, {39, 174}};

    wram[0x09ebu] = (uint8_t)((NmiRandom() % 16u) * 2u);
    wram[0x09ecu] = 0;
    if (NmiRandom() & 1u) {
        const unsigned cell = NmiRandom() & 7u;

        wram[0x09ebu] = 0;
        wram[(uint16_t)(dp + 0x58u)] = cells[cell][0];
        wram[(uint16_t)(dp + 0x5au)] = cells[cell][1];
    }
}

#include "lufia2_text_window_seed.h"
#include "lufia2_text_condition_seed.h"

static Lufia2ExecutionResult VerifyTextCondition(const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    const uint32_t join = Lufia2TextEvaluateCondition(memory, cpu);
    return ExecutionHandoff(cpu, join);
}

/* Printable text in WRAM, glyph buffer off the stack page. */
static void SeedTextStep(uint8_t *wram, uint16_t dp) {
    const uint16_t text = (uint16_t)(0x2000u + (NmiRandom() & 0x3fffu));

    (void)dp;
    if (NmiRandom() & 3u)
        wram[0x1d0ffu] = 0;
    if (NmiRandom() & 3u)
        wram[0x1259u] = 0;
    if (NmiRandom() & 3u)
        wram[0x099bu] |= 0x01u;
    if (NmiRandom() & 1u) {
        wram[0x09b9u] = 0x7eu;
        wram[0x09b7u] = (uint8_t)text;
        wram[0x09b8u] = (uint8_t)(text >> 8);
        for (unsigned i = 0; i < 4u; ++i)
            wram[(uint16_t)(text + i)] = (NmiRandom() & 7u)
                ? (uint8_t)(0x20u + NmiRandom() % 0x60u) : (uint8_t)NmiRandom();
    }
    if (NmiRandom() & 1u)
        SeedTextScript(wram);
    wram[0x09b2u] = (uint8_t)(0x20u + (NmiRandom() & 0x3fu));
    wram[0x109b2u] = (uint8_t)(0x20u + (NmiRandom() & 0x3fu));
    /* Line starts off the stack page too ($03 adds $400). */
    wram[0x1251u] = (uint8_t)(0x20u + (wram[0x1251u] & 0x3fu));
    wram[0x11251u] = (uint8_t)(0x20u + (wram[0x11251u] & 0x3fu));
    SeedTextWindowEngine(wram, dp);
    SeedTextConditionEngine(wram, dp);
}

/* Forward-only event scripts of the native opcodes, in WRAM. */
static void SeedEventScripts(uint8_t *wram, uint32_t (*random)(void)) {
    static const uint8_t kOps[147] = {
        0x01u, 0x0cu, 0x08u, 0x09u, 0x0au, 0x0du, 0x71u,
        0x19u, 0x1eu, 0x2bu, 0x1bu, 0x1cu, 0x57u, 0x2fu,
        0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u,
        0x38u, 0x39u, 0x3au, 0x3bu, 0x3cu, 0x3du, 0x3eu, 0x3fu, 0x40u,
        0x79u, 0x83u, 0x84u, 0x86u, 0x9du, 0x9eu, 0xa2u, 0xabu, 0xb5u,
        0xb8u, 0x5fu, 0x68u, 0x6bu, 0x58u, 0x24u, 0x25u, 0x29u,
        0xa9u, 0xaau, 0x55u, 0x69u, 0x85u,
        0x64u, 0x65u, 0x66u, 0x67u, 0x7cu, 0x7du, 0x7eu, 0x7fu, 0x80u,
        0x81u, 0xa3u, 0xa4u, 0xa5u, 0xa6u, 0xafu, 0xb0u, 0xb1u, 0xb2u,
        0xb4u, 0xb6u, 0xb7u, 0xbdu, 0x82u,
        0x12u, 0x6du, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u, 0x18u, 0x6eu,
        0x6fu, 0x72u, 0x73u, 0x74u, 0x75u, 0x76u, 0x77u, 0x04u, 0x05u,
        0x70u, 0x23u, 0x6au, 0xaeu, 0x0fu, 0x6cu, 0xa7u, 0x9cu, 0x9fu,
        0x5eu, 0x41u, 0x46u, 0x4bu, 0x50u, 0x54u, 0x26u, 0x27u,
        0x02u, 0x03u, 0x28u, 0xa0u, 0xa1u, 0xbau, 0x1du, 0x63u,
        0x87u, 0x88u, 0x89u, 0x20u, 0x10u, 0x7bu, 0x94u, 0x95u, 0x96u,
        0x97u, 0x98u, 0x99u, 0x9au, 0x8au, 0x8bu, 0x60u, 0x61u, 0x78u,
        0x59u, 0x21u, 0x22u, 0x2au, 0xbcu, 0xb3u, 0x5au, 0x5bu, 0x5cu,
        0x5du};
    uint8_t script[384];
    uint16_t starts[48];
    uint16_t words[96];
    uint8_t owner[96];
    unsigned n = 0, len = 16, patches = 0;
    const uint8_t bank = (random() & 1u) ? 0x7eu : 0x7fu;
    const uint16_t base = bank == 0x7eu && !(random() & 7u)
        ? (uint16_t)(0xff00u + (random() & 0xf0u))
        : (uint16_t)(0x9000u + (random() & 0x3fffu));
    const uint32_t map_0692 = bank == 0x7eu ? 0x0692u : 0x10692u;

    /* Base + $0A: word offset of the condition list table. */
    for (unsigned i = 0; i < 16u; ++i)
        script[i] = (uint8_t)random();

    while (len < 220u && n < 47u) {
        const unsigned roll = random() % 32u;
        uint8_t op;

        starts[n++] = (uint16_t)len;
        if (roll < 2u) {
            script[len++] = 0x00u;
            continue;
        }
        if (roll < 4u) {
            script[len++] = 0x11u;
            script[len++] = (uint8_t)random();
            continue;
        }
        if (roll < 5u) {
            script[len++] = (uint8_t)random();
            continue;
        }
        op = kOps[random() % 147u];
        script[len++] = op;
        switch (op) {
        case 0x01u: case 0x0cu: case 0x08u: case 0x09u:
            script[len++] = (random() & 3u) ? (uint8_t)random()
                                            : (uint8_t)(0xfbu + random() % 5u);
            break;
        case 0x0du: case 0x71u:
            script[len++] = (random() & 1u) ? wram[map_0692]
                                            : (uint8_t)random();
            break;
        case 0x1bu: case 0x1cu:
            script[len++] = (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x0au: case 0x19u: case 0x1eu: case 0x2bu: case 0x57u:
        case 0x86u: case 0xa2u: case 0xabu: case 0xb5u: case 0xb8u:
            break;
        case 0x79u:
            script[len++] = (uint8_t)random();
            break;
        case 0x6bu: case 0x29u: case 0xaau:
            break;
        case 0xa9u: case 0x26u: case 0x27u:
            /* Up to three arguments, $FF, then the target word. */
            for (unsigned a = random() % 4u; a > 0u; --a)
                script[len++] = (random() & 3u) ? (uint8_t)(random() % 0xfbu)
                                                : (uint8_t)(0xfbu + random() % 4u);
            script[len++] = 0xffu;
            break;
        case 0x24u:
            /* Tile x, tile y, spawn id. */
            script[len++] = (uint8_t)(random() & 0x7fu);
            script[len++] = (uint8_t)(random() & 0x7fu);
            script[len++] = (uint8_t)random();
            break;
        case 0x55u: case 0x69u: {
            /* Point, then own slot, point, entity or actor. */
            static const uint8_t kArea[4] = {0xfbu, 0xe0u, 0x60u, 0x20u};
            const unsigned kind = random() & 3u;

            script[len++] = (op == 0x69u && !(random() & 3u))
                ? (uint8_t)(0xa0u + (random() & 0x1fu))
                : (uint8_t)(0xe0u + (random() & 0x1fu));
            script[len++] = kind == 0u ? 0xfbu
                : (uint8_t)(kArea[kind] + ((random() & 15u) ? (random() & (kind == 1u ? 0x1fu : 7u)) : (random() & 0x1fu)));
            break;
        }
        case 0x64u: case 0x65u: case 0x66u: case 0x67u:
        case 0x7cu: case 0x7du: case 0x7eu: case 0x7fu: case 0x80u:
        case 0x81u: case 0xa3u: case 0xa4u: case 0xa5u: case 0xa6u:
        case 0xafu: case 0xb0u: case 0xb1u: case 0xb2u: {
            /* Target (mostly a point), object or actor, bytes. */
            const unsigned tail = op >= 0xafu ? 1u
                : (op <= 0x67u || op == 0x81u) ? 1u : 2u;

            script[len++] = (random() & 3u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                : (random() & 1u) ? 0xfbu : (uint8_t)(0x20u + (random() & 7u));
            script[len++] = op >= 0xafu ? (uint8_t)(random() % 0x28u)
                                        : (uint8_t)(random() & 7u);
            for (unsigned b = 0; b < tail; ++b)
                script[len++] = (uint8_t)random();
            break;
        }
        case 0xb6u: case 0xb7u:
            break;
        case 0x23u: case 0x6au: case 0xaeu:
            script[len++] = (uint8_t)(random() & 0x1fu);  /* action */
            break;
        case 0x0fu: case 0x6cu: case 0xa7u:
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x9cu: case 0x5eu:
            script[len++] = (uint8_t)(random() & 0x3fu);
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x41u: case 0x46u: case 0x4bu: case 0x50u: case 0x54u:
            /* Point, then a byte for set/add/subtract. */
            script[len++] = (uint8_t)(0xe0u + (random() & 0x1fu));
            script[len++] = (uint8_t)random();
            break;
        case 0x02u: case 0x03u: case 0x28u: case 0xa0u: case 0xa1u: {
            /* Object bit, mostly one of the listed keys 0-7. */
            const unsigned roll = random() % 8u;

            script[len++] = roll < 5u ? (uint8_t)(0xc0u + (random() & 7u))
                : roll < 6u ? (uint8_t)(0xc0u + random() % 0x3bu)
                : roll < 7u ? (uint8_t)(0xfbu + random() % 5u)
                            : (uint8_t)random();
            break;
        }
        case 0xbau:
            /* $F026 key, then a position operand. */
            script[len++] = (random() & 7u) ? (uint8_t)(random() & 7u)
                                            : (uint8_t)random();
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x1du: case 0x63u: case 0x10u: case 0x7bu: case 0x59u:
            break;
        case 0x87u: case 0x88u: case 0x89u:
            /* Layer mask, then two position operands. */
            script[len++] = (random() & 1u) ? (uint8_t)(1u << (random() & 3u))
                                            : (uint8_t)random();
            for (unsigned b = 0; b < 2u; ++b)
                script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x94u: case 0x95u: case 0x96u:
            /* Position, then a $F016 map object key. */
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            script[len++] = (random() & 7u) ? (uint8_t)(random() & 7u)
                                            : (uint8_t)random();
            break;
        case 0x97u: case 0x98u: case 0x99u: case 0x9au: case 0x60u: case 0x61u:
            /* Actor id (+$4F in $05FA). */
            script[len++] = (random() & 7u) ? (uint8_t)(random() & 7u)
                                            : (uint8_t)random();
            break;
        case 0x21u:
            /* Cell x, y near the pending objects of $7F:D69C. */
            script[len++] = (uint8_t)(random() & 3u);
            script[len++] = (uint8_t)(random() & 3u);
            break;
        case 0x5au: case 0x5bu: case 0x5cu: case 0x5du:
            /* Pending object slot ($7F:D69C), then byte $22. */
            script[len++] = (random() & 3u) ? (uint8_t)(random() % 0x30u)
                                            : (uint8_t)(0xa0u + (random() & 0x1fu));
            script[len++] = (uint8_t)random();
            break;
        case 0xb3u:
            /* Area operand, map object key (variable), $E316 byte. */
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x60u + (random() & 7u));
            script[len++] = (random() & 3u) ? (uint8_t)(random() & 7u)
                                            : (uint8_t)(0xfbu + random() % 5u);
            script[len++] = (uint8_t)random();
            break;
        case 0xbcu:
            /* Actor id (+$4F in $05FA), then a position operand. */
            script[len++] = (random() & 7u) ? (uint8_t)(random() & 7u)
                                            : (uint8_t)random();
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x2au:
            /* Position, then a $F016 map object key. */
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            script[len++] = (random() & 7u) ? (uint8_t)(random() & 7u)
                                            : (uint8_t)random();
            break;
        case 0x22u:
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x78u:
            /* Area operand, count (rarely 0 = 256), spawn id. */
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x60u + (random() & 7u));
            script[len++] = (random() & 15u) ? (uint8_t)(1u + (random() & 3u)) : (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x20u:
            /* Actor ids (+$4F in $05FA) to $FF, rarely 30-69. */
            for (unsigned a = (random() & 15u) ? random() % 4u : 30u + random() % 40u;
                 a > 0u; --a)
                script[len++] = (random() & 7u) ? (uint8_t)(random() & 7u)
                                                : (uint8_t)(random() % 0xffu);
            script[len++] = 0xffu;
            break;
        case 0x9fu:
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            script[len++] = (uint8_t)((random() & 3u) == 0 ? 0xfbu : (random() & 1u) ? 0xe0u + (random() & 0x1fu) : 0x20u + (random() & 7u));
            break;
        case 0x75u: case 0x76u: case 0x77u:
            script[len++] = (random() & 1u) ? wram[0x1d0f4u] : (uint8_t)random();
            break;
        case 0x12u: case 0x6du: case 0x13u: case 0x14u: case 0x15u:
        case 0x16u: case 0x17u: case 0x18u: case 0x6eu: case 0x6fu:
        case 0x72u: case 0x73u: case 0x74u: case 0x04u: case 0x05u:
        case 0x70u: {
            /* Position, box, entity box or condition list. */
            const unsigned kind = random() & 3u;

            script[len++] = kind == 0u ? (uint8_t)((random() & 1u) ? 0xfbu : 0xe0u + (random() & 0x1fu))
                : kind == 1u ? (uint8_t)(0x60u + (random() & 7u))
                : kind == 2u ? (uint8_t)(0x80u + ((random() & 7u) ? (random() & 3u) : (random() & 0x3fu)))
                             : (uint8_t)(0x20u + (random() & 7u));
            if (op == 0x04u || op == 0x05u || op == 0x70u)
                script[len++] = (uint8_t)(random() & 3u);  /* object type */
            break;
        }
        case 0xbdu:
            script[len++] = (uint8_t)random();
            break;
        case 0xb4u:
            /* dx, dy, speed. */
            for (unsigned b = 0; b < 3u; ++b)
                script[len++] = (uint8_t)random();
            break;
        case 0x82u:
            /* Point, placed object (FF: none). */
            script[len++] = (uint8_t)(0xe0u + (random() & 0x1fu));
            script[len++] = (random() & 7u) ? (uint8_t)(random() % 0x30u) : 0xffu;
            break;
        case 0x85u:
            script[len++] = (random() & 1u) ? (uint8_t)(0x20u + (random() & 7u))
                                            : (uint8_t)(0xe0u + (random() & 0x1fu));
            script[len++] = (uint8_t)random();
            break;
        case 0x25u:
            /* Own position, a point, rarely an actor; spawn id. */
            script[len++] = (random() & 3u) == 0 ? 0xfbu
                : (random() & 15u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                                   : (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x5fu: case 0x68u: case 0x8au: case 0x8bu:
            /* Actor slot, or a variable $A0-$BF. */
            script[len++] = (random() & 3u) ? (uint8_t)(random() % 0x28u)
                                            : (uint8_t)(0xa0u + (random() & 0x1fu));
            break;
        case 0x58u:
            /* Layer, dx, dy, x speed, y speed. */
            script[len++] = (uint8_t)(random() & 3u);
            for (unsigned b = 0; b < 4u; ++b)
                script[len++] = (uint8_t)random();
            break;
        case 0x83u: case 0x84u:
            /* Point $E0-$FF, then an offset. */
            script[len++] = (random() & 7u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                                            : (uint8_t)random();
            script[len++] = (uint8_t)random();
            break;
        case 0x9du: case 0x9eu:
            /* Variable, point value operand, point offset. */
            script[len++] = (uint8_t)(random() & 0x3fu);
            script[len++] = (random() & 7u) ? (uint8_t)(0xe0u + (random() & 0x1fu))
                                            : (uint8_t)random();
            script[len++] = (uint8_t)(random() & 0x1fu);
            break;
        default: {
            /* $2F-$40: variable, then a byte or a value operand. */
            const unsigned roll = random() % 8u;

            script[len++] = (random() & 3u)
                ? (uint8_t)(random() & 0x3fu)
                : (uint8_t)(0xfbu + random() % 5u);
            if (op == 0x32u || op == 0x33u)
                break;
            script[len++] = roll < 2u ? (uint8_t)(0xa0u + (random() & 0x1fu))
                : roll < 3u ? 0xfbu
                : roll < 4u ? (uint8_t)(0xfcu + random() % 4u)
                : roll < 5u ? (uint8_t)(0xc0u + (random() & 0x3fu))
                            : (uint8_t)random();
            break;
        }
        }
        if (op == 0x08u || op == 0x09u || op == 0x1bu || op == 0x1cu ||
            op == 0x57u || (op >= 0x2fu && op <= 0x34u) ||
            (op >= 0x79u && op != 0xa9u && op != 0x88u && op != 0x89u &&
             op != 0x95u && op != 0x96u) ||
            op == 0x5fu || op == 0x68u || op == 0x6bu || op == 0x58u ||
            op == 0x24u || op == 0x25u || op == 0x29u || op == 0xaau ||
            op == 0x13u || op == 0x15u || op == 0x17u || op == 0x72u || op == 0x75u ||
            op == 0x05u || op == 0x23u || op == 0x6au || op == 0xaeu ||
            (op >= 0x41u && op <= 0x54u) ||
            op == 0x02u || op == 0x03u || op == 0x28u ||
            op == 0xbau || op == 0x1du || op == 0x63u || op == 0x87u ||
            op == 0x10u || op == 0x7bu || op == 0x94u || op == 0x60u || op == 0x61u ||
            op == 0x59u || op == 0x21u || op == 0x22u || op == 0x2au ||
            (op >= 0x5au && op <= 0x5du) ||
            op == 0xa7u || op == 0x9cu || op == 0x9fu || op == 0x5eu ||
            op == 0x55u || op == 0x69u || op == 0x85u ||
            (op >= 0x64u && op <= 0x67u))
            continue;
        for (unsigned w = op == 0x1eu ? 2u : 1u; w > 0u; --w) {
            owner[patches] = (uint8_t)(n - 1u);
            words[patches++] = (uint16_t)len;
            len += 2u;
        }
    }
    starts[n] = (uint16_t)len;
    script[len++] = 0x00u;
    /* Condition lists: keys 0-3, then entity or point boxes to $FF. */
    {
        const unsigned table = len;
        uint16_t lists[4];

        script[10] = (uint8_t)table;
        script[11] = (uint8_t)(table >> 8);
        len += 13u;
        for (unsigned k = 0; k < 4u; ++k) {
            lists[k] = (uint16_t)len;
            for (unsigned e = random() % 4u; e > 0u; --e)
                script[len++] = (random() & 1u) ? (uint8_t)(random() & 7u)
                                               : (uint8_t)(0x80u + (random() & 0x1fu));
            script[len++] = 0xffu;
        }
        for (unsigned k = 0; k < 4u; ++k) {
            script[table + 3u * k] = (uint8_t)k;
            script[table + 3u * k + 1u] = (uint8_t)lists[k];
            script[table + 3u * k + 2u] = (uint8_t)(lists[k] >> 8);
        }
        script[table + 12u] = 0xffu;
    }
    /* Base + 0: event list, keys 0-3 to opcodes. */
    {
        const unsigned table = len;

        script[0] = (uint8_t)table;
        script[1] = (uint8_t)(table >> 8);
        for (unsigned k = 0; k < 4u; ++k) {
            const uint16_t target = starts[random() % n];

            script[len++] = (uint8_t)k;
            script[len++] = (uint8_t)target;
            script[len++] = (uint8_t)(target >> 8);
        }
        script[len++] = 0xffu;
    }
    /* Gotos jump to a later opcode, rarely anywhere. */
    for (unsigned p = 0; p < patches; ++p) {
        const unsigned later = owner[p] + 1u + random() % (n - owner[p]);
        const uint16_t target = (random() % 64u)
            ? starts[later] : (uint16_t)random();

        script[words[p]] = (uint8_t)target;
        script[words[p] + 1u] = (uint8_t)(target >> 8);
    }
    for (unsigned i = 0; i < len; ++i) {
        uint32_t at = (uint32_t)base + i;
        uint32_t offset = bank == 0x7eu ? 0u : 0x10000u;

        if (at > 0xffffu) {
            at = 0x8000u + (at - 0x10000u);
            offset += 0x10000u;
        }
        if (offset < 0x20000u)
            wram[offset + at] = script[i];
    }
    /* Map actor list ($7E:F022, stride 3), entity list ($F024,
       stride 5), $F016 (10), objects ($F002, 15), object rows
       ($F004, 10), $F026 (4) with keys 0-7 and an $FF end, rarely
       random. */
    for (unsigned l = 0; l < 6u; ++l) {
        static const unsigned kStrides[6] = {3u, 5u, 10u, 15u, 10u, 4u};
        static const uint16_t kHeads[6] = {
            0xf022u, 0xf024u, 0xf016u, 0xf002u, 0xf004u, 0xf026u};
        const unsigned stride = kStrides[l];
        const uint16_t list = (uint16_t)(
            (l < 3u ? 0x0100u + l * 0x80u : 0x0100u * l) + (random() & 0x1fu));
        const unsigned entries = random() % 8u;
        uint16_t at = (uint16_t)(0xf000u + list);

        const uint16_t head = kHeads[l];

        wram[head] = (uint8_t)list;
        wram[head + 1u] = (uint8_t)(list >> 8);
        for (unsigned e = 0; e < entries; ++e, at = (uint16_t)(at + stride)) {
            wram[at] = (uint8_t)(random() & 7u);
            for (unsigned b = 1; b < stride; ++b)
                wram[(uint16_t)(at + b)] = (uint8_t)(
                    l == 4u && b == 5u ? random() & 1u
                    : l == 2u && b == 5u ? 1u + (random() & 1u)
                    : b < 5u ? random() & 3u
                    : l == 3u && b == 13u ? random() & 7u : random());
        }
        if (random() & 15u)
            wram[at] = 0xffu;
    }
    /* Small coordinates so leader, points, slots, entities and
       placed objects meet. */
    for (unsigned b = 0; b < 2u; ++b) {
        wram[0x10000u * b + 0x06bau] = (uint8_t)(random() & 3u);
        wram[0x10000u * b + 0x06e2u] = (uint8_t)(random() & 3u);
    }
    for (unsigned k = 0; k < 32u; ++k) {
        const uint8_t x = (uint8_t)(random() & 3u), y = (uint8_t)(random() & 3u);

        wram[0x1d1a3u + k] = x;
        wram[0x1d1e3u + k] = y;
        wram[0x1d223u + k] = (uint8_t)(x + (random() % 3u));
        wram[0x1d263u + k] = (uint8_t)(y + (random() % 3u));
    }
    for (unsigned k = 0; k < 16u; ++k)
        wram[0x1d17cu + k] = (uint8_t)(random() & 3u);
    for (unsigned k = 0; k < 0x30u; ++k) {
        wram[0x1d69cu + k] = (uint8_t)(random() & 3u);
        wram[0x1d6ccu + k] = (uint8_t)(random() & 3u);
        wram[0x1d6fcu + k] = (uint8_t)(0x10u + (random() & 3u));
    }
    /* A script bank in WRAM reads quotient and remainder of the
       DB-relative divider from WRAM. */
    for (unsigned b = 0; b < 2u; ++b)
        if (random() & 3u) {
            wram[0x10000u * b + 0x4214u] = (uint8_t)(random() & 3u);
            wram[0x10000u * b + 0x4216u] = (uint8_t)((random() & 0x80u) + random() % 5u);
        }
    /* Map width, layer bases and a low-entropy tile corner so
       positions share tiles; actor ids in $05FA of both banks. */
    wram[0x05b9u] = (uint8_t)(0x20u + (random() & 0x10u));
    for (unsigned k = 0; k < 8u; ++k) {
        const uint16_t layer = (uint16_t)(0x2000u + 0x0800u * k);

        wram[0x1d008u + 2u * k] = (uint8_t)layer;
        wram[0x1d009u + 2u * k] = (uint8_t)(layer >> 8);
        for (unsigned t = 0; t < 0x140u; t += 2u) {
            wram[0x10000u + layer + t] = (uint8_t)(random() & 1u);
            wram[0x10001u + layer + t] = (uint8_t)(random() & 0xfcu);
        }
    }
    for (unsigned b = 0; b < 2u; ++b)
        for (unsigned k = 0; k < 0x28u; ++k)
            wram[0x10000u * b + 0x05fau + k] = (random() & 3u)
                ? (uint8_t)(0x4fu + (random() & 7u)) : (uint8_t)random();
    /* Layer modes ($FF skips), cameras in both banks and map sizes
       for $10 and $7B; $0100 divides by zero. */
    for (unsigned layer = 0; layer < 8u; layer += 2u) {
        static const uint8_t kModes[16] = {
            0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 0x80u, 0x85u, 0x05u};
        const uint16_t width = (random() & 7u) ? (uint16_t)(random() & 0x7fu) : 0x0100u;
        const uint16_t height = (random() & 7u) ? (uint16_t)(random() & 0x7fu) : 0x0100u;

        wram[0x1d020u + layer] = (random() & 7u) ? kModes[random() & 15u] : 0xffu;
        for (unsigned b = 0; b < 2u; ++b) {
            const uint16_t x = (uint16_t)(random() & 0x07ffu);
            const uint16_t y = (uint16_t)(random() & 0x07ffu);

            wram[0x10000u * b + 0x121eu + layer] = (uint8_t)x;
            wram[0x10000u * b + 0x121fu + layer] = (uint8_t)(x >> 8);
            wram[0x10000u * b + 0x1226u + layer] = (uint8_t)y;
            wram[0x10000u * b + 0x1227u + layer] = (uint8_t)(y >> 8);
        }
        wram[0x1d010u + layer] = (uint8_t)width;
        wram[0x1d011u + layer] = (uint8_t)(width >> 8);
        wram[0x1d018u + layer] = (uint8_t)height;
        wram[0x1d019u + layer] = (uint8_t)(height >> 8);
        wram[0x1d0deu + layer] = (uint8_t)(random() % 9u);
        wram[0x1d0dfu + layer] = 0;
        wram[0x1d0e6u + layer] = (uint8_t)(random() % 9u);
        wram[0x1d0e7u + layer] = 0;
    }
    /* $0583 bit 7 in both script banks; queued object animations. */
    for (unsigned b = 0; b < 2u; ++b) {
        wram[0x10000u * b + 0x0583u] = (random() & 3u) ? 0x00u : 0x80u;
        wram[0x10000u * b + 0x1261u] = (uint8_t)random();   /* bit 6: $59 */
    }
    wram[0x05aau] = (uint8_t)(2u * (random() & 3u));        /* layer */
    /* Mostly free attribute cells near the origin, so pushes pass
       $83:D89E. */
    for (unsigned k = 0; k < 0x180u; ++k)
        wram[0x4000u + k] = (random() & 3u) ? 0x00u
            : (random() & 1u) ? 0x80u : (uint8_t)random();
    /* Pending object types ($7F:D7FC by $D6FC) often 1; secondary
       actors on small cells. */
    for (unsigned k = 0x10u; k < 0x14u; ++k)
        wram[0x1d7fcu + k] = (random() & 1u) ? (uint8_t)(1u + (random() & 1u))
                                             : (uint8_t)random();
    /* Tile attributes ($7F:D03E base) at $7F:6000: 0, 1, 9 or any. */
    wram[0x1d03eu] = 0x00u;
    wram[0x1d03fu] = 0x60u;
    for (unsigned k = 0; k < 0x400u; ++k) {
        static const uint8_t kAttributes[4] = {0x00u, 0x01u, 0x09u, 0x02u};

        wram[0x16000u + k] = (random() & 3u) ? kAttributes[random() & 3u]
                                             : (uint8_t)random();
    }
    for (unsigned k = 0; k < 0x20u; ++k) {
        const uint16_t fine_x = (uint16_t)(((random() & 7u) << 4) | (random() & 15u));
        const uint16_t fine_y = (uint16_t)(((random() & 7u) << 4) | (random() & 15u));

        wram[0x1ddfeu + 2u * k] = (uint8_t)fine_x;
        wram[0x1ddffu + 2u * k] = (uint8_t)(fine_x >> 8);
        wram[0x1de8eu + 2u * k] = (uint8_t)fine_y;
        wram[0x1de8fu + 2u * k] = (uint8_t)(fine_y >> 8);
    }
    wram[0x05abu] = 0;
    for (unsigned k = 0; k < 8u; ++k) {
        wram[0x1d057u + k] = (random() & 1u) ? (uint8_t)(0x80u | random())
                                             : (uint8_t)(random() & 0x7fu);
        wram[0x1d04fu + k] = (uint8_t)(random() & 7u);
    }
    wram[0x1d194u] = (uint8_t)base;
    wram[0x1d195u] = (uint8_t)(base >> 8);
    /* Rarely no base script ($FF). */
    wram[0x1d196u] = (random() % 16u) ? bank : 0xffu;
    if (random() & 1u)
        wram[0x11273u] = 0;
    for (unsigned t = 0; t < 8u; ++t) {
        uint32_t at = (uint32_t)base + starts[random() % n];
        uint8_t at_bank = bank;

        if (!(random() % 8u))
            continue;
        if (at > 0xffffu) {
            at = 0x8000u + (at - 0x10000u);
            ++at_bank;
        }
        wram[0x1d134u + 3u * t] = (uint8_t)at;
        wram[0x1d135u + 3u * t] = (uint8_t)(at >> 8);
        wram[0x1d136u + 3u * t] = at_bank;
    }
}

/* Actor slot $A7 with its AB4F offsets; sprite slot runs in
   $7E:E100 mostly free. */
static void SeedLoadSprite(uint8_t *wram, uint16_t dp) {
    const uint8_t slot = (uint8_t)(NmiRandom() % 0x28u);

    wram[(uint16_t)(dp + 0xa7u)] = slot;
    wram[(uint16_t)(dp + 0xa8u)] = 0;
    wram[(uint16_t)(dp + 0xa9u)] = (uint8_t)(2u * slot);
    wram[(uint16_t)(dp + 0xaau)] = 0;
    wram[(uint16_t)(dp + 0xabu)] = (uint8_t)(3u * slot);
    wram[(uint16_t)(dp + 0xacu)] = 0;
    for (unsigned k = 0; k < 0x80u; ++k)
        wram[0xe100u + k] = (NmiRandom() & 3u) ? 0x00u : (uint8_t)NmiRandom();
}

/* Sprite graphics queue $05C2: sources, some empty, in both banks. */
static void SeedSpriteUpload(uint8_t *wram, uint16_t dp) {
    (void)dp;
    for (unsigned b = 0; b < 2u; ++b)
        for (unsigned k = 0; k < 0x10u; k += 2u)
            if (NmiRandom() & 1u) {
                wram[0x10000u * b + 0x05c2u + k] = 0;
                wram[0x10000u * b + 0x05c3u + k] = 0;
            }
}

/* Title particles $C448: half idle, ROM sprite lists, x near the
   list offset and the screen edges. */
static void SeedTitleParticles(uint8_t *wram, uint16_t dp) {
    static const uint8_t edges[4] = {0x00u, 0xffu, 0x01u, 0x00u};

    (void)dp;
    for (unsigned b = 0; b < 2u; ++b)
        for (unsigned k = 0; k < 75u; ++k) {
            uint8_t *record = wram + 0x10000u * b + 0xc448u + 13u * k;
            const uint16_t list = (uint16_t)(record[3] | (record[4] << 8));

            if (NmiRandom() & 1u) {
                record[1] = 0;
                continue;
            }
            if (NmiRandom() & 7u)
                record[4] |= 0x80u;
            if (NmiRandom() & 1u)
                record[5] = Rom8(g_seed_rom,
                    (0x860000u + (list | 0x8000u) + 2u) & 0x00ffffffu);
            if (NmiRandom() & 1u)
                record[6] = edges[NmiRandom() & 3u];
        }
}

/* Title objects: active, timers 0/1/long, angles on every
   quadrant and near the $B400 wrap, depth 0-10, half-free pools. */
static void SeedTitleObjects(uint8_t *wram, uint16_t dp) {
    (void)dp;
#define RANDOM NmiRandom
    static const uint16_t objects[3] = {0xc400u, 0xc418u, 0xc430u};
    static const uint16_t sprites[3] = {0xc448u, 0xc455u, 0xc462u};

    for (unsigned b = 0; b < 2u; ++b) {
        uint8_t *bank = wram + 0x10000u * b;

        for (unsigned k = 0; k < 3u; ++k) {
            uint8_t *object = bank + objects[k];
            const unsigned timer = RANDOM() & 7u;

            if (RANDOM() & 7u)
                object[0x12] |= 0x80u;
            object[0x08] = (uint8_t)(timer < 2u ? timer : RANDOM());
            object[0x09] = (uint8_t)(timer < 4u ? 0u : RANDOM());
            object[0x05] = (uint8_t)(RANDOM() % 0xb4u);
            object[0x07] = (uint8_t)(RANDOM() % 0xb4u);
            if (RANDOM() & 1u)
                object[0x14] = (uint8_t)(RANDOM() & 3u);
            if (RANDOM() & 1u)
                object[0x16] = (uint8_t)(RANDOM() & 3u);
            if ((RANDOM() & 3u) == 0) {
                object[0x05] = 0xb3u;
                object[0x04] = (uint8_t)(0xf0u | RANDOM());
            }
            object[0x17] = (uint8_t)(RANDOM() % 11u);
            if (RANDOM() & 1u)
                bank[sprites[k] + 1u] = 0;
        }
        for (unsigned k = 0; k < 72u; ++k)
            if (RANDOM() & 1u)
                bank[0xc46fu + 1u + 13u * k] = 0;
    }
#undef RANDOM
}

/* Title layer 10 on or off an 8-pixel step; frame 63 wraps. */
static void SeedTitleLayers(uint8_t *wram, uint16_t dp) {
    (void)dp;
    for (unsigned b = 0; b < 2u; ++b) {
        uint8_t *bank = wram + 0x10000u * b;

        if (NmiRandom() & 1u)
            bank[0x15b7u] = (uint8_t)(bank[0x15a1u] >> 3);
        if ((NmiRandom() & 3u) == 0)
            bank[0x15b8u] = 0x3fu;
    }
}

/* Title palette timer: due, one frame off, or wrapping. */
static void SeedTitlePalette(uint8_t *wram, uint16_t dp) {
    static const uint8_t timers[4] = {0x01u, 0x02u, 0x00u, 0x01u};

    (void)dp;
    for (unsigned b = 0; b < 2u; ++b) {
        uint8_t *bank = wram + 0x10000u * b;

        if (NmiRandom() & 3u) {
            bank[0x14b5u] = timers[NmiRandom() & 3u];
            bank[0x14b6u] = 0x00u;
        }
        if (NmiRandom() & 1u)
            bank[0x15b9u] = (uint8_t)(NmiRandom() & 1u);
    }
}

/* World sprite chain: shake decaying out, frame near $4AA, angle
   quadrants, y near the hide band. */
static void SeedWorldChain(uint8_t *wram, uint16_t dp) {
    for (unsigned b = 0; b < 2u; ++b) {
        uint8_t *bank = wram + 0x10000u * b;
        const uint32_t pick = NmiRandom();

        if (pick & 1u)
            bank[0x1e51u] = (uint8_t)(pick >> 8 & 0x03u);
        if (pick & 2u)
            bank[0x1e53u] = (uint8_t)(pick >> 10 & 0x03u);
        if (pick & 4u)
            bank[0x1e59u] = (uint8_t)(0xd8u + (pick >> 12 & 0x1fu));
    }
    if (NmiRandom() & 1u) {
        wram[(uint16_t)(dp + 0x42u)] = (uint8_t)(0xa8u + (NmiRandom() & 3u));
        wram[(uint16_t)(dp + 0x43u)] = 0x04u;
    }
}

/* Member 0-7, levels 1-99 around the factor steps and the 98 cap;
   level 0 or a set $09FF loops up to 65,536 times (MMIO log cap). */
static void SeedPartyLevel(uint8_t *wram, uint16_t dp) {
    static const uint8_t levels[8] = {1u, 2u, 8u, 9u, 97u, 98u, 99u, 16u};

    (void)dp;
    for (unsigned b = 0; b < 2u; ++b) {
        uint8_t *bank = wram + 0x10000u * b;
        const uint32_t pick = NmiRandom();

        bank[0x09fau] = (uint8_t)(pick & 7u);
        bank[0x09feu] = (pick & 0x18u)
            ? (uint8_t)(1u + (pick >> 8) % 97u) : levels[(pick >> 5) & 7u];
        bank[0x09ffu] = 0;
    }
}

/* Map attributes: small maps (1-32 cells a side), tile classes 0-15
   and $80+, terminated lists at $7E:F000. */
static void SeedMapAttributes(uint8_t *wram, uint16_t dp) {
    (void)dp;
    for (unsigned i = 0; i < 0x108u; ++i)
        wram[0x1d010u + i] = (uint8_t)(1u + NmiRandom() % 32u);
    for (unsigned i = 0; i < 0x400u; ++i)
        if (NmiRandom() & 1u)
            wram[0x10000u + (NmiRandom() & 0xffffu)] &= 0x0fu;
    for (unsigned i = 0; i < 0x40u; ++i)
        wram[0x1f000u + (NmiRandom() & 0x0fffu)] = 0xffu;
    for (unsigned i = 0; i < 0x30u; ++i)
        if (NmiRandom() & 1u)
            wram[0x1d69cu + i] = 0xffu;
}

/* Monster 0-6, form 1-5, levels 1-99, $0A7F mostly 7. */
static void CapsuleSeed(uint8_t *wram, uint32_t (*random)(void)) {
    const unsigned monster = random() % 7u;

    wram[0x0a7fu] = (random() & 7u) ? 0x07u : (uint8_t)random();
    wram[0x11a3u] = (uint8_t)monster;
    wram[0x11a4u] = (uint8_t)(1u + random() % 5u);
    for (unsigned i = 0; i < 7u; ++i)
        wram[0x1f1a3u + i] = (uint8_t)(1u + random() % 99u);
    wram[0x10edu] = (uint8_t)(1u + random() % 99u);
}

static void SeedCapsule(uint8_t *wram, uint16_t dp) {
    (void)dp;
    CapsuleSeed(wram, NmiRandom);
}

static void M1X0Registers(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
}

/* A a tilemap offset, X = width 4-32 << 8 | height 3-24. */
static void WindowRegisters(Lufia2CpuState *input) {
    const uint16_t size = input->x;

    input->accumulator_is_8_bit = 0;
    input->index_is_8_bit = 0;
    input->accumulator &= 0x03feu;
    input->x = (uint16_t)(((4u + (size >> 8) % 29u) << 8) |
        (3u + (size & 0xffu) % 22u));
}

/* X a stats block at $1000-$13FE. */
static void BlockRegisters(Lufia2CpuState *input) {
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(0x1000u + (input->x & 0x03feu));
}

/* No raw tile pending in most cases. */
static void SeedMenuScreen(uint8_t *wram, uint16_t dp) {
    (void)dp;
    if (NmiRandom() & 7u)
        wram[0x0578u] = 0x00u;
    for (unsigned i = 0; i < 8u; ++i)                       /* old digits */
        wram[0x0567u + i] = (uint8_t)((NmiRandom() & 3u)
            ? 0x30u + NmiRandom() % 10u : 0x20u);
    wram[0x056fu] = 0x00u;
}

/* Party of 1-4. */
static void SeedShopParty(uint8_t *wram, uint16_t dp) {
    SeedMenuScreen(wram, dp);
    wram[0x0a7au] = (uint8_t)(1u + NmiRandom() % 4u);
}

/* Capsule name at $10DF, as the load leaves it. */
static void SeedCapsuleScreen(uint8_t *wram, uint16_t dp) {
    SeedMenuScreen(wram, dp);
    CapsuleSeed(wram, NmiRandom);
    for (unsigned i = 0; i < 5u; ++i)
        wram[0x10dfu + i] = (uint8_t)(0x41u + NmiRandom() % 26u);
    wram[0x10e4u] = 0x00u;
}

/* Menus run with D = 0; [$08] strings read $0008 directly. */
static void MenuD0Registers(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->direct_page = 0;
}

/* X a layer-2 tile offset. */
static void TileRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->x &= 0x0ffeu;
}

/* Shop list at $7E:3800 through [$FC], real items, index 0-3. */
static void SeedShopList(uint8_t *wram, uint16_t dp) {
    SeedMenuScreen(wram, dp);
    wram[0x00fcu] = 0x00u;
    wram[0x00fdu] = 0x38u;
    wram[0x00feu] = 0x7eu;
    wram[0x0000u] = (uint8_t)(NmiRandom() & 3u);
    wram[0x0001u] = 0x00u;
    for (unsigned i = 0; i < 16u; ++i) {
        const uint16_t item = (NmiRandom() & 15u) ? (uint16_t)(1u + NmiRandom() % 466u) : 0u;

        wram[0x3800u + 2u * i] = (uint8_t)item;
        wram[0x3801u + 2u * i] = (uint8_t)(item >> 8);
    }
}

static void ShopM0Registers(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 0;
    input->index_is_8_bit = 0;
    input->direct_page = 0;
}

/* Members at $1000-$13FF with real items; $0A62 mostly $25. */
static void SeedEquipment(uint8_t *wram, uint16_t dp) {
    (void)dp;
    wram[0x0a62u] = (NmiRandom() & 7u) ? 0x25u : (uint8_t)NmiRandom();
    wram[0x14b3u] = (uint8_t)(NmiRandom() & 3u);
    for (unsigned m = 0; m < 4u; ++m) {
        wram[0x0a80u + 2u * m] = 0x00u;
        wram[0x0a81u + 2u * m] = (uint8_t)(0x10u + m);
        for (unsigned s = 0; s < 6u; ++s) {
            const uint16_t item = (NmiRandom() & 3u) ? (uint16_t)(1u + NmiRandom() % 466u) : 0u;

            wram[0x1066u + 0x100u * m + 2u * s] = (uint8_t)item;
            wram[0x1067u + 0x100u * m + 2u * s] = (uint8_t)(item >> 8);
        }
    }
}

/* Warp rows: town names (entry + 3, $8E:D06E table) or $FFFF. */
static void SeedWarpList(uint8_t *wram, uint16_t dp) {
    static const uint16_t kTowns[10] = {
        0xd0a8u, 0xd0b1u, 0xd0beu, 0xd0c8u, 0xd0d2u,
        0xd0deu, 0xd0ebu, 0xd0f7u, 0xd100u, 0xd111u};

    SeedMenuScreen(wram, dp);
    for (unsigned i = 0; i < 16u; ++i) {
        const uint16_t town = (NmiRandom() & 7u) ? (uint16_t)(kTowns[NmiRandom() % 10u] + 3u) : 0xffffu;

        wram[0x943cu + 2u * i] = (uint8_t)town;
        wram[0x943du + 2u * i] = (uint8_t)(town >> 8);
    }
}

/* Item/spell lists: mode $09D1, real items at $0A8D, spells at [$2A]. */
static void SeedItemList(uint8_t *wram, uint16_t dp) {
    static const uint8_t kModes[6] = {0x00u, 0x02u, 0x04u, 0x07u, 0xffu, 0x01u};

    SeedMenuScreen(wram, dp);
    wram[0x09d1u] = kModes[NmiRandom() % 6u];
    wram[0x14b5u] = (uint8_t)(NmiRandom() % 5u);
    wram[0x14b6u] = 0x00u;
    wram[0x0000u] = (uint8_t)(NmiRandom() % 5u);          /* row index */
    wram[0x0001u] = 0x00u;
    wram[0x151du] = (uint8_t)(NmiRandom() % 24u);           /* near spell costs */
    wram[0x151eu] = 0x00u;
    if ((NmiRandom() & 3u) == 0)                             /* bits in the third word */
        for (unsigned i = 0; i < 4u; ++i)
            wram[0x091eu + i] = 0x00u;
    wram[0x0922u] &= 0x03u;                                  /* 34 scenario items */
    wram[0x0923u] = 0x00u;
    wram[0x0924u] = 0x00u;
    wram[0x0925u] = 0x00u;
    for (unsigned i = 0; i < 32u; ++i) {
        const uint16_t item = (NmiRandom() & 7u)
            ? (uint16_t)((1u + NmiRandom() % 466u) | ((NmiRandom() & 15u) << 9)) : 0u;

        wram[0x0a8du + 2u * i] = (uint8_t)item;
        wram[0x0a8eu + 2u * i] = (uint8_t)(item >> 8);
    }
    wram[0x002au] = 0x00u;
    wram[0x002bu] = 0x18u;                  /* low WRAM: any DB */
    for (unsigned i = 0; i < 32u; ++i)
        wram[0x1800u + i] = (NmiRandom() & 7u) ? (uint8_t)(NmiRandom() % 40u) : 0xffu;
}

/* Sprite slots: a quarter active, bank $7E. Frames at $3000 (64
   bytes: timer, size, 1-6 pieces, $FF); frame 28 loops ($FF), 29
   holds ($FE). Lists at $3800: 16 frames, entry 0 real. */
static void SpriteSeed(uint8_t *wram, uint32_t (*random)(void)) {
    for (unsigned f = 0; f < 30u; ++f) {
        uint8_t *frame = wram + 0x3000u + 64u * f;
        const unsigned pieces = 1u + random() % 6u;

        frame[0] = f == 28u ? 0xffu : f == 29u ? 0xfeu : (uint8_t)(1u + random() % 0xfcu);
        frame[1] = (uint8_t)random();
        for (unsigned p = 0; p < pieces; ++p)
            for (unsigned b = 0; b < 4u; ++b)
                frame[2u + 4u * p + b] = b == 0 ? (uint8_t)(random() % 0xffu) : (uint8_t)random();
        frame[2u + 4u * pieces] = 0xffu;
    }
    for (unsigned l = 0; l < 64u; ++l) {
        unsigned previous_real = 1;

        for (unsigned e = 0; e < 16u; ++e) {
            unsigned f = random() % 28u;
            const unsigned special = random() & 7u;
            uint16_t at;

            if (e > 0 && special == 0)
                f = 28u;
            else if (e > 0 && special == 1u && previous_real)
                f = 29u;
            previous_real = f < 28u;
            at = (uint16_t)(0x3000u + 64u * f);
            wram[0x3800u + 32u * l + 2u * e] = (uint8_t)at;
            wram[0x3801u + 32u * l + 2u * e] = (uint8_t)(at >> 8);
        }
    }
    for (unsigned x = 0; x < 48u; ++x) {
        const uint16_t list = (uint16_t)(0x3800u + 32u * (random() % 64u));
        const uint16_t frame = (uint16_t)(0x3000u + 64u * (random() % 28u));

        wram[0x11d8u + x] = (random() & 3u) ? 0x00u : 0x01u;
        wram[0x1238u + x] = 0x7eu;
        wram[0x1478u + x] = (random() & 1u) ? 0x01u : (uint8_t)random();
        wram[0x1448u + x] = (uint8_t)(random() % 15u);
        wram[0x12c8u + x] = (uint8_t)list;
        wram[0x12f8u + x] = (uint8_t)(list >> 8);
        wram[0x1328u + x] = (uint8_t)frame;
        wram[0x1358u + x] = (uint8_t)(frame >> 8);
    }
}

static void SeedSprites(uint8_t *wram, uint16_t dp) {
    (void)dp;
    SpriteSeed(wram, NmiRandom);
}

/* Stored member at $1800: level 1-99 at +level_at, a 1-12 letter
   name at +name_at ending in $FF; member 0-7. */
static void MemberSource(uint8_t *wram, unsigned level_at, unsigned name_at) {
    const unsigned length = 1u + NmiRandom() % 12u;

    wram[0x1800u + level_at] = (uint8_t)(1u + NmiRandom() % 99u);
    for (unsigned i = 0; i < length; ++i)
        wram[0x1800u + name_at + i] = (uint8_t)(0x41u + NmiRandom() % 26u);
    wram[0x1800u + name_at + length] = 0xffu;
    wram[0x09fau] = (uint8_t)(NmiRandom() & 7u);
    wram[0x09fbu] = 0x00u;
}

static void SeedMemberFull(uint8_t *wram, uint16_t dp) {
    (void)dp;
    MemberSource(wram, 6u, 10u);
}

static void SeedMemberBare(uint8_t *wram, uint16_t dp) {
    (void)dp;
    MemberSource(wram, 0u, 4u);
}

static void SeedLevelStats(uint8_t *wram, uint16_t dp) {
    (void)dp;
    wram[0x09fau] = (uint8_t)(NmiRandom() & 7u);
    wram[0x09feu] = (uint8_t)(1u + NmiRandom() % 99u);
    wram[0x09ffu] = 0x00u;
}

/* X the stored member, Y a block at $1000-$13FE. */
static void MemberRegisters(Lufia2CpuState *input) {
    input->index_is_8_bit = 0;
    input->x = 0x1800u;
    input->y = (uint16_t)(0x1000u + (input->y & 0x03feu));
}

static void Bank97Registers(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->data_bank = 0x97u;
}

/* Member $1BE8 (0-3), blocks at $1000 + $100 * n with real items. */
static void SeedIpSkills(uint8_t *wram, uint16_t dp) {
    (void)dp;
    wram[0x1be8u] = (uint8_t)(NmiRandom() & 3u);
    wram[0x1be9u] = 0x00u;
    for (unsigned m = 0; m < 4u; ++m) {
        wram[0x0a64u + 2u * m] = 0x00u;
        wram[0x0a65u + 2u * m] = (uint8_t)(0x10u + m);
        for (unsigned s = 0; s < 6u; ++s) {
            const uint16_t item = (NmiRandom() & 3u) ? (uint16_t)(1u + NmiRandom() % 466u) : 0u;

            wram[0x1066u + 0x100u * m + 2u * s] = (uint8_t)item;
            wram[0x1067u + 0x100u * m + 2u * s] = (uint8_t)(item >> 8);
        }
    }
}

/* Battle list: count $17, mode $1B 0-3, table $7E:DF00 random. */
static void SeedBattleList(uint8_t *wram, uint16_t dp) {
    wram[(uint16_t)(dp + 0x17u)] = (uint8_t)(NmiRandom() % 24u);
    wram[(uint16_t)(dp + 0x1bu)] = (uint8_t)(NmiRandom() & 3u);
}

/* X a list entry 2-25. */
static void ListRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(2u + input->x % 24u);
}

/* Popup slots: types 0/80/04/03/02/01, some kind 8, half zero words. */
static void PopupSeed(uint8_t *wram, uint32_t (*random)(void)) {
    static const uint8_t kTypes[6] = {0x00u, 0x80u, 0x04u, 0x03u, 0x02u, 0x01u};

    for (unsigned s = 0; s < 11u; ++s) {
        const uint32_t base = 0x1f60cu + 0x1eu * s;

        wram[base] = kTypes[random() % 6u];
        if ((random() & 3u) == 0u)
            wram[base + 2u] = 0x08u;
        for (unsigned w = 0; w < 13u; ++w) {
            if (random() & 1u) {
                wram[base + 4u + 2u * w] = 0x00u;
                wram[base + 5u + 2u * w] = 0x00u;
            }
        }
    }
    wram[0x11deu] = (random() & 1u) ? 0x00u : 0x01u;
}

static void SeedPopups(uint8_t *wram, uint16_t dp) {
    (void)dp;
    PopupSeed(wram, NmiRandom);
}

/* A a popup mode 0-5. */
static void PopupRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->accumulator = (uint16_t)((input->accumulator & 0xff00u) | (input->accumulator % 6u));
}

/* Palette source bank $24: WRAM or ROM. */
static void SeedPaletteBank(uint8_t *wram, uint16_t dp) {
    static const uint8_t kBanks[4] = {0x7eu, 0x7fu, 0x81u, 0x85u};

    wram[(uint16_t)(dp + 0x24u)] = kBanks[NmiRandom() & 3u];
}

static void M0X0Registers(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 0;
    input->index_is_8_bit = 0;
}

/* Blocks: tile $00, 1-8 x 1-8, target $7E:2000-$9FFE. */
static void BlockSeed(uint8_t *wram, uint16_t dp, uint32_t (*random)(void)) {
    const uint16_t at = (uint16_t)(0x2000u + (random() & 0x7ffeu));

    wram[(uint16_t)(dp + 0x02u)] = (uint8_t)(1u + random() % 8u);
    wram[(uint16_t)(dp + 0x03u)] = (uint8_t)(1u + random() % 8u);
    wram[(uint16_t)(dp + 0x08u)] = (uint8_t)at;
    wram[(uint16_t)(dp + 0x09u)] = (uint8_t)(at >> 8);
}

static void SeedBlocks(uint8_t *wram, uint16_t dp) {
    BlockSeed(wram, dp, NmiRandom);
}

/* A a nonzero target mask. */
static void MaskRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    if ((input->accumulator & 0xffu) == 0)
        input->accumulator |= 0x01u;
}

/* Frame: width 0-30, row pointer $0200-$11FE. */
static void FrameSeed(uint8_t *wram, uint32_t (*random)(void)) {
    const uint16_t row = (uint16_t)(0x0200u + (random() & 0x0ffeu));

    wram[0x09fau] = (uint8_t)(random() % 31u);
    wram[0x09fbu] = 0x00u;
    wram[0x09fcu] = (uint8_t)row;
    wram[0x09fdu] = (uint8_t)(row >> 8);
}

/* Party pointers $0A80: none or a record at $1000 + $100 * n. */
static void PartyPointerSeed(uint8_t *wram, uint32_t (*random)(void)) {
    for (unsigned n = 0; n < 4u; ++n) {
        const int used = (random() & 3u) != 0u;

        wram[0x0a80u + 2u * n] = 0x00u;
        wram[0x0a81u + 2u * n] = used ? (uint8_t)(0x10u + n) : 0x00u;
    }
}

static void SeedFrame(uint8_t *wram, uint16_t dp) {
    (void)dp;
    FrameSeed(wram, NmiRandom);
}

static void SeedPartyPointers(uint8_t *wram, uint16_t dp) {
    (void)dp;
    PartyPointerSeed(wram, NmiRandom);
}

/* M0X0, Y a row at $1000-$17FE, X a ROM template. */
static void FrameRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 0;
    input->index_is_8_bit = 0;
    input->y = (uint16_t)(0x1000u + (input->y & 0x07feu));
    input->x = (uint16_t)(0x8000u | (input->x & 0x7ffeu));
}

/* M0X0, X a buffer at $1000-$17FE. */
static void GaugeRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 0;
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(0x1000u + (input->x & 0x07feu));
}

/* Stat record $C1 at $1000 + $100 * n. */
static void StatSeed(uint8_t *wram, uint16_t dp, uint32_t (*random)(void)) {
    static const uint16_t capped_stats[4] = {0u, 199u, 200u, 201u};
    const uint16_t record = (uint16_t)(0x1000u + 0x100u * (random() & 3u));
    const unsigned variant = random();

    wram[(uint16_t)(dp + 0xc1u)] = 0x00u;
    wram[(uint16_t)(dp + 0xc2u)] = (uint8_t)(record >> 8);
    if (variant & 1u) {
        wram[record + 0x78u] = wram[record + 0x79u] = 0;
    }
    if (variant & 2u) {
        wram[record + 0x7au] = wram[record + 0x7bu] = 0;
    }
    if (variant & 4u) {
        const uint16_t stat = capped_stats[(variant >> 3) & 3u];

        wram[record + 0x5bu] = (uint8_t)stat;
        wram[record + 0x5cu] = (uint8_t)(stat >> 8);
        wram[record + 0x82u] = wram[record + 0x83u] = 0;
        wram[record + 0x90u] = wram[record + 0x91u] = 0;
    }
}

/* Fill: 1-32 columns, 1-16 rows. */
static void FillSeed(uint8_t *wram, uint32_t (*random)(void)) {
    wram[0x09f2u] = (uint8_t)(1u + random() % 32u);
    wram[0x09f3u] = (uint8_t)(1u + random() % 16u);
}

static void SeedStatRecord(uint8_t *wram, uint16_t dp) {
    StatSeed(wram, dp, NmiRandom);
}

static void SeedFillRect(uint8_t *wram, uint16_t dp) {
    (void)dp;
    FillSeed(wram, NmiRandom);
}

/* X a tilemap in $7E:2000-$9FFE. */
static void FillRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(0x2000u + (input->x & 0x7ffeu));
}

/* Window: 1-30 x 0-8, template in ROM. */
static void BattleWindowSeed(uint8_t *wram, uint32_t (*random)(void)) {
    const uint16_t from = (uint16_t)(0x8000u | (random() & 0x7ffeu));

    wram[0x09f2u] = (uint8_t)(1u + random() % 30u);
    wram[0x09f3u] = (uint8_t)(random() % 9u);
    wram[0x09f4u] = (uint8_t)from;
    wram[0x09f5u] = (uint8_t)(from >> 8);
}

static void SeedBattleWindow(uint8_t *wram, uint16_t dp) {
    (void)dp;
    BattleWindowSeed(wram, NmiRandom);
}

/* X a row at $1000-$17FE. */
static void BattleWindowRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(0x1000u + (input->x & 0x07feu));
}

static void SeedFilledWindow(uint8_t *wram, uint16_t dp) {
    (void)dp;
    BattleWindowSeed(wram, NmiRandom);
    if (wram[0x09f3u] == 0)
        wram[0x09f3u] = 1u;
}

/* Portraits 0-6 or none, pose slot $11/$12 at D 0. */
static void PortraitSeed(uint8_t *wram, uint32_t (*random)(void)) {
    static const uint8_t kPoses[3] = {0x00u, 0x02u, 0x05u};

    for (unsigned n = 0; n < 4u; ++n) {
        const uint32_t r = random();

        wram[0x153du + n] = (r & 7u) == 7u ? 0xffu : (uint8_t)(r % 7u);
    }
    wram[0x0011u] = (uint8_t)(random() & 3u);
    wram[0x0012u] = kPoses[random() % 3u];
}

static void SeedPortraits(uint8_t *wram, uint16_t dp) {
    (void)dp;
    PortraitSeed(wram, NmiRandom);
}

/* D 0, DB $97, A a slot 0-3. */
static void PortraitRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->direct_page = 0;
    input->data_bank = 0x97u;
    input->accumulator = (uint16_t)((input->accumulator & 0xff00u) | (input->accumulator & 3u));
}

/* Fade: palette $11 0-15, level $13 0-$40. */
static void FadeSeed(uint8_t *wram, uint16_t dp, uint32_t (*random)(void)) {
    const uint32_t r = random();

    wram[(uint16_t)(dp + 0x11u)] = (uint8_t)(r & 0x0fu);
    wram[(uint16_t)(dp + 0x13u)] = (r & 0x300u) == 0 ? 0x40u
        : (r & 0x300u) == 0x100u ? 0x00u : (uint8_t)((r >> 10) % 0x41u);
}

static void SeedFade(uint8_t *wram, uint16_t dp) {
    FadeSeed(wram, dp, NmiRandom);
}

/* Tile frame: 1-30 x 2-8. */
static void TileFrameSeed(uint8_t *wram, uint32_t (*random)(void)) {
    wram[0x09f2u] = (uint8_t)(1u + random() % 30u);
    wram[0x09f3u] = (uint8_t)(2u + random() % 7u);
}

static void SeedTileFrame(uint8_t *wram, uint16_t dp) {
    (void)dp;
    TileFrameSeed(wram, NmiRandom);
}

/* X $0800-$0FFE: frame lands at $10C0-$1B3E. */
static void TileFrameRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(0x0800u + (input->x & 0x07feu));
}

/* Record [$B2] at $7E:1000-$17FE, character 0-15. */
static void NewRecordSeed(uint8_t *wram, uint16_t dp, uint32_t (*random)(void)) {
    const uint16_t at = (uint16_t)(0x1000u + (random() & 0x07feu));

    wram[(uint16_t)(dp + 0xb2u)] = (uint8_t)at;
    wram[(uint16_t)(dp + 0xb3u)] = (uint8_t)(at >> 8);
    wram[0x09f2u] = (uint8_t)(random() & 0x0fu);
}

static void SeedInventoryAdd(uint8_t *wram, uint16_t dp) {
    (void)dp;
    InventoryAddSeed(wram, NmiRandom);
}

static void GlyphRegisters(Lufia2CpuState *input) {
    static unsigned character;
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->accumulator = (uint16_t)((input->accumulator & 0xff00u) | (character++ & 0xffu));
}

static void SeedLevelUp(uint8_t *wram, uint16_t dp) {
    LevelUpSeed(wram, dp, g_seed_rom, NmiRandom);
}

static void SeedNewRecord(uint8_t *wram, uint16_t dp) {
    NewRecordSeed(wram, dp, NmiRandom);
}

/* X a sprite slot 0-47. */
static void SlotRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(input->x % 48u);
}

/* Generated strings for $80:8878. */
typedef struct MenuSeed {
    uint8_t *wram;
    uint32_t (*random)(void);
    uint16_t next;
    uint16_t next0;
    uint16_t cells;
    int spells;                         /* spell names end only if D & $FF is 0 */
} MenuSeed;

enum { MENU_SEED_EMPTY = 0x2ff0u, MENU_SEED_EMPTY0 = 0x17f0u };

static void MenuSeedByte(MenuSeed *s, uint16_t *at, unsigned value) {
    s->wram[(*at)++] = (uint8_t)value;
}

static void MenuSeedWord(MenuSeed *s, uint16_t *at, unsigned value) {
    MenuSeedByte(s, at, value);
    MenuSeedByte(s, at, value >> 8);
}

/* Anything readable; only reads go through it. */
static uint16_t MenuSeedAny(MenuSeed *s) {
    return (uint16_t)(0x3000u + (s->random() & 0x0fffu));
}

static uint16_t MenuSeedCell(MenuSeed *s, unsigned size) {
    const uint16_t cell = s->cells;

    s->cells = (uint16_t)(s->cells + size);
    return cell;
}

static uint16_t MenuSeedString(MenuSeed *s, int depth, int bank0);

static void MenuSeedNested(MenuSeed *s, uint16_t *at, int depth) {
    const unsigned kind = s->random() & 3u;
    const unsigned high = s->random() & 0x80u;
    const int bank0 = kind >= 2u && (s->random() & 1u);
    const uint16_t target = MenuSeedString(s, depth + 1, bank0);
    uint16_t cell;

    MenuSeedByte(s, at, 0x02u);
    MenuSeedByte(s, at, kind | high);
    switch (kind) {
    case 0:
        MenuSeedWord(s, at, target);
        break;
    case 1:
        cell = MenuSeedCell(s, 2);
        s->wram[cell] = (uint8_t)target;
        s->wram[cell + 1u] = (uint8_t)(target >> 8);
        MenuSeedWord(s, at, cell);
        break;
    case 2:
        MenuSeedWord(s, at, target);
        MenuSeedByte(s, at, bank0 ? 0x00u : 0x7eu);
        break;
    default:
        cell = MenuSeedCell(s, 3);
        s->wram[cell] = (uint8_t)target;
        s->wram[cell + 1u] = (uint8_t)(target >> 8);
        s->wram[cell + 2u] = bank0 ? 0x00u : 0x7eu;
        MenuSeedWord(s, at, cell);
        break;
    }
}

/* $02 4-9: member, item or spell names. */
static void MenuSeedName(MenuSeed *s, uint16_t *at) {
    unsigned kind = 4u + s->random() % 6u;
    const int offset = kind >= 7u;
    uint16_t cell;

    if (!s->spells && (kind == 6u || kind == 9u))
        --kind;
    MenuSeedByte(s, at, 0x02u);
    MenuSeedByte(s, at, kind | (s->random() & 0x80u));
    cell = MenuSeedCell(s, 2);
    if (kind == 4u || kind == 7u) {
        s->wram[cell] = (uint8_t)(s->random() & 7u);
    } else {
        /* Real ids only (467 items, 40 spells); high bits masked. */
        const int item = kind == 5u || kind == 8u;
        const uint16_t id = (uint16_t)((s->random() & (item ? 0xfe00u : 0xff00u)) |
            (s->random() % (item ? 467u : 40u)));

        s->wram[cell] = (uint8_t)id;
        s->wram[cell + 1u] = (uint8_t)(id >> 8);
    }
    if (!offset) {
        MenuSeedWord(s, at, cell);
    } else {
        const uint16_t base = MenuSeedCell(s, 2);
        const uint16_t word = (uint16_t)s->random();

        s->wram[base] = (uint8_t)word;
        s->wram[base + 1u] = (uint8_t)(word >> 8);
        MenuSeedWord(s, at, base);
        MenuSeedWord(s, at, (uint16_t)(cell - word));
    }
}

static void MenuSeedBranch(MenuSeed *s, uint16_t *at, int depth) {
    const unsigned kind = s->random() & 3u;
    const unsigned flags = s->random() & 0xf8u;

    MenuSeedByte(s, at, 0x03u);
    MenuSeedByte(s, at, kind | flags);
    if (kind < 3u) {
        MenuSeedWord(s, at, MenuSeedAny(s));
        MenuSeedWord(s, at, s->random());
        if (flags & 0x08u) {
            MenuSeedWord(s, at, MenuSeedAny(s));
            MenuSeedWord(s, at, s->random());
        }
    }
    MenuSeedWord(s, at, MenuSeedString(s, depth, 0));
}

static uint16_t MenuSeedString(MenuSeed *s, int depth, int bank0) {
    uint16_t *next = bank0 ? &s->next0 : &s->next;
    const uint16_t limit = bank0 ? 0x1780u : 0x2f80u;
    const uint16_t start = *next;
    uint16_t at = start;
    unsigned count = 1u + s->random() % 6u;

    if (start > limit || s->cells > 0x3780u)
        return bank0 ? MENU_SEED_EMPTY0 : MENU_SEED_EMPTY;
    *next = (uint16_t)(start + 0x50u);
    while (count--) {
        const unsigned pick = s->random() % 26u;

        if (pick < 8u || (bank0 && pick >= 8u && pick < 13u)) {
            unsigned n = 1u + s->random() % 3u;

            while (n--)
                MenuSeedByte(s, &at, 0x10u + s->random() % 0xf0u);
            continue;
        }
        switch (pick) {
        case 8: {                                               /* number */
            const unsigned bytes = s->random() % 3u;
            const unsigned top = 0x80u >> bytes;
            const unsigned format = top | (s->random() & (top - 1u));

            MenuSeedByte(s, &at, 0x01u);
            MenuSeedByte(s, &at, format);
            MenuSeedByte(s, &at, s->random() & 7u);
            MenuSeedWord(s, &at, MenuSeedAny(s));
            if ((format & 3u) == 1u)
                MenuSeedByte(s, &at, s->random());
            break;
        }
        case 9:
            if (depth < 2)
                MenuSeedNested(s, &at, depth);
            break;
        case 10:
            MenuSeedName(s, &at);
            break;
        case 11:
            MenuSeedBranch(s, &at, depth);
            break;
        case 12:                                                /* raw tile */
            MenuSeedByte(s, &at, 0x04u);
            MenuSeedByte(s, &at, 0x10u + s->random() % 0xf0u);
            break;
        case 13:
            MenuSeedByte(s, &at, 0x05u);
            break;
        case 14:
            MenuSeedByte(s, &at, 0x06u);
            MenuSeedWord(s, &at, s->random() & 0x03feu);
            break;
        case 15:
        case 16:
        case 18:
        case 23:
            MenuSeedByte(s, &at, pick == 15u ? 0x07u : pick == 16u ? 0x08u
                : pick == 18u ? 0x0au : 0x0fu);
            break;
        case 17:
            MenuSeedByte(s, &at, 0x09u);
            MenuSeedByte(s, &at, s->random());
            break;
        case 19:
            MenuSeedByte(s, &at, 0x0bu);
            MenuSeedWord(s, &at, 0x4000u + (s->random() & 0x1ffeu));
            break;
        case 20:
            MenuSeedByte(s, &at, 0x0cu);
            MenuSeedByte(s, &at, s->random());
            break;
        case 21:
            MenuSeedByte(s, &at, 0x0du);
            MenuSeedWord(s, &at, MenuSeedAny(s));
            MenuSeedWord(s, &at, MenuSeedAny(s));
            break;
        case 22:
            MenuSeedByte(s, &at, 0x0eu);
            MenuSeedWord(s, &at, MenuSeedAny(s));
            break;
        case 24:                                                /* rare hand-offs */
            if ((s->random() & 7u) == 0) {
                switch (s->random() % 3u) {
                case 0:                                         /* $02 table end */
                    MenuSeedByte(s, &at, 0x02u);
                    MenuSeedByte(s, &at, 0x0au + s->random() % 0x76u);
                    break;
                case 1:                                         /* $03 table end */
                    MenuSeedByte(s, &at, 0x03u);
                    MenuSeedByte(s, &at, 0x04u + (s->random() & 3u));
                    break;
                default:                                        /* raw control */
                    MenuSeedByte(s, &at, 0x09u);
                    MenuSeedByte(s, &at, 0x10u + s->random() % 0x70u);
                    MenuSeedByte(s, &at, 0x04u);
                    MenuSeedByte(s, &at, 0x01u + s->random() % 0x0fu);
                    break;
                }
            }
            break;
        default:
            MenuSeedByte(s, &at, 0x0au);
            break;
        }
    }
    MenuSeedByte(s, &at, 0x00u);
    return start;
}

/* Main string at $7E:2000 plus member names. */
static void MenuSeedAll(uint8_t *wram, uint16_t dp, uint32_t (*random)(void)) {
    MenuSeed s;
    unsigned member;

    s.wram = wram;
    s.random = random;
    s.next = 0x2000u;
    s.next0 = 0x1000u;
    s.cells = 0x3000u;
    s.spells = (dp & 0x00ffu) == 0;
    wram[MENU_SEED_EMPTY] = 0x00u;
    wram[MENU_SEED_EMPTY0] = 0x00u;
    wram[(uint16_t)(dp + 0x5fu)] = 0x7eu;
    for (member = 0; member < 8u; ++member)                    /* old digits */
        wram[0x0567u + member] = (uint8_t)((random() & 3u)
            ? 0x30u + random() % 10u : 0x20u);
    wram[0x056fu] = 0x00u;
    wram[0x0578u] = 0x00u;
    wram[0x0579u] = 0x00u;
    wram[0x057au] = 0x44u;
    for (member = 0; member < 8u; ++member) {
        const uint16_t name = MenuSeedString(&s, 2, 1);

        wram[0x0a80u + 2u * member] = (uint8_t)name;
        wram[0x0a81u + 2u * member] = (uint8_t)(name >> 8);
    }
    (void)MenuSeedString(&s, 0, 0);
}

static void SeedMenuString(uint8_t *wram, uint16_t dp) {
    MenuSeedAll(wram, dp, NmiRandom);
}

/* X a tile slot, Y the string; 16-bit index. */
static void MenuStringRegisters(Lufia2CpuState *input) {
    input->index_is_8_bit = 0;
    input->x = (uint16_t)(0x4000u + (input->x & 0x1ffeu));
    input->y = 0x2000u;
}

/* Item $0A06 and spell $0A0B stay random. */
static void SeedAnyRecord(uint8_t *wram, uint16_t dp) {
    (void)wram;
    (void)dp;
}

static void SpellRegisters(Lufia2CpuState *input) {
    input->accumulator_is_8_bit = 1;
    input->index_is_8_bit = 0;
}

/* A real resource ($A7:8000; entry 680 is empty) unpacked to $2000+ of
   bank $7E or $7F, clear of the stack and direct page. */
static void SeedResource(uint8_t *wram, uint16_t dp) {
    const uint16_t entry = (uint16_t)(NmiRandom() % 680u);
    const uint16_t low = Rom16(g_seed_rom, 0xa78000u + 3u * entry);
    const uint16_t high = Rom16(g_seed_rom, 0xa78001u + 3u * entry);
    const uint32_t shifted = (uint32_t)high << 1;
    const uint8_t bank = (uint8_t)(0xa7u + ((shifted >> 8) & 0xffu) +
        (shifted >> 16));
    const uint16_t length =
        Rom16(g_seed_rom, ((uint32_t)bank << 16) | (uint16_t)(low | 0x8000u));
    const uint16_t start = (uint16_t)(0x2000u +
        NmiRandom() % (0xe000u - (length < 0xd000u ? length : 0xd000u)));

    wram[(uint16_t)(dp + 0x54u)] = (uint8_t)entry;
    wram[(uint16_t)(dp + 0x55u)] = (uint8_t)(entry >> 8);
    wram[(uint16_t)(dp + 0x60u)] = (uint8_t)start;
    wram[(uint16_t)(dp + 0x61u)] = (uint8_t)(start >> 8);
    wram[(uint16_t)(dp + 0x62u)] = (uint8_t)(NmiRandom() & 1u);
}

/* Event slot timers: idle, waiting, due ($81) or wrapping ($80). */
static void SeedEventTimers(uint8_t *wram, uint16_t dp) {
    const unsigned mode = NmiRandom() & 7u;

    (void)dp;
    for (unsigned t = 0; t < 8u; ++t) {
        const unsigned roll = NmiRandom() % 16u;

        wram[0x1d18cu + t] = roll < 6u ? (uint8_t)(NmiRandom() & 0x7fu)
            : roll < 11u ? (uint8_t)(0x82u + NmiRandom() % 0x7eu)
            : roll < 13u ? 0x80u : 0x81u;
    }
    SeedEventScripts(wram, NmiRandom);
    if (mode < 2u)
        for (unsigned t = 0; t < 8u; ++t)
            wram[0x1d18cu + t] &= 0x7fu;
    if (mode == 1u)
        wram[0x1d18cu + NmiRandom() % 8u] =
            (uint8_t)(0x82u + NmiRandom() % 0x7eu);
}

static void SeedTitleState(uint8_t *wram, uint16_t dp) {
    wram[dp + 0x30u] = (NmiRandom() & 15u) ? (uint8_t)(NmiRandom() % 9u)
                                      : (uint8_t)NmiRandom();
}

static const SmallTarget kSmallTargets[] = {
    {"83A0", 0x8383a0u, Lufia2FieldMenuRequest, 2, 0x838079u,
     SeedMenuRequest},
    {"867B", 0x83867bu, Lufia2FieldTakeButtons, 2, 0x8380b2u,
     SeedTakeButtons},
    {"8103", 0x838103u, Lufia2FieldStatusRequests, 2, 0x838091u,
     SeedStatusRequests},
    {"E746", 0x82e746u, Lufia2TitleStateDispatch, 3, 0x8ec2f3u,
     SeedTitleState},
    {"85DC", 0x8385dcu, Lufia2FieldReloadSetup, 3, 0x8ec30fu,
     SeedFieldReload},
    {"939C", 0x82939cu, Lufia2MenuNmi, 3, 0x000069u,
     SeedMenuNmi},
    {"8B4B", 0x828b4bu, Lufia2MenuButtons, 3, 0x828b1fu,
     SeedMenuButtons},
    {"9313", 0x829313u, Lufia2MenuWindowRequest, 2, 0x828b38u,
     SeedMenuWindow},
    {"C627", 0x82c627u, Lufia2MenuCursorBlink, 2, 0x828b3bu,
     SeedMenuBlink},
    {"B66E", 0x83b66eu, Lufia2FieldStairRects, 2, 0x838267u,
     SeedFieldRects},
    {"B711", 0x83b711u, Lufia2FieldEventRects, 2, 0x83827cu,
     SeedFieldRects},
    {"B747", 0x83b747u, Lufia2FieldAreaRects, 2, 0x83826au,
     SeedFieldRects},
    {"86C1", 0x8086c1u, Lufia2ScreenFade, 2, 0x808663u,
     SeedScreenFade},
    {"9EDD", 0x869eddu, Lufia2WorldMapRegionSearch, 2, 0x869e67u,
     SeedWorldRegion},
    {"9CB8", 0x809cb8u, Lufia2TextEngineStep, 3, 0x83807bu,
     SeedTextStep},
    {"81A9", 0x8681a9u, Lufia2SelectScreenNmi, 3, 0x000069u,
     SeedNmiTables},
    {"92A4", 0x8092a4u, Lufia2IntroNmi, 3, 0x000069u,
     SeedIntroNmi},
    {"B452", 0x85b452u, Lufia2BattleScript, 3, 0x81faecu,
     SeedBattleScript},
    {"CBAE", 0x80cbaeu, Lufia2FieldEventTimerTick, 3, 0x83823au,
     SeedEventTimers},
    {"A9BA", 0x83a9bau, Lufia2ActorLoadSprite, 3, 0x83a791u,
     SeedLoadSprite},
    {"8193", 0x848193u, Lufia2SpriteGraphicsUpload, 3, 0x83a7b4u,
     SeedSpriteUpload},
    {"88BE", 0x8688beu, Lufia2TitleParticleSprites, 2, 0x8682f4u,
     SeedTitleParticles},
    {"838C", 0x86838cu, Lufia2TitleObjects, 2, 0x8682eau,
     SeedTitleObjects},
    {"86ED", 0x8686edu, Lufia2TitleLayers, 2, 0x8682e7u,
     SeedTitleLayers},
    {"8996", 0x868996u, Lufia2TitlePaletteCycle, 2, 0x8682edu,
     SeedTitlePalette},
    {"E8CE", 0x86e8ceu, Lufia2WorldSpriteChain, 2, 0x86937du,
     SeedWorldChain},
    {"F9E9", 0x81f9e9u, Lufia2PartyExperienceForLevel, 3, 0x81f98bu,
     SeedPartyLevel},
    {"ED9C", 0x80ed9cu, Lufia2FieldBuildAttributes, 3, 0x8eb2e5u,
     SeedMapAttributes},
    {"8E9D", 0x808e9du, Lufia2DecompressResource, 3, 0x8091f9u,
     SeedResource},
    {"8878", 0x808878u, Lufia2MenuDrawString, 3, 0x828516u,
     SeedMenuString, MenuStringRegisters},
    {"F1C5", 0x81f1c5u, Lufia2LoadItemRecord, 3, 0x808b80u,
     SeedAnyRecord},
    {"F414", 0x81f414u, Lufia2LoadSpellRecord, 3, 0x808b64u,
     SeedAnyRecord, SpellRegisters},
    {"810E", 0x82810eu, Lufia2MenuDrawWindow, 2, 0x829b26u,
     SeedAnyRecord, WindowRegisters, Lufia2DecompBridge_810E, 2},
    {"C261", 0x82c261u, Lufia2CapsuleLoadStats, 3, 0x81ed33u,
     SeedCapsule, M1X0Registers},
    {"F4D5", 0x81f4d5u, Lufia2PartyDerivedStats, 3, 0x82d31au,
     SeedAnyRecord, BlockRegisters},
    {"C515", 0x82c515u, Lufia2CapsuleSetForms, 3, 0x82c34cu,
     SeedCapsule, M1X0Registers},
    {"C2FD", 0x82c2fdu, Lufia2CapsuleReset, 3, 0x848908u,
     SeedCapsule, M1X0Registers, Lufia2DecompBridge_C2FD, 4},
    {"C352", 0x82c352u, Lufia2CapsuleSetAll, 3, 0x83ad91u,
     SeedCapsule, M1X0Registers},
    {"CD83", 0x82cd83u, Lufia2CapsuleLevelUp, 3, 0x81dc52u,
     SeedCapsule, M1X0Registers, Lufia2DecompBridge_CD83, 4},
    {"8CF5", 0x868cf5u, Lufia2SpriteSetAnimation, 3, 0x82897bu,
     SeedAnyRecord, SlotRegisters, Lufia2DecompBridge_8CF5, 4},
    {"F194", 0x81f194u, Lufia2ItemRecordByte, 3, 0x82bd81u,
     SeedAnyRecord, NULL, Lufia2DecompBridge_F194, 4},
    {"9F6F", 0x829f6fu, Lufia2MenuEquipCommands, 2, 0x82b7ddu,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_9F6F, 4},
    {"D721", 0x82d721u, Lufia2MenuShopWindows, 2, 0x82daa3u,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_D721, 4},
    {"E49E", 0x82e49eu, Lufia2MenuShopTitle, 2, 0x82da9du,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_E49E, 4},
    {"EFC5", 0x82efc5u, Lufia2MenuSavedWindow, 2, 0x82eb57u,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_EFC5, 4},
    {"F0A2", 0x82f0a2u, Lufia2MenuNameEntryWindows, 2, 0x82ebf9u,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_82F0A2, 4},
    {"D749", 0x82d749u, Lufia2MenuShopParty, 2, 0x82daa6u,
     SeedShopParty, M1X0Registers, Lufia2DecompBridge_D749, 4},
    {"A2E3", 0x82a2e3u, Lufia2MenuCapsuleScreen, 2, 0x82c172u,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_A2E3, 4},
    {"950E", 0x82950eu, Lufia2MenuMemberStatus, 2, 0x82a41bu,
     SeedMenuScreen, TileRegisters, Lufia2DecompBridge_950E, 4},
    {"D07B", 0x82d07bu, Lufia2MenuCapsuleStatus, 2, 0x82a16cu,
     SeedCapsuleScreen, MenuD0Registers, Lufia2DecompBridge_D07B, 4},
    {"E297", 0x82e297u, Lufia2MenuShopSetup, 2, 0x82da9au,
     SeedMenuScreen, M1X0Registers, Lufia2DecompBridge_E297, 4},
    {"CE23", 0x82ce23u, Lufia2CapsuleExperienceRange, 2, 0x82c09bu,
     SeedCapsule, M1X0Registers, Lufia2DecompBridge_CE23, 4},
    {"CD1F", 0x82cd1fu, Lufia2CapsuleTryLearn, 3, 0x81dcc1u,
     SeedCapsule, M1X0Registers, Lufia2DecompBridge_CD1F, 4},
    {"E5E1", 0x82e5e1u, Lufia2MenuShopCompare, 2, 0x82dc26u,
     SeedShopParty, M1X0Registers, Lufia2DecompBridge_E5E1, 4},
    {"DCF4", 0x82dcf4u, Lufia2MenuShopRow, 2, 0x82dd4du,
     SeedShopList, ShopM0Registers, Lufia2DecompBridge_DCF4, 2},
    {"DCC1", 0x82dcc1u, Lufia2MenuShopRows, 2, 0x82dd4du,
     SeedShopList, MenuD0Registers, Lufia2DecompBridge_DCC1, 4},
    {"B2C5", 0x82b2c5u, Lufia2MenuEquipUpgrade, 2, 0x82ac29u,
     SeedEquipment, M1X0Registers, Lufia2DecompBridge_B2C5, 4},
    {"8378", 0x808378u, Lufia2Divide16, 3, 0x828cdau,
     SeedAnyRecord, M1X0Registers, Lufia2DecompBridge_8378, 4},
    {"8CDA", 0x868cdau, Lufia2SpriteSetTable, 3, 0x829f94u,
     SeedAnyRecord, SlotRegisters, Lufia2DecompBridge_8CDA, 5},
    {"F3F4", 0x81f3f4u, Lufia2SpellRecordByteC, 3, 0x82ac29u,
     SeedAnyRecord, SpellRegisters, Lufia2DecompBridge_F3F4, 4},
    {"F404", 0x81f404u, Lufia2SpellRecordByte8, 3, 0x82ac29u,
     SeedAnyRecord, SpellRegisters, Lufia2DecompBridge_F404, 4},
    {"9CB2", 0x829cb2u, Lufia2MenuWarpList, 2, 0x82af9fu,
     SeedWarpList, MenuD0Registers, Lufia2DecompBridge_9CB2, 4},
    {"A918", 0x82a918u, Lufia2MenuListCursor, 2, 0x82a7ffu,
     SeedItemList, MenuD0Registers, Lufia2DecompBridge_A918, 4},
    {"ACDB", 0x82acdbu, Lufia2MenuListRow, 2, 0x82ad55u,
     SeedItemList, MenuD0Registers, Lufia2DecompBridge_ACDB, 4},
    {"8B55", 0x868b55u, Lufia2SpriteFrame, 3, 0x828b05u,
     SeedSprites, M1X0Registers},
    {"8B73", 0x868b73u, Lufia2SpriteAnimateAll, 3, 0x868b5fu,
     SeedSprites, M1X0Registers},
    {"8BCF", 0x868bcfu, Lufia2SpriteClearOam, 3, 0x868b63u,
     SeedSprites, M1X0Registers},
    {"8BF5", 0x868bf5u, Lufia2SpriteBuildOam, 3, 0x868b67u,
     SeedSprites, M1X0Registers},
    {"F87F", 0x81f87fu, Lufia2PartyBaseStats, 3, 0x81ee32u,
     SeedLevelStats, Bank97Registers, Lufia2DecompBridge_F87F, 4},
    {"ED8E", 0x81ed8eu, Lufia2PartyUnpackMember, 2, 0x81c000u,
     SeedMemberFull, MemberRegisters, Lufia2DecompBridge_ED8E, 3},
    {"EE94", 0x81ee94u, Lufia2PartyUnpackMemberBare, 2, 0x81c000u,
     SeedMemberBare, MemberRegisters, Lufia2DecompBridge_EE94, 3},
    {"C129", 0x81c129u, Lufia2BattleIpSkills, 2, 0x81d14bu,
     SeedIpSkills, M1X0Registers, Lufia2DecompBridge_C129, 4},
    {"DFA2", 0x81dfa2u, Lufia2BattleListRows, 2, 0x81d000u,
     SeedBattleList, ListRegisters, Lufia2DecompBridge_DFA2, 4},
    {"C35F", 0x81c35fu, Lufia2BattlePopups, 3, 0x81aa9au,
     SeedPopups, PopupRegisters},
    {"B264", 0x81b264u, Lufia2BattleActiveMask, 2, 0x81b22du,
     SeedAnyRecord, M1X0Registers},
    {"B2B5", 0x81b2b5u, Lufia2BattleTargetRecord, 3, 0x819442u,
     SeedAnyRecord, M1X0Registers},
    {"B2DB", 0x81b2dbu, Lufia2BattleTargetSlot, 3, 0x81c68bu,
     SeedAnyRecord, M1X0Registers},
    {"B505", 0x81b505u, Lufia2BattleBlend, 2, 0x81b4ceu,
     SeedAnyRecord, M1X0Registers},
    {"B54A", 0x81b54au, Lufia2ColorToGray, 2, 0x81b325u,
     SeedAnyRecord, M0X0Registers},
    {"B5A3", 0x81b5a3u, Lufia2BattleHideOam, 2, 0x81b5c8u,
     SeedAnyRecord, M1X0Registers},
    {"B974", 0x81b974u, Lufia2BattleLoadPalette, 2, 0x8186ceu,
     SeedPaletteBank, M1X0Registers},
    {"B9AF", 0x81b9afu, Lufia2BattleCommitPalettes, 3, 0x8186e7u,
     SeedAnyRecord, M1X0Registers},
    {"C2C0", 0x81c2c0u, Lufia2BattleLoadDisplayDefaults, 2, 0x818626u,
     SeedAnyRecord, M1X0Registers},
    {"C2D0", 0x81c2d0u, Lufia2BattleClearBackgroundTilemap, 2, 0x818543u,
     SeedAnyRecord, M1X0Registers},
    {"C2E3", 0x81c2e3u, Lufia2BattleResetPartyTilemap, 2, 0x818546u,
     SeedAnyRecord, M1X0Registers},
    {"C2FB", 0x81c2fbu, Lufia2BattleClearWindowTilemap, 2, 0x818549u,
     SeedAnyRecord, M1X0Registers},
    {"C30E", 0x81c30eu, Lufia2BattleClearTilemap3800, 2, 0x81854cu,
     SeedAnyRecord, M1X0Registers},
    {"C5CF", 0x81c5cfu, Lufia2BattleTargetPointer, 2, 0x81b1d3u,
     SeedAnyRecord, MaskRegisters},
    {"BE58", 0x81be58u, Lufia2BattleTileBlock, 2, 0x81be56u,
     SeedBlocks, M1X0Registers},
    {"BD4B", 0x81bd4bu, Lufia2BattleSpriteBlock, 2, 0x81bd49u,
     SeedBlocks, M1X0Registers},
    {"E479", 0x81e479u, Lufia2BattleFrameTop, 2, 0x81e442u,
     SeedFrame, FrameRegisters},
    {"E4AD", 0x81e4adu, Lufia2BattleFrameSides, 2, 0x81e44eu,
     SeedFrame, FrameRegisters},
    {"E542", 0x81e542u, Lufia2BattleFrameRow, 2, 0x81e526u,
     SeedFrame, FrameRegisters},
    {"E570", 0x81e570u, Lufia2BattleFrameEnds, 2, 0x81e534u,
     SeedFrame, FrameRegisters},
    {"E5C1", 0x81e5c1u, Lufia2BattleGaugeBlock, 2, 0x81e5a3u,
     SeedAnyRecord, GaugeRegisters},
    {"E604", 0x81e604u, Lufia2BattleGaugeColumn, 2, 0x81e5acu,
     SeedAnyRecord, GaugeRegisters},
    {"F291", 0x81f291u, Lufia2ItemTextPointer, 3, 0x81cd61u,
     SeedAnyRecord, M1X0Registers},
    {"F446", 0x81f446u, Lufia2SpellTextPointer, 2, 0x81f3e6u,
     SeedAnyRecord, M1X0Registers},
    {"F5ED", 0x81f5edu, Lufia2PartyRestore11, 3, 0x80a4e6u,
     SeedPartyPointers, M1X0Registers},
    {"F60B", 0x81f60bu, Lufia2PartyRestore13, 3, 0x80a4eau,
     SeedPartyPointers, M1X0Registers},
    {"F7BD", 0x81f7bdu, Lufia2BattleTable9EBA, 2, 0x81ecb3u,
     SeedAnyRecord, M1X0Registers},
    {"FB79", 0x81fb79u, Lufia2CharacterSpriteByte, 3, 0x8184beu,
     SeedAnyRecord, M1X0Registers},
    {"EC41", 0x81ec41u, Lufia2BattleClearF000, 2, 0x81ec3bu,
     SeedAnyRecord, M1X0Registers},
    {"F2A9", 0x81f2a9u, Lufia2ItemNameTrimmed, 3, 0x81f1dau,
     SeedAnyRecord, M1X0Registers},
    {"F4ED", 0x81f4edu, Lufia2PartyStatTotals, 2, 0x81f4bbu,
     SeedStatRecord, M1X0Registers},
    {"F78D", 0x81f78du, Lufia2PartyPointers, 2, 0x81f74cu,
     SeedAnyRecord, M1X0Registers},
    {"FBA2", 0x81fba2u, Lufia2SpriteSizePacked, 3, 0x858925u,
     SeedAnyRecord, M1X0Registers},
    {"FBDB", 0x81fbdbu, Lufia2CharacterSpriteBox, 3, 0x81c6eeu,
     SeedAnyRecord, M1X0Registers},
    {"FCE2", 0x81fce2u, Lufia2CharacterSpritePointer, 3, 0x81b1dfu,
     SeedAnyRecord, M1X0Registers},
    {"E7D2", 0x81e7d2u, Lufia2BattleFillRect, 2, 0x81e184u,
     SeedFillRect, FillRegisters},
    {"E808", 0x81e808u, Lufia2DecimalDigits3, 2, 0x81e1fau,
     SeedAnyRecord, M1X0Registers},
    {"EB34", 0x81eb34u, Lufia2BattlePaletteCopy, 2, 0x81ea45u,
     SeedAnyRecord, M1X0Registers},
    {"EB62", 0x81eb62u, Lufia2BattlePaletteSplit, 2, 0x81ea95u,
     SeedAnyRecord, M1X0Registers},
    {"F057", 0x81f057u, Lufia2InventoryCount, 3, 0x83c48du,
     SeedAnyRecord, M1X0Registers},
    {"E503", 0x81e503u, Lufia2BattleWindow, 2, 0x81cc65u,
     SeedBattleWindow, BattleWindowRegisters},
    {"E593", 0x81e593u, Lufia2BattleGaugePanel, 2, 0x81deeeu,
     SeedAnyRecord, M1X0Registers},
    {"F4E9", 0x81f4e9u, Lufia2PartyStatTotalsFar, 3, 0x85c717u,
     SeedStatRecord, M1X0Registers},
    {"F789", 0x81f789u, Lufia2PartyPointersFar, 3, 0x85c6ebu,
     SeedAnyRecord, M1X0Registers},
    {"BD47", 0x81bd47u, Lufia2BattleSpriteBlockFar, 3, 0x858ba7u,
     SeedBlocks, M1X0Registers},
    {"BE54", 0x81be54u, Lufia2BattleTileBlockFar, 3, 0x858db3u,
     SeedBlocks, M1X0Registers},
    {"E3AE", 0x81e3aeu, Lufia2BattleWindowE3AE, 3, 0x81ddacu,
     SeedFilledWindow, BattleWindowRegisters},
    {"E3CD", 0x81e3cdu, Lufia2BattleWindowE3CD, 3, 0x859a22u,
     SeedFilledWindow, BattleWindowRegisters},
    {"BAE8", 0x81bae8u, Lufia2BattlePortraits, 3, 0x818664u,
     SeedPortraits, PortraitRegisters},
    {"BAFB", 0x81bafbu, Lufia2BattlePortrait, 2, 0x81baf3u,
     SeedPortraits, PortraitRegisters},
    {"BB75", 0x81bb75u, Lufia2BattlePortraitUpload, 2, 0x819b75u,
     SeedPortraits, PortraitRegisters},
    {"B48B", 0x81b48bu, Lufia2BattleFadeColor, 2, 0x81b47bu,
     SeedFade, M1X0Registers},
    {"B444", 0x81b444u, Lufia2BattlePaletteFade, 3, 0x85eb1au,
     SeedFade, M1X0Registers},
    {"E405", 0x81e405u, Lufia2BattleTileFrame, 2, 0x81e403u,
     SeedTileFrame, TileFrameRegisters},
    {"E3EC", 0x81e3ecu, Lufia2BattleTileWindow, 2, 0x81dec8u,
     SeedTileFrame, TileFrameRegisters},
    {"F979", 0x81f979u, Lufia2PartyLevelUpCheck, 2, 0x81f7d6u,
     SeedLevelUp, M1X0Registers},
    {"FC0B", 0x81fc0bu, Lufia2PartyNewRecord, 3, 0x8180fdu,
     SeedNewRecord, M1X0Registers},
    {"F0A2", 0x81f0a2u, Lufia2InventoryAdd, 2, 0x81f098u,
     SeedInventoryAdd, M1X0Registers, Lufia2DecompBridge_F0A2, 4},
    {"E835", 0x81e835u, Lufia2BattleGlyph, 2, 0x81d1cau,
     SeedAnyRecord, GlyphRegisters},
    {"C652", 0x80c652u, Lufia2TextMeasure, 2, 0x809d45u,
     SeedTextMeasure, RegistersTextMeasure, Lufia2DecompBridge_C652, 4},
    {"C784", 0x80c784u, Lufia2TextClearGlyphBuffer, 3, 0x80c246u,
     SeedTextGlyphClear, NULL, Lufia2DecompBridge_C784, 4},
    {"C56E", 0x80c56eu, Lufia2TextQueueWindowRow, 2, 0x80bcf4u,
     SeedTextWindowRow, RegistersTextWindowRow, Lufia2DecompBridge_C56E, 4},
    {"C305", 0x80c305u, Lufia2TextBuildWindow, 2, 0x80c27bu,
     SeedTextBuildWindow, RegistersTextBuildWindow, Lufia2DecompBridge_C305, 4},
    {"C23D", 0x80c23du, Lufia2TextPrepareWindow, 2, 0x809d48u,
     SeedTextPrepareWindow, RegistersTextBuildWindow, Lufia2DecompBridge_C23D, 4},
    {"FB1F", 0x82fb1fu, Lufia2ItemPossessionCount, 3, 0x80a1edu,
     SeedPossessionCount, RegistersPossessionCount, Lufia2DecompBridge_FB1F, 2},
    {"A1CC", 0x80a1ccu, Lufia2TextConditionRecord, 2, 0x80a165u,
     SeedConditionChild, RegistersConditionRecord},
    {"A1E4", 0x80a1e4u, Lufia2TextConditionItem, 2, 0x80a128u,
     SeedConditionChild, RegistersConditionChild},
    {"A074", 0x80a074u, VerifyTextCondition, 0, 0,
     SeedConditionExpression, RegistersConditionChild},
};

enum { SMALL_TARGETS = sizeof(kSmallTargets) / sizeof(kSmallTargets[0]) };

typedef struct TextWindowBusObservation {
    SnesVerifyBusEvent events[128];
    unsigned count;
    bool overflow;
    uint32_t join_read;
} TextWindowBusObservation;

/* Observe window/condition temporary frames before subsequent pushes erase
 * them, and the otherwise discarded M1 window join read. Instruction fetches
 * are outside both ranges. Full CPU/WRAM and MMIO checks stay independent. */
static void ObserveTextWindowBus(void *opaque, uint32_t address,
    uint8_t value, bool write) {
    TextWindowBusObservation *o = (TextWindowBusObservation *)opaque;
    if (!((write && address >= 0x001fd0u && address <= 0x001ff0u) ||
        (!write && address == o->join_read))) return;
    if (o->count == 128u) { o->overflow = true; return; }
    SnesVerifyBusEvent *event = &o->events[o->count++];
    event->address = address; event->value = value; event->write = write;
}

static bool SmallBridgeSameState(const CpuState *cpu, const Interp816 *ref) {
    return cpu->A == ref->a && cpu->X == ref->x && cpu->Y == ref->y &&
        cpu->S == ref->sp && cpu->D == ref->dp && cpu->DB == ref->db &&
        cpu->m_flag == (uint8_t)ref->mf && cpu->x_flag == (uint8_t)ref->xf &&
        cpu->_flag_C == (uint8_t)ref->c && cpu->_flag_Z == (uint8_t)ref->z &&
        cpu->_flag_N == (uint8_t)ref->n && cpu->_flag_V == (uint8_t)ref->v &&
        cpu->_flag_D == (uint8_t)ref->d && cpu->_flag_I == (uint8_t)ref->i;
}

static void SmallBridgeLoadCpu(
    CpuState *cpu, SnesVerifyBus *bus, const Lufia2CpuState *input,
    uint8_t hrv) {
    memset(cpu, 0, sizeof(*cpu));
    cpu->A = input->accumulator;
    cpu->X = input->x;
    cpu->Y = input->y;
    cpu->S = input->stack;
    cpu->D = input->direct_page;
    cpu->DB = input->data_bank;
    cpu->PB = input->program_bank;
    cpu->m_flag = input->accumulator_is_8_bit;
    cpu->x_flag = input->index_is_8_bit;
    cpu->_flag_C = input->carry;
    cpu->_flag_Z = input->zero;
    cpu->_flag_N = input->negative;
    cpu->_flag_V = input->overflow;
    cpu->_flag_D = input->decimal;
    cpu->_flag_I = input->irq_disable;
    cpu->host_return_valid = hrv;
    cpu->ram = bus->wram;
    cpu_mirrors_to_p(cpu);
}

static bool SmallBridgeSupported(
    const SmallTarget *target, const Lufia2CpuState *input) {
    if (input->decimal) return false;
    if (target->entry == 0x81f194u && !input->accumulator_is_8_bit)
        return !input->index_is_8_bit && input->program_bank == 0x81u &&
            input->data_bank == 0x7eu && input->direct_page == 0u &&
            input->stack >= 0x1f10u && input->stack <= 0x1ffcu;
    switch (target->bridge_width) {
    case 1: return true;
    case 2: return !input->accumulator_is_8_bit && !input->index_is_8_bit;
    case 3: return !input->index_is_8_bit;
    case 4: return input->accumulator_is_8_bit && !input->index_is_8_bit;
    case 5: return input->accumulator_is_8_bit;
    default: return false;
    }
}

/* Reuse the exact structured state already compared with the ROM above. */
static bool VerifySmallBridge(
    SnesVerifyBus *bus, const Interp816 *reference, const uint8_t *initial,
    const BusRegisters *initial_registers, const Lufia2CpuState *input,
    const SmallTarget *target, const Lufia2ExecutionResult *result,
    const uint8_t *expected_wram, const SnesVerifyBusEvent *expected_mmio,
    size_t expected_mmio_count, unsigned case_index) {
    const uint32_t exit = (target->call_return + 1u) & 0xffffffu;
    const BusRegisters reference_registers = SaveBusRegisters(bus);
    const unsigned mode = case_index % 3u;
    const uint8_t hrv = mode == 0u ? target->frame : mode == 1u ? 0u :
        target->frame == 2u ? 3u : 2u;
    CpuState cpu;
    RecompReturn ret;
    bool flow_ok;

    if (!target->bridge) return true;
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    RestoreBusRegisters(bus, initial_registers);
    SnesVerifyBusResetMmio(bus);
    g_bridge_bus = bus;
    memset(&g_small_bridge, 0, sizeof(g_small_bridge));
    SmallBridgeLoadCpu(&cpu, bus, input, hrv);
    ret = target->bridge(&cpu);

    if (!SmallBridgeSupported(target, input)) {
        flow_ok = ret == (RecompReturn)111 &&
            g_small_bridge.calls == 1u &&
            g_small_bridge.flow == SMALL_BRIDGE_TAIL &&
            g_small_bridge.target == target->entry &&
            g_small_bridge.site == target->entry &&
            g_small_bridge.entry_s == input->stack &&
            g_small_bridge.hrv == hrv &&
            cpu.A == input->accumulator && cpu.X == input->x &&
            cpu.Y == input->y && cpu.S == input->stack &&
            cpu.D == input->direct_page && cpu.DB == input->data_bank &&
            memcmp(bus->wram, initial, SNES_VERIFY_WRAM_SIZE) == 0 &&
            bus->mmio_count == 0u &&
            SameBusRegisters(bus, initial_registers);
    } else if (result->flow == LUFIA2_EXECUTION_BOUNDARY) {
        flow_ok = ret == (RecompReturn)111 &&
            g_small_bridge.calls == 1u &&
            g_small_bridge.flow == SMALL_BRIDGE_TAIL &&
            g_small_bridge.target == result->pc &&
            g_small_bridge.site == result->pc &&
            g_small_bridge.entry_s == input->stack &&
            g_small_bridge.hrv == hrv;
    } else if (mode == 0u) {
        flow_ok = ret == RECOMP_RETURN_NORMAL &&
            g_small_bridge.calls == 0u;
    } else {
        flow_ok = ret == (RecompReturn)111 &&
            g_small_bridge.calls == 1u &&
            g_small_bridge.flow == SMALL_BRIDGE_DISPATCH &&
            g_small_bridge.target == exit &&
            g_small_bridge.site == result->pc &&
            g_small_bridge.restore_s ==
                (uint16_t)(input->stack + target->frame);
    }
    if (SmallBridgeSupported(target, input))
        flow_ok = flow_ok && SmallBridgeSameState(&cpu, reference) &&
            memcmp(bus->wram, expected_wram, SNES_VERIFY_WRAM_SIZE) == 0 &&
            bus->mmio_count == expected_mmio_count && !bus->mmio_overflow &&
            memcmp(bus->mmio, expected_mmio,
                sizeof(SnesVerifyBusEvent) * expected_mmio_count) == 0 &&
            SameBusRegisters(bus, &reference_registers);
    if (!flow_ok) {
        if (target->entry == 0x81f194u)
            fprintf(stderr, "F194 input M=%u X=%u PB=%02X DB=%02X DP=%04X S=%04X\n",
                input->accumulator_is_8_bit, input->index_is_8_bit,
                input->program_bank, input->data_bank,
                input->direct_page, input->stack);
        fprintf(stderr,
            "FAIL %s bridge case %u: flow=%u target=%06X site=%06X "
            "return=%d A=%04X/%04X S=%04X/%04X\n",
            target->name, case_index, (unsigned)g_small_bridge.flow,
            g_small_bridge.target, g_small_bridge.site, (int)ret,
            cpu.A, reference->a, cpu.S, reference->sp);
    }
    if (flow_ok && case_index < 128u) {
        for (unsigned variant = 0; variant < 4u; ++variant) {
            Lufia2CpuState altered = *input;
            CpuState before;

            if (variant == 0u) {
                if (target->entry == 0x81ed8eu || target->entry == 0x81ee94u) {
                    altered.accumulator_is_8_bit = 0u;
                    altered.index_is_8_bit = 1u;
                    altered.x &= 0xffu;
                    altered.y &= 0xffu;
                } else {
                    if (target->bridge_width != 2u && target->bridge_width != 4u && target->bridge_width != 5u)
                        continue;
                    altered.accumulator_is_8_bit =
                        target->bridge_width == 2u ? 1u : 0u;
                    if (target->entry == 0x81f194u)
                        altered.data_bank = 0x80u;
                }
            } else if (variant == 1u) {
                if (target->bridge_width == 1u || target->bridge_width == 5u) continue;
                altered.index_is_8_bit = 1u;
                altered.x &= 0xffu;
                altered.y &= 0xffu;
            } else if (variant == 2u) {
                altered.decimal = 1u;
            } else {
                altered.accumulator_is_8_bit = 1u;
                altered.index_is_8_bit = 1u;
                altered.x &= 0xffu;
                altered.y &= 0xffu;
            }
            memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
            RestoreBusRegisters(bus, initial_registers);
            SnesVerifyBusResetMmio(bus);
            memset(&g_small_bridge, 0, sizeof(g_small_bridge));
            SmallBridgeLoadCpu(&cpu, bus, &altered, target->frame);
            if (variant == 3u) cpu.emulation = 1u;
            before = cpu;
            ret = target->bridge(&cpu);
            cpu.open_bus = before.open_bus;
            if (ret != (RecompReturn)111 ||
                g_small_bridge.calls != 1u ||
                g_small_bridge.flow != SMALL_BRIDGE_TAIL ||
                g_small_bridge.target != target->entry ||
                g_small_bridge.site != target->entry ||
                g_small_bridge.entry_s != altered.stack ||
                g_small_bridge.hrv != target->frame ||
                memcmp(&cpu, &before, sizeof(cpu)) != 0 ||
                memcmp(bus->wram, initial, SNES_VERIFY_WRAM_SIZE) != 0 ||
                bus->mmio_count != 0u ||
                !SameBusRegisters(bus, initial_registers)) {
                fprintf(stderr, "FAIL %s unsupported bridge case %u/%u\n",
                    target->name, case_index, variant);
                flow_ok = false;
                break;
            }
        }
    }
    return flow_ok;
}

static void ObserveCopyBoundaryWrites(
    void *context, uint32_t address, uint8_t value, bool write) {
    size_t *count = context;
    (void)address;
    (void)value;
    if (write)
        ++*count;
}

/* Whole small routine from its real call site. */
static bool RunSmallCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    const SmallTarget *target,
    SmallStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x83u, 0x80u, 0x7eu, 0x00u};
    const uint32_t exit = (target->call_return + 1u) & 0x00ffffffu;
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    static SnesVerifyBusEvent native_mmio[SNES_VERIFY_MAX_MMIO];
    size_t native_count;
    uint8_t *native_wram;
    BusRegisters registers;
    uint32_t last_instruction_pc = 0;
    unsigned instructions = 0;
    unsigned visits = 0;
    bool text_row_seen = false;
    bool stopped = false;
    bool same_mmio;
    TextWindowBusObservation native_observation = {0}, reference_observation = {0};
    size_t native_copy_writes = 0, reference_copy_writes = 0;
    const bool observe_copy = target->entry == 0x80cbaeu;
    const bool observe_window = target->entry == 0x80c305u || target->entry == 0x80c23du ||
        target->entry == 0x82fb1fu || target->entry == 0x80a1ccu ||
        target->entry == 0x80a1e4u || target->entry == 0x80a074u;

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = NmiRandom();
        memcpy(bus->wram + i, &word, 4);
    }
    memset(&input, 0, sizeof(input));
    input.direct_page = dps[NmiRandom() & 7u];
    /* C305's TDC supplies height's high byte. High-page DP can make its
     * tilemap overwrite the active return frame; use nonoverlapping states. */
    if (target->entry == 0x80c305u || target->entry == 0x80c23du ||
        target->entry == 0x809cb8u)
        input.direct_page &= 0x00ffu;
    target->seed(bus->wram, input.direct_page);
    bus->wram[0x1ff1u] = (uint8_t)target->call_return;
    bus->wram[0x1ff2u] = (uint8_t)(target->call_return >> 8);
    bus->wram[0x1ff3u] = (uint8_t)(target->call_return >> 16);
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = (uint8_t)(target->entry >> 16);
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = (NmiRandom() & 15u) ? 1u : 0u;
    input.index_is_8_bit = NmiRandom() & 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    registers = SaveBusRegisters(bus);
    if (target->registers)
        target->registers(&input);
    native = input;
    if (observe_window) {
        native_observation.join_read = 0x059e8du + input.x;
        reference_observation.join_read = native_observation.join_read;
        bus->event_observer = ObserveTextWindowBus;
        bus->event_observer_context = &native_observation;
    }
    if (observe_copy) {
        bus->event_observer = ObserveCopyBoundaryWrites;
        bus->event_observer_context = &native_copy_writes;
    }
    SnesVerifyBusResetMmio(bus);
    result = target->run(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        native.stack = (uint16_t)(native.stack + target->frame);
        native.program_bank = (uint8_t)(exit >> 16);
    }
    native_count = bus->mmio_count;
    memcpy(native_mmio, bus->mmio, sizeof(SnesVerifyBusEvent) * native_count);
    SnesVerifyBusResetMmio(bus);
    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    RestoreBusRegisters(bus, &registers);

    if (observe_copy)
        bus->event_observer_context = &reference_copy_writes;
    const bool copy_loop_boundary = observe_copy &&
        result.flow != LUFIA2_EXECUTION_RETURNED &&
        (result.pc == 0x838c0cu || result.pc == 0x838c29u || result.pc == 0x838c46u);

    if (observe_window) bus->event_observer_context = &reference_observation;

    InitInterp(reference, target->entry, &input);
    /* A looping event tick runs 4096 opcodes. */
    while (instructions < 10000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == exit
                : pc == result.pc && reference->sp == native.stack &&
                  (!copy_loop_boundary || reference_copy_writes == native_copy_writes) &&
                  visits++ == result.dispatches) {
            stopped = true;
            break;
        }
        if (pc == 0x80c56eu)
            text_row_seen = true;
        if (pc == 0x80c431u) {
            ++stats->window_join[reference->mf != 0];
            ++stats->text_windows[reference->mf ? 1 : 3];
        }
        if (pc == 0x80c4beu && reference->x <= 18u)
            ++stats->window_tail[reference->x / 6u];
        if (pc == 0x80bf89u) ++stats->window_lookup[0];
        if (pc == 0x80bf91u) ++stats->window_lookup[1];
        if (target->entry == 0x80a074u || target->entry == 0x809cb8u) {
            static const uint32_t family[15] = {0x80a0aau,0x80a0bcu,0x80a0e1u,0x80a0edu,
                0x80a1f6u,0x80a126u,0x80a163u,0x80a171u,0x80a17fu,0x80a18au,
                0x80a1afu,0x80a146u,0x80a12cu,0x80a269u,0x80a277u};
            for (unsigned k = 0; k < 15u; ++k) if (pc == family[k]) ++stats->condition_family[k];
            if (pc == 0x80a216u) ++stats->condition_compare[reference->a & 7u];
            if (pc == 0x80a25du) ++stats->condition_result[1];
            if (pc == 0x80a264u && !reference->mf) ++stats->condition_result[0];
            if (pc == 0x80a269u || pc == 0x80a277u) {
                const unsigned branch = pc == 0x80a277u;
                const bool previous = SnesVerifyBusRead(bus, ((uint32_t)reference->db << 16) | 0x1267u) != 0;
                ++stats->condition_goto[branch][branch ? !previous : previous];
            }
            if (pc == 0x80a1abu && (bus->wram[(uint16_t)(reference->dp + 0x22u)] & 0xf8u) == 0xe0u)
                ++stats->condition_search[SnesVerifyBusRead(bus, ((uint32_t)reference->db << 16) | 0x1267u) != 0];
        }
        last_instruction_pc = pc;
        interp816_runOpcode(reference);
        ++instructions;
    }
    same_mmio = native_count == bus->mmio_count && !bus->mmio_overflow &&
        memcmp(native_mmio, bus->mmio,
            sizeof(SnesVerifyBusEvent) * native_count) == 0;
    bus->event_observer = NULL;
    bus->event_observer_context = NULL;
    const bool same_window_bus = !observe_window ||
        (!native_observation.overflow && !reference_observation.overflow &&
         native_observation.count == reference_observation.count &&
         memcmp(native_observation.events, reference_observation.events,
             sizeof(SnesVerifyBusEvent) * native_observation.count) == 0);
    /* These standalone contracts expose the exact RTS instruction too. */
    const bool wrong_return =
        (target->entry == 0x81f0a2u || target->entry == 0x81e835u ||
         target->entry == 0x80c652u || target->entry == 0x80c784u ||
         target->entry == 0x80c56eu || target->entry == 0x80c305u ||
         target->entry == 0x80c23du || target->entry == 0x82fb1fu ||
         target->entry == 0x80a1ccu || target->entry == 0x80a1e4u) &&
        result.flow == LUFIA2_EXECUTION_RETURNED && result.pc != last_instruction_pc;
    if (!stopped || wrong_return || !same_mmio || !same_window_bus || !SameState(&native, reference) ||
        memcmp(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE) != 0) {
        size_t diff = 0;
        while (diff < SNES_VERIFY_WRAM_SIZE &&
               native_wram[diff] == bus->wram[diff])
            ++diff;
        fprintf(stderr,
            "FAIL %s case %u: flow=%u pc=%06X ref-return=%06X stop=%d "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u wram@%05X\n",
            target->name, case_index, (unsigned)result.flow, result.pc, last_instruction_pc,
            stopped ? 1 : 0,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.accumulator_is_8_bit, reference->mf,
            native.index_is_8_bit, reference->xf,
            native.carry, reference->c, native.zero, reference->z,
            native.negative, reference->n, (unsigned)diff);
        free(native_wram);
        return false;
    }
    if (!VerifySmallBridge(bus, reference, initial, &registers, &input,
            target, &result, native_wram, native_mmio, native_count,
            case_index)) {
        free(native_wram);
        return false;
    }
    free(native_wram);
    if (target->entry == 0x80a1ccu) ++stats->condition_record_bus[native_count != 0];
    if (target->entry == 0x80a074u) ++stats->condition_join[result.pc == 0x80a3c6u];
    if (target->entry == 0x82fb1fu) {
        const uint32_t exits[3] = {0x82fb2eu,0x82fb47u,0x82fb50u};
        for (unsigned k = 0; k < 3u; ++k) if (result.pc == exits[k]) ++stats->possession_exit[k];
    }
    if (target->entry == 0x809cb8u || target->entry == 0x80c23du ||
        target->entry == 0x80c305u) {
        if (result.pc == 0x80c2a1u) ++stats->text_windows[0];
        if (text_row_seen) ++stats->text_windows[2];
    }
    if (target->entry == 0x81f0a2u || target->entry == 0x81e835u) {
        const uint32_t exits[2][3] = {
            {0x81f0ccu, 0x81f0d8u, 0x81f113u},
            {0x81e847u, 0x81e86eu, 0x81e871u}};
        const unsigned row = target->entry == 0x81e835u;
        for (unsigned i = 0; i < 3u; ++i)
            if (result.pc == exits[row][i])
                ++stats->s19_returns[i];
    }
    if (target->entry == 0x81f979u && result.flow == LUFIA2_EXECUTION_RETURNED) {
        const unsigned outcome = native.accumulator & 0xffu;
        const uint16_t member = initial[0x09fau];
        const uint16_t record = Rom16(bus, 0x859ebau + 2u * member);

        if (outcome < 3u)
            ++stats->level_up_results[outcome];
        if (outcome == 0u && initial[record + 0x0eu] == 99u)
            ++stats->level_99_capped;
    }
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    return true;
}

enum { BATTLE_FRAME_CASES = 16384 };

typedef struct BattleFrameStats {
    unsigned returned;
    unsigned boundary;
    unsigned tilemap;
    unsigned grid;
    unsigned party;
    unsigned mmio_writes;
} BattleFrameStats;

/* Battle sprite records, small party blocks, sane buffers. */
static void SeedBattleFrame(uint8_t *wram, uint32_t (*random)(void)) {
    static const uint8_t layouts[8] = {1, 1, 1, 2, 2, 2, 0, 3};

    for (size_t i = 0; i < SNES_VERIFY_WRAM_SIZE; i += 4) {
        const uint32_t word = random();
        memcpy(wram + i, &word, 4);
    }
    wram[0x15abu] = (random() & 15u) ? layouts[random() & 7u]
                                     : (uint8_t)random();
    if (random() & 1u)
        wram[0x11deu] = 0;
    if (random() & 1u)
        wram[0x125fu] = 0;
    wram[0x153cu] = (random() & 15u) ? (uint8_t)(random() % 9u) : 0;
    wram[0x154eu] = (random() & 15u) ? (uint8_t)(random() % 7u) : 0;
    for (unsigned i = 0; i < 3u; ++i) {
        const uint16_t buffer = (uint16_t)(0x4800u + (random() & 0x03ffu));
        const uint16_t at = i == 0 ? 0x15c8u : i == 1 ? 0x15ccu : 0x15d4u;

        wram[at] = (uint8_t)buffer;
        wram[at + 1u] = (uint8_t)(buffer >> 8);
    }
    for (unsigned i = 0; i < 11u; ++i) {
        const uint16_t entity = (random() & 3u)
            ? (uint16_t)(0x0800u + (random() & 0x0fffu)) : 0;

        wram[0x0a64u + 2u * i] = (uint8_t)entity;
        wram[0x0a65u + 2u * i] = (uint8_t)(entity >> 8);
    }
    for (unsigned i = 0; i < 6u; ++i) {
        wram[0x13e7u + 15u * i] = (uint8_t)(1u + (random() & 3u));
        wram[0x13e8u + 15u * i] = (uint8_t)(1u + (random() & 3u));
    }
}

/* Whole $85:8A2F or $85:ECF0 from a JSL; MMIO order compared. */
static bool RunBattleFrameCase(
    SnesVerifyBus *bus,
    Interp816 *reference,
    uint8_t *initial,
    unsigned case_index,
    uint8_t upkeep,
    BattleFrameStats *stats) {
    static const uint16_t dps[8] = {0, 0, 0, 0, 0, 0, 0x0020u, 0x0400u};
    static const uint8_t banks[4] = {0x81u, 0x85u, 0x00u, 0x7eu};
    const uint32_t entry = upkeep ? 0x85ecf0u : 0x858a2fu;
    const uint32_t exit = upkeep ? 0x81887bu : 0x85ecf8u;
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
    SnesVerifyBusEvent *native_mmio;
    size_t native_count;
    uint8_t *native_wram;
    BusRegisters registers;
    unsigned instructions = 0;
    bool stopped = false;
    bool same_mmio;

    SeedBattleFrame(bus->wram, NmiRandom);
    /* JSL at $81:8877 or $85:ECF4. */
    bus->wram[0x1ff1u] = (uint8_t)(exit - 1u);
    bus->wram[0x1ff2u] = (uint8_t)((exit - 1u) >> 8);
    bus->wram[0x1ff3u] = (uint8_t)(exit >> 16);

    memset(&input, 0, sizeof(input));
    input.accumulator = (uint16_t)NmiRandom();
    input.x = (uint16_t)NmiRandom();
    input.y = (uint16_t)NmiRandom();
    input.stack = 0x1ff0u;
    input.direct_page = dps[NmiRandom() & 7u];
    input.data_bank = banks[NmiRandom() & 3u];
    input.program_bank = (uint8_t)(entry >> 16);
    input.carry = NmiRandom() & 1u;
    input.zero = NmiRandom() & 1u;
    input.negative = NmiRandom() & 1u;
    input.overflow = NmiRandom() & 1u;
    input.irq_disable = NmiRandom() & 1u;
    input.accumulator_is_8_bit = (NmiRandom() & 15u) ? 1u : 0u;
    input.index_is_8_bit = (NmiRandom() & 15u) ? 0u : 1u;
    if (input.index_is_8_bit) {
        input.x &= 0x00ffu;
        input.y &= 0x00ffu;
    }
    if (bus->wram[0x15abu] == 1u) {
        if (!bus->wram[0x11deu])
            ++stats->tilemap;
        else if (!bus->wram[0x125fu])
            ++stats->grid;
    } else if (bus->wram[0x15abu] == 2u) {
        ++stats->party;
    }

    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    registers = SaveBusRegisters(bus);
    native = input;
    SnesVerifyBusResetMmio(bus);
    result = upkeep ? Lufia2BattleFrameUpkeep(&memory, &native)
                    : Lufia2BattleSprites(&memory, &native);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        native.stack = (uint16_t)(native.stack + 3u);   /* RTL */
        native.program_bank = (uint8_t)(exit >> 16);
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
    InitInterp(reference, entry, &input);
    while (instructions < 400000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED
                ? pc == exit
                : pc == result.pc && reference->sp == native.stack) {
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
            "FAIL %06X case %u: pc=%06X stop=%d mmio=%u/%u same=%d insns=%u "
            "A=%04X/%04X X=%04X/%04X Y=%04X/%04X S=%04X/%04X DB=%02X/%02X "
            "M=%u/%u Xf=%u/%u C=%u/%u Z=%u/%u N=%u/%u V=%u/%u "
            "wram@%05X %02X/%02X\n",
            entry, case_index, result.pc, stopped ? 1 : 0,
            (unsigned)native_count, (unsigned)bus->mmio_count,
            same_mmio ? 1 : 0, instructions,
            native.accumulator, reference->a, native.x, reference->x,
            native.y, reference->y, native.stack, reference->sp,
            native.data_bank, reference->db,
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED)
        ++stats->returned;
    else
        ++stats->boundary;
    stats->mmio_writes += (unsigned)native_count;
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
    Lufia2CpuState input;
    Lufia2CpuState native;
    NativeMemory native_context = {bus};
    Lufia2Memory memory = {
        NativeRead, NativeWrite, &native_context};
    Lufia2ExecutionResult result;
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

    const uint32_t initial_wm_address = bus->wm_address;
    memcpy(initial, bus->wram, SNES_VERIFY_WRAM_SIZE);
    native = input;
    result = Lufia2ActorSecondaryUpdate(&memory, &native);

    native_wram = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    if (!native_wram)
        return false;
    const uint32_t native_wm_address = bus->wm_address;
    memcpy(native_wram, bus->wram, SNES_VERIFY_WRAM_SIZE);
    memcpy(bus->wram, initial, SNES_VERIFY_WRAM_SIZE);
    bus->wm_address = initial_wm_address;

    InitInterp(reference, 0x83d508u, &input);
    while (instructions < 4000000u) {
        const uint32_t pc = SnesVerifyPc24(reference);
        if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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

    if (!stopped || bus->wm_address != native_wm_address || !SameState(&native, reference) ||
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
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
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

static bool RunMovementGroup(SnesVerifyBus *bus, Interp816 *reference, uint8_t *initial,
                             unsigned cases) {
    unsigned total = 0;
    unsigned count = cases < MOVEMENT_HELPER_CASES ? cases : MOVEMENT_HELPER_CASES;
    for (unsigned i = 0; i < count; ++i) {
        if (!RunMovementStepCase(bus, reference, initial, i))
            return false;
        ++total;
    }
    count = cases < RESUME_EXACT_CASES ? cases : RESUME_EXACT_CASES;
    for (unsigned i = 0; i < count; ++i) {
        if (!RunResumeExactCase(bus, reference, initial, i, true))
            return false;
        ++total;
    }
    PrimaryUpdateStats secondary = {0}, primary = {0};
    PlayerStandardStats player = {0};
    for (unsigned i = 0; i < cases; ++i) {
        if (!RunSecondaryUpdateCase(bus, reference, initial, i, &secondary))
            return false;
        ++total;
    }
    for (unsigned i = 0; i < cases; ++i) {
        if (!RunPrimaryUpdateCase(bus, reference, initial, i, &primary))
            return false;
        ++total;
    }
    for (unsigned i = 0; i < cases; ++i) {
        if (!RunPlayerStandardCase(bus, reference, initial, i, &player))
            return false;
        ++total;
    }
    count = cases < GENERIC_HANDLER_CASES ? cases : GENERIC_HANDLER_CASES;
    for (unsigned h = 0; h < GENERIC_HANDLER_COUNT; ++h) {
        GenericHandlerStats stats = {0};
        for (unsigned i = 0; i < count; ++i) {
            if (!RunGenericHandlerCase(bus, reference, initial, kGenericHandlers[h].pc,
                                       i, kGenericHandlers[h].name,
                                       kGenericHandlers[h].low_script, &stats))
                return false;
            ++total;
        }
    }
    printf("PASS%u affected movement/VM/player states\n", total);
    return true;
}

int main(int argc, char **argv) {
    bool movement_only = false;
    unsigned movement_cases = 4096u;
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
    FieldScrollStats field_scroll = {0};
    unsigned field_idle_passed = 0;
    FieldChildStats field_idle = {0, 0};
    unsigned field_ticks_passed = 0;
    FieldChildStats field_ticks = {0, 0};
    unsigned field_colour_passed = 0;
    unsigned field_tick_passed = 0;
    unsigned battle_nmi_passed = 0;
    BattleFrameStats battle_sprites = {0, 0, 0, 0, 0, 0};
    BattleFrameStats battle_upkeep = {0, 0, 0, 0, 0, 0};
    unsigned battle_sprites_passed = 0;
    SmallStats small_stats[SMALL_TARGETS];
    unsigned small_passed[SMALL_TARGETS] = {0};
    WorldMapEdgeStats world_edges = {0, 0, 0, 0};
    unsigned world_edges_passed = 0;
    unsigned vram_slot_passed = 0;
    unsigned vram_slot_full = 0;
    WorldMapNmiStats world_nmi = {0, 0, 0, 0};
    unsigned world_nmi_passed = 0;
    FieldSpritesStats field_sprites = {0, 0, 0};
    unsigned field_sprites_passed = 0;
    unsigned battle_upkeep_passed = 0;
    BattleNmiStats battle_nmi = {0, 0, 0};
    FieldChildStats field_tick = {0, 0};
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
    uint32_t only_small = 0;
    unsigned only_small_cases = SMALL_CASES;
    unsigned only_small_start = 0;
    unsigned only_small_index = SMALL_TARGETS;
    bool skip_small = false;
    bool bus_initialized = false;

    memset(generic_stats, 0, sizeof(generic_stats));
    memset(&primary_update_stats, 0, sizeof(primary_update_stats));
    memset(&secondary_update_stats, 0, sizeof(secondary_update_stats));

    if (argc == 2 && strcmp(argv[1], "--list-small") == 0) {
        for (unsigned t = 0; t < SMALL_TARGETS; ++t)
            printf("%02X:%04X\n", (unsigned)(kSmallTargets[t].entry >> 16),
                (unsigned)(kSmallTargets[t].entry & 0xffffu));
        return 0;
    }
    if (argc != 2 && argc != 3 && argc != 4 && argc != 5 && argc != 6) {
        fprintf(
            stderr,
            "usage: %s <lufia2.sfc> [--movement [cases] | --skip-small | --only-small BB:AAAA [cases [start]]]\n",
            argv[0]);
        return 2;
    }
    if (argc == 3) {
        movement_only = strcmp(argv[2], "--movement") == 0;
        if (!movement_only && strcmp(argv[2], "--skip-small") != 0)
            return 2;
        skip_small = true;
    }
    if (argc == 4 && strcmp(argv[2], "--movement") == 0) {
        char *end;
        movement_cases = (unsigned)strtoul(argv[3], &end, 10);
        if (*end || !movement_cases || movement_cases > PRIMARY_UPDATE_CASES)
            return 2;
        movement_only = true;
        skip_small = true;
    }
    if (argc >= 4 && !movement_only) {
        unsigned bank, pc;
        char trailing;
        if (strcmp(argv[2], "--only-small") != 0 ||
            sscanf(argv[3], "%x:%x%c", &bank, &pc, &trailing) != 2 ||
            bank > 0xffu || pc > 0xffffu) return 2;
        only_small = (bank << 16) | pc;
        if (argc >= 5) {
            char *end;
            only_small_cases = (unsigned)strtoul(argv[4], &end, 0);
            if (*end || !only_small_cases || only_small_cases > SMALL_CASES)
                return 2;
        }
        if (argc == 6) {
            char *end;
            only_small_start = (unsigned)strtoul(argv[5], &end, 0);
            if (*end || only_small_start > SMALL_CASES - only_small_cases ||
                only_small != 0x82e49eu) return 2;
        }
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
    g_seed_rom = &bus;
    initial = (uint8_t *)malloc(SNES_VERIFY_WRAM_SIZE);
    reference =
        interp816_init(&bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!initial || !reference) {
        fprintf(stderr, "ERROR: verifier allocation failed\n");
        failed = 1;
        goto done;
    }
    if (movement_only) {
        failed = RunMovementGroup(&bus, reference, initial, movement_cases) ? 0 : 1;
        goto done;
    }

    if (only_small) {
        const SmallTarget *target = NULL;
        SmallStats stats = {0};
        unsigned passed = 0;

        for (unsigned t = 0; t < SMALL_TARGETS; ++t)
            if (kSmallTargets[t].entry == only_small)
                target = &kSmallTargets[only_small_index = t];
        if (!target) {
            fprintf(stderr, "unknown small target %06X\n", only_small);
            failed = 1;
            goto done;
        }
        g_nmi_rng = UINT64_C(0x3c6ef372fe94f82a) ^
            ((uint64_t)target->entry << 16);
        for (unsigned i = only_small_start;
             i < only_small_start + only_small_cases && failed < 20u; ++i) {
            if (target->entry == 0x82e49eu)
                g_nmi_rng = UINT64_C(0x3c6ef372fe94f82a) ^
                    ((uint64_t)target->entry << 16) ^
                    ((uint64_t)(i + 1u) * UINT64_C(0x9e3779b97f4a7c15));
            if (RunSmallCase(&bus, reference, initial, i, target, &stats))
                ++passed;
            else
                ++failed;
        }
        small_stats[only_small_index] = stats;
        small_passed[only_small_index] = passed;
        goto small_report;
    }

    fprintf(stderr, "actor dispatch phase: opcode handlers\n");
    fflush(stderr);

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


    fprintf(stderr, "actor dispatch phase: action and actor cores\n");
    fflush(stderr);
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
    fprintf(stderr, "actor dispatch phase: field and NMI whole functions\n");
    fflush(stderr);
    fprintf(stderr, "actor dispatch target: field NMI\n"); fflush(stderr);
    for (unsigned i = 0; i < FIELD_NMI_CASES && failed < 20; ++i) {
        if (RunFieldNmiCase(&bus, reference, initial, i, &field_nmi))
            ++field_nmi_passed;
        else
            ++failed;
    }
    memset(&field_scroll, 0, sizeof(field_scroll));
    fprintf(stderr, "actor dispatch target: field scroll\n"); fflush(stderr);
    for (unsigned i = 0; i < FIELD_SCROLL_CASES && failed < 20; ++i) {
        if (RunFieldScrollCase(&bus, reference, initial, i, &field_scroll))
            ++field_scroll_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch target: field idle/animation\n"); fflush(stderr);
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
    fprintf(stderr, "actor dispatch target: field colour\n"); fflush(stderr);
    for (unsigned i = 0; i < FIELD_COLOUR_CASES && failed < 20; ++i) {
        if (RunFieldColourCase(&bus, reference, initial, i, &field_colour))
            ++field_colour_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch target: field tick\n"); fflush(stderr);
    for (unsigned i = 0; i < FIELD_CHILD_CASES && failed < 20; ++i) {
        if (RunFieldTickCase(&bus, reference, initial, i, &field_tick))
            ++field_tick_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch target: battle NMI\n"); fflush(stderr);
    for (unsigned i = 0; i < BATTLE_NMI_CASES && failed < 20; ++i) {
        if (RunBattleNmiCase(&bus, reference, initial, i, &battle_nmi))
            ++battle_nmi_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch target: world map NMI\n"); fflush(stderr);
    for (unsigned i = 0; i < WORLD_MAP_NMI_CASES && failed < 20; ++i) {
        if (RunWorldMapNmiCase(&bus, reference, initial, i, &world_nmi))
            ++world_nmi_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch target: VRAM slot\n"); fflush(stderr);
    for (unsigned i = 0; i < VRAM_SLOT_CASES && failed < 20; ++i) {
        if (RunVramSlotCase(&bus, reference, initial, i, &vram_slot_full))
            ++vram_slot_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch target: world map edge\n"); fflush(stderr);
    for (unsigned i = 0; i < WORLD_MAP_EDGE_CASES && failed < 20; ++i) {
        if (RunWorldMapEdgeCase(&bus, reference, initial, i, &world_edges))
            ++world_edges_passed;
        else
            ++failed;
    }
    fprintf(stderr, "actor dispatch phase: small and frame targets\n");
    fflush(stderr);
    memset(small_stats, 0, sizeof(small_stats));
    for (unsigned t = 0; t < SMALL_TARGETS && !skip_small; ++t) {
        g_nmi_rng = UINT64_C(0x3c6ef372fe94f82a) ^
            ((uint64_t)kSmallTargets[t].entry << 16);
        for (unsigned i = 0; i < SMALL_CASES && failed < 20; ++i) {
            if (kSmallTargets[t].entry == 0x82e49eu)
                g_nmi_rng = UINT64_C(0x3c6ef372fe94f82a) ^
                    ((uint64_t)kSmallTargets[t].entry << 16) ^
                    ((uint64_t)(i + 1u) * UINT64_C(0x9e3779b97f4a7c15));
            if (RunSmallCase(&bus, reference, initial, i,
                    &kSmallTargets[t], &small_stats[t]))
                ++small_passed[t];
            else
                ++failed;
        }
    }
    for (unsigned i = 0; i < FIELD_SPRITES_CASES && failed < 20; ++i) {
        if (RunFieldSpritesCase(&bus, reference, initial, i, &field_sprites))
            ++field_sprites_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < BATTLE_FRAME_CASES && failed < 20; ++i) {
        if (RunBattleFrameCase(
                &bus, reference, initial, i, 0, &battle_sprites))
            ++battle_sprites_passed;
        else
            ++failed;
    }
    for (unsigned i = 0; i < BATTLE_FRAME_CASES && failed < 20; ++i) {
        if (RunBattleFrameCase(
                &bus, reference, initial, i, 1, &battle_upkeep))
            ++battle_upkeep_passed;
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
    fprintf(stderr, "actor dispatch phase: value helpers and final targets\n");
    fflush(stderr);
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
        "(RTS 829F %u; LLE 80:CC3F %u, 80:CC0E %u, B96D %u, 8225 %u, "
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
    printf("$80:9C72 whole-function cases passed:      %u / %u "
        "(RTL %u, LLE %u)\n",
        field_tick_passed, FIELD_CHILD_CASES, field_tick.returned,
        field_tick.boundary);
    printf("$86:CEF6 whole-function cases passed:      %u / %u "
        "(tile uploads %u, palette cycles %u, MMIO writes compared %u)\n",
        world_nmi_passed, WORLD_MAP_NMI_CASES, world_nmi.tiles,
        world_nmi.cycles, world_nmi.mmio_writes);
    printf("$85:ECDB whole-function cases passed:      %u / %u "
        "(queue full or X=1, LLE %u)\n",
        vram_slot_passed, VRAM_SLOT_CASES, vram_slot_full);
    printf("$86:99BF whole-function cases passed:      %u / %u "
        "(RTS %u, LLE %u, columns %u, rows %u)\n",
        world_edges_passed, WORLD_MAP_EDGE_CASES, world_edges.returned,
        world_edges.boundary, world_edges.columns, world_edges.rows);
small_report:
    for (unsigned t = 0; t < SMALL_TARGETS; ++t) {
        if (skip_small || (only_small && t != only_small_index)) continue;
        printf("$%02X:%s whole-function cases passed:      %u / %u "
            "(return %u, LLE %u)\n",
            (unsigned)(kSmallTargets[t].entry >> 16), kSmallTargets[t].name,
            small_passed[t], only_small ? only_small_cases : SMALL_CASES,
            small_stats[t].returned,
            small_stats[t].boundary);
        if (kSmallTargets[t].entry == 0x809cb8u) {
            const unsigned *paths = small_stats[t].text_windows;
            printf("$80:9CB8 T12 coverage: wait %u, explicit %u, C56E %u, actor %u\n",
                paths[0], paths[1], paths[2], paths[3]);
            /* Engine entry clears 125D; standalone C305 covers M1 join. */
            if (!paths[0] || !paths[2] || !paths[3])
                ++failed;
        }
        if (kSmallTargets[t].entry == 0x82fb1fu) {
            const unsigned *paths = small_stats[t].possession_exit;
            printf("$82:FB1F return coverage: possession %u, absent %u, packed %u\n",paths[0],paths[1],paths[2]);
            for (unsigned i = 0; i < 3u; ++i) if (!paths[i]) ++failed;
        }
        if (kSmallTargets[t].entry == 0x80a1ccu) {
            const unsigned *paths = small_stats[t].condition_record_bus;
            printf("$80:A1CC bus coverage: WRAM %u, CPU MMIO %u\n",paths[0],paths[1]);
            if (!paths[0] || !paths[1]) ++failed;
        }
        if (kSmallTargets[t].entry == 0x80a074u || kSmallTargets[t].entry == 0x809cb8u) {
            const SmallStats *s = &small_stats[t];
            printf("$80:%s T13 families:", kSmallTargets[t].name);
            for (unsigned i = 0; i < 15u; ++i) {
                printf(" %u",s->condition_family[i]);
                if (!s->condition_family[i]) ++failed;
            }
            printf("; comparisons:");
            for (unsigned i = 0; i < 8u; ++i) {
                printf(" %u",s->condition_compare[i]);
                if (!s->condition_compare[i]) ++failed;
            }
            printf("; false/true %u/%u; joins %u/%u\n",s->condition_result[0],s->condition_result[1],s->condition_join[0],s->condition_join[1]);
            if (!s->condition_result[0] || !s->condition_result[1]) ++failed;
            printf("$80:%s search miss/hit %u/%u; goto true skip/take %u/%u; false skip/take %u/%u\n",
                kSmallTargets[t].name,s->condition_search[0],s->condition_search[1],
                s->condition_goto[0][0],s->condition_goto[0][1],s->condition_goto[1][0],s->condition_goto[1][1]);
            if (!s->condition_search[0] || !s->condition_search[1]) ++failed;
            for (unsigned i = 0; i < 2u; ++i)
                for (unsigned j = 0; j < 2u; ++j) if (!s->condition_goto[i][j]) ++failed;
            if (kSmallTargets[t].entry == 0x80a074u && (!s->condition_join[0] || !s->condition_join[1])) ++failed;
        }
        if (kSmallTargets[t].entry == 0x80c305u) {
            const SmallStats *s = &small_stats[t];
            printf("$80:C305 coverage: join M0 %u M1 %u; tail %u/%u/%u/%u; lookup miss %u hit %u\n",
                s->window_join[0], s->window_join[1], s->window_tail[0],
                s->window_tail[1], s->window_tail[2], s->window_tail[3],
                s->window_lookup[0], s->window_lookup[1]);
            if (!s->window_join[0] || !s->window_join[1] ||
                !s->window_lookup[0] || !s->window_lookup[1]) ++failed;
            for (unsigned i = 0; i < 4u; ++i)
                if (!s->window_tail[i]) ++failed;
        }
        if (kSmallTargets[t].entry == 0x81f0a2u || kSmallTargets[t].entry == 0x81e835u) {
            const unsigned *returns = small_stats[t].s19_returns;
            printf("$81:%s return coverage: %u, %u, %u\n",
                kSmallTargets[t].name, returns[0], returns[1], returns[2]);
            if (!returns[0] || !returns[1] || !returns[2])
                ++failed;
        }
        if (kSmallTargets[t].entry == 0x81f979u) {
            const SmallStats *stats = &small_stats[t];

            printf("$81:F979 branch coverage: no raise %u, raised %u, stale %u, level 99 %u\n",
                stats->level_up_results[0], stats->level_up_results[1],
                stats->level_up_results[2], stats->level_99_capped);
            if (!stats->level_up_results[0] || !stats->level_up_results[1] ||
                !stats->level_up_results[2] || !stats->level_99_capped)
                ++failed;
        }
    }
    if (only_small) {
        printf("RESULT: %s\n", failed ? "FAIL" : "PASS");
        goto done;
    }
    printf("$83:A21A whole-function cases passed:      %u / %u "
        "(RTL %u, LLE %u, sprites %u)\n",
        field_sprites_passed, FIELD_SPRITES_CASES, field_sprites.returned,
        field_sprites.boundary, field_sprites.visible);
    printf("$85:8A2F whole-function cases passed:      %u / %u "
        "(RTL %u, LLE %u, tilemap %u, grid %u, party %u, MMIO %u)\n",
        battle_sprites_passed, BATTLE_FRAME_CASES, battle_sprites.returned,
        battle_sprites.boundary, battle_sprites.tilemap, battle_sprites.grid,
        battle_sprites.party, battle_sprites.mmio_writes);
    printf("$85:ECF0 whole-function cases passed:      %u / %u "
        "(RTL %u, LLE %u, tilemap %u, grid %u, party %u, MMIO %u)\n",
        battle_upkeep_passed, BATTLE_FRAME_CASES, battle_upkeep.returned,
        battle_upkeep.boundary, battle_upkeep.tilemap, battle_upkeep.grid,
        battle_upkeep.party, battle_upkeep.mmio_writes);
    printf("$85:8DC5 whole-function cases passed:      %u / %u "
        "(RTL %u, LLE %u, MMIO writes compared %u)\n",
        battle_nmi_passed, BATTLE_NMI_CASES, battle_nmi.returned,
        battle_nmi.boundary, battle_nmi.mmio_writes);
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
    printf("$83:FB12 indirect-jump cases passed:        %u / %u\n", resume_passed[1],
           RESUME_EXACT_CASES);
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
