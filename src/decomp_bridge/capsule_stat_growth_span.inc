static void CapsuleGrowthWriteWord(Interp816 *registers, uint16_t address, uint16_t value) {
    if (registers->write_word &&
        registers->write_word(registers->mem, address, address + 1u, value, true))
        return;
    registers->write(registers->mem, address + 1u, (uint8_t)(value >> 8));
    registers->write(registers->mem, address, (uint8_t)value);
}

static void CapsuleGrowthShiftWord(Interp816 *registers, uint16_t address) {
    const uint16_t value = Lufia2DecompReadData(registers, address, address + 1u, true);
    const uint16_t shifted = value >> 1;
    registers->c = (value & 1u) != 0u;
    CapsuleGrowthWriteWord(registers, address, shifted);
    registers->n = false;
    registers->z = shifted == 0u;
}

static int Lufia2DecompRunCapsuleStatGrowth(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc < 0x82d31cu || pc > 0x82d377u || registers->e || registers->mf || registers->xf ||
        registers->d || registers->dp || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        !Lufia2DecompFrameRangeQuiet(0x82d31cu, 0x82d378u))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x82d31cu;
    if (!cart || offset < delta) return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 92u ||
        memcmp(cart->rom + first,
            "\x29\xff\x00\x0a\x0a\x0a\x85\x17\x64\x11\xad\xed\x10\x29\xff\x00\xc9\x01\x00\xf0\x3e\xad\xa4\x11"
            "\x85\x13\x64\x14\xa9\x01\x00\x85\x15\xa5\x15\x4a\x4a\x4a\x4a\x48\x18\x65\x17\xaa\xbf\x20\xf4\xa6"
            "\x29\xff\x00\x85\x1c\x68\xc5\x13\x90\x06\xf0\x02\x46\x1c\x46\x1c\xa5\x1c\x18\x65\x11\x85\x11\xad"
            "\xed\x10\x29\xff\x00\xe6\x15\xc5\x15\xd0\xce\x46\x11\x46\x11\x46\x11\x46\x11\x60", 92u))
        return 0;
    const uint32_t bank = (uint32_t)registers->db << 16;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x82d31cu: goto mask_stat;
    case 0x82d31fu: goto scale_row;
    case 0x82d320u: goto scale_row;
    case 0x82d321u: goto scale_row;
    case 0x82d322u: goto store_row;
    case 0x82d324u: goto clear_sum;
    case 0x82d326u: goto load_level;
    case 0x82d329u: goto mask_level;
    case 0x82d32cu: goto test_first_level;
    case 0x82d32fu: goto branch_first_level;
    case 0x82d331u: goto load_form;
    case 0x82d334u: goto store_form;
    case 0x82d336u: goto clear_form_high;
    case 0x82d338u: goto start_counter;
    case 0x82d33bu: goto store_counter;
    case 0x82d33du: goto load_counter;
    case 0x82d33fu: goto select_growth_band;
    case 0x82d340u: goto select_growth_band;
    case 0x82d341u: goto select_growth_band;
    case 0x82d342u: goto select_growth_band;
    case 0x82d343u: goto save_band;
    case 0x82d344u: goto clear_row_carry;
    case 0x82d345u: goto add_row;
    case 0x82d347u: goto select_row;
    case 0x82d348u: goto load_rate;
    case 0x82d34cu: goto mask_rate;
    case 0x82d34fu: goto store_rate;
    case 0x82d351u: goto restore_band;
    case 0x82d352u: goto compare_form;
    case 0x82d354u: goto branch_lower_band;
    case 0x82d356u: goto branch_equal_band;
    case 0x82d358u: goto halve_rate;
    case 0x82d35au: goto halve_rate;
    case 0x82d35cu: goto load_scaled_rate;
    case 0x82d35eu: goto clear_sum_carry;
    case 0x82d35fu: goto add_sum;
    case 0x82d361u: goto store_sum;
    case 0x82d363u: goto reload_level;
    case 0x82d366u: goto mask_loop_level;
    case 0x82d369u: goto advance_counter;
    case 0x82d36bu: goto compare_level;
    case 0x82d36du: goto repeat_growth;
    case 0x82d36fu: goto normalize_sum;
    case 0x82d371u: goto normalize_sum;
    case 0x82d373u: goto normalize_sum;
    case 0x82d375u: goto normalize_sum;
    case 0x82d377u: goto return_short;
    default: return 0;
    }
mask_stat:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d31cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd31fu);
scale_row:
    while (registers->pc <= 0xd321u) {
        if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
        Lufia2SpanFetch(registers, 0x820000u | registers->pc, 1u, span.instructions != 0u);
        const uint16_t value = registers->a;
        registers->c = (value & 0x8000u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(value << 1), false);
        NativeSpanStep(&span, registers, cpu, 2u, registers->pc + 1u);
    }
store_row:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d322u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x17u, 0x18u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd324u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_sum:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d324u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x11u, 0x12u, 0u, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd326u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_level:
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(bank + 0x10edu) || !Lufia2DecompIsWramByte(bank + 0x10eeu)) goto finished;
    Lufia2SpanFetch(registers, 0x82d326u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, bank + 0x10edu, bank + 0x10eeu, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xd329u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
mask_level:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d329u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd32cu);
test_first_level:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d32cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 1u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd32fu);
branch_first_level:
    {
        const bool taken = registers->z;
        const unsigned cycles = taken ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x82d32fu, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xd36fu : 0xd331u);
        if (taken) goto normalize_sum;
    }
load_form:
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(bank + 0x11a4u) || !Lufia2DecompIsWramByte(bank + 0x11a5u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d331u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, bank + 0x11a4u, bank + 0x11a5u, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xd334u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_form:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d334u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x13u, 0x14u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd336u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_form_high:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d336u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x14u, 0x15u, 0u, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd338u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
start_counter:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d338u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 1u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd33bu);
store_counter:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d33bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x15u, 0x16u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd33du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
load_counter:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d33du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x15u, 0x16u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd33fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_growth_band:
    while (registers->pc <= 0xd342u) {
        if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
        Lufia2SpanFetch(registers, 0x820000u | registers->pc, 1u, span.instructions != 0u);
        registers->c = (registers->a & 1u) != 0u;
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->a >> 1, false);
        NativeSpanStep(&span, registers, cpu, 2u, registers->pc + 1u);
    }
save_band:
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu) goto finished;
    Lufia2SpanFetch(registers, 0x82d343u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp--, (uint8_t)(registers->a >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd344u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_row_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d344u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xd345u);
add_row:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d345u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, 0x17u, 0x18u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd347u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
select_row:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d347u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xaau);
    NativeSpanStep(&span, registers, cpu, 2u, 0xd348u);
load_rate:
    {
        const uint32_t address = 0xa6f420u + registers->x;
        if (!NativeSpanCanStep(&span, cpu, 6u) ||
            (!Lufia2DecompIsWramByte(address) && !Lufia2DecompIsLoRomDataByte(cart, address)) ||
            (!Lufia2DecompIsWramByte(address + 1u) && !Lufia2DecompIsLoRomDataByte(cart, address + 1u)))
            goto finished;
        Lufia2SpanFetch(registers, 0x82d348u, 4u, span.instructions != 0u);
        Lufia2DecompApplyOperand(registers, 0xa9u,
            Lufia2DecompReadData(registers, address, address + 1u, true), false);
        NativeSpanStep(&span, registers, cpu, 6u, 0xd34cu);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
mask_rate:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d34cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd34fu);
store_rate:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d34fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x1cu, 0x1du, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd351u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
restore_band:
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu) goto finished;
    Lufia2SpanFetch(registers, 0x82d351u, 1u, span.instructions != 0u);
    {
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        Lufia2DecompApplyOperand(registers, 0xa9u, value, false);
    }
    NativeSpanStep(&span, registers, cpu, 5u, 0xd352u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
compare_form:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d352u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, 0x13u, 0x14u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd354u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
branch_lower_band:
    {
        const bool taken = !registers->c;
        const unsigned cycles = taken ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x82d354u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xd35cu : 0xd356u);
        if (taken) goto load_scaled_rate;
    }
branch_equal_band:
    {
        const bool taken = registers->z;
        const unsigned cycles = taken ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x82d356u, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xd35au : 0xd358u);
        goto halve_rate;
    }
halve_rate:
    while (registers->pc <= 0xd35au) {
        if (!NativeSpanCanStep(&span, cpu, 7u)) goto finished;
        Lufia2SpanFetch(registers, 0x820000u | registers->pc, 2u, span.instructions != 0u);
        CapsuleGrowthShiftWord(registers, 0x1cu);
        NativeSpanStep(&span, registers, cpu, 7u, registers->pc + 2u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
load_scaled_rate:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d35cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, 0x1cu, 0x1du, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd35eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
clear_sum_carry:
    if (!NativeSpanCanStep(&span, cpu, 2u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d35eu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xd35fu);
add_sum:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d35fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, 0x11u, 0x12u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd361u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
store_sum:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d361u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, 0x11u, 0x12u, registers->a, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd363u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
reload_level:
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(bank + 0x10edu) || !Lufia2DecompIsWramByte(bank + 0x10eeu)) goto finished;
    Lufia2SpanFetch(registers, 0x82d363u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, bank + 0x10edu, bank + 0x10eeu, true), false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xd366u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
mask_loop_level:
    if (!NativeSpanCanStep(&span, cpu, 3u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d366u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0xffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xd369u);
advance_counter:
    if (!NativeSpanCanStep(&span, cpu, 7u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d369u, 2u, span.instructions != 0u);
    {
        const uint16_t value = (uint16_t)(Lufia2DecompReadData(registers, 0x15u, 0x16u, true) + 1u);
        CapsuleGrowthWriteWord(registers, 0x15u, value);
        registers->n = (value & 0x8000u) != 0u;
        registers->z = value == 0u;
    }
    NativeSpanStep(&span, registers, cpu, 7u, 0xd36bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
compare_level:
    if (!NativeSpanCanStep(&span, cpu, 4u)) goto finished;
    Lufia2SpanFetch(registers, 0x82d36bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, 0x15u, 0x16u, true), false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xd36du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
repeat_growth:
    {
        const bool taken = !registers->z;
        const unsigned cycles = taken ? 3u : 2u;
        if (!NativeSpanCanStep(&span, cpu, cycles)) goto finished;
        Lufia2SpanFetch(registers, 0x82d36du, 2u, span.instructions != 0u);
        NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xd33du : 0xd36fu);
        if (taken) {
            if (++span.slots < 8u) goto load_counter;
            goto finished;
        }
    }
normalize_sum:
    while (registers->pc <= 0xd375u) {
        if (!NativeSpanCanStep(&span, cpu, 7u)) goto finished;
        Lufia2SpanFetch(registers, 0x820000u | registers->pc, 2u, span.instructions != 0u);
        CapsuleGrowthShiftWord(registers, 0x11u);
        NativeSpanStep(&span, registers, cpu, 7u, registers->pc + 2u);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    }
return_short:
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x82d377u, 1u, span.instructions != 0u);
    {
        uint16_t address = registers->read(registers->mem, ++registers->sp);
        address |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        NativeSpanStep(&span, registers, cpu, 6u, (uint16_t)(address + 1u));
    }
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
finished:
    if (!span.instructions) return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
