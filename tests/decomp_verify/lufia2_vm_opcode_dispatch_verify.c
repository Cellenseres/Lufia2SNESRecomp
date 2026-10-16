#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "core/cpu_ops.h"
#include "system/wram.h"

static SnesVerifyBus original, native;
typedef struct Trace {
    SnesVerifyBusEvent events[128];
    unsigned count;
} Trace;
static Trace reference_trace, native_trace;
static unsigned suite, number, child_calls, child_prefix;
static Lufia2CpuState child_input;
static uint8_t child_wram[SNES_VERIFY_WRAM_SIZE];
static Trace *fetch_trace;
static uint32_t fetch_address;
static unsigned fetch_left;
static const uint8_t opcode_lengths[4][256] = {
    {2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3,
     1, 1, 3, 3, 3, 4, 3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2,
     2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3,
     3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, 1, 2, 3, 2, 2, 2, 2, 2,
     1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2,
     3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1,
     3, 3, 3, 4, 3, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2,
     2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2, 2, 1, 3,
     1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4},
    {2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3,
     1, 1, 3, 3, 3, 4, 3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2,
     2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3,
     3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, 1, 2, 3, 2, 2, 2, 2, 2,
     1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2,
     3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1,
     3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2,
     2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3,
     1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4},
    {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3,
     1, 1, 3, 3, 3, 4, 3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2,
     2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3,
     3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, 1, 2, 3, 2, 2, 2, 2, 2,
     1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2,
     3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1,
     3, 3, 3, 4, 3, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2,
     2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2, 2, 1, 2,
     1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4},
    {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3,
     1, 1, 3, 3, 3, 4, 3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2,
     2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3,
     3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, 1, 2, 3, 2, 2, 2, 2, 2,
     1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2,
     3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1,
     3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2,
     2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2,
     1, 1, 3, 3, 3, 4, 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4}};

static const uint32_t entries[] = {0x80CC35u, 0x83E0FCu, 0x83E10Fu};
static const unsigned prefix_counts[] = {8u, 17u, 8u};
static const uint32_t prefix_pcs[3][17] = {
    {0x80CC35u, 0x80CC38u, 0x80CC39u, 0x80CC3Au, 0x80CC3Cu, 0x80CC3Du, 0x80CC3Eu,
     0x80CC3Fu},
    {0x83E0FCu, 0x83E0FDu, 0x83E0FFu, 0x83E103u, 0x83E104u, 0x83E105u, 0x83E109u,
     0x83E10Au, 0x83E10Eu, 0x83E10Fu, 0x83E110u, 0x83E113u, 0x83E114u, 0x83E115u,
     0x83E116u, 0x83E118u, 0x83E119u},
    {0x83E10Fu, 0x83E110u, 0x83E113u, 0x83E114u, 0x83E115u, 0x83E116u, 0x83E118u,
     0x83E119u}};

static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *t = context;
    if (t == fetch_trace && !write && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    if (t->count == 128u)
        abort();
    t->events[t->count++] = (SnesVerifyBusEvent){address, value, write ? 1u : 0u};
}
static void step(Interp816 *c, SnesVerifyBus *b, Trace *t) {
    uint32_t pc = SnesVerifyPc24(c);
    uint8_t opcode = b->rom[((pc >> 16) & 127u) * 32768u + (pc & 32767u)];
    fetch_trace = t;
    fetch_address = pc;
    fetch_left = opcode_lengths[(unsigned)c->mf * 2u + (unsigned)c->xf][opcode];
    interp816_runOpcode(c);
    if (fetch_left) {
        fprintf(stderr, "fetch length at%06X\n", pc);
        exit(1);
    }
}
static void load(Interp816 *c, const Lufia2CpuState *n, uint32_t pc) {
    c->a = n->accumulator;
    c->x = n->x;
    c->y = n->y;
    c->sp = n->stack;
    c->dp = n->direct_page;
    c->db = n->data_bank;
    c->k = (uint8_t)(pc >> 16);
    c->pc = (uint16_t)pc;
    c->c = n->carry;
    c->z = n->zero;
    c->n = n->negative;
    c->v = n->overflow;
    c->d = n->decimal;
    c->i = n->irq_disable;
    c->mf = n->accumulator_is_8_bit;
    c->xf = n->index_is_8_bit;
    c->e = c->irqWanted = c->nmiWanted = c->waiting = c->stopped = 0;
    interp816_set_brk_hook_enabled(c, false);
}
static void store(Lufia2CpuState *n, const Interp816 *c) {
    n->accumulator = c->a;
    n->x = c->x;
    n->y = c->y;
    n->stack = c->sp;
    n->direct_page = c->dp;
    n->data_bank = c->db;
    n->program_bank = c->k;
    n->carry = c->c;
    n->zero = c->z;
    n->negative = c->n;
    n->overflow = c->v;
    n->decimal = c->d;
    n->irq_disable = c->i;
    n->accumulator_is_8_bit = c->mf;
    n->index_is_8_bit = c->xf;
}
static int same(const Lufia2CpuState *n, const Interp816 *c) {
    return n->accumulator == c->a && n->x == c->x && n->y == c->y &&
           n->stack == c->sp && n->direct_page == c->dp && n->data_bank == c->db &&
           n->program_bank == c->k && n->carry == c->c && n->zero == c->z &&
           n->negative == c->n && n->overflow == c->v && n->decimal == c->d &&
           n->irq_disable == c->i && n->accumulator_is_8_bit == c->mf &&
           n->index_is_8_bit == c->xf;
}
static int bus_state(void) {
    return original.multiply_a == native.multiply_a &&
           original.multiply_b == native.multiply_b &&
           original.multiply_result == native.multiply_result &&
           original.divide_a == native.divide_a &&
           original.divide_result == native.divide_result &&
           original.wm_address == native.wm_address && original.m7_a == native.m7_a &&
           original.m7_b == native.m7_b && original.m7_latch == native.m7_latch;
}
static int trace_equal(unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        SnesVerifyBusEvent *a = &reference_trace.events[i],
                           *b = &native_trace.events[i];
        if (a->address != b->address || a->value != b->value || a->write != b->write) {
            fprintf(stderr,
                    "operand bus suite%u case%u event%u %06X=%02X/%u vs%06X=%02X/%u\n",
                    suite, number, i, a->address, a->value, a->write, b->address,
                    b->value, b->write);
            return 0;
        }
    }
    return 1;
}
static uint8_t reader(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                      uint8_t frame) {
    SnesVerifyBus *b = context;
    if (target != 0x80e8b9u || site != 0x80cc35u || frame != 2u)
        abort();
    child_input = *n;
    child_prefix = native_trace.count;
    memcpy(child_wram, b->wram, sizeof child_wram);
    Interp816 *c = interp816_init(b, SnesVerifyBusRead, SnesVerifyBusWrite);
    load(c, n, target);
    uint16_t stack = (uint16_t)(n->stack + 2u);
    unsigned steps = 0;
    do {
        if (++steps > 64u)
            abort();
        step(c, b, &native_trace);
    } while (SnesVerifyPc24(c) != 0x80cc38u || c->sp != stack);
    store(n, c);
    interp816_free(c);
    ++child_calls;
    return 1u;
}
static void poke(uint32_t address, uint8_t value) {
    (void)SnesVerifyBusPoke(&native, address, value);
}
static const uint16_t positions[64] = {
    0,      1,      0x7f,   0xff,   0x100,  0x1f1e, 0x1f1f, 0x1f20, 0x1ffc, 0x1ffd,
    0x1ffe, 0x1fff, 0x2000, 0x2100, 0x2134, 0x2135, 0x2136, 0x2140, 0x2141, 0x2142,
    0x2143, 0x2180, 0x2181, 0x2182, 0x2183, 0x4000, 0x4202, 0x4203, 0x4204, 0x4205,
    0x4206, 0x4210, 0x4212, 0x4214, 0x4215, 0x4216, 0x4217, 0x5000, 0x6000, 0x7000,
    0x7ffc, 0x7ffd, 0x7ffe, 0x7fff, 0x8000, 0x8001, 0x8080, 0x8fff, 0x9000, 0xa000,
    0xc000, 0xd134, 0xdef0, 0xe5a4, 0xe7a2, 0xe8b9, 0xf000, 0xf2c8, 0xff00, 0xff7f,
    0xfffc, 0xfffd, 0xfffe, 0xffff};
static void seed(Lufia2CpuState *n) {
    for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
        native.wram[i] = (uint8_t)((i * 23u + number * 47u) ^ (i >> 7));
    memset(n, 0, sizeof *n);
    n->program_bank = (uint8_t)(suite == 0u ? 0x80u : 0x83u);
    n->accumulator_is_8_bit = 1u;
    n->data_bank = 0x7eu;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = 0x3000u;
    n->stack = (uint16_t)(0x1f20u + number % 0xddu);
    n->carry = (number >> 9) & 1u;
    n->zero = (number >> 10) & 1u;
    n->negative = (number >> 11) & 1u;
    n->overflow = (number >> 8) & 1u;
    n->irq_disable = (number >> 12) & 1u;
    unsigned opcode = number & 255u, record = number & 65535u;
    if (suite == 1u) {
        if (number < 131072u)
            n->data_bank = (uint8_t)(number >> 8);
        else {
            record = ((number - 131072u) % 32u) * 3u;
            n->data_bank = (uint8_t)((number - 131072u) / 64u);
        }
        uint8_t script_bank = (number & 65536u) ? 0x7fu : 0x7eu;
        uint16_t pointer = (uint16_t)(0x3000u + (number & 255u));
#ifdef RUNTIME_ABI
        record = (number % 32u) * 3u;
        if (number >= 131072u) {
            script_bank = (uint8_t)(0x80u + ((number >> 5) % 0x50u));
            pointer = (uint16_t)(0x8000u | (number * 7u & 0x7fffu));
        }
#endif
        uint32_t address = (0x7fdeeeu + record) & 0xffffffu;
        poke(address, (uint8_t)pointer);
        poke((address + 1u) & 0xffffffu, (uint8_t)(pointer >> 8));
        poke((address + 2u) & 0xffffffu, script_bank);
        poke(((uint32_t)script_bank << 16) | pointer, (uint8_t)opcode);
        poke(0xabu, (uint8_t)record);
        poke(0xacu, (uint8_t)(record >> 8));
    } else if (number >= 8192u && number < 139264u) {
        unsigned value = number - 8192u;
        n->data_bank = (value >> 16) ? 0x7fu : 0x7eu;
        n->y = (uint16_t)value;
    } else if (number >= 139264u) {
        unsigned value = number - 139264u;
        n->data_bank = (uint8_t)(value / 64u);
        n->y = positions[value % 64u];
#ifdef RUNTIME_ABI
        n->data_bank = (uint8_t)(0x80u + (n->data_bank % 0x50u));
        n->y |= 0x8000u;
#endif
    }
    if (suite != 1u)
        poke(((uint32_t)n->data_bank << 16) | n->y, (uint8_t)opcode);
    if (suite == 0u)
        poke(WRAM_FIELD_EVENT_RUNNING_BANK, (uint8_t)(number * 13u));
    native.multiply_a = (uint8_t)number;
    native.multiply_b = (uint8_t)(number >> 8);
    native.divide_a = (uint16_t)(number * 97u);
    native.multiply_result = (uint16_t)(native.divide_a % 3u);
    native.divide_result = (uint16_t)(native.divide_a / 3u);
    native.wm_address = number & 0x1ffffu;
    native.m7_a = (uint16_t)(number * 73u);
    native.m7_b = (uint8_t)(number * 17u);
    native.m7_latch = (uint8_t)(number * 3u);
    uint8_t *ram = original.wram;
    void *context = original.event_observer_context;
    original = native;
    original.wram = ram;
    original.event_observer_context = context;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(&original);
    SnesVerifyBusResetTrace(&native);
    SnesVerifyBusResetMmio(&original);
    SnesVerifyBusResetMmio(&native);
    reference_trace.count = native_trace.count = child_prefix = 0u;
}
static Lufia2ExecutionResult invoke(const Lufia2Memory *m, Lufia2CpuState *n) {
    switch (suite) {
    case 0:
        return Lufia2FieldDispatchEventOpcode(m, n, reader, m->context);
    case 1:
        return Lufia2ObjectBeginOpcodeDispatch(m, n);
    default:
        return Lufia2ObjectDispatchOpcode(m, n);
    }
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 3 && argc != 5) || !SnesVerifyLoadFile(argv[1], &rom, &size) ||
        size != 0x280000u)
        return 2;
    suite = (unsigned)strtoul(argv[2], NULL, 10);
    if (suite > 2u)
        return 2;
    if (argc == 5) {
        uint32_t pc = (uint32_t)strtoul(argv[3], NULL, 16);
        rom[((pc >> 16) & 127u) * 32768u + (pc & 32767u)] =
            (uint8_t)strtoul(argv[4], NULL, 16);
    }
    if (!SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    original.event_observer = native.event_observer = observe;
    original.event_observer_context = &reference_trace;
    native.event_observer_context = &native_trace;
    Interp816 *c = interp816_init(&original, SnesVerifyBusRead, SnesVerifyBusWrite);
    Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    static const unsigned counts[] = {155648u, 147456u, 155648u};
    for (number = 0u; number < counts[suite]; ++number) {
        Lufia2CpuState n;
        seed(&n);
        load(c, &n, entries[suite]);
        Lufia2ExecutionResult result = invoke(&m, &n);
        unsigned start = 0u;
        if (suite == 0u) {
            step(c, &original, &reference_trace);
            if (SnesVerifyPc24(c) != 0x80e8b9u || !same(&child_input, c) ||
                memcmp(original.wram, child_wram, sizeof child_wram) ||
                reference_trace.count != child_prefix || !trace_equal(child_prefix)) {
                fprintf(stderr, "reader child input mismatch case%u\n", number);
                return 1;
            }
            unsigned steps = 0u;
            do {
                if (++steps > 64u)
                    abort();
                step(c, &original, &reference_trace);
            } while (SnesVerifyPc24(c) != 0x80cc38u);
            start = 1u;
        }
        for (unsigned i = start; i < prefix_counts[suite]; ++i) {
            if (SnesVerifyPc24(c) != prefix_pcs[suite][i]) {
                fprintf(stderr, "prefix flow suite%u case%u at%06X\n", suite, number,
                        SnesVerifyPc24(c));
                return 1;
            }
            step(c, &original, &reference_trace);
        }
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.dispatches != 1u ||
            result.pc != SnesVerifyPc24(c) || n.resume_pc != result.pc ||
            !same(&n, c) || memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) ||
            reference_trace.count != native_trace.count ||
            !trace_equal(reference_trace.count) || !bus_state() ||
            original.mmio_overflow || native.mmio_overflow ||
            original.mmio_count != native.mmio_count) {
            fprintf(stderr,
                    "dispatch mismatch suite%u case%u pc%06X/%06X CPU%d bus%u/%u "
                    "A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X P%02X\n",
                    suite, number, result.pc, SnesVerifyPc24(c), same(&n, c),
                    native_trace.count, reference_trace.count, n.accumulator, c->a, n.x,
                    c->x, n.y, c->y, n.stack, c->sp, PackStatus(&n));
            return 1;
        }
        for (size_t i = 0; i < original.mmio_count; ++i) {
            SnesVerifyBusEvent *a = &original.mmio[i], *b = &native.mmio[i];
            if (a->address != b->address || a->value != b->value ||
                a->write != b->write)
                return 1;
        }
    }
    printf("PASS dispatch suite%u %u original-ROM states;CPU/fullWRAM/MMIO/all operand "
           "bus/target;actual reader calls%u\n",
           suite, counts[suite], child_calls);
    interp816_free(c);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
