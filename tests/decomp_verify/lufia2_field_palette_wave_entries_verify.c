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
static unsigned suite, number, force_return, limit_mode;
static uint16_t frame_offsets[32], zero_header, link_base;
static unsigned link_mode;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x83aeedu, 0x83af54u, 0x83aeb5u};
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
    memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(cpu, 0, sizeof *cpu);
    cpu->program_bank = 0x83u;
    cpu->data_bank = suite == 2u ? 0x83u : 0x7fu;
    cpu->accumulator_is_8_bit = 1u;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 29u);
    cpu->y = (uint16_t)(number * 113u);
    cpu->stack = (uint16_t)(0x1f00u + number % 253u);
    cpu->carry = number & 1u;
    cpu->zero = (number >> 1) & 1u;
    cpu->negative = (number >> 2) & 1u;
    cpu->overflow = (number >> 3) & 1u;
    cpu->irq_disable = (number >> 4) & 1u;
    unsigned count = number % 256u;
    native.wram[0x9a9u] = suite == 2u ? 1u : (uint8_t)((number & 7u) == 0u ? 6u : 0u);
    native.wram[0x1d0f7u] = (uint8_t)count;
    word(0x7fd0f5u, 0x8000u);
    for (unsigned i = 0; i < count; ++i) {
        word(0x7fec00u + 2u * i, (uint16_t)(frame_offsets[(number + i) % 32u] - 3u));
        native.wram[0x1ed00u + 2u * i] = (uint8_t)(number + i);
        native.wram[0x1ed01u + 2u * i] =
            (uint8_t[]){0u, 1u, 2u, 255u}[(number + i) % 4u];
    }
    uint16_t pointer = (uint16_t)(0xc002u + (number % 32u) * 4u);
    word(0x7fd0c9u, number % 4u == 0u ? 0xffffu : pointer);
    word(0x7fd0c6u, (uint16_t)(number * 113u));
    native.wram[pointer] = (uint8_t)number;
    native.wram[pointer + 1u] = (uint8_t)(number >> 5);
    native.wram[pointer + 2u] = (uint8_t)((number & 2u) ? 0xffu : number);
    native.wram[0x1d0c8u] = number % 4u >= 2u
                                ? (uint8_t)(native.wram[pointer + 1u] - 1u)
                                : (uint8_t)(number * 17u);
    if (link_mode) {
        native.wram[0x9a9u] = suite == 2u ? 1u : 0u;
        native.wram[0x1d0f7u] = 1u;
        word(0x7fec00u, (uint16_t)(zero_header - 3u));
        native.wram[0x1ed01u] = 1u;
        word(0x7fd0f5u, link_base);
    }
    if (limit_mode) {
        native.wram[0x9a9u] = 0u;
        native.wram[0x1d0f7u] = 1u;
        word(0x7fec00u, 0x7ffdu);
        native.wram[0x1ed01u] = 1u;
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
    static Lufia2ExecutionResult (*const functions[])(
        const Lufia2Memory *, Lufia2CpuState *) = {Lufia2FieldAdvancePaletteCycles,
                                                   Lufia2FieldStepWaveTable,
                                                   Lufia2FieldColourEffects};
    Lufia2ExecutionResult result = functions[suite](&memory, &cpu);
    uint32_t stop = result.pc;
    if (result.flow == LUFIA2_EXECUTION_RETURNED &&
        (force_return || number % 33u == 0u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    if (limit_mode && (result.flow != LUFIA2_EXECUTION_BOUNDARY || stop != 0x83af11u))
        return 1;
    unsigned steps = 0u, visits = 0u;
    for (; steps < 40000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop) {
            if (!limit_mode || visits++ == 65536u)
                break;
        }
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 40000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "effects%06X case%u CPU%d WRAM%d PC%06X/%06X steps%u traces%u/%u A%04X/%04X X%04X/%04X S%04X/%04X DB%02X/%02X\n",
            entries[suite], number, memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE), stop,
            SnesVerifyPc24(reference), steps, native_trace.count, original_trace.count,
            cpu.accumulator, reference->a, cpu.x, reference->x, cpu.stack,
            reference->sp, cpu.data_bank, reference->db);
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
    if (argc == 5) {
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned offset = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        if (offset >= size)
            return 2;
        rom[offset] = (uint8_t)strtoul(argv[4], NULL, 16);
    }
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    unsigned found = 0u;
    for (unsigned at = 0x8000u; at < 0xfffdu && found < 32u; ++at)
        if (rom[0x108000u + (at & 0x7fffu)])
            frame_offsets[found++] = (uint16_t)at;
    if (found != 32u)
        return 2;
    for (unsigned at = 0x8000u; at < 0xfffdu; ++at) {
        unsigned mapped = 0x108000u + (at & 0x7fffu);
        if (!zero_header && !rom[mapped])
            zero_header = (uint16_t)at;
        if (!link_base && at < 0xfffbu) {
            unsigned word = rom[mapped + 3u] | ((unsigned)rom[mapped + 4u] << 8);
            unsigned next = (at + word + 1u) & 65535u;
            if (next >= 0x8000u && rom[0x108000u + (next & 0x7fffu)])
                link_base = (uint16_t)at;
        }
    }
    if (!zero_header || !link_base)
        return 2;

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

    if (argc > 2 && strcmp(argv[2], "--cycle-link") == 0) {
        link_mode = 1u;
        unsigned cases = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 32u,
                 total = 0u;
#ifdef RUNTIME_ABI
        unsigned families = 1u;
#else
        unsigned families = 2u;
#endif
        for (unsigned family = 0u; family < families; ++family) {
            suite = family ? 2u : 0u;
            for (number = 0u; number < cases; ++number) {
                if (run(reference))
                    return 1;
                ++total;
            }
        }
        printf("PASS%u terminating zero-duration palette links;unchanged ROM data\n",
               total);
        return 0;
    }
    limit_mode = argc > 2 && strcmp(argv[2], "--cycle-limit") == 0;
    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0;
    if (limit_mode)
        memset(rom + 0x108000u, 0, 0x8000u);
    unsigned cases = limit_mode     ? 3u
                     : force_return ? 256u
                     : argc > 2     ? (unsigned)strtoul(argv[2], NULL, 10)
                                    : 1024u;
    if ((force_return || limit_mode) && argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0u;
    for (suite = 0; suite < (limit_mode ? 1u : 3u); ++suite) {
        for (number = 0; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS effect%06X states%u cumulative%u\n", entries[suite], cases, total);
        fflush(stdout);
    }
    printf("PASS%u field palette/wave states%s\n", total,
           limit_mode ? ";synthetic heap-only palette data" : "");
    return 0;
}
