static int Lufia2DecompRunRomToWramMove(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const bool palette = pc == 0x869150u;
    const uint8_t source_bank = palette ? 0x97u : 0x9bu;
    if ((!palette && pc != 0x80f351u) || opcode != 0x54u ||
        registers->e || registers->xf || registers->mf ||
        registers->x < 0x8000u || registers->y >= 0x2000u || room <= 0 ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || cart->rom[offset + 1u] != 0u || cart->rom[offset + 2u] != source_bank)
        return 0;

    const uint64_t time_budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 7u;
    const unsigned max_bytes = palette ? 32u : 8u;
    unsigned count = (unsigned long)room < max_bytes ? (unsigned)room : max_bytes;
    if (count > time_budget)
        count = (unsigned)time_budget;
    if (count > 0x10000u - registers->x)
        count = 0x10000u - registers->x;
    if (count > 0x2000u - registers->y)
        count = 0x2000u - registers->y;
    if (count > (unsigned)registers->a + 1u)
        count = (unsigned)registers->a + 1u;
    const uint32_t source = (uint32_t)source_bank << 16;
    if (!count || !Lufia2DecompIsLoRomDataByte(cart, source + registers->x + count - 1u))
        return 0;

    long progress = 0;
    for (unsigned byte = 0; byte < count; ++byte) {
        Lufia2SpanFetch(registers, pc, 3u, byte != 0u);
        registers->db = 0u;
        const uint8_t value = registers->read(registers->mem, source | registers->x);
        registers->write(registers->mem, registers->y++, value);
        ++registers->x;
        --registers->a;
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = byte + 1u;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    registers->pc = (uint16_t)(pc + (registers->a == 0xffffu ? 3u : 0u));
    registers->cyclesUsed = 7u;
    *tail_steps = progress ? (long)count - progress : 1L - (long)count;
    return 7u * count;
}
