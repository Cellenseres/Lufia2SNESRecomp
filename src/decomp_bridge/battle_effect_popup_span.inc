#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattleRestorePortraitSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                    uint8_t opcode, uint64_t dynamic_epoch,
                                                    uint64_t write_epoch, unsigned step_budget,
                                                    long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819dc9u && pc < 0x819df5u)
        end_pc = 0x819df5u;
    else if (pc >= 0x819df5u && pc < 0x819df6u)
        end_pc = 0x819df6u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819dc9u: byte_accumulator = true; break;
    case 0x819dcau: byte_accumulator = true; break;
    case 0x819dceu: byte_accumulator = true; break;
    case 0x819dd0u: byte_accumulator = true; break;
    case 0x819dd2u: byte_accumulator = true; break;
    case 0x819dd4u: byte_accumulator = true; break;
    case 0x819dd6u: byte_accumulator = true; break;
    case 0x819dd7u: width_neutral = true; break;
    case 0x819dddu: width_neutral = true; break;
    case 0x819ddfu: byte_accumulator = true; break;
    case 0x819de2u: byte_accumulator = true; break;
    case 0x819de4u: byte_accumulator = true; break;
    case 0x819de6u: byte_accumulator = true; break;
    case 0x819de7u: byte_accumulator = true; break;
    case 0x819de9u: byte_accumulator = true; break;
    case 0x819deau: byte_accumulator = true; break;
    case 0x819debu: byte_accumulator = true; break;
    case 0x819deeu: byte_accumulator = true; break;
    case 0x819defu: byte_accumulator = true; break;
    case 0x819df1u: byte_accumulator = true; break;
    case 0x819df4u: byte_accumulator = true; break;
    case 0x819df5u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819dc9u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 45u ||
        memcmp(cart->rom + first,
               "\xbb\xbf\x25\x00\x7e\x30\x25\xc9\x04\xf0\x21\x85\x11\x5a\xc2\x20\x29\x7f\x00\xaa\xe2\x20\xbd"
               "\x47\x15\x85\x12\x85\x12\x8b\xa9\x97\x48\xab\x20\x75\xbb\xab\xa9\xff\x8d\xb0\x15\x7a\x60",
               45u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819dc9u: goto select_target;
    case 0x819dcau: goto read_target;
    case 0x819dceu: goto skip_enemy;
    case 0x819dd0u: goto test_special;
    case 0x819dd2u: goto skip_special;
    case 0x819dd4u: goto store_slot;
    case 0x819dd6u: goto save_target;
    case 0x819dd7u: goto widen_pose;
    case 0x819dd9u: goto mask_slot;
    case 0x819ddcu: goto select_slot;
    case 0x819dddu: goto narrow_pose;
    case 0x819ddfu: goto read_pose;
    case 0x819de2u: goto store_pose;
    case 0x819de4u: goto repeat_pose_store;
    case 0x819de6u: goto save_bank;
    case 0x819de7u: goto pose_bank;
    case 0x819de9u: goto push_bank;
    case 0x819deau: goto select_bank;
    case 0x819debu: goto call_pose;
    case 0x819deeu: goto restore_bank;
    case 0x819defu: goto status_request;
    case 0x819df1u: goto store_request;
    case 0x819df4u: goto restore_target;
    case 0x819df5u: goto return_pose;
    default: return 0;
    }
select_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dc9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9dcau);
}
read_target: {
    const uint32_t address = (0x7e0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dcau, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9dceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_enemy: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dceu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9df5u : 0x9dd0u);
    if (taken)
        goto finished;
}
test_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dd0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9dd2u);
}
skip_special: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dd2u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9df5u : 0x9dd4u);
    if (taken)
        goto finished;
}
store_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dd4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9dd6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819dd6u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9dd7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_pose: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dd7u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9dd9u);
}
mask_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dd9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ddcu);
}
select_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ddcu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9dddu);
}
narrow_pose: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819dddu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ddfu);
}
read_pose: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1547u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x1547u >> 8) != ((0x1547u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ddfu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9de2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_pose: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819de2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9de4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_pose_store: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819de4u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9de6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819de6u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9de7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
pose_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819de7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0097u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9de9u);
}
push_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819de9u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9deau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819deau, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9debu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_pose: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819debu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819debu, 0x20u, 0x81bb75u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819deeu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9defu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
status_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819defu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9df1u);
}
store_request: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15b0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819df1u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9df4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819df4u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9df5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_pose: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819df5u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819df5u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleSendSoundSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                              uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                              unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819e13u && pc < 0x819e21u)
        end_pc = 0x819e21u;
    else if (pc >= 0x819e21u && pc < 0x819e22u)
        end_pc = 0x819e22u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819e13u: byte_accumulator = true; break;
    case 0x819e15u: byte_accumulator = true; break;
    case 0x819e16u: byte_accumulator = true; break;
    case 0x819e1au: byte_accumulator = true; break;
    case 0x819e1bu: width_neutral = true; break;
    case 0x819e1fu: width_neutral = true; break;
    case 0x819e21u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819e13u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 15u ||
        memcmp(cart->rom + first, "\xa7\xc3\x5a\x22\x3b\x95\x80\x7a\xc2\x20\xe6\xc3\xe2\x20\x60", 15u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819e13u: goto read_command;
    case 0x819e15u: goto save_target;
    case 0x819e16u: goto send_command;
    case 0x819e1au: goto restore_target;
    case 0x819e1bu: goto widen_stream;
    case 0x819e1du: goto advance_stream;
    case 0x819e1fu: goto narrow_command;
    case 0x819e21u: goto return_command;
    default: return 0;
    }
read_command: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e13u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e15u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e15u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9e16u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
send_command: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819e16u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819e16u, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e1au, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9e1bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e1bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e1du);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e1du, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e1fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_command: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e1fu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e21u);
    goto finished;
}
return_command: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819e21u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819e21u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleTargetPopupsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819e22u && pc < 0x819e34u)
        end_pc = 0x819e34u;
    else if (pc >= 0x819e34u && pc < 0x819f26u)
        end_pc = 0x819f26u;
    else if (pc >= 0x819f26u && pc < 0x819f38u)
        end_pc = 0x819f38u;
    else if (pc >= 0x819f38u && pc < 0x819f39u)
        end_pc = 0x819f39u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819e22u: byte_accumulator = true; break;
    case 0x819e24u: byte_accumulator = true; break;
    case 0x819e25u: byte_accumulator = true; break;
    case 0x819e26u: width_neutral = true; break;
    case 0x819e34u: width_neutral = true; break;
    case 0x819e36u: byte_accumulator = true; break;
    case 0x819e39u: byte_accumulator = true; break;
    case 0x819e3bu: byte_accumulator = true; break;
    case 0x819e3cu: byte_accumulator = true; break;
    case 0x819e3du: byte_accumulator = true; break;
    case 0x819e3fu: byte_accumulator = true; break;
    case 0x819e42u: byte_accumulator = true; break;
    case 0x819e44u: byte_accumulator = true; break;
    case 0x819e47u: byte_accumulator = true; break;
    case 0x819e48u: byte_accumulator = true; break;
    case 0x819e49u: byte_accumulator = true; break;
    case 0x819e4cu: byte_accumulator = true; break;
    case 0x819e4eu: byte_accumulator = true; break;
    case 0x819e50u: byte_accumulator = true; break;
    case 0x819e52u: byte_accumulator = true; break;
    case 0x819e55u: byte_accumulator = true; break;
    case 0x819e57u: byte_accumulator = true; break;
    case 0x819e59u: byte_accumulator = true; break;
    case 0x819e5cu: byte_accumulator = true; break;
    case 0x819e5eu: byte_accumulator = true; break;
    case 0x819e60u: byte_accumulator = true; break;
    case 0x819e63u: byte_accumulator = true; break;
    case 0x819e65u: byte_accumulator = true; break;
    case 0x819e67u: byte_accumulator = true; break;
    case 0x819e6au: byte_accumulator = true; break;
    case 0x819e6cu: byte_accumulator = true; break;
    case 0x819e6du: byte_accumulator = true; break;
    case 0x819e6fu: byte_accumulator = true; break;
    case 0x819e71u: byte_accumulator = true; break;
    case 0x819e73u: byte_accumulator = true; break;
    case 0x819e74u: byte_accumulator = true; break;
    case 0x819e76u: byte_accumulator = true; break;
    case 0x819e78u: byte_accumulator = true; break;
    case 0x819e7au: byte_accumulator = true; break;
    case 0x819e7cu: byte_accumulator = true; break;
    case 0x819e7du: byte_accumulator = true; break;
    case 0x819e80u: byte_accumulator = true; break;
    case 0x819e82u: byte_accumulator = true; break;
    case 0x819e83u: byte_accumulator = true; break;
    case 0x819e85u: byte_accumulator = true; break;
    case 0x819e87u: byte_accumulator = true; break;
    case 0x819e89u: byte_accumulator = true; break;
    case 0x819e8bu: byte_accumulator = true; break;
    case 0x819e8du: byte_accumulator = true; break;
    case 0x819e8eu: byte_accumulator = true; break;
    case 0x819e90u: byte_accumulator = true; break;
    case 0x819e92u: byte_accumulator = true; break;
    case 0x819e94u: byte_accumulator = true; break;
    case 0x819e95u: byte_accumulator = true; break;
    case 0x819e98u: byte_accumulator = true; break;
    case 0x819e99u: byte_accumulator = true; break;
    case 0x819e9bu: byte_accumulator = true; break;
    case 0x819e9du: byte_accumulator = true; break;
    case 0x819ea0u: byte_accumulator = true; break;
    case 0x819ea2u: byte_accumulator = true; break;
    case 0x819ea3u: byte_accumulator = true; break;
    case 0x819ea5u: byte_accumulator = true; break;
    case 0x819ea7u: byte_accumulator = true; break;
    case 0x819ea9u: byte_accumulator = true; break;
    case 0x819eaau: byte_accumulator = true; break;
    case 0x819eacu: byte_accumulator = true; break;
    case 0x819eaeu: byte_accumulator = true; break;
    case 0x819eb0u: byte_accumulator = true; break;
    case 0x819eb2u: byte_accumulator = true; break;
    case 0x819eb4u: byte_accumulator = true; break;
    case 0x819eb6u: byte_accumulator = true; break;
    case 0x819eb7u: byte_accumulator = true; break;
    case 0x819eb9u: byte_accumulator = true; break;
    case 0x819ebau: byte_accumulator = true; break;
    case 0x819ebcu: byte_accumulator = true; break;
    case 0x819ebeu: byte_accumulator = true; break;
    case 0x819ebfu: byte_accumulator = true; break;
    case 0x819ec1u: byte_accumulator = true; break;
    case 0x819ec3u: byte_accumulator = true; break;
    case 0x819ec5u: byte_accumulator = true; break;
    case 0x819ec6u: byte_accumulator = true; break;
    case 0x819ec8u: byte_accumulator = true; break;
    case 0x819ecau: byte_accumulator = true; break;
    case 0x819ecbu: byte_accumulator = true; break;
    case 0x819eccu: byte_accumulator = true; break;
    case 0x819eceu: byte_accumulator = true; break;
    case 0x819ed0u: byte_accumulator = true; break;
    case 0x819ed3u: byte_accumulator = true; break;
    case 0x819ed4u: byte_accumulator = true; break;
    case 0x819ed6u: byte_accumulator = true; break;
    case 0x819ed8u: byte_accumulator = true; break;
    case 0x819ed9u: byte_accumulator = true; break;
    case 0x819edbu: byte_accumulator = true; break;
    case 0x819eddu: byte_accumulator = true; break;
    case 0x819edfu: byte_accumulator = true; break;
    case 0x819ee0u: byte_accumulator = true; break;
    case 0x819ee2u: byte_accumulator = true; break;
    case 0x819ee4u: byte_accumulator = true; break;
    case 0x819ee5u: byte_accumulator = true; break;
    case 0x819ee6u: byte_accumulator = true; break;
    case 0x819ee8u: byte_accumulator = true; break;
    case 0x819eeau: byte_accumulator = true; break;
    case 0x819eedu: byte_accumulator = true; break;
    case 0x819eefu: byte_accumulator = true; break;
    case 0x819ef0u: byte_accumulator = true; break;
    case 0x819ef2u: byte_accumulator = true; break;
    case 0x819ef4u: byte_accumulator = true; break;
    case 0x819ef5u: byte_accumulator = true; break;
    case 0x819ef7u: byte_accumulator = true; break;
    case 0x819ef9u: byte_accumulator = true; break;
    case 0x819efcu: byte_accumulator = true; break;
    case 0x819efdu: byte_accumulator = true; break;
    case 0x819effu: byte_accumulator = true; break;
    case 0x819f01u: byte_accumulator = true; break;
    case 0x819f02u: byte_accumulator = true; break;
    case 0x819f04u: byte_accumulator = true; break;
    case 0x819f06u: byte_accumulator = true; break;
    case 0x819f08u: byte_accumulator = true; break;
    case 0x819f09u: byte_accumulator = true; break;
    case 0x819f0bu: byte_accumulator = true; break;
    case 0x819f0du: byte_accumulator = true; break;
    case 0x819f0eu: byte_accumulator = true; break;
    case 0x819f0fu: byte_accumulator = true; break;
    case 0x819f11u: byte_accumulator = true; break;
    case 0x819f13u: byte_accumulator = true; break;
    case 0x819f16u: byte_accumulator = true; break;
    case 0x819f18u: byte_accumulator = true; break;
    case 0x819f19u: byte_accumulator = true; break;
    case 0x819f1bu: byte_accumulator = true; break;
    case 0x819f1du: byte_accumulator = true; break;
    case 0x819f1eu: byte_accumulator = true; break;
    case 0x819f20u: byte_accumulator = true; break;
    case 0x819f22u: byte_accumulator = true; break;
    case 0x819f25u: byte_accumulator = true; break;
    case 0x819f26u: width_neutral = true; break;
    case 0x819f2eu: width_neutral = true; break;
    case 0x819f30u: byte_accumulator = true; break;
    case 0x819f32u: byte_accumulator = true; break;
    case 0x819f34u: byte_accumulator = true; break;
    case 0x819f37u: byte_accumulator = true; break;
    case 0x819f38u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819e22u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 279u ||
        memcmp(
            cart->rom + first,
            "\xa9\x7e\x48\xab\xc2\x20\xad\x0a\x4f\x29\xff\x00\x85\x02\x5a\xa2\x0b\x4f\xe2\x20\xbd\x09\x00\x29"
            "\x01\x0a\x0a\x85\x14\xbd\x00\x00\x85\x04\xbd\x03\x00\x0a\x0a\x7d\x01\x00\x85\x05\xa9\x01\x85\x13"
            "\xbd\x08\x00\xc9\x0a\xd0\x03\x4c\xd6\x9e\xc9\x0b\xd0\x03\x4c\xbc\x9e\xc9\x0c\xd0\x03\x4c\xff\x9e"
            "\xa5\x05\x38\xe9\x04\x85\x05\xa5\x04\x18\x69\x08\x85\x04\xa9\x05\x85\x11\xda\xbd\x08\x00\x30\x12"
            "\xca\xc6\x04\xc6\x04\xc6\x04\xc6\x04\xa5\x11\x3a\x85\x11\xc9\x01\xd0\xe9\xe8\xbd\x08\x00\x18\x69"
            "\x0a\x85\x12\x20\x39\x9f\xa5\x04\x18\x69\x08\x85\x04\xa5\x13\x18\x69\x02\x85\x13\xa5\x11\xc9\x04"
            "\xf0\x05\xe6\x11\xe8\x80\xdc\xfa\x80\x6a\xa5\x05\x38\xe9\x08\x85\x05\xa5\x04\x18\x69\x0c\x85\x04"
            "\xda\xe8\xa9\x16\x85\x12\x20\x39\x9f\xfa\x80\x50\xa5\x05\x38\xe9\x08\x85\x05\xa5\x04\x18\x69\x0c"
            "\x85\x04\xda\xe8\xa9\x14\x85\x12\x20\x39\x9f\xa5\x04\x38\xe9\x10\x85\x04\xe8\xa9\x15\x85\x12\x20"
            "\x39\x9f\xfa\x80\x27\xa5\x05\x38\xe9\x08\x85\x05\xa5\x04\x18\x69\x0c\x85\x04\xda\xe8\xa9\x17\x85"
            "\x12\x20\x39\x9f\xa5\x04\x38\xe9\x10\x85\x04\xe8\xa9\x18\x85\x12\x20\x39\x9f\xfa\xc2\x20\x8a\x18"
            "\x69\x0b\x00\xaa\xe2\x20\xc6\x02\xf0\x03\x4c\x34\x9e\x7a\x60",
            279u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819e22u: goto target_bank;
    case 0x819e24u: goto push_bank;
    case 0x819e25u: goto select_bank;
    case 0x819e26u: goto widen_count;
    case 0x819e28u: goto read_target_count;
    case 0x819e2bu: goto mask_target_count;
    case 0x819e2eu: goto store_target_count;
    case 0x819e30u: goto save_target;
    case 0x819e31u: goto first_target;
    case 0x819e34u: goto narrow_target;
    case 0x819e36u: goto read_attributes;
    case 0x819e39u: goto mask_attributes;
    case 0x819e3bu: goto shift_attributes_low;
    case 0x819e3cu: goto shift_attributes_high;
    case 0x819e3du: goto store_attributes;
    case 0x819e3fu: goto read_horizontal;
    case 0x819e42u: goto store_horizontal;
    case 0x819e44u: goto read_height;
    case 0x819e47u: goto shift_height_low;
    case 0x819e48u: goto shift_height_high;
    case 0x819e49u: goto add_vertical;
    case 0x819e4cu: goto store_vertical;
    case 0x819e4eu: goto initial_phase;
    case 0x819e50u: goto store_phase;
    case 0x819e52u: goto read_result;
    case 0x819e55u: goto test_miss;
    case 0x819e57u: goto skip_miss_branch;
    case 0x819e59u: goto select_miss;
    case 0x819e5cu: goto test_special;
    case 0x819e5eu: goto skip_special_branch;
    case 0x819e60u: goto select_special;
    case 0x819e63u: goto test_critical;
    case 0x819e65u: goto skip_critical_branch;
    case 0x819e67u: goto select_critical;
    case 0x819e6au: goto read_digit_vertical;
    case 0x819e6cu: goto set_digit_vertical_carry;
    case 0x819e6du: goto raise_digits;
    case 0x819e6fu: goto store_digit_vertical;
    case 0x819e71u: goto read_digit_horizontal;
    case 0x819e73u: goto clear_digit_horizontal_carry;
    case 0x819e74u: goto offset_digits;
    case 0x819e76u: goto store_digit_horizontal;
    case 0x819e78u: goto digit_count;
    case 0x819e7au: goto store_digit_count;
    case 0x819e7cu: goto save_digit_target;
    case 0x819e7du: goto read_leading_digit;
    case 0x819e80u: goto found_leading_digit;
    case 0x819e82u: goto previous_digit;
    case 0x819e83u: goto move_digit_left_one;
    case 0x819e85u: goto move_digit_left_two;
    case 0x819e87u: goto move_digit_left_three;
    case 0x819e89u: goto move_digit_left_four;
    case 0x819e8bu: goto read_digit_count;
    case 0x819e8du: goto decrease_digit_count;
    case 0x819e8eu: goto store_remaining_digits;
    case 0x819e90u: goto test_last_digit;
    case 0x819e92u: goto scan_leading_digits;
    case 0x819e94u: goto first_visible_digit;
    case 0x819e95u: goto read_visible_digit;
    case 0x819e98u: goto clear_digit_script_carry;
    case 0x819e99u: goto digit_script;
    case 0x819e9bu: goto store_digit_script;
    case 0x819e9du: goto spawn_digit;
    case 0x819ea0u: goto read_next_digit_horizontal;
    case 0x819ea2u: goto clear_next_digit_carry;
    case 0x819ea3u: goto next_digit_horizontal;
    case 0x819ea5u: goto store_next_digit_horizontal;
    case 0x819ea7u: goto read_digit_phase;
    case 0x819ea9u: goto clear_digit_phase_carry;
    case 0x819eaau: goto next_digit_phase;
    case 0x819eacu: goto store_digit_phase;
    case 0x819eaeu: goto read_remaining_digits;
    case 0x819eb0u: goto test_final_digit;
    case 0x819eb2u: goto finish_digits;
    case 0x819eb4u: goto count_visible_digits;
    case 0x819eb6u: goto next_visible_digit;
    case 0x819eb7u: goto repeat_digits;
    case 0x819eb9u: goto restore_digit_target;
    case 0x819ebau: goto finish_digit_target;
    case 0x819ebcu: goto read_special_vertical;
    case 0x819ebeu: goto set_special_vertical_carry;
    case 0x819ebfu: goto raise_special;
    case 0x819ec1u: goto store_special_vertical;
    case 0x819ec3u: goto read_special_horizontal;
    case 0x819ec5u: goto clear_special_horizontal_carry;
    case 0x819ec6u: goto offset_special;
    case 0x819ec8u: goto store_special_horizontal;
    case 0x819ecau: goto save_special_target;
    case 0x819ecbu: goto next_special_target;
    case 0x819eccu: goto special_script;
    case 0x819eceu: goto store_special_script;
    case 0x819ed0u: goto spawn_special;
    case 0x819ed3u: goto restore_special_target;
    case 0x819ed4u: goto finish_special_target;
    case 0x819ed6u: goto read_miss_vertical;
    case 0x819ed8u: goto set_miss_vertical_carry;
    case 0x819ed9u: goto raise_miss;
    case 0x819edbu: goto store_miss_vertical;
    case 0x819eddu: goto read_miss_horizontal;
    case 0x819edfu: goto clear_miss_horizontal_carry;
    case 0x819ee0u: goto offset_miss;
    case 0x819ee2u: goto store_miss_horizontal;
    case 0x819ee4u: goto save_miss_target;
    case 0x819ee5u: goto first_miss_target;
    case 0x819ee6u: goto first_miss_script;
    case 0x819ee8u: goto store_first_miss_script;
    case 0x819eeau: goto spawn_first_miss;
    case 0x819eedu: goto read_second_miss_horizontal;
    case 0x819eefu: goto set_second_miss_carry;
    case 0x819ef0u: goto offset_second_miss;
    case 0x819ef2u: goto store_second_miss_horizontal;
    case 0x819ef4u: goto second_miss_target;
    case 0x819ef5u: goto second_miss_script;
    case 0x819ef7u: goto store_second_miss_script;
    case 0x819ef9u: goto spawn_second_miss;
    case 0x819efcu: goto restore_miss_target;
    case 0x819efdu: goto finish_miss_target;
    case 0x819effu: goto read_critical_vertical;
    case 0x819f01u: goto set_critical_vertical_carry;
    case 0x819f02u: goto raise_critical;
    case 0x819f04u: goto store_critical_vertical;
    case 0x819f06u: goto read_critical_horizontal;
    case 0x819f08u: goto clear_critical_horizontal_carry;
    case 0x819f09u: goto offset_critical;
    case 0x819f0bu: goto store_critical_horizontal;
    case 0x819f0du: goto save_critical_target;
    case 0x819f0eu: goto first_critical_target;
    case 0x819f0fu: goto first_critical_script;
    case 0x819f11u: goto store_first_critical_script;
    case 0x819f13u: goto spawn_first_critical;
    case 0x819f16u: goto read_second_critical_horizontal;
    case 0x819f18u: goto set_second_critical_carry;
    case 0x819f19u: goto offset_second_critical;
    case 0x819f1bu: goto store_second_critical_horizontal;
    case 0x819f1du: goto second_critical_target;
    case 0x819f1eu: goto second_critical_script;
    case 0x819f20u: goto store_second_critical_script;
    case 0x819f22u: goto spawn_second_critical;
    case 0x819f25u: goto restore_critical_target;
    case 0x819f26u: goto widen_next_target;
    case 0x819f28u: goto current_target;
    case 0x819f29u: goto clear_next_target_carry;
    case 0x819f2au: goto next_target_offset;
    case 0x819f2du: goto select_next_target;
    case 0x819f2eu: goto narrow_next_target;
    case 0x819f30u: goto count_targets;
    case 0x819f32u: goto finish_targets;
    case 0x819f34u: goto repeat_targets;
    case 0x819f37u: goto restore_target;
    case 0x819f38u: goto return_popups;
    default: return 0;
    }
target_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e22u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e24u);
}
push_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e24u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e25u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e25u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9e26u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e26u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e28u);
}
read_target_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4f0au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e28u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e2bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_target_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e2bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e2eu);
}
store_target_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x02u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e2eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e30u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e30u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9e31u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e31u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x4f0bu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e34u);
    goto finished;
}
narrow_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e34u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e36u);
}
read_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0009u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0009u >> 8) != ((0x0009u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e36u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e39u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_attributes: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e39u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e3bu);
}
shift_attributes_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e3bu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e3cu);
}
shift_attributes_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e3cu, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e3du);
}
store_attributes: {
    const uint32_t address = (uint16_t)(registers->dp + 0x14u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e3du, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e3fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0000u >> 8) != ((0x0000u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e3fu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e42u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e42u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e44u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_height: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0003u >> 8) != ((0x0003u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e44u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e47u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
shift_height_low: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e47u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e48u);
}
shift_height_high: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e48u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x80u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e49u);
}
add_vertical: {
    const uint32_t address = ((((uint32_t)registers->db << 16) | 0x0001u) + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0001u >> 8) != ((0x0001u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e49u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e4cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e4cu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e4eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
initial_phase: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e4eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e50u);
}
store_phase: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e50u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e52u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_result: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0008u >> 8) != ((0x0008u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e52u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e55u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_miss: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e55u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x000au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e57u);
}
skip_miss_branch: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e57u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9e5cu : 0x9e59u);
    if (taken)
        goto test_special;
}
select_miss: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e59u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ed6u);
    goto finished;
}
test_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e5cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x000bu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e5eu);
}
skip_special_branch: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e5eu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9e63u : 0x9e60u);
    if (taken)
        goto test_critical;
}
select_special: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e60u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ebcu);
    goto finished;
}
test_critical: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e63u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x000cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e65u);
}
skip_critical_branch: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e65u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9e6au : 0x9e67u);
    if (taken)
        goto read_digit_vertical;
}
select_critical: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e67u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9effu);
    goto finished;
}
read_digit_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e6au, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e6cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_digit_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e6cu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e6du);
}
raise_digits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e6du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e6fu);
}
store_digit_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e6fu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e71u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_digit_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e71u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e73u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_digit_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e73u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e74u);
}
offset_digits: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e74u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e76u);
}
store_digit_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e76u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e78u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
digit_count: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e78u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0005u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e7au);
}
store_digit_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e7au, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e7cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_digit_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819e7cu, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9e7du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_leading_digit: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0008u >> 8) != ((0x0008u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e7du, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e80u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
found_leading_digit: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e80u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9e94u : 0x9e82u);
    if (taken)
        goto first_visible_digit;
}
previous_digit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e82u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x - 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e83u);
}
move_digit_left_one: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e83u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e85u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
move_digit_left_two: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e85u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e87u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
move_digit_left_three: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e87u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e89u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
move_digit_left_four: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e89u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e8bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_digit_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e8bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e8du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
decrease_digit_count: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e8du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffu), true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e8eu);
}
store_remaining_digits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e8eu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e90u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_last_digit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e90u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e92u);
}
scan_leading_digits: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e92u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9e7du : 0x9e94u);
    if (taken)
        goto finished;
}
first_visible_digit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e94u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e95u);
}
read_visible_digit: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0008u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x0008u >> 8) != ((0x0008u + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e95u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e98u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_digit_script_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e98u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e99u);
}
digit_script: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e99u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000au, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9e9bu);
}
store_digit_script: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819e9bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9e9du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spawn_digit: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819e9du);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819e9du, 0x20u, 0x819f39u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_next_digit_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ea0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ea2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_next_digit_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ea2u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ea3u);
}
next_digit_horizontal: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ea3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ea5u);
}
store_next_digit_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ea5u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ea7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_digit_phase: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ea7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ea9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_digit_phase_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ea9u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9eaau);
}
next_digit_phase: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eaau, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x0002u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9eacu);
}
store_digit_phase: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eacu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9eaeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_remaining_digits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eaeu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9eb0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_final_digit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eb0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9eb2u);
}
finish_digits: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eb2u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9eb9u : 0x9eb4u);
    if (taken)
        goto restore_digit_target;
}
count_visible_digits: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eb4u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9eb6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_visible_digit: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eb6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9eb7u);
}
repeat_digits: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eb7u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e95u);
    goto finished;
}
restore_digit_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819eb9u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9ebau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_digit_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ebau, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f26u);
    goto finished;
}
read_special_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ebcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ebeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_special_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ebeu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ebfu);
}
raise_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ebfu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ec1u);
}
store_special_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ec1u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ec3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_special_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ec3u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ec5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_special_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ec5u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ec6u);
}
offset_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ec6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ec8u);
}
store_special_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ec8u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ecau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819ecau, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9ecbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
next_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ecbu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9eccu);
}
special_script: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eccu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0016u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9eceu);
}
store_special_script: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eceu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ed0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spawn_special: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819ed0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819ed0u, 0x20u, 0x819f39u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819ed3u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9ed4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_special_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ed4u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f26u);
    goto finished;
}
read_miss_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ed6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ed8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_miss_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ed8u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ed9u);
}
raise_miss: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ed9u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9edbu);
}
store_miss_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819edbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9eddu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_miss_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eddu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9edfu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_miss_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819edfu, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ee0u);
}
offset_miss: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ee0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ee2u);
}
store_miss_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ee2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ee4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_miss_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819ee4u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9ee5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_miss_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ee5u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ee6u);
}
first_miss_script: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ee6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0014u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ee8u);
}
store_first_miss_script: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ee8u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9eeau);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spawn_first_miss: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819eeau);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819eeau, 0x20u, 0x819f39u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_second_miss_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eedu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9eefu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_second_miss_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819eefu, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ef0u);
}
offset_second_miss: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ef0u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ef2u);
}
store_second_miss_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ef2u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ef4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_miss_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ef4u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ef5u);
}
second_miss_script: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ef5u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0015u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ef7u);
}
store_second_miss_script: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ef7u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ef9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spawn_second_miss: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819ef9u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819ef9u, 0x20u, 0x819f39u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_miss_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819efcu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9efdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_miss_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819efdu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f26u);
    goto finished;
}
read_critical_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819effu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f01u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_critical_vertical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f01u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f02u);
}
raise_critical: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f02u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0008u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f04u);
}
store_critical_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f04u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f06u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_critical_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f06u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f08u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
clear_critical_horizontal_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f08u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f09u);
}
offset_critical: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f09u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000cu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f0bu);
}
store_critical_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f0bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f0du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_critical_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f0du, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9f0eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_critical_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f0eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f0fu);
}
first_critical_script: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f0fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0017u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f11u);
}
store_first_critical_script: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f11u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f13u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spawn_first_critical: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f13u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819f13u, 0x20u, 0x819f39u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_second_critical_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f16u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f18u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
set_second_critical_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f18u, 1u, span.instructions != 0u);
    registers->c = true;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f19u);
}
offset_second_critical: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f19u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe9u, 0x0010u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f1bu);
}
store_second_critical_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f1bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f1du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
second_critical_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f1du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, (uint16_t)(registers->x + 1u), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f1eu);
}
second_critical_script: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f1eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f20u);
}
store_second_critical_script: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f20u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f22u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
spawn_second_critical: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f22u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819f22u, 0x20u, 0x819f39u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_critical_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f25u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9f26u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_next_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f26u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f28u);
}
current_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f28u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f29u);
}
clear_next_target_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f29u, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f2au);
}
next_target_offset: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f2au, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000bu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f2du);
}
select_next_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f2du, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f2eu);
}
narrow_next_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f2eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f30u);
}
count_targets: {
    const uint32_t address = (uint16_t)(registers->dp + 0x02u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 5u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f30u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f32u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
finish_targets: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f32u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9f37u : 0x9f34u);
    if (taken)
        goto restore_target;
}
repeat_targets: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f34u, 3u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9e34u);
    goto finished;
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f37u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9f38u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_popups: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f38u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819f38u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattlePopupActorSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                               uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                               unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819f39u && pc < 0x819f8au)
        end_pc = 0x819f8au;
    else if (pc >= 0x819f8au && pc < 0x819f8bu)
        end_pc = 0x819f8bu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819f39u: byte_accumulator = true; break;
    case 0x819f3au: byte_accumulator = true; break;
    case 0x819f3du: width_neutral = true; break;
    case 0x819f6bu: width_neutral = true; break;
    case 0x819f6du: byte_accumulator = true; break;
    case 0x819f6fu: byte_accumulator = true; break;
    case 0x819f72u: byte_accumulator = true; break;
    case 0x819f74u: byte_accumulator = true; break;
    case 0x819f77u: byte_accumulator = true; break;
    case 0x819f79u: byte_accumulator = true; break;
    case 0x819f7cu: byte_accumulator = true; break;
    case 0x819f7eu: byte_accumulator = true; break;
    case 0x819f81u: byte_accumulator = true; break;
    case 0x819f83u: byte_accumulator = true; break;
    case 0x819f86u: byte_accumulator = true; break;
    case 0x819f89u: byte_accumulator = true; break;
    case 0x819f8au: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819f39u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 82u ||
        memcmp(cart->rom + first,
               "\xda\x20\x91\x8f\xc2\x20\x9b\xa5\x12\x29\xff\x00\x0a\xaa\xbf\x00\x82\x95\x99\x03\x00\xa5\x04"
               "\x29\xff\x00\x99\x1f\x00\xa5\x05\x29\xff\x00\x99\x21\x00\x7b\x99\x1b\x00\x99\x1d\x00\xa9\x80"
               "\x80\x99\x23\x00\xe2\x20\xa9\xff\x99\x25\x00\xa5\x14\x99\x28\x00\xa9\x01\x99\x00\x00\xa5\x13"
               "\x99\x02\x00\xa9\x01\x99\x01\x00\xee\xec\x1b\xfa\x60",
               82u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819f39u: goto save_source;
    case 0x819f3au: goto find_actor;
    case 0x819f3du: goto widen_script;
    case 0x819f3fu: goto select_actor;
    case 0x819f40u: goto read_script_index;
    case 0x819f42u: goto mask_script_index;
    case 0x819f45u: goto double_script_index;
    case 0x819f46u: goto select_script_index;
    case 0x819f47u: goto read_script_address;
    case 0x819f4bu: goto store_script_address;
    case 0x819f4eu: goto read_horizontal;
    case 0x819f50u: goto mask_horizontal;
    case 0x819f53u: goto store_horizontal;
    case 0x819f56u: goto read_vertical;
    case 0x819f58u: goto mask_vertical;
    case 0x819f5bu: goto store_vertical;
    case 0x819f5eu: goto zero_speed;
    case 0x819f5fu: goto store_horizontal_speed;
    case 0x819f62u: goto store_vertical_speed;
    case 0x819f65u: goto default_adjustment;
    case 0x819f68u: goto store_adjustment;
    case 0x819f6bu: goto narrow_actor;
    case 0x819f6du: goto disabled_target;
    case 0x819f6fu: goto store_target;
    case 0x819f72u: goto read_attributes;
    case 0x819f74u: goto store_attributes;
    case 0x819f77u: goto enable_actor;
    case 0x819f79u: goto store_enabled;
    case 0x819f7cu: goto read_phase;
    case 0x819f7eu: goto store_phase;
    case 0x819f81u: goto first_frame;
    case 0x819f83u: goto store_frame;
    case 0x819f86u: goto count_active;
    case 0x819f89u: goto restore_source;
    case 0x819f8au: goto return_actor;
    default: return 0;
    }
save_source: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f39u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9f3au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
find_actor: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f3au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819f3au, 0x20u, 0x818f91u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_script: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f3du, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f3fu);
}
select_actor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f3fu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->x, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f40u);
}
read_script_index: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f40u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f42u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_script_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f42u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f45u);
}
double_script_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f45u, 1u, span.instructions != 0u);
    registers->c = (registers->a & 0x8000u) != 0u;
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a << 1) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f46u);
}
select_script_index: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f46u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f47u);
}
read_script_address: {
    const uint32_t address = (0x958200u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f47u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f4bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_script_address: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f4bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f4eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_horizontal: {
    const uint32_t address = (uint16_t)(registers->dp + 0x04u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f4eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f50u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_horizontal: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f50u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f53u);
}
store_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001fu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f53u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f56u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_vertical: {
    const uint32_t address = (uint16_t)(registers->dp + 0x05u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f56u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f58u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_vertical: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f58u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f5bu);
}
store_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0021u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f5bu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
zero_speed: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f5eu, 1u, span.instructions != 0u);
    registers->a = registers->dp;
    registers->z = registers->a == 0u;
    registers->n = (registers->a & 0x8000u) != 0u;
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f5fu);
}
store_horizontal_speed: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001bu + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f5fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f62u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_vertical_speed: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001du + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f62u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f65u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
default_adjustment: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f65u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x8080u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f68u);
}
store_adjustment: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0023u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f68u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f6bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_actor: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f6bu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f6du);
}
disabled_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f6du, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f6fu);
}
store_target: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f6fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f72u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_attributes: {
    const uint32_t address = (uint16_t)(registers->dp + 0x14u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f72u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f74u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_attributes: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0028u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f74u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f77u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
enable_actor: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f77u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f79u);
}
store_enabled: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f79u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f7cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_phase: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f7cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f7eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_phase: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f7eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f81u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_frame: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f81u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f83u);
}
store_frame: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + 1u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f83u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f86u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_active: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1becu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f86u, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xeeu, address, high, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f89u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_source: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f89u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9f8au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_actor: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f8au);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819f8au, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleConditionalSoundSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                     uint8_t opcode, uint64_t dynamic_epoch,
                                                     uint64_t write_epoch, unsigned step_budget,
                                                     long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819f8bu && pc < 0x819faau)
        end_pc = 0x819faau;
    else if (pc >= 0x819faau && pc < 0x819fabu)
        end_pc = 0x819fabu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819f8bu: byte_accumulator = true; break;
    case 0x819f8du: byte_accumulator = true; break;
    case 0x819f8eu: byte_accumulator = true; break;
    case 0x819f8fu: byte_accumulator = true; break;
    case 0x819f92u: byte_accumulator = true; break;
    case 0x819f95u: byte_accumulator = true; break;
    case 0x819f97u: byte_accumulator = true; break;
    case 0x819f9au: byte_accumulator = true; break;
    case 0x819f9cu: byte_accumulator = true; break;
    case 0x819f9eu: byte_accumulator = true; break;
    case 0x819f9fu: byte_accumulator = true; break;
    case 0x819fa3u: byte_accumulator = true; break;
    case 0x819fa4u: width_neutral = true; break;
    case 0x819fa8u: width_neutral = true; break;
    case 0x819faau: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819f8bu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 32u ||
        memcmp(cart->rom + first,
               "\xa9\x7e\x48\xab\x20\xab\x8f\xe0\xe7\x4f\x90\x0d\xad\xde\x11\xd0\x08\xa7\xc3\x5a\x22\x3b\x95"
               "\x80\x7a\xc2\x20\xe6\xc3\xe2\x20\x60",
               32u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819f8bu: goto target_bank;
    case 0x819f8du: goto push_bank;
    case 0x819f8eu: goto select_bank;
    case 0x819f8fu: goto find_slot;
    case 0x819f92u: goto test_capacity;
    case 0x819f95u: goto skip_sound;
    case 0x819f97u: goto read_override;
    case 0x819f9au: goto skip_override;
    case 0x819f9cu: goto read_command;
    case 0x819f9eu: goto save_target;
    case 0x819f9fu: goto send_command;
    case 0x819fa3u: goto restore_target;
    case 0x819fa4u: goto widen_stream;
    case 0x819fa6u: goto advance_stream;
    case 0x819fa8u: goto narrow_command;
    case 0x819faau: goto return_command;
    default: return 0;
    }
target_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f8bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9f8du);
}
push_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f8du, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f8eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f8eu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9f8fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
find_slot: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f8fu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819f8fu, 0x20u, 0x818fabu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
test_capacity: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f92u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xe0u, 0x4fe7u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9f95u);
}
skip_sound: {
    const bool taken = !registers->c;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f95u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9fa4u : 0x9f97u);
    if (taken)
        goto widen_stream;
}
read_override: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x11deu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f97u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f9au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_override: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f9au, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9fa4u : 0x9f9cu);
    if (taken)
        goto widen_stream;
}
read_command: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 6u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819f9cu, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9f9eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819f9eu, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9f9fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
send_command: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819f9fu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819f9fu, 0x22u, 0x80953bu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819fa3u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9fa4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fa4u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9fa6u);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fa6u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fa8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_command: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fa8u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9faau);
    goto finished;
}
return_command: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819faau);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819faau, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleListedTargetScriptsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                        uint8_t opcode, uint64_t dynamic_epoch,
                                                        uint64_t write_epoch, unsigned step_budget,
                                                        long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819fabu && pc < 0x81a039u)
        end_pc = 0x81a039u;
    else if (pc >= 0x81a039u && pc < 0x81a03au)
        end_pc = 0x81a03au;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819fabu: byte_accumulator = true; break;
    case 0x819fadu: byte_accumulator = true; break;
    case 0x819faeu: byte_accumulator = true; break;
    case 0x819fafu: width_neutral = true; break;
    case 0x819fdbu: width_neutral = true; break;
    case 0x819fddu: byte_accumulator = true; break;
    case 0x819fe0u: width_neutral = true; break;
    case 0x81a037u: width_neutral = true; break;
    case 0x81a039u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819fabu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 143u ||
        memcmp(
            cart->rom + first,
            "\xa9\x7e\x48\xab\xc2\x20\xa7\xc3\x85\x00\xe6\xc3\xe6\xc3\xad\x0a\x4f\x29\xff\x00\xf0\x76\x85\x02"
            "\xb9\x13\x00\x85\x11\xb9\x15\x00\x85\x13\xb9\x17\x00\x85\x15\xb9\x19\x00\x85\x17\x5a\xa0\x0b\x4f"
            "\xe2\x20\x20\xab\x8f\xc2\x20\xa5\x00\x9d\x03\x00\xa5\x11\x9d\x13\x00\xa5\x13\x9d\x15\x00\xa5\x15"
            "\x9d\x17\x00\xa5\x17\x9d\x19\x00\xb9\x00\x00\x29\xff\x00\x9d\x1f\x00\xb9\x01\x00\x29\xff\x00\x9d"
            "\x21\x00\x9e\x1b\x00\x9e\x1d\x00\xb9\x02\x00\x9d\x23\x00\xa9\x01\x01\x9d\x00\x00\x9d\x01\x00\xb9"
            "\x0a\x00\x3a\x9d\x25\x00\x98\x18\x69\x0b\x00\xa8\xee\xec\x1b\xc6\x02\xd0\xa5\x7a\xe2\x20\x60",
            143u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819fabu: goto target_bank;
    case 0x819fadu: goto push_bank;
    case 0x819faeu: goto select_bank;
    case 0x819fafu: goto widen_script;
    case 0x819fb1u: goto read_script_address;
    case 0x819fb3u: goto store_script_address;
    case 0x819fb5u: goto advance_stream_low;
    case 0x819fb7u: goto advance_stream_high;
    case 0x819fb9u: goto read_target_count;
    case 0x819fbcu: goto mask_target_count;
    case 0x819fbfu: goto skip_empty_targets;
    case 0x819fc1u: goto store_target_count;
    case 0x819fc3u: goto read_parameter_one;
    case 0x819fc6u: goto save_parameter_one;
    case 0x819fc8u: goto read_parameter_two;
    case 0x819fcbu: goto save_parameter_two;
    case 0x819fcdu: goto read_parameter_three;
    case 0x819fd0u: goto save_parameter_three;
    case 0x819fd2u: goto read_parameter_four;
    case 0x819fd5u: goto save_parameter_four;
    case 0x819fd7u: goto save_target;
    case 0x819fd8u: goto first_target;
    case 0x819fdbu: goto narrow_slot;
    case 0x819fddu: goto find_slot;
    case 0x819fe0u: goto widen_slot;
    case 0x819fe2u: goto read_script_address_for_slot;
    case 0x819fe4u: goto store_script_address_for_slot;
    case 0x819fe7u: goto read_saved_parameter_one;
    case 0x819fe9u: goto store_parameter_one;
    case 0x819fecu: goto read_saved_parameter_two;
    case 0x819feeu: goto store_parameter_two;
    case 0x819ff1u: goto read_saved_parameter_three;
    case 0x819ff3u: goto store_parameter_three;
    case 0x819ff6u: goto read_saved_parameter_four;
    case 0x819ff8u: goto store_parameter_four;
    case 0x819ffbu: goto read_horizontal;
    case 0x819ffeu: goto mask_horizontal;
    case 0x81a001u: goto store_horizontal;
    case 0x81a004u: goto read_vertical;
    case 0x81a007u: goto mask_vertical;
    case 0x81a00au: goto store_vertical;
    case 0x81a00du: goto zero_horizontal_speed;
    case 0x81a010u: goto zero_vertical_speed;
    case 0x81a013u: goto read_adjustment;
    case 0x81a016u: goto store_adjustment;
    case 0x81a019u: goto initial_state;
    case 0x81a01cu: goto store_enabled_and_frame;
    case 0x81a01fu: goto store_frame_and_phase;
    case 0x81a022u: goto read_target;
    case 0x81a025u: goto zero_base_target;
    case 0x81a026u: goto store_target;
    case 0x81a029u: goto current_target;
    case 0x81a02au: goto clear_target_carry;
    case 0x81a02bu: goto next_target;
    case 0x81a02eu: goto select_next_target;
    case 0x81a02fu: goto count_active;
    case 0x81a032u: goto count_targets;
    case 0x81a034u: goto repeat_targets;
    case 0x81a036u: goto restore_target;
    case 0x81a037u: goto narrow_result;
    case 0x81a039u: goto return_scripts;
    default: return 0;
    }
target_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fabu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x007eu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9fadu);
}
push_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819fadu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9faeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819faeu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9fafu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_script: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fafu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9fb1u);
}
read_script_address: {
    const uint32_t pointer_low = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t pointer_high = (uint16_t)(pointer_low + 1u);
    const uint32_t pointer_bank = (uint16_t)(pointer_low + 2u);
    if (!Lufia2DecompIsWramByte(pointer_low) || !Lufia2DecompIsWramByte(pointer_high) ||
        !Lufia2DecompIsWramByte(pointer_bank))
        goto finished;
    const uint32_t expected = cpu->ram[pointer_low] | ((uint32_t)cpu->ram[pointer_high] << 8) |
                              ((uint32_t)cpu->ram[pointer_bank] << 16);
    const uint32_t address = (expected) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fb1u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fb3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_script_address: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fb3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fb5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_stream_low: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fb5u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fb7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
advance_stream_high: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fb7u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fb9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_target_count: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4f0au) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fb9u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fbcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_target_count: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fbcu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9fbfu);
}
skip_empty_targets: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fbfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0xa037u : 0x9fc1u);
    if (taken)
        goto narrow_result;
}
store_target_count: {
    const uint32_t address = (uint16_t)(registers->dp + 0x02u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fc1u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fc3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_parameter_one: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0013u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0013u >> 8) != ((0x0013u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fc3u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fc6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_parameter_one: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fc6u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fc8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_parameter_two: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0015u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0015u >> 8) != ((0x0015u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fc8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fcbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_parameter_two: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fcbu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fcdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_parameter_three: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0017u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0017u >> 8) != ((0x0017u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fcdu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fd0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_parameter_three: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fd0u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fd2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_parameter_four: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0019u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0019u >> 8) != ((0x0019u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fd2u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fd5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_parameter_four: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fd5u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fd7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819fd7u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9fd8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
first_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fd8u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, 0x4f0bu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9fdbu);
}
narrow_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fdbu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9fddu);
}
find_slot: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819fddu);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819fddu, 0x20u, 0x818fabu, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
widen_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fe0u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9fe2u);
}
read_script_address_for_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0x00u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fe2u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fe4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_script_address_for_slot: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0003u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fe4u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fe7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_saved_parameter_one: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fe7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fe9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_parameter_one: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0013u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fe9u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9fecu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_saved_parameter_two: {
    const uint32_t address = (uint16_t)(registers->dp + 0x13u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819fecu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9feeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_parameter_two: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0015u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819feeu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ff1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_saved_parameter_three: {
    const uint32_t address = (uint16_t)(registers->dp + 0x15u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ff1u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ff3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_parameter_three: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0017u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ff3u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ff6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_saved_parameter_four: {
    const uint32_t address = (uint16_t)(registers->dp + 0x17u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 4u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ff6u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ff8u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_parameter_four: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0019u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ff8u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ffbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0000u >> 8) != ((0x0000u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ffbu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ffeu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_horizontal: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ffeu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa001u);
}
store_horizontal: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001fu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a001u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa004u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0001u >> 8) != ((0x0001u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a004u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa007u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
mask_vertical: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a007u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x00ffu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa00au);
}
store_vertical: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0021u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a00au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa00du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
zero_horizontal_speed: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001bu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a00du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa010u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
zero_vertical_speed: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x001du + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a010u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, 0u, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa013u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_adjustment: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0002u + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x0002u >> 8) != ((0x0002u + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a013u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa016u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_adjustment: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0023u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a016u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa019u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
initial_state: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a019u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0101u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa01cu);
}
store_enabled_and_frame: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0000u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a01cu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa01fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_frame_and_phase: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0001u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a01fu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa022u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
read_target: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000au + registers->y) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u + ((0x000au >> 8) != ((0x000au + registers->y) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address) ||
        !Lufia2DecompIsDecompressSourceByte(cart, high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a022u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, true), false);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa025u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
zero_base_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a025u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, (uint16_t)((registers->a - 1u) & 0xffffu), false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa026u);
}
store_target: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 7u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a026u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa029u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
current_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a029u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa02au);
}
clear_target_carry: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a02au, 1u, span.instructions != 0u);
    registers->c = false;
    NativeSpanStep(&span, registers, cpu, 2u, 0xa02bu);
}
next_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a02bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x69u, 0x000bu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa02eu);
}
select_next_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a02eu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0xa02fu);
}
count_active: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x1becu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 8u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a02fu, 3u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xeeu, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa032u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
count_targets: {
    const uint32_t address = (uint16_t)(registers->dp + 0x02u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a032u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xc6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0xa034u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
repeat_targets: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a034u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9fdbu : 0xa036u);
    if (taken)
        goto finished;
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x81a036u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0xa037u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_result: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x81a037u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xa039u);
    goto finished;
}
return_scripts: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x81a039u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x81a039u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleEffectPopupSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x819dc9u && pc < 0x819df6u)
        return Lufia2DecompRunBattleRestorePortraitSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x819e13u && pc < 0x819e22u)
        return Lufia2DecompRunBattleSendSoundSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x819e22u && pc < 0x819f39u)
        return Lufia2DecompRunBattleTargetPopupsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x819f39u && pc < 0x819f8bu)
        return Lufia2DecompRunBattlePopupActorSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x819f8bu && pc < 0x819fabu)
        return Lufia2DecompRunBattleConditionalSoundSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    if (pc >= 0x819fabu && pc < 0x81a03au)
        return Lufia2DecompRunBattleListedTargetScriptsSpan(registers, cpu, pc, opcode, dynamic_epoch, write_epoch,
            step_budget, step_count);
    return 0;
}
