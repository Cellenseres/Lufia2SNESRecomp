#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/world_map.h"
#include "core/cpu_ops.h"
static void fault(unsigned line) {
    fprintf(stderr, "world fixture invariant line%u\n", line);
    exit(1);
}
typedef struct Trace {
    SnesVerifyBusEvent events[400000];
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
static ChildVisit native_visits[16], original_visits[16];
static const uint32_t entries[] = {
    0x86aeb1u, 0x86d330u, 0x869934u, 0x86af3au, 0x86af88u, 0x86af90u, 0x86af98u,
    0x86afa0u, 0x86afa8u, 0x86af71u, 0x86992bu, 0x86d32du, 0x86a357u};
static const uint32_t terminals[] = {
    0x86af09u, 0x86d349u, 0x86994bu, 0x86af6cu, 0x86af6cu, 0x86af6cu, 0x86af6cu,
    0x86af6cu, 0x86af6cu, 0x86af87u, 0x86994bu, 0x86d349u, 0x86a366u};
static const Service services[] = {
    Lufia2WorldMapRunSceneSequence,      Lufia2WorldMapFinishDisplayReset,
    Lufia2WorldMapRefreshObjectPose,     Lufia2WorldMapRunSceneTrack0,
    Lufia2WorldMapRunSceneTrack1,        Lufia2WorldMapRunSceneTrack2,
    Lufia2WorldMapRunSceneTrack3,        Lufia2WorldMapRunSceneTrack4,
    Lufia2WorldMapRunSceneTrack5,        Lufia2WorldMapUpdateSceneActor,
    Lufia2WorldMapUpdateObjectHeading,   Lufia2WorldMapResetDisplayState,
    Lufia2WorldMapPositionAnimatedObject};
static const unsigned case_counts[] = {4096, 2048, 8192, 1024, 1024, 1024, 1024,
                                       1024, 1024, 1024, 4096, 2048, 2048};
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_86AEB1(CpuState *);
extern RecompReturn Lufia2DecompBridge_86D330(CpuState *);
extern RecompReturn Lufia2DecompBridge_869934(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AF3A(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AF88(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AF90(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AF98(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AFA0(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AFA8(CpuState *);
extern RecompReturn Lufia2DecompBridge_86AF71(CpuState *);
extern RecompReturn Lufia2DecompBridge_86992B(CpuState *);
extern RecompReturn Lufia2DecompBridge_86D32D(CpuState *);
extern RecompReturn Lufia2DecompBridge_86A357(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_86AEB1, Lufia2DecompBridge_86D330, Lufia2DecompBridge_869934,
    Lufia2DecompBridge_86AF3A, Lufia2DecompBridge_86AF88, Lufia2DecompBridge_86AF90,
    Lufia2DecompBridge_86AF98, Lufia2DecompBridge_86AFA0, Lufia2DecompBridge_86AFA8,
    Lufia2DecompBridge_86AF71, Lufia2DecompBridge_86992B, Lufia2DecompBridge_86D32D,
    Lufia2DecompBridge_86A357};
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
static unsigned frame_size(void) { return suite == 0u ? 3u : 2u; }
#endif
static int allowed_site(uint32_t site) {
    if (suite == 0u)
        return site == 0x86aec8 || site == 0x86aed8 || site == 0x86aef6 ||
               site == 0x86aefe || site == 0x86af01 || site == 0x86af04;
    if (suite == 1u)
        return site == 0x86d33d;
    if (suite == 2u)
        return site == 0x869948;
    if (suite < 9u)
        return site == 0x86af45 || site == 0x86af48 || site == 0x86af54 ||
               site == 0x86af69;
    if (suite == 9u)
        return site == 0x86af84;
    if (suite == 10u)
        return site == 0x869931 || site == 0x869948;
    if (suite == 11u)
        return site == 0x86d32d || site == 0x86d33d;
    return site == 0x86a359;
}
static int actual_site(uint32_t site) {
    return site == 0x86aec8 || site == 0x86aefe || site == 0x86af04 ||
           site == 0x86d33d || site == 0x869948 || site == 0x86af54 ||
           site == 0x86af84 || site == 0x869931 || site == 0x86d32d || site == 0x86a359;
}
static void seed(Lufia2CpuState *n) {
    for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
        native.wram[i] = (uint8_t)((i * 23u + number * 47u + suite * 79u) >> 3);
    memset(n, 0, sizeof *n);
    memset(native_visits, 0, sizeof native_visits);
    memset(original_visits, 0, sizeof original_visits);
    native_calls = original_calls = 0;
    n->program_bank = 0x86;
    n->data_bank =
        suite ? 0x86
              : (uint8_t[]){0, 0x7e, 0x7f, 0x80, 0x86, 0xff}[(number / 256u) % 6u];
    n->accumulator_is_8_bit = 1;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(0x1469u + number % 32u * 0x1du);
    n->y = (uint16_t)(number * 113u);
    n->stack = (uint16_t)(0x1f40u + (number & 127u));
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 6) & 1u;
    for (unsigned bank = 0; bank < 2; ++bank) {
        unsigned off = bank * 65536u;
        native.wram[off + 0x11e7] = (uint8_t)(number % 24);
        native.wram[off + 0x1242] = (uint8_t)number;
    }
    native.wram[0x54] = (uint8_t)number;
    native.wram[0x583] = (number / 256u) & 1u ? 0x8f : 0x0f;
    native.wram[0x1203] = (uint8_t)number;
    native.wram[n->x + 15u] = (uint8_t)(number / 256u);
    native.wram[n->x + 16u] = (uint8_t)number;
    if (unwind_at || bad_at) {
        native.wram[0x583] = 0x0f;
        for (unsigned slot = 0; slot < 32; ++slot)
            native.wram[0x1469u + slot * 0x1du + 15u] = 7;
        native.wram[0x1203] = 0;
    }
    word((uint32_t)n->stack + 1u, 0x8123);
    native.wram[n->stack + 3u] = 0x83;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
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
        n->data_bank = 0x7f;
}
static void pop_near(const Lufia2Memory *m, Lufia2CpuState *n, uint32_t site) {
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(site + 2u)) {
        fputs("wrong child return frame\n", stderr);
        exit(1);
    }
    n->program_bank = 0x86;
}
static void execute_original_child(const Lufia2Memory *m, Lufia2CpuState *n,
                                   uint32_t target, uint32_t site) {
    Interp816 *c = interp816_init(m->context, SnesVerifyBusRead, SnesVerifyBusWrite);
    load(c, n, target);
    uint16_t end_stack = (uint16_t)(n->stack + 2u);
    unsigned steps = 0;
    while (c->sp != end_stack || SnesVerifyPc24(c) != site + 3u) {
        if (++steps > 100000u) {
            fprintf(stderr, "child limit suite%u case%u site%06X PC%06X\n", suite,
                    number, site, SnesVerifyPc24(c));
            exit(1);
        }
        instruction(m->context, c);
    }
    store(n, c);
    interp816_free(c);
}
static uint8_t nested_child(void *context, Lufia2CpuState *n, uint32_t target,
                            uint32_t site, uint8_t frame) {
    if (frame != 2u)
        exit(1);
    execute_original_child(context, n, target, site);
    return 1;
}
static void model_child(const Lufia2Memory *m, Lufia2CpuState *n, uint32_t target,
                        uint32_t site) {
    if (site == 0x86af04 || site == 0x86af84) {
        Lufia2ExecutionResult q =
            site == 0x86af04
                ? Lufia2WorldMapFinishDisplayReset(m, n, nested_child, (void *)m)
                : Lufia2WorldMapRefreshObjectPose(m, n, nested_child, (void *)m);
        if (q.flow != LUFIA2_EXECUTION_RETURNED)
            exit(1);
        pop_near(m, n, site);
        return;
    }
    if (actual_site(site)) {
        execute_original_child(m, n, target, site);
        return;
    }
    OpSta(m, n, 0x7ff100u + (site & 31u));
    OpLdx(n, (uint16_t)(0x1469u + (number + site) % 32u * 0x1du));
    OpLdy(n, (uint16_t)(number * 17u));
    OpLoadA(n, (uint8_t)(number * 37u));
    n->carry = (number >> 1) & 1u;
    n->overflow = (number >> 2) & 1u;
    if (site == 0x86af01) {
        OpStz(m, n, OpAbs(n, 0x581u));
        OpLda(m, n, OpAbs(n, 0x581u));
    }
    pop_near(m, n, site);
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned ordinal = ++native_calls;
    if (ordinal > 16u || !allowed_site(site) || frame != 2u) {
        fprintf(stderr, "unexpected child%06X/%06X\n", site, target);
        exit(1);
    }
    unsigned opcode = native.rom[((site >> 16) & 127u) * 32768u + (site & 32767u)];
    if (opcode != (site == 0x86aef6u ? 0xfcu : 0x20u)) {
        fputs("child site is not the original JSR\n", stderr);
        exit(1);
    }
    uint32_t expected = 0x860000u | romword(site + 1u);
    if (site == 0x86aef6)
        expected = 0x860000u | romword(0x86af0au + n->x);
    if (target != expected) {
        fprintf(stderr, "wrong target%06X expected%06X site%06X\n", target, expected,
                site);
        exit(1);
    }
    n->program_bank = 0x86;
    native_visits[ordinal - 1u] = (ChildVisit){target, site, frame, *n};
    if (ordinal == unwind_at)
        return 0;
    model_child(context, n, target, site);
    corrupt(n, ordinal);
    return 1;
}
#ifdef RUNTIME_ABI
static unsigned saved_counts[4];
static ChildVisit saved_visits[2][16];
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
    if (!allowed_site(site) || frame != 2u)
        exit(1);
    uint32_t target = 0x860000u | romword(site + 1u);
    if (site == 0x86aef6)
        target = 0x860000u | romword(0x86af0au + n->x);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, b, NULL, NULL};
    n->program_bank = 0x86;
    model_child(&m, n, target, site);
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
    for (unsigned i = 0; i < 16; ++i) {
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
            native.wram[0x583] = 0x0f;
            native.wram[0x1203] = 0;
            for (unsigned slot = 0; slot < 32; ++slot)
                native.wram[0x1469u + slot * 0x1du + 15u] = 7;
            static const uint32_t all_sites[] = {
                0x86aec8, 0x86aed8, 0x86aef6, 0x86aefe, 0x86af01, 0x86af04,
                0x86d33d, 0x869948, 0x86af45, 0x86af48, 0x86af54, 0x86af69,
                0x86af84, 0x869931, 0x86d32d, 0x86a359};
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
    q = runtime_invoke(&m, &n, child, (void *)&m, bridges[suite], NULL, services[suite],
                       entries[suite], suite, frame_size());
#else
    q = services[suite](&m, &n, child, (void *)&m);
#endif
    uint32_t stop = q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND
                        ? native_visits[native_calls - 1u].target
                        : q.pc;
    unsigned steps = 0, pending = 0;
    uint32_t pending_return = 0;
    for (; steps < 1000000u; ++steps) {
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
                           q.flow == LUFIA2_EXECUTION_BOUNDARY))
            break;
        if (!pending && allowed_site(pc)) {
            instruction(&original, c);
            if (original_calls == 16u)
                exit(1);
            unsigned ordinal = ++original_calls;
            Lufia2CpuState entry = {0};
            store(&entry, c);
            uint32_t target = SnesVerifyPc24(c);
            original_visits[ordinal - 1u] = (ChildVisit){target, pc, 2u, entry};
            if (ordinal == unwind_at)
                continue;
            if (actual_site(pc)) {
                pending = 1;
                pending_return = pc + 3u;
                continue;
            }
            Lufia2Memory rm = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                               NULL};
            model_child(&rm, &entry, target, pc);
            corrupt(&entry, ordinal);
            load(c, &entry, pc + 3u);
            continue;
        }
        instruction(&original, c);
    }
    Lufia2CpuState expected = n;
    store(&expected, c);
    if (steps == 1000000u ||
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
        for (suite = 0; suite < 13; ++suite)
            for (unsigned variant = 0; variant < 10; ++variant) {
                if (!suite && variant >= 8)
                    continue;
                number = 3;
                Lufia2CpuState n;
                seed(&n);
                if (variant == 0)
                    n.accumulator_is_8_bit = 0;
                else if (variant == 1)
                    SetIndexWidth(&n, 1);
                else if (variant == 2) {
                    n.accumulator_is_8_bit = 0;
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
        for (suite = 0; suite < 13; ++suite)
            for (unsigned variant = 0; variant < 14; ++variant) {
                if (!suite && (variant == 4 || variant == 5))
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
                    guest.m_flag = 0;
                else if (variant == 1)
                    guest.x_flag = 1;
                else if (variant == 2) {
                    guest.m_flag = 0;
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
        static const unsigned calls[] = {6, 1, 1, 4, 4, 4, 4, 4, 4, 1, 2, 2, 1};
        for (suite = 0; suite < 13; ++suite) {
            if (strcmp(argv[2], "--returns") == 0) {
                if (suite == 1 || suite == 2 || suite >= 10)
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
    for (suite = 0; suite < 13; ++suite) {
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
    for (unsigned i = 0; i < 13; ++i) {
        redirects += runtime_redirects[i];
        fallbacks += runtime_fallback[i];
    }
    printf("World production redirects:%u fallback:%u\n", redirects, fallbacks);
    if (fallbacks || (!selected && redirects != 570u))
        return 1;
#endif
    interp816_free(c);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
