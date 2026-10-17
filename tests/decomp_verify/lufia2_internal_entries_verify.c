#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "core/cpu_ops.h"
#include "lufia2/menu.h"
#include "lufia2/ancient_cave.h"
#include "lufia2/battle.h"
#include "lufia2/system.h"
#include "lufia2/title.h"
#include "system/wram.h"
#ifndef SELECT_COMPOSED_BANK
#define SELECT_COMPOSED_BANK 0x86u
#endif

typedef struct Trace {
    SnesVerifyBusEvent events[4000000];
    size_t count;
} Trace;
static SnesVerifyBus rb, nb;
static Interp816 *r;
static Trace rt, nt;
static Trace *fetch_trace = &rt;
static unsigned number, suite, forced, composed, wait_visits, force_return,
    budget_profile, alias_profile;
typedef Lufia2ExecutionResult (*Parent)(const Lufia2Memory *, Lufia2CpuState *,
                                        Lufia2PushedChildCall, void *);
typedef struct Entry {
    uint32_t pc, end;
    Parent run;
    uint8_t frame;
} Entry;
typedef struct Site {
    uint32_t pc, target;
    uint8_t frame;
    unsigned root, hits, first;
} Site;
static Site sites[] = {{0u, 0u, 0u, 0u, 0u, ~0u}};
enum { SITE_COUNT = 0u, PRIMARY_SITE_COUNT = 0u, ENTRY_COUNT = 4u };
static const unsigned width_modes[] = {3u, 3u, 0u, 0u};
static unsigned boundary_states, returned_states, real_rts, real_rtl;
static Lufia2ExecutionResult Leaf0(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2AncientCaveAppendSavedItem(m, n);
}
static Lufia2ExecutionResult Leaf1(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2AncientCaveTestBlueItem(m, n);
}
static Lufia2ExecutionResult Leaf2(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2BattleUploadVramQueue(m, n);
}
static Lufia2ExecutionResult Leaf3(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2BattleClearActionWorkspace(m, n);
}
static const Entry entries[] = {{0x848862u, 0x84886cu, Leaf0, 2u},
                                {0x84886du, 0u, Leaf1, 2u},
                                {0x858e98u, 0x858ed1u, Leaf2, 2u},
                                {0x859265u, 0x859274u, Leaf3, 3u}};
static void fail(const char *why, uint32_t pc) {
    fprintf(stderr, "battle action work suite%u case%u %s site%06X ROM%06X\n", suite,
            number, why, pc, SnesVerifyPc24(r));
    exit(1);
}
static int same_cpu(const Lufia2CpuState *n, int bank) {
    return n->accumulator == r->a && n->x == r->x && n->y == r->y &&
           n->stack == r->sp && n->direct_page == r->dp && n->data_bank == r->db &&
           (!bank || n->program_bank == r->k) && n->carry == r->c && n->zero == r->z &&
           n->negative == r->n && n->overflow == r->v && n->decimal == r->d &&
           n->irq_disable == r->i && n->accumulator_is_8_bit == r->mf &&
           n->index_is_8_bit == r->xf;
}
static void to_interp(const Lufia2CpuState *n, uint32_t pc) {
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
static void from_interp(Lufia2CpuState *n) {
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

static uint8_t lengths[4][256];
static unsigned fetch_left;
static uint32_t fetch_address;
static void observer(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *trace = context;
#ifdef RUNTIME_ABI
    if (!write && trace == &nt && runtime_return_reads)
        return;
#endif
    if (!write && trace == fetch_trace && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    if (trace->count == 4000000u)
        fail("trace cap", address);
    trace->events[trace->count++] =
        (SnesVerifyBusEvent){address, value, write ? 1u : 0u};
}
static void instruction(void) {
    uint32_t pc = SnesVerifyPc24(r);
    uint8_t op = SnesVerifyBusRead(&rb, pc);
    --rt.count;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][op];
    if (!fetch_left)
        exit(2);
    interp816_runOpcode(r);
    fetch_left = 0;
}
static void compare(const Lufia2CpuState *n, uint32_t pc, int bank) {
    if (!same_cpu(n, bank)) {
        Lufia2CpuState rc;
        from_interp(&rc);
        fprintf(
            stderr,
            "A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X PB%02X/%02X P%02X/%02X\n",
            n->accumulator, r->a, n->x, r->x, n->y, r->y, n->stack, r->sp, n->data_bank,
            r->db, n->program_bank, r->k, PackStatus(n), PackStatus(&rc));
        fail("CPU", pc);
    }
    if (memcmp(nb.wram, rb.wram, SNES_VERIFY_WRAM_SIZE))
        fail("WRAM", pc);
    if (nb.mmio_overflow || rb.mmio_overflow || nb.mmio_count != rb.mmio_count)
        fail("MMIO count", pc);
    if (nb.wm_address != rb.wm_address || nb.m7_a != rb.m7_a || nb.m7_b != rb.m7_b ||
        nb.m7_latch != rb.m7_latch || nb.multiply_a != rb.multiply_a ||
        nb.multiply_b != rb.multiply_b || nb.multiply_result != rb.multiply_result ||
        nb.divide_a != rb.divide_a || nb.divide_result != rb.divide_result)
        fail("device state", pc);
    for (size_t i = 0; i < nb.mmio_count; ++i)
        if (nb.mmio[i].address != rb.mmio[i].address ||
            nb.mmio[i].value != rb.mmio[i].value ||
            nb.mmio[i].write != rb.mmio[i].write)
            fail("MMIO order", pc);
    if (nt.count != rt.count)
        fail("write count", pc);
    for (size_t i = 0; i < nt.count; ++i)
        if (nt.events[i].write != rt.events[i].write ||
            nt.events[i].address != rt.events[i].address ||
            nt.events[i].value != rt.events[i].value)
            fail("bus order", pc);
    nt.count = rt.count = 0;
}

static void reach(uint32_t pc) {
    for (unsigned steps = 0; steps < 2000000u; ++steps) {
        uint32_t now = SnesVerifyPc24(r);
        if (now == pc)
            return;
        for (unsigned i = 0; i < SITE_COUNT; ++i)
            if (now == sites[i].pc)
                fail("extra/missing child", pc);
        instruction();
    }
    fail("reference step limit", pc);
}

static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t s, uint8_t f) {
    (void)b;
    (void)n;
    (void)s;
    (void)f;
}
static uint8_t child(void *x, Lufia2CpuState *n, uint32_t t, uint32_t s, uint8_t f) {
    (void)x;
    (void)n;
    (void)t;
    (void)s;
    (void)f;
    exit(1);
}
static void put(uint32_t address, uint8_t value) {
    unsigned bank = (address >> 16) & 255u, word = address & 65535u;
    if (bank == 0x7eu || bank == 0x7fu)
        nb.wram[(bank - 0x7eu) * 65536u + word] = value;
    else if ((bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)) && word < 0x2000u)
        nb.wram[word] = value;
}
static void word(uint32_t address, uint16_t value) {
    put(address, (uint8_t)value);
    put((address + 1u) & 0xffffffu, (uint8_t)(value >> 8));
}
static void seed(Lufia2CpuState *n) {
    for (size_t byte = 0u; byte < SNES_VERIFY_WRAM_SIZE; ++byte)
        nb.wram[byte] = (uint8_t)((byte * 23u + number * 47u) >> 3);
    memset(n, 0, sizeof *n);
    n->program_bank = (uint8_t)(entries[suite].pc >> 16);
    n->data_bank = (uint8_t)number;
    n->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 10) & 3u];
    n->accumulator = (uint16_t)(number * 137u + 0x48u);
    n->x = (uint16_t)(number * 33u + 0xf640u);
    n->y = (uint16_t)(number * 59u + 0x0101u);
    n->accumulator_is_8_bit = width_modes[suite] == 3u ? 0u : 1u;
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    word(0x60u, (uint16_t)(number * 73u));
    uint32_t db = (uint32_t)n->data_bank << 16;
    for (unsigned record = 0u; record < 16u; ++record) {
        word(db | (0x1a8fu + record * 6u),
             (uint16_t)(number >> (record % 8u) & 1u ? number * 137u + 1u : 0u));
        word(db | (0x1a91u + record * 6u), (uint16_t)(number * 73u + record * 119u));
        word(db | (0x1a93u + record * 6u), (uint16_t)(number * 19u + record * 43u));
    }
    if (alias_profile) {
        n->stack = 0x1f80u;
        word(0x60u, (uint16_t[]){0u, 1u, 0xffu, 0x3fffu, 0x4000u, 0x40feu, 0x40ffu,
                                 0x4100u, 0x4101u, 0x415fu, 0x4160u, 0x6081u, 0x60fbu,
                                 0xffffu}[number % 14u]);
    }
    word((uint16_t)(n->stack + 1u), (uint16_t)(0x9000u | (number & 255u)));
    if (entries[suite].frame == 3u)
        put((uint16_t)(n->stack + 3u), (uint8_t)(number * 73u));
    memcpy(rb.wram, nb.wram, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(&rb);
    SnesVerifyBusResetTrace(&nb);
    SnesVerifyBusResetMmio(&rb);
    SnesVerifyBusResetMmio(&nb);
    nt.count = rt.count = 0u;
    rb.wm_address = nb.wm_address = 0u;
    rb.m7_a = nb.m7_a = 0u;
    rb.m7_b = nb.m7_b = 0u;
    rb.m7_latch = nb.m7_latch = 0u;
    rb.multiply_a = nb.multiply_a = 0u;
    rb.multiply_b = nb.multiply_b = 0u;
    rb.multiply_result = nb.multiply_result = 0u;
    rb.divide_a = nb.divide_a = 0u;
    rb.divide_result = nb.divide_result = 0u;
}
static int valid_return(uint32_t pc) {
    return entries[suite].end ? pc == entries[suite].end
                              : pc == 0x848885u || pc == 0x848887u;
}
static void run(unsigned value, unsigned force) {
    number = value;
    forced = force;
    wait_visits = 0u;
    for (unsigned i = 0u; i < SITE_COUNT; ++i)
        sites[i].hits = 0u;
    Lufia2CpuState native;
    seed(&native);
    to_interp(&native, entries[suite].pc);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &nb, NULL, NULL};
    Lufia2ExecutionResult result = entries[suite].run(&memory, &native, child, &memory);
    if (force) {
        if (result.flow != LUFIA2_EXECUTION_CHILD_UNWOUND ||
            result.pc != sites[force - 1u].pc)
            fail("unwind flow", result.pc);
        compare(&native, result.pc, 1);
        return;
    }
    if (result.flow != LUFIA2_EXECUTION_RETURNED || !valid_return(result.pc))
        fail("return boundary", result.pc);
    reach(result.pc);
    compare(&native, result.pc, 1);
    ++returned_states;
    if (force_return) {
        if (entries[suite].frame == 2u) {
            SimulateRtsFrame(&memory, &native);
            ++real_rts;
        } else {
            (void)Pull8(&memory, &native);
            (void)Pull8(&memory, &native);
            native.program_bank = Pull8(&memory, &native);
            ++real_rtl;
        }
        instruction();
        compare(&native, SnesVerifyPc24(r), 1);
    }
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 5) {
        uint32_t pc = (uint32_t)strtoul(argv[3], NULL, 16);
        size_t at = ((pc >> 16) & 0x7fu) * 0x8000u + (pc & 0x7fffu);
        rom[at] = (uint8_t)strtoul(argv[4], NULL, 16);
    }
    if (!SnesVerifyBusInit(&rb, rom, size) || !SnesVerifyBusInit(&nb, rom, size))
        return 2;
    r = interp816_init(&rb, SnesVerifyBusRead, SnesVerifyBusWrite);
    interp816_reset(r);
    rb.event_observer = observer;
    rb.event_observer_context = &rt;
    nb.event_observer = observer;
    nb.event_observer_context = &nt;
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

    budget_profile = argc > 2 && strcmp(argv[2], "--budgets") == 0;
    alias_profile = argc > 2 && strcmp(argv[2], "--aliases") == 0;
    force_return = argc > 2 && strcmp(argv[2], "--returns") == 0 || alias_profile;
    composed = argc > 2 && strcmp(argv[2], "--composed") == 0;
    unsigned unwinds = argc > 2 && strcmp(argv[2], "--unwinds") == 0;
    unsigned cases = force_return     ? 1024u
                     : composed       ? 64u
                     : unwinds        ? 32u
                     : budget_profile ? 4u
                                      : 4096u;
    if (argc > 2 && !force_return && !composed && !unwinds && !budget_profile &&
        !alias_profile)
        cases = (unsigned)strtoul(argv[2], NULL, 10);
    if ((force_return || composed || unwinds || budget_profile || alias_profile) &&
        argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0u;
    if (unwinds) {
        for (unsigned which = 0u; which < PRIMARY_SITE_COUNT; ++which) {
            suite = sites[which].root;
            for (unsigned value = 0u; value < cases; ++value) {
                run(value * 513u, which + 1u);
                ++total;
            }
        }
    } else
        for (suite = 0u; suite < ENTRY_COUNT; ++suite) {
            if (alias_profile && suite != 0u)
                continue;
            for (unsigned value = 0u; value < cases; ++value) {
                run(value, 0u);
                ++total;
            }
            printf("PASS action work family%u entry%06X states%u cumulative%u\n", suite,
                   entries[suite].pc, cases, total);
            fflush(stdout);
        }
    printf("PASS%u action-work states; returned%u,boundaries%u,realRTS%u,realRTL%u\n",
           total, returned_states, boundary_states, real_rts, real_rtl);
    return 0;
}
