static void SpecialArithmeticWait(void *context, Lufia2CpuState *state, uint32_t pc) {
    FixedSpanClock *span = context;
    (void)state;
    assert(span->index < span->count && !span->remaining);
    const FixedSpanOpcode *op = span->timing + span->index;
    assert((span->bank | op->pc) == pc && !op->data_bytes);
    for (unsigned byte = 0u; byte < op->fetched; ++byte) {
        bool progress = false;
        const uint32_t address = pc + byte;
        (void)Lufia2DecompFrameReturnReadByte(span->cpu, address, &progress);
        span->fetch_master += cpu_region_speed(address);
        span->progress |= progress;
    }
    FinishFixedSpanOpcode(span);
}
