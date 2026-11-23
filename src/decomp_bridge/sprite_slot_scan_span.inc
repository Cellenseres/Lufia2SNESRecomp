static SpriteSlotScanResult Lufia2DecompRunSpriteSlotScan(Interp816 *registers, CpuState *cpu,
                                              uint32_t pc, uint8_t opcode,
                                              unsigned remaining_steps, bool auto_quiescent,
                                              uint64_t dynamic_epoch) {
    SpriteSlotScanResult result = {0};
    const bool animation = pc == 0x868b77u;
    const bool low_slots = pc == 0x868c10u;
    if ((!animation && !low_slots && pc != 0x868bffu) || opcode != 0xbdu ||
        registers->e || !registers->mf || registers->xf != animation ||
        registers->dp || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompIsWramByte(((uint32_t)registers->db << 16) | 0x11ffu))
        return result;

    const unsigned limit = low_slots ? 12u : 48u;
    if (registers->x >= limit || (!animation && !low_slots && registers->x < 12u))
        return result;

    const uint64_t time_budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 16u;
    unsigned count = remaining_steps / 5u;
    if (count > 8u)
        count = 8u;
    if (count > time_budget)
        count = (unsigned)time_budget;
    if (count > limit - registers->x)
        count = limit - registers->x;
    if (!count)
        return result;

    unsigned offset;
    const unsigned length = animation ? 13u : 14u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = animation ? "\xd8\x11\xf0\x03\x20\x87\x8b\xe8\xe0\x30\xd0\xf3" :
        low_slots ? "\xd8\x11\xf0\x03\x20\x1f\x8c\xe8\xe0\x0c\x00\xd0\xf2" :
                    "\xd8\x11\xf0\x03\x20\x1f\x8c\xe8\xe0\x30\x00\xd0\xf2";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return result;

    for (unsigned record = 0; record < count; ++record) {
        Lufia2SpanFetch(registers, pc, 3u, record != 0u);
        const unsigned address = 0x11d8u + registers->x;
        const uint8_t active = registers->read(registers->mem,
            ((uint32_t)registers->db << 16) | address);
        registers->a = (registers->a & 0xff00u) | active;
        registers->n = (active & 0x80u) != 0;
        registers->z = active == 0;
        const unsigned load_cycles = 4u + (address > 0x11ffu);
        Lufia2SpanAccountInstruction(cpu, load_cycles);
        if (auto_quiescent && dynamic_epoch != s_interp_dynamic_progress_epoch) {
            dynamic_epoch = s_interp_dynamic_progress_epoch;
            bridge_apu_flush(cpu);
            result.progress = true;
            result.instructions = 0;
        }
        Lufia2SpanFetch(registers, pc + 3u, 2u, true);
        const unsigned branch_cycles = active ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, branch_cycles);
        result.cycles += load_cycles + branch_cycles;
        result.instructions += 2u;
        registers->pc = (uint16_t)(pc + 5u);
        registers->cyclesUsed = branch_cycles;
        if (active)
            break;

        Lufia2SpanFetch(registers, pc + 8u, 1u, true);
        ++registers->x;
        registers->z = registers->x == 0;
        registers->n = false;
        Lufia2SpanAccountInstruction(cpu, 2u);
        const unsigned compare_cycles = animation ? 2u : 3u;
        Lufia2SpanFetch(registers, pc + 9u, compare_cycles, true);
        registers->c = registers->x >= limit;
        registers->z = registers->x == limit;
        registers->n = registers->x < limit;
        Lufia2SpanAccountInstruction(cpu, compare_cycles);
        Lufia2SpanFetch(registers, pc + length - 2u, 2u, true);
        registers->pc = (uint16_t)(registers->z ? pc + length : pc);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        result.cycles += 2u + compare_cycles + registers->cyclesUsed;
        result.instructions += 3u;
    }
    return result;
}
