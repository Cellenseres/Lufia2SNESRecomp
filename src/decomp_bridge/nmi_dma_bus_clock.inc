typedef struct NmiDmaClock {
    FixedSpanClock span;
    uint16_t word;
    uint32_t word_address;
} NmiDmaClock;

static uint8_t NmiDmaRead(void *context, uint32_t address) {
    NmiDmaClock *clock = context;
    return NmiClockRead(&clock->span, address);
}

static void NmiDmaWrite(void *context, uint32_t address, uint8_t value) {
    NmiDmaClock *clock = context;
    FixedSpanClock *bus = &clock->span;
    NmiClockBeginOpcode(bus);
    assert(bus->remaining);
    const FixedSpanOpcode *opcode = bus->timing + bus->index;
    const bool hardware_word = opcode->data_bytes == 2u &&
        (address & 0xffffu) >= 0x2100u && (address & 0xffffu) < 0x4380u;
    bool progress = false;
    /* Keep the interpreter's paired hardware write. */
    if (hardware_word && bus->remaining == 2u) {
        clock->word = value;
        clock->word_address = address;
    } else if (hardware_word) {
        assert(address == clock->word_address + 1u);
        const uint16_t word = (uint16_t)(clock->word | ((uint16_t)value << 8));
        Lufia2DecompFrameReturnWriteWord(bus->cpu, clock->word_address,
            address, word, &progress);
    } else {
        Lufia2DecompFrameReturnWriteByte(bus->cpu, address, value, &progress);
    }
    bus->data_master += cpu_region_speed(address);
    bus->progress |= progress;
    if (!--bus->remaining)
        FinishFixedSpanOpcode(bus);
}
