static int Lufia2DecompRunForwardBlockMove(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const bool palette = pc == 0x869150u;
    const bool capsule = pc == 0x82d038u || pc == 0x82d04fu;
    const bool field_restore = pc == 0x83b077u || pc == 0x83b083u;
    const bool menu_graphics = pc == 0x8690ccu || pc == 0x8690dfu || pc == 0x8690f2u || pc == 0x869105u || pc == 0x869118u;
    const uint8_t source_bank = capsule ? 0x7eu : field_restore ? 0xa6u : menu_graphics ? 0x9fu : palette ? 0x97u : 0x9bu;
    const uint8_t destination_bank = capsule ? 0x7eu : 0u;
    const unsigned destination_end = capsule ? 0x10000u : 0x2000u;
    if ((!palette && !capsule && !field_restore && !menu_graphics && pc != 0x80f351u) || opcode != 0x54u ||
        registers->e || registers->xf || registers->mf ||
        (!capsule && registers->x < 0x8000u) || registers->y >= destination_end || room <= 0 ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompFrameRangeQuiet(pc, pc + 3u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || cart->rom[offset + 1u] != destination_bank || cart->rom[offset + 2u] != source_bank)
        return 0;

    const uint64_t time_budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 7u;
    const unsigned max_bytes = palette || capsule || field_restore || menu_graphics ? 32u : 8u;
    unsigned count = (unsigned long)room < max_bytes ? (unsigned)room : max_bytes;
    if (count > time_budget)
        count = (unsigned)time_budget;
    if (count > 0x10000u - registers->x)
        count = 0x10000u - registers->x;
    if (count > destination_end - registers->y)
        count = destination_end - registers->y;
    if (count > (unsigned)registers->a + 1u)
        count = (unsigned)registers->a + 1u;
    const uint32_t source = (uint32_t)source_bank << 16;
    const uint32_t destination = (uint32_t)destination_bank << 16;
    if (!count || (!capsule && !Lufia2DecompIsLoRomDataByte(cart, source + registers->x + count - 1u)))
        return 0;

    long progress = 0;
    for (unsigned byte = 0; byte < count; ++byte) {
        Lufia2SpanFetch(registers, pc, 3u, byte != 0u);
        registers->db = destination_bank;
        const uint8_t value = registers->read(registers->mem, source | registers->x);
        registers->write(registers->mem, destination | registers->y++, value);
        ++registers->x;
        --registers->a;
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = byte + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    registers->pc = (uint16_t)(pc + (registers->a == 0xffffu ? 3u : 0u));
    registers->cyclesUsed = 7u;
    *tail_steps = progress ? (long)count - progress : 1L - (long)count;
    return 7u * count;
}

static int Lufia2DecompRunBackwardByteCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x85cf21u || opcode != 0xb7u || registers->e || !registers->mf ||
        registers->xf || registers->dp || !cpu->ram || cpu->ram != g_snes->ram ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 17u)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 7u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xb5\x97\xb8\x88\x10\xf9", 6u))
        return 0;
    const uint32_t source_pointer = cpu->ram[0xb5u] | ((uint32_t)cpu->ram[0xb6u] << 8) |
        ((uint32_t)cpu->ram[0xb7u] << 16);
    const uint32_t destination_pointer = cpu->ram[0xb8u] | ((uint32_t)cpu->ram[0xb9u] << 8) |
        ((uint32_t)cpu->ram[0xbau] << 16);
    const uint32_t source = (source_pointer + registers->y) & 0xffffffu;
    const uint32_t destination = (destination_pointer + registers->y) & 0xffffffu;
    if ((!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source)) ||
        !Lufia2DecompIsWramByte(destination))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    uint32_t address = Lufia2DecompReadData(registers, 0xb5u, 0xb6u, true);
    address |= (uint32_t)registers->read(registers->mem, 0xb7u) << 16;
    const uint8_t value = registers->read(registers->mem, (address + registers->y) & 0xffffffu);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    Lufia2SpanAccountInstruction(cpu, 6u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    address = Lufia2DecompReadData(registers, 0xb8u, 0xb9u, true);
    address |= (uint32_t)registers->read(registers->mem, 0xbau) << 16;
    registers->write(registers->mem, (address + registers->y) & 0xffffffu, value);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->pc = registers->n ? 0xcf28u : 0xcf21u;
    registers->cyclesUsed = registers->n ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 4 - progress : -3;
    return 14u + registers->cyclesUsed;
}
