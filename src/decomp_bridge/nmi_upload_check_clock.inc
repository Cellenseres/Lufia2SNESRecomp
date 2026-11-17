#include "lufia2/nmi_upload_checks.h"

static void ActorBridgeNmiUploadCheck(CpuState *cpu, uint32_t entry);

static uint32_t NmiUploadCheckHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeNmiUploadCheck &&
        (pc == 0x808706u || pc == 0x808736u) ? pc + 6u : 0u;
}

static bool NmiUploadCheckHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return NmiUploadCheckHookEnd(pc, hook) != 0u;
}

static void ActorBridgeNmiUploadCheck(CpuState *cpu, uint32_t entry) {
    const uint32_t end = NmiUploadCheckHookEnd(entry, ActorBridgeNmiUploadCheck);
    if (!end || cpu != &g_cpu || cpu->m_flag != 1u || cpu->x_flag ||
        !NmiClockWindow(cpu, false, entry, end, 128u, 128u,
            true, NmiUploadCheckHookOwned))
        return;
    const uint8_t request = cpu->ram[entry == 0x808706u ? 0x72u : 0x73u];
    const FixedSpanOpcode timing[] = {
        {(uint16_t)entry, 2u, 3u, 1u},
        {(uint16_t)(entry + 2u), 2u, 2u, 0u},
        {(uint16_t)(entry + 4u), 2u, (uint8_t)(request & 0xc0u ? 2u : 3u), 0u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = LUFIA2_NMI_UPLOAD_CHECK_OPCODES};
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiCheckUploadRequest(&memory, &state, entry);
    NmiClockBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY &&
        clock.index == LUFIA2_NMI_UPLOAD_CHECK_OPCODES && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
