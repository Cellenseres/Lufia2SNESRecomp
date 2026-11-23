static int RunBattleColorMultiply(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81b3f8u || opcode != 0xe2u || registers->e || registers->xf || registers->dp ||
        (registers->db & 0x7fu) >= 0x40u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 43u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 28u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\xa5\x15\x29\x1f\x8d\x03\x42\xc2\x20\xa5\x15\x4a\x4a"
        "\x4a\x4a\x4a\xe2\x20\x29\x1f\xac\x16\x42\x8d\x03\x42", 27u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->read(registers->mem, 0x15u), true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 2 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x1fu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 6u, 3u, true);
    registers->write(registers->mem, bank | 0x4203u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    const uint16_t color = Lufia2DecompReadData(registers, 0x15u, 0x16u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, color, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned shift = 0; shift < 5u; ++shift) {
        Lufia2SpanFetch(registers, pc + 13u + shift, 1u, true);
        registers->c = (registers->a & 1u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 18u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 20u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x1fu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 22u, 3u, true);
    const uint16_t result = Lufia2DecompReadData(registers, bank | 0x4216u, bank | 0x4217u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, result, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 14;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 25u, 3u, true);
    registers->write(registers->mem, bank | 0x4203u, (uint8_t)registers->a);
    registers->pc = 0xb414u;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 15;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 15 - progress : -14;
    return 43;
}

static int RunBattleColorMiddle(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81b414u || opcode != 0xc2u || registers->e || registers->xf || registers->dp ||
        (registers->db & 0x7fu) >= 0x40u || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 43u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 25u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\x98\x4a\x4a\x4a\x4a\x4a\xa4\x16\x85\x15\x98\xe2\x20"
        "\x4a\x4a\x29\x1f\xac\x16\x42\x8d\x03\x42", 24u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    for (unsigned shift = 0; shift < 5u; ++shift) {
        Lufia2SpanFetch(registers, pc + 3u + shift, 1u, true);
        registers->c = (registers->a & 1u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 8u, 2u, true);
    const uint16_t upper_color = Lufia2DecompReadData(registers, 0x16u, 0x17u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, upper_color, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 8 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 10u, 2u, true);
    Lufia2DecompWriteData(registers, 0x15u, 0x16u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 9;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 12u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    registers->mf = true;
    Lufia2SpanAccountInstruction(cpu, 3u);

    for (unsigned shift = 0; shift < 2u; ++shift) {
        Lufia2SpanFetch(registers, pc + 15u + shift, 1u, true);
        registers->c = (registers->a & 1u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)registers->a >> 1, true);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    Lufia2SpanFetch(registers, pc + 17u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x1fu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 19u, 3u, true);
    const uint16_t product = Lufia2DecompReadData(registers, bank | 0x4216u, bank | 0x4217u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, product, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 15;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 22u, 3u, true);
    registers->write(registers->mem, bank | 0x4203u, (uint8_t)registers->a);
    registers->pc = 0xb42du;
    registers->cyclesUsed = 4u;
    Lufia2SpanAccountInstruction(cpu, 4u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 16;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 16 - progress : -15;
    return 43;
}

static int RunBattleColorFinish(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc != 0x81b42du || opcode != 0xc2u || registers->e || registers->xf || registers->dp ||
        (registers->db & 0x7fu) >= 0x40u || registers->sp < 0x1f00u || registers->sp > 0x1ffdu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 51u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 23u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x20\x98\x29\xc0\x07\x04\x15\xad\x16\x42\x29\xc0\x07\x0a\x0a\x0a\x0a\x46\x15\x04\x15\x60", 22u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 3u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x07c0u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    Lufia2DecompModifyWram(registers, 0x04u, 0x15u, 0x16u, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    long progress = (dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch) ? 4 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    const uint16_t product = Lufia2DecompReadData(registers, bank | 0x4216u, bank | 0x4217u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, product, false);
    Lufia2SpanAccountInstruction(cpu, 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 5;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x07c0u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    for (unsigned shift = 0; shift < 4u; ++shift) {
        Lufia2SpanFetch(registers, pc + 14u + shift, 1u, true);
        registers->c = (registers->a & 0x8000u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    for (unsigned word = 0; word < 2u; ++word) {
        Lufia2SpanFetch(registers, pc + 18u + word * 2u, 2u, true);
        Lufia2DecompModifyWram(registers, word ? 0x04u : 0x46u, 0x15u, 0x16u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 11 + word;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 22u, 1u, true);
    uint16_t target = registers->read(registers->mem, ++registers->sp);
    target |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->pc = (uint16_t)(target + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 13;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 13 - progress : -12;
    return 51;
}

static int RunBattleFadeLoad(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                             uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool lighten = pc == 0x81b3d5u;
    if ((!lighten && pc != 0x81b3b1u) || opcode != 0xe8u || registers->e || registers->xf || registers->dp ||
        registers->x > 0x0e20u || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (lighten ? 25u : 23u))
        return 0;

    unsigned offset;
    const unsigned length = lighten ? 15u : 13u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = lighten ? "\xe8\xa9\xff\x7f\x38\xff\xdb\xf1\x7f\x85\x15\x20\xf8\xb3" :
        "\xe8\xc2\x20\xbf\xdb\xf1\x7f\x85\x15\x20\xf8\xb3";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u) || (lighten && registers->mf))
        return 0;

    for (unsigned step = 0; step < 2u; ++step) {
        Lufia2SpanFetch(registers, pc + step, 1u, step != 0u);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    unsigned source_offset = 4u;
    if (lighten) {
        Lufia2SpanFetch(registers, pc + 2u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x7fffu, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 5u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);
        source_offset = 6u;
    } else {
        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        registers->mf = false;
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    Lufia2SpanFetch(registers, pc + source_offset, 4u, true);
    const uint32_t source = 0x7ff1dbu + registers->x;
    const uint16_t color = Lufia2DecompReadData(registers, source, source + 1u, true);
    Lufia2DecompApplyOperand(registers, lighten ? 0xe9u : 0xa9u, color, false);
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool read_progress = dynamic_epoch != s_interp_dynamic_progress_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + source_offset + 4u, 2u, true);
    Lufia2DecompWriteData(registers, 0x15u, 0x16u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool color_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + source_offset + 6u, 3u, true);
    registers->write(registers->mem, registers->sp--, 0xb3u);
    registers->write(registers->mem, registers->sp--, lighten ? 0xe3u : 0xbdu);
    registers->pc = 0xb3f8u;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    const bool call_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = call_progress ? 0 : color_progress ? 1 : read_progress ? 2 : lighten ? -6 : -5;
    return lighten ? 25 : 23;
}

static int RunBattleFadeStore(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                              uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool lighten = pc == 0x81b3e4u;
    const uint32_t destination = ((uint32_t)registers->db << 16) + 0x0320u + registers->x;
    if ((!lighten && pc != 0x81b3beu) || opcode != (lighten ? 0xa9u : 0xa5u) || registers->e || registers->mf || registers->xf ||
        registers->dp || registers->x > 0x0e22u || !Lufia2DecompIsWramByte(destination) ||
        !Lufia2DecompIsWramByte(destination + 1u) || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted || BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (lighten ? 26u : 22u))
        return 0;

    unsigned offset;
    const unsigned length = lighten ? 13u : 11u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = lighten ? "\xff\x7f\x38\xe5\x15\x9d\x20\x03\xc6\x17\xd0\xe4" :
        "\x15\x9d\x20\x03\xe2\x20\xc6\x17\xd0\xe8";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (lighten) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x7fffu, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        registers->c = true;
        Lufia2SpanAccountInstruction(cpu, 2u);
    }

    const unsigned read_offset = lighten ? 4u : 0u;
    Lufia2SpanFetch(registers, pc + read_offset, 2u, lighten);
    const uint16_t color = Lufia2DecompReadData(registers, 0x15u, 0x16u, true);
    Lufia2DecompApplyOperand(registers, lighten ? 0xe9u : 0xa9u, color, false);
    Lufia2SpanAccountInstruction(cpu, 4u);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? (lighten ? 3 : 1) : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + read_offset + 2u, 3u, true);
    Lufia2DecompWriteData(registers, destination, destination + 1u, registers->a, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = lighten ? 4 : 2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    if (!lighten) {
        Lufia2SpanFetch(registers, pc + 5u, 2u, true);
        registers->mf = true;
        Lufia2SpanAccountInstruction(cpu, 3u);
    }

    Lufia2SpanFetch(registers, pc + length - 4u, 2u, true);
    Lufia2DecompModifyWram(registers, 0xc6u, 0x17u, 0x18u, lighten);
    Lufia2SpanAccountInstruction(cpu, lighten ? 7u : 5u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = lighten ? 5 : 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + length - 2u, 2u, true);
    registers->pc = lighten ? (registers->z ? 0xb3f1u : 0xb3d5u) : (registers->z ? 0xb3c9u : 0xb3b1u);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    const long instructions = lighten ? 6 : 5;
    *tail_steps = progress ? instructions - progress : 1 - instructions;
    return (lighten ? 23u : 19u) + registers->cyclesUsed;
}

static int Lufia2DecompRunBattleColorScale(Interp816 *registers, CpuState *cpu,
                                           uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                           uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x81b3b1u:
    case 0x81b3d5u:
        if (room >= (pc == 0x81b3b1u ? 6 : 7))
            return RunBattleFadeLoad(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b3beu:
    case 0x81b3e4u:
        if (room >= (pc == 0x81b3beu ? 5 : 6))
            return RunBattleFadeStore(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b3f8u:
        if (room >= 15)
            return RunBattleColorMultiply(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b414u:
        if (room >= 16)
            return RunBattleColorMiddle(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b42du:
        if (room >= 13)
            return RunBattleColorFinish(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    }
    return 0;
}
