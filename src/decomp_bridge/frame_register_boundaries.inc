#include "lufia2/frame_wait.h"

extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunFrameRegisterBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    if (registers->e || registers->k != pc >> 16 ||
        registers->pc != (uint16_t)pc || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted)
        return 0;
    uint8_t expected_opcode;
    switch (pc) {
    case 0x8293c2u: case 0x868b48u: case 0x86974fu: case 0x84a5a8u:
        expected_opcode = 0x48u;
        break;
    case 0x8293c3u: case 0x868b49u: case 0x809378u: case 0x84a5a9u:
    case 0x868b57u:
        expected_opcode = 0x08u;
        break;
    case 0x8293c4u: case 0x868b4au: case 0x809379u: case 0x84a5aau:
    case 0x868b58u:
        expected_opcode = 0xe2u;
        break;
    case 0x8293ccu: case 0x868b52u: case 0x809381u: case 0x84a5b2u:
    case 0x868b6fu:
        expected_opcode = 0x28u;
        break;
    case 0x8293cdu: case 0x868b53u: case 0x869756u: case 0x84a5b3u:
        expected_opcode = 0x68u;
        break;
    case 0x868b55u: expected_opcode = 0x5au; break;
    case 0x868b56u: expected_opcode = 0xdau; break;
    case 0x868b5au: expected_opcode = 0xc2u; break;
    case 0x868b70u: expected_opcode = 0xfau; break;
    case 0x868b71u: expected_opcode = 0x7au; break;
    case 0x868b72u: expected_opcode = 0x6bu; break;
    default: return 0;
    }
    if (opcode != expected_opcode)
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
    const bool width_change = opcode == 0xe2u || opcode == 0xc2u;
    if (width_change)
        (void)registers->read(registers->mem, pc + 1u);
    unsigned cycles;
    switch (opcode) {
    case 0x48u: cycles = 3u + !registers->mf; break;
    case 0x68u: cycles = 4u + !registers->mf; break;
    case 0x5au: case 0xdau: cycles = 3u + !registers->xf; break;
    case 0x7au: case 0xfau: cycles = 4u + !registers->xf; break;
    case 0x28u: cycles = 4u; break;
    case 0x6bu: cycles = 6u; break;
    default: cycles = 3u; break;
    }
    const Lufia2ExecutionResult result = pc >= 0x868b55u && pc <= 0x868b72u
        ? Lufia2SpriteFrameRegisterStep(&memory, &state, pc)
        : Lufia2FrameWaitRegisterStep(&memory, &state, pc);
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
