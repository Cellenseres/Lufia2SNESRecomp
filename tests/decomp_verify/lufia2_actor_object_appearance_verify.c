#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"
#include "lufia2/party.h"
#include "lufia2/actor.h"
#include "lufia2/field.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, alias_mode, boundaries;
static const uint16_t handlers[] = {0xc110u, 0xf6c6u, 0xf6cau, 0xb6b3u};
typedef Lufia2ExecutionResult (*Leaf)(const Lufia2Memory *, Lufia2CpuState *);
static const Leaf leaves[] = {Lufia2ActorPrepareAuxiliarySlot,
                              Lufia2FieldSetupObjectActorSpriteFar,
                              Lufia2FieldSetupObjectActorSprite};
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_83C110(CpuState *);
extern RecompReturn Lufia2DecompBridge_83F6C6(CpuState *);
extern RecompReturn Lufia2DecompBridge_83F6CA(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_83C110, Lufia2DecompBridge_83F6C6, Lufia2DecompBridge_83F6CA};
#endif
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;

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
    if (trace->count == 400000u) {
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
    SnesVerifyBusPoke(&native, a, (uint8_t)v);
    SnesVerifyBusPoke(&native, a + 1u, (uint8_t)(v >> 8));
}
static void seed(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    n->program_bank = suite < 3u ? 0x83u : 0x80u;
    n->accumulator_is_8_bit = 1u;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(suite == 0u ? number % 40u : number * 83u);
    n->y = (uint16_t)(number * 121u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f10u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f40u + number % 128u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    static const uint8_t banks[] = {0, 0x7e, 0x7f, 0x80, 0x83, 0x81};
    n->data_bank = banks[(number / 16u) % 6u];
    uint16_t slot = (uint16_t)(number % 40u);
    if (alias_mode) {
        n->stack = number % 4u < 2u ? 0x1f10u : 0x1ffcu;
        n->data_bank = number < 4u ? 0u : 0x7eu;
        slot = (uint16_t)(n->stack - (number & 1u ? 0u : 1u) - 0x1471u);
    }
    word(0x00a7u, slot);
    word(0x00a9u, (uint16_t)(slot * 2u));
    word(0x00abu, (uint16_t)(slot * 3u));
    for (unsigned i = 0; i < 256u; ++i) {
        native.wram[0x1d88cu + i] = (uint8_t)(number * 37u + i * 11u);
        native.wram[0x1d78cu + i] = (uint8_t)(number + i);
        native.wram[0x1d80cu + i] = (uint8_t)(number * 31u + i);
    }
    native.wram[0x1e5a6u + slot] = (uint8_t)(number * 13u + slot * 17u);
    for (unsigned b = 0; b < 6u; ++b) {
        uint32_t bank = (uint32_t)banks[b] << 16;
        for (unsigned i = 0; i < 40u; ++i) {
            SnesVerifyBusPoke(&native, bank | 0x06bau + i,
                              (uint8_t)(number * 73u + i * 19u));
            SnesVerifyBusPoke(&native, bank | 0x06e2u + i,
                              (uint8_t)(number * 137u + i * 11u));
            SnesVerifyBusPoke(&native, bank | 0x0692u + i, (uint8_t)(number + i));
        }
    }
    if (suite == 3u) {
        static const uint8_t slot_values[] = {0x48, 0x6f, 0x49, 0xff,
                                              0,    0x70, 0x48, 0x6f};
        uint8_t first = slot_values[number % 8u];
        uint8_t record = (uint8_t)(number * 37u);
        n->y = (uint16_t)(0x9000u + number % 8u * 32u);
        uint32_t script = ((uint32_t)n->data_bank << 16) | n->y;
#ifdef RUNTIME_ABI
        if (n->stack < 0x1f40u)
            n->stack = 0x1f40u;
        n->y = (uint16_t)(number * 59u);
        n->data_bank = number % 3u == 0u ? 0x7eu : number % 3u == 1u ? 0x83u : 0u;
        script = 0x7e9001u;
        native.wram[0x09b7u] = 0;
        native.wram[0x09b8u] = 0x90;
        native.wram[0x09b9u] = 0x7e;
        native.wram[0x9000u] = 0xbdu;
        native.wram[0x9003u] = 0x37u;
        native.wram[0x9004u] = 0xffu;
        native.wram[0x099bu] = native.wram[0x099cu] = native.wram[0x1259u] =
            native.wram[0x1d0ffu] = 0;
#endif
        SnesVerifyBusPoke(&native, script, first);
        SnesVerifyBusPoke(&native, script + 1u, record);
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static Lufia2ExecutionResult execute(const Lufia2Memory *m, Lufia2CpuState *n) {
    if (suite < 3u)
        return leaves[suite](m, n);
    uint32_t pc = Lufia2TextExecuteFieldCommand(m, n, handlers[suite]);
    Lufia2ExecutionResult r = {LUFIA2_EXECUTION_BOUNDARY, pc, 0u};
    return r;
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    uint8_t opcode = SnesVerifyBusRead(&original, pc);
    --original_trace.count;
    fetch_trace = &original_trace;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][opcode];
    if (!fetch_left) {
        fprintf(stderr, "unknown opcode%02X at%06X\n", opcode, pc);
        exit(1);
    }
    interp816_runOpcode(r);
    fetch_left = 0;
}
static int same_trace(void) {
    if (original_trace.count != native_trace.count)
        return 0;
    for (unsigned i = 0; i < original_trace.count; ++i) {
        SnesVerifyBusEvent *a = &original_trace.events[i], *b = &native_trace.events[i];
        if (a->address != b->address || a->value != b->value || a->write != b->write) {
            fprintf(stderr, "trace%u original%06X=%02X/%u native%06X=%02X/%u\n", i,
                    a->address, a->value, a->write, b->address, b->value, b->write);
            return 0;
        }
    }
    return 1;
}

static int run(Interp816 *r) {
    Lufia2CpuState n;
    seed(&n);
    uint32_t entry = ((uint32_t)n.program_bank << 16) | handlers[suite];
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    if (suite == 3u)
        entry = 0x809cb8u;
    load(r, &n, entry);
    Lufia2ExecutionResult result = runtime_invoke(
        &m, &n, NULL, NULL, suite < 3u ? bridges[suite] : Lufia2DecompBridge_9CB8,
        suite < 3u ? leaves[suite] : Lufia2TextEngineStep, NULL, entry, suite,
        suite == 2u ? 2u : 3u);
#else
    load(r, &n, entry);
    Lufia2ExecutionResult result = execute(&m, &n);
#endif
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++boundaries;
    uint32_t flow = result.pc;
    unsigned steps;
    if (alias_mode && (flow != (number & 1u ? 0x83ffc9u : 0x83f700u) ||
                       result.flow != LUFIA2_EXECUTION_BOUNDARY ||
                       n.stack != (number % 4u < 2u ? 0x1f10u : 0x1ffcu))) {
        fputs("wrong rewritten near return\n", stderr);
        return 1;
    }
    for (steps = 0; steps < 200000u; ++steps) {
        if (SnesVerifyPc24(r) == flow)
            break;
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 200000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(stderr,
                "appearance%04X case%u alias%u steps%u flow%06X originalPC%06X CPU%d "
                "WRAM%d events%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X "
                "DB%02X/%02X M%u/%u Xf%u/%u C%u/%u N%u/%u Z%u/%u\n",
                handlers[suite], number, alias_mode, steps, flow, SnesVerifyPc24(r),
                memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, n.accumulator, r->a, n.x,
                r->x, n.y, r->y, n.stack, r->sp, n.data_bank, r->db,
                n.accumulator_is_8_bit, r->mf, n.index_is_8_bit, r->xf, n.carry, r->c,
                n.negative, r->n, n.zero, r->z);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 3 && argc != 4 && argc != 6) ||
        !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    unsigned selected_handler = 0;
    if ((argc == 4 || argc == 6) && strcmp(argv[2], "--suite") == 0) {
        selected_handler = (unsigned)strtoul(argv[3], NULL, 16);
    }
    if ((argc == 4 && !selected_handler) || argc == 6) {
        unsigned argument = argc == 6 ? 4u : 2u;
        uint32_t pc = (uint32_t)strtoul(argv[argument], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[argument + 1u], NULL, 16);
        if (rom[at] == value)
            return 2;
        rom[at] = value;
    }
    if (!SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *r = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
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

    unsigned total = 0;
    if (argc == 3 && strcmp(argv[2], "--aliases") == 0) {
        alias_mode = 1;
        suite = 1;
        for (number = 0; number < 8u; ++number) {
            if (run(r))
                return 1;
        }
        printf("PASS 8 actual rewritten near returns/%u boundaries\n", boundaries);
        return 0;
    }
    if (argc != 3)
        for (suite = 0; suite < 4u; ++suite) {
            if (selected_handler && selected_handler != handlers[suite])
                continue;
            unsigned count = 4096u;
#ifdef RUNTIME_ABI
            count = 1024u;
#endif
            for (number = 0; number < count; ++number) {
                if (run(r))
                    return 1;
                ++total;
            }
            printf("PASS appearance%04X:%u CPU/full-WRAM/bus ROM states\n",
                   handlers[suite], count);
            fflush(stdout);
        }
#ifndef RUNTIME_ABI
    if (!selected_handler) {
        unsigned guards = 0;
        for (suite = 0; suite < 4u; ++suite) {
            if (suite == 2u)
                continue;
            number = 1;
            Lufia2CpuState base_state;
            seed(&base_state);
            Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                              NULL};
            for (unsigned k = 0; k < 14u; ++k) {
                Lufia2CpuState n = base_state;
                if (k == 0u)
                    n.accumulator_is_8_bit = 0;
                else if (k == 1u)
                    n.index_is_8_bit = 1;
                else if (k == 2u)
                    n.program_bank = 0;
                else if (k == 3u)
                    n.decimal = 1;
                else if (k < 10u)
                    n.direct_page =
                        (uint16_t[]){1, 0xff, 0x100, 0xffff, 0xffc0, 0x1ecb}[k - 4u];
                else
                    n.stack = (uint16_t[]){0, 0x54, 0x1f0f, 0x1ffd}[k - 10u];
                Lufia2CpuState before = n;
                if (suite < 3u)
                    before.resume_pc = (0x830000u | handlers[suite]);
                Lufia2ExecutionResult q = execute(&m, &n);
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY ||
                    memcmp(&before, &n, sizeof n) || native_trace.count) {
                    fprintf(stderr, "guard suite%u context%u CPU%d events%u\n", suite,
                            k, memcmp(&before, &n, sizeof n), native_trace.count);
                    return 1;
                }
                ++guards;
            }
            if (suite == 0u)
                for (unsigned k = 0; k < 4u; ++k) {
                    Lufia2CpuState n = base_state;
                    n.x = (uint16_t[]){40u, 255u, 256u, 65535u}[k];
                    Lufia2CpuState before = n;
                    before.resume_pc = 0x83c110u;
                    Lufia2ExecutionResult q = execute(&m, &n);
                    if (q.flow != LUFIA2_EXECUTION_BOUNDARY ||
                        memcmp(&before, &n, sizeof n) || native_trace.count)
                        return 1;
                    ++guards;
                }
        }
        printf("PASS appearance pre-access guards:%u\n", guards);
    }
#else
    for (unsigned k = 0; k < 4u; ++k)
        if (runtime_fallback[k])
            return 1;
#endif

#ifdef RUNTIME_ABI
    if (!selected_handler) {
        unsigned guards = 0;
        for (suite = 0; suite < 2u; ++suite)
            for (unsigned k = 0; k < (suite == 0u ? 8u : 7u); ++k) {
                number = 1;
                Lufia2CpuState n;
                seed(&n);
                uint32_t entry = 0x830000u | handlers[suite];
                if (k == 0u)
                    n.accumulator_is_8_bit = 0;
                else if (k == 1u)
                    n.index_is_8_bit = 1;
                else if (k == 2u)
                    n.decimal = 1;
                else if (k == 3u)
                    n.program_bank = 0;
                else if (k == 4u)
                    n.stack = 0x1ffdu;
                else if (k == 5u)
                    n.stack = 0;
                else if (k == 6u)
                    n.direct_page = 0xffffu;
                else
                    n.x = 40u;
                Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                  NULL};
                RuntimeCase t = {0};
                t.memory = &m;
                t.entry = entry;
                t.start = n.stack;
                t.hrv = 3u;
                t.guard_only = 1u;
                runtime_case = &t;
                CpuState c;
                memset(&c, 0, sizeof c);
                runtime_load(&c, &n);
                c.host_return_valid = 3u;
                CpuState before = c;
                RecompReturn ret = bridges[suite](&c);
                before.open_bus = c.open_bus;
                runtime_return_reads = 0;
                if (ret != RECOMP_RETURN_LLE_UNWIND_BASE || t.kind != 3u ||
                    t.target != entry || t.observed || memcmp(&before, &c, sizeof c))
                    return 1;
                ++guards;
                runtime_case = NULL;
            }
        printf("PASS production appearance guards:%u\n", guards);
    }
#endif
    printf("PASS appearance batch:%u states/%u boundaries\n", total, boundaries);
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
