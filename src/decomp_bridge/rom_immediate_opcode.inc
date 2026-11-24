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

static uint32_t Lufia2DecompMemoryOperandAddress(const Interp816 *registers,
    unsigned operand, bool direct_page, bool long_address, unsigned index) {
    if (direct_page)
        return (uint16_t)(registers->dp + operand + index);
    const uint32_t base = long_address ? operand : ((uint32_t)registers->db << 16) | operand;
    return (base + index) & 0xffffffu;
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

static bool Lufia2DecompIsRamBlockMove(uint32_t pc) {
    return pc == 0x83057du || pc == 0x86057du;
}

static uint32_t Lufia2DecompStackLongReturn(const uint8_t *bytes) {
    return bytes[0] | (uint32_t)bytes[1] << 8 | (uint32_t)bytes[2] << 16;
}

/* Match the saved return frames through the display transition's DMA calls. */
static bool Lufia2DecompDisplayTimedCall(uint32_t site, const Interp816 *registers) {
    unsigned depth;
    uint32_t display_return;
    switch (site) {
    case 0x859bf1u: depth = 3; display_return = 0x8195b1u; break;
    case 0x859c1fu: depth = 3; display_return = 0x8195efu; break;
    case 0x859c36u: depth = 3; display_return = 0x8195fbu; break;
    case 0x859c4du: depth = 3; display_return = 0x81962du; break;
    case 0x858a64u: case 0x858a68u: case 0x858a6cu:
    case 0x858a70u: case 0x858a74u: case 0x858a7du:
        depth = 3; display_return = 0x8195abu; break;
    case 0x85ec8bu: depth = 3; display_return = 0x8195c1u; break;
    case 0x81b5c6u: depth = 5; display_return = 0x8195c1u; break;
    case 0x81b62fu: depth = 6; display_return = 0x8195c1u; break;
    case 0x85ec81u: depth = 3; display_return = 0; break;
    case 0x85919cu: depth = 6; display_return = 0; break;
    default:
        return false;
    }

    const CpuState *cpu = registers->mem;
    if (!cpu || !cpu->ram || registers->e || registers->sp > 0x1fffu - depth - 3u)
        return false;

    const uint8_t *stack = cpu->ram + registers->sp + 1u;
    if (site == 0x81b5c6u || site == 0x81b62fu) {
        if (Lufia2DecompStackLongReturn(stack + depth - 3u) != 0x85ec8eu)
            return false;
    } else if (site == 0x85919cu) {
        if (Lufia2DecompStackLongReturn(stack + 3) != 0x85ec84u)
            return false;
    }

    const uint32_t caller = Lufia2DecompStackLongReturn(stack + depth);
    if (display_return)
        return caller == display_return;
    return caller == 0x8195c1u || caller == 0x8195f5u ||
           caller == 0x819601u || caller == 0x819633u;
}

/* The guest return remains on the stack across menu waits and snapshot loads. */
static bool Lufia2DecompMainMenuNeedsTimedCall(const Interp816 *registers) {
    const CpuState *cpu = registers->mem;
    if (!cpu || !cpu->ram || registers->e || !g_snes ||
        g_snes->inNmi || g_snes->inIrq ||
        registers->sp < 0x1c00u || registers->sp > 0x1ffcu)
        return false;

    for (unsigned slot = registers->sp + 1u; slot <= 0x1ffdu; ++slot) {
        const uint32_t caller = Lufia2DecompStackLongReturn(cpu->ram + slot);
        if (caller == 0x8eb014u || caller == 0x869b05u)
            return true;
    }
    return false;
}

static bool Lufia2DecompNeedsTimedCall(uint32_t site, uint32_t target,
                                      const Interp816 *registers) {
    if (target == 0x85972eu || target == 0x859790u)
        return true;

    if (target == 0x80e566u ||
        (site >= 0x80e566u && site <= 0x80e59du) ||
        (site >= 0x80e78du && site <= 0x80e7ddu) ||
        (site >= 0x80e8adu && site <= 0x80e8b8u) ||
        site == 0x80e15au || site == 0x80e458u || site == 0x80e46bu ||
        site == 0x80cd88u || site == 0x83f9a9u)
        return true;

    if (target == 0x818351u || site == 0x818394u ||
        (site >= 0x81839fu && site < 0x8184fcu) || site == 0x818504u || site == 0x818508u)
        return true;

    if (target == 0x838d42u || (site >= 0x838d42u && site <= 0x838dd9u) ||
        site == 0x838e09u || site == 0x838e1au)
        return true;

    if (target == 0x83e0fcu || site == 0x83ed4bu || site == 0x83e13du ||
        site == 0x83e1ccu || site == 0x83f234u)
        return true;

    if (target == 0x839e31u || (site >= 0x839013u && site <= 0x839f3eu) ||
        (site >= 0x83b5d3u && site <= 0x83b66du) ||
        site == 0x8082d7u || site == 0x80eca8u || site == 0x80ecacu || site == 0x80ecc0u)
        return true;

    if (site == 0x8eb019u || site == 0x838637u || site == 0x83863au ||
        (site >= 0x83863eu && site <= 0x83866du) ||
        site == 0x83b154u || site == 0x80efafu || site == 0x80f058u ||
        site == 0x80f24bu || site == 0x80f2e5u || site == 0x83a770u ||
        site == 0x83a78eu || site == 0x83a9c3u || site == 0x83a9c7u ||
        site == 0x83aa5fu || site == 0x83aa6eu || site == 0x83aa77u ||
        site == 0x83a7aeu || site == 0x83a7b1u || site == 0x838e6cu ||
        site == 0x80f4e0u || site == 0x80f72cu || site == 0x80ee30u ||
        site == 0x80ee75u || site == 0x83a461u || site == 0x83a4fau || site == 0x83a4fdu)
        return true;

    if (site == 0x8eb026u || site == 0x80e72du || site == 0x869b08u ||
        site == 0x869b0cu || site == 0x869b13u || site == 0x869b29u ||
        site == 0x869c52u || site == 0x869f14u || site == 0x869c5cu ||
        site == 0x869c65u)
        return true;

    if (site == 0x869aebu || site == 0x869aeeu || site == 0x869af1u)
        return true;

    if (site == 0x86d39cu || site == 0x86d32du || site == 0x86d33du) {
        const CpuState *cpu = registers->mem;
        if (cpu && cpu->ram && !registers->e &&
            registers->sp >= 0x1f00u && registers->sp <= 0x1ff9u) {
            const uint8_t *frames = cpu->ram + registers->sp + 3u;
            const uint8_t return_low = site == 0x86d39cu ? 0xf0u : 0xf3u;
            if (frames[0] == return_low && frames[1] == 0x9au &&
                frames[2] == 0x7eu && frames[3] == 0x92u)
                return true;
        }
    }

    if (site == 0x869b24u || site == 0x869b2cu)
        return true;

    switch (site) {
    case 0x80954bu: case 0x80954eu: case 0x80be1au: case 0x80be20u:
    case 0x8692a1u: case 0x8692a4u: case 0x8692b2u: case 0x8692b5u:
    case 0x8692b8u: case 0x8692c4u: case 0x8692c7u: case 0x8692cau:
    case 0x8692cdu: case 0x8692d0u: case 0x869748u: case 0x86995bu:
    case 0x869b30u: case 0x869b35u: case 0x869b3au: case 0x869b3fu:
    case 0x869b42u: case 0x869b45u: case 0x869b48u: case 0x869b4du:
    case 0x869b50u: case 0x869b53u: case 0x869b56u: case 0x869b62u:
    case 0x869b65u: case 0x869b68u: case 0x869b6bu: case 0x869b6eu:
    case 0x869b73u: case 0x869b78u: case 0x869b84u: case 0x869b87u:
    case 0x869b8au: case 0x869b95u: case 0x869ba0u: case 0x869ba3u:
    case 0x869ba9u: case 0x869bb2u: case 0x869bb5u: case 0x869bb8u:
    case 0x869bc1u: case 0x869bd7u: case 0x869c07u: case 0x869c0fu:
    case 0x869c2du: case 0x86a2b5u: case 0x86a45eu: case 0x86ac73u:
    case 0x86ac76u: case 0x86ace4u: case 0x86ada7u: case 0x86adb0u:
    case 0x86adeau: case 0x86ae5cu: case 0x86cbbfu: case 0x86cc63u:
    case 0x86cc66u: case 0x86cc70u: case 0x86cc79u: case 0x86cc8du:
    case 0x86cce4u: case 0x86cce7u: case 0x86cceau: case 0x86ccedu:
    case 0x86ccf0u: case 0x86ccf8u: case 0x86cd0du: case 0x86cd22u:
    case 0x86cd26u: case 0x86cd3cu: case 0x86cd41u: case 0x86cd4au:
    case 0x86cd52u: case 0x86cd58u: case 0x86cd5bu: case 0x86cd5eu:
    case 0x86d34au: case 0x86d356u: case 0x86d359u: case 0x86d378u:
    case 0x86d39cu: case 0x86d39fu: case 0x86e140u: case 0x86e1b9u:
    case 0x86e1bcu: case 0x86e1f5u: case 0x86e1ffu: case 0x86e202u:
    case 0x86e26bu: case 0x86e287u: case 0x86e3b9u: case 0x86e3ceu:
    case 0x86e3dbu: case 0x86e3feu: case 0x86e413u: case 0x86e418u:
    case 0x86e4dbu: case 0x86e5b4u: case 0x86e5b7u: case 0x86e5ceu: {
        /* The menu return and its caller distinguish these calls from map entry. */
        const CpuState *cpu = registers->mem;
        if (!cpu || !cpu->ram || registers->e || registers->sp < 0x1f00u)
            break;
        const uint8_t menu_frames[] = {0x9bu, 0x7eu, 0x92u};
        for (unsigned address = (unsigned)registers->sp + 1u; address <= 0x1ffcu; ++address) {
            const uint8_t return_low = cpu->ram[address];
            if ((return_low == 0x26u || return_low == 0x2eu) &&
                memcmp(cpu->ram + address + 1u, menu_frames, sizeof(menu_frames)) == 0)
                return true;
        }
        break;
    }
    default:
        break;
    }

    if ((target == 0x83b82fu && registers->mf && !registers->xf) ||
        site == 0x83b847u || site == 0x80b417u || site == 0x80b41cu ||
        site == 0x83fa1bu || site == 0x8eb000u || site == 0x8eb02fu)
        return true;

    /* Skip the child JSR frames and saved registers to find the menu return. */
    if (site == 0x83afddu || site == 0x83aee4u || site == 0x83aee7u) {
        const CpuState *cpu = registers->mem;
        const unsigned offset = site == 0x83afddu ? 4u : 8u;
        if (cpu && cpu->ram && !registers->e && registers->sp <= 0x1ffdu - offset &&
            Lufia2DecompStackLongReturn(cpu->ram + registers->sp + offset) == 0x8eb032u)
            return true;
    }

    if (target == 0x80f734u || target == 0x80f81cu || Lufia2DecompIsRamBlockMove(target))
        return true;

    switch (target) {
    case 0x81915bu: case 0x819169u: case 0x81917fu:
    case 0x81918fu: case 0x819198u: case 0x8191adu: case 0x8191b7u:
    case 0x81929eu: case 0x8192beu: case 0x8192e9u: case 0x8194cau:
    case 0x8194ebu: case 0x819500u: case 0x819515u: case 0x81952au:
    case 0x81953fu: case 0x819553u: case 0x819567u: case 0x81957bu:
    case 0x81958fu: case 0x81963au: case 0x819653u:
    case 0x819664u: case 0x81968au: case 0x8196c6u:
    case 0x8196f6u: case 0x8196fau:
    case 0x8196feu: case 0x819738u: case 0x81976bu: case 0x8197a5u:
    case 0x8191f2u: case 0x81920fu: case 0x819393u: case 0x81940eu:
    case 0x81986eu: case 0x819976u: case 0x819bbeu: case 0x819c76u:
    case 0x819d0bu: case 0x819d1fu: case 0x819dc9u: case 0x819e22u:
    case 0x819853u: case 0x819935u: case 0x819968u: case 0x819b56u:
    case 0x819cccu: case 0x819e13u: case 0x819f8bu:
    case 0x819999u: case 0x8199a5u: case 0x8199b0u:
    case 0x8199d0u: case 0x819a12u: case 0x819ab1u: case 0x819af7u:
    case 0x819b7eu: case 0x819ba3u: case 0x819bacu:
    case 0x819df6u:
    case 0x81a12au: case 0x81a1ccu: case 0x81a1ffu:
    case 0x81a455u: case 0x81a46eu: case 0x81a4f7u: case 0x81a512u:
    case 0x81a54au: case 0x81a5bdu: case 0x81a5d5u: case 0x81a5edu:
    case 0x81a62du: case 0x81a637u: case 0x81a64bu: case 0x81a655u:
    case 0x81a65fu: case 0x81a696u:
    case 0x819662u: case 0x8197d8u: case 0x819934u: case 0x819a54u:
    case 0x81a03au: case 0x81a04eu: case 0x81a062u: case 0x81a094u:
    case 0x81a0c4u: case 0x81a0f4u: case 0x81a162u: case 0x81a19cu:
    case 0x81a1ecu: case 0x81a1f5u: case 0x81a264u: case 0x81a295u:
    case 0x81a2b0u: case 0x81a31du: case 0x81a373u: case 0x81a388u:
    case 0x81a39du: case 0x81a3bfu: case 0x81a3d4u: case 0x81a431u:
    case 0x81a4a6u: case 0x81a4c3u: case 0x81a4dcu: case 0x81a52du:
    case 0x81a641u: case 0x81a676u:
    case 0x81a35eu: case 0x81a3e9u:
    case 0x81a40bu: case 0x81a489u: case 0x81a567u:
    case 0x81a588u: case 0x81a598u:
    case 0x81a605u: case 0x81a60fu: case 0x81a619u: case 0x81a623u:
    case 0x81a69au:
    case 0x81a6c6u: case 0x81a6f2u: case 0x81a71cu:
    case 0x81a746u: case 0x81a770u:
        return true;
    default:
        break;
    }

    if ((site >= 0x81958fu && site <= 0x819639u) ||
        Lufia2DecompDisplayTimedCall(site, registers))
        return true;

    if (site == 0x81a5aau || site == 0x81a6a9u || site == 0x81a6d5u ||
        site == 0x81a701u || site == 0x81a72bu || site == 0x81a755u || site == 0x81a77fu)
        return true;

    if (site == 0x85ddb0u || site == 0x85dde6u) {
        const CpuState *cpu = registers->mem;
        /* Skip the child's JSL frame and saved Y to find the effect call. */
        if (cpu && cpu->ram && !registers->e &&
            registers->sp >= 0x1f00u && registers->sp <= 0x1ff7u) {
            const uint32_t caller = Lufia2DecompStackLongReturn(cpu->ram + registers->sp + 6u);
            if (caller == 0x81a0acu || caller == 0x81a0dcu ||
                caller == 0x81a111u || caller == 0x81a27du ||
                caller == 0x81a2ddu || caller == 0x81a34eu ||
                caller == 0x81a5adu || caller == 0x81a6acu ||
                caller == 0x81a6d8u || caller == 0x81a704u ||
                caller == 0x81a72eu || caller == 0x81a758u || caller == 0x81a782u)
                return true;
        }
    }

    switch (site) {
    case 0x819a86u: case 0x81a08au: case 0x81a0a9u: case 0x81a0d9u:
    case 0x81a10eu: case 0x81a27au: case 0x81a2b4u: case 0x81a2dau:
    case 0x81a34bu: case 0x81a44au:
    case 0x819241u: case 0x8193a8u: case 0x8193abu: case 0x81943fu:
    case 0x819456u: case 0x819466u: case 0x819869u: case 0x819969u:
    case 0x81996cu:
    case 0x819970u: case 0x819b73u: case 0x819cf0u: case 0x819d00u:
    case 0x819d18u: case 0x819debu: case 0x819e16u: case 0x819e9du:
    case 0x819eeau: case 0x819ef9u: case 0x819f8fu: case 0x819f9fu:
        return true;
    default:
        break;
    }

    if (site == 0x819963u || site == 0x819b8au || site == 0x819b99u ||
        site == 0x819b9du || site == 0x819e02u)
        return true;

    if (site == 0x81b3bbu || site == 0x81b3e1u) {
        const CpuState *cpu = registers->mem;
        if (cpu && cpu->ram && !registers->e &&
            registers->sp >= 0x1f00u && registers->sp <= 0x1ffbu) {
            const uint8_t *caller = cpu->ram + registers->sp + 3u;
            if ((caller[0] == 0x65u && caller[1] == 0x99u) ||
                (caller[0] == 0x6bu && caller[1] == 0x98u))
                return true;
        }
    }

    switch (site) {
    case 0x858a64u: case 0x858a68u: case 0x858a6cu: case 0x858a70u:
    case 0x858a74u: case 0x858a7du: case 0x858a96u: case 0x858a9au:
    case 0x858a9eu: case 0x858aa2u: case 0x858aa6u: case 0x858aaau:
    case 0x858ba4u: case 0x858c18u: case 0x858c1eu: case 0x858d02u:
    case 0x858d08u: case 0x858db0u: case 0x81bd47u: case 0x81bdc8u: {
        const CpuState *cpu = registers->mem;
        if (!cpu || !cpu->ram || registers->e || registers->sp < 0x1f00u)
            break;
        for (unsigned address = (unsigned)registers->sp + 1u; address <= 0x1ff9u; ++address) {
            const uint8_t *caller = cpu->ram + address;
            const uint32_t return_pc = Lufia2DecompStackLongReturn(caller);
            if ((return_pc == 0x819b8du || return_pc == 0x819e05u) &&
                ((caller[5] == 0xa6u && caller[6] == 0x8au) ||
                 (caller[5] == 0x17u && caller[6] == 0x8bu)))
                return true;
        }
        break;
    }
    default:
        break;
    }

    switch (site) {
    case 0x818fc9u: case 0x81905fu: case 0x8192fbu: case 0x81932fu:
    case 0x819373u: case 0x819bb7u: case 0x81a215u:
        return true;
    default:
        break;
    }

    if (site == 0x8082aau || site == 0x81b7fdu || site == 0x81b801u ||
        site == 0x81b871u || site == 0x81b875u) {
        const CpuState *cpu = registers->mem;
        const bool multiply = site == 0x8082aau;
        const unsigned dispatch_offset =
            multiply || site == 0x81b871u || site == 0x81b875u ? 12u : 10u;
        if (cpu && cpu->ram && !registers->e && registers->sp >= 0x1f00u &&
            registers->sp <= 0x1ffeu - dispatch_offset) {
            const uint8_t *dispatch = cpu->ram + registers->sp + dispatch_offset;
            const bool effect_dispatch =
                (dispatch[0] == 0xa6u && dispatch[1] == 0x8au) ||
                (dispatch[0] == 0x17u && dispatch[1] == 0x8bu);
            if (effect_dispatch &&
                (multiply ? Lufia2DecompStackLongReturn(dispatch - 3u) == 0x81a218u
                          : dispatch[-3] == 0x4fu && dispatch[-2] == 0x8fu))
                return true;
        }
    }

    if (site == 0x85ec81u || site == 0x85ec8bu || site == 0x85919cu) {
        const CpuState *cpu = registers->mem;
        if (cpu && cpu->ram && !registers->e && registers->sp >= 0x1f00u)
            for (unsigned address = (unsigned)registers->sp + 1u; address <= 0x1ffdu; ++address)
                if (Lufia2DecompStackLongReturn(cpu->ram + address) == 0x819ba0u)
                    return true;
    }

    if (site == 0x81b871u || site == 0x81b875u) {
        const CpuState *cpu = registers->mem;
        if (cpu && cpu->ram && !registers->e &&
            registers->sp >= 0x1f00u && registers->sp <= 0x1ff4u) {
            const uint8_t *frames = cpu->ram + registers->sp;
            if (frames[6] == 0xaau && frames[7] == 0x93u &&
                ((frames[10] == 0xa6u && frames[11] == 0x8au) ||
                 (frames[10] == 0x17u && frames[11] == 0x8bu)))
                return true;
        }
    }

    if (site == 0x80954bu || site == 0x80954eu) {
        const CpuState *cpu = registers->mem;
        if (cpu && cpu->ram && !registers->e &&
            registers->sp >= 0x1f00u && registers->sp <= 0x1ff4u) {
            const uint8_t *frames = cpu->ram + registers->sp;
            const uint32_t caller = Lufia2DecompStackLongReturn(frames + 5u);
            if ((caller == 0x819e19u || caller == 0x819fa2u) &&
                ((frames[10] == 0xa6u && frames[11] == 0x8au) ||
                 (frames[10] == 0x17u && frames[11] == 0x8bu)))
                return true;
        }
    }

    switch (site) {
    case 0x81deecu: case 0x81deefu: case 0x81e5a1u: case 0x81e5aau:
    case 0x81e5bbu: case 0x81e63du: case 0x81e687u:
    case 0x859dd6u: case 0x859decu: {
        const CpuState *cpu = registers->mem;
        if (!cpu || !cpu->ram || registers->e || registers->sp < 0x1f00u)
            break;
        const bool long_return = site >> 16 == 0x85u;
        const unsigned dispatch_offset = long_return ? 5u : 4u;
        for (unsigned address = (unsigned)registers->sp + 1u;
             address <= 0x1ffeu - dispatch_offset; ++address) {
            const uint8_t *caller = cpu->ram + address;
            const uint8_t *dispatch = caller + dispatch_offset;
            if (((dispatch[0] == 0xa6u && dispatch[1] == 0x8au) ||
                 (dispatch[0] == 0x17u && dispatch[1] == 0x8bu)) &&
                (long_return ? Lufia2DecompStackLongReturn(caller) == 0x81996fu
                             : caller[0] == 0x6bu && caller[1] == 0x99u))
                return true;
        }
        break;
    }
    default:
        break;
    }

    if (site == 0x8190dfu || site == 0x819f3au) {
        const CpuState *cpu = registers->mem;
        const unsigned dispatch_offset = site == 0x8190dfu ? 3u : 11u;
        if (cpu && cpu->ram && !registers->e && registers->sp >= 0x1f00u &&
            registers->sp <= 0x1ffeu - dispatch_offset) {
            const uint8_t *dispatch = cpu->ram + registers->sp + dispatch_offset;
            const bool effect_dispatch =
                (dispatch[0] == 0xa6u && dispatch[1] == 0x8au) ||
                (dispatch[0] == 0x17u && dispatch[1] == 0x8bu);
            if (effect_dispatch && (site == 0x8190dfu ||
                (dispatch[-5] == 0x9eu && dispatch[-6] == 0x9fu) ||
                (dispatch[-5] == 0x9eu && dispatch[-6] == 0xecu) ||
                (dispatch[-5] == 0x9eu && dispatch[-6] == 0xfbu)))
                return true;
        }
    }

    if (site == 0x85b1d9u || site == 0x85b220u ||
        site == 0x85db33u || site == 0x85db54u) {
        const CpuState *cpu = registers->mem;
        const unsigned caller_offset = site == 0x85b1d9u ? 4u :
            site == 0x85b220u ? 10u : 5u;
        const unsigned dispatch_offset = caller_offset + (site < 0x85db33u ? 5u : 3u);
        if (cpu && cpu->ram && !registers->e && registers->sp >= 0x1f00u &&
            registers->sp <= 0x1ffeu - dispatch_offset) {
            const uint8_t *frames = cpu->ram + registers->sp;
            const uint8_t *dispatch = frames + dispatch_offset;
            const uint32_t caller = Lufia2DecompStackLongReturn(frames + caller_offset);
            if (((dispatch[0] == 0xa6u && dispatch[1] == 0x8au) ||
                 (dispatch[0] == 0x17u && dispatch[1] == 0x8bu)) &&
                caller == (site < 0x85db33u ? 0x819a89u : 0x81a08du))
                return true;
        }
    }

    /* Video commands need the original bus timing in their callees too. */
    switch (site) {
    case 0x85adb2u: case 0x85ae39u: case 0x85aebcu: case 0x85af2fu:
    case 0x85afb1u: case 0x85b029u: case 0x85b094u: case 0x85b0fcu:
    case 0x85b10cu: case 0x85b16eu: case 0x85b175u:
    case 0x819684u: case 0x8196f6u: case 0x8196fau:
    case 0x8199ecu: case 0x8199f2u: case 0x819a2eu: case 0x819a34u:
        return true;
    default:
        return Lufia2DecompMainMenuNeedsTimedCall(registers);
    }
}

static LUFIA2_ROM_NOINLINE int Lufia2DecompRunBlockMove(Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    const uint8_t *operands;
    if (cart) {
        operands = cart->rom + offset + 1u;
    } else {
        const CpuState *cpu = registers->mem;
        if (!Lufia2DecompIsRamBlockMove(pc) || opcode != 0x54u ||
            registers->k != (pc >> 16) || registers->pc != (uint16_t)pc ||
            registers->e || registers->waiting || registers->stopped ||
            registers->nmiWanted || registers->irqWanted ||
            !cpu || !cpu->ram || cpu->ram[(uint16_t)pc] != opcode ||
            !g_snes || !g_snes->cart)
            return 0;
        cart = g_snes->cart;
        if (cart->type != CART_LOROM || cart->superfx || cart->cx4 ||
            cart->dsp1 || cart->sdd1 || cart->sa1)
            return 0;
        operands = cpu->ram + (uint16_t)pc + 1u;
    }
    const uint32_t destination = ((uint32_t)operands[0] << 16) | registers->y;
    const uint32_t source = ((uint32_t)operands[1] << 16) | registers->x;
    if (!Lufia2DecompIsWramByte(destination) ||
        (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source)))
        return 0;

    const uint32_t program_bank = pc & 0xff0000u;
    registers->cyclesUsed = 0u;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 7u;
    const uint8_t destination_bank = registers->read(registers->mem, program_bank | registers->pc++);
    const uint8_t source_bank = registers->read(registers->mem, program_bank | registers->pc++);
    registers->db = destination_bank;
    const uint8_t value = registers->read(registers->mem, ((uint32_t)source_bank << 16) | registers->x);
    registers->write(registers->mem, ((uint32_t)destination_bank << 16) | registers->y, value);
    --registers->a;
    const int step = opcode == 0x54u ? 1 : -1;
    const unsigned mask = registers->xf ? 0xffu : 0xffffu;
    registers->x = (unsigned)(registers->x + step) & mask;
    registers->y = (unsigned)(registers->y + step) & mask;
    if (registers->a != 0xffffu)
        registers->pc = (uint16_t)pc;
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
    const uint32_t program_bank = pc & 0xff0000u;
    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = direct_page ? 3u + indexed : long_address || (indexed && !load) ? 5u : 4u;
    unsigned operand = 0u;
    for (unsigned byte = 1u; byte < length; ++byte)
        operand |= (unsigned)registers->read(registers->mem, program_bank | registers->pc++) << (8u * (byte - 1u));
    const uint32_t low = Lufia2DecompMemoryOperandAddress(registers, operand, direct_page, long_address, index);
    const uint32_t high = (low + 1u) & (direct_page ? 0xffffu : 0xffffffu);
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

static bool Lufia2DecompIsIndirectDataOpcode(uint8_t opcode) {
    const unsigned mode = opcode & 0x1fu;
    return mode == 0x01u || mode == 0x07u || mode == 0x11u || mode == 0x12u || mode == 0x17u || mode == 0x03u || mode == 0x13u;
}

static LUFIA2_ROM_NOINLINE int Lufia2DecompRunIndirectDataOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const unsigned mode = opcode & 0x1fu;
    const bool long_pointer = mode == 0x07u || mode == 0x17u;
    const bool index_x = mode == 0x01u;
    const bool index_y = mode == 0x11u || mode == 0x17u;
    const unsigned operation = opcode & 0xe0u;
    const bool load = operation != 0x80u;
    const bool stack_direct = mode == 0x03u;
    const bool stack_indirect = mode == 0x13u;
    const bool wide = !registers->mf;
    const unsigned pointer_base = stack_direct || stack_indirect ? registers->sp :
        registers->dp + (index_x ? registers->x : 0u);
    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 2u, &rom_offset);
    const unsigned last_pointer = stack_direct && !wide ? 0x1fffu : long_pointer ? 0x1ffdu : 0x1ffeu;
    if (!cart || (uint16_t)(pointer_base + cart->rom[rom_offset + 1u]) > last_pointer)
        return 0;

    const uint32_t program_bank = pc & 0xff0000u;
    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = stack_direct ? 4u : stack_indirect ? 7u :
        long_pointer || index_x || (index_y && !load) ? 6u : 5u;
    const unsigned operand = registers->read(registers->mem, program_bank | registers->pc++);
    const unsigned pointer = (uint16_t)(pointer_base + operand);
    registers->cyclesUsed += !stack_direct && !stack_indirect && (registers->dp & 0xffu) != 0u;
    const unsigned offset = stack_direct ? pointer : Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
    const uint8_t bank = stack_direct ? 0u :
        long_pointer ? registers->read(registers->mem, pointer + 2u) : registers->db;
    const unsigned index = index_y || stack_indirect ? registers->y : 0u;
    const uint32_t address = (((uint32_t)bank << 16) + offset + index) & 0xffffffu;
    if (index_y && !long_pointer) {
        const bool crossed_page = (offset >> 8) != ((offset + index) >> 8);
        registers->cyclesUsed += crossed_page || (!load && !registers->xf);
    }
    const uint32_t high = (address + 1u) & 0xffffffu;
    registers->cyclesUsed += wide;
    if (load) {
        const uint16_t value = Lufia2DecompReadData(registers, address, high, wide);
        Lufia2DecompApplyOperand(registers, (uint8_t)(operation | 0x09u), value, !wide);
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
    const bool push_address = opcode == 0xf4u;
    unsigned offset;
    if (!Lufia2DecompPlainRomOpcode(registers, pc, opcode, push_address ? 3u : 1u, &offset))
        return 0;
    const bool push = opcode == 0x08u || opcode == 0x0bu || opcode == 0x48u ||
        opcode == 0x4bu || opcode == 0x5au || opcode == 0x8bu || opcode == 0xdau || push_address;
    const bool index_x = opcode == 0xdau || opcode == 0xfau;
    const bool index_y = opcode == 0x5au || opcode == 0x7au;
    const bool accumulator = opcode == 0x48u || opcode == 0x68u;
    const bool wide = push_address || opcode == 0x0bu || opcode == 0x2bu ||
        ((index_x || index_y) && !registers->xf) || (accumulator && !registers->mf);
    const unsigned bytes = wide ? 2u : 1u;
    if (registers->sp < 0x1f00u || registers->sp > 0x1fffu ||
        (push ? registers->sp < 0x1f00u + bytes : registers->sp > 0x1fffu - bytes))
        return 0;

    registers->cyclesUsed = 0;
    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = push_address ? 5u : (push ? 3u : 4u) + wide;

    if (push) {
        uint16_t value;
        switch (opcode) {
        case 0xf4u: {
            const uint32_t bank = pc & 0xff0000u;
            value = registers->read(registers->mem, bank | registers->pc++);
            value |= (uint16_t)registers->read(registers->mem, bank | registers->pc++) << 8;
            /* Match the pinned SDK's compatibility behavior. */
            if (value == 0xc2aeu)
                registers->y = 0u;
            break;
        }
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

static int Lufia2DecompRunRomCall(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const bool long_call = opcode == 0x22u;
    const unsigned length = long_call ? 4u : 3u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || registers->sp < (long_call ? 0x1f03u : 0x1f02u))
        return 0;

    const uint32_t bank = long_call ? cart->rom[offset + 3u] : pc >> 16;
    const uint16_t address = cart->rom[offset + 1u] | ((uint16_t)cart->rom[offset + 2u] << 8);
    const uint32_t target = (bank << 16) | address;
    const bool wram_subroutine = !long_call && Lufia2DecompIsWramByte(target);
    if (!wram_subroutine &&
        (bank < 0x80u || bank > 0xbfu || address < 0x8000u ||
         (bank & 0x7fu) * 0x8000u + (address & 0x7fffu) >= cart->romSize))
        return 0;
    return Lufia2DecompRunDirectCallWithWidths(registers, pc, opcode, target,
        long_call, registers->mf, registers->xf);
}

static bool Lufia2DecompIsIndirectControlOpcode(uint8_t opcode) {
    return opcode == 0x6cu || opcode == 0x7cu || opcode == 0xdcu || opcode == 0xfcu;
}

static bool Lufia2DecompCanReadControlPointer(const Cart *cart, uint32_t address) {
    return Lufia2DecompIsWramByte(address) ||
        Lufia2DecompIsLoRomDataByte(cart, address | 0x800000u);
}

static LUFIA2_ROM_NOINLINE int Lufia2DecompRunIndirectControlOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const bool far = opcode == 0xdcu;
    const bool call = opcode == 0xfcu;
    const bool indexed = opcode == 0x7cu || call;
    const uint32_t program_bank = pc & 0xff0000u;
    const uint32_t pointer_bank = indexed ? program_bank : 0u;
    const unsigned index = indexed ? registers->x : 0u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || (call && (registers->sp < 0x1f02u || registers->sp > 0x1fffu)))
        return 0;

    const unsigned expected_operand = cart->rom[offset + 1u] | ((unsigned)cart->rom[offset + 2u] << 8);
    const unsigned expected_pointer = expected_operand + index;
    for (unsigned byte = 0u; byte < (far ? 3u : 2u); ++byte)
        if (!Lufia2DecompCanReadControlPointer(cart, pointer_bank | (uint16_t)(expected_pointer + byte)))
            return 0;

    registers->cyclesUsed = 0u;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = call ? 8u : opcode == 0x6cu ? 5u : 6u;
    unsigned pointer = registers->read(registers->mem, program_bank | registers->pc++);
    pointer |= (unsigned)registers->read(registers->mem, program_bank | registers->pc++) << 8;
    pointer += index;
    const uint16_t destination = Lufia2DecompReadData(registers, pointer_bank | (uint16_t)pointer,
        pointer_bank | (uint16_t)(pointer + 1u), true);
    if (call) {
        const uint16_t return_address = (uint16_t)(registers->pc - 1u);
        registers->write(registers->mem, registers->sp, (uint8_t)(return_address >> 8));
        --registers->sp;
        registers->write(registers->mem, registers->sp, (uint8_t)return_address);
        --registers->sp;
    }
    registers->pc = destination;
    if (far)
        registers->k = registers->read(registers->mem, (uint16_t)(pointer + 2u));
    return registers->cyclesUsed;
}

static bool Lufia2DecompIsNmiRomOpcode(uint8_t opcode) {
    return (opcode & 0x1fu) == 0x10u || opcode == 0xc2u || opcode == 0xe2u ||
        opcode == 0x80u || opcode == 0x82u || opcode == 0x4cu || opcode == 0x5cu ||
        Lufia2DecompIsImmediateDataOpcode(opcode) || Lufia2DecompIsRegisterOrStackOpcode(opcode) ||
        Lufia2DecompIsMemoryOpcode(opcode) || opcode == 0x60u || opcode == 0x6bu ||
        opcode == 0x20u || opcode == 0x22u || Lufia2DecompIsIndirectControlOpcode(opcode);
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
    const int aot_cycles = Lufia2DecompRunAotCallSite(registers, pc, opcode);
    if (aot_cycles || (opcode != 0x20u && opcode != 0x22u))
        return aot_cycles;
    return Lufia2DecompRunRomCall(registers, pc, opcode);
}

static LUFIA2_ROM_NOINLINE int Lufia2DecompRunAdditionalRomOpcode(Interp816 *registers,
    uint32_t pc, uint8_t opcode) {
    if (Lufia2DecompIsIndirectControlOpcode(opcode))
        return Lufia2DecompRunIndirectControlOpcode(registers, pc, opcode);
    if (opcode == 0x44u || opcode == 0x54u)
        return Lufia2DecompRunBlockMove(registers, pc, opcode);
    if (opcode == 0xf4u)
        return Lufia2DecompRunStackOpcode(registers, pc, opcode);
    if (Lufia2DecompIsIndirectDataOpcode(opcode))
        return Lufia2DecompRunIndirectDataOpcode(registers, pc, opcode);
    if (opcode != 0x00u || !registers->brkHookEnabled)
        return 0;
    unsigned rom_offset;
    if (!Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &rom_offset))
        return 0;
    /* The bridge's BRK marker consumes only the opcode byte. */
    registers->cyclesUsed = 0u;
    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = 7u;
    return registers->cyclesUsed;
}

#undef LUFIA2_MEMORY_MODIFY_CASES
#undef LUFIA2_ROM_NOINLINE
#undef LUFIA2_REGISTER_OPCODE_CASES
#undef LUFIA2_STACK_OPCODE_CASES
