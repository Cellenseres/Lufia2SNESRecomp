#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "core/cpu_ops.h"
#include "lufia2/menu.h"
#include "lufia2/item.h"
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
static Site sites[] = {
    {0x81f18fu, 0x81f114u, 3u, 1u, 0u, ~0u}, {0x81ebfdu, 0x81bae8u, 3u, 2u, 0u, ~0u},
    {0x81ec29u, 0x859bdau, 3u, 2u, 0u, ~0u}, {0x8594e0u, 0x859d1cu, 3u, 3u, 0u, ~0u},
    {0x85ce40u, 0x85ce6fu, 2u, 4u, 0u, ~0u}, {0x85ce56u, 0x85cf68u, 2u, 4u, 0u, ~0u},
    {0x85ce59u, 0x85d025u, 2u, 4u, 0u, ~0u}, {0x85ce61u, 0x85d0cdu, 2u, 4u, 0u, ~0u},
    {0x85ce64u, 0x85d94fu, 2u, 4u, 0u, ~0u}, {0x85cf85u, 0x85cfabu, 2u, 5u, 0u, ~0u},
    {0x85d039u, 0x85d080u, 2u, 6u, 0u, ~0u}, {0x85d043u, 0x85d080u, 2u, 6u, 0u, ~0u},
    {0x85d057u, 0x85c168u, 2u, 6u, 0u, ~0u}, {0x85d9acu, 0x85d9c9u, 3u, 7u, 0u, ~0u}};
enum { SITE_COUNT = 14u, PRIMARY_SITE_COUNT = 14u, ENTRY_COUNT = 12u };
static const unsigned width_modes[] = {0u, 1u, 2u, 0u, 2u, 0u, 0u, 0u, 0u, 0u, 3u, 0u};
static const unsigned child_m[] = {1u, 1u, 0u, 0u, 1u, 1u, 1u,
                                   1u, 1u, 0u, 1u, 1u, 1u, 1u};
static unsigned boundary_states, returned_states, real_rts, real_rtl;
static uint8_t child(void *, Lufia2CpuState *, uint32_t, uint32_t, uint8_t);
static Lufia2ExecutionResult Leaf0(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2InventoryRemoveQuantity(m, n);
}
static Lufia2ExecutionResult Leaf8(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2BattleInvalidateTurnQueueActor(m, n);
}
static Lufia2ExecutionResult Leaf9(const Lufia2Memory *m, Lufia2CpuState *n,
                                   Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2BattleRemoveTurnQueueActor(m, n);
}
static Lufia2ExecutionResult Leaf10(const Lufia2Memory *m, Lufia2CpuState *n,
                                    Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2BattleAccumulateActionAdjustment(m, n);
}
static Lufia2ExecutionResult Leaf11(const Lufia2Memory *m, Lufia2CpuState *n,
                                    Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2BattleReadActorBiasFlag(m, n);
}
static const Entry entries[] = {
    {0x81f114u, 0u, Leaf0, 3u},
    {0x81f15bu, 0u, Lufia2InventoryConsumeRequestedItem, 3u},
    {0x81ebf4u, 0x81ec34u, Lufia2BattleResetTargetMotion, 3u},
    {0x85948cu, 0x8594e6u, Lufia2BattleRefreshInactiveEnemySprites, 3u},
    {0x85ce21u, 0x85ce6eu, Lufia2BattlePrepareAlternateAction, 3u},
    {0x85cf68u, 0x85cfaau, Lufia2BattleBuildActionAdjustments, 2u},
    {0x85d025u, 0x85d07fu, Lufia2BattleCalculateActionBias, 2u},
    {0x85d94fu, 0x85d9c8u, Lufia2BattleStoreActionResult, 2u},
    {0x85d9f0u, 0x85da10u, Leaf8, 3u},
    {0x85937du, 0x8593b6u, Leaf9, 3u},
    {0x85cfabu, 0x85d024u, Leaf10, 2u},
    {0x85d080u, 0u, Leaf11, 2u}};
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

static void controlled(SnesVerifyBus *bus, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, bus, NULL, NULL};
    unsigned which = 0u;
    while (which < SITE_COUNT && sites[which].pc != site)
        ++which;
    if (which == SITE_COUNT)
        exit(2);
    SetAccumulatorWidth(n, (uint8_t)child_m[which]);
    SetIndexWidth(n, 0u);
    OpLoadA(n, (uint16_t)(number * 37u + (site >> 1)));
    n->carry = (number ^ site) & 1u;
    n->overflow = ((number >> 1) ^ site) & 1u;
    n->irq_disable = ((number >> 4) ^ site) & 1u;
    if (site == 0x85ce40u)
        Write8(&memory, 0x7ff4e2u, (uint8_t)(forced || number & 1u ? 2u : 0u));
    if (site == 0x85d057u)
        OpLdx(n, (uint16_t)(0x1800u + (number % 6u) * 32u));
    if (site == 0x85d9acu)
        OpLdx(n, (uint16_t)(0xf60cu + (number % 11u) * 30u));
    Write8(&memory, 0x7e3d00u + (site & 255u), (uint8_t)(number + site));
    if (frame == 2u)
        SimulateRtsFrame(&memory, n);
    else
        SimulateRtlFrame(&memory, n);
    n->program_bank = (uint8_t)(site >> 16);
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    const Lufia2Memory *memory = context;
    unsigned which = 0u;
    while (which < SITE_COUNT && sites[which].pc != site)
        ++which;
    if (which == SITE_COUNT || sites[which].target != target ||
        sites[which].frame != frame)
        fail("child contract", site);
    reach(site);
    instruction();
    if (SnesVerifyPc24(r) != target)
        fail("encoded child target", site);
    n->program_bank = (uint8_t)(target >> 16);
    compare(n, site, 1);
    ++sites[which].hits;
    if (sites[which].first == ~0u)
        sites[which].first = number;
    if (forced == which + 1u)
        return 0u;
    if (composed &&
        (target == 0x81f114u || target == 0x85cf68u || target == 0x85cfabu ||
         target == 0x85d025u || target == 0x85d080u || target == 0x85d94fu ||
         target == 0x85d9c9u)) {
        Lufia2ExecutionResult result;
        switch (target) {
        case 0x81f114u:
            result = Lufia2InventoryRemoveQuantity(memory, n);
            break;
        case 0x85cf68u:
            result = Lufia2BattleBuildActionAdjustments(memory, n, child, context);
            break;
        case 0x85cfabu:
            result = Lufia2BattleAccumulateActionAdjustment(memory, n);
            break;
        case 0x85d025u:
            result = Lufia2BattleCalculateActionBias(memory, n, child, context);
            break;
        case 0x85d080u:
            result = Lufia2BattleReadActorBiasFlag(memory, n);
            break;
        case 0x85d94fu:
            result = Lufia2BattleStoreActionResult(memory, n, child, context);
            break;
        case 0x85d9c9u:
            result = Lufia2BattleEffectRecord(memory, n);
            break;
        default:
            exit(2);
        }
        if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
            return 0u;
        if (result.flow != LUFIA2_EXECUTION_RETURNED)
            fail("composed child flow", result.pc);
        reach(result.pc);
        compare(n, site, 1);
        if (frame == 2u)
            SimulateRtsFrame(memory, n);
        else
            SimulateRtlFrame(memory, n);
        n->program_bank = (uint8_t)(site >> 16);
        instruction();
        compare(n, site, 1);
        return 1u;
    }
    controlled(&nb, n, site, frame);
    Lufia2CpuState reference;
    from_interp(&reference);
    controlled(&rb, &reference, site, frame);
    to_interp(&reference, site + (frame == 3u ? 4u : 3u));
    compare(n, site, 1);
    return 1u;
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
    unsigned mode = width_modes[suite];
    n->accumulator_is_8_bit = mode == 3u                 ? 0u
                              : mode == 1u || mode == 2u ? (number >> 8) & 1u
                                                         : 1u;
    n->index_is_8_bit = mode == 2u ? (number >> 9) & 1u : 0u;
    if (n->index_is_8_bit) {
        n->x &= 255u;
        n->y &= 255u;
    }
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    if (forced) {
        n->data_bank = 0x97u;
        n->stack = 0x1f80u;
    }
    if (composed) {
        n->data_bank = 0x97u;
        n->stack = 0x1f80u;
        if (suite == 10u)
            n->data_bank = 0x7fu;
    }
    if (alias_profile) {
        n->data_bank = suite == 10u ? 0x7fu : 0x7eu;
        n->stack = 0x1f80u;
    }
    uint32_t db = (uint32_t)n->data_bank << 16;
    for (unsigned actor = 0u; actor < 6u; ++actor) {
        unsigned pointer = 0x1800u + actor * 32u;
        word(db | (0x0a6eu + actor * 2u),
             (uint16_t)(((number >> actor) & 1u) ? pointer : 0u));
        put(db | (pointer + 15u), (uint8_t)(number + actor));
        put(db | (pointer + 16u), (uint8_t)(number >> actor));
        put(db | (0x134bu + actor), (uint8_t)((number >> actor) & 3u));
    }
    word(db | 0x0a88u, (uint16_t)(number & 2u ? 0x18e0u : 0u));
    put(db | 0x18efu, (uint8_t)(number & 4u ? 4u : 0u));
    put(db | 0x0a13u, (uint8_t)(number & 1u ? 0xffu : 0u));
    put(db | 0x09fau, (uint8_t)(number % 12u + 1u));
    for (unsigned record = 0u; record < 13u; ++record) {
        put(db | (0x1b8cu + record * 3u),
            (uint8_t)(record == 12u                  ? 0u
                      : record % 4u == (number & 3u) ? number % 12u + 1u
                                                     : record + 1u));
        put(db | (0x1b8du + record * 3u), (uint8_t)(number + record));
        put(db | (0x1b8eu + record * 3u), (uint8_t)(number >> 3));
    }
    uint16_t wanted = (uint16_t)((number * 17u) & 0x1ffu);
    for (unsigned slot = 0u; slot < 96u; ++slot)
        word(db | (0x0a8du + slot * 2u), (uint16_t)(((wanted + slot + 1u) & 0x1ffu) |
                                                    ((slot + number) % 128u) << 9));
    if (suite == 0u) {
        unsigned slot = (number >> 4) & 255u;
        put(db | 0x0a08u, (uint8_t)slot);
        word(db | (0x0a8du + slot * 2u),
             (uint16_t)(wanted | (((number >> 4) * 37u + number) & 127u) << 9));
        n->accumulator = (uint16_t)((n->accumulator & 0xff00u) | (number & 255u));
    }
    word(db | 0x0a06u, (uint16_t)(wanted | ((number * 7u) & 127u) << 9));
    if (suite == 1u && (forced || !(number & 1u))) {
        unsigned slot = forced ? 95u : (number >> 1) % 96u;
        word(db | (0x0a8du + slot * 2u), (uint16_t)(wanted | 0x200u));
    }
    for (unsigned record = 0u; record < 11u; ++record) {
        word(0x7ff60cu + record * 30u,
             (uint16_t)(record % 3u == number % 3u ? 0x80u : 0u));
        for (unsigned byte = 2u; byte < 30u; ++byte)
            put(0x7ff60cu + record * 30u + byte,
                (uint8_t)(number + record * 11u + byte));
    }
    word(0x7ff4d6u, (uint16_t)(number % 128u));
    put(0x7ff450u, (uint8_t)((number >> 1) % 128u));
    put(0x7ff45au, (uint8_t)(number & 0xc0u));
    put(0x7ff4e2u, (uint8_t)(forced || number & 1u ? 2u : 0u));
    word(db | 0x0a62u, (uint16_t)(number * 3u));
    word(db | 0x0a2du, (uint16_t)(number * 5u));
    put(db | 0x0a35u, (uint8_t)(number * 7u));
    put(db | 0x0a36u, (uint8_t)(number * 11u));
    if (forced) {
        put(db | 0x0a13u, 0xffu);
        put(0x7ff45au, 0u);
        put(0x7ff450u, 1u);
        word(db | 0x0a88u, 0x18e0u);
        put(db | 0x18efu, 4u);
    }
    word(0xb8u, 0x4000u);
    put(0xbau, 0x7eu);
    if (suite == 10u)
        put(((db + n->x) & 0xffffffu), (uint8_t)(number & 1u ? 0x80u : 0u));
    if (alias_profile && suite == 10u) {
        n->x = (uint16_t[]){0xf44eu, 0xf45cu, 0xf45eu, 0xf460u,
                            0xf462u, 0xf464u, 0xf46cu, 0xfff8u}[number % 8u];
        put(db + n->x, 0x80u);
    }
    if (alias_profile && suite == 7u) {
        uint32_t source =
            (uint32_t[]){0x7ff5d8u, 0x7ff5d6u, 0x7ff5d4u, 0x7ff5eeu,
                         0x7ff5dau, 0x7ffff0u, 0x7e09e6u, 0x7e1f6cu}[number % 8u];
        word(0xb8u, (uint16_t)source);
        put(0xbau, (uint8_t)(source >> 16));
        put(0x7ff450u, (uint8_t)(number % 255u + 1u));
    }
    if (alias_profile && suite == 8u) {
        n->data_bank = 0x7eu;
        n->accumulator = (uint16_t)((n->accumulator & 0xff00u) | 1u);
        n->y = 0x0101u;
        for (unsigned byte = 0x1b8cu; byte <= 0x1fa6u; ++byte)
            put(0x7e0000u | byte, 2u);
        put(0x7e1fa6u, 0u);
        put(0x7e0a13u, 0xffu);
        put(0x7e09fau, 0x90u);
    }
    if (budget_profile) {
        n->data_bank = 0x7fu;
        memset(nb.wram + 65536u, 0xffu, 65536u);
        n->stack = 0x1f80u;
        n->y = 0x0101u;
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
static void original_tail_step(Interp816 *cpu, SnesVerifyBus *bus, Trace *trace) {
    fetch_trace = trace;
    uint32_t pc = SnesVerifyPc24(cpu);
    uint8_t op = SnesVerifyBusRead(bus, pc);
    --trace->count;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)cpu->mf * 2u + (unsigned)cpu->xf][op];
    if (!fetch_left)
        exit(2);
    interp816_runOpcode(cpu);
    fetch_left = 0u;
    fetch_trace = &rt;
}
static void original_tail(const Lufia2CpuState *native, uint32_t pc) {
    nb.event_observer = NULL;
    Interp816 *tail = interp816_init(&nb, SnesVerifyBusRead, SnesVerifyBusWrite);
    interp816_reset(tail);
    Interp816 *reference = r;
    r = tail;
    to_interp(native, pc);
    r = reference;
    nb.event_observer = observer;
    nt.count = rt.count = 0u;
    for (unsigned steps = 0u; steps < 64u; ++steps) {
        original_tail_step(reference, &rb, &rt);
        original_tail_step(tail, &nb, &nt);
    }
    Lufia2CpuState continued;
    r = tail;
    from_interp(&continued);
    r = reference;
    compare(&continued, SnesVerifyPc24(reference), 1);
    interp816_free(tail);
}
static int valid_return(uint32_t pc) {
    if (entries[suite].end)
        return pc == entries[suite].end;
    if (suite == 0u)
        return pc == 0x81f158u || pc == 0x81f15au;
    if (suite == 1u)
        return pc == 0x81f184u || pc == 0x81f193u;
    if (suite == 11u)
        return pc == 0x85d096u || pc == 0x85d0b5u || pc == 0x85d0ccu;
    return 0;
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
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        if (suite != 8u || result.pc != 0x85d9fau)
            fail("unexpected continuation", result.pc);
        unsigned visits = 0u, steps = 0u;
        for (; steps < 2000000u; ++steps) {
            if (SnesVerifyPc24(r) == result.pc && ++visits == 4097u)
                break;
            instruction();
        }
        if (steps == 2000000u)
            fail("budget occurrence", result.pc);
        compare(&native, result.pc, 1);
        original_tail(&native, result.pc);
        ++boundary_states;
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
            if (budget_profile && suite != 8u)
                continue;
            if (alias_profile && suite != 7u && suite != 8u && suite != 10u)
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
