#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldOamParentSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                             uint64_t dynamic_epoch, uint64_t write_epoch,
                                             unsigned step_budget, long *step_count) {
    uint32_t first_pc, end_pc;
    if (pc >= 0x83a321u && pc < 0x83a365u) {
        first_pc = 0x83a321u;
        end_pc = 0x83a365u;
    } else if (pc >= 0x83a365u && pc < 0x83a369u) {
        first_pc = 0x83a365u;
        end_pc = 0x83a369u;
    } else if (pc >= 0x83a369u && pc < 0x83a36bu) {
        first_pc = 0x83a369u;
        end_pc = 0x83a36bu;
    } else if (pc >= 0x83a36bu && pc < 0x83a36eu) {
        first_pc = 0x83a36bu;
        end_pc = 0x83a36eu;
    } else if (pc >= 0x83a36eu && pc < 0x83a371u) {
        first_pc = 0x83a36eu;
        end_pc = 0x83a371u;
    } else if (pc >= 0x83a371u && pc < 0x83a375u) {
        first_pc = 0x83a371u;
        end_pc = 0x83a375u;
    } else if (pc >= 0x83a375u && pc < 0x83a377u) {
        first_pc = 0x83a375u;
        end_pc = 0x83a377u;
    } else if (pc >= 0x83a377u && pc < 0x83a37au) {
        first_pc = 0x83a377u;
        end_pc = 0x83a37au;
    } else if (pc >= 0x83a37au && pc < 0x83a39eu) {
        first_pc = 0x83a37au;
        end_pc = 0x83a39eu;
    } else if (pc >= 0x83a39eu && pc < 0x83a3bbu) {
        first_pc = 0x83a39eu;
        end_pc = 0x83a3bbu;
    } else if (pc >= 0x83a3bbu && pc < 0x83a3d0u) {
        first_pc = 0x83a3bbu;
        end_pc = 0x83a3d0u;
    } else if (pc >= 0x83a3d0u && pc < 0x83a3d3u) {
        first_pc = 0x83a3d0u;
        end_pc = 0x83a3d3u;
    } else if (pc >= 0x83a3d3u && pc < 0x83a401u) {
        first_pc = 0x83a3d3u;
        end_pc = 0x83a401u;
    } else if (pc >= 0x83a401u && pc < 0x83a44bu) {
        first_pc = 0x83a401u;
        end_pc = 0x83a44bu;
    } else if (pc >= 0x83a44bu && pc < 0x83a453u) {
        first_pc = 0x83a44bu;
        end_pc = 0x83a453u;
    } else if (pc >= 0x83a453u && pc < 0x83a461u) {
        first_pc = 0x83a453u;
        end_pc = 0x83a461u;
    } else if (pc >= 0x83a461u && pc < 0x83a464u) {
        first_pc = 0x83a461u;
        end_pc = 0x83a464u;
    } else if (pc >= 0x83a464u && pc < 0x83a473u) {
        first_pc = 0x83a464u;
        end_pc = 0x83a473u;
    } else if (pc >= 0x83a473u && pc < 0x83a488u) {
        first_pc = 0x83a473u;
        end_pc = 0x83a488u;
    } else if (pc >= 0x83a488u && pc < 0x83a48au) {
        first_pc = 0x83a488u;
        end_pc = 0x83a48au;
    } else
        return 0;
    const bool byte_accumulator =
        !((pc >= 0x83a37cu && pc <= 0x83a39eu) || (pc >= 0x83a3d5u && pc <= 0x83a401u) ||
          (pc >= 0x83a44du && pc <= 0x83a453u));
    const bool byte_index = !(pc >= 0x83a44du && pc <= 0x83a464u);
    const bool changes_accumulator = pc == 0x83a325u || pc == 0x83a37au || pc == 0x83a39eu ||
                                     pc == 0x83a3d3u || pc == 0x83a401u || pc == 0x83a44bu ||
                                     pc == 0x83a453u || pc == 0x83a464u || pc == 0x83a473u;
    const bool changes_index =
        pc == 0x83a39eu || pc == 0x83a401u || pc == 0x83a44bu || pc == 0x83a464u || pc == 0x83a473u;
    if (!cpu || !cpu->ram || registers->e || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        (pc < 0x83a488u && ((!changes_accumulator && registers->mf != byte_accumulator) ||
                            (!changes_index && registers->xf != byte_index))) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83a321u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 369u ||
        memcmp(cart->rom + first,
               "\x84\x60\x4b\xab\xe2\x20\xa5\x60\xd0\x03\x82\x45\x01\x64\xaa\x64\xac\xa9\x10\x85\x5f\xa2\x00"
               "\x86\x5d\xbf\x00\xe3\x7e\x85\xa7\xaa\x0a\x85\xa9\x18\x65\xa7\x85\xab\x64\x97\xe0\x28\x90\x6c"
               "\xad\x32\x07\xc9\x10\xb0\x24\x7b\xbf\x7e\xe3\x7f\xc9\xff\xf0\x18\xda\x18\x69\x28\x85\xa7\x22"
               "\x4f\xab\x83\xa6\xa7\x20\x92\xa4\xfa\x86\xa7\x22\x4f\xab\x83\x80\x03\x20\x92\xa4\xc2\x20\xa6"
               "\xa9\xbf\xae\xdd\x7f\x18\x7f\x8c\xdc\x7f\x38\xe5\x9f\x38\xe9\x08\x00\x85\x8f\xbf\x3e\xde\x7f"
               "\x18\x7f\x1c\xdd\x7f\x18\xe5\xa1\x85\x91\xe2\x30\xa6\xa7\xbf\xce\xe1\x7f\x0a\x04\x97\x7b\xbd"
               "\x22\x06\x29\x03\x4a\xeb\x6a\xeb\x4a\xeb\x6a\x04\x97\x82\x66\x00\xbd\x6a\x06\xdd\x71\x14\xf0"
               "\x10\xad\x32\x07\xc9\x0a\xb0\x09\xbd\x6a\x06\x9d\x71\x14\x20\xe5\xaa\xc2\x20\xa6\xa9\xbf\xae"
               "\xdd\x7f\x18\x7f\x8c\xdc\x7f\x38\xe5\x9f\x38\xe9\x08\x00\x38\xef\x81\xd0\x7f\x85\x8f\xbf\x3e"
               "\xde\x7f\x18\x7f\x1c\xdd\x7f\x38\xe5\xa1\x18\xef\x83\xd0\x7f\x85\x91\xe2\x30\xa6\xa7\xbf\xce"
               "\xe1\x7f\x29\x0e\x04\x97\xbd\x91\x12\x29\x18\xf0\x0d\xbd\x6a\x06\x29\x07\xc9\x06\x90\x04\xa9"
               "\x40\x04\x97\xa6\xa7\xbf\x16\xe3\x7f\x29\x30\x04\x97\xbf\xa6\xe2\x7f\x04\x97\xbf\x5e\xe2\x7f"
               "\x85\x9d\xbf\x16\xe2\x7f\xa8\xa5\x5f\x85\x94\x18\x79\xf4\xab\x85\x5f\x7b\xa5\x94\x85\x58\xc2"
               "\x30\x0a\x0a\x69\x00\x01\xa8\xe2\x20\x7b\xbf\x16\xe2\x7f\xc9\x04\x90\x01\x7b\x0a\xaa\xfc\x8a"
               "\xa4\xe2\x30\xa6\x5d\xe8\xe8\x86\x5d\xe4\x60\xf0\x03\x82\xc7\xfe\xe2\x30\xa5\x5f\x38\xe9\x11"
               "\x8f\xb0\xd0\x7f\xa5\x72\x89\x01\xd0\x04\xa9\x80\x85\x72\xab\x6b",
               361u) ||
        memcmp(cart->rom + first + 361u, "\xa2\xa4\xbf\xa4\x01\xa5\x34\xa5", 8u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a321u: goto store_visible_count;
    case 0x83a323u: goto push_program_bank;
    case 0x83a324u: goto select_program_bank;
    case 0x83a325u: goto narrow_visible_count;
    case 0x83a327u: goto load_visible_count;
    case 0x83a329u: goto has_visible_actors;
    case 0x83a32bu: goto jump_cleanup;
    case 0x83a32eu: goto clear_word_offset_high;
    case 0x83a330u: goto clear_record_offset_high;
    case 0x83a332u: goto first_oam_entry;
    case 0x83a334u: goto store_next_entry;
    case 0x83a336u: goto first_list_index;
    case 0x83a338u: goto store_list_index;
    case 0x83a33au: goto load_sorted_actor;
    case 0x83a33eu: goto save_actor_slot;
    case 0x83a340u: goto index_actor_slot;
    case 0x83a341u: goto double_slot_offset;
    case 0x83a342u: goto save_word_offset;
    case 0x83a344u: goto clear_record_carry;
    case 0x83a345u: goto add_record_slot;
    case 0x83a347u: goto save_record_offset;
    case 0x83a349u: goto clear_attributes;
    case 0x83a34bu: goto compare_object_slot;
    case 0x83a34du: goto choose_regular_actor;
    case 0x83a34fu: goto load_object_upload_cursor;
    case 0x83a352u: goto compare_object_upload_room;
    case 0x83a354u: goto object_upload_ready;
    case 0x83a356u: goto seed_linked_actor_high;
    case 0x83a357u: goto read_linked_actor;
    case 0x83a35bu: goto compare_no_link;
    case 0x83a35du: goto choose_unlinked_reload;
    case 0x83a35fu: goto push_object_slot;
    case 0x83a360u: goto clear_link_carry;
    case 0x83a361u: goto add_object_slot_base;
    case 0x83a363u: goto set_linked_slot;
    case 0x83a365u: goto set_linked_offsets;
    case 0x83a369u: goto index_linked_slot;
    case 0x83a36bu: goto reload_linked_object;
    case 0x83a36eu: goto pop_object_slot;
    case 0x83a36fu: goto restore_object_slot;
    case 0x83a371u: goto restore_object_offsets;
    case 0x83a375u: goto object_position_ready;
    case 0x83a377u: goto reload_unlinked_object;
    case 0x83a37au: goto widen_object_position;
    case 0x83a37cu: goto load_object_word_offset;
    case 0x83a37eu: goto read_object_x;
    case 0x83a382u: goto clear_object_x_carry;
    case 0x83a383u: goto add_object_x_offset;
    case 0x83a387u: goto set_object_camera_carry;
    case 0x83a388u: goto subtract_object_camera_x;
    case 0x83a38au: goto set_object_half_width_carry;
    case 0x83a38bu: goto subtract_object_half_width;
    case 0x83a38eu: goto save_object_x;
    case 0x83a390u: goto read_object_y;
    case 0x83a394u: goto clear_object_y_carry;
    case 0x83a395u: goto add_object_y_offset;
    case 0x83a399u: goto clear_object_camera_y_carry;
    case 0x83a39au: goto subtract_object_camera_y;
    case 0x83a39cu: goto save_object_y;
    case 0x83a39eu: goto narrow_object_flags;
    case 0x83a3a0u: goto load_object_slot;
    case 0x83a3a2u: goto read_object_palette;
    case 0x83a3a6u: goto shift_object_palette;
    case 0x83a3a7u: goto merge_object_palette;
    case 0x83a3a9u: goto seed_flip_high;
    case 0x83a3aau: goto read_object_state;
    case 0x83a3adu: goto mask_flip_bits;
    case 0x83a3afu: goto shift_horizontal_flip;
    case 0x83a3b0u: goto exchange_flip_low;
    case 0x83a3b1u: goto rotate_horizontal_flip;
    case 0x83a3b2u: goto exchange_flip_high;
    case 0x83a3b3u: goto shift_vertical_flip;
    case 0x83a3b4u: goto exchange_vertical_flip;
    case 0x83a3b5u: goto rotate_vertical_flip;
    case 0x83a3b6u: goto merge_flip_bits;
    case 0x83a3b8u: goto join_actor_attributes;
    case 0x83a3bbu: goto read_actor_frame;
    case 0x83a3beu: goto compare_uploaded_frame;
    case 0x83a3c1u: goto frame_is_current;
    case 0x83a3c3u: goto read_actor_upload_cursor;
    case 0x83a3c6u: goto compare_actor_upload_room;
    case 0x83a3c8u: goto skip_actor_upload;
    case 0x83a3cau: goto reload_actor_frame;
    case 0x83a3cdu: goto save_uploaded_frame;
    case 0x83a3d0u: goto queue_actor_frame;
    case 0x83a3d3u: goto widen_actor_position;
    case 0x83a3d5u: goto load_actor_word_offset;
    case 0x83a3d7u: goto read_actor_x;
    case 0x83a3dbu: goto clear_actor_x_carry;
    case 0x83a3dcu: goto add_actor_x_offset;
    case 0x83a3e0u: goto set_actor_camera_carry;
    case 0x83a3e1u: goto subtract_actor_camera_x;
    case 0x83a3e3u: goto set_actor_half_width_carry;
    case 0x83a3e4u: goto subtract_actor_half_width;
    case 0x83a3e7u: goto set_screen_x_carry;
    case 0x83a3e8u: goto subtract_screen_x;
    case 0x83a3ecu: goto save_actor_x;
    case 0x83a3eeu: goto read_actor_y;
    case 0x83a3f2u: goto clear_actor_y_carry;
    case 0x83a3f3u: goto add_actor_y_offset;
    case 0x83a3f7u: goto set_actor_camera_y_carry;
    case 0x83a3f8u: goto subtract_actor_camera_y;
    case 0x83a3fau: goto clear_screen_y_carry;
    case 0x83a3fbu: goto subtract_screen_y;
    case 0x83a3ffu: goto save_actor_y;
    case 0x83a401u: goto narrow_actor_attributes;
    case 0x83a403u: goto load_attribute_slot;
    case 0x83a405u: goto read_palette_bits;
    case 0x83a409u: goto mask_palette_bits;
    case 0x83a40bu: goto merge_palette_bits;
    case 0x83a40du: goto read_actor_priority;
    case 0x83a410u: goto mask_priority_flags;
    case 0x83a412u: goto has_priority_override;
    case 0x83a414u: goto read_priority_frame;
    case 0x83a417u: goto mask_priority_frame;
    case 0x83a419u: goto compare_priority_frame;
    case 0x83a41bu: goto skip_priority_bit;
    case 0x83a41du: goto load_priority_bit;
    case 0x83a41fu: goto merge_priority_bit;
    case 0x83a421u: goto load_render_slot;
    case 0x83a423u: goto read_page_flags;
    case 0x83a427u: goto mask_page_flags;
    case 0x83a429u: goto merge_page_flags;
    case 0x83a42bu: goto read_animation_flags;
    case 0x83a42fu: goto merge_animation_flags;
    case 0x83a431u: goto read_tile_index;
    case 0x83a435u: goto save_tile_index;
    case 0x83a437u: goto read_sprite_shape;
    case 0x83a43bu: goto index_sprite_shape;
    case 0x83a43cu: goto read_next_entry;
    case 0x83a43eu: goto save_first_entry;
    case 0x83a440u: goto clear_entry_count_carry;
    case 0x83a441u: goto add_shape_entry_count;
    case 0x83a444u: goto save_next_entry;
    case 0x83a446u: goto seed_entry_high;
    case 0x83a447u: goto read_first_entry;
    case 0x83a449u: goto save_high_bit_index;
    case 0x83a44bu: goto widen_oam_index;
    case 0x83a44du: goto shift_entry_once;
    case 0x83a44eu: goto shift_entry_twice;
    case 0x83a44fu: goto add_oam_base;
    case 0x83a452u: goto set_oam_output;
    case 0x83a453u: goto narrow_shape_selector;
    case 0x83a455u: goto seed_shape_high;
    case 0x83a456u: goto reload_sprite_shape;
    case 0x83a45au: goto compare_shape_limit;
    case 0x83a45cu: goto shape_is_valid;
    case 0x83a45eu: goto select_default_shape;
    case 0x83a45fu: goto double_shape_index;
    case 0x83a460u: goto set_shape_table_index;
    case 0x83a461u: goto draw_sprite_shape;
    case 0x83a464u: goto narrow_list_index;
    case 0x83a466u: goto reload_list_index;
    case 0x83a468u: goto advance_list_first;
    case 0x83a469u: goto advance_list_second;
    case 0x83a46au: goto save_list_index;
    case 0x83a46cu: goto compare_visible_count;
    case 0x83a46eu: goto finished_visible_actors;
    case 0x83a470u: goto repeat_visible_actor;
    case 0x83a473u: goto narrow_cleanup;
    case 0x83a475u: goto read_final_entry;
    case 0x83a477u: goto set_usage_carry;
    case 0x83a478u: goto subtract_reserved_entries;
    case 0x83a47au: goto publish_sprite_usage;
    case 0x83a47eu: goto read_oam_request;
    case 0x83a480u: goto test_oam_request;
    case 0x83a482u: goto preserve_oam_request;
    case 0x83a484u: goto load_oam_request;
    case 0x83a486u: goto store_oam_request;
    case 0x83a488u: goto restore_data_bank;
    case 0x83a489u: goto return_long;
    default: return 0;
    }
store_visible_count: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a321u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->y, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa323u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
push_program_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a323u, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp, (uint8_t)registers->k);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa324u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_program_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a324u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xa325u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_visible_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a325u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa327u);
}
load_visible_count: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a327u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa329u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
has_visible_actors: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a329u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa32eu : 0xa32bu);
    if (taken) {
        if (0x83a32eu < first_pc || 0x83a32eu >= end_pc)
            goto finished;
        goto clear_word_offset_high;
    }
}
jump_cleanup: {
    const bool taken = true;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a32bu, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa473u : 0xa32eu);
    if (taken) {
        if (0x83a473u < first_pc || 0x83a473u >= end_pc)
            goto finished;
        goto narrow_cleanup;
    }
}
clear_word_offset_high: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xaau);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a32eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa330u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_record_offset_high: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xacu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a330u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa332u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_oam_entry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a332u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa334u);
}
store_next_entry: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a334u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa336u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_list_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a336u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa338u);
}
store_list_index: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a338u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa33au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_sorted_actor: {
    const bool wide = false;
    const uint32_t address = (0x7ee300u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a33au, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa33eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_actor_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a33eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa340u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_actor_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a340u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)registers->a, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa341u);
}
double_slot_offset: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a341u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x0080u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa342u);
}
save_word_offset: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a342u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa344u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_record_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a344u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa345u);
}
add_record_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a345u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa347u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_record_offset: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xabu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a347u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa349u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_attributes: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a349u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa34bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_object_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a34bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x0028u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa34du);
}
choose_regular_actor: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a34du, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa3bbu : 0xa34fu);
    if (taken) {
        if (0x83a3bbu < first_pc || 0x83a3bbu >= end_pc)
            goto finished;
        goto read_actor_frame;
    }
}
load_object_upload_cursor: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a34fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa352u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_object_upload_room: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a352u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa354u);
}
object_upload_ready: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a354u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa37au : 0xa356u);
    if (taken) {
        if (0x83a37au < first_pc || 0x83a37au >= end_pc)
            goto finished;
        goto widen_object_position;
    }
}
seed_linked_actor_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a356u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa357u);
}
read_linked_actor: {
    const bool wide = false;
    const uint32_t address = (0x7fe37eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a357u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa35bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_no_link: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a35bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa35du);
}
choose_unlinked_reload: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a35du, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa377u : 0xa35fu);
    if (taken) {
        if (0x83a377u < first_pc || 0x83a377u >= end_pc)
            goto finished;
        goto reload_unlinked_object;
    }
}
push_object_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a35fu, 1u, span.instructions != 0u);
    registers->write(registers->mem, registers->sp, (uint8_t)registers->x);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa360u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_link_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a360u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa361u);
}
add_object_slot_base: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a361u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0028u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa363u);
}
set_linked_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a363u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa365u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
set_linked_offsets: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a365u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a365u, 0x22u, 0x83ab4fu, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
index_linked_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a369u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa36bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
reload_linked_object: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a36bu);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a36bu, 0x20u, 0x83a492u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
pop_object_slot: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a36eu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    Lufia2DecompApplyOperand(registers, 0xa2u, value, true);
    NativeSpanStep(&span, registers, cpu, 4u, 0xa36fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_object_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a36fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa371u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_object_offsets: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a371u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a371u, 0x22u, 0x83ab4fu, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
object_position_ready: {
    const bool taken = true;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a375u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa37au : 0xa377u);
    if (taken) {
        if (0x83a37au < first_pc || 0x83a37au >= end_pc)
            goto finished;
        goto widen_object_position;
    }
    goto finished;
}
reload_unlinked_object: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a377u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a377u, 0x20u, 0x83a492u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_object_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a37au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa37cu);
}
load_object_word_offset: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a37cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa37eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_x: {
    const bool wide = true;
    const uint32_t address = (0x7fddaeu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a37eu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa382u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_object_x_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a382u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa383u);
}
add_object_x_offset: {
    const bool wide = true;
    const uint32_t address = (0x7fdc8cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a383u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa387u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_object_camera_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a387u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa388u);
}
subtract_object_camera_x: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x9fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a388u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa38au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_object_half_width_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a38au, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa38bu);
}
subtract_object_half_width: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a38bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0008u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa38eu);
}
save_object_x: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a38eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa390u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_y: {
    const bool wide = true;
    const uint32_t address = (0x7fde3eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a390u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa394u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_object_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a394u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa395u);
}
add_object_y_offset: {
    const bool wide = true;
    const uint32_t address = (0x7fdd1cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a395u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa399u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_object_camera_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a399u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa39au);
}
subtract_object_camera_y: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0xa1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a39au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa39cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_object_y: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a39cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa39eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
narrow_object_flags: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a39eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa3a0u);
}
load_object_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3a0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3a2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_object_palette: {
    const bool wide = false;
    const uint32_t address = (0x7fe1ceu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3a2u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3a6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
shift_object_palette: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3a6u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x0080u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3a7u);
}
merge_object_palette: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3a7u, 2u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value | (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3a9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
seed_flip_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3a9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3aau);
}
read_object_state: {
    const bool wide = false;
    const bool crossed = (0x0622u >> 8) != ((0x0622u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0622u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3aau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3adu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_flip_bits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3adu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0003u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3afu);
}
shift_horizontal_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3afu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)((registers->a & 0x00ffu) >> 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3b0u);
}
exchange_flip_low: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b0u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa3b1u);
}
rotate_horizontal_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b1u, 1u, span.instructions != 0u);
    const bool carry = registers->c;
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             (uint8_t)(((registers->a & 0x00ffu) >> 1) | (carry ? 0x0080u : 0u)), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3b2u);
}
exchange_flip_high: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b2u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa3b3u);
}
shift_vertical_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b3u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)((registers->a & 0x00ffu) >> 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3b4u);
}
exchange_vertical_flip: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b4u, 1u, span.instructions != 0u);
    registers->a = (uint16_t)((registers->a >> 8) | (registers->a << 8));
    registers->z = (registers->a & 0xffu) == 0u;
    registers->n = (registers->a & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 3u, 0xa3b5u);
}
rotate_vertical_flip: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b5u, 1u, span.instructions != 0u);
    const bool carry = registers->c;
    registers->c = (registers->a & 1u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             (uint8_t)(((registers->a & 0x00ffu) >> 1) | (carry ? 0x0080u : 0u)), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3b6u);
}
merge_flip_bits: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b6u, 2u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value | (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3b8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
join_actor_attributes: {
    const bool taken = true;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3b8u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa421u : 0xa3bbu);
    if (taken) {
        if (0x83a421u < first_pc || 0x83a421u >= end_pc)
            goto finished;
        goto load_render_slot;
    }
    goto finished;
}
read_actor_frame: {
    const bool wide = false;
    const bool crossed = (0x066au >> 8) != ((0x066au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x066au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3bbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3beu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_uploaded_frame: {
    const bool wide = false;
    const bool crossed = (0x1471u >> 8) != ((0x1471u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1471u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3beu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3c1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
frame_is_current: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3c1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa3d3u : 0xa3c3u);
    if (taken) {
        if (0x83a3d3u < first_pc || 0x83a3d3u >= end_pc)
            goto finished;
        goto widen_actor_position;
    }
}
read_actor_upload_cursor: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0732u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3c3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3c6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_actor_upload_room: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3c6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x000au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3c8u);
}
skip_actor_upload: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3c8u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa3d3u : 0xa3cau);
    if (taken) {
        if (0x83a3d3u < first_pc || 0x83a3d3u >= end_pc)
            goto finished;
        goto widen_actor_position;
    }
}
reload_actor_frame: {
    const bool wide = false;
    const bool crossed = (0x066au >> 8) != ((0x066au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x066au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3cau, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3cdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_uploaded_frame: {
    const bool wide = false;
    const bool crossed = (0x1471u >> 8) != ((0x1471u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1471u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3cdu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3d0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
queue_actor_frame: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a3d0u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83a3d0u, 0x20u, 0x83aae5u, false,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_actor_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3d3u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa3d5u);
}
load_actor_word_offset: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa9u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3d5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3d7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_x: {
    const bool wide = true;
    const uint32_t address = (0x7fddaeu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3d7u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3dbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_actor_x_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3dbu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3dcu);
}
add_actor_x_offset: {
    const bool wide = true;
    const uint32_t address = (0x7fdc8cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3dcu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3e0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_actor_camera_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3e0u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3e1u);
}
subtract_actor_camera_x: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x9fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3e1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3e3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_actor_half_width_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3e3u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3e4u);
}
subtract_actor_half_width: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3e4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0008u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa3e7u);
}
set_screen_x_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3e7u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3e8u);
}
subtract_screen_x: {
    const bool wide = true;
    const uint32_t address = (0x7fd081u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3e8u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3ecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_actor_x: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x8fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3ecu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3eeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_y: {
    const bool wide = true;
    const uint32_t address = (0x7fde3eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3eeu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3f2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_actor_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3f2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3f3u);
}
add_actor_y_offset: {
    const bool wide = true;
    const uint32_t address = (0x7fdd1cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3f3u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3f7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_actor_camera_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3f7u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3f8u);
}
subtract_actor_camera_y: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0xa1u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3f8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3fau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_screen_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3fau, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa3fbu);
}
subtract_screen_y: {
    const bool wide = true;
    const uint32_t address = (0x7fd083u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3fbu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa3ffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_actor_y: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x91u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a3ffu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa401u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
narrow_actor_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a401u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa403u);
}
load_attribute_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a403u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa405u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_palette_bits: {
    const bool wide = false;
    const uint32_t address = (0x7fe1ceu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a405u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa409u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_palette_bits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a409u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x000eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa40bu);
}
merge_palette_bits: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a40bu, 2u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value | (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa40du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_priority: {
    const bool wide = false;
    const bool crossed = (0x1291u >> 8) != ((0x1291u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1291u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a40du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa410u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_priority_flags: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a410u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa412u);
}
has_priority_override: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a412u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa421u : 0xa414u);
    if (taken) {
        if (0x83a421u < first_pc || 0x83a421u >= end_pc)
            goto finished;
        goto load_render_slot;
    }
}
read_priority_frame: {
    const bool wide = false;
    const bool crossed = (0x066au >> 8) != ((0x066au + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x066au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a414u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa417u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_priority_frame: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a417u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0007u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa419u);
}
compare_priority_frame: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a419u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0006u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa41bu);
}
skip_priority_bit: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a41bu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa421u : 0xa41du);
    if (taken) {
        if (0x83a421u < first_pc || 0x83a421u >= end_pc)
            goto finished;
        goto load_render_slot;
    }
}
load_priority_bit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a41du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0040u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa41fu);
}
merge_priority_bit: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a41fu, 2u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value | (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa421u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_render_slot: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a421u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa423u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_page_flags: {
    const bool wide = false;
    const uint32_t address = (0x7fe316u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a423u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa427u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_page_flags: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a427u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0030u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa429u);
}
merge_page_flags: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a429u, 2u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value | (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa42bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_animation_flags: {
    const bool wide = false;
    const uint32_t address = (0x7fe2a6u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a42bu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa42fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
merge_animation_flags: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x97u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + 2u * wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a42fu, 2u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value | (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa431u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_tile_index: {
    const bool wide = false;
    const uint32_t address = (0x7fe25eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a431u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa435u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_tile_index: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x9du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a435u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa437u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_sprite_shape: {
    const bool wide = false;
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a437u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa43bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
index_sprite_shape: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a43bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)registers->a, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa43cu);
}
read_next_entry: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a43cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa43eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_first_entry: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x94u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a43eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa440u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_entry_count_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a440u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa441u);
}
add_shape_entry_count: {
    const bool wide = false;
    const bool crossed = (0xabf4u >> 8) != ((0xabf4u + registers->y) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xabf4u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a441u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa444u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_next_entry: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a444u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa446u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
seed_entry_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a446u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa447u);
}
read_first_entry: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x94u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a447u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa449u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_high_bit_index: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a449u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa44bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_oam_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a44bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa44du);
}
shift_entry_once: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a44du, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa44eu);
}
shift_entry_twice: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a44eu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa44fu);
}
add_oam_base: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a44fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0100u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa452u);
}
set_oam_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a452u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa453u);
    goto finished;
}
narrow_shape_selector: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a453u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa455u);
}
seed_shape_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a455u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa456u);
}
reload_sprite_shape: {
    const bool wide = false;
    const uint32_t address = (0x7fe216u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        (wide && !Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a456u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa45au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_shape_limit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a45au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa45cu);
}
shape_is_valid: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a45cu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa45fu : 0xa45eu);
    if (taken) {
        if (0x83a45fu < first_pc || 0x83a45fu >= end_pc)
            goto finished;
        goto double_shape_index;
    }
}
select_default_shape: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a45eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa45fu);
}
double_shape_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a45fu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x0080u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint8_t)(registers->a << 1), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa460u);
}
set_shape_table_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a460u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa461u);
    goto finished;
}
draw_sprite_shape: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu ||
        registers->x > 6u || (registers->x & 1u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a461u, 3u, span.instructions != 0u);
    const uint32_t pointer = 0x83a48au + registers->x;
    const uint16_t target = Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
    registers->write(registers->mem, registers->sp, 0xa4u);
    --registers->sp;
    registers->write(registers->mem, registers->sp, 0x63u);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, 8u, target);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
narrow_list_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a464u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa466u);
}
reload_list_index: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a466u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa468u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_list_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a468u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa469u);
}
advance_list_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a469u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa46au);
}
save_list_index: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a46au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa46cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_visible_count: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x60u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a46cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa46eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finished_visible_actors: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a46eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa473u : 0xa470u);
    if (taken) {
        if (0x83a473u < first_pc || 0x83a473u >= end_pc)
            goto finished;
        goto narrow_cleanup;
    }
}
repeat_visible_actor: {
    const bool taken = true;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a470u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa33au : 0xa473u);
    if (taken) {
        if (0x83a33au < first_pc || 0x83a33au >= end_pc)
            goto finished;
        goto load_sorted_actor;
    }
    goto finished;
}
narrow_cleanup: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a473u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa475u);
}
read_final_entry: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5fu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a475u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa477u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_usage_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a477u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa478u);
}
subtract_reserved_entries: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a478u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0011u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa47au);
}
publish_sprite_usage: {
    const bool wide = false;
    const uint32_t address = (0x7fd0b0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a47au, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa47eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_oam_request: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x72u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a47eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa480u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_oam_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a480u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa482u);
}
preserve_oam_request: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a482u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa488u : 0xa484u);
    if (taken) {
        if (0x83a488u < first_pc || 0x83a488u >= end_pc)
            goto finished;
        goto restore_data_bank;
    }
}
load_oam_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a484u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa486u);
}
store_oam_request: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x72u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        (wide && !Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a486u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa488u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_data_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83a488u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0xa489u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
return_long: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83a489u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83a489u, 0x6bu, true, registers->mf, true);
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
