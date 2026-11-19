extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunDivideFrameOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length;
    switch (pc) {
    case 0x808378u: expected_opcode = 0x08u; length = 1u; break;
    case 0x808379u: expected_opcode = 0xc2u; length = 2u; break;
    case 0x80837bu: expected_opcode = 0xa9u; length = 3u; break;
    case 0x80844eu: expected_opcode = 0x28u; length = 1u; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x80u ||
        registers->pc != (uint16_t)pc || registers->e || registers->xf ||
        ((opcode == 0xa9u || opcode == 0x28u) && registers->mf) ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = pc & 0x7fffu;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x800000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 3;
    switch (opcode) {
    case 0x08u:
        registers->write(registers->mem, registers->sp, interp816_getFlags(registers));
        --registers->sp;
        break;
    case 0xc2u: {
        const uint8_t mask = registers->read(registers->mem, 0x800000u | registers->pc++);
        interp816_setFlags(registers, interp816_getFlags(registers) & ~mask);
        break;
    }
    case 0x28u:
        registers->cyclesUsed = 4;
        interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
        break;
    case 0xa9u: {
        const uint32_t low_address = 0x800000u | registers->pc++;
        const uint32_t high_address = 0x800000u | registers->pc++;
        uint16_t value;
        if (!registers->read_word ||
            !registers->read_word(registers->mem, low_address, high_address, &value)) {
            const uint8_t low = registers->read(registers->mem, low_address);
            value = low | ((uint16_t)registers->read(registers->mem, high_address) << 8);
        }
        registers->a = value;
        registers->n = (value & 0x8000u) != 0u;
        registers->z = value == 0u;
        break;
    }
    }
    return registers->cyclesUsed;
}
