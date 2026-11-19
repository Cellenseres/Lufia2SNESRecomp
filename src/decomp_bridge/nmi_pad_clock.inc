#include "lufia2/nmi_pad_steps.h"

static void ActorBridgeNmiPadStep(CpuState *cpu, uint32_t entry);

static uint32_t NmiPadStepHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x808770u && hook == ActorBridgeNmiPadStep ? 0x8087a5u : 0u;
}

static bool NmiPadStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return NmiPadStepHookEnd(pc, hook) != 0u;
}

static void ActorBridgeNmiPadStep(CpuState *cpu, uint32_t entry) {
    /* The repeat path uses at most 69 cycles plus one refresh. */
    const unsigned budget = 640u;
    if (entry != 0x808770u || cpu != &g_cpu || cpu->m_flag || cpu->x_flag ||
        (cpu->X != 0u && cpu->X != 2u) ||
        !NmiClockWindow(cpu, false, entry, 0x8087a5u, budget, budget,
            true, NmiPadStepHookOwned))
        return;
    NmiUploadClock clock = {0};
    clock.bus.cpu = cpu;
    clock.bus.bank = 0x800000u;
    clock.pad = cpu->X;
    clock.pad_step = true;
    NmiUploadSelectPhase(&clock, NMI_UPLOAD_PAD_READ);
    const Lufia2Memory memory = {NmiUploadRead, NmiUploadWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2NmiReadPadStep(&memory, &state);
    NmiUploadBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(cpu->master_cycles - start <= budget);
    (void)start;
    assert(!clock.bus.remaining && !clock.word_pending &&
        clock.opcodes <= LUFIA2_NMI_PAD_STEP_OPCODES &&
        (clock.phase == NMI_UPLOAD_PAD_READ || clock.phase == NMI_UPLOAD_RESTORE));
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
