#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "text/text_internal.h"
#include "field/event_script_internal.h"
#include "system/wram.h"
#include "lufia2/party.h"
#include "lufia2/field.h"
#include "lufia2/battle.h"
#include "lufia2/system.h"
#include "lufia2/text.h"
#include "system/gold_internal.h"
#include "core/cpu_ops.h"

typedef struct Trace {
    SnesVerifyBusEvent events[400000];
    unsigned count;
} Trace;
static SnesVerifyBus original, native;
static Trace original_trace, native_trace;

typedef struct RecoveryClock {
    unsigned phase, polls, frame_reads;
} RecoveryClock;
static RecoveryClock original_clock, native_clock, probe_clock;
static unsigned number, unwind_at, bad_at, bad_kind, stalled_phase;
static unsigned native_calls, original_calls, handler_visits, original_active;
static Lufia2CpuState native_entries[106], original_entries[106];
static const uint32_t targets[] = {0x8093feu, 0x81f5bdu, 0x80bf0bu, 0x80969fu,
                                   0x848311u, 0x80bf0bu, 0x80bf43u};
static const uint32_t sites[] = {0x80a2e4u, 0x80a2f5u, 0x80a2fcu, 0x80a312u,
                                 0x80a317u, 0x80a31bu, 0x80a325u};
static const uint8_t frames[] = {3u, 3u, 2u, 3u, 3u, 2u, 2u};
#ifdef RUNTIME_ABI
extern RecompReturn Lufia2DecompBridge_9CB8(CpuState *);
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

static void word(uint32_t a, uint16_t v) {
    SnesVerifyBusPoke(&native, a, (uint8_t)v);
    SnesVerifyBusPoke(&native, a + 1u, (uint8_t)(v >> 8));
}

static uint16_t romword(uint32_t address) {
    unsigned a = ((address >> 16) & 127u) * 32768u + (address & 32767u);
    return (uint16_t)(native.rom[a] | native.rom[a + 1u] << 8);
}

static RecoveryClock *clock_for(SnesVerifyBus *b) {
    return b == &native     ? &native_clock
           : b == &original ? &original_clock
                            : &probe_clock;
}
static unsigned ram_offset(uint32_t a) {
    unsigned bank = a >> 16, low = a & 0xffffu;
    if (bank == 0x7eu)
        return low;
    if (bank < 0x40u || (bank >= 0x80u && bank < 0xc0u))
        return low < 0x2000u ? low : 0xffffffffu;
    return 0xffffffffu;
}
static uint8_t recovery_read(void *context, uint32_t address) {
    SnesVerifyBus *b = context;
    RecoveryClock *clock = clock_for(b);
    unsigned offset = ram_offset(address);
    if (offset == 0x40u && !(++clock->frame_reads & 1u))
        ++b->wram[0x40u];
    if (offset == 0x581u && clock->phase) {
        ++clock->polls;
        unsigned delay =
            (unsigned[]){1, 2, 3, 7, 31, 255, 1024, 17}[(number + clock->phase) % 8u];
        if (clock->phase != stalled_phase && clock->polls == delay)
            b->wram[0x581u] = 0;
    }
    return SnesVerifyBusRead(b, address);
}
static void recovery_write(void *context, uint32_t address, uint8_t value) {
    SnesVerifyBus *b = context;
    SnesVerifyBusWrite(b, address, value);
    if (ram_offset(address) == 0x581u && (value == 0xc1u || value == 0x81u)) {
        RecoveryClock *clock = clock_for(b);
        clock->phase = value == 0xc1u ? 1u : 2u;
        clock->polls = 0;
    }
}
static unsigned call_index(unsigned ordinal) {
    return ordinal == 1u     ? 0u
           : ordinal == 2u   ? 1u
           : ordinal <= 102u ? 2u
           : ordinal == 103u ? 3u
           : ordinal == 104u ? 4u
           : ordinal == 105u ? 5u
                             : 6u;
}
static void seed(Lufia2CpuState *n) {
    memset(native.wram, (int)(number * 71u), SNES_VERIFY_WRAM_SIZE);
    memset(n, 0, sizeof *n);
    memset(&original_clock, 0, sizeof original_clock);
    memset(&native_clock, 0, sizeof native_clock);
    memset(&probe_clock, 0, sizeof probe_clock);
    memset(native_entries, 0, sizeof native_entries);
    memset(original_entries, 0, sizeof original_entries);
    native_calls = original_calls = handler_visits = original_active = 0;
    n->program_bank = 0x80u;
    n->accumulator_is_8_bit = 1u;
    n->data_bank = (uint8_t[]){0, 0x7e, 0x80, 0x83}[number % 4u];
    n->accumulator = (uint16_t)(number * 197u);
    n->x = (uint16_t)(number * 83u);
    n->y = (uint16_t)(0x9000u + number % 16u * 64u);
    n->stack = (uint16_t)(number % 16u == 0u   ? 0x1f20u
                          : number % 16u == 1u ? 0x1ffcu
                                               : 0x1f80u + number % 64u);
    n->carry = number & 1u;
    n->zero = (number >> 1) & 1u;
    n->negative = (number >> 2) & 1u;
    n->overflow = (number >> 3) & 1u;
    n->irq_disable = (number >> 4) & 1u;
    for (unsigned i = 0; i < 4u; ++i) {
        uint16_t rec = (uint16_t)(0xc00u + i * 0x100u);
        if (i == 2u && number % 8u == 0u)
            rec = 0;
        word(0xa80u + i * 2u, rec);
        native.wram[rec + 15u] = (uint8_t)(number + i * 3u);
        word(rec + 0x25u, (uint16_t)(number * 29u + i * 137u));
        word(rec + 0x27u, (uint16_t)(number * 31u + i * 117u));
    }
    word(0xb59u, (uint16_t[]){0, 1, 0xfffe, 0xffff}[number % 4u]);
    word(0xa8au,
         (uint16_t[]){0, 1, 0xff, 0x967f, 0xffff, 0x8000, 0x1234, 0xfffe}[number % 8u]);
    native.wram[0xa8cu] = (uint8_t[]){0, 1, 0x98, 0xff}[(number / 8u) % 4u];
    native.wram[0x40u] = (uint8_t)(number * 29u);
    native.wram[0x99du] = (uint8_t)(number % 100u);
    native.wram[n->y] = (uint8_t)(number * 37u);
    if (number % 16u == 15u)
        n->y = 0xffffu;
#ifdef RUNTIME_ABI
    n->data_bank = 0x7eu;
    if (n->stack < 0x1f40u)
        n->stack = 0x1f40u;
    native.wram[0x099bu] = native.wram[0x099cu] = native.wram[0x1259u] =
        native.wram[0x1d0ffu] = 0;
    native.wram[0x0b54u] = 30u;
    native.wram[0x05b5u] = 0;
    word(0x9b7u, 0x9000u);
    native.wram[0x9b9u] = 0x7eu;
    unsigned opcode = 0;
    for (; opcode < 205u; ++opcode)
        if (romword(0x80ca14u + 2u * opcode) == 0xa2e1u)
            break;
    if (opcode == 205u)
        exit(2);
    native.wram[0x9000u] = (uint8_t)opcode;
    native.wram[0x9001u] = (uint8_t)(number * 37u);
    native.wram[0x9002u] = 0x37u;
    native.wram[0x9003u] = 0xffu;
    if (number & 1u)
        SetIndexWidth(n, 1u);
#endif
    memcpy(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE);
    reset_devices(&native);
    reset_devices(&original);
    original_trace.count = native_trace.count = 0;
}
static Trace *trace_for(SnesVerifyBus *b) {
    return b == &native ? &native_trace : b == &original ? &original_trace : NULL;
}
static void instruction(SnesVerifyBus *b, Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    Trace *trace = trace_for(b);
    uint8_t op = recovery_read(b, pc);
    if (trace)
        --trace->count;
    fetch_trace = trace;
    fetch_address = pc;
    fetch_left = lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][op];
    if (!fetch_left)
        exit(1);
    interp816_runOpcode(r);
    fetch_left = 0;
}
static void return_frame(const Lufia2Memory *m, Lufia2CpuState *n, unsigned index) {
    uint16_t back = Pull8(m, n);
    back |= (uint16_t)Pull8(m, n) << 8;
    if (back != (uint16_t)(sites[index] + frames[index]))
        exit(1);
    if (frames[index] == 3u && Pull8(m, n) != 0x80u)
        exit(1);
    n->program_bank = 0x80u;
}
static void wait_original(const Lufia2Memory *m, Lufia2CpuState *n) {
    Interp816 *r = interp816_init(m->context, recovery_read, recovery_write);
    load(r, n, 0x80bf0bu);
    unsigned steps = 0;
    while (SnesVerifyPc24(r) != 0x80bf11u && steps++ < 32u)
        instruction(m->context, r);
    if (steps == 32u)
        exit(1);
    instruction(m->context, r);
    store(n, r);
    interp816_free(r);
}
static void returned_child(const Lufia2Memory *m, Lufia2CpuState *n, unsigned index) {
    if (index == 2u || index == 5u) {
        wait_original(m, n);
        return;
    }
    if (index == 1u) {
        Lufia2ExecutionResult q = Lufia2PartyRestoreVitals(m, n);
        if (q.flow != LUFIA2_EXECUTION_RETURNED)
            exit(1);
    } else if (index == 6u)
        Lufia2ChangeGold(m, n, false);
    else {
        OpSta(m, n, 0x7ff110u + index * 2u);
        if (index == 4u) {
            OpLdx(n, 0xfffeu);
            OpLoadA(n, 0x20u);
        } else if (index != 0u) {
            OpLoadA(n, (uint16_t)(number * 17u));
            OpLdx(n, (uint16_t)(number * 31u));
            OpLdy(n, (uint16_t)(number * 11u));
        }
    }
    return_frame(m, n, index);
}
static void corrupt_return(Lufia2CpuState *n, unsigned ordinal) {
    if (ordinal != bad_at)
        return;
    if (bad_kind == 0u)
        n->accumulator_is_8_bit = 0u;
    else if (bad_kind == 1u)
        SetIndexWidth(n, 1u);
    else if (bad_kind == 2u)
        n->direct_page = 0x100u;
    else if (bad_kind == 3u)
        n->decimal = 1u;
    else
        n->program_bank = 0x83u;
}
static uint8_t child(void *context, Lufia2CpuState *n, uint32_t target, uint32_t site,
                     uint8_t frame) {
    unsigned ordinal = ++native_calls, index = call_index(ordinal);
    if (ordinal > 106u || target != targets[index] || site != sites[index] ||
        frame != frames[index])
        exit(1);
    n->program_bank = (uint8_t)(target >> 16);
    native_entries[ordinal - 1u] = *n;
    if (ordinal == unwind_at)
        return 0u;
    returned_child(context, n, index);
    corrupt_return(n, ordinal);
    return 1u;
}
#ifdef RUNTIME_ABI
static RecoveryClock saved_original_clock, saved_native_clock;
static unsigned saved_calls[4];
static Lufia2CpuState saved_native_entries[106], saved_original_entries[106];
void RuntimeSaveProbeVisits(void) {
    saved_original_clock = original_clock;
    saved_native_clock = native_clock;
    saved_calls[0] = native_calls;
    saved_calls[1] = original_calls;
    saved_calls[2] = unwind_at;
    saved_calls[3] = bad_at;
    memcpy(saved_native_entries, native_entries, sizeof native_entries);
    memcpy(saved_original_entries, original_entries, sizeof original_entries);
}
void RuntimeResetProbeVisits(void) {
    memset(&probe_clock, 0, sizeof probe_clock);
    native_calls = 0;
    unwind_at = bad_at = 0;
}
void RuntimeRestoreProbeVisits(void) {
    original_clock = saved_original_clock;
    native_clock = saved_native_clock;
    native_calls = saved_calls[0];
    original_calls = saved_calls[1];
    unwind_at = saved_calls[2];
    bad_at = saved_calls[3];
    memcpy(native_entries, saved_native_entries, sizeof native_entries);
    memcpy(original_entries, saved_original_entries, sizeof original_entries);
}
static void controlled(SnesVerifyBus *b, Lufia2CpuState *n, uint32_t site,
                       uint8_t frame) {
    unsigned i = 0;
    for (; i < 7u; ++i)
        if (site == sites[i])
            break;
    if (i == 7u || frame != frames[i])
        exit(1);
    Lufia2Memory m = {recovery_read, recovery_write, b, NULL, NULL};
    n->program_bank = (uint8_t)(targets[i] >> 16);
    returned_child(&m, n, i);
}
#endif
static int child_entry(Interp816 *r) {
    if (!original_active)
        return 0;
    unsigned ordinal = original_calls + 1u;
    if (ordinal > 106u)
        return 0;
    unsigned index = call_index(ordinal);
    if (SnesVerifyPc24(r) != targets[index])
        return 0;
    ++original_calls;
    store(&original_entries[ordinal - 1u], r);
    if (ordinal == unwind_at)
        return 1;
    if (index != 1u && index != 6u && index != 2u && index != 5u) {
        Lufia2CpuState n = {0};
        store(&n, r);
        Lufia2Memory m = {recovery_read, recovery_write, &original, NULL, NULL};
        returned_child(&m, &n, index);
        corrupt_return(&n, ordinal);
        load(r, &n, sites[index] + frames[index] + 1u);
        return 2;
    }
    return 0;
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
    uint32_t entry = 0x80a2e1u;
    Lufia2Memory m = {recovery_read, recovery_write, &native, NULL, NULL};
#ifdef RUNTIME_ABI
    entry = 0x809cb8u;
#endif
    load(r, &n, entry);
    Lufia2ExecutionResult q;
#ifdef RUNTIME_ABI
    if (!number && !unwind_at && !bad_at && !stalled_phase) {
        for (unsigned i = 0; i < 7u; ++i) {
            runtime_probe_desired_site = sites[i];
            runtime_probe(&m, &n, Lufia2DecompBridge_9CB8, entry, 0u, 3u);
        }
        runtime_probe_desired_site = 0;
    }
    q = runtime_invoke(&m, &n, child, (void *)&m, Lufia2DecompBridge_9CB8,
                       Lufia2TextEngineStep, Lufia2TextEngineStepWithServices, entry,
                       0u, 3u);
#else
    q = Lufia2TextRecoverParty(&m, &n, child, (void *)&m);
#endif
    uint32_t stop = q.pc;
    if (q.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        stop = targets[call_index(unwind_at)];
    unsigned steps;
    for (steps = 0; steps < 1000000u; ++steps) {
        uint32_t pc = SnesVerifyPc24(r);
        if (pc == 0x80a2e1u) {
            ++handler_visits;
            original_active = 1u;
        }
        if (pc == 0x809d00u && original_calls == 106u)
            original_active = 0u;
        if (bad_at && original_calls == bad_at) {
            unsigned index = call_index(bad_at);
            if ((index == 1u || index == 2u || index == 5u || index == 6u) &&
                pc == sites[index] + frames[index] + 1u) {
                Lufia2CpuState changed = {0};
                store(&changed, r);
                corrupt_return(&changed, bad_at);
                load(r, &changed, pc);
            }
        }
        if (pc == stop && handler_visits && (steps || stop != entry)) {
            if (unwind_at) {
                if (child_entry(r) != 1) {
                    instruction(&original, r);
                    continue;
                }
            } else if (stalled_phase && original_clock.phase == stalled_phase &&
                       original_clock.polls < 65536u) {
                instruction(&original, r);
                continue;
            }
            break;
        }
        int handled = child_entry(r);
        if (handled == 1)
            exit(1);
        if (handled == 2)
            continue;
        instruction(&original, r);
    }
    if (!unwind_at && !bad_at && !stalled_phase &&
        (native_calls != 106u || original_calls != 106u ||
         native_clock.frame_reads != 202u || original_clock.frame_reads != 202u)) {
        fputs("missing original recovery calls/frame reads\n", stderr);
        return 1;
    }
    Lufia2CpuState expected = n;
    store(&expected, r);
    if (steps == 1000000u || memcmp(&expected, &n, sizeof n) ||
        memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE) || !same_trace() ||
        native_calls != original_calls ||
        memcmp(native_entries, original_entries, sizeof native_entries)) {
        fprintf(stderr,
                "recovery case%u unwind%u bad%u/%u stall%u steps%u PC%06X CPU%d WRAM%d "
                "trace%u/%u calls%u/%u A%04X/%04X Y%04X/%04X S%04X/%04X PB%02X/%02X\n",
                number, unwind_at, bad_at, bad_kind, stalled_phase, steps, stop,
                memcmp(&expected, &n, sizeof n),
                memcmp(original.wram, native.wram, SNES_VERIFY_WRAM_SIZE),
                original_trace.count, native_trace.count, native_calls, original_calls,
                n.accumulator, r->a, n.y, r->y, n.stack, r->sp, n.program_bank, r->k);
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
    Interp816 *r = interp816_init(&original, recovery_read, recovery_write);
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

    if (selected_handler && selected_handler != 0xa2e1u)
        return 2;
    if (argc == 3 && strcmp(argv[2], "--guards") == 0) {
        unsigned guards = 0;
        for (unsigned variant = 0; variant < 15u; ++variant) {
            number = 3;
            Lufia2CpuState n;
            seed(&n);
            SetIndexWidth(&n, 0u);
            if (variant == 0u)
                n.accumulator_is_8_bit = 0;
            else if (variant == 1u)
                SetIndexWidth(&n, 1u);
            else if (variant == 2u)
                n.program_bank = 0;
            else if (variant == 3u)
                n.decimal = 1;
            else if (variant < 10u)
                n.direct_page =
                    (uint16_t[]){1, 0xff, 0x100, 0xffff, 0xffc0, 0x1ecb}[variant - 4u];
            else if (variant < 14u)
                n.stack = (uint16_t[]){0, 0x54, 0x1f1f, 0x1ffd}[variant - 10u];
            Lufia2CpuState before = n;
            before.resume_pc = 0x80a2e1u;
            Lufia2Memory m = {recovery_read, recovery_write, &native, NULL, NULL};
            Lufia2ExecutionResult q = Lufia2TextRecoverParty(
                &m, &n, variant == 14u ? NULL : child, (void *)&m);
            if (q.flow != LUFIA2_EXECUTION_BOUNDARY || q.pc != before.resume_pc ||
                memcmp(&before, &n, sizeof n) || native_trace.count)
                return 1;
            ++guards;
        }
#ifdef RUNTIME_ABI
        for (unsigned variant = 0; variant < 3u; ++variant) {
            number = 3;
            Lufia2CpuState n;
            seed(&n);
            Lufia2Memory m = {recovery_read, recovery_write, &native, NULL, NULL};
            RuntimeCase test = {0};
            test.memory = &m;
            test.entry = 0x809cb8u;
            test.start = n.stack;
            test.hrv = 3u;
            test.guard_only = 1u;
            runtime_case = &test;
            CpuState c;
            memset(&c, 0, sizeof c);
            runtime_load(&c, &n);
            c.host_return_valid = 3u;
            if (variant == 0u)
                c.m_flag = 0u;
            else if (variant == 1u)
                c._flag_D = 1u;
            else
                c.emulation = 1u;
            cpu_mirrors_to_p(&c);
            CpuState before = c;
            RecompReturn ret = Lufia2DecompBridge_9CB8(&c);
            runtime_return_reads = 0u;
            c.open_bus = before.open_bus;
            if (ret != RECOMP_RETURN_LLE_UNWIND_BASE || test.stub != 1u ||
                test.target != test.entry || test.kind != 3u ||
                memcmp(&c, &before, sizeof c) || native_trace.count)
                return 1;
            ++guards;
        }
        runtime_case = NULL;
#endif
        printf("PASS recovery guards:%u\n", guards);
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--returns") == 0) {
        for (unsigned i = 0; i < 7u; ++i) {
            bad_at = (unsigned[]){1, 2, 3, 103, 104, 105, 106}[i];
            for (bad_kind = 0; bad_kind < 4u; ++bad_kind)
                for (number = 0; number < 4u; ++number)
                    if (run(r))
                        return 1;
        }
        puts("PASS recovery changed-child contexts112");
        return 0;
    }
#ifdef RUNTIME_ABI
    if (argc == 3 && strcmp(argv[2], "--legacy-banks") == 0) {
        unsigned cases = 0;
        for (unsigned route = 0; route < 2u; ++route) {
            uint16_t handler = route ? 0xba39u : 0xab45u;
            unsigned opcode = 0;
            for (;opcode < 205u;++opcode)
                if (romword(0x80ca14u + opcode * 2u) == handler)break;
            if (opcode == 205u)return 2;
            for (number=0;number<64u;++number) {
                Lufia2CpuState n;seed(&n);
                native.wram[0x9000u]=(uint8_t)opcode;
                unsigned scene=0;
                for(;scene<9u;++scene)if(romword(0x80ba55u+scene*2u)==0xbb94u)break;
                if(scene==9u)return 2;
                native.wram[0x9001u]=(uint8_t)scene;
                memcpy(original.wram,native.wram,SNES_VERIFY_WRAM_SIZE);
                Lufia2Memory m={recovery_read,recovery_write,&native,NULL,NULL};
                load(r,&n,0x809cb8u);
                Lufia2ExecutionResult q=runtime_invoke(&m,&n,child,(void *)&m,
                    Lufia2DecompBridge_9CB8,Lufia2TextEngineStep,
                    Lufia2TextEngineStepWithServices,0x809cb8u,1u,3u);
                unsigned steps=0;
                while(SnesVerifyPc24(r)!=q.pc && steps++<10000u)instruction(&original,r);
                Lufia2CpuState expected=n;store(&expected,r);
                if(steps==10000u||q.flow!=LUFIA2_EXECUTION_BOUNDARY||
                   q.pc!=(route?0x83afeau:0x83acefu)||n.program_bank!=0x83u||
                   memcmp(&expected,&n,sizeof n)||
                   memcmp(original.wram,native.wram,SNES_VERIFY_WRAM_SIZE)||
                   !same_trace()||native_calls) {
                    fprintf(stderr,"legacy route%u case%u pc%06X flow%u PB%02X/%02X CPU%d RAM%d trace%u/%u calls%u\n",route,number,q.pc,q.flow,n.program_bank,r->k,memcmp(&expected,&n,sizeof n),memcmp(original.wram,native.wram,SNES_VERIFY_WRAM_SIZE),original_trace.count,native_trace.count,native_calls);
                    return 1;
                }
                ++cases;
            }
        }
        printf("PASS legacy text cross-bank handoffs:%u\n",cases);return 0;
    }
#endif
    unsigned count = 512u;
#ifdef RUNTIME_ABI
    count = 256u;
#endif
    if (argc == 3 && strcmp(argv[2], "--unwind") == 0) {
        for (unwind_at = 1u; unwind_at <= 106u; ++unwind_at)
            for (number = 0; number < 4u; ++number)
                if (run(r))
                    return 1;
        puts("PASS recovery unwind424");
        return 0;
    }
    if (argc == 3 && strcmp(argv[2], "--stalls") == 0) {
        for (stalled_phase = 1u; stalled_phase <= 2u; ++stalled_phase)
            for (number = 0; number < 8u; ++number)
                if (run(r))
                    return 1;
        puts("PASS recovery fade stalls16");
        return 0;
    }
    for (number = 0; number < count; ++number)
        if (run(r))
            return 1;
    printf("PASS recovery:%u complete CPU/fullWRAM/bus states,106 child calls each\n",
           count);
#ifdef RUNTIME_ABI
    printf("Recovery production redirects:%u, fallback:%u\n",
           runtime_redirects[0], runtime_fallback[0]);
    if (runtime_fallback[0] || runtime_redirects[0] != 105u)
        return 1;
#endif
    interp816_free(r);
    SnesVerifyBusDestroy(&original);
    SnesVerifyBusDestroy(&native);
    free(rom);
    return 0;
}
