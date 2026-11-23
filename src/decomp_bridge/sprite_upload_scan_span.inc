typedef struct SpriteUploadScanSpan {
    uint64_t budget;
    unsigned steps;
    unsigned cycles;
    unsigned instructions;
    unsigned progress;
    unsigned slots;
} SpriteUploadScanSpan;

static bool SpriteUploadScanCanStep(const SpriteUploadScanSpan *span, const CpuState *cpu, unsigned cycles) {
    // A single instruction keeps the original atomic deadline boundary.
    return span->instructions < span->steps &&
        (!span->instructions || (span->cycles + cycles <= span->budget &&
         (!s_lle_master_deadline || cpu->master_cycles < s_lle_master_deadline)));
}

static void SpriteUploadScanStep(SpriteUploadScanSpan *span, Interp816 *registers,
                                CpuState *cpu, unsigned cycles, uint16_t next) {
    Lufia2SpanAccountInstruction(cpu, cycles);
    registers->cyclesUsed = cycles;
    registers->pc = next;
    span->cycles += cycles;
    ++span->instructions;
}

static int Lufia2DecompRunSpriteUploadScan(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || !step_budget ||
        !Lufia2DecompFrameRangeQuiet(0x84819au, 0x84819fu) ||
        !Lufia2DecompFrameRangeQuiet(0x8481f6u, 0x8481fdu))
        return 0;
    if (pc != 0x84819au && pc != 0x84819du && pc != 0x8481f6u &&
        pc != 0x8481f7u && pc != 0x8481f8u && pc != 0x8481fbu)
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = (pc & 0xffffu) - 0x819au;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 0x63u ||
        memcmp(cart->rom + first, "\xbc\xc2\x05\xf0\x57", 5u) ||
        memcmp(cart->rom + first + 0x5cu, "\xe8\xe8\xe0\x10\x00\xd0\x9d", 7u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    SpriteUploadScanSpan span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x84819au: goto load_source;
    case 0x84819du: goto branch_source;
    case 0x8481f6u: goto advance_low;
    case 0x8481f7u: goto advance_high;
    case 0x8481f8u: goto compare_slot;
    case 0x8481fbu: goto repeat_scan;
    default: return 0;
    }
load_source:
    {
        const uint32_t low = (bank + 0x05c2u + registers->x) & 0xffffffu;
        const uint32_t high = (low + 1u) & 0xffffffu;
        const unsigned cycles = 5u + (registers->x >= 0x3eu);
        if (!SpriteUploadScanCanStep(&span, cpu, cycles) ||
            !Lufia2DecompIsWramByte(low) || !Lufia2DecompIsWramByte(high))
            goto finished;
        Lufia2SpanFetch(registers, 0x84819au, 3u, span.instructions != 0u);
        const uint16_t source = Lufia2DecompReadData(registers, low, high, true);
        Lufia2DecompApplyOperand(registers, 0xa0u, source, false);
        SpriteUploadScanStep(&span, registers, cpu, cycles, 0x819du);
        if (dynamic_epoch != s_interp_dynamic_progress_epoch || write_epoch != g_interp_bridge_write_epoch)
            span.progress = span.instructions;
        Lufia2SpanSyncProgress(cpu, &dynamic_epoch, &write_epoch);
    }
branch_source:
    {
        const bool empty = registers->z;
        const unsigned cycles = empty ? 3u : 2u;
        if (!SpriteUploadScanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x84819du, 2u, span.instructions != 0u);
        SpriteUploadScanStep(&span, registers, cpu, cycles, empty ? 0x81f6u : 0x819fu);
        if (!empty) goto finished;
    }
advance_low:
    if (!SpriteUploadScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x8481f6u, 1u, span.instructions != 0u);
    registers->x = (uint16_t)(registers->x + 1u);
    registers->n = (registers->x & 0x8000u) != 0u;
    registers->z = registers->x == 0u;
    SpriteUploadScanStep(&span, registers, cpu, 2u, 0x81f7u);
advance_high:
    if (!SpriteUploadScanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x8481f7u, 1u, span.instructions != 0u);
    registers->x = (uint16_t)(registers->x + 1u);
    registers->n = (registers->x & 0x8000u) != 0u;
    registers->z = registers->x == 0u;
    SpriteUploadScanStep(&span, registers, cpu, 2u, 0x81f8u);
compare_slot:
    if (!SpriteUploadScanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x8481f8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x10u, false);
    SpriteUploadScanStep(&span, registers, cpu, 3u, 0x81fbu);
repeat_scan:
    {
        const bool repeat = !registers->z;
        const unsigned cycles = repeat ? 3u : 2u;
        if (!SpriteUploadScanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x8481fbu, 2u, span.instructions != 0u);
        SpriteUploadScanStep(&span, registers, cpu, cycles, repeat ? 0x819au : 0x81fdu);
        if (repeat && ++span.slots < 8u) goto load_source;
    }
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
