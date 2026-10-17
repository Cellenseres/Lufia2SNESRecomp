#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "lufia2/world_map.h"
#include "lufia2/ancient_cave.h"
#include "cave/cave_internal.h"
#include "field/event_script_internal.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[4000000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, edge_profile, count_profile, alias_profile,
    zero_boundaries;
static unsigned child_visits[9];
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x839d88u, 0x86d1a1u, 0x86d1e8u, 0x86d271u};
static uint8_t small_blocks[256];
static unsigned small_count;
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
        fprintf(
            stderr,
            "dims%02X:%02X remaining%02X:%02X width%02X:%02X block%02X src%02X%02X dest%02X%02X suite%u\n",
            native.wram[0x57], native.wram[0x56], native.wram[0x59], native.wram[0x58],
            native.wram[0x5b], native.wram[0x5a], native.wram[0x22], native.wram[0x5e],
            native.wram[0x5d], native.wram[0x61], native.wram[0x60], suite);
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
    static const uint8_t banks[] = {0x86u, 0x06u, 0u, 0x3fu, 0x80u, 0xbfu, 0x7eu};
    static const uint16_t positions[] = {0u,      1u,      0x1fffu, 0x3fffu,
                                         0x7fffu, 0xff00u, 0xfffeu, 0xffffu};
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(child_visits, 0, sizeof child_visits);
    cpu->program_bank = suite ? 0x86u : 0x83u;
    cpu->data_bank = suite ? banks[(number >> 2) % 7u] : 0x7fu;
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->decimal = 0u;
    cpu->accumulator_is_8_bit = suite < 2u ? 1u : (number >> 3) & 1u;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 29u);
    cpu->y = (uint16_t)(0x0800u + (number & 15u) * 8u);
    if (!suite) {
        for (unsigned i = 0u; i < 0x10000u; ++i)
            native.wram[0x4000u + i] =
                (i + number) % 5u ? (uint8_t)(i * 37u + number) : 0u;
        native.wram[0x22u] =
            edge_profile ? (uint8_t)number : small_blocks[number % small_count];
        cpu->y =
            edge_profile ? positions[(number >> 5) & 7u] : (uint16_t)(number * 193u);
    } else {
        for (unsigned i = 0u; i < 512u; ++i)
            native.wram[0xd200u + i] = (uint8_t)((i + number) % 5u + 1u);
        for (unsigned i = 0u; i < 16u; ++i) {
            unsigned descriptor = 0x0800u + i * 8u;
            native.wram[descriptor] = (uint8_t)((i & 1u ? 0x80u : 0u) | (number % 3u));
            native.wram[descriptor + 1u] = (uint8_t)(number % 17u);
            native.wram[descriptor + 2u] = (uint8_t)(number % 4u + 1u);
            word(descriptor + 3u, (uint16_t)(number * 157u + i));
            native.wram[descriptor + 5u] = (uint8_t)number;
        }
        word(0x39u, edge_profile ? positions[number & 7u] : (uint16_t)(number * 193u));
        native.wram[0x34u] = (uint8_t)(edge_profile ? number : 0u);
        native.wram[0x36u] = (uint8_t)(edge_profile ? number * 7u : 0u);
        if (suite == 1u) {
            unsigned count = count_profile ? (uint8_t[]){0u,  1u,   2u,  16u,
                                                         32u, 127u, 255u}[number % 7u]
                                           : (number % 8u + 1u);
            native.wram[0x1365u] = (uint8_t)count;
            if (count)
                for (unsigned i = 0u; i < (count < 8u ? count : 8u); ++i) {
                    word(0x1367u + i * 2u, (uint16_t)(0x0800u + i * 8u));
                    word(0x13a7u + i * 2u,
                         (i + number) % 3u ? (uint16_t)(0x1000u + i * 64u) : 0u);
                }
        } else {
            cpu->accumulator =
                (uint16_t)((number * 197u & 0xff00u) |
                           (edge_profile ? (number & 255u) : number % 3u));
            if (edge_profile) {
                native.wram[cpu->y + 1u] = (uint8_t)number;
                native.wram[cpu->y + 2u] = (uint8_t)((number >> 2) & 255u);
            }
        }
    }
    if (alias_profile) {
        static const uint16_t descriptors[] = {
            0u,      0x22u,   0x33u,   0x37u,   0x39u,   0x1365u, 0x1efeu, 0x1f80u,
            0x2000u, 0x2116u, 0x2180u, 0x420bu, 0x4362u, 0x7fffu, 0xfffcu, 0xffffu};
        cpu->y = descriptors[number & 15u];
        cpu->accumulator = (uint16_t)(number * 197u);
        if (suite == 1u) {
            native.wram[0x1365u] = 1u;
            word(0x1367u, cpu->y);
            word(0x13a7u, (uint16_t)(0x20u + number));
        }
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
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
        Lufia2CaveDrawTileBlock, Lufia2WorldMapUploadTileQueue,
        Lufia2WorldMapUploadTileColumn, Lufia2WorldMapUploadTileBlock};
    Lufia2ExecutionResult result = functions[suite](&memory, &cpu);
    uint32_t stop = result.pc;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        if (suite || stop != 0x839daeu) {
            fprintf(stderr, "unexpected boundary%06X suite%u\n", stop, suite);
            return 1;
        }
        ++zero_boundaries;
    } else if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return 1;
    else if (force_return || number % 33u == 0u) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps;
    for (steps = 0u; steps < 8000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop && reference->sp == cpu.stack)
            break;
        instruction(&original, reference);
    }
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        Interp816 *tail =
            interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
        load(tail, &cpu, stop);
        for (unsigned i = 0u; i < 64u; ++i) {
            instruction(&native, tail);
            instruction(&original, reference);
        }
        store(&cpu, tail);
        interp816_free(tail);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
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
    if (steps == 8000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || mmio ||
        !same_trace()) {
        fprintf(
            stderr,
            "tile%06X family%u case%u CPU%d WRAM%d MMIO%d PC%06X/%06X steps%u traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
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

    for (unsigned i = 0u; i < 256u; ++i) {
        unsigned width = SnesVerifyBusRead(&native, 0x93d69du + i * 4u),
                 height = SnesVerifyBusRead(&native, 0x93d69eu + i * 4u);
        if (width * height && width * height <= 256u)
            small_blocks[small_count++] = (uint8_t)i;
    }
    if (!small_count)
        return 2;
    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0;
    edge_profile = argc > 2 && strcmp(argv[2], "--edges") == 0;
    count_profile = argc > 2 && strcmp(argv[2], "--counts") == 0;
    alias_profile = argc > 2 && strcmp(argv[2], "--aliases") == 0;
    unsigned cases = force_return    ? 256u
                     : edge_profile  ? 256u
                     : count_profile ? 28u
                     : argc > 2      ? (unsigned)strtoul(argv[2], NULL, 10)
                                     : 2048u;
    if ((force_return || edge_profile || count_profile) && argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (alias_profile)
        cases = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 512u;
    if (!cases)
        return 2;
    unsigned total = 0u;
    for (suite = (count_profile || alias_profile) ? 1u : 0u;
         suite < (count_profile ? 2u : 4u); ++suite) {
        for (number = 0u; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS tile family%u entry%06X states%u cumulative%u\n", suite,
               entries[suite], cases, total);
        fflush(stdout);
    }
    printf(
        "PASS%u original tile drawing/upload states, original continuation boundaries%u\n",
        total, zero_boundaries);
    return 0;
}
