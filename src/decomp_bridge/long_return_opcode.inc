extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunLongReturn(
    Interp816 *registers, uint32_t pc, uint8_t opcode, bool allow_index_8_bit) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != 0x6bu || registers->k != (pc >> 16) || registers->pc != (uint16_t)pc ||
        registers->e || !registers->mf || (registers->xf && !allow_index_8_bit) ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffcu ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = ((pc >> 16) & 0x7fu) * 0x8000u + (pc & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize <= offset ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    const uint16_t low = registers->read(registers->mem, ++registers->sp);
    const uint16_t high = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)((low | (high << 8)) + 1u);
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->cyclesUsed = 6;
    return registers->cyclesUsed;
}
