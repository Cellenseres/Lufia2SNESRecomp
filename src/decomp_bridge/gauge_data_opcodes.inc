extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunGaugeDataOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    uint8_t expected_opcode;
    switch (pc) {
    case 0x81e31cu: expected_opcode = 0xa9u; break;
    case 0x81e31fu: expected_opcode = 0xabu; break;
    case 0x81e31eu: expected_opcode = 0x48u; break;
    case 0x81e322u:
    case 0x81e326u:
    case 0x81e333u: expected_opcode = 0xa5u; break;
    case 0x81e328u:
    case 0x81e324u: expected_opcode = 0x85u; break;
    case 0x81e32eu: expected_opcode = 0xc9u; break;
    case 0x81e331u: expected_opcode = 0xf0u; break;
    case 0x81e335u: expected_opcode = 0xd0u; break;
    case 0x81e337u: expected_opcode = 0xe6u; break;
    default: return 0;
    }
    const bool push = expected_opcode == 0x48u;
    const bool compare = expected_opcode == 0xc9u;
    const bool pull_bank = expected_opcode == 0xabu;
    const bool load_byte = expected_opcode == 0xa9u;
    const bool increment = expected_opcode == 0xe6u;
    const bool accumulator_8_bit = push || pull_bank || load_byte;
    const unsigned length = push || pull_bank ? 1u : compare ? 3u : 2u;
    if (opcode != expected_opcode || registers->k != 0x81u ||
        registers->pc != (uint16_t)pc || registers->e || registers->mf != accumulator_8_bit || registers->xf ||
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
    registers->cyclesUsed = 3;
    if (push) {
        registers->write(registers->mem, registers->sp, (uint8_t)registers->a);
        --registers->sp;
        return registers->cyclesUsed;
    }

    if (pull_bank || load_byte) {
        uint8_t value;
        if (pull_bank) {
            registers->cyclesUsed = 4;
            value = registers->read(registers->mem, ++registers->sp);
            registers->db = value;
        } else {
            registers->cyclesUsed = 2;
            value = registers->read(registers->mem, 0x810000u | registers->pc++);
            registers->a = (registers->a & 0xff00u) | value;
        }
        registers->n = (value & 0x80u) != 0u;
        registers->z = value == 0u;
        return registers->cyclesUsed;
    }

    if (opcode == 0xf0u || opcode == 0xd0u) {
        registers->cyclesUsed = 2;
        const int8_t displacement = (int8_t)registers->read(registers->mem, 0x810000u | registers->pc++);
        if (registers->z == (opcode == 0xf0u)) {
            registers->pc = (uint16_t)(registers->pc + displacement);
            ++registers->cyclesUsed;
        }
        return registers->cyclesUsed;
    }

    uint32_t low_address;
    uint32_t high_address;
    if (compare) {
        low_address = 0x810000u | registers->pc++;
        high_address = 0x810000u | registers->pc++;
    } else {
        if (increment)
            registers->cyclesUsed = 5;
        const uint8_t operand = registers->read(registers->mem, 0x810000u | registers->pc++);
        low_address = (uint16_t)(registers->dp + operand);
        high_address = (uint16_t)(low_address + 1u);
        registers->cyclesUsed = (increment ? 7 : 4) + ((registers->dp & 0xffu) != 0u);
    }

    if (opcode == 0x85u) {
        if (!registers->write_word ||
            !registers->write_word(registers->mem, low_address, high_address, registers->a, false)) {
            registers->write(registers->mem, low_address, (uint8_t)registers->a);
            registers->write(registers->mem, high_address, (uint8_t)(registers->a >> 8));
        }
        return registers->cyclesUsed;
    }

    uint16_t value;
    if (!registers->read_word ||
        !registers->read_word(registers->mem, low_address, high_address, &value)) {
        const uint8_t low = registers->read(registers->mem, low_address);
        value = low | ((uint16_t)registers->read(registers->mem, high_address) << 8);
    }
    uint16_t result = value;
    if (increment) {
        result = (uint16_t)(value + 1u);
        if (!registers->write_word ||
            !registers->write_word(registers->mem, low_address, high_address, result, true)) {
            registers->write(registers->mem, high_address, (uint8_t)(result >> 8));
            registers->write(registers->mem, low_address, (uint8_t)result);
        }
    } else if (compare) {
        result = (uint16_t)(registers->a - value);
        registers->c = registers->a >= value;
    } else {
        registers->a = value;
    }
    registers->n = (result & 0x8000u) != 0u;
    registers->z = result == 0u;
    return registers->cyclesUsed;
}
