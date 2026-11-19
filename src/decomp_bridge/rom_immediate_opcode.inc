extern uint32_t g_interp816_cur_pc;

static bool Lufia2DecompIsImmediateDataOpcode(uint8_t opcode) {
    switch (opcode) {
    case 0xa9u: case 0xa2u: case 0xa0u:
    case 0xc9u: case 0x29u: case 0x89u:
        return true;
    default:
        return false;
    }
}

static void Lufia2DecompApplyOperand(Interp816 *registers, uint8_t opcode,
    uint16_t value, bool byte_operand) {
    const uint16_t mask = byte_operand ? 0xffu : 0xffffu;
    const uint16_t accumulator = registers->a & mask;
    uint16_t result = value;
    switch (opcode) {
    case 0x89u:
        registers->z = (accumulator & value) == 0u;
        return;
    case 0xc9u:
        registers->c = accumulator >= value;
        result = (uint16_t)(accumulator - value) & mask;
        break;
    case 0x29u:
        result = accumulator & value;
        registers->a = (registers->a & ~mask) | result;
        break;
    case 0xa9u:
        registers->a = (registers->a & ~mask) | result;
        break;
    case 0xa2u:
        registers->x = result;
        break;
    default:
        registers->y = result;
        break;
    }
    registers->z = result == 0u;
    registers->n = (result & (byte_operand ? 0x80u : 0x8000u)) != 0u;
}

static const Cart *Lufia2DecompPlainRomOpcode(const Interp816 *registers, uint32_t pc,
    uint8_t opcode, unsigned length, unsigned *rom_offset) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return NULL;
#endif
    const unsigned bank = pc >> 16;
    const unsigned address = pc & 0xffffu;
    if (bank < 0x80u || bank > 0xbfu ||
        address < 0x8000u || address + length > 0x10000u || registers->k != bank ||
        registers->pc != address || registers->e || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return NULL;

    const Cart *cart = g_snes->cart;
    *rom_offset = (bank & 0x7fu) * 0x8000u + (address & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < *rom_offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[*rom_offset] != opcode)
        return NULL;

    return cart;
}

static int Lufia2DecompRunRomImmediateOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const bool changes_status = opcode == 0xc2u || opcode == 0xe2u;
    const bool conditional = (opcode & 0x1fu) == 0x10u;
    const bool changes_bank = opcode == 0x5cu;
    const bool immediate_data = Lufia2DecompIsImmediateDataOpcode(opcode);
    const bool byte_operand = opcode == 0xa0u || opcode == 0xa2u ? registers->xf : registers->mf;
    const bool word_operand = changes_bank || opcode == 0x4cu || opcode == 0x82u ||
        (immediate_data && !byte_operand);
    const unsigned length = changes_bank ? 4u : word_operand ? 3u : 2u;
    unsigned rom_offset;
    if ((!changes_status && !conditional && !immediate_data && !word_operand && opcode != 0x80u) ||
        !Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &rom_offset))
        return 0;

    registers->cyclesUsed = 0;
    const uint32_t program_bank = pc & 0xff0000u;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    if (immediate_data) {
        registers->cyclesUsed = length;
        uint16_t value;
        /* BIT fetches its operands as individual instruction bytes. */
        if (byte_operand || opcode == 0x89u) {
            value = registers->read(registers->mem, program_bank | registers->pc++);
            if (!byte_operand)
                value |= (uint16_t)registers->read(registers->mem, program_bank | registers->pc++) << 8;
        } else {
            const uint32_t low = program_bank | registers->pc++;
            const uint32_t high = program_bank | registers->pc++;
            if (!registers->read_word || !registers->read_word(registers->mem, low, high, &value)) {
                const uint8_t low_byte = registers->read(registers->mem, low);
                value = low_byte | ((uint16_t)registers->read(registers->mem, high) << 8);
            }
        }
        Lufia2DecompApplyOperand(registers, opcode, value, byte_operand);
        return registers->cyclesUsed;
    }
    registers->cyclesUsed = conditional ? 2u : changes_bank || opcode == 0x82u ? 4u : 3u;
    const unsigned operand = registers->read(registers->mem, program_bank | registers->pc++);
    if (changes_status) {
        const unsigned status = interp816_getFlags(registers);
        interp816_setFlags(registers, (uint8_t)(opcode == 0xc2u ? status & ~operand : status | operand));
        return registers->cyclesUsed;
    }
    if (!conditional) {
        if (word_operand) {
            const unsigned high = registers->read(registers->mem, program_bank | registers->pc++);
            const unsigned word = operand | (high << 8);
            if (opcode == 0x82u) {
                const int displacement = (int)word - ((word & 0x8000u) ? 65536 : 0);
                registers->pc = (uint16_t)(registers->pc + displacement);
            } else {
                if (changes_bank)
                    registers->k = registers->read(registers->mem, program_bank | registers->pc++);
                registers->pc = (uint16_t)word;
            }
        } else {
            const int displacement = (int)operand - ((operand & 0x80u) ? 256 : 0);
            registers->pc = (uint16_t)(registers->pc + displacement);
        }
        return registers->cyclesUsed;
    }
    bool flag;
    switch (opcode >> 6) {
    case 0u: flag = registers->n; break;
    case 1u: flag = registers->v; break;
    case 2u: flag = registers->c; break;
    default: flag = registers->z; break;
    }
    if (flag == ((opcode & 0x20u) != 0u)) {
        const int displacement = (int)operand - ((operand & 0x80u) ? 256 : 0);
        registers->pc = (uint16_t)(registers->pc + displacement);
        ++registers->cyclesUsed;
    }
    return registers->cyclesUsed;
}

static bool Lufia2DecompIsDirectPageOpcode(uint8_t opcode) {
    return opcode == 0xa5u || opcode == 0x85u || opcode == 0x64u;
}

static int Lufia2DecompRunDirectPageOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 2u, &offset);
    if (!cart)
        return 0;
    const bool wide = !registers->mf;
    const uint16_t direct = (uint16_t)(registers->dp + cart->rom[offset + 1u]);
    if (direct > 0x1fffu - wide)
        return 0;

    const uint32_t program_bank = pc & 0xff0000u;
    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 3u;
    const unsigned operand = registers->read(registers->mem, program_bank | registers->pc++);
    const unsigned low = (uint16_t)(registers->dp + operand);
    const unsigned high = low + 1u;
    registers->cyclesUsed += wide + ((registers->dp & 0xffu) != 0u);

    if (opcode == 0xa5u) {
        uint16_t value;
        if (!wide) {
            value = registers->read(registers->mem, low);
        } else if (!registers->read_word || !registers->read_word(registers->mem, low, high, &value)) {
            const uint8_t low_byte = registers->read(registers->mem, low);
            value = low_byte | ((uint16_t)registers->read(registers->mem, high) << 8);
        }
        Lufia2DecompApplyOperand(registers, 0xa9u, value, !wide);
    } else {
        const uint16_t value = opcode == 0x64u ? 0u : registers->a;
        if (!wide) {
            registers->write(registers->mem, low, (uint8_t)value);
        } else if (!registers->write_word ||
                   !registers->write_word(registers->mem, low, high, value, false)) {
            registers->write(registers->mem, low, (uint8_t)value);
            registers->write(registers->mem, high, (uint8_t)(value >> 8));
        }
    }
    return registers->cyclesUsed;
}

/* Keeping the dispatch out of line avoids growing the interpreter loop. */
#if defined(_MSC_VER)
static __declspec(noinline)
#elif defined(__GNUC__)
static __attribute__((noinline))
#else
static
#endif
int Lufia2DecompRunNativeRomControl(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    if ((opcode & 0x1fu) == 0x10u || opcode == 0xc2u || opcode == 0xe2u ||
        opcode == 0x80u || opcode == 0x82u || opcode == 0x4cu || opcode == 0x5cu ||
        Lufia2DecompIsImmediateDataOpcode(opcode))
        return Lufia2DecompRunRomImmediateOpcode(registers, pc, opcode);
    if (Lufia2DecompIsDirectPageOpcode(opcode))
        return Lufia2DecompRunDirectPageOpcode(registers, pc, opcode);
    return Lufia2DecompRunAotCallSite(registers, pc, opcode);
}
