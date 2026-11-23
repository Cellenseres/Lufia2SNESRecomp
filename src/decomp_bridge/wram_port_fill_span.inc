static int Lufia2DecompRunWramPortFill(Interp816 *registers, CpuState *cpu, uint32_t pc,
                           uint8_t opcode) {
    const bool larger_fill = pc == 0x858a51u;
    if ((pc != 0x858d42u && pc != 0x858a51u) || opcode != 0x9cu || registers->e || !registers->mf ||
        registers->xf || registers->db != (larger_fill ? 0x81u : 0x85u) || registers->dp ||
        registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || !registers->y || registers->y > (larger_fill ? 0x02a0u : 0x0200u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x80\x21\x8d\x80\x21\x88\xd0\xf7", 8u))
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 13u;
    unsigned count = room < 8u ? (unsigned)room : 8u;
    if (count > registers->y)
        count = registers->y;
    if (!count)
        return 0;

    const uint32_t data_port = ((uint32_t)registers->db << 16) | 0x2180u;
    unsigned cycles = 0;
    for (unsigned i = 0; i < count; ++i) {
        Lufia2SpanFetch(registers, pc, 3u, i != 0u);
        registers->write(registers->mem, data_port, 0u);
        Lufia2SpanAccountInstruction(cpu, 4u);
        bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, data_port, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 4u);
        bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        --registers->y;
        registers->z = registers->y == 0;
        registers->n = (registers->y & 0x8000u) != 0;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 7u, 2u, true);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        registers->pc = (uint16_t)(registers->z ? pc + 9u : pc);
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 10u + registers->cyclesUsed;
    }
    return (int)cycles;
}

static int RunBattleWramPortCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if ((pc != 0x81bbbau && pc != 0x81bbd4u) || opcode != 0xb9u || registers->e ||
        !registers->mf || registers->xf || !registers->x || registers->x > 0x100u ||
        (registers->db & 0x7fu) >= 0x40u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 16u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\x00\x8d\x80\x21\xc8\xca\xd0\xf6", 9u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint32_t source = bank + registers->y;
    if (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source))
        return 0;

    const unsigned read_cycles = registers->y >= 0x100u ? 5u : 4u;
    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t value = registers->read(registers->mem, source);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    registers->write(registers->mem, bank | 0x2180u, value);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool write_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + 10u : pc);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = write_progress ? 3 : read_progress ? 4 : -4;
    return read_cycles + 8u + registers->cyclesUsed;
}

static int RunBattleWramPortNext(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81bbc4u || opcode != 0xc2u || registers->e || registers->xf || registers->dp ||
        (registers->db & 0x7fu) >= 0x40u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xa5\x15\x18\x69\x00\x02\x8d\x81\x21\xe2\x20\xa2\x00\x01", 15u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint16_t address = Lufia2DecompReadData(registers, 0x15u, 0x16u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, address, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0200u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    const uint32_t bank = (uint32_t)registers->db << 16;
    Lufia2DecompWriteData(registers, bank | 0x2181u, bank | 0x2182u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool port_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 13u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0100u, false);
    registers->pc = 0xbbd4u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = port_progress ? 2 : read_progress ? 5 : -6;
    return 23;
}

static int RunBattleWramPortReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81bbdeu || opcode != 0xabu || registers->e ||
        registers->sp < 0x1f00u || registers->sp > 0x1ffcu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 10u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 2u, &offset);
    if (!cart || cart->rom[offset + 1u] != 0x60u)
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    registers->db = registers->read(registers->mem, ++registers->sp);
    registers->n = (registers->db & 0x80u) != 0;
    registers->z = registers->db == 0;
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool bank_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    uint16_t return_address = registers->read(registers->mem, ++registers->sp);
    return_address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->pc = (uint16_t)(return_address + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : bank_progress ? 1 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 10;
}

static int RunBattlePortraitDestination(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81bb75u || opcode != 0x7bu || registers->e || !registers->mf || registers->xf ||
        registers->dp || (registers->db & 0x7fu) >= 0x40u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 48u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 27u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xa5\x11\x0a\xaa\xbc\x2c\xb5\x8c\x81\x21\x84\x15\x9c\x83\x21"
        "\xa5\x12\x0a\xaa\xbc\x34\xb5\x84\x13\xa5\x11", 26u) ||
        !Lufia2DecompIsLoRomDataByte(cart, bank | 0xb634u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0u, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    uint32_t instruction = pc + 1u;
    long progress = 0;
    unsigned extra_cycles = 0;
    for (unsigned table = 0; table < 2u; ++table) {
        const long first = table ? 9 : 2;
        Lufia2SpanFetch(registers, instruction, 2u, true);
        const uint8_t input = registers->read(registers->mem, 0x11u + table);
        Lufia2DecompApplyOperand(registers, 0xa9u, input, true);
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = first;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 2u;

        Lufia2SpanFetch(registers, instruction++, 1u, true);
        const uint8_t index = (uint8_t)(input << 1);
        registers->c = (input & 0x80u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, index, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, instruction++, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, index, false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, instruction, 3u, true);
        const uint32_t table_base = bank | (table ? 0xb534u : 0xb52cu);
        const uint32_t entry = table_base + registers->x;
        const uint16_t value = Lufia2DecompReadData(registers, entry, entry + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
        const unsigned crossed_page = (entry >> 8) != (table_base >> 8);
        Lufia2SpanAccountInstruction(cpu, 5u + crossed_page);
        extra_cycles += crossed_page;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = first + 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 3u;

        if (!table) {
            Lufia2SpanFetch(registers, instruction, 3u, true);
            Lufia2DecompWriteData(registers, bank | 0x2181u, bank | 0x2182u, value, true);
            Lufia2SpanAccountInstruction(cpu, 5u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 6;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            instruction += 3u;
        }

        Lufia2SpanFetch(registers, instruction, 2u, true);
        const uint32_t scratch = table ? 0x13u : 0x15u;
        Lufia2DecompWriteData(registers, scratch, scratch + 1u, value, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = table ? 13 : 7;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 2u;

        if (!table) {
            Lufia2SpanFetch(registers, instruction, 3u, true);
            registers->write(registers->mem, bank | 0x2183u, 0u);
            Lufia2SpanAccountInstruction(cpu, 4u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 8;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
            instruction += 3u;
        }
    }

    Lufia2SpanFetch(registers, instruction, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x11u), true);
    registers->pc = 0xbb90u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 14;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 14 - progress : -13;
    return 46u + extra_cycles;
}

static int RunBattlePortraitPointer(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81bb90u || opcode != 0xaau || registers->e || !registers->mf || registers->xf ||
        registers->a > 0xffu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 36u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\xbf\x3d\x15\x00\x0a\x18\x7f\x3d\x15\x00\xaa\xc2\x20\xbf\xc2\xe5\x8e\xa8\xe2\x20\xe8\xe8", 22u))
        return 0;
    if (!Lufia2DecompIsLoRomDataByte(cart, 0x8ee5c2u) ||
        !Lufia2DecompIsLoRomDataByte(cart, 0x8ee6c2u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 1u, 4u, true);
    const uint32_t slot = 0x153du + registers->x;
    const uint8_t portrait = registers->read(registers->mem, slot);
    Lufia2DecompApplyOperand(registers, 0xa9u, portrait, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    registers->c = (portrait & 0x80u) != 0;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(portrait << 1), true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 4u, true);
    const uint8_t addend = registers->read(registers->mem, slot);
    Lufia2DecompApplyOperand(registers, 0x69u, addend, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 14u, 4u, true);
    const uint32_t entry = 0x8ee5c2u + registers->x;
    const uint16_t offset_value = Lufia2DecompReadData(registers, entry, entry + 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, offset_value, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 8;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 18u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    for (unsigned increment = 0; increment < 2u; ++increment) {
        Lufia2SpanFetch(registers, pc + 21u + increment, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    registers->pc = 0xbba7u;
    registers->cyclesUsed = 2u;
    *tail_steps = progress ? 12 - progress : -11;
    return 36;
}

static int RunBattleWramPortSetup(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool load_bank = pc == 0x81bba7u;
    if ((!load_bank && pc != 0x81bbaeu) || opcode != (load_bank ? 0xbfu : 0xc2u) ||
        registers->e || registers->xf || registers->dp || registers->waiting ||
        registers->stopped || registers->irqWanted || registers->nmiWanted ||
        (load_bank && (!registers->mf || registers->sp < 0x1f02u || registers->sp > 0x1fffu)) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (load_bank ? 15u : 19u))
        return 0;

    unsigned offset;
    const unsigned length = load_bank ? 7u : 12u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = load_bank ? "\xc2\xe5\x8e\x8b\x48\xab" :
        "\x20\x98\x18\x65\x13\xa8\xe2\x20\xa2\x00\x01";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (load_bank) {
        const uint32_t address = 0x8ee5c2u + registers->x;
        if (!Lufia2DecompIsLoRomDataByte(cart, address))
            return 0;

        Lufia2SpanFetch(registers, pc, 4u, false);
        const uint8_t new_bank = registers->read(registers->mem, address);
        Lufia2DecompApplyOperand(registers, 0xa9u, new_bank, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        for (unsigned push = 0; push < 2u; ++push) {
            Lufia2SpanFetch(registers, pc + 4u + push, 1u, true);
            registers->write(registers->mem, registers->sp--, push ? new_bank : registers->db);
            Lufia2SpanAccountInstruction(cpu, 3u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 2 + push;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }

        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        registers->db = registers->read(registers->mem, ++registers->sp);
        registers->n = (registers->db & 0x80u) != 0;
        registers->z = registers->db == 0;
        registers->pc = 0xbbaeu;
        registers->cyclesUsed = 4u;
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 4;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        *tail_steps = progress ? 4 - progress : -3;
        return 15;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    const uint16_t pose = Lufia2DecompReadData(registers, 0x13u, 0x14u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, pose, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 6u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 9u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0100u, false);
    registers->pc = 0xbbbau;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 3 : -6;
    return 19;
}

static int Lufia2DecompRunBattleWramPort(Interp816 *registers, CpuState *cpu,
                                         uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                         uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x81bb75u:
        if (room >= 14)
            return RunBattlePortraitDestination(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81bb90u:
        if (room >= 12)
            return RunBattlePortraitPointer(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81bba7u:
    case 0x81bbaeu:
        if (room >= (pc == 0x81bba7u ? 4 : 7))
            return RunBattleWramPortSetup(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81bbbau:
    case 0x81bbd4u:
        if (room >= 5)
            return RunBattleWramPortCopy(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81bbc4u:
        if (room >= 7)
            return RunBattleWramPortNext(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81bbdeu:
        if (room >= 2)
            return RunBattleWramPortReturn(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    }
    return 0;
}

static int Lufia2DecompRunBattleTilemapClear(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if ((pc != 0x81c307u && pc != 0x81c31au) || opcode != 0x9cu || registers->e ||
        !registers->mf || registers->xf || (registers->db & 0x7fu) >= 0x40u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 9u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 6u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x80\x21\xca\xd0\xfa", 5u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint32_t port = ((uint32_t)registers->db << 16) | 0x2180u;
    registers->write(registers->mem, port, 0u);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool port_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + 6u : pc);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = port_progress ? 2 : -2;
    return 6u + registers->cyclesUsed;
}

static int Lufia2DecompRunBattlePalettePortCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                     uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool from_rom = pc == 0x858ac2u;
    const unsigned count = from_rom ? 0x40u : 0x20u;
    if ((!from_rom && pc != 0x858ae0u) || opcode != 0xbdu || registers->e || !registers->mf ||
        registers->xf || registers->db != (from_rom ? 0x97u : 0x7eu) || registers->x >= count ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 17u)
        return 0;

    const unsigned length = from_rom ? 12u : 13u;
    const char *signature = from_rom ? "\xf8\xcc\x8d\x80\x21\xe8\xe0\x40\x00\xd0\xf4" :
        "\x00\x48\x8f\x80\x21\x00\xe8\xe0\x20\x00\xd0\xf3";
    const uint32_t source = (from_rom ? 0x97ccf8u : 0x7e4800u) + registers->x;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u) ||
        (from_rom && !Lufia2DecompIsLoRomDataByte(cart, source)))
        return 0;

    const unsigned read_cycles = 4u + (from_rom && registers->x >= 8u);
    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t value = registers->read(registers->mem, source);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const unsigned store_length = from_rom ? 3u : 4u;
    uint32_t instruction = pc + 3u;
    Lufia2SpanFetch(registers, instruction, store_length, true);
    registers->write(registers->mem, from_rom ? 0x972180u : 0x002180u, value);
    Lufia2SpanAccountInstruction(cpu, store_length + 1u);
    const bool port_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    instruction += store_length;
    Lufia2SpanFetch(registers, instruction, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, instruction + 1u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, (uint16_t)count, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, instruction + 4u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + length : pc);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = port_progress ? 3 : read_progress ? 4 : -4;
    return read_cycles + store_length + 6u + registers->cyclesUsed;
}
