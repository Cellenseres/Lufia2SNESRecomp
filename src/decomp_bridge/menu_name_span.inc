#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuMemberPointerNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                    uint8_t opcode, uint64_t dynamic_epoch,
                                                    uint64_t write_epoch, unsigned step_budget,
                                                    long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b09u && pc < 0x808b12u)
        end_pc = 0x808b12u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b09u: byte_accumulator = true; break;
    case 0x808b0cu: byte_accumulator = true; break;
    case 0x808b0fu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b09u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x20\xa7\x8b\x20\x3f\x8b\x82\x98\xfd", 9u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b09u: goto read_member_pointer;
    case 0x808b0cu: goto draw_member_name;
    case 0x808b0fu: goto resume_string_reader;
    default: return 0;
    }
read_member_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b09u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b09u, 0x20u, 0x808ba7u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_member_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b0cu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b0cu, 0x20u, 0x808b3fu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_string_reader: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b0fu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuMemberOffsetNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b12u && pc < 0x808b1bu)
        end_pc = 0x808b1bu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b12u: byte_accumulator = true; break;
    case 0x808b15u: byte_accumulator = true; break;
    case 0x808b18u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b12u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x20\xb2\x8b\x20\x3f\x8b\x82\x8f\xfd", 9u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b12u: goto read_member_offset;
    case 0x808b15u: goto draw_member_name;
    case 0x808b18u: goto resume_string_reader;
    default: return 0;
    }
read_member_offset: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b12u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b12u, 0x20u, 0x808bb2u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_member_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b15u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b15u, 0x20u, 0x808b3fu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_string_reader: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b18u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuItemPointerNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b1bu && pc < 0x808b24u)
        end_pc = 0x808b24u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b1bu: byte_accumulator = true; break;
    case 0x808b1eu: byte_accumulator = true; break;
    case 0x808b21u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b1bu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x20\xa7\x8b\x20\x73\x8b\x82\x86\xfd", 9u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b1bu: goto read_item_pointer;
    case 0x808b1eu: goto draw_item_name;
    case 0x808b21u: goto resume_string_reader;
    default: return 0;
    }
read_item_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b1bu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b1bu, 0x20u, 0x808ba7u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_item_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b1eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b1eu, 0x20u, 0x808b73u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_string_reader: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b21u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuItemOffsetNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b24u && pc < 0x808b2du)
        end_pc = 0x808b2du;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b24u: byte_accumulator = true; break;
    case 0x808b27u: byte_accumulator = true; break;
    case 0x808b2au: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b24u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x20\xb2\x8b\x20\x73\x8b\x82\x7d\xfd", 9u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b24u: goto read_item_offset;
    case 0x808b27u: goto draw_item_name;
    case 0x808b2au: goto resume_string_reader;
    default: return 0;
    }
read_item_offset: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b24u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b24u, 0x20u, 0x808bb2u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_item_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b27u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b27u, 0x20u, 0x808b73u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_string_reader: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b2au, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuSpellPointerNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b2du && pc < 0x808b36u)
        end_pc = 0x808b36u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b2du: byte_accumulator = true; break;
    case 0x808b30u: byte_accumulator = true; break;
    case 0x808b33u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b2du;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x20\xa7\x8b\x20\x57\x8b\x82\x74\xfd", 9u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b2du: goto read_spell_pointer;
    case 0x808b30u: goto draw_spell_name;
    case 0x808b33u: goto resume_string_reader;
    default: return 0;
    }
read_spell_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b2du);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b2du, 0x20u, 0x808ba7u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_spell_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b30u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b30u, 0x20u, 0x808b57u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_string_reader: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b33u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuSpellOffsetNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b36u && pc < 0x808b3fu)
        end_pc = 0x808b3fu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b36u: byte_accumulator = true; break;
    case 0x808b39u: byte_accumulator = true; break;
    case 0x808b3cu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b36u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x20\xb2\x8b\x20\x57\x8b\x82\x6b\xfd", 9u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b36u: goto read_spell_offset;
    case 0x808b39u: goto draw_spell_name;
    case 0x808b3cu: goto resume_string_reader;
    default: return 0;
    }
read_spell_offset: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b36u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b36u, 0x20u, 0x808bb2u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_spell_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b39u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b39u, 0x20u, 0x808b57u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
resume_string_reader: {
    if (!NativeSpanCanStep(&span, cpu, 4u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b3cu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 4u, 0x88aau);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuMemberNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b3fu && pc < 0x808b56u)
        end_pc = 0x808b56u;
    else if (pc >= 0x808b56u && pc < 0x808b57u)
        end_pc = 0x808b57u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b3fu: byte_accumulator = true; break;
    case 0x808b41u: byte_accumulator = true; break;
    case 0x808b42u: byte_accumulator = true; break;
    case 0x808b44u: byte_accumulator = true; break;
    case 0x808b45u: width_neutral = true; break;
    case 0x808b4du: width_neutral = true; break;
    case 0x808b4fu: byte_accumulator = true; break;
    case 0x808b51u: byte_accumulator = true; break;
    case 0x808b53u: byte_accumulator = true; break;
    case 0x808b56u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b3fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 24u || memcmp(cart->rom + first,
                                              "\xa9\x00\xeb\xb2\x56\x0a\xc2\x20\xaa\xbd\x80\x0a\x85\x60\xe2"
                                              "\x20\xa9\x00\x85\x62\x20\xca\x8b\x60",
                                              24u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b3fu: goto clear_member_index_high;
    case 0x808b41u: goto exchange_member_index;
    case 0x808b42u: goto read_member_index;
    case 0x808b44u: goto member_table_offset;
    case 0x808b45u: goto widen_member_pointer;
    case 0x808b47u: goto select_member_pointer;
    case 0x808b48u: goto read_member_pointer;
    case 0x808b4bu: goto store_member_pointer;
    case 0x808b4du: goto narrow_member_bank;
    case 0x808b4fu: goto member_name_bank;
    case 0x808b51u: goto store_member_bank;
    case 0x808b53u: goto draw_member_string;
    case 0x808b56u: goto return_member_name;
    default: return 0;
    }
clear_member_index_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b3fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b41u);
}
exchange_member_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b41u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b42u);
}
read_member_index: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x56u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
    const uint32_t address = ((uint32_t)registers->db << 16) | expected;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b42u, 2u, span.instructions != 0u);
    const uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    const uint32_t source = ((uint32_t)registers->db << 16) | pointer;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
member_table_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b44u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b45u);
}
widen_member_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b45u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b47u);
}
select_member_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b47u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b48u);
}
read_member_pointer: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a80u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0a80u >> 8) != ((0x0a80u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b48u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b4bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_member_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b4bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b4du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_member_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b4du, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b4fu);
}
member_name_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b4fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b51u);
}
store_member_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x62u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b51u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b53u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
draw_member_string: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b53u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b53u, 0x20u, 0x808bcau, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_member_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b56u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x808b56u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuSpellNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                            uint64_t dynamic_epoch, uint64_t write_epoch,
                                            unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b57u && pc < 0x808b72u)
        end_pc = 0x808b72u;
    else if (pc >= 0x808b72u && pc < 0x808b73u)
        end_pc = 0x808b73u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b57u: width_neutral = true; break;
    case 0x808b5eu: width_neutral = true; break;
    case 0x808b60u: byte_accumulator = true; break;
    case 0x808b61u: byte_accumulator = true; break;
    case 0x808b65u: byte_accumulator = true; break;
    case 0x808b66u: byte_accumulator = true; break;
    case 0x808b69u: byte_accumulator = true; break;
    case 0x808b6bu: byte_accumulator = true; break;
    case 0x808b6du: byte_accumulator = true; break;
    case 0x808b6fu: byte_accumulator = true; break;
    case 0x808b72u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b57u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 28u || memcmp(cart->rom + first,
                                              "\xc2\x20\xb2\x56\x8d\x0b\x0a\xe2\x20\x5a\x22\x14\xf4\x81\x7a"
                                              "\xa2\x77\x0b\x86\x60\xa9\x00\x85\x62\x20\xca\x8b\x60",
                                              28u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b57u: goto widen_spell_id;
    case 0x808b59u: goto read_spell_id;
    case 0x808b5bu: goto store_spell_id;
    case 0x808b5eu: goto narrow_spell_record;
    case 0x808b60u: goto save_string_position;
    case 0x808b61u: goto load_spell_record;
    case 0x808b65u: goto restore_string_position;
    case 0x808b66u: goto spell_name_pointer;
    case 0x808b69u: goto store_spell_name_pointer;
    case 0x808b6bu: goto spell_name_bank;
    case 0x808b6du: goto store_spell_name_bank;
    case 0x808b6fu: goto draw_spell_string;
    case 0x808b72u: goto return_spell_name;
    default: return 0;
    }
widen_spell_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b57u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b59u);
}
read_spell_id: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x56u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
    const uint32_t address = ((uint32_t)registers->db << 16) | expected;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b59u, 2u, span.instructions != 0u);
    const uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    const uint32_t source = ((uint32_t)registers->db << 16) | pointer;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b5bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_spell_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a0bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b5bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_spell_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b5eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b60u);
}
save_string_position: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808b60u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8b61u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_spell_record: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b61u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b61u, 0x22u, 0x81f414u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_string_position: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808b65u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8b66u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spell_name_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b66u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0b77u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b69u);
}
store_spell_name_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b69u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b6bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spell_name_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b6bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b6du);
}
store_spell_name_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x62u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b6du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b6fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
draw_spell_string: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b6fu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b6fu, 0x20u, 0x808bcau, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_spell_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b72u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x808b72u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuItemNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808b73u && pc < 0x808ba6u)
        end_pc = 0x808ba6u;
    else if (pc >= 0x808ba6u && pc < 0x808ba7u)
        end_pc = 0x808ba7u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808b73u: width_neutral = true; break;
    case 0x808b7au: width_neutral = true; break;
    case 0x808b7cu: byte_accumulator = true; break;
    case 0x808b7du: byte_accumulator = true; break;
    case 0x808b81u: byte_accumulator = true; break;
    case 0x808b82u: byte_accumulator = true; break;
    case 0x808b85u: byte_accumulator = true; break;
    case 0x808b87u: byte_accumulator = true; break;
    case 0x808b89u: byte_accumulator = true; break;
    case 0x808b8bu: byte_accumulator = true; break;
    case 0x808b8eu: byte_accumulator = true; break;
    case 0x808b91u: byte_accumulator = true; break;
    case 0x808b94u: byte_accumulator = true; break;
    case 0x808b97u: byte_accumulator = true; break;
    case 0x808b99u: byte_accumulator = true; break;
    case 0x808b9bu: byte_accumulator = true; break;
    case 0x808b9eu: byte_accumulator = true; break;
    case 0x808b9fu: byte_accumulator = true; break;
    case 0x808ba0u: byte_accumulator = true; break;
    case 0x808ba3u: byte_accumulator = true; break;
    case 0x808ba6u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808b73u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 52u ||
        memcmp(cart->rom + first,
               "\xc2\x20\xb2\x56\x8d\x06\x0a\xe2\x20\x5a\x22\xc5\xf1\x81\x7a\xa2\x77\x0b\x86\x60\xa9\x00\x85"
               "\x62\xae\x75\x05\xad\x87\x0b\x9d\x40\x00\xad\x64\x05\x29\xe0\x09\x1c\x9d\x41\x00\xe8\xe8\x8e"
               "\x75\x05\x20\xca\x8b\x60",
               52u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808b73u: goto widen_item_id;
    case 0x808b75u: goto read_item_id;
    case 0x808b77u: goto store_item_id;
    case 0x808b7au: goto narrow_item_record;
    case 0x808b7cu: goto save_string_position;
    case 0x808b7du: goto load_item_record;
    case 0x808b81u: goto restore_string_position;
    case 0x808b82u: goto item_name_pointer;
    case 0x808b85u: goto store_item_name_pointer;
    case 0x808b87u: goto item_name_bank;
    case 0x808b89u: goto store_item_name_bank;
    case 0x808b8bu: goto read_icon_position;
    case 0x808b8eu: goto read_item_icon;
    case 0x808b91u: goto store_item_icon;
    case 0x808b94u: goto read_icon_attributes;
    case 0x808b97u: goto mask_icon_attributes;
    case 0x808b99u: goto item_icon_attributes;
    case 0x808b9bu: goto store_icon_attributes;
    case 0x808b9eu: goto next_icon_byte;
    case 0x808b9fu: goto next_icon_word;
    case 0x808ba0u: goto store_icon_position;
    case 0x808ba3u: goto draw_item_string;
    case 0x808ba6u: goto return_item_name;
    default: return 0;
    }
widen_item_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b73u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b75u);
}
read_item_id: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x56u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
    const uint32_t address = ((uint32_t)registers->db << 16) | expected;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b75u, 2u, span.instructions != 0u);
    const uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    const uint32_t source = ((uint32_t)registers->db << 16) | pointer;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b77u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_item_id: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a06u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b77u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b7au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_item_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b7au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b7cu);
}
save_string_position: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808b7cu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x8b7du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_record: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808b7du);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808b7du, 0x22u, 0x81f1c5u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_string_position: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x808b81u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8b82u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
item_name_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b82u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0b77u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8b85u);
}
store_item_name_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b85u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b87u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
item_name_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b87u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b89u);
}
store_item_name_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x62u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b89u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b8bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_icon_position: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0575u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b8bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8b8eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_item_icon: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0b87u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b8eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b91u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_item_icon: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0040u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b91u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b94u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_icon_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0564u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b94u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b97u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_icon_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b97u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00e0u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b99u);
}
item_icon_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b99u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x001cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b9bu);
}
store_icon_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0041u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b9bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8b9eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_icon_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b9eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8b9fu);
}
next_icon_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808b9fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8ba0u);
}
store_icon_position: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0575u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808ba0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, 5u, 0x8ba3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
draw_item_string: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808ba3u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x808ba3u, 0x20u, 0x808bcau, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_item_name: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808ba6u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x808ba6u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuNamePointerOffsetSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                    uint8_t opcode, uint64_t dynamic_epoch,
                                                    uint64_t write_epoch, unsigned step_budget,
                                                    long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x808bb2u && pc < 0x808bc9u)
        end_pc = 0x808bc9u;
    else if (pc >= 0x808bc9u && pc < 0x808bcau)
        end_pc = 0x808bcau;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x808bb2u: width_neutral = true; break;
    case 0x808bc7u: width_neutral = true; break;
    case 0x808bc9u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x808bb2u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 24u || memcmp(cart->rom + first,
                                              "\xc2\x20\xb7\x5d\x85\x60\xc8\xc8\xb7\x5d\x85\x56\xc8\xc8\xb2"
                                              "\x60\x18\x65\x56\x85\x56\xe2\x20\x60",
                                              24u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x808bb2u: goto widen_pointer_operands;
    case 0x808bb4u: goto read_pointer_operand;
    case 0x808bb6u: goto store_pointer_operand;
    case 0x808bb8u: goto next_pointer_byte;
    case 0x808bb9u: goto next_pointer_word;
    case 0x808bbau: goto read_offset_operand;
    case 0x808bbcu: goto store_offset_operand;
    case 0x808bbeu: goto next_offset_byte;
    case 0x808bbfu: goto next_offset_word;
    case 0x808bc0u: goto read_pointer_base;
    case 0x808bc2u: goto clear_offset_carry;
    case 0x808bc3u: goto add_pointer_offset;
    case 0x808bc5u: goto store_effective_pointer;
    case 0x808bc7u: goto narrow_pointer_result;
    case 0x808bc9u: goto return_pointer_offset;
    default: return 0;
    }
widen_pointer_operands: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bb2u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8bb4u);
}
read_pointer_operand: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bb4u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bb6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_pointer_operand: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bb6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bb8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_pointer_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bb8u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8bb9u);
}
next_pointer_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bb9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8bbau);
}
read_offset_operand: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bbau, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bbcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_offset_operand: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bbcu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bbeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_offset_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bbeu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8bbfu);
}
next_offset_word: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bbfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8bc0u);
}
read_pointer_base: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x60u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const unsigned expected = cpu->ram[pointer_low] | ((unsigned)cpu->ram[pointer_high] << 8);
    const uint32_t address = ((uint32_t)registers->db << 16) | expected;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bc0u, 2u, span.instructions != 0u);
    const uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    const uint32_t source = ((uint32_t)registers->db << 16) | pointer;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bc2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_offset_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bc2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8bc3u);
}
add_pointer_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bc3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bc5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_effective_pointer: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bc5u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8bc7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_pointer_result: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x808bc7u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8bc9u);
    goto finished;
}
return_pointer_offset: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x808bc9u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x808bc9u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

static int Lufia2DecompRunMenuNameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x808b09u && pc <= 0x808b0fu)
        return Lufia2DecompRunMenuMemberPointerNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b12u && pc <= 0x808b18u)
        return Lufia2DecompRunMenuMemberOffsetNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b1bu && pc <= 0x808b21u)
        return Lufia2DecompRunMenuItemPointerNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b24u && pc <= 0x808b2au)
        return Lufia2DecompRunMenuItemOffsetNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b2du && pc <= 0x808b33u)
        return Lufia2DecompRunMenuSpellPointerNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b36u && pc <= 0x808b3cu)
        return Lufia2DecompRunMenuSpellOffsetNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b3fu && pc <= 0x808b56u)
        return Lufia2DecompRunMenuMemberNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b57u && pc <= 0x808b72u)
        return Lufia2DecompRunMenuSpellNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808b73u && pc <= 0x808ba6u)
        return Lufia2DecompRunMenuItemNameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x808bb2u && pc <= 0x808bc9u)
        return Lufia2DecompRunMenuNamePointerOffsetSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
