#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "core/cpu_ops.h"

static void fault(unsigned line) {
    fprintf(stderr, "scene fixture invariant line%u\n", line);
    exit(1);
}

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, unwind_at, bad_at, bad_kind, native_calls,
    original_calls, visits;
static unsigned frame_reads[3];
static Lufia2CpuState native_entries[5], original_entries[5];
static const uint16_t callbacks[] = {0xba97, 0xbaa2, 0xbaf1, 0xbaf1};
static const unsigned counts[] = {2, 5, 1, 3};
static const uint32_t sites[4][5] = {{0x80ba9a, 0x80ba9e},
                                     {0x80baaa, 0x80bab0, 0x80bab8, 0x80babc, 0x80bac5},
                                     {0x80bb5b},
                                     {0x80baf8, 0x80bb20, 0x80bb3d}};
static const uint32_t targets[4][5] = {
    {0x8482e7, 0x80bf0b},
    {0x80bf0b, 0x848328, 0x808285, 0x83b007, 0x80bf0b},
    {0x80bf0b},
    {0x80bf0b, 0x8482e7, 0x80bf0b}};
static unsigned frame_size(unsigned i) {
    return targets[suite][i] == 0x80bf0b ? 2u : 3u;
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

static unsigned *clock_for(SnesVerifyBus *b) {
    return &frame_reads[b == &native ? 0 : b == &original ? 1 : 2];
}
static uint8_t scene_read(void *context, uint32_t address) {
    SnesVerifyBus *b = context;
    unsigned bank = address >> 16, low = address & 0xffffu;
    if (low == 0x40u &&
        (bank == 0x7e || bank < 0x40 || (bank >= 0x80 && bank < 0xc0)) &&
        !((*clock_for(b) += 1u) & 1u))
        ++b->wram[0x40];
    return SnesVerifyBusRead(b, address);
}
static void scene_write(void *context, uint32_t address, uint8_t value) {
    SnesVerifyBusWrite(context, address, value);
}
static void seed(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    memset(frame_reads, 0, sizeof frame_reads);
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    native_calls = original_calls = visits = 0;
    n->program_bank = 0x80;
    n->data_bank = (uint8_t)(number & 1u ? 0x7f : 0x7e);
    n->accumulator_is_8_bit = 1;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(0x9000u + number % 16u * 64u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f20u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f80u + number % 64u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    native.wram[0x40] = (uint8_t)(number * 29u);
    native.wram[0x5b5] = (number & 1u) ? 0x40 : 0;
    unsigned index = 0;
    while (index < 9u && romword(0x80ba55u + 2u * index) != callbacks[suite])
        ++index;
    if (index == 9)
        exit(2);
    if (number % 16u == 15u) {
        n->y = 0xffffu;
        n->data_bank = 0x7e;
    }
    SnesVerifyBusPoke(&native, ((uint32_t)n->data_bank << 16) | n->y, (uint8_t)index);
    unsigned next = (uint16_t)(n->y + 1u);
    if (!((int16_t)next < 0))
        next = 0x8000u;
    unsigned argument_bank = n->data_bank + (n->y == 0xffffu ? 1u : 0u);
    SnesVerifyBusPoke(&native, ((uint32_t)argument_bank << 16) | next,
                      (uint8_t)(suite == 2   ? 0
                                : suite == 3 ? number % 255u + 1u
                                             : number * 37u));
    word(0x9b9u, n->data_bank);
#ifdef RUNTIME_ABI
    n->data_bank = 0x7e;
    if (n->stack < 0x1f40)
        n->stack = 0x1f40;
    native.wram[0x99b] = native.wram[0x99c] = native.wram[0x1259] =
        native.wram[0x1d0ff] = 0;
    native.wram[0xb54] = 30;
    word(0x9b7, 0x9000);
    native.wram[0x9b9] = 0x7e;
    unsigned opcode = 0;
    while (opcode < 205u && romword(0x80ca14u + 2u * opcode) != 0xba39)
        ++opcode;
    if (opcode == 205)
        exit(2);
    native.wram[0x9000] = (uint8_t)opcode;
    native.wram[0x9001] = (uint8_t)index;
    native.wram[0x9002] = (uint8_t)(suite == 2   ? 0
                                    : suite == 3 ? number % 255u + 1u
                                                 : number * 37u);
    native.wram[0x9003] = 0x37;
    native.wram[0x9004] = 0xff;
    if (number & 1u)
        SetIndexWidth(n, 1);
#endif
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static Trace *trace_for(SnesVerifyBus *b) {
    return b == &native ? &native_trace : b == &original ? &original_trace : NULL;
}
static void instruction(SnesVerifyBus *b, Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    Trace *t = trace_for(b);
    uint8_t op = scene_read(b, pc);
    if (t)
        --t->count;
    fetch_trace = t;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][op];
    if (!fetch_left)
        fault(__LINE__);
    interp816_runOpcode(r);
    fetch_left = 0;
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
        n->program_bank = 0x83;
}
static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n, unsigned i) {
    if (targets[suite][i] == 0x80bf0b) {
        Interp816 *r = interp816_init(m->context, scene_read, scene_write);
        load(r, n, 0x80bf0b);
        unsigned steps = 0;
        while (SnesVerifyPc24(r) != 0x80bf11 && steps++ < 32)
            instruction(m->context, r);
        if (steps == 32)
            fault(__LINE__);
        instruction(m->context, r);
        store(n, r);
        interp816_free(r);
        return;
    }
    OpSta(m, n, 0x7ff100u + i * 2u);
    OpLdx(n, (uint16_t)(number * 19u));
    OpLoadA(n, (uint8_t)(number * 17u));
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(sites[suite][i] + frame_size(i)))
        fault(__LINE__);
    if (frame_size(i) == 3u && Pull8(m, n) != 0x80)
        fault(__LINE__);
    n->program_bank = 0x80;
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned ordinal = ++native_calls, i = ordinal - 1u;
    if (i >= counts[suite] || target != targets[suite][i] || site != sites[suite][i] ||
        frame != frame_size(i)) {
        fprintf(stderr,
                "scene child suite%u case%u ordinal%u got%06X/%06X/%u "
                "expected%06X/%06X/%u\n",
                suite, number, ordinal, target, site, frame, targets[suite][i],
                sites[suite][i], frame_size(i));
        fault(__LINE__);
    }
    n->program_bank = (uint8_t)(target >> 16);
    native_entries[i] = *n;
    if (ordinal == unwind_at)
        return 0;
    returned_child(context, n, i);
    corrupt(n, ordinal);
    return 1;
}
#ifdef RUNTIME_ABI
static unsigned saved_frame_reads[3], saved_calls[4];
static Lufia2CpuState saved_entries[2][5];
void RuntimeSaveProbeVisits(void) {
    memcpy(saved_frame_reads, frame_reads, sizeof frame_reads);
    saved_calls[0] = native_calls;
    saved_calls[1] = original_calls;
    saved_calls[2] = unwind_at;
    saved_calls[3] = bad_at;
    memcpy(saved_entries[0], native_entries, sizeof native_entries);
    memcpy(saved_entries[1], original_entries, sizeof original_entries);
}
void RuntimeResetProbeVisits(void) {
    frame_reads[2] = 0;
    native_calls = 0;
    unwind_at = bad_at = 0;
}
void RuntimeRestoreProbeVisits(void) {
    memcpy(frame_reads, saved_frame_reads, sizeof frame_reads);
    native_calls = saved_calls[0];
    original_calls = saved_calls[1];
    unwind_at = saved_calls[2];
    bad_at = saved_calls[3];
    memcpy(native_entries, saved_entries[0], sizeof native_entries);
    memcpy(original_entries, saved_entries[1], sizeof original_entries);
}
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    unsigned i = 0;
    while (i < counts[suite] && site != sites[suite][i])
        ++i;
    if (i == counts[suite] || frame != frame_size(i))
        fault(__LINE__);
    Lufia2Memory m = {scene_read, scene_write, b, NULL, NULL};
    n->program_bank = (uint8_t)(targets[suite][i] >> 16);
    returned_child(&m, n, i);
}
#endif
static int child_entry(Interp816 *r) {
    unsigned i = original_calls;
    if (!visits || i >= counts[suite] || SnesVerifyPc24(r) != targets[suite][i])
        return 0;
    ++original_calls;
    store(&original_entries[i], r);
    if (original_calls == unwind_at)
        return 1;
    if (targets[suite][i] != 0x80bf0b) {
        Lufia2CpuState n = {0};
        store(&n, r);
        Lufia2Memory m = {scene_read, scene_write, &original, NULL, NULL};
        returned_child(&m, &n, i);
        corrupt(&n, i + 1u);
        load(r, &n, sites[suite][i] + frame_size(i) + 1u);
        return 2;
    }
    return 0;
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
    uint32_t entry = 0x80ba39;
    Lufia2Memory m = {scene_read, scene_write, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    entry = 0x809cb8;
#endif
    load(r, &n, entry);
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    if (!number && !unwind_at && !bad_at) {
        for (unsigned i = 0; i < counts[suite]; ++i) {
            runtime_probe_desired_site = sites[suite][i];
            runtime_probe(&m, &n, Lufia2DecompBridge_9CB8, entry, 0, 3);
        }
        runtime_probe_desired_site = 0;
    }
    q = runtime_invoke(&m, &n, child, (void *)&m, Lufia2DecompBridge_9CB8,
                       Lufia2TextEngineStep, Lufia2TextEngineStepWithServices, entry, 0,
                       3);
#else
    q = Lufia2TextRunSceneOperation(&m, &n, child, (void *)&m);
#endif
    uint32_t stop = unwind_at ? targets[suite][unwind_at - 1u] : q.pc;
    unsigned steps;
    for (steps = 0; steps < 100000; ++steps) {
        uint32_t pc = SnesVerifyPc24(r);
        if (pc == 0x80ba39)
            ++visits;
        if (bad_at && original_calls == bad_at &&
            targets[suite][bad_at - 1u] == 0x80bf0b &&
            pc == sites[suite][bad_at - 1u] + 3u) {
            Lufia2CpuState changed = {0};
            store(&changed, r);
            corrupt(&changed, bad_at);
            load(r, &changed, pc);
        }
        if (pc == stop && visits && (steps || stop != entry)) {
            if (unwind_at) {
                if (child_entry(r) != 1) {
                    instruction(&original, r);
                    continue;
                }
            }
            break;
        }
        int handled = child_entry(r);
        if (handled == 1)
            fault(__LINE__);
        if (handled != 2)
            instruction(&original, r);
    }
    if (!unwind_at && !bad_at &&
        (native_calls != counts[suite] || original_calls != counts[suite])) {
        fprintf(stderr,
                "missing scene children suite%u case%u calls%u/%u stop%06X steps%u\n",
                suite, number, native_calls, original_calls, stop, steps);
        return 1;
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 100000 || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries)) {
        fprintf(stderr,
                "scene suite%u case%u unwind%u bad%u/%u PC%06X CPU%d WRAM%d "
                "traces%u/%u children%u/%u A%04X/%04X Y%04X/%04X S%04X/%04X\n",
                suite, number, unwind_at, bad_at, bad_kind, stop,
                memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, original_calls, native_calls,
                n.accumulator, r->a, n.y, r->y, n.stack, r->sp);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 3 && argc != 5) ||
        !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 5) {
        uint32_t pc = (uint32_t)strtoul(argv[3], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[4], NULL, 16);
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
    Interp816 *r = interp816_init(&original, scene_read, scene_write);
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
        for (unsigned variant = 0; variant < 15u; ++variant) {
            number = 3;
            Lufia2CpuState n;
            seed(&n);
            SetIndexWidth(&n, 0u);
            if (variant == 0u)
                n.accumulator_is_8_bit = 0;
            else if (variant == 1u)
                SetIndexWidth(&n, 1u);
            else if (variant == 2u)
                n.program_bank = 0;
            else if (variant == 3u)
                n.decimal = 1;
            else if (variant < 10u)
                n.direct_page =
                    (uint16_t[]){1, 0xff, 0x100, 0xffff, 0xffc0, 0x1ecb}[variant - 4u];
            else if (variant < 14u)
                n.stack = (uint16_t[]){0, 0x54, 0x1f1f, 0x1ffd}[variant - 10u];
            Lufia2CpuState before = n;
            before.resume_pc = 0x80ba39u;
            Lufia2Memory m = {scene_read, scene_write, &native, NULL, NULL};
            Lufia2ExecutionResult q = Lufia2TextRunSceneOperation(
                &m, &n, variant == 14u ? NULL : child, (void *)&m);
            if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
                memcmp(&before, &n, sizeof n) || native_trace.count)
                return 1;
            ++guards;
        }
#ifdef RUNTIME_ABI
        for (unsigned variant = 0; variant < 3u; ++variant) {
            number = 3;
            Lufia2CpuState n;
            seed(&n);
            Lufia2Memory m = {scene_read, scene_write, &native, NULL, NULL};
            RuntimeCase test = {0};
            test.memory = &m;
            test.entry = 0x809cb8u;
            test.start = n.stack;
            test.hrv = 3u;
            test.guard_only = 1u;
            runtime_case = &test;
            CpuState c;
            memset(&c, 0, sizeof c);
            runtime_load(&c, &n);
            c.host_return_valid = 3u;
            if (variant == 0u)
                c.m_flag = 0u;
            else if (variant == 1u)
                c._flag_D = 1u;
            else
                c.emulation = 1u;
            cpu_mirrors_to_p(&c);
            CpuState before = c;
            RecompReturn ret = Lufia2DecompBridge_9CB8(&c);
            runtime_return_reads = 0u;
            c.open_bus = before.open_bus;
            if (ret != RECOMP_RETURN_LLE_UNWIND_BASE || test.stub != 1u ||
                test.target != test.entry || test.kind != 3u ||
                memcmp(&c, &before, sizeof c) || native_trace.count)
                return 1;
            ++guards;
        }
        runtime_case = NULL;
#endif
        printf("PASS scene guards:%u\n", guards);
        return 0;
    }

#ifndef RUNTIME_ABI
    if (argc == 3 && strcmp(argv[2], "--indices") == 0) {
        for (unsigned index = 0; index < 256; ++index)
            for (unsigned flag = 0; flag < 2; ++flag) {
                number = index * 2u + flag;
                suite = 0;
                Lufia2CpuState n;
                seed(&n);
                uint32_t script = ((uint32_t)n.data_bank << 16) | n.y;
                SnesVerifyBusPoke(&native, script, (uint8_t)index);
                SnesVerifyBusPoke(&native, ((uint32_t)n.data_bank << 16) | 0x5b5u,
                                  (uint8_t)(flag ? 0x40 : 0));
                memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
                load(r, &n, 0x80ba39u);
                Lufia2Memory m = {scene_read, scene_write, &native, NULL, NULL};
                uint32_t stop = Lufia2TextExecuteCaveCommand(&m, &n, 0xba39u);
                unsigned steps = 0, dispatched = 0;
                for (; steps < 100000u; ++steps) {
                    uint32_t pc = SnesVerifyPc24(r);
                    if (dispatched && pc == stop)
                        break;
                    if (pc == 0x80ba42u)
                        dispatched = 1;
                    instruction(&original, r);
                }
                Lufia2CpuState expected = n;
                store(&expected, r);
                if (steps == 100000u || memcmp(&expected, &n, sizeof n) ||
                    memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) ||
                    !same_trace()) {
                    fprintf(stderr, "legacy index%u flag%u PC%06X\n", index, flag,
                            stop);
                    return 1;
                }
            }
        puts("PASS legacy scene byte indices512");
        return 0;
    }
#endif
    unsigned cases = 0;
    for (suite = 0; suite < 4; ++suite) {
        if (argc == 3 && strcmp(argv[2], "--unwind") == 0) {
            for (unwind_at = 1; unwind_at <= counts[suite]; ++unwind_at)
                for (number = 0; number < 8; ++number) {
                    if (run(r))
                        return 1;
                    ++cases;
                }
            unwind_at = 0;
        } else if (argc == 3 && strcmp(argv[2], "--returns") == 0) {
            for (bad_at = 1; bad_at <= counts[suite]; ++bad_at)
                for (bad_kind = 0; bad_kind < 4; ++bad_kind)
                    for (number = 0; number < 4; ++number) {
                        if (run(r))
                            return 1;
                        ++cases;
                    }
            bad_at = 0;
        } else
            for (number = 0; number < 1024; ++number) {
                if (run(r))
                    return 1;
                ++cases;
            }
    }
    printf("PASS scene CPU/fullWRAM/ordered bus/child entries:%u\n", cases);
#ifdef RUNTIME_ABI
    printf("Scene production redirects:%u,fallback:%u\n", runtime_redirects[0],
           runtime_fallback[0]);
    if (argc == 2 && (runtime_redirects[0] != 165 || runtime_fallback[0]))
        return 1;
#endif
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
