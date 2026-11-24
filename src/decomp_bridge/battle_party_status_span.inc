#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattlePartyRowsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x81e63au && pc < 0x81e63du)
        end_pc = 0x81e63du;
    else if (pc >= 0x81e63du && pc < 0x81e640u)
        end_pc = 0x81e640u;
    else if (pc >= 0x81e640u && pc < 0x81e644u)
        end_pc = 0x81e644u;
    else if (pc >= 0x81e644u && pc < 0x81e645u)
        end_pc = 0x81e645u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x81e63au: width_neutral = true; break;
    case 0x81e63du: width_neutral = true; break;
    case 0x81e640u: width_neutral = true; break;
    case 0x81e641u: width_neutral = true; break;
    case 0x81e642u: width_neutral = true; break;
    case 0x81e644u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x81e63au;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 11u ||
        memcmp(cart->rom + first, "\xa2\x06\x00\x20\x45\xe6\xca\xca\x10\xf9\x60", 11u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81e63au: goto last_party_slot;
    case 0x81e63du: goto draw_party_slot;
    case 0x81e640u: goto previous_slot_low;
    case 0x81e641u: goto previous_slot_high;
    case 0x81e642u: goto repeat_party_slots;
    case 0x81e644u: goto return_party_rows;
    default: return 0;
    }
last_party_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e63au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0006u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe63du);
    goto finished;
}
draw_party_slot: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e63du);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x81e63du, 0x20u, 0x81e645u, false,
                                                           registers->mf, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
previous_slot_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e640u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe641u);
}
previous_slot_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e641u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe642u);
}
repeat_party_slots: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e642u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe63du : 0xe644u);
    if (taken)
        goto finished;
    goto finished;
}
return_party_rows: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e644u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e644u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattlePartyStatusSpanBody(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                    uint8_t opcode, uint64_t dynamic_epoch,
                                                    uint64_t write_epoch, unsigned step_budget,
                                                    long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x81e645u && pc < 0x81e674u)
        end_pc = 0x81e674u;
    else if (pc >= 0x81e674u && pc < 0x81e677u)
        end_pc = 0x81e677u;
    else if (pc >= 0x81e677u && pc < 0x81e67au)
        end_pc = 0x81e67au;
    else if (pc >= 0x81e67au && pc < 0x81e687u)
        end_pc = 0x81e687u;
    else if (pc >= 0x81e687u && pc < 0x81e68au)
        end_pc = 0x81e68au;
    else if (pc >= 0x81e68au && pc < 0x81e6eeu)
        end_pc = 0x81e6eeu;
    else if (pc >= 0x81e6eeu && pc < 0x81e6f1u)
        end_pc = 0x81e6f1u;
    else if (pc >= 0x81e6f1u && pc < 0x81e711u)
        end_pc = 0x81e711u;
    else if (pc >= 0x81e711u && pc < 0x81e714u)
        end_pc = 0x81e714u;
    else if (pc >= 0x81e714u && pc < 0x81e735u)
        end_pc = 0x81e735u;
    else if (pc >= 0x81e735u && pc < 0x81e738u)
        end_pc = 0x81e738u;
    else if (pc >= 0x81e738u && pc < 0x81e73au)
        end_pc = 0x81e73au;
    else if (pc >= 0x81e73au && pc < 0x81e73bu)
        end_pc = 0x81e73bu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x81e645u: width_neutral = true; break;
    case 0x81e646u: width_neutral = true; break;
    case 0x81e647u: width_neutral = true; break;
    case 0x81e64eu: width_neutral = true; break;
    case 0x81e650u: byte_accumulator = true; break;
    case 0x81e653u: byte_accumulator = true; break;
    case 0x81e654u: byte_accumulator = true; break;
    case 0x81e655u: byte_accumulator = true; break;
    case 0x81e658u: byte_accumulator = true; break;
    case 0x81e65au: byte_accumulator = true; break;
    case 0x81e65bu: width_neutral = true; break;
    case 0x81e662u: width_neutral = true; break;
    case 0x81e664u: byte_accumulator = true; break;
    case 0x81e666u: byte_accumulator = true; break;
    case 0x81e669u: byte_accumulator = true; break;
    case 0x81e66bu: byte_accumulator = true; break;
    case 0x81e66eu: byte_accumulator = true; break;
    case 0x81e671u: byte_accumulator = true; break;
    case 0x81e674u: byte_accumulator = true; break;
    case 0x81e677u: byte_accumulator = true; break;
    case 0x81e67au: byte_accumulator = true; break;
    case 0x81e67bu: byte_accumulator = true; break;
    case 0x81e67cu: byte_accumulator = true; break;
    case 0x81e67eu: byte_accumulator = true; break;
    case 0x81e67fu: byte_accumulator = true; break;
    case 0x81e680u: byte_accumulator = true; break;
    case 0x81e682u: byte_accumulator = true; break;
    case 0x81e684u: byte_accumulator = true; break;
    case 0x81e687u: byte_accumulator = true; break;
    case 0x81e68au: byte_accumulator = true; break;
    case 0x81e68cu: byte_accumulator = true; break;
    case 0x81e68eu: byte_accumulator = true; break;
    case 0x81e68fu: byte_accumulator = true; break;
    case 0x81e691u: byte_accumulator = true; break;
    case 0x81e694u: byte_accumulator = true; break;
    case 0x81e695u: byte_accumulator = true; break;
    case 0x81e696u: byte_accumulator = true; break;
    case 0x81e697u: byte_accumulator = true; break;
    case 0x81e699u: byte_accumulator = true; break;
    case 0x81e69bu: byte_accumulator = true; break;
    case 0x81e69cu: byte_accumulator = true; break;
    case 0x81e69du: width_neutral = true; break;
    case 0x81e6aeu: width_neutral = true; break;
    case 0x81e6b0u: byte_accumulator = true; break;
    case 0x81e6b2u: byte_accumulator = true; break;
    case 0x81e6b4u: byte_accumulator = true; break;
    case 0x81e6b5u: byte_accumulator = true; break;
    case 0x81e6b8u: byte_accumulator = true; break;
    case 0x81e6bau: byte_accumulator = true; break;
    case 0x81e6bdu: byte_accumulator = true; break;
    case 0x81e6beu: byte_accumulator = true; break;
    case 0x81e6bfu: byte_accumulator = true; break;
    case 0x81e6c0u: byte_accumulator = true; break;
    case 0x81e6c2u: byte_accumulator = true; break;
    case 0x81e6c4u: byte_accumulator = true; break;
    case 0x81e6c6u: byte_accumulator = true; break;
    case 0x81e6c9u: byte_accumulator = true; break;
    case 0x81e6cbu: byte_accumulator = true; break;
    case 0x81e6ceu: byte_accumulator = true; break;
    case 0x81e6cfu: byte_accumulator = true; break;
    case 0x81e6d0u: width_neutral = true; break;
    case 0x81e6e2u: width_neutral = true; break;
    case 0x81e6e4u: byte_accumulator = true; break;
    case 0x81e6e6u: byte_accumulator = true; break;
    case 0x81e6e8u: byte_accumulator = true; break;
    case 0x81e6eau: byte_accumulator = true; break;
    case 0x81e6ecu: byte_accumulator = true; break;
    case 0x81e6edu: byte_accumulator = true; break;
    case 0x81e6eeu: byte_accumulator = true; break;
    case 0x81e6f1u: byte_accumulator = true; break;
    case 0x81e6f2u: byte_accumulator = true; break;
    case 0x81e6f3u: width_neutral = true; break;
    case 0x81e705u: width_neutral = true; break;
    case 0x81e707u: byte_accumulator = true; break;
    case 0x81e709u: byte_accumulator = true; break;
    case 0x81e70bu: byte_accumulator = true; break;
    case 0x81e70du: byte_accumulator = true; break;
    case 0x81e70fu: byte_accumulator = true; break;
    case 0x81e710u: byte_accumulator = true; break;
    case 0x81e711u: byte_accumulator = true; break;
    case 0x81e714u: byte_accumulator = true; break;
    case 0x81e715u: byte_accumulator = true; break;
    case 0x81e716u: width_neutral = true; break;
    case 0x81e71eu: width_neutral = true; break;
    case 0x81e720u: byte_accumulator = true; break;
    case 0x81e723u: byte_accumulator = true; break;
    case 0x81e725u: byte_accumulator = true; break;
    case 0x81e727u: byte_accumulator = true; break;
    case 0x81e729u: byte_accumulator = true; break;
    case 0x81e72bu: byte_accumulator = true; break;
    case 0x81e72du: byte_accumulator = true; break;
    case 0x81e72fu: byte_accumulator = true; break;
    case 0x81e731u: byte_accumulator = true; break;
    case 0x81e733u: byte_accumulator = true; break;
    case 0x81e735u: byte_accumulator = true; break;
    case 0x81e738u: byte_accumulator = true; break;
    case 0x81e739u: byte_accumulator = true; break;
    case 0x81e73au: byte_accumulator = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x81e645u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 246u ||
        memcmp(cart->rom + first,
               "\x8b\xda\xc2\x20\xbf\x19\x88\x81\x48\xe2\x20\xbc\x64\x0a\xbb\x7a\xe0\x00\x00\xd0\x20\x98\xc2"
               "\x20\x18\x69\x40\x00\xaa\xe2\x20\xa9\x06\x8d\xf2\x09\xa9\x04\x8d\xf3\x09\xa0\x50\x21\x8c\xf6"
               "\x09\x20\xd2\xe7\x4c\x38\xe7\xda\x5a\xa9\x7e\x48\xab\xa9\x05\x85\x12\xbd\x00\x00\x20\x35\xe8"
               "\x89\xff\xf0\x06\x38\xe9\x65\x99\x00\x00\xe8\xc8\xc8\xc6\x12\xd0\xe9\x7a\xfa\xc2\x20\x98\x18"
               "\x69\x40\x00\xa8\xda\x5a\xa3\x05\xaa\xbd\xe0\x1b\xaa\xe2\x20\xa9\x05\x85\x1b\x8a\x99\x00\x00"
               "\xa9\x21\x99\x01\x00\xe8\xc8\xc8\xc6\x1b\xd0\xf0\xa9\x50\x99\x00\x00\xa9\x21\x99\x01\x00\x7a"
               "\xfa\xc2\x20\x98\x18\x69\x40\x00\xa8\xbd\x11\x00\x85\x11\xbd\x25\x00\x85\x13\xe2\x20\xa9\x4a"
               "\x85\x16\xa9\x01\x85\x15\xda\x5a\x20\xaf\xe2\x7a\xfa\xc2\x20\x98\x18\x69\x40\x00\xa8\xbd\x13"
               "\x00\x85\x11\xbd\x27\x00\x85\x13\xe2\x20\xa9\x4c\x85\x16\xa9\x16\x85\x15\xda\x5a\x20\xaf\xe2"
               "\x7a\xfa\xc2\x20\x98\x18\x69\x40\x00\xa8\xe2\x20\xbd\xbc\x00\x85\x11\x64\x12\xa9\xff\x85\x13"
               "\x64\x14\xa9\x4e\x85\x16\xa9\x2b\x85\x15\x20\xaf\xe2\xfa\xab\x60",
               246u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x81e645u: goto save_caller_bank;
    case 0x81e646u: goto save_party_slot;
    case 0x81e647u: goto widen_name_position;
    case 0x81e649u: goto read_name_position;
    case 0x81e64du: goto save_name_position;
    case 0x81e64eu: goto narrow_name_character;
    case 0x81e650u: goto read_member_record;
    case 0x81e653u: goto select_member_record;
    case 0x81e654u: goto restore_name_position;
    case 0x81e655u: goto test_empty_slot;
    case 0x81e658u: goto skip_empty_slot;
    case 0x81e65au: goto empty_name_position;
    case 0x81e65bu: goto widen_empty_position;
    case 0x81e65du: goto clear_empty_position_carry;
    case 0x81e65eu: goto empty_status_row;
    case 0x81e661u: goto select_empty_status_row;
    case 0x81e662u: goto narrow_empty_size;
    case 0x81e664u: goto empty_columns;
    case 0x81e666u: goto store_empty_columns;
    case 0x81e669u: goto empty_rows;
    case 0x81e66bu: goto store_empty_rows;
    case 0x81e66eu: goto empty_tile;
    case 0x81e671u: goto store_empty_tile;
    case 0x81e674u: goto draw_empty_status;
    case 0x81e677u: goto finish_empty_slot;
    case 0x81e67au: goto save_member_record;
    case 0x81e67bu: goto save_name_target;
    case 0x81e67cu: goto member_data_bank;
    case 0x81e67eu: goto save_member_bank;
    case 0x81e67fu: goto select_member_bank;
    case 0x81e680u: goto name_length;
    case 0x81e682u: goto store_name_length;
    case 0x81e684u: goto read_name_character;
    case 0x81e687u: goto translate_name_character;
    case 0x81e68au: goto test_character_top;
    case 0x81e68cu: goto skip_character_top;
    case 0x81e68eu: goto set_character_carry;
    case 0x81e68fu: goto adjust_character_top;
    case 0x81e691u: goto store_character_top;
    case 0x81e694u: goto next_name_character;
    case 0x81e695u: goto next_name_tile_low;
    case 0x81e696u: goto next_name_tile_high;
    case 0x81e697u: goto count_name_character;
    case 0x81e699u: goto repeat_name_characters;
    case 0x81e69bu: goto restore_name_target;
    case 0x81e69cu: goto restore_member_record;
    case 0x81e69du: goto widen_symbol_position;
    case 0x81e69fu: goto name_target_position;
    case 0x81e6a0u: goto clear_symbol_position_carry;
    case 0x81e6a1u: goto symbol_row_position;
    case 0x81e6a4u: goto select_symbol_row;
    case 0x81e6a5u: goto save_symbol_record;
    case 0x81e6a6u: goto save_symbol_target;
    case 0x81e6a7u: goto read_saved_party_slot;
    case 0x81e6a9u: goto select_saved_party_slot;
    case 0x81e6aau: goto read_symbol_base;
    case 0x81e6adu: goto select_symbol_base;
    case 0x81e6aeu: goto narrow_symbol_tile;
    case 0x81e6b0u: goto symbol_count;
    case 0x81e6b2u: goto store_symbol_count;
    case 0x81e6b4u: goto symbol_tile;
    case 0x81e6b5u: goto store_symbol_tile;
    case 0x81e6b8u: goto symbol_attributes;
    case 0x81e6bau: goto store_symbol_attributes;
    case 0x81e6bdu: goto next_symbol_tile;
    case 0x81e6beu: goto next_symbol_target_low;
    case 0x81e6bfu: goto next_symbol_target_high;
    case 0x81e6c0u: goto count_symbol_tile;
    case 0x81e6c2u: goto repeat_symbol_tiles;
    case 0x81e6c4u: goto symbol_tail_tile;
    case 0x81e6c6u: goto store_symbol_tail;
    case 0x81e6c9u: goto symbol_tail_attributes;
    case 0x81e6cbu: goto store_symbol_tail_attributes;
    case 0x81e6ceu: goto restore_symbol_target;
    case 0x81e6cfu: goto restore_symbol_record;
    case 0x81e6d0u: goto widen_hp_position;
    case 0x81e6d2u: goto hp_source_position;
    case 0x81e6d3u: goto clear_hp_position_carry;
    case 0x81e6d4u: goto hp_target_position;
    case 0x81e6d7u: goto select_hp_target;
    case 0x81e6d8u: goto read_current_hp;
    case 0x81e6dbu: goto store_current_hp;
    case 0x81e6ddu: goto read_maximum_hp;
    case 0x81e6e0u: goto store_maximum_hp;
    case 0x81e6e2u: goto narrow_hp_tiles;
    case 0x81e6e4u: goto hp_tile;
    case 0x81e6e6u: goto store_hp_tile;
    case 0x81e6e8u: goto hp_offset;
    case 0x81e6eau: goto store_hp_offset;
    case 0x81e6ecu: goto save_hp_record;
    case 0x81e6edu: goto save_hp_target;
    case 0x81e6eeu: goto draw_hp_gauge;
    case 0x81e6f1u: goto restore_hp_target;
    case 0x81e6f2u: goto restore_hp_record;
    case 0x81e6f3u: goto widen_mp_position;
    case 0x81e6f5u: goto mp_source_position;
    case 0x81e6f6u: goto clear_mp_position_carry;
    case 0x81e6f7u: goto mp_target_position;
    case 0x81e6fau: goto select_mp_target;
    case 0x81e6fbu: goto read_current_mp;
    case 0x81e6feu: goto store_current_mp;
    case 0x81e700u: goto read_maximum_mp;
    case 0x81e703u: goto store_maximum_mp;
    case 0x81e705u: goto narrow_mp_tiles;
    case 0x81e707u: goto mp_tile;
    case 0x81e709u: goto store_mp_tile;
    case 0x81e70bu: goto mp_offset;
    case 0x81e70du: goto store_mp_offset;
    case 0x81e70fu: goto save_mp_record;
    case 0x81e710u: goto save_mp_target;
    case 0x81e711u: goto draw_mp_gauge;
    case 0x81e714u: goto restore_mp_target;
    case 0x81e715u: goto restore_mp_record;
    case 0x81e716u: goto widen_ip_position;
    case 0x81e718u: goto ip_source_position;
    case 0x81e719u: goto clear_ip_position_carry;
    case 0x81e71au: goto ip_target_position;
    case 0x81e71du: goto select_ip_target;
    case 0x81e71eu: goto narrow_ip_value;
    case 0x81e720u: goto read_current_ip;
    case 0x81e723u: goto store_current_ip;
    case 0x81e725u: goto clear_current_ip_high;
    case 0x81e727u: goto maximum_ip;
    case 0x81e729u: goto store_maximum_ip;
    case 0x81e72bu: goto clear_maximum_ip_high;
    case 0x81e72du: goto ip_tile;
    case 0x81e72fu: goto store_ip_tile;
    case 0x81e731u: goto ip_offset;
    case 0x81e733u: goto store_ip_offset;
    case 0x81e735u: goto draw_ip_gauge;
    case 0x81e738u: goto restore_party_slot;
    case 0x81e739u: goto restore_caller_bank;
    case 0x81e73au: goto return_status_row;
    default: return 0;
    }
save_caller_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e645u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe646u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_party_slot: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e646u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe647u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_name_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e647u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe649u);
}
read_name_position: {
    const uint32_t address = (0x818819u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e649u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe64du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_name_position: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e64du, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe64eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_name_character: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e64eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe650u);
}
read_member_record: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0a64u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0a64u >> 8) != ((0x0a64u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e650u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe653u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_member_record: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e653u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe654u);
}
restore_name_position: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e654u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe655u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_empty_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e655u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe658u);
}
skip_empty_slot: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e658u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe67au : 0xe65au);
    if (taken)
        goto finished;
}
empty_name_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e65au, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->y), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe65bu);
}
widen_empty_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e65bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe65du);
}
clear_empty_position_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e65du, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe65eu);
}
empty_status_row: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e65eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe661u);
}
select_empty_status_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e661u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe662u);
}
narrow_empty_size: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e662u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe664u);
}
empty_columns: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e664u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0006u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe666u);
}
store_empty_columns: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09f2u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e666u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe669u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
empty_rows: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e669u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe66bu);
}
store_empty_rows: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09f3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e66bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe66eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
empty_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e66eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x2150u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe671u);
}
store_empty_tile: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09f6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e671u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe674u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_empty_status: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e674u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x81e674u, 0x20u, 0x81e7d2u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finish_empty_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e677u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe738u);
    goto finished;
}
save_member_record: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e67au, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe67bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_name_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e67bu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe67cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
member_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e67cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe67eu);
}
save_member_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e67eu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe67fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_member_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e67fu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xe680u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
name_length: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e680u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0005u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe682u);
}
store_name_length: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e682u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe684u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_name_character: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0000u >> 8) != ((0x0000u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e684u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe687u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
translate_name_character: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e687u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x81e687u, 0x20u, 0x81e835u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
test_character_top: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e68au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe68cu);
}
skip_character_top: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e68cu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe694u : 0xe68eu);
    if (taken)
        goto next_name_character;
}
set_character_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e68eu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe68fu);
}
adjust_character_top: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e68fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0065u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe691u);
}
store_character_top: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e691u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe694u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_name_character: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e694u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe695u);
}
next_name_tile_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e695u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe696u);
}
next_name_tile_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e696u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe697u);
}
count_name_character: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e697u, 2u, span.instructions != 0u);
    const uint8_t updated = (uint8_t)(registers->read(registers->mem, address) - 1u);
    registers->write(registers->mem, address, updated);
    registers->z = updated == 0u;
    registers->n = (updated & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xe699u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_name_characters: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e699u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe684u : 0xe69bu);
    if (taken)
        goto finished;
}
restore_name_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e69bu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe69cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_member_record: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e69cu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe69du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_symbol_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e69du, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe69fu);
}
name_target_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e69fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6a0u);
}
clear_symbol_position_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a0u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6a1u);
}
symbol_row_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6a4u);
}
select_symbol_row: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6a5u);
}
save_symbol_record: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a5u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe6a6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_symbol_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a6u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe6a7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_saved_party_slot: {
    const uint32_t address = (uint16_t)(registers->sp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6a9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_saved_party_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6a9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6aau);
}
read_symbol_base: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1be0u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x1be0u >> 8) != ((0x1be0u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6aau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6adu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_symbol_base: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6adu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6aeu);
}
narrow_symbol_tile: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6aeu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6b0u);
}
symbol_count: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6b0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0005u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6b2u);
}
store_symbol_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1bu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6b2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6b4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
symbol_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6b4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->x), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6b5u);
}
store_symbol_tile: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6b5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6b8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
symbol_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6b8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0021u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6bau);
}
store_symbol_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6bau, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6bdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_symbol_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6bdu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6beu);
}
next_symbol_target_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6beu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6bfu);
}
next_symbol_target_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6bfu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6c0u);
}
count_symbol_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x1bu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6c0u, 2u, span.instructions != 0u);
    const uint8_t updated = (uint8_t)(registers->read(registers->mem, address) - 1u);
    registers->write(registers->mem, address, updated);
    registers->z = updated == 0u;
    registers->n = (updated & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6c2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_symbol_tiles: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6c2u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xe6b4u : 0xe6c4u);
    if (taken)
        goto finished;
}
symbol_tail_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6c4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0050u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6c6u);
}
store_symbol_tail: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6c6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6c9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
symbol_tail_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6c9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0021u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6cbu);
}
store_symbol_tail_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6cbu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_symbol_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6ceu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe6cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_symbol_record: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6cfu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe6d0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_hp_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6d0u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6d2u);
}
hp_source_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6d2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6d3u);
}
clear_hp_position_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6d3u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6d4u);
}
hp_target_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6d4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6d7u);
}
select_hp_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6d7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6d8u);
}
read_current_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0011u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0011u >> 8) != ((0x0011u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6d8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6dbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_current_hp: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6dbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6ddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_maximum_hp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0025u >> 8) != ((0x0025u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6ddu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6e0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_maximum_hp: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6e0u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6e2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_hp_tiles: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6e2u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6e4u);
}
hp_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6e4u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x004au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6e6u);
}
store_hp_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x16u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6e6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6e8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
hp_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6e8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6eau);
}
store_hp_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6eau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6ecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_hp_record: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6ecu, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe6edu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_hp_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6edu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe6eeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_hp_gauge: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e6eeu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x81e6eeu, 0x20u, 0x81e2afu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_hp_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6f1u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe6f2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_hp_record: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6f2u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe6f3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_mp_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6f3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6f5u);
}
mp_source_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6f5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6f6u);
}
clear_mp_position_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6f6u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6f7u);
}
mp_target_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6f7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe6fau);
}
select_mp_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6fau, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe6fbu);
}
read_current_mp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0013u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0013u >> 8) != ((0x0013u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6fbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe6feu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_current_mp: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e6feu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe700u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_maximum_mp: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0027u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0027u >> 8) != ((0x0027u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e700u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe703u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_maximum_mp: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e703u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe705u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_mp_tiles: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e705u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe707u);
}
mp_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e707u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x004cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe709u);
}
store_mp_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x16u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e709u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe70bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mp_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e70bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0016u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe70du);
}
store_mp_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e70du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe70fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_mp_record: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e70fu, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe710u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_mp_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e710u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0xe711u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_mp_gauge: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e711u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x81e711u, 0x20u, 0x81e2afu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_mp_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e714u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe715u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_mp_record: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e715u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe716u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_ip_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e716u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe718u);
}
ip_source_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e718u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe719u);
}
clear_ip_position_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e719u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xe71au);
}
ip_target_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e71au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0040u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe71du);
}
select_ip_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e71du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe71eu);
}
narrow_ip_value: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e71eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xe720u);
}
read_current_ip: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x00bcu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x00bcu >> 8) != ((0x00bcu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e720u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe723u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_current_ip: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e723u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe725u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_current_ip_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e725u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe727u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
maximum_ip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e727u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe729u);
}
store_maximum_ip: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e729u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe72bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_maximum_ip_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0x14u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e72bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe72du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
ip_tile: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e72du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x004eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe72fu);
}
store_ip_tile: {
    const uint32_t address = (uint16_t)(registers->dp + 0x16u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e72fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe731u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
ip_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e731u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x002bu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xe733u);
}
store_ip_offset: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x81e733u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xe735u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
draw_ip_gauge: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e735u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x81e735u, 0x20u, 0x81e2afu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_party_slot: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e738u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xe739u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_caller_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81e739u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xe73au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_status_row: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81e73au);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81e73au, 0x60u, false, true, true);
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

static int Lufia2DecompRunBattlePartyStatusSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                unsigned step_budget, long *step_count) {
    if (pc < 0x81e645u)
        return Lufia2DecompRunBattlePartyRowsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                  step_budget, step_count);
    return Lufia2DecompRunBattlePartyStatusSpanBody(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
                                                    step_budget, step_count);
}
