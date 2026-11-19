extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunGaugeLabelOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length;
    switch (pc) {
    case 0x81e2afu: expected_opcode = 0xa5u; length = 2u; break;
    case 0x81e2b8u: expected_opcode = 0xa9u; length = 2u; break;
    case 0x81e2b1u:
    case 0x81e2b5u:
    case 0x81e2bau:
    case 0x81e2bdu: expected_opcode = 0x99u; length = 3u; break;
    case 0x81e2b4u: expected_opcode = 0x1au; length = 1u; break;
    case 0x81e2c0u:
    case 0x81e2c1u:
    case 0x81e2c2u:
    case 0x81e2c3u:
    case 0x81e2c4u:
    case 0x81e2c5u: expected_opcode = 0x88u; length = 1u; break;
    case 0x81e2c6u: expected_opcode = 0x80u; length = 2u; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x81u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint16_t offset = (uint16_t)pc;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < (uint32_t)offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x810000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 2;
    uint8_t value;
    switch (opcode) {
    case 0xa5u: {
        registers->cyclesUsed = 3;
        const uint8_t operand = registers->read(registers->mem, 0x810000u | registers->pc++);
        registers->cyclesUsed += (registers->dp & 0xffu) != 0u;
        value = registers->read(registers->mem, (uint16_t)(registers->dp + operand));
        break;
    }
    case 0xa9u:
        value = registers->read(registers->mem, 0x810000u | registers->pc++);
        break;
    case 0x99u: {
        registers->cyclesUsed = 5;
        const uint8_t low = registers->read(registers->mem, 0x810000u | registers->pc++);
        const uint8_t high = registers->read(registers->mem, 0x810000u | registers->pc++);
        const uint16_t operand = low | ((uint16_t)high << 8);
        const uint32_t address = (((uint32_t)registers->db << 16) + operand + registers->y) & 0xffffffu;
        registers->cyclesUsed = 6;
        registers->write(registers->mem, address, (uint8_t)registers->a);
        return registers->cyclesUsed;
    }
    case 0x88u:
        --registers->y;
        registers->n = (registers->y & 0x8000u) != 0u;
        registers->z = registers->y == 0u;
        return registers->cyclesUsed;
    case 0x80u: {
        registers->cyclesUsed = 3;
        const int8_t displacement = (int8_t)registers->read(registers->mem, 0x810000u | registers->pc++);
        registers->pc = (uint16_t)(registers->pc + displacement);
        return registers->cyclesUsed;
    }
    default:
        value = (uint8_t)(registers->a + 1u);
        break;
    }

    registers->a = (registers->a & 0xff00u) | value;
    registers->n = (value & 0x80u) != 0u;
    registers->z = value == 0u;
    return registers->cyclesUsed;
}
