static int RunMenuButtonMasks(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    const uint32_t first = 0x828b51u;
    const uint32_t end = 0x828c35u;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (pc < first || pc >= end || (pc - first) % 19u || opcode != 0xa9u ||
        registers->e || !registers->mf || registers->dp ||
        !Lufia2DecompIsWramByte(bank | 0x14acu) || room < 9 ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, end - pc, &offset);
    if (!cart)
        return 0;

    static const uint8_t masks[] = {0x80u, 0x40u, 0x20u, 0x10u, 0x80u, 0x40u,
                                    0x20u, 0x10u, 0x08u, 0x04u, 0x02u, 0x01u};
    const unsigned first_button = (pc - first) / 19u;
    unsigned count = 12u - first_button;
    const uint64_t timing = BufferSpanCycleBudget(cpu, s_lle_master_deadline) / 27u;
    if (count > room / 9)
        count = (unsigned)(room / 9);
    if (count > timing)
        count = (unsigned)timing;

    long executed = 0;
    long progress = 0;
    unsigned cycles = 0;
    for (unsigned index = 0; index < count; ++index) {
        const unsigned button = first_button + index;
        const uint8_t mask = masks[button];
        const uint8_t latch = button < 4u ? 0x4au : 0x4bu;
        const uint8_t pressed = button < 4u ? 0xabu : 0xacu;
        const uint8_t expected[] = {0xa9u, mask, 0x25u, latch, 0xf0u, 13u,
            0xa9u, mask, 0x25u, (uint8_t)(latch - 4u), 0xf0u, 7u,
            0x14u, latch, 0xa9u, mask, 0x0cu, pressed, 0x14u};
        if (memcmp(cart->rom + offset + 19u * index, expected, sizeof expected))
            break;

        const uint32_t site = pc + 19u * index;
        for (unsigned test = 0; test < 2; ++test) {
            const uint32_t start = site + 6u * test;
            Lufia2SpanFetch(registers, start, 2u, index != 0u || test != 0u);
            Lufia2DecompApplyOperand(registers, 0xa9u, mask, true);
            Lufia2SpanAccountInstruction(cpu, 2u);

            Lufia2SpanFetch(registers, start + 2u, 2u, true);
            const uint8_t buttons = registers->read(registers->mem, (uint16_t)(latch - 4u * test));
            Lufia2DecompApplyOperand(registers, 0x29u, buttons, true);
            Lufia2SpanAccountInstruction(cpu, 3u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch)
                progress = executed + 2;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

            Lufia2SpanFetch(registers, start + 4u, 2u, true);
            registers->cyclesUsed = registers->z ? 3u : 2u;
            Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
            executed += 3;
            cycles += 5u + registers->cyclesUsed;
            if (registers->z)
                break;
        }

        if (!registers->z) {
            Lufia2SpanFetch(registers, site + 12u, 2u, true);
            Lufia2DecompModifyWram(registers, 0x14u, latch, latch + 1u, false);
            Lufia2SpanAccountInstruction(cpu, 5u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = executed + 1;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

            Lufia2SpanFetch(registers, site + 14u, 2u, true);
            Lufia2DecompApplyOperand(registers, 0xa9u, mask, true);
            Lufia2SpanAccountInstruction(cpu, 2u);

            Lufia2SpanFetch(registers, site + 16u, 3u, true);
            const uint32_t destination = bank | (0x1400u + pressed);
            Lufia2DecompModifyWram(registers, 0x0cu, destination, destination + 1u, false);
            registers->cyclesUsed = 6u;
            Lufia2SpanAccountInstruction(cpu, 6u);
            executed += 3;
            cycles += 13u;
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = executed;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }
        registers->pc = (uint16_t)(site + 19u);
    }

    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles;
}

static int RunMenuButtonBoundary(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                   uint64_t dynamic_epoch, uint64_t write_epoch, long *tail_steps) {
    const bool reset = pc == 0x828b4bu;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if ((!reset && pc != 0x828c35u) || opcode != (reset ? 0x9cu : 0xadu) ||
        registers->e || !registers->mf || !Lufia2DecompIsWramByte(bank | 0x14acu) ||
        (!reset && (registers->sp < 0x1f00u || registers->sp > 0x1ffcu)) ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < (reset ? 8u : 21u))
        return 0;

    unsigned offset;
    const unsigned length = reset ? 6u : 14u;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    const char *signature = reset ? "\xab\x14\x9c\xac\x14" :
        "\xab\x14\xd0\x07\xad\xac\x14\xd0\x02\x38\x6b\x18\x6b";
    if (!cart || memcmp(cart->rom + offset + 1u, signature, length - 1u))
        return 0;

    long progress = 0;
    if (reset) {
        for (unsigned byte = 0; byte < 2; ++byte) {
            Lufia2SpanFetch(registers, pc + 3u * byte, 3u, byte != 0u);
            registers->write(registers->mem, bank | (0x14abu + byte), 0u);
            Lufia2SpanAccountInstruction(cpu, 4u);
            if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
                progress = byte + 1u;
            Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
        }
        registers->pc = 0x8b51u;
        registers->cyclesUsed = 4u;
        *tail_steps = progress ? 2 - progress : -1;
        return 8;
    }

    unsigned cycles = 0;
    long executed = 0;
    for (unsigned byte = 0; byte < 2; ++byte) {
        const uint32_t start = pc + 5u * byte;
        Lufia2SpanFetch(registers, start, 3u, byte != 0u);
        const uint8_t pressed = registers->read(registers->mem, bank | (0x14abu + byte));
        Lufia2DecompApplyOperand(registers, 0xa9u, pressed, true);
        Lufia2SpanAccountInstruction(cpu, 4u);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch)
            progress = executed + 1;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);

        Lufia2SpanFetch(registers, start + 3u, 2u, true);
        const unsigned branch_cycles = pressed ? 3u : 2u;
        Lufia2SpanAccountInstruction(cpu, branch_cycles);
        cycles += 4u + branch_cycles;
        executed += 2;
        if (pressed)
            break;
    }

    const uint32_t return_site = registers->z ? 0x828c3fu : 0x828c41u;
    Lufia2SpanFetch(registers, return_site, 1u, true);
    registers->c = registers->z;
    Lufia2SpanAccountInstruction(cpu, 2u);

    Lufia2SpanFetch(registers, return_site + 1u, 1u, true);
    uint16_t address = registers->read(registers->mem, ++registers->sp);
    address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->k = registers->read(registers->mem, ++registers->sp);
    registers->pc = (uint16_t)(address + 1u);
    registers->cyclesUsed = 6u;
    Lufia2SpanAccountInstruction(cpu, 6u);
    executed += 2;
    if (dynamic_epoch != s_interp_dynamic_progress_epoch)
        progress = executed;
    Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    *tail_steps = progress ? executed - progress : 1 - executed;
    return cycles + 8u;
}

static int Lufia2DecompRunMenuButtons(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                      uint64_t dynamic_epoch, uint64_t write_epoch, long room, long *tail_steps) {
    if (pc == 0x828b4bu || pc == 0x828c35u) {
        if (room < (pc == 0x828b4bu ? 2 : 6))
            return 0;
        return RunMenuButtonBoundary(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, tail_steps);
    }
    return RunMenuButtonMasks(registers, cpu, pc, opcode, dynamic_epoch, write_epoch, room, tail_steps);
}
