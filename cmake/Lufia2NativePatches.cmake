include_guard(GLOBAL)

# This integration seam belongs to maintained build input. The owner creates
# an overlay copy; neither fetched sources nor game AOT output are edited.
function(lufia2_prepare_native_patch_sources sources_var core_root)
    file(SHA256 "${core_root}/runner/src/snes/interp_bridge.c" _core_hash)
    if(NOT _core_hash STREQUAL "3a41007e727e2d5c6b8d00441c40abd0ef0992ebcfd6b2cf404dd6ae3c06207a")
        message(FATAL_ERROR "Native patches require the reviewed pinned interpreter bridge; "
            "review the patch contract, then accept ${_core_hash}")
    endif()
    set(_result)
    set(_count 0)
    foreach(_input IN LISTS ${sources_var})
        get_filename_component(_name "${_input}" NAME)
        if(_name STREQUAL "interp_bridge.c")
            file(READ "${_input}" _text)
            string(REPLACE "\r\n" "\n" _text "${_text}")
            # Platform overlays run before this function. Pin the normalized
            # translation unit we actually modify, rather than claiming that
            # the pristine-core hash above also covers those earlier edits.
            string(SHA256 _input_hash "${_text}")
            if(NOT _input_hash STREQUAL
                   "b84aa7007b37032094d358ff379b1652c1556d3e4b3345afa6332d186be894cd")
                message(FATAL_ERROR
                    "Native patch input changed after the Platform overlay; "
                    "review the bridge contract, then accept ${_input_hash}")
            endif()
            set(_helpers "")
            if(LUFIA2_ENABLE_NATIVE_WAIT OR LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
                set(_old "int _cyc = interp816_runOpcode(&in);")
                string(REPLACE "${_old}" "" _removed "${_text}")
                string(LENGTH "${_text}" _before)
                string(LENGTH "${_removed}" _after)
                string(LENGTH "${_old}" _length)
                math(EXPR _delta "${_before} - ${_after}")
                if(NOT _delta EQUAL _length)
                    message(FATAL_ERROR "Native opcode seam must match exactly once")
                endif()
                set(_new "int _cyc = 0;")
                if(LUFIA2_ENABLE_NATIVE_WAIT)
                    string(APPEND _new [=[
        _cyc = (int)Lufia2NativeWaitTryStep(&in,
            auto_quiescent && !stop_on_rti && s_interp_bridge_depth == 1 &&
            pc_before == (((uint32_t)in.k << 16) | in.pc) &&
            ((in.pc == 0x900e && op == 0xc5) || (in.pc == 0x9010 && op == 0xf0)));]=])
                endif()
                if(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
                    string(APPEND _new [=[
        int _l2_actor_hooked = 0;
        int _l2_actor_d508_hooked = 0;
        if (((pc_before == 0x83C7F8u && op == 0xA6u) ||
             (pc_before == 0x83D508u && op == 0xA6u)) &&
            s_pre_opcode_hook_count > 0) {
            for (int _l2_hi = 0; _l2_hi < s_pre_opcode_hook_count; ++_l2_hi) {
                if (L2ActorEarlyReturnContainsOpcodePc(
                        s_pre_opcode_hooks[_l2_hi].pc24)) {
                    _l2_actor_hooked = 1;
                }
                if (L2ActorD508EarlyReturnContainsOpcodePc(
                        s_pre_opcode_hooks[_l2_hi].pc24)) {
                    _l2_actor_d508_hooked = 1;
                }
            }
        }
        if (!_cyc && lufia2_actor_early_return_enabled &&
            auto_quiescent && !stop_on_rti && s_interp_bridge_depth == 1 &&
            pc_before == 0x83C7F8u && op == 0xA6u &&
            !_l2_actor_hooked && !trace && !dtrace &&
            !wlog_state_sync && !Lufia2NativeObserversActive() &&
            s_lle_master_deadline > cpu->master_cycles &&
            s_lle_master_deadline - cpu->master_cycles >
                L2_ACTOR_EARLY_RETURN_MAX_MASTER &&
            L2ActorEarlyReturnFitsStepCap(steps, step_cap) &&
            g_snes && g_snes->cart && g_snes->cart->type == CART_LOROM &&
            !g_snes->inIrq &&
            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&
            L2ActorEarlyReturnBytesMatch(g_snes->cart->rom,
                                         g_snes->cart->romSize) &&
            L2ActorEarlyReturnEligible(cpu, &in, 1)) {
            _cyc = (int)L2ActorEarlyReturnStep(&in);
            steps += L2_ACTOR_EARLY_RETURN_OPCODES - 1;
            ++lufia2_actor_early_return_hits;
        }
        if (!_cyc && lufia2_actor_d508_early_return_enabled &&
            auto_quiescent && !stop_on_rti && s_interp_bridge_depth == 1 &&
            pc_before == 0x83D508u && op == 0xA6u &&
            !_l2_actor_d508_hooked && !trace && !dtrace &&
            !wlog_state_sync && !Lufia2NativeObserversActive() &&
            s_lle_master_deadline > cpu->master_cycles &&
            s_lle_master_deadline - cpu->master_cycles >
                L2_ACTOR_D508_EARLY_RETURN_MAX_MASTER &&
            L2ActorD508EarlyReturnFitsStepCap(steps, step_cap) &&
            g_snes && g_snes->cart && g_snes->cart->type == CART_LOROM &&
            !g_snes->inIrq &&
            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&
            L2ActorD508EarlyReturnBytesMatch(g_snes->cart->rom,
                                             g_snes->cart->romSize) &&
            L2ActorD508EarlyReturnEligible(cpu, &in, 1)) {
            _cyc = (int)L2ActorD508EarlyReturnStep(&in);
            steps += L2_ACTOR_D508_EARLY_RETURN_OPCODES - 1;
            ++lufia2_actor_d508_early_return_hits;
        }]=])
                endif()
                string(APPEND _new [=[
        if (!_cyc) _cyc = interp816_runOpcode(&in);]=])
                string(REPLACE "${_old}" "${_new}" _text "${_text}")
            endif()
            if(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD)
                set(_helpers [=[
static uint64_t s_lufia2_frame_wait_ff_pairs[5];
uint64_t lufia2_frame_wait_ff_pairs_count(void) {
    return s_lufia2_frame_wait_ff_pairs[0] +
           s_lufia2_frame_wait_ff_pairs[1] +
           s_lufia2_frame_wait_ff_pairs[2] +
           s_lufia2_frame_wait_ff_pairs[3] +
           s_lufia2_frame_wait_ff_pairs[4];
}
uint64_t lufia2_frame_wait_ff_site_pairs_count(unsigned site) {
    return site < 5u ? s_lufia2_frame_wait_ff_pairs[site] : 0u;
}
]=])
                set(_frame_wait_old [=[
        /* Star Ocean battle $00D9 work-wait specialization. This does not skip
]=])
                set(_frame_wait_new [=[
        /* Reviewed Lufia II frame waits. Every listed site is the exact
         * four-byte sequence CMP $40 / BEQ back to CMP.
         *
         * NMI changes the frame byte only between outer boundary runs. While
         * A equals $40, this exact loop cannot finish before the active master
         * deadline. Execute complete equal/taken pairs in one native batch,
         * retaining one complete pair for the reference interpreter. The
         * original bridge therefore still owns the deadline crossing, resume
         * PC and interrupt boundary. No game-state or rendering work is
         * bypassed. */
#if !defined(SNES_COSIM) && !defined(SNESRECOMP_INTERP_PROFILE) && \
    (!defined(SNESRECOMP_REVERSE_DEBUG) || !SNESRECOMP_REVERSE_DEBUG)
        if (lufia2_frame_wait_fastforward_enabled && auto_quiescent &&
            !stop_on_rti && s_interp_bridge_depth == 1 &&
            !s_interp_bus_timing_active &&
            s_lle_master_deadline > cpu->master_cycles && g_snes &&
            g_snes->cart && g_snes->cart->type == CART_LOROM && cpu->ram &&
            !in.e && in.mf && in.dp == 0 && !in.waiting &&
            !in.stopped && !in.nmiWanted && !in.irqWanted &&
            !g_snes->inNmi && !g_snes->inIrq && g_snes->nmiEnabled &&
            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&
            ((pc_before == 0x83900Eu && in.pc == 0x900Eu) ||
             (pc_before == 0x868B4Eu && in.pc == 0x8B4Eu) ||
             (pc_before == 0x85EC96u && in.pc == 0xEC96u) ||
             (pc_before == 0x848D4Fu && in.pc == 0x8D4Fu) ||
             (pc_before == 0x869752u && in.pc == 0x9752u)) &&
            !trace && !dtrace && !wlog_state_sync) {
            static int _l2_observer_active = -1;
            if (_l2_observer_active < 0) {
                _l2_observer_active =
                    getenv("SNESRECOMP_CYC_WATCH") != NULL ||
                    getenv("SNESRECOMP_IBRWATCH") != NULL ||
                    getenv("SNESRECOMP_INTERP_DTRACE") != NULL ||
                    getenv("SNESRECOMP_INTERP_MS_PROF") != NULL ||
                    getenv("SNESRECOMP_INTERP_RATE_LOG") != NULL ||
                    getenv("SNESRECOMP_WLOG_STATE") != NULL;
            }
            const unsigned _l2_site = pc_before == 0x83900Eu ? 0u :
                (pc_before == 0x868B4Eu ? 1u :
                (pc_before == 0x85EC96u ? 2u :
                (pc_before == 0x848D4Fu ? 3u : 4u)));
            const uint32_t _l2_cmp_pc24 = pc_before;
            const uint32_t _l2_beq_pc24 = pc_before + 2u;
            const uint32_t _l2_rom_offset = _l2_site == 0u ? 0x1900Eu :
                (_l2_site == 1u ? 0x30B4Eu :
                (_l2_site == 2u ? 0x2EC96u :
                (_l2_site == 3u ? 0x20D4Fu : 0x31752u)));
            int _l2_hooked = 0;
            const uint32_t _l2_cmp_hook_key =
                _l2_cmp_pc24 & 0x7FFFFFu;
            const uint32_t _l2_beq_hook_key =
                _l2_beq_pc24 & 0x7FFFFFu;
            for (int _l2_hi = 0; _l2_hi < s_pre_opcode_hook_count; ++_l2_hi) {
                if (s_pre_opcode_hooks[_l2_hi].pc24 == _l2_cmp_hook_key ||
                    s_pre_opcode_hooks[_l2_hi].pc24 == _l2_beq_hook_key) {
                    _l2_hooked = 1;
                    break;
                }
            }
            const int _l2_bytes_ok =
                g_snes->cart->rom &&
                g_snes->cart->romSize >= _l2_rom_offset + 4u &&
                g_snes->cart->rom[_l2_rom_offset + 0u] == 0xC5u &&
                g_snes->cart->rom[_l2_rom_offset + 1u] == 0x40u &&
                g_snes->cart->rom[_l2_rom_offset + 2u] == 0xF0u &&
                g_snes->cart->rom[_l2_rom_offset + 3u] == 0xFCu;
            if (!_l2_observer_active && !_l2_hooked && _l2_bytes_ok &&
                (uint8_t)in.a == cpu->ram[0x40u]) {
                const uint64_t _l2_cmp_master =
                    2ull * bridge_region_speed(_l2_cmp_pc24) +
                    bridge_region_speed(0x000040u);
                const uint64_t _l2_branch_master =
                    2ull * bridge_region_speed(_l2_beq_pc24) + 6ull;
                const uint64_t _l2_pair_master =
                    _l2_cmp_master + _l2_branch_master;
                const uint64_t _l2_remaining_steps = step_cap > steps
                    ? (uint64_t)(step_cap - steps) : 0u;
                const uint64_t _l2_pairs = L2FrameWaitBatchPairs(
                    cpu->master_cycles, s_lle_master_deadline,
                    _l2_pair_master, _l2_remaining_steps);
                if (_l2_pairs) {
                    /* Replay one pair through the original read callback.
                     * It preserves the final open-bus value, cart bus note,
                     * dynamic-value cache and continuous-read epoch. Further
                     * pairs repeat only the unchanged $40 read. */
                    (void)bridge_bus_read(cpu, _l2_cmp_pc24);
                    (void)bridge_bus_read(cpu, _l2_cmp_pc24 + 1u);
                    (void)bridge_bus_read(cpu, 0x000040u);
                    (void)bridge_bus_read(cpu, _l2_beq_pc24);
                    (void)bridge_bus_read(cpu, _l2_beq_pc24 + 1u);
                    s_interp_continuous_read_epoch += _l2_pairs - 1u;

                    L2FrameWaitApplyEqualBatchState(
                        &in, (uint16_t)_l2_cmp_pc24);
                    g_interp816_cur_pc = _l2_beq_pc24;

                    const uint64_t _l2_master =
                        _l2_pairs * _l2_pair_master;
                    cpu->cycles += _l2_pairs * 6u;
                    cpu->master_cycles += _l2_master;
                    cpu->coprocessor_master_cycles = cpu->master_cycles;
                    snes_sync_master_clock(g_snes, cpu->master_cycles);
                    cart_sync_coprocessors(g_snes->cart,
                                           cpu->master_cycles);
                    if (rtl_apu_extended_frame_timing() ||
                        !interp_bridge_use_absolute_apu_timeline(
                            rtl_apu_frame_timeline_active(),
                            g_snes && cart_has_sa1(g_snes->cart), false)) {
                        s_apu_pending_master += _l2_master;
                    }
                    bridge_apu_flush(cpu);

                    s_lufia2_frame_wait_ff_pairs[_l2_site] += _l2_pairs;
                    /* `continue` supplies the final loop increment. */
                    steps += (long)(_l2_pairs * 2u - 1u);
                    continue;
                }
            }
        }
#endif

        /* Star Ocean battle $00D9 work-wait specialization. This does not skip
]=])
                string(REPLACE "${_frame_wait_old}" "" _frame_wait_removed "${_text}")
                string(LENGTH "${_text}" _frame_wait_before)
                string(LENGTH "${_frame_wait_removed}" _frame_wait_after)
                string(LENGTH "${_frame_wait_old}" _frame_wait_length)
                math(EXPR _frame_wait_delta
                    "${_frame_wait_before} - ${_frame_wait_after}")
                if(NOT _frame_wait_delta EQUAL _frame_wait_length)
                    message(FATAL_ERROR
                        "Frame-wait batching seam must match exactly once")
                endif()
                string(REPLACE "${_frame_wait_old}" "${_frame_wait_new}"
                    _text "${_text}")
            endif()
            if(LUFIA2_ENABLE_QUIESCENCE_INDEX)
                include("${CMAKE_SOURCE_DIR}/cmake/Lufia2QuiescenceIndex.cmake")
                lufia2_apply_quiescence_index(_text)
            endif()
            if(LUFIA2_ENABLE_BRIDGE_AUDIT)
                include("${CMAKE_SOURCE_DIR}/cmake/Lufia2BridgeAudit.cmake")
                lufia2_apply_bridge_audit(_text)
            endif()
            set(_out "${CMAKE_BINARY_DIR}/generated/native-patches/interp_bridge.c")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/native-patches")
            file(WRITE "${_out}"
                "#include \"patches/native_patches.h\"\n${_helpers}\n${_text}")
            foreach(_prop IN ITEMS COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FLAGS
                    INCLUDE_DIRECTORIES LANGUAGE COMPILE_DEFINITIONS_RELEASE
                    COMPILE_DEFINITIONS_DEBUG COMPILE_DEFINITIONS_RELWITHDEBINFO
                    COMPILE_DEFINITIONS_MINSIZEREL)
                get_source_file_property(_value "${_input}" "${_prop}")
                if(NOT _value STREQUAL "NOTFOUND")
                    set_property(SOURCE "${_out}" PROPERTY "${_prop}" "${_value}")
                endif()
            endforeach()
            get_filename_component(_parent "${_input}" DIRECTORY)
            set_property(SOURCE "${_out}" APPEND PROPERTY INCLUDE_DIRECTORIES
                "${_parent};${core_root}/runner/src;${core_root}/runner/src/snes")
            list(APPEND _result "${_out}")
            math(EXPR _count "${_count}+1")
        else()
            list(APPEND _result "${_input}")
        endif()
    endforeach()
    if(NOT _count EQUAL 1)
        message(FATAL_ERROR "Native patches expected exactly one interpreter bridge")
    endif()
    set(${sources_var} "${_result}" PARENT_SCOPE)
endfunction()
