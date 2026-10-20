#include "lufia2/session_actors.h"
#include "buffer_span_bus.h"

typedef struct SessionActorBus {
    Interp816 *registers;
    unsigned reads;
    unsigned writes;
    bool reserved;
} SessionActorBus;

static uint8_t SessionActorRead(void *context, uint32_t address) {
    SessionActorBus *bus = context;
    const unsigned field = bus->reads++;
    const uint32_t pc = bus->reserved
        ? 0x83b3b5u + field * 6u : field ? 0x83b36au : 0x83b364u;
    BufferSpanFetch(bus->registers, pc, 3u);
    return bus->registers->read(bus->registers->mem, address);
}

static void SessionActorWrite(void *context, uint32_t address, uint8_t value) {
    SessionActorBus *bus = context;
    const unsigned field = bus->writes++;
    if (bus->reserved) {
        if (!field)
            BufferSpanFetch(bus->registers, 0x83b3b0u, 2u);
        else if (field == 4u)
            BufferSpanFetch(bus->registers, 0x83b3cau, 2u);
        BufferSpanFetch(bus->registers,
            field == 4u ? 0x83b3ccu : 0x83b3b2u + field * 6u, 3u);
    } else {
        if (field == 1u)
            BufferSpanFetch(bus->registers, 0x83b36du, 2u);
        BufferSpanFetch(bus->registers,
            !field ? 0x83b367u : field == 1u ? 0x83b36fu : 0x83b372u, 3u);
    }
    bus->registers->write(bus->registers->mem, address, value);
}

static bool SessionActorRam(uint8_t bank, uint16_t field, uint16_t slot) {
    const uint32_t address = (((uint32_t)bank << 16) + field + slot) & 0xffffffu;
    const uint8_t mapped_bank = (uint8_t)(address >> 16);
    return mapped_bank == 0x7eu || mapped_bank == 0x7fu ||
        ((mapped_bank < 0x40u || (mapped_bank >= 0x80u && mapped_bank < 0xc0u)) &&
         (uint16_t)address < 0x2000u);
}

static bool SessionActorRecordRam(const Interp816 *registers, bool reserved) {
    if (reserved) {
        static const uint16_t sources[] = {0x091du, 0x08ddu, 0x085du};
        static const uint16_t destinations[] = {0x05fau, 0x05d2u, 0x06bau, 0x06e2u, 0x0622u};
        for (unsigned field = 0u; field < sizeof sources / sizeof sources[0]; ++field)
            if (!SessionActorRam(registers->db, sources[field], 0u))
                return false;
        for (unsigned field = 0u; field < sizeof destinations / sizeof destinations[0]; ++field)
            if (!SessionActorRam(registers->db, destinations[field], registers->x))
                return false;
        return true;
    }
    return SessionActorRam(registers->db, 0x08deu, registers->x) &&
        SessionActorRam(registers->db, 0x081eu, registers->x) &&
        SessionActorRam(registers->db, 0x05d2u, registers->y) &&
        SessionActorRam(registers->db, 0x05fau, registers->y);
}

static unsigned SessionActorPage(uint16_t field, uint16_t index) {
    return ((uint32_t)field >> 8) != (((uint32_t)field + index) >> 8);
}

static unsigned SessionActorStore(const Interp816 *registers, uint16_t field, uint16_t index) {
    return !registers->xf || SessionActorPage(field, index);
}

static unsigned SessionActorPresentation(
    Interp816 *registers, Lufia2CpuState *state, uint8_t *last_cycles) {
    BufferSpanFetch(registers, 0x83b327u, 4u);
    const Lufia2SessionPresentation presentation = Lufia2FieldSelectSessionPresentation(
        registers->read(registers->mem, 0x0005b2u));
    BufferSpanFetch(registers, 0x83b32bu, 2u);
    BufferSpanFetch(registers, 0x83b32du, 2u);
    BufferSpanFetch(registers, 0x83b32fu, 2u);
    unsigned cycles = 11u;
    if (presentation.mode == 4u) {
        BufferSpanFetch(registers, 0x83b331u, 2u);
        BufferSpanFetch(registers, 0x83b333u, 2u);
        cycles += 5u;
    } else {
        ++cycles;
        BufferSpanFetch(registers, 0x83b335u, 2u);
        BufferSpanFetch(registers, 0x83b337u, 2u);
        cycles += 4u;
        if (presentation.mode == 5u) {
            BufferSpanFetch(registers, 0x83b339u, 2u);
            BufferSpanFetch(registers, 0x83b33bu, 2u);
            cycles += 5u;
        } else {
            ++cycles;
            BufferSpanFetch(registers, 0x83b33du, 2u);
            BufferSpanFetch(registers, 0x83b33fu, 2u);
            cycles += 4u;
            if (presentation.mode == 6u) {
                BufferSpanFetch(registers, 0x83b341u, 2u);
                cycles += 2u;
            } else {
                ++cycles;
            }
        }
    }
    if (presentation.changed) {
        BufferSpanFetch(registers, 0x83b343u, 4u);
        registers->write(registers->mem, 0x7fd0feu, presentation.value);
        cycles += 5u;
    }
    state->accumulator = (uint16_t)((state->accumulator & 0xff00u) | presentation.value);
    state->carry = presentation.mode >= 4u;
    state->negative = presentation.mode < 4u;
    state->zero = 0u;
    *last_cycles = presentation.changed ? 5u : 3u;
    return cycles;
}

static int Lufia2DecompRunSessionActorSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        uint32_t finish;
        uint8_t opcode;
        uint8_t maximum_cycles;
    } operations[] = {
        {0x83b327u, 0x83b347u, 0xafu, 28u},
        {0x83b364u, 0x83b375u, 0xbdu, 30u},
        {0x83b3b0u, 0x83b3cfu, 0xa9u, 51u}
    };
    unsigned operation = 0u;
    while (operation < sizeof operations / sizeof operations[0] && operations[operation].pc != pc)
        ++operation;
    if (operation == sizeof operations / sizeof operations[0] ||
        opcode != operations[operation].opcode || registers->pc != (uint16_t)pc ||
        registers->k != 0x83u || registers->e || !registers->mf ||
        registers->waiting || registers->stopped || registers->nmiWanted || registers->irqWanted ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < operations[operation].maximum_cycles)
        return 0;
    const bool reserved = operation == 2u;
    if (operation && !SessionActorRecordRam(registers, reserved))
        return 0;
    Lufia2CpuState state = BufferSpanLoadState(registers);
    uint8_t last_cycles;
    unsigned cycles;
    if (!operation) {
        cycles = SessionActorPresentation(registers, &state, &last_cycles);
    } else {
        SessionActorBus bus = {registers, 0u, 0u, reserved};
        const Lufia2Memory memory = {SessionActorRead, SessionActorWrite, &bus, NULL, NULL};
        const uint8_t value = reserved
            ? Lufia2FieldRestoreReservedActor(&memory, registers->db, registers->x)
            : Lufia2FieldRestoreSessionActor(&memory, registers->db, registers->x, registers->y);
        state.accumulator = (uint16_t)((state.accumulator & 0xff00u) | value);
        state.zero = value == 0u;
        state.negative = (value & 0x80u) != 0u;
        if (reserved) {
            cycles = 45u + SessionActorPage(0x0622u, registers->x);
            static const uint16_t fields[] = {0x05fau, 0x05d2u, 0x06bau, 0x06e2u, 0x0622u};
            for (unsigned field = 0u; field < sizeof fields / sizeof fields[0]; ++field)
                cycles += SessionActorStore(registers, fields[field], registers->x);
            last_cycles = (uint8_t)(5u + SessionActorStore(registers, 0x0622u, registers->x));
        } else {
            cycles = 25u + SessionActorPage(0x08deu, registers->x) +
                SessionActorPage(0x081eu, registers->x) +
                SessionActorStore(registers, 0x05d2u, registers->y) +
                SessionActorStore(registers, 0x081eu, registers->x) +
                SessionActorStore(registers, 0x05fau, registers->y);
            last_cycles = (uint8_t)(5u + SessionActorStore(registers, 0x05fau, registers->y));
        }
    }
    BufferSpanStoreState(registers, &state, operations[operation].finish, last_cycles);
    return (int)cycles;
}
