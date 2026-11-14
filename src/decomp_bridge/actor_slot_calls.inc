#include "lufia2/actor.h"

extern uint32_t g_interp816_cur_pc;

static void ActorSlotCallWrite(void *context, uint32_t address, uint8_t value) {
    Interp816 *registers = context;
    registers->write(registers->mem, address, value);
}

static int Lufia2DecompRunActorSlotCall(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint16_t target;
    switch (pc) {
    case 0x83bbb9u: target = 0xbbf3u; break;
    case 0x83bbc9u: target = 0xc7f8u; break;
    case 0x83bbccu: target = 0xd508u; break;
    default: return 0;
    }
    if (opcode != 0x20u || registers->e || registers->k != 0x83u ||
        registers->pc != (uint16_t)pc || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart || g_snes->cart->type != CART_LOROM)
        return 0;
    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x18000u + (pc & 0x7fffu);
    if (!cart->rom || cart->romSize < offset + 3u ||
        cart->rom[offset] != opcode ||
        cart->rom[offset + 1u] != (uint8_t)target ||
        cart->rom[offset + 2u] != (uint8_t)(target >> 8))
        return 0;

    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    (void)registers->read(registers->mem, pc + 1u);
    (void)registers->read(registers->mem, pc + 2u);
    Lufia2CpuState state = {0};
    state.stack = registers->sp;
    state.program_bank = registers->k;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    const Lufia2Memory memory = {NULL, ActorSlotCallWrite, registers, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2ActorCallSlotService(&memory, &state, pc);
    registers->sp = state.stack;
    registers->pc = (uint16_t)result.pc;
    registers->cyclesUsed = 6u;
    return 6;
}
