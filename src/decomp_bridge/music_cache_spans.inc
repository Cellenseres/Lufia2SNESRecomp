#include "lufia2/music_samples.h"
#include "buffer_span_bus.h"

typedef struct MusicCacheBus {
    Interp816 *registers;
    unsigned slots;
    bool pending;
} MusicCacheBus;

static void MusicCacheTail(MusicCacheBus *bus, bool search, bool found) {
    if (!bus->pending)
        return;
    if (search)
        BufferSpanFetch(bus->registers, 0x8094e2u, 2u);
    if (!found) {
        BufferSpanFetch(bus->registers, search ? 0x8094e4u : 0x8094b4u, 1u);
        BufferSpanFetch(bus->registers, search ? 0x8094e5u : 0x8094b5u, 2u);
    }
    bus->pending = false;
}

static uint8_t MusicCacheRead(void *context, uint32_t address) {
    MusicCacheBus *bus = context;
    MusicCacheTail(bus, true, false);
    BufferSpanFetch(bus->registers, 0x8094deu, 4u);
    ++bus->slots;
    bus->pending = true;
    return bus->registers->read(bus->registers->mem, address);
}

static void MusicCacheWrite(void *context, uint32_t address, uint8_t value) {
    MusicCacheBus *bus = context;
    MusicCacheTail(bus, false, false);
    BufferSpanFetch(bus->registers, 0x8094b0u, 4u);
    bus->registers->write(bus->registers->mem, address, value);
    ++bus->slots;
    bus->pending = true;
}

static int Lufia2DecompRunMusicCacheSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    const bool search = pc == 0x8094deu;
    if ((!search && pc != 0x8094b0u) || registers->k != 0x80u ||
        registers->pc != (uint16_t)pc || opcode != (search ? 0xdfu : 0x9fu) ||
        registers->e || !registers->mf || registers->xf || registers->x > 31u ||
        (!search && (uint8_t)registers->a != 0xffu) ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) <
            (registers->x + 1u) * (search ? 12u : 10u) - 1u)
        return 0;
    MusicCacheBus bus = {registers, 0u, false};
    const Lufia2Memory memory = {MusicCacheRead, MusicCacheWrite, &bus, NULL, NULL};
    Lufia2CpuState state = BufferSpanLoadState(registers);
    uint32_t finish;
    unsigned cycles;
    uint8_t last_cycles;
    if (search) {
        const Lufia2MusicSampleSearch cached = Lufia2MusicFindSample(
            &memory, registers->x, (uint8_t)registers->a);
        MusicCacheTail(&bus, true, cached.found);
        state.x = cached.slot;
        state.carry = (uint8_t)state.accumulator >= cached.value;
        state.zero = cached.found;
        state.negative = !cached.found;
        finish = cached.found ? 0x8094ffu : 0x8094e7u;
        cycles = cached.found ? (bus.slots - 1u) * 12u + 8u : bus.slots * 12u - 1u;
        last_cycles = cached.found ? 3u : 2u;
    } else {
        state.x = Lufia2MusicClearSampleSlots(&memory, registers->x);
        MusicCacheTail(&bus, false, false);
        state.negative = 1u;
        state.zero = 0u;
        finish = 0x8094b7u;
        cycles = bus.slots * 10u - 1u;
        last_cycles = 2u;
    }
    BufferSpanStoreState(registers, &state, finish, last_cycles);
    return (int)cycles;
}
