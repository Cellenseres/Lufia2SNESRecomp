extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunBattleStatusBranch(Interp816 *registers, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != 0x80u || registers->k != 0x81u || registers->pc != 0xe535u ||
        registers->e || registers->mf || registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize <= 0xe536u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[0xe535u] != opcode || cart->rom[0xe536u] != 0xf6u)
        return 0;

    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    g_interp816_cur_pc = 0x81e535u;
    registers->cyclesUsed = 3;
    const uint8_t displacement = registers->read(registers->mem, 0x810000u | registers->pc++);
    registers->pc = (uint16_t)(registers->pc + displacement - 256u);
    return registers->cyclesUsed;
}
