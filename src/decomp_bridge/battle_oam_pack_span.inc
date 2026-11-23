static int RunBattleOamAdmit(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                               uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81b705u || opcode != 0x85u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 19u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x5a\x89\xff\xf0\x74\xc9\x04\x90\x71\xa5\x58\x89\x03\xd0\x6b", 15u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->write(registers->mem, 0x5au, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool count_progress = write_epoch != g_interp_bridge_write_epoch ||
        dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 0xffu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    if (registers->z) {
        registers->pc = 0xb77fu;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = count_progress ? 2 : -2;
        return 8;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xc9u, 4u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    if (!registers->c) {
        registers->pc = 0xb780u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = count_progress ? 4 : -4;
        return 12;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    const uint8_t used = registers->read(registers->mem, 0x58u);
    Lufia2DecompApplyOperand(registers, 0xa9u, used, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool used_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x89u, 3u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->pc = registers->z ? 0xb715u : 0xb780u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = used_progress ? 2 : count_progress ? 7 : -7;
    return 16u + registers->cyclesUsed;
}

static int RunBattleOamPackShift(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool shift = pc == 0x81b7aeu;
    if ((!shift && pc != 0x81b7b6u) || opcode != (shift ? 0x06u : 0xc6u) ||
        registers->e || !registers->mf || registers->dp || registers->waiting ||
        registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (shift ? 24u : 8u))
        return 0;

    unsigned offset;
    const unsigned length = shift ? 12u : 4u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = shift ? "\x5b\x06\x5b\x0a\x0a\x09\x03\xc6\x59\x10\xf4" : "\x59\x10\xf4";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (shift) {
        for (unsigned step = 0; step < 2u; ++step) {
            Lufia2SpanFetch(registers, pc + step * 2u, 2u, step != 0u);
            Lufia2DecompModifyWram(registers, 0x06u, 0x5bu, 0x5cu, false);
            Lufia2SpanAccountInstruction(cpu, 5u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = 1 + step;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }

        for (unsigned step = 0; step < 2u; ++step) {
            Lufia2SpanFetch(registers, pc + 4u + step, 1u, true);
            const uint8_t low = (uint8_t)registers->a;
            registers->c = (low & 0x80u) != 0;
            Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(low << 1), true);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }

        Lufia2SpanFetch(registers, pc + 6u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0x09u, 3u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    const uint32_t decrement_pc = 0x81b7b6u;
    Lufia2SpanFetch(registers, decrement_pc, 2u, shift);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x59u, 0x5au, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const long instructions = shift ? 7 : 2;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = instructions - 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, decrement_pc + 2u, 2u, true);
    registers->pc = registers->n ? 0xb7bau : 0xb7aeu;
    registers->cyclesUsed = registers->n ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return (shift ? 21u : 5u) + registers->cyclesUsed;
}

static int RunBattleOamPackWrite(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t destination = ((uint32_t)registers->db << 16) + 0x0300u + registers->y;
    if (pc != 0x81b7bau || opcode != 0x39u || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->y >= 32u ||
        !Lufia2DecompIsWramByte(destination) || registers->sp < 0x1f00u || registers->sp > 0x1ffbu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 10u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x00\x03\x05\x5b\x99\x00\x03\xfa\x7a", 9u))
        return 0;

    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t old = registers->read(registers->mem, destination);
    Lufia2DecompApplyOperand(registers, 0x29u, old, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    const uint8_t packed = registers->read(registers->mem, 0x5bu);
    Lufia2DecompApplyOperand(registers, 0x09u, packed, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    registers->write(registers->mem, destination, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned index = 0; index < 2u; ++index) {
        Lufia2SpanFetch(registers, pc + 8u + index, 1u, true);
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, index ? 0xa0u : 0xa2u, value, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 4 + index;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    registers->pc = 0xb7c4u;
    registers->cyclesUsed = 5u;
    *tail_steps = progress ? 5 - progress : -4;
    return 23;
}

static int RunBattleOamNext(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81b7c4u || opcode != 0xc2u || registers->e || registers->xf || registers->dp ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 30u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 20u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\x98\x18\x69\x04\x00\xa8\xe2\x20\xe6\x58\xa5\x5a\x3a\xf0\x03\x4c\x05\xb7", 19u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 4u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 4u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 7u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xe6u, 0x58u, 0x59u, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    const bool used_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    const uint8_t count = registers->read(registers->mem, 0x5au);
    Lufia2DecompApplyOperand(registers, 0xa9u, count, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    const bool count_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 14u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(count - 1u), true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    if (registers->z) {
        registers->pc = 0xb7d8u;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = count_progress ? 2 : used_progress ? 3 : -9;
        return 28;
    }
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 17u, 3u, true);
    registers->pc = 0xb705u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = count_progress ? 3 : used_progress ? 4 : -10;
    return 30;
}

static int Lufia2DecompRunBattleOamPack(Interp816 *registers, CpuState *cpu,
                                           uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                           uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x81b705u:
        if (room >= 8)
            return RunBattleOamAdmit(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b7aeu:
    case 0x81b7b6u:
        if (room >= (pc == 0x81b7aeu ? 7 : 2))
            return RunBattleOamPackShift(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b7bau:
        if (room >= 5)
            return RunBattleOamPackWrite(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b7c4u:
        if (room >= 11)
            return RunBattleOamNext(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    }
    return 0;
}
