#include "lufia2/menu_input_phase.h"

extern uint32_t g_interp816_cur_pc;

typedef struct MenuInputBoundaryBus {
    Interp816 *registers;
    uint32_t word_low;
    uint32_t word_high;
    uint8_t high_byte;
    bool high_pending;
} MenuInputBoundaryBus;

static uint8_t MenuInputBoundaryRead(void *context, uint32_t address) {
    MenuInputBoundaryBus *bus = context;
    Interp816 *registers = bus->registers;
    if (bus->high_pending && address == bus->word_high) {
        bus->high_pending = false;
        return bus->high_byte;
    }
    if (address == bus->word_low && registers->read_word) {
        uint16_t value;
        if (registers->read_word(registers->mem, address, bus->word_high, &value)) {
            bus->high_byte = (uint8_t)(value >> 8);
            bus->high_pending = true;
            return (uint8_t)value;
        }
    }
    return registers->read(registers->mem, address);
}

static void MenuInputBoundaryWrite(void *context, uint32_t address, uint8_t value) {
    MenuInputBoundaryBus *bus = context;
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunMenuInputBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint8_t opcode;
        uint8_t length;
        uint8_t cycles;
    } boundaries[] = {
        {0x828b08u, 0xc9u, 2u, 2u},
        {0x828b0au, 0xd0u, 2u, 2u},
        {0x828b0cu, 0xaeu, 3u, 4u},
        {0x828b0fu, 0xcau, 1u, 2u},
        {0x828b10u, 0xcau, 1u, 2u},
        {0x828b11u, 0xcau, 1u, 2u},
        {0x828b12u, 0xcau, 1u, 2u},
        {0x828b13u, 0xcau, 1u, 2u},
        {0x828b14u, 0xbdu, 3u, 4u},
        {0x828b17u, 0x29u, 2u, 2u},
        {0x828b19u, 0x9du, 3u, 5u},
        {0x828b1cu, 0x22u, 4u, 8u},
        {0x828b20u, 0xb0u, 2u, 2u},
        {0x828b22u, 0x20u, 3u, 6u},
        {0x828b25u, 0xb0u, 2u, 2u},
        {0x828b27u, 0xc9u, 2u, 2u},
        {0x828b29u, 0xd0u, 2u, 2u},
        {0x828b2bu, 0xbdu, 3u, 4u},
        {0x828b2eu, 0x0au, 1u, 2u},
        {0x828b2fu, 0xb0u, 2u, 2u},
        {0x828b31u, 0x0au, 1u, 2u},
        {0x828b32u, 0xb0u, 2u, 2u},
        {0x828b34u, 0x80u, 2u, 3u},
        {0x828b36u, 0x20u, 3u, 6u},
        {0x828b39u, 0x20u, 3u, 6u},
        {0x828b3cu, 0x22u, 4u, 8u},
        {0x828b40u, 0x80u, 2u, 3u},
        {0x828b42u, 0xa9u, 2u, 2u},
        {0x828b44u, 0x60u, 1u, 6u},
        {0x828b45u, 0xa9u, 2u, 2u},
        {0x828b47u, 0x60u, 1u, 6u},
        {0x828b48u, 0xa9u, 2u, 2u},
        {0x828b4au, 0x60u, 1u, 6u},
    };
    if (registers->e || registers->k != 0x82u ||
        registers->pc != (uint16_t)pc || pc < 0x828b08u || pc > 0x828b4au ||
        !registers->mf || registers->xf || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted)
        return 0;
    unsigned selected;
    for (selected = 0; selected < sizeof boundaries / sizeof boundaries[0]; ++selected)
        if (boundaries[selected].pc == pc)
            break;
    if (selected == sizeof boundaries / sizeof boundaries[0] ||
        opcode != boundaries[selected].opcode)
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
    MenuInputBoundaryBus bus = {registers, UINT32_MAX, UINT32_MAX, 0u, false};
    if (opcode == 0xaeu) {
        bus.word_low = ((uint32_t)registers->db << 16) | 0x14a9u;
        bus.word_high = bus.word_low + 1u;
    }
    const Lufia2Memory memory = {
        MenuInputBoundaryRead, MenuInputBoundaryWrite, &bus, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    for (unsigned operand = 1; operand < boundaries[selected].length; ++operand)
        (void)registers->read(registers->mem, pc + operand);
    unsigned cycles = boundaries[selected].cycles;
    if (opcode == 0xaeu)
        cycles += 1u;
    if (opcode == 0xbdu)
        cycles += (0x14c1u >> 8) != ((0x14c1u + registers->x) >> 8);
    if (opcode == 0x9du)
        cycles += 1u;
    const Lufia2ExecutionResult result = Lufia2MenuInputStep(&memory, &state, pc);
    if (opcode == 0xb0u || opcode == 0xd0u)
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
