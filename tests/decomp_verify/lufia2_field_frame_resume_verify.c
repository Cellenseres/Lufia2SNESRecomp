#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/decomp.h"
#include "lufia2/field.h"
#include "system/wram.h"
#include "core/cpu_ops.h"

static SnesVerifyBus rb, nb;
static unsigned suite, number, fixture, input_index_width, real_nmi_events[2];
static uint32_t diagnostic_pc;

static uint8_t hardware_read(void *context, uint32_t address) {
    unsigned bank = address >> 16, offset = address & 65535u;
    if ((bank <= 0x3fu || (bank >= 0x80u && bank <= 0xbfu)) &&
        (offset == 0x4212u || (offset >= 0x4218u && offset <= 0x421bu))) {
        SnesVerifyBus *b = context;
        uint8_t sample = (offset == 0x421bu && (number & 2u)) ? 0x10u : 0u;
        if (b->event_observer)
            b->event_observer(b->event_observer_context, address, sample, false);
        return sample;
    }
    return SnesVerifyBusRead(context, address);
}
static void hardware_write(void *context, uint32_t address, uint8_t value) {
    SnesVerifyBusWrite(context, address, value);
}

static unsigned counts[1], hits[1][262144], child_returns;
static const uint32_t entries[] = {0x838007u};
static int same_cpu(const Lufia2CpuState *n, const Interp816 *r, int bank) {
    return n->accumulator == r->a && n->x == r->x && n->y == r->y &&
           n->stack == r->sp && n->direct_page == r->dp && n->data_bank == r->db &&
           (!bank || n->program_bank == r->k) && n->carry == r->c && n->zero == r->z &&
           n->negative == r->n && n->overflow == r->v && n->decimal == r->d &&
           n->irq_disable == r->i && n->accumulator_is_8_bit == r->mf &&
           n->index_is_8_bit == r->xf;
}
static void to_interp(Interp816 *r, const Lufia2CpuState *n, uint32_t pc) {
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
typedef struct Trace {
    SnesVerifyBusEvent events[262144];
    size_t count;
} Trace;
static Trace rt, nt;
static void *fetch_context = &rt;
static uint32_t fetch_address;
static unsigned fetch_left;
static const uint8_t lengths[4][256] = {
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
static void observer(void *context, uint32_t address, uint8_t value, bool write) {
    if (context == fetch_context && !write && fetch_left && address == fetch_address) {
        --fetch_left;
        ++fetch_address;
        return;
    }
    Trace *t = context;
    if (t->count == 262144u) {
        fprintf(stderr, "trace cap suite%u case%u last%06X pc%06X\n", suite, number,
                address, diagnostic_pc);
        exit(1);
    }
    t->events[t->count++] = (SnesVerifyBusEvent){address, value, write ? 1u : 0u};
}
static int same_writes(void) {
    if (rb.mmio_overflow || nb.mmio_overflow || rb.mmio_count != nb.mmio_count ||
        rt.count != nt.count) {
        fprintf(stderr, "mmio%zu/%zu trace%zu/%zu\n", nb.mmio_count, rb.mmio_count,
                nt.count, rt.count);
        for (size_t i = 0; i < (nt.count < rt.count ? nt.count : rt.count); ++i)
            if (nt.events[i].address != rt.events[i].address ||
                nt.events[i].value != rt.events[i].value ||
                nt.events[i].write != rt.events[i].write) {
                fprintf(stderr, "firsttrace%zu N%06X=%02X/%u R%06X=%02X/%u\n", i,
                        nt.events[i].address, nt.events[i].value, nt.events[i].write,
                        rt.events[i].address, rt.events[i].value, rt.events[i].write);
                break;
            }
        return 0;
    }

    for (size_t i = 0; i < rt.count; ++i)
        if (rt.events[i].address != nt.events[i].address ||
            rt.events[i].value != nt.events[i].value ||
            rt.events[i].write != nt.events[i].write) {
            fprintf(stderr, "trace%zu ROM%06X=%02X/%u native%06X=%02X/%u\n", i,
                    rt.events[i].address, rt.events[i].value, rt.events[i].write,
                    nt.events[i].address, nt.events[i].value, nt.events[i].write);
            return 0;
        }
    for (size_t i = 0; i < rb.mmio_count; ++i)
        if (rb.mmio[i].address != nb.mmio[i].address ||
            rb.mmio[i].value != nb.mmio[i].value ||
            rb.mmio[i].write != nb.mmio[i].write)
            return 0;
    return 1;
}

static void raw_word(unsigned offset, uint16_t value) {
    rb.wram[offset] = nb.wram[offset] = (uint8_t)value;
    rb.wram[offset + 1u] = nb.wram[offset + 1u] = (uint8_t)(value >> 8);
}

static void step_bus(Interp816 *r, SnesVerifyBus *b, Trace *t) {
    diagnostic_pc = SnesVerifyPc24(r);
    const uint8_t opcode = SnesVerifyBusRead(b, SnesVerifyPc24(r));
    --t->count;
    fetch_context = t;
    fetch_address = SnesVerifyPc24(r);
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][opcode];
    if (!fetch_left) {
        fprintf(stderr, "unknown instruction%06X\n", fetch_address);
        exit(1);
    }
    interp816_runOpcode(r);
    fetch_left = 0u;
}
static void step(Interp816 *r) { step_bus(r, &rb, &rt); }

static void from_interp(Lufia2CpuState *n, const Interp816 *c) {
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
static const uint32_t child_sites[] = {
    0x838004u, 0x838031u, 0x83804fu, 0x838069u, 0x838070u, 0x838073u, 0x838077u,
    0x83807au, 0x83807du, 0x838081u, 0x838084u, 0x838088u, 0x83808cu, 0x83808fu,
    0x838099u, 0x8380aau, 0x8380b0u, 0x8380bbu, 0x8380c6u};
static const uint32_t child_targets[] = {
    0x83900cu, 0x8384e1u, 0x83afeau, 0x8eb28fu, 0x83aeb5u, 0x809c72u, 0x8383a0u,
    0x838682u, 0x83bb93u, 0x83e03eu, 0x8ebd77u, 0x83a21au, 0x8381c6u, 0x838103u,
    0x80953bu, 0x8ebb2eu, 0x83867bu, 0x8380cdu, 0x8eb000u};
static const unsigned site_owner[] = {0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u,
                                      0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u,
                                      0x0u, 0x0u, 0x0u, 0x0u, 0x0u};

static unsigned visits[2][3], native_inputs, reference_inputs, returns, waits;
static unsigned site_visits[19u], first_case[19u];
static Lufia2CpuState inputs[4096];
static unsigned wait_visits[2];
static uint8_t controlled_child(const Lufia2Memory *m, Lufia2CpuState *n,
                                uint32_t target, uint32_t site, uint8_t frame) {
    SnesVerifyBus *b = m->context;
    unsigned side = b == &nb;
    Trace *t = side ? &nt : &rt;
    if (target == 0x83900cu) {
        Interp816 *c = interp816_init(b, hardware_read, hardware_write);
        to_interp(c, n, target);
        unsigned polls = 0u, steps;
        for (steps = 0u; steps < 20000u; ++steps) {
            uint32_t pc = SnesVerifyPc24(c);
            if (pc == site + 3u && c->sp == (uint16_t)(n->stack + 2u))
                break;
            if (pc == 0x83900eu && ++polls == 3u) {
                Lufia2CpuState saved = {0};
                from_interp(&saved, c);
                uint16_t counter =
                    (uint16_t)(b->wram[0x40u] | ((uint16_t)b->wram[0x41u] << 8));
                c->nmiWanted = true;
                interp816_runOpcode(c);
                if (SnesVerifyPc24(c) != 0x00862du ||
                    c->sp != (uint16_t)(saved.stack - 4u))
                    exit(1);
                unsigned ticks = 0u;
                while (SnesVerifyPc24(c) != pc && ticks++ < 20000u)
                    step_bus(c, b, t);
                if (ticks >= 20000u || !same_cpu(&saved, c, 1) ||
                    (uint16_t)(b->wram[0x40u] | ((uint16_t)b->wram[0x41u] << 8)) !=
                        (uint16_t)(counter + 1u))
                    exit(1);
                ++real_nmi_events[side];
            }
            step_bus(c, b, t);
        }
        if (steps == 20000u)
            exit(1);
        from_interp(n, c);
        interp816_free(c);
        ++wait_visits[side];
        if (wait_visits[side] == 4u) {
            n->resume_pc = 0x83acb7u;
            return 0u;
        }
        return 1u;
    }
    if (site == 0x83808fu)
        Write8(m, 0x17acu, (number & 32u) ? 8u : 0xffu);
    n->accumulator = (uint16_t)(number * 17u + (site & 255u));
    n->carry = (number >> 2) & 1u;
    n->zero = (number >> 3) & 1u;
    n->negative = (number >> 4) & 1u;
    n->overflow = (number >> 5) & 1u;
    n->irq_disable = (number >> 6) & 1u;
    if (site == 0x8380b0u)
        n->zero = (number & 512u) == 0u;
    if (site == 0x8380bbu)
        n->zero = (number & 1024u) != 0u;
    if (site == 0x838031u || site == 0x838069u)
        n->index_is_8_bit = 1u;
    if (site == 0x8380c6u)
        n->index_is_8_bit = 0u;
    if (n->index_is_8_bit) {
        n->x &= 255u;
        n->y &= 255u;
    }
    if (frame == 3u) {
        SimulateRtlFrame(m, n);
        n->program_bank = 0x83u;
    } else
        SimulateRtsFrame(m, n);
    return 1u;
}
static uint8_t caller_child(void *context, Lufia2CpuState *n, uint32_t target,
                            uint32_t site, uint8_t frame) {
    const Lufia2Memory *m = context;
    unsigned index;
    for (index = 0u; index < 19u; ++index)
        if (site_owner[index] == suite && child_sites[index] == site &&
            child_targets[index] == target)
            break;
    if (index == 19u || native_inputs == 4096u) {
        fprintf(stderr, "unexpected child %06X/%06X\n", site, target);
        exit(1);
    }
    n->program_bank = (uint8_t)(target >> 16);
    inputs[native_inputs++] = *n;
    if (!site_visits[index])
        first_case[index] = number;
    ++site_visits[index];
    ++returns;
    return controlled_child(m, n, target, site, frame);
}
static Lufia2ExecutionResult invoke(const Lufia2Memory *m, Lufia2CpuState *n) {
    return Lufia2FieldResumeFrame(m, n, caller_child, (void *)m);
}
static void seed(Lufia2CpuState *n) {
    memset(rb.wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(nb.wram, 0, SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof(*n));
    memset(wait_visits, 0, sizeof(wait_visits));
    n->program_bank = 0x83u;
    n->data_bank = (const uint8_t[]){0x83u, 0u, 0x7eu, 0x86u}[number & 3u];
    n->accumulator_is_8_bit = (number >> 10) & 1u;
    n->index_is_8_bit = (number >> 11) & 1u;
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 29u);
    n->y = (uint16_t)(number * 113u);
    n->stack = (uint16_t)(0x1f40u + (number & 31u));
    if (n->index_is_8_bit) {
        n->x &= 255u;
        n->y &= 255u;
    }
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    raw_word(0x40u, (uint16_t)(number * 257u));
    raw_word(0x5acu, (uint16_t)(number * 17u));
    raw_word(0x5aeu, (uint16_t)(number * 59u));
    rb.wram[0x57cu] = nb.wram[0x57cu] = (number & 1u) ? 1u : 0u;
    rb.wram[0x49u] = nb.wram[0x49u] = (number & 2u) ? 0x10u : 0u;
    rb.wram[0x622u] = nb.wram[0x622u] = (number & 4u) ? 0x80u : 0u;
    rb.wram[0x5b7u] = nb.wram[0x5b7u] = (number & 8u) ? 1u : 0u;
    rb.wram[0x5b5u] = nb.wram[0x5b5u] = (uint8_t)(number >> 4);
    rb.wram[0x5b3u] = nb.wram[0x5b3u] = (number & 16u) ? 8u : 0u;
    rb.wram[0x99bu] = nb.wram[0x99bu] = (number & 64u) ? 0x80u : 0u;
    rb.wram[0x1d0a1u] = nb.wram[0x1d0a1u] = (number & 128u) ? 1u : 0u;
    rb.wram[0x1d0feu] = nb.wram[0x1d0feu] = (number & 256u) ? 1u : 0u;
    for (unsigned off = 0x5a8u; off < 0x5aau; ++off)
        rb.wram[off] = nb.wram[off] = (uint8_t)(number * 37u + off);
    rb.wram[0x1261u] = nb.wram[0x1261u] = (uint8_t)(number * 17u);
    rb.wram[0x1262u] = nb.wram[0x1262u] = (uint8_t)(number * 23u);

    n->program_bank = n->data_bank = 0x83u;
    n->accumulator_is_8_bit = 1u;
    SnesVerifyBusResetMmio(&rb);
    SnesVerifyBusResetMmio(&nb);
    rt.count = nt.count = 0u;
    native_inputs = reference_inputs = 0u;
}
static int run_case(Interp816 *r, const Lufia2Memory *m) {
    Lufia2CpuState n;
    seed(&n);
    uint16_t stack = n.stack;
    (void)stack;
    to_interp(r, &n, entries[suite]);
    Lufia2ExecutionResult result = invoke(m, &n);
    if (result.flow != LUFIA2_EXECUTION_CHILD_UNWOUND &&
        result.flow != LUFIA2_EXECUTION_BOUNDARY) {
        fputs("unexpected parent boundary", stderr);
        return 1;
    }
    Lufia2Memory reference = {hardware_read, hardware_write, &rb, NULL, NULL};
    unsigned steps;
    for (steps = 0u; steps < 262144u; ++steps) {
        uint32_t pc = SnesVerifyPc24(r);
        uint8_t opcode = rb.rom[((pc >> 16) & 127u) * 32768u + (pc & 32767u)];
        if (pc == (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND ? n.resume_pc
                                                                 : result.pc) &&
            r->sp == n.stack && rt.count == nt.count)
            break;
        ++hits[suite][r->pc * 4u + (unsigned)r->mf * 2u + (unsigned)r->xf];
        step(r);
        if (opcode == 0x20u || opcode == 0xfcu || opcode == 0x22u) {
            unsigned slot;
            for (slot = 0u; slot < 19u; ++slot)
                if (child_sites[slot] == pc)
                    break;
            if (slot == 19u || (SnesVerifyPc24(r) != child_targets[slot])) {
                fprintf(stderr,
                        "child target mismatch case%u site%06X slot%u target%06X\n",
                        number, pc, slot, SnesVerifyPc24(r));
                return 1;
            }

            if (reference_inputs >= native_inputs) {
                fprintf(stderr, "missing native child input case%u site%06X\n", number,
                        pc);
                return 1;
            }
            if (!same_cpu(&inputs[reference_inputs++], r, 1)) {
                fprintf(
                    stderr,
                    "child input mismatch suite%u case%u site%06X A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X P%02X/%02X\n",
                    suite, number, pc, inputs[reference_inputs - 1u].accumulator, r->a,
                    inputs[reference_inputs - 1u].x, r->x,
                    inputs[reference_inputs - 1u].y, r->y,
                    inputs[reference_inputs - 1u].stack, r->sp,
                    inputs[reference_inputs - 1u].data_bank, r->db,
                    PackStatus(&inputs[reference_inputs - 1u]),
                    (r->n * 128u + r->v * 64u + r->mf * 32u + r->xf * 16u + r->d * 8u +
                     r->i * 4u + r->z * 2u + r->c));
                return 1;
            }
            Lufia2CpuState state = {0};
            from_interp(&state, r);
            unsigned frame = opcode == 0x22u ? 3u : 2u;
            uint8_t returned = controlled_child(&reference, &state, SnesVerifyPc24(r),
                                                pc, (uint8_t)frame);
            to_interp(r, &state, returned ? pc + frame + 1u : state.resume_pc);
        }
    }
    if (steps == 262144u || reference_inputs != native_inputs || !same_cpu(&n, r, 1) ||
        memcmp(rb.wram, nb.wram, SNES_VERIFY_WRAM_SIZE) || !same_writes() ||
        (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND ? n.resume_pc : result.pc) !=
            SnesVerifyPc24(r)) {
        fprintf(stderr,
                "A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X P%02X/%02X\n",
                n.accumulator, r->a, n.x, r->x, n.y, r->y, n.stack, r->sp, n.data_bank,
                r->db, PackStatus(&n),
                (r->n * 128u + r->v * 64u + r->mf * 32u + r->xf * 16u + r->d * 8u +
                 r->i * 4u + r->z * 2u + r->c));
        for (unsigned off = 0; off < SNES_VERIFY_WRAM_SIZE; ++off)
            if (rb.wram[off] != nb.wram[off]) {
                fprintf(stderr, "first WRAM%05X N%02X R%02X\n", off, nb.wram[off],
                        rb.wram[off]);
                break;
            }
        fprintf(stderr, "scene mismatch suite%u case%u pc%06X/%06X CPU%d WRAM%d\n",
                suite, number, SnesVerifyPc24(r), result.pc, same_cpu(&n, r, 1),
                memcmp(rb.wram, nb.wram, SNES_VERIFY_WRAM_SIZE));
        return 1;
    }

    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc != 0x83b18eu)
        return 1;
    ++counts[suite];
    return 0;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 4 && argc != 5) ||
        !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc >= 4) {
        uint32_t pc = (uint32_t)strtoul(argv[2], NULL, 16);
        rom[((pc >> 16) & 127u) * 32768u + (pc & 32767u)] =
            (uint8_t)strtoul(argv[3], NULL, 16);
    }
    if (!SnesVerifyBusInit(&rb, rom, size) || !SnesVerifyBusInit(&nb, rom, size))
        return 2;
    Interp816 *r = interp816_init(&rb, hardware_read, hardware_write);
    Lufia2Memory m = {hardware_read, hardware_write, &nb, NULL, NULL};
    rb.event_observer = observer;
    rb.event_observer_context = &rt;
    nb.event_observer = observer;
    nb.event_observer_context = &nt;

    fixture = 0u;
    input_index_width = 1u;
    for (suite = 0u; suite < 1u; ++suite)
        for (number = 0u; number < 4096u; ++number)
            if (run_case(r, &m))
                return 1;
    printf(
        "PASS world scene field loop4096 original-ROM parent states;controlled service returns%u;actual ROM wait events%u/%u\n",
        returns, real_nmi_events[0], real_nmi_events[1]);
    for (unsigned index = 0u; index < 1u; ++index) {
        printf("ROOT%u:%u states\n", index, counts[index]);
        for (unsigned i = 0u; i < 262144u; ++i)
            if (hits[index][i])
                printf("COVER%u %04X M%uX%u %u\n", index, i / 4u, (i / 2u) & 1u, i & 1u,
                       hits[index][i]);
    }
    for (unsigned index = 0u; index < 19u; ++index)
        printf("SITE%u:%u first%u/%u\n", index, site_visits[index], first_case[index],
               site_owner[index]);

    for (unsigned site = 0u; site < 19u; ++site)
        if (!site_visits[site]) {
            fprintf(stderr, "unreached field child%u\n", site);
            return 1;
        }
    interp816_free(r);
    SnesVerifyBusDestroy(&rb);
    SnesVerifyBusDestroy(&nb);
    free(rom);
    return 0;
}
