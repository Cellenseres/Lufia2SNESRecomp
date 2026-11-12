#include "lufia2/nmi_finish.h"

enum { NMI_RESTORE_CLOCK_BUDGET = 592u, NMI_RESTORE_NO_RESET_BUDGET = 504u };

typedef struct NmiRestoreClock {
    FixedSpanClock span;
    uint8_t interrupt_high;
    bool interrupt_high_pending;
} NmiRestoreClock;

static bool NmiRestoreHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x80869bu && hook == ActorBridgeNmiResume;
}

static bool NmiRestoreSupported(const CpuState *cpu) {
    if (!g_snes || !cpu->ram || cpu->ram != g_snes->ram || cpu->m_flag ||
        (cpu->ram[0x520u] && cpu->ram[0x46u] == 0x30u && cpu->ram[0x47u] == 0x30u))
        return false;
    const unsigned budget = cpu->ram[0x520u]
        ? NMI_RESTORE_CLOCK_BUDGET : NMI_RESTORE_NO_RESET_BUDGET;
    return NmiClockWindow(cpu, false, 0x80869bu, 0x8086bfu, budget,
        NMI_RESTORE_CLOCK_BUDGET, NmiRestoreHookOwned);
}

static uint8_t NmiRestoreRead(void *context, uint32_t address) {
    NmiRestoreClock *clock = context;
    if (clock->interrupt_high_pending) {
        assert(address == 0x804211u);
        clock->interrupt_high_pending = false;
        return clock->interrupt_high;
    }
    BeginFixedSpanOpcode(&clock->span);
    if (address != 0x804210u || clock->span.remaining != 2u)
        return FixedSpanRead(&clock->span, address);
    assert(clock->span.remaining == 2u);
    bool progress = false;
    const uint16_t value = Lufia2DecompFrameReturnReadWord(
        clock->span.cpu, address, address + 1u, &progress);
    clock->span.data_master += cpu_region_speed(address) + cpu_region_speed(address + 1u);
    clock->span.progress |= progress;
    clock->span.remaining = 0u;
    clock->interrupt_high = (uint8_t)(value >> 8);
    clock->interrupt_high_pending = true;
    FinishFixedSpanOpcode(&clock->span);
    return (uint8_t)value;
}

static void NmiRestoreWrite(void *context, uint32_t address, uint8_t value) {
    NmiRestoreClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
}

static bool RunNativeNmiRestore(CpuState *cpu) {
    if (!NmiRestoreSupported(cpu))
        return false;
    static const FixedSpanOpcode timing[] = {
        {0x869bu, 2u, 7u, 4u}, {0x869du, 2u, 7u, 4u},
        {0x869fu, 4u, 6u, 2u}, {0x86a3u, 3u, 3u, 0u},
        {0x86a6u, 2u, 3u, 0u}, {0x86b5u, 3u, 5u, 2u},
        {0x86b8u, 2u, 3u, 0u}, {0x86bau, 1u, 5u, 2u},
        {0x86bbu, 1u, 5u, 2u}, {0x86bcu, 1u, 5u, 2u},
        {0x86bdu, 1u, 4u, 1u}, {0x86beu, 1u, 4u, 1u}
    };
    static const FixedSpanOpcode reset_enabled[] = {
        {0x869bu, 2u, 7u, 4u}, {0x869du, 2u, 7u, 4u},
        {0x869fu, 4u, 6u, 2u}, {0x86a3u, 3u, 3u, 0u},
        {0x86a6u, 2u, 2u, 0u}, {0x86a8u, 2u, 4u, 2u},
        {0x86aau, 3u, 3u, 0u}, {0x86adu, 2u, 3u, 0u},
        {0x86afu, 2u, 3u, 0u}, {0x86b5u, 3u, 4u, 1u},
        {0x86b8u, 2u, 3u, 0u}, {0x86bau, 1u, 5u, 2u},
        {0x86bbu, 1u, 5u, 2u}, {0x86bcu, 1u, 5u, 2u},
        {0x86bdu, 1u, 4u, 1u}, {0x86beu, 1u, 4u, 1u}
    };
    NmiRestoreClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x800000u;
    const bool reset_allowed = cpu->ram[0x520u] != 0u;
    clock.span.timing = reset_allowed ? reset_enabled : timing;
    clock.span.count = reset_allowed ? sizeof reset_enabled / sizeof reset_enabled[0]
        : sizeof timing / sizeof timing[0];
    const Lufia2Memory memory = {NmiRestoreRead, NmiRestoreWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiRestoreInterruptFrame(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x8086bfu);
    assert(clock.span.index == clock.span.count && !clock.span.remaining &&
        !clock.interrupt_high_pending);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
    return true;
}
