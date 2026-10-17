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
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, unknown_mode;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x80e4c5u, 0x80e55bu, 0x80e132u,
                                   0x80e15au, 0x80cd8bu, 0x80cd88u,
                                   0x80e458u, 0x80e78du, 0x80e566u};
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

static void seed_base(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    cpu->program_bank = 0x80u;
    cpu->data_bank = (uint8_t[]){0u, 0x83u, 0x3fu, 0xbfu, 0x7eu}[number % 5u];
    if (suite >= 7u)
        cpu->data_bank = 0x7eu;
    cpu->accumulator = (uint16_t)(number * 197u);
    if (suite >= 2u && suite <= 6u)
        cpu->accumulator = (uint16_t)((cpu->accumulator & 0xff00u) | 0xe0u);
    cpu->x = suite == 8u ? (uint16_t)(2u * (number % 3u)) : 0u;
    cpu->y = suite == 7u ? (uint16_t)(2u * (number % 4u)) : 0x8300u;
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->accumulator_is_8_bit = 1u;
    cpu->index_is_8_bit = cpu->decimal = 0u;
    unsigned left = number & 7u, top = (number >> 3) & 7u;
    unsigned width = number % 4u + 1u, height = (number / 4u) % 4u + 1u;
    for (unsigned i = 0; i < 4u; ++i) {
        native.wram[EVENT_POINT_X - 0x7e0000u + i] = (uint8_t)left;
        native.wram[EVENT_POINT_Y - 0x7e0000u + i] = (uint8_t)top;
        native.wram[EVENT_POINT_D223 - 0x7e0000u + i] = (uint8_t)(left + width);
        native.wram[EVENT_POINT_D263 - 0x7e0000u + i] = (uint8_t)(top + height);
    }
    native.wram[0x9fu] = (uint8_t)left;
    native.wram[0xa0u] = (uint8_t)top;
    native.wram[0xa1u] = (uint8_t)(left + width);
    native.wram[0xa2u] = (uint8_t)(top + height);
    native.wram[WRAM_ACTOR_TILE_X] = (uint8_t)(number & 1u ? left : left + width);
    native.wram[WRAM_ACTOR_TILE_Y] = (uint8_t)(number & 2u ? top : top + height);
    word(0xa7u, (uint16_t)(number % 8u));
    word(WRAM_FIELD_SECTION_WIDTH, 32u);
    native.wram[0xaeu] = (uint8_t)(number & 7u);
    for (unsigned i = 0; i < 48u; ++i) {
        native.wram[0x1d69cu + i] = (uint8_t)(i * 3u + number);
        native.wram[0x1d6ccu + i] = (uint8_t)(i * 7u + number);
        native.wram[0x1d6fcu + i] = (uint8_t)(i % 8u);
    }
    if (number & 4u) {
        unsigned i = number % 48u;
        native.wram[0x1d69cu + i] = (uint8_t)left;
        native.wram[0x1d6ccu + i] = (uint8_t)top;
        native.wram[0x1d6fcu + i] = (uint8_t)(number & 7u);
    }
    native.wram[EVENT_CONDITION - 0x7e0000u] = (uint8_t)number;
    native.wram[0x1d19bu] = (uint8_t)(0x80u + number % 4u);
    word(EVENT_SCRIPT_BASE, 0x8000u);
    native.wram[EVENT_SCRIPT_BASE_BANK - 0x7e0000u] = 0x7eu;
    word(EVENT_SCRIPT_POINTER, 0x8300u);
    native.wram[EVENT_SCRIPT_BANK - 0x7e0000u] = cpu->data_bank;
    for (unsigned at = 0; at < 16u; at += 2u)
        word(0x7e8000u + at, 0x0080u);
    for (unsigned i = 0; i < 3u; ++i) {
        native.wram[0x8080u + 3u * i] = (uint8_t)i;
        word(0x7e8081u + 3u * i, 0x0100u);
    }
    native.wram[0x8089u] = native.wram[0x808au] = 0xffu;
    native.wram[0x8100u] = 0x80u;
    native.wram[0x8101u] = 0x81u;
    native.wram[0x8102u] = 0x82u;
    native.wram[0x8103u] = 0xffu;
    if (unknown_mode) {
        cpu->x = (uint16_t[]){6u, 8u, 10u, 0xfffeu, 0xffffu, 0x7fffu}[number % 6u];
        native.wram[0x1d19bu] = (uint8_t)(0x80u + number % 3u);
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    if (suite == 7u)
        native.wram[(uint16_t)(cpu->stack + 3u)] = 0x83u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
}

static unsigned caller_index;
static uint32_t caller_pc;
static const uint8_t caller_opcodes[] = {0x12u, 0x6du, 0x13u, 0x15u, 0x16u,
                                         0x6eu, 0x17u, 0x18u, 0x6fu, 0x72u,
                                         0x73u, 0x74u, 0x04u, 0x70u, 0x05u};
static void seed(Lufia2CpuState *cpu) {
    suite = 0u;
    seed_base(cpu);
    cpu->data_bank = 0x7eu;
    cpu->y = 0x8300u;
    unsigned opcode = caller_opcodes[caller_index];
    cpu->accumulator = cpu->x = (uint16_t)(2u * opcode);
    cpu->carry = 0u;
    cpu->zero = cpu->negative = 0u;
    unsigned table = 0x65a4u + 2u * opcode;
    caller_pc = 0x800000u | native.rom[table] | ((uint32_t)native.rom[table + 1u] << 8);
    word(0x7ef022u, 0x0100u);
    native.wram[0xf100u] = 0u;
    native.wram[0xf101u] = (uint8_t)(number & 7u);
    native.wram[0xf102u] = (uint8_t)((number >> 3) & 7u);
    native.wram[0xf103u] = 0xffu;
    word(0x7ef024u, 0x0120u);
    native.wram[0xf120u] = 0u;
    for (unsigned i = 0; i < 4u; ++i)
        native.wram[0xf121u + i] = native.wram[0x9fu + i];
    native.wram[0xf125u] = 0xffu;
    if (number & 4u)
        native.wram[0x1d6fcu + number % 48u] = 0x10u;
    native.wram[0x8300u] = (uint8_t[]){0x20u, 0xe0u, 0x80u}[number % 3u];
    unsigned argument = 0x8301u;
    if (opcode == 0x04u || opcode == 0x70u || opcode == 0x05u)
        native.wram[argument++] = 0u;
    if (opcode == 0x13u || opcode == 0x15u || opcode == 0x17u || opcode == 0x72u ||
        opcode == 0x05u)
        native.wram[argument] = 0u;
    else
        word(0x7e0000u + argument, 0x0350u);
    native.wram[0x8350u] = 0u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
}
static Lufia2ExecutionResult compose(const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    uint32_t handoff = 0u;
    unsigned result =
        Lufia2EventConditionOpcode(memory, cpu, (uint16_t)caller_pc, &handoff);
    if (result != EVENT_OPCODE_NEXT && result != EVENT_OPCODE_HANDOFF) {
        fputs("unexpected condition caller flow\n", stderr);
        exit(2);
    }
    return ExecutionHandoff(cpu, result == EVENT_OPCODE_NEXT ? 0x80cc35u : handoff);
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, caller_pc);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result = compose(&memory, &cpu);
    uint32_t stop = result.pc;
    if (result.flow == LUFIA2_EXECUTION_RETURNED &&
        (force_return || number % 33u == 0u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        if (suite == 7u)
            cpu.program_bank = Pull8(&memory, &cpu);
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps, jumped = 0u;
    for (steps = 0; steps < 2000000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(reference);
        if (pc == stop &&
            (!unknown_mode || result.flow != LUFIA2_EXECUTION_BOUNDARY || jumped))
            break;
        instruction(&original, reference);
        if (pc == 0x80e58bu)
            jumped = 1u;
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 2000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "condition%06X case%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X flow%u\n",
            caller_pc, number, steps, stop, SnesVerifyPc24(reference),
            memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, cpu.accumulator, reference->a,
            cpu.x, reference->x, cpu.y, reference->y, cpu.stack, reference->sp,
            cpu.data_bank, reference->db, (unsigned)result.flow);
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
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        if (at >= size)
            return 2;
        rom[at] = (uint8_t)strtoul(argv[4], NULL, 16);
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

    (void)entries;
    unsigned cases = argc > 2 ? (unsigned)strtoul(argv[2], NULL, 10) : 256u;
    if (!cases)
        return 2;
    unsigned total = 0u;
    for (caller_index = 0u; caller_index < sizeof caller_opcodes; ++caller_index) {
        for (number = 0u; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS caller opcode%02X states%u\n", caller_opcodes[caller_index],
               cases);
        fflush(stdout);
    }
    printf("PASS%u affected original condition-opcode compositions\n", total);
    return 0;
}
