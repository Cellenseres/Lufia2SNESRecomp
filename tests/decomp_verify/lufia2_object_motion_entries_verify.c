#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "field/event_script_internal.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[4000000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, edge_profile, parent_bank_profile;
static unsigned child_visits[7];
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {
    0x83e1adu, 0x83e1b9u, 0x83ee9du, 0x83eea9u, 0x83eed8u, 0x83ed63u, 0x83ee0eu,
    0x83e03eu, 0x83e03eu, 0x83e03eu, 0x83e03eu, 0x83e03eu, 0x83e03eu};
static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *trace = context;
#ifdef RUNTIME_ABI
    if (!write && trace == &native_trace && runtime_return_reads)
        return;
#endif
    if (!write && trace == fetch_trace && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    if (trace->count == sizeof(trace->events) / sizeof(trace->events[0])) {
        fprintf(stderr, "trace ceiling case%u side%s fetch%06X\n", number,
                trace == &original_trace ? "original" : "native", fetch_address);
        exit(1);
    }
    trace->events[trace->count++] =
        (SnesVerifyBusEvent){address, value, write ? 1u : 0u};
}
static void load(Interp816 *r, const Lufia2CpuState *n, uint32_t pc) {
    r->a = n->accumulator;
    r->x = n->x;
    r->y = n->y;
    r->sp = n->stack;
    r->dp = n->direct_page;
    r->db = n->data_bank;
    r->k = (uint8_t)(pc >> 16);
    r->pc = (uint16_t)pc;
    r->c = n->carry;
    r->z = n->zero;
    r->n = n->negative;
    r->v = n->overflow;
    r->d = n->decimal;
    r->i = n->irq_disable;
    r->mf = n->accumulator_is_8_bit;
    r->xf = n->index_is_8_bit;
    r->e = r->irqWanted = r->nmiWanted = r->waiting = r->stopped = 0;
    interp816_set_brk_hook_enabled(r, false);
}
static void store(Lufia2CpuState *n, const Interp816 *r) {
    n->accumulator = r->a;
    n->x = r->x;
    n->y = r->y;
    n->stack = r->sp;
    n->direct_page = r->dp;
    n->data_bank = r->db;
    n->program_bank = r->k;
    n->carry = r->c;
    n->zero = r->z;
    n->negative = r->n;
    n->overflow = r->v;
    n->decimal = r->d;
    n->irq_disable = r->i;
    n->accumulator_is_8_bit = r->mf;
    n->index_is_8_bit = r->xf;
}
static void reset_devices(SnesVerifyBus *b) {
    SnesVerifyBusResetTrace(b);
    SnesVerifyBusResetMmio(b);
    b->multiply_a = b->multiply_b = 0;
    b->multiply_result = 0;
    b->divide_a = b->divide_result = 0;
    b->wm_address = 0;
    b->m7_latch = 0;
    b->m7_a = 0;
    b->m7_b = 0;
}
static void word(uint32_t a, uint16_t v) {
    if (a >= 0x2000u && a < 0x7e0000u) {
        fprintf(stderr, "invalid seed word address%06X\n", a);
        exit(2);
    }
    SnesVerifyBusPoke(&native, a, (uint8_t)v);
    SnesVerifyBusPoke(&native, a + 1u, (uint8_t)(v >> 8));
}
static void instruction(SnesVerifyBus *b, Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    Trace *trace = b == &original ? &original_trace : &native_trace;
    if (b == &original)
        for (unsigned i = 0u; i < 7u; ++i)
            if (pc == entries[i])
                ++child_visits[i];
    uint8_t op = SnesVerifyBusRead(b, pc);
    --trace->count;
    fetch_trace = trace;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][op];
    if (!fetch_left)
        exit(2);
    interp816_runOpcode(r);
    fetch_left = 0;
}
static int same_trace(void) {
    for (unsigned i = 0; i < original_trace.count && i < native_trace.count; ++i) {
        SnesVerifyBusEvent *a = &original_trace.events[i], *b = &native_trace.events[i];
        if (a->address != b->address || a->value != b->value || a->write != b->write) {
            fprintf(stderr, "trace%u original%06X=%02X/%u native%06X=%02X/%u\n", i,
                    a->address, a->value, a->write, b->address, b->value, b->write);
            return 0;
        }
    }
    return original_trace.count == native_trace.count;
}

static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, suite >= 7u ? 0 : (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(child_visits, 0, sizeof child_visits);
    cpu->program_bank = 0x83u;
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    unsigned bank = number % 129u;
    cpu->data_bank = suite < 4u ? (uint8_t)number
                     : bank == 128u
                         ? 0x7eu
                         : (uint8_t)((bank & 0x3fu) | (bank & 0x40u ? 0x80u : 0u));
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->decimal = 0u;
    cpu->accumulator_is_8_bit = suite >= 4u;
    cpu->index_is_8_bit = (number >> 8) & 1u;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 29u);
    cpu->y = (uint16_t)(number * 113u);
    if (edge_profile) {
        static const uint32_t bases[] = {
            WRAM_OBJECT_DISPLAY_OFFSET_X, WRAM_OBJECT_DISPLAY_OFFSET_Y,
            WRAM_OBJECT_FINE_X,           WRAM_OBJECT_FINE_Y,
            WRAM_OBJECT_SPRITE_SLOT_A,    WRAM_OBJECT_SPRITE_SLOT_B,
            WRAM_OBJECT_SPRITE_SIZE,      WRAM_OBJECT_ANIMATION_FLAGS};
        uint32_t base = bases[suite < 4u ? suite : 4u + (number >> 4) % 4u];
        static const uint16_t offsets[] = {0u,      1u,      0x1ffeu, 0x1fffu, 0x2000u,
                                           0x2100u, 0x211bu, 0x2180u, 0x4202u, 0x4203u,
                                           0x7fffu, 0x8000u, 0xfffeu, 0xffffu};
        uint16_t target = (number & 15u) < 14u
                              ? offsets[number & 15u]
                              : (uint16_t)(cpu->stack + 1u + (number & 1u));
        cpu->x = (uint16_t)(0x800000u + target - base);
        cpu->y = target;
        cpu->accumulator =
            (uint16_t[]){0u, 1u, 0x7fffu, 0x8000u, 0xffffu, 0xffu, 0x100u, 0x5555u}
                [(number >> 5) & 7u];
    }
    if (cpu->index_is_8_bit) {
        cpu->x &= 255u;
        cpu->y &= 255u;
    }
    word(0xa7u, edge_profile
                    ? (uint16_t)(0x800000u + cpu->stack + 1u - WRAM_OBJECT_SPRITE_SIZE)
                    : (uint16_t)(number * 13u));
    native.wram[WRAM_RANDOM_NEXT_INDEX] = (uint8_t)number;
    for (unsigned i = 0u; i < WRAM_RANDOM_TABLE_COUNT; ++i)
        native.wram[WRAM_RANDOM_TABLE + i] = (uint8_t)(number * 37u + i * 19u);
    if (suite >= 7u) {
        unsigned slot = number % 30u + 1u, record = slot * 3u;
        cpu->data_bank = (uint8_t[]){0u, 0x83u, 0x7eu, 0x80u}[number % 4u];
        native.wram[WRAM_OBJECT_STATE + slot] = 0x90u;
        native.wram[WRAM_UNK_7FDFAE - 0x7e0000u + slot] = 1u;
        word(WRAM_OBJECT_SCRIPT + record, 0x1800u);
        native.wram[WRAM_OBJECT_SCRIPT - 0x7e0000u + record + 2u] =
            parent_bank_profile ? (uint8_t[]){0x7eu, 0u, 0x83u, 0x80u}[number % 4u]
                                : 0x7eu;
        native.wram[0x1800u] =
            (uint8_t[]){0xf6u, 0xf7u, 0xfbu, 0xfcu, 0xf2u, 0xf1u}[suite - 7u];
        native.wram[0x1801u] = suite == 12u ? 0x42u : (uint8_t)number;
        native.wram[0x1802u] = (suite == 7u || suite == 9u || suite == 12u)
                                   ? 0x41u
                                   : (uint8_t)(number * 37u);
        native.wram[0x1803u] = 0x41u;
        native.wram[WRAM_OBJECT_ANIMATION_ID - 0x7e0000u + 31u] = 0x42u;
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
    original.m7_a = native.m7_a = (uint16_t)(number * 593u);
    original.m7_b = native.m7_b = (uint8_t)(number * 19u);
    original.m7_latch = native.m7_latch = (uint8_t)(number * 73u);
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, entries[suite]);
    const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                 NULL};
    static Lufia2ExecutionResult (*const functions[])(const Lufia2Memory *,
                                                      Lufia2CpuState *) = {
        Lufia2ObjectAddDisplayOffsetX,  Lufia2ObjectAddDisplayOffsetY,
        Lufia2ObjectAddFinePositionX,   Lufia2ObjectAddFinePositionY,
        Lufia2ObjectReadRandomOffset,   Lufia2ObjectReadSubcommand,
        Lufia2ObjectCopyAnimationState, Lufia2ObjectSlotsUpdate,
        Lufia2ObjectSlotsUpdate,        Lufia2ObjectSlotsUpdate,
        Lufia2ObjectSlotsUpdate,        Lufia2ObjectSlotsUpdate,
        Lufia2ObjectSlotsUpdate};
    Lufia2ExecutionResult result = functions[suite](&memory, &cpu);
    uint32_t stop = result.pc;
    if (result.flow != LUFIA2_EXECUTION_RETURNED) {
        fprintf(stderr, "unexpected flow%u PC%06X suite%u case%u\n", result.flow,
                result.pc, suite, number);
        return 1;
    }
    if (force_return || number % 33u == 0u) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        stop = 0x830000u | (uint16_t)(back + 1u);
    }
    unsigned steps;
    for (steps = 0u; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop && reference->sp == cpu.stack)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    static const unsigned used[] = {0u, 0u, 2u, 2u, 4u, 6u};
    int missing_child = suite >= 7u && !child_visits[used[suite - 7u]];
    int mmio = original.mmio_count != native.mmio_count || original.mmio_overflow ||
               native.mmio_overflow ||
               memcmp(original.mmio, native.mmio,
                      original.mmio_count * sizeof original.mmio[0]) ||
               original.m7_a != native.m7_a || original.m7_b != native.m7_b ||
               original.m7_latch != native.m7_latch ||
               original.wm_address != native.wm_address ||
               original.multiply_a != native.multiply_a ||
               original.multiply_b != native.multiply_b ||
               original.multiply_result != native.multiply_result ||
               original.divide_a != native.divide_a ||
               original.divide_result != native.divide_result;
    if (steps == 1000000u || missing_child || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || mmio ||
        (suite < 7u && !same_trace())) {
        fprintf(
            stderr,
            "object%06X family%u case%u CPU%d WRAM%d MMIO%d PC%06X/%06X steps%u traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
            entries[suite], suite, number, memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE), mmio, stop,
            SnesVerifyPc24(reference), steps, native_trace.count, original_trace.count,
            cpu.accumulator, reference->a, cpu.x, reference->x, cpu.y, reference->y,
            cpu.stack, reference->sp, cpu.data_bank, reference->db);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc > 4) {
        uint32_t address = (uint32_t)strtoul(argv[3], NULL, 16);
        size_t offset = ((address >> 16) & 0x7fu) * 0x8000u + (address & 0x7fffu);
        if (offset >= size)
            return 2;
        rom[offset] = (uint8_t)strtoul(argv[4], NULL, 16);
    }
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *reference =
        interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    static const uint8_t base_lengths[256] = {
        2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 3u, 2u, 4u, 2u, 2u, 2u, 2u, 2u,
        1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 1u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 4u, 3u, 3u, 4u, 1u, 2u, 3u, 2u,
        2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 3u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u,
        3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
    };
    static const uint8_t accumulator_immediates[] = {0x09, 0x29, 0x49, 0x69,
                                                     0x89, 0xa9, 0xc9, 0xe9};
    static const uint8_t index_immediates[] = {0xa0, 0xa2, 0xc0, 0xe0};
    for (unsigned m = 0; m < 2u; ++m)
        for (unsigned x = 0; x < 2u; ++x) {
            memcpy(lengths[m * 2u + x], base_lengths, sizeof base_lengths);
            if (!m)
                for (unsigned i = 0; i < 8u; ++i)
                    ++lengths[m * 2u + x][accumulator_immediates[i]];
            if (!x)
                for (unsigned i = 0; i < 4u; ++i)
                    ++lengths[m * 2u + x][index_immediates[i]];
        }

    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0;
    edge_profile = argc > 2 && strcmp(argv[2], "--edges") == 0;
    parent_bank_profile = argc > 2 && strcmp(argv[2], "--parent-banks") == 0;
    unsigned cases = force_return          ? 256u
                     : edge_profile        ? 4096u
                     : parent_bank_profile ? 256u
                     : argc > 2            ? (unsigned)strtoul(argv[2], NULL, 10)
                                           : 2048u;
    if ((force_return || edge_profile || parent_bank_profile) && argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0u;
    for (suite = parent_bank_profile ? 7u : 0u; suite < (edge_profile ? 7u : 13u);
         ++suite) {
        for (number = 0u; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS object family%u entry%06X states%u cumulative%u\n", suite,
               entries[suite], cases, total);
        fflush(stdout);
    }
    printf("PASS%u original object helper and caller states\n", total);
    return 0;
}
