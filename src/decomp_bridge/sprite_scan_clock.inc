typedef struct SpriteScanSpan {
    uint64_t budget;
    unsigned steps;
    unsigned cycles;
    unsigned instructions;
    unsigned progress;
    unsigned slots;
} SpriteScanSpan;

static bool SpriteScanCanStep(const SpriteScanSpan *span, const CpuState *cpu, unsigned cycles) {
    // A single instruction keeps the original atomic deadline boundary.
    return span->instructions < span->steps &&
        (!span->instructions || (span->cycles + cycles <= span->budget &&
         (!s_lle_master_deadline || cpu->master_cycles < s_lle_master_deadline)));
}

static void SpriteScanStep(SpriteScanSpan *span, Interp816 *registers,
                                CpuState *cpu, unsigned cycles, uint16_t next) {
    Lufia2SpanAccountInstruction(cpu, cycles);
    registers->cyclesUsed = cycles;
    registers->pc = next;
    span->cycles += cycles;
    ++span->instructions;
}
