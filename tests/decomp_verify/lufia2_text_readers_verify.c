#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"
#include "lufia2/party.h"
#include "lufia2/field.h"
#include "lufia2/battle.h"
#include "lufia2/system.h"
#include "lufia2/text.h"
#include "system/gold_internal.h"
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;

static unsigned suite, number, vm_mode, vm_legacy, vm_bank, aliases;
static unsigned original_vm_visits, original_goto_visits, unexpected_children;
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_80C0B7(CpuState *);
extern RecompReturn Lufia2DecompBridge_80C0D0(CpuState *);
extern RecompReturn Lufia2DecompBridge_80C102(CpuState *);
extern RecompReturn Lufia2DecompBridge_9CB8(CpuState *);
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

static uint16_t romword(uint32_t address) {
    unsigned a = ((address >> 16) & 127u) * 32768u + (address & 32767u);
    return (uint16_t)(native.rom[a] | native.rom[a + 1u] << 8);
}

static void seed(Lufia2CpuState *n) {
    memset(n, 0, sizeof *n);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    n->program_bank = 0x80;
    n->data_bank = (uint8_t)(number >> 1);
    n->accumulator_is_8_bit = (uint8_t)(number & 1u);
    n->accumulator = (uint16_t)(number * 197u + 0x1234u);
    n->x = (uint16_t)(number * 83u);
    static const uint16_t pointers[] = {0,      1,      0x7ffe, 0x7fff, 0x8000, 0x8001,
                                        0x8fff, 0x9000, 0xc0b7, 0xc0d0, 0xfffd, 0xfffe,
                                        0xffff, 0x1f00, 0x1ffb, 0x2180};
    n->y = pointers[(number >> 9) & 15u];
    n->stack = (uint16_t[]){0x1f00, 0x1f10, 0x1ffc, 0x1f80}[(number >> 13) & 3u];
    unsigned flags = number >> 13;
    n->carry = flags & 1u;
    n->zero = (flags >> 1) & 1u;
    n->negative = (flags >> 2) & 1u;
    n->overflow = (number >> 6) & 1u;
    n->irq_disable = (number >> 7) & 1u;
    if (suite == 2u || suite == 3u || suite == 5u) {
        n->stack = (uint16_t[]){0x1f00, 0x1f10, 0x1fff, 0x1f80}[(number >> 13) & 3u];
        n->direct_page = (uint16_t)(flags * 0x1234u);
        n->decimal = flags & 1u;
    }
    if (suite >= 4u) {
        n->accumulator = (uint16_t)number;
        n->accumulator_is_8_bit = 0;
        n->decimal = 0;
    }
    native.wram[0x9b9u] = native.wram[0x109b9u] = n->data_bank;
    if (aliases) {
        n->data_bank = 0;
        n->stack = (uint16_t)(0x9b7u + ((number >> 1) & 3u));
        n->y = (uint16_t[]){0, 0x7fff, 0xffff}[(number >> 3) % 3u];
    }
    if (vm_mode) {
        n->data_bank = 0x7e;
        n->accumulator_is_8_bit = 1;
        n->stack = (uint16_t)(0x1f80u + (number % 64u));
        n->direct_page = 0;
        n->decimal = 0;
        n->y = (uint16_t)(number * 59u);
        if (number & 1u)
            SetIndexWidth(n, 1u);
        native.wram[0x99b] = native.wram[0x99c] = native.wram[0x1259] =
            native.wram[0x1d0ff] = 0;
        native.wram[0xb54] = 30u;
        native.wram[0x5b5] = 0;
        word(0x9b7u, 0x9000u);
        word(0x9a0u, 0x007eu);
        word(0x99eu, 0xa000u);
        native.wram[0x9b9u] = 0x7eu;
        unsigned op = 0;
        for (; op < 205; ++op)
            if (romword(0x80ca14u + 2u * op) == 0xa041u)
                break;
        if (op == 205u)
            exit(2);
        native.wram[0x9000] = (uint8_t)op;
        native.wram[0x9001] = (uint8_t)(number * 37u);
        native.wram[0x9002] = (uint8_t)number;
        for (unsigned i = 0; i < 256; ++i)
            word(0x7e9003u + i * 2u, 0x0020u);
        native.wram[0x79eu + native.wram[0x9001]] = (uint8_t)(number * 73u);
        native.wram[0xa020] = 0x37;
        native.wram[0xa021] = 0xff;

        if (vm_bank) {
            unsigned address = (number & 2u) ? 0xfff0u : 0xfffcu;
            word(0x9b7u, (uint16_t)address);
            native.wram[address] = (uint8_t)op;
            native.wram[address + 1u] = 0;
            native.wram[address + 2u] = (number & 2u) ? 0u : 1u;
            native.wram[0x79eu] = (number & 2u) ? 0xffu : 0u;
            word(0x7f099eu, 0xa000u);
            word(0x7f09a0u, 0x007fu);
            native.wram[0x109b9u] = 0x7fu;
            native.wram[0x1099bu] = native.wram[0x1099cu] = 0;
            native.wram[0x11254u] = native.wram[0x11259u] = 0;
            if (number & 2u)
                word(0x7f81f1u, 0x0020u);
            else {
                native.wram[0xffffu] = 0x20u;
                native.wram[0x10000u] = 0;
                word(0x7f8000u, 0x1234u);
            }
            native.wram[0x1a020u] = 0;
        }
    } else if (suite < 2u || suite == 4u) {
        word((uint16_t)(n->stack + 1u), 0x9234u);
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
    original_vm_visits = original_goto_visits = 0;
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

static uint8_t no_child(void *context, Lufia2CpuState *n, uint32_t target,
                        uint32_t site, uint8_t frame) {
    (void)context;
    (void)n;
    (void)target;
    (void)frame;
    fprintf(stderr, "Unexpected VM child:%06X\n", site);
    ++unexpected_children;
    return 0;
}
#ifdef RUNTIME_ABI
void RuntimeSaveProbeVisits(void) {}
void RuntimeResetProbeVisits(void) {}
void RuntimeRestoreProbeVisits(void) {}
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    (void)b;
    (void)n;
    (void)frame;
    fprintf(stderr, "Unexpected probe child%06X\n", site);
    ++unexpected_children;
}
#endif
static int run(Interp816 *r) {
    Lufia2CpuState n;
    seed(&n);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    uint32_t entry = vm_mode       ? 0x809cb8u
                     : suite == 4u ? 0x80c102u
                     : suite == 5u ? 0x80a3d7u
                     : suite == 0u ? 0x80c0b7u
                     : suite == 1u ? 0x80c0d0u
                     : suite == 2u ? 0x80c0d0u
                                   : 0x80a3c6u;
    load(r, &n, entry);
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    if (vm_mode)
        q = runtime_invoke(&m, &n, no_child, &m, Lufia2DecompBridge_9CB8,
                           Lufia2TextEngineStep, NULL, entry, 2u, 3u);
    else
        q = runtime_invoke(&m, &n, NULL, NULL,
                           suite == 4u ? Lufia2DecompBridge_80C102
                           : suite     ? Lufia2DecompBridge_80C0D0
                                       : Lufia2DecompBridge_80C0B7,
                           suite == 4u ? Lufia2SceneScriptSeekRelative
                           : suite     ? Lufia2SceneScriptReadWord
                                       : Lufia2SceneScriptReadOperand,
                           NULL, entry, suite == 4u ? 3u : suite, 2u);
#else
    if (vm_mode)
        q = vm_legacy ? Lufia2TextEngineStep(&m, &n)
                      : Lufia2TextEngineStepWithServices(&m, &n, no_child, &m);
    else if (suite == 4u)
        q = Lufia2SceneScriptSeekRelative(&m, &n);
    else if (suite == 5u) {
        Lufia2TextSetScriptPointer(&m, &n, 0xa3d9u);
        q = ExecutionReturned(0x80a3dau);
    } else if (suite == 0u)
        q = Lufia2SceneScriptReadOperand(&m, &n);
    else if (suite == 1u)
        q = Lufia2SceneScriptReadWord(&m, &n);
    else if (suite == 2u) {
        Lufia2TextNextByte(&m, &n, 0xc0d2u);
        q = ExecutionReturned(0x80c0d3u);
    } else {
        Lufia2TextNextWord(&m, &n, 0xa3c8u);
        q = ExecutionReturned(0x80a3c9u);
    }
#endif
    if (unexpected_children)
        return 1;
    if (vm_mode &&
        ((vm_legacy && q.pc != 0x80a3c6u) || (!vm_legacy && q.pc == 0x80a3c6u)))
        return 1;
    unsigned steps;
    for (steps = 0; steps < 20000u; ++steps) {
        unsigned pc = SnesVerifyPc24(r);
        if (pc == 0x80a041u)
            ++original_vm_visits;
        if (pc == 0x80a3c6u)
            ++original_goto_visits;
        if (pc == q.pc && (!vm_mode || original_goto_visits))
            break;
        instruction(&original, r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 20000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        (vm_mode && (!original_vm_visits || !original_goto_visits))) {
        fprintf(
            stderr,
            "reader%u case%u VM%u legacy%u alias%u steps%u PC%06X/%06X CPU%d WRAM%d traces%u/%u A%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X M%u/%u X%u/%u\n",
            suite, number, vm_mode, vm_legacy, aliases, steps, q.pc, SnesVerifyPc24(r),
            memcmp(&expected, &n, sizeof n),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, n.accumulator, r->a, n.y, r->y,
            n.stack, r->sp, n.data_bank, r->db, n.accumulator_is_8_bit, r->mf,
            n.index_is_8_bit, r->xf);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int VerifyReaderGuards(int argc, char **argv);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifyReaderGuards(2, argv);
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    unsigned first_suite = 0;
    if (argc == 3 && strcmp(argv[2], "--fault-byte") == 0)
        rom[0x40b8u] ^= 1u;
    if (argc == 3 && strcmp(argv[2], "--fault-word") == 0) {
        rom[0x40d3u] = 0xeau;
        first_suite = 1;
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

    if (argc == 3 && strcmp(argv[2], "--vm-bank") == 0) {
        vm_mode = vm_bank = 1;
        for (number = 0; number < 64u; ++number)
            if (run(r))
                return 1;
#ifdef RUNTIME_ABI
        if (runtime_fallback[2] || runtime_native[2] != 64u)
            return 1;
#endif
        puts(
            "PASS reached A041 across WRAM bank edge:64 original ROM/CPU/WRAM/bus states");
        return 0;
    }
    if (argc == 3 &&
        (strcmp(argv[2], "--vm") == 0 || strcmp(argv[2], "--vm-legacy") == 0)) {
        vm_mode = 1;
        vm_legacy = strcmp(argv[2], "--vm-legacy") == 0;
#ifdef RUNTIME_ABI
        if (vm_legacy)
            return 2;
#endif
        for (number = 0; number < 4096u; ++number)
            if (run(r))
                return 1;
#ifdef RUNTIME_ABI
        if (runtime_fallback[2] || runtime_native[2] != 4096u)
            return 1;
#endif
        printf("PASS reached A041 VM+goto:%u states, legacy%u\n", number, vm_legacy);
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--seek") == 0) {
        unsigned limit = 6u;
#ifdef RUNTIME_ABI
        limit = 5u;
#endif
        for (suite = 4u; suite < limit; ++suite) {
            for (number = 0; number < 65536u; ++number)
                if (run(r))
                    return 1;
            printf(
                "PASS pointer reader%u:65536 originalROM CPU/fullWRAM/orderedbus states\n",
                suite);
        }
#ifdef RUNTIME_ABI
        if (runtime_fallback[3] || runtime_native[3] != 65536u)
            return 1;
#endif
        return 0;
    }
    unsigned total = 0, limit = 4u;
#ifdef RUNTIME_ABI
    limit = 2u;
#endif
    for (suite = first_suite; suite < limit; ++suite) {
        unsigned count = suite < 2u ? 65536u : 32768u;
        for (number = 0; number < count; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        printf("PASS reader%u:%u originalROM CPU/fullWRAM/orderedbus states\n", suite,
               count);
        fflush(stdout);
    }
    aliases = 1;
    suite = 0;
    for (number = 0; number < 24u; ++number)
        if (run(r))
            return 1;
#ifdef RUNTIME_ABI
    if (runtime_fallback[0] || runtime_fallback[1])
        return 1;
#endif
    printf("PASS reader batch:%u states +24 intentional bank/return-frame aliases\n",
           total);
    return 0;
}

static unsigned guards;
static int guard_one(const Lufia2CpuState *input, unsigned emulation) {
    Lufia2CpuState n = *input;
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    RuntimeCase t = {0};
    t.memory = &m;
    t.entry = 0x80c0d0u;
    t.start = n.stack;
    t.hrv = (unsigned[]){0, 2, 3}[guards % 3u];
    t.guard_only = 1u;
    runtime_case = &t;
    CpuState c;
    memset(&c, 0, sizeof c);
    runtime_load(&c, &n);
    c.host_return_valid = (uint8)t.hrv;
    c.open_bus = 0xa5u;
    c.emulation = (uint8)emulation;
    cpu_mirrors_to_p(&c);
    CpuState expected = c;
    RecompReturn ret = Lufia2DecompBridge_80C0D0(&c);
    runtime_return_reads = 0;
    if (ret != RECOMP_RETURN_LLE_UNWIND_BASE || t.stub != 1u || t.kind != 3u ||
        t.target != t.entry || memcmp(&c, &expected, sizeof c) || native_trace.count) {
        fprintf(stderr, "Consumer guard%u M%uX%u S%04X DP%04X PB%02X E%u traces%u\n",
                guards, n.accumulator_is_8_bit, n.index_is_8_bit, n.stack,
                n.direct_page, n.program_bank, emulation, native_trace.count);
        return 1;
    }
#else
    (void)emulation;
    Lufia2CpuState before = n;
    before.resume_pc = 0x80c0d0u;
    Lufia2ExecutionResult q = Lufia2SceneScriptReadWord(&m, &n);
    if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
        memcmp(&before, &n, sizeof n) || native_trace.count)
        return 1;
#endif
    ++guards;
    return 0;
}
static int VerifyReaderGuards(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc != 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Lufia2CpuState valid = {0};
    valid.program_bank = 0x80;
    valid.data_bank = 0x7e;
    valid.stack = 0x1f80;
    valid.y = 0x9000;
    valid.accumulator = 0x1234;
    valid.x = 0x5678;
    for (unsigned m = 0; m < 2u; ++m) {
        valid.accumulator_is_8_bit = (uint8_t)m;
        for (unsigned pb = 0; pb < 256u; ++pb) {
            if (pb == 0x80u)
                continue;
            Lufia2CpuState n = valid;
            n.program_bank = (uint8_t)pb;
            if (guard_one(&n, 0u))
                return 1;
        }
        for (unsigned dp = 1; dp < 65536u; ++dp) {
            Lufia2CpuState n = valid;
            n.direct_page = (uint16_t)dp;
            if (guard_one(&n, 0u))
                return 1;
        }
        for (unsigned sp = 0; sp < 65536u; ++sp) {
            if (sp >= 0x1f00u && sp <= 0x1ffcu)
                continue;
            Lufia2CpuState n = valid;
            n.stack = (uint16_t)sp;
            if (guard_one(&n, 0u))
                return 1;
        }
        Lufia2CpuState n = valid;
        n.decimal = 1;
        if (guard_one(&n, 0u))
            return 1;
        n = valid;
        SetIndexWidth(&n, 1u);
        if (guard_one(&n, 0u))
            return 1;
    }
#ifdef RUNTIME_ABI
    valid.accumulator_is_8_bit = 1;
    SetIndexWidth(&valid, 1);
    valid.stack = 0x180;
    if (guard_one(&valid, 1))
        return 1;
#endif
    printf("PASS word-reader pre-access guards:%u\n", guards);
    return 0;
}
