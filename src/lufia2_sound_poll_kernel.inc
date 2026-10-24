/* Included inside the pinned interpreter bridge's instruction loop.
 * This is a host dispatch optimization, not a guest-time fast-forward.
 * The original opcode executor, bus callbacks and clock accounting retire
 * every CMP/LDA/branch separately. Interrupts/deadlines return to the outer loop.
 */
#if !defined(SNES_COSIM) && !defined(SNESRECOMP_INTERP_PROFILE) && \
    (!defined(SNESRECOMP_REVERSE_DEBUG) || !SNESRECOMP_REVERSE_DEBUG)
{
const bool frame_poll = pc_before == 0x8293c8u;
const bool load_poll = pc_before == 0x809a26u || pc_before == 0x809a2cu;
const uint32_t cmp_pc = frame_poll ? 0x8293c8u :
    (pc_before == 0x809a0fu || pc_before == 0x809a17u || load_poll ? pc_before : 0x809997u);
const uint32_t branch_pc = cmp_pc + (frame_poll ? 2u : 4u);
const uint32_t rom_offset = ((cmp_pc >> 16) & 127u) * 32768u + (cmp_pc & 32767u);
const unsigned signature_size = frame_poll ? 4u : 6u;
const char *signature = frame_poll ? "\xc5\x40\xf0\xfc" : load_poll ?
    (cmp_pc == 0x809a2cu ? "\xaf\x43\x21\x00\xd0\xfa" : "\xaf\x42\x21\x00\xd0\xfa") :
    (cmp_pc == 0x809a17u ? "\xcf\x43\x21\x00\xd0\xfa" : "\xcf\x42\x21\x00\xd0\xfa");
if ((frame_poll ? lufia2_frame_poll_kernel_enabled : lufia2_sound_poll_kernel_enabled) &&
    auto_quiescent && !stop_on_rti &&
    !n_stop && !s_sched_yield_pc && s_interp_bridge_depth == 1 &&
    pc_before == cmp_pc && in.mf && (!frame_poll || in.dp == 0) &&
    !in.e && !in.waiting && !in.stopped &&
    !in.nmiWanted && !in.irqWanted && !s_interp_bus_timing_active &&
    !trace && !dtrace && !wlog_state_sync && !g_interp_bridge_pc_hook &&
    !s_interp_pctrace && !Lufia2NativeObserversActive() &&
    s_lle_master_deadline > cpu->master_cycles && g_snes &&
    g_snes->cart && g_snes->cart->type == CART_LOROM &&
    !g_snes->cart->superfx && !g_snes->cart->cx4 && !g_snes->cart->sa1 &&
    g_snes->cart->rom && g_snes->cart->romSize >= rom_offset + signature_size &&
    !memcmp(g_snes->cart->rom + rom_offset, signature, signature_size)) {
    bool hooked = false;
    if (snesrecomp_hook_pages_contains(s_pre_opcode_hook_pages, cmp_pc))
        for (int hi = 0; hi < s_pre_opcode_hook_count; ++hi)
            hooked |= s_pre_opcode_hooks[hi].pc24 == (cmp_pc & 0x7fffffu) ||
                      s_pre_opcode_hooks[hi].pc24 == (branch_pc & 0x7fffffu);
    /* A redirect or observer may previously have recorded a poll PC without
     * retiring it. Reject that history instead of assuming an executed read.
     * Existing BNE entries at the current epoch are harmless: the next CMP
     * advances the epoch before its BNE. Future-epoch entries fail closed. */
    for (unsigned qi = 0; !hooked && qi < 64u; ++qi) {
        const QuiescentState *old = &qring[qi];
        if (old->step && ((old->pc == cmp_pc &&
                old->continuous_read_epoch >= s_interp_continuous_read_epoch) ||
            (old->pc == branch_pc &&
                old->continuous_read_epoch > s_interp_continuous_read_epoch))) hooked = true;
    }
    /* A CMP read increments continuous_read_epoch; the following BNE has a
     * different PC. Neither state can match an earlier ring entry at the
     * same read epoch. Retire overwritten ring slots, preserving eviction
     * of unrelated states while avoiding the impossible equality search.
     * Stop well before epoch wrap, where that argument would no longer hold.
     */
    unsigned retired = 0;
    for (;;) {
        const uint32_t poll_pc = ((uint32_t)in.k << 16) | in.pc;
        if (hooked || (poll_pc != cmp_pc && poll_pc != branch_pc) ||
            steps >= step_cap || retired == 512u ||
            s_interp_continuous_read_epoch > UINT64_MAX - 1024u ||
            cpu->master_cycles >= s_lle_master_deadline ||
            (!in.i && g_snes->inIrq)) break;
        const unsigned slot = (unsigned)(steps & 63);
        if (qfp[slot]) qcnt[qfp[slot] & 1023u]--;
        qfp[slot] = 0;
        qring[slot].step = 0;

        /* Diagnosis needs the opcode value only. The guarded immutable LoROM
         * signature was checked above; runOpcode below performs the original
         * timed fetch. Do not issue an extra diagnostic bus read before it. */
        const uint8_t poll_op = g_snes->cart->rom[rom_offset + poll_pc - cmp_pc];
        ITraceEnt event = { poll_pc, poll_op };
        if (itn < 8) head[itn] = event;
        ++itn;
        ITraceRecentEnt *recent =
            &g_itrace_recent[g_itrace_recent_n++ & (ITRACE_RECENT_LEN - 1)];
        recent->pc = poll_pc; recent->frame = snes_frame_counter;
        recent->sp = in.sp; recent->op = poll_op; recent->pad = 0;

        s_interp_bus_master = 0;
        s_interp_bus_cycles = 0;
        s_interp_bus_timing_active = 1;
        cpu->coprocessor_master_cycles = cpu->master_cycles;
        int cycles = interp816_runOpcode(&in);
        s_interp_bus_timing_active = 0;
        if (cycles <= 0) cycles = 1;
        const unsigned internal = (unsigned)cycles > s_interp_bus_cycles ?
            (unsigned)cycles - s_interp_bus_cycles : 0;
        const uint64_t master = s_interp_bus_master + (uint64_t)internal * 6u;
        ++g_interp_bridge_steps;
        cpu->cycles += (uint64_t)cycles;
        cpu->master_cycles += master;
        snes_refresh_charge();
        cpu->coprocessor_master_cycles = cpu->master_cycles;
        snes_sync_master_clock(g_snes, cpu->master_cycles);
        cart_sync_coprocessors(g_snes->cart, cpu->master_cycles);
        if (rtl_apu_extended_frame_timing() ||
            !interp_bridge_use_absolute_apu_timeline(
                rtl_apu_frame_timeline_active(), cart_has_sa1(g_snes->cart), false)) {
            s_apu_pending_master += master;
            if (s_apu_pending_master >= bridge_bounce_flush_thresh()) bridge_apu_flush(cpu);
        }
        if (progress_write_epoch != g_interp_bridge_write_epoch ||
            progress_dynamic_epoch != s_interp_dynamic_progress_epoch) {
            progress_write_epoch = g_interp_bridge_write_epoch;
            progress_dynamic_epoch = s_interp_dynamic_progress_epoch;
            bridge_apu_flush(cpu);
            steps = 0;
        }
        if (frame_poll) ++lufia2_frame_poll_kernel_steps;
        else ++lufia2_sound_poll_kernel_steps;
        ++retired;
        ++steps;
    }
    if (retired) {
        /* The enclosing for-loop supplies the last instruction increment. */
        --steps;
        continue;
    }
}
}
#endif
