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

static unsigned suite, number, unwind_at, bad_at, bad_kind;
static unsigned native_calls, original_calls, handler_visits;
static Lufia2CpuState native_entries[4], original_entries[4];
static const uint16_t handlers[] = {0xac6a, 0xbdbb, 0xbbeb, 0xbdca};
static const unsigned call_counts[] = {1, 2, 2, 2};
static const uint32_t sites[4][4] = {
    {0x80ac6b}, {0x80bdbd, 0x80bdc1}, {0x80bbf5, 0x80bbf9}, {0x80bdd4, 0x80bdd8}};
static const uint32_t targets[4][4] = {
    {0x8ebcad}, {0x80f821, 0x838e4c}, {0x80bfaa, 0x83b76e}, {0x80bfaa, 0x83b76e}};
static unsigned frame_at(unsigned i) {
    (void)i;
    return 3u;
}
#ifdef RUNTIME_ABI
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
    native_calls = original_calls = handler_visits = 0;
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    n->program_bank = 0x80u;
    n->accumulator_is_8_bit = 1u;
    n->data_bank = (uint8_t[]){0, 0x7e, 0x7f, 0x80, 0x83, 0x81}[(number / 16u) % 6u];
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(0x9000u + number % 16u * 64u);
    if (number % 16u == 15u)
        n->y = 0xffffu;
    n->stack = (uint16_t)(number % 16u == 0   ? 0x1f10u
                          : number % 16u == 1 ? 0x1ffcu
                                              : 0x1f80u + number % 64u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    word(0x9b9u, n->data_bank);
    SnesVerifyBusPoke(&native, ((uint32_t)n->data_bank << 16) | n->y,
                      (uint8_t)(number * 37u));
#ifdef RUNTIME_ABI
    n->data_bank = 0x7eu;
    n->stack = 0x1f80u + (number % 64u);
    n->y = (uint16_t)(number * 59u);
    native.wram[0x99b] = native.wram[0x99c] = native.wram[0x1259] =
        native.wram[0x1d0ff] = 0;
    native.wram[0xb54] = 30u;
    native.wram[0x5b5] = 0;
    word(0x9b7u, 0x9000u);
    native.wram[0x9b9u] = 0x7eu;
    unsigned opcode = 0;
    for (; opcode < 205u; ++opcode)
        if (romword(0x80ca14u + 2u * opcode) == handlers[suite])
            break;
    if (opcode == 205u)
        exit(2);
    native.wram[0x9000] = (uint8_t)opcode;
    unsigned count = suite >= 2u ? 1u : 0u;
    native.wram[0x9001] = (uint8_t)(number * 37u);
    native.wram[0x9001u + count] = 0x37u;
    native.wram[0x9002u + count] = 0xffu;
#endif
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
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
static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n, unsigned i) {
    OpSta(m, n, 0x7ff110u + i * 2u);
    OpLoadA(n, (uint16_t)(number * 17u + i));
    OpLdx(n, (uint16_t)(number * 31u + i));
    OpLdy(n, (uint16_t)(number * 43u + i * 71u));
    n->carry = (number + i) & 1u;
    n->overflow = (number >> 2) & 1u;
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(sites[suite][i] + frame_at(i)))
        exit(2);
    if (frame_at(i) == 3u && Pull8(m, n) != 0x80u)
        exit(2);
    n->program_bank = 0x80u;
    if (i + 1u == bad_at) {
        if (bad_kind == 0)
            n->accumulator_is_8_bit = 0;
        if (bad_kind == 1)
            SetIndexWidth(n, 1);
        if (bad_kind == 2)
            n->direct_page = 0x100u;
        if (bad_kind == 3)
            n->decimal = 1;
        if (bad_kind == 4)
            n->data_bank = 0;
        if (bad_kind == 5) {
            SetIndexWidth(n, 1);
            n->data_bank = 0x83u;
        }
    }
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned i = native_calls++;
    if (i >= call_counts[suite] || target != targets[suite][i] ||
        site != sites[suite][i] || frame != frame_at(i))
        exit(2);
    n->program_bank = (uint8_t)(target >> 16);
    native_entries[i] = *n;
    if (i + 1u == unwind_at)
        return 0u;
    returned_child(context, n, i);
    return 1u;
}
#ifdef RUNTIME_ABI
static unsigned saved_calls[3];
static Lufia2CpuState saved_entries[4];
void RuntimeSaveProbeVisits(void) {
    saved_calls[0] = native_calls;
    saved_calls[1] = unwind_at;
    saved_calls[2] = bad_at;
    memcpy(saved_entries, native_entries, sizeof saved_entries);
}
void RuntimeResetProbeVisits(void) {
    native_calls = 0;
    unwind_at = bad_at = 0;
}
void RuntimeRestoreProbeVisits(void) {
    native_calls = saved_calls[0];
    unwind_at = saved_calls[1];
    bad_at = saved_calls[2];
    memcpy(native_entries, saved_entries, sizeof saved_entries);
}
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    unsigned i = 0;
    for (; i < call_counts[suite]; ++i)
        if (site == sites[suite][i])
            break;
    if (i == call_counts[suite] || frame != frame_at(i))
        exit(2);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, b, NULL, NULL};
    n->program_bank = (uint8_t)(targets[suite][i] >> 16);
    returned_child(&m, n, i);
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

static int run(Interp816 *r) {
    Lufia2CpuState n;
    seed(&n);
    uint32_t entry = 0x800000u | handlers[suite];
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    entry = 0x809cb8u;
    if (!number && !unwind_at && !bad_at) {
        for (unsigned i = 0; i < call_counts[suite]; ++i) {
            runtime_probe_desired_site = sites[suite][i];
            runtime_probe(&m, &n, Lufia2DecompBridge_9CB8, entry, suite, 3u);
        }
        runtime_probe_desired_site = 0;
    }
#endif
    load(r, &n, entry);
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    q = runtime_invoke(&m, &n, child, &m, Lufia2DecompBridge_9CB8, Lufia2TextEngineStep,
                       Lufia2TextEngineStepWithServices, entry, suite, 3u);
#else
    q = Lufia2TextRunFieldCommand(&m, &n, handlers[suite], child, &m);
#endif
    uint32_t stop = q.pc;
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[suite][unwind_at - 1u];
    unsigned steps;
    for (steps = 0; steps < 20000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(r);
        if (pc == (0x800000u | handlers[suite]))
            ++handler_visits;
        unsigned i = original_calls;
        if (i < call_counts[suite] && pc == targets[suite][i]) {
            ++original_calls;
            store(&original_entries[i], r);
            if (i + 1u == unwind_at)
                break;
            Lufia2CpuState c = {0};
            store(&c, r);
            Lufia2Memory om = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                               NULL};
            returned_child(&om, &c, i);
            load(r, &c, sites[suite][i] + frame_at(i) + 1u);
            continue;
        }
        if (pc == stop && handler_visits)
            break;
        instruction(&original, r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 20000u || !handler_visits || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries)) {
        fprintf(
            stderr,
            "menu%04X case%u unwind%u bad%u/%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u calls%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X M%u/%u X%u/%u\n",
            handlers[suite], number, unwind_at, bad_at, bad_kind, steps, stop,
            SnesVerifyPc24(r), memcmp(&expected, &n, sizeof n),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, native_calls, original_calls,
            n.accumulator, r->a, n.x, r->x, n.y, r->y, n.stack, r->sp, n.data_bank,
            r->db, n.accumulator_is_8_bit, r->mf, n.index_is_8_bit, r->xf);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int VerifyFieldGuards(int argc, char **argv);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifyFieldGuards(2, argv);
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 4) {
        unsigned pc = (unsigned)strtoul(argv[2], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t v = (uint8_t)strtoul(argv[3], NULL, 16);
        if (rom[at] == v)
            return 2;
        rom[at] = v;
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

    unsigned total = 0, unwinds = 0, returns = 0;
    for (suite = 0; suite < 4u; ++suite) {
        unsigned cases = 4096u;
#ifdef RUNTIME_ABI
        cases = 1024u;
#endif
        for (number = 0; number < cases; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        for (unwind_at = 1; unwind_at <= call_counts[suite]; ++unwind_at)
            for (number = 0; number < 8u; ++number) {
                if (run(r))
                    return 1;
                ++unwinds;
            }
        unwind_at = 0;
        for (bad_at = 1; bad_at <= call_counts[suite]; ++bad_at)
            for (bad_kind = 0; bad_kind < 6u; ++bad_kind)
                for (number = 0; number < 8u; ++number) {
                    if (run(r))
                        return 1;
                    ++returns;
                }
        bad_at = 0;
        printf("PASS field%04X matrix%u cumulative%u unwinds%u contexts%u\n",
               handlers[suite], cases, total, unwinds, returns);
        fflush(stdout);
    }
#ifdef RUNTIME_ABI
    unsigned redirects = 0;
    for (unsigned i = 0; i < 4u; ++i) {
        if (runtime_fallback[i])
            return 1;
        redirects += runtime_redirects[i];
    }
    printf("PASS actual ABI redirects:%u\n", redirects);
#endif
    printf("PASS field caller batch:%u states/%u unwinds/%u changed contexts\n", total,
           unwinds, returns);
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}

static unsigned guard_count;
static int CheckGuard(Lufia2CpuState *state, unsigned handler, bool missing_child) {
    Lufia2CpuState before = *state;
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    native_trace.count = 0;
    Lufia2ExecutionResult q = Lufia2TextRunFieldCommand(
        &memory, state, (uint16_t)handler, missing_child ? NULL : child, &memory);
    before.resume_pc = 0x800000u | handler;
    if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
        memcmp(state, &before, sizeof before) || native_trace.count || native_calls) {
        fprintf(stderr, "Field guard%u handler%04X accesses%u children%u\n",
                guard_count, handler, native_trace.count, native_calls);
        return 1;
    }
    ++guard_count;
    return 0;
}
static int VerifyFieldGuards(int argc, char **argv) {
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
    valid.accumulator_is_8_bit = 1;
    valid.stack = 0x1f80;
    valid.y = 0x9000;
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    for (unsigned h = 0; h < 65536u; ++h) {
        if (h == 0xac6a || h == 0xbdbb || h == 0xbbeb || h == 0xbdca)
            continue;
        Lufia2CpuState n = valid, before = n;
        Lufia2ExecutionResult q =
            Lufia2TextRunFieldCommand(&memory, &n, (uint16_t)h, child, &memory);
        if (q.flow != LUFIA2_EXECUTION_RETURNED || q.pc ||
            memcmp(&n, &before, sizeof n) || native_trace.count || native_calls)
            return 1;
        ++guard_count;
    }
    for (suite = 0; suite < 4u; ++suite) {
        unsigned h = handlers[suite];
        for (unsigned pb = 0; pb < 256u; ++pb) {
            if (pb == 0x80u)
                continue;
            Lufia2CpuState n = valid;
            n.program_bank = (uint8_t)pb;
            if (CheckGuard(&n, h, false))
                return 1;
        }
        for (unsigned dp = 1; dp < 65536u; ++dp) {
            Lufia2CpuState n = valid;
            n.direct_page = (uint16_t)dp;
            if (CheckGuard(&n, h, false))
                return 1;
        }
        for (unsigned sp = 0; sp < 65536u; ++sp) {
            if (sp >= 0x1f10u && sp <= 0x1ffcu)
                continue;
            Lufia2CpuState n = valid;
            n.stack = (uint16_t)sp;
            if (CheckGuard(&n, h, false))
                return 1;
        }
        for (unsigned k = 0; k < 4u; ++k) {
            Lufia2CpuState n = valid;
            if (k == 0)
                n.accumulator_is_8_bit = 0;
            else if (k == 1)
                SetIndexWidth(&n, 1);
            else if (k == 2)
                n.decimal = 1;
            if (CheckGuard(&n, h, k == 3))
                return 1;
        }
    }
    printf("PASS field pre-access and unknown guards:%u\n", guard_count);
    return 0;
}
