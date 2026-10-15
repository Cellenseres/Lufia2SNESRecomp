#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"
#include "lufia2/party.h"
#include "lufia2/field.h"
#include "lufia2/battle.h"
#include "lufia2/system.h"
#include "lufia2/text.h"
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, boundaries, unwind_mode, original_route_visits;
static const uint16_t handlers[] = {0xbf12u, 0xbf43u, 0xa350u, 0xa480u, 0xa4cbu};
static const uint32_t targets[] = {0x8385dcu, 0x83afcdu, 0x80a368u};
static const uint32_t sites[] = {0x80a35cu, 0x80a360u, 0x80a364u};
static unsigned native_children, original_children;
static Lufia2CpuState native_child_entry[3], original_child_entry[3];
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_80BF12(CpuState *);
extern RecompReturn Lufia2DecompBridge_80BF43(CpuState *);
extern RecompReturn Lufia2DecompBridge_80A350(CpuState *);
extern RecompReturn Lufia2DecompBridge_9CB8(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_80BF12, Lufia2DecompBridge_80BF43, Lufia2DecompBridge_80A350};
#endif
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
    if (trace->count == 400000u) {
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

static uint16_t romword(uint32_t address) {
    unsigned a = ((address >> 16) & 127u) * 32768u + (address & 32767u);
    return (uint16_t)(native.rom[a] | native.rom[a + 1u] << 8);
}

static void seed(Lufia2CpuState *n) {
    static const uint16_t amounts[] = {0,      1,      2,      0xff,   0x100,  0x7fff,
                                       0x8000, 0x9660, 0x967e, 0x967f, 0x9680, 0xff00,
                                       0xfffe, 0xffff, 0x1234, 0xabcd};
    static const uint16_t balances[] = {0,      1,      2,      0xfe,   0xff,   0x100,
                                        0x7fff, 0x8000, 0x9660, 0x967e, 0x967f, 0x9680,
                                        0xff00, 0xfffe, 0xffff, 0x1234};
    static const uint8_t high[] = {0, 1, 0x97, 0x98, 0x99, 0x7f, 0xfe, 0xff};
    static const uint8_t banks[] = {0, 0x7e, 0x7f, 0x80, 0x83, 0x81};
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    memset(native_child_entry, 0, sizeof native_child_entry);
    memset(original_child_entry, 0, sizeof original_child_entry);
    native_children = original_children = original_route_visits = 0;
    n->program_bank = 0x80u;
    n->accumulator_is_8_bit = 1u;
    n->accumulator =
        number < 16384u ? amounts[number % 16u] : (uint16_t)(number - 16384u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(0x9000u + number % 16u * 64u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f20u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f80u + number % 64u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    n->data_bank = banks[(number / 2048u) % 6u];
    uint32_t base = (uint32_t)n->data_bank << 16;
    word(base | 0x0a8au, balances[(number / 16u) % 16u]);
    SnesVerifyBusPoke(&native, base | 0x0a8cu, high[(number / 256u) % 8u]);
    if (suite >= 3u) {
        n->data_bank = 0x7eu;
        word(0x7e0a8au, balances[(number / 16u) % 16u]);
        native.wram[0xa8cu] = high[(number / 256u) % 8u];
        native.wram[0x099bu] = native.wram[0x099cu] = native.wram[0x1259u] =
            native.wram[0x1d0ffu] = 0;
        native.wram[0x0b54u] = 30u;
        native.wram[0x05b5u] = 0;
        word(0x09b7u, 0x9000u);
        native.wram[0x09b9u] = 0x7eu;
        memset(native.wram + 0x9000u, 0, 2048u);
        native.wram[0x9000u] = (uint8_t)(suite == 3u ? 0x22u : 0x26u);
        word(0x9001u, amounts[number % 16u]);
        native.wram[0x9003u] = 0x37u;
        native.wram[0x9004u] = 0xffu;
        if (n->stack < 0x1f40u)
            n->stack = 0x1f40u;
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n, unsigned index) {
    if (index == 2u) {
        Lufia2ExecutionResult result = Lufia2SceneResetTextState(m, n);
        if (result.flow != LUFIA2_EXECUTION_RETURNED || result.pc != 0x80a371u)
            exit(1);
    } else {
        OpLoadA(n, (uint16_t)(number * 17u));
        OpSta(m, n, 0x7ff110u + index * 2u);
        OpLdx(n, (uint16_t)(number * 31u));
        OpLdy(n, (uint16_t)(number * 11u));
        if (number % 8u == 3u) {
            SetAccumulatorWidth(n, 0u);
            SetIndexWidth(n, 1u);
        } else {
            SetAccumulatorWidth(n, 1u);
            SetIndexWidth(n, 0u);
        }
        n->data_bank = (uint8_t[]){0x80, 0x7e, 0x7f, 0x83}[(number + index) % 4u];
        n->carry = (number + index) & 1u;
        n->overflow = (number >> 1) & 1u;
    }
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(sites[index] + (index == 2u ? 2u : 3u)))
        exit(1);
    if (index != 2u && Pull8(m, n) != 0x80u)
        exit(1);
    n->program_bank = 0x80u;
}
#ifdef RUNTIME_ABI
static void controlled(SnesVerifyBus *bus, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    unsigned i = 0;
    for (; i < 3u; ++i)
        if (sites[i] == site)
            break;
    if (i == 3u || frame != (i == 2u ? 2u : 3u))
        exit(1);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, bus, NULL, NULL};
    n->program_bank = (uint8_t)(targets[i] >> 16);
    returned_child(&m, n, i);
}
#endif
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned index = native_children++;
    if (index > 2u || target != targets[index] || site != sites[index] ||
        frame != (index == 2u ? 2u : 3u))
        exit(1);
    n->program_bank = (uint8_t)(target >> 16);
    native_child_entry[index] = *n;
    if (unwind_mode == index + 1u)
        return 0u;
    returned_child(context, n, index);
    return 1u;
}
static Lufia2ExecutionResult execute(const Lufia2Memory *m, Lufia2CpuState *n) {
    if (!suite)
        return Lufia2GoldAdd(m, n);
    if (suite == 1u)
        return Lufia2GoldSpend(m, n);
    if (suite == 2u)
        return Lufia2SceneReloadAndFadeIn(m, n, child, (void *)m);
    return Lufia2TextEngineStep(m, n);
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    if (suite >= 3u && pc == (0x800000u | handlers[suite]))
        ++original_route_visits;
    if (suite == 2u)
        for (unsigned i = 0; i < 3u; ++i)
            if (pc == targets[i]) {
                Lufia2CpuState n = {0};
                store(&n, r);
                original_child_entry[i] = n;
                ++original_children;
                if (i != 2u) {
                    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &original,
                                      NULL, NULL};
                    returned_child(&m, &n, i);
                    load(r, &n, sites[i] + 4u);
                    return;
                }
            }
    uint8_t opcode = SnesVerifyBusRead(&original, pc);
    --original_trace.count;
    fetch_trace = &original_trace;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][opcode];
    if (!fetch_left)
        exit(1);
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
    uint32_t entry = suite >= 3u ? 0x809cb8u : 0x800000u | handlers[suite];
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    load(r, &n, entry);
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    if (suite == 2u && number == 0u && !unwind_mode) {
        for (unsigned i = 0; i < 3u; ++i) {
            runtime_probe_desired_site = sites[i];
            runtime_probe(&m, &n, bridges[2], entry, 2u, 2u);
        }
        runtime_probe_desired_site = 0;
    }
    RuntimeLeaf leaf = suite == 0u   ? Lufia2GoldAdd
                       : suite == 1u ? Lufia2GoldSpend
                                     : Lufia2TextEngineStep;
    q = runtime_invoke(&m, &n, suite == 2u ? child : NULL, (void *)&m,
                       suite < 3u ? bridges[suite] : Lufia2DecompBridge_9CB8, leaf,
                       suite == 2u ? Lufia2SceneReloadAndFadeIn : NULL, entry, suite,
                       suite < 3u ? 2u : 3u);
#else
    q = execute(&m, &n);
#endif
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        q.flow = LUFIA2_EXECUTION_BOUNDARY;
        q.pc = targets[unwind_mode - 1u];
    }
    if (q.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++boundaries;
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(r) == q.pc && (steps || q.pc != entry)) {
            if (suite == 2u && unwind_mode) {
                unsigned i = unwind_mode - 1u;
                store(&original_child_entry[i], r);
                ++original_children;
            }
            break;
        }
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 1000000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_children != original_children ||
        (suite >= 3u && !original_route_visits) ||
        memcmp(native_child_entry, original_child_entry, sizeof native_child_entry)) {
        fprintf(stderr,
                "gold%04X case%u steps%u flow%06X CPU%d WRAM%d trace%u/%u "
                "children%u/%u visits%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X "
                "PB%02X/%02X DB%02X/%02X\n",
                handlers[suite], number, steps, q.pc, memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, native_children,
                original_children, original_route_visits, n.accumulator, r->a, n.x,
                r->x, n.y, r->y, n.stack, r->sp, n.program_bank, r->k, n.data_bank,
                r->db);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 3 && argc != 4 && argc != 6) ||
        !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    unsigned selected_handler = 0;
    if ((argc == 4 || argc == 6) && strcmp(argv[2], "--suite") == 0) {
        selected_handler = (unsigned)strtoul(argv[3], NULL, 16);
    }
    if ((argc == 4 && !selected_handler) || argc == 6) {
        unsigned argument = argc == 6 ? 4u : 2u;
        uint32_t pc = (uint32_t)strtoul(argv[argument], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[argument + 1u], NULL, 16);
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
    if (argc == 3 && strcmp(argv[2], "--guards") == 0) {
        unsigned guards = 0;
        for (suite = 0; suite < 3u; ++suite)
            for (unsigned variant = 0; variant < 14u; ++variant) {
                number = 3;
                Lufia2CpuState n;
                seed(&n);
                if (variant == 0)
                    n.accumulator_is_8_bit = 0;
                else if (variant == 1)
                    n.index_is_8_bit = 1;
                else if (variant == 2)
                    n.program_bank = 0;
                else if (variant == 3)
                    n.decimal = 1;
                else if (variant < 10)
                    n.direct_page = (uint16_t[]){
                        1, 0xff, 0x100, 0xffff, 0xffc0, 0x1ecb}[variant - 4u];
                else
                    n.stack = (uint16_t[]){0, 0x54, 0x1f1f, 0x1ffd}[variant - 10u];
                Lufia2CpuState before = n;
                before.resume_pc = 0x800000u | handlers[suite];
                Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                  NULL};
                Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
                q = runtime_invoke(&m, &n, suite == 2u ? child : NULL, (void *)&m,
                                   bridges[suite],
                                   suite == 0u ? Lufia2GoldAdd : Lufia2GoldSpend,
                                   suite == 2u ? Lufia2SceneReloadAndFadeIn : NULL,
                                   before.resume_pc, suite, 2u);
#else
                q = execute(&m, &n);
#endif
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
                    memcmp(&before, &n, sizeof n) || native_trace.count)
                    return 1;
                ++guards;
            }
        suite = 2u;
        number = 0;
        Lufia2CpuState n;
        seed(&n);
        Lufia2CpuState before = n;
        before.resume_pc = 0x80a350u;
        Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
        Lufia2ExecutionResult q = Lufia2SceneReloadAndFadeIn(&m, &n, NULL, NULL);
        if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
            memcmp(&before, &n, sizeof n) || native_trace.count)
            return 1;
        printf("PASS gold context guards:%u\n", guards + 1u);
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--unwind") == 0) {
        suite = 2u;
        for (unwind_mode = 1u; unwind_mode <= 3u; ++unwind_mode)
            for (number = 0; number < 64u; ++number)
                if (run(r))
                    return 1;
        puts("PASS scene child unwind192");
        return 0;
    }
    for (suite = 0; suite < 5u; ++suite) {
        if (selected_handler && selected_handler != handlers[suite])
            continue;
        unsigned count = suite < 2u ? 81920u : suite == 2u ? 1024u : 4096u;
#ifdef RUNTIME_ABI
        count = suite < 2u ? 16384u : suite == 2u ? 256u : 1024u;
#endif
        for (number = 0; number < count; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        printf("PASS gold%04X:%u CPU/fullWRAM/bus states\n", handlers[suite], count);
        fflush(stdout);
    }
    printf("PASS gold batch:%u states/%u boundaries\n", total, boundaries);
#ifdef RUNTIME_ABI
    for (unsigned i = 0; i < 5u; ++i)
        if (runtime_fallback[i])
            return 1;
#endif
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
