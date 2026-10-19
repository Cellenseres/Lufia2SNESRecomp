#include "lufia2/menu_buffers.h"

extern uint32_t g_interp816_cur_pc;

enum { MENU_BUFFER_MAX_SPAN = 14u };

static void MenuBufferFetch(Interp816 *registers, uint32_t pc, unsigned length) {
    g_interp816_cur_pc = pc;
    for (unsigned byte = 0; byte < length; ++byte)
        (void)registers->read(registers->mem, pc + byte);
}

typedef struct MenuBufferBus {
    Interp816 *registers;
    uint32_t pc;
    unsigned store_length;
    bool pending_tail;
} MenuBufferBus;

static void MenuBufferFinishIteration(MenuBufferBus *bus) {
    if (!bus->pending_tail)
        return;
    const uint32_t advance = bus->pc + 4u + bus->store_length;
    MenuBufferFetch(bus->registers, advance, 1u);
    MenuBufferFetch(bus->registers, advance + 1u, 3u);
    MenuBufferFetch(bus->registers, advance + 4u, 2u);
    bus->pending_tail = false;
}

static uint8_t MenuBufferRead(void *context, uint32_t address) {
    MenuBufferBus *bus = context;
    MenuBufferFinishIteration(bus);
    MenuBufferFetch(bus->registers, bus->pc, 4u);
    return bus->registers->read(bus->registers->mem, address);
}

static void MenuBufferWrite(void *context, uint32_t address, uint8_t value) {
    MenuBufferBus *bus = context;
    MenuBufferFetch(bus->registers, bus->pc + 4u, bus->store_length);
    bus->registers->write(bus->registers->mem, address, value);
    bus->pending_tail = true;
}

static int Lufia2DecompRunMenuBufferSpan(Interp816 *registers, CpuState *cpu,
                         uint32_t pc, uint8_t opcode) {
    const bool message = pc == 0x82f695u;
    if ((pc != 0x82f685u && !message) || opcode != 0xbfu ||
        registers->k != 0x82u || registers->e || !registers->mf ||
        registers->xf || registers->d || registers->dp ||
        !(registers->db == 0x7eu || registers->db < 0x40u ||
          (registers->db >= 0x80u && registers->db < 0xc0u)))
        return 0;
    const unsigned limit = message ? 347u : 4312u;
    if (registers->x >= limit)
        return 0;
    const unsigned master_per_cycle = g_memsel ? 6u : 8u;
    uint64_t budget = UINT64_MAX;
    if (s_lle_master_deadline) {
        if (cpu->master_cycles >= s_lle_master_deadline)
            return 0;
        budget = (s_lle_master_deadline - cpu->master_cycles) / master_per_cycle;
    }
    unsigned count = limit - registers->x;
    if (count > MENU_BUFFER_MAX_SPAN)
        count = MENU_BUFFER_MAX_SPAN;
    const unsigned iteration_cycles = message ? 18u : 19u;
    while (count && iteration_cycles * count - (registers->x + count == limit) > budget)
        --count;
    if (!count)
        return 0;
    Lufia2CpuState state = {0};
    state.accumulator = registers->a;
    state.x = registers->x;
    state.y = registers->y;
    state.stack = registers->sp;
    state.direct_page = registers->dp;
    state.data_bank = registers->db;
    state.program_bank = registers->k;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    state.carry = registers->c;
    state.zero = registers->z;
    state.negative = registers->n;
    state.overflow = registers->v;
    state.decimal = registers->d;
    state.irq_disable = registers->i;
    MenuBufferBus bus = {registers, pc, message ? 4u : 3u, false};
    const Lufia2Memory memory = {MenuBufferRead, MenuBufferWrite, &bus, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2MenuRestoreFieldBufferSpan(
        &memory, &state, message ? LUFIA2_MENU_MESSAGE_SNAPSHOT
                                 : LUFIA2_MENU_FIELD_SNAPSHOT, count);
    MenuBufferFinishIteration(&bus);
    registers->a = state.accumulator;
    registers->x = state.x;
    registers->y = state.y;
    registers->sp = state.stack;
    registers->pc = (uint16_t)result.pc;
    registers->k = state.program_bank;
    registers->c = state.carry;
    registers->z = state.zero;
    registers->n = state.negative;
    registers->v = state.overflow;
    registers->i = state.irq_disable;
    registers->d = state.decimal;
    registers->mf = state.accumulator_is_8_bit;
    registers->xf = state.index_is_8_bit;
    registers->cyclesUsed = state.zero ? 2u : 3u;
    const unsigned cycles = iteration_cycles * count - state.zero;
    return (int)cycles;
}
