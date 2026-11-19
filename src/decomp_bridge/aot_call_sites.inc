static int Lufia2DecompRunAotCallSite(Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    size_t first = 0u;
    size_t end = sizeof lufia2_aot_call_sites / sizeof lufia2_aot_call_sites[0] - 1u;
    while (first < end) {
        const size_t middle = first + (end - first) / 2u;
        if (lufia2_aot_call_sites[middle].pc < pc)
            first = middle + 1u;
        else
            end = middle;
    }
    const struct Lufia2AotCallSite site = lufia2_aot_call_sites[first];
    const unsigned width = (registers->mf ? 2u : 0u) | (registers->xf ? 1u : 0u);
    const unsigned attributes = site.target_and_widths >> 24;
    const uint32_t target = site.target_and_widths & 0xffffffu;
    if (site.pc != pc || !(attributes & (1u << width)))
        return 0;
    if (attributes & 0x10u) {
        if (opcode != target)
            return 0;
        return Lufia2DecompRunReturn(registers, pc, opcode, opcode == 0x6bu,
                                     registers->mf, registers->xf);
    }
    if (opcode == 0x20u && (target >> 16) != (pc >> 16))
        return 0;
    return Lufia2DecompRunDirectCallWithWidths(
        registers, pc, opcode, target, opcode == 0x22u, registers->mf, registers->xf);
}
