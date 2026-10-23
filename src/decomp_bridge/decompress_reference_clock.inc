#include "lufia2/decompress_steps.h"

typedef enum DecompressReferencePhase {
    DECOMPRESS_REFERENCE_HEADER,
    DECOMPRESS_LONG_REFERENCE,
    DECOMPRESS_COPY_SETUP
} DecompressReferencePhase;

typedef struct DecompressReferenceClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[32];
    unsigned count, opcodes, progress_mask, stage;
    uint16_t mirror;
    DecompressReferencePhase phase;
    bool long_form;
} DecompressReferenceClock;

static void ReferenceAppend(DecompressReferenceClock *clock,
    uint16_t pc, uint8_t fetched, uint8_t cycles, uint8_t data_bytes) {
    assert(clock->count < 32u);
    clock->timing[clock->count++] = (FixedSpanOpcode){
        (uint16_t)(pc + clock->mirror), fetched, cycles, data_bytes};
}

static void ReferenceBegin(DecompressReferenceClock *clock, CpuState *cpu) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->span.count = clock->count;
}

static void ReferenceNext(DecompressReferenceClock *clock,
    uint16_t increment, uint16_t branch, uint16_t rollover, bool wraps) {
    ReferenceAppend(clock, increment, 1u, 2u, 0u);
    ReferenceAppend(clock, branch, 2u, wraps ? 3u : 2u, 0u);
    if (wraps) {
        ReferenceAppend(clock, rollover, 3u, 3u, 0u);
        ReferenceAppend(clock, (uint16_t)(rollover + 3u), 2u, 5u, 2u);
        ReferenceAppend(clock, (uint16_t)(rollover + 5u), 2u, 3u, 0u);
    }
}

static void ReferenceHeaderTiming(DecompressReferenceClock *clock, bool wraps) {
    ReferenceNext(clock, 0x8f44u, 0x8f45u, 0x8f32u, wraps);
    ReferenceAppend(clock, 0x8f47u, 1u, 3u, 0u);
    ReferenceAppend(clock, 0x8f48u, 2u, 6u, 4u);
    ReferenceAppend(clock, 0x8f4au, 2u, 2u, 0u);
}

static void ReferenceLongTiming(DecompressReferenceClock *clock, bool wraps) {
    ReferenceAppend(clock, 0x8f6fu, 2u, 6u, 4u);
    ReferenceAppend(clock, 0x8f71u, 1u, 4u, 2u);
    ReferenceAppend(clock, 0x8f72u, 1u, 2u, 0u);
    ReferenceNext(clock, 0x8f73u, 0x8f74u, 0x8f39u, wraps);
    ReferenceAppend(clock, 0x8f76u, 2u, 6u, 4u);
    ReferenceAppend(clock, 0x8f78u, 2u, 3u, 1u);
    ReferenceAppend(clock, 0x8f7au, 2u, 2u, 0u);
    ReferenceAppend(clock, 0x8f7cu, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f7du, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f7eu, 2u, 3u, 1u);
    ReferenceAppend(clock, 0x8f80u, 2u, 3u, 0u);
    ReferenceAppend(clock, 0x8f82u, 1u, 2u, 0u);
    for (uint16_t pc = 0x8f83u; pc <= 0x8f86u; ++pc)
        ReferenceAppend(clock, pc, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f87u, 3u, 3u, 0u);
    ReferenceAppend(clock, 0x8f8au, 2u, 7u, 4u);
    ReferenceAppend(clock, 0x8f8cu, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f8du, 2u, 7u, 4u);
    ReferenceAppend(clock, 0x8f8fu, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f90u, 1u, 5u, 2u);
    ReferenceAppend(clock, 0x8f91u, 2u, 3u, 0u);
    clock->stage = 1u;
}

static void ReferenceSetupTiming(DecompressReferenceClock *clock) {
    ReferenceAppend(clock, 0x8f5cu, 2u, 4u, 2u);
    ReferenceAppend(clock, 0x8f5eu, 1u, 4u, 2u);
    ReferenceAppend(clock, 0x8f5fu, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f60u, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f61u, 2u, 4u, 2u);
    ReferenceAppend(clock, 0x8f63u, 1u, 2u, 0u);
    ReferenceAppend(clock, 0x8f64u, 2u, 4u, 2u);
    clock->stage = 1u;
}

static void ReferencePrepare(DecompressReferenceClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count)
        return;
    assert(clock->opcodes + clock->count <= 32u);
    clock->progress_mask |= clock->span.progress_mask << clock->opcodes;
    clock->opcodes += clock->count;
    clock->count = 0u;
    if (clock->stage) {
        clock->stage = 2u;
        return;
    }
    ReferenceAppend(clock, 0x8f4cu, 2u, clock->long_form ? 3u : 2u, 0u);
    if (!clock->long_form) {
        ReferenceAppend(clock, 0x8f4eu, 1u, 2u, 0u);
        ReferenceAppend(clock, 0x8f4fu, 2u, 3u, 1u);
        ReferenceAppend(clock, 0x8f51u, 2u, 6u, 4u);
        ReferenceAppend(clock, 0x8f53u, 2u, 3u, 0u);
        for (uint16_t pc = 0x8f55u; pc <= 0x8f58u; ++pc)
            ReferenceAppend(clock, pc, 1u, 2u, 0u);
        ReferenceAppend(clock, 0x8f59u, 3u, 3u, 0u);
    }
    clock->stage = 1u;
    ReferenceBegin(clock, clock->span.cpu);
}

static uint8_t ReferenceRead(void *context, uint32_t address) {
    DecompressReferenceClock *clock = context;
    ReferencePrepare(clock);
    assert(clock->stage != 2u);
    BeginFixedSpanOpcode(&clock->span);
    const uint16_t pc = clock->span.timing[clock->span.index].pc;
    const unsigned remaining = clock->span.remaining;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (clock->phase == DECOMPRESS_REFERENCE_HEADER && !clock->stage &&
        pc == (uint16_t)(0x8f48u + clock->mirror) && remaining == 1u)
        clock->long_form = (value & 0x0fu) == 0u;
    return value;
}

static void ReferenceWrite(void *context, uint32_t address, uint8_t value) {
    DecompressReferenceClock *clock = context;
    ReferencePrepare(clock);
    assert(clock->stage != 2u);
    FixedSpanWrite(&clock->span, address, value);
}

static bool ReferenceClockSupported(const CpuState *cpu, uint32_t entry,
    DecompressReferencePhase *phase) {
    if (cpu->DB != 0x7eu && cpu->DB != 0x7fu)
        return false;
    const uint32_t first = entry - (cpu->DB == 0x7fu ? 0xa4u : 0u);
    if (first == 0x808f44u)
        *phase = DECOMPRESS_REFERENCE_HEADER;
    else if (first == 0x808f6fu)
        *phase = DECOMPRESS_LONG_REFERENCE;
    else if (first == 0x808f5cu)
        *phase = DECOMPRESS_COPY_SETUP;
    else
        return false;
    const bool setup = *phase == DECOMPRESS_COPY_SETUP;
    const unsigned budget = setup ? 256u :
        *phase == DECOMPRESS_LONG_REFERENCE ? 768u : 512u;
    if (cpu->D || cpu->_flag_D || cpu->x_flag || cpu->m_flag != !setup ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->master_cycles > UINT64_MAX - budget ||
        cpu->coprocessor_master_cycles != cpu->master_cycles || !cpu->ram)
        return false;
    if (!setup && (cpu->ram[0x5du] || cpu->ram[0x5eu] ||
        cpu->ram[0x5fu] < 0x80u || cpu->ram[0x5fu] >= 0xd0u ||
        cpu->Y < 0x8000u ||
        (cpu->Y == 0xffffu && cpu->ram[0x5fu] == 0xcfu)))
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= 1364u || refresh_clock != cpu->master_cycles ||
        !g_snes || g_snes->vPos >= 262u)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu, true,
        fence - budget - 1u, budget);
}

static void ActorBridgeDecompressReferenceClock(CpuState *cpu, uint32_t entry) {
    DecompressReferencePhase phase;
    if (!ReferenceClockSupported(cpu, entry, &phase))
        return;
    DecompressReferenceClock clock = {0};
    clock.phase = phase;
    clock.mirror = cpu->DB == 0x7fu ? 0xa4u : 0u;
    if (phase == DECOMPRESS_REFERENCE_HEADER)
        ReferenceHeaderTiming(&clock, cpu->Y == 0xffffu);
    else if (phase == DECOMPRESS_LONG_REFERENCE)
        ReferenceLongTiming(&clock, cpu->Y == 0xffffu);
    else
        ReferenceSetupTiming(&clock);
    ReferenceBegin(&clock, cpu);
    const Lufia2Memory memory = {ReferenceRead, ReferenceWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = phase == DECOMPRESS_REFERENCE_HEADER
        ? Lufia2DecompressReferenceStep(&memory, &state, entry)
        : phase == DECOMPRESS_LONG_REFERENCE
        ? Lufia2DecompressLongReferenceStep(&memory, &state, entry)
        : Lufia2DecompressSetupCopyStep(&memory, &state, entry);
    assert(!clock.span.remaining);
    while (clock.stage != 2u) {
        ReferencePrepare(&clock);
        assert(!clock.span.remaining);
    }
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && clock.opcodes <= 27u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
