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

static unsigned suite, number, unwind_site, bad_site, bad_kind, rewrite_return,
    hold_count;
static uint16_t rewritten_back = 0x9d42u;
static unsigned native_calls, original_calls, handler_visits;
static unsigned native_ids[12288], original_ids[12288];
static Lufia2CpuState native_entries[12288], original_entries[12288];
static const uint16_t handlers[] = {0xa414, 0xa41f, 0xabd0};
static const uint32_t sites[] = {0x80a449, 0x80a457, 0x80a46a,
                                 0x80abd9, 0x80abe4, 0x80abec};
static const uint32_t targets[] = {0x8ec1c5, 0x82e746, 0x80a350,
                                   0x84878a, 0x84890b, 0x80a350};
static unsigned frame_at(unsigned i) { return i == 2u || i == 5u ? 2u : 3u; }
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

static unsigned seed_for(unsigned id) {
    return id < 3u ? 257u : id == 3u ? 0u : id == 4u ? 1u : 513u;
}
static void seed(Lufia2CpuState *n) {
    memset(n, 0, sizeof *n);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    native_calls = original_calls = handler_visits = 0;
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    memset(native_ids, 0, sizeof native_ids);
    memset(original_ids, 0, sizeof original_ids);
    n->program_bank = 0x80u;
    n->accumulator_is_8_bit = 1u;
    n->data_bank = number & 256u ? 0x7fu : 0x7eu;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(0x9000u + number % 16u * 64u);
    n->stack = (uint16_t)(number % 16u == 0   ? 0x1f20u
                          : number % 16u == 1 ? 0x1ffcu
                                              : 0x1f80u + number % 64u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    word(0x9b9u, n->data_bank);
    uint32_t script = ((uint32_t)n->data_bank << 16) | n->y;
    SnesVerifyBusPoke(&native, script, (uint8_t)(suite < 2u ? number * 37u : number));
    SnesVerifyBusPoke(&native, script + 1u, (uint8_t)number);
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
    unsigned op = 0;
    for (; op < 205u; ++op)
        if (romword(0x80ca14u + 2u * op) == handlers[suite])
            break;
    if (op == 205u)
        exit(2);
    native.wram[0x9000] = (uint8_t)op;
    native.wram[0x9001] = (uint8_t)(suite < 2u ? number * 37u : number);
    native.wram[0x9002] = (uint8_t)number;
    unsigned count = suite < 2u ? 2u : 1u;
    native.wram[0x9001 + count] = 0x37u;
    native.wram[0x9002 + count] = 0xffu;
#endif
    if (number & 1024u)
        native.wram[0x1d4f2u] = 0;
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

static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n, unsigned id,
                           unsigned ordinal) {
    OpSta(m, n, 0x7ff110u + id * 2u);
    OpLoadA(n, (uint16_t)(number * 17u + ordinal));
    OpLdx(n, (uint16_t)(number * 31u + ordinal));
    OpLdy(n, (uint16_t)(number * 43u + ordinal * 71u));
    n->carry = (number + ordinal) & 1u;
    n->overflow = (number >> 2) & 1u;
    if (id == 1u) {
        Write16Absolute(m, n, 0x9cfu, number & 256u ? 0xffffu : 0x0123u);
        if (hold_count)
            Write8(m, 0x7fd4f1u, 2u);
    }
    if (id == 4u)
        Write8(m, OpDp(n, 0xaeu), number & 512u ? 0x80u : 0u);
    if (id == 2u || id == 5u)
        n->data_bank = 0x80u;
    if (rewrite_return && id == 0u && ordinal == 1u) {
        uint16_t low = (uint16_t)(n->stack + frame_at(id) + 3u);
        Write8(m, low, (uint8_t)rewritten_back);
        Write8(m, (uint16_t)(low + 1u), (uint8_t)(rewritten_back >> 8));
    }
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(sites[id] + frame_at(id)))
        exit(2);
    if (frame_at(id) == 3u && Pull8(m, n) != 0x80u)
        exit(2);
    n->program_bank = 0x80u;
    if (id + 1u == bad_site) {
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
    unsigned id = 0;
    for (; id < 6u; ++id)
        if (site == sites[id])
            break;
    if (id == 6u || target != targets[id] || frame != frame_at(id) ||
        native_calls == 12288u)
        exit(2);
    n->program_bank = (uint8_t)(target >> 16);
    native_ids[native_calls] = id;
    native_entries[native_calls++] = *n;
    if (id + 1u == unwind_site)
        return 0u;
    returned_child(context, n, id, native_calls);
    return 1u;
}
#ifdef RUNTIME_ABI
static unsigned saved_calls[3];
static Lufia2CpuState saved_entries[12288];
static unsigned saved_ids[12288];
void RuntimeSaveProbeVisits(void) {
    saved_calls[0] = native_calls;
    saved_calls[1] = unwind_site;
    saved_calls[2] = bad_site;
    memcpy(saved_entries, native_entries, sizeof saved_entries);
    memcpy(saved_ids, native_ids, sizeof saved_ids);
}
void RuntimeResetProbeVisits(void) {
    native_calls = 0;
    unwind_site = bad_site = 0;
}
void RuntimeRestoreProbeVisits(void) {
    native_calls = saved_calls[0];
    unwind_site = saved_calls[1];
    bad_site = saved_calls[2];
    memcpy(native_entries, saved_entries, sizeof saved_entries);
    memcpy(native_ids, saved_ids, sizeof saved_ids);
}
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    unsigned id = 0;
    for (; id < 6u; ++id)
        if (site == sites[id])
            break;
    if (id == 6u || frame != frame_at(id))
        exit(2);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, b, NULL, NULL};
    n->program_bank = (uint8_t)(targets[id] >> 16);
    returned_child(&m, n, id, native_calls + 1u);
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
    if (!unwind_site && !bad_site && !rewrite_return && !hold_count &&
        (number == 257u || number == 0u || number == 1u || number == 513u)) {
        unsigned begin = suite < 2u ? 0u : 3u, end = suite < 2u ? 3u : 6u;
        for (unsigned id = begin; id < end; ++id) {
            if (number != seed_for(id))
                continue;
            runtime_probe_desired_site = sites[id];
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
    q = suite < 2u ? Lufia2TextRunMenuCommand(&m, &n, handlers[suite], child, &m)
                   : Lufia2TextRunCaveTransition(&m, &n, child, &m);
#endif
    if (bad_kind == 0u &&
        ((suite < 2u && bad_site == 2u && q.pc != 0x80a45fu) ||
         (suite < 2u && bad_site == 3u && (number & 1024u) && q.pc != 0x80a479u) ||
         (suite == 2u && bad_site == 6u && q.pc != 0x80abefu))) {
        fprintf(stderr, "unreached M0 fence:%06X\n", q.pc);
        return 1;
    }
    uint32_t stop = q.pc;
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[unwind_site - 1u];
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(r);
        if (pc == (0x800000u | handlers[suite]))
            ++handler_visits;
        if (pc == stop && handler_visits && original_calls == native_calls)
            break;
        unsigned id = 0;
        for (; id < 6u; ++id)
            if (pc == sites[id])
                break;
        if (id < 6u) {
            instruction(&original, r);
            if (SnesVerifyPc24(r) != targets[id] || original_calls == 12288u)
                exit(2);
            original_ids[original_calls] = id;
            store(&original_entries[original_calls++], r);
            if (id + 1u == unwind_site)
                break;
            Lufia2CpuState c = {0};
            store(&c, r);
            Lufia2Memory om = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                               NULL};
            returned_child(&om, &c, id, original_calls);
            load(r, &c, sites[id] + frame_at(id) + 1u);
            continue;
        }
        instruction(&original, r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 1000000u || !handler_visits || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries) ||
        memcmp(native_ids, original_ids, sizeof native_ids)) {
        fprintf(
            stderr,
            "reward/cave%04X case%u unwind%u bad%u/%u rewrite%u hold%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u calls%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X M%u/%u X%u/%u\n",
            handlers[suite], number, unwind_site, bad_site, bad_kind, rewrite_return,
            hold_count, steps, stop, SnesVerifyPc24(r), memcmp(&expected, &n, sizeof n),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, native_calls, original_calls,
            n.accumulator, r->a, n.x, r->x, n.y, r->y, n.stack, r->sp, n.data_bank,
            r->db, n.accumulator_is_8_bit, r->mf, n.index_is_8_bit, r->xf);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int verify_guards(void);
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 4) {
        unsigned pc = (unsigned)strtoul(argv[2], NULL, 16),
                 at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
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

    if (argc == 3 && strcmp(argv[2], "--return-boundaries") == 0) {
        static const uint16_t landings[] = {0,      0xffff, 0x9cff, 0x9d00,
                                            0x9db0, 0xa41c, 0xa428, 0x8000};
        rewrite_return = 1;
        number = 1;
        for (suite = 0; suite < 2; ++suite)
            for (unsigned i = 0; i < 8; ++i) {
                rewritten_back = (uint16_t)(landings[i] - 1u);
                if (run(r))
                    return 1;
            }
        puts("PASS reward near-return boundary targets:16");
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return verify_guards();
    unsigned total = 0, unwinds = 0, contexts = 0, rewrites = 0;
    for (suite = 0; suite < 3u; ++suite) {
        for (number = 0; number < 1024u; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        unsigned begin = suite < 2u ? 0u : 3u, end = suite < 2u ? 3u : 6u;
        for (unsigned id = begin; id < end; ++id) {
            unwind_site = id + 1u;
            for (unsigned k = 0; k < 8u; ++k) {
                number = seed_for(id) + k * 1024u;
                if (run(r))
                    return 1;
                ++unwinds;
            }
            unwind_site = 0;
            bad_site = id + 1u;
            for (bad_kind = 0; bad_kind < 6u; ++bad_kind)
                for (unsigned k = 0; k < 8u; ++k) {
                    number = seed_for(id) + k * 1024u;
                    if (run(r))
                        return 1;
                    ++contexts;
                }
            bad_site = 0;
        }
        if (suite < 2u) {
            rewrite_return = 1u;
            for (number = 1; number < 33u; ++number) {
                if (run(r))
                    return 1;
                ++rewrites;
            }
            rewrite_return = 0;
            hold_count = 1;
            number = 1;
            if (run(r))
                return 1;
            hold_count = 0;
        }
        printf(
            "PASS reward/cave%04X cumulative matrix%u unwinds%u contexts%u rewritten returns%u\n",
            handlers[suite], total, unwinds, contexts, rewrites);
        fflush(stdout);
    }
#ifdef RUNTIME_ABI
    unsigned redirects = 0;
    for (unsigned i = 0; i < 3u; ++i) {
        if (runtime_fallback[i])
            return 1;
        redirects += runtime_redirects[i];
    }
    printf("PASS actual ABI redirects:%u\n", redirects);
#endif
    printf(
        "PASS reward/cave batch:%u states/%u unwinds/%u contexts/%u rewritten returns/2 loop budgets\n",
        total, unwinds, contexts, rewrites);
    return 0;
}

static int verify_guards(void) {
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    unsigned total = 0;
    number = 3;
    suite = 0;
    Lufia2CpuState pristine;
    seed(&pristine);
    static const uint16_t known[] = {0xa32b, 0xa372, 0xa51d, 0xbd99,
                                     0xb909, 0xa414, 0xa41f};
    for (unsigned h = 0; h < 65536u; ++h) {
        unsigned i = 0;
        for (; i < 7u; ++i)
            if (h == known[i])
                break;
        if (i < 7u)
            continue;
        Lufia2CpuState n = pristine;
        Lufia2CpuState before = n;
        Lufia2ExecutionResult q =
            Lufia2TextRunMenuCommand(&m, &n, (uint16_t)h, child, &m);
        if (q.flow != LUFIA2_EXECUTION_RETURNED || q.pc ||
            memcmp(&n, &before, sizeof n) || native_trace.count)
            return 1;
        ++total;
    }
    for (unsigned h = 0; h < 65536u; ++h) {
        if (h == 0xabd0u || h == 0xba39u)
            continue;
        Lufia2CpuState n = pristine;
        Lufia2CpuState before = n;
        if (Lufia2TextExecuteCaveCommand(&m, &n, (uint16_t)h) ||
            memcmp(&n, &before, sizeof n) || native_trace.count)
            return 1;
        ++total;
    }
    for (suite = 0; suite < 3u; ++suite)
        for (unsigned k = 0; k < 15u; ++k) {
            Lufia2CpuState n;
            seed(&n);
            if (k == 0)
                n.accumulator_is_8_bit = 0;
            else if (k == 1)
                SetIndexWidth(&n, 1);
            else if (k == 2)
                n.program_bank = 0;
            else if (k == 3)
                n.decimal = 1;
            else if (k < 10)
                n.direct_page =
                    (uint16_t[]){1, 0xff, 0x100, 0xffff, 0xffc0, 0x1ecb}[k - 4];
            else if (k < 14)
                n.stack = (uint16_t[]){0, 0x54, suite < 2u ? 0x1f0fu : 0x1f1fu,
                                       0x1ffd}[k - 10];
            Lufia2CpuState before = n;
            before.resume_pc = 0x800000u | handlers[suite];
            Lufia2ExecutionResult q =
                suite < 2u
                    ? Lufia2TextRunMenuCommand(&m, &n, handlers[suite],
                                               k == 14 ? NULL : child, &m)
                    : Lufia2TextRunCaveTransition(&m, &n, k == 14 ? NULL : child, &m);
            if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
                memcmp(&n, &before, sizeof n) || native_trace.count)
                return 1;
            ++total;
        }
    puts("PASS reward/cave pre-access and unknown guards:131108");
    return total == 131108u ? 0 : 1;
}
