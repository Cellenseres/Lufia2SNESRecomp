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
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, boundaries, unwind_mode, original_route_visits,
    empty_window;
static const uint16_t handlers[] = {0x8351u, 0xab51u, 0xbc55u, 0xb7feu,
                                    0x9ec8u, 0xaca7u, 0xbc3du};
static Lufia2CpuState native_child_entry, original_child_entry;
static unsigned native_children, original_children;
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_818351(CpuState *);
extern RecompReturn Lufia2DecompBridge_9CB8(CpuState *);
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

static void seed(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    native_children = original_children = original_route_visits = 0;
    n->program_bank = suite ? 0x80u : 0x81u;
    n->accumulator_is_8_bit = 1u;
    n->accumulator = (uint16_t)((number * 197u & 0xff00u) | (number % 32u));
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(0x9000u + number % 16u * 32u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f20u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f80u + number % 64u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    static const uint8_t banks[] = {0, 0x7e, 0x7f, 0x80, 0x83, 0x97};
    n->data_bank = banks[(number / 32u) % 6u];
    if (suite)
        n->data_bank = number % 2u ? 0x7eu : 0x83u;
    uint16_t slot = (uint16_t)(number % 16u);
    uint8_t wanted = (uint8_t)(0x50u + slot);
    word(0xa7u, slot);
    word(0xa9u, slot * 2u);
    word(0xabu, slot * 3u);
    for (unsigned i = 0; i < 40u; ++i) {
        native.wram[0x05fau + i] = (uint8_t)(0x50u + i);
        native.wram[0x0622u + i] = 0;
        native.wram[0x0736u + i] = (uint8_t)(number % 2u ? 0x20u : 0);
    }
    native.wram[0x05acu] = suite ? 0xf0u : (uint8_t)(number / 32u);
    native.wram[0x05b3u] = (uint8_t[]){0, 0x10, 0x18, 0x08}[(number / 16u) % 4u];
    native.wram[0x05b6u] = (uint8_t)(number % 4u ? 1u : 0u);
    native.wram[0x09b9u] = n->data_bank;
    for (unsigned i = 0; i < 32u; ++i)
        native.wram[0x1f966u + i] = (uint8_t)(number % 8u);
    word(0x7ef010u, 0x100u);
    for (unsigned i = 0; i < 32u; ++i) {
        unsigned a = 0xf100u + i * 8u;
        native.wram[a] = (uint8_t)(i + 1u);
        native.wram[a + 1u] = (uint8_t)(i * 7u);
        native.wram[a + 2u] = (uint8_t)(i * 11u);
    }
    native.wram[0xf200u] = 0xffu;
    for (unsigned i = 0; i < 4u; ++i) {
        native.wram[0xf600u + 8u * i] = (uint8_t)(i + 1u);
        native.wram[0x0a7bu + i] = (uint8_t)i;
    }
    native.wram[0xf600u + 8u * (number % 5u)] = 0xffu;
    native.wram[0x1e697u] = (uint8_t)(number % 16u);
    native.wram[0x09b7u] = 0;
    native.wram[0x09b8u] = 0x90;
    native.wram[0x099bu] = 1;
    native.wram[0x099cu] = 0;
    native.wram[0x09a7u] = 0;
    native.wram[0x1259u] = 0;
    native.wram[0x1d0ffu] = 0;
    native.wram[0x1255u] = 0x10;
    word(0x09b1u, 0xd000u);
    word(0x09b5u, 0xd000u);
    memset(native.wram + 0x9000u, 0, 2048u);
    unsigned script = n->y;
    native.wram[script] = wanted;
    native.wram[script + 1u] = (uint8_t)(number % 16u);
    if (suite == 3u) {
        native.wram[script] = (uint8_t)(number % 8u);
        for (unsigned j = 1; j < (empty_window ? 1u : 2u + number % 20u); ++j)
            native.wram[script + j] = (uint8_t)('A' + j % 26u);
        native.wram[script + (empty_window ? 1u : 2u + number % 20u)] = 0;
    }
    if (suite == 4u)
        native.wram[script] = (uint8_t)number;
#ifdef RUNTIME_ABI
    if (suite) {
        native.wram[0x099bu] = 0;
        native.wram[0x09b9u] = 0x7eu;
        if (n->stack < 0x1f40u)
            n->stack = 0x1f40u;
        const uint8_t *rom = native.rom;
        unsigned opcode = 0;
        for (; opcode < 205u; ++opcode) {
            unsigned a = 0x4a14u + opcode * 2u;
            if ((unsigned)(rom[a] | rom[a + 1u] << 8) == handlers[suite])
                break;
        }
        if (opcode == 205u)
            exit(2);
        memmove(native.wram + 0x9001u, native.wram + script, 64u);
        native.wram[0x9000u] = (uint8_t)opcode;
        unsigned end = suite == 2u || suite == 6u ? 0x9003u
                       : suite == 5u              ? 0x9002u
                                                  : 0x9001u;
        if (suite == 2u || suite == 6u) {
            native.wram[0x9001u] = wanted;
            native.wram[0x9002u] = (uint8_t)(number % 16u);
        }
        if (suite != 3u && suite != 4u) {
            native.wram[end] = 0x37u;
            native.wram[end + 1u] = 0xffu;
        }
        if (suite == 4u) {
            native.wram[0x099cu] = (uint8_t)(number % 2u ? 0x10u : 0u);
            native.wram[0x1d089u] = (uint8_t)(1u + number % 32u);
            native.wram[0x1d087u] = (uint8_t)(number % 8u);
            word(0x7fd085u, (uint16_t)(number % 64u * 2u));
        }
    }
#endif
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n) {
    OpLoadA(n, (uint16_t)(number * 17u));
    OpSta(m, n, 0x7ff110u);
    OpLdx(n, (uint16_t)(number * 31u));
    OpLdy(n, (uint16_t)(number * 11u));
    n->carry = (number >> 2) & 1u;
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != 0x8396u) {
        fputs("wrong encounter child frame\n", stderr);
        exit(1);
    }
    n->program_bank = 0x81u;
    if (number % 8u == 3u) {
        OpSepWidths(n, 0x30u);
        n->data_bank = 0x7fu;
        n->decimal = 1u;
    }
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    const Lufia2Memory *m = context;
    if (target != 0x81839fu || site != 0x818394u || frame != 2u) {
        fputs("wrong encounter child\n", stderr);
        exit(1);
    }
    ++native_children;
    native_child_entry = *n;
    if (unwind_mode) {
        n->program_bank = 0x81u;
        return 0u;
    }
    returned_child(m, n);
    return 1u;
}
static Lufia2ExecutionResult execute(const Lufia2Memory *m, Lufia2CpuState *n) {
    if (!suite) {
        Lufia2ExecutionResult r = Lufia2FieldGenerateEncounter(m, n, child, (void *)m);
        if (r.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
            r.flow = LUFIA2_EXECUTION_BOUNDARY;
            r.pc = 0x81839fu;
        }
        return r;
    }
    uint32_t pc;
    if (suite == 3u)
        pc = Lufia2TextExecuteScriptCommand(m, n, handlers[suite]);
    else if (suite == 4u)
        pc = Lufia2TextExecuteRenderCommand(m, n, handlers[suite]);
    else if (suite == 6u) {
        Lufia2TextNextByte(m, n, 0xbc40u);
        return ExecutionHandoff(n, 0x80bc41u);
    } else
        pc = Lufia2TextExecuteActorVisibility(m, n, handlers[suite]);
    return (Lufia2ExecutionResult){LUFIA2_EXECUTION_BOUNDARY, pc, 0u};
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    if (suite && pc == (0x800000u | handlers[suite]))
        ++original_route_visits;
    if (!suite && !unwind_mode && pc == 0x81839fu) {
        Lufia2CpuState n = {0};
        store(&n, r);
        original_child_entry = n;
        Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL, NULL};
        returned_child(&m, &n);
        load(r, &n, 0x818397u);
        ++original_children;
        return;
    }
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
    uint32_t entry = ((uint32_t)n.program_bank << 16) | handlers[suite];
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    load(r, &n, entry);
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    if (suite) {
        entry = 0x809cb8u;
        load(r, &n, entry);
    }
    q = runtime_invoke(&m, &n, suite ? NULL : child, (void *)&m,
                       suite ? Lufia2DecompBridge_9CB8 : Lufia2DecompBridge_818351,
                       suite ? Lufia2TextEngineStep : NULL,
                       suite ? NULL : Lufia2FieldGenerateEncounter, entry, suite, 3u);
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        q.flow = LUFIA2_EXECUTION_BOUNDARY;
        q.pc = 0x81839fu;
    }
#else
    q = execute(&m, &n);
#endif
    if (q.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++boundaries;
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(r) == q.pc) {
            if (!suite && unwind_mode) {
                ++original_children;
                original_child_entry = native_child_entry;
                store(&original_child_entry, r);
            }
            break;
        }
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 1000000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_children != original_children || (suite && !original_route_visits) ||
        (native_children && memcmp(&native_child_entry, &original_child_entry,
                                   sizeof native_child_entry))) {
        fprintf(stderr,
                "encounter%04X case%u steps%u flow%06X CPU%d WRAM%d trace%u/%u "
                "child%u/%u visits%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X "
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
    if (argc == 3 && strcmp(argv[2], "--empty-window") == 0) {
        suite = 3;
        empty_window = 1;
        for (number = 0; number < 128u; ++number)
            if (run(r))
                return 1;
        puts("PASS empty centered-window prefix128");
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--guards") == 0) {
        unsigned guards = 0;
        for (suite = 0; suite < 6u; ++suite) {
            for (unsigned variant = 0; variant < (suite ? 14u : 238u); ++variant) {
                number = 3u;
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
                else if (variant < 14)
                    n.stack = (uint16_t[]){0, 0x54, 0x1f0f, 0x1ffd}[variant - 10u];
                else
                    n.accumulator = (uint16_t)(32u + variant - 14u);
                Lufia2CpuState before = n;
                Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                  NULL};
                Lufia2ExecutionResult q;
                if (!suite) {
                    before.resume_pc = 0x818351u;
#ifdef RUNTIME_ABI
                    q = runtime_invoke(
                        &m, &n, child, (void *)&m, Lufia2DecompBridge_818351, NULL,
                        Lufia2FieldGenerateEncounter, 0x818351u, suite, 3u);
#else
                    q = execute(&m, &n);
#endif
                } else
                    q = execute(&m, &n);
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY ||
                    q.pc != (suite ? 0u : 0x818351u) || memcmp(&before, &n, sizeof n) ||
                    native_trace.count) {
                    fprintf(stderr, "guard%u/%u failed\n", suite, variant);
                    return 1;
                }
                ++guards;
            }
        }
        printf("PASS encounter context and invalid-slot guards:%u\n", guards);
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--unwind") == 0) {
        suite = 0;
        unwind_mode = 1;
        for (number = 0; number < 128u; ++number)
            if (run(r))
                return 1;
        puts("PASS encounter unwind128");
        return 0;
    }
    for (suite = 0; suite < 7u; ++suite) {
        if (selected_handler && selected_handler != handlers[suite])
            continue;
#ifndef RUNTIME_ABI
        if (suite == 6u)
            continue;
#endif
        unsigned count = 4096u;
#ifdef RUNTIME_ABI
        count = 1024u;
#endif
        if (!suite)
            count = 8192u;
        for (number = 0; number < count; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        printf("PASS encounter%04X:%u CPU/fullWRAM/bus states\n", handlers[suite],
               count);
        fflush(stdout);
    }
    printf("PASS encounter batch:%u states/%u boundaries\n", total, boundaries);
#ifdef RUNTIME_ABI
    for (unsigned i = 0; i < 7u; ++i)
        if (runtime_fallback[i])
            return 1;
#endif
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
