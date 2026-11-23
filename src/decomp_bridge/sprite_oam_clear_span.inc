static void SpriteOamClearStore(Interp816 *registers, CpuState *cpu,
                                uint32_t pc, uint16_t value, bool auto_quiescent, bool probe) {
    Lufia2SpanFetch(registers, pc, 3u, probe);
    const uint32_t address = ((uint32_t)registers->db << 16) | (0x0100u + registers->x);
    Lufia2DecompWriteData(registers, address, address + 1u, value, true);
    Lufia2SpanAccountInstruction(cpu, 7u);
    if (auto_quiescent)
        bridge_apu_flush(cpu);
}

static unsigned SpriteOamClearStep(Interp816 *registers, CpuState *cpu,
                                    uint32_t pc, bool high_table, bool auto_quiescent, bool probe) {
    unsigned cycles = 0;
    if (!high_table) {
        SpriteOamClearStore(registers, cpu, pc, registers->a, auto_quiescent, probe);
        Lufia2SpanFetch(registers, pc + 3u, 1u, true);
        ++registers->x;
        registers->n = (registers->x & 0x8000u) != 0;
        registers->z = registers->x == 0;
        Lufia2SpanAccountInstruction(cpu, 2u);
        Lufia2SpanFetch(registers, pc + 4u, 1u, true);
        ++registers->x;
        registers->n = (registers->x & 0x8000u) != 0;
        registers->z = registers->x == 0;
        Lufia2SpanAccountInstruction(cpu, 2u);
        pc += 5u;
        cycles = 11u;
    }

    SpriteOamClearStore(registers, cpu, pc, 0u, auto_quiescent, !high_table || probe);
    Lufia2SpanFetch(registers, pc + 3u, 1u, true);
    ++registers->x;
    registers->n = (registers->x & 0x8000u) != 0;
    registers->z = registers->x == 0;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 4u, 1u, true);
    ++registers->x;
    registers->n = (registers->x & 0x8000u) != 0;
    registers->z = registers->x == 0;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 5u, 1u, true);
    --registers->y;
    registers->n = (registers->y & 0x8000u) != 0;
    registers->z = registers->y == 0;
    Lufia2SpanAccountInstruction(cpu, 2u);
    Lufia2SpanFetch(registers, pc + 6u, 2u, true);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    registers->pc = registers->z ? (uint16_t)(pc + 8u)
                                 : (uint16_t)(pc - (high_table ? 0u : 5u));
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return cycles + 13u + registers->cyclesUsed;
}

static int Lufia2DecompRunSpriteOamClearSpan(Interp816 *registers, CpuState *cpu,
                                uint32_t pc, uint8_t opcode, bool auto_quiescent) {
    const bool high = pc == 0x868beau;
    if ((!high && pc != 0x868bdau) || opcode != (high ? 0x9eu : 0x9du) ||
        registers->e || registers->mf || registers->xf || registers->dp ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0100u) ||
        !registers->y || registers->y > (high ? 16u : 128u) ||
        registers->x != (high ? 0x0220u - 2u * registers->y : 0x0200u - 4u * registers->y))
        return 0;

    const unsigned stride_cycles = high ? 16u : 27u;
    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / stride_cycles;
    unsigned count = room < 8u ? (unsigned)room : 8u;
    if (!count)
        return 0;
    if (count > registers->y)
        count = registers->y;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, high ? 8u : 13u, &offset);
    const char *signature = high ? "\x00\x01\xe8\xe8\x88\xd0\xf8" :
        "\x00\x01\xe8\xe8\x9e\x00\x01\xe8\xe8\x88\xd0\xf3";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, high ? 7u : 12u))
        return 0;

    unsigned cycles = 0;
    for (unsigned iteration = 0; iteration < count; ++iteration)
        cycles += SpriteOamClearStep(registers, cpu, pc, high, auto_quiescent, iteration != 0u);
    return (int)cycles;
}

static int Lufia2DecompRunSpritePlaneClear(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const bool tilemap = pc == 0x868e45u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!tilemap && pc != 0x868e71u) || opcode != 0x9eu || registers->e ||
        registers->mf == tilemap || registers->xf ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (tilemap ? 38u : 13u))
        return 0;
    if (tilemap ? (registers->db != 0x7eu || (registers->x & 1u) || registers->x >= 0x0800u)
                : !Lufia2DecompIsWramByte(bank + registers->x))
        return 0;

    unsigned offset;
    const unsigned length = tilemap ? 19u : 7u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = tilemap ? "\x00\x20\x9e\x00\x28\x9e\x00\x30\x9e\x00\x38\xe8\xe8\xe0\x00\x08\xd0\xed" :
        "\x00\x00\xe8\x88\xd0\xf9";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    const unsigned instructions = tilemap ? 8u : 4u;
    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline);
    const unsigned iteration_cycles = tilemap ? 38u : 13u;
    unsigned limit = (unsigned)(room / instructions);
    if (limit > 8u)
        limit = 8u;
    if (limit > budget / iteration_cycles)
        limit = (unsigned)(budget / iteration_cycles);
    const unsigned remaining = tilemap ? (0x0800u - registers->x) / 2u :
        (registers->y ? registers->y : 0x10000u);
    if (limit > remaining)
        limit = remaining;
    if (!limit)
        return 0;

    long progress = 0;
    long executed = 0;
    unsigned cycles = 0;
    const unsigned writes = tilemap ? 4u : 1u;
    for (unsigned record = 0; record < limit; ++record) {
        if (!tilemap && !Lufia2DecompIsWramByte(bank + registers->x))
            break;

        for (unsigned plane = 0; plane < writes; ++plane) {
            Lufia2SpanFetch(registers, pc + plane * 3u, 3u, record != 0u || plane != 0u);
            const uint32_t address = bank + registers->x + (tilemap ? 0x2000u + plane * 0x0800u : 0u);
            Lufia2DecompWriteData(registers, address, address + 1u, 0u, tilemap);
            Lufia2SpanAccountInstruction(cpu, tilemap ? 7u : 6u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = executed + 1 + plane;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }

        uint32_t instruction = pc + writes * 3u;
        Lufia2SpanFetch(registers, instruction++, 1u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, instruction++, 1u, true);
        if (tilemap)
            Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
        else
            Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y - 1u), false);
        Lufia2SpanAccountInstruction(cpu, 2u);

        if (tilemap) {
            Lufia2SpanFetch(registers, instruction, 3u, true);
            Lufia2DecompApplyOperand(registers, 0xe0u, 0x0800u, false);
            Lufia2SpanAccountInstruction(cpu, 3u);
            instruction += 3u;
        }

        Lufia2SpanFetch(registers, instruction, 2u, true);
        registers->pc = (uint16_t)(registers->z ? pc + length : pc);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += iteration_cycles - 3u + registers->cyclesUsed;
        executed += instructions;
    }
    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles;
}
