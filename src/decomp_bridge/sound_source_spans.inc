#include "lufia2/sound_source.h"
#include "buffer_span_bus.h"

typedef struct SoundSourceBus {
    Interp816 *registers;
    uint32_t first;
    uint32_t second;
    unsigned reads;
    unsigned writes;
} SoundSourceBus;

static uint8_t SoundSourceRead(void *context, uint32_t address) {
    SoundSourceBus *bus = context;
    if (bus->reads) {
        BufferSpanFetch(bus->registers, bus->first + 5u, 1u);
        BufferSpanFetch(bus->registers, bus->first + 6u, 2u);
    }
    BufferSpanFetch(bus->registers, bus->reads++ ? bus->second : bus->first, 3u);
    return bus->registers->read(bus->registers->mem, address);
}

static void SoundSourceWrite(void *context, uint32_t address, uint8_t value) {
    SoundSourceBus *bus = context;
    BufferSpanFetch(bus->registers, (bus->writes++ ? bus->second : bus->first) + 3u, 2u);
    bus->registers->write(bus->registers->mem, address, value);
}

static bool SoundSourceMemory(uint8_t bank, uint16_t source) {
    if (bank == 0x7eu || bank == 0x7fu)
        return true;
    if ((bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)) && source < 0x1fffu)
        return true;
    return (bank & 0x7fu) < 0x50u && source >= 0x8000u;
}

static int Lufia2DecompRunSoundSourceSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t first;
        uint32_t second;
        uint32_t finish;
    } operations[] = {
        {0x809953u, 0x80995eu, 0x809969u},
        {0x80997du, 0x809988u, 0x809993u},
        {0x809815u, 0x809823u, 0x809831u},
        {0x809848u, 0x809856u, 0x809864u}
    };
    unsigned operation = 0u;
    while (operation < sizeof operations / sizeof operations[0] && operations[operation].first != pc)
        ++operation;
    if (operation == sizeof operations / sizeof operations[0] || opcode != 0xb9u ||
        registers->k != 0x80u || registers->pc != (uint16_t)pc ||
        registers->e || !registers->mf || registers->xf || registers->y > 0xfffdu ||
        !SoundSourceMemory(registers->db, registers->y) || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted)
        return 0;
    const unsigned cycles = 24u + (registers->y >= 0x100u) +
        (registers->y + 1u >= 0x100u) + 2u * ((registers->dp & 0xffu) != 0u);
    if (BufferSpanCycleBudget(cpu, s_lle_master_deadline) < cycles)
        return 0;
    SoundSourceBus bus = {registers, pc, operations[operation].second, 0u, 0u};
    const Lufia2Memory memory = {SoundSourceRead, SoundSourceWrite, &bus, NULL, NULL};
    Lufia2CpuState state = BufferSpanLoadState(registers);
    const Lufia2SoundSourcePair pair = Lufia2SoundReadSourcePair(
        &memory, registers->db, registers->dp, registers->y);
    BufferSpanFetch(registers, bus.second + 5u, 1u);
    BufferSpanFetch(registers, bus.second + 6u, 2u);
    state.accumulator = (uint16_t)((state.accumulator & 0xff00u) | pair.last_byte);
    state.y = pair.next;
    state.negative = (state.y & 0x8000u) != 0u;
    state.zero = 0u;
    BufferSpanStoreState(registers, &state, operations[operation].finish, 3u);
    return (int)cycles;
}
