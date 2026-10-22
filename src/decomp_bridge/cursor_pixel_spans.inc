#include "lufia2/menu_cursor_step.h"
#include "buffer_span_bus.h"

enum { CURSOR_PIXEL_CYCLE_RESERVE = 64u };

typedef struct CursorPixelBus {
    Interp816 *registers;
    CpuState *clock;
    void (*account_instruction)(CpuState *, unsigned);
    uint32_t error_pc;
    uint32_t move_pc;
    unsigned reads;
    unsigned writes;
    unsigned cycles;
    unsigned pending_cycles;
    bool vertical;
    bool secondary_compare;
    bool row_crosses;
    Lufia2MenuSlidePhase phase;
} CursorPixelBus;

static void CursorPixelFinishInstruction(CursorPixelBus *bus) {
    if (bus->pending_cycles) {
        bus->account_instruction(bus->clock, bus->pending_cycles);
        bus->pending_cycles = 0u;
    }
}

static void CursorPixelFetch(CursorPixelBus *bus, uint32_t pc,
    unsigned bytes, unsigned cycles) {
    CursorPixelFinishInstruction(bus);
    BufferSpanFetch(bus->registers, pc, bytes);
    bus->cycles += cycles;
    bus->pending_cycles = cycles;
}

static void CursorPixelMoveRead(CursorPixelBus *bus, unsigned read) {
    const uint32_t pc = bus->move_pc;
    if (!read) {
        CursorPixelFetch(bus, pc, 3u, 4u + (bus->vertical && bus->row_crosses));
    } else if (read == 1u) {
        CursorPixelFetch(bus, pc + 3u, 1u, 2u);
        CursorPixelFetch(bus, pc + 4u, 2u, 3u);
    } else if (bus->vertical) {
        if (read == 2u) {
            CursorPixelFetch(bus, pc + 9u, 2u, 3u);
        } else if (read == 3u) {
            bus->secondary_compare = true;
            CursorPixelFetch(bus, pc + 11u, 2u, 2u);
            CursorPixelFetch(bus, pc + 13u, 3u, 4u);
        } else {
            CursorPixelFetch(bus, pc + 16u, 2u, 3u);
        }
    } else if (read == 2u) {
        CursorPixelFetch(bus, pc + 11u, 3u, 4u + bus->row_crosses);
    } else if (read == 3u) {
        CursorPixelFetch(bus, pc + 16u, 3u, 4u);
    } else if (read == 4u) {
        CursorPixelFetch(bus, pc + 19u, 2u, 3u);
    } else if (read == 5u) {
        bus->secondary_compare = true;
        CursorPixelFetch(bus, pc + 21u, 2u, 2u);
        CursorPixelFetch(bus, pc + 23u, 3u, 4u + bus->row_crosses);
    } else {
        CursorPixelFetch(bus, pc + 26u, 2u, 3u);
    }
}

static uint8_t CursorPixelRead(void *context, uint32_t address) {
    CursorPixelBus *bus = context;
    const unsigned read = bus->reads++;
    if (bus->phase == LUFIA2_MENU_SLIDE_POSITION) {
        CursorPixelMoveRead(bus, read);
    } else if (!read) {
        CursorPixelFetch(bus, bus->error_pc, 2u, 3u);
    } else {
        CursorPixelFetch(bus, bus->error_pc + 2u, 1u, 2u);
        CursorPixelFetch(bus, bus->error_pc + 3u, 2u, 3u);
    }
    return bus->registers->read(bus->registers->mem, address);
}

static void CursorPixelWrite(void *context, uint32_t address, uint8_t value) {
    CursorPixelBus *bus = context;
    const unsigned write = bus->writes++;
    if (bus->phase == LUFIA2_MENU_SLIDE_ERROR)
        CursorPixelFetch(bus, bus->error_pc + 5u, 2u, 3u);
    else if (!write)
        CursorPixelFetch(bus, bus->move_pc + 6u, 3u, 6u);
    else
        CursorPixelFetch(bus, bus->move_pc + (write == 1u ? 9u : 14u), 2u, 3u);
    bus->registers->write(bus->registers->mem, address, value);
}

static void CursorPixelFinish(CursorPixelBus *bus, uint32_t next) {
    if (bus->phase == LUFIA2_MENU_SLIDE_ERROR) {
        CursorPixelFetch(bus, bus->error_pc + 7u, 2u,
            next == bus->move_pc ? 3u : 2u);
        return;
    }
    const bool complete = next == 0x828b07u;
    const uint32_t pc = bus->move_pc;
    if (bus->secondary_compare) {
        CursorPixelFetch(bus, pc + (bus->vertical ? 18u : 28u),
            2u, complete ? 2u : 3u);
        if (complete)
            CursorPixelFetch(bus, pc + (bus->vertical ? 20u : 30u), 3u, 3u);
    } else {
        CursorPixelFetch(bus, pc + (bus->vertical ? 11u : 21u), 2u, 3u);
    }
}

static int Lufia2DecompRunCursorPixelSpan(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode,
    void (*account_instruction)(CpuState *, unsigned)) {
    const bool vertical = pc == 0x828a75u || pc == 0x828a9bu || pc == 0x828a81u;
    const bool continuation = pc == 0x828a9bu || pc == 0x828ad6u;
    const bool position = pc == 0x828a81u || pc == 0x828ab2u;
    if (!account_instruction ||
        (!vertical && pc != 0x828aa6u && pc != 0x828ad6u && pc != 0x828ab2u) ||
        opcode != (continuation ? 0x80u : position ? 0xb9u : 0xa5u) ||
        registers->k != 0x82u || registers->pc != (uint16_t)pc ||
        !BufferSpanReady(registers) || registers->y >= 48u ||
        registers->sp < 0x1f12u || registers->sp > 0x1ffcu ||
        !(registers->db < 0x40u ||
            (registers->db >= 0x80u && registers->db < 0xc0u)) ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < CURSOR_PIXEL_CYCLE_RESERVE)
        return 0;
    CursorPixelBus bus = {0};
    bus.registers = registers;
    bus.clock = cpu;
    bus.account_instruction = account_instruction;
    bus.vertical = vertical;
    bus.phase = position ? LUFIA2_MENU_SLIDE_POSITION : LUFIA2_MENU_SLIDE_ERROR;
    bus.row_crosses = registers->y >= 24u;
    bus.error_pc = vertical ? 0x828a75u : 0x828aa6u;
    bus.move_pc = bus.error_pc + 12u;
    if (continuation)
        CursorPixelFetch(&bus, pc, 2u, 3u);
    Lufia2CpuState state = BufferSpanLoadState(registers);
    const Lufia2Memory memory = {CursorPixelRead, CursorPixelWrite, &bus, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2MenuAdvanceCursorSlide(
        &memory, &state, vertical ? LUFIA2_MENU_SLIDE_VERTICAL
                                : LUFIA2_MENU_SLIDE_HORIZONTAL, bus.phase);
    CursorPixelFinish(&bus, result.pc);
    const uint8_t last_cycles = (uint8_t)bus.pending_cycles;
    CursorPixelFinishInstruction(&bus);
    BufferSpanStoreState(registers, &state, result.pc, last_cycles);
    return (int)bus.cycles;
}
