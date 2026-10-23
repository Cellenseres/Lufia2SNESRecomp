typedef enum PrimaryWaveStage {
    WAVE_OPEN, WAVE_PREFIX, WAVE_ROWS, WAVE_SUFFIX
} PrimaryWaveStage;

typedef struct PrimaryWaveClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[28];
    PrimaryWaveStage stage;
    unsigned opcodes;
    unsigned row;
    unsigned remaining;
    bool fixed;
} PrimaryWaveClock;

static void WaveOpcode(PrimaryWaveClock *wave, uint16_t pc, unsigned fetched,
                        unsigned cycles, unsigned data_bytes) {
    assert(wave->span.count < 28u);
    wave->timing[wave->span.count++] = (FixedSpanOpcode){pc,
        (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data_bytes};
}

static void WavePrefix(PrimaryWaveClock *wave) {
    WaveOpcode(wave, 0xa8eau, 2u, wave->fixed ? 3u : 2u, 0u);
    if (wave->fixed) {
        static const FixedSpanOpcode fixed[] = {
            {0xa933u, 1u, 4u, 2u}, {0xa934u, 1u, 3u, 1u},
            {0xa935u, 2u, 2u, 0u}, {0xa937u, 1u, 3u, 1u},
            {0xa938u, 1u, 4u, 1u}, {0xa939u, 3u, 3u, 0u},
            {0xa93cu, 2u, 3u, 0u}, {0xa93eu, 3u, 3u, 0u},
            {0xa941u, 3u, 3u, 0u}
        };
        for (unsigned i = 0u; i < sizeof fixed / sizeof fixed[0]; ++i)
            wave->timing[wave->span.count++] = fixed[i];
    } else {
        static const FixedSpanOpcode moving[] = {
            {0xa8ecu, 3u, 4u, 1u}, {0xa8efu, 1u, 4u, 2u},
            {0xa8f0u, 2u, 3u, 0u}, {0xa8f2u, 1u, 2u, 0u},
            {0xa8f3u, 2u, 2u, 0u}, {0xa8f5u, 2u, 3u, 1u},
            {0xa8f7u, 2u, 2u, 0u}, {0xa8f9u, 1u, 2u, 0u},
            {0xa8fau, 2u, 3u, 1u}, {0xa8fcu, 2u, 3u, 1u},
            {0xa8feu, 1u, 2u, 0u}, {0xa8ffu, 2u, 3u, 1u},
            {0xa901u, 1u, 2u, 0u}, {0xa902u, 1u, 2u, 0u},
            {0xa903u, 1u, 2u, 0u}, {0xa904u, 1u, 2u, 0u},
            {0xa905u, 2u, 2u, 0u}, {0xa907u, 1u, 2u, 0u},
            {0xa908u, 1u, 3u, 1u}, {0xa909u, 2u, 2u, 0u},
            {0xa90bu, 1u, 3u, 1u}, {0xa90cu, 1u, 4u, 1u},
            {0xa90du, 2u, 3u, 0u}, {0xa90fu, 4u, 6u, 2u},
            {0xa913u, 2u, 2u, 0u}
        };
        for (unsigned i = 0u; i < sizeof moving / sizeof moving[0]; ++i)
            wave->timing[wave->span.count++] = moving[i];
    }
    wave->stage = WAVE_PREFIX;
}

static void WaveFixedRow(PrimaryWaveClock *wave) {
    const unsigned row = wave->row;
    if (row == 8u || row == 64u) {
        const uint16_t pc = row == 8u ? 0xa94cu : 0xa95au;
        WaveOpcode(wave, pc, 3u, 3u, 0u);
        WaveOpcode(wave, (uint16_t)(pc + 3u), 3u, 3u, 0u);
    }
    const uint16_t pc = row < 8u ? 0xa944u : row < 64u ? 0xa952u : 0xa960u;
    WaveOpcode(wave, pc, 3u, 7u, 2u);
    WaveOpcode(wave, (uint16_t)(pc + 3u), 1u, 2u, 0u);
    WaveOpcode(wave, (uint16_t)(pc + 4u), 1u, 2u, 0u);
    WaveOpcode(wave, (uint16_t)(pc + 5u), 1u, 2u, 0u);
    const bool band_end = row == 7u || row == 63u || row == 71u;
    WaveOpcode(wave, (uint16_t)(pc + 6u), 2u, band_end ? 2u : 3u, 0u);
}

static void WaveMovingRow(PrimaryWaveClock *wave) {
    assert(wave->remaining);
    WaveOpcode(wave, 0xa915u, 3u, wave->row >= 26u ? 7u : 6u, 2u);
    WaveOpcode(wave, 0xa918u, 1u, 2u, 0u);
    --wave->remaining;
    WaveOpcode(wave, 0xa919u, 2u, wave->remaining ? 3u : 2u, 0u);
    if (!wave->remaining) {
        WaveOpcode(wave, 0xa91bu, 2u, 2u, 0u);
        WaveOpcode(wave, 0xa91du, 1u, 2u, 0u);
        WaveOpcode(wave, 0xa91eu, 3u, 3u, 0u);
        wave->remaining = 12u;
    }
    WaveOpcode(wave, 0xa921u, 1u, 2u, 0u);
    WaveOpcode(wave, 0xa922u, 1u, 2u, 0u);
    WaveOpcode(wave, 0xa923u, 2u, 2u, 0u);
    WaveOpcode(wave, 0xa925u, 2u, wave->row == 71u ? 2u : 3u, 0u);
}

static void WaveSuffix(PrimaryWaveClock *wave) {
    if (wave->fixed) {
        static const FixedSpanOpcode fixed[] = {
            {0xa968u, 2u, 3u, 0u}, {0xa96au, 1u, 4u, 1u},
            {0xa96bu, 1u, 5u, 2u}, {0xa96cu, 2u, 2u, 0u},
            {0xa96eu, 3u, 4u, 1u}
        };
        for (unsigned i = 0u; i < sizeof fixed / sizeof fixed[0]; ++i)
            wave->timing[wave->span.count++] = fixed[i];
    } else {
        static const FixedSpanOpcode moving[] = {
            {0xa927u, 1u, 4u, 1u}, {0xa928u, 2u, 3u, 0u},
            {0xa92au, 2u, 3u, 0u}, {0xa92cu, 1u, 5u, 2u},
            {0xa92du, 2u, 2u, 0u}, {0xa92fu, 3u, 4u, 1u}
        };
        for (unsigned i = 0u; i < sizeof moving / sizeof moving[0]; ++i)
            wave->timing[wave->span.count++] = moving[i];
    }
    wave->stage = WAVE_SUFFIX;
}

static void WaveNextSegment(PrimaryWaveClock *wave) {
    BeginFixedSpanOpcode(&wave->span);
    if (wave->span.remaining)
        return;
    assert(wave->span.index == wave->span.count);
    wave->opcodes += wave->span.count;
    CpuState *cpu = wave->span.cpu;
    wave->span = (FixedSpanClock){0};
    wave->span.cpu = cpu;
    wave->span.bank = 0x850000u;
    wave->span.timing = wave->timing;
    if (wave->stage == WAVE_OPEN) {
        WavePrefix(wave);
    } else if (wave->row < 72u) {
        if (wave->fixed)
            WaveFixedRow(wave);
        else
            WaveMovingRow(wave);
        ++wave->row;
        wave->stage = WAVE_ROWS;
    } else {
        assert(wave->stage == WAVE_ROWS);
        WaveSuffix(wave);
    }
    BeginFixedSpanOpcode(&wave->span);
    assert(wave->span.remaining);
}

static uint8_t WaveClockRead(void *context, uint32_t address) {
    PrimaryWaveClock *wave = context;
    const bool opening = wave->stage == WAVE_OPEN && !wave->span.index;
    if (!opening)
        WaveNextSegment(wave);
    const uint8_t value = FixedSpanRead(&wave->span, address);
    if (opening) {
        assert(address == 0x851b23u);
        wave->fixed = value != 0u;
    } else if (address == 0x851b22u) {
        wave->remaining = 3u * (4u - (value & 3u));
    }
    return value;
}

static void WaveClockWrite(void *context, uint32_t address, uint8_t value) {
    PrimaryWaveClock *wave = context;
    WaveNextSegment(wave);
    FixedSpanWrite(&wave->span, address, value);
}

static void ActorBridgePrimaryWaveClock(CpuState *cpu, uint32_t entry) {
    if (!NmiTimerClockSupported(cpu, entry, 555u, 13600u))
        return;
    PrimaryWaveClock wave = {0};
    wave.span.cpu = cpu;
    wave.span.bank = 0x850000u;
    wave.span.timing = wave.timing;
    WaveOpcode(&wave, 0xa8e7u, 3u, 4u, 1u);
    const Lufia2Memory memory = {WaveClockRead, WaveClockWrite, &wave, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleBuildPrimaryWaveTable(&memory, &state, NULL, NULL);
    BeginFixedSpanOpcode(&wave.span);
    wave.opcodes += wave.span.count;
    assert(wave.stage == WAVE_SUFFIX && wave.row == 72u && !wave.span.remaining);
    assert(wave.span.index == wave.span.count);
    assert(wave.opcodes == (wave.fixed ? 380u : 555u));
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    assert(result.pc == (wave.fixed ? 0x85a971u : 0x85a932u));
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(wave.opcodes, 0u);
    interp_bridge_pre_opcode_redirect(result.pc);
}
