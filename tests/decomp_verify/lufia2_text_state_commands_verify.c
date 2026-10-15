#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"

typedef struct Trace {
    SnesVerifyBusEvent events[2048];
    unsigned count;
} Trace;

static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, exhaustive;
static const uint16_t handlers[] = {0xa3ffu, 0xabf9u, 0xac73u, 0xadecu, 0xb67du,
                                    0xb9b2u, 0xb9dau, 0xba00u, 0xb7a6u, 0xb7b4u,
                                    0xb7c2u, 0xb7efu, 0xb33eu};
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;

static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *trace = context;
    if (!write && trace == fetch_trace && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    if (trace->count == 2048u) {
        fputs("trace ceiling\n", stderr);
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
static void seed(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->stack = (uint16_t)(0x1f20u + number % 200u);
    n->program_bank = 0x80u;
    n->data_bank = (uint8_t)(number >> 4);
    n->accumulator_is_8_bit = 1u;
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    n->decimal = (number >> 5) & 1u;
    static const uint16_t cursors[] = {0x8000u, 0x9000u, 0xfffcu, 0xfffdu,
                                       0xfffeu, 0xffffu, 0x7fffu, 0x09b9u};
    n->y = number % 16u < 8u ? cursors[number % 8u]
                             : (uint16_t)(0x8000u + (number * 23u) % 0x7ff0u);
    native.wram[0x09b9u] = n->data_bank;
    if (exhaustive) {
        n->data_bank = 0x7eu;
        n->y = 0x9000u;
        n->decimal = (exhaustive == 2u);
        native.wram[0x09b9u] = 0x7eu;
        native.wram[0x9000u] = 0x4du;
        native.wram[0x9001u] = (uint8_t)number;
        native.wram[0x079eu + 0x4du] = (uint8_t)(number >> 8);
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    uint8_t opcode = SnesVerifyBusRead(&original, pc);
    --original_trace.count;
    fetch_trace = &original_trace;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][opcode];
    if (!fetch_left) {
        fprintf(stderr, "unknown opcode%02X at%06X\n", opcode, pc);
        exit(1);
    }
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
static int run(Interp816 *r) {
    Lufia2CpuState n;
    seed(&n);
    load(r, &n, 0x800000u | handlers[suite]);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    if (!Lufia2TextExecuteStateCommand(&memory, &n, handlers[suite]))
        return 1;
    unsigned steps;
    for (steps = 0; steps < 2048u; ++steps) {
        if (SnesVerifyPc24(r) == 0x809d00u)
            break;
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 2048u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(stderr,
                "text state handler%04X case%u exhaustive%u CPU%d WRAM%d traces%u/%u "
                "A%04X/%04X X%04X/%04X Y%04X/%04X C%u/%u V%u/%u D%u/%u\n",
                handlers[suite], number, exhaustive, memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, n.accumulator, r->a, n.x,
                r->x, n.y, r->y, n.carry, r->c, n.overflow, r->v, n.decimal, r->d);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 3 && argc != 4) ||
        !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 4) {
        uint32_t pc = (uint32_t)strtoul(argv[2], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[3], NULL, 16);
        if (rom[at] == value)
            return 2;
        rom[at] = value;
    }
    if (!SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    original.event_observer = observe;
    original.event_observer_context = &original_trace;
    native.event_observer = observe;
    native.event_observer_context = &native_trace;
    Interp816 *r = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
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

    unsigned total = 0;
    if (argc != 3) {
        for (suite = 0; suite < 13u; ++suite) {
            for (number = 0; number < 4096u; ++number) {
                if (run(r))
                    return 1;
                ++total;
            }
            printf("PASS text command%04X:4096 original-ROM states\n", handlers[suite]);
            fflush(stdout);
        }
        suite = 0;
        for (exhaustive = 1; exhaustive <= 2u; ++exhaustive) {
            for (number = 0; number < 65536u; ++number) {
                if (run(r))
                    return 1;
                ++total;
            }
            printf("PASS variable subtraction:65536 byte pairs, decimal%u\n",
                   exhaustive == 2u);
            fflush(stdout);
        }
    }
    exhaustive = 0;
    number = 0;
    Lufia2CpuState untouched;
    seed(&untouched);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    unsigned guards = 0;
    for (unsigned handler = 0; handler < 65536u; ++handler) {
        unsigned i = 0;
        for (; i < 13u; ++i)
            if (handler == handlers[i])
                break;
        if (i < 13u)
            continue;
        Lufia2CpuState state = untouched;
        if (Lufia2TextExecuteStateCommand(&memory, &state, (uint16_t)handler) ||
            memcmp(&state, &untouched, sizeof state) || native_trace.count)
            return 1;
        ++guards;
    }
    for (unsigned command = 0; command < 13u; ++command) {
        for (unsigned context = 0; context < 8u; ++context) {
            Lufia2CpuState state = untouched;
            if (context == 0u)
                state.accumulator_is_8_bit = 0;
            else if (context == 1u)
                state.index_is_8_bit = 1;
            else {
                static const uint16_t pages[] = {1u, 0xffu, 0x100u,
                    0xffffu, 0xffc0u, 0x1ecbu};
                state.direct_page = pages[context - 2u];
            }
            Lufia2CpuState before = state;
            if (Lufia2TextExecuteStateCommand(&memory, &state, handlers[command]) ||
                memcmp(&state, &before, sizeof state) || native_trace.count)
                return 1;
            ++guards;
        }
    }
    printf("PASS unknown/unsupported command guards:%u, CPU/memory untouched\n", guards);
    printf("PASS text state commands:%u CPU/full-WRAM/ordered bus states\n", total);
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
