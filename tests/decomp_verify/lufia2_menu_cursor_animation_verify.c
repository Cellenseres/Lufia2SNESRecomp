#include "interp816.h"
#include "lufia2/menu.h"
#include "snes_function_verify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestBus {
    SnesVerifyBus bus;
    unsigned ticks, writes;
    uint64_t hash;
} TestBus;

static TestBus original, native;
static unsigned number, calls, forced, changed, animated, returns, boundaries, unwinds;
static uint8_t snapshot[SNES_VERIFY_WRAM_SIZE];
static Interp816 *reference;

static void fail(const char *why) {
    fprintf(stderr, "cursor case%u calls%u force%u: %s ROM%06X\n", number, calls,
            forced, why, SnesVerifyPc24(reference));
    exit(1);
}
static uint8_t read_bus(void *context, uint32_t address) {
    TestBus *t = context;
    if (address == 0x40u && ++t->ticks % 2u == 0u)
        ++t->bus.wram[0x40u];
    return SnesVerifyBusRead(&t->bus, address);
}
static void write_bus(void *context, uint32_t address, uint8_t value) {
    TestBus *t = context;
    SnesVerifyBusWrite(&t->bus, address, value);
    ++t->writes;
    t->hash ^= ((uint64_t)address << 8) | value;
    t->hash *= 1099511628211ull;
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
static void compare(const Lufia2CpuState *n) {
    Lufia2CpuState expected = *n;
    store(&expected, reference);
    if (memcmp(&expected, n, sizeof expected)) {
        fprintf(stderr,
                "A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X C%u/%u Z%u/%u N%u/%u "
                "V%u/%u M%u/%u\n",
                n->accumulator, reference->a, n->x, reference->x, n->y, reference->y,
                n->stack, reference->sp, n->carry, reference->c, n->zero, reference->z,
                n->negative, reference->n, n->overflow, reference->v,
                n->accumulator_is_8_bit, reference->mf);
        fail("CPU");
    }
    if (memcmp(native.bus.wram, original.bus.wram, SNES_VERIFY_WRAM_SIZE)) {
        for (unsigned i = 0; i < SNES_VERIFY_WRAM_SIZE; ++i)
            if (native.bus.wram[i] != original.bus.wram[i]) {
                fprintf(stderr, "WRAM%05X %02X/%02X\n", i, native.bus.wram[i],
                        original.bus.wram[i]);
                break;
            }
        fail("WRAM");
    }
    if (native.writes != original.writes || native.hash != original.hash)
        fail("ordered writes");
    if (native.bus.mmio_overflow || original.bus.mmio_overflow ||
        native.bus.mmio_count != original.bus.mmio_count)
        fail("MMIO count");
    for (size_t i = 0; i < native.bus.mmio_count; ++i)
        if (memcmp(&native.bus.mmio[i], &original.bus.mmio[i],
                   sizeof(SnesVerifyBusEvent)))
            fail("MMIO order");
}
static void reach(Interp816 *r, uint32_t pc, uint16_t stack) {
    for (unsigned steps = 0; steps < 300000u; ++steps) {
        if (SnesVerifyPc24(r) == pc && r->sp == stack)
            return;
        interp816_runOpcode(r);
    }
    fail("instruction ceiling");
}
static void alter(Interp816 *r) {
    if (changed == 1u)
        r->mf = 0;
    if (changed == 2u) {
        r->xf = 1;
        r->x &= 255u;
        r->y &= 255u;
    }
    if (changed == 3u)
        r->db = 0x7eu;
    if (changed == 4u)
        r->dp = 0x100u;
    if (changed == 5u)
        r->d = 1;
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    (void)context;
    if (target != 0x828afau || frame != 2u || (site != 0x828a98u && site != 0x828ad3u))
        fail("child site");
    ++calls;
    reach(reference, site, (uint16_t)(n->stack + 2u));
    interp816_runOpcode(reference);
    compare(n);
    if (calls == forced)
        return 0u;
    Interp816 *r = interp816_init(&native, read_bus, write_bus);
    load(r, n, target);
    reach(r, site + 3u, (uint16_t)(n->stack + 2u));
    store(n, r);
    interp816_free(r);
    reach(reference, site + 3u, n->stack);
    compare(n);
    if (changed == 6u) {
        unsigned position = site == 0x828a98u ? 0x13e8u + n->y : 0x1388u + n->y;
        unsigned destination = site == 0x828a98u ? 0xe0u : 0xdfu;
        uint8_t next = (uint8_t)(native.bus.wram[position] ^ 0x80u);
        write_bus(&native, destination, next);
        write_bus(&original, destination, next);
    }
    if (changed) {
        alter(reference);
        store(n, reference);
    }
    compare(n);
    return 1u;
}
static void reset(TestBus *t) {
    if (animated)
        memcpy(t->bus.wram, snapshot, sizeof snapshot);
    else {
        memset(t->bus.wram, (int)(number * 19u), SNES_VERIFY_WRAM_SIZE);
        memset(t->bus.wram + 0x11d8u, 0, 48u);
    }
    t->ticks = t->writes = 0;
    t->hash = 1469598103934665603ull;
    SnesVerifyBusResetTrace(&t->bus);
    SnesVerifyBusResetMmio(&t->bus);
    t->bus.wm_address = 0;
}
static void run(unsigned fx, unsigned fy, unsigned tx, unsigned ty, unsigned use_from) {
    reset(&native);
    reset(&original);
    calls = 0;
    Lufia2CpuState n = {0};
    n.accumulator = (uint16_t)(number * 256u + use_from);
    n.x = 9u;
    n.y = 7u;
    n.program_bank = 0x82u;
    n.data_bank = 0x83u;
    n.accumulator_is_8_bit = 1;
    n.stack = (uint16_t)(0x1f20u + number % 190u);
    n.irq_disable = 1;
    if (animated) {
        n.stack = 0x1ff0u;
        n.accumulator = (uint16_t)use_from;
    }
    n.carry = number & 1u;
    n.zero = (number >> 1) & 1u;
    n.negative = (number >> 2) & 1u;
    n.overflow = (number >> 3) & 1u;
    native.bus.wram[0x138fu] = (uint8_t)fx;
    native.bus.wram[0x13efu] = (uint8_t)fy;
    native.bus.wram[0x1391u] = (uint8_t)tx;
    native.bus.wram[0x13f1u] = (uint8_t)ty;
    memcpy(original.bus.wram, native.bus.wram, SNES_VERIFY_WRAM_SIZE);
    load(reference, &n, 0x8289fau);
    Lufia2Memory memory = {read_bus, write_bus, &native, NULL, NULL};
    Lufia2ExecutionResult result = Lufia2MenuCursorSlide(&memory, &n, child, NULL);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        if (result.pc != 0x828b07u)
            fail("return PC");
        reach(reference, result.pc, n.stack);
        ++returns;
    } else if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        if (!forced || calls != forced)
            fail("unwind contract");
        ++unwinds;
    } else {
        if (changed == 6u) {
            if (result.pc != 0x828a75u && result.pc != 0x828aa6u)
                fail("budget continuation");
            reach(reference, result.pc, n.stack);
        } else if (!changed || result.pc != SnesVerifyPc24(reference))
            fail("continuation contract");
        ++boundaries;
    }
    compare(&n);
}
static void build_sprite_preview(void) {
    static const unsigned slots[] = {0u, 7u, 9u, 11u};
    static const unsigned animations[] = {0u, 1u, 0u, 4u};
    static const unsigned frames[] = {0u, 4u, 3u, 0u};
    static const unsigned timers[] = {2u, 4u, 5u, 16u};
    memset(native.bus.wram, 0, SNES_VERIFY_WRAM_SIZE);
    for (unsigned i = 0; i < 4u; ++i) {
        Lufia2CpuState n = {0};
        n.program_bank = 0x86u;
        n.data_bank = 0x83u;
        n.accumulator_is_8_bit = 1u;
        n.stack = 0x1ff0u;
        n.x = (uint16_t)slots[i];
        n.y = (uint16_t)(i ? 10u : 0u);
        native.bus.wram[0x11d8u + slots[i]] = 1u;
        native.bus.wram[0x1448u + slots[i]] = (uint8_t)frames[i];
        Interp816 *r = interp816_init(&native, read_bus, write_bus);
        load(r, &n, 0x868cdau);
        reach(r, 0x868cf4u, n.stack);
        store(&n, r);
        n.accumulator = (uint16_t)animations[i];
        load(r, &n, 0x868cf5u);
        reach(r, 0x868d46u, n.stack);
        native.bus.wram[0x1478u + slots[i]] = (uint8_t)timers[i];
        interp816_free(r);
    }
    memcpy(snapshot, native.bus.wram, sizeof snapshot);
}

int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if ((argc != 2 && argc != 3 && argc != 4 && argc != 5) ||
        !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    if (argc == 5) {
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned offset = ((pc >> 16) & 127u) * 32768u + (pc & 32767u);
        uint8_t value = (uint8_t)strtoul(argv[4], NULL, 16);
        if (rom[offset] == value)
            return 2;
        rom[offset] = value;
    }
    if (argc >= 3) {
        FILE *f = fopen(argv[2], "rb");
        if (!f || fread(snapshot, 1, sizeof snapshot, f) != sizeof snapshot)
            return 2;
        fclose(f);
    }
    if (!SnesVerifyBusInit(&native.bus, rom, size) ||
        !SnesVerifyBusInit(&original.bus, rom, size))
        return 2;
    reference = interp816_init(&original, read_bus, write_bus);
    if (argc == 2)
        build_sprite_preview();
    if (argc != 4) {
        for (unsigned value = 0; value < 256u; ++value)
            for (unsigned direction = 0; direction < 8u; ++direction) {
                number = value * 8u + direction;
                unsigned a = direction & 1u ? 255u - value : 0u,
                         b = direction & 1u ? 255u : value;
                run(direction & 2u ? 33u : a, direction & 2u ? a : 55u,
                    direction & 2u ? 33u : b, direction & 2u ? b : 55u, direction >> 2);
            }
        for (unsigned i = 0; i < 2048u; ++i) {
            number = 2048u + i;
            run(i * 17u & 255u, i * 113u & 255u, i * 53u & 255u, i * 29u & 255u,
                i & 1u);
        }
        animated = 1;
        for (unsigned i = 0; i < 256u; ++i) {
            number = 4096u + i;
            run(88u, 118u, 24u + (i & 15u), 144u - (i >> 4), i & 1u);
        }
        forced = 1u;
        number = 4352u;
        run(88u, 118u, 24u, 144u, 0u);
        forced = 2u;
        ++number;
        run(88u, 118u, 24u, 144u, 0u);
        forced = 0;
        for (changed = 1u; changed <= 5u; ++changed) {
            ++number;
            run(88u, 118u, 24u, 144u, 0u);
        }
    }
    animated = 0;
    changed = 6u;
    ++number;
    run(88u, 118u, 24u, 144u, 0u);
    ++number;
    run(88u, 118u, 90u, 144u, 0u);
    printf("PASS cursor %u returns/%u continuations/%u unwinds; CPU/full "
           "WRAM/ordered writes/MMIO; real animation children\n",
           returns, boundaries, unwinds);
    interp816_free(reference);
    SnesVerifyBusDestroy(&native.bus);
    SnesVerifyBusDestroy(&original.bus);
    free(rom);
    return 0;
}
