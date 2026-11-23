static int Lufia2DecompRunBattleHdmaCallFrame(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool returning = pc == 0x8192e7u;
    if ((!returning && pc != 0x8192dfu) || opcode != (returning ? 0x7au : 0x5au) ||
        registers->e || !registers->mf || registers->xf ||
        registers->sp < (returning ? 0x1f00u : 0x1f05u) ||
        registers->sp > (returning ? 0x1ffbu : 0x1fffu) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (returning ? 11u : 18u))
        return 0;

    unsigned offset;
    const unsigned length = returning ? 2u : 8u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = returning ? "\x60" : "\x4b\xf4\xe6\x92\xdc\x9b\x12";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    if (returning) {
        uint16_t y = registers->read(registers->mem, ++registers->sp);
        y |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa0u, y, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        const bool pull_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanNearReturn(registers, cpu, 0x8192e8u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : pull_progress ? 1 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 11;
    }

    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->y >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->y);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch
        ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    registers->write(registers->mem, registers->sp--, registers->k);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    registers->write(registers->mem, registers->sp--, 0x92u);
    registers->write(registers->mem, registers->sp--, 0xe6u);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    registers->pc = Lufia2DecompReadData(registers, 0x129bu, 0x129cu, true);
    registers->k = registers->read(registers->mem, 0x129du);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : progress ? 4 - progress : -3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 18;
}
