static bool Lufia2DecompIsFieldDisplayWriteByte(uint32_t address, bool dma) {
    if (Lufia2DecompIsWramByte(address))
        return true;
    if (((address >> 16) & 0x7fu) >= 0x40u)
        return false;
    const uint16_t port = (uint16_t)address;
    return (port >= 0x2100u && port <= 0x2133u) || port == 0x420cu ||
           (dma && (port == 0x420bu || (port >= 0x4300u && port <= 0x430fu)));
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldRestoreSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                           uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                           long *step_count) {
    uint32_t first_pc, end_pc;
    if (pc >= 0x83b062u && pc < 0x83b077u) {
        first_pc = 0x83b062u;
        end_pc = 0x83b077u;
    } else if (pc >= 0x83b07au && pc < 0x83b083u) {
        first_pc = 0x83b07au;
        end_pc = 0x83b083u;
    } else if (pc >= 0x83b086u && pc < 0x83b0b8u) {
        first_pc = 0x83b086u;
        end_pc = 0x83b0b8u;
    } else if (pc >= 0x83b0b8u && pc < 0x83b0c7u) {
        first_pc = 0x83b0b8u;
        end_pc = 0x83b0c7u;
    } else if (pc >= 0x83b0c7u && pc < 0x83b0d9u) {
        first_pc = 0x83b0c7u;
        end_pc = 0x83b0d9u;
    } else if (pc >= 0x83b0d9u && pc < 0x83b0f3u) {
        first_pc = 0x83b0d9u;
        end_pc = 0x83b0f3u;
    } else if (pc >= 0x83b0f3u && pc < 0x83b106u) {
        first_pc = 0x83b0f3u;
        end_pc = 0x83b106u;
    } else if (pc >= 0x83b106u && pc < 0x83b10eu) {
        first_pc = 0x83b106u;
        end_pc = 0x83b10eu;
    } else if (pc >= 0x83b10eu && pc < 0x83b110u) {
        first_pc = 0x83b10eu;
        end_pc = 0x83b110u;
    } else if (pc >= 0x83b110u && pc < 0x83b113u) {
        first_pc = 0x83b110u;
        end_pc = 0x83b113u;
    } else if (pc >= 0x83b113u && pc < 0x83b116u) {
        first_pc = 0x83b113u;
        end_pc = 0x83b116u;
    } else if (pc >= 0x83b116u && pc < 0x83b131u) {
        first_pc = 0x83b116u;
        end_pc = 0x83b131u;
    } else if (pc >= 0x83b131u && pc < 0x83b154u) {
        first_pc = 0x83b131u;
        end_pc = 0x83b154u;
    } else if (pc >= 0x83b154u && pc < 0x83b158u) {
        first_pc = 0x83b154u;
        end_pc = 0x83b158u;
    } else if (pc >= 0x83b158u && pc < 0x83b15eu) {
        first_pc = 0x83b158u;
        end_pc = 0x83b15eu;
    } else if (pc >= 0x8eba81u && pc < 0x8ebae7u) {
        first_pc = 0x8eba81u;
        end_pc = 0x8ebae7u;
    } else
        return 0;
    const bool dynamic_widths = (pc <= 0x83b067u || (pc >= 0x83b158u && pc <= 0x83b15du) || pc == 0x8eba81u ||
                                 pc == 0x8eba82u || pc >= 0x8ebae5u);
    const bool byte_accumulator =
        !((pc >= 0x83b06eu && pc <= 0x83b086u) || (pc >= 0x83b08bu && pc <= 0x83b0c7u));
    const bool byte_index = (pc >= 0x83b069u && pc <= 0x83b06cu);
    const bool changes_accumulator = pc == 0x83b067u || pc == 0x83b06cu || pc == 0x83b086u ||
                                     pc == 0x83b089u || pc == 0x83b0c7u || pc == 0x8eba82u;
    const bool changes_index = pc == 0x83b067u || pc == 0x83b06cu || pc == 0x8eba84u;
    if (!cpu || !cpu->ram || registers->e || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        (!dynamic_widths && ((!changes_accumulator && registers->mf != byte_accumulator) ||
                             (!changes_index && registers->xf != byte_index))) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    if (pc >= 0x83b062u && pc < 0x83b15eu) {
        const unsigned delta = pc - 0x83b062u;
        if (!cart || offset < delta)
            return 0;
        const unsigned first = offset - delta;
        if (cart->romSize < first + 252u ||
            memcmp(
                cart->rom + first,
                "\x48\xda\x5a\x8b\x08\xe2\x30\x4b\xab\x8b\xc2\x30\xa2\x00\xbf\xa0\x20\x04\xa9\xff\x00\x54\x00"
                "\xa6\xa2\x0d\xfb\xa0\x20\x03\xa9\x1f\x00\x54\x00\xa6\xe2\x20\xab\xc2\x20\xa9\x00\x00\x8f\xf8"
                "\xd4\x7f\x8f\x9a\xd5\x7f\x8d\x46\x12\x8d\x48\x12\x8d\x4a\x12\x8d\x4c\x12\x8d\x3e\x12\x8d\x40"
                "\x12\x8d\x42\x12\x8d\x44\x12\x8d\x5d\x12\xa2\x0e\x00\xa0\x00\x00\x7b\x9e\xc2\x05\x9f\x5a\xd5"
                "\x7f\x9f\x7a\xd5\x7f\xca\xca\x10\xf1\xe2\x20\xa9\x01\x1c\xa9\x09\xa9\x80\x85\x73\x64\x72\x64"
                "\x81\x9c\x0c\x42\x64\x74\x9c\xaa\x09\x9c\x09\x12\xa9\xff\x85\x71\x64\x6a\xa2\xa9\x9f\x86\x68"
                "\xa9\x83\x85\x6a\xa2\x00\x00\xbd\x5e\xb1\xc9\xff\xf0\x19\x85\x5d\xbd\x5f\xb1\x85\x5e\xe8\xe8"
                "\xa0\x00\x00\xbd\x5e\xb1\xe8\xc9\xff\xf0\xe5\x91\x5d\xc8\x80\xf3\xa2\x06\x00\x9e\x0d\x21\x9e"
                "\x0d\x21\x9e\x0e\x21\x9e\x0e\x21\x9e\x94\x05\x9e\x95\x05\x9e\x96\x05\x9e\x97\x05\xca\x10\xe5"
                "\xa9\x33\x8d\x23\x21\x8d\x24\x21\x8d\x25\x21\x9c\x2a\x21\x9c\x2b\x21\xa9\x08\x8d\x26\x21\xa9"
                "\xf7\x8d\x27\x21\xa9\x1f\x8d\x2e\x21\x8d\x2f\x21\x22\x07\xb0\x83\x28\xab\x7a\xfa\x68\x60",
                252u))
            return 0;
    } else if (pc >= 0x8eba81u && pc < 0x8ebae7u) {
        const unsigned delta = pc - 0x8eba81u;
        if (!cart || offset < delta)
            return 0;
        const unsigned first = offset - delta;
        if (cart->romSize < first + 102u ||
            memcmp(cart->rom + first,
                   "\x08\xe2\x20\xc2\x10\x9c\x34\x07\x9c\x35\x07\x7b\x8f\x81\xd0\x7f\x8f\x82\xd0\x7f\x8f\x83"
                   "\xd0\x7f\x8f\x84\xd0\x7f\xa2\x07\x00\x9f\x4f\xd0\x7f\x9f\x57\xd0\x7f\xca\x10\xf5\xa2\x07"
                   "\x00\x9f\x8c\xd1\x7f\xca\x10\xf9\x8f\xb3\x09\x00\x8f\xae\xd0\x7f\xa9\xff\x8d\x69\x12\x9c"
                   "\xa6\x09\x9c\xad\x09\x9c\x65\x12\x9c\x60\x12\xa9\x02\x8d\xaa\x05\x9c\xab\x05\x9c\x9b\x09"
                   "\x9c\x9c\x09\x9c\xb7\x05\x9c\xaa\x17\x9c\xab\x17\x28\x6b",
                   102u))
            return 0;
    } else
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83b062u: goto restore_save_accumulator;
    case 0x83b063u: goto restore_save_x;
    case 0x83b064u: goto restore_save_y;
    case 0x83b065u: goto restore_save_bank;
    case 0x83b066u: goto restore_save_status;
    case 0x83b067u: goto restore_narrow_registers;
    case 0x83b069u: goto restore_push_program_bank;
    case 0x83b06au: goto restore_select_program_bank;
    case 0x83b06bu: goto restore_save_move_bank;
    case 0x83b06cu: goto restore_widen_registers;
    case 0x83b06eu: goto restore_work_source;
    case 0x83b071u: goto restore_work_target;
    case 0x83b074u: goto restore_work_length;
    case 0x83b07au: goto restore_palette_source;
    case 0x83b07du: goto restore_palette_target;
    case 0x83b080u: goto restore_palette_length;
    case 0x83b086u: goto restore_narrow_after_moves;
    case 0x83b088u: goto restore_restore_move_bank;
    case 0x83b089u: goto restore_widen_clear_value;
    case 0x83b08bu: goto restore_clear_value;
    case 0x83b08eu: goto restore_clear_column_count;
    case 0x83b092u: goto restore_clear_column_state;
    case 0x83b096u: goto restore_clear_upload_word_0;
    case 0x83b099u: goto restore_clear_upload_word_1;
    case 0x83b09cu: goto restore_clear_upload_word_2;
    case 0x83b09fu: goto restore_clear_upload_word_3;
    case 0x83b0a2u: goto restore_clear_upload_word_4;
    case 0x83b0a5u: goto restore_clear_upload_word_5;
    case 0x83b0a8u: goto restore_clear_upload_word_6;
    case 0x83b0abu: goto restore_clear_upload_word_7;
    case 0x83b0aeu: goto restore_clear_upload_word_8;
    case 0x83b0b1u: goto restore_last_upload_slot;
    case 0x83b0b4u: goto restore_clear_y;
    case 0x83b0b7u: goto restore_seed_clear_value;
    case 0x83b0b8u: goto restore_clear_upload_slot;
    case 0x83b0bbu: goto restore_clear_column_slot;
    case 0x83b0bfu: goto restore_clear_row_slot;
    case 0x83b0c3u: goto restore_previous_slot_first;
    case 0x83b0c4u: goto restore_previous_slot_second;
    case 0x83b0c5u: goto restore_repeat_slot_clear;
    case 0x83b0c7u: goto restore_narrow_display_state;
    case 0x83b0c9u: goto restore_display_test_mask;
    case 0x83b0cbu: goto restore_clear_display_bits;
    case 0x83b0ceu: goto restore_upload_flags;
    case 0x83b0d0u: goto restore_store_upload_flags;
    case 0x83b0d2u: goto restore_clear_oam_request;
    case 0x83b0d4u: goto restore_clear_hdma_request;
    case 0x83b0d6u: goto restore_disable_hdma;
    case 0x83b0d9u: goto restore_clear_tile_request;
    case 0x83b0dbu: goto restore_clear_transition_flags;
    case 0x83b0deu: goto restore_clear_transition_state;
    case 0x83b0e1u: goto restore_text_wait_actor;
    case 0x83b0e3u: goto restore_store_wait_actor;
    case 0x83b0e5u: goto restore_clear_nmi_bank;
    case 0x83b0e7u: goto restore_field_nmi_target;
    case 0x83b0eau: goto restore_store_nmi_target;
    case 0x83b0ecu: goto restore_field_nmi_bank;
    case 0x83b0eeu: goto restore_publish_nmi_bank;
    case 0x83b0f0u: goto restore_first_table_byte;
    case 0x83b0f3u: goto restore_read_table_address;
    case 0x83b0f6u: goto restore_test_table_end;
    case 0x83b0f8u: goto restore_finish_display_table;
    case 0x83b0fau: goto restore_save_table_address_low;
    case 0x83b0fcu: goto restore_read_table_address_high;
    case 0x83b0ffu: goto restore_save_table_address_high;
    case 0x83b101u: goto restore_next_table_address_first;
    case 0x83b102u: goto restore_next_table_address_second;
    case 0x83b103u: goto restore_first_table_output;
    case 0x83b106u: goto restore_read_table_value;
    case 0x83b109u: goto restore_next_table_byte;
    case 0x83b10au: goto restore_test_value_end;
    case 0x83b10cu: goto restore_next_table_record;
    case 0x83b10eu: goto restore_write_table_value;
    case 0x83b110u: goto restore_next_table_output;
    case 0x83b111u: goto restore_repeat_table_value;
    case 0x83b113u: goto restore_last_scroll_register;
    case 0x83b116u: goto restore_clear_scroll_x_first;
    case 0x83b119u: goto restore_clear_scroll_x_second;
    case 0x83b11cu: goto restore_clear_scroll_y_first;
    case 0x83b11fu: goto restore_clear_scroll_y_second;
    case 0x83b122u: goto restore_clear_scroll_shadow_0;
    case 0x83b125u: goto restore_clear_scroll_shadow_1;
    case 0x83b128u: goto restore_clear_scroll_shadow_2;
    case 0x83b12bu: goto restore_clear_scroll_shadow_3;
    case 0x83b12eu: goto restore_previous_scroll_register;
    case 0x83b12fu: goto restore_repeat_scroll_clear;
    case 0x83b131u: goto restore_window_selection;
    case 0x83b133u: goto restore_select_window_bg12;
    case 0x83b136u: goto restore_select_window_bg34;
    case 0x83b139u: goto restore_select_window_objects;
    case 0x83b13cu: goto restore_clear_window_bg_logic;
    case 0x83b13fu: goto restore_clear_window_object_logic;
    case 0x83b142u: goto restore_window_left;
    case 0x83b144u: goto restore_set_window_left;
    case 0x83b147u: goto restore_window_right;
    case 0x83b149u: goto restore_set_window_right;
    case 0x83b14cu: goto restore_window_layers;
    case 0x83b14eu: goto restore_set_main_window_layers;
    case 0x83b151u: goto restore_set_sub_window_layers;
    case 0x83b154u: goto restore_upload_fixed_graphics;
    case 0x83b158u: goto restore_restore_status;
    case 0x83b159u: goto restore_restore_bank;
    case 0x83b15au: goto restore_restore_y;
    case 0x83b15bu: goto restore_restore_x;
    case 0x83b15cu: goto restore_restore_accumulator;
    case 0x83b15du: goto restore_return_field;
    case 0x8eba81u: goto reset_save_status;
    case 0x8eba82u: goto reset_narrow_accumulator;
    case 0x8eba84u: goto reset_widen_index;
    case 0x8eba86u: goto reset_clear_camera_actor;
    case 0x8eba89u: goto reset_clear_camera_state;
    case 0x8eba8cu: goto reset_seed_screen_offsets;
    case 0x8eba8du: goto reset_clear_screen_x_low;
    case 0x8eba91u: goto reset_clear_screen_x_high;
    case 0x8eba95u: goto reset_clear_screen_y_low;
    case 0x8eba99u: goto reset_clear_screen_y_high;
    case 0x8eba9du: goto reset_last_animation_slot;
    case 0x8ebaa0u: goto reset_clear_animation_object;
    case 0x8ebaa4u: goto reset_clear_animation_state;
    case 0x8ebaa8u: goto reset_previous_animation_slot;
    case 0x8ebaa9u: goto reset_repeat_animation_clear;
    case 0x8ebaabu: goto reset_last_unknown_slot;
    case 0x8ebaaeu: goto reset_clear_unknown_slot;
    case 0x8ebab2u: goto reset_previous_unknown_slot;
    case 0x8ebab3u: goto reset_repeat_unknown_clear;
    case 0x8ebab5u: goto reset_clear_actor_state;
    case 0x8ebab9u: goto reset_clear_display_state;
    case 0x8ebabdu: goto reset_wait_actor_sentinel;
    case 0x8ebabfu: goto reset_set_wait_actor;
    case 0x8ebac2u: goto reset_clear_effect_state;
    case 0x8ebac5u: goto reset_clear_effect_request;
    case 0x8ebac8u: goto reset_clear_battle_wait;
    case 0x8ebacbu: goto reset_clear_background_state;
    case 0x8ebaceu: goto reset_first_layer_table;
    case 0x8ebad0u: goto reset_select_layer_table;
    case 0x8ebad3u: goto reset_clear_layer_table_high;
    case 0x8ebad6u: goto reset_clear_text_state;
    case 0x8ebad9u: goto reset_clear_text_control;
    case 0x8ebadcu: goto reset_clear_field_requests;
    case 0x8ebadfu: goto reset_clear_animation_mask;
    case 0x8ebae2u: goto reset_clear_next_animation_mask;
    case 0x8ebae5u: goto reset_restore_status;
    case 0x8ebae6u: goto reset_return_reset;
    default: return 0;
    }
restore_save_accumulator: {
    const bool wide = !registers->mf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b062u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb063u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_save_x: {
    const bool wide = !registers->xf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b063u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb064u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_save_y: {
    const bool wide = !registers->xf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b064u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb065u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_save_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b065u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb066u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_save_status: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b066u, 1u, span.instructions != 0u);
    const uint16_t value = interp816_getFlags(registers);
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb067u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_narrow_registers: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b067u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb069u);
}
restore_push_program_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b069u, 1u, span.instructions != 0u);
    const uint16_t value = registers->k;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb06au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_select_program_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b06au, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb06bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_save_move_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b06bu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb06cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_widen_registers: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b06cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x30u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb06eu);
}
restore_work_source: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b06eu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0xbf00u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb071u);
}
restore_work_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b071u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0420u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb074u);
}
restore_work_length: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b074u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb077u);
    goto finished;
}
restore_palette_source: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b07au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0xfb0du, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb07du);
}
restore_palette_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b07du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0320u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb080u);
}
restore_palette_length: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b080u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x001fu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb083u);
    goto finished;
}
restore_narrow_after_moves: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b086u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb088u);
}
restore_restore_move_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b088u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb089u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_widen_clear_value: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b089u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb08bu);
}
restore_clear_value: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b08bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb08eu);
}
restore_clear_column_count: {
    const bool wide = true;
    const uint32_t address = (0x7fd4f8u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b08eu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb092u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_column_state: {
    const bool wide = true;
    const uint32_t address = (0x7fd59au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b092u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb096u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_0: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1246u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b096u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb099u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_1: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1248u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b099u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb09cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_2: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x124au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b09cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb09fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_3: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x124cu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b09fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0a2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_4: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x123eu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0a2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0a5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_5: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1240u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0a5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0a8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_6: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1242u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0a8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0abu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_7: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1244u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0abu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0aeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_upload_word_8: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x125du) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0aeu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0b1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_last_upload_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0b1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x000eu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb0b4u);
}
restore_clear_y: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0b4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb0b7u);
}
restore_seed_clear_value: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0b7u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0b8u);
    goto finished;
}
restore_clear_upload_slot: {
    const bool wide = true;
    const bool crossed = (0x05c2u >> 8) != ((0x05c2u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05c2u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0b8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0bbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_column_slot: {
    const bool wide = true;
    const uint32_t address = (0x7fd55au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0bbu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_row_slot: {
    const bool wide = true;
    const uint32_t address = (0x7fd57au + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0bfu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0c3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_previous_slot_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0c3u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0c4u);
}
restore_previous_slot_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0c4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0c5u);
}
restore_repeat_slot_clear: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0c5u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xb0b8u : 0xb0c7u);
    if (taken) {
        if (0x83b0b8u < 0x83b0c7u || 0x83b0b8u < first_pc || 0x83b0b8u >= end_pc)
            goto finished;
        goto restore_clear_upload_slot;
    }
    goto finished;
}
restore_narrow_display_state: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0c7u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb0c9u);
}
restore_display_test_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0c9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0cbu);
}
restore_clear_display_bits: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09a9u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 2u * wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0cbu, 3u, span.instructions != 0u);
    const uint8_t value = registers->read(registers->mem, address);
    registers->z = (value & (uint8_t)registers->a) == 0u;
    registers->write(registers->mem, address, value & ~(uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0ceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_upload_flags: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0ceu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0d0u);
}
restore_store_upload_flags: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x73u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0d0u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0d2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_oam_request: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x72u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0d2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0d4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_hdma_request: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x81u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0d4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0d6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_disable_hdma: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x420cu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0d6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0d9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_clear_tile_request: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x74u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0d9u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0dbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_transition_flags: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09aau) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0dbu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0deu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_transition_state: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1209u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0deu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0e1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_text_wait_actor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0e1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0e3u);
}
restore_store_wait_actor: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x71u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0e3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0e5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_nmi_bank: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x6au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0e5u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0e7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_field_nmi_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0e7u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x9fa9u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb0eau);
}
restore_store_nmi_target: {
    const bool wide = true;
    const uint32_t address = (uint16_t)(registers->dp + 0x68u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0eau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0ecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_field_nmi_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0ecu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0083u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0eeu);
}
restore_publish_nmi_bank: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x6au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0eeu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0f0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_first_table_byte: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0f0u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb0f3u);
    goto finished;
}
restore_read_table_address: {
    const bool wide = false;
    const bool crossed = (0xb15eu >> 8) != ((0xb15eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xb15eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
          (!wide || Lufia2DecompIsDecompressSourceByte(cart, high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0f3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0f6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_test_table_end: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0f6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb0f8u);
}
restore_finish_display_table: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0f8u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xb113u : 0xb0fau);
    if (taken) {
        if (0x83b113u < 0x83b0fau || 0x83b113u < first_pc || 0x83b113u >= end_pc)
            goto finished;
        goto restore_last_scroll_register;
    }
}
restore_save_table_address_low: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5du);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0fau, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0fcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_read_table_address_high: {
    const bool wide = false;
    const bool crossed = (0xb15fu >> 8) != ((0xb15fu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xb15fu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
          (!wide || Lufia2DecompIsDecompressSourceByte(cart, high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0fcu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb0ffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_save_table_address_high: {
    const bool wide = false;
    const uint32_t address = (uint16_t)(registers->dp + 0x5eu);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + wide + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b0ffu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb101u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_next_table_address_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b101u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb102u);
}
restore_next_table_address_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b102u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb103u);
}
restore_first_table_output: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b103u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x0000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb106u);
    goto finished;
}
restore_read_table_value: {
    const bool wide = false;
    const bool crossed = (0xb15eu >> 8) != ((0xb15eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xb15eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
          (!wide || Lufia2DecompIsDecompressSourceByte(cart, high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b106u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, wide), !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb109u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_next_table_byte: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b109u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb10au);
}
restore_test_value_end: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b10au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb10cu);
}
restore_next_table_record: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b10cu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xb0f3u : 0xb10eu);
    if (taken) {
        if (0x83b0f3u < 0x83b10eu || 0x83b0f3u < first_pc || 0x83b0f3u >= end_pc)
            goto finished;
        goto restore_read_table_address;
    }
    goto finished;
}
restore_write_table_value: {
    const uint16_t pointer_low = (uint16_t)(registers->dp + 0x5du);
    const uint16_t pointer_high = (uint16_t)(pointer_low + 1u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high))
        goto finished;
    const uint16_t pointer = cpu->ram[pointer_low] | ((uint16_t)cpu->ram[pointer_high] << 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + pointer + registers->y) & 0xffffffu;
    const bool crossed = (pointer >> 8) != ((pointer + registers->y) >> 8);
    const unsigned cycles = 6u + (!registers->xf || crossed) + direct_penalty;
    const bool ppu = (((address >> 16) & 0x7fu) < 0x40u) && (address & 0xffffu) >= 0x2100u &&
                     (address & 0xffffu) <= 0x2133u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || (!Lufia2DecompIsWramByte(address) && !ppu))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b10eu, 2u, span.instructions != 0u);
    (void)Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    registers->write(registers->mem, address, (uint8_t)registers->a);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb110u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_next_table_output: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b110u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint16_t)(registers->y + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb111u);
}
restore_repeat_table_value: {
    const bool taken = true;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b111u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xb106u : 0xb113u);
    if (taken) {
        if (0x83b106u < 0x83b113u || 0x83b106u < first_pc || 0x83b106u >= end_pc)
            goto finished;
        goto restore_read_table_value;
    }
    goto finished;
}
restore_last_scroll_register: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b113u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0006u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb116u);
    goto finished;
}
restore_clear_scroll_x_first: {
    const bool wide = false;
    const bool crossed = (0x210du >> 8) != ((0x210du + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x210du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b116u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb119u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_x_second: {
    const bool wide = false;
    const bool crossed = (0x210du >> 8) != ((0x210du + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x210du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b119u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb11cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_y_first: {
    const bool wide = false;
    const bool crossed = (0x210eu >> 8) != ((0x210eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x210eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b11cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb11fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_y_second: {
    const bool wide = false;
    const bool crossed = (0x210eu >> 8) != ((0x210eu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x210eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b11fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb122u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_shadow_0: {
    const bool wide = false;
    const bool crossed = (0x0594u >> 8) != ((0x0594u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0594u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b122u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb125u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_shadow_1: {
    const bool wide = false;
    const bool crossed = (0x0595u >> 8) != ((0x0595u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0595u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b125u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb128u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_shadow_2: {
    const bool wide = false;
    const bool crossed = (0x0596u >> 8) != ((0x0596u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0596u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b128u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb12bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_scroll_shadow_3: {
    const bool wide = false;
    const bool crossed = (0x0597u >> 8) != ((0x0597u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0597u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b12bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb12eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_previous_scroll_register: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b12eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb12fu);
}
restore_repeat_scroll_clear: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b12fu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xb116u : 0xb131u);
    if (taken) {
        if (0x83b116u < 0x83b131u || 0x83b116u < first_pc || 0x83b116u >= end_pc)
            goto finished;
        goto restore_clear_scroll_x_first;
    }
    goto finished;
}
restore_window_selection: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b131u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0033u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb133u);
}
restore_select_window_bg12: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2123u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b133u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb136u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_select_window_bg34: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2124u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b136u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb139u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_select_window_objects: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2125u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b139u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb13cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_window_bg_logic: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x212au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b13cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb13fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_clear_window_object_logic: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x212bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b13fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb142u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_window_left: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b142u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb144u);
}
restore_set_window_left: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2126u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b144u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb147u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_window_right: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b147u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00f7u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb149u);
}
restore_set_window_right: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2127u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b149u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb14cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_window_layers: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b14cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x001fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb14eu);
}
restore_set_main_window_layers: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x212eu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b14eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb151u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_set_sub_window_layers: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x212fu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b151u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb154u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_upload_fixed_graphics: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83b154u);
    const int cycles = Lufia2DecompRunDirectCallWithWidths(registers, 0x83b154u, 0x22u, 0x83b007u, true,
                                                           registers->mf, registers->xf);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_restore_status: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b158u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    interp816_setFlags(registers, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb159u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_restore_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b159u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb15au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_restore_y: {
    const bool wide = !registers->xf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b15au, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb15bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_restore_x: {
    const bool wide = !registers->xf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b15bu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb15cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_restore_accumulator: {
    const bool wide = !registers->mf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b15cu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb15du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_return_field: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83b15du);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83b15du, 0x60u, false, registers->mf, true);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
reset_save_status: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba81u, 1u, span.instructions != 0u);
    const uint16_t value = interp816_getFlags(registers);
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xba82u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba82u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xba84u);
}
reset_widen_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba84u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xba86u);
}
reset_clear_camera_actor: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0734u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba86u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xba89u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_camera_state: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0735u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba89u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xba8cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_seed_screen_offsets: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba8cu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xba8du);
}
reset_clear_screen_x_low: {
    const bool wide = false;
    const uint32_t address = (0x7fd081u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba8du, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xba91u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_screen_x_high: {
    const bool wide = false;
    const uint32_t address = (0x7fd082u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba91u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xba95u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_screen_y_low: {
    const bool wide = false;
    const uint32_t address = (0x7fd083u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba95u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xba99u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_screen_y_high: {
    const bool wide = false;
    const uint32_t address = (0x7fd084u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba99u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xba9du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_last_animation_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8eba9du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0007u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbaa0u);
}
reset_clear_animation_object: {
    const bool wide = false;
    const uint32_t address = (0x7fd04fu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaa0u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbaa4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_animation_state: {
    const bool wide = false;
    const uint32_t address = (0x7fd057u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaa4u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbaa8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_previous_animation_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaa8u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbaa9u);
}
reset_repeat_animation_clear: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaa9u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbaa0u : 0xbaabu);
    if (taken) {
        if (0x8ebaa0u < 0x8ebaabu || 0x8ebaa0u < first_pc || 0x8ebaa0u >= end_pc)
            goto finished;
        goto reset_clear_animation_object;
    }
}
reset_last_unknown_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaabu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0007u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xbaaeu);
}
reset_clear_unknown_slot: {
    const bool wide = false;
    const uint32_t address = (0x7fd18cu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaaeu, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbab2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_previous_unknown_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebab2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbab3u);
}
reset_repeat_unknown_clear: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebab3u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xbaaeu : 0xbab5u);
    if (taken) {
        if (0x8ebaaeu < 0x8ebab5u || 0x8ebaaeu < first_pc || 0x8ebaaeu >= end_pc)
            goto finished;
        goto reset_clear_unknown_slot;
    }
}
reset_clear_actor_state: {
    const bool wide = false;
    const uint32_t address = (0x0009b3u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebab5u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbab9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_display_state: {
    const bool wide = false;
    const uint32_t address = (0x7fd0aeu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && (!wide || Lufia2DecompIsWramByte(high))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebab9u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbabdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_wait_actor_sentinel: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebabdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbabfu);
}
reset_set_wait_actor: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1269u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebabfu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbac2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_effect_state: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09a6u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebac2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbac5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_effect_request: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x09adu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebac5u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbac8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_battle_wait: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1265u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebac8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbacbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_background_state: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1260u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebacbu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbaceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_first_layer_table: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebaceu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xbad0u);
}
reset_select_layer_table: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05aau) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebad0u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbad3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_layer_table_high: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05abu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebad3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbad6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_text_state: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x099bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebad6u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbad9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_text_control: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x099cu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebad9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbadcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_field_requests: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x05b7u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebadcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbadfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_animation_mask: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x17aau) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebadfu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbae2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_clear_next_animation_mask: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x17abu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, false) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, false))))
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebae2u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbae5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_restore_status: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x8ebae5u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    interp816_setFlags(registers, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xbae6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
reset_return_reset: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x8ebae6u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x8ebae6u, 0x6bu, true, registers->mf, true);
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
