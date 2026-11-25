static int Lufia2DecompRunRamTransferSpan(Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
                                          uint64_t dynamic_epoch, uint64_t write_epoch, unsigned step_budget,
                                          long *step_count) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) ||                                             \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    const bool copying = pc == 0x83057du || pc == 0x86057du;
    const bool returning = pc == 0x830580u || pc == 0x860580u;
    if ((!copying && !returning) || !cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram ||
        !g_snes->cart || registers->k != (pc >> 16) || registers->pc != (uint16_t)pc || registers->e ||
        !step_budget || registers->waiting || registers->stopped || registers->irqWanted ||
        registers->nmiWanted)
        return 0;
    const Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || !cart->rom || cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 ||
        cart->sa1)
        return 0;
    NativeSpanState span = {BufferSpanCycleBudget(cpu, s_lle_master_deadline), step_budget, 0, 0, 0, 0};
    if (returning) {
        if (opcode != 0x60u || !NativeSpanCanStep(&span, cpu, 6u))
            return 0;
        const int cycles = Lufia2DecompRunReturn(registers, pc, opcode, false, registers->mf, true);
        if (!cycles)
            return 0;
        NativeSpanStep(&span, registers, cpu, (unsigned)cycles, registers->pc);
        NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
    } else {
        if (opcode != 0x54u || !Lufia2DecompFrameRangeQuiet(pc + 1u, pc + 3u))
            return 0;
        // A loop-head hook must run again before the next byte.
        const unsigned byte_limit = Lufia2DecompFrameRangeQuiet(pc, pc + 3u) ? 32u : 1u;
        const unsigned index_mask = registers->xf ? 0xffu : 0xffffu;
        while (span.instructions < byte_limit && NativeSpanCanStep(&span, cpu, 7u)) {
            if (cpu->ram[0x057du] != 0x54u || registers->irqWanted || registers->nmiWanted)
                break;
            const uint32_t destination = ((uint32_t)cpu->ram[0x057eu] << 16) | registers->y;
            const uint32_t source = ((uint32_t)cpu->ram[0x057fu] << 16) | registers->x;
            if (!Lufia2DecompIsWramByte(destination) ||
                (!Lufia2DecompIsWramByte(source) && !Lufia2DecompIsLoRomDataByte(cart, source)))
                break;
            Lufia2SpanFetch(registers, pc, 1u, span.instructions != 0u);
            // Copies may rewrite this stub, so fetch both bank operands for every byte.
            const uint8_t destination_bank = registers->read(registers->mem, pc + 1u);
            const uint8_t source_bank = registers->read(registers->mem, pc + 2u);
            registers->db = destination_bank;
            const uint8_t value =
                registers->read(registers->mem, ((uint32_t)source_bank << 16) | registers->x);
            registers->write(registers->mem, ((uint32_t)destination_bank << 16) | registers->y, value);
            registers->x = (registers->x + 1u) & index_mask;
            registers->y = (registers->y + 1u) & index_mask;
            --registers->a;
            NativeSpanStep(&span, registers, cpu, 7u, registers->a == 0xffffu ? 0x0580u : 0x057du);
            NativeSpanSyncProgress(&span, cpu, &dynamic_epoch, &write_epoch);
            if (registers->a == 0xffffu)
                break;
        }
    }
    if (!span.instructions)
        return 0;
    *step_count = span.progress ? span.instructions - span.progress : *step_count + span.instructions - 1;
    return (int)span.cycles;
}
