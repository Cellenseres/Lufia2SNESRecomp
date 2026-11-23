static bool MenuBranchCanReadWords(const Cart *cart, const CpuState *cpu, uint16_t index) {
    const uint32_t pointer = cpu->ram[0x5du] |
        ((uint32_t)cpu->ram[0x5eu] << 8) | ((uint32_t)cpu->ram[0x5fu] << 16);
    for (unsigned word = 0; word < 2u; ++word) {
        const uint32_t low = (pointer + (uint16_t)(index + word * 2u)) & 0xffffffu;
        const uint32_t high = (low + 1u) & 0xffffffu;
        if ((!Lufia2DecompIsWramByte(low) && !Lufia2DecompIsLoRomDataByte(cart, low)) ||
            (!Lufia2DecompIsWramByte(high) && !Lufia2DecompIsLoRomDataByte(cart, high)))
            return false;
    }
    return true;
}

static uint16_t MenuBranchReadStringWord(Interp816 *registers) {
    uint32_t address = Lufia2DecompReadData(registers, 0x5du, 0x5eu, true);
    address |= (uint32_t)registers->read(registers->mem, 0x5fu) << 16;
    address = (address + registers->y) & 0xffffffu;
    return Lufia2DecompReadData(registers, address, (address + 1u) & 0xffffffu, true);
}

static int MenuBranchOperands(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808c9eu || opcode != 0xc2u || registers->e || registers->xf ||
        registers->dp || registers->db != 0x7eu || !cpu->ram ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 55u)
        return 0;

    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 27u, &rom_offset);
    if (!cart || memcmp(cart->rom + rom_offset + 1u,
        "\x20\xb7\x5d\x85\x54\xb2\x54\x85\x54\xc8\xc8\xb7\x5d\x85\x56"
        "\xc8\xc8\xe2\x20\xad\x7b\x05\x89\x08\xf0\x24", 26u))
        return 0;

    if (!MenuBranchCanReadWords(cart, cpu, registers->y))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    uint32_t instruction = pc + 2u;
    for (unsigned word = 0; word < 2u; ++word) {
        const unsigned destination = 0x54u + word * 2u;
        Lufia2SpanFetch(registers, instruction, 2u, true);
        const uint16_t operand = MenuBranchReadStringWord(registers);
        Lufia2DecompApplyOperand(registers, 0xa9u, operand, false);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 2u, 2u, true);
        Lufia2DecompWriteData(registers, destination, destination + 1u, operand, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 4u;

        if (!word) {
            Lufia2SpanFetch(registers, instruction, 2u, true);
            const uint32_t indirect = 0x7e0000u + Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
            const uint16_t argument = Lufia2DecompReadData(registers, indirect, indirect + 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa9u, argument, false);
            Lufia2SpanAccountInstruction(cpu, 6u);
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

            Lufia2SpanFetch(registers, instruction + 2u, 2u, true);
            Lufia2DecompWriteData(registers, 0x54u, 0x55u, argument, true);
            Lufia2SpanAccountInstruction(cpu, 4u);
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            instruction += 4u;
        }

        for (unsigned increment = 0; increment < 2u; ++increment) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }
    }

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 20u, 3u, true);
    const uint8_t format = registers->read(registers->mem, 0x7e057bu);
    Lufia2DecompApplyOperand(registers, 0xa9u, format, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 23u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 8u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 25u, 2u, true);
    registers->pc = registers->z ? 0x8cddu : 0x8cb9u;
    registers->cyclesUsed = registers->z ? 3u : 2u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 2 : 6;
    return 52u + registers->cyclesUsed;
}

static int MenuBranchIndex(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808cb9u || opcode != 0xc2u || registers->e || registers->dp ||
        registers->db != 0x7eu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 27u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 13u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x20\xa5\x54\x18\x65\x56\x85\x54\xb2\x54\x85\x54", 12u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint16_t pointer = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, pointer, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    const uint16_t displacement = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, displacement, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompWriteData(registers, 0x54u, 0x55u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    const uint32_t address = 0x7e0000u + Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    Lufia2DecompWriteData(registers, 0x54u, 0x55u, value, true);
    registers->pc = 0x8cc6u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = 0;
    return 27;
}

static int MenuBranchOffset(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x808cc6u || opcode != 0xb7u || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->db != 0x7eu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || !cpu->ram ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 54u)
        return 0;

    unsigned rom_offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &rom_offset);
    if (!cart || memcmp(cart->rom + rom_offset + 1u,
        "\x5d\x85\x56\xc8\xc8\xb2\x56\x18\x77\x5d\xc8\xc8"
        "\x85\x56\xb2\x56\x85\x56\xe2\x20\x80\x13", 22u))
        return 0;

    if (!MenuBranchCanReadWords(cart, cpu, registers->y))
        return 0;

    uint32_t instruction = pc;
    for (unsigned word = 0; word < 2u; ++word) {
        Lufia2SpanFetch(registers, instruction, 2u, word != 0u);
        const uint16_t operand = MenuBranchReadStringWord(registers);
        Lufia2DecompApplyOperand(registers, word ? 0x69u : 0xa9u, operand, false);
        Lufia2SpanAccountInstruction(cpu, 7u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 2u;

        if (!word) {
            Lufia2SpanFetch(registers, instruction, 2u, true);
            Lufia2DecompWriteData(registers, 0x56u, 0x57u, registers->a, true);
            Lufia2SpanAccountInstruction(cpu, 4u);
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            instruction += 2u;
        }

        for (unsigned increment = 0; increment < 2u; ++increment) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        if (word) {
            Lufia2SpanFetch(registers, instruction, 2u, true);
            Lufia2DecompWriteData(registers, 0x56u, 0x57u, registers->a, true);
            Lufia2SpanAccountInstruction(cpu, 4u);
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            instruction += 2u;
        }

        Lufia2SpanFetch(registers, instruction, 2u, true);
        const uint32_t indirect = 0x7e0000u + Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
        const uint16_t value = Lufia2DecompReadData(registers, indirect, indirect + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
        Lufia2SpanAccountInstruction(cpu, 6u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 2u;

        if (!word) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            registers->c = false;
            Lufia2SpanAccountInstruction(cpu, 2u);
        }
    }

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    Lufia2DecompWriteData(registers, 0x56u, 0x57u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 21u, 2u, true);
    registers->pc = 0x8cf0u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = 2;
    return 54;
}

static int MenuBranchFinish(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool mask_words = pc == 0x808cf0u;
    if ((!mask_words && pc != 0x808cddu) || opcode != 0xadu ||
        registers->e || !registers->mf || registers->dp || registers->db != 0x7eu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (mask_words ? 14u : 28u))
        return 0;

    unsigned offset;
    const unsigned length = mask_words ? 11u : 19u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = mask_words ? "\x7b\x05\x89\x80\xd0\x04\x64\x55\x64\x57" :
        "\x7b\x05\x89\x40\xd0\x0c\x89\x20\xf0\x08\xc2\x20\xb2\x56\x85\x56\xe2\x20";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t format = registers->read(registers->mem, 0x7e057bu);
    Lufia2DecompApplyOperand(registers, 0xa9u, format, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, mask_words ? 0x80u : 0x40u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    if (!registers->z) {
        registers->pc = mask_words ? 0x8cfbu : 0x8cf0u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 2 : -2;
        return 9;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    if (mask_words) {
        for (unsigned operand = 0; operand < 2u; ++operand) {
            Lufia2SpanFetch(registers, pc + 7u + operand * 2u, 2u, true);
            registers->write(registers->mem, 0x55u + operand * 2u, 0u);
            Lufia2SpanAccountInstruction(cpu, 3u);
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }
        registers->pc = 0x8cfbu;
        registers->cyclesUsed = 3u;
        *tail_steps = 0;
        return 14;
    }

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x20u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    if (registers->z) {
        registers->pc = 0x8cf0u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 4 : -4;
        return 13;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    const uint32_t address = 0x7e0000u + Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    Lufia2DecompWriteData(registers, 0x56u, 0x57u, value, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->mf = true;
    registers->pc = 0x8cf0u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = 1;
    return 28;
}

static int Lufia2DecompRunMenuBranch(Interp816 *registers, CpuState *cpu,
                                      uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                      uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x808c9eu:
        if (room >= 15)
            return MenuBranchOperands(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808cb9u:
        if (room >= 7)
            return MenuBranchIndex(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808cc6u:
        if (room >= 14)
            return MenuBranchOffset(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x808cddu:
    case 0x808cf0u:
        if (room >= (pc == 0x808cddu ? 9 : 5))
            return MenuBranchFinish(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    }
    return 0;
}

static int RunMenuComparisonFlow(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool advance = pc == 0x808c39u;
    if ((!advance && pc != 0x808c1cu) || opcode != (advance ? 0xc8u : 0xadu) ||
        registers->e || registers->xf || !registers->mf || registers->db != 0x7eu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (advance ? 8u : 9u))
        return 0;

    unsigned offset;
    const unsigned length = advance ? 5u : 7u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = advance ? "\xc8\x82\x6c\xfc" : "\x7b\x05\x89\x10\xd0\x0c";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (advance) {
        for (unsigned byte = 0; byte < 2u; ++byte) {
            Lufia2SpanFetch(registers, pc + byte, 1u, byte != 0u);
            Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        registers->pc = 0x88aau;
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        *tail_steps = -2;
        return 8;
    }

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t format = registers->read(registers->mem, 0x7e057bu);
    Lufia2DecompApplyOperand(registers, 0xa9u, format, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x10u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->pc = registers->z ? 0x8c23u : 0x8c2fu;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 2 : -2;
    return 6u + registers->cyclesUsed;
}

static int RunMenuCompareWords(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool equality = pc == 0x808c23u;
    const bool bit_test = pc == 0x808c2fu;
    const bool less = pc == 0x808c41u;
    if ((!equality && !bit_test && !less && pc != 0x808c51u) || opcode != 0xc2u ||
        registers->e || registers->dp || registers->db != 0x7eu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (bit_test ? 17u : 20u))
        return 0;

    unsigned offset;
    const unsigned length = bit_test ? 10u : equality ? 12u : 13u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = equality ? "\x20\xa5\x54\xc5\x56\xe2\x20\xf0\x31\x80\x0a" :
        bit_test ? "\x20\xa5\x54\x24\x56\xe2\x20\xd0\x25" :
        less ? "\x20\xa5\x54\xc5\x56\xe2\x20\x90\xee\x82\x10\x00" :
               "\x20\xa5\x54\xc5\x56\xe2\x20\xb0\xde\x82\x00\x00";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint16_t left = Lufia2DecompReadData(registers, 0x54u, 0x55u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, left, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool left_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    const uint16_t right = Lufia2DecompReadData(registers, 0x56u, 0x57u, true);
    if (bit_test) {
        registers->n = (right & 0x8000u) != 0;
        registers->v = (right & 0x4000u) != 0;
    }
    Lufia2DecompApplyOperand(registers, bit_test ? 0x89u : 0xc9u, right, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool right_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    const bool taken = equality ? registers->z : bit_test ? !registers->z : registers->c != less;
    if (taken || bit_test) {
        registers->pc = taken ? (equality || bit_test ? 0x8c5eu : 0x8c39u) : 0x8c39u;
        registers->cyclesUsed = taken ? 3u : 2u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        *tail_steps = right_progress ? 2 : left_progress ? 3 : -4;
        return 14u + registers->cyclesUsed;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 10u, equality ? 2u : 3u, true);
    registers->pc = equality ? 0x8c39u : 0x8c5eu;
    registers->cyclesUsed = equality ? 3u : 4u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = right_progress ? 3 : left_progress ? 4 : -5;
    return 16u + registers->cyclesUsed;
}

static int Lufia2DecompRunMenuComparison(Interp816 *registers, CpuState *cpu,
                                        uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                        uint64_t write_epoch, long room, long *tail_steps) {
    if (pc == 0x808c1cu || pc == 0x808c39u)
        return room >= 3
            ? RunMenuComparisonFlow(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps)
            : 0;

    return room >= (pc == 0x808c2fu ? 5 : 6)
        ? RunMenuCompareWords(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps)
        : 0;
}
