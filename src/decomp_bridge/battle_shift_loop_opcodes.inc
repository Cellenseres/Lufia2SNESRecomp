extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunBattleShiftLoopOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    unsigned length;
    switch (pc) {
    case 0x85ce08u: expected_opcode = 0x1au; length = 1u; break;
    case 0x85ce09u: expected_opcode = 0x4eu; length = 3u; break;
    case 0x85ce0cu: expected_opcode = 0x90u; length = 2u; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x85u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x28000u + (pc & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x850000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 2;
    if (opcode == 0x90u) {
        const int8_t displacement = (int8_t)registers->read(registers->mem, 0x850000u | registers->pc++);
        if (!registers->c) {
            registers->pc = (uint16_t)(registers->pc + displacement);
            ++registers->cyclesUsed;
        }
        return registers->cyclesUsed;
    }

    uint8_t result;
    if (opcode == 0x4eu) {
        registers->cyclesUsed = 6;
        const uint8_t low = registers->read(registers->mem, 0x850000u | registers->pc++);
        const uint8_t high = registers->read(registers->mem, 0x850000u | registers->pc++);
        const uint32_t address = ((uint32_t)registers->db << 16) | ((uint16_t)high << 8) | low;
        const uint8_t value = registers->read(registers->mem, address);
        registers->c = (value & 1u) != 0u;
        result = value >> 1;
        registers->write(registers->mem, address, result);
    } else {
        result = (uint8_t)(registers->a + 1u);
        registers->a = (registers->a & 0xff00u) | result;
    }
    registers->n = (result & 0x80u) != 0u;
    registers->z = result == 0u;
    return registers->cyclesUsed;
}
