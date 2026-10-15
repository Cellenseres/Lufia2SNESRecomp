#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"
#include "lufia2/party.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;

static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, exhaustive, boundaries;
static const uint16_t handlers[] = {0x9dfau, 0x9e83u, 0x9f10u, 0xa041u, 0xac6au,
                                    0xbbebu, 0xbdbbu, 0xbdcau, 0xbd99u};
static uint32_t ExecuteSceneControl(const Lufia2Memory *m, Lufia2CpuState *c,
                                    uint16_t op) {
    uint32_t flow = Lufia2TextExecuteScriptCommand(m, c, op);
    if (!flow)
        flow = Lufia2TextExecuteFieldCommand(m, c, op);
    return flow ? flow : Lufia2TextExecuteMenuCommand(m, c, op);
}
static unsigned native_checkpoints, original_checkpoints;
static Lufia2CpuState native_checkpoint_state, original_checkpoint_state;
static void observe_checkpoint(void *context, Lufia2CpuState *c, uint32_t pc) {
    (void)context;
    if (pc != 0x82803du) {
        fprintf(stderr, "unexpected checkpoint%06X\n", pc);
        exit(1);
    }
    ++native_checkpoints;
    native_checkpoint_state = *c;
}
#ifdef RUNTIME_ABI
void RuntimeObserveMenuCheckpoint(Lufia2CpuState *c, uint32_t pc) {
    observe_checkpoint(NULL, c, pc);
}
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
static void script_byte(uint8_t bank, uint16_t cursor, unsigned offset, uint8_t value) {
    unsigned sum = (unsigned)cursor + offset;
    if (sum > 0xffffu) {
        bank = (uint8_t)(bank + 1u);
        sum = 0x8000u + (sum & 65535u);
    }
    SnesVerifyBusPoke(&native, ((uint32_t)bank << 16) | sum, value);
}
static void word(uint32_t address, uint16_t value) {
    SnesVerifyBusPoke(&native, address, (uint8_t)value);
    SnesVerifyBusPoke(&native, address + 1u, (uint8_t)(value >> 8));
}
static void seed(Lufia2CpuState *n) {
    seed_actor(n);
    native_checkpoints = original_checkpoints = 0;
    static const uint8_t banks[] = {0, 0x7e, 0x7f, 0x80, 0x83, 0x81};
    n->data_bank = banks[(number / 16u) % 6u];
    static const uint16_t cursors[] = {0x9000, 0x8000, 0xffff, 0xfffe,
                                       0x7fff, 0x09b9, 0xfffd, 0x9000};
    n->y = cursors[number % 8u];
    uint8_t first = (uint8_t)(number * 37u), second = (uint8_t)(number * 73u);
    uint32_t db = (uint32_t)n->data_bank << 16, script = db | n->y;
    for (unsigned i = 0; i < 6u; ++i) {
        uint32_t bank = (uint32_t)banks[i] << 16;
        SnesVerifyBusPoke(&native, bank | 0x09b9u, n->data_bank);
        SnesVerifyBusPoke(&native, bank | 0x0583u,
                          (uint8_t)(number % 4u < 2u ? 0x80u : 0x0fu));
        SnesVerifyBusPoke(&native, bank | 0x099cu,
                          (uint8_t)(number % 4u == 0u ? 0u : 8u));
        SnesVerifyBusPoke(&native, bank | 0x126fu, (uint8_t)(number * 13u));
    }
    word(0x7ef006u, 0x0100u);
    native.wram[0xf100u] = first;
    native.wram[0xf105u] = (uint8_t[]){0, 2, 7, 1, 0xf0, 0xff, 0x12, 0x27}[number % 8u];
    native.wram[0xf106u] = (uint8_t)(number * 17u);
    native.wram[0xf107u] = (uint8_t)(number * 31u);
    native.wram[0xf108u] = (uint8_t)(number % 4u == 0u ? 0xff : number);
    word(0x7e05b9u, 8u);
    word(0x7e05bbu, 8u);
    word(0x7e120au, 0x0101u);
    word(0x7e120cu, 0x0202u);
#ifdef RUNTIME_ABI
    if (n->stack < 0x1f40u)
        n->stack = 0x1f40u;
    n->data_bank = number % 3u == 0u ? 0x7eu : number % 3u == 1u ? 0x83u : 0u;
    n->y = (uint16_t)(number * 59u);
    native.wram[0x09b7u] = 0;
    native.wram[0x09b8u] = 0x90;
    native.wram[0x09b9u] = 0x7e;
    native.wram[0x099bu] = native.wram[0x099cu] = native.wram[0x1259u] =
        native.wram[0x1d0ffu] = 0;
    native.wram[0x9000u] =
        (uint8_t[]){0x4u, 0xau, 0x10u, 0x12u, 0xadu, 0x5eu, 0x82u, 0x83u, 0x81u}[suite];
    script = 0x7e9001u;
    static const unsigned counts[] = {1, 2, 1, 2, 0, 1, 0, 1, 1};
    native.wram[0x9001u + counts[suite]] = 0x37u;
    native.wram[0x9002u + counts[suite]] = 0xffu;
    if (suite == 1u) {
        first = 2;
        second = 0;
        native.wram[0x8fffu] = 0x37u;
    }
    if (suite == 2u) {
        first = 2;
        native.wram[0x9006u] = 0x41u;
        native.wram[0x9007u] = 1u;
        native.wram[0x09a7u] = 2;
    }
    if (suite == 3u) {
        first = (uint8_t)(number % 256u);
        second = (uint8_t)(number % 8u);
        memset(native.wram + 0x9003u, 0, 512u);
        word(0x7e099eu, 0x9800u);
        word(0x7e09a0u, 0x7eu);
        native.wram[0x9800u] = 0x37u;
        native.wram[0x9801u] = 0xffu;
    }
#else
    script_byte(n->data_bank, n->y, 0, first);
    script_byte(n->data_bank, n->y, 1, second);
#endif
    if (suite < 4u || suite == 5u || suite == 7u || suite == 8u)
        SnesVerifyBusPoke(&native, script, first);
    if (suite == 1u || suite == 3u)
        SnesVerifyBusPoke(&native, script + 1u, second);
    if (suite == 5u || suite == 7u)
        native.wram[0xf100u] = SnesVerifyBusRead(&native, script);
    if (suite == 6u && number % 8u >= 4u) {
        word(0x0005aau, 0u);
        word(0x7fd008u, 0x2000u);
        word(0x7fd00au, 0x3000u);
        native.wram[0x1d010u] = native.wram[0x1d018u] = 8u;
        native.wram[0x06bau] = 1u;
        native.wram[0x06e2u] = native.wram[0x0692u] = 0u;
        memset(native.wram + 0x12000u, 0, 256u);
        memset(native.wram + 0x1c000u, 0, 16u);
    }
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    if (pc == 0x82803du) {
        ++original_checkpoints;
        original_checkpoint_state = native_checkpoint_state;
        store(&original_checkpoint_state, r);
    }
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
    if (suite == 6u) {
        unsigned left = 0u, right = 0u;
        while (left < original_trace.count || right < native_trace.count) {
            Trace *sides[] = {&original_trace, &native_trace};
            unsigned *indices[] = {&left, &right};
            for (unsigned side = 0u; side < 2u; ++side) {
                while (*indices[side] < sides[side]->count) {
                    const SnesVerifyBusEvent *event =
                        &sides[side]->events[*indices[side]];
                    unsigned bank = event->address >> 16;
                    if (event->write ||
                        !(bank == 0x7eu || bank == 0x7fu ||
                          (!(bank & 0x40u) && (event->address & 65535u) < 0x2000u)))
                        break;
                    ++*indices[side];
                }
            }
            if (left == original_trace.count || right == native_trace.count)
                return left == original_trace.count && right == native_trace.count;
            const SnesVerifyBusEvent *a = &original_trace.events[left++];
            const SnesVerifyBusEvent *b = &native_trace.events[right++];
            if (a->address != b->address || a->value != b->value || a->write != b->write)
                return 0;
        }
        return 1;
    }
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
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native,
                           observe_checkpoint, NULL};
    uint32_t entry = ((uint32_t)n.program_bank << 16) | handlers[suite];
#ifdef RUNTIME_ABI
    entry = 0x809cb8u;
    load(r, &n, entry);
    Lufia2ExecutionResult result =
        runtime_invoke(&memory, &n, NULL, NULL, Lufia2DecompBridge_9CB8,
                       Lufia2TextEngineStep, NULL, entry, 0u, 3u);
    uint32_t flow = result.pc;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        ++boundaries;
#else
    load(r, &n, entry);
    uint32_t flow = ExecuteSceneControl(&memory, &n, handlers[suite]);
#endif
    if (!flow)
        return 1;
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        if (SnesVerifyPc24(r) == flow)
            break;
        step(r);
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 1000000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_checkpoints != original_checkpoints ||
        (native_checkpoints &&
         memcmp(&native_checkpoint_state, &original_checkpoint_state, sizeof n))) {
        fprintf(stderr,
                "menu%04X case%u steps%u flow%06X originalPC%06X CPU%d WRAM%d "
                "traces%u/%u hooks%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X "
                "PB%02X/%02X DB%02X/%02X N%u/%u Z%u/%u C%u/%u\n",
                handlers[suite], number, steps, flow, SnesVerifyPc24(r),
                memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, native_checkpoints,
                original_checkpoints, n.accumulator, r->a, n.x, r->x, n.y, r->y,
                n.stack, r->sp, n.program_bank, r->k, n.data_bank, r->db, n.negative,
                r->n, n.zero, r->z, n.carry, r->c);
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
    for (suite = 0; suite < 9u; ++suite) {
        if (argc == 3 && strcmp(argv[2], "--guards") == 0)
            break;
        if (selected_handler && selected_handler != handlers[suite])
            continue;
        unsigned cases = 4096u;
#ifdef RUNTIME_ABI
        cases = 1024u;
#endif
        for (number = 0; number < cases; ++number) {
            if (run(r))
                return 1;
            ++total;
        }
        printf("PASS menu%04X:%u CPU/full-WRAM/bus/checkpoint ROM states\n",
               handlers[suite], cases);
        fflush(stdout);
    }
#ifndef RUNTIME_ABI
    if (!selected_handler) {
        Lufia2CpuState untouched;
        number = suite = 0;
        seed(&untouched);
        Lufia2Memory m = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
        unsigned guards = 0;
        for (unsigned h = 0; h < 65536u; ++h) {
            unsigned j = 0;
            for (; j < 4u; ++j)
                if (h == handlers[j])
                    break;
            if (j < 4u)
                continue;
            Lufia2CpuState n = untouched;
            if (Lufia2TextExecuteScriptCommand(&m, &n, (uint16_t)h) ||
                memcmp(&n, &untouched, sizeof n) || native_trace.count)
                return 1;
            ++guards;
        }
        for (suite = 0; suite < 9u; ++suite)
            for (unsigned k = 0; k < 14u; ++k) {
                Lufia2CpuState n = untouched;
                if (k == 0u)
                    n.accumulator_is_8_bit = 0;
                else if (k == 1u)
                    n.index_is_8_bit = 1;
                else if (k < 8u)
                    n.direct_page = (uint16_t[]){1u,      0xffu,   0x100u,
                                                 0xffffu, 0xffc0u, 0x1ecbu}[k - 2u];
                else if (k == 8u)
                    n.program_bank = 0;
                else if (k == 9u)
                    n.decimal = 1u;
                else
                    n.stack = (uint16_t[]){0, 0x54, 0x1f0f, 0x1ffd}[k - 10u];
                Lufia2CpuState before = n;
                if (ExecuteSceneControl(&m, &n, handlers[suite]) ||
                    memcmp(&before, &n, sizeof n) || native_trace.count)
                    return 1;
                ++guards;
            }
        printf("PASS menu unsupported/unknown guards:%u\n", guards);
    }
#else
    if (runtime_fallback[0] || runtime_native[0] != total)
        return 1;
#endif
    printf("PASS menu batch:%u states/%u boundaries\n", total, boundaries);
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
