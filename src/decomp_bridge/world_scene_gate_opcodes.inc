extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunWorldSceneGateOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length = 2u;
    switch (pc) {
    case 0x869e17u: expected_opcode = 0xa6u; break;
    case 0x869e19u: expected_opcode = 0xe0u; length = 3u; break;
    case 0x869e1cu: expected_opcode = 0xd0u; break;
    case 0x869e21u: expected_opcode = 0xadu; length = 3u; break;
    case 0x869e24u: expected_opcode = 0x30u; break;
    case 0x869e38u: expected_opcode = 0x80u; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x86u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x30000u + (pc & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x860000u | registers->pc++);
    g_interp816_cur_pc = pc;

    if (opcode == 0xd0u || opcode == 0x30u || opcode == 0x80u) {
        const bool unconditional = opcode == 0x80u;
        registers->cyclesUsed = unconditional ? 3 : 2;
        const int8_t displacement = (int8_t)registers->read(registers->mem, 0x860000u | registers->pc++);
        if (unconditional || (opcode == 0xd0u ? !registers->z : registers->n)) {
            registers->pc = (uint16_t)(registers->pc + displacement);
            registers->cyclesUsed += !unconditional;
        }
        return registers->cyclesUsed;
    }

    if (opcode == 0xadu) {
        registers->cyclesUsed = 4;
        const uint8_t low = registers->read(registers->mem, 0x860000u | registers->pc++);
        const uint8_t high = registers->read(registers->mem, 0x860000u | registers->pc++);
        const uint32_t address = ((uint32_t)registers->db << 16) | ((uint16_t)high << 8) | low;
        const uint8_t value = registers->read(registers->mem, address);
        registers->a = (registers->a & 0xff00u) | value;
        registers->n = (value & 0x80u) != 0u;
        registers->z = value == 0u;
        return registers->cyclesUsed;
    }

    const bool compare = opcode == 0xe0u;
    uint32_t low_address, high_address;
    if (compare) {
        low_address = 0x860000u | registers->pc++;
        high_address = 0x860000u | registers->pc++;
        registers->cyclesUsed = 3;
    } else {
        registers->cyclesUsed = 3;
        const uint8_t operand = registers->read(registers->mem, 0x860000u | registers->pc++);
        low_address = (uint16_t)(registers->dp + operand);
        high_address = (uint16_t)(low_address + 1u);
        registers->cyclesUsed = 4 + ((registers->dp & 0xffu) != 0u);
    }

    uint16_t value;
    if (!registers->read_word ||
        !registers->read_word(registers->mem, low_address, high_address, &value)) {
        const uint8_t low = registers->read(registers->mem, low_address);
        value = low | ((uint16_t)registers->read(registers->mem, high_address) << 8);
    }
    uint16_t result = value;
    if (compare) {
        result = (uint16_t)(registers->x - value);
        registers->c = registers->x >= value;
    } else {
        registers->x = value;
    }
    registers->n = (result & 0x8000u) != 0u;
    registers->z = result == 0u;
    return registers->cyclesUsed;
}
