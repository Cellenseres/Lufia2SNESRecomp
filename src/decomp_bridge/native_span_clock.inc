typedef struct NativeSpanState {
    uint64_t budget;
    unsigned steps;
    unsigned cycles;
    unsigned instructions;
    unsigned progress;
    unsigned slots;
} NativeSpanState;

static bool NativeSpanCanStep(const NativeSpanState *span, const CpuState *cpu, unsigned cycles) {
    // A single instruction keeps the original atomic deadline boundary.
    return span->instructions < span->steps &&
        (!span->instructions || (span->cycles + cycles <= span->budget &&
         (!s_lle_master_deadline || cpu->master_cycles < s_lle_master_deadline)));
}

static void NativeSpanStep(NativeSpanState *span, Interp816 *registers,
                           CpuState *cpu, unsigned cycles, uint16_t next) {
    Lufia2SpanAccountInstruction(cpu, cycles);
    registers->cyclesUsed = cycles;
    registers->pc = next;
    span->cycles += cycles;
    ++span->instructions;
}

static void NativeSpanSyncProgress(NativeSpanState *span, CpuState *cpu,
    uint64_t *dynamic_epoch, uint64_t *write_epoch) {
    if (*dynamic_epoch != s_interp_dynamic_progress_epoch || *write_epoch != g_interp_bridge_write_epoch)
        span->progress = span->instructions;
    Lufia2SpanSyncProgress(cpu, dynamic_epoch, write_epoch);
}
