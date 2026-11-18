extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunTurnDisplayReturn(Interp816 *registers, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != 0x6bu || registers->k != 0x85u || registers->pc != 0x9e04u ||
        registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffcu ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x29e04u;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize <= offset ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    (void)registers->read(registers->mem, 0x850000u | registers->pc++);
    g_interp816_cur_pc = 0x859e04u;
    const uint16_t low = registers->read(registers->mem, ++registers->sp);
    const uint16_t high = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)((low | (high << 8)) + 1u);
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->cyclesUsed = 6;
    return registers->cyclesUsed;
}
