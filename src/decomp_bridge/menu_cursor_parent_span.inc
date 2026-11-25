/* Calls, returns and the table jump start their own phase: the interpreter loop
 * takes its call and menu-return decisions from the first opcode of a span. */
#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunMenuPresentDisplaySpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x828704u && pc < 0x82870bu)
        end_pc = 0x82870bu;
    else if (pc >= 0x82870bu && pc < 0x82870fu)
        end_pc = 0x82870fu;
    else if (pc >= 0x82870fu && pc < 0x828713u)
        end_pc = 0x828713u;
    else if (pc >= 0x828713u && pc < 0x82871bu)
        end_pc = 0x82871bu;
    else if (pc >= 0x82871bu && pc < 0x82871fu)
        end_pc = 0x82871fu;
    else if (pc >= 0x82871fu && pc < 0x828720u)
        end_pc = 0x828720u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828704u: byte_accumulator = true; break;
    case 0x828707u: byte_accumulator = true; break;
    case 0x828709u: byte_accumulator = true; break;
    case 0x82870bu: byte_accumulator = true; break;
    case 0x82870fu: byte_accumulator = true; break;
    case 0x828713u: byte_accumulator = true; break;
    case 0x828715u: byte_accumulator = true; break;
    case 0x828717u: byte_accumulator = true; break;
    case 0x828719u: byte_accumulator = true; break;
    case 0x82871bu: byte_accumulator = true; break;
    case 0x82871fu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x828704u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 28u || memcmp(cart->rom + first,
                                              "\xa2\xff\xff\x86\x4a\x86\x4c\x22\xcf\x8b\x86\x22\xf5\x8b\x86"
                                              "\xa9\x88\x85\x74\xa9\x80\x85\x72\x22\x1c\x8b\x86\x60",
                                              28u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x828704u: goto set_x;
    case 0x828707u: goto store_x_oam_first;
    case 0x828709u: goto store_x_oam_second;
    case 0x82870bu: goto call_868bcf;
    case 0x82870fu: goto call_868bf5;
    case 0x828713u: goto set_accumulator;
    case 0x828715u: goto store_request;
    case 0x828717u: goto set_accumulator_2;
    case 0x828719u: goto store_second_request;
    case 0x82871bu: goto call_868b1c;
    case 0x82871fu: goto return_caller;
    default: return 0;
    }
set_x: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828704u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0xffffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8707u);
}
store_x_oam_first: {
    const uint32_t address = (uint16_t)(registers->dp + 0x4au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828707u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8709u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_x_oam_second: {
    const uint32_t address = (uint16_t)(registers->dp + 0x4cu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828709u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x870bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_868bcf: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82870bu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82870bu, 0x22u, 0x868bcfu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_868bf5: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82870fu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82870fu, 0x22u, 0x868bf5u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828713u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0088u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8715u);
}
store_request: {
    const uint32_t address = (uint16_t)(registers->dp + 0x74u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828715u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8717u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828717u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8719u);
}
store_second_request: {
    const uint32_t address = (uint16_t)(registers->dp + 0x72u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828719u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x871bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_868b1c: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82871bu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82871bu, 0x22u, 0x868b1cu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82871fu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82871fu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuCursorInputSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x828720u && pc < 0x828750u)
        end_pc = 0x828750u;
    else if (pc >= 0x828750u && pc < 0x828753u)
        end_pc = 0x828753u;
    else if (pc >= 0x828753u && pc < 0x828760u)
        end_pc = 0x828760u;
    else if (pc >= 0x828760u && pc < 0x828764u)
        end_pc = 0x828764u;
    else if (pc >= 0x828764u && pc < 0x82878eu)
        end_pc = 0x82878eu;
    else if (pc >= 0x82878eu && pc < 0x828791u)
        end_pc = 0x828791u;
    else if (pc >= 0x828791u && pc < 0x8287a8u)
        end_pc = 0x8287a8u;
    else if (pc >= 0x8287a8u && pc < 0x8287acu)
        end_pc = 0x8287acu;
    else if (pc >= 0x8287acu && pc < 0x8287b5u)
        end_pc = 0x8287b5u;
    else if (pc >= 0x8287b5u && pc < 0x8287b9u)
        end_pc = 0x8287b9u;
    else if (pc >= 0x8287b9u && pc < 0x8287cfu)
        end_pc = 0x8287cfu;
    else if (pc >= 0x8287cfu && pc < 0x8287d2u)
        end_pc = 0x8287d2u;
    else if (pc >= 0x8287d2u && pc < 0x8287e6u)
        end_pc = 0x8287e6u;
    else if (pc >= 0x8287e6u && pc < 0x8287eau)
        end_pc = 0x8287eau;
    else if (pc >= 0x8287eau && pc < 0x8287f2u)
        end_pc = 0x8287f2u;
    else if (pc >= 0x8287f2u && pc < 0x8287f5u)
        end_pc = 0x8287f5u;
    else if (pc >= 0x8287f5u && pc < 0x828812u)
        end_pc = 0x828812u;
    else if (pc >= 0x828812u && pc < 0x828815u)
        end_pc = 0x828815u;
    else if (pc >= 0x828815u && pc < 0x828823u)
        end_pc = 0x828823u;
    else if (pc >= 0x828823u && pc < 0x828827u)
        end_pc = 0x828827u;
    else if (pc >= 0x828827u && pc < 0x82882au)
        end_pc = 0x82882au;
    else if (pc >= 0x82882au && pc < 0x828836u)
        end_pc = 0x828836u;
    else if (pc >= 0x828836u && pc < 0x828839u)
        end_pc = 0x828839u;
    else if (pc >= 0x828839u && pc < 0x828845u)
        end_pc = 0x828845u;
    else if (pc >= 0x828845u && pc < 0x828848u)
        end_pc = 0x828848u;
    else if (pc >= 0x828848u && pc < 0x828856u)
        end_pc = 0x828856u;
    else if (pc >= 0x828856u && pc < 0x82885au)
        end_pc = 0x82885au;
    else if (pc >= 0x82885au && pc < 0x82885du)
        end_pc = 0x82885du;
    else if (pc >= 0x82885du && pc < 0x828868u)
        end_pc = 0x828868u;
    else if (pc >= 0x828868u && pc < 0x82886cu)
        end_pc = 0x82886cu;
    else if (pc >= 0x82886cu && pc < 0x82886fu)
        end_pc = 0x82886fu;
    else if (pc >= 0x82886fu && pc < 0x82887bu)
        end_pc = 0x82887bu;
    else if (pc >= 0x82887bu && pc < 0x82887eu)
        end_pc = 0x82887eu;
    else if (pc >= 0x82887eu && pc < 0x82888au)
        end_pc = 0x82888au;
    else if (pc >= 0x82888au && pc < 0x82888du)
        end_pc = 0x82888du;
    else if (pc >= 0x82888du && pc < 0x828894u)
        end_pc = 0x828894u;
    else if (pc >= 0x828894u && pc < 0x828897u)
        end_pc = 0x828897u;
    else if (pc >= 0x828897u && pc < 0x82889au)
        end_pc = 0x82889au;
    else if (pc >= 0x82889au && pc < 0x82889cu)
        end_pc = 0x82889cu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x828720u: byte_accumulator = true; break;
    case 0x828723u: byte_accumulator = true; break;
    case 0x828725u: byte_accumulator = true; break;
    case 0x828727u: byte_accumulator = true; break;
    case 0x82872au: byte_accumulator = true; break;
    case 0x82872du: byte_accumulator = true; break;
    case 0x82872fu: byte_accumulator = true; break;
    case 0x828731u: byte_accumulator = true; break;
    case 0x828734u: byte_accumulator = true; break;
    case 0x828736u: byte_accumulator = true; break;
    case 0x828739u: byte_accumulator = true; break;
    case 0x82873bu: byte_accumulator = true; break;
    case 0x82873eu: byte_accumulator = true; break;
    case 0x828741u: byte_accumulator = true; break;
    case 0x828744u: byte_accumulator = true; break;
    case 0x828747u: byte_accumulator = true; break;
    case 0x828748u: byte_accumulator = true; break;
    case 0x82874bu: byte_accumulator = true; break;
    case 0x82874eu: byte_accumulator = true; break;
    case 0x828750u: byte_accumulator = true; break;
    case 0x828753u: byte_accumulator = true; break;
    case 0x828756u: byte_accumulator = true; break;
    case 0x828759u: byte_accumulator = true; break;
    case 0x82875bu: byte_accumulator = true; break;
    case 0x82875eu: byte_accumulator = true; break;
    case 0x828760u: byte_accumulator = true; break;
    case 0x828764u: byte_accumulator = true; break;
    case 0x828767u: byte_accumulator = true; break;
    case 0x828769u: byte_accumulator = true; break;
    case 0x82876bu: byte_accumulator = true; break;
    case 0x82876eu: byte_accumulator = true; break;
    case 0x828771u: byte_accumulator = true; break;
    case 0x828773u: byte_accumulator = true; break;
    case 0x828776u: byte_accumulator = true; break;
    case 0x828779u: byte_accumulator = true; break;
    case 0x82877bu: byte_accumulator = true; break;
    case 0x82877eu: byte_accumulator = true; break;
    case 0x828780u: byte_accumulator = true; break;
    case 0x828783u: byte_accumulator = true; break;
    case 0x828786u: byte_accumulator = true; break;
    case 0x828789u: byte_accumulator = true; break;
    case 0x82878cu: byte_accumulator = true; break;
    case 0x82878eu: byte_accumulator = true; break;
    case 0x828791u: byte_accumulator = true; break;
    case 0x828794u: byte_accumulator = true; break;
    case 0x828797u: byte_accumulator = true; break;
    case 0x828799u: byte_accumulator = true; break;
    case 0x82879bu: byte_accumulator = true; break;
    case 0x82879eu: byte_accumulator = true; break;
    case 0x8287a1u: byte_accumulator = true; break;
    case 0x8287a3u: byte_accumulator = true; break;
    case 0x8287a6u: byte_accumulator = true; break;
    case 0x8287a8u: byte_accumulator = true; break;
    case 0x8287acu: byte_accumulator = true; break;
    case 0x8287afu: byte_accumulator = true; break;
    case 0x8287b1u: byte_accumulator = true; break;
    case 0x8287b3u: byte_accumulator = true; break;
    case 0x8287b5u: byte_accumulator = true; break;
    case 0x8287b9u: byte_accumulator = true; break;
    case 0x8287bcu: byte_accumulator = true; break;
    case 0x8287bfu: byte_accumulator = true; break;
    case 0x8287c1u: byte_accumulator = true; break;
    case 0x8287c3u: byte_accumulator = true; break;
    case 0x8287c6u: byte_accumulator = true; break;
    case 0x8287c7u: byte_accumulator = true; break;
    case 0x8287cau: byte_accumulator = true; break;
    case 0x8287cdu: byte_accumulator = true; break;
    case 0x8287cfu: byte_accumulator = true; break;
    case 0x8287d2u: byte_accumulator = true; break;
    case 0x8287d5u: byte_accumulator = true; break;
    case 0x8287d8u: byte_accumulator = true; break;
    case 0x8287dau: byte_accumulator = true; break;
    case 0x8287ddu: byte_accumulator = true; break;
    case 0x8287e0u: byte_accumulator = true; break;
    case 0x8287e2u: byte_accumulator = true; break;
    case 0x8287e4u: byte_accumulator = true; break;
    case 0x8287e6u: byte_accumulator = true; break;
    case 0x8287eau: byte_accumulator = true; break;
    case 0x8287edu: byte_accumulator = true; break;
    case 0x8287f0u: byte_accumulator = true; break;
    case 0x8287f2u: byte_accumulator = true; break;
    case 0x8287f5u: byte_accumulator = true; break;
    case 0x8287f8u: byte_accumulator = true; break;
    case 0x8287fbu: byte_accumulator = true; break;
    case 0x8287fdu: byte_accumulator = true; break;
    case 0x828800u: byte_accumulator = true; break;
    case 0x828803u: byte_accumulator = true; break;
    case 0x828805u: byte_accumulator = true; break;
    case 0x828808u: byte_accumulator = true; break;
    case 0x82880bu: byte_accumulator = true; break;
    case 0x82880eu: byte_accumulator = true; break;
    case 0x828810u: byte_accumulator = true; break;
    case 0x828812u: byte_accumulator = true; break;
    case 0x828815u: byte_accumulator = true; break;
    case 0x828817u: byte_accumulator = true; break;
    case 0x82881au: byte_accumulator = true; break;
    case 0x82881du: byte_accumulator = true; break;
    case 0x82881fu: byte_accumulator = true; break;
    case 0x828821u: byte_accumulator = true; break;
    case 0x828823u: byte_accumulator = true; break;
    case 0x828827u: byte_accumulator = true; break;
    case 0x82882au: byte_accumulator = true; break;
    case 0x82882cu: byte_accumulator = true; break;
    case 0x82882fu: byte_accumulator = true; break;
    case 0x828832u: byte_accumulator = true; break;
    case 0x828834u: byte_accumulator = true; break;
    case 0x828836u: byte_accumulator = true; break;
    case 0x828839u: byte_accumulator = true; break;
    case 0x82883bu: byte_accumulator = true; break;
    case 0x82883eu: byte_accumulator = true; break;
    case 0x828841u: byte_accumulator = true; break;
    case 0x828843u: byte_accumulator = true; break;
    case 0x828845u: byte_accumulator = true; break;
    case 0x828848u: byte_accumulator = true; break;
    case 0x82884au: byte_accumulator = true; break;
    case 0x82884du: byte_accumulator = true; break;
    case 0x828850u: byte_accumulator = true; break;
    case 0x828852u: byte_accumulator = true; break;
    case 0x828854u: byte_accumulator = true; break;
    case 0x828856u: byte_accumulator = true; break;
    case 0x82885au: byte_accumulator = true; break;
    case 0x82885du: byte_accumulator = true; break;
    case 0x82885fu: byte_accumulator = true; break;
    case 0x828862u: byte_accumulator = true; break;
    case 0x828864u: byte_accumulator = true; break;
    case 0x828866u: byte_accumulator = true; break;
    case 0x828868u: byte_accumulator = true; break;
    case 0x82886cu: byte_accumulator = true; break;
    case 0x82886fu: byte_accumulator = true; break;
    case 0x828871u: byte_accumulator = true; break;
    case 0x828874u: byte_accumulator = true; break;
    case 0x828877u: byte_accumulator = true; break;
    case 0x828879u: byte_accumulator = true; break;
    case 0x82887bu: byte_accumulator = true; break;
    case 0x82887eu: byte_accumulator = true; break;
    case 0x828880u: byte_accumulator = true; break;
    case 0x828883u: byte_accumulator = true; break;
    case 0x828886u: byte_accumulator = true; break;
    case 0x828888u: byte_accumulator = true; break;
    case 0x82888au: byte_accumulator = true; break;
    case 0x82888du: byte_accumulator = true; break;
    case 0x82888fu: byte_accumulator = true; break;
    case 0x828892u: width_neutral = true; break;
    case 0x828894u: byte_accumulator = true; break;
    case 0x828897u: byte_accumulator = true; break;
    case 0x82889au: byte_accumulator = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x828720u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 380u ||
        memcmp(
            cart->rom + first,
            "\xad\xac\x14\x89\x08\xf0\x40\xde\xeb\x14\xbd\xeb\x14\xc9\x00\x10\x2d\xbd\xc1\x14\xf0\x0e\xbd\xc1"
            "\x14\x09\x80\x9d\xc1\x14\x9e\xeb\x14\x4c\x92\x88\xbd\xf7\x14\x3a\x9d\xeb\x14\xbd\xc7\x14\xf0\x0e"
            "\x20\xa0\x88\xad\xb3\x14\xdd\xc7\x14\x30\x4b\x9e\xeb\x14\xa9\x00\x22\x3b\x95\x80\x4c\x92\x88\x89"
            "\x04\xf0\x44\xfe\xeb\x14\xbd\xc1\x14\xf0\x16\xbd\xeb\x14\xdd\xf7\x14\x30\x2b\xbd\xc1\x14\x09\x40"
            "\x9d\xc1\x14\xde\xeb\x14\x4c\x92\x88\xbd\xc7\x14\xf0\x0d\x20\xa0\x88\xad\xb3\x14\xdd\xc7\x14\x30"
            "\x0d\x80\x08\xbd\xeb\x14\xdd\xf7\x14\x30\x03\x9e\xeb\x14\xa9\x00\x22\x3b\x95\x80\x4c\x92\x88\x89"
            "\x02\xf0\x2d\xa9\x00\x22\x3b\x95\x80\xde\xd9\x14\xbd\xd9\x14\xc9\x00\x10\x1a\xbd\xf1\x14\x3a\x9d"
            "\xd9\x14\xbd\xc7\x14\xf0\x0e\x20\xa0\x88\xad\xb3\x14\xdd\xc7\x14\x30\x03\x9e\xd9\x14\x4c\x92\x88"
            "\x89\x01\xf0\x27\xa9\x00\x22\x3b\x95\x80\xfe\xd9\x14\xbd\xc7\x14\xf0\x0b\x20\xa0\x88\xad\xb3\x14"
            "\xdd\xc7\x14\x10\x08\xbd\xd9\x14\xdd\xf1\x14\x30\x03\x9e\xd9\x14\x4c\x92\x88\xad\xab\x14\x89\x80"
            "\xf0\x08\x20\xa0\x88\xa9\x02\x4c\x9c\x88\xad\xac\x14\x89\x80\xf0\x0e\xa9\x01\x22\x3b\x95\x80\x20"
            "\xa0\x88\xa9\x03\x4c\x9c\x88\xad\xab\x14\x89\x40\xf0\x08\x20\xa0\x88\xa9\x04\x4c\x9c\x88\xad\xac"
            "\x14\x89\x40\xf0\x08\x20\xa0\x88\xa9\x05\x4c\x9c\x88\xad\xab\x14\x89\x20\xf0\x0e\xa9\x00\x22\x3b"
            "\x95\x80\x20\xa0\x88\xa9\x06\x4c\x9c\x88\x89\x10\xf0\x0e\xa9\x00\x22\x3b\x95\x80\x20\xa0\x88\xa9"
            "\x07\x4c\x9c\x88\xad\xac\x14\x89\x10\xf0\x08\x20\xa0\x88\xa9\x08\x4c\x9c\x88\xad\xac\x14\x89\x20"
            "\xf0\x14\x20\xa0\x88\xa9\x09\x4c\x9c\x88\xe2\x20\x20\xa0\x88\x20\xcb\x88\xa9\x01",
            380u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x828720u: goto load_pressed_b;
    case 0x828723u: goto test_accumulator_bits;
    case 0x828725u: goto branch_equal;
    case 0x828727u: goto decrement_item_row;
    case 0x82872au: goto load_item_row;
    case 0x82872du: goto compare_accumulator;
    case 0x82872fu: goto branch_positive;
    case 0x828731u: goto load_item_flags;
    case 0x828734u: goto branch_equal_2;
    case 0x828736u: goto load_item_flags_2;
    case 0x828739u: goto set_accumulator_bits;
    case 0x82873bu: goto store_item_flags;
    case 0x82873eu: goto clear_item_row;
    case 0x828741u: goto skip_ahead;
    case 0x828744u: goto load_item_rows;
    case 0x828747u: goto decrement_accumulator;
    case 0x828748u: goto store_item_row;
    case 0x82874bu: goto load_item_limit;
    case 0x82874eu: goto branch_equal_3;
    case 0x828750u: goto call_8288a0;
    case 0x828753u: goto load_list_index;
    case 0x828756u: goto compare_item_limit;
    case 0x828759u: goto branch_negative;
    case 0x82875bu: goto clear_item_row_2;
    case 0x82875eu: goto set_accumulator;
    case 0x828760u: goto call_80953b;
    case 0x828764u: goto skip_ahead_2;
    case 0x828767u: goto test_accumulator_bits_2;
    case 0x828769u: goto branch_equal_4;
    case 0x82876bu: goto increment_item_row;
    case 0x82876eu: goto load_item_flags_3;
    case 0x828771u: goto branch_equal_5;
    case 0x828773u: goto load_item_row_2;
    case 0x828776u: goto compare_item_rows;
    case 0x828779u: goto branch_negative_2;
    case 0x82877bu: goto load_item_flags_4;
    case 0x82877eu: goto set_accumulator_bits_2;
    case 0x828780u: goto store_item_flags_2;
    case 0x828783u: goto decrement_item_row_2;
    case 0x828786u: goto skip_ahead_3;
    case 0x828789u: goto load_item_limit_2;
    case 0x82878cu: goto branch_equal_6;
    case 0x82878eu: goto call_8288a0_2;
    case 0x828791u: goto load_list_index_2;
    case 0x828794u: goto compare_item_limit_2;
    case 0x828797u: goto branch_negative_3;
    case 0x828799u: goto skip_ahead_4;
    case 0x82879bu: goto load_item_row_3;
    case 0x82879eu: goto compare_item_rows_2;
    case 0x8287a1u: goto branch_negative_4;
    case 0x8287a3u: goto clear_item_row_3;
    case 0x8287a6u: goto set_accumulator_2;
    case 0x8287a8u: goto call_80953b_2;
    case 0x8287acu: goto skip_ahead_5;
    case 0x8287afu: goto test_accumulator_bits_3;
    case 0x8287b1u: goto branch_equal_7;
    case 0x8287b3u: goto set_accumulator_3;
    case 0x8287b5u: goto call_80953b_3;
    case 0x8287b9u: goto decrement_item_column;
    case 0x8287bcu: goto load_item_column;
    case 0x8287bfu: goto compare_accumulator_2;
    case 0x8287c1u: goto branch_positive_2;
    case 0x8287c3u: goto load_item_columns;
    case 0x8287c6u: goto decrement_accumulator_2;
    case 0x8287c7u: goto store_item_column;
    case 0x8287cau: goto load_item_limit_3;
    case 0x8287cdu: goto branch_equal_8;
    case 0x8287cfu: goto call_8288a0_3;
    case 0x8287d2u: goto load_list_index_3;
    case 0x8287d5u: goto compare_item_limit_3;
    case 0x8287d8u: goto branch_negative_5;
    case 0x8287dau: goto clear_item_column;
    case 0x8287ddu: goto skip_ahead_6;
    case 0x8287e0u: goto test_accumulator_bits_4;
    case 0x8287e2u: goto branch_equal_9;
    case 0x8287e4u: goto set_accumulator_4;
    case 0x8287e6u: goto call_80953b_4;
    case 0x8287eau: goto increment_item_column;
    case 0x8287edu: goto load_item_limit_4;
    case 0x8287f0u: goto branch_equal_10;
    case 0x8287f2u: goto call_8288a0_4;
    case 0x8287f5u: goto load_list_index_4;
    case 0x8287f8u: goto compare_item_limit_4;
    case 0x8287fbu: goto branch_positive_3;
    case 0x8287fdu: goto load_item_column_2;
    case 0x828800u: goto compare_item_columns;
    case 0x828803u: goto branch_negative_6;
    case 0x828805u: goto clear_item_column_2;
    case 0x828808u: goto skip_ahead_7;
    case 0x82880bu: goto load_pressed_a;
    case 0x82880eu: goto test_accumulator_bits_5;
    case 0x828810u: goto branch_equal_11;
    case 0x828812u: goto call_8288a0_5;
    case 0x828815u: goto set_accumulator_5;
    case 0x828817u: goto skip_ahead_8;
    case 0x82881au: goto load_pressed_b_2;
    case 0x82881du: goto test_accumulator_bits_6;
    case 0x82881fu: goto branch_equal_12;
    case 0x828821u: goto set_accumulator_6;
    case 0x828823u: goto call_80953b_5;
    case 0x828827u: goto call_8288a0_6;
    case 0x82882au: goto set_accumulator_7;
    case 0x82882cu: goto skip_ahead_9;
    case 0x82882fu: goto load_pressed_a_2;
    case 0x828832u: goto test_accumulator_bits_7;
    case 0x828834u: goto branch_equal_13;
    case 0x828836u: goto call_8288a0_7;
    case 0x828839u: goto set_accumulator_8;
    case 0x82883bu: goto skip_ahead_10;
    case 0x82883eu: goto load_pressed_b_3;
    case 0x828841u: goto test_accumulator_bits_8;
    case 0x828843u: goto branch_equal_14;
    case 0x828845u: goto call_8288a0_8;
    case 0x828848u: goto set_accumulator_9;
    case 0x82884au: goto skip_ahead_11;
    case 0x82884du: goto load_pressed_a_3;
    case 0x828850u: goto test_accumulator_bits_9;
    case 0x828852u: goto branch_equal_15;
    case 0x828854u: goto set_accumulator_10;
    case 0x828856u: goto call_80953b_6;
    case 0x82885au: goto call_8288a0_9;
    case 0x82885du: goto set_accumulator_11;
    case 0x82885fu: goto skip_ahead_12;
    case 0x828862u: goto test_accumulator_bits_10;
    case 0x828864u: goto branch_equal_16;
    case 0x828866u: goto set_accumulator_12;
    case 0x828868u: goto call_80953b_7;
    case 0x82886cu: goto call_8288a0_10;
    case 0x82886fu: goto set_accumulator_13;
    case 0x828871u: goto skip_ahead_13;
    case 0x828874u: goto load_pressed_b_4;
    case 0x828877u: goto test_accumulator_bits_11;
    case 0x828879u: goto branch_equal_17;
    case 0x82887bu: goto call_8288a0_11;
    case 0x82887eu: goto set_accumulator_14;
    case 0x828880u: goto skip_ahead_14;
    case 0x828883u: goto load_pressed_b_5;
    case 0x828886u: goto test_accumulator_bits_12;
    case 0x828888u: goto branch_equal_18;
    case 0x82888au: goto call_8288a0_12;
    case 0x82888du: goto set_accumulator_15;
    case 0x82888fu: goto skip_ahead_15;
    case 0x828892u: goto narrow_accumulator;
    case 0x828894u: goto call_8288a0_13;
    case 0x828897u: goto call_8288cb;
    case 0x82889au: goto set_accumulator_16;
    default: return 0;
    }
load_pressed_b: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828720u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8723u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828723u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8725u);
}
branch_equal: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828725u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8767u : 0x8727u);
    if (taken)
        goto finished;
}
decrement_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828727u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xdeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x872au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14ebu >> 8) != ((0x14ebu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82872au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x872du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82872du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x872fu);
}
branch_positive: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82872fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x875eu : 0x8731u);
    if (taken)
        goto finished;
}
load_item_flags: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c1u >> 8) != ((0x14c1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828731u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8734u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_equal_2: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828734u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8744u : 0x8736u);
    if (taken)
        goto load_item_rows;
}
load_item_flags_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c1u >> 8) != ((0x14c1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828736u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8739u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_bits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828739u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x873bu);
}
store_item_flags: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82873bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x873eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82873eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8741u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828741u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8892u);
    goto finished;
}
load_item_rows: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14f7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14f7u >> 8) != ((0x14f7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828744u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8747u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
decrement_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828747u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8748u);
}
store_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828748u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x874bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_limit: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82874bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x874eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_equal_3: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82874eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x875eu : 0x8750u);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828750u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828750u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_list_index: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828753u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8756u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_limit: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828756u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8759u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_negative: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828759u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87a6u : 0x875bu);
    if (taken)
        goto finished;
}
clear_item_row_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82875bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x875eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82875eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8760u);
    goto finished;
}
call_80953b: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828760u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828760u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
skip_ahead_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828764u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8892u);
    goto finished;
}
test_accumulator_bits_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828767u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8769u);
}
branch_equal_4: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828769u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87afu : 0x876bu);
    if (taken)
        goto finished;
}
increment_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82876bu, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xfeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x876eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_flags_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c1u >> 8) != ((0x14c1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82876eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8771u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_equal_5: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828771u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8789u : 0x8773u);
    if (taken)
        goto load_item_limit_2;
}
load_item_row_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14ebu >> 8) != ((0x14ebu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828773u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8776u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_rows: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14f7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14f7u >> 8) != ((0x14f7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828776u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8779u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_negative_2: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828779u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87a6u : 0x877bu);
    if (taken)
        goto finished;
}
load_item_flags_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c1u >> 8) != ((0x14c1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82877bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x877eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_bits_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82877eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0040u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8780u);
}
store_item_flags_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828780u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8783u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
decrement_item_row_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828783u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xdeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8786u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead_3: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828786u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8892u);
    goto finished;
}
load_item_limit_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828789u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x878cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_equal_6: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82878cu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x879bu : 0x878eu);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_2: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82878eu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82878eu, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_list_index_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828791u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8794u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_limit_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828794u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8797u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_negative_3: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828797u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87a6u : 0x8799u);
    if (taken)
        goto set_accumulator_2;
}
skip_ahead_4: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828799u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x87a3u);
    goto finished;
}
load_item_row_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14ebu >> 8) != ((0x14ebu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82879bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x879eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_rows_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14f7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14f7u >> 8) != ((0x14f7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82879eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87a1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_negative_4: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287a1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87a6u : 0x87a3u);
    if (taken)
        goto set_accumulator_2;
}
clear_item_row_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287a3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87a6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287a6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87a8u);
    goto finished;
}
call_80953b_2: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8287a8u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8287a8u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
skip_ahead_5: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287acu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8892u);
    goto finished;
}
test_accumulator_bits_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287afu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87b1u);
}
branch_equal_7: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287b1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87e0u : 0x87b3u);
    if (taken)
        goto finished;
}
set_accumulator_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287b3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87b5u);
    goto finished;
}
call_80953b_3: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8287b5u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8287b5u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
decrement_item_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287b9u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xdeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87bcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14d9u >> 8) != ((0x14d9u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287bcu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287bfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87c1u);
}
branch_positive_2: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287c1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87ddu : 0x87c3u);
    if (taken)
        goto finished;
}
load_item_columns: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14f1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14f1u >> 8) != ((0x14f1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287c3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87c6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
decrement_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287c6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87c7u);
}
store_item_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287c7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87cau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_limit_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287cau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87cdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_equal_8: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287cdu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87ddu : 0x87cfu);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_3: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8287cfu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8287cfu, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_list_index_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287d2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87d5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_limit_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287d5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87d8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_negative_5: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287d8u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87ddu : 0x87dau);
    if (taken)
        goto skip_ahead_6;
}
clear_item_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287dau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87ddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead_6: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287ddu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8892u);
    goto finished;
}
test_accumulator_bits_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287e0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87e2u);
}
branch_equal_9: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287e2u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x880bu : 0x87e4u);
    if (taken)
        goto finished;
}
set_accumulator_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287e4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x87e6u);
    goto finished;
}
call_80953b_4: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8287e6u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8287e6u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
increment_item_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287eau, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xfeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87edu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_item_limit_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287edu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87f0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_equal_10: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287f0u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x87fdu : 0x87f2u);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_4: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8287f2u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8287f2u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_list_index_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287f5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87f8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_limit_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14c7u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14c7u >> 8) != ((0x14c7u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287f8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x87fbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_positive_3: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287fbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8805u : 0x87fdu);
    if (taken)
        goto clear_item_column_2;
}
load_item_column_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14d9u >> 8) != ((0x14d9u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8287fdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8800u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_item_columns: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14f1u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14f1u >> 8) != ((0x14f1u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828800u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8803u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_negative_6: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828803u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8808u : 0x8805u);
    if (taken)
        goto skip_ahead_7;
}
clear_item_column_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828805u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8808u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead_7: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828808u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8892u);
    goto finished;
}
load_pressed_a: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14abu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82880bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x880eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82880eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8810u);
}
branch_equal_11: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828810u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x881au : 0x8812u);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_5: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828812u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828812u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828815u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8817u);
}
skip_ahead_8: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828817u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
load_pressed_b_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82881au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x881du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_6: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82881du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x881fu);
}
branch_equal_12: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82881fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x882fu : 0x8821u);
    if (taken)
        goto finished;
}
set_accumulator_6: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828821u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8823u);
    goto finished;
}
call_80953b_5: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828823u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828823u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8288a0_6: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828827u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828827u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_7: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82882au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0003u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x882cu);
}
skip_ahead_9: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82882cu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
load_pressed_a_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14abu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82882fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8832u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_7: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828832u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0040u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8834u);
}
branch_equal_13: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828834u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x883eu : 0x8836u);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_7: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828836u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828836u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_8: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828839u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x883bu);
}
skip_ahead_10: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82883bu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
load_pressed_b_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82883eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8841u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_8: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828841u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0040u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8843u);
}
branch_equal_14: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828843u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x884du : 0x8845u);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_8: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828845u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828845u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_9: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828848u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0005u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x884au);
}
skip_ahead_11: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82884au, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
load_pressed_a_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14abu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82884du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8850u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_9: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828850u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0020u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8852u);
}
branch_equal_15: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828852u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8862u : 0x8854u);
    if (taken)
        goto finished;
}
set_accumulator_10: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828854u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8856u);
    goto finished;
}
call_80953b_6: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828856u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828856u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8288a0_9: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82885au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82885au, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_11: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82885du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0006u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x885fu);
}
skip_ahead_12: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82885fu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
test_accumulator_bits_10: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828862u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8864u);
}
branch_equal_16: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828864u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8874u : 0x8866u);
    if (taken)
        goto finished;
}
set_accumulator_12: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828866u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8868u);
    goto finished;
}
call_80953b_7: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828868u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828868u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8288a0_10: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82886cu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82886cu, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_13: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82886fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0007u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8871u);
}
skip_ahead_13: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828871u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
load_pressed_b_4: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828874u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8877u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_11: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828877u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8879u);
}
branch_equal_17: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828879u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8883u : 0x887bu);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_11: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82887bu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82887bu, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_14: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82887eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8880u);
}
skip_ahead_14: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828880u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
load_pressed_b_5: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14acu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828883u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8886u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_accumulator_bits_12: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828886u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0020u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8888u);
}
branch_equal_18: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828888u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x889eu : 0x888au);
    if (taken)
        goto finished;
    goto finished;
}
call_8288a0_12: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82888au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82888au, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_15: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82888du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0009u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x888fu);
}
skip_ahead_15: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82888fu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x889cu);
    goto finished;
}
narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828892u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8894u);
    goto finished;
}
call_8288a0_13: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828894u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828894u, 0x20u, 0x8288a0u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8288cb: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x828897u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x828897u, 0x20u, 0x8288cbu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator_16: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82889au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x889cu);
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
static int Lufia2DecompRunMenuValidateCursorSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82898du && pc < 0x8289a0u)
        end_pc = 0x8289a0u;
    else if (pc >= 0x8289a0u && pc < 0x8289a3u)
        end_pc = 0x8289a3u;
    else if (pc >= 0x8289a3u && pc < 0x8289a4u)
        end_pc = 0x8289a4u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82898du: byte_accumulator = true; break;
    case 0x828990u: byte_accumulator = true; break;
    case 0x828991u: byte_accumulator = true; break;
    case 0x828992u: byte_accumulator = true; break;
    case 0x828995u: byte_accumulator = true; break;
    case 0x828998u: byte_accumulator = true; break;
    case 0x82899au: byte_accumulator = true; break;
    case 0x82899du: byte_accumulator = true; break;
    case 0x8289a0u: byte_accumulator = true; break;
    case 0x8289a3u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82898du;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 23u ||
        memcmp(cart->rom + first,
               "\xbd\xeb\x14\x0a\x18\x7d\xd9\x14\xcd\x7a\x0a\x90\x09\x9e\xd9\x14\x9e\xeb\x14\x20\xcb\x88\x60",
               23u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82898du: goto load_item_row;
    case 0x828990u: goto double_accumulator;
    case 0x828991u: goto clear_carry;
    case 0x828992u: goto add_item_column;
    case 0x828995u: goto compare_party_count;
    case 0x828998u: goto branch_no_carry;
    case 0x82899au: goto clear_item_column;
    case 0x82899du: goto clear_item_row;
    case 0x8289a0u: goto call_8288cb;
    case 0x8289a3u: goto return_caller;
    default: return 0;
    }
load_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14ebu >> 8) != ((0x14ebu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82898du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8990u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828990u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8991u);
}
clear_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828991u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8992u);
}
add_item_column: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x14d9u) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x14d9u >> 8) != ((0x14d9u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828992u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8995u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_party_count: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x0a7au) + 0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828995u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8998u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_no_carry: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828998u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x89a3u : 0x899au);
    if (taken)
        goto finished;
}
clear_item_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d9u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82899au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x899du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_item_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14ebu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82899du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89a0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8288cb: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289a0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289a0u, 0x20u, 0x8288cbu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289a3u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8289a3u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuAuxiliarySpritesSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8289a4u && pc < 0x8289b0u)
        end_pc = 0x8289b0u;
    else if (pc >= 0x8289b0u && pc < 0x8289b4u)
        end_pc = 0x8289b4u;
    else if (pc >= 0x8289b4u && pc < 0x8289bbu)
        end_pc = 0x8289bbu;
    else if (pc >= 0x8289bbu && pc < 0x8289bcu)
        end_pc = 0x8289bcu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8289a4u: byte_accumulator = true; break;
    case 0x8289a6u: byte_accumulator = true; break;
    case 0x8289a8u: byte_accumulator = true; break;
    case 0x8289aau: byte_accumulator = true; break;
    case 0x8289adu: byte_accumulator = true; break;
    case 0x8289b0u: byte_accumulator = true; break;
    case 0x8289b4u: byte_accumulator = true; break;
    case 0x8289b5u: byte_accumulator = true; break;
    case 0x8289b7u: byte_accumulator = true; break;
    case 0x8289b9u: byte_accumulator = true; break;
    case 0x8289bbu: width_neutral = true; break;
    default: break;
    }
    bool byte_index = false;
    switch (pc) {
    case 0x8289a6u:
    case 0x8289a8u:
    case 0x8289aau:
    case 0x8289adu:
    case 0x8289b0u:
    case 0x8289b4u:
    case 0x8289b5u:
    case 0x8289b7u:
    case 0x8289b9u: byte_index = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e ||
        registers->xf != byte_index || (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8289a4u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 24u || memcmp(cart->rom + first,
                                              "\xe2\x10\xa0\x0a\xa2\x05\x9e\xd4\x14\x9e\xe6\x14\x22\xda\x8c"
                                              "\x86\xe8\xe0\x10\xd0\xf1\xc2\x10\x60",
                                              24u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8289a4u: goto narrow_indices;
    case 0x8289a6u: goto set_y;
    case 0x8289a8u: goto set_x;
    case 0x8289aau: goto clear_sprite_column;
    case 0x8289adu: goto clear_sprite_row;
    case 0x8289b0u: goto call_868cda;
    case 0x8289b4u: goto next_x;
    case 0x8289b5u: goto compare_x;
    case 0x8289b7u: goto branch_different;
    case 0x8289b9u: goto widen_indices;
    case 0x8289bbu: goto return_caller;
    default: return 0;
    }
narrow_indices: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289a4u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x89a6u);
}
set_y: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289a6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x000au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89a8u);
}
set_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289a8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0005u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89aau);
}
clear_sprite_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14d4u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x14d4u >> 8) != ((0x14d4u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289aau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89adu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_sprite_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14e6u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x14e6u >> 8) != ((0x14e6u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289adu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89b0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_868cda: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289b0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289b0u, 0x22u, 0x868cdau, true, true, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
next_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289b4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)((uint16_t)(registers->x + 1u)), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89b5u);
}
compare_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289b5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89b7u);
}
branch_different: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289b7u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x89aau : 0x89b9u);
    if (taken)
        goto finished;
}
widen_indices: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289b9u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x89bbu);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289bbu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8289bbu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuReplaceCursorSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8289bcu && pc < 0x8289c0u)
        end_pc = 0x8289c0u;
    else if (pc >= 0x8289c0u && pc < 0x8289c3u)
        end_pc = 0x8289c3u;
    else if (pc >= 0x8289c3u && pc < 0x8289c4u)
        end_pc = 0x8289c4u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8289bcu: byte_accumulator = true; break;
    case 0x8289bfu: byte_accumulator = true; break;
    case 0x8289c0u: byte_accumulator = true; break;
    case 0x8289c3u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8289bcu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 8u || memcmp(cart->rom + first, "\x9e\xd8\x11\xbb\x20\x5b\x89\x60", 8u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8289bcu: goto clear_sprite_active;
    case 0x8289bfu: goto copy_y_to_x;
    case 0x8289c0u: goto call_82895b;
    case 0x8289c3u: goto return_caller;
    default: return 0;
    }
clear_sprite_active: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289bcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
copy_y_to_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289bfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89c0u);
    goto finished;
}
call_82895b: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289c0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289c0u, 0x20u, 0x82895bu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289c3u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8289c3u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuCursorPairSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8289c4u && pc < 0x8289d1u)
        end_pc = 0x8289d1u;
    else if (pc >= 0x8289d1u && pc < 0x8289d4u)
        end_pc = 0x8289d4u;
    else if (pc >= 0x8289d4u && pc < 0x8289e1u)
        end_pc = 0x8289e1u;
    else if (pc >= 0x8289e1u && pc < 0x8289e4u)
        end_pc = 0x8289e4u;
    else if (pc >= 0x8289e4u && pc < 0x8289e8u)
        end_pc = 0x8289e8u;
    else if (pc >= 0x8289e8u && pc < 0x8289ebu)
        end_pc = 0x8289ebu;
    else if (pc >= 0x8289ebu && pc < 0x8289ecu)
        end_pc = 0x8289ecu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8289c4u: byte_accumulator = true; break;
    case 0x8289c6u: byte_accumulator = true; break;
    case 0x8289c8u: byte_accumulator = true; break;
    case 0x8289cau: byte_accumulator = true; break;
    case 0x8289ccu: byte_accumulator = true; break;
    case 0x8289cfu: byte_accumulator = true; break;
    case 0x8289d1u: byte_accumulator = true; break;
    case 0x8289d4u: byte_accumulator = true; break;
    case 0x8289d6u: byte_accumulator = true; break;
    case 0x8289d7u: byte_accumulator = true; break;
    case 0x8289d8u: byte_accumulator = true; break;
    case 0x8289d9u: byte_accumulator = true; break;
    case 0x8289dau: byte_accumulator = true; break;
    case 0x8289dbu: byte_accumulator = true; break;
    case 0x8289ddu: byte_accumulator = true; break;
    case 0x8289dfu: byte_accumulator = true; break;
    case 0x8289e1u: byte_accumulator = true; break;
    case 0x8289e4u: byte_accumulator = true; break;
    case 0x8289e6u: byte_accumulator = true; break;
    case 0x8289e8u: byte_accumulator = true; break;
    case 0x8289ebu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8289c4u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 40u ||
        memcmp(cart->rom + first,
               "\x86\x58\x84\x5a\xa5\x57\xd0\x05\x9e\xd8\x11\x80\x03\x20\x5b\x89\xa6\x5a\xca\xca\xca\xca\xca"
               "\x86\x63\xa5\x56\xa6\x5a\x20\x5b\x89\xa4\x54\xa6\x63\x20\x1e\x89\x60",
               40u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8289c4u: goto store_x_first_word;
    case 0x8289c6u: goto store_y_second_word;
    case 0x8289c8u: goto load_pair_second_style;
    case 0x8289cau: goto branch_different;
    case 0x8289ccu: goto clear_sprite_active;
    case 0x8289cfu: goto skip_ahead;
    case 0x8289d1u: goto call_82895b;
    case 0x8289d4u: goto load_x_second_word;
    case 0x8289d6u: goto previous_x;
    case 0x8289d7u: goto previous_x_2;
    case 0x8289d8u: goto previous_x_3;
    case 0x8289d9u: goto previous_x_4;
    case 0x8289dau: goto previous_x_5;
    case 0x8289dbu: goto store_x_pair_item;
    case 0x8289ddu: goto load_pair_style;
    case 0x8289dfu: goto load_x_second_word_2;
    case 0x8289e1u: goto call_82895b_2;
    case 0x8289e4u: goto load_y_pair_grid;
    case 0x8289e6u: goto load_x_pair_item;
    case 0x8289e8u: goto call_82891e;
    case 0x8289ebu: goto return_caller;
    default: return 0;
    }
store_x_first_word: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289c4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89c6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_y_second_word: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289c6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89c8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_pair_second_style: {
    const uint32_t address = (uint16_t)(registers->dp + 0x57u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289c8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89cau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_different: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289cau, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x89d1u : 0x89ccu);
    if (taken)
        goto finished;
}
clear_sprite_active: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11d8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289ccu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289cfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x89d4u);
    goto finished;
}
call_82895b: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289d1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289d1u, 0x20u, 0x82895bu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_x_second_word: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289d4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89d6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
previous_x: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289d6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89d7u);
}
previous_x_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289d7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89d8u);
}
previous_x_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289d8u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89d9u);
}
previous_x_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289d9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89dau);
}
previous_x_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289dau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89dbu);
}
store_x_pair_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289dbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89ddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_pair_style: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289ddu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89dfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_second_word_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289dfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89e1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_82895b_2: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289e1u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289e1u, 0x20u, 0x82895bu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_y_pair_grid: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289e4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89e6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_pair_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289e6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_82891e: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289e8u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289e8u, 0x20u, 0x82891eu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289ebu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8289ebu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuMoveCursorPairSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8289ecu && pc < 0x8289efu)
        end_pc = 0x8289efu;
    else if (pc >= 0x8289efu && pc < 0x8289f6u)
        end_pc = 0x8289f6u;
    else if (pc >= 0x8289f6u && pc < 0x8289f9u)
        end_pc = 0x8289f9u;
    else if (pc >= 0x8289f9u && pc < 0x8289fau)
        end_pc = 0x8289fau;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8289ecu: byte_accumulator = true; break;
    case 0x8289efu: byte_accumulator = true; break;
    case 0x8289f1u: byte_accumulator = true; break;
    case 0x8289f3u: byte_accumulator = true; break;
    case 0x8289f6u: byte_accumulator = true; break;
    case 0x8289f9u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8289ecu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 14u ||
        memcmp(cart->rom + first, "\x20\xc4\x89\xa9\x01\xa6\x58\xac\xa9\x14\x20\xfa\x89\x60", 14u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8289ecu: goto call_8289c4;
    case 0x8289efu: goto set_accumulator;
    case 0x8289f1u: goto load_x_first_word;
    case 0x8289f3u: goto load_y_cursor_slot;
    case 0x8289f6u: goto call_8289fa;
    case 0x8289f9u: goto return_caller;
    default: return 0;
    }
call_8289c4: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289ecu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289ecu, 0x20u, 0x8289c4u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289efu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89f1u);
}
load_x_first_word: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289f1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89f3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_y_cursor_slot: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x14a9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289f3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x89f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_8289fa: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289f6u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x8289f6u, 0x20u, 0x8289fau, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_caller: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8289f9u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8289f9u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunMenuCursorSlideStartSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x8289fau && pc < 0x828a75u)
        end_pc = 0x828a75u;
    else if (pc >= 0x828a9du && pc < 0x828aa6u)
        end_pc = 0x828aa6u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x8289fau: byte_accumulator = true; break;
    case 0x8289fcu: byte_accumulator = true; break;
    case 0x8289feu: byte_accumulator = true; break;
    case 0x828a00u: byte_accumulator = true; break;
    case 0x828a02u: byte_accumulator = true; break;
    case 0x828a05u: byte_accumulator = true; break;
    case 0x828a07u: byte_accumulator = true; break;
    case 0x828a0au: byte_accumulator = true; break;
    case 0x828a0cu: byte_accumulator = true; break;
    case 0x828a0fu: byte_accumulator = true; break;
    case 0x828a11u: byte_accumulator = true; break;
    case 0x828a14u: byte_accumulator = true; break;
    case 0x828a16u: byte_accumulator = true; break;
    case 0x828a18u: byte_accumulator = true; break;
    case 0x828a1au: byte_accumulator = true; break;
    case 0x828a1du: byte_accumulator = true; break;
    case 0x828a1fu: byte_accumulator = true; break;
    case 0x828a22u: byte_accumulator = true; break;
    case 0x828a24u: byte_accumulator = true; break;
    case 0x828a26u: byte_accumulator = true; break;
    case 0x828a28u: byte_accumulator = true; break;
    case 0x828a2au: byte_accumulator = true; break;
    case 0x828a2cu: byte_accumulator = true; break;
    case 0x828a2eu: byte_accumulator = true; break;
    case 0x828a2fu: byte_accumulator = true; break;
    case 0x828a31u: byte_accumulator = true; break;
    case 0x828a33u: byte_accumulator = true; break;
    case 0x828a35u: byte_accumulator = true; break;
    case 0x828a37u: byte_accumulator = true; break;
    case 0x828a39u: byte_accumulator = true; break;
    case 0x828a3bu: byte_accumulator = true; break;
    case 0x828a3cu: byte_accumulator = true; break;
    case 0x828a3eu: byte_accumulator = true; break;
    case 0x828a40u: byte_accumulator = true; break;
    case 0x828a42u: byte_accumulator = true; break;
    case 0x828a44u: byte_accumulator = true; break;
    case 0x828a46u: byte_accumulator = true; break;
    case 0x828a48u: byte_accumulator = true; break;
    case 0x828a4au: byte_accumulator = true; break;
    case 0x828a4cu: byte_accumulator = true; break;
    case 0x828a4du: byte_accumulator = true; break;
    case 0x828a4fu: byte_accumulator = true; break;
    case 0x828a51u: byte_accumulator = true; break;
    case 0x828a53u: byte_accumulator = true; break;
    case 0x828a55u: byte_accumulator = true; break;
    case 0x828a57u: byte_accumulator = true; break;
    case 0x828a59u: byte_accumulator = true; break;
    case 0x828a5au: byte_accumulator = true; break;
    case 0x828a5cu: byte_accumulator = true; break;
    case 0x828a5eu: byte_accumulator = true; break;
    case 0x828a60u: byte_accumulator = true; break;
    case 0x828a62u: byte_accumulator = true; break;
    case 0x828a64u: byte_accumulator = true; break;
    case 0x828a66u: byte_accumulator = true; break;
    case 0x828a68u: byte_accumulator = true; break;
    case 0x828a6au: byte_accumulator = true; break;
    case 0x828a6cu: byte_accumulator = true; break;
    case 0x828a6fu: byte_accumulator = true; break;
    case 0x828a70u: byte_accumulator = true; break;
    case 0x828a72u: byte_accumulator = true; break;
    case 0x828a9du: byte_accumulator = true; break;
    case 0x828aa0u: byte_accumulator = true; break;
    case 0x828aa1u: byte_accumulator = true; break;
    case 0x828aa3u: byte_accumulator = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x8289fau;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 172u ||
        memcmp(cart->rom + first,
               "\x86\x00\xc9\x00\xf0\x02\x84\x00\xb9\x88\x13\x85\xdf\xb9\xe8\x13\x85\xe0\xbd\x88\x13\x85\xe1"
               "\xbd\xe8\x13\x85\xe2\xa4\x00\xa5\xe1\x99\x88\x13\xa5\xe2\x99\xe8\x13\xa9\x10\x85\xed\xa5\xdf"
               "\xc5\xe1\x90\x0d\xa5\xdf\x38\xe5\xe1\x85\xe3\xa9\x01\x85\xe5\x80\x0b\xa5\xe1\x38\xe5\xdf\x85"
               "\xe3\xa9\xff\x85\xe5\xa5\xe0\xc5\xe2\x90\x0d\xa5\xe0\x38\xe5\xe2\x85\xe4\xa9\x01\x85\xe6\x80"
               "\x0b\xa5\xe2\x38\xe5\xe0\x85\xe4\xa9\xff\x85\xe6\xa6\xe3\x86\xe1\xa5\xe3\xc5\xe4\xb0\x31\xb9"
               "\xe8\x13\x38\xe5\xe6\x99\xe8\x13\xa5\xe4\x38\xe5\xe3\x85\xe4\xb0\x03\x20\xd8\x8a\xb9\xe8\x13"
               "\x18\x65\xe6\x99\xe8\x13\xc5\xe0\xd0\x0a\xb9\x88\x13\xc5\xdf\xd0\x03\x4c\x07\x8b\x20\xfa\x8a"
               "\x80\xd8\xb9\x88\x13\x38\xe5\xe5\x99\x88\x13",
               172u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x8289fau: goto store_x_slide_item;
    case 0x8289fcu: goto compare_accumulator;
    case 0x8289feu: goto branch_equal;
    case 0x828a00u: goto store_y_slide_item;
    case 0x828a02u: goto load_sprite_x;
    case 0x828a05u: goto store_start_x;
    case 0x828a07u: goto load_sprite_y;
    case 0x828a0au: goto store_start_y;
    case 0x828a0cu: goto load_sprite_x_2;
    case 0x828a0fu: goto store_target_x;
    case 0x828a11u: goto load_sprite_y_2;
    case 0x828a14u: goto store_target_y;
    case 0x828a16u: goto load_y_slide_item;
    case 0x828a18u: goto load_target_x;
    case 0x828a1au: goto store_sprite_x;
    case 0x828a1du: goto load_target_y;
    case 0x828a1fu: goto store_sprite_y;
    case 0x828a22u: goto set_accumulator;
    case 0x828a24u: goto store_slide_count;
    case 0x828a26u: goto load_start_x;
    case 0x828a28u: goto compare_target_x;
    case 0x828a2au: goto branch_no_carry;
    case 0x828a2cu: goto load_start_x_2;
    case 0x828a2eu: goto set_carry;
    case 0x828a2fu: goto subtract_target_x;
    case 0x828a31u: goto store_distance_x;
    case 0x828a33u: goto set_accumulator_2;
    case 0x828a35u: goto store_step_x;
    case 0x828a37u: goto skip_ahead;
    case 0x828a39u: goto load_target_x_2;
    case 0x828a3bu: goto set_carry_2;
    case 0x828a3cu: goto subtract_start_x;
    case 0x828a3eu: goto store_distance_x_2;
    case 0x828a40u: goto set_accumulator_3;
    case 0x828a42u: goto store_step_x_2;
    case 0x828a44u: goto load_start_y;
    case 0x828a46u: goto compare_target_y;
    case 0x828a48u: goto branch_no_carry_2;
    case 0x828a4au: goto load_start_y_2;
    case 0x828a4cu: goto set_carry_3;
    case 0x828a4du: goto subtract_target_y;
    case 0x828a4fu: goto store_distance_y;
    case 0x828a51u: goto set_accumulator_4;
    case 0x828a53u: goto store_step_y;
    case 0x828a55u: goto skip_ahead_2;
    case 0x828a57u: goto load_target_y_2;
    case 0x828a59u: goto set_carry_4;
    case 0x828a5au: goto subtract_start_y;
    case 0x828a5cu: goto store_distance_y_2;
    case 0x828a5eu: goto set_accumulator_5;
    case 0x828a60u: goto store_step_y_2;
    case 0x828a62u: goto load_x_distance_x;
    case 0x828a64u: goto store_x_target_x;
    case 0x828a66u: goto load_distance_x;
    case 0x828a68u: goto compare_distance_y;
    case 0x828a6au: goto branch_carry;
    case 0x828a6cu: goto load_sprite_y_3;
    case 0x828a6fu: goto set_carry_5;
    case 0x828a70u: goto subtract_step_y;
    case 0x828a72u: goto store_sprite_y_2;
    case 0x828a9du: goto load_sprite_x_3;
    case 0x828aa0u: goto set_carry_6;
    case 0x828aa1u: goto subtract_step_x;
    case 0x828aa3u: goto store_sprite_x_2;
    default: return 0;
    }
store_x_slide_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289fau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x89fcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289fcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x89feu);
}
branch_equal: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8289feu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a02u : 0x8a00u);
    if (taken)
        goto load_sprite_x;
}
store_y_slide_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a00u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a02u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_sprite_x: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1388u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x1388u >> 8) != ((0x1388u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a02u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a05u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_start_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xdfu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a05u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a07u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_sprite_y: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e8u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x13e8u >> 8) != ((0x13e8u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a07u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a0au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_start_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe0u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a0au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a0cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_sprite_x_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1388u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x1388u >> 8) != ((0x1388u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a0cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a0fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_target_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a0fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a11u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_sprite_y_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e8u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x13e8u >> 8) != ((0x13e8u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a11u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a14u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_target_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe2u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a14u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a16u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_y_slide_item: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a16u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a18u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_target_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a18u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a1au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_sprite_x: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1388u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a1au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a1du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_target_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe2u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a1du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a1fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_sprite_y: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e8u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a1fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a22u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a22u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a24u);
}
store_slide_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0xedu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a24u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a26u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_start_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xdfu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a26u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a28u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_target_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a28u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a2au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_no_carry: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a2au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a39u : 0x8a2cu);
    if (taken)
        goto load_target_x_2;
}
load_start_x_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xdfu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a2cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a2eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a2eu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a2fu);
}
subtract_target_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a2fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a31u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_distance_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a31u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a33u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a33u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a35u);
}
store_step_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe5u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a35u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a37u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a37u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8a44u);
    goto finished;
}
load_target_x_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a39u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a3bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry_2: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a3bu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a3cu);
}
subtract_start_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xdfu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a3cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a3eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_distance_x_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a3eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a40u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a40u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a42u);
}
store_step_x_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe5u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a42u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_start_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe0u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a44u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a46u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_target_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe2u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a46u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a48u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_no_carry_2: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a48u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a57u : 0x8a4au);
    if (taken)
        goto load_target_y_2;
}
load_start_y_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe0u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a4au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a4cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry_3: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a4cu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a4du);
}
subtract_target_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe2u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a4du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a4fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_distance_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe4u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a4fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a51u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a51u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a53u);
}
store_step_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe6u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a53u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a55u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_ahead_2: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a55u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x8a62u);
    goto finished;
}
load_target_y_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe2u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a57u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a59u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry_4: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a59u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a5au);
}
subtract_start_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe0u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a5au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a5cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_distance_y_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe4u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a5cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_accumulator_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a5eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a60u);
}
store_step_y_2: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe6u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a60u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a62u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_x_distance_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a62u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a64u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_x_target_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a64u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a66u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_distance_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a66u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a68u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_distance_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe4u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a68u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a6au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
branch_carry: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a6au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x8a9du : 0x8a6cu);
    if (taken)
        goto finished;
}
load_sprite_y_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e8u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x13e8u >> 8) != ((0x13e8u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a6cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a6fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry_5: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a6fu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8a70u);
}
subtract_step_y: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe6u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a70u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a72u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_sprite_y_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x13e8u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a72u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8a75u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
load_sprite_x_3: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1388u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x1388u >> 8) != ((0x1388u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828a9du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8aa0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_carry_6: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x828aa0u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x8aa1u);
}
subtract_step_x: {
    const uint32_t address = (uint16_t)(registers->dp + 0xe5u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828aa1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8aa3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_sprite_x_2: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1388u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x828aa3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x8aa6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}

static int Lufia2DecompRunMenuCursorParentSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    if (pc >= 0x828704u && pc <= 0x82871fu)
        return Lufia2DecompRunMenuPresentDisplaySpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x828720u && pc <= 0x82889bu)
        return Lufia2DecompRunMenuCursorInputSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                  step_budget, step_count);
    if (pc >= 0x82898du && pc <= 0x8289a3u)
        return Lufia2DecompRunMenuValidateCursorSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x8289a4u && pc <= 0x8289bbu)
        return Lufia2DecompRunMenuAuxiliarySpritesSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                       step_budget, step_count);
    if (pc >= 0x8289bcu && pc <= 0x8289c3u)
        return Lufia2DecompRunMenuReplaceCursorSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
    if (pc >= 0x8289c4u && pc <= 0x8289ebu)
        return Lufia2DecompRunMenuCursorPairSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                 step_budget, step_count);
    if (pc >= 0x8289ecu && pc <= 0x8289f9u)
        return Lufia2DecompRunMenuMoveCursorPairSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                     step_budget, step_count);
    if (pc >= 0x8289fau && pc <= 0x828aa5u)
        return Lufia2DecompRunMenuCursorSlideStartSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                       step_budget, step_count);
    return 0;
}
