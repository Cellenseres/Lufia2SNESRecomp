#include "lufia2/battle_command_steps.h"

extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunCommandTileCall(Interp816 *registers, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != 0x20u || registers->k != 0x81u || registers->pc != 0xcbafu ||
        registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        registers->sp < 0x1f00u || registers->sp > 0x1fffu ||
        !g_snes || !g_snes->cart)
        return 0;
    const Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < 0xcbb2u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[0xcbafu] != opcode ||
        cart->rom[0xcbb0u] != 0x58u || cart->rom[0xcbb1u] != 0xbeu)
        return 0;

    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    g_interp816_cur_pc = 0x81cbafu;
    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    (void)registers->read(registers->mem, 0x810000u | registers->pc++);

    const Lufia2Memory memory = {NULL, registers->write, registers->mem, NULL, NULL};
    Lufia2CpuState state = {0};
    state.program_bank = registers->k;
    state.stack = registers->sp;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    const Lufia2ExecutionResult result = Lufia2BattleCommandCallStep(&memory, &state, 0x81cbafu);
    registers->sp = state.stack;
    registers->pc = (uint16_t)result.pc;
    registers->cyclesUsed = 6;
    return registers->cyclesUsed;
}
