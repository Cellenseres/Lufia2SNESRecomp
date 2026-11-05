#include "boot_nmi_wait.h"

static void ActorBridgeBootNmiWait(CpuState *cpu, uint32_t entry);

bool Lufia2DecompBootNmiWaitHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeBootNmiWait &&
           (pc == 0x808084u || pc == 0x808086u);
}

static Lufia2ExecutionResult BootNmiWaitIteration(const Lufia2Memory *memory,
    Lufia2CpuState *cpu, uint32_t entry) {
    const Lufia2ExecutionResult load = Lufia2BootNmiWaitStep(memory, cpu, entry);
    if (load.flow != LUFIA2_EXECUTION_BOUNDARY || load.pc != entry + 2u)
        return load;
    return Lufia2BootNmiWaitStep(memory, cpu, load.pc);
}

static void ActorBridgeBootNmiWait(CpuState *cpu, uint32_t entry) {
    if ((entry != 0x808084u && entry != 0x808086u) || !g_snes ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->m_flag != 1u || cpu->x_flag != 1u || cpu->D || cpu->_flag_D ||
        !Lufia2DecompFrameRangeAllows(entry, entry + 2u,
                                     Lufia2DecompBootNmiWaitHookOwned) ||
        !ActorBridgeSingleOpcodeSupported(cpu, entry, 80u))
        return;
    if (entry == 0x808084u && Lufia2DecompFrameOwnedSpanBudget(
            entry, entry + 4u, ActorBridgeBootNmiWait) >= 16u &&
        ActorBridgePollRun(cpu, entry,
            BootNmiWaitIteration, 2u, true, Lufia2DecompBootNmiWaitHookOwned, 1240u))
        return;
    const bool branch = entry == 0x808086u;
    const ActorTimedOpcode opcode = {
        (uint16_t)entry, 2u, branch ? 2u : 3u, 0u, 0u, branch};
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    bus.opcode = &opcode;
    bus.cycles = opcode.cycles;
    for (unsigned byte = 0u; byte < 2u; ++byte)
        (void)ActorBridgeTimedRead(&bus, entry + byte);
    const Lufia2Memory memory = {ActorBridgeTimedRead, NULL, &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BootNmiWaitStep(&memory, &state, entry);
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(1u, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
