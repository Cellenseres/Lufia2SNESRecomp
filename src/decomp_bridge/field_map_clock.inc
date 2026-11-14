static const FixedSpanOpcode field_leader_probe_timing[] = {
    {0xc0efu, 3u, 4u, 1u}, {0xc0f2u, 2u, 3u, 1u},
    {0xc0f4u, 3u, 4u, 1u}, {0xc0f7u, 2u, 3u, 1u}
};

static const FixedSpanOpcode field_map_value_timing[] = {
    {0xfb71u, 3u, 6u, 2u},
    {0xf9d4u, 2u, 3u, 1u}, {0xf9d6u, 1u, 3u, 0u},
    {0xf9d7u, 2u, 3u, 1u}, {0xf9d9u, 3u, 6u, 2u},
    {0xf9f7u, 4u, 5u, 1u}, {0xf9fbu, 4u, 5u, 1u},
    {0xf9ffu, 4u, 5u, 1u}, {0xfa03u, 2u, 2u, 0u},
    {0xfa05u, 1u, 3u, 0u}, {0xfa06u, 2u, 3u, 0u},
    {0xfa08u, 1u, 2u, 0u}, {0xfa09u, 4u, 6u, 2u},
    {0xfa0du, 1u, 2u, 0u}, {0xfa0eu, 1u, 2u, 0u},
    {0xfa0fu, 2u, 3u, 0u}, {0xfa11u, 1u, 6u, 2u},
    {0xf9dcu, 2u, 3u, 0u}, {0xf9deu, 1u, 4u, 2u},
    {0xf9dfu, 4u, 6u, 2u}, {0xf9e3u, 1u, 2u, 0u},
    {0xf9e4u, 1u, 5u, 2u}, {0xf9e5u, 1u, 2u, 0u},
    {0xf9e6u, 4u, 6u, 2u}, {0xf9eau, 1u, 2u, 0u},
    {0xf9ebu, 2u, 3u, 0u}, {0xf9edu, 1u, 6u, 2u},
    {0xfb74u, 2u, 3u, 0u}, {0xfb76u, 4u, 6u, 2u},
    {0xfb7au, 3u, 3u, 0u}, {0xfb7du, 1u, 2u, 0u},
    {0xfb7eu, 4u, 6u, 2u}, {0xfb82u, 1u, 2u, 0u},
    {0xfb83u, 2u, 3u, 0u}, {0xfb85u, 1u, 2u, 0u},
    {0xfb86u, 4u, 5u, 1u}
};

_Static_assert(sizeof field_map_value_timing / sizeof *field_map_value_timing + 2u <=
    LUFIA2_FIELD_MAP_OPCODE_BUDGET, "include the original caller and return");

typedef struct FieldMapClock {
    PairedSpanClock paired;
    uint16_t product;
} FieldMapClock;

static uint8_t FieldMapRead(void *context, uint32_t address) {
    FieldMapClock *clock = context;
    PreparePairedSpan(&clock->paired);
    FixedSpanClock *span = &clock->paired.span;
    assert(span->index < span->count);
    if (span->timing[span->index].pc != 0xfa09u)
        return FixedSpanRead(span, address);
    bool progress = false;
    uint8_t value;
    /* The original word read has one paired hardware bus service. */
    if (span->remaining == 2u) {
        assert(address == 0x4216u);
        clock->product = Lufia2DecompFrameReturnReadWord(
            span->cpu, address, address + 1u, &progress);
        value = (uint8_t)clock->product;
    } else {
        assert(span->remaining == 1u && address == 0x4217u);
        value = (uint8_t)(clock->product >> 8);
    }
    span->data_master += cpu_region_speed(address);
    span->progress |= progress;
    if (!--span->remaining)
        FinishFixedSpanOpcode(span);
    return value;
}

static void FieldMapWrite(void *context, uint32_t address, uint8_t value) {
    FieldMapClock *clock = context;
    PairedSpanWrite(&clock->paired, address, value);
}

static bool FieldMapCallerSupported(uint32_t entry, uint32_t rpc) {
    return (entry == 0x83ab4fu && rpc == 0x83bba5u) ||
        (entry == 0x83c0efu && (rpc == 0x83822du || rpc == 0x838281u)) ||
        (entry == 0x83fb71u && rpc == 0x838285u) ||
        (entry == 0x83f9d4u && rpc == 0x83fb74u) ||
        (entry == 0x83f9f7u && rpc == 0x83f9dcu);
}

static unsigned FieldMapClockReserve(const FixedSpanOpcode *plan, unsigned count) {
    unsigned master = 44u;
    for (unsigned opcode = 0u; opcode < count; ++opcode) {
        const unsigned bus = plan[opcode].fetched + plan[opcode].data_bytes;
        master += 8u * bus + 6u * (plan[opcode].cycles - bus);
    }
    return master + 80u * (master / 1284u + 2u);
}

static bool FieldMapClockSupported(const CpuState *cpu, uint32_t entry, unsigned reserve) {
    const unsigned frame_size = (entry == 0x83fb71u || entry == 0x83ab4fu) ? 3u : 2u;
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag > (entry == 0x83ab4fu ? 1u : 0u) ||
        cpu->host_return_valid != frame_size || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        g_interp_apu_driving || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !Lufia2DecompFramePairedCallReady(cpu, entry))
        return false;
    const uint32_t rpc = Lufia2DecompFramePairedReturnPc(cpu, entry);
    if (!FieldMapCallerSupported(entry, rpc) ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) !=
            (uint16_t)(rpc - 1u) ||
        (frame_size == 3u && cpu->ram[cpu->S + 3u] != (rpc >> 16)) ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, reserve) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x1fb8au)
        return false;
    if (entry == 0x83ab4fu) {
        if (cpu->ram[0xa7u] >= 40u ||
            !Lufia2DecompFrameRangeQuiet(0x83ab4fu, 0x83ab61u))
            return false;
    } else if (entry == 0x83c0efu) {
        if (!Lufia2DecompFrameRangeQuiet(0x83c0efu, 0x83c0fau))
            return false;
    } else {
        if (!Lufia2DecompFrameRangeQuiet(0x83f9f7u, 0x83fa12u))
            return false;
        if (entry != 0x83f9f7u) {
            const unsigned layer = cpu->ram[0x5aau] | (unsigned)cpu->ram[0x5abu] << 8;
            if (layer > 6u || (layer & 1u) ||
                !Lufia2DecompFrameRangeQuiet(0x83f9d4u, 0x83f9eeu))
                return false;
            if (entry == 0x83fb71u) {
                const unsigned base = cpu->ram[0x1d008u + layer] |
                    (unsigned)cpu->ram[0x1d009u + layer] << 8;
                const uint16_t cell = (uint16_t)(base +
                    2u * (cpu->ram[0x8fu] + cpu->ram[0x91u] * cpu->ram[0x5b9u]));
                if (cell == 0xffffu || !Lufia2DecompFrameRangeQuiet(0x83fb71u, 0x83fb8bu))
                    return false;
            }
        }
    }
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedFieldMap(CpuState *cpu, uint32_t entry) {
    const FixedSpanOpcode *plan = entry == 0x83ab4fu ? actor_record_offset_timing
        : entry == 0x83c0efu ? field_leader_probe_timing
        : field_map_value_timing + (entry == 0x83f9d4u ? 1u : entry == 0x83f9f7u ? 5u : 0u);
    const unsigned count = entry == 0x83ab4fu ? 9u : entry == 0x83c0efu ? 4u : entry == 0x83f9d4u ? 25u
        : entry == 0x83f9f7u ? 11u : 36u;
    const unsigned reserve = FieldMapClockReserve(plan, count);
    if (!FieldMapClockSupported(cpu, entry, reserve)) {
        const uint32_t original = (uint32_t)cpu->PB << 16 | (entry & 0xffffu);
        return interp_tier_dispatch_tail(cpu, original, original, cpu->S, cpu->host_return_valid);
    }
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, entry)};
    FieldMapClock clock = {.paired = BeginPairedSpanClock(cpu, entry, plan, count)};
    const Lufia2Memory memory = {FieldMapRead, FieldMapWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    if (entry == 0x83ab4fu)
        (void)Lufia2ActorSetRecordOffsets(&memory, &state);
    else if (entry == 0x83c0efu)
        (void)Lufia2FieldProbeLeaderPosition(&memory, &state);
    else if (entry == 0x83fb71u)
        Lufia2ActorReadMapCellValue(&memory, &state);
    else if (entry == 0x83f9d4u)
        Lufia2ActorResolveMapCellOffset(&memory, &state);
    else
        (void)Lufia2MapCellOffset(&memory, &state);
    PreparePairedSpan(&clock.paired);
    assert(clock.paired.offset == count && !clock.paired.span.remaining);
    ActorBridgeStore(cpu, &state);
    /* The inlined JSR children leave the paired ABI in two-byte return mode. */
    if (entry == 0x83fb71u)
        cpu->host_return_valid = 2u;
    const uint32_t source = entry == 0x83ab4fu ? 0x83ab60u : entry == 0x83c0efu ? 0x83c0f9u : entry == 0x83fb71u
        ? 0x83fb8au : entry == 0x83f9d4u ? 0x83f9edu : 0x83fa11u;
    bool progress = false;
    const RecompReturn returned = (entry == 0x83fb71u || entry == 0x83ab4fu)
        ? ReturnTimedLongCallWithProgress(cpu, &frame, source, &progress)
        : ReturnTimedMenuCallWithProgress(cpu, &frame, source, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, entry, 1u, progress);
    assert(cpu->master_cycles - start <= reserve);
    return returned;
}
