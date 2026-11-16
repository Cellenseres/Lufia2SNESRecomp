static void ActorBridgePlayerContinuation(CpuState *cpu, uint32_t entry) {
    uint32_t end;
    switch (entry) {
    case 0x83c0efu: end = 0x83c0f9u; break;
    case 0x83c202u: case 0x83c217u: case 0x83c243u: end = 0x83c245u; break;
    case 0x83c206u: end = 0x83c20fu; break;
    default: return;
    }
    if (cpu != &g_cpu || cpu->PB != 0x83u || cpu->DB != 0x83u ||
        cpu->D || cpu->_flag_D || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - 256u ||
        !NativeOpcodeWindowSupported(cpu, entry, 256u, 0x1ffcu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, end, PlayerStepHookOwned))
        return;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return;
    const unsigned budget = Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgePlayerStep);
    FixedSpanOpcode timing[6];
    unsigned count = 0u;
    const bool probe = entry == 0x83c0efu;
    if (probe) {
        memcpy(timing, field_leader_probe_timing, sizeof field_leader_probe_timing);
        count = 4u;
    } else {
        bool turn = entry == 0x83c206u;
        if (entry == 0x83c202u) {
            timing[count++] = (FixedSpanOpcode){0xc202u, 2u, 3u, 0u};
            timing[count++] = (FixedSpanOpcode){0xc204u, 2u,
                (uint8_t)(cpu->_flag_C ? 3u : 2u), 0u};
            turn = !cpu->_flag_C;
        } else if (entry == 0x83c217u) {
            timing[count++] = (FixedSpanOpcode){0xc217u, 2u, 3u, 0u};
        }
        if (turn) {
            static const FixedSpanOpcode request[] = {
                {0xc206u, 2u, 3u, 1u}, {0xc208u, 4u, 5u, 1u},
                {0xc20cu, 1u, 2u, 0u}, {0xc20du, 2u, 2u, 0u}
            };
            memcpy(timing + count, request, sizeof request);
            count += 4u;
        } else {
            timing[count++] = (FixedSpanOpcode){0xc243u, 2u, 3u, 0u};
        }
    }
    if (budget < count)
        return;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u,
        .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = probe
        ? Lufia2FieldProbeLeaderPosition(&memory, &state)
        : Lufia2PlayerSlotContinueStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == (probe ? LUFIA2_EXECUTION_RETURNED : LUFIA2_EXECUTION_BOUNDARY));
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
