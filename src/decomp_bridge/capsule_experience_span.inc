#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunCapsuleBuildExperienceSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                     uint8_t opcode, uint64_t dynamic_epoch,
                                                     uint64_t write_epoch, unsigned step_budget,
                                                     long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82ce52u && pc < 0x82ce68u)
        end_pc = 0x82ce68u;
    else if (pc >= 0x82ce68u && pc < 0x82ce69u)
        end_pc = 0x82ce69u;
    else if (pc >= 0x82ce69u && pc < 0x82ce8au)
        end_pc = 0x82ce8au;
    else if (pc >= 0x82ce8au && pc < 0x82ce8du)
        end_pc = 0x82ce8du;
    else if (pc >= 0x82ce8du && pc < 0x82ceaau)
        end_pc = 0x82ceaau;
    else if (pc >= 0x82ceaau && pc < 0x82ceabu)
        end_pc = 0x82ceabu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82ce52u: byte_accumulator = true; break;
    case 0x82ce55u: byte_accumulator = true; break;
    case 0x82ce57u: byte_accumulator = true; break;
    case 0x82ce59u: byte_accumulator = true; break;
    case 0x82ce5bu: byte_accumulator = true; break;
    case 0x82ce5eu: byte_accumulator = true; break;
    case 0x82ce60u: byte_accumulator = true; break;
    case 0x82ce63u: byte_accumulator = true; break;
    case 0x82ce65u: byte_accumulator = true; break;
    case 0x82ce68u: width_neutral = true; break;
    case 0x82ce69u: byte_accumulator = true; break;
    case 0x82ce6bu: byte_accumulator = true; break;
    case 0x82ce6eu: byte_accumulator = true; break;
    case 0x82ce71u: byte_accumulator = true; break;
    case 0x82ce74u: byte_accumulator = true; break;
    case 0x82ce77u: byte_accumulator = true; break;
    case 0x82ce7au: byte_accumulator = true; break;
    case 0x82ce7du: byte_accumulator = true; break;
    case 0x82ce80u: byte_accumulator = true; break;
    case 0x82ce83u: byte_accumulator = true; break;
    case 0x82ce86u: byte_accumulator = true; break;
    case 0x82ce87u: byte_accumulator = true; break;
    case 0x82ce8au: byte_accumulator = true; break;
    case 0x82ce8du: byte_accumulator = true; break;
    case 0x82ce8eu: byte_accumulator = true; break;
    case 0x82ce8fu: byte_accumulator = true; break;
    case 0x82ce91u: byte_accumulator = true; break;
    case 0x82ce94u: byte_accumulator = true; break;
    case 0x82ce95u: byte_accumulator = true; break;
    case 0x82ce97u: byte_accumulator = true; break;
    case 0x82ce9au: byte_accumulator = true; break;
    case 0x82ce9du: byte_accumulator = true; break;
    case 0x82ce9fu: byte_accumulator = true; break;
    case 0x82cea2u: byte_accumulator = true; break;
    case 0x82cea5u: byte_accumulator = true; break;
    case 0x82cea7u: byte_accumulator = true; break;
    case 0x82ceaau: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82ce52u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 89u ||
        memcmp(cart->rom + first,
               "\xad\xed\x10\xc9\x62\x90\x10\xa9\x7f\x8d\x41\x11\xa9\x96\x8d\x42\x11\xa9\x98\x8d\x43\x11\x60"
               "\xa9\x14\x9c\xd5\x09\x8d\xd6\x09\x9c\xd7\x09\x9c\xd8\x09\x9c\x41\x11\x9c\x42\x11\x9c\x43\x11"
               "\xad\xed\x10\x9c\xed\x10\x48\xee\xed\x10\x20\xab\xce\x68\x3a\xd0\xf5\xad\x41\x11\x38\xe9\x0a"
               "\x8d\x41\x11\xad\x42\x11\xe9\x00\x8d\x42\x11\xad\x43\x11\xe9\x00\x8d\x43\x11\x60",
               89u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82ce52u: goto read_requested_level;
    case 0x82ce55u: goto test_capped_level;
    case 0x82ce57u: goto use_uncapped_level;
    case 0x82ce59u: goto capped_low;
    case 0x82ce5bu: goto store_capped_low;
    case 0x82ce5eu: goto capped_middle;
    case 0x82ce60u: goto store_capped_middle;
    case 0x82ce63u: goto capped_high;
    case 0x82ce65u: goto store_capped_high;
    case 0x82ce68u: goto return_capped_level;
    case 0x82ce69u: goto initial_step;
    case 0x82ce6bu: goto clear_step_fraction;
    case 0x82ce6eu: goto store_initial_step;
    case 0x82ce71u: goto clear_step_middle;
    case 0x82ce74u: goto clear_step_high;
    case 0x82ce77u: goto clear_experience_low;
    case 0x82ce7au: goto clear_experience_middle;
    case 0x82ce7du: goto clear_experience_high;
    case 0x82ce80u: goto read_level_count;
    case 0x82ce83u: goto clear_work_level;
    case 0x82ce86u: goto save_level_count;
    case 0x82ce87u: goto advance_work_level;
    case 0x82ce8au: goto advance_experience;
    case 0x82ce8du: goto restore_level_count;
    case 0x82ce8eu: goto count_remaining_level;
    case 0x82ce8fu: goto repeat_levels;
    case 0x82ce91u: goto read_total_low;
    case 0x82ce94u: goto set_offset_borrow;
    case 0x82ce95u: goto subtract_level_offset;
    case 0x82ce97u: goto store_total_low;
    case 0x82ce9au: goto read_total_middle;
    case 0x82ce9du: goto subtract_offset_middle;
    case 0x82ce9fu: goto store_total_middle;
    case 0x82cea2u: goto read_total_high;
    case 0x82cea5u: goto subtract_offset_high;
    case 0x82cea7u: goto store_total_high;
    case 0x82ceaau: goto return_total_experience;
    default: return 0;
    }
read_requested_level: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x10edu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce52u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce55u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_capped_level: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce55u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0062u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce57u);
}
use_uncapped_level: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce57u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xce69u : 0xce59u);
    if (taken)
        goto finished;
}
capped_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce59u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce5bu);
}
store_capped_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce5bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
capped_middle: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce5eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0096u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce60u);
}
store_capped_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce60u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce63u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
capped_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce63u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0098u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce65u);
}
store_capped_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce65u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce68u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_capped_level: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82ce68u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82ce68u, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
initial_step: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce69u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0014u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce6bu);
}
clear_step_fraction: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d5u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce6bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce6eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_initial_step: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce6eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce71u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_step_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d7u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce71u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce74u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_step_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce74u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce77u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_experience_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce77u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce7au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_experience_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce7au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce7du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_experience_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce7du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce80u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_level_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x10edu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce80u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce83u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_work_level: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x10edu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce83u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_level_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce86u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xce87u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_work_level: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x10edu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce87u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xeeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce8au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
advance_experience: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82ce8au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x82ce8au, 0x20u, 0x82ceabu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_level_count: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce8du, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    Lufia2DecompApplyOperand(registers, 0xa9u, value, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xce8eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_remaining_level: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce8eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce8fu);
}
repeat_levels: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce8fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xce86u : 0xce91u);
    if (taken)
        goto finished;
}
read_total_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce91u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce94u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_offset_borrow: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce94u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xce95u);
}
subtract_level_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce95u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x000au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce97u);
}
store_total_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce97u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce9au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_total_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce9au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xce9du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
subtract_offset_middle: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce9du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xce9fu);
}
store_total_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ce9fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcea2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_total_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cea2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcea5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
subtract_offset_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cea5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcea7u);
}
store_total_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cea7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceaau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_total_experience: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82ceaau);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82ceaau, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunCapsuleAdvanceExperienceSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                       uint8_t opcode, uint64_t dynamic_epoch,
                                                       uint64_t write_epoch, unsigned step_budget,
                                                       long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x82ceabu && pc < 0x82cf71u)
        end_pc = 0x82cf71u;
    else if (pc >= 0x82cf71u && pc < 0x82cf72u)
        end_pc = 0x82cf72u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x82ceabu: byte_accumulator = true; break;
    case 0x82ceaeu: byte_accumulator = true; break;
    case 0x82ceafu: byte_accumulator = true; break;
    case 0x82ceb2u: byte_accumulator = true; break;
    case 0x82ceb5u: byte_accumulator = true; break;
    case 0x82ceb8u: byte_accumulator = true; break;
    case 0x82cebbu: byte_accumulator = true; break;
    case 0x82cebeu: byte_accumulator = true; break;
    case 0x82cec1u: byte_accumulator = true; break;
    case 0x82cec4u: byte_accumulator = true; break;
    case 0x82cec7u: width_neutral = true; break;
    case 0x82ced0u: width_neutral = true; break;
    case 0x82ced2u: byte_accumulator = true; break;
    case 0x82ced5u: byte_accumulator = true; break;
    case 0x82ced7u: byte_accumulator = true; break;
    case 0x82ced9u: byte_accumulator = true; break;
    case 0x82cedbu: byte_accumulator = true; break;
    case 0x82cedeu: byte_accumulator = true; break;
    case 0x82cee0u: byte_accumulator = true; break;
    case 0x82cee3u: byte_accumulator = true; break;
    case 0x82cee5u: byte_accumulator = true; break;
    case 0x82cee8u: width_neutral = true; break;
    case 0x82cef5u: width_neutral = true; break;
    case 0x82cef7u: byte_accumulator = true; break;
    case 0x82cefbu: byte_accumulator = true; break;
    case 0x82cefcu: byte_accumulator = true; break;
    case 0x82ceffu: byte_accumulator = true; break;
    case 0x82cf02u: byte_accumulator = true; break;
    case 0x82cf05u: byte_accumulator = true; break;
    case 0x82cf06u: byte_accumulator = true; break;
    case 0x82cf07u: byte_accumulator = true; break;
    case 0x82cf08u: byte_accumulator = true; break;
    case 0x82cf0bu: byte_accumulator = true; break;
    case 0x82cf0du: byte_accumulator = true; break;
    case 0x82cf10u: byte_accumulator = true; break;
    case 0x82cf13u: byte_accumulator = true; break;
    case 0x82cf14u: byte_accumulator = true; break;
    case 0x82cf15u: byte_accumulator = true; break;
    case 0x82cf16u: byte_accumulator = true; break;
    case 0x82cf19u: byte_accumulator = true; break;
    case 0x82cf1bu: byte_accumulator = true; break;
    case 0x82cf1eu: byte_accumulator = true; break;
    case 0x82cf21u: byte_accumulator = true; break;
    case 0x82cf22u: byte_accumulator = true; break;
    case 0x82cf23u: byte_accumulator = true; break;
    case 0x82cf24u: byte_accumulator = true; break;
    case 0x82cf27u: byte_accumulator = true; break;
    case 0x82cf29u: byte_accumulator = true; break;
    case 0x82cf2cu: byte_accumulator = true; break;
    case 0x82cf2fu: byte_accumulator = true; break;
    case 0x82cf30u: byte_accumulator = true; break;
    case 0x82cf31u: byte_accumulator = true; break;
    case 0x82cf32u: byte_accumulator = true; break;
    case 0x82cf35u: byte_accumulator = true; break;
    case 0x82cf37u: byte_accumulator = true; break;
    case 0x82cf39u: byte_accumulator = true; break;
    case 0x82cf3au: byte_accumulator = true; break;
    case 0x82cf3cu: byte_accumulator = true; break;
    case 0x82cf3eu: byte_accumulator = true; break;
    case 0x82cf40u: byte_accumulator = true; break;
    case 0x82cf42u: byte_accumulator = true; break;
    case 0x82cf44u: byte_accumulator = true; break;
    case 0x82cf46u: byte_accumulator = true; break;
    case 0x82cf48u: byte_accumulator = true; break;
    case 0x82cf4au: byte_accumulator = true; break;
    case 0x82cf4cu: byte_accumulator = true; break;
    case 0x82cf4eu: byte_accumulator = true; break;
    case 0x82cf50u: byte_accumulator = true; break;
    case 0x82cf53u: byte_accumulator = true; break;
    case 0x82cf54u: byte_accumulator = true; break;
    case 0x82cf56u: byte_accumulator = true; break;
    case 0x82cf59u: byte_accumulator = true; break;
    case 0x82cf5cu: byte_accumulator = true; break;
    case 0x82cf5eu: byte_accumulator = true; break;
    case 0x82cf61u: byte_accumulator = true; break;
    case 0x82cf64u: byte_accumulator = true; break;
    case 0x82cf66u: byte_accumulator = true; break;
    case 0x82cf69u: byte_accumulator = true; break;
    case 0x82cf6cu: byte_accumulator = true; break;
    case 0x82cf6eu: byte_accumulator = true; break;
    case 0x82cf71u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82ceabu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 199u ||
        memcmp(cart->rom + first,
               "\xad\x41\x11\x18\x6d\xd6\x09\x8d\x41\x11\xad\x42\x11\x6d\xd7\x09\x8d\x42\x11\xad\x43\x11\x6d"
               "\xd8\x09\x8d\x43\x11\xc2\x20\xad\x41\x11\x38\xe9\x7f\x96\xe2\x20\xad\x43\x11\xe9\x98\x90\x0f"
               "\xa9\x7f\x8d\x41\x11\xa9\x96\x8d\x42\x11\xa9\x98\x8d\x43\x11\xc2\x20\xad\xed\x10\x29\xff\x00"
               "\x1a\x4a\x4a\x4a\xaa\xe2\x20\xbf\xcb\xe4\x8e\x1a\x8d\x02\x42\xad\xd5\x09\x8d\x03\x42\xea\xea"
               "\xea\xae\x16\x42\x86\x11\xad\xd6\x09\x8d\x03\x42\xea\xea\xea\xae\x16\x42\x86\x13\xad\xd7\x09"
               "\x8d\x03\x42\xea\xea\xea\xae\x16\x42\x86\x15\xad\xd8\x09\x8d\x03\x42\xea\xea\xea\xae\x16\x42"
               "\x86\x17\xa5\x12\x18\x65\x13\x85\x12\xa5\x14\x65\x15\x85\x13\xa5\x17\x65\x16\x85\x14\xa9\x00"
               "\x65\x18\x85\x15\xad\xd5\x09\x18\x65\x12\x8d\xd5\x09\xad\xd6\x09\x65\x13\x8d\xd6\x09\xad\xd7"
               "\x09\x65\x14\x8d\xd7\x09\xad\xd8\x09\x65\x15\x8d\xd8\x09\x60",
               199u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82ceabu: goto read_total_low;
    case 0x82ceaeu: goto clear_step_carry;
    case 0x82ceafu: goto add_step_low;
    case 0x82ceb2u: goto store_total_low;
    case 0x82ceb5u: goto read_total_middle;
    case 0x82ceb8u: goto add_step_middle;
    case 0x82cebbu: goto store_total_middle;
    case 0x82cebeu: goto read_total_high;
    case 0x82cec1u: goto add_step_high;
    case 0x82cec4u: goto store_total_high;
    case 0x82cec7u: goto widen_cap_comparison;
    case 0x82cec9u: goto read_total_word;
    case 0x82ceccu: goto set_cap_borrow;
    case 0x82cecdu: goto subtract_maximum_word;
    case 0x82ced0u: goto narrow_cap_comparison;
    case 0x82ced2u: goto read_total_upper;
    case 0x82ced5u: goto subtract_maximum_upper;
    case 0x82ced7u: goto keep_uncapped_total;
    case 0x82ced9u: goto capped_low;
    case 0x82cedbu: goto store_capped_low;
    case 0x82cedeu: goto capped_middle;
    case 0x82cee0u: goto store_capped_middle;
    case 0x82cee3u: goto capped_high;
    case 0x82cee5u: goto store_capped_high;
    case 0x82cee8u: goto widen_growth_index;
    case 0x82ceeau: goto read_work_level;
    case 0x82ceedu: goto isolate_work_level;
    case 0x82cef0u: goto next_growth_level;
    case 0x82cef1u: goto scale_growth_index_first;
    case 0x82cef2u: goto scale_growth_index_second;
    case 0x82cef3u: goto scale_growth_index_third;
    case 0x82cef4u: goto select_growth_factor;
    case 0x82cef5u: goto narrow_growth_factor;
    case 0x82cef7u: goto read_growth_factor;
    case 0x82cefbu: goto increment_growth_factor;
    case 0x82cefcu: goto write_growth_multiplier;
    case 0x82ceffu: goto read_step_fraction;
    case 0x82cf02u: goto multiply_fraction;
    case 0x82cf05u: goto wait_fraction_first;
    case 0x82cf06u: goto wait_fraction_second;
    case 0x82cf07u: goto wait_fraction_third;
    case 0x82cf08u: goto read_fraction_product;
    case 0x82cf0bu: goto store_fraction_product;
    case 0x82cf0du: goto read_step_low;
    case 0x82cf10u: goto multiply_low;
    case 0x82cf13u: goto wait_low_first;
    case 0x82cf14u: goto wait_low_second;
    case 0x82cf15u: goto wait_low_third;
    case 0x82cf16u: goto read_low_product;
    case 0x82cf19u: goto store_low_product;
    case 0x82cf1bu: goto read_step_middle;
    case 0x82cf1eu: goto multiply_middle;
    case 0x82cf21u: goto wait_middle_first;
    case 0x82cf22u: goto wait_middle_second;
    case 0x82cf23u: goto wait_middle_third;
    case 0x82cf24u: goto read_middle_product;
    case 0x82cf27u: goto store_middle_product;
    case 0x82cf29u: goto read_step_high;
    case 0x82cf2cu: goto multiply_high;
    case 0x82cf2fu: goto wait_high_first;
    case 0x82cf30u: goto wait_high_second;
    case 0x82cf31u: goto wait_high_third;
    case 0x82cf32u: goto read_high_product;
    case 0x82cf35u: goto store_high_product;
    case 0x82cf37u: goto read_fraction_product_high;
    case 0x82cf39u: goto clear_growth_carry;
    case 0x82cf3au: goto combine_low_product;
    case 0x82cf3cu: goto store_growth_low;
    case 0x82cf3eu: goto read_low_product_high;
    case 0x82cf40u: goto combine_middle_product;
    case 0x82cf42u: goto store_growth_middle;
    case 0x82cf44u: goto read_high_product_low;
    case 0x82cf46u: goto combine_middle_product_high;
    case 0x82cf48u: goto store_growth_high;
    case 0x82cf4au: goto clear_growth_upper;
    case 0x82cf4cu: goto combine_high_product_high;
    case 0x82cf4eu: goto store_growth_upper;
    case 0x82cf50u: goto read_step_fraction_again;
    case 0x82cf53u: goto clear_step_growth_carry;
    case 0x82cf54u: goto grow_fraction;
    case 0x82cf56u: goto store_fraction;
    case 0x82cf59u: goto read_step_low_again;
    case 0x82cf5cu: goto grow_low;
    case 0x82cf5eu: goto store_step_low;
    case 0x82cf61u: goto read_step_middle_again;
    case 0x82cf64u: goto grow_middle;
    case 0x82cf66u: goto store_step_middle;
    case 0x82cf69u: goto read_step_high_again;
    case 0x82cf6cu: goto grow_high;
    case 0x82cf6eu: goto store_step_high;
    case 0x82cf71u: goto return_experience_step;
    default: return 0;
    }
read_total_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceabu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceaeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_step_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceaeu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xceafu);
}
add_step_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceafu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceb2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_total_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceb2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceb5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_total_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceb5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceb8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
add_step_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d7u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceb8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcebbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_total_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cebbu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcebeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_total_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cebeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcec1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
add_step_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cec1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcec4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_total_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cec4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcec7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_cap_comparison: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cec7u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xcec9u);
}
read_total_word: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cec9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceccu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_cap_borrow: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceccu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xcecdu);
}
subtract_maximum_word: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cecdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x967fu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xced0u);
}
narrow_cap_comparison: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ced0u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xced2u);
}
read_total_upper: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ced2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xced5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
subtract_maximum_upper: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ced5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0098u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xced7u);
}
keep_uncapped_total: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ced7u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xcee8u : 0xced9u);
    if (taken)
        goto widen_growth_index;
}
capped_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ced9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcedbu);
}
store_capped_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1141u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cedbu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcedeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
capped_middle: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cedeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0096u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcee0u);
}
store_capped_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1142u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cee0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcee3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
capped_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cee3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0098u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcee5u);
}
store_capped_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1143u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cee5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcee8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_growth_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cee8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xceeau);
}
read_work_level: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x10edu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceeau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xceedu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
isolate_work_level: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceedu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xcef0u);
}
next_growth_level: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcef1u);
}
scale_growth_index_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef1u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcef2u);
}
scale_growth_index_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef2u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcef3u);
}
scale_growth_index_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef3u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a & 0xffffu) >> 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcef4u);
}
select_growth_factor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcef5u);
}
narrow_growth_factor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef5u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xcef7u);
}
read_growth_factor: {
    const uint32_t address = (0x8ee4cbu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cef7u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcefbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
increment_growth_factor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cefbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcefcu);
}
write_growth_multiplier: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4202u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cefcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xceffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_fraction: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d5u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82ceffu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf02u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_fraction: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4203u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf02u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xcf05u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
wait_fraction_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf05u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf06u);
}
wait_fraction_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf06u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf07u);
}
wait_fraction_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf07u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf08u);
}
read_fraction_product: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4216u;
    const uint32_t high = address + 1u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        (!mapped_math && (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf08u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xcf0bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_fraction_product: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf0bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf0du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf0du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf10u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_low: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4203u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf10u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xcf13u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
wait_low_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf13u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf14u);
}
wait_low_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf14u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf15u);
}
wait_low_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf15u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf16u);
}
read_low_product: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4216u;
    const uint32_t high = address + 1u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        (!mapped_math && (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf16u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xcf19u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_low_product: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf19u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf1bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d7u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf1bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf1eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_middle: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4203u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf1eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xcf21u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
wait_middle_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf21u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf22u);
}
wait_middle_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf22u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf23u);
}
wait_middle_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf23u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf24u);
}
read_middle_product: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4216u;
    const uint32_t high = address + 1u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        (!mapped_math && (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf24u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xcf27u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_middle_product: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf27u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf29u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf29u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf2cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_high: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4203u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf2cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xcf2fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
wait_high_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf2fu, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf30u);
}
wait_high_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf30u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf31u);
}
wait_high_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf31u, 1u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf32u);
}
read_high_product: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4216u;
    const uint32_t high = address + 1u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 5u) ||
        (!mapped_math && (!Lufia2DecompIsWramByte(address) || !Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf32u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xcf35u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_high_product: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf35u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf37u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_fraction_product_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf37u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf39u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_growth_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf39u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf3au);
}
combine_low_product: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf3au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf3cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_growth_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf3cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf3eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_low_product_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x14u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf3eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf40u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
combine_middle_product: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf40u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf42u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_growth_middle: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf42u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_high_product_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf44u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf46u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
combine_middle_product_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x16u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf46u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf48u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_growth_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x14u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf48u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf4au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_growth_upper: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf4au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf4cu);
}
combine_high_product_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x18u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf4cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf4eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_growth_upper: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf4eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf50u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_fraction_again: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d5u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf50u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf53u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_step_growth_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf53u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xcf54u);
}
grow_fraction: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf54u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf56u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_fraction: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d5u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf56u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf59u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_low_again: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf59u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf5cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
grow_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf5cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_step_low: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf5eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf61u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_middle_again: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d7u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf61u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf64u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
grow_middle: {
    const uint32_t address = (uint16_t)(registers->dp + 0x14u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf64u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf66u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_step_middle: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d7u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf66u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf69u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_step_high_again: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf69u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf6cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
grow_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf6cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf6eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_step_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09d8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x82cf6eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xcf71u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_experience_step: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x82cf71u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x82cf71u, 0x60u, false, registers->mf, true);
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

static int Lufia2DecompRunCapsuleExperienceSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    if (pc < 0x82ceabu)
        return Lufia2DecompRunCapsuleBuildExperienceSpan(registers, cpu, pc, opcode, dynamic_epoch,
                                                         write_epoch, step_budget, step_count);
    return Lufia2DecompRunCapsuleAdvanceExperienceSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                       step_budget, step_count);
}
