extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunActorLoopWidthOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length = 2u;
    switch (pc) {
    case 0x83a7b5u: expected_opcode = 0xe2u; break;
    case 0x83a7b7u: expected_opcode = 0x80u; break;
    case 0x83a7c0u: expected_opcode = 0xa6u; break;
    case 0x83a7c5u: expected_opcode = 0xc2u; break;
    case 0x83a7c7u: expected_opcode = 0xa2u; length = 3u; break;
    default: return 0;
    }
    const bool index_8_bit = opcode != 0xe2u && opcode != 0xa2u;
    if (opcode != expected_opcode || registers->k != 0x83u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf ||
        registers->xf != index_8_bit || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x18000u + (pc & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x830000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 3;
    if (opcode == 0xa2u) {
        const uint32_t low_address = 0x830000u | registers->pc++;
        const uint32_t high_address = 0x830000u | registers->pc++;
        uint16_t value;
        if (!registers->read_word ||
            !registers->read_word(registers->mem, low_address, high_address, &value)) {
            const uint8_t low = registers->read(registers->mem, low_address);
            value = low | ((uint16_t)registers->read(registers->mem, high_address) << 8);
        }
        registers->x = value;
        registers->n = (value & 0x8000u) != 0u;
        registers->z = value == 0u;
        return registers->cyclesUsed;
    }

    const uint8_t operand = registers->read(registers->mem, 0x830000u | registers->pc++);
    if (opcode == 0xa6u) {
        registers->cyclesUsed += (registers->dp & 0xffu) != 0u;
        registers->x = registers->read(registers->mem, (uint16_t)(registers->dp + operand));
        registers->n = (registers->x & 0x80u) != 0u;
        registers->z = registers->x == 0u;
    } else if (opcode == 0x80u) {
        registers->pc = (uint16_t)(registers->pc + (int8_t)operand);
    } else {
        const uint8_t flags = interp816_getFlags(registers);
        interp816_setFlags(registers, opcode == 0xe2u ? flags | operand : flags & ~operand);
    }
    return registers->cyclesUsed;
}
