extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunBattleWidthOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    const bool narrow = pc == 0x818b49u;
    if ((!narrow && pc != 0x818b43u && pc != 0x81e320u) ||
        opcode != (narrow ? 0xe2u : 0xc2u) ||
        registers->k != 0x81u || registers->pc != (uint16_t)pc ||
        registers->e || registers->mf == narrow || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint16_t offset = (uint16_t)pc;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize <= (uint32_t)offset + 1u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode || cart->rom[offset + 1u] != 0x20u)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 3;
    const uint8_t mask = registers->read(registers->mem, 0x810000u | registers->pc++);
    if (mask == 0x20u) {
        registers->mf = narrow;
    } else {
        const uint8_t flags = interp816_getFlags(registers);
        interp816_setFlags(registers, narrow ? flags | mask : flags & ~mask);
    }
    return registers->cyclesUsed;
}
