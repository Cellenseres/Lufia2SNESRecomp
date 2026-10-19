#include "lufia2/field_phase.h"

extern uint32_t g_interp816_cur_pc;

typedef struct FieldFrameBoundaryBus {
    Interp816 *registers;
    uint32_t word_low;
    uint32_t word_high;
    uint8_t high_byte;
    bool high_pending;
} FieldFrameBoundaryBus;

static uint8_t FieldFrameBoundaryRead(void *context, uint32_t address) {
    FieldFrameBoundaryBus *bus = context;
    Interp816 *registers = bus->registers;
    if (bus->high_pending && address == bus->word_high) {
        bus->high_pending = false;
        return bus->high_byte;
    }
    if (address == bus->word_low && registers->read_word) {
        bus->word_low = UINT32_MAX;
        uint16_t value;
        if (registers->read_word(registers->mem, address, bus->word_high, &value)) {
            bus->high_byte = (uint8_t)(value >> 8);
            bus->high_pending = true;
            return (uint8_t)value;
        }
    }
    return registers->read(registers->mem, address);
}

static void FieldFrameBoundaryWrite(void *context, uint32_t address, uint8_t value) {
    FieldFrameBoundaryBus *bus = context;
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunFieldFrameBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint8_t opcode;
        uint8_t length;
        uint8_t cycles;
        uint8_t modes;
    } boundaries[] = {
        {0x838000u, 0xe2u, 2u, 3u, 15u},
        {0x838002u, 0x4bu, 1u, 3u, 15u},
        {0x838003u, 0xabu, 1u, 4u, 15u},
        {0x838004u, 0x20u, 3u, 6u, 15u},
        {0x838007u, 0xa9u, 2u, 2u, 10u},
        {0x838009u, 0x8du, 3u, 4u, 10u},
        {0x83800cu, 0xadu, 3u, 4u, 10u},
        {0x83800fu, 0xf0u, 2u, 2u, 15u},
        {0x838011u, 0xa9u, 2u, 2u, 10u},
        {0x838013u, 0x24u, 2u, 3u, 10u},
        {0x838015u, 0xd0u, 2u, 2u, 15u},
        {0x838017u, 0xadu, 3u, 4u, 10u},
        {0x83801au, 0x89u, 2u, 2u, 10u},
        {0x83801cu, 0xd0u, 2u, 2u, 15u},
        {0x83801eu, 0xadu, 3u, 4u, 10u},
        {0x838021u, 0xd0u, 2u, 2u, 15u},
        {0x838023u, 0xa9u, 2u, 2u, 10u},
        {0x838025u, 0x1cu, 3u, 6u, 10u},
        {0x838028u, 0xf0u, 2u, 2u, 15u},
        {0x83802au, 0xadu, 3u, 4u, 10u},
        {0x83802du, 0x89u, 2u, 2u, 10u},
        {0x83802fu, 0xf0u, 2u, 2u, 15u},
        {0x838031u, 0x20u, 3u, 6u, 15u},
        {0x838034u, 0x80u, 2u, 3u, 15u},
        {0x838036u, 0xa9u, 2u, 2u, 10u},
        {0x838038u, 0x1cu, 3u, 6u, 10u},
        {0x83803bu, 0xf0u, 2u, 2u, 15u},
        {0x83803du, 0xc2u, 2u, 3u, 15u},
        {0x83803fu, 0xadu, 3u, 4u, 5u},
        {0x838042u, 0x48u, 1u, 3u, 15u},
        {0x838043u, 0xadu, 3u, 4u, 5u},
        {0x838046u, 0x8du, 3u, 4u, 5u},
        {0x838049u, 0x68u, 1u, 4u, 15u},
        {0x83804au, 0x8du, 3u, 4u, 5u},
        {0x83804du, 0xe2u, 2u, 3u, 15u},
        {0x83804fu, 0x22u, 4u, 8u, 15u},
        {0x838053u, 0x9cu, 3u, 4u, 10u},
        {0x838056u, 0x9cu, 3u, 4u, 10u},
        {0x838059u, 0x9cu, 3u, 4u, 10u},
        {0x83805cu, 0x9cu, 3u, 4u, 10u},
        {0x83805fu, 0x4cu, 3u, 3u, 15u},
        {0x838062u, 0xa9u, 2u, 2u, 10u},
        {0x838064u, 0x1cu, 3u, 6u, 10u},
        {0x838067u, 0xf0u, 2u, 2u, 15u},
        {0x838069u, 0x22u, 4u, 8u, 15u},
        {0x83806du, 0x82u, 3u, 4u, 15u},
        {0x838070u, 0x20u, 3u, 6u, 15u},
        {0x838073u, 0x22u, 4u, 8u, 15u},
        {0x838077u, 0x20u, 3u, 6u, 15u},
        {0x83807au, 0x20u, 3u, 6u, 15u},
        {0x83807du, 0x22u, 4u, 8u, 15u},
        {0x838081u, 0x20u, 3u, 6u, 15u},
        {0x838084u, 0x22u, 4u, 8u, 15u},
        {0x838088u, 0x22u, 4u, 8u, 15u},
        {0x83808cu, 0x20u, 3u, 6u, 15u},
        {0x83808fu, 0x20u, 3u, 6u, 15u},
        {0x838092u, 0xadu, 3u, 4u, 10u},
        {0x838095u, 0xc9u, 2u, 2u, 10u},
        {0x838097u, 0xf0u, 2u, 2u, 15u},
        {0x838099u, 0x22u, 4u, 8u, 15u},
        {0x83809du, 0xadu, 3u, 4u, 10u},
        {0x8380a0u, 0x10u, 2u, 2u, 15u},
        {0x8380a2u, 0x82u, 3u, 4u, 15u},
        {0x8380a5u, 0xadu, 3u, 4u, 10u},
        {0x8380a8u, 0xf0u, 2u, 2u, 15u},
        {0x8380aau, 0x22u, 4u, 8u, 15u},
        {0x8380aeu, 0xa9u, 2u, 2u, 10u},
        {0x8380b0u, 0x20u, 3u, 6u, 15u},
        {0x8380b3u, 0xf0u, 2u, 2u, 15u},
        {0x8380b5u, 0xafu, 4u, 5u, 10u},
        {0x8380b9u, 0xd0u, 2u, 2u, 15u},
        {0x8380bbu, 0x20u, 3u, 6u, 15u},
        {0x8380beu, 0xd0u, 2u, 2u, 15u},
        {0x8380c0u, 0xafu, 4u, 5u, 10u},
        {0x8380c4u, 0xd0u, 2u, 2u, 15u},
        {0x8380c6u, 0x22u, 4u, 8u, 15u},
        {0x8380cau, 0x82u, 3u, 4u, 15u},
    };
    if (registers->e || registers->k != 0x83u ||
        registers->pc != (uint16_t)pc || pc < 0x838000u || pc > 0x8380cau ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted)
        return 0;
    unsigned selected;
    for (selected = 0; selected < sizeof boundaries / sizeof boundaries[0]; ++selected)
        if (boundaries[selected].pc == pc)
            break;
    const unsigned mode = registers->mf | ((unsigned)registers->xf << 1);
    if (selected == sizeof boundaries / sizeof boundaries[0] ||
        opcode != boundaries[selected].opcode ||
        !(boundaries[selected].modes & (1u << mode)))
        return 0;
    Lufia2CpuState state = {0};
    state.accumulator = registers->a;
    state.x = registers->x;
    state.y = registers->y;
    state.stack = registers->sp;
    state.direct_page = registers->dp;
    state.data_bank = registers->db;
    state.program_bank = registers->k;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    state.carry = registers->c;
    state.zero = registers->z;
    state.negative = registers->n;
    state.overflow = registers->v;
    state.decimal = registers->d;
    state.irq_disable = registers->i;
    FieldFrameBoundaryBus bus = {registers, UINT32_MAX, UINT32_MAX, 0u, false};
    if (opcode == 0xadu && !registers->mf) {
        const uint16_t source = pc == 0x83803fu ? 0x05acu : 0x05aeu;
        bus.word_low = ((uint32_t)registers->db << 16) | source;
        bus.word_high = bus.word_low + 1u;
    }
    const Lufia2Memory memory = {
        FieldFrameBoundaryRead, FieldFrameBoundaryWrite, &bus, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    for (unsigned operand = 1; operand < boundaries[selected].length; ++operand)
        (void)registers->read(registers->mem, pc + operand);
    unsigned cycles = boundaries[selected].cycles;
    if (opcode == 0x24u)
        cycles += (registers->dp & 0xffu) != 0u;
    if (opcode == 0x48u || opcode == 0x68u || opcode == 0xadu || opcode == 0x8du)
        cycles += !registers->mf;
    const Lufia2ExecutionResult result = Lufia2FieldFrameStep(&memory, &state, pc);
    if (opcode == 0xd0u || opcode == 0xf0u || opcode == 0x10u)
        cycles += result.pc != pc + 2u;
    registers->a = state.accumulator;
    registers->x = state.x;
    registers->y = state.y;
    registers->sp = state.stack;
    registers->k = state.program_bank;
    registers->db = state.data_bank;
    registers->pc = (uint16_t)result.pc;
    registers->mf = state.accumulator_is_8_bit;
    registers->xf = state.index_is_8_bit;
    registers->c = state.carry;
    registers->z = state.zero;
    registers->n = state.negative;
    registers->v = state.overflow;
    registers->d = state.decimal;
    registers->i = state.irq_disable;
    registers->cyclesUsed = (uint8_t)cycles;
    return (int)cycles;
}
