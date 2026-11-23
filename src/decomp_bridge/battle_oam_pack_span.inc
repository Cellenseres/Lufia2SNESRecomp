static bool BattleOamCanReturn(const Interp816 *registers, const Cart *cart,
                               unsigned opcode_offset, uint64_t budget, unsigned cycles) {
    return budget >= cycles && registers->sp >= 0x1f00u && registers->sp <= 0x1ffdu &&
        opcode_offset < cart->romSize && cart->rom[opcode_offset] == 0x60u;
}

static int BattleOamReturn(Interp816 *registers, CpuState *cpu, uint32_t pc, int cycles,
                            uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    Lufia2SpanFetch(registers, pc, 1u, true);
    uint16_t address = registers->read(registers->mem, ++registers->sp);
    address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->pc = (uint16_t)(address + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        *tail_steps = 0;
    else
        *tail_steps += *tail_steps < 0 ? -1 : 1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return cycles + 6;
}

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
        if (BattleOamCanReturn(registers, cart, offset + 0x7au,
                               BufferSpanCycleBudget(cpu, s_lle_master_deadline), 6u))
            return BattleOamReturn(registers, cpu, 0x81b77fu, 8, dynamic_epoch, write_epoch, tail_steps);
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
        if (BattleOamCanReturn(registers, cart, offset + 20u,
                               BufferSpanCycleBudget(cpu, s_lle_master_deadline), 6u))
            return BattleOamReturn(registers, cpu, 0x81b7d8u, 28, dynamic_epoch, write_epoch, tail_steps);
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

static int RunBattleOamLowCopy(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool four_sprites = pc == 0x81b715u;
    const unsigned sprites = four_sprites ? 4u : 1u;
    const unsigned words = sprites * 2u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!four_sprites && pc != 0x81b780u) || opcode != 0xc2u || registers->e || registers->xf ||
        registers->x > 0x10000u - sprites * 5u || registers->y > 0x0200u - sprites * 4u ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 3u + words * 13u)
        return 0;

    unsigned offset;
    const unsigned length = 2u + words * 6u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || cart->rom[offset + 1u] != 0x20u)
        return 0;

    for (unsigned word = 0; word < words; ++word) {
        const unsigned source_offset = (word / 2u) * 5u + (word & 1u) * 2u;
        const uint32_t source = bank + registers->x + source_offset;
        const uint32_t destination = bank + 0x0100u + registers->y + word * 2u;
        const uint8_t signature[] = {0xbdu, (uint8_t)source_offset, 0u, 0x99u, (uint8_t)(word * 2u), 1u};
        if (memcmp(cart->rom + offset + 2u + word * 6u, signature, sizeof(signature)) ||
            (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source)) ||
            (!Lufia2DecompIsWramByte(source + 1u) && !Lufia2DecompIsLoRomDataByte(cart, source + 1u)) ||
            !Lufia2DecompIsWramByte(destination) || !Lufia2DecompIsWramByte(destination + 1u))
            return 0;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = false;
    Lufia2SpanAccountInstruction(cpu, 3u);

    unsigned cycles = 3;
    long progress = 0;
    for (unsigned word = 0; word < words; ++word) {
        const unsigned source_offset = (word / 2u) * 5u + (word & 1u) * 2u;
        const uint32_t source = bank + registers->x + source_offset;
        const unsigned read_cycles = (registers->x + source_offset) >= 0x0100u ? 6u : 5u;
        Lufia2SpanFetch(registers, pc + 2u + word * 6u, 3u, true);
        const uint16_t value = Lufia2DecompReadData(registers, source, source + 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 2 + word * 2u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        const uint32_t destination = bank + 0x0100u + registers->y + word * 2u;
        Lufia2SpanFetch(registers, pc + 5u + word * 6u, 3u, true);
        Lufia2DecompWriteData(registers, destination, destination + 1u, value, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 3 + word * 2u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        cycles += read_cycles + 7u;
    }

    registers->pc = (uint16_t)(pc + length);
    registers->cyclesUsed = 7u;
    *tail_steps = progress ? 1L + words * 2L - progress : -2L * words;
    return cycles;
}

static int RunBattleOamCursors(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool four_sprites = pc == 0x81b747u;
    if ((!four_sprites && pc != 0x81b78eu) || opcode != 0x98u || registers->e || registers->mf ||
        registers->xf || registers->sp < 0x1f04u || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (four_sprites ? 35u : 30u))
        return 0;

    unsigned offset;
    const unsigned length = four_sprites ? 19u : 15u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = four_sprites ? "\x18\x69\x10\x00\x48\x4a\x4a\x4a\x4a\xa8\x8a\x18\x69\x14\x00\x48\xe2\x20" :
        "\x48\x4a\x4a\x4a\x4a\xa8\x8a\x18\x69\x05\x00\x48\xe2\x20";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    uint32_t instruction = pc + 1u;
    if (four_sprites) {
        Lufia2SpanFetch(registers, instruction++, 1u, true);
        registers->c = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, instruction, 3u, true);
        Lufia2DecompApplyOperand(registers, 0x69u, 16u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);
        instruction += 3u;
    }

    Lufia2SpanFetch(registers, instruction++, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->a >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool oam_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    for (unsigned shift = 0; shift < 4u; ++shift) {
        Lufia2SpanFetch(registers, instruction++, 1u, true);
        registers->c = (registers->a & 1u) != 0;
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
        Lufia2SpanAccountInstruction(cpu, 2u);
    }
    Lufia2SpanFetch(registers, instruction++, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, instruction++, 1u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, instruction++, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, instruction, 3u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, four_sprites ? 20u : 5u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    instruction += 3u;

    Lufia2SpanFetch(registers, instruction++, 1u, true);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->a >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 4u);
    const bool source_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, instruction, 2u, true);
    registers->mf = true;
    registers->pc = (uint16_t)(instruction + 2u);
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = source_progress ? 1 : oam_progress ? 10 : four_sprites ? -13 : -11;
    return four_sprites ? 35u : 30u;
}

static int RunBattleOamPackFour(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                    uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const uint32_t bank = (uint32_t)registers->db << 16;
    const uint32_t destination = bank + 0x02ffu + registers->y;
    if (pc != 0x81b75au || opcode != 0xbdu || registers->e || !registers->mf || registers->xf ||
        registers->x > 0xffecu || !Lufia2DecompIsWramByte(destination) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 38u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 21u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x13\x00\x0a\x0a\x1d\x0e\x00\x0a\x0a\x1d\x09\x00\x0a\x0a\x1d\x04\x00\x99\xff\x02", 20u))
        return 0;

    for (unsigned part = 0; part < 4u; ++part) {
        const uint32_t source = bank + registers->x + 19u - part * 5u;
        if (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source))
            return 0;
    }

    uint32_t instruction = pc;
    unsigned cycles = 0;
    long progress = 0;
    long instructions = 0;
    for (unsigned part = 0; part < 4u; ++part) {
        const unsigned source_offset = 19u - part * 5u;
        const unsigned read_cycles = registers->x + source_offset >= 0x0100u ? 5u : 4u;
        Lufia2SpanFetch(registers, instruction, 3u, part != 0u);
        uint8_t attributes = registers->read(registers->mem, bank + registers->x + source_offset);
        Lufia2DecompApplyOperand(registers, part ? 0x09u : 0xa9u, attributes, true);
        Lufia2SpanAccountInstruction(cpu, read_cycles);
        cycles += read_cycles;
        ++instructions;
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = instructions;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        instruction += 3u;

        if (part == 3u)
            break;

        for (unsigned shift = 0; shift < 2u; ++shift) {
            Lufia2SpanFetch(registers, instruction++, 1u, true);
            attributes = (uint8_t)registers->a;
            registers->c = (attributes & 0x80u) != 0;
            Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(attributes << 1), true);
            Lufia2SpanAccountInstruction(cpu, 2u);
        }
        cycles += 4u;
        instructions += 2;
    }

    Lufia2SpanFetch(registers, instruction, 3u, true);
    registers->write(registers->mem, destination, (uint8_t)registers->a);
    registers->pc = 0xb76fu;
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 11;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? 11 - progress : -10;
    return cycles + 6u;
}

static int RunBattleOamPackFourTail(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                        uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    if (pc != 0x81b76fu || opcode != 0xfau || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->sp < 0x1f00u || registers->sp > 0x1ffbu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 30u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 16u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x7a\xa5\x58\x18\x69\x04\x85\x58\xa5\x5a\x38\xe9\x04\xd0\x86", 15u))
        return 0;

    long progress = 0;
    for (unsigned index = 0; index < 2u; ++index) {
        Lufia2SpanFetch(registers, pc + index, 1u, index != 0u);
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, index ? 0xa0u : 0xa2u, value, false);
        Lufia2SpanAccountInstruction(cpu, 5u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = 1 + index;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 2u, 2u, true);
    const uint8_t used = registers->read(registers->mem, 0x58u);
    Lufia2DecompApplyOperand(registers, 0xa9u, used, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    registers->c = false;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x69u, 4u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    registers->write(registers->mem, 0x58u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    const uint8_t remaining = registers->read(registers->mem, 0x5au);
    Lufia2DecompApplyOperand(registers, 0xa9u, remaining, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 7;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 11u, 1u, true);
    registers->c = true;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 12u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xe9u, 4u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 14u, 2u, true);
    registers->pc = registers->z ? 0xb77fu : 0xb705u;
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = progress ? 10 - progress : -9;
    if (registers->z && room >= 11 &&
        BattleOamCanReturn(registers, cart, offset + 16u,
                          BufferSpanCycleBudget(cpu, s_lle_master_deadline), 6u))
        return BattleOamReturn(registers, cpu, 0x81b77fu, 29, dynamic_epoch, write_epoch, tail_steps);
    return 27u + registers->cyclesUsed;
}

static int RunBattleOamAttributeSource(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    if (pc != 0x81b79du || opcode != 0xbdu || registers->e || !registers->mf || registers->xf ||
        registers->dp || registers->x > 0xfffbu || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 23u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 17u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u,
        "\x04\x00\x29\x03\x85\x5b\xa5\x58\x29\x03\x85\x59\xa9\xfc\x80\x08", 16u))
        return 0;

    const uint32_t attribute = ((uint32_t)registers->db << 16) + registers->x + 4u;
    if (!Lufia2DecompIsWramByte(attribute) && !Lufia2DecompIsLoRomDataByte(cart, attribute))
        return 0;

    const unsigned read_cycles = registers->x + 4u >= 0x0100u ? 5u : 4u;
    Lufia2SpanFetch(registers, pc, 3u, false);
    const uint8_t bits = registers->read(registers->mem, attribute);
    Lufia2DecompApplyOperand(registers, 0xa9u, bits, true);
    Lufia2SpanAccountInstruction(cpu, read_cycles);
    long progress = dynamic_epoch != s_interp_dynamic_progress_epoch ? 1 : 0;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 3u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 3u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 5u, 2u, true);
    registers->write(registers->mem, 0x5bu, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 3;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 7u, 2u, true);
    const uint8_t used = registers->read(registers->mem, 0x58u);
    Lufia2DecompApplyOperand(registers, 0xa9u, used, true);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = 4;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 9u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0x29u, 3u, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 11u, 2u, true);
    registers->write(registers->mem, 0x59u, (uint8_t)registers->a);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
        progress = 6;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

    Lufia2SpanFetch(registers, pc + 13u, 2u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xfcu, true);
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, pc + 15u, 2u, true);
    registers->pc = 0xb7b6u;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 8 - progress : -7;
    return 18u + read_cycles;
}

static int Lufia2DecompRunBattleOamPack(Interp816 *registers, CpuState *cpu,
                                           uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch,
                                           uint64_t write_epoch, long room, long *tail_steps) {
    switch (pc) {
    case 0x81b705u:
        if (room >= 8)
            return RunBattleOamAdmit(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b715u:
    case 0x81b780u:
        if (room >= (pc == 0x81b715u ? 17 : 5))
            return RunBattleOamLowCopy(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b747u:
    case 0x81b78eu:
        if (room >= (pc == 0x81b747u ? 14 : 12))
            return RunBattleOamCursors(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b75au:
        if (room >= 11)
            return RunBattleOamPackFour(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
        break;
    case 0x81b76fu:
        if (room >= 10)
            return RunBattleOamPackFourTail(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
        break;
    case 0x81b79du:
        if (room >= 8)
            return RunBattleOamAttributeSource(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
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
