static int Lufia2RunNativeContinuation(Interp816 *registers, CpuState *cpu, uint32_t pc,
    uint64_t dynamic_epoch, uint64_t write_epoch, bool batched, unsigned step_budget, long *step_count) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (!cpu || !cpu->ram || !g_snes || cpu->ram != g_snes->ram || !g_snes->cart ||
        !step_budget || registers->e || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        (registers->xf && (registers->x > 0xffu || registers->y > 0xffu)))
        return 0;
    size_t first = 0u;
    size_t end = sizeof native_continuation_ranges / sizeof native_continuation_ranges[0];
    while (first < end) {
        const size_t middle = first + (end - first) / 2u;
        if (native_continuation_ranges[middle].last < pc)
            first = middle + 1u;
        else
            end = middle;
    }
    if (first == sizeof native_continuation_ranges / sizeof native_continuation_ranges[0] ||
        native_continuation_ranges[first].first > pc)
        return 0;

    const Cart *cart = g_snes->cart;
    const uint32_t rom_bank = ((pc >> 16) & 0x7fu) * 0x8000u;
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < rom_bank + 0x8000u ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return 0;
    Continuation run = {
        {BufferSpanCycleBudget(cpu, s_lle_master_deadline), batched ? step_budget : 1u, 0u, 0u, 0u, 0u},
        registers, cpu, cart, cart->rom + rom_bank, pc & 0xff0000u, dynamic_epoch, write_epoch,
        batched};
    native_continuation_ranges[first].run(&run, (uint16_t)pc);
    if (!run.span.instructions)
        return 0;
    if (batched)
        *step_count = run.span.progress ? run.span.instructions - run.span.progress
                                        : *step_count + run.span.instructions - 1;
    return (int)run.span.cycles;
}

#undef CONTINUATION_STEP
#undef CONTINUATION_IMMEDIATE
#undef CONTINUATION_CORE
#undef CONTINUATION_NOINLINE
