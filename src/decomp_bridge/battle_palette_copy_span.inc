static int Lufia2DecompRunBattlePaletteCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81b9b7u || opcode != 0xbfu || registers->e || !registers->mf ||
        registers->xf || registers->db || registers->x > 0x01ffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 16u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xdb\xf1\x7f\x9d\x20\x03\xca\x10\xf6", 9u))
        return 0;

    Lufia2SpanFetch(registers, pc, 4u, false);
    const uint8_t value = registers->read(registers->mem, 0x7ff1dbu + registers->x);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    registers->write(registers->mem, 0x0320u + registers->x, value);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool write_progress = write_epoch != g_interp_bridge_write_epoch ||
        dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->pc = registers->n ? 0xb9c1u : 0xb9b7u;
    registers->cyclesUsed = registers->n ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = write_progress ? 2 : read_progress ? 3 : -3;
    return 13u + registers->cyclesUsed;
}
