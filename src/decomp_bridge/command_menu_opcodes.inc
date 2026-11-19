extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunCommandMenuOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length;
    uint32_t expected_operand;
    switch (pc) {
    case 0x81cb96u:
        expected_opcode = 0xa2u;
        length = 3u;
        expected_operand = 0u;
        break;
    case 0x81cbe7u:
    case 0x81cce2u:
        expected_opcode = 0xa9u;
        length = 2u;
        expected_operand = 0xffu;
        break;
    case 0x81cbe9u:
    case 0x81cce4u:
        expected_opcode = 0x8fu;
        length = 4u;
        expected_operand = 0x12f3u;
        break;
    default:
        return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x81u || registers->pc != (uint16_t)pc ||
        registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint16_t offset = (uint16_t)pc;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;
    for (unsigned i = 1; i < length; ++i)
        if (cart->rom[offset + i] != (uint8_t)(expected_operand >> (8u * (i - 1u))))
            return 0;

    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = opcode == 0x8fu ? 5 : length;
    if (opcode == 0xa2u) {
        const uint32_t low_address = 0x810000u | registers->pc++;
        const uint32_t high_address = 0x810000u | registers->pc++;
        uint16_t value;
        if (!registers->read_word ||
            !registers->read_word(registers->mem, low_address, high_address, &value)) {
            const uint8_t low = registers->read(registers->mem, low_address);
            value = low | ((uint16_t)registers->read(registers->mem, high_address) << 8);
        }
        registers->x = value;
        registers->n = (value & 0x8000u) != 0;
        registers->z = value == 0u;
        return registers->cyclesUsed;
    }
    uint32_t operand = 0u;
    for (unsigned i = 1; i < length; ++i) {
        const uint8_t byte = registers->read(registers->mem, 0x810000u | registers->pc++);
        operand |= (uint32_t)byte << (8u * (i - 1u));
    }
    if (opcode == 0xa9u) {
        registers->a = (registers->a & 0xff00u) | operand;
        registers->n = (operand & 0x80u) != 0;
        registers->z = operand == 0u;
    } else {
        registers->write(registers->mem, operand, (uint8_t)registers->a);
    }
    return registers->cyclesUsed;
}
