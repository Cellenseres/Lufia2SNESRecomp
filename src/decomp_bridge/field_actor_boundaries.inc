#include "lufia2/field_phase.h"

extern uint32_t g_interp816_cur_pc;

typedef struct FieldActorBoundaryBus {
    Interp816 *registers;
    uint32_t word_low;
    uint32_t word_high;
    uint8_t high_byte;
    bool high_pending;
} FieldActorBoundaryBus;

static uint8_t FieldActorBoundaryRead(void *context, uint32_t address) {
    FieldActorBoundaryBus *bus = context;
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

static void FieldActorBoundaryWrite(void *context, uint32_t address, uint8_t value) {
    FieldActorBoundaryBus *bus = context;
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunFieldActorBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint8_t opcode;
        uint8_t length;
        uint8_t cycles;
        uint8_t modes;
        uint16_t indexed_base;
    } boundaries[] = {
        {0x83b30du, 0x9cu, 3u, 4u, 10u, 0x0000u},
        {0x83b310u, 0x7bu, 1u, 2u, 15u, 0x0000u},
        {0x83b311u, 0x8fu, 4u, 5u, 10u, 0x0000u},
        {0x83b315u, 0x8fu, 4u, 5u, 10u, 0x0000u},
        {0x83b319u, 0xa9u, 2u, 2u, 10u, 0x0000u},
        {0x83b31bu, 0x8fu, 4u, 5u, 10u, 0x0000u},
        {0x83b31fu, 0x8fu, 4u, 5u, 10u, 0x0000u},
        {0x83b323u, 0x22u, 4u, 8u, 15u, 0x0000u},
        {0x83b327u, 0xafu, 4u, 5u, 10u, 0x0000u},
        {0x83b32bu, 0x29u, 2u, 2u, 10u, 0x0000u},
        {0x83b32du, 0xc9u, 2u, 2u, 10u, 0x0000u},
        {0x83b32fu, 0xd0u, 2u, 2u, 15u, 0x0000u},
        {0x83b331u, 0xa9u, 2u, 2u, 10u, 0x0000u},
        {0x83b333u, 0x80u, 2u, 3u, 15u, 0x0000u},
        {0x83b335u, 0xc9u, 2u, 2u, 10u, 0x0000u},
        {0x83b337u, 0xd0u, 2u, 2u, 15u, 0x0000u},
        {0x83b339u, 0xa9u, 2u, 2u, 10u, 0x0000u},
        {0x83b33bu, 0x80u, 2u, 3u, 15u, 0x0000u},
        {0x83b33du, 0xc9u, 2u, 2u, 10u, 0x0000u},
        {0x83b33fu, 0xd0u, 2u, 2u, 15u, 0x0000u},
        {0x83b341u, 0xa9u, 2u, 2u, 10u, 0x0000u},
        {0x83b343u, 0x8fu, 4u, 5u, 10u, 0x0000u},
        {0x83b347u, 0x20u, 3u, 6u, 15u, 0x0000u},
        {0x83b34au, 0xc2u, 2u, 3u, 15u, 0x0000u},
        {0x83b34cu, 0xa2u, 3u, 2u, 3u, 0x0000u},
        {0x83b34fu, 0xbdu, 3u, 4u, 10u, 0x081eu},
        {0x83b352u, 0xc9u, 2u, 2u, 10u, 0x0000u},
        {0x83b354u, 0xf0u, 2u, 2u, 15u, 0x0000u},
        {0x83b356u, 0x22u, 4u, 8u, 15u, 0x0000u},
        {0x83b35au, 0xbdu, 3u, 4u, 10u, 0x085eu},
        {0x83b35du, 0xcdu, 3u, 4u, 10u, 0x0000u},
        {0x83b360u, 0xd0u, 2u, 2u, 15u, 0x0000u},
        {0x83b362u, 0xa4u, 2u, 3u, 15u, 0x0000u},
        {0x83b364u, 0xbdu, 3u, 4u, 10u, 0x08deu},
        {0x83b367u, 0x99u, 3u, 5u, 10u, 0x05d2u},
        {0x83b36au, 0xbdu, 3u, 4u, 10u, 0x081eu},
        {0x83b36du, 0x29u, 2u, 2u, 10u, 0x0000u},
        {0x83b36fu, 0x9du, 3u, 5u, 10u, 0x081eu},
        {0x83b372u, 0x99u, 3u, 5u, 10u, 0x05fau},
        {0x83b375u, 0xdau, 1u, 3u, 15u, 0x0000u},
        {0x83b376u, 0xbdu, 3u, 4u, 10u, 0x089eu},
        {0x83b379u, 0xc9u, 2u, 2u, 10u, 0x0000u},
        {0x83b37bu, 0xd0u, 2u, 2u, 15u, 0x0000u},
        {0x83b37du, 0x9cu, 3u, 4u, 10u, 0x0000u},
        {0x83b380u, 0x9cu, 3u, 4u, 10u, 0x0000u},
        {0x83b383u, 0x9cu, 3u, 4u, 10u, 0x0000u},
        {0x83b386u, 0x9cu, 3u, 4u, 10u, 0x0000u},
        {0x83b389u, 0x8du, 3u, 4u, 10u, 0x0000u},
        {0x83b38cu, 0x8du, 3u, 4u, 10u, 0x0000u},
        {0x83b38fu, 0x9cu, 3u, 4u, 10u, 0x0000u},
        {0x83b392u, 0x80u, 2u, 3u, 15u, 0x0000u},
        {0x83b394u, 0x22u, 4u, 8u, 15u, 0x0000u},
        {0x83b398u, 0x22u, 4u, 8u, 15u, 0x0000u},
        {0x83b39cu, 0xfau, 1u, 4u, 15u, 0x0000u},
        {0x83b39du, 0xe6u, 2u, 5u, 10u, 0x0000u},
        {0x83b39fu, 0xcau, 1u, 2u, 15u, 0x0000u},
        {0x83b3a0u, 0x10u, 2u, 2u, 15u, 0x0000u},
        {0x83b3a2u, 0xadu, 3u, 4u, 10u, 0x0000u},
        {0x83b3a5u, 0xcdu, 3u, 4u, 10u, 0x0000u},
        {0x83b3a8u, 0xd0u, 2u, 2u, 15u, 0x0000u},
        {0x83b3aau, 0x22u, 4u, 8u, 15u, 0x0000u},
        {0x83b3aeu, 0xa6u, 2u, 3u, 15u, 0x0000u},
        {0x83b3b0u, 0xa9u, 2u, 2u, 10u, 0x0000u},
        {0x83b3b2u, 0x9du, 3u, 5u, 10u, 0x05fau},
        {0x83b3b5u, 0xadu, 3u, 4u, 10u, 0x0000u},
        {0x83b3b8u, 0x9du, 3u, 5u, 10u, 0x05d2u},
        {0x83b3bbu, 0xadu, 3u, 4u, 10u, 0x0000u},
        {0x83b3beu, 0x9du, 3u, 5u, 10u, 0x06bau},
        {0x83b3c1u, 0xadu, 3u, 4u, 10u, 0x0000u},
        {0x83b3c4u, 0x9du, 3u, 5u, 10u, 0x06e2u},
        {0x83b3c7u, 0xbdu, 3u, 4u, 10u, 0x0622u},
        {0x83b3cau, 0x29u, 2u, 2u, 10u, 0x0000u},
        {0x83b3ccu, 0x9du, 3u, 5u, 10u, 0x0622u},
    };
    if (registers->e || registers->k != 0x83u ||
        registers->pc != (uint16_t)pc || pc < 0x83b30du || pc > 0x83b3ccu ||
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
    FieldActorBoundaryBus bus = {registers, UINT32_MAX, UINT32_MAX, 0u, false};
    if (opcode == 0xa2u) {
        bus.word_low = pc + 1u;
        bus.word_high = pc + 2u;
    } else if ((opcode == 0xa4u || opcode == 0xa6u) && !registers->xf) {
        bus.word_low = (uint16_t)(registers->dp + 0xa7u);
        bus.word_high = (uint16_t)(bus.word_low + 1u);
    }
    const Lufia2Memory memory = {
        FieldActorBoundaryRead, FieldActorBoundaryWrite, &bus, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    if (opcode != 0xa2u)
        for (unsigned operand = 1; operand < boundaries[selected].length; ++operand)
            (void)registers->read(registers->mem, pc + operand);
    unsigned cycles = boundaries[selected].cycles;
    if (opcode == 0xa4u || opcode == 0xa6u || opcode == 0xe6u)
        cycles += (registers->dp & 0xffu) != 0u;
    if (opcode == 0xa2u || opcode == 0xa4u || opcode == 0xa6u ||
        opcode == 0xdau || opcode == 0xfau)
        cycles += !registers->xf;
    if (opcode == 0xbdu || opcode == 0x99u || opcode == 0x9du) {
        const uint16_t base = boundaries[selected].indexed_base;
        const uint16_t index = opcode == 0x99u ? registers->y : registers->x;
        const bool crossed = ((uint32_t)base >> 8) != ((base + (uint32_t)index) >> 8);
        cycles += crossed || (opcode != 0xbdu && !registers->xf);
    }
    const Lufia2ExecutionResult result = Lufia2FieldActorInstallStep(&memory, &state, pc);
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
