#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "lufia2/text.h"
#include "lufia2/field.h"
#include "core/cpu_ops.h"
#include "system/wram.h"
#include "text/text_internal.h"
typedef struct Trace {
    SnesVerifyBusEvent events[1200000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;
static unsigned suite, number, unwind_at, bad_at, bad_kind;
static uint8_t lengths[4][256];
static Trace *fetch_trace;
static unsigned fetch_left;
static uint32_t fetch_address;
static unsigned native_calls, original_calls;
static Lufia2CpuState native_entries[4], original_entries[4];
static const uint16_t handlers[] = {0xa2b0, 0xb721, 0xb341, 0xb34f, 0xb494, 0xaf6b,
                                    0xafc0, 0xb62b, 0xb6f4, 0xb72d, 0xb646, 0xb659};
static const unsigned counts[] = {1, 1, 2, 2, 1, 1, 1, 2, 3, 4, 1, 3};
static const uint32_t sites[12][4] = {{0x80a2b3},
                                      {0x80b725},
                                      {0x80b365, 0x80b36c},
                                      {0x80b365, 0x80b36c},
                                      {0x80b494},
                                      {0x80af6b},
                                      {0x80afdf},
                                      {0x80b630, 0x80b639},
                                      {0x80b703, 0x80b70c, 0x80b710},
                                      {0x80b732, 0x80b74c, 0x80b753, 0x80b761},
                                      {0x80b648},
                                      {0x80b65c, 0x80b663, 0x80b66e}};
static const uint32_t targets[12][4] = {{0x80ed9c},
                                        {0x80ed9c},
                                        {0x838d42, 0x838e4c},
                                        {0x838d42, 0x838e4c},
                                        {0x838e4c},
                                        {0x80f2f3},
                                        {0x80d533},
                                        {0x838b6a, 0x838e76},
                                        {0x838b6a, 0x838e76, 0x80cc26},
                                        {0x838b6a, 0x80f47a, 0x80f47a, 0x80ed9c},
                                        {0x838c8a},
                                        {0x80f47a, 0x80f47a, 0x808285}};
static unsigned selected[4], reached[12][4], is_engine;
static unsigned site_index(uint32_t site) {
    for (unsigned i = 0; i < counts[suite]; ++i)
        if (sites[suite][i] == site)
            return i;
    fprintf(stderr, "unexpected site%06X suite%u\n", site, suite);
    exit(1);
}
Lufia2ExecutionResult ProofSceneCommand(const Lufia2Memory *memory, Lufia2CpuState *cpu,
                                        uint16_t handler,
                                        Lufia2PushedChildCall callback, void *context) {
    Lufia2ExecutionResult result =
        Lufia2TextRunCameraCommand(memory, cpu, handler, callback, context);
    if (result.flow == LUFIA2_EXECUTION_RETURNED && !result.pc)
        result = Lufia2TextRunRenderCommand(memory, cpu, handler, callback, context);
    return result;
}
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
    if (trace->count == 1200000u) {
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

static uint32_t address(const Lufia2CpuState *cpu, uint16_t at) {
    return ((uint32_t)cpu->data_bank << 16) | at;
}
static unsigned use_real_children;
static unsigned real_child(uint32_t target) {
    return target == 0x838d42u || target == 0x80f2f3u || target == 0x80cc26u;
}
static void run_real_child(const Lufia2Memory *memory, Lufia2CpuState *cpu,
                           uint32_t target) {
    Lufia2ExecutionResult result;
    if (target == 0x838d42u)
        result = Lufia2FieldSetupLayerScroll(memory, cpu);
    else if (target == 0x80f2f3u)
        result = Lufia2FieldSetSceneDisplay(memory, cpu);
    else
        result = Lufia2FieldRequestTilemapUploads(memory, cpu);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        abort();
    SimulateRtlFrame(memory, cpu);
    cpu->program_bank = 0x80u;
}
static void controlled_return(const Lufia2Memory *memory, Lufia2CpuState *cpu,
                              unsigned call) {
    unsigned index = selected[call - 1u];
    OpSta(memory, cpu, 0x7ff110u + index * 2u);
    OpLoadA(cpu, (uint16_t)(number * 17u + index));
    OpLdx(cpu, (uint16_t)(number * 31u + index));
    cpu->carry = (number + index) & 1u;
    cpu->overflow = (number >> 2) & 1u;
    uint16_t back = Pull8(memory, cpu);
    back |= (uint16_t)Pull8(memory, cpu) << 8;
    if (back != (uint16_t)(sites[suite][index] + 3u))
        abort();
    cpu->program_bank = Pull8(memory, cpu);
    if (bad_at && call == bad_at) {
        if (bad_kind == 0)
            cpu->accumulator_is_8_bit = 0;
        if (bad_kind == 1)
            SetIndexWidth(cpu, 1u);
        if (bad_kind == 2)
            cpu->direct_page = 0x100u;
        if (bad_kind == 3)
            cpu->decimal = 1;
        if (bad_kind == 4)
            cpu->data_bank ^= 1u;
        if (bad_kind == 5)
            cpu->program_bank = 0x86u;
        if (bad_kind == 6)
            ++cpu->stack;
    }
}
static uint8_t child(void *context, Lufia2CpuState *cpu, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned index = site_index(site), call = native_calls++;
    if (call >= 4u || target != targets[suite][index] || frame != 3u)
        abort();
    selected[call] = index;
    ++reached[suite][index];
    cpu->program_bank = (uint8_t)(target >> 16);
    native_entries[call] = *cpu;
    if (native_calls == unwind_at)
        return 0u;
    if (use_real_children && real_child(target))
        run_real_child(context, cpu, target);
    else
        controlled_return(context, cpu, native_calls);
    return 1u;
}
static void seed_byte(uint32_t at, uint8_t value) {
    if (SnesVerifyBusPoke(&native, at, value))
        return;
    if ((at & 0xffffu) < 0x8000u)
        abort();
    unsigned offset = ((at >> 16) & 0x7fu) * 0x8000u + (at & 0x7fffu);
    if (offset >= native.rom_size)
        abort();
    ((uint8_t *)native.rom)[offset] = value;
}
static void seed(Lufia2CpuState *cpu) {
    memset(cpu, 0, sizeof *cpu);
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    native_calls = original_calls = 0;

    cpu->program_bank = 0x80u;
    cpu->data_bank = (uint8_t[]){0x7e, 0x7f, 0x80, 0, 0x83}[(number >> 6) % 5u];
    cpu->accumulator = (uint16_t)(number * 197u);
    cpu->x = (uint16_t)(number * 83u);
    cpu->y = cpu->data_bank == 0x7eu || cpu->data_bank == 0x7fu ? 0x9000u : 0xe000u;
    cpu->stack = (uint16_t)(0x1f80u + number % 64u);
    UnpackStatus(cpu, (uint8_t)(number * 73u & 0xc7u));
    cpu->accumulator_is_8_bit = 1u;
    cpu->index_is_8_bit = cpu->decimal = 0;
    for (unsigned bank = 0; bank < 2u; ++bank) {
        uint32_t a = 0x7e0000u + bank * 65536u;
        SnesVerifyBusPoke(&native, a + WRAM_BRIGHTNESS,
                          (uint8_t)(number & 1u ? 0x80u : 0u));
        SnesVerifyBusPoke(&native, a + WRAM_FADE_CONTROL,
                          (uint8_t)((number >> 1) & 1u));
        SnesVerifyBusPoke(&native, a + TEXT_WINDOW_STATE,
                          (uint8_t)(number & 4u ? 0x40u : 0u));
        word(a + WRAM_FIELD_LAYER_TABLE_OFFSET, (uint16_t)(2u * ((number >> 3) & 3u)));
        for (unsigned layer = 0; layer < 8u; layer += 2u) {
            word(a + WRAM_FIELD_LAYER_SCROLL_X + layer,
                 (uint16_t)(number * 41u + layer));
            word(a + WRAM_FIELD_LAYER_SCROLL_Y + layer,
                 (uint16_t)(number * 67u + layer));
        }
    }
    word(0x7ef00eu, 0x0100u);
    word(0x7ef014u, 0x0200u);
    for (unsigned i = 0; i < 3u; ++i) {
        unsigned a = 0x7ef100u + i * 3u;
        SnesVerifyBusPoke(&native, a, (uint8_t)i);
        SnesVerifyBusPoke(&native, a + 1u, (uint8_t)(number * 11u + i));
        SnesVerifyBusPoke(&native, a + 2u, (uint8_t)(number * 19u + i));
        a = 0x7ef200u + i * 5u;
        SnesVerifyBusPoke(&native, a, (uint8_t)i);
        word(a + 1u, (uint16_t)(number * 7u + i));
        word(a + 3u, (uint16_t)(number * 13u + i));
    }
    SnesVerifyBusPoke(&native, 0x7ef109u, 0xffu);
    SnesVerifyBusPoke(&native, 0x7ef20fu, 0xffu);
    for (unsigned i = 0; i < 8u; ++i)
        seed_byte(address(cpu, (uint16_t)(cpu->y + i)),
                  (uint8_t)(i < 2u ? (number + i) % 4u : number * 37u));
    seed_byte(address(cpu, TEXT_SCRIPT_BANK), cpu->data_bank);

    cpu->data_bank = number & 1u ? 0x7eu : 0x80u;
    unsigned opcode = 0;
    for (; opcode < 205u; ++opcode) {
        unsigned at = 0x4a14u + 2u * opcode;
        unsigned target = native.rom[at] | ((unsigned)native.rom[at + 1u] << 8);
        if (target == handlers[suite])
            break;
    }
    if (opcode == 205u)
        abort();
    unsigned operands = (unsigned[]){0, 0, 2, 1, 0, 0, 3, 2, 3, 2, 0, 0}[suite];
    unsigned script =
        cpu->data_bank == 0x7eu || cpu->data_bank == 0x7fu ? 0x9000u : 0xe000u;
    seed_byte(address(cpu, (uint16_t)script), (uint8_t)opcode);
    for (unsigned i = 0; i < operands; ++i)
        seed_byte(address(cpu, (uint16_t)(script + 1u + i)),
                  (uint8_t)(i < 2u ? (number + i) % 4u : number * 37u));
    seed_byte(address(cpu, (uint16_t)(script + 1u + operands)), 0x37u);
    seed_byte(address(cpu, (uint16_t)(script + 2u + operands)), 0xffu);
    native.wram[0x99bu] = native.wram[0x1259u] = native.wram[0x1d0ffu] = 0;
    native.wram[0xb54u] = 30u;
    native.wram[0x5b5u] = 0;
    word(0x9b7u, (uint16_t)script);
    native.wram[0x9b9u] = cpu->data_bank;
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    native_trace.count = original_trace.count = 0;
}

static int run(Interp816 *reference) {
    Lufia2CpuState cpu;
    seed(&cpu);
    load(reference, &cpu, 0x809cb8u);
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &native, NULL, NULL};
    Lufia2ExecutionResult result =
        Lufia2TextEngineStepWithServices(&memory, &cpu, child, &memory);
    uint32_t stop = result.pc;
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[suite][selected[unwind_at - 1u]];
    unsigned steps, visits = 0;
    for (steps = 0; steps < 200000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(reference);
        unsigned index = 0;
        if (pc == (0x800000u | handlers[suite]))
            ++visits;
        for (; index < counts[suite]; ++index)
            if (pc == targets[suite][index])
                break;
        if (index < counts[suite]) {
            uint16_t back = original.wram[(uint16_t)(reference->sp + 1u)];
            back |= (uint16_t)original.wram[(uint16_t)(reference->sp + 2u)] << 8;
            index = site_index(0x800000u | (uint16_t)(back - 3u));
            unsigned call = original_calls++;
            if (call >= 4u)
                abort();
            selected[call] = index;
            store(&original_entries[call], reference);
            if (original_calls == unwind_at)
                break;
            if (use_real_children && real_child(targets[suite][index])) {
                instruction(&original, reference);
                continue;
            }
            Lufia2CpuState state = {0};
            store(&state, reference);
            Lufia2Memory bus = {SnesVerifyBusRead, SnesVerifyBusWrite, &original, NULL,
                                NULL};
            controlled_return(&bus, &state, original_calls);
            load(reference, &state,
                 ((uint32_t)state.program_bank << 16) |
                     (uint16_t)(sites[suite][index] + 4u));
            continue;
        }
        if (pc == stop && visits)
            break;
        instruction(&original, reference);
    }
    Lufia2CpuState expected = cpu;
    store(&expected, reference);
    if (steps == 200000u || !visits || memcmp(&expected, &cpu, sizeof cpu) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries)) {
        fprintf(
            stderr,
            "scene%04X case%u unwind%u bad%u/%u steps%u stop%06X PC%06X CPU%d WRAM%d traces%u/%u calls%u/%u A%04X/%04X X%04X/%04X Y%04X/%04X S%04X/%04X DB%02X/%02X M%u/%u X%u/%u\n",
            handlers[suite], number, unwind_at, bad_at, bad_kind, steps, stop,
            SnesVerifyPc24(reference), memcmp(&expected, &cpu, sizeof cpu),
            memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
            original_trace.count, native_trace.count, native_calls, original_calls,
            cpu.accumulator, reference->a, cpu.x, reference->x, cpu.y, reference->y,
            cpu.stack, reference->sp, cpu.data_bank, reference->db,
            cpu.accumulator_is_8_bit, reference->mf, cpu.index_is_8_bit, reference->xf);
        return 1;
    }
    return 0;
}
int g_interp_apu_driving;
static int VerifySceneGuards(void);
int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[2], "--guards") == 0)
        return VerifySceneGuards();
    use_real_children = argc == 3 && strcmp(argv[2], "--real") == 0;
    (void)is_engine;
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u ||
        !SnesVerifyBusInit(&original, rom, size) ||
        !SnesVerifyBusInit(&native, rom, size))
        return 2;
    if (argc > 4) {
        unsigned pc = (unsigned)strtoul(argv[3], NULL, 16);
        unsigned at = ((pc >> 16) & 0x7fu) * 0x8000u + (pc & 0x7fffu);
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

    unsigned total = 0, cases = argc > 2 && !use_real_children
                                    ? (unsigned)strtoul(argv[2], NULL, 10)
                                    : 512u;
    for (suite = 0; suite < 12u; ++suite) {
        for (number = 0; number < cases; ++number) {
            if (run(reference))
                return 1;
            ++total;
        }
        if (!use_real_children) {
            for (unwind_at = 1; unwind_at <= counts[suite]; ++unwind_at)
                for (number = 0; number < 32u; ++number) {
                    if (run(reference))
                        return 1;
                    ++total;
                }
            unwind_at = 0;
            for (bad_at = 1; bad_at <= counts[suite]; ++bad_at)
                for (bad_kind = 0; bad_kind < 7u; ++bad_kind)
                    for (number = 0; number < 32u; ++number) {
                        if (run(reference))
                            return 1;
                        ++total;
                    }
            bad_at = 0;
        }
        printf("PASS scene%04X states%u cumulative%u\n", handlers[suite], cases, total);
        fflush(stdout);
    }
    for (unsigned a = 0; a < 12u; ++a)
        for (unsigned b = 0; b < counts[a]; ++b)
            if (!reached[a][b]) {
                fprintf(stderr, "unreached%u/%u\n", a, b);
                return 1;
            }
    printf(
        "PASS%u whole-engine camera/render original-ROM states;CPU/fullWRAM/orderedbus/real child frames\n",
        total);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text/text_internal.h"
#include "core/cpu_ops.h"
static unsigned checked, family;
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
static uint8_t poison_child(void *context, Lufia2CpuState *cpu, uint32_t target,
                            uint32_t site, uint8_t frame) {
    (void)context;
    (void)cpu;
    (void)target;
    (void)site;
    (void)frame;
    fputs("guard child\n", stderr);
    exit(1);
}
static int check(uint16_t handler, unsigned variant, unsigned unknown) {
    const Lufia2Memory memory = {poison_read, poison_write, NULL, NULL, NULL};
    Lufia2CpuState state = {0};
    state.program_bank = 0x80u;
    state.accumulator_is_8_bit = 1u;
    state.stack = 0x1f80u;
    state.accumulator = 0x1234u;
    state.x = 0xabcdu;
    state.y = 0x9000u;
    state.data_bank = 0x7eu;
    state.carry = state.overflow = state.irq_disable = 1;
    Lufia2PushedChildCall child = poison_child;
    switch (variant) {
    case 0:
        child = NULL;
        break;
    case 1:
        state.program_bank = 0u;
        break;
    case 2:
        state.program_bank = 0x7eu;
        break;
    case 3:
        state.program_bank = 0x86u;
        break;
    case 4:
        state.program_bank = 0xffu;
        break;
    case 5:
        state.accumulator_is_8_bit = 0u;
        break;
    case 6:
        state.index_is_8_bit = 1u;
        break;
    case 7:
        state.decimal = 1u;
        break;
    case 8:
        state.direct_page = 1u;
        break;
    case 9:
        state.direct_page = 0x100u;
        break;
    case 10:
        state.direct_page = 0xffffu;
        break;
    case 11:
        state.stack = 0u;
        break;
    case 12:
        state.stack = 0x1f0fu;
        break;
    case 13:
        state.stack = 0x1ffdu;
        break;
    case 14:
        state.stack = 0xffffu;
        break;
    }
    Lufia2CpuState expected = state;
    Lufia2ExecutionResult result =
        (family ? Lufia2TextRunRenderCommand
                : Lufia2TextRunCameraCommand)(&memory, &state, handler, child, NULL);
    if (!unknown)
        expected.resume_pc = 0x800000u | handler;
    if (memcmp(&expected, &state, sizeof state) ||
        result.flow !=
            (unknown ? LUFIA2_EXECUTION_RETURNED : LUFIA2_EXECUTION_BOUNDARY) ||
        result.pc != (unknown ? 0u : 0x800000u | handler))
        return 1;
    ++checked;
    return 0;
}
static int VerifySceneGuards(void) {
    static const uint16_t guard_handlers[2][6] = {
        {0xa2b0, 0xb721, 0xb341, 0xb34f, 0xb494, 0xaf6b},
        {0xafc0, 0xb62b, 0xb6f4, 0xb72d, 0xb646, 0xb659}};
    for (family = 0; family < 2u; ++family) {
        for (unsigned handler = 0; handler < 65536u; ++handler) {
            unsigned known = 0;
            for (unsigned i = 0; i < 6u; ++i)
                known |= handler == guard_handlers[family][i];
            if (!known && check((uint16_t)handler, 15u, 1u))
                return 1;
        }
        for (unsigned i = 0; i < 6u; ++i)
            for (unsigned variant = 0; variant < 15u; ++variant)
                if (check(guard_handlers[family][i], variant, 0u))
                    return 1;
    }
    printf("PASS%u unsupported-handler/context pre-access guards\n", checked);
    return 0;
}
