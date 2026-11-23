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

static int Lufia2DecompRunSpriteOamClearBoundary(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool start = pc == 0x868bcfu;
    const bool high_table = pc == 0x868be7u;
    const bool finish = pc == 0x868bf2u;
    const uint8_t expected = start ? 0xc2u : high_table ? 0xa0u : 0xe2u;
    const unsigned cycles = start ? 12u : high_table ? 3u : 9u;
    if ((!start && !high_table && !finish) || opcode != expected || registers->e || registers->xf ||
        (!start && registers->mf) ||
        (finish && (registers->sp < 0x1f00u || registers->sp > 0x1ffcu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;

    unsigned offset;
    const unsigned length = start ? 11u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = start ? "\x20\xa0\x80\x00\xa2\x00\x00\xa9\x00\xf0" :
        high_table ? "\x10\x00" : "\x20\x6b";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    if (high_table) {
        Lufia2SpanFetch(registers, pc, 3u, false);
        Lufia2DecompApplyOperand(registers, 0xa0u, 16u, false);
        registers->pc = 0x8beau;
        registers->cyclesUsed = 3u;
        Lufia2SpanAccountInstruction(cpu, 3u);
        *tail_steps = 0;
        return 3;
    }

    Lufia2SpanFetch(registers, pc, 2u, false);
    registers->mf = finish;
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (finish) {
        Lufia2SpanFetch(registers, pc + 2u, 1u, true);
        uint16_t return_address = registers->read(registers->mem, ++registers->sp);
        return_address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        registers->pc = (uint16_t)(return_address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 9;
    }

    Lufia2SpanFetch(registers, pc + 2u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa0u, 128u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 5u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);

    Lufia2SpanFetch(registers, pc + 8u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0xf000u, false);
    registers->pc = 0x8bdau;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = -3;
    return 12;
}

static int Lufia2DecompRunSpriteOamBuildBoundary(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool start = pc == 0x868bf5u;
    const bool return_now = pc == 0x868c1eu;
    if ((!start && !return_now && pc != 0x868c0du) || opcode != (return_now ? 0x6bu : 0xa2u) ||
        registers->e || !registers->mf || registers->xf || (start && registers->dp) ||
        (return_now && (registers->sp < 0x1f00u || registers->sp > 0x1ffcu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (start ? 14u : return_now ? 6u : 3u))
        return 0;

    unsigned offset;
    const unsigned length = start ? 10u : return_now ? 1u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || (!return_now && memcmp(cart->rom + offset + 1u,
        start ? "\x00\x00\x86\x54\x86\x5a\xa2\x0c\x00" : "\x00\x00", length - 1u)))
        return 0;

    Lufia2SpanFetch(registers, pc, return_now ? 1u : 3u, false);
    if (return_now) {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        registers->k = registers->read(registers->mem, ++registers->sp);
        registers->pc = (uint16_t)(address + 1u);
        registers->cyclesUsed = 6u;
        Lufia2SpanAccountInstruction(cpu, 6u);
        *tail_steps = 0;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        return 6;
    }

    Lufia2DecompApplyOperand(registers, 0xa2u, 0u, false);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (!start) {
        registers->pc = 0x8c10u;
        registers->cyclesUsed = 3u;
        *tail_steps = 0;
        return 3;
    }

    const uint32_t cursors[] = {0x54u, 0x5au};
    long progress = 0;
    for (unsigned index = 0; index < 2; ++index) {
        Lufia2SpanFetch(registers, pc + 3u + 2u * index, 2u, true);
        Lufia2DecompWriteData(registers, cursors[index], cursors[index] + 1u, 0u, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            progress = 2u + index;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    Lufia2SpanFetch(registers, pc + 7u, 3u, true);
    Lufia2DecompApplyOperand(registers, 0xa2u, 12u, false);
    registers->pc = 0x8bffu;
    registers->cyclesUsed = 3u;
    Lufia2SpanAccountInstruction(cpu, 3u);
    *tail_steps = progress ? 4 - progress : -3;
    return 14;
}

static int Lufia2DecompRunSpriteFrameCall(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool wait_call = pc == 0x868b68u;
    if ((!wait_call && pc != 0x868b5cu && pc != 0x868b60u && pc != 0x868b64u) ||
        opcode != (wait_call ? 0xa9u : 0x22u) ||
        registers->e || !registers->mf || registers->xf || (wait_call && registers->dp) ||
        registers->sp < (wait_call ? 0x1f02u : 0x1f03u) || registers->sp > 0x1fffu ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (wait_call ? 11u : 8u))
        return 0;

    const uint16_t target = wait_call ? 0x8b48u : pc == 0x868b5cu ? 0x8b73u :
        pc == 0x868b60u ? 0x8bcfu : 0x8bf5u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, wait_call ? 7u : 4u, &offset);
    if (!cart)
        return 0;
    if (wait_call ? memcmp(cart->rom + offset + 1u, "\x80\x85\x72\x20\x48\x8b", 6u) != 0 :
        (cart->rom[offset + 1u] != (uint8_t)target || cart->rom[offset + 2u] != target >> 8 ||
         cart->rom[offset + 3u] != 0x86u))
        return 0;

    bool flag_progress = false;
    if (wait_call) {
        Lufia2SpanFetch(registers, pc, 2u, false);
        Lufia2DecompApplyOperand(registers, 0xa9u, 0x80u, true);
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        registers->write(registers->mem, 0x72u, 0x80u);
        Lufia2SpanAccountInstruction(cpu, 3u);
        flag_progress = dynamic_epoch != s_interp_dynamic_progress_epoch ||
            write_epoch != g_interp_bridge_write_epoch;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }

    const uint32_t call_site = wait_call ? pc + 4u : pc;
    const uint16_t return_address = (uint16_t)(call_site + (wait_call ? 2u : 3u));
    Lufia2SpanFetch(registers, call_site, wait_call ? 3u : 4u, wait_call);
    if (!wait_call)
        registers->write(registers->mem, registers->sp--, registers->k);
    registers->write(registers->mem, registers->sp--, (uint8_t)(return_address >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)return_address);
    registers->pc = target;
    registers->cyclesUsed = wait_call ? 6u : 8u;
    Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
    *tail_steps = !wait_call || dynamic_epoch != s_interp_dynamic_progress_epoch ||
        write_epoch != g_interp_bridge_write_epoch ? 0 : flag_progress ? 1 : -2;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return wait_call ? 11 : 8;
}

static int Lufia2DecompRunSpriteAnimateBoundary(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool entering = pc == 0x868b73u;
    if ((!entering && pc != 0x868b84u) || opcode != (entering ? 0xe2u : 0xc2u) ||
        registers->e || !registers->mf ||
        (!entering && (registers->sp < 0x1f00u || registers->sp > 0x1ffcu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (entering ? 5u : 9u))
        return 0;

    unsigned offset;
    const unsigned length = entering ? 4u : 3u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, entering ? "\x10\xa2\x00" : "\x10\x6b", length - 1u))
        return 0;

    Lufia2SpanFetch(registers, pc, 2u, false);
    const uint8_t status = interp816_getFlags(registers);
    interp816_setFlags(registers, entering ? status | 0x10u : status & ~0x10u);
    Lufia2SpanAccountInstruction(cpu, 3u);
    if (entering) {
        Lufia2SpanFetch(registers, pc + 2u, 2u, true);
        Lufia2DecompApplyOperand(registers, 0xa2u, 0u, true);
        registers->pc = 0x8b77u;
        registers->cyclesUsed = 2u;
        Lufia2SpanAccountInstruction(cpu, 2u);
        *tail_steps = -1;
        return 5;
    }

    Lufia2SpanFetch(registers, pc + 2u, 1u, true);
    uint16_t address = registers->read(registers->mem, ++registers->sp);
    address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(address + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    *tail_steps = dynamic_epoch != s_interp_dynamic_progress_epoch ? 0 : -1;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    return 9;
}
