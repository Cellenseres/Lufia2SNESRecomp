#include "lufia2/decompress_steps.h"

enum { DECOMPRESS_PHASE_BUDGET = 384u };

typedef struct DecompressClock {
    CpuState *cpu;
    FixedSpanClock span;
    FixedSpanOpcode timing[12];
    unsigned count, opcodes, progress_mask, stage;
    uint16_t mirror, next_destination, destination_end;
    bool symbol, source_wrap, negative;
} DecompressClock;

static void DecompressClockAppend(DecompressClock *clock,
    uint16_t pc, uint8_t fetched, uint8_t cycles, uint8_t data_bytes) {
    assert(clock->count < 12u);
    clock->timing[clock->count++] = (FixedSpanOpcode){
        (uint16_t)(pc + clock->mirror), fetched, cycles, data_bytes};
}

static void DecompressClockBegin(DecompressClock *clock) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->span.count = clock->count;
}

static void DecompressClockCollect(DecompressClock *clock) {
    assert(clock->opcodes + clock->count <= 11u);
    clock->progress_mask |= clock->span.progress_mask << clock->opcodes;
    clock->opcodes += clock->count;
    clock->count = 0u;
}

static void DecompressSymbolTail(DecompressClock *clock) {
    const bool at_end = clock->next_destination == clock->destination_end;
    DecompressClockAppend(clock, 0x8f04u, 2u, at_end ? 3u : 2u, 0u);
    if (!at_end) {
        DecompressClockAppend(clock, 0x8f06u, 1u, 2u, 0u);
        DecompressClockAppend(clock, 0x8f07u, 2u,
            clock->source_wrap ? 2u : 3u, 0u);
        if (clock->source_wrap) {
            DecompressClockAppend(clock, 0x8f09u, 3u, 3u, 0u);
            DecompressClockAppend(clock, 0x8f0cu, 2u, 5u, 2u);
            DecompressClockAppend(clock, 0x8f0eu, 2u, 3u, 0u);
        }
    }
}

static void DecompressClockPrepare(DecompressClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count)
        return;
    DecompressClockCollect(clock);
    if (!clock->symbol) {
        clock->stage = 3u;
        return;
    }
    if (!clock->stage) {
        DecompressClockAppend(clock, 0x8efcu, 2u,
            clock->negative ? 3u : 2u, 0u);
        if (!clock->negative) {
            DecompressClockAppend(clock, 0x8efeu, 3u, 6u, 1u);
            DecompressClockAppend(clock, 0x8f01u, 1u, 2u, 0u);
            DecompressClockAppend(clock, 0x8f02u, 2u, 4u, 2u);
        }
        clock->stage = 1u;
    } else if (clock->stage == 1u && !clock->negative) {
        DecompressSymbolTail(clock);
        clock->stage = 2u;
    } else {
        clock->stage = 3u;
        return;
    }
    DecompressClockBegin(clock);
}

static uint8_t DecompressClockRead(void *context, uint32_t address) {
    DecompressClock *clock = context;
    DecompressClockPrepare(clock);
    assert(clock->stage != 3u);
    const unsigned opcode = clock->span.index;
    const unsigned remaining = clock->span.remaining;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (clock->symbol && !clock->stage && !clock->span.remaining)
        clock->negative = (value & 0x80u) != 0u;
    if (clock->symbol && clock->stage == 1u && opcode == 3u) {
        if (remaining == 2u)
            clock->destination_end = value;
        else
            clock->destination_end |= (uint16_t)value << 8;
    }
    return value;
}

static void DecompressClockWrite(void *context, uint32_t address, uint8_t value) {
    DecompressClock *clock = context;
    DecompressClockPrepare(clock);
    assert(clock->stage != 3u);
    FixedSpanWrite(&clock->span, address, value);
}

static void DecompressControlTiming(DecompressClock *clock) {
    DecompressClockAppend(clock, 0x8eefu, 2u, 6u, 4u);
    DecompressClockAppend(clock, 0x8ef1u, 1u, 2u, 0u);
    DecompressClockAppend(clock, 0x8ef2u, 2u, clock->source_wrap ? 3u : 2u, 0u);
    if (clock->source_wrap) {
        DecompressClockAppend(clock, 0x8f24u, 3u, 3u, 0u);
        DecompressClockAppend(clock, 0x8f27u, 2u, 5u, 2u);
        DecompressClockAppend(clock, 0x8f29u, 2u, 3u, 0u);
    }
    DecompressClockAppend(clock, 0x8ef4u, 2u, 3u, 1u);
    DecompressClockAppend(clock, 0x8ef6u, 2u, 2u, 0u);
    DecompressClockAppend(clock, 0x8ef8u, 2u, 3u, 1u);
}

static void DecompressCopyTiming(DecompressClock *clock) {
    DecompressClockAppend(clock, 0x8f66u, 3u, 7u, 2u);
    if (!clock->cpu->A) {
        DecompressClockAppend(clock, 0x8f69u, 1u, 2u, 0u);
        DecompressClockAppend(clock, 0x8f6au, 1u, 5u, 2u);
        DecompressClockAppend(clock, 0x8f6bu, 2u, 3u, 0u);
        DecompressClockAppend(clock, 0x8f6du, 2u, 3u, 0u);
    }
}

static bool DecompressClockSupported(const CpuState *cpu, uint32_t entry) {
    const uint32_t mirror = cpu->DB == 0x7fu ? 0xa4u : 0u;
    if (cpu->DB != 0x7eu && cpu->DB != 0x7fu)
        return false;
    const uint32_t first = entry - mirror;
    const bool copy = first == 0x808f66u;
    if ((!copy && first != 0x808eefu && first != 0x808efau) ||
        cpu->D || cpu->_flag_D || cpu->x_flag || cpu->m_flag != !copy ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->master_cycles > UINT64_MAX - DECOMPRESS_PHASE_BUDGET ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    if (!cpu->ram || (!copy &&
        (cpu->ram[0x5du] || cpu->ram[0x5eu] ||
         cpu->ram[0x5fu] < 0x80u || cpu->ram[0x5fu] >= 0xd0u ||
         cpu->Y < 0x8000u)) ||
        (first == 0x808efau && cpu->X < 0x2000u))
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= 1364u || refresh_clock != cpu->master_cycles ||
        !g_snes || g_snes->vPos >= 262u)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu, true,
        fence - DECOMPRESS_PHASE_BUDGET - 1u, DECOMPRESS_PHASE_BUDGET);
}

static void ActorBridgeDecompressClock(CpuState *cpu, uint32_t entry) {
    if (!DecompressClockSupported(cpu, entry))
        return;
    DecompressClock clock = {0};
    clock.cpu = cpu;
    clock.mirror = cpu->DB == 0x7fu ? 0xa4u : 0u;
    clock.source_wrap = cpu->Y == 0xffffu;
    clock.next_destination = (uint16_t)(cpu->X + 1u);
    const uint32_t first = entry - clock.mirror;
    if (first == 0x808eefu)
        DecompressControlTiming(&clock);
    else if (first == 0x808efau) {
        clock.symbol = true;
        DecompressClockAppend(&clock, 0x8efau, 2u, 6u, 4u);
    } else
        DecompressCopyTiming(&clock);
    DecompressClockBegin(&clock);
    const Lufia2Memory memory = {
        DecompressClockRead, DecompressClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = first == 0x808eefu
        ? Lufia2DecompressControlStep(&memory, &state, entry)
        : clock.symbol ? Lufia2DecompressSymbolStep(&memory, &state, entry)
        : Lufia2DecompressCopyByteStep(&memory, &state, entry);
    assert(!clock.span.remaining);
    while (clock.stage != 3u) {
        DecompressClockPrepare(&clock);
        assert(!clock.span.remaining);
    }
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && clock.opcodes <= 11u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
