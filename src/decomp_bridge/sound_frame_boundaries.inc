#include "lufia2/sound_phase.h"

extern uint32_t g_interp816_cur_pc;

typedef struct SoundFrameBoundaryBus {
    Interp816 *registers;
    uint32_t word_low;
    uint32_t word_high;
    uint8_t high_byte;
    bool high_pending;
} SoundFrameBoundaryBus;

static uint8_t SoundFrameBoundaryRead(void *context, uint32_t address) {
    SoundFrameBoundaryBus *bus = context;
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

static void SoundFrameBoundaryWrite(void *context, uint32_t address, uint8_t value) {
    SoundFrameBoundaryBus *bus = context;
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunSoundFrameBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint8_t opcode;
        uint8_t length;
        uint8_t cycles;
        uint8_t modes;
    } boundaries[] = {
        {0x8098ecu, 0xb7u, 2u, 6u, 2u},
        {0x8098eeu, 0x8du, 3u, 4u, 2u},
        {0x8098f1u, 0x8au, 1u, 2u, 2u},
        {0x8098f2u, 0x8du, 3u, 4u, 2u},
        {0x8098f5u, 0x09u, 2u, 2u, 2u},
        {0x8098f7u, 0xcdu, 3u, 4u, 2u},
        {0x8098fau, 0xd0u, 2u, 2u, 2u},
        {0x8098fcu, 0xc8u, 1u, 2u, 2u},
        {0x8098fdu, 0xd0u, 2u, 2u, 2u},
        {0x8098ffu, 0xe6u, 2u, 5u, 2u},
        {0x809901u, 0xa0u, 3u, 2u, 2u},
        {0x809904u, 0xcau, 1u, 2u, 2u},
        {0x809905u, 0xd0u, 2u, 2u, 2u},
        {0x809907u, 0xfau, 1u, 4u, 2u},
        {0x809908u, 0xa9u, 2u, 2u, 2u},
        {0x80990au, 0x8du, 3u, 4u, 2u},
        {0x80990du, 0x20u, 3u, 6u, 2u},
        {0x809910u, 0x60u, 1u, 6u, 15u},
        {0x809a0au, 0x08u, 1u, 3u, 15u},
        {0x809a0bu, 0xe2u, 2u, 3u, 15u},
        {0x809a0du, 0xa9u, 2u, 2u, 10u},
        {0x809a0fu, 0xcfu, 4u, 5u, 10u},
        {0x809a13u, 0xd0u, 2u, 2u, 15u},
        {0x809a15u, 0xa9u, 2u, 2u, 10u},
        {0x809a17u, 0xcfu, 4u, 5u, 10u},
        {0x809a1bu, 0xd0u, 2u, 2u, 15u},
        {0x809a1du, 0x7bu, 1u, 2u, 15u},
        {0x809a1eu, 0x8fu, 4u, 5u, 10u},
        {0x809a22u, 0x8fu, 4u, 5u, 10u},
        {0x809a26u, 0xafu, 4u, 5u, 10u},
        {0x809a2au, 0xd0u, 2u, 2u, 15u},
        {0x809a2cu, 0xafu, 4u, 5u, 10u},
        {0x809a30u, 0xd0u, 2u, 2u, 15u},
        {0x809a32u, 0x28u, 1u, 4u, 15u},
        {0x809a33u, 0x60u, 1u, 6u, 15u},
    };
    if (registers->e || registers->k != 0x80u ||
        registers->pc != (uint16_t)pc || pc < 0x8098ecu || pc > 0x809a33u ||
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
    SoundFrameBoundaryBus bus = {registers, UINT32_MAX, UINT32_MAX, 0u, false};
    if (opcode == 0xb7u) {
        bus.word_low = (uint16_t)(registers->dp + 0x5du);
        bus.word_high = (uint16_t)(bus.word_low + 1u);
    } else if (opcode == 0xa0u) {
        bus.word_low = pc + 1u;
        bus.word_high = pc + 2u;
    }
    const Lufia2Memory memory = {
        SoundFrameBoundaryRead, SoundFrameBoundaryWrite, &bus, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    if (opcode != 0xa0u)
        for (unsigned operand = 1; operand < boundaries[selected].length; ++operand)
            (void)registers->read(registers->mem, pc + operand);
    unsigned cycles = boundaries[selected].cycles;
    if (opcode == 0xb7u || opcode == 0xe6u)
        cycles += (registers->dp & 0xffu) != 0u;
    if (opcode == 0xfau || opcode == 0xa0u)
        cycles += !registers->xf;
    const Lufia2ExecutionResult result = pc < 0x809a0au
        ? Lufia2SoundHeaderStep(&memory, &state, pc)
        : Lufia2SoundReplyStep(&memory, &state, pc);
    if (opcode == 0xd0u)
        cycles += result.pc != pc + 2u;
    registers->a = state.accumulator;
    registers->x = state.x;
    registers->y = state.y;
    registers->sp = state.stack;
    registers->k = state.program_bank;
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
