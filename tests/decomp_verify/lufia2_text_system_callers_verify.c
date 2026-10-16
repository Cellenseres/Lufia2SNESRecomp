#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/text.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
#include "text/text_internal.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, unwind_at, bad_at, bad_kind;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static unsigned native_calls, original_calls;
static Lufia2CpuState native_entries[4], original_entries[4];
static const uint16_t handlers[] = {0xb871, 0xb8b1, 0xb887, 0xb8bd,
                                    0xb8c7, 0xb8dc, 0xaddc, 0x9d4c};
static const unsigned counts[] = {2, 1, 2, 1, 1, 1, 1, 1};
static const uint32_t sites[8][2] = {
    {0x80b877, 0x80b87b}, {0x80b8b1}, {0x80b892, 0x80b896}, {0x80b8c0},
    {0x80b8c7},           {0x80b8dc}, {0x809dac},           {0x809dac}};
static const uint32_t targets[8][2] = {
    {0x8096cc, 0x809692}, {0x809528}, {0x80969f, 0x848311}, {0x848775},
    {0x8096f2},           {0x8095f0}, {0x80ef2f},           {0x80ef2f}};
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

static uint32_t address(const Lufia2CpuState *cpu, uint16_t at) {
    return ((uint32_t)cpu->data_bank << 16) | at;
}
static void controlled_return(const Lufia2Memory *memory, Lufia2CpuState *cpu,
                              unsigned call) {
    unsigned index = call - 1u;
    OpSta(memory, cpu, 0x7ff110u + index * 2u);
    OpLoadA(cpu, (uint16_t)(number * 17u + index));
    OpLdx(cpu, (uint16_t)(number * 31u + index));
    cpu->carry = (number + index) & 1u;
    cpu->overflow = (number >> 2) & 1u;
    uint16_t back = Pull8(memory, cpu);
    back |= (uint16_t)Pull8(memory, cpu) << 8;
    if (back != (uint16_t)(sites[suite][index] + 3u))
        abort();
    cpu->program_bank = Pull8(memory, cpu);
    if (bad_at && call == bad_at) {
        if (bad_kind == 0)
            cpu->accumulator_is_8_bit = 0;
        if (bad_kind == 1)
            SetIndexWidth(cpu, 1u);
        if (bad_kind == 2)
            cpu->direct_page = 0x100u;
        if (bad_kind == 3)
            cpu->decimal = 1;
        if (bad_kind == 4)
            cpu->data_bank ^= 1u;
        if (bad_kind == 5)
            cpu->program_bank = 0x86u;
        if (bad_kind == 6)
            ++cpu->stack;
    }
}
static uint8_t child(void *context, Lufia2CpuState *cpu, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned index = native_calls++;
    if (index >= counts[suite] || site != sites[suite][index] ||
        target != targets[suite][index] || frame != 3u)
        abort();
    cpu->program_bank = (uint8_t)(target >> 16);
    native_entries[index] = *cpu;
    if (native_calls == unwind_at)
        return 0u;
    controlled_return(context, cpu, native_calls);
    return 1u;
}
static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    native_calls = original_calls = 0;
    cpu->program_bank = 0x80u;
    cpu->data_bank =
        (uint8_t[]){0u, 0x7eu, 0x7fu, 0x80u, 0x83u, 0x81u}[(number >> 8) % 6u];
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 83u);
    cpu->y = (uint16_t)((cpu->data_bank == 0x7eu || cpu->data_bank == 0x7fu ? 0x9000u
                                                                            : 0x1800u) +
                        number % 8u * 32u);
    cpu->stack = (uint16_t)(0x1f80u + number % 64u);
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xcfu));
    cpu->accumulator_is_8_bit = 1u;
    cpu->index_is_8_bit = cpu->decimal = 0;
    uint8_t operand = (uint8_t)(number * 37u);
    for (unsigned bank = 0; bank < 2u; ++bank) {
        uint32_t a = 0x7e0000u + bank * 65536u;
        SnesVerifyBusPoke(&native, a + 0x099du,
                          (uint8_t)(operand + ((number & 7u) != 0u)));
        SnesVerifyBusPoke(&native, a + 0x099cu,
                          (uint8_t[]){0, 1, 0x80, 0x81}[(number >> 2) & 3u]);
        SnesVerifyBusPoke(&native, a + 0x09a7u, (uint8_t)(number & 2u));
        SnesVerifyBusPoke(&native, a + 0x1254u, 0u);
        SnesVerifyBusPoke(&native, a + WRAM_SCREEN_EFFECTS, (uint8_t)(number & 0x40u));
        word(a + WRAM_FIELD_LAYER_TABLE_OFFSET, 0u);
        word(a + WRAM_FIELD_LAYER_SCROLL_X, (uint16_t)(number * 41u));
        word(a + WRAM_FIELD_LAYER_SCROLL_Y, (uint16_t)(number * 67u));
    }
    word(0x7fddaeu, (uint16_t)(number * 41u + 128u + (number & 1u)));
    word(0x7fde3eu, (uint16_t)(number * 67u + 112u));
    SnesVerifyBusPoke(&native, address(cpu, cpu->y), operand);
    SnesVerifyBusPoke(&native, address(cpu, 0x9b9u), cpu->data_bank);
    if (unwind_at || bad_at) {
        SnesVerifyBusPoke(&native, address(cpu, 0x099du), (uint8_t)(operand + 1u));
        SnesVerifyBusPoke(&native, address(cpu, 0x099cu), 0x80u);
        SnesVerifyBusPoke(&native, address(cpu, WRAM_SCREEN_EFFECTS), 0x40u);
    }

    cpu->data_bank = number & 1u ? 0x7eu : 0u;
    unsigned opcode = 0;
    for (; opcode < 205u; ++opcode) {
        unsigned at = 0x4a14u + 2u * opcode;
        unsigned target = native.rom[at] | ((unsigned)native.rom[at + 1u] << 8);
        if (target == handlers[suite])
            break;
    }
    if (opcode == 205u)
        abort();
    unsigned operands = (suite == 2u || suite == 3u || suite == 4u) ? 1u : 0u;
    native.wram[0x9000u] = (uint8_t)opcode;
    native.wram[0x9001u] = operand;
    native.wram[0x9001u + operands] = 0x37u;
    native.wram[0x9002u + operands] = 0xffu;
    native.wram[0x99bu] = suite == 7u ? 1u : 0u;
    native.wram[0x1259u] = native.wram[0x1d0ffu] = 0;
    native.wram[0xb54u] = 30u;
    native.wram[0x5b5u] = 0;
    word(0x9b7u, 0x9000u);
    native.wram[0x9b9u] = 0x7eu;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, 0x809cb8u);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result =
        Lufia2TextEngineStepWithServices(&memory, &cpu, child, &memory);
    uint32_t stop = result.pc;
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[suite][unwind_at - 1u];
    unsigned steps, visits = 0u;
    for (steps = 0; steps < 200000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(reference);
        unsigned index = original_calls;
        if (pc == (0x800000u | handlers[suite]))
            ++visits;
        if (index < counts[suite] && pc == targets[suite][index]) {
            ++original_calls;
            store(&original_entries[index], reference);
            if (original_calls == unwind_at)
                break;
            Lufia2CpuState state = {0};
            store(&state, reference);
            Lufia2Memory bus = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                                NULL};
            controlled_return(&bus, &state, original_calls);
            load(reference, &state,
                 ((uint32_t)state.program_bank << 16) |
                     (uint16_t)(sites[suite][index] + 4u));
            continue;
        }
        if (pc == stop && visits)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 200000u || !visits || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries)) {
        fprintf(
            stderr,
            "system%04X case%u unwind%u bad%u/%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u calls%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X M%u/%u X%u/%u\n",
            handlers[suite], number, unwind_at, bad_at, bad_kind, steps, stop,
            SnesVerifyPc24(reference), memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, native_calls, original_calls,
            cpu.accumulator, reference->a, cpu.x, reference->x, cpu.y, reference->y,
            cpu.stack, reference->sp, cpu.data_bank, reference->db,
            cpu.accumulator_is_8_bit, reference->mf, cpu.index_is_8_bit, reference->xf);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int VerifySystemGuards(void);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifySystemGuards();
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

    unsigned total = 0,
             cases = argc == 3 ? (unsigned)strtoul(argv[2], NULL, 10) : 1024u;
    for (suite = 0; suite < 8u; ++suite) {
        for (number = 0; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        for (unwind_at = 1; unwind_at <= counts[suite]; ++unwind_at)
            for (number = 0; number < 32u; ++number) {
                if (run(reference))
                    return 1;
                ++total;
            }
        unwind_at = 0;
        for (bad_at = 1; bad_at <= counts[suite]; ++bad_at)
            for (bad_kind = 0; bad_kind < 7u; ++bad_kind)
                for (number = 0; number < 32u; ++number) {
                    if (run(reference))
                        return 1;
                    ++total;
                }
        bad_at = 0;
        printf("PASS system%04X states%u cumulative%u\n", handlers[suite], cases,
               total);
        fflush(stdout);
    }
    printf(
        "PASS%u whole TextEngine original-ROM states;CPU/fullWRAM/orderedbus/real child frames\n",
        total);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text/text_internal.h"
#include "core/cpu_ops.h"
static unsigned checked;
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
static uint8_t poison_child(void *context, Lufia2CpuState *cpu, uint32_t target,
                            uint32_t site, uint8_t frame) {
    (void)context;
    (void)cpu;
    (void)target;
    (void)site;
    (void)frame;
    fputs("guard child\n", stderr);
    exit(1);
}
static int check(uint16_t handler, unsigned variant, unsigned unknown) {
    const Lufia2Memory memory = {poison_read, poison_write, NULL, NULL, NULL};
    Lufia2CpuState state = {0};
    state.program_bank = 0x80u;
    state.accumulator_is_8_bit = 1u;
    state.stack = 0x1f80u;
    state.accumulator = 0x1234u;
    state.x = 0xabcdu;
    state.y = 0x9000u;
    state.data_bank = 0x7eu;
    state.carry = state.overflow = state.irq_disable = 1;
    Lufia2PushedChildCall child = poison_child;
    switch (variant) {
    case 0:
        child = NULL;
        break;
    case 1:
        state.program_bank = 0u;
        break;
    case 2:
        state.program_bank = 0x7eu;
        break;
    case 3:
        state.program_bank = 0x86u;
        break;
    case 4:
        state.program_bank = 0xffu;
        break;
    case 5:
        state.accumulator_is_8_bit = 0u;
        break;
    case 6:
        state.index_is_8_bit = 1u;
        break;
    case 7:
        state.decimal = 1u;
        break;
    case 8:
        state.direct_page = 1u;
        break;
    case 9:
        state.direct_page = 0x100u;
        break;
    case 10:
        state.direct_page = 0xffffu;
        break;
    case 11:
        state.stack = 0u;
        break;
    case 12:
        state.stack = 0x1f0fu;
        break;
    case 13:
        state.stack = 0x1ffdu;
        break;
    case 14:
        state.stack = 0xffffu;
        break;
    }
    Lufia2CpuState expected = state;
    Lufia2ExecutionResult result =
        Lufia2TextRunSystemCommand(&memory, &state, handler, child, NULL);
    if (!unknown)
        expected.resume_pc = 0x800000u | handler;
    if (memcmp(&expected, &state, sizeof state) ||
        result.flow !=
            (unknown ? LUFIA2_EXECUTION_RETURNED : LUFIA2_EXECUTION_BOUNDARY) ||
        result.pc != (unknown ? 0u : 0x800000u | handler))
        return 1;
    ++checked;
    return 0;
}
static int VerifySystemGuards(void) {
    static const uint16_t guard_handlers[] = {0xb871, 0xb8b1, 0xb887, 0xb8bd,
                                        0xb8c7, 0xb8dc, 0xaddc};
    for (unsigned handler = 0; handler < 65536u; ++handler) {
        unsigned known = 0;
        for (unsigned i = 0; i < 7u; ++i)
            known |= handler == guard_handlers[i];
        if (!known && check((uint16_t)handler, 15u, 1u))
            return 1;
    }
    for (unsigned i = 0; i < 7u; ++i)
        for (unsigned variant = 0; variant < 15u; ++variant)
            if (check(guard_handlers[i], variant, 0u))
                return 1;
    printf("PASS%u unsupported-handler/context pre-access guards\n", checked);
    return 0;
}
