static int Lufia2DecompRunBattleEffectYield(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x819169u || opcode != 0xa9u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->y > 0xfffbu || registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 40u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 22u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x7e\x48\xab\xc2\x20\xa5\xc3\x99\x03\x00"
        "\xe2\x20\xb9\x02\x00\x99\x01\x00\x4c\x58\x8c", 21u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    const uint16_t continuation = Lufia2DecompReadData(registers, 0xc3u, 0xc4u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, continuation, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const uint32_t effect = 0x7e0000u + registers->y;
    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    Lufia2DecompWriteData(registers, effect + 3u, effect + 4u, continuation, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    const uint8_t delay = registers->read(registers->mem, effect + 2u);
    Lufia2DecompApplyOperand(registers, 0xa9u, delay, true);
    const unsigned read_cycles = 4u + (registers->y >= 0xfeu);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 16u, 3u, true);
    registers->write(registers->mem, effect + 1u, delay);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 19u, 3u, true);
    registers->pc = 0x8c58u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 10 - progress : -9;
    return 35u + read_cycles;
}

static int Lufia2DecompRunBattleEffectDispatchReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool pull_index = pc == 0x818c58u;
    const unsigned cycles = pull_index ? 5u : 6u;
    const bool repeat_return = pc == 0x819552u || pc == 0x819566u ||
        pc == 0x81957au || pc == 0x81958eu ||
        pc == 0x81a128u || pc == 0x81a161u || pc == 0x81a6c5u ||
        pc == 0x818faau || pc == 0x818fc4u;
    if ((!pull_index && pc != 0x818c59u && !repeat_return) || opcode != (pull_index ? 0xfau : 0x60u) ||
        registers->e || (pull_index && registers->xf) ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;

    unsigned offset;
    if (!Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    uint16_t word = registers->read(registers->mem, ++registers->sp);
    word |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    if (pull_index) {
        Lufia2DecompApplyOperand(registers, 0xa2u, word, false);
        registers->pc = 0x8c59u;
    } else {
        registers->pc = (uint16_t)(word + 1u);
    }
    registers->cyclesUsed = cycles;
    Lufia2SpanAccountInstruction(cpu, cycles);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return cycles;
}

static int Lufia2DecompRunBattleEffectRepeat(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    static const uint16_t entries[] = {0x953fu, 0x9553u, 0x9567u, 0x957bu};
    unsigned slot = 0;
    while (slot < sizeof(entries) / sizeof(entries[0]) && pc != (0x810000u | entries[slot]))
        ++slot;
    if (slot == sizeof(entries) / sizeof(entries[0]) || opcode != 0xa9u || registers->e ||
        !registers->mf || registers->xf || registers->dp || registers->y > 0xffedu ||
        registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 37u)
        return 0;

    unsigned offset;
    const uint8_t counter = 7u + slot * 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 20u, &offset);
    const uint8_t signature[] = {
        0x7e, 0x48, 0xab, 0xbb, 0xde, counter, 0x00, 0xf0, 0x09, 0xc2,
        0x20, 0xb9, counter + 1u, 0x00, 0x85, 0xc3, 0xe2, 0x20, 0x60,
    };
    if (!cart || memcmp(cart->rom + offset + 1u, signature, sizeof(signature)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    const uint32_t counter_address = 0x7e0000u + registers->x + counter;
    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    Lufia2DecompModifyWram(registers, 0xdeu, counter_address, counter_address + 1u, false);
    Lufia2SpanAccountInstruction(cpu, 8u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    if (registers->z) {
        registers->pc = (uint16_t)(pc + 19u);
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 6 - progress : -5;
        return 22;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 12u, 3u, true);
    const uint16_t continuation = Lufia2DecompReadData(registers, counter_address + 1u, counter_address + 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, continuation, false);
    const unsigned read_cycles = 5u + (counter + 1u + registers->y >= 0x100u);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    Lufia2DecompWriteData(registers, 0xc3u, 0xc4u, continuation, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    registers->mf = true;
    registers->pc = (uint16_t)(pc + 19u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 10 - progress : -9;
    return 31u + read_cycles;
}

static int Lufia2DecompRunBattleEffectStreamWord(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81a12au || opcode != 0xa9u || registers->e || !registers->mf || registers->xf ||
        registers->dp || !cpu->ram || cpu->ram != g_snes->ram ||
        registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 39u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x7e\x48\xab\xc2\x20\xa7\xc3\xe6\xc3\xe6\xc3\xe2\x20\x85\x00", 15u))
        return 0;
    const uint32_t pointer = cpu->ram[0xc3u] | ((uint32_t)cpu->ram[0xc4u] << 8) |
        ((uint32_t)cpu->ram[0xc5u] << 16);
    const uint32_t high = (pointer + 1u) & 0xffffffu;
    if (!(Lufia2DecompIsWramByte(pointer) && Lufia2DecompIsWramByte(high)) &&
        !(Lufia2DecompIsLoRomDataByte(cart, pointer) && Lufia2DecompIsLoRomDataByte(cart, high)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    uint32_t address = Lufia2DecompReadData(registers, 0xc3u, 0xc4u, true);
    address |= (uint32_t)registers->read(registers->mem, 0xc5u) << 16;
    const uint16_t word = Lufia2DecompReadData(registers, address, (address + 1u) & 0xffffffu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, word, false);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned byte = 0; byte < 2u; ++byte) {
        Lufia2SpanFetch(registers, pc + 8u + byte * 2u, 2u, true);
        Lufia2DecompModifyWram(registers, 0xe6u, 0xc3u, 0xc4u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 6u + byte;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->write(registers->mem, 0x00u, (uint8_t)word);
    registers->pc = 0xa13au;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 9 - progress : -8;
    return 39;
}

static int Lufia2DecompRunBattleEffectPosition(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool apply_displacement = pc == 0x81a6adu;
    if ((!apply_displacement && pc != 0x81a112u) || opcode != 0xc2u || registers->e || registers->xf ||
        registers->dp || registers->db != 0x7eu || registers->y > 0xffe1u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (apply_displacement ? 51u : 44u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, apply_displacement ? 24u : 22u, &offset);
    if (!cart || (apply_displacement ? memcmp(cart->rom + offset + 1u,
        "\x20\xe6\xc3\xb9\x1b\x00\x18\x65\x56\x99\x1b\x00\xb9\x1d\x00\x18\x65\x58\x99\x1d\x00\xe2\x20", 23u) :
        memcmp(cart->rom + offset + 1u,
        "\x20\xb9\x1b\x00\x38\xe5\x56\x99\x1b\x00\xb9\x1d\x00\x38\xe5\x58\x99\x1d\x00\xe2\x20", 21u)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    unsigned cycles = 6u;
    long progress = 0;
    if (apply_displacement) {
        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        Lufia2DecompModifyWram(registers, 0xe6u, 0xc3u, 0xc4u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        cycles += 7u;
    }
    const unsigned axis_start = apply_displacement ? 4u : 2u;
    const long first_step = apply_displacement ? 3 : 2;
    for (unsigned axis = 0; axis < 2u; ++axis) {
        const uint32_t entry = pc + axis_start + axis * 9u;
        const unsigned field = 0x1bu + axis * 2u;
        const uint32_t address = 0x7e0000u + registers->y + field;
        const uint32_t origin = 0x56u + axis * 2u;
        const unsigned read_cycles = 5u + (registers->y + field >= 0x100u);
        const long step = first_step + axis * 4u;

        Lufia2SpanFetch(registers, entry, 3u, true);
        const uint16_t position = Lufia2DecompReadData(registers, address, address + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, position, false);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = step;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, entry + 3u, 1u, true);
        registers->c = !apply_displacement;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, entry + 4u, 2u, true);
        const uint16_t base = Lufia2DecompReadData(registers, origin, origin + 1u, true);
        Lufia2DecompApplyOperand(registers, apply_displacement ? 0x69u : 0xe9u, base, false);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = step + 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, entry + 6u, 3u, true);
        Lufia2DecompWriteData(registers, address, address + 1u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = step + 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        cycles += read_cycles + 13u;
    }

    Lufia2SpanFetch(registers, pc + axis_start + 18u, 2u, true);
    registers->mf = true;
    registers->pc = (uint16_t)(pc + axis_start + 20u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    const long instruction_count = apply_displacement ? 11 : 10;
    *tail_steps = progress ? instruction_count - progress : 1 - instruction_count;
    return cycles;
}

static int Lufia2DecompRunBattleEffectStreamParameter(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81a100u || opcode != 0xc2u || registers->e || registers->dp ||
        !cpu->ram || cpu->ram != g_snes->ram || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 38u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 14u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xe6\xc3\xa7\xc3\x85\x5a\xe6\xc3\xe6\xc3\xe2\x20", 13u))
        return 0;

    const uint16_t cursor = cpu->ram[0xc3u] | ((uint16_t)cpu->ram[0xc4u] << 8);
    const uint32_t source = ((uint32_t)cpu->ram[0xc5u] << 16) | (uint16_t)(cursor + 1u);
    const uint32_t high = (source + 1u) & 0xffffffu;
    if (!(Lufia2DecompIsWramByte(source) && Lufia2DecompIsWramByte(high)) &&
        !(Lufia2DecompIsLoRomDataByte(cart, source) && Lufia2DecompIsLoRomDataByte(cart, high)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xe6u, 0xc3u, 0xc4u, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    uint32_t address = Lufia2DecompReadData(registers, 0xc3u, 0xc4u, true);
    address |= (uint32_t)registers->read(registers->mem, 0xc5u) << 16;
    const uint16_t parameter = Lufia2DecompReadData(registers, address, (address + 1u) & 0xffffffu, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, parameter, false);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompWriteData(registers, 0x5au, 0x5bu, parameter, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned byte = 0; byte < 2u; ++byte) {
        Lufia2SpanFetch(registers, pc + 8u + byte * 2u, 2u, true);
        Lufia2DecompModifyWram(registers, 0xe6u, 0xc3u, 0xc4u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 5u + byte;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->mf = true;
    registers->pc = 0xa10eu;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 7 - progress : -6;
    return 38;
}

static int Lufia2DecompRunBattleEffectPackedFields(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t destination = ((uint32_t)registers->db << 16) + registers->y;
    if (pc != 0x81a13au || opcode != 0x29u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->y > 0xffd4u ||
        !Lufia2DecompIsWramByte(destination + 0x2bu) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 66u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 39u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xc0\x99\x2b\x00\xa5\x00\x29\x01\x99\x2a\x00\xa5\x00\x29\x0e\x99\x28\x00"
        "\xa5\x00\x4a\x4a\x4a\x4a\x29\x03\x99\x29\x00\xeb\x99\x27\x00\xa9\x01\x99\x26\x00", 38u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xc0u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    registers->write(registers->mem, destination + 0x2bu, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    static const uint8_t masks[] = {1u, 0x0eu};
    static const uint8_t fields[] = {0x2au, 0x28u};
    for (unsigned index = 0; index < 2u; ++index) {
        const uint32_t entry = pc + 5u + index * 7u;
        const long step = 3 + index * 3u;
        Lufia2SpanFetch(registers, entry, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0u), true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = step;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, entry + 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x29u, masks[index], true);
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, entry + 4u, 3u, true);
        registers->write(registers->mem, destination + fields[index], (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 6u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = step + 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0u), true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    for (unsigned shift = 0; shift < 4u; ++shift) {
        Lufia2SpanFetch(registers, pc + 21u + shift, 1u, true);
        registers->c = (registers->a & 1u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)registers->a >> 1, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, pc + 25u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 3u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 27u, 3u, true);
    registers->write(registers->mem, destination + 0x29u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 15;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 30u, 1u, true);
    MenuGlyphExchangeBytes(registers);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, pc + 31u, 3u, true);
    registers->write(registers->mem, destination + 0x27u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 17;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 34u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 1u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 36u, 3u, true);
    registers->write(registers->mem, destination + 0x26u, 1u);
    registers->pc = 0xa161u;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 19;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 19 - progress : -18;
    return 66;
}

static int Lufia2DecompRunBattleEffectDisplacementSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool add_direction = pc == 0x81a0f4u;
    if ((!add_direction && pc != 0x81a69au) || opcode != 0xa9u || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->y > 0xffdcu || registers->sp < 0x1f01u || registers->sp > 0x1fffu ||
        !cpu->ram || cpu->ram != g_snes->ram || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (add_direction ? 25u : 29u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, add_direction ? 12u : 15u, &offset);
    if (!cart || (add_direction ? memcmp(cart->rom + offset + 1u,
        "\x7e\x48\xab\xa7\xc3\x18\x79\x23\x00\x85\x54", 11u) :
        memcmp(cart->rom + offset + 1u,
        "\x7e\x48\xab\xa7\xc3\x85\x5a\x64\x5b\xb9\x23\x00\x85\x54", 14u)))
        return 0;

    const uint32_t source = cpu->ram[0xc3u] | ((uint32_t)cpu->ram[0xc4u] << 8) |
        ((uint32_t)cpu->ram[0xc5u] << 16);
    if (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    uint32_t address = Lufia2DecompReadData(registers, 0xc3u, 0xc4u, true);
    address |= (uint32_t)registers->read(registers->mem, 0xc5u) << 16;
    const uint8_t distance = registers->read(registers->mem, address);
    Lufia2DecompApplyOperand(registers, 0xa9u, distance, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (add_direction) {
        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 7u, 3u, true);
        const uint8_t direction = registers->read(registers->mem, 0x7e0023u + registers->y);
        Lufia2DecompApplyOperand(registers, 0x69u, direction, true);
        const unsigned read_cycles = 4u + (registers->y >= 0xddu);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 6;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 10u, 2u, true);
        registers->write(registers->mem, 0x54u, (uint8_t)registers->a);
        registers->pc = 0xa100u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 7;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = progress ? 7 - progress : -6;
        return 20u + read_cycles;
    }

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    registers->write(registers->mem, 0x5au, distance);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->write(registers->mem, 0x5bu, 0u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const unsigned read_cycles = 4u + (registers->y >= 0xddu);
    Lufia2SpanFetch(registers, pc + 10u, 3u, true);
    const uint8_t direction = registers->read(registers->mem, 0x7e0023u + registers->y);
    Lufia2DecompApplyOperand(registers, 0xa9u, direction, true);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->write(registers->mem, 0x54u, direction);
    registers->pc = 0xa6a9u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 8 - progress : -7;
    return 24u + read_cycles;
}

static int Lufia2DecompRunBattleEffectSetMode(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc < 0x81a605u || pc > 0x81a62du || (pc - 0x81a605u) % 10u ||
        opcode != 0xa9u || registers->e || !registers->mf || registers->xf || registers->y > 0xfffdu ||
        registers->sp < 0x1f01u || registers->sp > 0x1ffdu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    const uint8_t mode = (pc - 0x81a605u) / 10u + 1u;
    const uint8_t signature[] = {0x7eu, 0x48u, 0xabu, 0xa9u, mode, 0x99u, 0x02u, 0x00u, 0x60u};
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, signature, sizeof(signature)))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x7eu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->z = registers->db == 0u;
    registers->n = (registers->db & 0x80u) != 0u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, mode, true);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    registers->write(registers->mem, 0x7e0002u + registers->y, mode);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 1u, true);
    uint16_t return_pc = registers->read(registers->mem, ++registers->sp);
    return_pc |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->pc = (uint16_t)(return_pc + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 6 - progress : -5;
    return 23;
}

static int Lufia2DecompRunBattleRecordSearch(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool secondary = pc == 0x818faeu || pc == 0x818fb3u;
    const uint16_t search_pc = secondary ? 0x8faeu : 0x8f94u;
    const uint16_t record_size = secondary ? 44u : 45u;
    const uint16_t limit = secondary ? 0x54b7u : 0x5ff7u;
    const bool advance = pc == (0x810000u | (search_pc + 5u));
    const uint32_t address = ((uint32_t)registers->db << 16) + registers->x;
    if ((!advance && pc != (0x810000u | search_pc)) || opcode != (advance ? 0xc2u : 0xbdu) ||
        registers->e || !registers->mf || registers->xf ||
        (!advance && !Lufia2DecompIsWramByte(address)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (advance ? 21u : 28u))
        return 0;
    unsigned offset;
    const uint8_t signature[] = {
        0xbd, 0x00, 0x00, 0xf0, 0x11, 0xc2, 0x20, 0x8a, 0x18, 0x69,
        (uint8_t)record_size, 0x00, 0xaa, 0xe2, 0x20, 0xe0,
        (uint8_t)limit, (uint8_t)(limit >> 8), 0xd0, 0xec,
    };
    const unsigned first = advance ? 5u : 0u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, sizeof(signature) - first, &offset);
    if (!cart || memcmp(cart->rom + offset, signature + first, sizeof(signature) - first))
        return 0;

    unsigned cycles = 0;
    bool progress = false;
    if (!advance) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, address), true);
        const unsigned load_cycles = 4u + (registers->x >= 0x100u);
        Lufia2SpanAccountInstruction(cpu, load_cycles);
        progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        if (registers->z) {
            registers->pc = search_pc + 22u;
            registers->cyclesUsed = 3u;
            Lufia2SpanAccountInstruction(cpu, 3u);
            *tail_steps = progress ? 1 : -1;
            return load_cycles + 3u;
        }
        Lufia2SpanAccountInstruction(cpu, 2u);
        cycles = load_cycles + 2u;
    }

    const uint32_t body_pc = advance ? pc : pc + 5u;
    Lufia2SpanFetch(registers, body_pc, 2u, !advance);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, body_pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, body_pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, body_pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, record_size, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, body_pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, body_pc + 8u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, body_pc + 10u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, limit, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    Lufia2SpanFetch(registers, body_pc + 13u, 2u, true);
    registers->pc = registers->z ? search_pc + 20u : search_pc;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    const long instructions = advance ? 8 : 10;
    *tail_steps = progress ? instructions - 1 : 1 - instructions;
    return cycles + 18u + registers->cyclesUsed;
}
