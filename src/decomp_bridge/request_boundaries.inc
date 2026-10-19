#include "lufia2/sound_phase.h"
#include "lufia2/menu_input_phase.h"

extern uint32_t g_interp816_cur_pc;

typedef struct RequestBoundaryBus {
    Interp816 *registers;
    uint32_t word_low;
    uint32_t word_high;
    uint8_t high_byte;
    bool high_pending;
} RequestBoundaryBus;

static uint8_t RequestBoundaryRead(void *context, uint32_t address) {
    RequestBoundaryBus *bus = context;
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

static void RequestBoundaryWrite(void *context, uint32_t address, uint8_t value) {
    RequestBoundaryBus *bus = context;
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunRequestBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint8_t opcode;
        uint8_t length;
        uint8_t cycles;
        uint8_t modes;
    } boundaries[] = {
        {0x80953bu, 0x48u, 1u, 3u, 15u},
        {0x80953cu, 0x08u, 1u, 3u, 15u},
        {0x80953du, 0xe2u, 2u, 3u, 15u},
        {0x80953fu, 0xc2u, 2u, 3u, 15u},
        {0x809541u, 0xc9u, 2u, 2u, 10u},
        {0x809543u, 0xb0u, 2u, 2u, 15u},
        {0x809545u, 0x8fu, 4u, 5u, 10u},
        {0x809549u, 0xa9u, 2u, 2u, 10u},
        {0x80954bu, 0x20u, 3u, 6u, 15u},
        {0x80954eu, 0x20u, 3u, 6u, 15u},
        {0x809551u, 0x28u, 1u, 4u, 15u},
        {0x809552u, 0x68u, 1u, 4u, 15u},
        {0x809553u, 0x6bu, 1u, 6u, 15u},
        {0x8099fdu, 0x08u, 1u, 3u, 15u},
        {0x8099feu, 0xe2u, 2u, 3u, 15u},
        {0x809a00u, 0x8fu, 4u, 5u, 10u},
        {0x809a04u, 0x8fu, 4u, 5u, 10u},
        {0x809a08u, 0x28u, 1u, 4u, 15u},
        {0x809a09u, 0x60u, 1u, 6u, 15u},
        {0x829313u, 0xadu, 3u, 4u, 2u},
        {0x829316u, 0xf0u, 2u, 2u, 2u},
        {0x829318u, 0xa9u, 2u, 2u, 2u},
        {0x82931au, 0x8du, 3u, 4u, 2u},
        {0x82931du, 0xa9u, 2u, 2u, 2u},
        {0x82931fu, 0x85u, 2u, 3u, 2u},
        {0x829321u, 0xa0u, 3u, 2u, 2u},
        {0x829324u, 0xa2u, 3u, 2u, 2u},
        {0x829327u, 0x22u, 4u, 8u, 2u},
        {0x82932bu, 0xa9u, 2u, 2u, 2u},
        {0x82932du, 0x04u, 2u, 5u, 2u},
        {0x82932fu, 0x60u, 1u, 6u, 2u},
    };
    if (registers->e || registers->k != (uint8_t)(pc >> 16) ||
        registers->pc != (uint16_t)pc || pc < 0x80953bu || pc > 0x82932fu ||
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
    RequestBoundaryBus bus = {registers, UINT32_MAX, UINT32_MAX, 0u, false};
    if (opcode == 0xa0u || opcode == 0xa2u) {
        bus.word_low = pc + 1u;
        bus.word_high = pc + 2u;
    }
    const Lufia2Memory memory = {
        RequestBoundaryRead, RequestBoundaryWrite, &bus, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    if (opcode != 0xa0u && opcode != 0xa2u)
        for (unsigned operand = 1; operand < boundaries[selected].length; ++operand)
            (void)registers->read(registers->mem, pc + operand);
    unsigned cycles = boundaries[selected].cycles;
    if (opcode == 0x85u || opcode == 0x04u)
        cycles += (registers->dp & 0xffu) != 0u;
    if (opcode == 0xa0u || opcode == 0xa2u)
        cycles += !registers->xf;
    if (opcode == 0x48u || opcode == 0x68u)
        cycles += !registers->mf;
    const Lufia2ExecutionResult result = pc < 0x820000u
        ? Lufia2SoundCommandStep(&memory, &state, pc)
        : Lufia2MenuWindowStep(&memory, &state, pc);
    if (opcode == 0xb0u || opcode == 0xf0u)
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
