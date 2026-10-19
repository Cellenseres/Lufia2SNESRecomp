#include "lufia2/save_buffers.h"
#include "buffer_span_bus.h"

static int Lufia2DecompRunSaveRecordTailSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    static const struct SaveRecordTailOperation {
        uint32_t pc;
        Lufia2SaveRecordTail tail;
        Lufia2SaveBufferDirection direction;
    } operations[] = {
        {0x85c6cbu, LUFIA2_SAVE_CAPSULE_STATS, LUFIA2_SAVE_FROM_RECORD},
        {0x85c6d9u, LUFIA2_SAVE_BATTLE_SUBMENU_CHOICES, LUFIA2_SAVE_FROM_RECORD},
        {0x85c9eeu, LUFIA2_SAVE_CAPSULE_STATS, LUFIA2_SAVE_TO_RECORD},
        {0x85c9fcu, LUFIA2_SAVE_BATTLE_SUBMENU_CHOICES, LUFIA2_SAVE_TO_RECORD}
    };
    const struct SaveRecordTailOperation *operation = NULL;
    for (unsigned i = 0; i < sizeof operations / sizeof operations[0]; ++i)
        if (operations[i].pc == pc) {
            operation = &operations[i];
            break;
        }
    if (!operation || !BufferSpanReady(registers) || registers->k != 0x85u ||
        registers->db != 0x7eu || registers->x >
            (operation->tail == LUFIA2_SAVE_CAPSULE_STATS ? 0x3eu : 0x1bu))
        return 0;
    const bool restoring = operation->direction == LUFIA2_SAVE_FROM_RECORD;
    if (opcode != (restoring ? 0xb9u : 0xbfu))
        return 0;
    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline);
    unsigned available = registers->x + 1u;
    if (available > BUFFER_SPAN_MAX_BYTES)
        available = BUFFER_SPAN_MAX_BYTES;
    unsigned count = 0u;
    unsigned cycles = 0u;
    while (count < available) {
        const uint16_t y = (uint16_t)(registers->y + count);
        const unsigned iteration = (restoring ? 16u + (y >= 0x100u) : 18u)
            - (registers->x == count);
        if ((uint64_t)cycles + iteration > budget)
            break;
        cycles += iteration;
        ++count;
    }
    if (!count)
        return 0;
    Lufia2CpuState state = BufferSpanLoadState(registers);
    BufferSpanBus bus = {registers, pc, restoring ? 3u : 4u,
        restoring ? 4u : 3u, 1u, false};
    const Lufia2Memory memory = {BufferSpanRead, BufferSpanWrite, &bus, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2SaveCopyRecordTailSpan(
        &memory, &state, operation->tail, operation->direction, count);
    BufferSpanFinishIteration(&bus);
    BufferSpanStoreState(registers, &state, result.pc, state.negative ? 2u : 3u);
    return (int)cycles;
}

static int Lufia2DecompRunSaveBufferSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    static const struct SaveBufferOperation {
        uint32_t pc;
        Lufia2SaveBuffer buffer;
        Lufia2SaveBufferDirection direction;
        uint8_t opcode;
        unsigned load_length;
        unsigned store_length;
    } operations[] = {
        {0x85c669u, LUFIA2_SAVE_FIELD_STATE, LUFIA2_SAVE_FROM_RECORD, 0xbdu, 3u, 3u},
        {0x85c67bu, LUFIA2_SAVE_PACKED_ITEM_COUNTS, LUFIA2_SAVE_FROM_RECORD, 0xbfu, 4u, 3u},
        {0x85c986u, LUFIA2_SAVE_FIELD_STATE, LUFIA2_SAVE_TO_RECORD, 0xb9u, 3u, 3u},
        {0x85c998u, LUFIA2_SAVE_PACKED_ITEM_COUNTS, LUFIA2_SAVE_TO_RECORD, 0xb9u, 3u, 4u}
    };
    const struct SaveBufferOperation *operation = NULL;
    for (unsigned i = 0; i < sizeof operations / sizeof operations[0]; ++i)
        if (operations[i].pc == pc) {
            operation = &operations[i];
            break;
        }
    if (!operation)
        return Lufia2DecompRunSaveRecordTailSpan(registers, cpu, pc, opcode);
    if (opcode != operation->opcode || !BufferSpanReady(registers) ||
        registers->k != 0x85u)
        return 0;
    const bool counts = operation->buffer == LUFIA2_SAVE_PACKED_ITEM_COUNTS;
    const bool restoring = operation->direction == LUFIA2_SAVE_FROM_RECORD;
    if (registers->db != (counts ? 0x7fu : 0x7eu) ||
        registers->y > (counts ? 0xffu : 0xe9u))
        return 0;
    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline);
    unsigned available = registers->y + 1u;
    if (available > BUFFER_SPAN_MAX_BYTES)
        available = BUFFER_SPAN_MAX_BYTES;
    unsigned count = 0u;
    unsigned cycles = 0u;
    while (count < available) {
        const uint16_t x = (uint16_t)(registers->x + count);
        const uint16_t y = (uint16_t)(registers->y - count);
        unsigned load_cycles;
        if (restoring)
            load_cycles = counts ? 5u : 4u + (x >= 0x100u);
        else
            load_cycles = 4u + ((counts ? 0x80u : 0x8au) + y > 0xffu);
        const unsigned store_cycles = counts && !restoring ? 5u : 6u;
        const unsigned iteration = load_cycles + store_cycles + 4u + (y ? 3u : 2u);
        if ((uint64_t)cycles + iteration > budget)
            break;
        cycles += iteration;
        ++count;
    }
    if (!count)
        return 0;
    Lufia2CpuState state = BufferSpanLoadState(registers);
    BufferSpanBus bus = {registers, pc, operation->load_length,
        operation->store_length, 1u, false};
    const Lufia2Memory memory = {BufferSpanRead, BufferSpanWrite, &bus, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2SaveCopyBufferSpan(
        &memory, &state, operation->buffer, operation->direction, count);
    BufferSpanFinishIteration(&bus);
    BufferSpanStoreState(registers, &state, result.pc, state.negative ? 2u : 3u);
    return (int)cycles;
}
