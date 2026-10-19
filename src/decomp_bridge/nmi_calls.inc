#include "lufia2/nmi_calls.h"

extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompNmiRunCallBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode,
    int interrupt_owner, int auto_quiescent) {
    if (!interrupt_owner || auto_quiescent || !g_snes ||
        registers->e || registers->k != 0x80u || registers->pc != (uint16_t)pc ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || registers->sp < 0x1f02u || registers->sp > 0x1ffcu)
        return 0;
    unsigned cycles;
    switch (pc) {
    case 0x808649u:
    case 0x808650u:
    case 0x808657u:
    case 0x80865eu:
    case 0x808661u:
        if (!g_snes->inNmi || opcode != 0x20u || registers->sp == 0x1ffcu)
            return 0;
        cycles = 6u;
        break;
    case 0x8086bfu:
        if (opcode != 0x40u)
            return 0;
        cycles = 7u;
        break;
#if defined(LUFIA2_HAVE_NATIVE_NMI_CALLBACK_BOUNDARIES)
    case 0x800067u:
    case 0x80006cu:
        if (opcode != 0x22u || registers->sp == 0x1ffcu)
            return 0;
        cycles = 8u;
        break;
    case 0x80006bu:
    case 0x800070u:
        if (opcode != 0x60u || registers->sp == 0x1ffcu)
            return 0;
        cycles = 6u;
        break;
#endif
    default:
        return 0;
    }
    Lufia2CpuState state = {0};
    state.stack = registers->sp;
    state.program_bank = registers->k;
    state.x = registers->x;
    state.y = registers->y;
    const Lufia2Memory memory = {
        registers->read, registers->write, registers->mem, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    Lufia2ExecutionResult result;
    if (opcode == 0x20u) {
        (void)registers->read(registers->mem, pc + 1u);
        (void)registers->read(registers->mem, pc + 2u);
        result = Lufia2NmiBeginChild(&memory, &state, pc);
#if defined(LUFIA2_HAVE_NATIVE_NMI_CALLBACK_BOUNDARIES)
    } else if (opcode == 0x22u) {
        result = Lufia2NmiInvokeCallback(&memory, &state, pc);
    } else if (opcode == 0x60u) {
        result = Lufia2NmiReturnCallback(&memory, &state, pc);
#endif
    } else {
        result = Lufia2NmiReturnInterrupt(&memory, &state);
        registers->x = state.x;
        registers->y = state.y;
        registers->c = state.carry;
        registers->z = state.zero;
        registers->n = state.negative;
        registers->v = state.overflow;
        registers->d = state.decimal;
        registers->i = state.irq_disable;
        registers->mf = state.accumulator_is_8_bit;
        registers->xf = state.index_is_8_bit;
    }
    registers->sp = state.stack;
    registers->k = state.program_bank;
    registers->pc = (uint16_t)result.pc;
    registers->cyclesUsed = (uint8_t)cycles;
    return (int)cycles;
}
