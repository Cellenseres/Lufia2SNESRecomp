#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/party.h"
#include "core/cpu_ops.h"
#include "system/wram.h"

typedef struct Trace {
    SnesVerifyBusEvent events[128];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static Trace *fetch_trace;
static uint32_t fetch_address;
static unsigned fetch_left, number;
static const uint16_t instruction_pcs[] = {0x9971u, 0x9972u, 0x9973u, 0x9975u,
                                           0x9977u, 0x997Au, 0x997Bu, 0x997Cu,
                                           0x997Fu, 0x9981u, 0x9982u};
static const uint8_t instruction_lengths[] = {1u, 1u, 2u, 2u, 3u, 1u,
                                              1u, 3u, 2u, 1u, 1u};

static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *trace = context;
    if (trace == fetch_trace && !write && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    if (trace->count == 128u)
        abort();
    trace->events[trace->count++] =
        (SnesVerifyBusEvent){address, value, write ? 1u : 0u};
}
static void load(Interp816 *c, const Lufia2CpuState *n) {
    c->a = n->accumulator;
    c->x = n->x;
    c->y = n->y;
    c->sp = n->stack;
    c->dp = n->direct_page;
    c->db = n->data_bank;
    c->k = n->program_bank;
    c->pc = 0x9971;
    c->c = n->carry;
    c->z = n->zero;
    c->n = n->negative;
    c->v = n->overflow;
    c->d = n->decimal;
    c->i = n->irq_disable;
    c->mf = n->accumulator_is_8_bit;
    c->xf = n->index_is_8_bit;
    c->e = c->irqWanted = c->nmiWanted = c->waiting = c->stopped = 0;
    interp816_set_brk_hook_enabled(c, false);
}
static int same(const Interp816 *c, const Lufia2CpuState *n) {
    return c->a == n->accumulator && c->x == n->x && c->y == n->y &&
           c->sp == n->stack && c->dp == n->direct_page && c->db == n->data_bank &&
           c->k == n->program_bank && c->c == n->carry && c->z == n->zero &&
           c->n == n->negative && c->v == n->overflow && c->d == n->decimal &&
           c->i == n->irq_disable && c->mf == n->accumulator_is_8_bit &&
           c->xf == n->index_is_8_bit;
}
static Lufia2ExecutionResult position(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2PartySelectActiveEquipmentMember(m, n);
}
static void seed(Lufia2CpuState *n) {
    for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
        native.wram[i] = (uint8_t)((i * 23u + number * 47u) ^ (i >> 7));
    memset(n, 0, sizeof *n);
    n->program_bank = 0x82u;
    n->data_bank = 0x7eu;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(number * 113u);
    n->stack = (uint16_t)(0x1f10u + number % 0xedu);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    n->accumulator_is_8_bit = (number >> 16) & 1u;
    n->index_is_8_bit = (number >> 17) & 1u;
    n->direct_page = (uint16_t)number;
    if (number >= 262144u) {
        unsigned value = number - 262144u;
        n->direct_page = 0u;
        n->accumulator_is_8_bit = (value >> 13) & 1u;
        n->index_is_8_bit = (value >> 14) & 1u;
        n->data_bank = (uint8_t)(value >> 5);
    }
    if (n->index_is_8_bit) {
        n->x &= 255u;
        n->y &= 255u;
    }
    const Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    const uint16_t slot = (uint16_t)(number * 37u);
    OpWrite16(&m,
              AbsoluteIndexedAddress(n, WRAM_MENU_PARTY_CHARACTER_OFFSET,
                                     (uint16_t)((slot & 255u) * 2u)),
              (uint16_t)(number * 59u ^ 0xa55au));
    Write16Direct(&m, n, 0x22u, slot);
    Write8(&m, (uint32_t)n->stack + 1u, 0x23u);
    Write8(&m, (uint32_t)n->stack + 2u, 0x81u);
    uint8_t *ram = original.wram;
    void *context = original.event_observer_context;
    original = native;
    original.wram = ram;
    original.event_observer_context = context;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(&original);
    SnesVerifyBusResetTrace(&native);
    SnesVerifyBusResetMmio(&original);
    SnesVerifyBusResetMmio(&native);
    original_trace.count = native_trace.count = 0u;
}
int g_interp_apu_driving;
static int VerifyAllWidths(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 4) || !SnesVerifyLoadFile(argv[1], &rom, &size) ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc == 4) {
        uint32_t pc = (uint32_t)strtoul(argv[2], NULL, 16);
        rom[((pc >> 16) & 127u) * 32768u + (pc & 32767u)] =
            (uint8_t)strtoul(argv[3], NULL, 16);
    }
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *c = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    const Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    const unsigned cases = 294912u;
    for (number = 0; number < cases; ++number) {
        Lufia2CpuState n;
        seed(&n);
        load(c, &n);
        Lufia2ExecutionResult result = position(&m, &n);
        for (unsigned i = 0; i < 11u; ++i) {
            if (SnesVerifyPc24(c) != (0x820000u | instruction_pcs[i])) {
                fprintf(stderr, "position flow case%u instruction%u PC%06X\n", number,
                        i, SnesVerifyPc24(c));
                return 1;
            }
            fetch_trace = &original_trace;
            fetch_address = SnesVerifyPc24(c);
            fetch_left = instruction_lengths[i];
            interp816_runOpcode(c);
            fetch_left = 0;
        }
        if (result.flow != LUFIA2_EXECUTION_RETURNED || result.pc != 0x829983u ||
            SnesVerifyPc24(c) != result.pc || !same(c, &n) ||
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) ||
            original_trace.count != native_trace.count) {
            fprintf(stderr,
                    "position mismatch case%u Xwidth%u CPU%d events%u/%u A%04X/%04X "
                    "X%04X/%04X S%04X/%04X\n",
                    number, n.index_is_8_bit, same(c, &n), original_trace.count,
                    native_trace.count, n.accumulator, c->a, n.x, c->x, n.stack, c->sp);
            return 1;
        }
        for (unsigned i = 0; i < original_trace.count; ++i) {
            const SnesVerifyBusEvent *a = &original_trace.events[i],
                                     *b = &native_trace.events[i];
            if (a->address != b->address || a->value != b->value ||
                a->write != b->write) {
                fprintf(stderr, "position bus case%u event%u %06X/%06X\n", number, i,
                        a->address, b->address);
                return 1;
            }
        }
    }
    printf("PASS selection width%u original-ROM "
           "states;allentrywidths/allDP/stackaliases/directdata "
           "banks;CPU/fullWRAM/ordered operand bus\n",
           cases);
    interp816_free(c);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}

static int VerifyStackAliases(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc != 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    original.event_observer = native.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer_context = &native_trace;
    Interp816 *c = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                 NULL};
    const uint16_t stacks[] = {0x1f10u, 0x1f80u, 0x1ffcu};
    unsigned cases = 0u;
    for (unsigned mode = 0u; mode < 4u; ++mode)
        for (unsigned sample = 0u; sample < 3u; ++sample)
            for (int delta = -4; delta <= 3; ++delta)
                for (number = 0u; number < 256u; ++number) {
                    Lufia2CpuState n;
                    seed(&n);
                    n.accumulator_is_8_bit = mode & 1u;
                    n.index_is_8_bit = (mode >> 1) & 1u;
                    n.stack = stacks[sample];
                    n.direct_page = (uint16_t)((int)n.stack + delta - 0x2a);
                    n.program_bank = 0x82u;
                    n.data_bank = 0x7eu;
                    n.decimal = 0u;
                    n.x = n.index_is_8_bit ? 0xcdu : 0xabcdu;
                    n.y = n.index_is_8_bit ? 0x34u : 0x1234u;
                    Write16Direct(&memory, &n, 0x22u, 3u);
                    OpWrite16(&memory, 0x7e0a86u,
                              (uint16_t)(number | ((number ^ 0xa5u) << 8)));
                    Write8(&memory, (uint32_t)n.stack + 1u, 0x23u);
                    Write8(&memory, (uint32_t)n.stack + 2u, 0x81u);
                    uint8_t *ram = original.wram;
                    void *context = original.event_observer_context;
                    original = native;
                    original.wram = ram;
                    original.event_observer_context = context;
                    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
                    SnesVerifyBusResetTrace(&original);
                    SnesVerifyBusResetTrace(&native);
                    SnesVerifyBusResetMmio(&original);
                    SnesVerifyBusResetMmio(&native);
                    original_trace.count = native_trace.count = 0u;
                    load(c, &n);
                    Lufia2ExecutionResult result = position(&memory, &n);
                    for (unsigned i = 0u; i < 11u; ++i) {
                        if (SnesVerifyPc24(c) != (0x820000u | instruction_pcs[i]))
                            return 1;
                        fetch_trace = &original_trace;
                        fetch_address = SnesVerifyPc24(c);
                        fetch_left = instruction_lengths[i];
                        interp816_runOpcode(c);
                        if (fetch_left)
                            return 1;
                    }
                    if (result.flow != LUFIA2_EXECUTION_RETURNED ||
                        result.pc != 0x829983u || !same(c, &n) ||
                        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) ||
                        original_trace.count != native_trace.count) {
                        fprintf(stderr,
                                "width alias mode%u stack%04X delta%d value%02X CPU%d "
                                "S%04X/%04X P%02X\n",
                                mode, stacks[sample], delta, number, same(c, &n),
                                n.stack, c->sp, PackStatus(&n));
                        return 1;
                    }
                    for (unsigned i = 0u; i < original_trace.count; ++i) {
                        SnesVerifyBusEvent *a = &original_trace.events[i],
                                           *b = &native_trace.events[i];
                        if (a->address != b->address || a->value != b->value ||
                            a->write != b->write) {
                            fprintf(stderr, "width alias operand bus%u\n", i);
                            return 1;
                        }
                    }
                    ++cases;
                }
    printf(
        "PASS width stack aliases%u;all4 entry widths/all256 saved-status bytes/three "
        "stacks/eight overlapping destinations;CPU/fullWRAM/alloperandbus\n",
        cases);
    interp816_free(c);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}

int main(int argc, char **argv) {
    return VerifyAllWidths(argc, argv) || VerifyStackAliases(argc, argv);
}
