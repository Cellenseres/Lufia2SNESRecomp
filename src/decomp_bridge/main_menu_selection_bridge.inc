static RecompReturn ActorBridgeMemberDispatch(CpuState *cpu,
    const ActorBridgeFrame *frame) {
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuDispatchMainSelection(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
#if defined(LUFIA2_HAVE_NATIVE_MEMBER_SCREENS)
        if ((result.pc == 0x82b2feu || result.pc == 0x82b370u) &&
            cpu->S == frame->entry_s && cpu_dispatch_has_entry(cpu, result.pc)) {
            cpu->host_return_valid = frame->hrv;
            return ActorBridgeMemberScreen(cpu, result.pc);
        }
#endif
#if defined(LUFIA2_HAVE_NATIVE_MAIN_MENUS)
        if ((result.pc == 0x82bf13u || result.pc == 0x82bfc5u ||
             result.pc == 0x82a88eu) && cpu->S == frame->entry_s &&
            cpu_dispatch_has_entry(cpu, result.pc)) {
            cpu->host_return_valid = frame->hrv;
            return ActorBridgeMainMenu(cpu, result.pc);
        }
#endif
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame->entry_s, frame->hrv);
    }
    return ActorBridgeReturn(cpu, frame, 2u, result.pc);
}
