extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunSoundTableLoopOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    switch (pc) {
    case 0x80972bu: expected_opcode = 0x48u; break;
    case 0x80972fu: expected_opcode = 0x68u; break;
    case 0x809730u: expected_opcode = 0x1au; break;
    case 0x809731u: expected_opcode = 0xcau; break;
    case 0x809732u: expected_opcode = 0xd0u; break;
    default: return 0;
    }
    if (opcode != expected_opcode || registers->k != 0x80u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = pc & 0x7fffu;
    const unsigned length = opcode == 0xd0u ? 2u : 1u;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x800000u | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 2;

    if (opcode == 0xd0u) {
        const int8_t displacement = (int8_t)registers->read(registers->mem, 0x800000u | registers->pc++);
        if (!registers->z) {
            registers->pc = (uint16_t)(registers->pc + displacement);
            ++registers->cyclesUsed;
        }
        return registers->cyclesUsed;
    }
    if (opcode == 0xcau) {
        --registers->x;
        registers->n = (registers->x & 0x8000u) != 0u;
        registers->z = registers->x == 0u;
        return registers->cyclesUsed;
    }
    if (opcode == 0x48u) {
        registers->cyclesUsed = 3;
        registers->write(registers->mem, registers->sp, (uint8_t)registers->a);
        --registers->sp;
        return registers->cyclesUsed;
    }

    uint8_t value;
    if (opcode == 0x68u) {
        registers->cyclesUsed = 4;
        value = registers->read(registers->mem, ++registers->sp);
    } else {
        value = (uint8_t)(registers->a + 1u);
    }
    registers->a = (registers->a & 0xff00u) | value;
    registers->n = (value & 0x80u) != 0u;
    registers->z = value == 0u;
    return registers->cyclesUsed;
}
