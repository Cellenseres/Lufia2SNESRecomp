#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "system/wram.h"
#include "lufia2/system.h"
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[4096];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;

static unsigned suite, number, whole, unwind_at, bad_at, clock_edges, rti_mode,
    real_rti_count;
static Interp816 *native_rti;
static unsigned checkpoint_native_count, checkpoint_original_count;
static unsigned native_calls, original_calls;
static Lufia2CpuState checkpoint_native_state, checkpoint_original_state;
static Lufia2CpuState native_entries[5], original_entries[5];
static const uint32_t entries[] = {0x808638, 0x80864c, 0x808653,
                                   0x80865a, 0x808661, 0x808664};
static const uint32_t sites[] = {0x808649, 0x808650, 0x808657, 0x80865e, 0x808661};
static const uint32_t targets[] = {0x808703, 0x800067, 0x80006c, 0x8087a7, 0x8086c1};
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
    if (trace->count == 4096u) {
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
    memset(n, 0, sizeof *n);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    n->program_bank = 0x80;
    n->data_bank = (uint8_t)(number >> 8);
    n->accumulator = (uint16_t)(number * 197u + 0x1234u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(number * 59u);
    n->accumulator_is_8_bit = (uint8_t)(!whole && suite != 0u);
    SetIndexWidth(n, n->accumulator_is_8_bit);
    n->stack = (uint16_t[]){0x1f00, 0x1f80, 0x1ff3, 0x1fca}[(number >> 6) & 3u];
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    native.wram[0x6a] = (number & 0x100u) ? 0x83u : 0u;
    native.wram[0x6f] = (number & 0x200u) ? 0x86u : 0u;
    native.wram[0x71] = (number & 0x400u) ? 0x81u : 0u;
    native.wram[0x81] = (uint8_t)(number >> 11);
    if (whole && (unwind_at || bad_at))
        native.wram[0x6a] = native.wram[0x6f] = native.wram[0x71] = 1;
    static const uint8_t edge[] = {0, 1, 58, 59, 60, 127, 128, 254, 255};
    for (unsigned bank = 0; bank < 2u; ++bank) {
        unsigned a = bank * 65536u;
        native.wram[a + 0xb50] = (uint8_t)number;
        native.wram[a + 0xb4f] = edge[(number >> 8) % 9u];
        native.wram[a + 0xb4e] = edge[((number >> 8) / 9u) % 9u];
        native.wram[a + 0xb4d] = (uint8_t)((number >> 8) * 73u);
    }
    native.wram[0x520] = (uint8_t)((number >> 3) & 1u);
    word(0x46u, number % 8u == 0 ? 0x3030u : (uint16_t)(number * 19u));
    for (unsigned i = 1; i <= 12u; ++i)
        native.wram[n->stack + i] = (uint8_t)(number * 37u + i * 71u);
    native.wram[n->stack + 8u] = (uint8_t)(number >> 8);
    native.wram[n->stack + 9u] = (uint8_t)(number >> 5);
    native.wram[n->stack + 10u] = 0x10;
    native.wram[n->stack + 11u] = 0x90;
    native.wram[n->stack + 12u] = 0x83;
    if (clock_edges) {
        n->data_bank = 0x80;
        native.wram[0x520] = 0;
        native.wram[0xb50] = native.wram[0xb4f] = native.wram[0xb4e] = 59;
        native.wram[0xb4d] = (uint8_t)number;
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
    checkpoint_native_count = checkpoint_original_count = native_calls =
        original_calls = 0;
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
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

static void checkpoint(void *context, Lufia2CpuState *n, uint32_t pc) {
    (void)context;
    if (pc != 0x808699u)
        exit(2);
    checkpoint_native_state = *n;
    ++checkpoint_native_count;
}
static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n, unsigned index) {
    OpSta(m, n, 0x7ff110u + index * 2u);
    OpLoadA(n, (uint16_t)(number * 17u + index));
    OpLdx(n, (uint16_t)(number * 31u + index));
    OpLdy(n, (uint16_t)(number * 43u + index));
    n->carry = (number + index) & 1u;
    n->overflow = (number >> 2) & 1u;
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(sites[index] + 2u))
        exit(2);
    if (index + 1u == bad_at)
        n->accumulator_is_8_bit = 0;
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned i = 0;
    for (; i < 5u; ++i)
        if (site == sites[i])
            break;
    if (i == 5u || target != targets[i] || frame != 2u)
        exit(2);
    ++native_calls;
    native_entries[i] = *n;
    if (i + 1u == unwind_at)
        return 0;
    returned_child(context, n, i);
    return 1;
}
static int run(Interp816 *r) {
    Lufia2CpuState n;
    seed(&n);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    uint32_t entry = whole ? 0x808638u : entries[suite];
    load(r, &n, entry);
    Lufia2ExecutionResult q = whole
                                  ? Lufia2MainNmi(&m, &n, child, checkpoint, &m)
                                  : Lufia2MainNmiResume(&m, &n, entry, checkpoint, &m);
    uint32_t stop = q.pc;
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[unwind_at - 1u];
    unsigned steps;
    for (steps = 0; steps < 2000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(r);
        if (pc == stop && !(whole && q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND))
            break;
        if (pc == 0x808699u) {
            checkpoint_original_state = checkpoint_native_state;
            store(&checkpoint_original_state, r);
            ++checkpoint_original_count;
        }
        if (whole) {
            unsigned i = 0;
            for (; i < 5u; ++i)
                if (pc == targets[i])
                    break;
            if (i < 5u) {
                ++original_calls;
                original_entries[i] = native_entries[i];
                store(&original_entries[i], r);
                if (i + 1u == unwind_at)
                    break;
                Lufia2CpuState c = n;
                store(&c, r);
                Lufia2Memory om = {SnesVerifyBusRead, SnesVerifyBusWrite, &original,
                                   NULL, NULL};
                returned_child(&om, &c, i);
                load(r, &c, sites[i] + 3u);
                continue;
            }
        }
        instruction(&original, r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 2000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries) ||
        checkpoint_native_count != checkpoint_original_count ||
        (checkpoint_native_count &&
         memcmp(&checkpoint_native_state, &checkpoint_original_state,
                sizeof checkpoint_native_state))) {
        fprintf(
            stderr,
            "NMI phase%u whole%u case%u unwind%u bad%u steps%u PC%06X/%06X CPU%d WRAM%d bus%u/%u calls%u/%u checkpoint%u/%u A%04X/%04X S%04X/%04X M%u/%u X%u/%u\n",
            suite, whole, number, unwind_at, bad_at, steps, stop, SnesVerifyPc24(r),
            memcmp(&expected, &n, sizeof n),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, native_calls, original_calls,
            checkpoint_native_count, checkpoint_original_count, n.accumulator, r->a,
            n.stack, r->sp, n.accumulator_is_8_bit, r->mf, n.index_is_8_bit, r->xf);
        return 1;
    }
    if (rti_mode && stop == 0x8086bfu) {
        load(native_rti, &n, stop);
        instruction(&native, native_rti);
        instruction(&original, r);
        Lufia2CpuState a = n, b = n;
        store(&a, native_rti);
        store(&b, r);
        if (SnesVerifyPc24(r) != 0x839010u || SnesVerifyPc24(native_rti) != 0x839010u ||
            memcmp(&a, &b, sizeof a) ||
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace())
            return 1;
        ++real_rti_count;
    }
    return 0;
}
static uint8_t GuardRead(void *context, uint32_t address) {
    (void)context;
    (void)address;
    abort();
}
static void GuardWrite(void *context, uint32_t address, uint8_t value) {
    (void)context;
    (void)address;
    (void)value;
    abort();
}
static unsigned guard_cases;
static int GuardState(const Lufia2CpuState *input, uint32_t entry) {
    Lufia2Memory memory = {GuardRead, GuardWrite, NULL, NULL, NULL};
    Lufia2CpuState cpu = *input;
    cpu.resume_pc = entry;
    Lufia2CpuState expected = cpu;
    Lufia2ExecutionResult result =
        Lufia2MainNmiResume(&memory, &cpu, entry, NULL, NULL);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != entry ||
        memcmp(&cpu, &expected, sizeof cpu))
        return 1;
    ++guard_cases;
    return 0;
}
static int VerifyGuards(void) {
    for (unsigned phase = 0; phase < 6; ++phase) {
        Lufia2CpuState cpu = {0};
        cpu.program_bank = 0x80;
        cpu.data_bank = 0x80;
        cpu.stack = 0x1f80;
        cpu.accumulator_is_8_bit = cpu.index_is_8_bit = phase != 0;
        for (unsigned value = 0; value < 65536; ++value) {
            if (value) {
                cpu.direct_page = (uint16_t)value;
                if (GuardState(&cpu, entries[phase]))
                    return 1;
                cpu.direct_page = 0;
            }
            if (value < 0x1f00 || value > 0x1ff3) {
                cpu.stack = (uint16_t)value;
                if (GuardState(&cpu, entries[phase]))
                    return 1;
                cpu.stack = 0x1f80;
            }
        }
        for (unsigned value = 0; value < 256; ++value)
            if (value != 0x80) {
                cpu.program_bank = (uint8_t)value;
                if (GuardState(&cpu, entries[phase]))
                    return 1;
                cpu.program_bank = 0x80;
            }
        for (unsigned mode = 0; mode < 4; ++mode) {
            cpu.accumulator_is_8_bit = mode & 1u;
            cpu.index_is_8_bit = (mode >> 1) & 1u;
            if (mode != (phase ? 3u : 0u) && GuardState(&cpu, entries[phase]))
                return 1;
        }
        cpu.accumulator_is_8_bit = cpu.index_is_8_bit = phase != 0;
        cpu.decimal = 1;
        if (GuardState(&cpu, entries[phase]))
            return 1;
    }
    Lufia2CpuState cpu = {0};
    cpu.program_bank = 0x80;
    cpu.stack = 0x1f80;
    for (unsigned low = 0; low < 65536; ++low) {
        unsigned known = 0;
        for (unsigned i = 0; i < 6; ++i)
            known |= 0x800000u + low == entries[i];
        if (!known && GuardState(&cpu, 0x800000u + low))
            return 1;
    }
    printf("PASS NMI core guards:%u pre-access states\n", guard_cases);
    return 0;
}

int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifyGuards();
    if (argc == 4) {
        uint32_t pc = (uint32_t)strtoul(argv[2], NULL, 16);
        unsigned offset = ((pc >> 16) & 0x7fu) * 0x8000u + (pc & 0x7fffu);
        if (!(pc & 0x8000u) || offset >= size)
            return 2;
        rom[offset] = (uint8_t)strtoul(argv[3], NULL, 16);
    }
    if (!SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *r = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    native_rti = interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
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

    if (argc == 3 && strcmp(argv[2], "--rti") == 0) {
        rti_mode = 1;
        suite = 5;
        for (unsigned i = 0; i < 256u; ++i) {
            number = (i << 8) | 59u;
            if (run(r))
                return 1;
        }
        clock_edges = 1;
        for (number = 0; number < 1024u; ++number)
            if (run(r))
                return 1;
        printf("PASS actual interpreted RTI after native finish:%u compositions\n",
               real_rti_count);
        return real_rti_count == 1280u ? 0 : 1;
    }
    if (argc == 3 && strcmp(argv[2], "--main") == 0) {
        whole = 1;
        for (number = 0; number < 4096u; ++number)
            if (run(r))
                return 1;
        for (unwind_at = 1; unwind_at <= 5u; ++unwind_at)
            for (number = 0; number < 16u; ++number)
                if (run(r))
                    return 1;
        unwind_at = 0;
        for (bad_at = 1; bad_at <= 5u; ++bad_at)
            for (number = 0; number < 16u; ++number)
                if (run(r))
                    return 1;
        puts("PASS original MainNmi:4096 states,80 child unwinds,80 M0 boundaries");
        return 0;
    }
    unsigned total = 0;
    for (suite = 0; suite < 6u; ++suite) {
        for (number = 0; number < 65536u; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        printf(
            "PASS NMI phase%u:65536 original ROM CPU/fullWRAM/orderedbus/checkpoint states\n",
            suite);
        fflush(stdout);
    }
    clock_edges = 1;
    suite = 5;
    for (number = 0; number < 1024u; ++number)
        if (run(r))
            return 1;
    printf("PASS NMI phases:%u states+1024 all-hour rollover edges\n", total);
    return 0;
}
