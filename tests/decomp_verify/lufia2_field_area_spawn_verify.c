#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, edge_mode;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x80d533u, 0x80d583u};
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
    if (trace->count == 12000000u) {
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

static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    cpu->program_bank = 0x80u;
    cpu->data_bank = (uint8_t)number;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 83u);
    cpu->y = (uint16_t)(number * 59u);
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xcfu));
    cpu->accumulator_is_8_bit = 1u;
    cpu->index_is_8_bit = cpu->decimal = 0;

    for (unsigned i = 0; i < WRAM_RANDOM_TABLE_COUNT; ++i)
        native.wram[WRAM_RANDOM_TABLE + i] = (uint8_t)(number * 13u + i * 29u);
    native.wram[WRAM_RANDOM_NEXT_INDEX] = (uint8_t)(number % 256u);
    if (!suite) {
        native.wram[0] = (uint8_t)(number < 256u ? number
                                                 : (uint8_t[]){1, 2, 3, 5, 8, 16, 32,
                                                               0}[number & 7u]);
        native.wram[1] = (uint8_t)(number * 37u);
        native.wram[0x9f] = (uint8_t)(number * 13u);
        native.wram[0xa0] = (uint8_t)(number * 17u);
        native.wram[0xa1] = (uint8_t)(number * 31u);
        native.wram[0xa2] = (uint8_t)(number * 43u);
        native.wram[0xa7] = (uint8_t)(number % 40u);
        word(0xa9u, (uint16_t)(number * 2u));
        unsigned occupied = (number >> 3) % 33u;
        for (unsigned slot = 0; slot < 32u; ++slot)
            native.wram[WRAM_OBJECT_STATE + slot] =
                (uint8_t)(slot < occupied ? 0x80u : 0u);
    }
    if (edge_mode && !suite) {
        native.wram[0] = (uint8_t)(number % 17u == 0u ? 0u : 1u);
        native.wram[0xa7] = (uint8_t)number;
        word(0xa9u, (uint16_t[]){0u, 1u, 0x21ffu, 0x4200u, 0x7fffu, 0xfffcu, 0xfffeu,
                                 0xffffu}[number % 8u]);
        for (unsigned slot = 0; slot < 32u; ++slot)
            native.wram[WRAM_OBJECT_STATE + slot] = 0x80u;
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    native.wram[(uint16_t)(cpu->stack + 3u)] = (uint8_t)(0x80u + (number & 7u));
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, entries[suite]);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result = suite
                                       ? Lufia2FieldRandomAreaCoordinate(&memory, &cpu)
                                       : Lufia2FieldSpawnActorsInArea(&memory, &cpu);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return 1;
    uint32_t stop = result.pc;
    if (force_return || (!edge_mode && number % 33u == 0u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        if (!suite)
            cpu.program_bank = Pull8(&memory, &cpu);
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps;
    for (steps = 0; steps < 2000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 2000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "area%06X case%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
            entries[suite], number, steps, stop, SnesVerifyPc24(reference),
            memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, cpu.accumulator, reference->a,
            cpu.x, reference->x, cpu.y, reference->y, cpu.stack, reference->sp,
            cpu.data_bank, reference->db);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int VerifyAreaGuards(void);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifyAreaGuards();
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc > 4) {
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        if (at >= size)
            return 2;
        rom[at] = (uint8_t)strtoul(argv[4], NULL, 16);
    }
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *reference =
        interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
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

    edge_mode = argc == 3 && strcmp(argv[2], "--edges") == 0;
    unsigned total = 0;
    force_return = argc == 3 && strcmp(argv[2], "--returns") == 0;
    unsigned cases = edge_mode      ? 512u
                     : force_return ? 256u
                     : argc > 2     ? (unsigned)strtoul(argv[2], NULL, 10)
                                    : 4096u;
    for (suite = 0; suite < (edge_mode ? 1u : 2u); ++suite) {
        unsigned limit = suite ? cases * 4u : cases;
        for (number = 0; number < limit; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS area%06X states%u cumulative%u\n", entries[suite], limit, total);
        fflush(stdout);
    }
    printf(
        "PASS%u whole area-spawn/coordinate states with actual RTS/RTL compositions\n",
        total);
    return 0;
}

static uint8_t poison_read(void *context, uint32_t at) {
    (void)context;
    fprintf(stderr, "guard read%06X\n", at);
    exit(1);
}
static void poison_write(void *context, uint32_t at, uint8_t value) {
    (void)context;
    (void)value;
    fprintf(stderr, "guard write%06X\n", at);
    exit(1);
}
static unsigned guard_count, consumer_guards;
static int check_guard(unsigned which, Lufia2CpuState state, unsigned emulation) {
    const Lufia2Memory memory = {poison_read, poison_write, NULL, NULL, NULL};
    uint32_t entry = which ? 0x80d583u : 0x80d533u;
#ifdef RUNTIME_ABI
    CpuState host;
    memset(&host, 0, sizeof host);
    runtime_load(&host, &state);
    host.emulation = (uint8_t)emulation;
    host.host_return_valid = 0;
    CpuState before = host;
    RuntimeCase test = {0};
    test.memory = &memory;
    test.start = state.stack;
    test.entry = entry;
    test.guard_only = 1;
    runtime_case = &test;
    RecompReturn result =
        which ? Lufia2DecompBridge_80D583(&host) : Lufia2DecompBridge_80D533(&host);
    uint32_t target = ((uint32_t)state.program_bank << 16) | (uint16_t)entry;
    if (result != RECOMP_RETURN_LLE_UNWIND_BASE || test.stub != 1u || test.kind != 3u ||
        test.target != target || test.source != target ||
        memcmp(&before, &host, sizeof host))
        return 1;
    ++consumer_guards;
#else
    (void)emulation;
    Lufia2CpuState before = state;
    before.resume_pc = entry;
    Lufia2ExecutionResult result =
        which ? Lufia2FieldRandomAreaCoordinate(&memory, &state)
              : Lufia2FieldSpawnActorsInArea(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != entry ||
        memcmp(&before, &state, sizeof state))
        return 1;
    ++guard_count;
#endif
    return 0;
}
static int VerifyAreaGuards(void) {
    Lufia2CpuState valid = {0};
    valid.program_bank = 0x80u;
    valid.data_bank = 0x7eu;
    valid.accumulator_is_8_bit = 1u;
    valid.stack = 0x1f80u;
    valid.accumulator = 0x1234u;
    valid.x = 0xabcdu;
    valid.y = 0x9876u;
    valid.carry = valid.irq_disable = 1u;
    for (unsigned which = 0; which < 2u; ++which) {
        for (unsigned stack = 0; stack < 65536u; ++stack)
            if (stack < 0x1f00u || stack > 0x1ffcu) {
                Lufia2CpuState state = valid;
                state.stack = (uint16_t)stack;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        for (unsigned page = 1; page < 65536u; ++page) {
            Lufia2CpuState state = valid;
            state.direct_page = (uint16_t)page;
            if (check_guard(which, state, 0u))
                return 1;
        }
        for (unsigned bank = 0; bank < 256u; ++bank)
            if (bank != 0x80u) {
                Lufia2CpuState state = valid;
                state.program_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        unsigned limit = 8u;
#ifdef RUNTIME_ABI
        limit = 16u;
#endif
        for (unsigned flags = 0; flags < limit; ++flags)
            if (flags != 1u) {
                Lufia2CpuState state = valid;
                state.accumulator_is_8_bit = (uint8_t)(flags & 1u);
                state.index_is_8_bit = (uint8_t)((flags >> 1) & 1u);
                state.decimal = (uint8_t)((flags >> 2) & 1u);
                if (check_guard(which, state, flags >> 3))
                    return 1;
            }
    }
    printf("PASS semantic%u/actualproducer%u pre-access guards\n", guard_count,
           consumer_guards);
    return 0;
}
