static int Lufia2DecompRunBootEntry(Interp816 *in, CpuState *cpu, uint32_t entry) {
    if (entry != 0x808035u || !cpu_dispatch_has_entry(cpu, entry))
        return 0;
    extern bool Lufia2DecompBootEntryReady(CpuState *, uint32_t);
    sync_interp_to_cpu(in, cpu);
    if (Lufia2DecompBootEntryReady(cpu, entry)) {
        bridge_apu_flush(cpu);
        const int saved_apu = g_interp_apu_driving;
        const int saved_base = s_interp_bounce_recomp_base;
        const int saved_owner = s_interp_bounce_owner_depth;
        g_interp_apu_driving = 0;
        s_interp_bounce_recomp_base = g_recomp_stack_top;
        s_interp_bounce_owner_depth = s_interp_bridge_depth;
        ++g_interp_bridge_bounces;
        if (g_interp_bridge_bounce_hook)
            g_interp_bridge_bounce_hook(entry, cpu->m_flag, cpu->x_flag);
        s_lle_next_unwind_is_deadline = 0;
        const RecompReturn result = cpu_dispatch_pc_from(cpu, entry,
            cpu->S, entry);
        s_interp_bounce_owner_depth = saved_owner;
        s_interp_bounce_recomp_base = saved_base;
        g_interp_apu_driving = saved_apu;
        sync_cpu_to_interp(cpu, in);
        if (result != RECOMP_RETURN_NORMAL && s_lle_unwind_active &&
            s_lle_unwind_owner_depth == s_interp_bridge_depth) {
            const uint32_t target = s_lle_unwind_pc24;
            const int reached_deadline = s_lle_unwind_is_deadline;
            s_lle_unwind_active = 0;
            s_lle_unwind_owner_depth = 0;
            s_lle_unwind_is_deadline = 0;
            in->k = (uint8_t)(target >> 16);
            in->pc = (uint16_t)target;
            sync_interp_to_cpu(in, cpu);
            if (reached_deadline) {
                s_lle_resume_pc24 = target;
                bridge_apu_flush(cpu);
                return 2;
            }
            return 1;
        }
        fprintf(stderr, "[boot-entry] unexpected return %d\n", (int)result);
        bridge_apu_flush(cpu);
        return -1;
    }
    return 0;
}
