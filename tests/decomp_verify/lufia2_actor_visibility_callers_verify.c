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
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, boundaries, unwind_mode;
static unsigned slot_mode;
static const uint16_t handlers[] = {0xbf6fu, 0xbaf6u, 0xbb17u, 0xa8f8u,
                                    0xaca7u, 0xddb5u, 0xddccu};
static unsigned native_children, original_children;
static unsigned original_route_visits;
static Lufia2CpuState native_child_entry, original_child_entry;
static uint32_t child_handoff;
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_80BF6F(CpuState *);
extern RecompReturn Lufia2DecompBridge_8EBAF6(CpuState *);
extern RecompReturn Lufia2DecompBridge_8EBB17(CpuState *);
extern RecompReturn Lufia2DecompBridge_CBAE(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_80BF6F, Lufia2DecompBridge_8EBAF6, Lufia2DecompBridge_8EBB17};
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
    memset(&native_child_entry, 0, sizeof native_child_entry);
    memset(&original_child_entry, 0, sizeof original_child_entry);
    child_handoff = 0;
    n->program_bank = suite == 1u || suite == 2u ? 0x8eu : 0x80u;
    n->accumulator_is_8_bit = 1u;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(number % 8u == 7u ? 0xffffu : 0x9000u + number % 8u * 32u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f10u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f40u + number % 128u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    static const uint8_t banks[] = {0, 0x7e, 0x7f, 0x80, 0x83, 0x81};
    n->data_bank = banks[(number / 16u) % 6u];
    uint16_t slot = (uint16_t)(number % 40u);
    if (slot_mode)
        slot = (uint16_t[]){40u, 255u, 256u, 65535u,
                            (uint16_t)(n->stack + 1u - 0x0622u),
                            (uint16_t)(n->stack + 2u - 0x0736u), 0xfa85u,
                            0xf96fu}[number % 8u];
    uint8_t wanted = (uint8_t)(0x50u + slot);
    word(0xa7u, slot);
    word(0xa9u, (uint16_t)(slot * 2u));
    word(0xabu, (uint16_t)(slot * 3u));
    for (unsigned k = 0; k < 6u; ++k) {
        uint32_t bank = (uint32_t)banks[k] << 16;
        SnesVerifyBusPoke(&native, bank | 0x09b9u, n->data_bank);
        for (unsigned i = 0; i < 40u; ++i) {
            SnesVerifyBusPoke(&native, bank | 0x05fau + i, (uint8_t)(0x50u + i));
            SnesVerifyBusPoke(&native, bank | 0x0622u + i,
                              (uint8_t)((number + i * 7u) & 0xfbu));
            SnesVerifyBusPoke(&native, bank | 0x0736u + i, (uint8_t)(number * 13u + i));
            SnesVerifyBusPoke(&native, bank | 0x06bau + i, (uint8_t)(i % 8u));
            SnesVerifyBusPoke(&native, bank | 0x06e2u + i, (uint8_t)((i / 8u) % 8u));
            SnesVerifyBusPoke(&native, bank | 0x070au + i, (uint8_t)(number % 16u));
        }
    }
    for (unsigned i = 0; i < 40u; ++i) {
        native.wram[0x1e216u + i] = (uint8_t)(i % 4u);
        native.wram[0x1e25eu + i] = (uint8_t)(i * 4u);
        native.wram[0x1e2a6u + i] = 0;
    }
    memset(native.wram + (WRAM_SPRITE_ALLOCATION_FLAGS - 0x7e0000u), 0, 128u);
    word(0x0005b9u, 8u);
    word(0x0005bbu, 8u);
    uint32_t script = ((uint32_t)n->data_bank << 16) | n->y;
    if (suite == 0u)
        n->accumulator = (uint16_t)((n->accumulator & 0xff00u) | wanted);
    if (suite == 3u || suite == 4u) {
        SnesVerifyBusPoke(&native, script, wanted);
        SnesVerifyBusPoke(&native, script + 1u, (uint8_t)(number % 16u));
    }
    if (suite >= 5u) {
        SnesVerifyBusPoke(&native, script, (uint8_t)(wanted - 0x4fu));
        word(EVENT_SCRIPT_POINTER, n->y);
        SnesVerifyBusPoke(&native, EVENT_SCRIPT_BANK, n->data_bank);
    }
    if (number % 8u == 0u) {
        for (unsigned i = 0; i < 40u; ++i)
            SnesVerifyBusPoke(&native, ((uint32_t)n->data_bank << 16) | 0x0622u + i,
                              4u);
    }
#ifdef RUNTIME_ABI
    if (suite >= 3u)
        n->data_bank = number % 3u == 0u ? 0x7eu : number % 3u == 1u ? 0x83u : 0u;
    if (n->stack < 0x1f40u)
        n->stack = 0x1f40u;
    if (suite == 3u || suite == 4u) {
        native.wram[0x09b7u] = 0;
        native.wram[0x09b8u] = 0x90;
        native.wram[0x09b9u] = 0x7e;
        native.wram[0x099bu] = native.wram[0x099cu] = native.wram[0x1259u] =
            native.wram[0x1d0ffu] = 0;
        native.wram[0x9000u] = suite == 3u ? 52u : 183u;
        native.wram[0x9001u] = wanted;
        native.wram[0x9002u] = (uint8_t)(number % 16u);
        unsigned end = suite == 3u ? 0x9003u : 0x9002u;
        native.wram[end] = 0x37u;
        native.wram[end + 1u] = 0xffu;
    }
    if (suite >= 5u) {
        memset(native.wram + 0x1d18cu, 0, 8u);
        native.wram[0x1d18cu] = 0x81u;
        word(0x7fd134u, 0x9000u);
        native.wram[0x1d136u] = 0x7eu;
        native.wram[0x9000u] = (uint8_t)(suite == 5u ? 0x60u : 0x61u);
        native.wram[0x9001u] = (uint8_t)(wanted - 0x4fu);
        native.wram[0x9002u] = 0;
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
    n->carry = (number >> 2) & 1u;
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    uint8_t bank = Pull8(m, n);
    if (back != 0xbb15u || bank != 0x8eu) {
        fputs("wrong child frame\n", stderr);
        exit(1);
    }
    n->program_bank = bank;
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    const Lufia2Memory *m = context;
    if (target != 0x818351u || site != 0x8ebb12u || frame != 3u) {
        fputs("wrong visibility child\n", stderr);
        exit(1);
    }
    ++native_children;
    native_child_entry = *n;
    native_child_entry.program_bank = 0x81u;
    if (unwind_mode) {
        child_handoff = target;
        n->program_bank = 0x81u;
        return 0u;
    }
    returned_child(m, n);
    return 1u;
}
static Lufia2ExecutionResult execute(const Lufia2Memory *m, Lufia2CpuState *n) {
    if (suite == 0u)
        return Lufia2FieldFindWindowActor(m, n);
    if (suite == 1u) {
        Lufia2ExecutionResult r = Lufia2FieldRevealActor(m, n, child, (void *)m);
        if (r.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
            r.flow = LUFIA2_EXECUTION_BOUNDARY;
            r.pc = child_handoff;
        }
        return r;
    }
    if (suite == 2u)
        return Lufia2FieldHideActor(m, n);
    uint32_t pc;
    if (suite < 5u)
        pc = Lufia2TextExecuteActorVisibility(m, n, handlers[suite]);
    else {
        EventRun run = {0};
        uint32_t handoff = 0;
        unsigned flow = Lufia2EventActorOpcode(m, n, handlers[suite], &run, &handoff);
        pc = flow == EVENT_OPCODE_NEXT ? 0x80cc35u : handoff;
    }
    return (Lufia2ExecutionResult){LUFIA2_EXECUTION_BOUNDARY, pc, 0u};
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    if (suite >= 3u && pc == (0x800000u | handlers[suite]))
        ++original_route_visits;
    if (suite == 1u && !unwind_mode && pc == 0x818351u) {
        Lufia2CpuState n = {0};
        store(&n, r);
        original_child_entry = native_child_entry;
        store(&original_child_entry, r);
        Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL, NULL};
        returned_child(&m, &n);
        load(r, &n, 0x8ebb16u);
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
    if (suite >= 3u) {
        entry = suite < 5u ? 0x809cb8u : 0x80cbaeu;
        load(r, &n, entry);
    }
    q = runtime_invoke(&m, &n, suite == 1u ? child : NULL, (void *)&m,
                       suite < 3u   ? bridges[suite]
                       : suite < 5u ? Lufia2DecompBridge_9CB8
                                    : Lufia2DecompBridge_CBAE,
                       suite == 0u   ? Lufia2FieldFindWindowActor
                       : suite == 2u ? Lufia2FieldHideActor
                       : suite < 5u  ? Lufia2TextEngineStep
                                     : Lufia2FieldEventTimerTick,
                       suite == 1u ? Lufia2FieldRevealActor : NULL, entry, suite, 3u);
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        q.flow = LUFIA2_EXECUTION_BOUNDARY;
        q.pc = child_handoff;
        n.program_bank = (uint8_t)(child_handoff >> 16);
    }
#else
    q = execute(&m, &n);
#endif
    if (q.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++boundaries;
    uint32_t flow = q.pc;
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(r) == flow) {
            if (unwind_mode && flow == 0x818351u)
                ++original_children;
            if (unwind_mode && flow == 0x818351u) {
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
        native_children != original_children ||
        (suite >= 3u && !original_route_visits) ||
        (native_children && memcmp(&native_child_entry, &original_child_entry,
                                   sizeof native_child_entry))) {
        fprintf(stderr,
                "visibility%04X case%u steps%u flow%06X CPU%d WRAM%d trace%u/%u "
                "children%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X PB%02X/%02X "
                "DB%02X/%02X\n",
                handlers[suite], number, steps, flow, memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, native_children,
                original_children, n.accumulator, r->a, n.x, r->x, n.y, r->y, n.stack,
                r->sp, n.program_bank, r->k, n.data_bank, r->db);
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
    if (argc == 3 && strcmp(argv[2], "--slots") == 0) {
        slot_mode = 1u;
        for (suite = 1u; suite < 3u; ++suite)
            for (number = 0u; number < 128u; ++number)
                if (run(r))
                    return 1;
        printf("PASS wide/stack-alias actor slots:256 states/%u boundaries\n", boundaries);
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--unwind") == 0) {
        unwind_mode = 1u;
        suite = 1u;
        for (number = 0; number < 128u; ++number)
            if (run(r))
                return 1;
        printf("PASS visibility child unwind:128 states/%u boundaries\n", boundaries);
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--guards") == 0) {
        unsigned guards = 0u;
        for (suite = 0u; suite < 5u; ++suite) {
            number = 3u;
            Lufia2CpuState base;
            seed(&base);
            Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
            for (unsigned variant = 0u; variant < 14u; ++variant) {
                Lufia2CpuState n = base;
                if (variant == 0u)
                    n.accumulator_is_8_bit = 0u;
                else if (variant == 1u)
                    n.index_is_8_bit = 1u;
                else if (variant == 2u)
                    n.program_bank = 0u;
                else if (variant == 3u)
                    n.decimal = 1u;
                else if (variant < 10u)
                    n.direct_page = (uint16_t[]){1, 0xff, 0x100, 0xffff, 0xffc0,
                                                  0x1ecb}[variant - 4u];
                else
                    n.stack = (uint16_t[]){0, 0x54, 0x1f0f, 0x1ffd}[variant - 10u];
                Lufia2CpuState before = n;
                uint32_t entry = ((uint32_t)base.program_bank << 16) | handlers[suite];
                if (suite < 3u)
                    before.resume_pc = entry;
                Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
                if (suite < 3u)
                    q = runtime_invoke(&m, &n, suite == 1u ? child : NULL, (void *)&m,
                                       bridges[suite],
                                       suite == 0u ? Lufia2FieldFindWindowActor
                                                    : Lufia2FieldHideActor,
                                       suite == 1u ? Lufia2FieldRevealActor : NULL,
                                       entry, suite, 3u);
                else
#endif
                    q = execute(&m, &n);
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY ||
                    q.pc != (suite < 3u ? entry : 0u) ||
                    memcmp(&before, &n, sizeof n) || native_trace.count)
                    return 1;
                ++guards;
            }
        }
        for (unsigned opcode = 0u; opcode < 205u; ++opcode) {
            unsigned at = 0x4a14u + 2u * opcode;
            uint16_t handler = (uint16_t)(rom[at] | ((uint16_t)rom[at + 1u] << 8));
            if (handler == 0xa8f8u || handler == 0xaca7u)
                continue;
            suite = 3u;
            number = 3u;
            Lufia2CpuState n;
            seed(&n);
            Lufia2CpuState before = n;
            Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
            if (Lufia2TextExecuteActorVisibility(&m, &n, handler) ||
                memcmp(&before, &n, sizeof n) || native_trace.count)
                return 1;
            ++guards;
        }
        printf("PASS visibility context/other-ROM-route guards:%u\n", guards);
        return 0;
    }
    for (suite = 0; suite < 7u; ++suite) {
        if (selected_handler && selected_handler != handlers[suite])
            continue;
        unsigned count = 4096u;
#ifdef RUNTIME_ABI
        count = 1024u;
#endif
        for (number = 0; number < count; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        printf("PASS visibility%04X:%u CPU/fullWRAM/bus states\n", handlers[suite],
               count);
        fflush(stdout);
    }
    printf("PASS visibility batch:%u states/%u boundaries\n", total, boundaries);
#ifdef RUNTIME_ABI
    for (unsigned index = 0u; index < 7u; ++index)
        if (runtime_fallback[index])
            return 1;
#endif
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
