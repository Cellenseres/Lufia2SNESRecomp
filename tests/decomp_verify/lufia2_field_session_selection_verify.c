#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "field/event_script_internal.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned number, defeat_mode, begin_mode;
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
    if (trace->count == sizeof(trace->events) / sizeof(trace->events[0])) {
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
    if (a >= 0x2000u && a < 0x7e0000u) {
        fprintf(stderr, "invalid seed word address%06X\n", a);
        exit(2);
    }
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
    for (unsigned i = 0; i < original_trace.count && i < native_trace.count; ++i) {
        SnesVerifyBusEvent *a = &original_trace.events[i], *b = &native_trace.events[i];
        if (a->address != b->address || a->value != b->value || a->write != b->write) {
            fprintf(stderr, "trace%u original%06X=%02X/%u native%06X=%02X/%u\n", i,
                    a->address, a->value, a->write, b->address, b->value, b->write);
            return 0;
        }
    }
    return original_trace.count == native_trace.count;
}

typedef struct ChildSite {
    uint32_t site, target;
    uint8_t frame;
} ChildSite;
static const ChildSite child_sites[] = {
    {0x83acf5u, 0x83adcau, 3u}, {0x83acf9u, 0x83b062u, 2u}, {0x83ad11u, 0x83900cu, 2u},
    {0x83ad18u, 0x82e746u, 3u}, {0x83ad3au, 0x80e844u, 3u}, {0x83ad3eu, 0x83b5adu, 2u},
    {0x83ad41u, 0x83b5d3u, 3u}, {0x83ad45u, 0x83ab61u, 2u}, {0x83ad48u, 0x83a82eu, 3u},
    {0x83ad51u, 0x80beafu, 3u}, {0x83ad5au, 0x83b53bu, 3u}, {0x83ad5eu, 0x80f821u, 3u},
    {0x83ad65u, 0x8093feu, 3u}, {0x83ad6fu, 0x8385dcu, 3u}, {0x83ad73u, 0x83afcdu, 3u},
    {0x83ad7au, 0x83addfu, 2u}, {0x83ad7du, 0x83a686u, 2u}, {0x83ad8eu, 0x82c352u, 3u},
    {0x83b1a4u, 0x83900cu, 2u}, {0x83b1e1u, 0x848328u, 3u}, {0x83b1e9u, 0x83b503u, 2u},
    {0x83b1fdu, 0x83b5adu, 2u}, {0x83b200u, 0x80be4du, 3u}, {0x83b21cu, 0x86919eu, 3u},
    {0x83b220u, 0x83b062u, 2u}, {0x83b26du, 0x839e31u, 3u}, {0x83b271u, 0x83ac7au, 3u},
    {0x83b275u, 0x83ab61u, 2u}, {0x83b278u, 0x80e844u, 3u}, {0x83b27cu, 0x83b5adu, 2u},
    {0x83b27fu, 0x83b5d3u, 3u}, {0x83b28du, 0x80be61u, 3u}, {0x83b2a2u, 0x83a686u, 2u},
    {0x83b2a5u, 0x83b512u, 2u}, {0x83b2d3u, 0x80c05cu, 3u}, {0x83b2dcu, 0x80c05cu, 3u},
    {0x83b2e5u, 0x80c05cu, 3u}, {0x83b323u, 0x80beafu, 3u}, {0x83b347u, 0x83b5adu, 2u},
    {0x83b356u, 0x83ab4fu, 3u}, {0x83b394u, 0x80c01du, 3u}, {0x83b398u, 0x80c1a7u, 3u},
    {0x83b3aau, 0x8ebc99u, 3u}, {0x83b3ddu, 0x83ab4fu, 3u}, {0x83b3f3u, 0x83a746u, 3u},
    {0x83b3f7u, 0x83a82eu, 3u}, {0x83b3fbu, 0x848204u, 3u}, {0x83b406u, 0x83b53bu, 3u},
    {0x83b40au, 0x80be4du, 3u}, {0x83b41cu, 0x80f821u, 3u}, {0x83b420u, 0x80be75u, 3u},
    {0x83b424u, 0x83b52eu, 2u}, {0x83b427u, 0x8385dcu, 3u}, {0x83b460u, 0x80c12eu, 3u},
    {0x83b469u, 0x83bb76u, 2u}, {0x83b471u, 0x809cb8u, 3u}, {0x83b47fu, 0x80c825u, 3u},
    {0x83b4a3u, 0x83ab4fu, 3u}, {0x83b4a9u, 0x83d350u, 3u}, {0x83b4c1u, 0x83ab4fu, 3u},
    {0x83b4c9u, 0x83d350u, 3u}, {0x83b4d4u, 0x83bb93u, 3u}, {0x83b4d8u, 0x83a21au, 3u},
    {0x83b4e9u, 0x83c0efu, 2u}, {0x83b4f2u, 0x80e722u, 3u}, {0x83b4f6u, 0x80cbaeu, 3u},
    {0x83b4fcu, 0x83afcdu, 3u}};

static Lufia2CpuState child_inputs[1024];
static unsigned native_inputs, reference_inputs, child_visits[2][128], site_hits[128];
static uint32_t stop_site;
static unsigned first_cases[128], width_kind;
static unsigned alter_width, world_returns[2];
static unsigned site_index(uint32_t site, uint32_t target, unsigned frame) {
    if (site == 0x83acc3u && target == 0x83adcau && frame == 3u)
        return 67u;
    for (unsigned i = 0; i < sizeof child_sites / sizeof child_sites[0]; ++i)
        if (child_sites[i].site == site && child_sites[i].target == target &&
            child_sites[i].frame == frame)
            return i;
    fprintf(stderr, "unexpected child%06X target%06X frame%u\n", site, target, frame);
    exit(1);
}
static uint8_t child_return(const Lufia2Memory *memory, Lufia2CpuState *cpu,
                            uint32_t target, uint32_t site, uint8_t frame) {
    SnesVerifyBus *bus = memory->context;
    unsigned side = bus == &native, index = site_index(site, target, frame);
    unsigned visit = ++child_visits[side][index];
    if (site == stop_site || (site == 0x83b1a4u && visit > 3u)) {
        cpu->resume_pc = target;
        return 0u;
    }
    unsigned profile = (number >> 10) % 14u;
    if (profile && site == 0x83b1e9u) {
        unsigned flags = profile == 1u                    ? 4u
                         : profile == 4u || profile >= 8u ? 0u
                         : profile == 5u                  ? 0x40u
                         : profile == 6u                  ? 2u
                                                          : 1u;
        Write8(memory, 0x5b3u, (uint8_t)flags);
        if (profile == 2u)
            Write8(memory, 0x5acu, (uint8_t)(world_returns[side] ? 3u : 0u));
        if (profile == 3u || profile == 13u) {
            Write8(memory, 0x5acu, 0xf0u);
            Write8(memory, 0x5b4u, (uint8_t)(profile == 3u));
        }
    }
    if (profile == 2u && site == 0x83b21cu)
        ++world_returns[side];
    if (profile == 12u && site == 0x83b28du && visit == 1u)
        Write8(memory, 0x5b3u, 0x20u);
    if (profile && site == 0x83b2a5u) {
        unsigned parameter = profile == 8u    ? 1u
                             : profile == 9u  ? 7u
                             : profile == 11u ? 4u + number % 3u
                                              : 2u;
        Write8(memory, 0x5b2u, (uint8_t)parameter);
    }
    if (profile == 7u && site == 0x83b347u) {
        for (unsigned i = 0; i < 63u; ++i) {
            Write8(memory, 0x85eu + i, bus->wram[0x5acu]);
            Write8(memory, 0x89eu + i, (number & 32u) ? 0xffu : 0u);
        }
        Write8(memory, 0x89du, bus->wram[0x5acu]);
    }
    cpu->accumulator = (uint16_t)(number * 17u + (site & 255u));
    cpu->carry = (number >> 1) & 1u;
    cpu->zero = (number >> 2) & 1u;
    cpu->negative = (number >> 3) & 1u;
    cpu->overflow = (number >> 4) & 1u;
    cpu->irq_disable = (number >> 5) & 1u;
    if (alter_width && site == alter_width) {
        if (width_kind == 2u)
            SetIndexWidth(cpu, 1u);
        else
            SetAccumulatorWidth(cpu, 0u);
    }
    if (frame == 3u) {
        SimulateRtlFrame(memory, cpu);
        cpu->program_bank = 0x83u;
    } else
        SimulateRtsFrame(memory, cpu);
    return 1u;
}
static uint8_t child_call(void *context, Lufia2CpuState *cpu, uint32_t target,
                          uint32_t site, uint8_t frame) {
    const Lufia2Memory *memory = context;
    unsigned index = site_index(site, target, frame);
    cpu->program_bank = (uint8_t)(target >> 16);
    if (native_inputs == 1024u)
        exit(1);
    child_inputs[native_inputs++] = *cpu;
    if (!site_hits[index])
        first_cases[index] = number;
    ++site_hits[index];
    return child_return(memory, cpu, target, site, frame);
}
static void seed(Lufia2CpuState *cpu) {
    memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(cpu, 0, sizeof *cpu);
    memset(child_visits, 0, sizeof child_visits);
    memset(world_returns, 0, sizeof world_returns);
    native_inputs = reference_inputs = 0u;
    cpu->program_bank = 0x83u;
    cpu->data_bank = (uint8_t)number;
    cpu->accumulator_is_8_bit = (number >> 8) & 1u;
    cpu->index_is_8_bit = (number >> 9) & 1u;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 29u);
    cpu->y = (uint16_t)(number * 113u);
    if (cpu->index_is_8_bit) {
        cpu->x &= 255u;
        cpu->y &= 255u;
    }
    cpu->stack = number % 4u == 0u ? 0x1fffu : (uint16_t)(0x1f10u + number % 128u);
    cpu->carry = number & 1u;
    cpu->zero = (number >> 1) & 1u;
    cpu->negative = (number >> 2) & 1u;
    cpu->overflow = (number >> 3) & 1u;
    cpu->irq_disable = (number >> 4) & 1u;
    native.wram[0xb51u] =
        (uint8_t)((number & 1u ? 0x80u : 0u) | (number & 2u ? 2u : 0u));
    native.wram[0xb50u] = (number & 4u) ? 0x80u : 0u;
    native.wram[0x47u] = (uint8_t)((number & 8u) ? 0x80u : 0u);
    native.wram[0x5b3u] = (uint8_t)((number & 16u) ? 4u : 0u);
    native.wram[0x5acu] = (uint8_t)(number % 4u);
    native.wram[0x89du] = (uint8_t)number;
    memset(native.wram + 0x89eu, (number & 32u) ? 0xff : 0, 32u);
    native.wram[0x53u] = (uint8_t)(number % 8u);
    if (begin_mode) {
        cpu->data_bank = 0x83u;
        cpu->accumulator_is_8_bit = cpu->index_is_8_bit = 1u;
        cpu->x &= 255u;
        cpu->y &= 255u;
    }
    if (defeat_mode) {
        cpu->program_bank = 0x86u;
        cpu->data_bank = 0x86u;
        cpu->accumulator_is_8_bit = 1u;
        cpu->index_is_8_bit = 0u;
        native.wram[0x1f8a2u] = 1u;
        for (unsigned at = 0x1e00u; at < 0x2000u; ++at)
            native.wram[at] = (uint8_t)(at * 11u + number * 37u);
    }
    word(0x40u, (uint16_t)(number * 257u));
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, defeat_mode ? 0x869d9eu : begin_mode ? 0x83acb7u : 0x83acefu);
    if (defeat_mode) {
        Interp816 *arrival =
            interp816_init(&native, SnesVerifyBusRead, SnesVerifyBusWrite);
        load(arrival, &cpu, 0x869d9eu);
        unsigned steps = 0u;
        while (SnesVerifyPc24(arrival) != 0x83acefu && steps++ < 32u)
            instruction(&native, arrival);
        if (steps >= 32u || arrival->sp != 0x1fffu || arrival->db != 0x86u) {
            fputs("wrong world defeat arrival", stderr);
            return 1;
        }
        store(&cpu, arrival);
        interp816_free(arrival);
    }
    const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                 NULL};
    Lufia2ExecutionResult result =
        begin_mode
            ? Lufia2FieldBeginSessionSetup(&memory, &cpu, child_call, (void *)&memory)
            : Lufia2FieldSelectSessionSetup(&memory, &cpu, child_call, (void *)&memory);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        fputs("selection cannot return\n", stderr);
        return 1;
    }
    uint32_t stop =
        result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND ? cpu.resume_pc : result.pc;
    const Lufia2Memory ref_memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &original,
                                     NULL, NULL};
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop && reference->sp == cpu.stack &&
            reference_inputs == native_inputs)
            break;
        uint32_t site = SnesVerifyPc24(reference);
        uint8_t opcode = original.rom[((site >> 16) & 127u) * 32768u + (site & 32767u)];
        instruction(&original, reference);
        if (opcode == 0x20u || opcode == 0x22u) {
            uint32_t target = SnesVerifyPc24(reference);
            uint8_t frame = opcode == 0x22u ? 3u : 2u;
            (void)site_index(site, target, frame);
            if (result.flow == LUFIA2_EXECUTION_BOUNDARY && target == stop &&
                reference_inputs == native_inputs && reference->sp == cpu.stack)
                break;

            if (reference_inputs >= native_inputs)
                return 1;
            Lufia2CpuState state = child_inputs[reference_inputs++], expected = state;
            store(&expected, reference);
            if (memcmp(&expected, &state, sizeof state)) {
                fprintf(stderr, "child input%06X case%u\n", site, number);
                return 1;
            }
            uint8_t returned = child_return(&ref_memory, &state, target, site, frame);
            load(reference, &state, returned ? site + frame + 1u : state.resume_pc);
        }
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 1000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "selection case%u PC%06X/%06X CPU%d WRAM%d children%u/%u steps%u A%04X/%04X X%04X/%04X S%04X/%04X DB%02X/%02X\n",
            number, stop, SnesVerifyPc24(reference),
            memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE), native_inputs,
            reference_inputs, steps, cpu.accumulator, reference->a, cpu.x, reference->x,
            cpu.stack, reference->sp, cpu.data_bank, reference->db);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
#ifndef RUNTIME_ABI
    if (argc > 2 &&
        (strcmp(argv[2], "--redirects") == 0 || strcmp(argv[2], "--missing") == 0)) {
        fputs("This selector requires the production ABI fixture.\n", stderr);
        return 2;
    }
#endif

    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc == 5) {
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned offset = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        if (offset >= size)
            return 2;
        rom[offset] = (uint8_t)strtoul(argv[4], NULL, 16);
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

    if (argc >= 3 && strcmp(argv[2], "--begin") == 0) {
        begin_mode = 1u;
        unsigned total = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 512u;
        for (unsigned i = 0u; i < total; ++i) {
            number = i * 29u % 14336u;
            if (run(reference))
                return 1;
        }
        printf("PASS%u affected ACB7 session parents;original M1X1\n", total);
        return 0;
    }
    if (argc >= 3 && strcmp(argv[2], "--defeat") == 0) {
        defeat_mode = 1u;
        unsigned total = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 512u;
        for (unsigned i = 0u; i < total; ++i) {
            number = i * 29u % 14336u;
            if (run(reference))
                return 1;
        }
        printf("PASS%u original world-defeat JML arrivals;stack reset1FFF\n", total);
        return 0;
    }
    if (argc >= 3 &&
        (strcmp(argv[2], "--interruptions") == 0 || strcmp(argv[2], "--widths") == 0 ||
         strcmp(argv[2], "--sites") == 0 || strcmp(argv[2], "--redirects") == 0 ||
         strcmp(argv[2], "--missing") == 0)) {
        unsigned discovery[] = {0u,    1u,    2u,     3u,     1024u,  2048u,
                                3072u, 4096u, 5120u,  6144u,  7168u,  7200u,
                                8192u, 9216u, 10240u, 11264u, 12288u, 13312u};
        for (unsigned i = 0; i < sizeof discovery / sizeof discovery[0]; ++i) {
            number = discovery[i];
            if (run(reference))
                return 1;
        }
        unsigned total = 0u;
        for (unsigned i = 0; i < sizeof child_sites / sizeof child_sites[0]; ++i) {
            if (!site_hits[i]) {
                fprintf(stderr, "uncovered child%06X\n", child_sites[i].site);
                return 1;
            }
            number = first_cases[i];
#ifdef RUNTIME_ABI
            if (strcmp(argv[2], "--redirects") == 0) {
                Lufia2CpuState state;
                seed(&state);
                const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite,
                                             &native, NULL, NULL};
                runtime_probe_desired_site = child_sites[i].site;
                unsigned before = runtime_redirects[0];
                runtime_probe(&memory, &state, Lufia2DecompBridge_83ACEF, 0x83acefu, 0u,
                              3u);
                if (runtime_redirects[0] - before != 15u)
                    return 1;
                total += 15u;
                continue;
            }
            if (strcmp(argv[2], "--missing") == 0) {
                unsigned duplicate = 0u;
                for (unsigned j = 0; j < i; ++j)
                    if (child_sites[j].target == child_sites[i].target)
                        duplicate = 1u;
                if (duplicate)
                    continue;
                runtime_missing_target = child_sites[i].target;
                if (run(reference))
                    return 1;
                ++total;
                continue;
            }
#endif

            if (strcmp(argv[2], "--sites") == 0) {
                printf("FIRST%06X case%u\n", child_sites[i].site, number);
                continue;
            }
            if (strcmp(argv[2], "--interruptions") == 0) {
                stop_site = child_sites[i].site;
                if (run(reference))
                    return 1;
                ++total;
            } else
                for (width_kind = 1u; width_kind <= 2u; ++width_kind) {
                    alter_width = child_sites[i].site;
                    if (run(reference))
                        return 1;
                    ++total;
                }
        }
        printf("PASS%u field-selection child contracts;18 discovery states\n", total);
        return 0;
    }
    unsigned cases = argc > 2 ? (unsigned)strtoul(argv[2], NULL, 10) : 1024u;
    if (!cases)
        return 2;
    unsigned first = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 0u;
    for (number = first; number < first + cases; ++number)
        if (run(reference))
            return 1;
    printf("PASS%u original field-selection states;explicit child services\n", cases);
    for (unsigned i = 0; i < sizeof child_sites / sizeof child_sites[0]; ++i)
        printf("SITE%06X hits%u\n", child_sites[i].site, site_hits[i]);
    return 0;
}
