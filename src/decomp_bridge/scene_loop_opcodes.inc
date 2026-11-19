extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunSceneLoopOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    const bool decrement = pc == 0x83a6c4u;
    const bool store_slot = pc == 0x83a68cu;
    if ((!decrement && !store_slot && pc != 0x83a6c5u) ||
        opcode != (decrement ? 0xcau : store_slot ? 0x86u : 0x10u) ||
        (store_slot && registers->dp != 0u) ||
        registers->k != 0x83u || registers->pc != (uint16_t)pc ||
        registers->e || !registers->mf || !registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x18000u + (pc & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom ||
        cart->romSize <= offset + (decrement ? 0u : 1u) ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode ||
        (!decrement && cart->rom[offset + 1u] != (store_slot ? 0xa7u : 0xc5u)))
        return 0;

    (void)registers->read(registers->mem, 0x830000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 2;
    if (decrement) {
        registers->x = (uint8_t)(registers->x - 1u);
        registers->z = registers->x == 0;
        registers->n = (registers->x & 0x80u) != 0;
    } else if (store_slot) {
        const uint8_t address = registers->read(registers->mem, 0x830000u | registers->pc++);
        registers->cyclesUsed = 3;
        registers->write(registers->mem, address, (uint8_t)registers->x);
    } else {
        const uint8_t displacement = registers->read(registers->mem, 0x830000u | registers->pc++);
        if (!registers->n) {
            registers->pc = (uint16_t)(registers->pc + displacement - 256u);
            ++registers->cyclesUsed;
        }
    }
    return registers->cyclesUsed;
}
