#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"
#include "lufia2/party.h"
#include "lufia2/item.h"
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, boundaries, edge_mode;
static const uint16_t handlers[] = {0xf629u, 0xfd15u, 0xfa57u, 0xfa77u, 0xfaa0u,
                                    0xfad0u, 0xfcedu, 0xfc97u, 0xfcc7u, 0xa49du,
                                    0xa4a6u, 0xa4ddu, 0xa50bu, 0xfcadu};
typedef Lufia2ExecutionResult (*Leaf)(const Lufia2Memory *, Lufia2CpuState *);
static const Leaf leaves[] = {Lufia2PartyClearStatuses,
                              Lufia2PartyClearEquipment,
                              Lufia2ItemRemoveRequested,
                              Lufia2ItemClearRequestedPossession,
                              Lufia2InventoryIncreaseSlotQuantity,
                              Lufia2ItemSubtractSlotQuantity,
                              Lufia2ItemSelectPackedCount,
                              Lufia2ItemIncrementPackedCount,
                              Lufia2ItemDecrementPackedCount};
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_81F629(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FD15(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FA57(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FA77(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FAA0(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FAD0(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FCED(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FC97(CpuState *);
extern RecompReturn Lufia2DecompBridge_82FCC7(CpuState *);
static const RuntimeBridge bridges[] = {
    Lufia2DecompBridge_81F629, Lufia2DecompBridge_82FD15, Lufia2DecompBridge_82FA57,
    Lufia2DecompBridge_82FA77, Lufia2DecompBridge_82FAA0, Lufia2DecompBridge_82FAD0,
    Lufia2DecompBridge_82FCED, Lufia2DecompBridge_82FC97, Lufia2DecompBridge_82FCC7};
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

static void word(uint32_t at, uint16_t v) {
    SnesVerifyBusPoke(&native, at, (uint8_t)v);
    SnesVerifyBusPoke(&native, at + 1u, (uint8_t)(v >> 8));
}
static uint16_t readword(uint32_t at) {
    uint16_t v = SnesVerifyBusRead(&native, at);
    return v | ((uint16_t)SnesVerifyBusRead(&native, at + 1u) << 8);
}
static void seed(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    n->program_bank = suite == 0u ? 0x81u : suite < 9u || suite == 13u ? 0x82u : 0x80u;
    n->accumulator_is_8_bit = suite == 0u || suite >= 9u;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(number * 121u);
    static const uint8_t banks[] = {0, 0x7e, 0x7f, 0x80, 0x83, 0x81};
    n->data_bank = banks[(number / 16u) % 6u];
    if (suite == 4u)
        n->data_bank = 0x7eu;
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f10u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f30u + number % 128u);
    uint32_t db = (uint32_t)n->data_bank << 16;
    for (unsigned bank = 0; bank < 6u; ++bank) {
        uint32_t at = (uint32_t)banks[bank] << 16;
        for (unsigned member = 0; member < 4u; ++member) {
            uint16_t record = readword(0x859ebau + member * 2u);
            word(at | 0xa80u + member * 2u, number % 8u == member ? 0u : record);
            SnesVerifyBusPoke(&native, at | record + 15u,
                              (uint8_t)(number + member * 31u));
            word(at | record + 0x25u, (uint16_t)(number * 47u + member));
            word(at | record + 0x27u, (uint16_t)(number * 17u + member));
        }
    }
    uint16_t item = (uint16_t)(number % 512u);
    if (number % 4u < 2u)
        item = readword(0x97fda0u + 2u * ((number / 4u) % 32u));
    word(db | 0x09cfu, item);
    word(db | 0x09cdu, (uint16_t)(number % 128u));
    for (unsigned i = 0; i < 96u; ++i)
        word(db | 0x0a8du + 2u * i,
             (uint16_t)(((number + i) % 128u << 9) | ((item + i + 1u) & 511u)));
    if (number % 4u == 2u)
        word(db | 0x0a8du + 2u * ((number / 4u) % 96u),
             (uint16_t)(item | (number % 128u << 9)));
    for (unsigned i = 0; i < 8u; ++i)
        word(db | 0x091eu + 2u * i, number % 4u == 0u ? 0xffffu : 0u);
    if (suite == 1u) {
        n->accumulator = (uint16_t)((number * 197u & 0xff00u) | number % 7u);
        uint16_t record = readword(0x859ebau + 2u * (number % 7u));
        for (unsigned slot = 0; slot < 6u; ++slot)
            word(db | record + 0x66u + slot * 2u,
                 (uint16_t)(number * 11u + slot * 73u));
    }
    if (suite == 4u || suite == 5u) {
        n->x = (uint16_t)(2u * ((number / 256u) % 96u));
        n->accumulator =
            (uint16_t)(((number * 67u) & 0xff00u) | ((number / 128u) & 127u));
        word(db | 0x0a8du + n->x,
             (uint16_t)((number % 128u << 9) | ((number & 1u) ? 511u : 0u)));
    }
    if (suite >= 6u && suite <= 8u) {
        n->x = (uint16_t)((number * 71u & 0xfe00u) | ((number / 8192u) * 2u) |
                          ((number / 4096u) & 1u));
        n->accumulator =
            (uint16_t)((number * 137u & 0xff00u) | ((number / 256u) % 16u));
        if (number >= 8192u)
            n->accumulator |= 0xf0u;
        native.wram[0x1f080u + ((n->x & 511u) >> 1)] = (uint8_t)number;
    }
    if (suite >= 9u && suite < 13u) {
        if (n->stack < 0x1f30u)
            n->stack = 0x1f30u;
        uint8_t low = (uint8_t)item;
        uint8_t qty = (uint8_t)(number % 128u);
        if (suite == 12u)
            low = (uint8_t)(number % 7u);
        uint32_t script = 0x7e9000u;
        word(db | 0x09b9u, n->data_bank);
        n->y = 0x9000u;
#ifdef RUNTIME_ABI
        n->y = (uint16_t)(number * 59u);
        n->data_bank = number % 3u == 0u ? 0x7eu : number % 3u == 1u ? 0x83u : 0u;
        native.wram[0x09b7u] = 0x00u;
        native.wram[0x09b8u] = 0x90u;
        native.wram[0x09b9u] = 0x7eu;
        native.wram[0x099bu] = native.wram[0x099cu] = native.wram[0x1259u] = 0u;
        script = 0x7e9001u;
        static const uint8_t opcodes[] = {0x24, 0x25, 0x28, 0xba};
        native.wram[0x9000u] = opcodes[suite - 9u];
        unsigned count = suite == 11u ? 0u : suite == 12u ? 1u : 2u;
        native.wram[0x9001u + count] = 0x37u;
        native.wram[0x9002u + count] = 0xffu;
#else
        script = ((uint32_t)n->data_bank << 16) | n->y;
#endif
        if (edge_mode) {
            n->stack = (uint16_t)(0x1f10u + number % 4u);
            if (suite == 12u)
                low = (uint8_t)(7u + number);
        }
        if (suite != 11u)
            SnesVerifyBusPoke(&native, script, low);
        if (suite == 9u || suite == 10u)
            SnesVerifyBusPoke(&native, script + 1u, qty);
    }
    if (suite == 13u) {
        n->accumulator_is_8_bit = 1u;
        n->decimal = (number >> 11) & 1u;
        n->index_is_8_bit = (number >> 10) & 1u;
        if (n->index_is_8_bit) {
            n->x &= 255u;
            n->y &= 255u;
        }
        static const uint16_t pages[] = {0, 0x100, 0x1e80, 0xffb0};
        n->direct_page = pages[(number >> 8) & 3u];
        SnesVerifyBusPoke(&native, (uint16_t)(n->direct_page + 0x54u), (uint8_t)number);
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&original);
    reset_devices(&native);
    original_trace.count = native_trace.count = 0;
}
static Lufia2ExecutionResult execute(const Lufia2Memory *m, Lufia2CpuState *c) {
    if (suite < 9u)
        return leaves[suite](m, c);
    if (suite == 13u) {
        AslDirect8(m, c, 0x54u);
        return ExecutionReturned(0x82fcafu);
    }
    uint32_t pc = Lufia2TextExecutePartyCommand(m, c, handlers[suite]);
    Lufia2ExecutionResult r = {LUFIA2_EXECUTION_BOUNDARY, pc, 0u};
    return r;
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
    uint32_t entry = ((uint32_t)n.program_bank << 16) | handlers[suite];
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    if (suite >= 9u)
        entry = 0x809cb8u;
    load(r, &n, entry);
    Lufia2ExecutionResult result = runtime_invoke(
        &memory, &n, NULL, NULL, suite < 9u ? bridges[suite] : Lufia2DecompBridge_9CB8,
        suite < 9u ? leaves[suite] : Lufia2TextEngineStep, NULL, entry, suite,
        suite == 0u || suite == 1u || suite == 2u || suite >= 9u ? 3u : 2u);
#else
    load(r, &n, entry);
    Lufia2ExecutionResult result = execute(&memory, &n);
#endif
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++boundaries;
    uint32_t flow = result.pc;
    unsigned steps, visits = 0;
    for (steps = 0; steps < 100000u; ++steps) {
        if (SnesVerifyPc24(r) == flow && (!(flow == 0x809d3bu || flow == 0x809d22u) ||
                                          ++visits == result.dispatches + 1u))
            break;
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 100000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "item handler%04X case%u steps%u flow%06X CPU%d WRAM%d trace%u/%u "
            "A%04X/%04X X%04X/%04X Y%04X/%04X C%u/%u N%u/%u Z%u/%u V%u/%u S%04X/%04X\n",
            handlers[suite], number, steps, flow, memcmp(&expected, &n, sizeof n),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, n.accumulator, r->a, n.x, r->x,
            n.y, r->y, n.carry, r->c, n.negative, r->n, n.zero, r->z, n.overflow, r->v,
            n.stack, r->sp);
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
    if (argc != 3)
        for (suite = 0; suite < 14u; ++suite) {
#ifdef RUNTIME_ABI
            if (suite == 13u)
                continue;
#endif
            if (selected_handler && selected_handler != handlers[suite])
                continue;
            unsigned count = suite == 4u || suite == 5u   ? 32768u
                             : suite >= 6u && suite <= 8u ? 12288u
                                                          : 4096u;
#ifdef RUNTIME_ABI
            count = 1024u;
#endif
            for (number = 0; number < count; ++number) {
                if (run(r))
                    return 1;
                ++total;
            }
            printf("PASS item/equipment%04X:%u CPU/full-WRAM/ordered bus states\n",
                   handlers[suite], count);
            fflush(stdout);
        }
#ifdef RUNTIME_ABI
    for (unsigned k = 0; k < 13u; ++k) {
        if (runtime_fallback[k])
            return 1;
    }
#else
    if (!selected_handler) {
        unsigned guards = 0;
        for (suite = 0; suite < 13u; ++suite) {
            if (suite == 4u)
                continue;
            number = 1;
            Lufia2CpuState base_state;
            seed(&base_state);
            Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                              NULL};
            unsigned contexts = suite < 9u ? 12u : 14u;
            for (unsigned k = 0; k < contexts; ++k) {
                Lufia2CpuState n = base_state;
                if (k == 0u)
                    n.accumulator_is_8_bit = !n.accumulator_is_8_bit;
                else if (k == 1u)
                    n.index_is_8_bit = 1;
                else if (k == 2u)
                    n.program_bank = 0;
                else if (k < 7u)
                    n.stack = (uint16_t[]){0, 0x54, 0x1f07, 0x1ffd}[k - 3u];
                else if (k == 7u && suite != 0u)
                    n.decimal = 1;
                else if (k >= 8u && suite != 0u)
                    n.direct_page =
                        (uint16_t[]){1, 0x100, 0xffff, 0xffc0, 0x1f00, 0x54}[k - 8u];
                else
                    continue;
                Lufia2CpuState before = n;
                if (suite < 9u)
                    before.resume_pc =
                        ((uint32_t)n.program_bank << 16) | handlers[suite];
                if (suite < 9u && k == 2u)
                    before.resume_pc =
                        (suite == 0u ? 0x810000u : 0x820000u) | handlers[suite];
                Lufia2ExecutionResult q = execute(&m, &n);
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY ||
                    memcmp(&before, &n, sizeof n) || native_trace.count) {
                    fprintf(stderr, "guard suite%u k%u CPU%d pc%06X trace%u\n", suite,
                            k, memcmp(&before, &n, sizeof n), q.pc, native_trace.count);
                    return 1;
                }
                ++guards;
            }
        }
        for (suite = 0; suite < 2u; ++suite) {
            unsigned chosen = suite;
            suite = chosen == 0u ? 1u : 5u;
            number = 1;
            Lufia2CpuState b;
            seed(&b);
            Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                              NULL};
            for (unsigned k = 0; k < 3u; ++k) {
                Lufia2CpuState n = b;
                if (chosen == 0u)
                    n.accumulator = (uint16_t)(7u + k * 124u);
                else
                    n.x = (uint16_t[]){1u, 192u, 65535u}[k];
                Lufia2CpuState before = n;
                before.resume_pc = 0x820000u | handlers[suite];
                Lufia2ExecutionResult q = execute(&m, &n);
                if (q.flow != LUFIA2_EXECUTION_BOUNDARY ||
                    memcmp(&before, &n, sizeof n) || native_trace.count)
                    return 1;
                ++guards;
            }
            suite = chosen;
        }
        printf("PASS item/equipment unsupported guards:%u\n", guards);
        if (argc == 3 && strcmp(argv[2], "--edges") == 0) {
            edge_mode = 1;
            for (suite = 9u; suite < 13u; ++suite) {
                for (number = 0; number < 64u; ++number)
                    if (run(r))
                        return 1;
            }
            printf("PASS text item callers:256 exact guarded child boundaries\n");
        }
    }
#endif

#ifdef RUNTIME_ABI
    if (!selected_handler) {
        unsigned guards = 0;
        for (suite = 0; suite < 9u; ++suite)
            for (unsigned kind = 0; kind < 7u; ++kind) {
                if (kind == 6u && suite == 0u)
                    continue;
                number = 1;
                Lufia2CpuState n;
                seed(&n);
                uint32_t entry =
                    (suite == 0u ? 0x810000u : 0x820000u) | handlers[suite];
                if (kind == 0u)
                    n.accumulator_is_8_bit = !n.accumulator_is_8_bit;
                else if (kind == 1u)
                    n.index_is_8_bit = 1u;
                else if (kind == 2u)
                    n.decimal = 1u;
                else if (kind == 3u)
                    n.program_bank = 0;
                else if (kind == 4u)
                    n.stack = 0x1ffdu;
                else if (kind == 5u)
                    n.stack = 0;
                else
                    n.direct_page = 0xffffu;
                Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL,
                                  NULL};
                RuntimeCase t = {0};
                t.memory = &m;
                t.entry = entry;
                t.start = n.stack;
                t.hrv = 3u;
                t.guard_only = 1u;
                runtime_case = &t;
                CpuState c;
                memset(&c, 0, sizeof c);
                runtime_load(&c, &n);
                c.host_return_valid = 3u;
                CpuState before = c;
                RecompReturn ret = bridges[suite](&c);
                before.open_bus = c.open_bus;
                runtime_return_reads = 0;
                if (ret != RECOMP_RETURN_LLE_UNWIND_BASE || t.kind != 3u ||
                    t.target != entry || t.observed || memcmp(&c, &before, sizeof c)) {
                    fprintf(stderr, "ABI guard suite%u kind%u failed\n", suite, kind);
                    return 1;
                }
                ++guards;
                runtime_case = NULL;
            }
        printf("PASS production ABI unsupported guards:%u\n", guards);
    }
#endif
    printf("PASS item/equipment batch:%u states/%u boundaries\n", total, boundaries);
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
