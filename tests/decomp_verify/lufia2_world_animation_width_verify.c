#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/world_map.h"
#include "core/cpu_ops.h"
typedef struct Trace {
    SnesVerifyBusEvent events[4096];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left, number;
static uint32_t fetch_address;
static void fault(unsigned line) {
    fprintf(stderr, "animation invariant line%u case%u\n", line, number);
    exit(1);
}
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
    if (trace->count == 4096u) {
        fprintf(stderr, "trace ceiling case%u side%s fetch%06X\n", number,
                trace == &original_trace ? "original" : "native", fetch_address);
        fault(__LINE__);
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
static void instruction(SnesVerifyBus *b, Interp816 *c) {
    Trace *t = b == &native ? &native_trace : &original_trace;
    uint32_t pc = SnesVerifyPc24(c);
    uint8_t op = SnesVerifyBusRead(b, pc);
    if (t)
        --t->count;
    fetch_trace = t;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)c->mf * 2u + (unsigned)c->xf][op];
    if (!fetch_left) {
        fprintf(stderr, "unknown instruction%06X\n", pc);
        exit(1);
    }
    interp816_runOpcode(c);
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

#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_86E0B9(CpuState *);
extern RecompReturn Lufia2DecompBridge_86E11F(CpuState *);
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t s, uint8_t f) {
    (void)b;
    (void)n;
    (void)s;
    (void)f;
    volatile unsigned unexpected = 1u;
    if (unexpected)
        fault(__LINE__);
}
void RuntimeSaveProbeVisits(void) {}
void RuntimeRestoreProbeVisits(void) {}
void RuntimeResetProbeVisits(void) {}
#endif
static void seed(Lufia2CpuState *n) {
    for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
        native.wram[i] = (uint8_t)((i * 17u + number * 137u) >> 4);
    memset(n, 0, sizeof *n);
    n->accumulator_is_8_bit = (uint8_t)(number & 1u);
    n->program_bank = 0x86;
    n->data_bank = 0x86;
    n->accumulator = (uint16_t)((number / 2u) * 197u);
    n->x = (uint16_t)(0x1469u + (number / 512u % 22u) * 29u);
    n->y = (uint16_t)(number * 31u);
    n->stack = (uint16_t)(0x1f20u + (number % 96u));
    n->direct_page = (uint16_t[]){0, 1, 0x100, 0x7f00}[(number / 1024u) % 4u];
    n->carry = (number >> 1) & 1u;
    n->zero = (number >> 2) & 1u;
    n->negative = (number >> 3) & 1u;
    n->overflow = (number >> 4) & 1u;
    n->irq_disable = (number >> 5) & 1u;
    n->decimal = (number >> 6) & 1u;
    native.wram[(uint16_t)(n->direct_page + n->x + 15u)] = (uint8_t)(number / 32u % 8u);
    native.wram[(uint16_t)(n->stack + 1u)] = 0x22;
    native.wram[(uint16_t)(n->stack + 2u)] = 0x91;
}
static int run(Interp816 *c) {
    Lufia2CpuState n;
    seed(&n);
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(&original);
    SnesVerifyBusResetTrace(&native);
    original_trace.count = native_trace.count = 0;
    load(c, &n, 0x86e0b9u);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    Lufia2ExecutionResult q =
        runtime_invoke(&m, &n, NULL, NULL, Lufia2DecompBridge_86E0B9,
                       Lufia2WorldMapStartAnimation, NULL, 0x86e0b9u, 0, 2);
#else
    Lufia2ExecutionResult q = Lufia2WorldMapStartAnimation(&m, &n);
#endif
    unsigned steps = 0;
    while (SnesVerifyPc24(c) != 0x86e11au && steps < 128u) {
        instruction(&original, c);
        ++steps;
    }
    Lufia2CpuState expected = n;
    store(&expected, c);
    if (steps == 128u || q.flow != LUFIA2_EXECUTION_RETURNED || q.pc != 0x86e11au ||
        memcmp(&n, &expected, sizeof n) ||
        memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(stderr,
                "animation case%u M%u steps%u CPU%d WRAM%d bus%u/%u A%04X/%04X "
                "S%04X/%04X\n",
                number, number & 1u, steps, memcmp(&n, &expected, sizeof n),
                memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE),
                native_trace.count, original_trace.count, n.accumulator, c->a, n.stack,
                c->sp);
        return 1;
    }
    return 0;
}
#ifdef RUNTIME_ABI
static void bridge_guard(unsigned kind, unsigned width, unsigned hrv) {
    Lufia2CpuState n;
    number = width;
    seed(&n);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    RuntimeCase t = {0};
    t.memory = &m;
    t.start = n.stack;
    t.entry = kind == 4u ? 0x86e11fu : 0x86e0b9u;
    t.hrv = hrv;
    t.guard_only = 1;
    runtime_case = &t;
    CpuState c = {0};
    runtime_load(&c, &n);
    c.host_return_valid = (uint8_t)hrv;
    switch (kind) {
    case 0:
        c.emulation = 1;
        break;
    case 1:
        c.PB = 0x85;
        break;
    case 2:
        c.S = 0x1ffd;
        t.start = c.S;
        break;
    case 3:
        c.x_flag = 1;
        break;
    case 4:
        c.m_flag = 0;
        break;
    }
    cpu_mirrors_to_p(&c);
    CpuState before = c;
    native_trace.count = 0;
    RecompReturn q =
        kind == 4u ? Lufia2DecompBridge_86E11F(&c) : Lufia2DecompBridge_86E0B9(&c);
    runtime_return_reads = 0;
    if (q != RECOMP_RETURN_LLE_UNWIND_BASE || memcmp(&before, &c, sizeof c) ||
        native_trace.count || t.stub != 1u || t.kind != 3u || t.target != t.entry)
        fault(__LINE__);
    runtime_case = NULL;
}
#endif
static int guards(void) {
#ifdef RUNTIME_ABI
    for (unsigned k = 0; k < 5u; ++k)
        for (unsigned m = 0; m < 2u; ++m)
            for (unsigned h = 0; h < 4u; ++h)
                bridge_guard(k, m, h);
    printf("40 ABI guards PASS; ");
#endif

    for (unsigned m = 0; m < 2u; ++m) {
        Lufia2CpuState n;
        number = m;
        seed(&n);
        n.index_is_8_bit = 1;
        Lufia2CpuState before = n;
        Lufia2Memory mem = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
        native_trace.count = 0;
        Lufia2ExecutionResult q = Lufia2WorldMapStartAnimation(&mem, &n);
        before.resume_pc = 0x86e0b9;
        if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != 0x86e0b9u ||
            memcmp(&n, &before, sizeof n) || native_trace.count)
            fault(__LINE__);
    }
    printf("2 semantic guards PASS\n");
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size))
        return 2;
    if (argc == 5) {
        unsigned at = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned off = ((at >> 16) & 127u) * 32768u + (at & 32767u);
        if (off >= size)
            return 2;
        rom[off] = (uint8_t)strtoul(argv[4], NULL, 16);
    }
    if (!SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    for (unsigned m = 0; m < 2u; ++m)
        for (unsigned x = 0; x < 2u; ++x) {
            uint8_t *l = lengths[m * 2u + x];
            l[0x95] = l[0x85] = l[0xa5] = l[0xb5] = l[0xb1] = l[0xb4] = l[0x74] =
                l[0x80] = l[0x10] = l[0x30] = l[0xe2] = l[0xc2] = 2;
            l[0x29] = l[0xa9] = (uint8_t)(m ? 2 : 3);
            l[0xb9] = l[0xbd] = 3;
            l[0x0a] = l[0xa8] = l[0xda] = l[0xfa] = l[0xbb] = 1;
        }
    if (argc >= 3 && strcmp(argv[2], "--guards") == 0)
        return guards();
    Interp816 *c = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!c)
        return 2;
    unsigned count = 16384;
    for (number = 0; number < count; ++number)
        if (run(c))
            return 1;
#ifdef RUNTIME_ABI
    if (runtime_native[0] != count || runtime_fallback[0])
        fault(__LINE__);
#endif
    printf("Animation entry widths PASS %u cases\n", count);
    return 0;
}
