#include "lufia2/battle_effect_slot_steps.h"

extern uint32_t g_interp816_cur_pc;

static uint8_t EffectCommandCallRead(void *context, uint32_t address) {
    Interp816 *registers = context;
    return registers->read(registers->mem, address);
}

static void EffectCommandCallWrite(void *context, uint32_t address, uint8_t value) {
    Interp816 *registers = context;
    registers->write(registers->mem, address, value);
}

static int Lufia2DecompRunEffectCommandStep(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    unsigned fetched, cycles;
    uint8_t operand;
    switch (pc) {
    case 0x818aa4u:
    case 0x818b15u:
        if (opcode != 0xfcu)
            return 0;
        fetched = 3u;
        cycles = 8u;
        operand = 0x5cu;
        break;
    case 0x818aa7u:
    case 0x818b18u:
        if (opcode != 0x80u)
            return 0;
        fetched = 2u;
        cycles = 3u;
        operand = pc == 0x818aa7u ? 0xecu : 0xeeu;
        break;
    case 0x818aa9u:
    case 0x818b1au:
        if (opcode != 0xabu)
            return 0;
        fetched = 1u;
        cycles = 4u;
        operand = 0u;
        break;
    default:
        return 0;
    }
    if (registers->e || registers->k != 0x81u ||
        registers->pc != (uint16_t)pc || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart || g_snes->cart->type != CART_LOROM)
        return 0;
    /* Hardware words use the SDK atomic bus; stay on the ROM command table. */
    if (opcode == 0xfcu && (registers->x > 0x1feu || (registers->x & 1u)))
        return 0;
    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x8000u + (pc & 0x7fffu);
    if (!cart->rom || cart->romSize < offset + fetched ||
        cart->rom[offset] != opcode ||
        (fetched >= 2u && cart->rom[offset + 1u] != operand) ||
        (fetched == 3u && cart->rom[offset + 2u] != 0x8cu) ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return 0;

    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    for (unsigned byte = 1u; byte < fetched; ++byte)
        (void)registers->read(registers->mem, pc + byte);
    Lufia2CpuState state = {0};
    state.stack = registers->sp;
    state.program_bank = registers->k;
    state.x = registers->x;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    const Lufia2Memory memory = {EffectCommandCallRead, EffectCommandCallWrite, registers, NULL, NULL};
    const Lufia2ExecutionResult result = opcode == 0xfcu
        ? Lufia2BattleEffectCallCommandStep(&memory, &state, pc)
        : Lufia2BattleEffectResumeCommandStep(&memory, &state, pc);
    registers->sp = state.stack;
    registers->pc = (uint16_t)result.pc;
    if (opcode == 0xabu) {
        registers->db = state.data_bank;
        registers->n = state.negative;
        registers->z = state.zero;
    }
    registers->cyclesUsed = cycles;
    return (int)cycles;
}
