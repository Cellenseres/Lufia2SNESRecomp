static int FinishWorldMenuIdle(Interp816 *registers, CpuState *cpu, unsigned cycles, bool can_return,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (registers->pc != 0x9b2fu || !can_return)
        return cycles;

    Lufia2SpanNearReturn(registers, cpu, 0x869b2fu);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        *tail_steps = 0;
    else
        *tail_steps += *tail_steps < 0 ? -1 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return cycles + 6u;
}

static int Lufia2DecompRunWorldMenuIdle(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x869adbu || opcode != 0xa5u || registers->e || !registers->mf || registers->dp ||
        !Lufia2DecompIsWramByte(bank | 0x09e1u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 20u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x46\x89\x40\xf0\x4e\xad\xe1\x09\xf0\x05\xad\xe0\x09\xd0\x44", 15u))
        return 0;

    const bool can_return = registers->sp >= 0x1f00u && registers->sp <= 0x1ffdu &&
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) >= 26u &&
        offset + 0x54u < cart->romSize && cart->rom[offset + 0x54u] == 0x60u;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint8_t input = registers->read(registers->mem, 0x46u);
    Lufia2DecompApplyOperand(registers, 0xa9u, input, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x40u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    if (registers->z) {
        registers->pc = 0x9b2fu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 2 : -2;
        return FinishWorldMenuIdle(registers, cpu, 8u, can_return, dynamic_epoch, write_epoch, tail_steps);
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    const uint8_t menu_open = registers->read(registers->mem, bank | 0x09e1u);
    Lufia2DecompApplyOperand(registers, 0xa9u, menu_open, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    if (registers->z) {
        registers->pc = 0x9aebu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 5 - progress : -4;
        return 14;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 11u, 3u, true);
    const uint8_t blocked = registers->read(registers->mem, bank | 0x09e0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, blocked, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->pc = registers->z ? 0x9aebu : 0x9b2fu;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 7 - progress : -6;
    return FinishWorldMenuIdle(registers, cpu, 17u + registers->cyclesUsed, can_return,
        dynamic_epoch, write_epoch, tail_steps);
}
