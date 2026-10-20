#include "lufia2/scene_rebuild.h"
#include "buffer_span_bus.h"

enum { SCENE_REBUILD_MAX_SCAN = 8u };

typedef struct SceneRebuildBus {
    Interp816 *registers;
    Lufia2SceneRebuildPhase phase;
    unsigned reads;
    unsigned writes;
    unsigned cycles;
    uint8_t slot;
    uint8_t sprite;
    bool carry_entry;
    bool hidden;
    bool pending_scan;
} SceneRebuildBus;

static void SceneRebuildFetch(SceneRebuildBus *bus, uint32_t pc, unsigned bytes) {
    BufferSpanFetch(bus->registers, pc, bytes);
}

static void SceneRebuildFinishScan(SceneRebuildBus *bus) {
    if (!bus->pending_scan)
        return;
    SceneRebuildFetch(bus, 0x83a83eu, 2u);
    SceneRebuildFetch(bus, 0x83a840u, 2u);
    if (bus->sprite == 0xfeu) {
        SceneRebuildFetch(bus, 0x83a842u, 3u);
        bus->cycles += 12u;
    } else {
        SceneRebuildFetch(bus, 0x83a845u, 2u);
        SceneRebuildFetch(bus, 0x83a847u, 2u);
        if (bus->sprite != 0xffu) {
            bus->cycles += 14u;
            bus->pending_scan = false;
            return;
        }
        SceneRebuildFetch(bus, 0x83a849u, 3u);
        bus->cycles += 17u;
    }
    SceneRebuildFetch(bus, 0x83a973u, 1u);
    SceneRebuildFetch(bus, 0x83a974u, 2u);
    bus->cycles += 2u + (bus->slot ? 2u : 3u);
    if (bus->slot) {
        SceneRebuildFetch(bus, 0x83a976u, 3u);
        bus->cycles += 4u;
    }
    --bus->slot;
    bus->pending_scan = false;
}

static uint8_t SceneRebuildRead(void *context, uint32_t address) {
    SceneRebuildBus *bus = context;
    const unsigned read = bus->reads++;
    if (bus->phase == LUFIA2_SCENE_REBUILD_FIND_SPRITE) {
        SceneRebuildFinishScan(bus);
        SceneRebuildFetch(bus, 0x83a83bu, 3u);
    } else if (bus->phase == LUFIA2_SCENE_REBUILD_RESET_FLAGS) {
        static const uint32_t pcs[] = {
            0x83a852u, 0x83a854u, 0x83a85cu, 0x83a864u};
        SceneRebuildFetch(bus, pcs[read], read ? 3u : 2u);
    } else if (!read) {
        SceneRebuildFetch(bus, 0x83a8a2u, 2u);
        SceneRebuildFetch(bus, bus->carry_entry ? 0x83a8a4u : 0x83a8a8u, 2u);
    } else {
        SceneRebuildFetch(bus, read == 1u ? 0x83a8b3u : 0x83a8b5u,
                           read == 1u ? 2u : 4u);
    }
    const uint8_t value = bus->registers->read(bus->registers->mem, address);
    if (bus->phase == LUFIA2_SCENE_REBUILD_FIND_SPRITE) {
        bus->sprite = value;
        bus->pending_scan = true;
    } else if (!read) {
        bus->slot = value;
    } else if (bus->phase == LUFIA2_SCENE_REBUILD_PARTY_OFFSETS && read == 2u) {
        bus->hidden = value != 0u;
    }
    return value;
}

static void SceneRebuildWrite(void *context, uint32_t address, uint8_t value) {
    SceneRebuildBus *bus = context;
    unsigned field = bus->writes++;
    if (bus->phase == LUFIA2_SCENE_REBUILD_RESET_FLAGS) {
        SceneRebuildFetch(bus, field ? 0x83a85fu : 0x83a857u, 2u);
        SceneRebuildFetch(bus, field ? 0x83a861u : 0x83a859u, 3u);
    } else if (!bus->carry_entry && !field) {
        SceneRebuildFetch(bus, 0x83a8aau, 2u);
        SceneRebuildFetch(bus, 0x83a8acu, 2u);
        SceneRebuildFetch(bus, 0x83a8aeu, 1u);
        SceneRebuildFetch(bus, 0x83a8afu, 4u);
    } else {
        if (!bus->carry_entry)
            --field;
        if (!field) {
            if (bus->carry_entry)
                SceneRebuildFetch(bus, 0x83a8a6u, 2u);
            else
                SceneRebuildFetch(bus, 0x83a8b9u, 2u);
            SceneRebuildFetch(bus, bus->hidden ? 0x83a8bbu : 0x83a8c9u,
                               bus->hidden ? 2u : 1u);
            SceneRebuildFetch(bus, bus->hidden ? 0x83a8bdu : 0x83a8cau, 4u);
        } else if (field == 1u) {
            if (bus->hidden)
                SceneRebuildFetch(bus, 0x83a8c1u, 2u);
            SceneRebuildFetch(bus, bus->hidden ? 0x83a8c3u : 0x83a8ceu, 4u);
        } else {
            if (field == 2u && bus->hidden)
                SceneRebuildFetch(bus, 0x83a8c7u, 2u);
            SceneRebuildFetch(bus, field == 2u ? 0x83a8d2u : 0x83a8d8u, 2u);
            SceneRebuildFetch(bus, field == 2u ? 0x83a8d4u : 0x83a8dau, 4u);
        }
    }
    bus->registers->write(bus->registers->mem, address, value);
}

static int Lufia2DecompRunSceneRebuildSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        Lufia2SceneRebuildPhase phase;
        uint8_t opcode;
        uint8_t maximum_cycles;
    } operations[] = {
        {0x83a83bu, LUFIA2_SCENE_REBUILD_FIND_SPRITE, 0xbdu, 25u},
        {0x83a852u, LUFIA2_SCENE_REBUILD_RESET_FLAGS, 0xa6u, 34u},
        {0x83a8a2u, LUFIA2_SCENE_REBUILD_PARTY_OFFSETS, 0x90u, 58u}
    };
    unsigned operation = 0u;
    while (operation < sizeof(operations) / sizeof(operations[0]) &&
           operations[operation].pc != pc)
        ++operation;
    if (operation == sizeof(operations) / sizeof(operations[0]) ||
        opcode != operations[operation].opcode ||
        registers->pc != (uint16_t)pc || registers->k != 0x83u ||
        registers->e || !registers->mf || !registers->xf ||
        registers->d || registers->dp || registers->waiting || registers->stopped ||
        registers->nmiWanted || registers->irqWanted ||
        !(registers->db == 0x7eu || registers->db < 0x40u ||
          (registers->db >= 0x80u && registers->db < 0xc0u)))
        return 0;
    const Lufia2SceneRebuildPhase phase = operations[operation].phase;
    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline);
    unsigned slots = 1u;
    if (phase == LUFIA2_SCENE_REBUILD_FIND_SPRITE) {
        if (registers->x >= 40u || budget < 25u)
            return 0;
        slots = registers->x + 1u;
        if (slots > SCENE_REBUILD_MAX_SCAN)
            slots = SCENE_REBUILD_MAX_SCAN;
        if ((uint64_t)slots > budget / 25u)
            slots = (unsigned)(budget / 25u);
    } else if (budget < operations[operation].maximum_cycles) {
        return 0;
    }
    Lufia2CpuState state = BufferSpanLoadState(registers);
    SceneRebuildBus bus = {registers, phase, 0u, 0u, 0u,
        (uint8_t)registers->x, 0u, registers->c != 0u,
        registers->c != 0u, false};
    const Lufia2Memory memory = {SceneRebuildRead, SceneRebuildWrite, &bus, NULL, NULL};
    const Lufia2ExecutionResult result =
        Lufia2ActorRebuildScenePhase(&memory, &state, phase, slots);
    unsigned cycles;
    uint8_t last_cycles;
    if (phase == LUFIA2_SCENE_REBUILD_FIND_SPRITE) {
        SceneRebuildFinishScan(&bus);
        cycles = bus.cycles;
        last_cycles = result.pc == 0x83a83bu ? 4u : 3u;
    } else if (phase == LUFIA2_SCENE_REBUILD_RESET_FLAGS) {
        cycles = 29u + 2u * (0x22u + bus.slot > 0xffu) +
            2u * (0x6au + bus.slot > 0xffu) + (0xd2u + bus.slot > 0xffu);
        last_cycles = (uint8_t)(4u + (0xd2u + bus.slot > 0xffu));
    } else {
        const bool skipped = !bus.carry_entry && bus.slot >= 4u;
        if (skipped) {
            SceneRebuildFetch(&bus, 0x83a8aau, 2u);
            SceneRebuildFetch(&bus, 0x83a8acu, 2u);
        }
        cycles = bus.carry_entry ? 39u : skipped ? 11u : bus.hidden ? 58u : 54u;
        last_cycles = skipped ? 3u : 5u;
    }
    BufferSpanStoreState(registers, &state, result.pc, last_cycles);
    return (int)cycles;
}
