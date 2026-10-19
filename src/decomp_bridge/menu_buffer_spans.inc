#include "lufia2/menu_buffers.h"
#include "buffer_span_bus.h"

static int Lufia2DecompRunMenuBufferSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    const bool inventory = pc == 0x82f695u;
    if ((pc != 0x82f685u && !inventory) || opcode != 0xbfu ||
        registers->k != 0x82u || !BufferSpanReady(registers) ||
        !(registers->db == 0x7eu || registers->db < 0x40u ||
          (registers->db >= 0x80u && registers->db < 0xc0u)))
        return 0;
    const unsigned limit = inventory ? 347u : 4312u;
    if (registers->x >= limit)
        return 0;
    const uint64_t budget = BufferSpanCycleBudget(cpu, s_lle_master_deadline);
    unsigned count = limit - registers->x;
    if (count > BUFFER_SPAN_MAX_BYTES)
        count = BUFFER_SPAN_MAX_BYTES;
    const unsigned iteration_cycles = inventory ? 18u : 19u;
    while (count && iteration_cycles * count - (registers->x + count == limit) > budget)
        --count;
    if (!count)
        return 0;
    Lufia2CpuState state = BufferSpanLoadState(registers);
    BufferSpanBus bus = {registers, pc, 4u, inventory ? 4u : 3u, 3u, false};
    const Lufia2Memory memory = {BufferSpanRead, BufferSpanWrite, &bus, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2MenuRestoreFieldBufferSpan(
        &memory, &state, inventory ? LUFIA2_MENU_INVENTORY_SNAPSHOT
                                 : LUFIA2_MENU_FIELD_SNAPSHOT, count);
    BufferSpanFinishIteration(&bus);
    BufferSpanStoreState(registers, &state, result.pc, state.zero ? 2u : 3u);
    const unsigned cycles = iteration_cycles * count - state.zero;
    return (int)cycles;
}
