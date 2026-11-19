extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunDisplayRegisterLoopOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length;
    switch (pc) {
    case 0x818633u: expected_opcode = 0xbeu; length = 3u; break;
    case 0x818636u: expected_opcode = 0x30u; length = 2u; break;
    case 0x818638u:
    case 0x81863cu: expected_opcode = 0xc8u; length = 1u; break;
    case 0x818639u: expected_opcode = 0xb9u; length = 3u; break;
    case 0x81863du: expected_opcode = 0x9du; length = 3u; break;
    case 0x818640u: expected_opcode = 0x80u; length = 2u; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x81u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || !registers->xf ||
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

    if (opcode == 0x30u || opcode == 0x80u) {
        const bool unconditional = opcode == 0x80u;
        registers->cyclesUsed = unconditional ? 3 : 2;
        const int8_t displacement = (int8_t)registers->read(registers->mem, 0x810000u | registers->pc++);
        if (unconditional || registers->n) {
            registers->pc = (uint16_t)(registers->pc + displacement);
            registers->cyclesUsed += !unconditional;
        }
        return registers->cyclesUsed;
    }

    uint8_t value;
    if (opcode == 0xc8u) {
        registers->cyclesUsed = 2;
        value = (uint8_t)(registers->y + 1u);
        registers->y = value;
    } else {
        const bool store = opcode == 0x9du;
        registers->cyclesUsed = store ? 5 : 4;
        const uint8_t low = registers->read(registers->mem, 0x810000u | registers->pc++);
        const uint8_t high = registers->read(registers->mem, 0x810000u | registers->pc++);
        const uint16_t operand = low | ((uint16_t)high << 8);
        const uint32_t indexed = (uint32_t)operand + (store ? registers->x : registers->y);
        const uint32_t address = (((uint32_t)registers->db << 16) + indexed) & 0xffffffu;
        registers->cyclesUsed += (operand >> 8) != (indexed >> 8);
        if (store) {
            registers->write(registers->mem, address, (uint8_t)registers->a);
            return registers->cyclesUsed;
        }
        value = registers->read(registers->mem, address);
        if (opcode == 0xbeu)
            registers->x = value;
        else
            registers->a = (registers->a & 0xff00u) | value;
    }

    registers->n = (value & 0x80u) != 0u;
    registers->z = value == 0u;
    return registers->cyclesUsed;
}
