extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunReturn(
    Interp816 *registers, uint32_t pc, uint8_t opcode, bool long_return,
    bool accumulator_8_bit, bool allow_index_8_bit) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != (long_return ? 0x6bu : 0x60u) || registers->k != (pc >> 16) || registers->pc != (uint16_t)pc ||
        registers->e || registers->mf != accumulator_8_bit || (registers->xf && !allow_index_8_bit) ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        registers->sp < 0x1f00u || registers->sp > (long_return ? 0x1ffcu : 0x1ffdu) ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || !cart->rom ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return 0;
    const uint16_t address = (uint16_t)pc;
    if (address >= 0x8000u) {
        const uint32_t offset = ((pc >> 16) & 0x7fu) * 0x8000u + (address & 0x7fffu);
        if (cart->romSize <= offset || cart->rom[offset] != opcode)
            return 0;
    } else {
        const CpuState *cpu = registers->mem;
        const unsigned bank = pc >> 16;
        if (long_return || address >= 0x2000u || bank < 0x80u || bank > 0xbfu ||
            !cpu || !cpu->ram || cpu->ram[address] != opcode)
            return 0;
    }

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 6;
    const uint16_t low = registers->read(registers->mem, ++registers->sp);
    const uint16_t high = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)((low | (high << 8)) + 1u);
    if (long_return)
        registers->k = registers->read(registers->mem, ++registers->sp);
    return registers->cyclesUsed;
}
