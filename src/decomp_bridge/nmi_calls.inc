#include "lufia2/nmi_calls.h"
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
#include "frame_returns.h"
#endif

extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompNmiRunCallBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode,
    int interrupt_owner, int auto_quiescent, int observed) {
    if (!interrupt_owner || auto_quiescent || !g_snes ||
        registers->e || registers->k != 0x80u || registers->pc != (uint16_t)pc ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || registers->sp < 0x1f02u || registers->sp > 0x1ffcu)
        return 0;
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
    if ((pc == 0x8087ccu || pc == 0x8087f0u || pc == 0x808808u ||
         pc == 0x808702u || pc == 0x80882du) &&
        (observed ||
         !Lufia2DecompFrameRangeQuiet(pc, pc + (opcode == 0x20u ? 3u : 1u))))
        return 0;
#else
    (void)observed;
#endif
    unsigned cycles;
    switch (pc) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
    case 0x8087ccu: case 0x808808u:
        if (!g_snes->inNmi || g_snes->inIrq)
            return 0;
        return Lufia2DecompRunDirectCall(registers, pc, opcode,
            pc == 0x808808u ? 0x80882eu : 0x80884fu, false);
    case 0x8087f0u:
#endif
    case 0x808649u:
    case 0x808650u:
    case 0x808657u:
    case 0x80865eu:
    case 0x808661u:
        if (!g_snes->inNmi || opcode != 0x20u || registers->sp == 0x1ffcu)
            return 0;
        cycles = 6u;
        break;
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
    case 0x808702u: case 0x80882du:
        if (!g_snes->inNmi || opcode != 0x60u)
            return 0;
        cycles = 6u;
        break;
#endif
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
    } else if (opcode == 0x60u && (pc == 0x80006bu || pc == 0x800070u)) {
        result = Lufia2NmiReturnCallback(&memory, &state, pc);
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
    } else if (opcode == 0x60u) {
        result = Lufia2NmiReturnChild(&memory, &state, pc);
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
