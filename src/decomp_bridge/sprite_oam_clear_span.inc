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
