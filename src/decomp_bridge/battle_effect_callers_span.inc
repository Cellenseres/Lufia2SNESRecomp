#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBattlePortraitPoseSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819b56u && pc < 0x819b7du)
        end_pc = 0x819b7du;
    else if (pc >= 0x819b7du && pc < 0x819b7eu)
        end_pc = 0x819b7eu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819b56u: byte_accumulator = true; break;
    case 0x819b58u: byte_accumulator = true; break;
    case 0x819b5au: width_neutral = true; break;
    case 0x819b5eu: width_neutral = true; break;
    case 0x819b60u: byte_accumulator = true; break;
    case 0x819b61u: byte_accumulator = true; break;
    case 0x819b65u: byte_accumulator = true; break;
    case 0x819b67u: byte_accumulator = true; break;
    case 0x819b69u: byte_accumulator = true; break;
    case 0x819b6bu: byte_accumulator = true; break;
    case 0x819b6du: byte_accumulator = true; break;
    case 0x819b6eu: byte_accumulator = true; break;
    case 0x819b6fu: byte_accumulator = true; break;
    case 0x819b71u: byte_accumulator = true; break;
    case 0x819b72u: byte_accumulator = true; break;
    case 0x819b73u: byte_accumulator = true; break;
    case 0x819b76u: byte_accumulator = true; break;
    case 0x819b77u: byte_accumulator = true; break;
    case 0x819b78u: byte_accumulator = true; break;
    case 0x819b7au: byte_accumulator = true; break;
    case 0x819b7du: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819b56u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 40u ||
        memcmp(cart->rom + first,
               "\xa7\xc3\x85\x12\xc2\x20\xe6\xc3\xe2\x20\xbb\xbf\x25\x00\x7e\x30\x16\xc9\x04\xf0\x12\x85\x11"
               "\x5a\x8b\xa9\x97\x48\xab\x20\x75\xbb\xab\x7a\xa9\xff\x8d\xb0\x15\x60",
               40u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819b56u: goto read_pose;
    case 0x819b58u: goto store_pose;
    case 0x819b5au: goto widen_stream;
    case 0x819b5cu: goto advance_stream;
    case 0x819b5eu: goto narrow_pose;
    case 0x819b60u: goto select_target;
    case 0x819b61u: goto read_target;
    case 0x819b65u: goto skip_enemy;
    case 0x819b67u: goto test_special;
    case 0x819b69u: goto skip_special;
    case 0x819b6bu: goto store_slot;
    case 0x819b6du: goto save_target;
    case 0x819b6eu: goto save_bank;
    case 0x819b6fu: goto pose_bank;
    case 0x819b71u: goto push_bank;
    case 0x819b72u: goto select_bank;
    case 0x819b73u: goto call_pose;
    case 0x819b76u: goto restore_bank;
    case 0x819b77u: goto restore_target;
    case 0x819b78u: goto status_request;
    case 0x819b7au: goto store_request;
    case 0x819b7du: goto return_pose;
    default: return 0;
    }
read_pose: {
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
    Lufia2SpanFetch(registers, 0x819b56u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b58u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_pose: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b58u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b5au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b5au, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9b5cu);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b5cu, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b5eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_pose: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b5eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9b60u);
}
select_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b60u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9b61u);
}
read_target: {
    const uint32_t address = (0x7e0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b61u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b65u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_enemy: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b65u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9b7du : 0x9b67u);
    if (taken)
        goto finished;
}
test_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b67u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9b69u);
}
skip_special: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b69u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9b7du : 0x9b6bu);
    if (taken)
        goto finished;
}
store_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b6bu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b6du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b6du, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9b6eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b6eu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9b6fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
pose_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b6fu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0097u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9b71u);
}
push_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b71u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9b72u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b72u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9b73u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_pose: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819b73u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819b73u, 0x20u, 0x81bb75u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b76u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9b77u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b77u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9b78u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
status_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b78u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9b7au);
}
store_request: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15b0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b7au, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b7du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_pose: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819b7du);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819b7du, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleEffectFrameSetupSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                     uint8_t opcode, uint64_t dynamic_epoch,
                                                     uint64_t write_epoch, unsigned step_budget,
                                                     long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819b7eu && pc < 0x819ba2u)
        end_pc = 0x819ba2u;
    else if (pc >= 0x819ba2u && pc < 0x819ba3u)
        end_pc = 0x819ba3u;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819b7eu: byte_accumulator = true; break;
    case 0x819b80u: width_neutral = true; break;
    case 0x819b84u: width_neutral = true; break;
    case 0x819b86u: byte_accumulator = true; break;
    case 0x819b87u: byte_accumulator = true; break;
    case 0x819b8au: byte_accumulator = true; break;
    case 0x819b8eu: byte_accumulator = true; break;
    case 0x819b90u: byte_accumulator = true; break;
    case 0x819b93u: byte_accumulator = true; break;
    case 0x819b95u: byte_accumulator = true; break;
    case 0x819b99u: byte_accumulator = true; break;
    case 0x819b9du: byte_accumulator = true; break;
    case 0x819ba1u: byte_accumulator = true; break;
    case 0x819ba2u: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819b7eu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 37u ||
        memcmp(cart->rom + first,
               "\xa7\xc3\xc2\x20\xe6\xc3\xe2\x20\x5a\x8d\xab\x15\x22\x39\x8a\x85\xa9\xff\x8d\xae\x15\xa9\xff"
               "\x8f\xf3\x12\x00\x22\xaf\xb9\x81\x22\x81\xec\x85\x7a\x60",
               37u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819b7eu: goto read_frame;
    case 0x819b80u: goto widen_stream;
    case 0x819b82u: goto advance_stream;
    case 0x819b84u: goto narrow_frame;
    case 0x819b86u: goto save_target;
    case 0x819b87u: goto store_frame;
    case 0x819b8au: goto call_frame;
    case 0x819b8eu: goto status_request;
    case 0x819b90u: goto store_rebuild;
    case 0x819b93u: goto frame_marker;
    case 0x819b95u: goto store_marker;
    case 0x819b99u: goto call_prepare;
    case 0x819b9du: goto call_update;
    case 0x819ba1u: goto restore_target;
    case 0x819ba2u: goto return_frame;
    default: return 0;
    }
read_frame: {
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
    Lufia2SpanFetch(registers, 0x819b7eu, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b80u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b80u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9b82u);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b82u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b84u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_frame: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b84u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9b86u);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819b86u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9b87u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_frame: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15abu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b87u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b8au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_frame: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819b8au);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819b8au, 0x22u, 0x858a39u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
status_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b8eu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9b90u);
}
store_rebuild: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15aeu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b90u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9b93u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
frame_marker: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b93u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9b95u);
}
store_marker: {
    const uint32_t address = 0x0012f3u;
    const uint32_t high = address + 1u;
    if (!NativeSpanCanStep(&span, cpu, 5u) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819b95u, 4u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9b99u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_prepare: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819b99u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819b99u, 0x22u, 0x81b9afu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
call_update: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819b9du);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819b9du, 0x22u, 0x85ec81u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819ba1u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9ba2u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_frame: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819ba2u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819ba2u, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleEffectSkipArgumentSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                       uint8_t opcode, uint64_t dynamic_epoch,
                                                       uint64_t write_epoch, unsigned step_budget,
                                                       long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819ba3u && pc < 0x819babu)
        end_pc = 0x819babu;
    else if (pc >= 0x819babu && pc < 0x819bacu)
        end_pc = 0x819bacu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819ba3u: byte_accumulator = true; break;
    case 0x819ba5u: width_neutral = true; break;
    case 0x819ba9u: width_neutral = true; break;
    case 0x819babu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819ba3u;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 9u || memcmp(cart->rom + first, "\xa7\xc3\xc2\x20\xe6\xc3\xe2\x20\x60", 9u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819ba3u: goto read_argument;
    case 0x819ba5u: goto widen_stream;
    case 0x819ba7u: goto advance_stream;
    case 0x819ba9u: goto narrow_argument;
    case 0x819babu: goto return_argument;
    default: return 0;
    }
read_argument: {
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
    Lufia2SpanFetch(registers, 0x819ba3u, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ba5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ba5u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ba7u);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ba7u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ba9u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_argument: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ba9u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9babu);
    goto finished;
}
return_argument: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819babu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819babu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattlePartyTargetToggleSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                      uint8_t opcode, uint64_t dynamic_epoch,
                                                      uint64_t write_epoch, unsigned step_budget,
                                                      long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819bacu && pc < 0x819bbdu)
        end_pc = 0x819bbdu;
    else if (pc >= 0x819bbdu && pc < 0x819bbeu)
        end_pc = 0x819bbeu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819bacu: byte_accumulator = true; break;
    case 0x819badu: byte_accumulator = true; break;
    case 0x819bb1u: byte_accumulator = true; break;
    case 0x819bb3u: byte_accumulator = true; break;
    case 0x819bb5u: byte_accumulator = true; break;
    case 0x819bb6u: byte_accumulator = true; break;
    case 0x819bb7u: byte_accumulator = true; break;
    case 0x819bbbu: byte_accumulator = true; break;
    case 0x819bbcu: byte_accumulator = true; break;
    case 0x819bbdu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819bacu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 18u ||
        memcmp(cart->rom + first, "\xbb\xbf\x25\x00\x7e\x10\x0a\x85\x11\x5a\x8b\x22\x0e\x92\x85\xab\x7a\x60",
               18u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819bacu: goto select_target;
    case 0x819badu: goto read_target;
    case 0x819bb1u: goto skip_party;
    case 0x819bb3u: goto store_slot;
    case 0x819bb5u: goto save_target;
    case 0x819bb6u: goto save_bank;
    case 0x819bb7u: goto call_toggle;
    case 0x819bbbu: goto restore_bank;
    case 0x819bbcu: goto restore_target;
    case 0x819bbdu: goto return_toggle;
    default: return 0;
    }
select_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bacu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9badu);
}
read_target: {
    const uint32_t address = (0x7e0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819badu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bb1u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_party: {
    const bool taken = !registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bb1u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9bbdu : 0x9bb3u);
    if (taken)
        goto finished;
}
store_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819bb3u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9bb5u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bb5u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9bb6u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bb6u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9bb7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_toggle: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819bb7u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819bb7u, 0x22u, 0x85920eu, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bbbu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9bbcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819bbcu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9bbdu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_toggle: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819bbdu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819bbdu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattlePortraitStatusSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                   uint8_t opcode, uint64_t dynamic_epoch,
                                                   uint64_t write_epoch, unsigned step_budget,
                                                   long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819cccu && pc < 0x819d0au)
        end_pc = 0x819d0au;
    else if (pc >= 0x819d0au && pc < 0x819d0bu)
        end_pc = 0x819d0bu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819cccu: byte_accumulator = true; break;
    case 0x819cceu: byte_accumulator = true; break;
    case 0x819cd0u: width_neutral = true; break;
    case 0x819cd4u: width_neutral = true; break;
    case 0x819cd6u: byte_accumulator = true; break;
    case 0x819cd7u: byte_accumulator = true; break;
    case 0x819cdbu: byte_accumulator = true; break;
    case 0x819cddu: byte_accumulator = true; break;
    case 0x819cdfu: byte_accumulator = true; break;
    case 0x819ce1u: byte_accumulator = true; break;
    case 0x819ce3u: byte_accumulator = true; break;
    case 0x819ce4u: width_neutral = true; break;
    case 0x819ceau: width_neutral = true; break;
    case 0x819cecu: byte_accumulator = true; break;
    case 0x819cf0u: byte_accumulator = true; break;
    case 0x819cf4u: byte_accumulator = true; break;
    case 0x819cf7u: byte_accumulator = true; break;
    case 0x819cf9u: byte_accumulator = true; break;
    case 0x819cfbu: byte_accumulator = true; break;
    case 0x819cfcu: byte_accumulator = true; break;
    case 0x819cfeu: byte_accumulator = true; break;
    case 0x819cffu: byte_accumulator = true; break;
    case 0x819d00u: byte_accumulator = true; break;
    case 0x819d03u: byte_accumulator = true; break;
    case 0x819d04u: byte_accumulator = true; break;
    case 0x819d06u: byte_accumulator = true; break;
    case 0x819d09u: byte_accumulator = true; break;
    case 0x819d0au: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819cccu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 63u ||
        memcmp(cart->rom + first,
               "\xa7\xc3\x85\x12\xc2\x20\xe6\xc3\xe2\x20\xbb\xbf\x25\x00\x7e\x30\x2d\xc9\x04\xf0\x29\x85\x11"
               "\x5a\xc2\x20\x29\x7f\x00\xaa\xe2\x20\xbf\xec\xff\x96\x22\xb5\xb2\x81\xbd\x0f\x00\x89\x2d\xd0"
               "\x0e\x8b\xa9\x97\x48\xab\x20\x75\xbb\xab\xa9\xff\x8d\xb0\x15\x7a\x60",
               63u))
        return 0;
    const unsigned direct_penalty = (registers->dp & 0xffu) != 0u;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819cccu: goto read_pose;
    case 0x819cceu: goto store_pose;
    case 0x819cd0u: goto widen_stream;
    case 0x819cd2u: goto advance_stream;
    case 0x819cd4u: goto narrow_pose;
    case 0x819cd6u: goto select_target;
    case 0x819cd7u: goto read_target;
    case 0x819cdbu: goto skip_enemy;
    case 0x819cddu: goto test_special;
    case 0x819cdfu: goto skip_special;
    case 0x819ce1u: goto store_slot;
    case 0x819ce3u: goto save_target;
    case 0x819ce4u: goto widen_mask;
    case 0x819ce6u: goto mask_slot;
    case 0x819ce9u: goto select_slot;
    case 0x819ceau: goto narrow_mask;
    case 0x819cecu: goto read_mask;
    case 0x819cf0u: goto call_battler;
    case 0x819cf4u: goto read_status;
    case 0x819cf7u: goto test_status;
    case 0x819cf9u: goto skip_suppressed;
    case 0x819cfbu: goto save_bank;
    case 0x819cfcu: goto pose_bank;
    case 0x819cfeu: goto push_bank;
    case 0x819cffu: goto select_bank;
    case 0x819d00u: goto call_pose;
    case 0x819d03u: goto restore_bank;
    case 0x819d04u: goto status_request;
    case 0x819d06u: goto store_request;
    case 0x819d09u: goto restore_target;
    case 0x819d0au: goto return_pose;
    default: return 0;
    }
read_pose: {
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
    Lufia2SpanFetch(registers, 0x819cccu, 2u, span.instructions != 0u);
    uint32_t pointer = Lufia2DecompReadData(registers, pointer_low, pointer_high, true);
    pointer |= (uint32_t)registers->read(registers->mem, pointer_bank) << 16;
    const uint32_t source = (pointer) & 0xffffffu;
    Lufia2DecompApplyOperand(registers, 0xa9u,
                             Lufia2DecompReadData(registers, source, (source + 1u) & 0xffffffu, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cceu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
store_pose: {
    const uint32_t address = (uint16_t)(registers->dp + 0x12u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cceu, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cd0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_stream: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cd0u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9cd2u);
}
advance_stream: {
    const uint32_t address = (uint16_t)(registers->dp + 0xc3u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 7u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address) ||
        !Lufia2DecompIsWramByte(high))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cd2u, 2u, span.instructions != 0u);
    Lufia2DecompModifyWram(registers, 0xe6u, address, high, true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cd4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
narrow_pose: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cd4u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9cd6u);
}
select_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cd6u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cd7u);
}
read_target: {
    const uint32_t address = (0x7e0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cd7u, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cdbu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_enemy: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cdbu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9d0au : 0x9cddu);
    if (taken)
        goto finished;
}
test_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cddu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cdfu);
}
skip_special: {
    const bool taken = registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cdfu, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9d0au : 0x9ce1u);
    if (taken)
        goto finished;
}
store_slot: {
    const uint32_t address = (uint16_t)(registers->dp + 0x11u);
    const uint32_t high = (uint16_t)(address + 1u);
    const unsigned cycles = 3u + direct_penalty;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ce1u, 2u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9ce3u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819ce3u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9ce4u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
widen_mask: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ce4u, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ce6u);
}
mask_slot: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ce6u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x29u, 0x007fu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9ce9u);
}
select_slot: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ce9u, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->a, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9ceau);
}
narrow_mask: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819ceau, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9cecu);
}
read_mask: {
    const uint32_t address = (0x96ffecu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cecu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cf0u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_battler: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819cf0u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819cf0u, 0x22u, 0x81b2b5u, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
read_status: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x000fu + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + ((0x000fu >> 8) != ((0x000fu + registers->x) >> 8));
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cf4u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9cf7u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
test_status: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cf7u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0x89u, 0x002du, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cf9u);
}
skip_suppressed: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cf9u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9d09u : 0x9cfbu);
    if (taken)
        goto restore_target;
}
save_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819cfbu, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9cfcu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
pose_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819cfcu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0097u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9cfeu);
}
push_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819cfeu, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9cffu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
select_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819cffu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9d00u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_pose: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819d00u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819d00u, 0x20u, 0x81bb75u, false, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819d03u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9d04u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
status_request: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d04u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00ffu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9d06u);
}
store_request: {
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x15b0u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsWramByte(address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d06u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, false);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9d09u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819d09u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9d0au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_pose: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819d0au);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819d0au, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleSpecialTargetToggleSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                        uint8_t opcode, uint64_t dynamic_epoch,
                                                        uint64_t write_epoch, unsigned step_budget,
                                                        long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x819d0bu && pc < 0x819d1eu)
        end_pc = 0x819d1eu;
    else if (pc >= 0x819d1eu && pc < 0x819d1fu)
        end_pc = 0x819d1fu;
    else
        return 0;
    bool width_neutral = false;
    bool byte_accumulator = false;
    switch (pc) {
    case 0x819d0bu: byte_accumulator = true; break;
    case 0x819d0cu: byte_accumulator = true; break;
    case 0x819d10u: byte_accumulator = true; break;
    case 0x819d12u: byte_accumulator = true; break;
    case 0x819d14u: byte_accumulator = true; break;
    case 0x819d16u: byte_accumulator = true; break;
    case 0x819d17u: byte_accumulator = true; break;
    case 0x819d18u: byte_accumulator = true; break;
    case 0x819d1cu: byte_accumulator = true; break;
    case 0x819d1du: byte_accumulator = true; break;
    case 0x819d1eu: width_neutral = true; break;
    default: break;
    }
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || registers->e || registers->xf ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted || (!width_neutral && registers->mf != byte_accumulator) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    const unsigned delta = pc - 0x819d0bu;
    if (!cart || offset < delta)
        return 0;
    const unsigned first = offset - delta;
    if (cart->romSize < first + 20u ||
        memcmp(cart->rom + first,
               "\xbb\xbf\x25\x00\x7e\x30\x0c\xc9\x04\xd0\x08\x5a\x8b\x22\x2d\x92\x85\xab\x7a\x60", 20u))
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x819d0bu: goto select_target;
    case 0x819d0cu: goto read_target;
    case 0x819d10u: goto skip_enemy;
    case 0x819d12u: goto test_special;
    case 0x819d14u: goto skip_other;
    case 0x819d16u: goto save_target;
    case 0x819d17u: goto save_bank;
    case 0x819d18u: goto call_toggle;
    case 0x819d1cu: goto restore_bank;
    case 0x819d1du: goto restore_target;
    case 0x819d1eu: goto return_toggle;
    default: return 0;
    }
select_target: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d0bu, 1u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, registers->y, false);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9d0cu);
}
read_target: {
    const uint32_t address = (0x7e0025u + registers->x) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 5u;
    if (!NativeSpanCanStep(&span, cpu, cycles) || !Lufia2DecompIsDecompressSourceByte(cart, address))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d0cu, 4u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, Lufia2DecompReadData(registers, address, high, false), true);
    NativeSpanStep(&span, registers, cpu, cycles, 0x9d10u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
skip_enemy: {
    const bool taken = registers->n;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d10u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9d1eu : 0x9d12u);
    if (taken)
        goto finished;
}
test_special: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d12u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xc9u, 0x0004u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0x9d14u);
}
skip_other: {
    const bool taken = !registers->z;
    const unsigned cycles = taken ? 3u : 2u;
    if (!NativeSpanCanStep(&span, cpu, cycles))
        goto finished;
    Lufia2SpanFetch(registers, 0x819d14u, 2u, span.instructions != 0u);
    NativeSpanStep(&span, registers, cpu, cycles, taken ? 0x9d1eu : 0x9d16u);
    if (taken)
        goto finished;
}
save_target: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f02u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819d16u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    registers->write(registers->mem, registers->sp--, (uint8_t)(value >> 8));
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 4u, 0x9d17u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
save_bank: {
    if (!NativeSpanCanStep(&span, cpu, 3u) || registers->sp < 0x1f01u || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819d17u, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    registers->write(registers->mem, registers->sp--, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, 3u, 0x9d18u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
call_toggle: {
    if (!NativeSpanCanStep(&span, cpu, 8u) || registers->sp < 0x1f03u || registers->sp > 0x1fffu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819d18u);
    const int cycles =
        Lufia2DecompRunDirectCallWithWidths(registers, 0x819d18u, 0x22u, 0x85922du, true, true, false);
    if (!cycles)
        goto finished;
    NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
restore_bank: {
    if (!NativeSpanCanStep(&span, cpu, 4u) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819d1cu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    registers->db = (uint8_t)value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, 4u, 0x9d1du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
restore_target: {
    if (!NativeSpanCanStep(&span, cpu, 5u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    Lufia2SpanFetch(registers, 0x819d1du, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, false);
    NativeSpanStep(&span, registers, cpu, 5u, 0x9d1eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
return_toggle: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffdu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x819d1eu);
    const int cycles = Lufia2DecompRunReturn(registers, 0x819d1eu, 0x60u, false, registers->mf, true);
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
static int Lufia2DecompRunBattleEffectCallersSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget, long *step_count) {
    if (pc >= 0x819b56u && pc <= 0x819b7du)
        return Lufia2DecompRunBattlePortraitPoseSpan(registers, cpu, pc, opcode, dynamic_epoch,
            write_epoch, step_budget, step_count);
    if (pc >= 0x819b7eu && pc <= 0x819ba2u)
        return Lufia2DecompRunBattleEffectFrameSetupSpan(registers, cpu, pc, opcode, dynamic_epoch,
            write_epoch, step_budget, step_count);
    if (pc >= 0x819ba3u && pc <= 0x819babu)
        return Lufia2DecompRunBattleEffectSkipArgumentSpan(registers, cpu, pc, opcode, dynamic_epoch,
            write_epoch, step_budget, step_count);
    if (pc >= 0x819bacu && pc <= 0x819bbdu)
        return Lufia2DecompRunBattlePartyTargetToggleSpan(registers, cpu, pc, opcode, dynamic_epoch,
            write_epoch, step_budget, step_count);
    if (pc >= 0x819cccu && pc <= 0x819d0au)
        return Lufia2DecompRunBattlePortraitStatusSpan(registers, cpu, pc, opcode, dynamic_epoch,
            write_epoch, step_budget, step_count);
    return Lufia2DecompRunBattleSpecialTargetToggleSpan(registers, cpu, pc, opcode, dynamic_epoch,
        write_epoch, step_budget, step_count);
}
