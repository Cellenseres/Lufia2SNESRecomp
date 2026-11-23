static int Lufia2DecompRunActorMenuClear(Interp816 *registers, CpuState *cpu,
                              uint32_t pc, uint8_t opcode) {
    if (pc != 0x81f1ceu || opcode != 0xcau || registers->e || !registers->mf ||
        registers->xf || registers->dp || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        registers->x <= 0x0b77u || registers->x > 0x0badu ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x0badu))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x9e\x00\x00\xe0\x77\x0b\xd0\xf7", 8u))
        return 0;

    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 14u;
    unsigned count = budget < 8u ? (unsigned)budget : 8u;
    const unsigned remaining = registers->x - 0x0b77u;
    if (count > remaining)
        count = remaining;
    if (!count)
        return 0;

    unsigned cycles = 0;
    for (unsigned i = 0; i < count; ++i) {
        Lufia2SpanFetch(registers, pc, 1u, i != 0u);
        --registers->x;
        registers->n = false;
        registers->z = false;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 1u, 3u, true);
        registers->write(registers->mem, ((uint32_t)registers->db << 16) | registers->x, 0u);
        Lufia2SpanAccountInstruction(cpu, 6u);
        bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 4u, 3u, true);
        Lufia2DecompApplyOperand(registers, 0xe0u, 0x0b77u, false);
        Lufia2SpanAccountInstruction(cpu, 3u);

        Lufia2SpanFetch(registers, pc + 7u, 2u, true);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        registers->pc = registers->z ? 0xf1d7u : 0xf1ceu;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 11u + registers->cyclesUsed;
    }
    return (int)cycles;
}
