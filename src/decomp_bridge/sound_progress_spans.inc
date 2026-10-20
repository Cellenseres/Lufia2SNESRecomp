#include "lufia2/sound_progress.h"
#include "buffer_span_bus.h"

typedef struct SoundProgressBus {
    Interp816 *registers;
} SoundProgressBus;

static uint8_t SoundProgressRead(void *context, uint32_t address) {
    SoundProgressBus *bus = context;
    BufferSpanFetch(bus->registers, 0x8098ffu, 2u);
    return bus->registers->read(bus->registers->mem, address);
}

static void SoundProgressWrite(void *context, uint32_t address, uint8_t value) {
    SoundProgressBus *bus = context;
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunSoundProgressSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    if (pc != 0x8098fcu || opcode != 0xc8u || registers->k != 0x80u ||
        registers->pc != (uint16_t)pc || registers->e || !registers->mf || registers->xf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted)
        return 0;
    const bool wrap = registers->y == 0xffffu;
    if (wrap && (uint16_t)(registers->dp + 0x5fu) >= 0x2000u)
        return 0;
    const bool more = registers->x != 1u;
    const unsigned cycles = (wrap ? 16u : 9u) + more +
        (wrap && (registers->dp & 0xffu) != 0u);
    if (BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;
    BufferSpanFetch(registers, 0x8098fcu, 1u);
    BufferSpanFetch(registers, 0x8098fdu, 2u);
    SoundProgressBus bus = {registers};
    const Lufia2Memory memory = {SoundProgressRead, SoundProgressWrite, &bus, NULL, NULL};
    Lufia2CpuState state = BufferSpanLoadState(registers);
    const Lufia2SoundHeaderProgress progress = Lufia2SoundAdvanceHeader(
        &memory, registers->dp, registers->y, registers->x);
    if (wrap)
        BufferSpanFetch(registers, 0x809901u, 3u);
    BufferSpanFetch(registers, 0x809904u, 1u);
    BufferSpanFetch(registers, 0x809905u, 2u);
    state.y = progress.source;
    state.x = progress.remaining;
    state.negative = (state.x & 0x8000u) != 0u;
    state.zero = state.x == 0u;
    BufferSpanStoreState(registers, &state, more ? 0x8098ecu : 0x809907u, more ? 3u : 2u);
    return (int)cycles;
}
