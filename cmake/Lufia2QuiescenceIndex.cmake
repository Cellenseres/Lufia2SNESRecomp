include_guard(GLOBAL)
function(lufia2_apply_quiescence_index text_var)
    set(_indexed "${${text_var}}")
    macro(l2qi_replace old new)
        string(REPLACE "${old}" "" _removed "${_indexed}")
        string(LENGTH "${_indexed}" _before)
        string(LENGTH "${_removed}" _after)
        string(LENGTH "${old}" _length)
        math(EXPR _delta "${_before}-${_after}")
        if(NOT _delta EQUAL _length)
            message(FATAL_ERROR "Quiescence index anchor mismatch; review pinned bridge")
        endif()
        string(REPLACE "${old}" "${new}" _indexed "${_indexed}")
    endmacro()
    l2qi_replace([==[    memset(qring, 0, sizeof qring);]==] [==[    memset(qring, 0, sizeof qring);
    L2QIndex qindex;
    /* Latched once per bridge call: an A/B switch between calls can then
     * never leave a running search looking at an uninitialized index. */
    const int use_quiescence_index = lufia2_quiescence_index_enabled;
    if (use_quiescence_index && auto_quiescent) L2QInit(&qindex);]==])
    l2qi_replace([==[            for (unsigned qi=0; qi<64; qi++) {
                QuiescentState *old=&qring[qi];
                if (old->step && steps-old->step<=256 &&
                    old->pc==now.pc && old->a==now.a && old->x==now.x &&
                    old->y==now.y && old->sp==now.sp && old->dp==now.dp &&
                    old->db==now.db && old->k==now.k && old->c==now.c &&
                    old->z==now.z && old->v==now.v && old->n==now.n &&
                    old->i==now.i && old->d==now.d && old->mf==now.mf &&
                    old->xf==now.xf && old->e==now.e &&
                    old->write_epoch==now.write_epoch &&
                    old->continuous_read_epoch==now.continuous_read_epoch) {
                    now.repeats=old->repeats+1;
                    if (now.repeats>=2) {
                        /* Stable CPU/memory state is a genuine cooperative
                         * wait.  The owning scheduler advances idle hardware
                         * to the next timer comparator or vblank and resumes
                         * here after servicing that event.  Burning the poll
                         * inside the interpreter until any enabled timer fired
                         * made a single host frame consume many guest frames
                         * and could starve rendering.  Live MMIO polls are not
                         * mistaken for this path: continuous_read_epoch changes
                         * on every such read. */
                        s_lle_resume_pc24=pc_before;
                        s_lle_quiescent_yield = 1;
                        /* Flush accumulated SPC time BEFORE yielding so the
                         * SPC700 processes any pending port writes (Star Ocean
                         * boot handshake: CPU writes to $2140-$2143 and polls
                         * $2140 for response; without this flush the SPC stays
                         * frozen and the game hangs at $C8F425). */
                        bridge_apu_flush(cpu);
                        sync_interp_to_cpu(&in,cpu);
                        return 1;
                    }
                    break;
                }
            }
]==] [==[            if (use_quiescence_index) {
            uint64_t candidates=L2QCandidates(&qindex,now.pc,now.continuous_read_epoch,now.write_epoch);
            for (unsigned qi=L2QPop(&candidates); qi<64; qi=L2QPop(&candidates)) {
                QuiescentState *old=&qring[qi];
                if (old->step && steps-old->step<=256 &&
                    old->pc==now.pc && old->a==now.a && old->x==now.x &&
                    old->y==now.y && old->sp==now.sp && old->dp==now.dp &&
                    old->db==now.db && old->k==now.k && old->c==now.c &&
                    old->z==now.z && old->v==now.v && old->n==now.n &&
                    old->i==now.i && old->d==now.d && old->mf==now.mf &&
                    old->xf==now.xf && old->e==now.e &&
                    old->write_epoch==now.write_epoch &&
                    old->continuous_read_epoch==now.continuous_read_epoch) {
                    now.repeats=old->repeats+1;
                    if (now.repeats>=2) {
                        /* Stable CPU/memory state is a genuine cooperative
                         * wait.  The owning scheduler advances idle hardware
                         * to the next timer comparator or vblank and resumes
                         * here after servicing that event.  Burning the poll
                         * inside the interpreter until any enabled timer fired
                         * made a single host frame consume many guest frames
                         * and could starve rendering.  Live MMIO polls are not
                         * mistaken for this path: continuous_read_epoch changes
                         * on every such read. */
                        s_lle_resume_pc24=pc_before;
                        s_lle_quiescent_yield = 1;
                        /* Flush accumulated SPC time BEFORE yielding so the
                         * SPC700 processes any pending port writes (Star Ocean
                         * boot handshake: CPU writes to $2140-$2143 and polls
                         * $2140 for response; without this flush the SPC stays
                         * frozen and the game hangs at $C8F425). */
                        bridge_apu_flush(cpu);
                        sync_interp_to_cpu(&in,cpu);
                        return 1;
                    }
                    break;
                }
            }
            } else {
            for (unsigned qi=0; qi<64; qi++) {
                QuiescentState *old=&qring[qi];
                if (old->step && steps-old->step<=256 &&
                    old->pc==now.pc && old->a==now.a && old->x==now.x &&
                    old->y==now.y && old->sp==now.sp && old->dp==now.dp &&
                    old->db==now.db && old->k==now.k && old->c==now.c &&
                    old->z==now.z && old->v==now.v && old->n==now.n &&
                    old->i==now.i && old->d==now.d && old->mf==now.mf &&
                    old->xf==now.xf && old->e==now.e &&
                    old->write_epoch==now.write_epoch &&
                    old->continuous_read_epoch==now.continuous_read_epoch) {
                    now.repeats=old->repeats+1;
                    if (now.repeats>=2) {
                        /* Stable CPU/memory state is a genuine cooperative
                         * wait.  The owning scheduler advances idle hardware
                         * to the next timer comparator or vblank and resumes
                         * here after servicing that event.  Burning the poll
                         * inside the interpreter until any enabled timer fired
                         * made a single host frame consume many guest frames
                         * and could starve rendering.  Live MMIO polls are not
                         * mistaken for this path: continuous_read_epoch changes
                         * on every such read. */
                        s_lle_resume_pc24=pc_before;
                        s_lle_quiescent_yield = 1;
                        /* Flush accumulated SPC time BEFORE yielding so the
                         * SPC700 processes any pending port writes (Star Ocean
                         * boot handshake: CPU writes to $2140-$2143 and polls
                         * $2140 for response; without this flush the SPC stays
                         * frozen and the game hangs at $C8F425). */
                        bridge_apu_flush(cpu);
                        sync_interp_to_cpu(&in,cpu);
                        return 1;
                    }
                    break;
                }
            }
            }
]==])
    l2qi_replace([==[            qring[steps & 63]=now;]==] [==[            if (use_quiescence_index)
                L2QSet(&qindex,(unsigned)(steps & 63),now.pc,now.continuous_read_epoch,now.write_epoch,now.step != 0);
            qring[steps & 63]=now;]==])
    set(${text_var} "#include \"patches/quiescence_index.h\"\n${_indexed}" PARENT_SCOPE)
endfunction()
