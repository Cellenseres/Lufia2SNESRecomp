bool Lufia2DecompFieldRegionHasBudget(const CpuState *cpu, unsigned count) {
    return cpu == s_field_region_context.cpu && s_field_region_context.steps &&
        s_field_region_context.step_cap >= *s_field_region_context.steps &&
        s_field_region_context.step_cap - *s_field_region_context.steps >= (long)count;
}

void Lufia2DecompFieldRegionAccountPrefix(
    const CpuState *cpu, unsigned count, unsigned progress_mask) {
    assert(cpu == s_field_region_context.cpu && count <= 32u);
    if (!count)
        return;
    long *steps = s_field_region_context.steps;
    *steps += (long)count;
    if (s_frame_return_auto_quiescent) {
        for (unsigned opcode = 0u; opcode < count && progress_mask;
                ++opcode, progress_mask >>= 1u)
            if (progress_mask & 1u)
                *steps = (long)(count - opcode - 1u);
        *s_field_region_context.write_epoch = g_interp_bridge_write_epoch;
        *s_field_region_context.dynamic_epoch = s_interp_dynamic_progress_epoch;
    }
}

uint8_t Lufia2DecompFieldRegionInvokeChild(CpuState *cpu, void *context,
    Lufia2CpuState *state, uint32_t target, uint32_t site, uint8_t frame_size,
    Lufia2PushedChildCall child) {
    assert(cpu == s_field_region_context.cpu && child);
    const bool tracked = FramePairedCallTracks(target);
    if (!tracked)
        return child(context, state, target, site, frame_size);
    const FramePairedCallContext previous = s_frame_paired_call;
    const uint32_t previous_return = s_frame_paired_return_pc;
    const long room = s_field_region_context.step_cap - *s_field_region_context.steps;
    s_frame_paired_call = (FramePairedCallContext){cpu, target & 0x7fffffu,
        FramePairedCallBlocked(target, false, s_frame_return_auto_quiescent,
            g_interp_bridge_pc_hook || Lufia2NativeObserversActive(), room),
        s_field_region_context.steps, s_field_region_context.write_epoch,
        s_field_region_context.dynamic_epoch};
    s_frame_paired_return_pc = site + (uint32_t)frame_size + 1u;
    const uint8_t returned = child(context, state, target, site, frame_size);
    s_frame_paired_call = previous;
    s_frame_paired_return_pc = previous_return;
    return returned;
}
