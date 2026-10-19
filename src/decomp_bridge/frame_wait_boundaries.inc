#include "lufia2/frame_wait.h"
#include "lufia2/system.h"

extern uint32_t g_interp816_cur_pc;

typedef struct FrameWaitBoundaryBus {
    Interp816 *registers;
    uint32_t word_low;
    uint32_t word_high;
    uint8_t high_byte;
    bool high_pending;
} FrameWaitBoundaryBus;

static uint8_t FrameWaitBoundaryRead(void *context, uint32_t address) {
    FrameWaitBoundaryBus *bus = context;
    Interp816 *registers = bus->registers;
    if (bus->high_pending && address == bus->word_high) {
        bus->high_pending = false;
        return bus->high_byte;
    }
    if (address == bus->word_low && registers->read_word) {
        uint16_t value;
        if (registers->read_word(registers->mem, address, bus->word_high, &value)) {
            bus->high_byte = (uint8_t)(value >> 8);
            bus->high_pending = true;
            return (uint8_t)value;
        }
    }
    return registers->read(registers->mem, address);
}

static int Lufia2DecompRunFrameWaitBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    static const uint32_t counters[] = {
        0x8293c6u, 0x83900cu, 0x8eb550u, 0x868b4cu, 0x869750u,
        0x80bf0bu, 0x848d4du, 0x80937bu, 0x848363u, 0x84a5acu,
        0x85851cu, 0x85ec94u, 0x85ecb7u, 0x8eb288u, 0x8ec2c4u};
    static const uint32_t returns[] = {
        0x8293ceu, 0x839012u, 0x8eb556u, 0x868b54u, 0x869757u, 0x80bf11u,
        0x848d53u, 0x809382u, 0x848369u, 0x84a5b4u, 0x858522u, 0x8eb28eu};
    if (registers->e || registers->k != pc >> 16 ||
        registers->pc != (uint16_t)pc || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted)
        return 0;
    bool returning = false;
    uint8_t expected_opcode = 0;
    for (unsigned i = 0; i < sizeof counters / sizeof counters[0]; ++i) {
        if (pc == counters[i]) expected_opcode = 0xa5u;
        if (pc == counters[i] + 2u) expected_opcode = 0xc5u;
        if (pc == counters[i] + 4u) expected_opcode = 0xf0u;
    }
    for (unsigned i = 0; i < sizeof returns / sizeof returns[0]; ++i)
        if (pc == returns[i]) {
            expected_opcode = 0x60u;
            returning = true;
            break;
        }
    if (!expected_opcode || opcode != expected_opcode)
        return 0;
    Lufia2CpuState state = {0};
    state.accumulator = registers->a;
    state.stack = registers->sp;
    state.direct_page = registers->dp;
    state.program_bank = registers->k;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    state.carry = registers->c;
    state.zero = registers->z;
    state.negative = registers->n;
    FrameWaitBoundaryBus bus = {registers, UINT32_MAX, UINT32_MAX, 0u, false};
    if (!returning && opcode != 0xf0u && !registers->mf) {
        bus.word_low = (uint16_t)(registers->dp + 0x40u);
        bus.word_high = (uint16_t)(bus.word_low + 1u);
    }
    const Lufia2Memory memory = {FrameWaitBoundaryRead, NULL, &bus, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    if (!returning)
        (void)registers->read(registers->mem, pc + 1u);
    unsigned cycles;
    Lufia2ExecutionResult result;
    if (returning) {
        cycles = 6u;
        result = Lufia2FrameWaitReturnStep(&memory, &state, pc);
    } else {
        cycles = opcode == 0xf0u ? 2u + registers->z
            : 3u + !registers->mf + ((registers->dp & 0xffu) != 0u);
        result = Lufia2FrameWaitAdditionalStep(&memory, &state, pc);
    }
    registers->a = state.accumulator;
    registers->sp = state.stack;
    registers->k = state.program_bank;
    registers->pc = (uint16_t)result.pc;
    registers->c = state.carry;
    registers->z = state.zero;
    registers->n = state.negative;
    registers->cyclesUsed = (uint8_t)cycles;
    return (int)cycles;
}
