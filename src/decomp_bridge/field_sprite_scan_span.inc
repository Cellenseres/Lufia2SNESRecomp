typedef struct FieldSpriteScanSpan {
    uint64_t budget;
    unsigned steps;
    unsigned cycles;
    unsigned instructions;
    unsigned progress;
    unsigned slots;
} FieldSpriteScanSpan;

static bool FieldSpriteScanCanStep(const FieldSpriteScanSpan *span, const CpuState *cpu, unsigned cycles) {
    // A single instruction keeps the original atomic deadline boundary.
    return span->instructions < span->steps &&
        (!span->instructions || (span->cycles + cycles <= span->budget &&
         (!s_lle_master_deadline || cpu->master_cycles < s_lle_master_deadline)));
}

static void FieldSpriteScanStep(FieldSpriteScanSpan *span, Interp816 *registers,
                                CpuState *cpu, unsigned cycles, uint16_t next) {
    Lufia2SpanAccountInstruction(cpu, cycles);
    registers->cyclesUsed = cycles;
    registers->pc = next;
    span->cycles += cycles;
    ++span->instructions;
}

static int Lufia2DecompRunFieldSpriteScan(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (registers->e || !registers->mf || !registers->xf || registers->x > 0xffu ||
        registers->y > 0xffu || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted || !step_budget ||
        !Lufia2DecompFrameRangeQuiet(0x83a29bu, 0x83a2a2u) ||
        !Lufia2DecompFrameRangeQuiet(0x83a319u, 0x83a321u))
        return 0;
    const bool header = pc == 0x83a29bu || pc == 0x83a29eu || pc == 0x83a2a0u;
    const bool tail = pc == 0x83a319u || pc == 0x83a31au || pc == 0x83a31cu || pc == 0x83a31eu;
    if (!header && !tail) return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0xa29bu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 0x86u ||
        memcmp(cart->rom + first, "\xbd\x22\x06\x89\x04\xd0\x77", 7u) ||
        memcmp(cart->rom + first + 0x7eu, "\xe8\xe0\x48\xf0\x03\x82\x7a\xff", 8u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    if (!Lufia2DecompIsWramByte(bank + 0x0622u) ||
        !Lufia2DecompIsWramByte(bank + 0x0721u))
        return 0;
    FieldSpriteScanSpan span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a29bu: goto load_status;
    case 0x83a29eu: goto test_status;
    case 0x83a2a0u: goto branch_status;
    case 0x83a319u: goto advance_slot;
    case 0x83a31au: goto compare_slot;
    case 0x83a31cu: goto branch_end;
    case 0x83a31eu: goto repeat_scan;
    default: return 0;
    }
load_status:
    {
        const unsigned cycles = 4u + (registers->x >= 0xdeu);
        if (!FieldSpriteScanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83a29bu, 3u, span.instructions != 0u);
        const uint8_t status = registers->read(registers->mem, bank + 0x0622u + registers->x);
        Lufia2DecompApplyOperand(registers, 0xa9u, status, true);
        FieldSpriteScanStep(&span, registers, cpu, cycles, 0xa29eu);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            span.progress = span.instructions;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
test_status:
    if (!FieldSpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83a29eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 4u, true);
    FieldSpriteScanStep(&span, registers, cpu, 2u, 0xa2a0u);
branch_status:
    {
        const bool skip = !registers->z;
        const unsigned cycles = skip ? 3u : 2u;
        if (!FieldSpriteScanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83a2a0u, 2u, span.instructions != 0u);
        FieldSpriteScanStep(&span, registers, cpu, cycles, skip ? 0xa319u : 0xa2a2u);
        if (!skip) goto finished;
    }
advance_slot:
    if (!FieldSpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83a319u, 1u, span.instructions != 0u);
    registers->x = (uint8_t)(registers->x + 1u);
    registers->n = (registers->x & 0x80u) != 0u;
    registers->z = registers->x == 0u;
    FieldSpriteScanStep(&span, registers, cpu, 2u, 0xa31au);
compare_slot:
    if (!FieldSpriteScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x83a31au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x48u, true);
    FieldSpriteScanStep(&span, registers, cpu, 2u, 0xa31cu);
branch_end:
    {
        const bool end = registers->z;
        const unsigned cycles = end ? 3u : 2u;
        if (!FieldSpriteScanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x83a31cu, 2u, span.instructions != 0u);
        FieldSpriteScanStep(&span, registers, cpu, cycles, end ? 0xa321u : 0xa31eu);
        if (end) goto finished;
    }
repeat_scan:
    if (!FieldSpriteScanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x83a31eu, 3u, span.instructions != 0u);
    FieldSpriteScanStep(&span, registers, cpu, 4u, 0xa29bu);
    if (++span.slots < 8u) goto load_status;
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
