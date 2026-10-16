#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "field/event_script_internal.h"
#include "text/text_internal.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, caller_kind;
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

static void script_byte(uint32_t at, uint8_t value);
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
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    native.wram[(uint16_t)(cpu->stack + 3u)] = (uint8_t)(0x80u + (number & 7u));

    cpu->data_bank = number & 1u ? 0x7eu : 0x80u;
    cpu->y = cpu->data_bank == 0x7eu ? 0x9000u : 0xe000u;
    cpu->stack = (uint16_t)(0x1f80u + (number & 63u));
    uint32_t script = ((uint32_t)cpu->data_bank << 16) | cpu->y;
    unsigned count = number & 255u, identifier = (number * 37u) & 127u;
    if (caller_kind) {
        word(0x7ef014u, 0x0200u);
        native.wram[0xf200u] = 0u;
        word(0x7ef201u, (uint16_t)(number * 29u));
        word(0x7ef203u, (uint16_t)(number * 47u));
        native.wram[0xf205u] = 0xffu;
        script_byte(script, 0u);
        script_byte(script + 1u, (uint8_t)count);
        script_byte(script + 2u, (uint8_t)identifier);
        native.wram[TEXT_SCRIPT_BANK] = cpu->data_bank;
    } else {
        unsigned index = number % 27u;
        native.wram[(EVENT_POINT_X & 0x1ffffu) + index] = (uint8_t)(number * 13u);
        native.wram[(EVENT_POINT_Y & 0x1ffffu) + index] = (uint8_t)(number * 17u);
        native.wram[(EVENT_POINT_D223 & 0x1ffffu) + index] = (uint8_t)(number * 31u);
        native.wram[(EVENT_POINT_D263 & 0x1ffffu) + index] = (uint8_t)(number * 43u);
        script_byte(script, (uint8_t)(0xe0u + index));
        script_byte(script + 1u, (uint8_t)count);
        script_byte(script + 2u, (uint8_t)identifier);
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
}
static uint8_t spawn_child(void *context, Lufia2CpuState *cpu, uint32_t target,
                           uint32_t site, uint8_t frame) {
    if (target != 0x80d533u || site != 0x80afdfu || frame != 3u)
        abort();
    const Lufia2Memory *memory = context;
    cpu->program_bank = 0x80u;
    Lufia2ExecutionResult result = Lufia2FieldSpawnActorsInArea(memory, cpu);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        abort();
    SimulateRtlFrame(memory, cpu);
    cpu->program_bank = 0x80u;
    return 1u;
}
static void script_byte(uint32_t at, uint8_t value) {
    if (SnesVerifyBusPoke(&native, at, value))
        return;
    unsigned offset = ((at >> 16) & 127u) * 32768u + (at & 32767u);
    if (offset >= native.rom_size)
        abort();
    ((uint8_t *)native.rom)[offset] = value;
}
static Lufia2ExecutionResult run_caller(const Lufia2Memory *memory,
                                        Lufia2CpuState *cpu) {
    if (caller_kind)
        return Lufia2TextRunRenderCommand(memory, cpu, 0xafc0u, spawn_child,
                                          (void *)memory);
    uint32_t handoff = 0;
    unsigned flow =
        Lufia2EventActorOpcode(memory, cpu, EVENT_OP_SPAWN_IN_AREA, NULL, &handoff);
    if (flow != EVENT_OPCODE_NEXT || handoff) {
        fprintf(stderr, "caller case%u flow%u handoff%06X\n", number, flow, handoff);
        abort();
    }
    return ExecutionReturned(0x80cc35u);
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, caller_kind ? 0x80afc0u : 0x80d516u);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result = run_caller(&memory, &cpu);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return 1;
    uint32_t stop = result.pc;
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
int main(int argc, char **argv) {
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

    (void)force_return;
    suite = 0;
    unsigned cases = argc > 2 ? (unsigned)strtoul(argv[2], NULL, 10) : 512u;
    for (caller_kind = 0; caller_kind < 2u; ++caller_kind) {
        for (number = 0; number < cases; ++number)
            if (run(reference))
                return 1;
        printf("PASS%u actual %s area-spawn caller compositions\n", cases,
               caller_kind ? "text" : "event");
    }
    return 0;
}
