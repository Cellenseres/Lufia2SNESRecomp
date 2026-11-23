static int Lufia2DecompRunWramPortFill(Interp816 *registers, CpuState *cpu, uint32_t pc,
                           uint8_t opcode) {
    const bool larger_fill = pc == 0x858a51u;
    if ((pc != 0x858d42u && pc != 0x858a51u) || opcode != 0x9cu || registers->e || !registers->mf ||
        registers->xf || registers->db != (larger_fill ? 0x81u : 0x85u) || registers->dp ||
        registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || !registers->y || registers->y > (larger_fill ? 0x02a0u : 0x0200u))
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 9u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\x80\x21\x8d\x80\x21\x88\xd0\xf7", 8u))
        return 0;

    const uint64_t room = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 13u;
    unsigned count = room < 8u ? (unsigned)room : 8u;
    if (count > registers->y)
        count = registers->y;
    if (!count)
        return 0;

    const uint32_t data_port = ((uint32_t)registers->db << 16) | 0x2180u;
    unsigned cycles = 0;
    for (unsigned i = 0; i < count; ++i) {
        Lufia2SpanFetch(registers, pc, 3u, i != 0u);
        registers->write(registers->mem, data_port, 0u);
        Lufia2SpanAccountInstruction(cpu, 4u);
        bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 3u, 3u, true);
        registers->write(registers->mem, data_port, (uint8_t)registers->a);
        Lufia2SpanAccountInstruction(cpu, 4u);
        bridge_apu_flush(cpu);

        Lufia2SpanFetch(registers, pc + 6u, 1u, true);
        --registers->y;
        registers->z = registers->y == 0;
        registers->n = (registers->y & 0x8000u) != 0;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 7u, 2u, true);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        registers->pc = (uint16_t)(registers->z ? pc + 9u : pc);
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 10u + registers->cyclesUsed;
    }
    return (int)cycles;
}
