#include "lufia2/nmi_dma_setup.h"

static void ActorBridgeNmiDmaSetup(CpuState *cpu, uint32_t entry);

static uint32_t NmiDmaSetupHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeNmiDmaSetup)
        return 0u;
    return pc == 0x80870cu ? 0x808731u : pc == 0x80873cu ? 0x808760u : 0u;
}

static bool NmiDmaSetupHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return NmiDmaSetupHookEnd(pc, hook) != 0u;
}

static void ActorBridgeNmiDmaSetup(CpuState *cpu, uint32_t entry) {
    const uint32_t end = NmiDmaSetupHookEnd(entry, ActorBridgeNmiDmaSetup);
    if (!end || cpu != &g_cpu || cpu->m_flag != 1u || cpu->x_flag ||
        !NmiClockWindow(cpu, false, entry, end, 512u, 512u, true,
            NmiDmaSetupHookOwned))
        return;
    const uint64_t start = cpu->master_cycles;
    NmiUploadClock clock = {0};
    clock.bus.cpu = cpu;
    clock.bus.bank = 0x800000u;
    NmiUploadSelectPhase(&clock, entry == 0x80870cu
        ? NMI_UPLOAD_OAM_DATA : NMI_UPLOAD_PALETTE_DATA);
    /* Stop before the existing LDA/STA pair that starts the transfer. */
    clock.bus.count = LUFIA2_NMI_DMA_SETUP_OPCODES;
    const Lufia2Memory memory = {NmiUploadRead, NmiUploadWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiPrepareUploadDma(&memory, &state, entry);
    assert(clock.bus.index == clock.bus.count && !clock.bus.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == end);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= 512u);
    Lufia2DecompFrameReturnComplete(clock.bus.count, clock.bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
