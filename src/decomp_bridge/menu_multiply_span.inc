static int RunMenuMultiplyStep(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x82800du || opcode != 0x6eu || registers->e || registers->mf || registers->xf ||
        !Lufia2DecompIsWramByte(bank + 0x1570u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 48u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 24u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x72\x15\x90\x0a\xad\x76\x15\x18\x6d\x70\x15\x8d\x76\x15"
        "\x6e\x76\x15\x6e\x74\x15\xca\xd0\xe8", 23u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    Lufia2DecompModifyWram(registers, 0x6eu, bank + 0x1572u, bank + 0x1573u, true);
    Lufia2SpanAccountInstruction(cpu, 8u);
    long progress = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    const bool add = registers->c;
    Lufia2SpanAccountInstruction(cpu, add ? 2u : 3u);
    if (add) {
        Lufia2SpanFetch(registers, pc + 5u, 3u, true);
        const uint16_t high = Lufia2DecompReadData(registers, bank + 0x1576u, bank + 0x1577u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, high, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 3;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 8u, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 9u, 3u, true);
        const uint16_t factor = Lufia2DecompReadData(registers, bank + 0x1570u, bank + 0x1571u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, factor, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 5;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 12u, 3u, true);
        Lufia2DecompWriteData(registers, bank + 0x1576u, bank + 0x1577u, registers->a, true);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 6;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    const long instructions = add ? 10 : 6;
    for (unsigned word = 0; word < 2; ++word) {
        const uint32_t product = bank + 0x1576u - word * 2u;
        Lufia2SpanFetch(registers, pc + 15u + word * 3u, 3u, true);
        Lufia2DecompModifyWram(registers, 0x6eu, product, product + 1u, true);
        Lufia2SpanAccountInstruction(cpu, 8u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = instructions - 3 + word;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 21u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 22u, 2u, true);
    registers->pc = registers->z ? 0x8025u : 0x800du;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return (add ? 45u : 29u) + registers->cyclesUsed;
}

static int RunMenuMultiplyFrame(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                 uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool entering = pc == 0x828000u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!entering && pc != 0x828025u) || opcode != (entering ? 0xdau : 0x28u) ||
        registers->e || registers->sp < 0x1f03u || registers->sp > (entering ? 0x1fffu : 0x1ff9u) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (entering ? 23u : 15u))
        return 0;

    unsigned offset;
    const unsigned length = entering ? 13u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = entering ? "\x08\xc2\x30\x9c\x74\x15\x9c\x76\x15\xa2\x10\x00" : "\xfa\x6b";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (entering) {
        if (!Lufia2DecompIsWramByte(bank + 0x1574u))
            return 0;
        const unsigned push_cycles = registers->xf ? 3u : 4u;
        Lufia2SpanFetch(registers, pc, 1u, false);
        if (!registers->xf)
            registers->write(registers->mem, registers->sp--, (uint8_t)(registers->x >> 8));
        registers->write(registers->mem, registers->sp--, (uint8_t)registers->x);
        Lufia2SpanAccountInstruction(cpu, push_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 1u, 1u, true);
        registers->write(registers->mem, registers->sp--, interp816_getFlags(registers));
        Lufia2SpanAccountInstruction(cpu, 3u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        registers->mf = registers->xf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);

        for (unsigned index = 0; index < 2u; ++index) {
            Lufia2SpanFetch(registers, pc + 4u + index * 3u, 3u, true);
            const uint32_t product = bank + 0x1574u + index * 2u;
            Lufia2DecompWriteData(registers, product, product + 1u, 0u, true);
            Lufia2SpanAccountInstruction(cpu, 5u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 4 + index;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }

        Lufia2SpanFetch(registers, pc + 10u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 16u, false);
        registers->pc = 0x800du;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = progress ? 6 - progress : -5;
        return 19u + push_cycles;
    }

    Lufia2SpanFetch(registers, pc, 1u, false);
    interp816_setFlags(registers, registers->read(registers->mem, ++registers->sp));
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    const unsigned pull_cycles = registers->xf ? 4u : 5u;
    Lufia2SpanFetch(registers, pc + 1u, 1u, true);
    uint16_t index = registers->read(registers->mem, ++registers->sp);
    if (!registers->xf)
        index |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, index, registers->xf);
    Lufia2SpanAccountInstruction(cpu, pull_cycles);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    uint16_t low = registers->read(registers->mem, ++registers->sp);
    low |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(low + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 3 - progress : -2;
    return 10u + pull_cycles;
}

static int Lufia2DecompRunMenuMultiply(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    if (pc == 0x82800du)
        return room >= 10
            ? RunMenuMultiplyStep(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps)
            : 0;

    return room >= (pc == 0x828000u ? 6 : 3)
        ? RunMenuMultiplyFrame(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps)
        : 0;
}
