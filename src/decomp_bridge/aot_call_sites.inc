static int Lufia2DecompRunAotCallSite(Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    if (opcode == 0x60u || opcode == 0x6bu) {
        const unsigned bank = pc >> 16;
        const unsigned address = (uint16_t)pc;
        if (bank < 0x80u || bank > 0xbfu ||
            (address < 0x8000u && (opcode != 0x60u || address >= 0x2000u)))
            return 0;
        return Lufia2DecompRunReturn(registers, pc, opcode, opcode == 0x6bu,
                                     registers->mf, registers->xf);
    }
    const uint32_t page = (pc >> LUFIA2_AOT_CALL_SITE_PAGE_BITS) -
                          LUFIA2_AOT_CALL_SITE_FIRST_PAGE;
    const size_t page_count =
        sizeof lufia2_aot_call_page_offsets / sizeof lufia2_aot_call_page_offsets[0] - 1u;
    if (page >= page_count)
        return 0;
    size_t first = lufia2_aot_call_page_offsets[page];
    size_t end = lufia2_aot_call_page_offsets[page + 1u];
    if (first == end)
        return 0;
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
    if (opcode == 0x20u && (target >> 16) != (pc >> 16))
        return 0;
    return Lufia2DecompRunDirectCallWithWidths(
        registers, pc, opcode, target, opcode == 0x22u, registers->mf, registers->xf);
}
