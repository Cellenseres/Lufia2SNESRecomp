#ifndef LUFIA2_DECOMP_BUFFER_SPAN_BUS_H
#define LUFIA2_DECOMP_BUFFER_SPAN_BUS_H

#include "interp816.h"
#include "cpu_state.h"
#include "lufia2/execution.h"

extern uint32_t g_interp816_cur_pc;

enum { BUFFER_SPAN_MAX_BYTES = 14u };

typedef struct BufferSpanBus {
    Interp816 *registers;
    uint32_t pc;
    unsigned load_length;
    unsigned store_length;
    unsigned counter_length;
    bool pending_tail;
} BufferSpanBus;

static bool BufferSpanReady(const Interp816 *registers) {
    return !registers->e && registers->mf && !registers->xf &&
        !registers->d && !registers->dp && !registers->waiting &&
        !registers->stopped && !registers->nmiWanted && !registers->irqWanted;
}

static uint64_t BufferSpanCycleBudget(const CpuState *cpu, uint64_t deadline) {
    if (!deadline)
        return UINT64_MAX;
    if (cpu->master_cycles >= deadline)
        return 0u;
    return (deadline - cpu->master_cycles) / (g_memsel ? 6u : 8u);
}

static void BufferSpanFetch(Interp816 *registers, uint32_t pc, unsigned length) {
    g_interp816_cur_pc = pc;
    for (unsigned byte = 0; byte < length; ++byte)
        (void)registers->read(registers->mem, pc + byte);
}

static void BufferSpanFinishIteration(BufferSpanBus *bus) {
    if (!bus->pending_tail)
        return;
    const uint32_t advance = bus->pc + bus->load_length + bus->store_length;
    BufferSpanFetch(bus->registers, advance, 1u);
    BufferSpanFetch(bus->registers, advance + 1u, bus->counter_length);
    BufferSpanFetch(bus->registers, advance + 1u + bus->counter_length, 2u);
    bus->pending_tail = false;
}

static uint8_t BufferSpanRead(void *context, uint32_t address) {
    BufferSpanBus *bus = context;
    BufferSpanFinishIteration(bus);
    BufferSpanFetch(bus->registers, bus->pc, bus->load_length);
    return bus->registers->read(bus->registers->mem, address);
}

static void BufferSpanWrite(void *context, uint32_t address, uint8_t value) {
    BufferSpanBus *bus = context;
    BufferSpanFetch(bus->registers, bus->pc + bus->load_length, bus->store_length);
    bus->registers->write(bus->registers->mem, address, value);
    bus->pending_tail = true;
}

static Lufia2CpuState BufferSpanLoadState(const Interp816 *registers) {
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
    return state;
}

static void BufferSpanStoreState(Interp816 *registers,
                                 const Lufia2CpuState *state,
                                 uint32_t next, uint8_t last_cycles) {
    registers->a = state->accumulator;
    registers->x = state->x;
    registers->y = state->y;
    registers->sp = state->stack;
    registers->dp = state->direct_page;
    registers->db = state->data_bank;
    registers->pc = (uint16_t)next;
    registers->k = state->program_bank;
    registers->c = state->carry;
    registers->z = state->zero;
    registers->n = state->negative;
    registers->v = state->overflow;
    registers->i = state->irq_disable;
    registers->d = state->decimal;
    registers->mf = state->accumulator_is_8_bit;
    registers->xf = state->index_is_8_bit;
    registers->cyclesUsed = last_cycles;
}

#endif
