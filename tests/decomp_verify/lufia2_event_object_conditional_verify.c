#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp816.h"
#include "snes_function_verify.h"
#include "core/cpu_ops.h"
#include "lufia2/decomp.h"

static SnesVerifyBus buses[2];
static SnesVerifyBusEvent traces[2][524288];
static unsigned trace_count[2], hits[65536], scenario, child_count[2],
    diagnostic_count[2];

static uint32_t fetch_first, fetch_end;
static const uint8_t instruction_lengths[4][256] = {
    {
        2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 3u, 2u, 4u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 1u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 4u, 3u, 3u, 4u, 1u, 2u, 3u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 3u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        3u, 2u, 3u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 3u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 3u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
    },
    {
        2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 3u, 2u, 4u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 1u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 4u, 3u, 3u, 4u, 1u, 2u, 3u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 3u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
    },
    {
        2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 3u, 2u, 4u, 2u, 2u, 2u, 2u, 2u,
        1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 1u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 4u, 3u, 3u, 4u, 1u, 2u, 3u, 2u,
        2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 3u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u,
        3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
        3u, 2u, 3u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u,
        2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u, 3u, 2u, 2u, 2u, 2u, 2u, 2u, 2u,
        1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 3u, 1u, 1u,
        3u, 3u, 3u, 4u, 3u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 2u, 1u, 1u, 3u, 3u, 3u, 4u,
        2u, 2u, 2u, 2u, 3u, 2u, 2u, 2u, 1u, 3u, 1u, 1u, 3u, 3u, 3u, 4u,
    },
    {
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
    },
};

static uint8_t *original_rom;
static unsigned random_value;
static uint8_t next_byte(void) {
    random_value ^= random_value << 13;
    random_value ^= random_value >> 17;
    random_value ^= random_value << 5;
    return (uint8_t)random_value;
}
static void observe(void *context, uint32_t address, uint8_t value, bool write) {
    unsigned side = (unsigned)(uintptr_t)context;
    if (!side && !write && address >= fetch_first && address < fetch_end)
        return;
    if (trace_count[side] == 524288u)
        abort();
    SnesVerifyBusEvent *event = &traces[side][trace_count[side]++];
    event->address = address;
    event->value = value;
    event->write = write;
}
static void to_reference(Interp816 *r, const Lufia2CpuState *n, uint32_t pc) {
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
    r->mf = n->accumulator_is_8_bit != 0u;
    r->xf = n->index_is_8_bit != 0u;
    r->e = r->irqWanted = r->nmiWanted = r->waiting = r->stopped = false;
    interp816_set_brk_hook_enabled(r, false);
}
static void from_reference(Lufia2CpuState *n, const Interp816 *r) {
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
static int same_cpu(const Lufia2CpuState *n, const Interp816 *r) {
    Lufia2CpuState other = {0};
    from_reference(&other, r);
    return n->accumulator == other.accumulator && n->x == other.x && n->y == other.y &&
           n->stack == other.stack && n->direct_page == other.direct_page &&
           n->data_bank == other.data_bank && n->program_bank == other.program_bank &&
           PackStatus(n) == PackStatus(&other);
}
static void step(Interp816 *r) {
    uint32_t pc = SnesVerifyPc24(r);
    unsigned opcode = original_rom[((pc >> 16) & 127u) * 32768u + (pc & 32767u)];
    fetch_first = pc;
    fetch_end =
        pc + instruction_lengths[(unsigned)r->mf * 2u + (unsigned)r->xf][opcode];
    ++hits[r->pc];
    interp816_runOpcode(r);
    fetch_first = fetch_end = 0u;
}

static uint64_t child_hash[2];
static const uint32_t sites[] = {0x80cc0eu, 0x80cc1au, 0x80d623u},
                      targets[] = {0x80cc26u, 0x808285u, 0x858415u};
static uint16_t script_offsets[64];
static unsigned script_banks[64], script_bank_count;
static void witness(unsigned side, const Lufia2CpuState *cpu, unsigned child) {
    const uint32_t words[] = {cpu->accumulator, cpu->x,          cpu->y,
                              cpu->stack,       cpu->data_bank,  cpu->program_bank,
                              cpu->direct_page, PackStatus(cpu), child};
    for (unsigned i = 0u; i < sizeof(words) / sizeof(words[0]); ++i)
        child_hash[side] = (child_hash[side] ^ words[i]) * 1099511628211ull;
}
static void controlled_child(unsigned side, Lufia2CpuState *cpu, unsigned child) {
    Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &buses[side], NULL,
                           NULL};
    witness(side, cpu, child);
    ++child_count[side];
    if (child == 2u)
        ++diagnostic_count[side];
    OpLoadA(cpu, (uint16_t)(scenario * 197u + child));
    cpu->carry = (scenario >> 3) & 1u;
    OpLdx(cpu, (uint16_t)(scenario * 113u));
    OpLdy(cpu, (uint16_t)(scenario * 71u));
    Write8(&memory, 0x7e1400u + child, (uint8_t)scenario);
    if (child == 2u)
        Write8(&memory, 0x7e1273u, (scenario & 1u) ? 0u : 8u);
    SimulateRtlFrame(&memory, cpu);
    cpu->program_bank = 0x80u;
}
static uint8_t child_call(void *context, Lufia2CpuState *cpu, uint32_t target,
                          uint32_t site, uint8_t frame) {
    (void)context;
    unsigned i;
    for (i = 0u; i < 3u; ++i)
        if (site == sites[i])
            break;
    if (i == 3u || target != targets[i] || frame != 3u)
        abort();
    cpu->program_bank = (uint8_t)(target >> 16);
    controlled_child(1u, cpu, i);
    return 1u;
}
int g_interp_apu_driving;
int main(int argc, char **argv) {
    uint8_t *rom;
    size_t size;
    if (argc < 2 || !SnesVerifyLoadFile(argv[1], &rom, &size) || size != 0x280000u)
        return 2;
    original_rom = rom;
    if (argc == 5 && strcmp(argv[2], "--fault") == 0) {
        unsigned address = (unsigned)strtoul(argv[3], NULL, 16),
                 value = (unsigned)strtoul(argv[4], NULL, 16);
        size_t offset = ((address >> 16) & 127u) * 32768u + (address & 32767u);
        if (offset >= size)
            return 2;
        rom[offset] = (uint8_t)value;
    }
    bool legacy = argc == 3 && strcmp(argv[2], "--legacy") == 0;
    bool low_stack = argc == 3 && strcmp(argv[2], "--low-stack") == 0;
    bool handoff = legacy || low_stack;
    for (unsigned bank = 0u; bank < 64u; ++bank) {
        size_t base = (size_t)bank * 32768u;
        unsigned offset;
        for (offset = 0u; offset < 32766u; ++offset)
            if (rom[base + offset] == 0x8fu && rom[base + offset + 2u] == 0u)
                break;
        if (offset == 32766u)
            continue;
        script_offsets[bank] = (uint16_t)(0x8000u + offset);
        script_banks[script_bank_count++] = bank;
    }
    if (!script_bank_count)
        return 2;
    for (unsigned side = 0u; side < 2u; ++side) {
        if (!SnesVerifyBusInit(&buses[side], rom, size))
            return 2;
        buses[side].event_observer = observe;
        buses[side].event_observer_context = (void *)(uintptr_t)side;
    }
    Interp816 *r = interp816_init(&buses[0], SnesVerifyBusRead, SnesVerifyBusWrite);
    const Lufia2Memory memory = {SnesVerifyBusRead, SnesVerifyBusWrite, &buses[1], NULL,
                                 NULL};

    unsigned redrawn = 0u, uploaded = 0u, plain = 0u;
    unsigned list = 0u;
    while (list < 0x474bu && rom[0x11u * 32768u + 0x38b5u + list] != 0xffu)
        ++list;
    if (list == 0x474bu)
        return 2;
    for (scenario = 0u; scenario < 512u; ++scenario) {
        random_value = scenario + 0xcbaeu;
        unsigned slot = (scenario >> 5) & 7u,
                 bank = script_banks[(scenario >> 1) % script_bank_count];
        uint16_t text = script_offsets[bank];
        for (unsigned side = 0u; side < 2u; ++side) {
            memset(buses[side].wram, (int)next_byte(), SNES_VERIFY_WRAM_SIZE);
        }
        memcpy(buses[1].wram, buses[0].wram, SNES_VERIFY_WRAM_SIZE);
        for (unsigned side = 0u; side < 2u; ++side) {
            memset(buses[side].wram + 0x1d18cu, 0, 8u);
            buses[side].wram[0x1d18cu + slot] = 0x81u;
            buses[side].wram[0x1d134u + 3u * slot] = (uint8_t)text;
            buses[side].wram[0x1d135u + 3u * slot] = (uint8_t)(text >> 8);
            buses[side].wram[0x1d136u + 3u * slot] = (uint8_t)(0x80u + bank);
            for (unsigned layer = 0u; layer < 4u; ++layer)
                buses[side].wram[0x1d020u + 2u * layer] = 0xffu;
            buses[side].wram[0x583] = (uint8_t)scenario;
            buses[side].wram[0x1d1a0u] = (uint8_t)list;
            buses[side].wram[0x1d1a1u] = (uint8_t)(list >> 8);
            trace_count[side] = child_count[side] = diagnostic_count[side] = 0u;
            child_hash[side] = 14695981039346656037ull;
            SnesVerifyBusResetTrace(&buses[side]);
            SnesVerifyBusResetMmio(&buses[side]);
        }
        Lufia2CpuState n = {0};
        n.program_bank = 0x80u;
        n.data_bank = (uint8_t)(scenario >> 4);
        n.accumulator_is_8_bit = 1u;
        n.index_is_8_bit = scenario & 1u;
        n.stack = (uint16_t)(low_stack ? 0x1f20u + scenario % 12u
                                       : 0x1f80u + scenario % 110u);
        n.accumulator = (uint16_t)(scenario * 197u);
        n.x = (uint16_t)(scenario * 113u);
        n.y = (uint16_t)(scenario * 71u);
        if (n.index_is_8_bit) {
            n.x &= 255u;
            n.y &= 255u;
        }
        n.carry = scenario & 1u;
        n.zero = (scenario >> 1) & 1u;
        n.negative = (scenario >> 2) & 1u;
        n.overflow = (scenario >> 3) & 1u;
        n.irq_disable = (scenario >> 4) & 1u;
        to_reference(r, &n, 0x80cbaeu);
        Lufia2ExecutionResult result =
            legacy
                ? Lufia2FieldEventTimerTick(&memory, &n)
                : Lufia2FieldEventTimerTickWithServices(&memory, &n, child_call, NULL);
        unsigned steps;
        for (steps = 0u; steps < 100000u; ++steps) {
            uint32_t pc = SnesVerifyPc24(r);
            if (pc == result.pc)
                break;
            unsigned found;
            for (found = 0u; found < 3u; ++found)
                if (pc == targets[found])
                    break;
            if (found < 3u) {
                Lufia2CpuState c = {0};
                from_reference(&c, r);
                controlled_child(0u, &c, found);
                to_reference(r, &c, sites[found] + 4u);
                continue;
            }
            step(r);
        }
        if (steps == 100000u ||
            result.flow !=
                (handoff ? LUFIA2_EXECUTION_BOUNDARY : LUFIA2_EXECUTION_RETURNED) ||
            result.pc != (handoff ? 0x858415u : 0x80cc25u) ||
            n.program_bank != (handoff ? 0x85u : 0x80u) ||
            n.data_bank != (handoff ? 0x91u : (uint8_t)(scenario >> 4)) ||
            (handoff && n.x != 0x8743u) || !same_cpu(&n, r) ||
            memcmp(buses[0].wram, buses[1].wram, SNES_VERIFY_WRAM_SIZE) ||
            trace_count[0] != trace_count[1] || child_count[0] != child_count[1] ||
            child_hash[0] != child_hash[1] ||
            diagnostic_count[0] != (handoff ? 0u : 1u) ||
            diagnostic_count[1] != diagnostic_count[0]) {
            fprintf(
                stderr,
                "redraw mismatch case%u PC%06X/%06X CPU%d WRAM%d trace%u/%u child%u/%u\n",
                scenario, SnesVerifyPc24(r), result.pc, same_cpu(&n, r),
                memcmp(buses[0].wram, buses[1].wram, SNES_VERIFY_WRAM_SIZE),
                trace_count[0], trace_count[1], child_count[0], child_count[1]);
            return 1;
        }
        for (unsigned i = 0u; i < trace_count[0]; ++i)
            if (traces[0][i].address != traces[1][i].address ||
                traces[0][i].value != traces[1][i].value ||
                traces[0][i].write != traces[1][i].write) {
                fprintf(stderr, "redraw bus case%u event%u %06X/%06X\n", scenario, i,
                        traces[0][i].address, traces[1][i].address);
                return 1;
            }
        if (child_count[1] > 1u) {
            ++redrawn;
            uploaded += child_count[1] == 3u;
        } else
            ++plain;
    }
    printf(
        "PASS512 original-ROM conditional-object VM states;legacy%u low-stack%u;diagnostic then END/redraw/upload;CPU/WRAM/bus/child witnesses;%u/%u/%u\n",
        legacy, low_stack, redrawn, uploaded, plain);
    interp816_free(r);
    for (unsigned side = 0u; side < 2u; ++side)
        SnesVerifyBusDestroy(&buses[side]);
    free(rom);
    return 0;
}
