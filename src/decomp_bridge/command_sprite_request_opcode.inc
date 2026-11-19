extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunCommandSpriteRequest(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    const bool load_request = pc == 0x81cbe7u;
    if ((!load_request && pc != 0x81cbe9u) ||
        opcode != (load_request ? 0xa9u : 0x8fu) ||
        registers->k != 0x81u || registers->pc != (uint16_t)pc ||
        registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint16_t offset = (uint16_t)pc;
    const unsigned length = load_request ? 2u : 4u;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode ||
        (load_request ? cart->rom[offset + 1u] != 0xffu
                      : cart->rom[offset + 1u] != 0xf3u ||
                        cart->rom[offset + 2u] != 0x12u || cart->rom[offset + 3u] != 0u))
        return 0;

    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = load_request ? 2 : 5;
    const uint8_t low = registers->read(registers->mem, 0x810000u | registers->pc++);
    if (load_request) {
        registers->a = (registers->a & 0xff00u) | low;
        registers->n = (low & 0x80u) != 0;
        registers->z = low == 0u;
    } else {
        const uint8_t high = registers->read(registers->mem, 0x810000u | registers->pc++);
        const uint8_t bank = registers->read(registers->mem, 0x810000u | registers->pc++);
        const uint32_t address = ((uint32_t)bank << 16) | ((uint32_t)high << 8) | low;
        registers->write(registers->mem, address, (uint8_t)registers->a);
    }
    return registers->cyclesUsed;
}
