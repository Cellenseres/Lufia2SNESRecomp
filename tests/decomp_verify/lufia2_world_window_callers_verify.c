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
static unsigned effective_unwinds, effective_returns;
static Lufia2ExecutionResult ServiceC536(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapSelectSlopedWindowRecord(m, c);
}
static Lufia2ExecutionResult ServiceC321(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapSelectSceneWindowRecord(m, c);
}
static Lufia2ExecutionResult ServiceA52F(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapMultiply16(m, c);
}
static const uint32_t entries[] = {0x86c536u, 0x86c54bu, 0x86c4e0u, 0x86c5f8u,
                                   0x86cb21u, 0x86cb49u, 0x86cb9eu, 0x86b039u,
                                   0x86b1deu, 0x86c321u, 0x86a52fu};
static const uint32_t terminals[] = {0x86c54a, 0x86c581, 0x86c535, 0x86c755,
                                     0x86cb40, 0x86cb97, 0x86cbbe, 0x86b0b9,
                                     0x86b24b, 0x86c335, 0x86a582};
static int valid_terminal(uint32_t pc) {
    return pc == terminals[suite] || (suite == 3u && pc == 0x86c67eu) ||
           (suite == 5u && pc == 0x86cb9du);
}
static const Service services[] = {ServiceC536,
                                   Lufia2WorldMapLoadSlopedWindowRecord,
                                   Lufia2WorldMapSubmitSlopedWindowRecord,
                                   Lufia2WorldMapBuildSlopedWindow,
                                   Lufia2WorldMapClipWindowEndpoint,
                                   Lufia2WorldMapClipWindowLowerRight,
                                   Lufia2WorldMapResolveWindowRightIntersection,
                                   Lufia2WorldMapStepExpandingSceneWindows,
                                   Lufia2WorldMapStepSceneWindowAnimations,
                                   ServiceC321,
                                   ServiceA52F};
static const unsigned case_counts[] = {2048, 2048, 2048, 8192, 4096, 8192,
                                       4096, 1024, 1024, 2048, 8192};
static int narrow(void) {
    return suite != 5u && suite != 6u && (suite != 10u || (number & 1u));
}
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_86C536(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C54B(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C4E0(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C5F8(CpuState *);
extern RecompReturn Lufia2DecompBridge_86CB21(CpuState *);
extern RecompReturn Lufia2DecompBridge_86CB49(CpuState *);
extern RecompReturn Lufia2DecompBridge_86CB9E(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B039(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B1DE(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C321(CpuState *);
extern RecompReturn Lufia2DecompBridge_86A52F(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_86C536, Lufia2DecompBridge_86C54B, Lufia2DecompBridge_86C4E0,
    Lufia2DecompBridge_86C5F8, Lufia2DecompBridge_86CB21, Lufia2DecompBridge_86CB49,
    Lufia2DecompBridge_86CB9E, Lufia2DecompBridge_86B039, Lufia2DecompBridge_86B1DE,
    Lufia2DecompBridge_86C321, Lufia2DecompBridge_86A52F};
static const RuntimeLeaf leaves[] = {Lufia2WorldMapSelectSlopedWindowRecord,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     Lufia2WorldMapSelectSceneWindowRecord,
                                     Lufia2WorldMapMultiply16};
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
static void instruction(SnesVerifyBus *b, Interp816 *c) {
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
static unsigned frame_size(void) { return suite == 3u ? 3u : 2u; }
#endif
static const uint32_t call_sites[11][17] = {
    {0},
    {0x86c54b},
    {0x86c4e0, 0x86c526},
    {0x86c627, 0x86c640, 0x86c657, 0x86c670, 0x86c687, 0x86c6d2, 0x86c6f8, 0x86c702,
     0x86c722, 0x86c73f, 0x86c74e, 0x86c76a, 0x86c775, 0x86c778, 0x86c789, 0x86c794,
     0x86c797},
    {0x86cb3b},
    {0x86cb5b, 0x86cb77, 0x86cb88},
    {0x86cbb6},
    {0x86b03f, 0x86b075, 0x86b0a1},
    {0x86b1e4, 0x86b1f1, 0x86b1fe, 0x86b20b, 0x86b248},
    {0},
    {0}};
static int is_leaf(void) { return suite == 0u || suite >= 9u; }
static uint8_t child_frame(uint32_t site) {
    return site == 0x86c526u || site == 0x86c311u ? 3u : 2u;
}
static int allowed_site(uint32_t site) {
    for (unsigned i = 0; i < 17u; ++i)
        if (call_sites[suite][i] && call_sites[suite][i] == site)
            return 1;
    return 0;
}
static uint32_t child_target(const SnesVerifyBus *b, uint32_t site, uint16_t index) {
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
static int actual_site(uint32_t site) {
    return composition && site != 0x86b1f1u && site != 0x86b20bu;
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
    n->accumulator_is_8_bit = (uint8_t)narrow();
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 257u);
    n->y = (uint16_t[]){
        0xb100, 0xb200, 0xb300, 0x1ec2, 0x1ef2, 0, 0xfffe, 0xffff}[(number >> 5) % 8u];
    n->stack = (uint16_t)(0x1f60u + (number & 63u));
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 6) & 1u;
    word(0, (uint16_t)(number * 109u));
    word(4, (uint16_t)(number * 991u));
    word(0x58,
         (uint16_t[]){128, 0, 1, 255, 256, 0xffff, 0x8000, 192}[(number >> 8) % 8u]);
    word(0x5a,
         (uint16_t[]){112, 0, 1, 223, 224, 0xffff, 0x8000, 48}[(number >> 5) % 8u]);
    word(0x24, (uint16_t[]){0, 1, 2, 128, 255, 256, 511, 0xffff}[(number >> 6) % 8u]);
    word(0x26, (uint16_t)(number & 255u));
    word(0x28, (uint16_t[]){0, 1, 2, 127, 128, 223, 224, 255}[(number >> 8) % 8u]);
    native.wram[0x22] = (uint8_t)number;
    native.wram[0x23] = (uint8_t[]){0, 1, 2, 63, 64, 127, 128, 255}[(number >> 8) % 8u];
    word(0x60, (uint16_t[]){0xd400, 0xd600, 0xd800, 0xda00}[(number >> 5) % 4u]);
    native.wram[0x62] = 0x7f;
    native.wram[0x11db] = (uint8_t)(number >> 2);
    native.wram[0x11dc] = (uint8_t)(number >> 1);
    word(0x15,
         (uint16_t[]){0, 1, 127, 255, 256, 257, 0x7fff, 0xffff}[(number >> 5) % 8u]);
    word(0x17,
         (uint16_t[]){0, 1, 112, 223, 224, 225, 0x7fff, 0xffff}[(number >> 8) % 8u]);
    word(0x11, (uint16_t)(number & 255u));
    for (unsigned i = 0; i < 6u; ++i) {
        unsigned at = 0x1e68u + i * 15u;
        word(at, 128);
        word(at + 2u, 112);
        native.wram[at + 4u] = (uint8_t)(number + i * 32u);
        native.wram[at + 5u] = 32;
        word(at + 6u, 128);
        word(at + 8u, 0);
        word(at + 10u, (uint16_t)(1u + (number + i) % 96u));
        word(at + 12u, (uint16_t)(1u + (number + i) % 3u));
    }
    for (unsigned i = 0; i < 3u; ++i) {
        unsigned at = 0x1e50u + i * 6u;
        native.wram[at] = 128;
        native.wram[at + 1u] = 112;
        native.wram[at + 2u] = (uint8_t)(1u + (number + i) % 96u);
        native.wram[at + 3u] = 0;
        native.wram[at + 4u] = (uint8_t)(1u + (number + i) % 2u);
        native.wram[at + 5u] = 0;
    }
    native.wram[0x1226] = (uint8_t)number;
    native.wram[0x1228] = (uint8_t)number;
    native.wram[0x122a] = (uint8_t[]){0, 1, 0x80, 0xff}[(number >> 4) % 4u];
    native.wram[0x122b] = (uint8_t)(number % 16u);
    native.wram[0x122c] = 0;
    for (unsigned i = 0; i < 3u; ++i)
        word(0x1534u + i * 0x1du + 5u, 0xffff);
    if (suite == 1u || suite == 2u)
        n->accumulator = (uint16_t)((n->accumulator & 0xff00u) | (number % 6u));
    if (suite >= 3u && suite <= 6u && (unwind_at || bad_at || !number)) {
        word(0x28, 96);
        word(0x24, 128);
        word(0x58, 128);
        word(0x5a, 112);
        native.wram[0x22] =
            (uint8_t[]){0x20, 0x60, 0x40, 0x80, 0xc0, 0xe0, 0, 0xff}[number % 8u];
        native.wram[0x23] = 0x20;
        word(0x15, 0x100);
        word(0x17, (number & 2u) ? 0xdf : 0xe0);
    }
    if (suite == 10u) {
        word(0, (uint16_t)(number * 117u));
        word(4, (uint16_t)(number * 991u));
    }
    if (suite == 7u) {
        for (unsigned i = 0; i < 6u; ++i) {
            unsigned at = 0x1e68u + i * 15u;
            native.wram[at + 12u] = (uint8_t[]){
                0, 1, 9, 10, 11, 64, 96, 127, 128, 255}[(number / 8u + i) % 10u];
            native.wram[at + 8u] =
                (uint8_t[]){0, 1, 5, 127, 255}[(number / 16u + i) % 5u];
            native.wram[at + 10u] = (uint8_t[]){
                0, 1, 8, 9, 10, 11, 63, 96, 127, 255}[(number / 4u + i) % 10u];
        }
    }
    if ((unwind_at || bad_at) && suite >= 7u && suite <= 8u) {
        for (unsigned i = 0; i < 6u; ++i) {
            word(0x1e68u + i * 15u + 10u, 1);
            word(0x1e68u + i * 15u + 12u, 1);
        }
        for (unsigned i = 0; i < 3u; ++i)
            native.wram[0x1e50u + i * 6u + 4u] = 1;
    }
    word((uint32_t)n->stack + 1u, 0x8123);
    native.wram[n->stack + 3u] = 0x83;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
    composition = number < 32u && !unwind_at && !bad_at;
}
static void corrupt(Lufia2CpuState *n, unsigned ordinal) {
    if (ordinal != bad_at)
        return;
    if (bad_kind == 0) {
        if (suite == 4u)
            n->data_bank = 0x40;
        else
            n->accumulator_is_8_bit = (uint8_t)!n->accumulator_is_8_bit;
    } else if (bad_kind == 1)
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
    if (composition) {
        Lufia2ExecutionResult q = ExecutionReturned(0u);
        int canonical = 1;
        switch (target) {
        case 0x86c536:
            q = Lufia2WorldMapSelectSlopedWindowRecord(m, n);
            break;
        case 0x86c321:
            q = Lufia2WorldMapSelectSceneWindowRecord(m, n);
            break;
        case 0x86c54b:
            q = Lufia2WorldMapLoadSlopedWindowRecord(m, n, nested_child, (void *)m);
            break;
        case 0x86c4e0:
            q = Lufia2WorldMapSubmitSlopedWindowRecord(m, n, nested_child, (void *)m);
            break;
        case 0x86c5f8:
            q = Lufia2WorldMapBuildSlopedWindow(m, n, nested_child, (void *)m);
            break;
        case 0x86cb21:
            q = Lufia2WorldMapClipWindowEndpoint(m, n, nested_child, (void *)m);
            break;
        case 0x86cb49:
            q = Lufia2WorldMapClipWindowLowerRight(m, n, nested_child, (void *)m);
            break;
        case 0x86cb9e:
            q = Lufia2WorldMapResolveWindowRightIntersection(m, n, nested_child,
                                                             (void *)m);
            break;
        case 0x86a52f:
            q = Lufia2WorldMapMultiply16(m, n);
            break;
        case 0x86c2d9:
            q = Lufia2WorldMapSubmitSceneWindowRecord(m, n, nested_child, (void *)m);
            break;
        case 0x86c358:
            q = Lufia2WorldMapBuildCircularWindow(m, n, nested_child, (void *)m);
            break;
        case 0x86c4a8:
            q = Lufia2WorldMapBuildCircleExtentTable(m, n);
            break;
        case 0x86c40a:
            q = Lufia2WorldMapAppendUpperCircleRows(m, n);
            break;
        case 0x86c444:
            q = Lufia2WorldMapAppendLowerCircleRows(m, n);
            break;
        case 0x86ca06:
            if (n->data_bank != 0x7f) {
                canonical = 0;
                break;
            }
            q = Lufia2WorldMapDrawSlopedWindowEdge(m, n, nested_child, (void *)m);
            break;
        case 0x86ca2f:
            q = Lufia2WorldMapDrawUpperRightWindowEdge(m, n);
            break;
        case 0x86ca5e:
            q = Lufia2WorldMapDrawLowerRightWindowEdge(m, n);
            break;
        case 0x86ca83:
            q = Lufia2WorldMapDrawLowerLeftWindowEdge(m, n);
            break;
        case 0x86cab2:
            q = Lufia2WorldMapDrawUpperLeftWindowEdge(m, n);
            break;
        case 0x86cad7:
            if (n->data_bank != 0x7f) {
                canonical = 0;
                break;
            }
            q = Lufia2WorldMapInitializeWindowRows(m, n);
            break;
        case 0x86c831:
            if (n->data_bank != 0x7f) {
                canonical = 0;
                break;
            }
            q = Lufia2WorldMapCountWindowEdgeRows(m, n);
            break;
        case 0x86c843:
            if (n->data_bank != 0x7f) {
                canonical = 0;
                break;
            }
            q = Lufia2WorldMapResolveWindowEdgeEnd(m, n);
            break;
        case 0x86c854:
            if (n->data_bank != 0x7f) {
                canonical = 0;
                break;
            }
            q = Lufia2WorldMapBuildLowerWindowBand(m, n);
            break;
        case 0x86c928:
            if (n->data_bank != 0x7f) {
                canonical = 0;
                break;
            }
            q = Lufia2WorldMapBuildUpperWindowBand(m, n);
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
        if (site == 0x86b1f1u || site == 0x86b20bu) {
            composition = 0;
            model_child(m, n, target, site);
            composition = 1;
            return;
        }
        execute_original_child(m, n, target, site);
        return;
    }
    if (target == 0x86c536u || target == 0x86c321u) {
        Lufia2ExecutionResult q = target == 0x86c536u
                                      ? Lufia2WorldMapSelectSlopedWindowRecord(m, n)
                                      : Lufia2WorldMapSelectSceneWindowRecord(m, n);
        if (q.flow != LUFIA2_EXECUTION_RETURNED)
            fault(__LINE__);
        pop_near(m, n, site);
        return;
    }
    OpSta(m, n, 0x7ff100u + (site & 31u));
    if (target == 0x86a417u) {
        OpWrite16(m, OpDp(n, 8u), 0);
        OpWrite16(m, OpDp(n, 0x0bu), 0);
    }
    if (target == 0x86cb21u) {
        OpWrite16(m, OpDp(n, 0x15u), site == 0x86c640u ? 64u : 192u);
        OpWrite16(m, OpDp(n, 0x17u), site == 0x86c640u ? 48u : 160u);
    }
    if (target == 0x86ca06u)
        Write8(m, DirectAddress(n, 3u), (uint8_t)((number & 7u) ? 0u : 32u));
    OpLdx(n, (uint16_t)(number * 197u));
    OpLdy(n, (uint16_t)(number * 17u));
    n->accumulator_is_8_bit = (uint8_t)(suite == 5u || suite == 6u ? 0u
                                        : suite == 4u              ? (number & 1u)
                                                                   : 1u);
    OpLoadA(n, target == 0x86cb9eu ? (uint16_t)((number & 1u) ? 0xe0u : 0u)
                                   : (uint16_t)(number * 37u));
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
    if (opcode != ((site == 0x86cb3bu || site == 0x86b1f1u || site == 0x86b20bu) ? 0xfcu
                   : frame == 3u                                                 ? 0x22u
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
                0x86b03f, 0x86b075, 0x86b0a1, 0x86b1e4, 0x86b1f1, 0x86b1fe, 0x86b20b,
                0x86b248, 0x86c4e0, 0x86c526, 0x86c54b, 0x86c627, 0x86c640, 0x86c657,
                0x86c670, 0x86c687, 0x86c6d2, 0x86c6f8, 0x86c702, 0x86c722, 0x86c73f,
                0x86c74e, 0x86c76a, 0x86c775, 0x86c778, 0x86c789, 0x86c794, 0x86c797,
                0x86cb3b, 0x86cb5b, 0x86cb77, 0x86cb88, 0x86cbb6};
            for (unsigned i = 0; i < sizeof all_sites / sizeof all_sites[0]; ++i)
                if (allowed_site(all_sites[i])) {
                    runtime_probe_desired_site = all_sites[i];
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
            if (actual_site(pc)) {
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
        unsigned guards = 0;
        for (suite = 0; suite < 11; ++suite)
            for (unsigned variant = 0; variant < 10; ++variant) {
                if ((is_leaf() && variant == 7u) ||
                    (suite == 10u && variant != 3u && variant != 6u))
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
        for (suite = 0; suite < 11; ++suite)
            for (unsigned variant = 0; variant < 15; ++variant) {
                if (variant == 14u)
                    continue;
                if (suite == 10u && variant != 3u && variant != 8u && variant != 9u &&
                    variant != 11u && variant != 13u)
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
        static const unsigned calls[] = {0, 1, 2, 11, 1, 2, 1, 13, 19, 0, 0};
        for (suite = 0; suite < 11; ++suite) {
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
    unsigned total = 0;
    for (suite = 0; suite < 11; ++suite) {
        if (selected && selected != entries[suite])
            continue;
        unsigned count = case_counts[suite];
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
    for (unsigned i = 0; i < 11; ++i) {
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
