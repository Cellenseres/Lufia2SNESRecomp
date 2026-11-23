/* The whole cursor hook runs first; partial steps keep frame events observable. */
static int Lufia2DecompRunEffectSlotCursorOpcode(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    const bool actors = pc >= 0x818b1bu && pc < 0x818b25u;
    const uint32_t entry = actors ? 0x818b1bu : 0x818aaau;
    if (pc < entry || pc >= entry + 10u || registers->e || registers->xf)
        return 0;
    const unsigned position = pc - entry;
    static const uint8_t opcodes[] = {0xc2u, 0u, 0x98u, 0x18u, 0x69u, 0u, 0u, 0xa8u, 0xe2u, 0u};
    if (!opcodes[position] || opcode != opcodes[position] ||
        (position != 0u && position != 8u && registers->mf))
        return 0;
    const unsigned length = position == 4u ? 3u : position == 0u || position == 8u ? 2u : 1u;
    const unsigned cycles = length == 1u ? 2u : 3u;
    const uint16_t stride = actors ? 45u : 44u;
    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, length, &offset);
    if (!cart || (length == 2u && cart->rom[offset + 1u] != 0x20u) ||
        (length == 3u && (cart->rom[offset + 1u] != stride || cart->rom[offset + 2u] != 0u)))
        return 0;

    registers->cyclesUsed = 0u;
    (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = cycles;
    for (unsigned byte = 1u; byte < length; ++byte)
        (void)registers->read(registers->mem, (pc & 0xff0000u) | registers->pc++);
    switch (position) {
    case 0u:
        registers->mf = false;
        break;
    case 2u:
        Lufia2DecompApplyOperand(registers, 0xa9u, registers->y, false);
        break;
    case 3u:
        registers->c = false;
        break;
    case 4u:
        Lufia2DecompApplyOperand(registers, 0x69u, stride, false);
        break;
    case 7u:
        Lufia2DecompApplyOperand(registers, 0xa0u, registers->a, false);
        break;
    case 8u:
        registers->mf = true;
        break;
    }
    return (int)cycles;
}
