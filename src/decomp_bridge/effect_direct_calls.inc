static int Lufia2DecompRunEffectDirectCall(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    switch (pc) {
    case 0x818b2au:
        return Lufia2DecompRunDirectCall(registers, pc, opcode, 0x818e92u, false);
    case 0x818bbdu:
        return Lufia2DecompRunDirectCall(registers, pc, opcode, 0x858a2fu, true);
    case 0x818bc7u:
        return Lufia2DecompRunDirectCall(registers, pc, opcode, 0x85ec81u, true);
    default:
        return 0;
    }
}
