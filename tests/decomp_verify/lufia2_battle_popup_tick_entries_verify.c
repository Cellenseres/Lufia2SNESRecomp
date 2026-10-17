#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/field.h"
#include "lufia2/actor.h"
#include "lufia2/battle.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, force_return;
static unsigned child_calls[6], stop_child;
static uint32_t child_stop, child_stop_site;
static unsigned edge_mode, unwind_mode, unwind_visits[6];
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static const uint32_t entries[] = {0x81c3eeu, 0x81c477u, 0x81c652u, 0x81c35fu,
                                   0x81c600u};
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
    SnesVerifyBusPoke(&native, a, (uint8_t)v);
    SnesVerifyBusPoke(&native, a + 1u, (uint8_t)(v >> 8));
}
static void instruction(SnesVerifyBus *b, Interp816 *r) {
    if (!b->event_observer) {
        interp816_runOpcode(r);
        return;
    }
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

static void seed(Lufia2CpuState *cpu) {
    stop_child = 0u;
    child_stop_site = 0u;
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    cpu->program_bank = 0x81u;
    cpu->data_bank = (suite == 2u || suite == 4u)
                         ? (uint8_t[]){0u, 0x81u, 0x3fu, 0xbfu}[(number >> 6) & 3u]
                         : 0x7eu;
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 83u);
    cpu->y = (uint16_t)(number * 29u);
    cpu->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->accumulator_is_8_bit = 1u;
    cpu->index_is_8_bit = cpu->decimal = 0u;
    for (unsigned i = 0; i < 11u; ++i) {
        uint16_t record = (uint16_t)(0x0b00u + 0xc0u * i);
        word(0x0a64u + 2u * i, (number % 11u == i && number % 7u == 0u) ? 0u : record);
        native.wram[record + 0x0fu] = (uint8_t)(number / 11u);
        word(record + 0x11u, (uint16_t[]){0u, 1u, 2u, 19u, 20u, 21u, 100u, 0x7fffu,
                                          0xffffu}[(number + i) % 9u]);
        word(record + 0xbau, (uint16_t[]){0u, 1u, 0xfffeu, 0xffffu}[(number + i) & 3u]);
        native.wram[record + 0xbcu] = (uint8_t)number;
        word(record + 0x50u, (uint16_t)(i % 7u));
        unsigned slot = 0x1f60cu + 30u * i;
        native.wram[slot] = (uint8_t[]){0x80u, 4u, 3u, 2u, 0u}[(number + i) % 5u];
        native.wram[slot + 2u] = (uint8_t)((number + i) % 18u);
        for (unsigned k = 0; k < 13u; ++k)
            word(0x7e0000u + slot + 4u + 2u * k,
                 k == (number + i) % 14u ? (uint16_t)(number * 113u) : 0u);
    }
    native.wram[0x11deu] = (uint8_t)(number & 1u);
    word(0x1605u, (uint16_t)~number);
    word(0x1608u, (uint16_t)(0xfff0u + number));
    word(0x1607u, (uint16_t)number);
    word(0x160au, (uint16_t)(number * 7u));
    for (unsigned i = 0; i < 55u; ++i)
        native.wram[WRAM_RANDOM_TABLE + i] = (uint8_t)(number * 71u + i * 29u);
    native.wram[WRAM_RANDOM_NEXT_INDEX] =
        (uint8_t[]){0u, 52u, 53u, 54u, 55u, 255u}[number % 6u];
    native.wram[0u] = (uint8_t)(number & 1u ? 0x80u : 0u);
    native.wram[1u] = (uint8_t)(1u << ((number / 2u) % (number & 1u ? 6u : 5u)));
    native.wram[2u] = (uint8_t)number;
    native.wram[0x11u] = (uint8_t)(number % 256u);
    native.wram[0x12u] = (uint8_t)(number % 11u + 1u);
    native.wram[0x13u] = (uint8_t)(number % 256u);
    word(0x15u, (uint16_t[]){0u, 1u, 9u, 10u, 99u, 100u, 999u, 1000u, 9999u, 10000u,
                             0x7fffu, 0x8000u, 0xffffu}[number % 13u]);
    word(0x19u, (uint16_t)(30u * (number % 11u)));
    word(0x1cu, (uint16_t)(11u * (number % 11u)));
    if (suite == 3u)
        cpu->accumulator = (uint16_t)((cpu->accumulator & 0xff00u) | (number % 256u));

    if (edge_mode && suite == 0u) {
        native.wram[0x11u] =
            (uint8_t[]){0u, 1u, 2u, 3u, 4u, 255u}[(number / 256u) % 6u];
        unsigned offset =
            (unsigned[]){0u, 30u, 300u, 0x09f0u, 0xfff0u, 0xffffu}[number % 6u];
        word(0x19u, (uint16_t)offset);
        SnesVerifyBusPoke(&native, 0x7ff60eu + offset, (uint8_t)number);
    }
    if (edge_mode && suite == 1u) {
        native.wram[0x13u] = (uint8_t)(number / 13u);
        word(0x1cu, (uint16_t[]){0u, 110u, 0xffffu, 0xb0f5u, 0xd075u}[number % 5u]);
    }
    if (edge_mode && suite == 2u) {
        cpu->data_bank = (uint8_t)((number & 0x3fu) | (number & 0x40u ? 0x80u : 0u));
        native.wram[0u] = (uint8_t)(number & 0x80u);
        native.wram[1u] = (uint8_t)(number & 0x7fu);
        if (!number) {
            stop_child = 1u;
        }
    }
    if (unwind_mode && suite == 2u) {
        stop_child = (number / 256u) % 6u + 1u;
        if (number % 64u == 0u) {
            native.wram[0u] = native.wram[1u] = 0u;
            stop_child = 1u;
        }
    }
    word((uint16_t)(cpu->stack + 1u), (uint16_t)(0x9000u + number % 256u));
    if (suite == 3u)
        native.wram[(uint16_t)(cpu->stack + 3u)] = 0x82u;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0u;
    child_stop = 0u;
}

static uint8_t child(void *context, Lufia2CpuState *cpu, uint32_t target, uint32_t site,
                     uint8_t frame) {
    SnesVerifyBus *bus = context ? context : &native;
    static const uint32_t sites[] = {0x81c658u, 0x81c66fu, 0x81c688u,
                                     0x81c6adu, 0x81c6ebu, 0x81c718u};
    unsigned at;
    for (at = 0; at < 6u; ++at)
        if (site == sites[at])
            break;
    if (at == 6u && suite != 4u) {
        fprintf(stderr, "unexpected child%06X site%06X\n", target, site);
        exit(2);
    }
    if (at < 6u)
        ++child_calls[at];
    if ((stop_child && site == sites[stop_child - 1u]) || (suite == 4u && at == 6u)) {
        cpu->program_bank = (uint8_t)(target >> 16);
        child_stop = target;
        child_stop_site = site;
        if (at < 6u)
            ++unwind_visits[at];
        return 0u;
    }
    Interp816 *runner = interp816_init(bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!runner)
        exit(2);
    load(runner, cpu, target);
    uint32_t back = (site & 0xff0000u) | (uint16_t)(site + frame + 1u);
    uint16_t stack = (uint16_t)(cpu->stack + frame);
    unsigned steps;
    for (steps = 0; steps < 200000u; ++steps) {
        if (SnesVerifyPc24(runner) == back && runner->sp == stack)
            break;
        instruction(bus, runner);
    }
    if (steps == 200000u) {
        fprintf(stderr, "child ceiling site%06X PC%06X case%u\n", site,
                SnesVerifyPc24(runner), number);
        exit(1);
    }
    store(cpu, runner);
    interp816_free(runner);
    return 1u;
}

static Lufia2ExecutionResult invoke(const Lufia2Memory *memory, Lufia2CpuState *cpu) {
    switch (suite) {
    case 0u:
        return Lufia2BattleBuildSlotPopup(memory, cpu);
    case 1u:
        return Lufia2BattleBuildPopupRecord(memory, cpu);
    case 2u:
        return Lufia2BattleApplyTargetStatusTick(memory, cpu, child, NULL);
    case 3u:
        return Lufia2BattlePopups(memory, cpu);
    default:
        return Lufia2BattleStatusTick(memory, cpu, child, NULL);
    }
}
static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, entries[suite]);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result = invoke(&memory, &cpu);
    uint32_t stop = child_stop ? child_stop : result.pc;
    if (result.flow == LUFIA2_EXECUTION_RETURNED &&
        (force_return || number % 33u == 0u)) {
        uint16_t back = Pull8(&memory, &cpu);
        back |= (uint16_t)Pull8(&memory, &cpu) << 8;
        if (suite == 3u)
            cpu.program_bank = Pull8(&memory, &cpu);
        stop = ((uint32_t)cpu.program_bank << 16) | (uint16_t)(back + 1u);
    }
    unsigned steps;
    uint32_t previous = 0u;
    for (steps = 0; steps < 2000000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(reference);
        if (pc == stop && (!child_stop || previous == child_stop_site))
            break;
        previous = pc;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 2000000u || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(
            stderr,
            "popup%06X case%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X flow%u\n",
            entries[suite], number, steps, stop, SnesVerifyPc24(reference),
            memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, cpu.accumulator, reference->a,
            cpu.x, reference->x, cpu.y, reference->y, cpu.stack, reference->sp,
            cpu.data_bank, reference->db, (unsigned)result.flow);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int VerifyPopupGuards(void);
static uint8_t poison_read(void *, uint32_t);
static void poison_write(void *, uint32_t, uint8_t);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifyPopupGuards();
    if (argc == 3 && strcmp(argv[2], "--null-child") == 0) {
        Lufia2CpuState cpu = {0};
        cpu.program_bank = 0x81u;
        cpu.data_bank = 0x81u;
        cpu.accumulator_is_8_bit = 1u;
        cpu.stack = 0x1f80u;
        Lufia2CpuState before = cpu;
        before.resume_pc = 0x81c652u;
        Lufia2Memory memory = {poison_read, poison_write, NULL, NULL, NULL};
        Lufia2ExecutionResult result =
            Lufia2BattleApplyTargetStatusTick(&memory, &cpu, NULL, NULL);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x81c652u ||
            memcmp(&before, &cpu, sizeof cpu))
            return 1;
        puts("PASS null child rejected before bus access");
        return 0;
    }

    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc > 4) {
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned at = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        if (at >= size)
            return 2;
        rom[at] = (uint8_t)strtoul(argv[4], NULL, 16);
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

    unsigned edge_tick = argc >= 3 && strcmp(argv[2], "--edge-tick") == 0;
    edge_mode = edge_tick || (argc >= 3 && strcmp(argv[2], "--edges") == 0);
    unwind_mode = argc >= 3 && strcmp(argv[2], "--unwinds") == 0;
    force_return = argc >= 3 && strcmp(argv[2], "--returns") == 0;
    unsigned entries_only = argc >= 3 && strcmp(argv[2], "--entries") == 0;
    unsigned callers_only = argc >= 3 && strcmp(argv[2], "--callers") == 0;
    unsigned mode =
        edge_mode || unwind_mode || force_return || entries_only || callers_only;
    unsigned cases = mode       ? 4096u
                     : argc > 2 ? (unsigned)strtoul(argv[2], NULL, 10)
                                : 4096u;
    if (mode && argc == 4)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0;
    unsigned first = callers_only ? 3u : (unwind_mode || edge_tick) ? 2u : 0u;
    unsigned last = entries_only || edge_mode || unwind_mode || force_return ? 3u : 5u;
    for (suite = first; suite < last; ++suite) {
        unsigned limit = edge_mode ? (unsigned[]){1536u, 3328u, 256u}[suite] : cases;
        for (number = 0; number < limit; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        printf("PASS popup%06X states%u cumulative%u\n", entries[suite], limit, total);
        fflush(stdout);
    }
    for (unsigned i = 0; i < 6u; ++i) {
        printf("child%u visits%u interrupted%u\n", i, child_calls[i], unwind_visits[i]);
        if (!edge_mode && !callers_only && cases >= 256u && !child_calls[i])
            return 1;
        if (unwind_mode && !unwind_visits[i])
            return 1;
    }
    printf("PASS%u popup and target-tick states\n", total);
    return 0;
}

static uint8_t poison_read(void *context, uint32_t at) {
    (void)context;
    fprintf(stderr, "guard read%06X\n", at);
    exit(1);
}
static void poison_write(void *context, uint32_t at, uint8_t value) {
    (void)context;
    (void)value;
    fprintf(stderr, "guard write%06X\n", at);
    exit(1);
}
static unsigned guard_count, consumer_guards;
static int check_guard(unsigned which, Lufia2CpuState state, unsigned emulation) {
    const Lufia2Memory memory = {poison_read, poison_write, NULL, NULL, NULL};
    uint32_t entry = (uint32_t[]){0x81c3eeu, 0x81c477u, 0x81c652u}[which];
#ifdef RUNTIME_ABI
    CpuState host;
    memset(&host, 0, sizeof host);
    runtime_load(&host, &state);
    host.emulation = (uint8_t)emulation;
    host.host_return_valid = 0;
    CpuState before = host;
    RuntimeCase test = {0};
    test.memory = &memory;
    test.start = state.stack;
    test.entry = entry;
    test.guard_only = 1;
    runtime_case = &test;
    RecompReturn result =
        ((RuntimeBridge[]){Lufia2DecompBridge_81C3EE, Lufia2DecompBridge_81C477,
                           Lufia2DecompBridge_81C652})[which](&host);
    uint32_t target = ((uint32_t)state.program_bank << 16) | (uint16_t)entry;
    if (result != RECOMP_RETURN_LLE_UNWIND_BASE || test.stub != 1u || test.kind != 3u ||
        test.target != target || test.source != target ||
        memcmp(&before, &host, sizeof host))
        return 1;
    ++consumer_guards;
#else
    (void)emulation;
    Lufia2CpuState before = state;
    before.resume_pc = entry;
    suite = which;
    Lufia2ExecutionResult result = invoke(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != entry ||
        memcmp(&before, &state, sizeof state))
        return 1;
    ++guard_count;
#endif
    return 0;
}
static int VerifyPopupGuards(void) {
    Lufia2CpuState valid = {0};
    valid.program_bank = 0x81u;
    valid.data_bank = 0x7eu;
    valid.accumulator_is_8_bit = 1u;
    valid.stack = 0x1f80u;
    valid.accumulator = 0x1234u;
    valid.x = 0xabcdu;
    valid.y = 0x9876u;
    valid.carry = valid.irq_disable = 1u;
    for (unsigned which = 0; which < 3u; ++which) {
        valid.data_bank = which == 2u ? 0x81u : 0x7eu;
        for (unsigned bank = 0; bank < 256u; ++bank)
            if (which == 2u ? (bank & 0x7fu) >= 0x40u : bank != 0x7eu) {
                Lufia2CpuState state = valid;
                state.data_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }

        for (unsigned stack = 0; stack < 65536u; ++stack)
            if (stack < 0x1f00u || stack > 0x1ffcu) {
                Lufia2CpuState state = valid;
                state.stack = (uint16_t)stack;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        for (unsigned page = 1; page < 65536u; ++page) {
            Lufia2CpuState state = valid;
            state.direct_page = (uint16_t)page;
            if (check_guard(which, state, 0u))
                return 1;
        }
        for (unsigned bank = 0; bank < 256u; ++bank)
            if (bank != 0x81u) {
                Lufia2CpuState state = valid;
                state.program_bank = (uint8_t)bank;
                if (check_guard(which, state, 0u))
                    return 1;
            }
        unsigned limit = 8u;
#ifdef RUNTIME_ABI
        limit = 16u;
#endif
        for (unsigned flags = 0; flags < limit; ++flags)
            if (flags != 1u) {
                Lufia2CpuState state = valid;
                state.accumulator_is_8_bit = (uint8_t)(flags & 1u);
                state.index_is_8_bit = (uint8_t)((flags >> 1) & 1u);
                state.decimal = (uint8_t)((flags >> 2) & 1u);
                if (check_guard(which, state, flags >> 3))
                    return 1;
            }
    }
    printf("PASS semantic%u/actualproducer%u pre-access guards\n", guard_count,
           consumer_guards);
    return 0;
}
