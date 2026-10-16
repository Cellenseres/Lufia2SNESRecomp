#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "snes_cycles.h"
#include "core/cpu_ops.h"
#include "lufia2/system.h"
#include "lufia2/menu.h"
#include "decomp_bridge/frame_return_timing.h"
typedef struct Trace {
    SnesVerifyBusEvent events[4096];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static uint8_t *rom;

static unsigned number, suite, fast_rom;
static unsigned fetch_left;
static uint32_t fetch_address;
static Trace *fetch_trace;
static unsigned bus_cycles, bus_master, timing_active, step_count;
static unsigned step_cycles[64], step_master[64];
static const uint32_t entries[] = {0x8293ccu, 0x868b52u, 0x869756u, 0x868b6fu};
static const uint32_t returns[] = {0x8293ceu, 0x868b54u, 0x869757u, 0x868b72u};
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
    fetch_trace = &original_trace;
    fetch_left = 1u;
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
    cpu->x = (uint16_t)(number * 83u + 0x9876u);
    cpu->y = (uint16_t)(number * 59u + 0x2468u);
    cpu->direct_page = (uint16_t)(number * 8191u);
    cpu->data_bank = (uint8_t)(number * 57u);
    cpu->program_bank = (uint8_t)(entries[suite] >> 16);
    UnpackStatus(cpu, (uint8_t)(number * 73u));
    cpu->accumulator_is_8_bit = suite == 2u ? (number >> 8) & 1u : 1u;
    cpu->index_is_8_bit = suite == 3u ? 0u : (number >> 9) & 1u;
    if (cpu->index_is_8_bit) {
        cpu->x &= 255u;
        cpu->y &= 255u;
    }
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f80u, 0x1fd0u,
                              suite == 3u ? 0x1ff7u : 0x1ff8u}[(number >> 10) & 3u];
    for (unsigned i = 1u; i < 7u; ++i)
        native.wram[cpu->stack + i] = (uint8_t)(number * 113u + i * 37u);
    native.wram[cpu->stack + 1u] = (uint8_t)number;
    unsigned bytes = suite == 2u   ? (cpu->accumulator_is_8_bit ? 1u : 2u)
                     : suite == 3u ? 1u + 2u * ((number & 0x10u) ? 1u : 2u)
                                   : 1u + ((number & 0x20u) ? 1u : 2u);
    native.wram[cpu->stack + bytes + 1u] = 0x22u;
    native.wram[cpu->stack + bytes + 2u] = 0x90u;
    if (suite == 3u)
        native.wram[cpu->stack + bytes + 3u] = 0x83u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    original_trace.count = native_trace.count = step_count = 0u;
    SnesVerifyBusResetTrace(&original);
    SnesVerifyBusResetTrace(&native);
}
static Lufia2ExecutionResult execute(const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    return suite == 3u ? Lufia2SpriteFrameRestoreRegisters(memory, cpu)
                       : Lufia2FrameWaitRestoreRegisters(memory, cpu, entries[suite]);
}
static int same_state(const Lufia2CpuState *cpu, Interp816 *reference) {
    Lufia2CpuState expected = *cpu;
    store(&expected, reference);
    return memcmp(cpu, &expected, sizeof expected) ||
           memcmp(native.wram, original.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace();
}
static int run(Interp816 *reference, Interp816 *returned, unsigned compose) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, entries[suite]);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2FrameReturnSchedule schedule = Lufia2FrameReturnScheduleFor(
        entries[suite], native.wram[cpu.stack + 1u], cpu.accumulator_is_8_bit);
    Lufia2ExecutionResult result = execute(&memory, &cpu);
    unsigned steps;
    for (steps = 0; steps < 8u && SnesVerifyPc24(reference) != result.pc; ++steps)
        instruction(reference);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != returns[suite] ||
        cpu.resume_pc != result.pc || steps == 8u || same_state(&cpu, reference) ||
        step_count != schedule.count) {
        fprintf(stderr, "restore%06X case%u PC%06X/%06X steps%u/%u CPU/bus mismatch\n",
                entries[suite], number, SnesVerifyPc24(reference), result.pc,
                step_count, schedule.count);
        return 1;
    }
    unsigned fetch_speed = (unsigned)snes_region_speed(entries[suite], fast_rom);
    for (unsigned i = 0; i < schedule.count; ++i)
        if (step_cycles[i] != Lufia2FrameReturnCpuCycles(schedule.pulled_bytes[i]) ||
            step_master[i] !=
                Lufia2FrameReturnMasterClocks(schedule.pulled_bytes[i], fetch_speed)) {
            fprintf(
                stderr, "restore cost%06X case%u step%u cpu%u/%u master%u/%u\n",
                entries[suite], number, i, step_cycles[i],
                Lufia2FrameReturnCpuCycles(schedule.pulled_bytes[i]), step_master[i],
                Lufia2FrameReturnMasterClocks(schedule.pulled_bytes[i], fetch_speed));
            return 1;
        }
    if (!compose)
        return 0;
    load(returned, &cpu, result.pc);
    fetch_trace = &native_trace;
    fetch_left = 1u;
    fetch_address = result.pc;
    interp816_runOpcode(returned);
    fetch_left = 0u;
    instruction(reference);
    Lufia2CpuState left = cpu, right = cpu;
    store(&left, returned);
    store(&right, reference);
    uint32_t landing = ((suite == 3u ? 0x83u : (entries[suite] >> 16)) << 16) | 0x9023u;
    return SnesVerifyPc24(returned) != landing ||
           SnesVerifyPc24(reference) != landing || memcmp(&left, &right, sizeof left) ||
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
        uint32_t address = (uint32_t)strtoul(argv[2], NULL, 16);
        unsigned value = (unsigned)strtoul(argv[3], NULL, 16);
        size_t offset = ((address >> 16) & 127u) * 32768u + (address & 32767u);
        if (offset >= size)
            return 2;
        rom[offset] = (uint8_t)value;
    }
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *reference =
        interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    Interp816 *returned =
        interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
    unsigned compose = argc == 3 && strcmp(argv[2], "--returns") == 0;
    unsigned total = 0u;
    for (suite = 0; suite < 4u; ++suite)
        for (fast_rom = 0; fast_rom < 2u; ++fast_rom)
            for (number = 0; number < (compose ? 256u : 8192u); ++number) {
                if (run(reference, returned, compose))
                    return 1;
                ++total;
            }
    printf(
        "PASS%u four restore tails;allP/widths/stack/registers/ROMspeeds;CPU/fullWRAM/orderedbus/peropcodeCPU+master;actual RTS/RTL%u\n",
        total, compose ? total : 0u);
    return 0;
}
