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
static unsigned number, suite, fast_rom;
static unsigned fetch_left;
static uint32_t fetch_address;
static Trace *fetch_trace;
static unsigned bus_cycles, bus_master, timing_active, step_count;
static unsigned step_cycles[64], step_master[64];
static uint32_t entries[] = {0x00862du, 0x8086c1u};
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
static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    cpu->accumulator = (uint16_t)(number * 197u + 0x1234u);
    cpu->x = (uint16_t)(number * 83u);
    cpu->y = (uint16_t)(number * 59u);
    UnpackStatus(cpu, (uint8_t)number);
    cpu->program_bank = (number & 1024u) ? 0x80 : 0;
    cpu->data_bank = (uint8_t)(number * 57u);
    cpu->direct_page = (uint16_t)(number * 8191u);
    cpu->stack = (uint16_t[]){0x1f08u, 0x1f80u, 0x1ffbu, 0x1fd0u}[(number >> 8) & 3u];
    entries[0] = ((uint32_t)cpu->program_bank << 16) | 0x862du;
    if (cpu->index_is_8_bit) {
        cpu->x &= 255u;
        cpu->y &= 255u;
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
    step_count = 0;
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    load(reference, &cpu, entries[0]);
    Lufia2NmiSchedule schedule = Lufia2NmiEntrySchedule();
    Lufia2ExecutionResult result = Lufia2NmiSaveInterruptFrame(&memory, &cpu);
    uint32_t stop = 0x808638u;
    if (result.pc != stop)
        return 1;
    unsigned steps = 0;
    while (SnesVerifyPc24(reference) != stop && steps++ < 64u)
        instruction(reference);
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps >= 64u || memcmp(&cpu, &expected, sizeof cpu) ||
        memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(stderr,
                "slice%u case%u fast%u entry%06X stop%06X CPU%d WRAM%d bus%u/%u\n",
                suite, number, fast_rom, entries[0], SnesVerifyPc24(reference),
                memcmp(&cpu, &expected, sizeof cpu),
                memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE),
                native_trace.count, original_trace.count);
        return 1;
    }
    if (schedule.count != step_count) {
        fprintf(stderr, "step count%u/%u slice%u case%u\n", schedule.count, step_count,
                suite, number);
        return 1;
    }
    for (unsigned i = 0; i < step_count; ++i)
        if (Lufia2NmiCpuCycles(schedule.steps[i]) != step_cycles[i] ||
            Lufia2NmiMasterClocks(schedule.steps[i],
                                  (entries[0] >> 16) == 0x80u && fast_rom ? 6u : 8u) !=
                step_master[i]) {
            fprintf(stderr, "cost step%u slice%u case%u\n", i, suite, number);
            return 1;
        }
    return 0;
}
static int same_state(Lufia2CpuState *cpu, Interp816 *reference) {
    Lufia2CpuState expected = *cpu;
    store(&expected, reference);
    return memcmp(cpu, &expected, sizeof expected) ||
           memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace();
}
static int interrupt_return(Interp816 *reference, Interp816 *returned) {
    Lufia2CpuState cpu;
    seed(&cpu);
    cpu.direct_page = 0;
    cpu.decimal = 0;
    native.wram[cpu.stack + 1u] = (uint8_t)number;
    native.wram[cpu.stack + 2u] = 0x10;
    native.wram[cpu.stack + 3u] = 0x90;
    native.wram[cpu.stack + 4u] = 0x83;
    native.wram[0x520] = 0;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    load(reference, &cpu, entries[0]);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result = Lufia2NmiSaveInterruptFrame(&memory, &cpu);
    unsigned steps = 0;
    while (SnesVerifyPc24(reference) != result.pc && steps++ < 64)
        instruction(reference);
    if (steps >= 64 || same_state(&cpu, reference))
        return 1;
    cpu.data_bank = 0x80;
    SetAccumulatorWidth(&cpu, 0);
    SetIndexWidth(&cpu, 1);
    load(reference, &cpu, 0x80869bu);
    result = Lufia2NmiRestoreInterruptFrame(&memory, &cpu);
    if (result.pc != 0x8086bfu)
        return 1;
    steps = 0;
    while (SnesVerifyPc24(reference) != result.pc && steps++ < 64)
        instruction(reference);
    if (steps >= 64 || same_state(&cpu, reference))
        return 1;
    load(returned, &cpu, result.pc);
    fetch_trace = &native_trace;
    fetch_left = 1;
    fetch_address = result.pc;
    interp816_runOpcode(returned);
    fetch_left = 0;
    instruction(reference);
    Lufia2CpuState left = cpu, right = cpu;
    store(&left, returned);
    store(&right, reference);
    return SnesVerifyPc24(returned) != 0x839010u ||
           SnesVerifyPc24(reference) != 0x839010u ||
           memcmp(&left, &right, sizeof left) ||
           memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace();
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
              *returned =
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

    if (argc == 3 && strcmp(argv[2], "--returns") == 0) {
        for (unsigned row = 0; row < 512u; ++row) {
            number = (row * 17u) % 8192u;
            fast_rom = row & 1u;
            if (interrupt_return(reference, returned))
                return 1;
        }
        puts("PASS512 saved-frame/restoration/actual RTI compositions");
        return 0;
    }
    for (fast_rom = 0; fast_rom < 2u; ++fast_rom)
        for (number = 0; number < 8192u; ++number)
            if (run(reference))
                return 1;
    puts(
        "PASS16384 NMI entry CPU/WRAM/ordered bus/per-opcode cost states;all modes and both mirrors/speeds");
    return 0;
}
