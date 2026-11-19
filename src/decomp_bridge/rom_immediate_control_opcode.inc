extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunRomImmediateControl(Interp816 *registers, uint32_t pc, uint8_t opcode) {
#if defined(SNES_COSIM) || defined(SNESRECOMP_INTERP_PROFILE) || \
    (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    return 0;
#endif
    const unsigned bank = pc >> 16;
    const unsigned address = pc & 0xffffu;
    const bool changes_status = opcode == 0xc2u || opcode == 0xe2u;
    const bool conditional = (opcode & 0x1fu) == 0x10u;
    const bool changes_bank = opcode == 0x5cu;
    const bool word_operand = changes_bank || opcode == 0x4cu || opcode == 0x82u;
    const unsigned length = changes_bank ? 4u : word_operand ? 3u : 2u;
    if ((!changes_status && !conditional && !word_operand && opcode != 0x80u) || bank < 0x80u || bank > 0xbfu ||
        address < 0x8000u || address + length > 0x10000u || registers->k != bank ||
        registers->pc != address || registers->e || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted ||
        !g_snes || !g_snes->cart)
        return 0;

    const Cart *cart = g_snes->cart;
    const unsigned offset = (bank & 0x7fu) * 0x8000u + (address & 0x7fffu);
    if (cart->type != CART_LOROM || !cart->rom || cart->romSize < offset + length ||
        cart->superfx || cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1 ||
        cart->rom[offset] != opcode)
        return 0;

    registers->cyclesUsed = 0;
    const uint32_t program_bank = pc & 0xff0000u;
    (void)registers->read(registers->mem, program_bank | registers->pc++);
    g_interp816_cur_pc = pc;
    registers->cyclesUsed = conditional ? 2u : changes_bank || opcode == 0x82u ? 4u : 3u;
    const unsigned operand = registers->read(registers->mem, program_bank | registers->pc++);
    if (changes_status) {
        const unsigned status = interp816_getFlags(registers);
        interp816_setFlags(registers, (uint8_t)(opcode == 0xc2u ? status & ~operand : status | operand));
        return registers->cyclesUsed;
    }
    if (!conditional) {
        if (word_operand) {
            const unsigned high = registers->read(registers->mem, program_bank | registers->pc++);
            const unsigned word = operand | (high << 8);
            if (opcode == 0x82u) {
                const int displacement = (int)word - ((word & 0x8000u) ? 65536 : 0);
                registers->pc = (uint16_t)(registers->pc + displacement);
            } else {
                if (changes_bank)
                    registers->k = registers->read(registers->mem, program_bank | registers->pc++);
                registers->pc = (uint16_t)word;
            }
        } else {
            const int displacement = (int)operand - ((operand & 0x80u) ? 256 : 0);
            registers->pc = (uint16_t)(registers->pc + displacement);
        }
        return registers->cyclesUsed;
    }
    bool flag;
    switch (opcode >> 6) {
    case 0u: flag = registers->n; break;
    case 1u: flag = registers->v; break;
    case 2u: flag = registers->c; break;
    default: flag = registers->z; break;
    }
    if (flag == ((opcode & 0x20u) != 0u)) {
        const int displacement = (int)operand - ((operand & 0x80u) ? 256 : 0);
        registers->pc = (uint16_t)(registers->pc + displacement);
        ++registers->cyclesUsed;
    }
    return registers->cyclesUsed;
}
/* Keeping the dispatch out of line avoids growing the interpreter loop. */
#if defined(_MSC_VER)
static __declspec(noinline)
#elif defined(__GNUC__)
static __attribute__((noinline))
#else
static
#endif
int Lufia2DecompRunNativeRomControl(Interp816 *registers, uint32_t pc, uint8_t opcode) {
    if ((opcode & 0x1fu) == 0x10u || opcode == 0xc2u || opcode == 0xe2u ||
        opcode == 0x80u || opcode == 0x82u || opcode == 0x4cu || opcode == 0x5cu)
        return Lufia2DecompRunRomImmediateControl(registers, pc, opcode);
    return Lufia2DecompRunAotCallSite(registers, pc, opcode);
}
