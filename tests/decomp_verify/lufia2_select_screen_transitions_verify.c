#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "core/cpu_ops.h"
#include "lufia2/menu.h"
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
    budget_profile;
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
    {0x868317u, 0x8686edu, 2u, 0u, 0u, ~0u}, {0x86831au, 0x868637u, 2u, 0u, 0u, ~0u},
    {0x86831fu, 0x86838cu, 2u, 0u, 0u, ~0u}, {0x868322u, 0x868996u, 2u, 0u, 0u, ~0u},
    {0x868325u, 0x868bcfu, 3u, 0u, 0u, ~0u}, {0x868329u, 0x8688beu, 2u, 0u, 0u, ~0u},
    {0x868330u, 0x868b48u, 2u, 0u, 0u, ~0u}, {0x86833au, 0x828b4bu, 3u, 2u, 0u, ~0u},
    {0x86834cu, 0x8686edu, 2u, 2u, 0u, ~0u}, {0x86834fu, 0x868b48u, 2u, 2u, 0u, ~0u},
    {0x868385u, 0x868655u, 2u, 3u, 0u, ~0u}, {0x868388u, 0x86866au, 2u, 3u, 0u, ~0u},
    {0x8682cau, 0x80953bu, 3u, 4u, 0u, ~0u}, {0x8682ceu, 0x828b4bu, 3u, 4u, 0u, ~0u},
    {0x8682e0u, 0x868637u, 2u, 4u, 0u, ~0u}, {0x8682e5u, 0x8686edu, 2u, 4u, 0u, ~0u},
    {0x8682e8u, 0x86838cu, 2u, 4u, 0u, ~0u}, {0x8682ebu, 0x868996u, 2u, 4u, 0u, ~0u},
    {0x8682eeu, 0x868bcfu, 3u, 4u, 0u, ~0u}, {0x8682f2u, 0x8688beu, 2u, 4u, 0u, ~0u},
    {0x8682f9u, 0x868b48u, 2u, 4u, 0u, ~0u}, {0x8682feu, 0x8689fdu, 2u, 4u, 0u, ~0u},
    {0x8688a2u, 0x868b48u, 2u, 5u, 0u, ~0u}, {0x868793u, 0x868740u, 2u, 5u, 0u, ~0u},
    {0x86879cu, 0x868740u, 2u, 5u, 0u, ~0u}, {0x8687a5u, 0x868740u, 2u, 5u, 0u, ~0u},
    {0x8687afu, 0x8687d3u, 2u, 5u, 0u, ~0u}, {0x8687b9u, 0x868865u, 2u, 5u, 0u, ~0u},
    {0x8687beu, 0x868865u, 2u, 5u, 0u, ~0u}, {0x8687c3u, 0x868865u, 2u, 5u, 0u, ~0u},
    {0x8687cfu, 0x868b48u, 2u, 5u, 0u, ~0u}, {0x8681d0u, 0x868784u, 2u, 5u, 0u, ~0u},
    {0x8681dbu, 0x8689fdu, 2u, 5u, 0u, ~0u}, {0x8681deu, 0x868305u, 2u, 5u, 0u, ~0u},
    {0x8681e7u, 0x8681fbu, 2u, 5u, 0u, ~0u}, {0x8681eeu, 0x868298u, 2u, 5u, 0u, ~0u},
    {0x8680e9u, 0x808e9du, 3u, 5u, 0u, ~0u}, {0x868100u, 0x828044u, 3u, 5u, 0u, ~0u},
    {0x868112u, 0x808e9du, 3u, 5u, 0u, ~0u}, {0x868129u, 0x828044u, 3u, 5u, 0u, ~0u},
    {0x86813bu, 0x808e9du, 3u, 5u, 0u, ~0u}, {0x868152u, 0x828044u, 3u, 5u, 0u, ~0u},
    {0x868188u, 0x808e9du, 3u, 5u, 0u, ~0u}, {0x868196u, 0x808e9du, 3u, 5u, 0u, ~0u},
    {0x8681a4u, 0x808e9du, 3u, 5u, 0u, ~0u}, {0x8680cfu, 0x868bcfu, 3u, 5u, 0u, ~0u},
    {0x8680d7u, 0x868b48u, 2u, 5u, 0u, ~0u}, {0x868009u, 0x868b48u, 2u, 5u, 0u, ~0u},
    {0x86800cu, 0x868dbbu, 3u, 5u, 0u, ~0u}, {0x86801fu, 0x868033u, 2u, 5u, 0u, ~0u},
    {0x868022u, 0x8680dbu, 2u, 5u, 0u, ~0u}, {0x868025u, 0x868157u, 2u, 5u, 0u, ~0u},
    {0x868028u, 0x86817au, 2u, 5u, 0u, ~0u}, {0x86802bu, 0x8681d0u, 2u, 5u, 0u, ~0u},
    {0x86802eu, 0x868dbbu, 3u, 5u, 0u, ~0u}};
enum { SITE_COUNT = 55u, PRIMARY_SITE_COUNT = 22u, ENTRY_COUNT = 6u };
static uint8_t child(void *, Lufia2CpuState *, uint32_t, uint32_t, uint8_t);
static const Entry entries[] = {
    {0x868305u, 0x868338u, Lufia2SelectScreenFadeIn, 2u},
    {0x86830fu, 0x868338u, Lufia2SelectScreenFadeOut, 2u},
    {0x868339u, 0u, Lufia2SelectScreenWaitForInput, 2u},
    {0x86835bu, 0x86838bu, Lufia2SelectScreenConfigureObject, 2u},
    {0x8682c8u, 0u, Lufia2SelectScreenWaitForObjects, 2u},
    {0x868000u, 0x868032u, Lufia2SelectScreenRun, 3u}};
static void fail(const char *why, uint32_t pc) {
    fprintf(stderr, "select transitions suite%u case%u %s site%06X ROM%06X\n", suite,
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
    SetAccumulatorWidth(n, 1u);
    SetIndexWidth(n, 0u);
    OpLoadA(n, (uint16_t)(number * 37u));
    unsigned visits = 0u;
    for (unsigned i = 0u; i < SITE_COUNT; ++i)
        if (sites[i].pc == site) {
            visits = sites[i].hits;
            break;
        }
    const unsigned delay = 1u + (number >> 2) % 4u;
    n->carry = number & 1u;
    n->overflow = (number >> 1) & 1u;
    if (site == 0x86831au)
        n->carry = forced ? 0u : number & 1u;
    if (site == 0x8682e0u)
        n->carry = forced ? (forced == 22u)
                          : (!budget_profile && !(number & 1u) && visits >= delay);
    if (site == 0x868330u)
        Write8(&memory, 0x7e0581u,
               (uint8_t)((!budget_profile && visits >= delay) ? 0u : 0x80u));
    if (site == 0x86833au || site == 0x8682ceu) {
        unsigned pressed =
            !budget_profile && !forced && (number & 1u) && visits >= delay;
        unsigned button = (number >> 4) % 3u;
        Write8(&memory, 0x7e14abu, (uint8_t)(pressed && button == 0u ? 0x80u : 0u));
        Write8(&memory, 0x7e14acu,
               (uint8_t)(pressed && button ? button == 1u ? 0x80u : 0x10u : 0u));
    }
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
    if (site == 0x828065u) {
        if (target != 0x8293c2u || frame != 2u)
            fail("nested wait contract", site);
        reach(site);
        instruction();
        compare(n, site, 0);
        ++wait_visits;
        controlled(&nb, n, site, frame);
        Lufia2CpuState reference;
        from_interp(&reference);
        controlled(&rb, &reference, site, frame);
        to_interp(&reference, site + 3u);
        compare(n, site, 1);
        return 1u;
    }
    unsigned which = 0;
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
    n->program_bank = (uint8_t)(target >> 16);
    if (composed &&
        (target == 0x868305u || target == 0x86830fu || target == 0x868740u ||
         target == 0x8687d3u || target == 0x8689fdu || target == 0x868865u ||
         target == 0x868784u || target == 0x8681d0u || target == 0x868033u ||
         target == 0x8680dbu || target == 0x868157u || target == 0x86817au ||
         target == 0x868dbbu || target == 0x868bcfu || target == 0x808e9du ||
         target == 0x828044u)) {
        Lufia2ExecutionResult result;
        switch (target) {
        case 0x868305u:
            result = Lufia2SelectScreenFadeIn(memory, n, child, context);
            break;
        case 0x86830fu:
            result = Lufia2SelectScreenFadeOut(memory, n, child, context);
            break;
        case 0x868740u:
            result = Lufia2TitleCopyTileRows(memory, n);
            break;
        case 0x8687d3u:
            result = Lufia2SelectScreenClearState(memory, n);
            break;
        case 0x8689fdu:
            result = Lufia2SelectScreenClearSpriteStatus(memory, n);
            break;
        case 0x868865u:
            result = Lufia2SelectScreenConfigureHdmaSlot(memory, n, child, context);
            break;
        case 0x868784u:
            result = Lufia2SelectScreenInitializeLayout(memory, n, child, context);
            break;
        case 0x8681d0u:
            result = Lufia2SelectScreenControl(memory, n, child, context);
            break;
        case 0x868033u:
            result = Lufia2SelectScreenConfigureDisplay(memory, n, child, context);
            break;
        case 0x8680dbu:
            result = Lufia2SelectScreenLoadGraphics(memory, n, child, context);
            break;
        case 0x868157u:
            result = Lufia2SelectScreenLoadPalette(memory, n);
            break;
        case 0x86817au:
            result = Lufia2SelectScreenLoadMaps(memory, n, child, context);
            break;
        case 0x868dbbu:
            result = Lufia2MenuResetDisplayRequests(memory, n);
            break;
        case 0x868bcfu:
            result = Lufia2SpriteClearOam(memory, n);
            break;
        case 0x808e9du:
            result = Lufia2DecompressResource(memory, n);
            break;
        case 0x828044u:
            result = Lufia2MenuQueueVideoWrite(memory, n, child, context);
            break;
        default:
            fail("unknown native child", target);
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

static void seed(Lufia2CpuState *n) {
    for (size_t byte = 0; byte < SNES_VERIFY_WRAM_SIZE; ++byte)
        nb.wram[byte] = (uint8_t)((byte * 23u + number * 47u) >> 3);
    memset(n, 0, sizeof *n);
    n->program_bank = 0x86u;
    n->data_bank = 0x7eu;
    n->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 10) & 3u];
    n->accumulator = (uint16_t)(number * 137u);
    n->x = (uint16_t)(number % 8u + 1u);
    n->y = (uint16_t)(number * 59u);
    n->accumulator_is_8_bit = (suite == 3u || suite == 5u) ? ((number >> 8) & 1u) : 1u;
    n->index_is_8_bit = suite == 5u ? ((number >> 9) & 1u) : 0u;
    if (n->index_is_8_bit) {
        n->x &= 255u;
        n->y &= 255u;
    }
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    nb.wram[0x30u] = (uint8_t[]){0u, 1u, 2u, 0x80u, 0xffu}[(number >> 1) % 5u];
    if (suite == 5u)
        nb.wram[0x30u] = 0u;
    if (budget_profile && suite == 2u)
        n->x = 0u;
    if (composed) {
        n->data_bank = SELECT_COMPOSED_BANK;
        n->stack = 0x1f80u;
        for (unsigned map = 0u; map < 3u; ++map) {
            unsigned source = 0x93c0u + map * 0x1000u;
            uint16_t width = (uint16_t)((number % 4u + 1u) * 8u),
                     height = (uint16_t)(((number >> 2) % 4u + 1u) * 8u);
            nb.wram[source] = (uint8_t)width;
            nb.wram[source + 1u] = 0u;
            nb.wram[source + 2u] = (uint8_t)height;
            nb.wram[source + 3u] = 0u;
            for (unsigned word = 0u; word < (unsigned)width * height; ++word) {
                uint16_t tile = (uint16_t)(number * 73u + word + map * 0x1000u);
                nb.wram[source + 4u + word * 2u] = (uint8_t)tile;
                nb.wram[source + 5u + word * 2u] = (uint8_t)(tile >> 8);
            }
        }
    }
    if (composed && suite != 5u)
        n->data_bank = 0x7eu;
    if (suite == 5u && n->stack < 0x1f20u)
        n->stack = 0x1f20u;
    nb.wram[(uint16_t)(n->stack + 1u)] = (uint8_t)number;
    nb.wram[(uint16_t)(n->stack + 2u)] = 0x90u;
    if (suite == 5u)
        nb.wram[(uint16_t)(n->stack + 3u)] = (uint8_t)(number * 73u);
    memcpy(rb.wram, nb.wram, SNES_VERIFY_WRAM_SIZE);
    SnesVerifyBusResetTrace(&rb);
    SnesVerifyBusResetTrace(&nb);
    SnesVerifyBusResetMmio(&rb);
    SnesVerifyBusResetMmio(&nb);
    nt.count = rt.count = 0;
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
            fail("child unwind flow", result.pc);
        compare(&native, result.pc, 1);
    } else {
        if (budget_profile) {
            uint32_t boundary = suite < 2u    ? 0x868317u
                                : suite == 2u ? 0x868339u
                                              : 0x8682ceu;
            if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != boundary)
                fail("budget boundary", result.pc);
        } else if (result.flow != LUFIA2_EXECUTION_RETURNED ||
                   (entries[suite].end ? result.pc != entries[suite].end
                    : suite == 2u ? result.pc != 0x868357u && result.pc != 0x86835au
                                  : result.pc != 0x868302u && result.pc != 0x868304u))
            fail("return boundary", result.pc);
        reach(result.pc);
        compare(&native, result.pc, 1);
        if (budget_profile)
            original_tail(&native, result.pc);
        if (force_return && !budget_profile) {
            if (entries[suite].frame == 2u)
                SimulateRtsFrame(&memory, &native);
            else {
                (void)Pull8(&memory, &native);
                (void)Pull8(&memory, &native);
                native.program_bank = Pull8(&memory, &native);
            }
            instruction();
            compare(&native, SnesVerifyPc24(r), 1);
        }
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
    unsigned edges = argc > 2 && strcmp(argv[2], "--edges") == 0;
    unsigned root = argc > 2 && strcmp(argv[2], "--root") == 0;
    force_return = (argc > 2 && strcmp(argv[2], "--returns") == 0) || root || edges;
    composed = (argc > 2 && strcmp(argv[2], "--composed") == 0) || root;
    unsigned unwinds = argc > 2 && strcmp(argv[2], "--unwinds") == 0;
    unsigned cases = root           ? 64u
                     : force_return ? 1024u
                     : composed     ? 64u
                     : unwinds      ? 16u
                     : argc > 2     ? (unsigned)strtoul(argv[2], NULL, 10)
                                    : 4096u;
    if ((root || force_return || composed || unwinds || budget_profile || edges) &&
        argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (budget_profile)
        cases = argc > 3 ? (unsigned)strtoul(argv[3], NULL, 10) : 4u;
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
        for (suite = root ? 5u : 0u; suite < (root ? 6u : 5u); ++suite) {
            if (budget_profile && suite == 3u)
                continue;
            for (unsigned value = 0u; value < cases; ++value) {
                run(value + (edges ? 2048u : 0u), 0u);
                ++total;
            }
            printf("PASS select control family%u entry%06X states%u cumulative%u\n",
                   suite, entries[suite].pc, cases, total);
            fflush(stdout);
        }
    printf(
        "PASS%u selection-transition states; composed%u, returns%u, unwinds%u, root%u\n",
        total, composed, force_return, unwinds, root);
    return 0;
}
