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
static unsigned suite, number;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x80ef2fu, 0x80eeebu, 0x80ed9cu};
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
    if (trace->count == 1200000u) {
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
    word(WRAM_FIELD_SECTION_WIDTH, (uint16_t)(number & 255u));
    for (unsigned slot = 0; slot < 40u; ++slot) {
        native.wram[WRAM_ACTOR_STATE + slot] = (uint8_t)((number + slot * 17u) & 7u);
        native.wram[WRAM_ACTOR_TILE_X + slot] = (uint8_t)(number * 23u + slot * 3u);
        native.wram[WRAM_ACTOR_TILE_Y + slot] = (uint8_t)(number * 17u + slot * 7u);
        native.wram[0x1e216u + slot] = (uint8_t)(number + slot);
        native.wram[0x05d2u + slot] =
            (uint8_t[]){0x71u, 0x72u, 0x73u, 0x74u, 0u, 0xffu}[(number + slot) % 6u];
    }
    if (suite == 1u) {
        cpu->x = 0x20u;
        uint16_t start =
            (uint16_t)(number & 16u ? 0xfff0u : 0x100u + number % 64u * 128u);
        unsigned count = number % 16u;
        for (unsigned i = 0; i < count; ++i) {
            uint32_t at = 0x7ef000u + (uint16_t)(start + i * 3u);
            SnesVerifyBusPoke(&native, at, (uint8_t)i);
            SnesVerifyBusPoke(&native, 0x7ef000u + (uint16_t)(start + i * 3u + 1u),
                              (uint8_t)(number * 11u + i * 3u));
            SnesVerifyBusPoke(&native, 0x7ef000u + (uint16_t)(start + i * 3u + 2u),
                              (uint8_t)(number * 13u + i * 7u));
        }
        SnesVerifyBusPoke(&native, 0x7ef000u + (uint16_t)(start + count * 3u), 0xffu);
        word(0x7ef020u, start);
    }

    if (suite == 2u) {
        memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
        cpu->accumulator = (uint16_t)(number % 4u);
        cpu->data_bank = (uint8_t)number;
        if (number & 1u)
            SetIndexWidth(cpu, 1u);
        for (unsigned layer = 0; layer < 4u; ++layer) {
            native.wram[0x1d010u + layer] = (uint8_t)(2u + (number + layer) % 6u);
            native.wram[0x1d018u + layer] = (uint8_t)(3u + (number + layer) % 5u);
            word(0x7fd008u + layer * 2u, (uint16_t)(layer * 0x1000u));
            for (unsigned cell = 0; cell < 512u; ++cell) {
                unsigned a = 0x10000u + layer * 0x1000u + cell * 2u;
                native.wram[a] = (uint8_t)(cell % 64u);
                native.wram[a + 1u] = (uint8_t)((cell + number) & 12u);
            }
        }
        word(0x7fd03eu, 0x8000u);
        for (unsigned i = 0; i < 1024u; ++i)
            native.wram[0x18000u + i] = (uint8_t)((number + i) % 32u);
        for (unsigned slot = 0; slot < 40u; ++slot) {
            native.wram[WRAM_ACTOR_STATE + slot] = (uint8_t)((number + slot) & 7u);
            native.wram[WRAM_ACTOR_TILE_X + slot] = (uint8_t)(slot + 2u);
            native.wram[WRAM_ACTOR_TILE_Y + slot] = (uint8_t)(slot + 3u);
            native.wram[0x1e216u + slot] = (uint8_t)(slot & 3u);
            native.wram[0x5d2u + slot] = (uint8_t)(0x70u + (slot % 5u));
        }
        word(0x7ef026u, 0x400u);
        word(0x7ef01eu, 0x500u);
        unsigned count = number % 4u;
        for (unsigned i = 0; i < count; ++i) {
            unsigned a = 0xf400u + i * 4u;
            native.wram[a] = 0u;
            native.wram[a + 1u] = (uint8_t)(i + 2u);
            native.wram[a + 2u] = (uint8_t)(i + 3u);
            a = 0xf500u + i * 3u;
            native.wram[a] = 0;
            native.wram[a + 1u] = (uint8_t)(i + 5u);
            native.wram[a + 2u] = (uint8_t)(i + 7u);
        }
        native.wram[0xf400u + count * 4u] = native.wram[0xf500u + count * 3u] = 0xffu;
        for (unsigned i = 0; i < 48u; ++i)
            native.wram[0x1d69cu + i] = 0xffu;
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
    Lufia2ExecutionResult result =
        suite == 2u ? Lufia2FieldBuildAttributes(&memory, &cpu)
        : suite     ? Lufia2FieldMarkListedCells(&memory, &cpu)
                    : Lufia2FieldMarkVisibleActorCells(&memory, &cpu);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return 1;
    uint32_t stop = result.pc;
    if ((number & 31u) == ((number >> 5) & 31u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        if (suite != 1u)
            cpu.program_bank = Pull8(&memory, &cpu);
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps;
    for (steps = 0; steps < 200000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 200000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "attribute%06X case%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
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
static int VerifyAttributeGuards(void);
static int VerifyListBudget(Interp816 *);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifyAttributeGuards();
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;

    if (argc == 4) {
        unsigned pc = (unsigned)strtoul(argv[2], NULL, 16),
                 at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[3], NULL, 16);
        if (at >= size || rom[at] == value)
            return 2;
        rom[at] = value;
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

    if (argc == 3 && strcmp(argv[2], "--budget") == 0) {
        suite = 1;
        number = 0;
        return VerifyListBudget(reference);
    }
    unsigned parent = argc == 3 && strcmp(argv[2], "--parent") == 0;
    unsigned returns = argc == 3 && strcmp(argv[2], "--returns") == 0;
    unsigned cases = parent      ? 512u
                     : returns   ? 1024u
                     : argc == 3 ? (unsigned)strtoul(argv[2], NULL, 10)
                                 : 32768u;
    unsigned total = 0;
    for (suite = parent ? 2u : 0u; suite < (parent ? 3u : 2u); ++suite) {
        for (unsigned n = 0; n < cases; ++n) {
            number = returns ? n * 32u + n % 32u : n;
            if (run(reference))
                return 1;
            ++total;
        }
    }
    printf("PASS%u original attribute-entry/parent/return states\n", total);
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
    uint32_t entry = which ? 0x80eeebu : 0x80ef2fu;
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
        which ? Lufia2DecompBridge_80EEEB(&host) : Lufia2DecompBridge_80EF2F(&host);
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
        which ? Lufia2FieldMarkListedCells(&memory, &state)
              : Lufia2FieldMarkVisibleActorCells(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != entry ||
        memcmp(&before, &state, sizeof state))
        return 1;
    ++guard_count;
#endif
    return 0;
}
static int VerifyAttributeGuards(void) {
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

static int VerifyListBudget(Interp816 *reference) {

    Lufia2CpuState cpu;
    seed(&cpu);
    memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
    cpu.accumulator = 0;
    cpu.x = 0x20u;
    cpu.y = 0;
    cpu.data_bank = 0;
    cpu.stack = 0x1f80u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
    load(reference, &cpu, 0x80eeebu);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};

    Lufia2ExecutionResult result = Lufia2FieldMarkListedCells(&memory, &cpu);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x80eef8u ||
        result.dispatches != 65536u)
        return 1;
    unsigned steps, visits = 0;
    for (steps = 0; steps < 3000000u; ++steps) {
        if (SnesVerifyPc24(reference) == 0x80eef8u && ++visits == 65537u)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 3000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "attribute%06X case%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
            entries[suite], number, steps, result.pc, SnesVerifyPc24(reference),
            memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, cpu.accumulator, reference->a,
            cpu.x, reference->x, cpu.y, reference->y, cpu.stack, reference->sp,
            cpu.data_bank, reference->db);
        return 1;
    }
    return 0;
}