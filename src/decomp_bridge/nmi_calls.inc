#include "lufia2/nmi_calls.h"
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
#include "frame_returns.h"
#endif

extern uint32_t g_interp816_cur_pc;

#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
static int Lufia2DecompRunNmiTilemapReadback(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != 0xa5u || !registers->mf || registers->xf || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = pc & 0x7fffu;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize <= offset + 1u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x800000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 3;
    const uint8_t operand = registers->read(registers->mem, 0x800000u | registers->pc++);
    registers->cyclesUsed += (registers->dp & 0xffu) != 0u;
    const uint8_t value = registers->read(registers->mem, (uint16_t)(registers->dp + operand));
    registers->a = (registers->a & 0xff00u) | value;
    registers->n = (value & 0x80u) != 0u;
    registers->z = value == 0u;
    return registers->cyclesUsed;
}
#endif

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
         pc == 0x80880bu || pc == 0x808702u || pc == 0x80882du) &&
        (observed ||
         !Lufia2DecompFrameRangeQuiet(pc, pc + (opcode == 0x20u ? 3u :
             opcode == 0xa5u ? 2u : 1u))))
        return 0;
#else
    (void)observed;
#endif
    unsigned cycles;
    switch (pc) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_CHILD_BOUNDARIES)
    case 0x80880bu:
        if (!g_snes->inNmi || g_snes->inIrq)
            return 0;
        return Lufia2DecompRunNmiTilemapReadback(registers, pc, opcode);
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
