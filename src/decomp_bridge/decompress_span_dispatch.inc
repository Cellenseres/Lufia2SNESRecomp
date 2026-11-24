#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__)
__attribute__((noinline))
#endif
static int Lufia2DecompRunDecompressSpan(Interp816 *registers, CpuState *cpu,
    uint32_t pc, uint8_t opcode, uint64_t dynamic_epoch, uint64_t write_epoch,
    unsigned step_budget, long *step_count) {
    if (pc == 0x808efau || pc == 0x808efcu || pc == 0x808f40u || pc == 0x808f42u)
        return Lufia2DecompRunDecompressTail(registers, cpu, pc, opcode,
            dynamic_epoch, write_epoch, step_budget, step_count);
    return Lufia2DecompRunDecompressLiteral(registers, cpu, pc, opcode,
        dynamic_epoch, write_epoch, step_budget, step_count);
}
