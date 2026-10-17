#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "lufia2/menu.h"
#include "field/event_script_internal.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[262144];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, edge_profile, zero_profile,
    long_fill_profile;
static unsigned child_visits[4];
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x808ba7u, 0x808bb2u, 0x808c6cu, 0x808c9eu,
                                   0x808878u, 0x808878u, 0x808878u, 0x808878u};
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
        for (unsigned i = 0u; i < 4u; ++i)
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
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(child_visits, 0, sizeof child_visits);
    cpu->program_bank = 0x80u;
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    unsigned bank = number % 129u;
    cpu->data_bank =
        bank == 128u ? 0x7eu : (uint8_t)((bank & 0x3fu) | (bank & 0x40u ? 0x80u : 0u));
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->decimal = cpu->index_is_8_bit = 0u;
    cpu->accumulator_is_8_bit = suite == 2u ? 1u : (number >> 7) & 1u;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 113u);
    cpu->y = (uint16_t)(number % 16u);
    word(0x5du, 0x1800u);
    native.wram[0x5fu] = (uint8_t[]){0u, 0x7eu, 0x83u, 0x80u}[number % 4u];
    uint16_t script = (uint16_t)(0x1800u + cpu->y);
    word(0x7e0000u + script, 0x1920u);
    word(0x7e0000u + script + 2u, 0x1950u);
    word(0x7e0000u + script + 4u, 0x1960u);
    word(0x7e0000u + script + 6u, (uint16_t)(number * 3u));
    word(0x7e1920u, (uint16_t)(0x1940u + number % 32u));
    word(0x7e1960u, 0x1970u);
    word(0x56u, 0x1940u);
    for (unsigned i = 0u; i < 8u; ++i)
        native.wram[0x1940u + i] = (uint8_t)(number * 37u + i * 71u);
    native.wram[0x57bu] =
        suite == 2u ? (uint8_t)((uint8_t[]){0x80u, 0x40u, 0x20u}[(number >> 5) % 3u] |
                                (number & 31u))
                    : (uint8_t)number;
    if (edge_profile) {
        static const uint16_t bases[] = {0u,      0x54u,   0x56u,   0x5du,
                                         0x60u,   0x1ffeu, 0x1fffu, 0x7fffu,
                                         0x8000u, 0xfffdu, 0xfffeu, 0xffffu};
        static const uint16_t offsets[] = {0u, 1u, 0x7fffu, 0x8000u, 0xfffeu, 0xffffu};
        static const uint16_t values[] = {0u,     0x54u,   0x56u,   0x570u,
                                          0x57bu, 0x1fffu, 0x7fffu, 0xffffu};
        word(0x5du, bases[number / 8u % 12u]);
        native.wram[0x5fu] = (uint8_t[]){0u,    0x7eu, 0x7fu, 0x80u,
                                         0x83u, 0xbfu, 0xfeu, 0xffu}[number % 8u];
        cpu->y = offsets[number / 96u % 6u];
        word(0x56u, values[number / 576u % 8u]);
    }
    if (zero_profile)
        native.wram[0x57bu] = 0u;
    if (long_fill_profile) {
        cpu->data_bank = number & 1u ? 0x83u : 0x7eu;
        cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[number / 8u % 4u];
        native.wram[0x57bu] = (uint8_t[]){4u, 0xcu, 0x14u, 0x1cu}[number / 2u % 4u];
        memset(native.wram + 0x1940u, 0x80u | (number & 0x7fu), 8u);
    }
    if (suite >= 4u) {
        memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
        cpu->accumulator_is_8_bit = (number >> 8) & 1u;
        cpu->index_is_8_bit = (number >> 9) & 1u;
        cpu->x = 0xe0u;
        cpu->y = cpu->index_is_8_bit ? 0x80u : 0x1800u;
        native.wram[0x5fu] = 0x7eu;
        unsigned at = cpu->y;
        if (suite == 4u) {
            native.wram[at++] = 1u;
            native.wram[at++] =
                (uint8_t)((uint8_t[]){0x80u, 0x40u, 0x20u}[number % 3u] |
                          (number & 0x1eu));
            native.wram[at++] = (uint8_t)(number % 8u);
            word(0x7e0000u + at, 0x1940u);
            at += 2u;
            native.wram[0x1940u] = (uint8_t)number;
            native.wram[0x1941u] = (uint8_t)(number * 37u);
            native.wram[0x1942u] = (uint8_t)(number * 71u);
        } else if (suite < 7u) {
            native.wram[at++] = 2u;
            native.wram[at++] = suite == 5u ? 4u : 7u;
            word(0x7e0000u + at, 0x1920u);
            at += 2u;
            if (suite == 6u) {
                word(0x7e1920u, 0x1940u);
                word(0x7e0000u + at, (uint16_t)(number % 16u));
                at += 2u;
                native.wram[0x1940u + number % 16u] = (uint8_t)(number % 8u);
            } else
                native.wram[0x1920u] = (uint8_t)(number % 8u);
            for (unsigned i = 0u; i < 8u; ++i)
                word(0x7e0a80u + 2u * i, 0x1a00u);
            native.wram[0x1a00u] = 0x41u;
        } else {
            uint8_t flags = (uint8_t[]){0u, 0x80u, 0x20u, 0x40u,
                                        8u, 0x88u, 0x28u, 0x48u}[number % 8u];
            native.wram[at++] = 3u;
            native.wram[at++] = (uint8_t)(flags | number % 3u);
            word(0x7e0000u + at, 0x1920u);
            at += 2u;
            word(0x7e1920u, flags & 8u ? 0x1940u : (uint16_t)number);
            word(0x7e0000u + at, flags & 8u ? 0x10u : (uint16_t)(number * 3u));
            at += 2u;
            if (flags & 8u) {
                word(0x7e0000u + at, 0x1960u);
                at += 2u;
                word(0x7e0000u + at, (uint16_t)(number % 16u));
                at += 2u;
                word(0x7e1950u, (uint16_t)number);
                word(0x7e1960u, 0x1980u);
            }
            word(0x7e0000u + at, (uint16_t)(at + 2u));
            at += 2u;
        }
        native.wram[at] = 0u;
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    if (suite >= 4u)
        native.wram[(uint16_t)(cpu->stack + 3u)] = 0x82u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, entries[suite]);
    const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                 NULL};
    static Lufia2ExecutionResult (*const functions[])(const Lufia2Memory *,
                                                      Lufia2CpuState *) = {
        Lufia2MenuReadPointerOperand, Lufia2MenuReadOffsetPointerOperand,
        Lufia2MenuLoadNumberBytes,    Lufia2MenuReadBranchOperands,
        Lufia2MenuDrawString,         Lufia2MenuDrawString,
        Lufia2MenuDrawString,         Lufia2MenuDrawString};
    uint16_t entry_stack = cpu.stack;
    uint16_t entry_frame =
        (uint16_t)(native.wram[(uint16_t)(entry_stack + 1u)] |
                   ((uint16_t)native.wram[(uint16_t)(entry_stack + 2u)] << 8));
    Lufia2ExecutionResult result = functions[suite](&memory, &cpu);
    uint32_t stop = result.pc;
    if (zero_profile) {
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || stop != 0x808c7bu ||
            cpu.stack != entry_stack || cpu.y || A8(&cpu) ||
            !cpu.accumulator_is_8_bit || cpu.index_is_8_bit)
            return 1;
        unsigned steps = 0u;
        while (SnesVerifyPc24(reference) != stop && steps++ < 100u)
            instruction(&original, reference);
        Lufia2CpuState prefix = cpu;
        store(&prefix, reference);
        if (steps >= 100u || memcmp(&prefix, &cpu, sizeof cpu) ||
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace())
            return 1;
        Interp816 *tail =
            interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
        load(tail, &cpu, stop);
        unsigned visits = 0u;
        steps = 0u;
        while (steps++ < 10000u) {
            if (SnesVerifyPc24(tail) == stop && visits++ == 64u)
                break;
            instruction(&native, tail);
        }
        if (steps >= 10000u)
            return 1;
        store(&cpu, tail);
        interp816_free(tail);
    } else if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return 1;
    if (long_fill_profile) {
        uint16_t after =
            (uint16_t)(native.wram[(uint16_t)(entry_stack + 1u)] |
                       ((uint16_t)native.wram[(uint16_t)(entry_stack + 2u)] << 8));
        if (after == entry_frame || cpu.y != 3u)
            return 1;
    }

    if (!zero_profile && (force_return || number % 33u == 0u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        if (suite >= 4u)
            cpu.program_bank = Pull8(&memory, &cpu);
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps;
    unsigned stop_visits = 0u;
    for (steps = 0u; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop && reference->sp == cpu.stack) {
            if (!zero_profile || stop_visits++ == 64u)
                break;
        }
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (suite >= 4u) {
        unsigned child = suite == 4u ? 2u : suite == 5u ? 0u : suite == 6u ? 1u : 3u;
        if (!child_visits[child]) {
            fprintf(stderr, "missing menu child family%u case%u\n", suite, number);
            return 1;
        }
    }
    int mmio = original.mmio_count != native.mmio_count ||
               (long_fill_profile ? original.mmio_overflow != native.mmio_overflow
                                  : (original.mmio_overflow || native.mmio_overflow)) ||
               memcmp(original.mmio, native.mmio,
                      original.mmio_count * sizeof original.mmio[0]) ||
               original.wm_address != native.wm_address ||
               original.m7_a != native.m7_a || original.m7_b != native.m7_b ||
               original.m7_latch != native.m7_latch ||
               original.multiply_a != native.multiply_a ||
               original.multiply_b != native.multiply_b ||
               original.multiply_result != native.multiply_result ||
               original.divide_a != native.divide_a ||
               original.divide_result != native.divide_result;
    if (steps == 1000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || mmio ||
        (suite < 4u && !same_trace())) {
        fprintf(
            stderr,
            "operand%06X case%u CPU%d WRAM%d MMIO%d PC%06X/%06X steps%u traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
            entries[suite], number, memcmp(&expected, &cpu, sizeof cpu),
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

    if (argc > 2 && strcmp(argv[2], "--parents") == 0) {
#ifdef RUNTIME_ABI
        fputs(
            "Parent cohort requires standalone original-caller proof; no whole-parent ABI claim.\n",
            stderr);
        return 2;
#else
        unsigned count = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 1024u,
                 total = 0u;
        if (!count)
            return 2;
        for (suite = 4u; suite < 8u; ++suite) {
            for (number = 0u; number < count; ++number) {
                if (run(reference))
                    return 1;
                ++total;
            }
            printf("PASS menu parent%u states%u cumulative%u\n", suite, count, total);
            fflush(stdout);
        }
        printf("PASS%u original menu string caller states\n", total);
        return 0;
#endif
    }
    if (argc > 2 && (strcmp(argv[2], "--zero-format") == 0 ||
                     strcmp(argv[2], "--long-fill") == 0)) {
        zero_profile = strcmp(argv[2], "--zero-format") == 0;
        long_fill_profile = !zero_profile;
        suite = 2u;
        unsigned count = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10)
                                  : (zero_profile ? 1024u : 32u);
        if (!count)
            return 2;
        for (number = 0u; number < count; ++number)
            if (run(reference))
                return 1;
        printf("PASS%u original number %s states\n", count,
               zero_profile ? "zero-prefix-and64-loop-continuation"
                            : "finite-wrap-fill-and-rewritten-frame");
        return 0;
    }
    edge_profile = argc > 2 && strcmp(argv[2], "--edges") == 0;
    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0;
    unsigned cases = edge_profile
                         ? (argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 4096u)
                     : force_return ? 256u
                     : argc > 2     ? (unsigned)strtoul(argv[2], NULL, 10)
                                    : 4096u;
    if (force_return && argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned start = argc == 4 && !edge_profile && !force_return
                         ? (unsigned)strtoul(argv[3], NULL, 10)
                         : 0u;
    if (start > 0xffffffffu - cases)
        return 2;
    unsigned total = 0u;
    for (suite = 0u; suite < 4u; ++suite) {
        for (number = start; number < start + cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS operand%06X states%u cumulative%u\n", entries[suite], cases,
               total);
        fflush(stdout);
    }
    printf("PASS%u original menu operand states\n", total);
    return 0;
}
