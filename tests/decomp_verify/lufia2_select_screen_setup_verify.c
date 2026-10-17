#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "core/cpu_ops.h"
#include "lufia2/menu.h"
#include "lufia2/system.h"
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
static unsigned number, suite, forced, composed, wait_visits, force_return;
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
    {0x8680e9u, 0x808e9du, 3u, 0u, 0u, ~0u}, {0x868100u, 0x828044u, 3u, 0u, 0u, ~0u},
    {0x868112u, 0x808e9du, 3u, 0u, 0u, ~0u}, {0x868129u, 0x828044u, 3u, 0u, 0u, ~0u},
    {0x86813bu, 0x808e9du, 3u, 0u, 0u, ~0u}, {0x868152u, 0x828044u, 3u, 0u, 0u, ~0u},
    {0x868188u, 0x808e9du, 3u, 2u, 0u, ~0u}, {0x868196u, 0x808e9du, 3u, 2u, 0u, ~0u},
    {0x8681a4u, 0x808e9du, 3u, 2u, 0u, ~0u}, {0x8680cfu, 0x868bcfu, 3u, 3u, 0u, ~0u},
    {0x8680d7u, 0x868b48u, 2u, 3u, 0u, ~0u}, {0x868009u, 0x868b48u, 2u, 4u, 0u, ~0u},
    {0x86800cu, 0x868dbbu, 3u, 4u, 0u, ~0u}, {0x86801fu, 0x868033u, 2u, 4u, 0u, ~0u},
    {0x868022u, 0x8680dbu, 2u, 4u, 0u, ~0u}, {0x868025u, 0x868157u, 2u, 4u, 0u, ~0u},
    {0x868028u, 0x86817au, 2u, 4u, 0u, ~0u}, {0x86802bu, 0x8681d0u, 2u, 4u, 0u, ~0u},
    {0x86802eu, 0x868dbbu, 3u, 4u, 0u, ~0u}};
enum { SITE_COUNT = 19u, ENTRY_COUNT = 5u };
static uint8_t child(void *, Lufia2CpuState *, uint32_t, uint32_t, uint8_t);
static Lufia2ExecutionResult Palette(const Lufia2Memory *m, Lufia2CpuState *n,
                                     Lufia2PushedChildCall c, void *x) {
    (void)c;
    (void)x;
    return Lufia2SelectScreenLoadPalette(m, n);
}
static const Entry entries[] = {
    {0x8680dbu, 0x868156u, Lufia2SelectScreenLoadGraphics, 2u},
    {0x868157u, 0x868179u, Palette, 2u},
    {0x86817au, 0x8681a8u, Lufia2SelectScreenLoadMaps, 2u},
    {0x868033u, 0x8680dau, Lufia2SelectScreenConfigureDisplay, 2u},
    {0x868000u, 0x868032u, Lufia2SelectScreenRun, 3u}};
static void fail(const char *why, uint32_t pc) {
    fprintf(stderr, "menu graphics setup suite%u case%u %s site%06X ROM%06X\n", suite,
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
    if (!write && trace == &rt && fetch_left && address == fetch_address) {
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
    n->carry = number & 1u;
    n->overflow = (number >> 1) & 1u;
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
    if (composed && target != 0x868b48u && target != 0x8681d0u) {
        Lufia2ExecutionResult result;
        if (target == 0x808e9du)
            result = Lufia2DecompressResource(memory, n);
        else if (target == 0x828044u)
            result = Lufia2MenuQueueVideoWrite(memory, n, child, context);
        else if (target == 0x868033u)
            result = Lufia2SelectScreenConfigureDisplay(memory, n, child, context);
        else if (target == 0x8680dbu)
            result = Lufia2SelectScreenLoadGraphics(memory, n, child, context);
        else if (target == 0x868157u)
            result = Lufia2SelectScreenLoadPalette(memory, n);
        else if (target == 0x86817au)
            result = Lufia2SelectScreenLoadMaps(memory, n, child, context);
        else if (target == 0x868dbbu)
            result = Lufia2MenuResetDisplayRequests(memory, n);
        else if (target == 0x868bcfu)
            result = Lufia2SpriteClearOam(memory, n);
        else
            fail("unknown native child", target);
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
    n->data_bank = (uint8_t)number;
    n->stack = (uint16_t[]){0x1f00u, 0x1f20u, 0x1f80u, 0x1ffcu}[(number >> 8) & 3u];
    if (suite == 4u) {
        if (n->stack < 0x1f20u)
            n->stack = 0x1f20u;
        n->index_is_8_bit = (number >> 9) & 1u;
        if (n->index_is_8_bit) {
            n->x &= 255u;
            n->y &= 255u;
        }
    }
    n->accumulator = (uint16_t)(number * 137u);
    n->x = (uint16_t)(number * 33u);
    n->y = (uint16_t)(number * 59u);
    n->accumulator_is_8_bit = (suite == 1u || suite == 4u) ? ((number >> 8) & 1u) : 1u;
    if (n->index_is_8_bit) {
        n->x &= 255u;
        n->y &= 255u;
    }
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    if (composed) {
        n->data_bank = SELECT_COMPOSED_BANK;
        n->stack = 0x1f80u;
    }
    nb.wram[(uint16_t)(n->stack + 1u)] = (uint8_t)number;
    nb.wram[(uint16_t)(n->stack + 2u)] = 0x90u;
    if (suite == 4u)
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
static void run(unsigned value, unsigned force) {
    number = value;
    forced = force;
    wait_visits = 0u;
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
        if (result.flow != LUFIA2_EXECUTION_RETURNED ||
            (entries[suite].end ? result.pc != entries[suite].end
                                : result.pc != 0x80c081u && result.pc != 0x80c092u))
            fail("return boundary", result.pc);
        reach(result.pc);
        compare(&native, result.pc, 1);
        if (force_return) {
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

    unsigned extension = argc > 2 && strncmp(argv[2], "--extension", 11) == 0;
    force_return = argc > 2 && (strcmp(argv[2], "--returns") == 0 ||
                                strcmp(argv[2], "--extension-returns") == 0);
    composed = argc > 2 && (strcmp(argv[2], "--composed") == 0 ||
                            strcmp(argv[2], "--extension-composed") == 0);
    unsigned unwinds = argc > 2 && (strcmp(argv[2], "--unwinds") == 0 ||
                                    strcmp(argv[2], "--extension-unwinds") == 0);
    unsigned cases = force_return ? 256u
                     : composed   ? 64u
                     : unwinds    ? 4u
                     : extension  ? 2048u
                     : argc > 2   ? (unsigned)strtoul(argv[2], NULL, 10)
                                  : 2048u;
    if ((force_return || composed || unwinds || extension) && argc > 3)
        cases = (unsigned)strtoul(argv[3], NULL, 10);
    if (!cases)
        return 2;
    unsigned total = 0u;
    if (unwinds) {
        for (unsigned which = extension ? 9u : 0u; which < SITE_COUNT; ++which) {
            suite = sites[which].root;
            for (unsigned value = 0u; value < cases; ++value) {
                run(value * 513u, which + 1u);
                ++total;
            }
        }
    } else
        for (suite = extension ? 3u : 0u; suite < ENTRY_COUNT; ++suite) {
            for (unsigned value = 0u; value < cases; ++value) {
                run(value, 0u);
                ++total;
            }
            printf("PASS select assets family%u entry%06X states%u cumulative%u\n",
                   suite, entries[suite].pc, cases, total);
            fflush(stdout);
        }
    printf(
        "PASS%u select-screen caller states; composed%u, returns%u, unwinds%u, nested wait visits%u\n",
        total, composed, force_return, unwinds, wait_visits);
    return 0;
}
