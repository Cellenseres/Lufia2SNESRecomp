#if defined(_MSC_VER)
#define LUFIA2_ROM_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define LUFIA2_ROM_NOINLINE __attribute__((noinline))
#else
#define LUFIA2_ROM_NOINLINE
#endif

extern uint32_t g_interp816_cur_pc;

static bool Lufia2DecompIsImmediateDataOpcode(uint8_t opcode) {
    switch (opcode) {
    case 0xa9u: case 0xa2u: case 0xa0u:
    case 0xc9u: case 0xc0u: case 0xe0u:
    case 0x09u: case 0x29u: case 0x49u: case 0x89u:
    case 0x69u: case 0xe9u:
        return true;
    default:
        return false;
    }
}

static LUFIA2_ROM_NOINLINE uint16_t Lufia2DecompAddSubtract(Interp816 *registers, uint16_t accumulator,
    uint16_t operand, uint16_t mask, unsigned sign, bool subtract) {
    if (subtract)
        operand ^= mask;

    int sum = accumulator + operand + registers->c;
    if (registers->d) {
        sum = registers->c;
        for (unsigned shift = 0u; shift < (sign == 0x80u ? 4u : 12u); shift += 4u) {
            const unsigned digit = 0xfu << shift;
            const unsigned carry = 0x10u << shift;
            const unsigned correction = 6u << shift;
            sum += (accumulator & digit) + (operand & digit);
            if (subtract) {
                if (sum < (int)carry)
                    sum = (unsigned)(sum - (int)correction) & (carry - 1u);
            } else if (sum > (int)(carry - correction - 1u)) {
                sum = ((sum + correction) & (carry - 1u)) | carry;
            }
        }
        const unsigned top_digit = sign | (sign >> 1) | (sign >> 2) | (sign >> 3);
        sum += (accumulator & top_digit) + (operand & top_digit);
    }

    registers->v = ((accumulator ^ operand) & sign) == 0u &&
                   ((operand ^ (unsigned)sum) & sign) != 0u;
    if (registers->d) {
        const int correction = (int)(sign >> 3) * 6;
        if (subtract ? sum <= mask : sum > mask - correction)
            sum += subtract ? -correction : correction;
    }
    registers->c = sum > mask;
    return (uint16_t)sum & mask;
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
    case 0xc9u: case 0xc0u: case 0xe0u: {
        const uint16_t source = (opcode == 0xe0u ? registers->x :
            opcode == 0xc0u ? registers->y : accumulator) & mask;
        registers->c = source >= value;
        result = (uint16_t)(source - value) & mask;
        break;
    }
    case 0x69u: case 0xe9u:
        result = Lufia2DecompAddSubtract(registers, accumulator, value, mask,
            byte_operand ? 0x80u : 0x8000u, opcode == 0xe9u);
        registers->a = (registers->a & ~mask) | result;
        break;
    case 0x09u: case 0x29u: case 0x49u:
        result = opcode == 0x09u ? accumulator | value :
            opcode == 0x29u ? accumulator & value : accumulator ^ value;
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

static uint16_t Lufia2DecompReadData(Interp816 *registers, uint32_t low, uint32_t high, bool wide) {
    if (!wide)
        return registers->read(registers->mem, low);
    uint16_t value;
    if (registers->read_word && registers->read_word(registers->mem, low, high, &value))
        return value;
    const uint8_t low_byte = registers->read(registers->mem, low);
    return low_byte | ((uint16_t)registers->read(registers->mem, high) << 8);
}

static void Lufia2DecompWriteData(Interp816 *registers, uint32_t low, uint32_t high,
    uint16_t value, bool wide) {
    if (wide && registers->write_word &&
        registers->write_word(registers->mem, low, high, value, false))
        return;
    registers->write(registers->mem, low, (uint8_t)value);
    if (wide)
        registers->write(registers->mem, high, (uint8_t)(value >> 8));
}

static int Lufia2DecompRunRomImmediateOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const bool changes_status = opcode == 0xc2u || opcode == 0xe2u;
    const bool conditional = (opcode & 0x1fu) == 0x10u;
    const bool changes_bank = opcode == 0x5cu;
    const bool immediate_data = Lufia2DecompIsImmediateDataOpcode(opcode);
    const bool index_operand = opcode == 0xa0u || opcode == 0xa2u || opcode == 0xc0u || opcode == 0xe0u;
    const bool byte_operand = index_operand ? registers->xf : registers->mf;
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
            value = Lufia2DecompReadData(registers, low, high, true);
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
    switch (opcode) {
    case 0xa5u: case 0x85u: case 0x64u:
    case 0xa6u: case 0xa4u: case 0x86u: case 0x84u:
    case 0xb5u: case 0x95u: case 0x74u: case 0xb4u: case 0x94u: case 0xb6u: case 0x96u:
    case 0x04u: case 0x14u: case 0xc6u: case 0xd6u: case 0xe6u: case 0xf6u:
    case 0x06u: case 0x16u: case 0x26u: case 0x36u:
    case 0x46u: case 0x56u: case 0x66u: case 0x76u:
        return true;
    default:
        return false;
    }
}

#define LUFIA2_MEMORY_MODIFY_CASES \
    case 0x04u: case 0x14u: case 0x0cu: case 0x1cu: \
    case 0xc6u: case 0xd6u: case 0xceu: case 0xdeu: \
    case 0xe6u: case 0xf6u: case 0xeeu: case 0xfeu: \
    case 0x06u: case 0x16u: case 0x0eu: case 0x1eu: \
    case 0x26u: case 0x36u: case 0x2eu: case 0x3eu: \
    case 0x46u: case 0x56u: case 0x4eu: case 0x5eu: \
    case 0x66u: case 0x76u: case 0x6eu: case 0x7eu:

static uint8_t Lufia2DecompMemoryReadOperation(uint8_t opcode) {
    switch (opcode) {
    case 0xc4u: case 0xccu:
        return 0xc0u;
    case 0xe4u: case 0xecu:
        return 0xe0u;
    case 0x24u: case 0x34u: case 0x2cu: case 0x3cu:
        return 0x89u;
    default:
        break;
    }

    const uint8_t group = opcode & 0xe0u;
    if (group == 0x80u || group == 0xa0u)
        return 0u;
    switch (opcode & 0x1fu) {
    case 0x05u: case 0x0du: case 0x0fu: case 0x15u:
    case 0x19u: case 0x1du: case 0x1fu:
        return group | 0x09u;
    default:
        return 0u;
    }
}

static bool Lufia2DecompIsMemoryOpcode(uint8_t opcode) {
    switch (opcode) {
    case 0xa5u: case 0x85u: case 0x64u: case 0xadu: case 0x8du: case 0x9cu:
    case 0xbdu: case 0xb9u: case 0x9du: case 0x99u: case 0x9eu:
    case 0xafu: case 0xbfu: case 0x8fu: case 0x9fu:
    case 0xa6u: case 0xa4u: case 0x86u: case 0x84u:
    case 0xb5u: case 0x95u: case 0x74u: case 0xb4u: case 0x94u: case 0xb6u: case 0x96u:
    LUFIA2_MEMORY_MODIFY_CASES
    case 0xaeu: case 0xacu: case 0x8eu: case 0x8cu: case 0xbeu: case 0xbcu:
    case 0xa7u: case 0xb7u: case 0x87u: case 0x97u:
        return true;
    default:
        return Lufia2DecompMemoryReadOperation(opcode) != 0u;
    }
}

static bool Lufia2DecompIsWramByte(uint32_t address) {
    const unsigned bank = address >> 16;
    return bank == 0x7eu || bank == 0x7fu ||
        ((bank & 0x7fu) < 0x40u && (address & 0xffffu) < 0x2000u);
}

static bool Lufia2DecompIsLoRomDataByte(const Cart *cart, uint32_t address) {
    const unsigned bank = address >> 16;
    const unsigned offset = address & 0x7fffu;
    return bank >= 0x80u && bank <= 0xbfu && (address & 0x8000u) != 0u &&
        (bank & 0x7fu) * 0x8000u + offset < cart->romSize;
}

static LUFIA2_ROM_NOINLINE bool Lufia2DecompCanReadLoRomData(const Cart *cart, uint32_t address, bool wide) {
    return Lufia2DecompIsLoRomDataByte(cart, address) &&
        (!wide || Lufia2DecompIsLoRomDataByte(cart, (address + 1u) & 0xffffffu));
}

static uint32_t Lufia2DecompMemoryOperandAddress(const Interp816 *registers,
    unsigned operand, bool direct_page, bool long_address, unsigned index) {
    if (direct_page)
        return (uint16_t)(registers->dp + operand + index);
    const uint32_t base = long_address ? operand : ((uint32_t)registers->db << 16) | operand;
    return (base + index) & 0xffffffu;
}

static LUFIA2_ROM_NOINLINE bool Lufia2DecompCanAccessVideoDmaRegister(uint8_t opcode, uint32_t address, bool wide) {
    const unsigned bank = (address >> 16) & 0x7fu;
    const unsigned port = address & 0xffffu;
    if (bank > 0x3fu)
        return false;
    if (opcode == 0xadu)
        return !wide && port == 0x4212u;
    if (opcode != 0x8du && opcode != 0x8eu && opcode != 0x8cu)
        return false;
    if (port >= 0x4360u && port + wide <= 0x4366u)
        return true;
    if (port == 0x2102u || port == 0x2116u)
        return true;
    return !wide && (port == 0x2100u || port == 0x2121u || port == 0x420bu);
}

static bool Lufia2DecompIsMemoryModifyOpcode(uint8_t opcode) {
    switch (opcode) {
    LUFIA2_MEMORY_MODIFY_CASES
        return true;
    default:
        return false;
    }
}

static void Lufia2DecompModifyWram(Interp816 *registers, uint8_t opcode,
    uint32_t low, uint32_t high, bool wide) {
    uint16_t value = Lufia2DecompReadData(registers, low, high, wide);
    const bool bit_mask = (opcode & 7u) == 4u;
    const uint16_t width_mask = wide ? 0xffffu : 0xffu;
    const uint16_t sign = wide ? 0x8000u : 0x80u;
    const bool rotate_right = (opcode & 0xe0u) == 0x60u;
    const bool low_bit = (value & 1u) != 0u;
    if (bit_mask) {
        registers->z = (value & registers->a) == 0u;
        value = opcode & 0x10u ? value & ~registers->a : value | registers->a;
    } else if (opcode < 0x40u) {
        const bool incoming = (opcode & 0x20u) != 0u && registers->c;
        registers->c = (value & sign) != 0u;
        value = (uint16_t)((value << 1) | incoming);
    } else if (opcode < 0x80u) {
        value = (value >> 1) | (rotate_right && registers->c ? sign : 0u);
        if (!rotate_right)
            registers->c = low_bit;
    } else {
        value = (uint16_t)(value + (opcode & 0x20u ? 1 : -1));
    }
    value &= width_mask;
    if (!wide) {
        registers->write(registers->mem, low, (uint8_t)value);
    } else if (!registers->write_word || !registers->write_word(registers->mem, low, high, value, true)) {
        registers->write(registers->mem, high, (uint8_t)(value >> 8));
        registers->write(registers->mem, low, (uint8_t)value);
    }
    if (!bit_mask) {
        registers->n = (value & sign) != 0u;
        registers->z = value == 0u;
        if (rotate_right)
            registers->c = low_bit;
    }
}

static LUFIA2_ROM_NOINLINE int Lufia2DecompRunMemoryModifyOpcode(Interp816 *registers,
    uint32_t pc, uint8_t opcode) {
    const bool direct_page = Lufia2DecompIsDirectPageOpcode(opcode);
    const bool indexed = opcode == 0xd6u || opcode == 0xf6u || opcode == 0xdeu || opcode == 0xfeu ||
        (opcode < 0x80u && (opcode & 0x17u) == 0x16u);
    const unsigned index = indexed ? registers->x : 0u;
    const unsigned length = direct_page ? 2u : 3u;
    const bool wide = !registers->mf;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart)
        return 0;

    unsigned operand = cart->rom[offset + 1u];
    if (!direct_page)
        operand |= (unsigned)cart->rom[offset + 2u] << 8;
    const uint32_t address = Lufia2DecompMemoryOperandAddress(registers, operand, direct_page, false, index);
    if (!Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte((address + 1u) & 0xffffffu)))
        return 0;

    const uint32_t bank = pc & 0xff0000u;
    registers->cyclesUsed = 0u;
    (void)registers->read(registers->mem, bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = (direct_page ? 5u : 6u) + indexed;
    operand = registers->read(registers->mem, bank | registers->pc++);
    if (!direct_page)
        operand |= (unsigned)registers->read(registers->mem, bank | registers->pc++) << 8;

    if (direct_page && (registers->dp & 0xffu))
        ++registers->cyclesUsed;
    if (indexed && !direct_page &&
        (!registers->xf || (operand >> 8) != ((operand + index) >> 8)))
        ++registers->cyclesUsed;
    const uint32_t low = Lufia2DecompMemoryOperandAddress(registers, operand, direct_page, false, index);
    registers->cyclesUsed += 2u * wide;
    Lufia2DecompModifyWram(registers, opcode, low, (low + 1u) & 0xffffffu, wide);
    return registers->cyclesUsed;
}

static int Lufia2DecompRunMemoryOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const uint8_t operation = Lufia2DecompMemoryReadOperation(opcode);
    const unsigned address_mode = opcode & 0x1fu;
    const bool direct_page = Lufia2DecompIsDirectPageOpcode(opcode) ||
        (operation != 0u && (address_mode == 0x05u || address_mode == 0x15u)) ||
        opcode == 0x24u || opcode == 0x34u || opcode == 0xc4u || opcode == 0xe4u;
    const bool long_address = opcode == 0xafu || opcode == 0xbfu || opcode == 0x8fu || opcode == 0x9fu ||
        (operation != 0u && (address_mode == 0x0fu || address_mode == 0x1fu));
    const bool index_x = opcode == 0xbdu || opcode == 0x9du || opcode == 0x9eu ||
        opcode == 0xbfu || opcode == 0x9fu || opcode == 0xb5u || opcode == 0x95u ||
        opcode == 0x74u || opcode == 0xb4u || opcode == 0x94u || opcode == 0xbcu || opcode == 0x34u || opcode == 0x3cu ||
        (operation != 0u && (address_mode == 0x15u || address_mode == 0x1du || address_mode == 0x1fu));
    const bool index_y = opcode == 0xb9u || opcode == 0x99u || opcode == 0xb6u ||
        opcode == 0x96u || opcode == 0xbeu || (operation != 0u && address_mode == 0x19u);
    const bool indexed = index_x || index_y;
    const unsigned index = index_x ? registers->x : index_y ? registers->y : 0u;
    const bool load = operation != 0u || opcode == 0xa5u || opcode == 0xadu || opcode == 0xbdu ||
        opcode == 0xb9u || opcode == 0xafu || opcode == 0xbfu ||
        opcode == 0xa6u || opcode == 0xa4u || opcode == 0xaeu || opcode == 0xacu ||
        opcode == 0xb5u || opcode == 0xb4u || opcode == 0xb6u || opcode == 0xbcu || opcode == 0xbeu;
    const bool data_x = opcode == 0xa6u || opcode == 0xaeu || opcode == 0x86u || opcode == 0x8eu ||
        opcode == 0xb6u || opcode == 0x96u || opcode == 0xbeu;
    const bool data_y = opcode == 0xa4u || opcode == 0xacu || opcode == 0x84u || opcode == 0x8cu ||
        opcode == 0xb4u || opcode == 0x94u || opcode == 0xbcu;
    const unsigned length = direct_page ? 2u : long_address ? 4u : 3u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart)
        return 0;
    const bool index_data = data_x || data_y || operation == 0xc0u || operation == 0xe0u;
    const bool wide = !(index_data ? registers->xf : registers->mf);
    unsigned operand = 0u;
    for (unsigned byte = 1u; byte < length; ++byte)
        operand |= (unsigned)cart->rom[offset + byte] << (8u * (byte - 1u));
    const uint32_t address = Lufia2DecompMemoryOperandAddress(registers, operand, direct_page, long_address, index);
    if ((!Lufia2DecompIsWramByte(address) ||
         (wide && !Lufia2DecompIsWramByte((address + 1u) & 0xffffffu))) &&
        !(load && Lufia2DecompCanReadLoRomData(cart, address, wide)) &&
        !Lufia2DecompCanAccessVideoDmaRegister(opcode, address, wide))
        return 0;

    const uint32_t program_bank = pc & 0xff0000u;
    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = direct_page ? 3u + indexed : long_address || (indexed && !load) ? 5u : 4u;
    operand = 0u;
    for (unsigned byte = 1u; byte < length; ++byte)
        operand |= (unsigned)registers->read(registers->mem, program_bank | registers->pc++) << (8u * (byte - 1u));
    const uint32_t low = Lufia2DecompMemoryOperandAddress(registers, operand, direct_page, long_address, index);
    const uint32_t high = (low + 1u) & 0xffffffu;
    const bool crossed_page = (operand >> 8) != ((operand + index) >> 8);
    registers->cyclesUsed += wide + (direct_page && (registers->dp & 0xffu) != 0u) +
        (indexed && !direct_page && !long_address && (crossed_page || (!load && !registers->xf)));

    if (load) {
        const uint16_t value = Lufia2DecompReadData(registers, low, high, wide);
        if (operation == 0x89u) {
            const uint16_t sign = wide ? 0x8000u : 0x80u;
            registers->n = (value & sign) != 0u;
            registers->v = (value & (sign >> 1)) != 0u;
        }
        Lufia2DecompApplyOperand(registers,
            operation ? operation : data_x ? 0xa2u : data_y ? 0xa0u : 0xa9u, value, !wide);
    } else {
        const bool zero = opcode == 0x64u || opcode == 0x9cu || opcode == 0x9eu || opcode == 0x74u;
        const uint16_t value = zero ? 0u : data_x ? registers->x : data_y ? registers->y : registers->a;
        Lufia2DecompWriteData(registers, low, high, value, wide);
    }
    return registers->cyclesUsed;
}

static bool Lufia2DecompIsLongIndirectOpcode(uint8_t opcode) {
    return opcode == 0xa7u || opcode == 0xb7u || opcode == 0x87u || opcode == 0x97u;
}

static int Lufia2DecompRunLongIndirectOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 2u, &rom_offset);
    if (!cart || (uint16_t)(registers->dp + cart->rom[rom_offset + 1u]) > 0x1ffdu)
        return 0;

    const uint32_t program_bank = pc & 0xff0000u;
    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 6u;
    const unsigned operand = registers->read(registers->mem, program_bank | registers->pc++);
    const unsigned pointer = (uint16_t)(registers->dp + operand);
    registers->cyclesUsed += (registers->dp & 0xffu) != 0u;
    uint32_t address = Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
    address |= (uint32_t)registers->read(registers->mem, pointer + 2u) << 16;
    if (opcode & 0x10u)
        address = (address + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const bool wide = !registers->mf;
    registers->cyclesUsed += wide;
    if (opcode & 0x20u) {
        const uint16_t value = Lufia2DecompReadData(registers, address, high, wide);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, !wide);
    } else {
        Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    }
    return registers->cyclesUsed;
}

#define LUFIA2_REGISTER_OPCODE_CASES \
    case 0x0au: case 0x1au: case 0x2au: case 0x3au: case 0x4au: \
    case 0x6au: case 0x88u: case 0xc8u: case 0xcau: case 0xe8u: \
    case 0x18u: case 0x38u: case 0x58u: case 0x78u: case 0xb8u: \
    case 0xd8u: case 0xf8u: case 0xeau: case 0x8au: case 0x98u: \
    case 0xa8u: case 0xaau: case 0x9bu: case 0xbbu: case 0xebu: \
    case 0x1bu: case 0x3bu: case 0x5bu: case 0x7bu: case 0x9au: case 0xbau:

static bool Lufia2DecompIsRegisterOpcode(uint8_t opcode) {
    switch (opcode) {
    LUFIA2_REGISTER_OPCODE_CASES
        return true;
    default:
        return false;
    }
}


static bool Lufia2DecompRunTransferOrFlags(Interp816 *registers, uint8_t opcode) {
    uint8_t destination;
    uint16_t value;
    bool byte;
    switch (opcode) {
    case 0x18u: case 0x38u:
        registers->c = opcode == 0x38u;
        return true;
    case 0x58u: case 0x78u:
        registers->i = opcode == 0x78u;
        return true;
    case 0xb8u:
        registers->v = false;
        return true;
    case 0xd8u: case 0xf8u:
        registers->d = opcode == 0xf8u;
        return true;
    case 0xeau:
        return true;
    case 0xebu:
        registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
        registers->n = (registers->a & 0x80u) != 0u;
        registers->z = (uint8_t)registers->a == 0u;
        return true;
    case 0x1bu: case 0x9au:
        registers->sp = opcode == 0x1bu ? registers->a : registers->x;
        return true;
    case 0x5bu:
        registers->dp = registers->a;
        registers->n = (registers->dp & 0x8000u) != 0u;
        registers->z = registers->dp == 0u;
        return true;
    case 0x3bu: case 0x7bu:
        destination = 0xa9u;
        value = opcode == 0x3bu ? registers->sp : registers->dp;
        byte = false;
        break;
    case 0xbau:
        destination = 0xa2u;
        value = registers->sp;
        byte = registers->xf;
        break;
    case 0x8au: case 0x98u:
        destination = 0xa9u;
        value = opcode == 0x8au ? registers->x : registers->y;
        byte = registers->mf;
        break;
    case 0xaau: case 0xa8u:
        destination = opcode == 0xaau ? 0xa2u : 0xa0u;
        value = registers->a;
        byte = registers->xf;
        break;
    case 0x9bu: case 0xbbu:
        destination = opcode == 0xbbu ? 0xa2u : 0xa0u;
        value = opcode == 0xbbu ? registers->y : registers->x;
        byte = registers->xf;
        break;
    default:
        return false;
    }
    Lufia2DecompApplyOperand(registers, destination, byte ? (uint8_t)value : value, byte);
    return true;
}

static LUFIA2_ROM_NOINLINE int Lufia2DecompRunRegisterOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    unsigned offset;
    if (!Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset))
        return 0;
    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = opcode == 0xebu ? 3u : 2u;
    if (Lufia2DecompRunTransferOrFlags(registers, opcode))
        return registers->cyclesUsed;

    const bool index_x = opcode == 0xe8u || opcode == 0xcau;
    const bool index_y = opcode == 0xc8u || opcode == 0x88u;
    const bool byte = index_x || index_y ? registers->xf : registers->mf;
    const uint16_t mask = byte ? 0xffu : 0xffffu;
    const uint16_t sign = byte ? 0x80u : 0x8000u;
    uint16_t *destination = index_x ? &registers->x : index_y ? &registers->y : &registers->a;
    const uint16_t value = *destination & mask;

    uint16_t result = value;
    switch (opcode) {
    case 0x1au: case 0xe8u: case 0xc8u:
        ++result;
        break;
    case 0x3au: case 0xcau: case 0x88u:
        --result;
        break;
    case 0x0au: case 0x2au:
        result = (uint16_t)((value << 1) | (opcode == 0x2au && registers->c));
        registers->c = (value & sign) != 0u;
        break;
    case 0x4au: case 0x6au:
        result = (uint16_t)((value >> 1) | ((opcode == 0x6au && registers->c) ? sign : 0u));
        registers->c = (value & 1u) != 0u;
        break;
    }
    result &= mask;
    *destination = result | (index_x || index_y ? 0u : *destination & ~mask);
    registers->n = (result & sign) != 0u;
    registers->z = result == 0u;
    return registers->cyclesUsed;
}

#define LUFIA2_STACK_OPCODE_CASES \
    case 0x08u: case 0x0bu: case 0x28u: case 0x2bu: case 0x48u: \
    case 0x4bu: case 0x5au: case 0x68u: case 0x7au: case 0x8bu: \
    case 0xabu: case 0xdau: case 0xfau:

static bool Lufia2DecompIsRegisterOrStackOpcode(uint8_t opcode) {
    switch (opcode) {
    LUFIA2_STACK_OPCODE_CASES
    LUFIA2_REGISTER_OPCODE_CASES
        return true;
    default:
        return false;
    }
}

static bool Lufia2DecompIsStackOpcode(uint8_t opcode) {
    switch (opcode) {
    LUFIA2_STACK_OPCODE_CASES
        return true;
    default:
        return false;
    }
}

static int Lufia2DecompRunStackOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    unsigned offset;
    if (!Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset))
        return 0;
    const bool push = opcode == 0x08u || opcode == 0x0bu || opcode == 0x48u ||
        opcode == 0x4bu || opcode == 0x5au || opcode == 0x8bu || opcode == 0xdau;
    const bool index_x = opcode == 0xdau || opcode == 0xfau;
    const bool index_y = opcode == 0x5au || opcode == 0x7au;
    const bool accumulator = opcode == 0x48u || opcode == 0x68u;
    const bool wide = opcode == 0x0bu || opcode == 0x2bu ||
        ((index_x || index_y) && !registers->xf) || (accumulator && !registers->mf);
    const unsigned bytes = wide ? 2u : 1u;
    if (registers->sp < 0x1f00u || registers->sp > 0x1fffu ||
        (push ? registers->sp < 0x1f00u + bytes : registers->sp > 0x1fffu - bytes))
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = (push ? 3u : 4u) + wide;

    if (push) {
        uint16_t value;
        switch (opcode) {
        case 0x08u: value = interp816_getFlags(registers); break;
        case 0x0bu: value = registers->dp; break;
        case 0x4bu: value = registers->k; break;
        case 0x5au: value = registers->y; break;
        case 0x8bu: value = registers->db; break;
        case 0xdau: value = registers->x; break;
        default: value = registers->a; break;
        }
        if (wide) {
            registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
            --registers->sp;
        }
        registers->write(registers->mem, registers->sp, (uint8_t)value);
        --registers->sp;
        return registers->cyclesUsed;
    }

    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    switch (opcode) {
    case 0x28u:
        interp816_setFlags(registers, (uint8_t)value);
        break;
    case 0x2bu:
        registers->dp = value;
        registers->n = (value & 0x8000u) != 0u;
        registers->z = value == 0u;
        break;
    case 0xabu:
        registers->db = (uint8_t)value;
        registers->n = (value & 0x80u) != 0u;
        registers->z = value == 0u;
        break;
    default:
        Lufia2DecompApplyOperand(registers, index_x ? 0xa2u : index_y ? 0xa0u : 0xa9u, value, !wide);
        break;
    }
    return registers->cyclesUsed;
}

static bool Lufia2DecompIsNmiRomOpcode(uint8_t opcode) {
    return (opcode & 0x1fu) == 0x10u || opcode == 0xc2u || opcode == 0xe2u ||
        opcode == 0x80u || opcode == 0x82u || opcode == 0x4cu || opcode == 0x5cu ||
        Lufia2DecompIsImmediateDataOpcode(opcode) || Lufia2DecompIsRegisterOrStackOpcode(opcode) ||
        Lufia2DecompIsMemoryOpcode(opcode) || opcode == 0x60u || opcode == 0x6bu;
}

/* Keeping the dispatch out of line avoids growing the interpreter loop. */
static LUFIA2_ROM_NOINLINE int Lufia2DecompRunNativeRomControl(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    if ((opcode & 0x1fu) == 0x10u || opcode == 0xc2u || opcode == 0xe2u ||
        opcode == 0x80u || opcode == 0x82u || opcode == 0x4cu || opcode == 0x5cu ||
        Lufia2DecompIsImmediateDataOpcode(opcode))
        return Lufia2DecompRunRomImmediateOpcode(registers, pc, opcode);
    if (Lufia2DecompIsMemoryOpcode(opcode)) {
        if (Lufia2DecompIsMemoryModifyOpcode(opcode))
            return Lufia2DecompRunMemoryModifyOpcode(registers, pc, opcode);
        if (Lufia2DecompIsLongIndirectOpcode(opcode))
            return Lufia2DecompRunLongIndirectOpcode(registers, pc, opcode);
        return Lufia2DecompRunMemoryOpcode(registers, pc, opcode);
    }
    if (Lufia2DecompIsRegisterOpcode(opcode))
        return Lufia2DecompRunRegisterOpcode(registers, pc, opcode);
    if (Lufia2DecompIsStackOpcode(opcode))
        return Lufia2DecompRunStackOpcode(registers, pc, opcode);
    return Lufia2DecompRunAotCallSite(registers, pc, opcode);
}

#undef LUFIA2_MEMORY_MODIFY_CASES
#undef LUFIA2_ROM_NOINLINE
#undef LUFIA2_REGISTER_OPCODE_CASES
#undef LUFIA2_STACK_OPCODE_CASES
