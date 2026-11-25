#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldBuildAttributesSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x80eda5u && pc < 0x80edeau)
        end_pc = 0x80edeau;
    else if (pc >= 0x80edeau && pc < 0x80edffu)
        end_pc = 0x80edffu;
    else if (pc >= 0x80edffu && pc < 0x80ee05u)
        end_pc = 0x80ee05u;
    else if (pc >= 0x80ee05u && pc < 0x80ee17u)
        end_pc = 0x80ee17u;
    else if (pc >= 0x80ee17u && pc < 0x80ee18u)
        end_pc = 0x80ee18u;
    else if (pc >= 0x80ee18u && pc < 0x80ee2cu)
        end_pc = 0x80ee2cu;
    else if (pc >= 0x80ee2cu && pc < 0x80ee30u)
        end_pc = 0x80ee30u;
    else if (pc >= 0x80ee30u && pc < 0x80ee78u)
        end_pc = 0x80ee78u;
    else if (pc >= 0x80ee78u && pc < 0x80ee7bu)
        end_pc = 0x80ee7bu;
    else if (pc >= 0x80ee7bu && pc < 0x80ee84u)
        end_pc = 0x80ee84u;
    else if (pc >= 0x80ee84u && pc < 0x80eedeu)
        end_pc = 0x80eedeu;
    else if (pc >= 0x80eedeu && pc < 0x80eee5u)
        end_pc = 0x80eee5u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x80eda5u: byte_accumulator = true; break;
    case 0x80eda6u: byte_accumulator = true; break;
    case 0x80eda8u: byte_accumulator = true; break;
    case 0x80eda9u: byte_accumulator = true; break;
    case 0x80edaau: byte_accumulator = true; break;
    case 0x80edacu: byte_accumulator = true; break;
    case 0x80edadu: byte_accumulator = true; break;
    case 0x80edaeu: byte_accumulator = true; break;
    case 0x80edb2u: byte_accumulator = true; break;
    case 0x80edb6u: byte_accumulator = true; break;
    case 0x80edb9u: byte_accumulator = true; break;
    case 0x80edbcu: byte_accumulator = true; break;
    case 0x80edc0u: byte_accumulator = true; break;
    case 0x80edc4u: byte_accumulator = true; break;
    case 0x80edc7u: byte_accumulator = true; break;
    case 0x80edcau: byte_accumulator = true; break;
    case 0x80edccu: byte_accumulator = true; break;
    case 0x80edceu: width_neutral = true; break;
    case 0x80ede8u: width_neutral = true; break;
    case 0x80edeau: byte_accumulator = true; break;
    case 0x80edebu: byte_accumulator = true; break;
    case 0x80edefu: byte_accumulator = true; break;
    case 0x80edf1u: byte_accumulator = true; break;
    case 0x80edf2u: byte_accumulator = true; break;
    case 0x80edf6u: byte_accumulator = true; break;
    case 0x80edf7u: byte_accumulator = true; break;
    case 0x80edf9u: byte_accumulator = true; break;
    case 0x80edfbu: byte_accumulator = true; break;
    case 0x80edfdu: byte_accumulator = true; break;
    case 0x80edffu: byte_accumulator = true; break;
    case 0x80ee01u: byte_accumulator = true; break;
    case 0x80ee03u: byte_accumulator = true; break;
    case 0x80ee05u: byte_accumulator = true; break;
    case 0x80ee07u: byte_accumulator = true; break;
    case 0x80ee09u: byte_accumulator = true; break;
    case 0x80ee0bu: byte_accumulator = true; break;
    case 0x80ee0du: byte_accumulator = true; break;
    case 0x80ee0fu: byte_accumulator = true; break;
    case 0x80ee11u: byte_accumulator = true; break;
    case 0x80ee13u: byte_accumulator = true; break;
    case 0x80ee15u: byte_accumulator = true; break;
    case 0x80ee17u: byte_accumulator = true; break;
    case 0x80ee18u: byte_accumulator = true; break;
    case 0x80ee1cu: byte_accumulator = true; break;
    case 0x80ee1eu: byte_accumulator = true; break;
    case 0x80ee1fu: byte_accumulator = true; break;
    case 0x80ee20u: byte_accumulator = true; break;
    case 0x80ee22u: byte_accumulator = true; break;
    case 0x80ee25u: byte_accumulator = true; break;
    case 0x80ee26u: byte_accumulator = true; break;
    case 0x80ee27u: byte_accumulator = true; break;
    case 0x80ee28u: byte_accumulator = true; break;
    case 0x80ee2au: byte_accumulator = true; break;
    case 0x80ee2cu: byte_accumulator = true; break;
    case 0x80ee2eu: byte_accumulator = true; break;
    case 0x80ee30u: byte_accumulator = true; break;
    case 0x80ee34u: width_neutral = true; break;
    case 0x80ee3bu: width_neutral = true; break;
    case 0x80ee3du: byte_accumulator = true; break;
    case 0x80ee41u: byte_accumulator = true; break;
    case 0x80ee43u: byte_accumulator = true; break;
    case 0x80ee45u: byte_accumulator = true; break;
    case 0x80ee49u: byte_accumulator = true; break;
    case 0x80ee4cu: byte_accumulator = true; break;
    case 0x80ee4fu: byte_accumulator = true; break;
    case 0x80ee52u: byte_accumulator = true; break;
    case 0x80ee53u: byte_accumulator = true; break;
    case 0x80ee57u: byte_accumulator = true; break;
    case 0x80ee58u: width_neutral = true; break;
    case 0x80ee5fu: width_neutral = true; break;
    case 0x80ee61u: byte_accumulator = true; break;
    case 0x80ee65u: byte_accumulator = true; break;
    case 0x80ee67u: byte_accumulator = true; break;
    case 0x80ee6bu: byte_accumulator = true; break;
    case 0x80ee6cu: byte_accumulator = true; break;
    case 0x80ee6du: byte_accumulator = true; break;
    case 0x80ee6eu: byte_accumulator = true; break;
    case 0x80ee6fu: byte_accumulator = true; break;
    case 0x80ee70u: byte_accumulator = true; break;
    case 0x80ee72u: byte_accumulator = true; break;
    case 0x80ee75u: byte_accumulator = true; break;
    case 0x80ee78u: byte_accumulator = true; break;
    case 0x80ee7bu: byte_accumulator = true; break;
    case 0x80ee7cu: byte_accumulator = true; break;
    case 0x80ee80u: byte_accumulator = true; break;
    case 0x80ee82u: byte_accumulator = true; break;
    case 0x80ee84u: byte_accumulator = true; break;
    case 0x80ee86u: byte_accumulator = true; break;
    case 0x80ee8au: byte_accumulator = true; break;
    case 0x80ee8cu: byte_accumulator = true; break;
    case 0x80ee8eu: byte_accumulator = true; break;
    case 0x80ee8fu: byte_accumulator = true; break;
    case 0x80ee93u: byte_accumulator = true; break;
    case 0x80ee94u: byte_accumulator = true; break;
    case 0x80ee96u: byte_accumulator = true; break;
    case 0x80ee99u: byte_accumulator = true; break;
    case 0x80ee9du: byte_accumulator = true; break;
    case 0x80eea1u: byte_accumulator = true; break;
    case 0x80eea5u: byte_accumulator = true; break;
    case 0x80eea7u: byte_accumulator = true; break;
    case 0x80eeabu: byte_accumulator = true; break;
    case 0x80eeadu: byte_accumulator = true; break;
    case 0x80eeafu: byte_accumulator = true; break;
    case 0x80eeb0u: byte_accumulator = true; break;
    case 0x80eeb2u: byte_accumulator = true; break;
    case 0x80eeb3u: byte_accumulator = true; break;
    case 0x80eeb5u: byte_accumulator = true; break;
    case 0x80eeb9u: byte_accumulator = true; break;
    case 0x80eebbu: byte_accumulator = true; break;
    case 0x80eebdu: byte_accumulator = true; break;
    case 0x80eebfu: byte_accumulator = true; break;
    case 0x80eec3u: byte_accumulator = true; break;
    case 0x80eec5u: byte_accumulator = true; break;
    case 0x80eec9u: width_neutral = true; break;
    case 0x80eed2u: width_neutral = true; break;
    case 0x80eed4u: byte_accumulator = true; break;
    case 0x80eed8u: byte_accumulator = true; break;
    case 0x80eedau: byte_accumulator = true; break;
    case 0x80eedeu: byte_accumulator = true; break;
    case 0x80eedfu: byte_accumulator = true; break;
    case 0x80eee0u: byte_accumulator = true; break;
    case 0x80eee3u: byte_accumulator = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80eda5u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 320u ||
        memcmp(cart->rom + first,
               "\xeb\xa9\x00\xeb\xaa\xa9\x7e\x48\xab\xbf\x10\xd0\x7f\x8f\x02\x42\x00\x8d\xb9\x05\x9c\xba\x05"
               "\xbf\x18\xd0\x7f\x8f\x03\x42\x00\x8d\xbb\x05\x9c\xbc\x05\xa9\x7f\x85\x62\xc2\x20\xaf\x3e\xd0"
               "\x7f\x85\x60\xbf\x08\xd0\x7f\xaa\xa0\x00\x00\xaf\x16\x42\x00\x18\x69\x00\x01\x85\x54\xe2\x20"
               "\x5a\xbf\x01\x00\x7f\x29\x03\xeb\xbf\x00\x00\x7f\xa8\x64\x56\xb7\x60\x89\xf0\xf0\x06\xa9\x08"
               "\x85\x56\x80\x12\xc9\x08\xf0\x0a\xc9\x09\xf0\x02\x80\x08\xa9\x80\x80\x02\xa9\x02\x85\x56\x7a"
               "\xbf\x01\x00\x7f\x29\x0c\x0a\x0a\x05\x56\x99\x00\x40\xc8\xe8\xe8\xc6\x54\xd0\xbe\xc6\x55\xd0"
               "\xba\x22\x2f\xef\x80\xc2\x20\xaf\x26\xf0\x7e\xaa\xe2\x20\xbf\x00\xf0\x7e\xc9\xff\xf0\x2d\xbf"
               "\x02\xf0\x7e\x8d\x02\x42\xad\xb9\x05\x8d\x03\x42\x7b\xbf\x01\xf0\x7e\xda\xc2\x20\x18\x6d\x16"
               "\x42\xaa\xe2\x20\xbf\x00\x40\x7e\x09\x01\x9f\x00\x40\x7e\xfa\xe8\xe8\xe8\xe8\x80\xcb\xa2\x1e"
               "\x00\x20\xeb\xee\xa2\x00\x00\xda\xbf\x9c\xd6\x7f\xc9\xff\xf0\x5a\x85\x8f\xbf\xcc\xd6\x7f\x85"
               "\x91\xa9\x0a\xeb\xbf\xfc\xd6\x7f\x38\xe9\x10\xa2\x16\x00\x22\xaa\xbf\x80\xbf\x07\xf0\x7e\x1f"
               "\x08\xf0\x7e\xf0\x37\xbf\x05\xf0\x7e\x85\x54\xa5\x91\x38\xe5\x54\x1a\x85\x91\x22\xa5\xf9\x83"
               "\xa5\x54\xc9\x02\xd0\x15\xbf\x00\x40\x7e\x09\x40\x9f\x00\x40\x7e\xc2\x20\x8a\x18\x6f\xb9\x05"
               "\x00\xaa\xe2\x20\xbf\x00\x40\x7e\x09\x08\x9f\x00\x40\x7e\xfa\xe8\xe0\x30\x00\xd0\x96",
               320u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80eda5u: goto exchange_map_id;
    case 0x80eda6u: goto clear_map_id_high;
    case 0x80eda8u: goto restore_map_id;
    case 0x80eda9u: goto select_map_layout;
    case 0x80edaau: goto wram_bank;
    case 0x80edacu: goto save_wram_bank;
    case 0x80edadu: goto set_wram_bank;
    case 0x80edaeu: goto layer_width;
    case 0x80edb2u: goto multiply_width;
    case 0x80edb6u: goto store_map_width;
    case 0x80edb9u: goto clear_map_width_high;
    case 0x80edbcu: goto layer_height;
    case 0x80edc0u: goto multiply_height;
    case 0x80edc4u: goto store_map_height;
    case 0x80edc7u: goto clear_map_height_high;
    case 0x80edcau: goto class_bank;
    case 0x80edccu: goto store_class_bank;
    case 0x80edceu: goto widen_class_pointer;
    case 0x80edd0u: goto read_class_base;
    case 0x80edd4u: goto store_class_base;
    case 0x80edd6u: goto read_cell_base;
    case 0x80eddau: goto select_cell_source;
    case 0x80eddbu: goto first_target_cell;
    case 0x80eddeu: goto read_map_cell_count;
    case 0x80ede2u: goto clear_count_carry;
    case 0x80ede3u: goto pad_map_cell_count;
    case 0x80ede6u: goto store_map_cell_count;
    case 0x80ede8u: goto narrow_cell_class;
    case 0x80edeau: goto save_target_cell;
    case 0x80edebu: goto read_cell_high;
    case 0x80edefu: goto mask_cell_high;
    case 0x80edf1u: goto exchange_cell_high;
    case 0x80edf2u: goto read_cell_low;
    case 0x80edf6u: goto select_cell_class;
    case 0x80edf7u: goto clear_class_bits;
    case 0x80edf9u: goto read_cell_class;
    case 0x80edfbu: goto test_class_high;
    case 0x80edfdu: goto select_low_class;
    case 0x80edffu: goto high_class_bit;
    case 0x80ee01u: goto store_high_class_bit;
    case 0x80ee03u: goto finish_high_class;
    case 0x80ee05u: goto test_blocked_class;
    case 0x80ee07u: goto select_blocked_class;
    case 0x80ee09u: goto test_special_class;
    case 0x80ee0bu: goto select_special_class;
    case 0x80ee0du: goto finish_normal_class;
    case 0x80ee0fu: goto special_class_bit;
    case 0x80ee11u: goto finish_special_class;
    case 0x80ee13u: goto blocked_class_bit;
    case 0x80ee15u: goto store_low_class_bits;
    case 0x80ee17u: goto restore_target_cell;
    case 0x80ee18u: goto read_cell_flags;
    case 0x80ee1cu: goto mask_cell_flags;
    case 0x80ee1eu: goto shift_cell_flags_first;
    case 0x80ee1fu: goto shift_cell_flags_second;
    case 0x80ee20u: goto merge_class_bits;
    case 0x80ee22u: goto write_cell_attributes;
    case 0x80ee25u: goto next_target_cell;
    case 0x80ee26u: goto next_source_low;
    case 0x80ee27u: goto next_source_high;
    case 0x80ee28u: goto count_cell_low;
    case 0x80ee2au: goto repeat_cell_low;
    case 0x80ee2cu: goto count_cell_high;
    case 0x80ee2eu: goto repeat_cell_high;
    case 0x80ee30u: goto mark_actor_cells;
    case 0x80ee34u: goto widen_actor_list;
    case 0x80ee36u: goto read_actor_list;
    case 0x80ee3au: goto select_actor_list;
    case 0x80ee3bu: goto narrow_actor_record;
    case 0x80ee3du: goto read_actor_record;
    case 0x80ee41u: goto test_actor_list_end;
    case 0x80ee43u: goto finish_actor_list;
    case 0x80ee45u: goto read_actor_row;
    case 0x80ee49u: goto multiply_actor_row;
    case 0x80ee4cu: goto read_actor_map_width;
    case 0x80ee4fu: goto multiply_actor_width;
    case 0x80ee52u: goto clear_actor_column_high;
    case 0x80ee53u: goto read_actor_column;
    case 0x80ee57u: goto save_actor_record;
    case 0x80ee58u: goto widen_actor_cell;
    case 0x80ee5au: goto clear_actor_cell_carry;
    case 0x80ee5bu: goto add_actor_row_product;
    case 0x80ee5eu: goto select_actor_cell;
    case 0x80ee5fu: goto narrow_actor_attribute;
    case 0x80ee61u: goto read_actor_attribute;
    case 0x80ee65u: goto actor_occupancy_bit;
    case 0x80ee67u: goto store_actor_attribute;
    case 0x80ee6bu: goto restore_actor_record;
    case 0x80ee6cu: goto next_actor_record_first;
    case 0x80ee6du: goto next_actor_record_second;
    case 0x80ee6eu: goto next_actor_record_third;
    case 0x80ee6fu: goto next_actor_record_fourth;
    case 0x80ee70u: goto repeat_actor_records;
    case 0x80ee72u: goto listed_cells_record;
    case 0x80ee75u: goto mark_listed_cells;
    case 0x80ee78u: goto first_object_record;
    case 0x80ee7bu: goto save_object_record;
    case 0x80ee7cu: goto read_object_column;
    case 0x80ee80u: goto test_empty_object;
    case 0x80ee82u: goto skip_empty_object;
    case 0x80ee84u: goto store_object_column;
    case 0x80ee86u: goto read_object_row;
    case 0x80ee8au: goto store_object_row;
    case 0x80ee8cu: goto object_record_high;
    case 0x80ee8eu: goto exchange_object_record;
    case 0x80ee8fu: goto read_object_record;
    case 0x80ee93u: goto set_object_record_carry;
    case 0x80ee94u: goto object_record_offset;
    case 0x80ee96u: goto object_search_list;
    case 0x80ee99u: goto find_object_record;
    case 0x80ee9du: goto read_object_flag_low;
    case 0x80eea1u: goto merge_object_flags;
    case 0x80eea5u: goto skip_inactive_object;
    case 0x80eea7u: goto read_object_size;
    case 0x80eeabu: goto store_object_size;
    case 0x80eeadu: goto read_object_probe_row;
    case 0x80eeafu: goto set_object_row_carry;
    case 0x80eeb0u: goto subtract_object_height;
    case 0x80eeb2u: goto next_object_row;
    case 0x80eeb3u: goto store_object_probe_row;
    case 0x80eeb5u: goto locate_object_cell;
    case 0x80eeb9u: goto read_object_height;
    case 0x80eebbu: goto test_tall_object;
    case 0x80eebdu: goto skip_upper_object_cell;
    case 0x80eebfu: goto read_upper_object_attribute;
    case 0x80eec3u: goto upper_object_bit;
    case 0x80eec5u: goto store_upper_object_attribute;
    case 0x80eec9u: goto widen_object_stride;
    case 0x80eecbu: goto object_cell_position;
    case 0x80eeccu: goto clear_object_stride_carry;
    case 0x80eecdu: goto add_object_row_stride;
    case 0x80eed1u: goto select_lower_object_cell;
    case 0x80eed2u: goto narrow_lower_object_attribute;
    case 0x80eed4u: goto read_lower_object_attribute;
    case 0x80eed8u: goto lower_object_bit;
    case 0x80eedau: goto store_lower_object_attribute;
    case 0x80eedeu: goto restore_object_record;
    case 0x80eedfu: goto next_object_record;
    case 0x80eee0u: goto test_object_list_end;
    case 0x80eee3u: goto repeat_object_records;
    default: return 0;
    }
exchange_map_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda5u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeda6u);
}
clear_map_id_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeda8u);
}
restore_map_id: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda8u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeda9u);
}
select_map_layout: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xedaau);
}
wram_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edaau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xedacu);
}
save_wram_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80edacu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xedadu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_wram_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80edadu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xedaeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
layer_width: {
    const uint32_t address = (0x7fd010u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edaeu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedb2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_width: {
    const uint32_t address = 0x004202u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edb2u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xedb6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_map_width: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05b9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edb6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedb9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_map_width_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05bau) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edb9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedbcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
layer_height: {
    const uint32_t address = (0x7fd018u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edbcu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedc0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_height: {
    const uint32_t address = 0x004203u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edc0u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xedc4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_map_height: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05bbu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edc4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedc7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_map_height_high: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05bcu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edc7u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedcau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
class_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edcau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xedccu);
}
store_class_bank: {
    const uint32_t address = (uint16_t)(registers->dp + 0x62u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edccu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_class_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edceu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xedd0u);
}
read_class_base: {
    const uint32_t address = 0x7fd03eu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edd0u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xedd4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_class_base: {
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edd4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedd6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_cell_base: {
    const uint32_t address = (0x7fd008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edd6u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeddau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_cell_source: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eddau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeddbu);
}
first_target_cell: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eddbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeddeu);
}
read_map_cell_count: {
    const uint32_t address = 0x004216u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eddeu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xede2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_count_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ede2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xede3u);
}
pad_map_cell_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ede3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0100u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xede6u);
}
store_map_cell_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ede6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xede8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_cell_class: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ede8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xedeau);
    goto finished;
}
save_target_cell: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80edeau, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xedebu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_cell_high: {
    const uint32_t address = (0x7f0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edebu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedefu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_cell_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edefu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0003u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xedf1u);
}
exchange_cell_high: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edf1u, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0xedf2u);
}
read_cell_low: {
    const uint32_t address = (0x7f0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edf2u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedf6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_cell_class: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edf6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xedf7u);
}
clear_class_bits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edf7u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedf9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_cell_class: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0x60u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edf9u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer + registers->y) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xedfbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_class_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edfbu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x00f0u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xedfdu);
}
select_low_class: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edfdu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xee05u : 0xedffu);
    if (taken)
        goto finished;
    goto finished;
}
high_class_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80edffu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee01u);
}
store_high_class_bit: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee01u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee03u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_high_class: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee03u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee17u);
    goto finished;
}
test_blocked_class: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee05u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee07u);
}
select_blocked_class: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee07u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xee13u : 0xee09u);
    if (taken)
        goto blocked_class_bit;
}
test_special_class: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee09u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0009u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee0bu);
}
select_special_class: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee0bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xee0fu : 0xee0du);
    if (taken)
        goto special_class_bit;
}
finish_normal_class: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee0du, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee17u);
    goto finished;
}
special_class_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee0fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee11u);
}
finish_special_class: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee11u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee15u);
    goto finished;
}
blocked_class_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee13u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee15u);
}
store_low_class_bits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee15u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee17u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_target_cell: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee17u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xee18u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_cell_flags: {
    const uint32_t address = (0x7f0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee18u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee1cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_cell_flags: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee1cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x000cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee1eu);
}
shift_cell_flags_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee1eu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee1fu);
}
shift_cell_flags_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee1fu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee20u);
}
merge_class_bits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee20u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee22u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
write_cell_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee22u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee25u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_target_cell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee25u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee26u);
}
next_source_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee26u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee27u);
}
next_source_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee27u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee28u);
}
count_cell_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee28u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee2au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_cell_low: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee2au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xedeau : 0xee2cu);
    if (taken)
        goto finished;
    goto finished;
}
count_cell_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x55u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee2cu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee2eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_cell_high: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee2eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xedeau : 0xee30u);
    if (taken)
        goto finished;
    goto finished;
}
mark_actor_cells: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80ee30u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x80ee30u, 0x22u, 0x80ef2fu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_actor_list: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee34u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee36u);
}
read_actor_list: {
    const uint32_t address = 0x7ef026u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee36u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xee3au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_actor_list: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee3au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee3bu);
}
narrow_actor_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee3bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee3du);
}
read_actor_record: {
    const uint32_t address = (0x7ef000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee3du, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee41u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_actor_list_end: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee41u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee43u);
}
finish_actor_list: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee43u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xee72u : 0xee45u);
    if (taken)
        goto listed_cells_record;
}
read_actor_row: {
    const uint32_t address = (0x7ef002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee45u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee49u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_actor_row: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4202u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee49u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xee4cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_map_width: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05b9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee4cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee4fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_actor_width: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4203u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee4fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xee52u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_actor_column_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee52u, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0xee53u);
}
read_actor_column: {
    const uint32_t address = (0x7ef001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee53u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee57u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_actor_record: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee57u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xee58u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_actor_cell: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee58u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee5au);
}
clear_actor_cell_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee5au, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xee5bu);
}
add_actor_row_product: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4216u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        (!mapped_math && (!Lufia2DecompIsDecompressSourceByte(cart, address) ||
                          !Lufia2DecompIsDecompressSourceByte(cart, high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee5bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_actor_cell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee5eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee5fu);
}
narrow_actor_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee5fu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee61u);
}
read_actor_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee61u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee65u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
actor_occupancy_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee65u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee67u);
}
store_actor_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee67u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee6bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_actor_record: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee6bu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xee6cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_actor_record_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee6cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee6du);
}
next_actor_record_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee6du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee6eu);
}
next_actor_record_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee6eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee6fu);
}
next_actor_record_fourth: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee6fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee70u);
}
repeat_actor_records: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee70u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee3du);
    goto finished;
}
listed_cells_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee72u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x001eu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee75u);
}
mark_listed_cells: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80ee75u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x80ee75u, 0x20u, 0x80eeebu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
first_object_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee78u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee7bu);
    goto finished;
}
save_object_record: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee7bu, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xee7cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_column: {
    const uint32_t address = (0x7fd69cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee7cu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee80u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_empty_object: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee80u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee82u);
}
skip_empty_object: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee82u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xeedeu : 0xee84u);
    if (taken)
        goto finished;
    goto finished;
}
store_object_column: {
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee84u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_row: {
    const uint32_t address = (0x7fd6ccu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee86u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee8au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_object_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee8au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee8cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
object_record_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee8cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x000au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee8eu);
}
exchange_object_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee8eu, 1u, span.instructions != 0u);
    Lufia2DecompRunTransferOrFlags(registers, 0xebu);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee8fu);
}
read_object_record: {
    const uint32_t address = (0x7fd6fcu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee8fu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xee93u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_object_record_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee93u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xee94u);
}
object_record_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee94u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xee96u);
}
object_search_list: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee96u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0016u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xee99u);
}
find_object_record: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80ee99u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x80ee99u, 0x22u, 0x80bfaau, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_object_flag_low: {
    const uint32_t address = (0x7ef007u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ee9du, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeea1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
merge_object_flags: {
    const uint32_t address = (0x7ef008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eea1u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeea5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_inactive_object: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eea5u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xeedeu : 0xeea7u);
    if (taken)
        goto finished;
}
read_object_size: {
    const uint32_t address = (0x7ef005u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eea7u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeeabu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_object_size: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeabu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeeadu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_probe_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeadu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeeafu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_object_row_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeafu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xeeb0u);
}
subtract_object_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeb0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeeb2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_object_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeb2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a + 1u) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeeb3u);
}
store_object_probe_row: {
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeb3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeeb5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
locate_object_cell: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80eeb5u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x80eeb5u, 0x22u, 0x83f9a5u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_object_height: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeb9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeebbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_tall_object: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eebbu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeebdu);
}
skip_upper_object_cell: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eebdu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xeed4u : 0xeebfu);
    if (taken)
        goto read_lower_object_attribute;
}
read_upper_object_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eebfu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeec3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
upper_object_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eec3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0040u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeec5u);
}
store_upper_object_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eec5u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeec9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_object_stride: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eec9u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeecbu);
}
object_cell_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eecbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeeccu);
}
clear_object_stride_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeccu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xeecdu);
}
add_object_row_stride: {
    const uint32_t address = 0x0005b9u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eecdu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xeed1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_lower_object_cell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eed1u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeed2u);
}
narrow_lower_object_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eed2u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeed4u);
}
read_lower_object_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eed4u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeed8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
lower_object_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eed8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeedau);
}
store_lower_object_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eedau, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeedeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_object_record: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80eedeu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xeedfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_object_record: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eedfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeee0u);
}
test_object_list_end: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0030u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeee3u);
}
repeat_object_records: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee3u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xee7bu : 0xeee5u);
    if (taken)
        goto finished;
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
static int Lufia2DecompRunFieldListedAttributesSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                    uint8_t opcode, uint64_t dynamic_epoch,
                                                    uint64_t write_epoch, unsigned step_budget,
                                                    long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x80eeebu && pc < 0x80ef2eu)
        end_pc = 0x80ef2eu;
    else if (pc >= 0x80ef2eu && pc < 0x80ef2fu)
        end_pc = 0x80ef2fu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x80eeebu: byte_accumulator = true; break;
    case 0x80eeedu: byte_accumulator = true; break;
    case 0x80eeeeu: byte_accumulator = true; break;
    case 0x80eeefu: width_neutral = true; break;
    case 0x80eef6u: width_neutral = true; break;
    case 0x80eef8u: byte_accumulator = true; break;
    case 0x80eefcu: byte_accumulator = true; break;
    case 0x80eefeu: byte_accumulator = true; break;
    case 0x80ef00u: byte_accumulator = true; break;
    case 0x80ef04u: byte_accumulator = true; break;
    case 0x80ef08u: byte_accumulator = true; break;
    case 0x80ef0bu: byte_accumulator = true; break;
    case 0x80ef0fu: byte_accumulator = true; break;
    case 0x80ef10u: byte_accumulator = true; break;
    case 0x80ef14u: width_neutral = true; break;
    case 0x80ef22u: width_neutral = true; break;
    case 0x80ef24u: byte_accumulator = true; break;
    case 0x80ef27u: byte_accumulator = true; break;
    case 0x80ef29u: byte_accumulator = true; break;
    case 0x80ef2cu: byte_accumulator = true; break;
    case 0x80ef2eu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80eeebu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 68u ||
        memcmp(cart->rom + first,
               "\xa9\x7e\x48\xab\xc2\x20\xbf\x00\xf0\x7e\xaa\xe2\x20\xbf\x00\xf0\x7e\xc9\xff\xf0\x2e\xbf\x02"
               "\xf0\x7e\x8f\x02\x42\x00\xad\xb9\x05\x8f\x03\x42\x00\x7b\xbf\x01\xf0\x7e\xc2\x20\x18\x6f\x16"
               "\x42\x00\xa8\x8a\x18\x69\x03\x00\xaa\xe2\x20\xb9\x00\x40\x09\x04\x99\x00\x40\x80\xca\x60",
               68u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80eeebu: goto listed_wram_bank;
    case 0x80eeedu: goto save_listed_bank;
    case 0x80eeeeu: goto set_listed_bank;
    case 0x80eeefu: goto widen_list_pointer;
    case 0x80eef1u: goto read_list_pointer;
    case 0x80eef5u: goto select_list_pointer;
    case 0x80eef6u: goto narrow_list_record;
    case 0x80eef8u: goto read_list_marker;
    case 0x80eefcu: goto test_list_end;
    case 0x80eefeu: goto finish_list;
    case 0x80ef00u: goto read_list_row;
    case 0x80ef04u: goto multiply_list_row;
    case 0x80ef08u: goto read_list_width;
    case 0x80ef0bu: goto multiply_list_width;
    case 0x80ef0fu: goto clear_list_column_high;
    case 0x80ef10u: goto read_list_column;
    case 0x80ef14u: goto widen_list_cell;
    case 0x80ef16u: goto clear_list_carry;
    case 0x80ef17u: goto add_list_row_product;
    case 0x80ef1bu: goto select_list_cell;
    case 0x80ef1cu: goto list_record_position;
    case 0x80ef1du: goto clear_list_stride_carry;
    case 0x80ef1eu: goto next_list_record;
    case 0x80ef21u: goto select_next_list_record;
    case 0x80ef22u: goto narrow_list_attribute;
    case 0x80ef24u: goto read_list_attribute;
    case 0x80ef27u: goto listed_attribute_bit;
    case 0x80ef29u: goto store_list_attribute;
    case 0x80ef2cu: goto repeat_list_records;
    case 0x80ef2eu: goto return_listed_cells;
    default: return 0;
    }
listed_wram_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeebu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeeedu);
}
save_listed_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeedu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeeeeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_listed_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeeeu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xeeefu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_list_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eeefu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeef1u);
}
read_list_pointer: {
    const uint32_t address = (0x7ef000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eef1u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeef5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_list_pointer: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eef5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeef6u);
}
narrow_list_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eef6u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeef8u);
}
read_list_marker: {
    const uint32_t address = (0x7ef000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eef8u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeefcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_list_end: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eefcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xeefeu);
}
finish_list: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eefeu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xef2eu : 0xef00u);
    if (taken)
        goto finished;
}
read_list_row: {
    const uint32_t address = (0x7ef002u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef00u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef04u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_list_row: {
    const uint32_t address = 0x004202u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef04u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xef08u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_list_width: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05b9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef08u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef0bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_list_width: {
    const uint32_t address = 0x004203u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 5u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef0bu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xef0fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_list_column_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef0fu, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0xef10u);
}
read_list_column: {
    const uint32_t address = (0x7ef001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef10u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef14u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_list_cell: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef14u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef16u);
}
clear_list_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef16u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xef17u);
}
add_list_row_product: {
    const uint32_t address = 0x004216u;
    const uint32_t high = (address + 1u) & 0xffffffu;
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef17u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, 6u, 0xef1bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_list_cell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef1bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef1cu);
}
list_record_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef1cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef1du);
}
clear_list_stride_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef1du, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xef1eu);
}
next_list_record: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef1eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0003u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef21u);
}
select_next_list_record: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef21u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef22u);
}
narrow_list_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef22u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef24u);
}
read_list_attribute: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x4000u >> 8) != ((0x4000u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef24u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef27u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
listed_attribute_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef27u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef29u);
}
store_list_attribute: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef29u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef2cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_list_records: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef2cu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xeef8u);
    goto finished;
}
return_listed_cells: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80ef2eu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x80ef2eu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunFieldActorOccupancySpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x80ef2fu && pc < 0x80ef34u)
        end_pc = 0x80ef34u;
    else if (pc >= 0x80ef3bu && pc < 0x80ef8au)
        end_pc = 0x80ef8au;
    else if (pc >= 0x80ef8du && pc < 0x80ef8eu)
        end_pc = 0x80ef8eu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x80ef2fu: byte_accumulator = true; break;
    case 0x80ef30u: byte_accumulator = true; break;
    case 0x80ef31u: byte_accumulator = true; break;
    case 0x80ef3bu: byte_accumulator = true; break;
    case 0x80ef3eu: byte_accumulator = true; break;
    case 0x80ef41u: byte_accumulator = true; break;
    case 0x80ef44u: byte_accumulator = true; break;
    case 0x80ef47u: byte_accumulator = true; break;
    case 0x80ef48u: byte_accumulator = true; break;
    case 0x80ef4au: byte_accumulator = true; break;
    case 0x80ef4eu: byte_accumulator = true; break;
    case 0x80ef50u: byte_accumulator = true; break;
    case 0x80ef52u: byte_accumulator = true; break;
    case 0x80ef55u: byte_accumulator = true; break;
    case 0x80ef57u: byte_accumulator = true; break;
    case 0x80ef59u: byte_accumulator = true; break;
    case 0x80ef5bu: byte_accumulator = true; break;
    case 0x80ef5du: byte_accumulator = true; break;
    case 0x80ef5fu: byte_accumulator = true; break;
    case 0x80ef61u: byte_accumulator = true; break;
    case 0x80ef63u: byte_accumulator = true; break;
    case 0x80ef65u: byte_accumulator = true; break;
    case 0x80ef66u: byte_accumulator = true; break;
    case 0x80ef69u: width_neutral = true; break;
    case 0x80ef70u: width_neutral = true; break;
    case 0x80ef72u: byte_accumulator = true; break;
    case 0x80ef76u: byte_accumulator = true; break;
    case 0x80ef78u: byte_accumulator = true; break;
    case 0x80ef7cu: byte_accumulator = true; break;
    case 0x80ef7eu: byte_accumulator = true; break;
    case 0x80ef80u: byte_accumulator = true; break;
    case 0x80ef84u: byte_accumulator = true; break;
    case 0x80ef86u: byte_accumulator = true; break;
    case 0x80ef8du: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x80ef2fu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 95u ||
        memcmp(
            cart->rom + first,
            "\x4b\xab\xa0\x27\x00\xb9\x22\x06\x89\x06\xd0\x4f\xb9\xe2\x06\x8d\x02\x42\xad\xb9\x05\x8d\x03\x42"
            "\xbb\x64\x9a\xbf\x16\xe2\x7f\xc9\x02\x90\x13\xbd\xd2\x05\xc9\x71\xf0\x0c\xc9\x72\xf0\x08\xc9\x73"
            "\xf0\x04\xa9\xff\x85\x9a\x7b\xb9\xba\x06\xc2\x20\x18\x6d\x16\x42\xaa\xe2\x20\xbf\x00\x40\x7e\x09"
            "\x01\x9f\x00\x40\x7e\xa5\x9a\xf0\x0a\xbf\x01\x40\x7e\x09\x01\x9f\x01\x40\x7e\x88\x10\xa7\x6b",
            95u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80ef2fu: goto save_code_bank;
    case 0x80ef30u: goto set_actor_bank;
    case 0x80ef31u: goto last_actor;
    case 0x80ef3bu: goto read_actor_row;
    case 0x80ef3eu: goto multiply_actor_row;
    case 0x80ef41u: goto read_actor_width;
    case 0x80ef44u: goto multiply_actor_width;
    case 0x80ef47u: goto select_actor_record;
    case 0x80ef48u: goto clear_wide_actor;
    case 0x80ef4au: goto read_actor_extent;
    case 0x80ef4eu: goto test_actor_extent;
    case 0x80ef50u: goto skip_narrow_actor;
    case 0x80ef52u: goto read_actor_kind;
    case 0x80ef55u: goto test_actor_kind_first;
    case 0x80ef57u: goto select_narrow_first;
    case 0x80ef59u: goto test_actor_kind_second;
    case 0x80ef5bu: goto select_narrow_second;
    case 0x80ef5du: goto test_actor_kind_third;
    case 0x80ef5fu: goto select_narrow_third;
    case 0x80ef61u: goto wide_actor_marker;
    case 0x80ef63u: goto store_wide_actor;
    case 0x80ef65u: goto clear_actor_column_high;
    case 0x80ef66u: goto read_actor_column;
    case 0x80ef69u: goto widen_actor_cell;
    case 0x80ef6bu: goto clear_actor_carry;
    case 0x80ef6cu: goto add_actor_product;
    case 0x80ef6fu: goto select_actor_cell;
    case 0x80ef70u: goto narrow_actor_attribute;
    case 0x80ef72u: goto read_actor_attribute;
    case 0x80ef76u: goto occupancy_bit;
    case 0x80ef78u: goto store_actor_attribute;
    case 0x80ef7cu: goto read_wide_actor;
    case 0x80ef7eu: goto skip_second_cell;
    case 0x80ef80u: goto read_second_attribute;
    case 0x80ef84u: goto second_occupancy_bit;
    case 0x80ef86u: goto store_second_attribute;
    case 0x80ef8du: goto return_actor_cells;
    default: return 0;
    }
save_code_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef2fu, 1u, span.instructions != 0u);
    const uint16_t value = registers->k;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef30u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_actor_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef30u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xef31u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
last_actor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef31u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0027u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef34u);
    goto finished;
}
read_actor_row: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x06e2u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x06e2u >> 8) != ((0x06e2u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef3bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef3eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_actor_row: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4202u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef3eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xef41u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_width: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05b9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef41u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
multiply_actor_width: {
    const uint32_t address = ((uint32_t)registers->db << 16) | 0x4203u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, 4u) || (!mapped_math && !Lufia2DecompIsWramByte(address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef44u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, address + 1u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 4u, 0xef47u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_actor_record: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef47u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef48u);
}
clear_wide_actor: {
    const uint32_t address = (uint16_t)(registers->dp + 0x9au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef48u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef4au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_extent: {
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef4au, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef4eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_actor_extent: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef4eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef50u);
}
skip_narrow_actor: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef50u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xef65u : 0xef52u);
    if (taken)
        goto clear_actor_column_high;
}
read_actor_kind: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05d2u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x05d2u >> 8) != ((0x05d2u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef52u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef55u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_actor_kind_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef55u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0071u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef57u);
}
select_narrow_first: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef57u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xef65u : 0xef59u);
    if (taken)
        goto clear_actor_column_high;
}
test_actor_kind_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef59u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0072u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef5bu);
}
select_narrow_second: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef5bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xef65u : 0xef5du);
    if (taken)
        goto clear_actor_column_high;
}
test_actor_kind_third: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef5du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0073u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef5fu);
}
select_narrow_third: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef5fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xef65u : 0xef61u);
    if (taken)
        goto clear_actor_column_high;
}
wide_actor_marker: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef61u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef63u);
}
store_wide_actor: {
    const uint32_t address = (uint16_t)(registers->dp + 0x9au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef63u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef65u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_actor_column_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef65u, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0xef66u);
}
read_actor_column: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x06bau + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x06bau >> 8) != ((0x06bau + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef66u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef69u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_actor_cell: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef69u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef6bu);
}
clear_actor_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef6bu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xef6cu);
}
add_actor_product: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4216u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    const bool mapped_math = (registers->db & 0x7fu) < 0x40u;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        (!mapped_math && (!Lufia2DecompIsDecompressSourceByte(cart, address) ||
                          !Lufia2DecompIsDecompressSourceByte(cart, high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef6cu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef6fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_actor_cell: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef6fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef70u);
}
narrow_actor_attribute: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef70u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xef72u);
}
read_actor_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef72u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef76u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
occupancy_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef76u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef78u);
}
store_actor_attribute: {
    const uint32_t address = (0x7e4000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef78u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef7cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_wide_actor: {
    const uint32_t address = (uint16_t)(registers->dp + 0x9au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef7cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef7eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_second_cell: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef7eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xef8au : 0xef80u);
    if (taken)
        goto finished;
}
read_second_attribute: {
    const uint32_t address = (0x7e4001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef80u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef84u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_occupancy_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef84u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xef86u);
}
store_second_attribute: {
    const uint32_t address = (0x7e4001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x80ef86u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xef8au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_actor_cells: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80ef8du);
    const int cycles = Lufia2DecompRunReturn(registers, 0x80ef8du, 0x6bu, true, registers->mf, true);
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

static int Lufia2DecompRunFieldAttributeFrameSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                  uint8_t opcode, uint64_t dynamic_epoch,
                                                  uint64_t write_epoch, unsigned step_budget,
                                                  long *step_count) {
    uint32_t start_pc, end_pc;
    if (pc >= 0x80ed9cu && pc < 0x80eda5u) {
        start_pc = 0x80ed9cu;
        end_pc = 0x80eda5u;
    } else if (pc >= 0x80eee5u && pc < 0x80eeebu) {
        start_pc = 0x80eee5u;
        end_pc = 0x80eeebu;
    } else
        return 0;
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || !step_budget ||
        registers->waiting || registers->stopped || registers->irqWanted || registers->nmiWanted ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - start_pc;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (start_pc == 0x80ed9cu &&
        (cart->romSize < first + 9u || memcmp(cart->rom + first, "\x48\xda\x5a\x8b\x08\xe2\x20\xc2\x10", 9u)))
        return 0;
    else if (start_pc == 0x80eee5u &&
             (cart->romSize < first + 6u || memcmp(cart->rom + first, "\x28\xab\x7a\xfa\x68\x6b", 6u)))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x80ed9cu: goto save_map_id;
    case 0x80ed9du: goto save_caller_x;
    case 0x80ed9eu: goto save_caller_y;
    case 0x80ed9fu: goto save_caller_bank;
    case 0x80eda0u: goto save_caller_flags;
    case 0x80eda1u: goto narrow_map_id;
    case 0x80eda3u: goto widen_map_indices;
    case 0x80eee5u: goto restore_caller_flags;
    case 0x80eee6u: goto restore_caller_bank;
    case 0x80eee7u: goto restore_caller_y;
    case 0x80eee8u: goto restore_caller_x;
    case 0x80eee9u: goto restore_map_id;
    case 0x80eeeau: goto return_attributes;
    default: return 0;
    }
save_map_id: {
    const bool byte = registers->mf;
    const unsigned cycles = byte ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (byte ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ed9cu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    if (!byte)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xed9du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_caller_x: {
    const bool byte = registers->xf;
    const unsigned cycles = byte ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (byte ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ed9du, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    if (!byte)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xed9eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_caller_y: {
    const bool byte = registers->xf;
    const unsigned cycles = byte ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (byte ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ed9eu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    if (!byte)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xed9fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_caller_bank: {
    const bool byte = true;
    const unsigned cycles = byte ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (byte ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80ed9fu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    if (!byte)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeda0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_caller_flags: {
    const bool byte = true;
    const unsigned cycles = byte ? 3u : 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < (byte ? 0x1f01u : 0x1f02u) ||
        registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda0u, 1u, span.instructions != 0u);
    const uint16_t value = interp816_getFlags(registers);
    if (!byte)
        registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeda1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_map_id: {
    const unsigned cycles = 3u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda1u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeda3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_map_indices: {
    const unsigned cycles = 3u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eda3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeda5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_caller_flags: {
    const bool byte = true;
    const unsigned cycles = byte ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (byte ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee5u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!byte)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    interp816_setFlags(registers, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeee6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_caller_bank: {
    const bool byte = true;
    const unsigned cycles = byte ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (byte ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee6u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!byte)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xeee7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_caller_y: {
    const bool byte = registers->xf;
    const unsigned cycles = byte ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (byte ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee7u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!byte)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, byte);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeee8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_caller_x: {
    const bool byte = registers->xf;
    const unsigned cycles = byte ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (byte ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee8u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!byte)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, byte);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeee9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_map_id: {
    const bool byte = registers->mf;
    const unsigned cycles = byte ? 4u : 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u ||
        registers->sp > (byte ? 0x1ffeu : 0x1ffdu))
        goto finished;
    Lufia2SpanFetch(registers, 0x80eee9u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (!byte)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, byte);
    NativeSpanStep(&span, registers, cpu, cycles, 0xeeeau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 6u))
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x80eeeau);
    const int cycles = Lufia2DecompRunReturn(registers, 0x80eeeau, 0x6bu, true, registers->mf, true);
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

static int Lufia2DecompRunFieldAttributesSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if ((pc >= 0x80ed9cu && pc < 0x80eda5u) || (pc >= 0x80eee5u && pc < 0x80eeebu))
        return Lufia2DecompRunFieldAttributeFrameSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x80eda5u && pc < 0x80eee5u)
        return Lufia2DecompRunFieldBuildAttributesSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x80eeebu && pc < 0x80ef2fu)
        return Lufia2DecompRunFieldListedAttributesSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x80ef2fu && pc < 0x80ef8eu)
        return Lufia2DecompRunFieldActorOccupancySpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
