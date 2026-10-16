#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/world_map.h"
#include "lufia2/system.h"
#include "core/cpu_ops.h"
static void fault(unsigned line) {
    fprintf(stderr, "world fixture invariant line%u\n", line);
    exit(1);
}
typedef struct Trace {
    SnesVerifyBusEvent events[2000000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, unwind_at, bad_at, bad_kind, native_calls,
    original_calls;
typedef Lufia2ExecutionResult (*Service)(const Lufia2Memory *, Lufia2CpuState *,
                                         Lufia2PushedChildCall, void *);
typedef struct ChildVisit {
    uint32_t target, site;
    uint8_t frame;
    Lufia2CpuState cpu;
} ChildVisit;
static ChildVisit native_visits[128], original_visits[128];
static unsigned composition;
static unsigned invalid_routing;
static unsigned guarding;
static unsigned effective_unwinds, effective_returns;
static int probing(void) {
#ifdef RUNTIME_ABI
    return runtime_probe_desired_site != 0u;
#else
    return 0;
#endif
}

static Lufia2ExecutionResult ServiceB7CF(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapStopWindowSparkEmission(m, c);
}
static Lufia2ExecutionResult ServiceB7D3(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapBeginWindowColorPulse(m, c);
}
static Lufia2ExecutionResult ServiceB7D5(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapRestartWindowColorPulse(m, c);
}
static const uint32_t entries[] = {
    0x86b454u, 0x86b47eu, 0x86b4beu, 0x86b4f2u, 0x86b508u, 0x86b748u, 0x86b7cfu,
    0x86b7d3u, 0x86b7d5u, 0x86b7dfu, 0x86b7f7u, 0x86b7feu, 0x86b350u, 0x86b1deu};
static const uint32_t terminals[] = {0x86b47d, 0x86b4bd, 0x86b4f1, 0x86b507, 0x86b51b,
                                     0x86b7ce, 0x86b7d2, 0x86b7de, 0x86b7de, 0x86b7f6,
                                     0x86b7f6, 0x86b7f6, 0x86b39b, 0x86b24b};
static int valid_terminal(uint32_t pc) {
    return pc == terminals[suite] || (suite == 0u && pc == 0x86b45eu);
}
static const Service services[] = {Lufia2WorldMapGrowSceneWindow,
                                   Lufia2WorldMapRotateSceneWindow,
                                   Lufia2WorldMapContractSceneWindow,
                                   Lufia2WorldMapSubmitPhasedSceneWindow,
                                   Lufia2WorldMapContractSceneCircle,
                                   Lufia2WorldMapStepLayeredSceneWindows,
                                   ServiceB7CF,
                                   ServiceB7D3,
                                   ServiceB7D5,
                                   Lufia2WorldMapLoadLayeredSceneWindow0,
                                   Lufia2WorldMapLoadLayeredSceneWindow1,
                                   Lufia2WorldMapLoadLayeredSceneWindow2,
                                   Lufia2WorldMapCycleScenePaletteFade,
                                   Lufia2WorldMapStepSceneWindowAnimations};
static const unsigned case_counts[] = {4096, 4096, 4096, 2048, 2048, 4096, 1024,
                                       2048, 4096, 1024, 1024, 1024, 512,  2048};
static int narrow(void) { return 1; }
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_86B454(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B47E(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B4BE(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B4F2(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B508(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B748(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B7CF(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B7D3(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B7D5(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B7DF(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B7F7(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B7FE(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B350(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B1DE(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_86B454, Lufia2DecompBridge_86B47E, Lufia2DecompBridge_86B4BE,
    Lufia2DecompBridge_86B4F2, Lufia2DecompBridge_86B508, Lufia2DecompBridge_86B748,
    Lufia2DecompBridge_86B7CF, Lufia2DecompBridge_86B7D3, Lufia2DecompBridge_86B7D5,
    Lufia2DecompBridge_86B7DF, Lufia2DecompBridge_86B7F7, Lufia2DecompBridge_86B7FE,
    Lufia2DecompBridge_86B350, Lufia2DecompBridge_86B1DE};
static const RuntimeLeaf leaves[] = {NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     Lufia2WorldMapStopWindowSparkEmission,
                                     Lufia2WorldMapBeginWindowColorPulse,
                                     Lufia2WorldMapRestartWindowColorPulse,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL};
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
    if (trace->count == 2000000u) {
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

static Trace *trace_for(SnesVerifyBus *b) {
    return b == &native ? &native_trace : b == &original ? &original_trace : NULL;
}
static void model_child(const Lufia2Memory *, Lufia2CpuState *, uint32_t, uint32_t);
static void instruction(SnesVerifyBus *b, Interp816 *c) {
    if ((SnesVerifyPc24(c) == 0x86d378u || SnesVerifyPc24(c) == 0x86974fu)) {
        Lufia2CpuState n = {0};
        store(&n, c);
        uint16_t back = (uint16_t)(b->wram[(uint16_t)(c->sp + 1u)] |
                                   (uint16_t)b->wram[(uint16_t)(c->sp + 2u)] << 8);
        uint32_t site = 0x860000u | (uint16_t)(back - 2u);
        Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, b, NULL, NULL};
        model_child(&m, &n, SnesVerifyPc24(c), site);
        load(c, &n, site + 3u);
        return;
    }
    uint32_t pc = SnesVerifyPc24(c);
    Trace *t = trace_for(b);
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

#ifdef RUNTIME_ABI
static unsigned frame_size(void) { return 2u; }
#endif
static const uint32_t call_sites[14][5] = {
    {0x86b47a},
    {0x86b4ba},
    {0x86b4ee},
    {0x86b504},
    {0x86b518},
    {0x86b763, 0x86b768, 0x86b786, 0x86b7c4, 0x86b7cb},
    {0},
    {0},
    {0},
    {0x86b7e4, 0x86b7f3},
    {0x86b7e4, 0x86b7f3},
    {0x86b7e4, 0x86b7f3},
    {0x86b360, 0x86b38b, 0x86b38e, 0x86b391, 0x86b394},
    {0x86b1e4, 0x86b1f1, 0x86b1fe, 0x86b20b, 0x86b248}};
static int is_leaf(void) { return suite == 6u || suite == 7u || suite == 8u; }
static uint8_t child_frame(uint32_t site) {
    return site == 0x86c311u || site == 0x86c526u ? 3u : 2u;
}
static int allowed_site(uint32_t site) {
    for (unsigned i = 0; i < 5u; ++i)
        if (call_sites[suite][i] && call_sites[suite][i] == site)
            return 1;
    return 0;
}
static uint32_t child_target(const SnesVerifyBus *b, uint32_t site, uint16_t index) {
    if (site == 0x86b907u)
        return 0x860000u | b->wram[0x122eu] | (uint32_t)b->wram[0x122fu] << 8;
    unsigned at = ((site >> 16) & 127u) * 32768u + (site & 32767u);
    uint16_t encoded = (uint16_t)(b->rom[at + 1u] | b->rom[at + 2u] << 8);
    if (b->rom[at] == 0xfcu) {
        uint16_t pointer = (uint16_t)(encoded + index);
        unsigned lo = 3u * 65536u + (pointer & 32767u);
        unsigned hi = 3u * 65536u + ((uint16_t)(pointer + 1u) & 32767u);
        return 0x860000u | b->rom[lo] | (uint32_t)b->rom[hi] << 8;
    }
    return encoded |
           (child_frame(site) == 3u ? (uint32_t)b->rom[at + 3u] << 16 : 0x860000u);
}

static int known_handler(uint32_t target) {
    return target == 0x86b454u || target == 0x86b47eu || target == 0x86b4beu ||
           target == 0x86b4f2u || target == 0x86b508u;
}
static int actual_site(uint32_t site, uint32_t target) {
    if (!composition || target == 0x86d378u || target == 0x86974fu)
        return 0;
    return (site != 0x86b1f1u && site != 0x86b20bu) || known_handler(target);
}
static void seed(Lufia2CpuState *n) {
    for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
        native.wram[i] = (uint8_t)((i * 23u + number * 47u + suite * 79u) >> 3);
    memset(n, 0, sizeof *n);
    memset(native_visits, 0, sizeof native_visits);
    memset(original_visits, 0, sizeof original_visits);
    native_calls = original_calls = 0;
    n->program_bank = 0x86;
    n->data_bank = 0x86;
    n->accumulator_is_8_bit = 1;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(0x1e68u + (number % 6u) * 15u);
    n->y = n->x;
    n->stack = (uint16_t)(0x1f60u + (number & 63u));
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 6) & 1u;
    word(0, (uint16_t)(number * 109u));
    word(4, (uint16_t)(number * 991u));
    word(6, (uint16_t)(number * 433u));
    native.wram[0x63] = (uint8_t)(number % 6u);
    for (unsigned i = 0; i < 6u; ++i) {
        unsigned at = 0x1e68u + i * 15u;
        word(at, 128);
        word(at + 2u, 112);
        native.wram[at + 4u] = (uint8_t)(number + i * 32u);
        native.wram[at + 5u] =
            (uint8_t[]){0, 1, 2, 31, 32, 33, 128, 255}[(number / 4u + i) % 8u];
        word(at + 6u, (uint16_t)(number * 127u));
        native.wram[at + 8u] =
            (uint8_t[]){0, 1, 7, 8, 9, 127, 128, 255}[(number / 8u + i) % 8u];
        native.wram[at + 9u] = 0;
        native.wram[at + 10u] =
            (uint8_t[]){0, 1, 7, 8, 9, 127, 128, 255}[(number / 4u + i) % 8u];
        native.wram[at + 11u] = 0;
        native.wram[at + 12u] = (uint8_t)((number + i) % 5u);
        native.wram[at + 13u] =
            (uint8_t[]){0, 1, 7, 8, 11, 12, 13, 19, 20, 23, 24, 27, 28, 63, 64, 255}
                [(number / 16u + i) % 16u];
        native.wram[at + 14u] =
            (uint8_t[]){0, 1, 23, 24, 25, 27, 28, 255}[(number / 32u + i) % 8u];
    }
    for (unsigned i = 0; i < 3u; ++i) {
        unsigned at = 0x1e50u + i * 6u;
        native.wram[at] = 128;
        native.wram[at + 1u] = 112;
        native.wram[at + 2u] =
            (uint8_t[]){0, 1, 7, 8, 127, 128, 254, 255}[(number / 8u + i) % 8u];
        native.wram[at + 3u] = 0;
        native.wram[at + 4u] = (uint8_t)((number + i) & 1u);
        native.wram[at + 5u] =
            (uint8_t[]){0, 1, 7, 8, 127, 128, 254, 255}[(number / 32u + i) % 8u];
    }
    native.wram[0x1226] = (uint8_t)number;
    native.wram[0x1228] = (uint8_t)(number % 3u);
    native.wram[0x122a] = (uint8_t)number;
    native.wram[0x122b] =
        (uint8_t[]){0, 1, 2, 9, 10, 127, 128, 255}[(number >> 4) % 8u];
    word(0x122c, 0);
    for (unsigned i = 0; i < 3u; ++i)
        word(0x1534u + i * 0x1du + 5u, 0xffff);
    native.wram[0x11db] = 0;
    native.wram[0x11dc] = 0;
    if (suite == 0u)
        native.wram[n->y + 6u] = (uint8_t)number;
    if (suite == 4u)
        n->y = (uint16_t)(0x1e50u + (number % 3u) * 6u);
    if (suite >= 9u && suite <= 11u)
        n->y = 0xb805;
    if (unwind_at || bad_at || probing()) {
        native.wram[0x122a] = 1;
        native.wram[0x122b] = 1;
        native.wram[0x1228] = (uint8_t)(number & 1u);
        native.wram[0x63] = (uint8_t)(number % 6u);
        for (unsigned i = 0; i < 6u; ++i) {
            unsigned at = 0x1e68u + i * 15u;
            native.wram[at + 6u] = 0;
            native.wram[at + 8u] = 0;
            native.wram[at + 10u] = 16;
            native.wram[at + 12u] = 1;
            native.wram[at + 13u] = 0;
            native.wram[at + 14u] = 0;
        }
        for (unsigned i = 0; i < 3u; ++i)
            native.wram[0x1e50u + i * 6u + 4u] = 1;
        if (suite == 3u)
            native.wram[0x1228] = (uint8_t)((number % 6u) % 3u);
    }
#ifdef RUNTIME_ABI
    if (probing() && suite == 5u)
        native.wram[0x1228] = runtime_probe_desired_site == 0x86b768u ? 0u : 1u;
#endif
    if (suite == 1u && !unwind_at && !bad_at && !probing() && !guarding) {
        native.wram[n->y + 5u] =
            (uint8_t[]){0, 1, 2, 30, 31, 32, 33, 128, 255}[(number / 4u) % 9u];
        native.wram[n->y + 13u] = (uint8_t[]){
            0,  1,  7,  8,  10, 11, 12, 13, 19, 20,  23,  24,
            27, 28, 62, 63, 64, 65, 75, 76, 77, 127, 254, 255}[(number / 16u) % 24u];
    }
    if (invalid_routing && suite == 13u)
        native.wram[0x1e50u + 6u + 4u] = 2;
    word((uint32_t)n->stack + 1u, 0x8123);
    native.wram[n->stack + 3u] = 0x83;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
    composition =
        number < 32u && !unwind_at && !bad_at && !probing() && !invalid_routing;
}
static void corrupt(Lufia2CpuState *n, unsigned ordinal) {
    if (ordinal != bad_at)
        return;
    if (bad_kind == 0)
        n->accumulator_is_8_bit = 0;
    else if (bad_kind == 1)
        SetIndexWidth(n, 1);
    else if (bad_kind == 2)
        n->direct_page = 0x100;
    else if (bad_kind == 3)
        n->decimal = 1;
    else
        n->data_bank = n->data_bank == 0x7f ? 0x7e : 0x7f;
}

static void pop_near(const Lufia2Memory *m, Lufia2CpuState *n, uint32_t site) {
    uint8_t frame = child_frame(site);
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(site + frame))
        fault(__LINE__);
    if (frame == 3u && Pull8(m, n) != 0x86u)
        fault(__LINE__);
    n->program_bank = 0x86;
}
static void execute_original_child(const Lufia2Memory *m, Lufia2CpuState *n,
                                   uint32_t target, uint32_t site) {
    Interp816 *c = interp816_init(m->context, SnesVerifyBusRead, SnesVerifyBusWrite);
    load(c, n, target);
    uint16_t end_stack = (uint16_t)(n->stack + child_frame(site));
    unsigned steps = 0;
    while (c->sp != end_stack || SnesVerifyPc24(c) != site + child_frame(site) + 1u) {
        if (++steps > 2000000u) {
            fprintf(stderr, "child limit suite%u case%u site%06X PC%06X\n", suite,
                    number, site, SnesVerifyPc24(c));
            exit(1);
        }
        instruction(m->context, c);
    }
    store(n, c);
    interp816_free(c);
}

static uint8_t nested_child(void *, Lufia2CpuState *, uint32_t, uint32_t, uint8_t);
static void model_child(const Lufia2Memory *m, Lufia2CpuState *n, uint32_t target,
                        uint32_t site) {
    if (composition && target != 0x86d378u && target != 0x86974fu &&
        ((site != 0x86b1f1u && site != 0x86b20bu) || known_handler(target))) {
        Lufia2ExecutionResult q = ExecutionReturned(0u);
        int canonical = 1;
        switch (target) {
        case 0x86b454:
            q = Lufia2WorldMapGrowSceneWindow(m, n, nested_child, (void *)m);
            break;
        case 0x86b47e:
            q = Lufia2WorldMapRotateSceneWindow(m, n, nested_child, (void *)m);
            break;
        case 0x86b4be:
            q = Lufia2WorldMapContractSceneWindow(m, n, nested_child, (void *)m);
            break;
        case 0x86b4f2:
            q = Lufia2WorldMapSubmitPhasedSceneWindow(m, n, nested_child, (void *)m);
            break;
        case 0x86b508:
            q = Lufia2WorldMapContractSceneCircle(m, n, nested_child, (void *)m);
            break;
        case 0x86b748:
            q = Lufia2WorldMapStepLayeredSceneWindows(m, n, nested_child, (void *)m);
            break;
        case 0x86b7cf:
            q = Lufia2WorldMapStopWindowSparkEmission(m, n);
            break;
        case 0x86b7d3:
            q = Lufia2WorldMapBeginWindowColorPulse(m, n);
            break;
        case 0x86b7d5:
            q = Lufia2WorldMapRestartWindowColorPulse(m, n);
            break;
        case 0x86b7df:
            q = Lufia2WorldMapLoadLayeredSceneWindow0(m, n, nested_child, (void *)m);
            break;
        case 0x86b7f7:
            q = Lufia2WorldMapLoadLayeredSceneWindow1(m, n, nested_child, (void *)m);
            break;
        case 0x86b7fe:
            q = Lufia2WorldMapLoadLayeredSceneWindow2(m, n, nested_child, (void *)m);
            break;
        case 0x86b350:
            q = Lufia2WorldMapCycleScenePaletteFade(m, n, nested_child, (void *)m);
            break;
        case 0x86b1de:
            q = Lufia2WorldMapStepSceneWindowAnimations(m, n, nested_child, (void *)m);
            break;

        default:
            canonical = 0;
            break;
        }
        if (canonical) {
            if (q.flow != LUFIA2_EXECUTION_RETURNED)
                fault(__LINE__);
            pop_near(m, n, site);
            return;
        }
        execute_original_child(m, n, target, site);
        return;
    }
    if (target == 0x86c536u || target == 0x86c321u || target == 0x86c54bu) {
        execute_original_child(m, n, target, site);
        return;
    }
    OpSta(m, n, 0x7ff100u + (site & 31u));
    OpSepWidths(n, 0x20u);
    OpLdy(n, (uint16_t)(number * 17u));
    OpLoadA(n, (uint16_t)(number * 37u));
    n->carry = (number >> 1) & 1u;
    n->overflow = (number >> 2) & 1u;
    pop_near(m, n, site);
}
static uint8_t nested_child(void *context, Lufia2CpuState *n, uint32_t target,
                            uint32_t site, uint8_t frame) {
    if (frame != child_frame(site))
        fault(__LINE__);
    model_child(context, n, target, site);
    return 1;
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned ordinal = ++native_calls;
    if (ordinal > 128 || !allowed_site(site) || frame != child_frame(site))
        fault(__LINE__);
    unsigned at = ((site >> 16) & 127u) * 32768u + (site & 32767u);
    unsigned opcode = native.rom[at];
    uint32_t expected = child_target(&native, site, n->x);
    if (opcode != ((site == 0x86b1f1u || site == 0x86b20bu) ? 0xfcu
                   : frame == 3u                            ? 0x22u
                                                            : 0x20u) ||
        target != expected)
        fault(__LINE__);
    n->program_bank = (uint8_t)(target >> 16);
    native_visits[ordinal - 1u] = (ChildVisit){target, site, frame, *n};
    if (ordinal == unwind_at)
        return 0;
    model_child(context, n, target, site);
    corrupt(n, ordinal);
    return 1;
}
#ifdef RUNTIME_ABI
static unsigned saved_counts[4];
static ChildVisit saved_visits[2][128];
void RuntimeSaveProbeVisits(void) {
    saved_counts[0] = native_calls;
    saved_counts[1] = original_calls;
    saved_counts[2] = unwind_at;
    saved_counts[3] = bad_at;
    memcpy(saved_visits[0], native_visits, sizeof native_visits);
    memcpy(saved_visits[1], original_visits, sizeof original_visits);
}
void RuntimeResetProbeVisits(void) {
    native_calls = 0;
    unwind_at = bad_at = 0;
}
void RuntimeRestoreProbeVisits(void) {
    native_calls = saved_counts[0];
    original_calls = saved_counts[1];
    unwind_at = saved_counts[2];
    bad_at = saved_counts[3];
    memcpy(native_visits, saved_visits[0], sizeof native_visits);
    memcpy(original_visits, saved_visits[1], sizeof original_visits);
}
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    if (!allowed_site(site) || frame != child_frame(site))
        fault(__LINE__);
    uint32_t target = child_target(b, site, n->x);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, b, NULL, NULL};
    n->program_bank = 0x86;
    unsigned saved = composition;
    composition = 0;
    model_child(&m, n, target, site);
    composition = saved;
}
#endif
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

static int same_visits(void) {
    for (unsigned i = 0; i < 128; ++i) {
        ChildVisit *a = &native_visits[i], *b = &original_visits[i];
        if (a->target != b->target || a->site != b->site || a->frame != b->frame ||
            memcmp(&a->cpu, &b->cpu, sizeof a->cpu))
            return 0;
    }
    return 1;
}
static int run(Interp816 *c) {
    Lufia2CpuState n;
    seed(&n);
    load(c, &n, entries[suite]);
    uint16_t stack = n.stack;
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    if (!number && !unwind_at && !bad_at) {
        {
            Lufia2CpuState probe_state = n;
            static const uint32_t all_sites[] = {
                0x86b1e4, 0x86b1f1, 0x86b1fe, 0x86b20b, 0x86b248, 0x86b360,
                0x86b38b, 0x86b38e, 0x86b391, 0x86b394, 0x86b47a, 0x86b4ba,
                0x86b4ee, 0x86b504, 0x86b518, 0x86b763, 0x86b768, 0x86b786,
                0x86b7c4, 0x86b7cb, 0x86b7e4, 0x86b7f3};
            for (unsigned i = 0; i < sizeof all_sites / sizeof all_sites[0]; ++i)
                if (allowed_site(all_sites[i])) {
                    runtime_probe_desired_site = all_sites[i];
                    seed(&probe_state);
                    runtime_probe(&m, &probe_state, bridges[suite], entries[suite],
                                  suite, frame_size());
                }
        }
        runtime_probe_desired_site = 0;
        seed(&n);
        load(c, &n, entries[suite]);
    }
    q = runtime_invoke(&m, &n, child, (void *)&m, bridges[suite], leaves[suite],
                       is_leaf() ? NULL : services[suite], entries[suite], suite,
                       frame_size());
#else
    q = services[suite](&m, &n, child, (void *)&m);
#endif
    uint32_t stop = q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND
                        ? native_visits[native_calls - 1u].target
                        : q.pc;
    unsigned steps = 0, pending = 0;
    uint32_t pending_return = 0;
    for (; steps < 2000000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(c);
        if (pending && pc == pending_return) {
            Lufia2CpuState changed = {0};
            store(&changed, c);
            corrupt(&changed, original_calls);
            load(c, &changed, pc);
            pending = 0;
        }
        if (pc == stop && ((q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND &&
                            original_calls == native_calls && c->sp == n.stack) ||
                           (q.flow == LUFIA2_EXECUTION_RETURNED && c->sp == stack) ||
                           (q.flow == LUFIA2_EXECUTION_BOUNDARY &&
                            original_calls == native_calls && c->sp == n.stack)))
            break;
        if (!pending && allowed_site(pc)) {
            instruction(&original, c);
            if (original_calls == 128u)
                exit(1);
            unsigned ordinal = ++original_calls;
            Lufia2CpuState entry = {0};
            store(&entry, c);
            uint32_t target = SnesVerifyPc24(c);
            original_visits[ordinal - 1u] =
                (ChildVisit){target, pc, child_frame(pc), entry};
            if (ordinal == unwind_at)
                continue;
            if (actual_site(pc, target)) {
                pending = 1;
                pending_return = pc + child_frame(pc) + 1u;
                continue;
            }
            Lufia2Memory rm = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                               NULL};
            model_child(&rm, &entry, target, pc);
            corrupt(&entry, ordinal);
            load(c, &entry, pc + child_frame(pc) + 1u);
            continue;
        }
        instruction(&original, c);
    }
    Lufia2CpuState expected = n;
    store(&expected, c);
    if (steps == 2000000u ||
        (q.flow == LUFIA2_EXECUTION_RETURNED && !valid_terminal(q.pc)) ||
        memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls || !same_visits()) {
        fprintf(stderr,
                "world suite%u case%u unwind%u bad%u/%u stop%06X steps%u CPU%d WRAM%d "
                "traces%u/%u calls%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X "
                "DB%02X/%02X\n",
                suite, number, unwind_at, bad_at, bad_kind, stop, steps,
                memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, native_calls, original_calls,
                n.accumulator, c->a, n.x, c->x, n.y, c->y, n.stack, c->sp, n.data_bank,
                c->db);
        for (unsigned i = 0; i < native_calls && i < original_calls; ++i) {
            ChildVisit *a = &native_visits[i], *b = &original_visits[i];
            if (a->target != b->target || a->site != b->site || a->frame != b->frame ||
                memcmp(&a->cpu, &b->cpu, sizeof a->cpu)) {
                fprintf(stderr,
                        "visit%u target%06X/%06X site%06X/%06X frame%u/%u CPU%d "
                        "resume%06X/%06X\n",
                        i, a->target, b->target, a->site, b->site, a->frame, b->frame,
                        memcmp(&a->cpu, &b->cpu, sizeof a->cpu), a->cpu.resume_pc,
                        b->cpu.resume_pc);
                for (unsigned z = 0; z < sizeof a->cpu; ++z)
                    if (((uint8_t *)&a->cpu)[z] != ((uint8_t *)&b->cpu)[z])
                        fprintf(stderr, "cpu byte%u %02X/%02X\n", z,
                                ((uint8_t *)&a->cpu)[z], ((uint8_t *)&b->cpu)[z]);
                break;
            }
        }
        return 1;
    }
    if (unwind_at && q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        ++effective_unwinds;
    if (bad_at && q.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++effective_returns;
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    invalid_routing = argc == 3 && strcmp(argv[2], "--invalid-routing") == 0;
    unsigned selected = argc >= 4 ? (unsigned)strtoul(argv[3], NULL, 16) : 0;
    if (argc == 6) {
        uint32_t pc = (uint32_t)strtoul(argv[4], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[5], NULL, 16);
        if (at >= size || rom[at] == value)
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
    Interp816 *c = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
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

    if (argc == 3 && strcmp(argv[2], "--guards") == 0) {
        guarding = 1;
        unsigned guards = 0;
        for (suite = 0; suite < 14; ++suite)
            for (unsigned variant = 0; variant < 10; ++variant) {
                if (is_leaf() && variant == 7u)
                    continue;
                number = 3;
                Lufia2CpuState n;
                seed(&n);
                if (variant == 0)
                    n.accumulator_is_8_bit = (uint8_t)!narrow();
                else if (variant == 1)
                    SetIndexWidth(&n, 1);
                else if (variant == 2) {
                    n.accumulator_is_8_bit = (uint8_t)!narrow();
                    SetIndexWidth(&n, 1);
                } else if (variant == 3)
                    n.program_bank = 0;
                else if (variant == 4)
                    n.direct_page = 1;
                else if (variant == 5)
                    n.direct_page = 0xffff;
                else if (variant == 6)
                    n.decimal = 1;
                else if (variant == 8)
                    n.data_bank = 0x7e;
                else if (variant == 9)
                    n.data_bank = 0x40;
                Lufia2CpuState expected = n;
                expected.resume_pc = entries[suite];
                Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                  NULL};
                Lufia2ExecutionResult q =
                    services[suite](&m, &n, variant == 7 ? NULL : child, (void *)&m);
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != expected.resume_pc ||
                    memcmp(&expected, &n, sizeof n) || native_trace.count)
                    return 1;
                ++guards;
            }
#ifdef RUNTIME_ABI
        for (suite = 0; suite < 14; ++suite)
            for (unsigned variant = 0; variant < 15; ++variant) {
                if (variant == 14u)
                    continue;
                number = 3;
                Lufia2CpuState n;
                seed(&n);
                Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                  NULL};
                RuntimeCase test = {0};
                test.memory = &m;
                test.entry = entries[suite];
                test.start = n.stack;
                test.hrv = frame_size();
                test.guard_only = 1;
                runtime_case = &test;
                CpuState guest;
                memset(&guest, 0, sizeof guest);
                runtime_load(&guest, &n);
                guest.host_return_valid = (uint8)frame_size();
                if (variant == 0)
                    guest.m_flag = (uint8)!narrow();
                else if (variant == 1)
                    guest.x_flag = 1;
                else if (variant == 2) {
                    guest.m_flag = (uint8)!narrow();
                    guest.x_flag = 1;
                } else if (variant == 3)
                    guest.PB = 0;
                else if (variant == 4)
                    guest.DB = 0x7e;
                else if (variant == 5)
                    guest.DB = 0x40;
                else if (variant == 6)
                    guest.D = 1;
                else if (variant == 7)
                    guest.D = 0xffff;
                else if (variant == 8)
                    guest._flag_D = 1;
                else if (variant == 9)
                    guest.emulation = 1;
                else if (variant == 14u)
                    guest.S = 0x1f23u;
                else
                    guest.S = (uint16_t[]){0x1f1f, 0x1ffd, 0, 0xffff}[variant - 10];
                test.start = guest.S;
                cpu_mirrors_to_p(&guest);
                CpuState expected = guest;
                RecompReturn ret = bridges[suite](&guest);
                runtime_return_reads = 0;
                guest.open_bus = expected.open_bus;
                if (ret != RECOMP_RETURN_LLE_UNWIND_BASE || test.stub != 1 ||
                    test.target != test.entry || test.kind != 3 ||
                    memcmp(&guest, &expected, sizeof guest) || native_trace.count) {
                    fprintf(stderr, "ABI guard suite%u variant%u\n", suite, variant);
                    return 1;
                }
                ++guards;
            }
        runtime_case = NULL;
#endif
        printf("PASS world guards:%u\n", guards);
        return 0;
    }
    if (argc == 3 &&
        (strcmp(argv[2], "--unwind") == 0 || strcmp(argv[2], "--returns") == 0)) {
        unsigned cases = 0;
        static const unsigned calls[] = {1, 1, 1, 1, 1, 8, 0, 0, 0, 2, 2, 2, 50, 19};
        for (suite = 0; suite < 14; ++suite) {
            if (strcmp(argv[2], "--returns") == 0) {
                if (is_leaf())
                    continue;
                for (bad_at = 1; bad_at <= calls[suite]; ++bad_at)
                    for (bad_kind = 0; bad_kind < 5; ++bad_kind)
                        for (number = 0; number < 4; ++number) {
                            if (run(c))
                                return 1;
                            ++cases;
                        }
                bad_at = 0;
            } else {
                for (unwind_at = 1; unwind_at <= calls[suite]; ++unwind_at)
                    for (number = 0; number < 8; ++number) {
                        if (run(c))
                            return 1;
                        ++cases;
                    }
                unwind_at = 0;
            }
        }
        printf("PASS world child contracts:%u reached-unwinds:%u reached-returns:%u\n",
               cases, effective_unwinds, effective_returns);
        return 0;
    }
    if (invalid_routing)
        selected = 0x86b1deu;
    unsigned total = 0;
    for (suite = 0; suite < 14; ++suite) {
        if (selected && selected != entries[suite])
            continue;
        unsigned count = invalid_routing ? 256u : case_counts[suite];
        if (argc == 3 && strcmp(argv[2], "--probes") == 0)
            count = 2;
#ifdef RUNTIME_ABI
        count /= 2u;
#endif
        for (number = 0; number < count; ++number) {
            if (run(c))
                return 1;
            ++total;
        }
    }
    printf("PASS world scene original CPU/fullWRAM/ordered bus/child snapshots:%u\n",
           total);
#ifdef RUNTIME_ABI
    unsigned redirects = 0, fallbacks = 0;
    for (unsigned i = 0; i < 14; ++i) {
        redirects += runtime_redirects[i];
        fallbacks += runtime_fallback[i];
    }
    printf("World production redirects:%u fallback:%u\n", redirects, fallbacks);
    if (fallbacks || (!selected && !redirects))
        return 1;
#endif
    interp816_free(c);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
