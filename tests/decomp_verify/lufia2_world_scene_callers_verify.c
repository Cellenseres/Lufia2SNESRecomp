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
static Lufia2ExecutionResult ServiceBEC4(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapOffsetSceneObject(m, c);
}
static Lufia2ExecutionResult ServiceC321(const Lufia2Memory *m, Lufia2CpuState *c,
                                         Lufia2PushedChildCall child, void *ctx) {
    (void)child;
    (void)ctx;
    return Lufia2WorldMapSelectSceneWindowRecord(m, c);
}
static const uint32_t entries[] = {
    0x86b15au, 0x86b59eu, 0x86b63bu, 0x86bb5cu, 0x86be30u, 0x86c04eu,
    0x86b19au, 0x86bbc4u, 0x86be72u, 0x86be7au, 0x86bec4u, 0x86c321u,
    0x86c336u, 0x86c2d9u, 0x86afb2u, 0x86afe3u, 0x86bbe6u, 0x86c08cu};
static const uint32_t terminals[] = {
    0x86b199u, 0x86b5d1u, 0x86b731u, 0x86bb8bu, 0x86be71u, 0x86c08bu,
    0x86b1acu, 0x86bbe5u, 0x86be79u, 0x86bec3u, 0x86bed8u, 0x86c335u,
    0x86c357u, 0x86c320u, 0x86afdcu, 0x86b01fu, 0x86bbf8u, 0x86c09eu};
static const Service services[] = {Lufia2WorldMapRunGroupScene2,
                                   Lufia2WorldMapRunObjectScene,
                                   Lufia2WorldMapRunGroupTransitionScene,
                                   Lufia2WorldMapRunNearbyGroupScene,
                                   Lufia2WorldMapRunOffsetGroupScene,
                                   Lufia2WorldMapRunDualObjectScene,
                                   Lufia2WorldMapLoadGroupSceneBuffer,
                                   Lufia2WorldMapPositionNearbySceneGroup,
                                   Lufia2WorldMapStartSceneObjectAnimation,
                                   Lufia2WorldMapPositionOffsetSceneGroup,
                                   ServiceBEC4,
                                   ServiceC321,
                                   Lufia2WorldMapLoadSceneWindowRecord,
                                   Lufia2WorldMapSubmitSceneWindowRecord,
                                   Lufia2WorldMapRunGroupScene0,
                                   Lufia2WorldMapRunGroupScene1,
                                   Lufia2WorldMapLoadSceneBuffer,
                                   Lufia2WorldMapLoadAlternateSceneBuffer};
static const unsigned case_counts[] = {512,  512,  512, 512,  512,  512,
                                       512,  512,  512, 1024, 4096, 4096,
                                       1024, 1024, 512, 512,  512,  512};
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_86B15A(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B59E(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B63B(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BB5C(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BE30(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C04E(CpuState *);
extern RecompReturn Lufia2DecompBridge_86B19A(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BBC4(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BE72(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BE7A(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BEC4(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C321(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C336(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C2D9(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AFB2(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AFE3(CpuState *);
extern RecompReturn Lufia2DecompBridge_86BBE6(CpuState *);
extern RecompReturn Lufia2DecompBridge_86C08C(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_86B15A, Lufia2DecompBridge_86B59E, Lufia2DecompBridge_86B63B,
    Lufia2DecompBridge_86BB5C, Lufia2DecompBridge_86BE30, Lufia2DecompBridge_86C04E,
    Lufia2DecompBridge_86B19A, Lufia2DecompBridge_86BBC4, Lufia2DecompBridge_86BE72,
    Lufia2DecompBridge_86BE7A, Lufia2DecompBridge_86BEC4, Lufia2DecompBridge_86C321,
    Lufia2DecompBridge_86C336, Lufia2DecompBridge_86C2D9, Lufia2DecompBridge_86AFB2,
    Lufia2DecompBridge_86AFE3, Lufia2DecompBridge_86BBE6, Lufia2DecompBridge_86C08C};
static const RuntimeLeaf leaves[] = {NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     Lufia2WorldMapOffsetSceneObject,
                                     Lufia2WorldMapSelectSceneWindowRecord,
                                     NULL,
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
static unsigned frame_size(void) { return 2u; }
#endif
static const uint32_t call_sites[18][24] = {
    {0x86b15f, 0x86b168, 0x86b16b, 0x86b16e, 0x86b183, 0x86b186, 0x86b196},
    {0x86b5a4, 0x86b5a7, 0x86b5b3, 0x86b5c0, 0x86b5ce},
    {0x86b640, 0x86b649, 0x86b64c, 0x86b64f, 0x86b664, 0x86b667, 0x86b66f,
     0x86b676, 0x86b67d, 0x86b695, 0x86b698, 0x86b6ba, 0x86b6be, 0x86b6c7,
     0x86b6cd, 0x86b6e2, 0x86b6ec, 0x86b705, 0x86b71b, 0x86b72e},
    {0x86bb5c, 0x86bb5f, 0x86bb68, 0x86bb6b, 0x86bb6e, 0x86bb71, 0x86bb74, 0x86bb77,
     0x86bb88},
    {0x86be30, 0x86be39, 0x86be3c, 0x86be3f, 0x86be42, 0x86be45, 0x86be4b, 0x86be51,
     0x86be59, 0x86be60, 0x86be6e},
    {0x86c04e, 0x86c057, 0x86c05a, 0x86c05d, 0x86c067, 0x86c071, 0x86c088},
    {0x86b1a8},
    {0x86bbe2},
    {0x86be76},
    {0x86be98, 0x86bea6, 0x86beb2, 0x86bebe},
    {0},
    {0},
    {0x86c336},
    {0x86c2d9, 0x86c311},
    {0x86afb8, 0x86afbb, 0x86afbe, 0x86afd3, 0x86afd9},
    {0x86afe9, 0x86afec, 0x86afef, 0x86b004, 0x86b007, 0x86b01c},
    {0x86bbf4},
    {0x86c09a}};
static int is_leaf(void) { return suite == 10u || suite == 11u; }
static uint8_t child_frame(uint32_t site) {
    return site == 0x86b1a8u || site == 0x86b6bau || site == 0x86c311u ||
                   site == 0x86bbf4u || site == 0x86c09au
               ? 3u
               : 2u;
}
static int allowed_site(uint32_t site) {
    for (unsigned i = 0; i < 24u; ++i)
        if (call_sites[suite][i] && site == call_sites[suite][i])
            return 1;
    return 0;
}

static int actual_site(uint32_t site) {
    if (!composition)
        return 0;
    uint32_t target =
        romword(site + 1u) |
        (child_frame(site) == 3u
             ? (uint32_t)
                       native.rom[((site >> 16) & 127u) * 32768u + (site & 32767u) + 3u]
                   << 16
             : 0x860000u);
    switch (target) {
    case 0x869470:
    case 0x86c284:
    case 0x86c2b3:
    case 0x86c582:
    case 0x86bb8c:
    case 0x86bbc4:
    case 0x86be72:
    case 0x86be7a:
    case 0x86bec4:
    case 0x86c321:
    case 0x86c336:
    case 0x86b19a:
    case 0x86bbe6:
    case 0x86c08c:
    case 0x808e9d:
        return 1;
    default:
        return 0;
    }
}
static void seed(Lufia2CpuState *n) {
    for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
        native.wram[i] = (uint8_t)((i * 23u + number * 47u + suite * 79u) >> 3);
    memset(n, 0, sizeof *n);
    memset(native_visits, 0, sizeof native_visits);
    memset(original_visits, 0, sizeof original_visits);
    native_calls = original_calls = 0;
    n->program_bank = n->data_bank = 0x86;
    n->accumulator_is_8_bit = suite != 10u;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(0x1469u + (number % 32u) * 0x1du);
    n->y = (uint16_t)(number * 113u);
    n->stack = (uint16_t)(0x1f40u + (number & 127u));
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 6) & 1u;
    for (unsigned i = 0; i < 32; ++i) {
        native.wram[0x1469u + i * 0x1du + 15u] = 0;
        native.wram[0x1469u + i * 0x1du + 16u] = 0x1c;
    }
    word(0x11e8, (uint16_t)(number * 127u));
    word(0x11ea, (uint16_t)(number * 113u));
    word(0x1519, (uint16_t)(number * 239u));
    word(0x151c, (uint16_t)(number * 71u));
    native.wram[0] = (uint8_t)(number & 3u);
    native.wram[0x11db] = (uint8_t)number;
    native.wram[0x11dc] = (uint8_t)(number >> 1);
    if (suite == 13u && number >= 256u)
        native.wram[0x11db] ^= 1u;
    if (suite == 10u)
        n->x =
            (uint16_t[]){0,      1,      2,      4,      5,      6,
                         0x1519, 0x151c, 0xffff, 0xfffe, 0x1fff, 0x1469}[(number >> 4) %
                                                                         12u];
    if (suite == 12u)
        n->y = (uint16_t[]){
            0xb82f, 0, 1, 0x1e50, 0x1fff, 0xffff, 0xfffe, 0x3ff}[(number >> 4) % 8u];
    word((uint32_t)n->stack + 1u, 0x8123);
    native.wram[n->stack + 3u] = 0x83;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
    composition = number < 4u && !unwind_at && !bad_at;
}
static void corrupt(Lufia2CpuState *n, unsigned ordinal) {
    if (ordinal != bad_at)
        return;
    if (bad_kind == 0)
        n->accumulator_is_8_bit = !n->accumulator_is_8_bit;
    else if (bad_kind == 1)
        SetIndexWidth(n, 1);
    else if (bad_kind == 2)
        n->direct_page = 0x100;
    else if (bad_kind == 3)
        n->decimal = 1;
    else
        n->data_bank = 0x7f;
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

static void model_child(const Lufia2Memory *, Lufia2CpuState *, uint32_t, uint32_t);
static uint8_t nested_child(void *context, Lufia2CpuState *n, uint32_t target,
                            uint32_t site, uint8_t frame) {
    if (frame != child_frame(site))
        fault(__LINE__);
    model_child(context, n, target, site);
    return 1;
}
static void model_child(const Lufia2Memory *m, Lufia2CpuState *n, uint32_t target,
                        uint32_t site) {
    if (composition) {
        Lufia2ExecutionResult q = ExecutionReturned(0u);
        int canonical = 1;
        if (target == 0x86c284)
            q = Lufia2WorldMapStartSceneActors(m, n, nested_child, (void *)m);
        else if (target == 0x86c2b3)
            q = Lufia2WorldMapPositionSceneGroup(m, n);
        else if (target == 0x86c582)
            q = Lufia2WorldMapPrepareSceneWindowTables(m, n);
        else if (target == 0x86966c)
            q = Lufia2WorldMapDarkenScenePalette(m, n);
        else if (target == 0x86bb8c)
            q = Lufia2WorldMapInstallScenePalette(m, n, nested_child, (void *)m);
        else if (target == 0x86bf99)
            q = Lufia2WorldMapUploadSceneGraphics(m, n, nested_child, (void *)m);
        else if (target == 0x86b15a)
            q = Lufia2WorldMapRunGroupScene2(m, n, nested_child, (void *)m);
        else if (target == 0x86b59e)
            q = Lufia2WorldMapRunObjectScene(m, n, nested_child, (void *)m);
        else if (target == 0x86b63b)
            q = Lufia2WorldMapRunGroupTransitionScene(m, n, nested_child, (void *)m);
        else if (target == 0x86bb5c)
            q = Lufia2WorldMapRunNearbyGroupScene(m, n, nested_child, (void *)m);
        else if (target == 0x86be30)
            q = Lufia2WorldMapRunOffsetGroupScene(m, n, nested_child, (void *)m);
        else if (target == 0x86c04e)
            q = Lufia2WorldMapRunDualObjectScene(m, n, nested_child, (void *)m);
        else if (target == 0x86b19a)
            q = Lufia2WorldMapLoadGroupSceneBuffer(m, n, nested_child, (void *)m);
        else if (target == 0x86bbc4)
            q = Lufia2WorldMapPositionNearbySceneGroup(m, n, nested_child, (void *)m);
        else if (target == 0x86be72)
            q = Lufia2WorldMapStartSceneObjectAnimation(m, n, nested_child, (void *)m);
        else if (target == 0x86be7a)
            q = Lufia2WorldMapPositionOffsetSceneGroup(m, n, nested_child, (void *)m);
        else if (target == 0x86bec4)
            q = Lufia2WorldMapOffsetSceneObject(m, n);
        else if (target == 0x86c321)
            q = Lufia2WorldMapSelectSceneWindowRecord(m, n);
        else if (target == 0x86c336)
            q = Lufia2WorldMapLoadSceneWindowRecord(m, n, nested_child, (void *)m);
        else if (target == 0x86c2d9)
            q = Lufia2WorldMapSubmitSceneWindowRecord(m, n, nested_child, (void *)m);
        else if (target == 0x86bbe6)
            q = Lufia2WorldMapLoadSceneBuffer(m, n, nested_child, (void *)m);
        else if (target == 0x86c08c)
            q = Lufia2WorldMapLoadAlternateSceneBuffer(m, n, nested_child, (void *)m);
        else if (target == 0x808e9d)
            q = Lufia2DecompressResource(m, n);
        else
            canonical = 0;
        if (canonical) {
            if (q.flow != LUFIA2_EXECUTION_RETURNED)
                fault(__LINE__);
            pop_near(m, n, site);
            return;
        }
        if (target == 0x869470 || target == 0x86c2ad || target == 0x86e0b9 ||
            target == 0x86d3a5) {
            execute_original_child(m, n, target, site);
            return;
        }
    }
    OpSta(m, n, 0x7ff100u + (site & 31u));
    OpLdx(n, (uint16_t)(0x1469u + (number + site) % 32u * 0x1du));
    OpLdy(n, (uint16_t)(number * 17u));
    OpLoadA(n, (uint8_t)(number * 37u));
    n->carry = (number >> 1) & 1u;
    n->overflow = (number >> 2) & 1u;
    if (target == 0x808e9d) {
        Write16Direct(m, n, 0x60u, (uint16_t)(0xe000u + (number & 31u)));
        Write8(m, DirectAddress(n, 0x62u), (uint8_t)(0x7eu + (number & 1u)));
        Write16Direct(m, n, 0x58u, (uint16_t)(number * 173u));
    }
    pop_near(m, n, site);
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned ordinal = ++native_calls;
    if (ordinal > 128 || !allowed_site(site) || frame != child_frame(site))
        fault(__LINE__);
    unsigned at = ((site >> 16) & 127u) * 32768u + (site & 32767u);
    unsigned opcode = native.rom[at];
    uint32_t expected = romword(site + 1u) |
                        (frame == 3u ? (uint32_t)native.rom[at + 3u] << 16 : 0x860000u);
    if (opcode != (frame == 3u ? 0x22u : 0x20u) || target != expected)
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
    unsigned at = ((site >> 16) & 127u) * 32768u + (site & 32767u);
    uint32_t target = romword(site + 1u) |
                      (frame == 3u ? (uint32_t)b->rom[at + 3u] << 16 : 0x860000u);
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
                0x86afb8, 0x86afbb, 0x86afbe, 0x86afd3, 0x86afd9, 0x86afe9, 0x86afec,
                0x86afef, 0x86b004, 0x86b007, 0x86b01c, 0x86b15f, 0x86b168, 0x86b16b,
                0x86b16e, 0x86b183, 0x86b186, 0x86b196, 0x86b1a8, 0x86b5a4, 0x86b5a7,
                0x86b5b3, 0x86b5c0, 0x86b5ce, 0x86b640, 0x86b649, 0x86b64c, 0x86b64f,
                0x86b664, 0x86b667, 0x86b66f, 0x86b676, 0x86b67d, 0x86b695, 0x86b698,
                0x86b6ba, 0x86b6be, 0x86b6c7, 0x86b6cd, 0x86b6e2, 0x86b6ec, 0x86b705,
                0x86b71b, 0x86b72e, 0x86bb5c, 0x86bb5f, 0x86bb68, 0x86bb6b, 0x86bb6e,
                0x86bb71, 0x86bb74, 0x86bb77, 0x86bb88, 0x86bbe2, 0x86bbf4, 0x86be30,
                0x86be39, 0x86be3c, 0x86be3f, 0x86be42, 0x86be45, 0x86be4b, 0x86be51,
                0x86be59, 0x86be60, 0x86be6e, 0x86be76, 0x86be98, 0x86bea6, 0x86beb2,
                0x86bebe, 0x86c04e, 0x86c057, 0x86c05a, 0x86c05d, 0x86c067, 0x86c071,
                0x86c088, 0x86c09a, 0x86c2d9, 0x86c311, 0x86c336};
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
        (q.flow == LUFIA2_EXECUTION_RETURNED && q.pc != terminals[suite]) ||
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
        for (suite = 0; suite < 18; ++suite)
            for (unsigned variant = 0; variant < 10; ++variant) {
                if (is_leaf() && variant == 7)
                    continue;
                number = 3;
                Lufia2CpuState n;
                seed(&n);
                if (variant == 0)
                    n.accumulator_is_8_bit = !n.accumulator_is_8_bit;
                else if (variant == 1)
                    SetIndexWidth(&n, 1);
                else if (variant == 2) {
                    n.accumulator_is_8_bit = !n.accumulator_is_8_bit;
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
                    n.data_bank = 0x7f;
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
        for (suite = 0; suite < 18; ++suite)
            for (unsigned variant = 0; variant < 14; ++variant) {

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
                    guest.m_flag = !guest.m_flag;
                else if (variant == 1)
                    guest.x_flag = 1;
                else if (variant == 2) {
                    guest.m_flag = !guest.m_flag;
                    guest.x_flag = 1;
                } else if (variant == 3)
                    guest.PB = 0;
                else if (variant == 4)
                    guest.DB = 0x7e;
                else if (variant == 5)
                    guest.DB = 0x7f;
                else if (variant == 6)
                    guest.D = 1;
                else if (variant == 7)
                    guest.D = 0xffff;
                else if (variant == 8)
                    guest._flag_D = 1;
                else if (variant == 9)
                    guest.emulation = 1;
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
        static const unsigned calls[] = {7, 5, 20, 9, 11, 7, 1, 1, 1,
                                         4, 0, 0,  1, 2,  5, 6, 1, 1};
        for (suite = 0; suite < 18; ++suite) {
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
        printf("PASS world child contracts:%u\n", cases);
        return 0;
    }
    unsigned total = 0;
    for (suite = 0; suite < 18; ++suite) {
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
    for (unsigned i = 0; i < 18; ++i) {
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
