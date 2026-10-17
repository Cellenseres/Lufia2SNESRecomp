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
static unsigned suite, number, force_return, unknown_mode, edge_mode, limit_mode;
static unsigned returned[7], boundaries[7], rewritten[7];
static unsigned allocation_tail, tail_loop;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x83aa50u, 0x83dffdu, 0x80e92au, 0x83a9bau,
                                   0x80e15au, 0x80cd88u, 0x80e458u};
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

static void SeedLegacyConditions(Lufia2CpuState *cpu) {
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

    if (edge_mode) {
        unsigned bank = number % 129u;
        cpu->data_bank = bank == 128u
                             ? 0x7eu
                             : (uint8_t)((bank & 0x3fu) | (bank & 0x40u ? 0x80u : 0u));
        if (suite == 8u)
            cpu->data_bank = 0x7eu;
        if (suite == 0u) {
            word(0xa7u, (uint16_t[]){0u, 7u, 0xffffu, 0x5024u, 0x70a4u, 0x4da5u,
                                     0x2ea4u, 0x8000u}[number % 8u]);
            if (!number)
                word(0xa7u, 0x4da5u);
            native.wram[0x1d19bu] = (uint8_t)(number * 73u + 19u);
        }
        if (suite >= 2u && suite <= 6u) {
            cpu->accumulator =
                (uint16_t)((cpu->accumulator & 0xff00u) |
                           (uint8_t[]){0xe0u, 0xfbu, 0x60u, 0x20u}[number % 4u]);
            word(0x7ef022u, 0x0100u);
            native.wram[0xf100u] = 0u;
            native.wram[0xf101u] = (uint8_t)left;
            native.wram[0xf102u] = (uint8_t)top;
            native.wram[0xf103u] = 0xffu;
            word(0x7ef024u, 0x0120u);
            native.wram[0xf120u] = 0u;
            for (unsigned i = 0; i < 4u; ++i)
                native.wram[0xf121u + i] = native.wram[0x9fu + i];
            native.wram[0xf125u] = 0xffu;
            for (unsigned i = 0; i < 8u; ++i) {
                native.wram[EVENT_SLOT_X - 0x7e0000u + i] = (uint8_t)left;
                native.wram[EVENT_SLOT_Y - 0x7e0000u + i] = (uint8_t)top;
            }
            if (suite == 6u && number < 3u) {
                cpu->accumulator = (uint16_t)((cpu->accumulator & 0xff00u) | 0xe0u);
                native.wram[EVENT_POINT_D223 - 0x7e0000u] =
                    (uint8_t)(left + (number != 1u ? 0u : 1u));
                native.wram[EVENT_POINT_D263 - 0x7e0000u] =
                    (uint8_t)(top + (number != 0u ? 0u : 1u));
                native.wram[0xaeu] = 0u;
            }
        }
        if (suite == 7u) {
            cpu->x = (uint16_t[]){0u, 2u, 10u, 0x8000u, 0xffffu}[number % 5u];
            cpu->y = cpu->x >= 0x8000u ? 0u : (uint16_t)(number * 197u);
        }
        if (suite == 8u)
            cpu->x = (uint16_t[]){0u, 2u, 4u, 6u, 8u, 0xffffu}[number % 6u];
    }

    if (limit_mode) {
        native.wram[EVENT_SCRIPT_BASE_BANK - 0x7e0000u] = 0xa0u;
        native.wram[EVENT_SCRIPT_BANK - 0x7e0000u] = 0xa0u;
        cpu->x = 0u;
        if (suite == 7u)
            cpu->y = 2u;
        else {
            cpu->data_bank = 0xa0u;
            native.wram[0x1d19bu] = 0x80u;
            word(0x7ef024u, 0x0200u);
            native.wram[0xf200u] = native.wram[0xf201u] = native.wram[0xf202u] = 0u;
            native.wram[0xf203u] = native.wram[0xf204u] = 1u;
            native.wram[0xf205u] = 0xffu;
            native.wram[WRAM_ACTOR_TILE_X] = native.wram[WRAM_ACTOR_TILE_Y] = 5u;
        }
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    if (suite == 7u)
        native.wram[(uint16_t)(cpu->stack + 3u)] = 0x83u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
}
static void seed(Lufia2CpuState *cpu) {
    if (suite == 2u || suite >= 4u) {
        unsigned current = suite;
        suite = current == 4u ? 3u : current == 6u ? 6u : 5u;
        SeedLegacyConditions(cpu);
        suite = current;
        cpu->accumulator =
            (uint16_t)((cpu->accumulator & 0xff00u) |
                       (current == 2u
                            ? number & 255u
                            : (uint8_t[]){0xfbu, 0xe0u, 0x60u, 0x20u}[number % 4u]));
        for (unsigned i = 0u; i < 8u; ++i) {
            native.wram[EVENT_SLOT_X - 0x7e0000u + i] = (uint8_t)(number % 8u);
            native.wram[EVENT_SLOT_Y - 0x7e0000u + i] = (uint8_t)(number / 8u % 8u);
        }
        word(0x7ef022u, 0x0800u);
        word(0x7ef024u, 0x0200u);
        unsigned areas = number & 256u ? 8u : 128u;
        for (unsigned i = 0u; i < areas; ++i) {
            unsigned at = 0xf200u + i * 5u;
            native.wram[at] = (uint8_t)i;
            native.wram[at + 1u] = (uint8_t)(i % 8u);
            native.wram[at + 2u] = (uint8_t)(i / 8u % 8u);
            native.wram[at + 3u] = (uint8_t)(i % 8u + 1u);
            native.wram[at + 4u] = (uint8_t)(i / 8u % 8u + 1u);
        }
        native.wram[0xf200u + areas * 5u] = 0xffu;
        for (unsigned i = 0u; i < 255u; ++i) {
            unsigned at = 0xf800u + i * 3u;
            native.wram[at] = (uint8_t)i;
            native.wram[at + 1u] = (uint8_t)(i % 8u);
            native.wram[at + 2u] = (uint8_t)(i / 8u % 8u);
        }
        native.wram[0xfafdu] = 0xffu;
    } else {
        memset(cpu, 0, sizeof *cpu);
        memset(native.wram, suite == 1u ? (int)(number * 71u) : 0,
               SNES_VERIFY_WRAM_SIZE);
        cpu->program_bank = 0x83u;
        unsigned bank = number % 129u;
        cpu->data_bank = suite == 1u ? (uint8_t)number
                         : bank == 128u
                             ? 0x7eu
                             : (uint8_t)((bank & 0x3fu) | (bank & 0x40u ? 0x80u : 0u));
        UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
        cpu->accumulator_is_8_bit = (number >> 8) & 1u;
        cpu->index_is_8_bit = (number >> 9) & 1u;
        cpu->decimal = 0u;
        cpu->accumulator = (uint16_t)(number * 197u);
        cpu->x = (uint16_t)(number * 29u);
        cpu->y = (uint16_t)(number * 113u);
        if (cpu->index_is_8_bit) {
            cpu->x &= 255u;
            cpu->y &= 255u;
        }
        cpu->stack = (uint16_t[]){0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 10) % 3u];
        word(0xa7u, (uint16_t)(number % 40u));
        word(0xa9u, (uint16_t)(number * 3u));
        word(0xabu, (uint16_t[]){0x4013u, 0xffffu, 0u, 117u, 0x210fu, 0x2111u, 0x8000u,
                                 0x40ffu}[number % 8u]);
        if (suite == 1u && number % 8u == 0u)
            word(0xabu, (uint16_t)(0x800000u + cpu->stack + 1u - WRAM_OBJECT_SCRIPT));
        unsigned slot = number % 40u;
        native.wram[WRAM_UNK_7FE216 - 0x7e0000u + slot] = (uint8_t)number;
        memset(native.wram + 0xe100u, suite == 0u && (number & 1024u) ? 0xff : 0, 256u);
        for (unsigned i = 0u; i < 40u; ++i)
            native.wram[0x1291u + i] = (uint8_t)(number + i);
        word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
        if (suite == 1u || suite == 3u)
            native.wram[(uint16_t)(cpu->stack + 3u)] = 0x82u;
    }
    if (limit_mode) {
        memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
        cpu->accumulator = (uint16_t)((cpu->accumulator & 0xff00u) | 0x9eu);
        word(0x7ef024u, 0u);
        word((uint16_t)(cpu->stack + 1u), 0x9123u);
    }
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
        Lufia2ActorAllocateSprite,    Lufia2ActorLoadSecondaryScript,
        Lufia2FieldResolveEventArea,  Lufia2ActorLoadSprite,
        Lufia2FieldTestLeaderArea,    Lufia2FieldFindObjectInArea,
        Lufia2FieldFindCellBitsInArea};
    uint16_t entry_stack = cpu.stack;
    uint8_t entry_status = PackStatus(&cpu), entry_bank = cpu.data_bank;
    uint32_t before = native.wram[(uint16_t)(entry_stack + 1u)] |
                      ((uint32_t)native.wram[(uint16_t)(entry_stack + 2u)] << 8);
    if (suite == 1u || suite == 3u)
        before |= (uint32_t)native.wram[(uint16_t)(entry_stack + 3u)] << 16;
    Lufia2ExecutionResult result = functions[suite](&memory, &cpu);
    uint32_t stop = result.pc;
#ifdef RUNTIME_ABI
    if (runtime_fallback[suite] || unexpected_controlled)
        return 1;
#endif
    if (limit_mode &&
        (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x80bfbcu ||
         cpu.stack != (uint16_t)(entry_stack - 4u) || cpu.data_bank != 0x7eu ||
         native.wram[(uint16_t)(entry_stack - 3u)] != entry_bank))
        return 1;
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        ++returned[suite];
        uint32_t after = native.wram[(uint16_t)(entry_stack + 1u)] |
                         ((uint32_t)native.wram[(uint16_t)(entry_stack + 2u)] << 8);
        if (suite == 1u || suite == 3u)
            after |= (uint32_t)native.wram[(uint16_t)(entry_stack + 3u)] << 16;
        if (before != after)
            ++rewritten[suite];
    } else {
        ++boundaries[suite];
        if (suite == 0u &&
            (result.pc != 0x83aa5fu || A8(&cpu) ||
             cpu.stack != (uint16_t)(entry_stack - 1u) || !cpu.accumulator_is_8_bit ||
             !cpu.index_is_8_bit || native.wram[entry_stack] != entry_status))
            return 1;
    }

    tail_loop = 0u;
    if (allocation_tail) {
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x83aa5fu)
            return 1;
        Interp816 *tail =
            interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
        load(tail, &cpu, result.pc);
        unsigned steps = 0u, visits = 0u;
        unsigned occupied = (number & 1024u) != 0u;
        uint32_t landing = occupied ? 0x83ab8au : 0x83aa7cu;
        for (; steps < 1000000u; ++steps) {
            if (SnesVerifyPc24(tail) == landing) {
                if (!occupied || visits++ == 64u)
                    break;
            }
            instruction(&native, tail);
        }
        if (steps == 1000000u)
            return 1;
        store(&cpu, tail);
        interp816_free(tail);
        tail_loop = occupied;
        result.flow = occupied ? LUFIA2_EXECUTION_BOUNDARY : LUFIA2_EXECUTION_RETURNED;
        result.pc = stop = landing;
    }
    if (result.flow == LUFIA2_EXECUTION_RETURNED &&
        (force_return || number % 33u == 0u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        if (suite == 1u || suite == 3u)
            cpu.program_bank = Pull8(&memory, &cpu);
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps, stop_visits = 0u;
    for (steps = 0u; steps < 40000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop && reference->sp == cpu.stack) {
            unsigned required = limit_mode ? EVENT_SEARCH_LIMIT : tail_loop ? 64u : 0u;
            if (stop_visits++ == required)
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
            "resource%06X case%u CPU%d WRAM%d PC%06X/%06X steps%u traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X flow%u\n",
            entries[suite], number, memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE), stop,
            SnesVerifyPc24(reference), steps, native_trace.count, original_trace.count,
            cpu.accumulator, reference->a, cpu.x, reference->x, cpu.y, reference->y,
            cpu.stack, reference->sp, cpu.data_bank, reference->db,
            (unsigned)result.flow);
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

    if (argc > 2 && strcmp(argv[2], "--allocation-tail") == 0) {
        allocation_tail = 1u;
        suite = 0u;
        unsigned total = 0u, free_states = 0u, occupied_states = 0u;
        for (number = 0u; number < 4096u; ++number) {
            unsigned type = number & 255u;
            if (rom[0x1abf4u + type])
                continue;
            if (run(reference))
                return 1;
            ++total;
            if (number & 1024u)
                ++occupied_states;
            else
                ++free_states;
        }
        printf(
            "PASS%u zero-count original allocation continuations;free%u occupied64loops%u\n",
            total, free_states, occupied_states);
        return 0;
    }
    if (argc > 2 && strcmp(argv[2], "--list-limit") == 0) {
        limit_mode = 1u;
        suite = 2u;
        for (number = 0u; number < 3u; ++number)
            if (run(reference))
                return 1;
        if (boundaries[2] != 3u)
            return 1;
        puts("PASS3 original event-area list-limit live frames");
        return 0;
    }
    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0;
    unsigned cases = force_return ? 256u
                     : argc > 2   ? (unsigned)strtoul(argv[2], NULL, 10)
                                  : 2048u;
    if (force_return && argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0u;
    for (suite = 0; suite < 7u; ++suite) {
        for (number = 0u; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf(
            "PASS resource%06X states%u cumulative%u returned%u boundaries%u rewritten%u\n",
            entries[suite], cases, total, returned[suite], boundaries[suite],
            rewritten[suite]);
        fflush(stdout);
    }
    printf("PASS%u actor/resource/area original states\n", total);
    return 0;
}
