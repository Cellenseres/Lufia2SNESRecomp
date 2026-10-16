#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "snes_cycles.h"
#include "core/cpu_ops.h"
#include "lufia2/system.h"
#include "decomp_bridge/nmi_timing.h"

typedef struct Trace {
    SnesVerifyBusEvent events[4096];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static uint8_t *rom;
static uint8_t lengths[4][256];
static unsigned number, suite, fast_rom, clock_edges, rti_mode, rti_count;
static unsigned fetch_left;
static uint32_t fetch_address;
static Trace *fetch_trace;
static unsigned bus_cycles, bus_master, timing_active, step_count;
static unsigned step_cycles[64], step_master[64];
static const uint32_t entries[] = {0x808664u, 0x80869bu};

static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *trace = context;
    if (timing_active && trace == &original_trace) {
        ++bus_cycles;
        bus_master += (unsigned)snes_region_speed(address, fast_rom);
    }
    if (!write && trace == fetch_trace && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    if (trace->count == 4096u)
        exit(2);
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
static void word(unsigned address, uint16_t value) {
    native.wram[address] = (uint8_t)value;
    native.wram[address + 1] = (uint8_t)(value >> 8);
}
static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    cpu->program_bank = cpu->data_bank = 0x80;
    cpu->accumulator = (uint16_t)(number * 197u + 0x1234u);
    cpu->x = (uint8_t)(number * 83u);
    cpu->y = (uint8_t)(number * 59u);
    cpu->accumulator_is_8_bit = suite == 0;
    cpu->index_is_8_bit = 1;
    cpu->stack = (uint16_t[]){0x1f00, 0x1f80, 0x1ff3, 0x1fca}[(number >> 6) & 3u];
    cpu->carry = number & 1u;
    cpu->zero = (number >> 1) & 1u;
    cpu->negative = (number >> 2) & 1u;
    cpu->overflow = (number >> 3) & 1u;
    cpu->irq_disable = (number >> 4) & 1u;
    static const uint8_t edges[] = {0, 1, 58, 59, 60, 127, 128, 254, 255};
    native.wram[0xb50] = (uint8_t)number;
    native.wram[0xb4f] = edges[(number >> 8) % 9];
    native.wram[0xb4e] = edges[((number >> 8) / 9) % 9];
    native.wram[0xb4d] = (uint8_t)((number >> 8) * 73u);
    native.wram[0x520] = (number & 8u) ? 0x80u : 0u;
    native.wram[0x521] = (uint8_t)(number * 11u);
    word(0x46, number % 8u == 0 ? 0x3030u : (uint16_t)(number * 19u));
    for (unsigned i = 1; i <= 12u; ++i)
        native.wram[cpu->stack + i] = (uint8_t)(number * 37u + i * 71u);
    native.wram[cpu->stack + 8u] = (uint8_t)(number >> 8);
    native.wram[cpu->stack + 9u] = (uint8_t)(number >> 5);
    native.wram[cpu->stack + 10u] = 0x10;
    native.wram[cpu->stack + 11u] = 0x90;
    native.wram[cpu->stack + 12u] = 0x83;
    if (clock_edges) {
        native.wram[0x520] = 0;
        native.wram[0xb50] = native.wram[0xb4f] = native.wram[0xb4e] = 59;
        native.wram[0xb4d] = (uint8_t)number;
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
    step_count = 0;
}
static int same_trace(void) {
    if (original_trace.count != native_trace.count)
        return 0;
    for (unsigned i = 0; i < original_trace.count; ++i) {
        SnesVerifyBusEvent *a = &original_trace.events[i], *b = &native_trace.events[i];
        if (a->address != b->address || a->value != b->value || a->write != b->write)
            return 0;
    }
    return 1;
}
static void instruction(Interp816 *cpu) {
    uint32_t pc = SnesVerifyPc24(cpu);
    uint8_t opcode = rom[((pc >> 16) & 0x7fu) * 0x8000u + (pc & 0x7fffu)];
    fetch_trace = &original_trace;
    fetch_left = lengths[(unsigned)cpu->mf * 2u + (unsigned)cpu->xf][opcode];
    fetch_address = pc;
    bus_cycles = bus_master = 0;
    timing_active = 1;
    unsigned cycles = (unsigned)interp816_runOpcode(cpu);
    timing_active = 0;
    fetch_left = 0;
    if (step_count == 64u)
        exit(2);
    step_cycles[step_count] = cycles;
    step_master[step_count++] =
        bus_master + (cycles > bus_cycles ? cycles - bus_cycles : 0u) * 6u;
}
static int run(Interp816 *reference, Interp816 *native_rti) {
    Lufia2CpuState cpu;
    seed(&cpu);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    load(reference, &cpu, entries[suite]);
    Lufia2NmiSchedule schedule = suite == 0 ? Lufia2NmiClockSchedule(native.wram)
                                            : Lufia2NmiRestoreSchedule(native.wram);
    Lufia2ExecutionResult result = suite == 0
                                       ? Lufia2NmiDisplayAndClock(&memory, &cpu)
                                       : Lufia2NmiRestoreInterruptFrame(&memory, &cpu);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entries[suite])
        return 1;
    unsigned steps = 0;
    while (SnesVerifyPc24(reference) != result.pc && steps++ < 64u)
        instruction(reference);
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps >= 64u || memcmp(&cpu, &expected, sizeof cpu) ||
        memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(stderr, "slice%u case%u fast%u stop%06X/%06X CPU%d WRAM%d bus%u/%u\n",
                suite, number, fast_rom, result.pc, SnesVerifyPc24(reference),
                memcmp(&cpu, &expected, sizeof cpu),
                memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE),
                native_trace.count, original_trace.count);
        return 1;
    }
    if (schedule.count != step_count) {
        fprintf(stderr, "step count%u/%u\n", schedule.count, step_count);
        return 1;
    }
    for (unsigned i = 0; i < step_count; ++i)
        if (Lufia2NmiCpuCycles(schedule.steps[i]) != step_cycles[i] ||
            Lufia2NmiMasterClocks(schedule.steps[i], fast_rom ? 6u : 8u) !=
                step_master[i]) {
            fprintf(stderr, "cost step%u slice%u case%u\n", i, suite, number);
            return 1;
        }
    if (rti_mode && result.pc == 0x8086bfu) {
        load(native_rti, &cpu, result.pc);
        fetch_trace = &native_trace;
        fetch_left = 1;
        fetch_address = result.pc;
        interp816_runOpcode(native_rti);
        fetch_left = 0;
        instruction(reference);
        Lufia2CpuState left = cpu, right = cpu;
        store(&left, native_rti);
        store(&right, reference);
        if (SnesVerifyPc24(native_rti) != 0x839010u ||
            SnesVerifyPc24(reference) != 0x839010u ||
            memcmp(&left, &right, sizeof left) ||
            memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace())
            return 1;
        ++rti_count;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc == 4) {
        unsigned address = (unsigned)strtoul(argv[2], NULL, 16),
                 value = (unsigned)strtoul(argv[3], NULL, 16);
        size_t offset = ((address >> 16) & 0x7fu) * 0x8000u + (address & 0x7fffu);
        if (offset >= size)
            return 2;
        rom[offset] = (uint8_t)value;
    }
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *reference =
                  interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite),
              *native_rti =
                  interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
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
        suite = 1;
        rti_mode = 1;
        for (unsigned i = 0; i < 256u; ++i) {
            number = (i << 8) | 59u;
            if (run(reference, native_rti))
                return 1;
        }
        clock_edges = 1;
        for (number = 0; number < 1024u; ++number)
            if (run(reference, native_rti))
                return 1;
        printf("PASS new restore slice plus actual interpreted RTI:%u\n", rti_count);
        return rti_count == 1280u ? 0 : 1;
    }
    unsigned total = 0;
    for (fast_rom = 0; fast_rom < 2u; ++fast_rom)
        for (suite = 0; suite < 2u; ++suite) {
            for (number = 0; number < 65536u; ++number) {
                if (run(reference, native_rti))
                    return 1;
                ++total;
            }
            printf("PASS slice%u speed%u:65536 CPU/WRAM/bus/per-opcode-cost states\n",
                   suite, fast_rom);
            fflush(stdout);
        }
    suite = 0;
    clock_edges = 1;
    for (number = 0; number < 1024u; ++number)
        if (run(reference, native_rti))
            return 1;
    printf("PASS two original NMI slices:%u plus1024 all-hour rollover edges\n", total);
    return 0;
}
