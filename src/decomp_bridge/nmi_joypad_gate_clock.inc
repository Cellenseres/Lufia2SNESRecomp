#include "lufia2/nmi_joypad_gate.h"

static void ActorBridgeNmiJoypadGate(CpuState *cpu, uint32_t entry);

typedef struct NmiJoypadGateClock {
    FixedSpanClock bus;
    FixedSpanOpcode timing[LUFIA2_NMI_JOYPAD_GATE_OPCODES];
} NmiJoypadGateClock;

static uint32_t NmiJoypadGateHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x808765u && hook == ActorBridgeNmiJoypadGate ? 0x80876bu : 0u;
}

static bool NmiJoypadGateHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return NmiJoypadGateHookEnd(pc, hook) != 0u;
}

static uint8_t NmiJoypadGateRead(void *context, uint32_t address) {
    NmiJoypadGateClock *clock = context;
    const uint8_t status = NmiClockRead(&clock->bus, address);
    clock->timing[2].cycles = (uint8_t)(2u + (status & 1u));
    return status;
}

static void ActorBridgeNmiJoypadGate(CpuState *cpu, uint32_t entry) {
    if (!NmiJoypadGateHookEnd(entry, ActorBridgeNmiJoypadGate) ||
        cpu != &g_cpu || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        !NmiClockWindow(cpu, cpu->x_flag != 0u, entry, 0x80876bu, 128u, 128u,
            true, NmiJoypadGateHookOwned))
        return;
    const uint64_t start = cpu->master_cycles;
    NmiJoypadGateClock clock = {0};
    clock.timing[0] = (FixedSpanOpcode){0x8765u, 3u, 4u, 1u};
    clock.timing[1] = (FixedSpanOpcode){0x8768u, 1u, 2u, 0u};
    clock.timing[2] = (FixedSpanOpcode){0x8769u, 2u, 2u, 0u};
    clock.bus = (FixedSpanClock){.cpu = cpu, .bank = 0x800000u,
        .timing = clock.timing, .count = LUFIA2_NMI_JOYPAD_GATE_OPCODES};
    const Lufia2Memory memory = {NmiJoypadGateRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiPollJoypadReady(&memory, &state);
    NmiClockBeginOpcode(&clock.bus);
    assert(clock.bus.index == clock.bus.count && !clock.bus.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        (result.pc == entry || result.pc == 0x80876bu));
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= 128u);
    Lufia2DecompFrameReturnComplete(clock.bus.count, clock.bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
