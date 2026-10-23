#include "lufia2/battle_input_steps.h"

enum { BATTLE_INPUT_CLOCK_BUDGET = 512u };

typedef struct BattleInputClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[20];
    unsigned count, opcodes, progress_mask, stage;
    uint8_t held_high;
    bool resume, enabled, start_pressed;
} BattleInputClock;

static void InputClockAppend(BattleInputClock *clock,
    uint16_t pc, uint8_t fetched, uint8_t cycles, uint8_t data_bytes) {
    assert(clock->count < 20u);
    clock->timing[clock->count++] =
        (FixedSpanOpcode){pc, fetched, cycles, data_bytes};
}

static void InputClockBegin(BattleInputClock *clock, CpuState *cpu) {
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x850000u;
    clock->span.timing = clock->timing;
    clock->span.count = clock->count;
}

static void InputClockStartPress(BattleInputClock *clock) {
    const uint16_t first = clock->resume ? 0xecbdu : 0xec9fu;
    InputClockAppend(clock, first, 2u, 3u, 1u);
    InputClockAppend(clock, (uint16_t)(first + 2u), 2u, 3u, 1u);
    InputClockAppend(clock, (uint16_t)(first + 4u), 2u, 2u, 0u);
}

static void InputClockFilter(BattleInputClock *clock) {
    InputClockAppend(clock, 0xecceu, 2u, 3u, 0u);
    InputClockAppend(clock, 0xecd0u, 2u, 4u, 2u);
    InputClockAppend(clock, 0xecd2u, 2u, 4u, 2u);
    InputClockAppend(clock, 0xecd4u, 2u, 7u, 4u);
    InputClockAppend(clock, 0xecd6u, 2u, 4u, 2u);
    InputClockAppend(clock, 0xecd8u, 2u, 3u, 0u);
}

static void InputClockSavePause(BattleInputClock *clock) {
    InputClockAppend(clock, 0xeca7u, 2u, 5u, 2u);
    InputClockAppend(clock, 0xeca9u, 2u, 3u, 1u);
    InputClockAppend(clock, 0xecabu, 1u, 3u, 1u);
    InputClockAppend(clock, 0xecacu, 3u, 4u, 1u);
    InputClockAppend(clock, 0xecafu, 1u, 3u, 1u);
    InputClockAppend(clock, 0xecb0u, 2u, 2u, 0u);
    InputClockAppend(clock, 0xecb2u, 2u, 5u, 2u);
    InputClockAppend(clock, 0xecb4u, 3u, 4u, 1u);
}

static void InputClockRestorePause(BattleInputClock *clock) {
    InputClockAppend(clock, 0xecc5u, 2u, 5u, 2u);
    InputClockAppend(clock, 0xecc7u, 1u, 4u, 1u);
    InputClockAppend(clock, 0xecc8u, 3u, 4u, 1u);
    InputClockAppend(clock, 0xeccbu, 1u, 4u, 1u);
    InputClockAppend(clock, 0xecccu, 2u, 3u, 1u);
}

static void InputClockPrepare(BattleInputClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count)
        return;
    assert(clock->opcodes + clock->count <= 15u);
    clock->progress_mask |= clock->span.progress_mask << clock->opcodes;
    clock->opcodes += clock->count;
    clock->count = 0u;
    if (!clock->stage) {
        InputClockAppend(clock, 0xec9du, 2u, clock->enabled ? 2u : 3u, 0u);
        if (clock->enabled) {
            InputClockStartPress(clock);
            clock->stage = 1u;
        } else {
            InputClockFilter(clock);
            clock->stage = 2u;
        }
    } else if (clock->stage == 1u) {
        InputClockAppend(clock, clock->resume ? 0xecc3u : 0xeca5u,
            2u, clock->start_pressed ? 2u : 3u, 0u);
        if (clock->start_pressed) {
            if (clock->resume) {
                InputClockRestorePause(clock);
                InputClockFilter(clock);
            } else {
                InputClockSavePause(clock);
            }
        } else if (!clock->resume) {
            InputClockFilter(clock);
        }
        clock->stage = 2u;
    } else {
        clock->stage = 3u;
        return;
    }
    InputClockBegin(clock, clock->span.cpu);
}

static uint8_t InputClockRead(void *context, uint32_t address) {
    BattleInputClock *clock = context;
    InputClockPrepare(clock);
    assert(clock->stage != 3u);
    BeginFixedSpanOpcode(&clock->span);
    const uint16_t pc = clock->span.timing[clock->span.index].pc;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (!clock->stage)
        clock->enabled = value != 0u;
    if (clock->stage == 1u) {
        const uint16_t first = clock->resume ? 0xecbdu : 0xec9fu;
        if (pc == first)
            clock->held_high = value;
        if (pc == first + 2u)
            clock->start_pressed = (clock->held_high & value & 0x10u) != 0u;
    }
    return value;
}

static void InputClockWrite(void *context, uint32_t address, uint8_t value) {
    BattleInputClock *clock = context;
    InputClockPrepare(clock);
    assert(clock->stage != 3u);
    FixedSpanWrite(&clock->span, address, value);
}

static bool InputClockSupported(const CpuState *cpu, uint32_t entry) {
    if ((entry != 0x85ec9au && entry != 0x85ecbdu) || cpu->D ||
        cpu->m_flag != 1u || cpu->x_flag ||
        ((cpu->DB & 0x7fu) >= 0x40u && cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->master_cycles > UINT64_MAX - BATTLE_INPUT_CLOCK_BUDGET ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    uint64_t phase, refresh_clock;
    snes_refresh_state_get(&phase, &refresh_clock);
    if (phase >= 1364u || refresh_clock != cpu->master_cycles ||
        !g_snes || g_snes->vPos >= 262u)
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffbu, true,
        fence - BATTLE_INPUT_CLOCK_BUDGET - 1u, BATTLE_INPUT_CLOCK_BUDGET);
}

static void ActorBridgeBattleInputClock(CpuState *cpu, uint32_t entry) {
    if (!InputClockSupported(cpu, entry))
        return;
    BattleInputClock clock = {0};
    clock.resume = entry == 0x85ecbdu;
    if (clock.resume) {
        clock.stage = 1u;
        InputClockStartPress(&clock);
    } else {
        InputClockAppend(&clock, 0xec9au, 3u, 4u, 1u);
    }
    InputClockBegin(&clock, cpu);
    const Lufia2Memory memory = {InputClockRead, InputClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = clock.resume
        ? Lufia2BattlePauseAfterWait(&memory, &state)
        : Lufia2BattleInputAfterWait(&memory, &state);
    assert(!clock.span.remaining);
    while (clock.stage != 3u) {
        InputClockPrepare(&clock);
        assert(!clock.span.remaining);
    }
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && clock.opcodes <= 15u);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
