#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunBank82Span(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc <= 0x8280a4u)
        return Lufia2DecompRunMenuGridFrame(registers, cpu, pc, opcode,
            dynamic_epoch, write_epoch, step_budget, step_count);
    if (pc <= 0x82827du) {
        if (pc >= 0x828230u && pc <= 0x828258u)
            return Lufia2DecompRunMenuWindowSide(registers, cpu, pc, opcode,
                dynamic_epoch, write_epoch, step_budget, step_count);
        return Lufia2DecompRunMenuWindowCorner(registers, cpu, pc, opcode,
            dynamic_epoch, write_epoch, step_budget, step_count);
    }
    if (pc <= 0x828af9u)
        return Lufia2DecompRunMenuCursorCorrect(registers, cpu, pc, opcode,
            dynamic_epoch, write_epoch, step_budget, step_count);
    return Lufia2DecompRunCapsuleStatGrowth(registers, cpu, pc, opcode,
        dynamic_epoch, write_epoch, step_budget, step_count);
}
