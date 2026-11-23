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
    if ((!pull_index && pc != 0x818c59u) || opcode != (pull_index ? 0xfau : 0x60u) ||
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
