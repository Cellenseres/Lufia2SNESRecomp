if (pc_before == 0x86925cu && auto_quiescent && !stop_on_rti && !n_stop &&
    s_interp_bridge_depth == 1 && !trace && !dtrace && !wlog_state_sync &&
    !s_interp_pctrace && !g_interp_bridge_pc_hook && !s_interp_bus_timing_active &&
    steps >= 0 && step_cap > steps && step_cap - steps >= 32) {
    sync_interp_to_cpu(&in, cpu);
    const int saved_blocked = s_frame_return_interrupt_blocked;
    const int current_blocked = in.waiting || in.stopped || in.nmiWanted || in.irqWanted;
    s_frame_return_interrupt_blocked = current_blocked;
    const bool can_resume = Lufia2DecompWorldFrameCanResume(cpu);
    s_frame_return_interrupt_blocked = saved_blocked;
    if (can_resume) {
        if (s_apu_pending_master >= bridge_bounce_flush_thresh())
            bridge_apu_flush(cpu);
        const int saved_auto = s_frame_return_auto_quiescent;
        const int saved_apu = g_interp_apu_driving;
        const int saved_base = s_interp_bounce_recomp_base;
        const int saved_owner = s_interp_bounce_owner_depth;
        const unsigned saved_hrv = cpu->host_return_valid;
        s_frame_return_auto_quiescent = auto_quiescent;
        g_interp_apu_driving = 0;
        s_interp_bounce_recomp_base = g_recomp_stack_top;
        s_interp_bounce_owner_depth = s_interp_bridge_depth;
        s_lle_next_unwind_is_deadline = 0;
        cpu->host_return_valid = 0;
        uint32_t next_pc = pc_before;
        s_frame_return_interrupt_blocked = current_blocked;
        const RecompReturn result = Lufia2DecompWorldFrameResume(cpu, &next_pc);
        s_frame_return_interrupt_blocked = saved_blocked;
        cpu->host_return_valid = (uint8_t)saved_hrv;
        s_interp_bounce_owner_depth = saved_owner;
        s_interp_bounce_recomp_base = saved_base;
        g_interp_apu_driving = saved_apu;
        s_frame_return_auto_quiescent = saved_auto;
        sync_cpu_to_interp(cpu, &in);
        if (result != RECOMP_RETURN_NORMAL) {
            if (s_lle_unwind_active &&
                s_lle_unwind_owner_depth == s_interp_bridge_depth) {
                const uint32_t landing = s_lle_unwind_pc24;
                const int deadline = s_lle_unwind_is_deadline;
                s_lle_unwind_active = 0;
                s_lle_unwind_owner_depth = 0;
                s_lle_unwind_is_deadline = 0;
                in.k = (uint8_t)(landing >> 16);
                in.pc = (uint16_t)landing;
                if (deadline) {
                    s_lle_resume_pc24 = landing;
                    sync_interp_to_cpu(&in, cpu);
                    bridge_apu_flush(cpu);
                    return 1;
                }
                continue;
            }
            if (s_lle_unwind_active) {
                sync_interp_to_cpu(&in, cpu);
                return 1;
            }
            if (result == RECOMP_RETURN_SKIP_1 && !interp_owner_crossed(in.sp)) {
                in.k = (uint8_t)(next_pc >> 16);
                in.pc = (uint16_t)next_pc;
                if (reset_cap_on_bounce)
                    steps = 0;
                continue;
            }
            sync_interp_to_cpu(&in, cpu);
            if (yield_pc) {
                bridge_apu_flush(cpu);
                return 0;
            }
            return 1;
        }
        in.k = (uint8_t)(next_pc >> 16);
        in.pc = (uint16_t)next_pc;
        if (reset_cap_on_bounce)
            steps = 0;
        continue;
    }
}
