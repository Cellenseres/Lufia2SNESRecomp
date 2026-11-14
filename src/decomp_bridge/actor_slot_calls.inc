#include "lufia2/actor.h"

extern uint32_t g_interp816_cur_pc;

static void ActorSlotCallWrite(void *context, uint32_t address, uint8_t value) {
    Interp816 *registers = context;
    registers->write(registers->mem, address, value);
}

static uint8_t ActorSlotPullReturnByte(Interp816 *registers) {
    registers->sp = (uint16_t)(registers->sp + 1u);
    return registers->read(registers->mem, registers->sp);
}

static int Lufia2DecompRunActorSlotCall(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint16_t target = 0u;
    unsigned fetched = 3u, cycles = 6u;
    uint8_t expected = 0x20u;
    switch (pc) {
    case 0x83bbb9u: target = 0xbbf3u; break;
    case 0x83bbc9u: target = 0xc7f8u; break;
    case 0x83bbccu: target = 0xd508u; break;
    case 0x83bbbcu:
        expected = 0x80u;
        target = 0x0eu;
        fetched = 2u;
        cycles = 3u;
        break;
    case 0x83bbf2u:
        expected = 0x6bu;
        fetched = 1u;
        break;
    default: return 0;
    }
#if defined(SNESRECOMP_INTERP_PROFILE)
    if (expected != 0x20u)
        return 0;
#endif
    if (opcode != expected || registers->e || registers->k != 0x83u ||
        registers->pc != (uint16_t)pc || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart || g_snes->cart->type != CART_LOROM)
        return 0;
    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x18000u + (pc & 0x7fffu);
    if (!cart->rom || cart->romSize < offset + fetched ||
        cart->rom[offset] != opcode ||
        (fetched >= 2u && cart->rom[offset + 1u] != (uint8_t)target) ||
        (fetched == 3u && cart->rom[offset + 2u] != (uint8_t)(target >> 8)) ||
        (expected != 0x20u &&
         (cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)))
        return 0;

    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    for (unsigned byte = 1u; byte < fetched; ++byte)
        (void)registers->read(registers->mem, pc + byte);
    if (expected == 0x6bu) {
        const uint8_t low = ActorSlotPullReturnByte(registers);
        const uint8_t high = ActorSlotPullReturnByte(registers);
        registers->k = ActorSlotPullReturnByte(registers);
        registers->pc = (uint16_t)(((uint16_t)high << 8u | low) + 1u);
    } else if (expected == 0x80u) {
        registers->pc = 0xbbccu;
    } else {
        Lufia2CpuState state = {0};
        state.stack = registers->sp;
        state.program_bank = registers->k;
        state.accumulator_is_8_bit = registers->mf;
        state.index_is_8_bit = registers->xf;
        const Lufia2Memory memory = {NULL, ActorSlotCallWrite, registers, NULL, NULL};
        const Lufia2ExecutionResult result = Lufia2ActorCallSlotService(&memory, &state, pc);
        registers->sp = state.stack;
        registers->pc = (uint16_t)result.pc;
    }
    registers->cyclesUsed = cycles;
    return (int)cycles;
}
