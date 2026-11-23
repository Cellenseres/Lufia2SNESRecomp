static int Lufia2DecompRunForwardBlockMove(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                  uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const bool palette = pc == 0x869150u;
    const bool capsule = pc == 0x82d038u || pc == 0x82d04fu;
    const uint8_t source_bank = capsule ? 0x7eu : palette ? 0x97u : 0x9bu;
    const uint8_t destination_bank = capsule ? 0x7eu : 0u;
    const unsigned destination_end = capsule ? 0x10000u : 0x2000u;
    if ((!palette && !capsule && pc != 0x80f351u) || opcode != 0x54u ||
        registers->e || registers->xf || registers->mf ||
        (!capsule && registers->x < 0x8000u) || registers->y >= destination_end || room <= 0 ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 3u, &offset);
    if (!cart || cart->rom[offset + 1u] != destination_bank || cart->rom[offset + 2u] != source_bank)
        return 0;

    const uint64_t time_budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 7u;
    const unsigned max_bytes = palette || capsule ? 32u : 8u;
    unsigned count = (unsigned long)room < max_bytes ? (unsigned)room : max_bytes;
    if (count > time_budget)
        count = (unsigned)time_budget;
    if (count > 0x10000u - registers->x)
        count = 0x10000u - registers->x;
    if (count > destination_end - registers->y)
        count = destination_end - registers->y;
    if (count > (unsigned)registers->a + 1u)
        count = (unsigned)registers->a + 1u;
    const uint32_t source = (uint32_t)source_bank << 16;
    const uint32_t destination = (uint32_t)destination_bank << 16;
    if (!count || (!capsule && !Lufia2DecompIsLoRomDataByte(cart, source + registers->x + count - 1u)))
        return 0;

    long progress = 0;
    for (unsigned byte = 0; byte < count; ++byte) {
        Lufia2SpanFetch(registers, pc, 3u, byte != 0u);
        registers->db = destination_bank;
        const uint8_t value = registers->read(registers->mem, source | registers->x);
        registers->write(registers->mem, destination | registers->y++, value);
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
