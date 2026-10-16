#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/text.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
#include "text/text_internal.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, unwind_at, bad_at, bad_kind, legacy, low_stack;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static unsigned native_calls, original_calls;
static Lufia2CpuState native_entries[4], original_entries[4];
static const uint32_t entries[] = {0x80a80fu, 0x80b397u, 0x80b4c4u, 0x80b849u};
static const uint32_t targets[] = {0x80b404u, 0x8093feu, 0x848328u, 0x848311u};
static const uint32_t sites[] = {0x80b3f1u, 0x80b862u, 0x80b866u, 0x80b86au};
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

static unsigned child_index(uint32_t target) {
    for (unsigned i = 0; i < 4u; ++i)
        if (target == targets[i])
            return i;
    return 4u;
}
static void controlled_return(const Lufia2Memory *memory, Lufia2CpuState *cpu,
                              unsigned index, unsigned call) {
    OpSta(memory, cpu, 0x7ff110u + index * 2u);
    if (!index)
        Write8(memory, OpAbs(cpu, WRAM_TEXT_WAIT_ACTOR),
               (uint8_t)(number & 1u ? 0xffu : 0u));
    OpLoadA(cpu, (uint16_t)(number * 17u + index));
    OpLdx(cpu, (uint16_t)(number * 31u + index));
    cpu->carry = (number + index) & 1u;
    cpu->overflow = (number >> 2) & 1u;
    uint16_t back = Pull8(memory, cpu);
    back |= (uint16_t)Pull8(memory, cpu) << 8;
    if (back != (uint16_t)(sites[index] + 3u))
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
    unsigned index = child_index(target), call = native_calls++;
    if (index == 4u || site != sites[index] || frame != 3u || call == 4u)
        abort();
    cpu->program_bank = (uint8_t)(target >> 16);
    native_entries[call] = *cpu;
    if (native_calls == unwind_at)
        return 0u;
    controlled_return(context, cpu, index, native_calls);
    return 1u;
}
static uint32_t data_address(const Lufia2CpuState *cpu, uint16_t at) {
    return ((uint32_t)cpu->data_bank << 16) | at;
}
static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    native_calls = original_calls = 0;
    cpu->program_bank = 0x80u;
    cpu->data_bank = (uint8_t[]){0u, 0x7eu, 0x80u, 0x83u}[(number >> 8) & 3u];
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 83u);
    cpu->y = cpu->data_bank == 0x7eu ? (uint16_t)(0x9000u + number % 8u * 32u)
                                     : (uint16_t)(0x1800u + number % 8u * 32u);
    cpu->accumulator_is_8_bit = 1u;
    cpu->stack = (uint16_t[]){0x1f23u, 0x1f40u, 0x1f80u, 0x1ffcu}[(number >> 10) & 3u];
    if (low_stack)
        cpu->stack = (uint16_t)(0x1f00u + number % 35u);
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xcfu));
    cpu->accumulator_is_8_bit = 1u;
    cpu->index_is_8_bit = 0;
    cpu->decimal = 0;
    cpu->stack = (uint16_t)(0x1f80u + number % 64u);
    unsigned mode = number % 16u;
    uint8_t first = (uint8_t)(number % 3u), second = (uint8_t)(number % 5u),
            third = (uint8_t)(number * 37u);
    for (unsigned bank = 0u; bank < 2u; ++bank) {
        uint32_t base = 0x7e0000u + bank * 65536u;
        for (unsigned slot = 0; slot < 40u; ++slot) {
            SnesVerifyBusPoke(&native, base + WRAM_ACTOR_ID + slot,
                              (uint8_t)(slot < 5u ? slot : 0xffu));
            SnesVerifyBusPoke(
                &native, base + WRAM_ACTOR_STATE + slot,
                (uint8_t[]){0, 8, 0x80, 0x88}[(number / 16u + slot) % 4u]);
            SnesVerifyBusPoke(&native, base + WRAM_ACTOR_TILE_X + slot,
                              (uint8_t)(slot + 3u));
            SnesVerifyBusPoke(&native, base + WRAM_ACTOR_TILE_Y + slot,
                              (uint8_t)(slot + 7u));
        }
        SnesVerifyBusPoke(&native, base + WRAM_TEXT_WAIT_ACTOR,
                          (uint8_t)(mode < 12u ? 0xffu : second));
        SnesVerifyBusPoke(&native, base + 0x099du,
                          (uint8_t)(first + (mode < 12u ? 1u : 0u)));
        SnesVerifyBusPoke(&native, base + 0x099cu, mode == 11u ? 0x40u : 0u);
        SnesVerifyBusPoke(&native, base + WRAM_SCREEN_EFFECTS, (uint8_t)(number * 13u));
        word(base + WRAM_FIELD_LAYER_TABLE_OFFSET,
             (uint16_t)(2u * ((number >> 4) & 3u)));
        for (unsigned layer = 0u; layer < 8u; layer += 2u) {
            word(base + WRAM_FIELD_LAYER_SCROLL_X + layer,
                 (uint16_t)(number * 41u + layer));
            word(base + WRAM_FIELD_LAYER_SCROLL_Y + layer,
                 (uint16_t)(number * 67u + layer));
        }
    }
    word(0x7ef012u, 0x100u);
    word(0x7ef00eu, 0x200u);
    for (unsigned i = 0; i < 3u; ++i) {
        unsigned a = 0x7ef100u + i * 6u;
        SnesVerifyBusPoke(&native, a, (uint8_t)i);
        SnesVerifyBusPoke(&native, a + 3u, (uint8_t)(number * 17u));
        word(a + 4u, (uint16_t)(number * 31u));
        a = 0x7ef200u + i * 3u;
        SnesVerifyBusPoke(&native, a, (uint8_t)i);
        SnesVerifyBusPoke(&native, a + 1u, (uint8_t)(number * 11u));
        SnesVerifyBusPoke(&native, a + 2u, (uint8_t)(number * 19u));
    }
    SnesVerifyBusPoke(&native, 0x7ef112u, 0xffu);
    SnesVerifyBusPoke(&native, 0x7ef209u, 0xffu);
    word(0x7fddaeu, (uint16_t)(number * 41u + 128u));
    word(0x7fde3eu, (uint16_t)(number * 67u + 112u));
    word(0x7fd08bu, (uint16_t)(number * 41u));
    word(0x7fd08du, (uint16_t)(number * 67u));
    if (suite == 2u) {
        first = (uint8_t)(number & 255u);
        second = (uint8_t)(number >> 4);
        if (mode == 0u)
            second = 0xffu;
        else if (mode == 1u)
            second = 0u;
        if (mode == 2u)
            first = 0x60u | (first & 15u);
        else if (mode == 3u)
            first = 0x70u | (first & 15u);
    }
    if (unwind_at || bad_at) {
        SnesVerifyBusPoke(&native, data_address(cpu, WRAM_TEXT_WAIT_ACTOR), 0xffu);
        SnesVerifyBusPoke(&native, data_address(cpu, 0x099du), (uint8_t)(first + 1u));
        SnesVerifyBusPoke(&native, data_address(cpu, 0x099cu), 0u);
    }
    SnesVerifyBusPoke(&native, data_address(cpu, cpu->y), first);
    SnesVerifyBusPoke(&native, data_address(cpu, (uint16_t)(cpu->y + 1u)), second);
    SnesVerifyBusPoke(&native, data_address(cpu, (uint16_t)(cpu->y + 2u)), third);
    SnesVerifyBusPoke(&native, data_address(cpu, 0x9b9u), cpu->data_bank);

    unsigned operand_count =
        suite == 0u   ? 2u
        : suite == 1u ? 0u
        : suite == 2u ? ((first & 0xf0u) == 0x60u && second != 0xffu ? 3u : 2u)
                      : 1u;
    native.wram[0x9000u] = (uint8_t[]){0x33u, 0x3cu, 0x41u, 0x4bu}[suite];
    native.wram[0x9001u] = first;
    native.wram[0x9002u] = second;
    native.wram[0x9003u] = third;
    native.wram[0x9001u + operand_count] = 0x37u;
    native.wram[0x9002u + operand_count] = 0xffu;
    native.wram[0x99bu] = native.wram[0x99cu] = native.wram[0x1259u] =
        native.wram[0x1d0ffu] = 0;
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
        Lufia2TextEngineStepWithServices(&memory, &cpu, legacy ? NULL : child, &memory);
    uint32_t stop = result.pc;
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[suite == 1u ? 0u : unwind_at];
    unsigned steps, visits = 0u;
    for (steps = 0u; steps < 200000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(reference);
        unsigned index = child_index(pc);
        if (pc == entries[suite])
            ++visits;
        if (index < 4u &&
            ((suite == 1u && index == 0u) || (suite == 3u && index > 0u))) {
            unsigned call = original_calls++;
            if (call >= 4u)
                abort();
            store(&original_entries[call], reference);
            if (original_calls == unwind_at)
                break;
            Lufia2CpuState state = {0};
            store(&state, reference);
            Lufia2Memory bus = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                                NULL};
            controlled_return(&bus, &state, index, original_calls);
            load(reference, &state,
                 ((uint32_t)state.program_bank << 16) | (uint16_t)(sites[index] + 4u));
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
            "text%06X case%u legacy%u low%u unwind%u bad%u/%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u calls%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X M%u/%u X%u/%u\n",
            entries[suite], number, legacy, low_stack, unwind_at, bad_at, bad_kind,
            steps, stop, SnesVerifyPc24(reference), memcmp(&expected, &cpu, sizeof cpu),
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
int main(int argc, char **argv) {
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

    unsigned total = 0u;
    for (suite = 0u; suite < 4u; ++suite) {
        for (number = 0; number < 1024u; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        legacy = 1u;
        for (number = 0; number < 128u; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        legacy = 0u;
    }
    for (suite = 1u; suite < 4u; suite += 2u) {
        unsigned children = suite == 1u ? 1u : 3u;
        for (unwind_at = 1; unwind_at <= children; ++unwind_at)
            for (number = 0; number < 32u; ++number) {
                if (run(reference))
                    return 1;
                ++total;
            }
        unwind_at = 0u;
        for (bad_at = 1; bad_at <= children; ++bad_at)
            for (bad_kind = 0; bad_kind < 7u; ++bad_kind)
                for (number = 0; number < 32u; ++number) {
                    if (run(reference))
                        return 1;
                    ++total;
                }
        bad_at = 0u;
    }
    printf(
        "PASS%u complete TextEngine states,legacy boundaries,child-unwinds,changed returns\n",
        total);
    return 0;
}
