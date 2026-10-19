#include "lufia2/select_phase.h"

extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunSelectTransitionBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint8_t opcode;
        uint8_t length;
        uint8_t cycles;
    } boundaries[] = {
        {0x8682c8u, 0xa9u, 2u, 2u},
        {0x8682cau, 0x22u, 4u, 8u},
        {0x8682ceu, 0x22u, 4u, 8u},
        {0x8682d2u, 0xadu, 3u, 4u},
        {0x8682d5u, 0x89u, 2u, 2u},
        {0x8682d7u, 0xd0u, 2u, 2u},
        {0x8682d9u, 0xadu, 3u, 4u},
        {0x8682dcu, 0x89u, 2u, 2u},
        {0x8682deu, 0xd0u, 2u, 2u},
        {0x8682e0u, 0x20u, 3u, 6u},
        {0x8682e3u, 0xb0u, 2u, 2u},
        {0x8682e5u, 0x20u, 3u, 6u},
        {0x8682e8u, 0x20u, 3u, 6u},
        {0x8682ebu, 0x20u, 3u, 6u},
        {0x8682eeu, 0x22u, 4u, 8u},
        {0x8682f2u, 0x20u, 3u, 6u},
        {0x8682f5u, 0xa9u, 2u, 2u},
        {0x8682f7u, 0x85u, 2u, 3u},
        {0x8682f9u, 0x20u, 3u, 6u},
        {0x8682fcu, 0x80u, 2u, 3u},
        {0x8682feu, 0x20u, 3u, 6u},
        {0x868301u, 0x18u, 1u, 2u},
        {0x868302u, 0x60u, 1u, 6u},
        {0x868303u, 0x38u, 1u, 2u},
        {0x868304u, 0x60u, 1u, 6u},
        {0x868305u, 0x9cu, 3u, 4u},
        {0x868308u, 0xa9u, 2u, 2u},
        {0x86830au, 0x8du, 3u, 4u},
        {0x86830du, 0x80u, 2u, 3u},
        {0x86830fu, 0x9cu, 3u, 4u},
        {0x868312u, 0xa9u, 2u, 2u},
        {0x868314u, 0x8du, 3u, 4u},
        {0x868317u, 0x20u, 3u, 6u},
        {0x86831au, 0x20u, 3u, 6u},
        {0x86831du, 0xb0u, 2u, 2u},
        {0x86831fu, 0x20u, 3u, 6u},
        {0x868322u, 0x20u, 3u, 6u},
        {0x868325u, 0x22u, 4u, 8u},
        {0x868329u, 0x20u, 3u, 6u},
        {0x86832cu, 0xa9u, 2u, 2u},
        {0x86832eu, 0x85u, 2u, 3u},
        {0x868330u, 0x20u, 3u, 6u},
        {0x868333u, 0xadu, 3u, 4u},
        {0x868336u, 0xd0u, 2u, 2u},
        {0x868338u, 0x60u, 1u, 6u},
        {0x868339u, 0xdau, 1u, 3u},
        {0x86833au, 0x22u, 4u, 8u},
        {0x86833eu, 0xadu, 3u, 4u},
        {0x868341u, 0x89u, 2u, 2u},
        {0x868343u, 0xd0u, 2u, 2u},
        {0x868345u, 0xadu, 3u, 4u},
        {0x868348u, 0x89u, 2u, 2u},
        {0x86834au, 0xd0u, 2u, 2u},
        {0x86834cu, 0x20u, 3u, 6u},
        {0x86834fu, 0x20u, 3u, 6u},
        {0x868352u, 0xfau, 1u, 4u},
        {0x868353u, 0xcau, 1u, 2u},
        {0x868354u, 0xd0u, 2u, 2u},
        {0x868356u, 0x18u, 1u, 2u},
        {0x868357u, 0x60u, 1u, 6u},
        {0x868358u, 0xfau, 1u, 4u},
        {0x868359u, 0x38u, 1u, 2u},
        {0x86835au, 0x60u, 1u, 6u},
    };
    if (registers->e || registers->k != 0x86u ||
        registers->pc != (uint16_t)pc || pc < 0x8682c8u || pc > 0x86835au ||
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
    const Lufia2Memory memory = {
        registers->read, registers->write, registers->mem, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    for (unsigned operand = 1; operand < boundaries[selected].length; ++operand)
        (void)registers->read(registers->mem, pc + operand);
    unsigned cycles = boundaries[selected].cycles;
    if (opcode == 0x85u)
        cycles += (registers->dp & 0xffu) != 0u;
    if (opcode == 0xfau || opcode == 0xdau)
        cycles += !registers->xf;
    const Lufia2ExecutionResult result = Lufia2SelectTransitionStep(&memory, &state, pc);
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
