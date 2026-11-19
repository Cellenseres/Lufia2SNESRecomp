extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunSoundCpuReturnOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    switch (pc) {
    case 0x8098a0u: expected_opcode = 0xabu; break;
    case 0x8098a1u: expected_opcode = 0x28u; break;
    case 0x8098a2u: expected_opcode = 0x7au; break;
    case 0x8098a3u: expected_opcode = 0xfau; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x80u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;
    const Cart *cart = g_snes->cart;
    const uint32_t offset = pc & 0x7fffu;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize <= offset ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x800000u | registers->pc++);
    g_interp816_cur_pc = pc;
    if (opcode == 0x28u) {
        registers->cyclesUsed = 4;
        interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
        return registers->cyclesUsed;
    }
    if (opcode == 0xabu) {
        registers->cyclesUsed = 4;
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0u;
        registers->z = registers->db == 0u;
        return registers->cyclesUsed;
    }

    registers->cyclesUsed = 5;
    const uint8_t low = registers->read(registers->mem, ++registers->sp);
    const uint8_t high = registers->read(registers->mem, ++registers->sp);
    const uint16_t value = low | ((uint16_t)high << 8);
    if (opcode == 0x7au)
        registers->y = value;
    else
        registers->x = value;
    registers->n = (value & 0x8000u) != 0u;
    registers->z = value == 0u;
    return registers->cyclesUsed;
}
