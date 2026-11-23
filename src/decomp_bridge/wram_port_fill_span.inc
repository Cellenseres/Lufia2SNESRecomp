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

static int Lufia2DecompRunBattleWramPort(Interp816 *registers, CpuState *cpu,
                                         uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                         uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
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
