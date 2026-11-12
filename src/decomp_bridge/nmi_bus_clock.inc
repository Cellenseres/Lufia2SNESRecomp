static void NmiClockBeginOpcode(FixedSpanClock *clock) {
    while (!clock->remaining && clock->index < clock->count) {
        const FixedSpanOpcode *timing = clock->timing + clock->index;
        const uint32_t pc = clock->bank | timing->pc;
        Cart *cart = g_snes->cart;
        const uint32_t last = pc + timing->fetched - 1u;
        /* Immutable ROM; the admitted window excludes observers. */
        clock->cpu->open_bus = cart->rom[last & 0x7fffu];
        cart->cpuBusAddress = last;
        clock->fetch_master += timing->fetched * cpu_region_speed(pc);
        clock->remaining = timing->data_bytes;
        if (!clock->remaining)
            FinishFixedSpanOpcode(clock);
    }
}

static uint8_t NmiClockRead(void *context, uint32_t address) {
    FixedSpanClock *clock = context;
    NmiClockBeginOpcode(clock);
    assert(clock->remaining);
    bool progress = false;
    const uint8_t value = Lufia2DecompFrameReturnReadByte(clock->cpu, address, &progress);
    clock->data_master += cpu_region_speed(address);
    clock->progress |= progress;
    if (!--clock->remaining)
        FinishFixedSpanOpcode(clock);
    return value;
}

#if defined(LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK)
static void NmiClockWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanClock *clock = context;
    NmiClockBeginOpcode(clock);
    assert(clock->remaining);
    bool progress = false;
    Lufia2DecompFrameReturnWriteByte(clock->cpu, address, value, &progress);
    clock->data_master += cpu_region_speed(address);
    clock->progress |= progress;
    if (!--clock->remaining)
        FinishFixedSpanOpcode(clock);
}
#endif
