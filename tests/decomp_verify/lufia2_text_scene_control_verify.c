#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;

static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, exhaustive;
static const uint16_t handlers[] = {
    0xaa43u, 0xaa49u, 0xaa83u, 0xaaa1u, 0xaabfu, 0xaae6u, 0xab1du, 0xad02u,
    0xad12u, 0xad2fu, 0xb021u, 0xb92cu, 0xb93du, 0xb341u, 0xb34fu, 0xb494u,
    0xb49bu, 0xb4adu, 0xa2b0u, 0xb721u, 0xae42u, 0xaef2u, 0xaf6bu};
static uint32_t ExecuteSceneControl(const Lufia2Memory *m, Lufia2CpuState *c,
                                    uint16_t op) {
    uint32_t flow = Lufia2TextExecuteObjectCommand(m, c, op);
    if (flow)
        return flow;
    flow = Lufia2TextExecuteCameraCommand(m, c, op);
    if (flow)
        return flow;
    return Lufia2TextExecuteColorCommand(m, c, op) ? LUFIA2_TEXT_COMMAND_NEXT : 0u;
}
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;

static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    Trace *trace = context;
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
static void seed_actor(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f10u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f20u + number % 200u);
    n->program_bank = 0x80u;
    n->data_bank = (uint8_t)(number >> 4);
    n->accumulator_is_8_bit = 1u;
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    static const uint16_t cursors[] = {0x8000u, 0x9000u, 0xfffcu, 0xfffdu,
                                       0xfffeu, 0xffffu, 0x7fffu, 0x09b9u};
    n->y = number % 16u < 8u ? cursors[number % 8u]
                             : (uint16_t)(0x8000u + (number * 23u) % 0x7ff0u);
    if (number % 3u)
        n->data_bank = number % 3u == 1u ? 0x7eu : 0x83u;
    uint32_t bank = (uint32_t)n->data_bank << 16;
    SnesVerifyBusPoke(&native, bank | n->y,
                      (uint8_t)(number % 8u == 7u ? 0xffu : 0xc0u + number % 31u));
    SnesVerifyBusPoke(&native, bank | (uint16_t)(n->y + 1u), (uint8_t)(number * 37u));
    SnesVerifyBusPoke(&native, bank | (uint16_t)(n->y + 2u), (uint8_t)(number * 73u));
    SnesVerifyBusPoke(&native, bank | 0x09b9u, n->data_bank);
    uint8_t wanted = SnesVerifyBusRead(&native, bank | n->y);
    for (unsigned slot = 0; slot < 40u; ++slot) {
        SnesVerifyBusPoke(&native, bank | 0x05fau + slot, (uint8_t)(0x10u + slot));
        SnesVerifyBusPoke(&native, bank | 0x0622u + slot,
                          (uint8_t)(number + slot * 67u));
        SnesVerifyBusPoke(&native, bank | 0x066au + slot, (uint8_t)(number + slot));
        SnesVerifyBusPoke(&native, bank | 0x070au + slot,
                          (uint8_t)(number * 17u + slot));
    }
    unsigned target = (number / 8u) % 40u;
    if (number % 8u == 1u)
        target = 0;
    if (number % 8u == 2u)
        target = 39u;
    if (number % 8u != 0u && number % 8u != 6u) {
        SnesVerifyBusPoke(&native, bank | 0x05fau + target, wanted);
        SnesVerifyBusPoke(&native, bank | 0x0622u + target, (uint8_t)(number & 0xfbu));
        if (number % 8u == 4u) {
            SnesVerifyBusPoke(&native, bank | 0x05fau, wanted);
            SnesVerifyBusPoke(&native, bank | 0x0622u, 4u);
        }
        if (number % 8u == 5u)
            for (unsigned slot = 0; slot < 40u; ++slot) {
                SnesVerifyBusPoke(&native, bank | 0x05fau + slot, wanted);
                SnesVerifyBusPoke(&native, bank | 0x0622u + slot, 0u);
            }
    }
    uint16_t stale = (uint16_t)(number % 4u ? number % 40u : number * 19u);
    native.wram[0xa7u] = (uint8_t)stale;
    native.wram[0xa8u] = (uint8_t)(stale >> 8);
    native.wram[0xa9u] = (uint8_t)(stale * 2u);
    native.wram[0xaau] = (uint8_t)(stale * 2u >> 8);
    native.wram[0xabu] = (uint8_t)(stale * 3u);
    native.wram[0xacu] = (uint8_t)(stale * 3u >> 8);
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static uint8_t operand_at(const Lufia2CpuState *n, unsigned operand,
                          uint8_t *after_bank) {
    uint8_t bank = n->data_bank, value = 0;
    uint16_t cursor = n->y;
    do {
        value = SnesVerifyBusRead(&native, ((uint32_t)bank << 16) | cursor);
        if (++cursor < 0x8000u) {
            bank =
                (uint8_t)(SnesVerifyBusRead(&native, ((uint32_t)bank << 16) | 0x09b9u) +
                          1u);
            cursor = 0x8000u;
        }
    } while (operand--);
    *after_bank = bank;
    return value;
}
static void seed(Lufia2CpuState *n) {
    seed_actor(n);
    uint16_t handler = handlers[suite];
    unsigned count =
        suite < 4u ? 3u
        : handler == 0xaae6u || handler == 0xad02u || handler == 0xad12u ||
                handler == 0xb021u || handler == 0xb341u || handler == 0xb4adu
            ? 2u
        : handler == 0xae42u ? 3u
        : handler == 0xaef2u ? 5u
        : handler == 0xaabfu || handler == 0xab1du || handler == 0xad2fu ||
                handler == 0xb92cu || handler == 0xb93du || handler == 0xb34fu
            ? 1u
            : 0u;
    if (n->y >= 0x8000u) {
        uint16_t cursor = n->y;
        uint8_t bank = n->data_bank;
        for (unsigned i = 0; i < count; ++i) {
            SnesVerifyBusPoke(&native, ((uint32_t)bank << 16) | cursor,
                              (uint8_t)(number * (17u + 46u * i)));
            if (++cursor < 0x8000u) {
                bank = (uint8_t)(SnesVerifyBusRead(&native,
                                                   ((uint32_t)bank << 16) | 0x09b9u) +
                                 1u);
                cursor = 0x8000u;
            }
        }
    }
    uint8_t bank, key = operand_at(n, 0u, &bank);
    uint32_t base = (uint32_t)bank << 16;
    for (unsigned slot = 0; slot < 40u; ++slot) {
        SnesVerifyBusPoke(&native, base | (0x05fau + slot), (uint8_t)(0x10u + slot));
        SnesVerifyBusPoke(&native, base | (0x0622u + slot),
                          (uint8_t)(number + 67u * slot));
        SnesVerifyBusPoke(&native, base | (0x06bau + slot), (uint8_t)(number + slot));
        SnesVerifyBusPoke(&native, base | (0x06e2u + slot),
                          (uint8_t)(number * 31u + slot));
        SnesVerifyBusPoke(&native, base | (0x0692u + slot),
                          (uint8_t)(number + slot * 3u));
    }
    unsigned target = (number / 8u) % 40u;
    if (number % 8u)
        SnesVerifyBusPoke(&native, base | (0x05fau + target), key);
    if (handler == 0xad02u || handler == 0xad12u) {
        uint8_t other = operand_at(n, 1u, &bank);
        base = (uint32_t)bank << 16;
        if (number % 8u != 2u)
            SnesVerifyBusPoke(&native, base | (0x05fau + (target + 17u) % 40u), other);
    }
    for (unsigned slot = 0; slot < 32u; ++slot) {
        SnesVerifyBusPoke(&native, base | (0x1559u + slot), (uint8_t)(0x40u + slot));
        SnesVerifyBusPoke(&native, base | (0x15d9u + slot), (uint8_t)(number + slot));
        SnesVerifyBusPoke(&native, base | (0x1639u + slot),
                          (uint8_t)(number / 2u + slot));
        SnesVerifyBusPoke(&native, base | (0x1699u + slot),
                          (uint8_t)(number / 3u + slot));
    }
    if (number % 8u)
        SnesVerifyBusPoke(&native, base | (0x1559u + number % 32u), key);
    if (number % 8u == 4u)
        for (unsigned slot = 0; slot < 32u; ++slot)
            SnesVerifyBusPoke(&native, base | (0x1559u + slot), key);
    for (unsigned slot = 0; slot < 8u; ++slot) {
        SnesVerifyBusPoke(&native, WRAM_FIELD_ANIMATION_SLOT_STATE + slot,
                          (uint8_t)(number % 4u ? 0x90u : 0u));
    }
    native.wram[0xa7u] = (uint8_t)target;
    native.wram[0xa8u] = 0;
    native.wram[0xa9u] = (uint8_t)(target * 2u);
    native.wram[0xaau] = 0;
    native.wram[0xabu] = (uint8_t)(target * 3u);
    native.wram[0xacu] = native.wram[0xadu] = 0;
    native.wram[0xf00eu] = 0x80u;
    native.wram[0xf00fu] = 0;
    for (unsigned i = 0; i < 24u; ++i) {
        unsigned at = 0xf080u + i * 3u;
        native.wram[at] = (uint8_t)(0x50u + i);
        native.wram[at + 1u] = (uint8_t)(number + i);
        native.wram[at + 2u] = (uint8_t)(number * 17u + i);
    }
    if (number % 8u)
        native.wram[0xf080u + number % 24u * 3u] = key;
    native.wram[0xf0c8u] = 0xffu;
    if (handler == 0xa2b0u || handler == 0xb721u) {
        for (unsigned i = 0; i < 256u; ++i) {
            SnesVerifyBusPoke(&native, WRAM_FIELD_LAYER_WIDTH + i, 1u);
            SnesVerifyBusPoke(&native, WRAM_FIELD_LAYER_HEIGHT + i, 1u);
        }
        native.wram[0xf026u] = 0x80u;
        native.wram[0xf027u] = 1u;
        native.wram[0xf180u] = 0xffu;
        native.wram[0xf01eu] = 0x90u;
        native.wram[0xf01fu] = 1u;
        native.wram[0xf190u] = 0xffu;
        for (unsigned i = 0; i < 48u; ++i)
            SnesVerifyBusPoke(&native, WRAM_FIELD_PENDING_RECORD_X + i, 0xffu);
    }
    if (exhaustive == 3u) {
        memset(native.wram, 0, SNES_VERIFY_WRAM_SIZE);
        n->data_bank = 0;
        n->y = 0x8000u;
        n->stack = 0x1f20u;
        native.wram[0xf00eu] = 0x80u;
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
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
    load(r, &n, 0x800000u | handlers[suite]);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    uint32_t flow = ExecuteSceneControl(&memory, &n, handlers[suite]);
    if (flow == LUFIA2_TEXT_COMMAND_UNHANDLED)
        return 1;
    unsigned steps;
    unsigned visits = 0;
    for (steps = 0; steps < 2000000u; ++steps) {
        if (SnesVerifyPc24(r) == flow && (flow != 0x80bfbcu || ++visits == 65537u))
            break;
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 2000000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace()) {
        fprintf(stderr,
                "text scene control handler%04X case%u exhaustive%u CPU%d WRAM%d "
                "traces%u/%u "
                "A%04X/%04X X%04X/%04X Y%04X/%04X C%u/%u V%u/%u D%u/%u\n",
                handlers[suite], number, exhaustive, memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, n.accumulator, r->a, n.x,
                r->x, n.y, r->y, n.carry, r->c, n.overflow, r->v, n.decimal, r->d);
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
        for (suite = 0; suite < 23u; ++suite) {
            if (selected_handler && handlers[suite] != selected_handler)
                continue;
            for (number = 0; number < 4096u; ++number) {
                if (run(r))
                    return 1;
                ++total;
            }
            printf("PASS scene control%04X:4096 original-ROM states\n",
                   handlers[suite]);
            fflush(stdout);
        }
    if (argc == 3 && strcmp(argv[2], "--budget") == 0) {
        exhaustive = 3u;
        number = 0;
        for (suite = 0; suite < 23u; ++suite) {
            if (handlers[suite] != 0xb021u && handlers[suite] != 0xb34fu)
                continue;
            if (run(r))
                return 1;
        }
        puts("PASS scene search budgets:2 exact65536-visit ROM continuations");
    }
    if (selected_handler) {
        puts("PASS selected scene-control group");
        return 0;
    }
    exhaustive = 0;
    number = 0;
    suite = 0;
    Lufia2CpuState untouched;
    seed(&untouched);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    unsigned guards = 0;
    for (unsigned handler = 0; handler < 65536u; ++handler) {
        unsigned i = 0;
        for (; i < 23u; ++i)
            if (handler == handlers[i])
                break;
        if (i < 23u)
            continue;
        Lufia2CpuState state = untouched;
        if (ExecuteSceneControl(&memory, &state, (uint16_t)handler) ||
            memcmp(&state, &untouched, sizeof state) || native_trace.count)
            return 1;
        ++guards;
    }
    for (unsigned command = 0; command < 23u; ++command) {
        for (unsigned context = 0; context < 14u; ++context) {
            Lufia2CpuState state = untouched;
            if (context == 0u)
                state.accumulator_is_8_bit = 0;
            else if (context == 1u)
                state.index_is_8_bit = 1;
            else if (context < 8u) {
                static const uint16_t pages[] = {1u,      0xffu,   0x100u,
                                                 0xffffu, 0xffc0u, 0x1ecbu};
                state.direct_page = pages[context - 2u];
            } else if (context == 8u)
                state.program_bank = 0;
            else if (context == 9u)
                state.decimal = 1u;
            else {
                static const uint16_t stacks[] = {0u, 0x54u, 0x1f0fu, 0x1ffdu};
                state.stack = stacks[context - 10u];
            }
            Lufia2CpuState before = state;
            if (ExecuteSceneControl(&memory, &state, handlers[command]) ||
                memcmp(&state, &before, sizeof state) || native_trace.count)
                return 1;
            ++guards;
        }
    }
    printf("PASS unknown/unsupported command guards:%u, CPU/memory untouched\n",
           guards);
    printf("PASS text scene control commands:%u CPU/full-WRAM/ordered bus states\n",
           total);
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
