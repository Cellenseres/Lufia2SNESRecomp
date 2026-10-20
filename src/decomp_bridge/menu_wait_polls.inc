#include "lufia2/menu_wait.h"
#include "buffer_span_bus.h"

enum { MENU_WAIT_POLL_REFRESH_RESERVE = 40u };

typedef struct MenuWaitPollBus {
    Interp816 *registers;
    Lufia2MenuWaitPoll poll;
    uint32_t pc;
    unsigned reads;
    uint8_t high_byte;
    bool high_pending;
} MenuWaitPollBus;

static bool MenuWaitPollTestsButtons(Lufia2MenuWaitPoll poll) {
    return poll == LUFIA2_MENU_WAIT_FRAME_BUTTONS ||
        poll == LUFIA2_MENU_WAIT_MOTION_BUTTONS;
}

static void MenuWaitPollFetch(MenuWaitPollBus *bus, uint32_t pc, unsigned bytes) {
    BufferSpanFetch(bus->registers, pc, bytes);
}

static uint8_t MenuWaitPollRead(void *context, uint32_t address) {
    MenuWaitPollBus *bus = context;
    Interp816 *registers = bus->registers;
    const unsigned read = bus->reads++;
    if (bus->poll == LUFIA2_MENU_WAIT_BUTTON_MASKS) {
        if (!(read & 1u)) {
            if (!read) {
                MenuWaitPollFetch(bus, bus->pc, 2u);
                MenuWaitPollFetch(bus, bus->pc + 2u, 3u);
            } else if (read == 2u) {
                MenuWaitPollFetch(bus, bus->pc + 5u, 3u);
            } else {
                MenuWaitPollFetch(bus, bus->pc + 8u, 2u);
                MenuWaitPollFetch(bus, bus->pc + 10u, 3u);
            }
            if (registers->read_word) {
                uint16_t value;
                if (registers->read_word(registers->mem, address, address + 1u,
                                         &value)) {
                    bus->high_byte = (uint8_t)(value >> 8);
                    bus->high_pending = true;
                    return (uint8_t)value;
                }
            }
        } else if (bus->high_pending) {
            bus->high_pending = false;
            return bus->high_byte;
        }
    } else if (MenuWaitPollTestsButtons(bus->poll)) {
        if (read) {
            MenuWaitPollFetch(bus, bus->pc + 3u, 2u);
            MenuWaitPollFetch(bus, bus->pc + 5u, 2u);
        }
        MenuWaitPollFetch(bus, bus->pc + read * 7u, 3u);
    } else {
        const bool motion = bus->poll == LUFIA2_MENU_WAIT_SPRITE_MOTION;
        MenuWaitPollFetch(bus, bus->pc, motion ? 3u : 1u);
    }
    return registers->read(registers->mem, address);
}

static void MenuWaitPollFinish(MenuWaitPollBus *bus, uint32_t next) {
    if (MenuWaitPollTestsButtons(bus->poll)) {
        const uint32_t last = bus->pc + (bus->reads - 1u) * 7u;
        MenuWaitPollFetch(bus, last + 3u, 2u);
        MenuWaitPollFetch(bus, last + 5u, 2u);
    } else if (bus->poll == LUFIA2_MENU_WAIT_BUTTON_MASKS) {
        MenuWaitPollFetch(bus, bus->reads == 4u ? 0x829369u : 0x82936eu, 2u);
        if (next == 0x829372u) {
            MenuWaitPollFetch(bus, 0x829370u, 2u);
        } else {
            MenuWaitPollFetch(bus, next - 3u, 2u);
            MenuWaitPollFetch(bus, next - 1u, 1u);
        }
    } else if (bus->poll == LUFIA2_MENU_WAIT_SPRITE_MOTION) {
        MenuWaitPollFetch(bus, bus->pc + 3u, 2u);
    } else {
        MenuWaitPollFetch(bus, bus->pc + 1u, 1u);
        MenuWaitPollFetch(bus, bus->pc + 2u, 2u);
    }
}

static int Lufia2DecompRunMenuWaitPoll(
    Interp816 *registers, CpuState *cpu, uint32_t pc, uint8_t opcode) {
    static const struct {
        uint32_t pc;
        Lufia2MenuWaitPoll poll;
        uint8_t opcode;
        uint8_t maximum_cycles;
    } operations[] = {
        {0x82933au, LUFIA2_MENU_WAIT_FRAME_BUTTONS, 0xadu, 17u},
        {0x829384u, LUFIA2_MENU_WAIT_MOTION_BUTTONS, 0xadu, 17u},
        {0x829361u, LUFIA2_MENU_WAIT_BUTTON_MASKS, 0xc2u, 28u},
        {0x82934cu, LUFIA2_MENU_WAIT_FRAME_COUNT, 0x68u, 9u},
        {0x829358u, LUFIA2_MENU_WAIT_SPRITE_COUNT, 0x68u, 9u},
        {0x829396u, LUFIA2_MENU_WAIT_SPRITE_MOTION, 0xadu, 7u}
    };
    unsigned operation = 0u;
    while (operation < sizeof(operations) / sizeof(operations[0]) &&
           operations[operation].pc != pc)
        ++operation;
    if (operation == sizeof(operations) / sizeof(operations[0]) ||
        opcode != operations[operation].opcode ||
        registers->pc != (uint16_t)pc || registers->k != 0x82u ||
        !BufferSpanReady(registers) || registers->sp < 0x1f20u ||
        registers->sp > 0x1ffbu ||
        !(registers->db == 0x7eu || registers->db < 0x40u ||
          (registers->db >= 0x80u && registers->db < 0xc0u)))
        return 0;
    if (s_lle_master_deadline &&
        (cpu->master_cycles >= s_lle_master_deadline ||
         s_lle_master_deadline - cpu->master_cycles <
             (uint64_t)operations[operation].maximum_cycles * 8u +
                 MENU_WAIT_POLL_REFRESH_RESERVE))
        return 0;
    Lufia2CpuState state = BufferSpanLoadState(registers);
    const Lufia2MenuWaitPoll poll = operations[operation].poll;
    MenuWaitPollBus bus = {registers, poll, pc, 0u, 0u, false};
    const Lufia2Memory memory = {MenuWaitPollRead, NULL, &bus, NULL, NULL};
    const Lufia2ExecutionResult result = Lufia2MenuPollInputWait(&memory, &state, poll);
    MenuWaitPollFinish(&bus, result.pc);
    unsigned cycles;
    uint8_t last_cycles;
    if (MenuWaitPollTestsButtons(poll)) {
        last_cycles = state.zero ? 2u : 3u;
        cycles = bus.reads * 8u + !state.zero;
    } else if (poll == LUFIA2_MENU_WAIT_BUTTON_MASKS) {
        const bool none = result.pc == 0x829372u;
        last_cycles = none ? 3u : 2u;
        cycles = none ? 25u : bus.reads == 4u ? 21u : 28u;
    } else {
        last_cycles = state.zero ? 2u : 3u;
        cycles = (poll == LUFIA2_MENU_WAIT_SPRITE_MOTION ? 6u : 8u) + !state.zero;
    }
    BufferSpanStoreState(registers, &state, result.pc, last_cycles);
    return (int)cycles;
}
