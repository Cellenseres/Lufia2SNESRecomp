static int Lufia2DecompRunMenuGlyphVariant(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                         uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool raw = pc == 0x808dcau;
    const bool lookup = pc == 0x808dd0u;
    if ((!raw && !lookup && pc != 0x808de4u) ||
        opcode != (raw ? 0x9cu : lookup ? 0xdau : 0x9du) ||
        registers->e || !registers->mf || registers->xf || registers->dp ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0578u) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (raw ? 10u : lookup ? 39u : 38u))
        return 0;

    unsigned offset;
    const unsigned length = raw ? 6u : lookup ? 20u : 21u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = raw ? "\x78\x05\xeb\x80\xee" :
        lookup ? "\xeb\xa9\x00\xeb\x38\xe9\xcd\xaa\xbf\x16\x8e\x80\xeb\xbf\x49\x8e\x80\xeb\xfa" :
                 "\x40\x00\xad\x64\x05\x9d\x41\x00\xeb\x9d\x00\x00\xad\x64\x05\x9d\x01\x00\x80\xce";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    const uint32_t bank = (uint32_t)registers->db << 16;
    if (raw) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        registers->write(registers->mem, bank | 0x0578u, 0u);
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        MenuGlyphExchangeBytes(registers);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 4u, 2u, true);
        registers->pc = 0x8dbeu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = 2;
        return 10;
    }

    if (lookup) {
        if (registers->d || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
            !Lufia2DecompIsLoRomDataByte(cart, 0x808f48u))
            return 0;

        Lufia2SpanFetch(registers, pc, 1u, false);
        registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 1u, 1u, true);
        MenuGlyphExchangeBytes(registers);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 4u, 1u, true);
        MenuGlyphExchangeBytes(registers);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 6u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xe9u, 0xcdu, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 8u, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        for (unsigned row = 0; row < 2u; ++row) {
            Lufia2SpanFetch(registers, pc + 9u + row * 5u, 4u, true);
            const uint32_t table = row ? 0x808e49u : 0x808e16u;
            const uint8_t tile = registers->read(registers->mem, table + registers->x);
            Lufia2DecompApplyOperand(registers, 0xa9u, tile, true);
            Lufia2SpanAccountInstruction(cpu, 5u);

            Lufia2SpanFetch(registers, pc + 13u + row * 5u, 1u, true);
            MenuGlyphExchangeBytes(registers);
            Lufia2SpanAccountInstruction(cpu, 3u);
        }

        Lufia2SpanFetch(registers, pc + 19u, 1u, true);
        uint16_t tile_index = registers->read(registers->mem, ++registers->sp);
        tile_index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa2u, tile_index, false);
        registers->pc = 0x8de4u;
        registers->cyclesUsed = 5u;
        Lufia2SpanAccountInstruction(cpu, 5u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : 11;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 39;
    }

    const uint32_t upper_tile = bank + registers->x;
    const uint32_t lower_tile = upper_tile + 0x40u;
    if (!Lufia2DecompIsWramByte(upper_tile) || !Lufia2DecompIsWramByte(upper_tile + 1u) ||
        !Lufia2DecompIsWramByte(lower_tile) || !Lufia2DecompIsWramByte(lower_tile + 1u))
        return 0;

    uint32_t instruction = pc;
    for (unsigned row = 0; row < 2u; ++row) {
        const uint32_t tile = row ? upper_tile : lower_tile;
        Lufia2SpanFetch(registers, instruction, 3u, row != 0u);
        registers->write(registers->mem, tile, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 6u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 3u, 3u, true);
        const uint8_t attribute = registers->read(registers->mem, bank | 0x0564u);
        Lufia2DecompApplyOperand(registers, 0xa9u, attribute, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, instruction + 6u, 3u, true);
        registers->write(registers->mem, tile + 1u, attribute);
        Lufia2SpanAccountInstruction(cpu, 6u);
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        if (!row) {
            Lufia2SpanFetch(registers, instruction + 9u, 1u, true);
            MenuGlyphExchangeBytes(registers);
            Lufia2SpanAccountInstruction(cpu, 3u);
            instruction += 10u;
        }
    }
    Lufia2SpanFetch(registers, pc + 19u, 2u, true);
    registers->pc = 0x8dc7u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = 1;
    return 38;
}
