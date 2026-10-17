#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "lufia2/ancient_cave.h"
#include "cave/cave_internal.h"
#include "field/event_script_internal.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[4000000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return, edge_profile, arithmetic_profile,
    grid_profile;
static unsigned child_visits[15];
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {
    0x839b18u, 0x839b27u, 0x839b3cu, 0x839b48u, 0x839b62u, 0x839bddu,
    0x839ca0u, 0x839ccbu, 0x839cd6u, 0x839cf3u, 0x839d68u, 0x839d11u,
    0x839d23u, 0x839d39u, 0x839d46u, 0x839b44u, 0x8eb847u};
static const uint8_t modes[] = {1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
                                1u, 2u, 1u, 1u, 1u, 0u, 1u, 2u};
static const uint8_t work_banks[] = {1u, 1u, 0u, 0u, 1u, 1u, 0u, 0u, 0u,
                                     0u, 0u, 1u, 1u, 1u, 1u, 0u, 0u};
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
    if (b == &original)
        for (unsigned i = 0u; i < 15u; ++i)
            if (pc == entries[i])
                ++child_visits[i];
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

static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(child_visits, 0, sizeof child_visits);
    cpu->program_bank = suite == 16u ? 0x8eu : 0x83u;
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    cpu->data_bank = work_banks[suite] ? 0x7fu : (uint8_t)number;
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->decimal = 0u;
    cpu->accumulator_is_8_bit = modes[suite] == 2u ? (number >> 8) & 1u : modes[suite];
    cpu->index_is_8_bit = 0u;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 29u);
    cpu->y = (uint16_t)(number * 113u);
    for (unsigned i = 0u; i < 256u; ++i) {
        unsigned row = i >> 4, column = i & 15u;
        uint8_t value = row && row < 15u && column && column < 15u
                            ? (uint8_t)((row / 3u + column / 3u + number) % 7u + 1u)
                            : 0u;
        if ((i * 13u + number) % 5u == 0u)
            value = 0u;
        if (number & 16u)
            value = (uint8_t)(value | ((i + number) & 0xc0u));
        native.wram[CAVE_ROOM_GRID_LONG - 0x7e0000u + i] = value;
    }
    if (grid_profile) {
        unsigned shape = number % 8u;
        for (unsigned i = 0u; i < 256u; ++i) {
            uint8_t value =
                (uint8_t)(shape == 0u   ? 0u
                          : shape == 1u ? 1u
                          : shape == 2u ? 0x80u
                          : shape == 3u ? (uint8_t)((i >> 4) + (i & 15u)) % 2u + 1u
                          : shape == 4u ? (uint8_t)(i == 0x22u)
                          : shape == 5u ? ((i >> 4) == 2u ? 1u : 0u)
                          : shape == 6u ? 0x40u
                                        : 0xffu);
            native.wram[CAVE_ROOM_GRID_LONG - 0x7e0000u + i] = value;
        }
    } else
        native.wram[CAVE_ROOM_GRID_LONG - 0x7e0000u + 0x22u] = 1u;
    native.wram[0x1e6a9u] = 0x22u;
    native.wram[0x1e6afu] = (uint8_t)number;
    native.wram[0x2au] = (uint8_t)(number % 9u + 1u);
    native.wram[0x2du] = (uint8_t)(number % 32u);
    native.wram[0x2eu] = 0u;
    native.wram[0x24u] = (uint8_t)(number % 8u + 1u);
    native.wram[0x25u] = (uint8_t)((number / 8u) % 8u + 1u);
    native.wram[0x26u] = (uint8_t)(number % 13u + 1u);
    native.wram[0x27u] = (uint8_t)((number / 13u) % 13u + 1u);
    native.wram[0x8fu] = (uint8_t)(number * 37u);
    native.wram[0x90u] = (uint8_t)(number * 53u);
    native.wram[0x91u] = (uint8_t)(number * 71u);
    native.wram[0x92u] = (uint8_t)(number * 97u);
    native.wram[0x54u] = (uint8_t)(number % 127u + 1u);
    native.wram[0x55u] = (uint8_t)(number * 17u);
    native.wram[WRAM_RANDOM_NEXT_INDEX] = (uint8_t)number;
    for (unsigned i = 0u; i < WRAM_RANDOM_TABLE_COUNT; ++i)
        native.wram[WRAM_RANDOM_TABLE + i] = (uint8_t)(number * 37u + i * 19u);
    if (edge_profile) {
        static const uint8_t values[] = {0u, 1u, 2u, 15u, 16u, 127u, 128u, 255u};
        cpu->accumulator =
            (uint16_t)((uint16_t)values[(number >> 3) & 7u] << 8 | values[number & 7u]);
        native.wram[0x8fu] = values[number & 7u];
        native.wram[0x91u] = values[(number >> 3) & 7u];
        native.wram[0x90u] = values[(number >> 6) & 7u];
        native.wram[0x92u] = values[(number >> 9) & 7u];
        native.wram[0x2au] = values[number & 7u];
        native.wram[0x54u] = values[(number >> 3) & 7u];
        word(0x2du, (uint16_t[]){0u, 1u, 0xffffu, 0x190fu, 0x1fffu, 0x390eu, 0x8000u,
                                 0xfffeu}[number & 7u]);
        cpu->y = (uint16_t[]){0u,      1u,      0xffffu, 0xc0f1u,
                              0xe0f1u, 0x190fu, 0x8000u, 0xfffeu}[number & 7u];
        if (suite == 6u) {
            native.wram[0x24u] = (number & 1u) ? 0u : 1u;
            native.wram[0x25u] = (number & 2u) ? 0u : 1u;
        }
    }
    if (arithmetic_profile) {
        cpu->accumulator = (uint16_t)number;
        native.wram[0x8fu] = (uint8_t)number;
        native.wram[0x91u] = (uint8_t)(number >> 8);
    }
    if (suite == 16u) {
        cpu->x = 0u;
        cpu->y = 0u;
        native.wram[WRAM_CAVE_HEADER_ROOM_POINT_COUNT - 0x7e0000u] =
            (uint8_t)(number % 5u);
        native.wram[WRAM_CAVE_HEADER_OBJECT_COUNT - 0x7e0000u] = (uint8_t)(number % 5u);
        native.wram[0x1e6f1u] = 0x22u;
        native.wram[0x1e6f2u] = 0u;
        native.wram[0x1e6f3u] = 0x31u;
        native.wram[0x1e6f4u] = 0xffu;
        for (unsigned i = 0u; i < 8u; ++i) {
            native.wram[CAVE_OBJECT_COLUMNS_LONG - 0x7e0000u + i] =
                (uint8_t)(number + i + 1u);
            native.wram[CAVE_OBJECT_ROWS_LONG - 0x7e0000u + i] =
                (uint8_t)(number + i * 3u);
            native.wram[CAVE_CHEST_COLUMNS_LONG - 0x7e0000u + i] =
                (uint8_t)(number + i * 7u);
            native.wram[CAVE_CHEST_ROWS_LONG - 0x7e0000u + i] =
                (uint8_t)(number + i * 11u);
        }
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    if (suite >= 15u)
        native.wram[cpu->stack + 3u] = 0x82u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
    original.m7_a = native.m7_a = (uint16_t)(number * 593u);
    original.m7_b = native.m7_b = (uint8_t)(number * 19u);
    original.m7_latch = native.m7_latch = (uint8_t)(number * 73u);
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, entries[suite]);
    const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                 NULL};
    static Lufia2ExecutionResult (*const functions[])(const Lufia2Memory *,
                                                      Lufia2CpuState *) = {
        Lufia2CaveClearVisitedCells,     Lufia2CaveCountOccupiedCells,
        Lufia2CaveOffsetStairPosition,   Lufia2CaveResolveCellOrigin,
        Lufia2CaveLinkRoomCells,         Lufia2CaveMergeRoomCells,
        Lufia2CaveFillRoomRectangle,     Lufia2CaveResolveCellIndex,
        Lufia2CaveResolveBlockOffset,    Lufia2CaveResolveTileOffset,
        Lufia2CaveResolveWordTileOffset, Lufia2CavePickUnvisitedCell,
        Lufia2CaveFindUnvisitedCell,     Lufia2CaveAppendRoomLink,
        Lufia2CaveReplaceUpperTileIndex, Lufia2CaveRoomHeaderCoordinates,
        Lufia2CaveBuildMapHeader};
    Lufia2ExecutionResult result = functions[suite](&memory, &cpu);
    uint32_t stop = result.pc;
    if (result.flow != LUFIA2_EXECUTION_RETURNED) {
        fprintf(stderr, "unexpected flow%u PC%06X suite%u case%u\n", result.flow,
                result.pc, suite, number);
        return 1;
    }
    if (force_return || number % 33u == 0u) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        cpu.program_bank = suite >= 15u ? Pull8(&memory, &cpu) : 0x83u;
        stop = (uint32_t)cpu.program_bank << 16 | (uint16_t)(back + 1u);
    }
    unsigned steps;
    for (steps = 0u; steps < 4000000u; ++steps) {
        if (SnesVerifyPc24(reference) == stop && reference->sp == cpu.stack)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    int missing_child = suite >= 15u && !child_visits[3u];
    int mmio = original.mmio_count != native.mmio_count || original.mmio_overflow ||
               native.mmio_overflow ||
               memcmp(original.mmio, native.mmio,
                      original.mmio_count * sizeof original.mmio[0]) ||
               original.m7_a != native.m7_a || original.m7_b != native.m7_b ||
               original.m7_latch != native.m7_latch ||
               original.wm_address != native.wm_address ||
               original.multiply_a != native.multiply_a ||
               original.multiply_b != native.multiply_b ||
               original.multiply_result != native.multiply_result ||
               original.divide_a != native.divide_a ||
               original.divide_result != native.divide_result;
    if (steps == 4000000u || missing_child || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || mmio ||
        (suite < 16u && !same_trace())) {
        fprintf(
            stderr,
            "cave%06X family%u case%u CPU%d WRAM%d MMIO%d PC%06X/%06X steps%u traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X\n",
            entries[suite], suite, number, memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE), mmio, stop,
            SnesVerifyPc24(reference), steps, native_trace.count, original_trace.count,
            cpu.accumulator, reference->a, cpu.x, reference->x, cpu.y, reference->y,
            cpu.stack, reference->sp, cpu.data_bank, reference->db);
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
        uint32_t address = (uint32_t)strtoul(argv[3], NULL, 16);
        size_t offset = ((address >> 16) & 0x7fu) * 0x8000u + (address & 0x7fffu);
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

    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0;
    edge_profile = argc > 2 && strcmp(argv[2], "--edges") == 0;
    arithmetic_profile = argc > 2 && strcmp(argv[2], "--arithmetic") == 0;
    grid_profile = argc > 2 && strcmp(argv[2], "--grids") == 0;
    unsigned cases = grid_profile         ? 64u
                     : force_return       ? 128u
                     : edge_profile       ? 64u
                     : arithmetic_profile ? 65536u
                     : argc > 2           ? (unsigned)strtoul(argv[2], NULL, 10)
                                          : 1024u;
    if ((force_return || edge_profile || arithmetic_profile || grid_profile) &&
        argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0u;
    for (suite = 0u; suite < (edge_profile         ? 15u
                              : arithmetic_profile ? 11u
                                                   : 17u);
         ++suite) {
        if (grid_profile && suite != 0u && suite != 1u && suite != 4u && suite != 5u &&
            suite != 11u && suite != 12u)
            continue;
        if (arithmetic_profile && suite != 2u && suite != 3u && suite != 7u &&
            suite != 8u && suite != 9u && suite != 10u)
            continue;
        for (number = 0u; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS cave family%u entry%06X states%u cumulative%u\n", suite,
               entries[suite], cases, total);
        fflush(stdout);
    }
    printf("PASS%u original Cave grid helper and caller states\n", total);
    return 0;
}
