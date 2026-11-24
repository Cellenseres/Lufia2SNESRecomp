#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunFieldFixedGraphicsSpan(Interp816 *registers, CpuState *cpu, uint32_t pc,
                                                 uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
                                                 unsigned step_budget, long *step_count) {
    uint32_t end_pc;
    if (pc >= 0x83b007u && pc < 0x83b036u) {
        end_pc = 0x83b036u;
    } else if (pc >= 0x83b036u && pc < 0x83b05cu) {
        end_pc = 0x83b05cu;
    } else if (pc >= 0x83b05cu && pc < 0x83b062u) {
        end_pc = 0x83b062u;
    } else
        return 0;
    const bool dynamic_widths = (pc <= 0x83b00cu || pc >= 0x83b05cu);
    const bool byte_accumulator = true;
    const bool byte_index = false;
    const bool changes_accumulator = pc == 0x83b00cu;
    const bool changes_index = pc == 0x83b00eu;
    if (!cpu || !cpu->ram || registers->e || !step_budget || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)) ||
        (!dynamic_widths && ((!changes_accumulator && registers->mf != byte_accumulator) ||
                             (!changes_index && registers->xf != byte_index))) ||
        (pc + 1u < end_pc && !Lufia2DecompFrameRangeQuiet(pc + 1u, end_pc)))
        return 0;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 1u, &offset);
    if (pc >= 0x83b007u && pc < 0x83b062u) {
        const unsigned delta = pc - 0x83b007u;
        if (!cart || offset < delta)
            return 0;
        const unsigned first = offset - delta;
        if (cart->romSize < first + 91u ||
            memcmp(
                cart->rom + first,
                "\x48\xda\x5a\x8b\x08\xe2\x20\xc2\x10\xa9\x01\x8d\x00\x43\xa2\x00\x85\x8e\x02\x43\xa9\x9f\x8d"
                "\x04\x43\xa2\x00\x10\x8e\x05\x43\xa9\x18\x8d\x01\x43\xa2\x00\x10\x8e\x16\x21\xa9\x01\x8d\x0b"
                "\x42\xa9\x01\x8d\x00\x43\xa2\xfc\xe2\x8e\x02\x43\xa9\xa5\x8d\x04\x43\xa2\x00\x03\x8e\x05\x43"
                "\xa9\x18\x8d\x01\x43\xa2\x80\x16\x8e\x16\x21\xa9\x01\x8d\x0b\x42\x28\xab\x7a\xfa\x68\x6b",
                91u))
            return 0;
    } else
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    switch (pc) {
    case 0x83b007u: goto graphics_save_accumulator;
    case 0x83b008u: goto graphics_save_x;
    case 0x83b009u: goto graphics_save_y;
    case 0x83b00au: goto graphics_save_bank;
    case 0x83b00bu: goto graphics_save_status;
    case 0x83b00cu: goto graphics_narrow_accumulator;
    case 0x83b00eu: goto graphics_widen_index;
    case 0x83b010u: goto graphics_first_dma_mode;
    case 0x83b012u: goto graphics_set_first_dma_mode;
    case 0x83b015u: goto graphics_first_source_offset;
    case 0x83b018u: goto graphics_set_first_source_offset;
    case 0x83b01bu: goto graphics_first_source_bank;
    case 0x83b01du: goto graphics_set_first_source_bank;
    case 0x83b020u: goto graphics_first_transfer_size;
    case 0x83b023u: goto graphics_set_first_transfer_size;
    case 0x83b026u: goto graphics_first_dma_port;
    case 0x83b028u: goto graphics_set_first_dma_port;
    case 0x83b02bu: goto graphics_first_vram_target;
    case 0x83b02eu: goto graphics_set_first_vram_target;
    case 0x83b031u: goto graphics_first_dma_mask;
    case 0x83b033u: goto graphics_start_first_dma;
    case 0x83b036u: goto graphics_second_dma_mode;
    case 0x83b038u: goto graphics_set_second_dma_mode;
    case 0x83b03bu: goto graphics_second_source_offset;
    case 0x83b03eu: goto graphics_set_second_source_offset;
    case 0x83b041u: goto graphics_second_source_bank;
    case 0x83b043u: goto graphics_set_second_source_bank;
    case 0x83b046u: goto graphics_second_transfer_size;
    case 0x83b049u: goto graphics_set_second_transfer_size;
    case 0x83b04cu: goto graphics_second_dma_port;
    case 0x83b04eu: goto graphics_set_second_dma_port;
    case 0x83b051u: goto graphics_second_vram_target;
    case 0x83b054u: goto graphics_set_second_vram_target;
    case 0x83b057u: goto graphics_second_dma_mask;
    case 0x83b059u: goto graphics_start_second_dma;
    case 0x83b05cu: goto graphics_restore_status;
    case 0x83b05du: goto graphics_restore_bank;
    case 0x83b05eu: goto graphics_restore_y;
    case 0x83b05fu: goto graphics_restore_x;
    case 0x83b060u: goto graphics_restore_accumulator;
    case 0x83b061u: goto graphics_return_graphics;
    default: return 0;
    }
graphics_save_accumulator: {
    const bool wide = !registers->mf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b007u, 1u, span.instructions != 0u);
    const uint16_t value = registers->a;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb008u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_save_x: {
    const bool wide = !registers->xf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b008u, 1u, span.instructions != 0u);
    const uint16_t value = registers->x;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb009u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_save_y: {
    const bool wide = !registers->xf;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b009u, 1u, span.instructions != 0u);
    const uint16_t value = registers->y;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb00au);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_save_bank: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b00au, 1u, span.instructions != 0u);
    const uint16_t value = registers->db;
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb00bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_save_status: {
    const bool wide = false;
    const unsigned cycles = 3u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f01u + wide || registers->sp > 0x1fffu)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b00bu, 1u, span.instructions != 0u);
    const uint16_t value = interp816_getFlags(registers);
    if (wide) {
        registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
        --registers->sp;
    }
    registers->write(registers->mem, registers->sp, (uint8_t)value);
    --registers->sp;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb00cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_narrow_accumulator: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b00cu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) | 0x20u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb00eu);
}
graphics_widen_index: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b00eu, 2u, span.instructions != 0u);
    interp816_setFlags(registers, interp816_getFlags(registers) & ~0x10u);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb010u);
}
graphics_first_dma_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b010u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb012u);
}
graphics_set_first_dma_mode: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4300u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b012u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb015u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_first_source_offset: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b015u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x8500u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb018u);
}
graphics_set_first_source_offset: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4302u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b018u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb01bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_first_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b01bu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x009fu, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb01du);
}
graphics_set_first_source_bank: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4304u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b01du, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb020u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_first_transfer_size: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b020u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x1000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb023u);
}
graphics_set_first_transfer_size: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4305u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b023u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb026u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_first_dma_port: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b026u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb028u);
}
graphics_set_first_dma_port: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4301u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b028u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb02bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_first_vram_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b02bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x1000u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb02eu);
}
graphics_set_first_vram_target: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2116u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b02eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb031u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_first_dma_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b031u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb033u);
}
graphics_start_first_dma: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x420bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b033u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb036u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
graphics_second_dma_mode: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b036u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb038u);
}
graphics_set_second_dma_mode: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4300u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b038u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb03bu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_second_source_offset: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b03bu, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0xe2fcu, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb03eu);
}
graphics_set_second_source_offset: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4302u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b03eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb041u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_second_source_bank: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b041u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x00a5u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb043u);
}
graphics_set_second_source_bank: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4304u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b043u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb046u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_second_transfer_size: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b046u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x0300u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb049u);
}
graphics_set_second_transfer_size: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4305u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b049u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb04cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_second_dma_port: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b04cu, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0018u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb04eu);
}
graphics_set_second_dma_port: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x4301u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b04eu, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb051u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_second_vram_target: {
    if (!NativeSpanCanStep(&span, cpu, 3u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b051u, 3u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa2u, 0x1680u, false);
    NativeSpanStep(&span, registers, cpu, 3u, 0xb054u);
}
graphics_set_second_vram_target: {
    const bool wide = true;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x2116u) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b054u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->x, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb057u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_second_dma_mask: {
    if (!NativeSpanCanStep(&span, cpu, 2u))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b057u, 2u, span.instructions != 0u);
    Lufia2DecompApplyOperand(registers, 0xa9u, 0x0001u, true);
    NativeSpanStep(&span, registers, cpu, 2u, 0xb059u);
}
graphics_start_second_dma: {
    const bool wide = false;
    const uint32_t address = (((uint32_t)registers->db << 16) + 0x420bu) & 0xffffffu;
    const uint32_t high = (address + 1u) & 0xffffffu;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) ||
        !(Lufia2DecompIsFieldDisplayWriteByte(address, true) &&
          (!wide || Lufia2DecompIsFieldDisplayWriteByte(high, true))))
        goto finished;
    Lufia2SpanFetch(registers, 0x83b059u, 3u, span.instructions != 0u);
    Lufia2DecompWriteData(registers, address, high, registers->a, wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb05cu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    goto finished;
}
graphics_restore_status: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b05cu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    interp816_setFlags(registers, (uint8_t)value);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb05du);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_restore_bank: {
    const bool wide = false;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b05du, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    registers->db = value;
    registers->z = value == 0u;
    registers->n = (value & 0x80u) != 0u;
    NativeSpanStep(&span, registers, cpu, cycles, 0xb05eu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_restore_y: {
    const bool wide = !registers->xf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b05eu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa0u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb05fu);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_restore_x: {
    const bool wide = !registers->xf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b05fu, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa2u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb060u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_restore_accumulator: {
    const bool wide = !registers->mf;
    const unsigned cycles = 4u + wide;
    if (!NativeSpanCanStep(&span, cpu, cycles) || registers->sp < 0x1f00u || registers->sp > 0x1ffeu - wide)
        goto finished;
    Lufia2SpanFetch(registers, 0x83b060u, 1u, span.instructions != 0u);
    uint16_t value = registers->read(registers->mem, ++registers->sp);
    if (wide)
        value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
    Lufia2DecompApplyOperand(registers, 0xa9u, value, !wide);
    NativeSpanStep(&span, registers, cpu, cycles, 0xb061u);
    NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
}
graphics_return_graphics: {
    if (!NativeSpanCanStep(&span, cpu, 6u) || registers->sp < 0x1f00u || registers->sp > 0x1ffcu)
        goto finished;
    if (span.instructions)
        Lufia2SpanProbeOpcode(registers, 0x83b061u);
    const int cycles = Lufia2DecompRunReturn(registers, 0x83b061u, 0x6bu, true, registers->mf, true);
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
