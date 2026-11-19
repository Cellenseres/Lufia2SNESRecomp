extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunBattleDisplayOpcode(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    bool load;
    switch (pc) {
    case 0x859bf5u:
    case 0x859bfbu:
    case 0x859c01u:
        load = true;
        break;
    case 0x859bf8u:
    case 0x859bfeu:
    case 0x859c04u:
        load = false;
        break;
    default:
        return 0;
    }
    if (opcode != (load ? 0xa9u : 0x99u) || registers->k != 0x85u ||
        registers->pc != (uint16_t)pc || registers->e || registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t offset = 0x28000u + (pc & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + 3u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, 0x850000u | registers->pc++);
    g_interp816_cur_pc = pc;
    if (load) {
        const uint32_t low_address = 0x850000u | registers->pc++;
        const uint32_t high_address = 0x850000u | registers->pc++;
        uint16_t value;
        registers->cyclesUsed = 3;
        if (!registers->read_word ||
            !registers->read_word(registers->mem, low_address, high_address, &value)) {
            const uint8_t low = registers->read(registers->mem, low_address);
            value = low | ((uint16_t)registers->read(registers->mem, high_address) << 8);
        }
        registers->a = value;
        registers->n = (value & 0x8000u) != 0u;
        registers->z = value == 0u;
        return registers->cyclesUsed;
    }

    registers->cyclesUsed = 5;
    const uint8_t low = registers->read(registers->mem, 0x850000u | registers->pc++);
    const uint8_t high = registers->read(registers->mem, 0x850000u | registers->pc++);
    const uint16_t operand = low | ((uint16_t)high << 8);
    const uint32_t low_address = (((uint32_t)registers->db << 16) + operand + registers->y) & 0xffffffu;
    const uint32_t high_address = (low_address + 1u) & 0xffffffu;
    registers->cyclesUsed = 7;
    if (!registers->write_word ||
        !registers->write_word(registers->mem, low_address, high_address, registers->a, false)) {
        registers->write(registers->mem, low_address, (uint8_t)registers->a);
        registers->write(registers->mem, high_address, (uint8_t)(registers->a >> 8));
    }
    return registers->cyclesUsed;
}
