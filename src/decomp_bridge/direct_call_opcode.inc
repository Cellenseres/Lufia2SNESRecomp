extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunDirectCallWithWidth(
    Interp816 *registers, uint32_t pc, uint8_t opcode, uint32_t target,
    bool long_call, bool accumulator_8_bit) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode != (long_call ? 0x22u : 0x20u) ||
        registers->k != (pc >> 16) || registers->pc != (uint16_t)pc ||
        registers->e || registers->mf != accumulator_8_bit || registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        registers->sp < 0x1f00u || registers->sp > 0x1fffu ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = ((pc >> 16) & 0x7fu) * 0x8000u + (pc & 0x7fffu);
    const unsigned length = long_call ? 4u : 3u;
    if (cart->type != CART_LOROM || !cart->rom ||
        cart->romSize < offset + length || cart->superfx || cart->cx4 ||
        cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode ||
        cart->rom[offset + 1u] != (uint8_t)target ||
        cart->rom[offset + 2u] != (uint8_t)(target >> 8) ||
        (long_call && cart->rom[offset + 3u] != (uint8_t)(target >> 16)))
        return 0;

    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    for (unsigned i = 1u; i < length; ++i)
        (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);

    const uint16_t pushed_pc = (uint16_t)(registers->pc - 1u);
    if (long_call) {
        registers->write(registers->mem, registers->sp, registers->k);
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)(pushed_pc >> 8));
    --registers->sp;
    registers->write(registers->mem, registers->sp, (uint8_t)pushed_pc);
    --registers->sp;
    registers->pc = (uint16_t)target;
    registers->k = (uint8_t)(target >> 16);
    registers->cyclesUsed = long_call ? 8 : 6;
    return registers->cyclesUsed;
}

static int Lufia2DecompRunDirectCall(
    Interp816 *registers, uint32_t pc, uint8_t opcode, uint32_t target, bool long_call) {
    return Lufia2DecompRunDirectCallWithWidth(registers, pc, opcode, target, long_call, true);
}
