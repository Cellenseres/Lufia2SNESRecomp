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
        if (active) {
            if (registers->sp >= 0x1f02u && registers->sp <= 0x1fffu) {
                const uint64_t write_epoch = g_interp_bridge_write_epoch;
                Lufia2SpanFetch(registers, pc + 5u, 3u, true);
                const uint16_t return_address = (uint16_t)(pc + 7u);
                registers->write(registers->mem, registers->sp--, (uint8_t)(return_address >> 8));
                registers->write(registers->mem, registers->sp--, (uint8_t)return_address);
                registers->pc = animation ? 0x8b87u : 0x8c1fu;
                registers->cyclesUsed = 6u;
                Lufia2SpanAccountInstruction(cpu, 6u);
                result.cycles += 6u;
                ++result.instructions;
                if (auto_quiescent && (write_epoch != g_interp_bridge_write_epoch ||
                                       dynamic_epoch != s_interp_dynamic_progress_epoch)) {
                    bridge_apu_flush(cpu);
                    result.progress = true;
                    result.instructions = 1u;
                }
            }
            break;
        }

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

static int Lufia2DecompRunSpriteSlotAdvance(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    const bool animation = pc == 0x868b7fu;
    const bool low_slots = pc == 0x868c18u;
    if ((!animation && !low_slots && pc != 0x868c07u) || opcode != 0xe8u ||
        registers->e || registers->xf != animation || registers->waiting || registers->stopped ||
        registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (animation ? 7u : 8u))
        return 0;

    const unsigned length = animation ? 5u : 6u;
    const unsigned limit = low_slots ? 12u : 48u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = animation ? "\xe0\x30\xd0\xf3" :
        low_slots ? "\xe0\x0c\x00\xd0\xf2" : "\xe0\x30\x00\xd0\xf2";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 1u, false);
    const uint16_t next = (uint16_t)(registers->x + 1u);
    Lufia2DecompApplyOperand(registers, 0xa2u, animation ? (uint8_t)next : next, animation);
    Lufia2SpanAccountInstruction(cpu, 2u);

    const unsigned compare_cycles = animation ? 2u : 3u;
    Lufia2SpanFetch(registers, pc + 1u, compare_cycles, true);
    Lufia2DecompApplyOperand(registers, 0xe0u, limit, animation);
    Lufia2SpanAccountInstruction(cpu, compare_cycles);

    Lufia2SpanFetch(registers, pc + length - 2u, 2u, true);
    registers->pc = (uint16_t)(registers->z ? pc + length : pc - 8u);
    registers->cyclesUsed = registers->z ? 2u : 3u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    return 2u + compare_cycles + registers->cyclesUsed;
}
