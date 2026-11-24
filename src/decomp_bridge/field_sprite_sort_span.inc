#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldSortVisibleSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    const uint32_t end_pc = pc < 0x83a29bu ? 0x83a29bu : 0x83a319u;
    const bool byte_accumulator = pc <= 0x83a2aau || pc >= 0x83a317u;
    const bool dynamic_accumulator_widths = pc < 0x83a29bu || pc == 0x83a2aau || pc == 0x83a315u;
    if (pc < 0x83a295u || pc >= 0x83a319u || !cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram ||
        registers->e || !step_budget || registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || (!dynamic_accumulator_widths && registers->mf != byte_accumulator) ||
        (pc >= 0x83a29bu && pc < 0x83a2a2u) || !registers->xf ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x83a295u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 132u ||
        memcmp(cart->rom + first,
               "\x7b\xa8\xa2\x00\xe2\x20\xbd\x22\x06\x89\x04\xd0\x77\xbf\x16\xe3\x7f\x89\x80\xd0\x6f\xc2\x20"
               "\x8a\x85\xa7\x0a\xaa\xbf\xae\xdd\x7f\x18\x69\x30\x00\xc5\x54\x30\x58\xc5\x56\x10\x54\xbf\x3e"
               "\xde\x7f\x18\x69\x20\x00\xc5\x58\x30\x48\xc5\x5a\x10\x44\x09\x00\x10\x85\x63\xa6\xa7\xbf\x16"
               "\xe3\x7f\x89\x01\x00\xf0\x07\xa9\x00\x10\x14\x63\x80\x0a\x89\x02\x00\xf0\x05\xa9\x00\x20\x04"
               "\x63\xbb\xf0\x14\xbd\xfe\xe1\xc5\x63\xb0\x0d\x9d\x00\xe2\xbd\xfe\xe2\x9d\x00\xe3\xca\xca\xd0"
               "\xec\xa5\x63\x9d\x00\xe2\xa5\xa7\x9d\x00\xe3\xc8\xc8\xe2\x20\xa6\xa7",
               132u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83a295u: goto clear_from_direct_page;
    case 0x83a296u: goto first_sorted_actor;
    case 0x83a297u: goto first_actor_slot;
    case 0x83a299u: goto narrow_sort_state;
    case 0x83a2a2u: goto load_visibility;
    case 0x83a2a6u: goto test_hidden;
    case 0x83a2a8u: goto skip_hidden;
    case 0x83a2aau: goto widen_position;
    case 0x83a2acu: goto load_slot;
    case 0x83a2adu: goto save_slot;
    case 0x83a2afu: goto double_slot;
    case 0x83a2b0u: goto select_position;
    case 0x83a2b1u: goto load_world_x;
    case 0x83a2b5u: goto clear_x_carry;
    case 0x83a2b6u: goto add_x_margin;
    case 0x83a2b9u: goto compare_left;
    case 0x83a2bbu: goto skip_left;
    case 0x83a2bdu: goto compare_right;
    case 0x83a2bfu: goto skip_right;
    case 0x83a2c1u: goto load_world_y;
    case 0x83a2c5u: goto clear_y_carry;
    case 0x83a2c6u: goto add_y_margin;
    case 0x83a2c9u: goto compare_top;
    case 0x83a2cbu: goto skip_above;
    case 0x83a2cdu: goto compare_bottom;
    case 0x83a2cfu: goto skip_below;
    case 0x83a2d1u: goto mark_sort_key;
    case 0x83a2d4u: goto save_sort_key;
    case 0x83a2d6u: goto restore_slot;
    case 0x83a2d8u: goto load_sort_flags;
    case 0x83a2dcu: goto test_front_depth;
    case 0x83a2dfu: goto choose_front_depth;
    case 0x83a2e1u: goto front_depth_mask;
    case 0x83a2e4u: goto clear_front_depth;
    case 0x83a2e6u: goto finish_front_depth;
    case 0x83a2e8u: goto test_back_depth;
    case 0x83a2ebu: goto finish_back_depth;
    case 0x83a2edu: goto back_depth_mask;
    case 0x83a2f0u: goto set_back_depth;
    case 0x83a2f2u: goto first_sort_slot;
    case 0x83a2f3u: goto test_first_sort_slot;
    case 0x83a2f5u: goto load_previous_key;
    case 0x83a2f8u: goto compare_sort_key;
    case 0x83a2fau: goto finish_insertion;
    case 0x83a2fcu: goto move_previous_key;
    case 0x83a2ffu: goto load_previous_actor;
    case 0x83a302u: goto move_previous_actor;
    case 0x83a305u: goto previous_slot_first;
    case 0x83a306u: goto previous_slot_second;
    case 0x83a307u: goto repeat_insertion;
    case 0x83a309u: goto read_sort_key;
    case 0x83a30bu: goto store_sort_key;
    case 0x83a30eu: goto read_actor_slot;
    case 0x83a310u: goto store_actor_slot;
    case 0x83a313u: goto next_sort_slot_first;
    case 0x83a314u: goto next_sort_slot_second;
    case 0x83a315u: goto narrow_actor_slot;
    case 0x83a317u: goto restore_actor_slot;
    default: return 0;
    }
clear_from_direct_page: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a295u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->dp, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa296u);
}
first_sorted_actor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a296u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)registers->a, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa297u);
}
first_actor_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a297u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x00u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa299u);
}
narrow_sort_state: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a299u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa29bu);
    goto finished;
}
load_visibility: {
    const uint32_t address = (0x7fe316u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2a2u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2a6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_hidden: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2a6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0080u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2a8u);
}
skip_hidden: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2a8u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa319u : 0xa2aau);
    if (taken) {
        goto finished;
    }
}
widen_position: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2aau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2acu);
}
load_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2acu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2adu);
}
save_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2adu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2afu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
double_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2afu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)(registers->a << 1), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2b0u);
}
select_position: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2b0u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)registers->a, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2b1u);
}
load_world_x: {
    const uint32_t address = (0x7fddaeu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2b1u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2b5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_x_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2b5u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2b6u);
}
add_x_margin: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2b6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0030u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2b9u);
}
compare_left: {
    const uint32_t address = (uint16_t)(registers->dp + 0x54u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2b9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2bbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_left: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2bbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa315u : 0xa2bdu);
    if (taken) {
        goto narrow_actor_slot;
    }
}
compare_right: {
    const uint32_t address = (uint16_t)(registers->dp + 0x56u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2bdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2bfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_right: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2bfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa315u : 0xa2c1u);
    if (taken) {
        goto narrow_actor_slot;
    }
}
load_world_y: {
    const uint32_t address = (0x7fde3eu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2c1u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2c5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_y_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2c5u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2c6u);
}
add_y_margin: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2c6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0020u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2c9u);
}
compare_top: {
    const uint32_t address = (uint16_t)(registers->dp + 0x58u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2c9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2cbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_above: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2cbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa315u : 0xa2cdu);
    if (taken) {
        goto narrow_actor_slot;
    }
}
compare_bottom: {
    const uint32_t address = (uint16_t)(registers->dp + 0x5au);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2cdu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2cfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_below: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2cfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa315u : 0xa2d1u);
    if (taken) {
        goto narrow_actor_slot;
    }
}
mark_sort_key: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2d1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x09u, 0x1000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2d4u);
}
save_sort_key: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2d4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2d6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2d6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2d8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_sort_flags: {
    const uint32_t address = (0x7fe316u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2d8u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2dcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_front_depth: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2dcu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0001u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2dfu);
}
choose_front_depth: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2dfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa2e8u : 0xa2e1u);
    if (taken) {
        goto test_back_depth;
    }
}
front_depth_mask: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2e1u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x1000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2e4u);
}
clear_front_depth: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2e4u, 2u, span.instructions != 0u);
    const uint16_t previous = Lufia2DecompReadData(registers, address, high, true);
    registers->z = (previous & registers->a) == 0u;
    const uint16_t updated = (uint16_t)(previous & ~registers->a);
    registers->write(registers->mem, high, (uint8_t)(updated >> 8));
    registers->write(registers->mem, address, (uint8_t)updated);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2e6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_front_depth: {
    const bool taken = true;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2e6u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa2f2u : 0xa2e8u);
    if (taken) {
        goto first_sort_slot;
    }
}
test_back_depth: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2e8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x0002u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2ebu);
}
finish_back_depth: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2ebu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa2f2u : 0xa2edu);
    if (taken) {
        goto first_sort_slot;
    }
}
back_depth_mask: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2edu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x2000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa2f0u);
}
set_back_depth: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2f0u, 2u, span.instructions != 0u);
    const uint16_t previous = Lufia2DecompReadData(registers, address, high, true);
    registers->z = (previous & registers->a) == 0u;
    const uint16_t updated = (uint16_t)(previous | registers->a);
    registers->write(registers->mem, high, (uint8_t)(updated >> 8));
    registers->write(registers->mem, address, (uint8_t)updated);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2f2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_sort_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2f2u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)registers->y, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa2f3u);
}
test_first_sort_slot: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2f3u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa309u : 0xa2f5u);
    if (taken) {
        goto read_sort_key;
    }
}
load_previous_key: {
    const bool crossed = (0xe1feu >> 8) != ((0xe1feu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xe1feu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2f5u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2f8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
compare_sort_key: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2f8u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2fau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_insertion: {
    const bool taken = registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2fau, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa309u : 0xa2fcu);
    if (taken) {
        goto read_sort_key;
    }
}
move_previous_key: {
    const bool crossed = (0xe200u >> 8) != ((0xe200u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xe200u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2fcu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa2ffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
load_previous_actor: {
    const bool crossed = (0xe2feu >> 8) != ((0xe2feu + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xe2feu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + crossed;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a2ffu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa302u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
move_previous_actor: {
    const bool crossed = (0xe300u >> 8) != ((0xe300u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xe300u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a302u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa305u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
previous_slot_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a305u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x - 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa306u);
}
previous_slot_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a306u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint8_t)(registers->x - 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa307u);
}
repeat_insertion: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a307u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa2f5u : 0xa309u);
    if (taken) {
        goto finished;
    }
}
read_sort_key: {
    const uint32_t address = (uint16_t)(registers->dp + 0x63u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a309u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa30bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_sort_key: {
    const bool crossed = (0xe200u >> 8) != ((0xe200u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xe200u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a30bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa30eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_actor_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address) &&
                                                    Lufia2DecompIsDecompressSourceByte(cart, high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a30eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa310u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_actor_slot: {
    const bool crossed = (0xe300u >> 8) != ((0xe300u + registers->x) >> 8);
    const uint32_t address = (((uint32_t)registers->db << 16) + 0xe300u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + (!registers->xf || crossed);
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsWramByte(address) && Lufia2DecompIsWramByte(high)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a310u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa313u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_sort_slot_first: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a313u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa314u);
}
next_sort_slot_second: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a314u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, (uint8_t)(registers->y + 1u), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa315u);
}
narrow_actor_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a315u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa317u);
}
restore_actor_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0xa7u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !(Lufia2DecompIsDecompressSourceByte(cart, address)))
        goto finished;
    Lufia2SpanFetch(registers, 0x83a317u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa319u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
finished:
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
