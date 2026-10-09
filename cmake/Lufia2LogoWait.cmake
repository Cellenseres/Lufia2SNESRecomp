include_guard(GLOBAL)

# The logo's $80:8084 LDA $6A / BNE wait is separate from the five
# previously reviewed CMP $40 frame waits. It never touches an APU port.
function(lufia2_prepare_logo_wait sources_var)
    set(_sources "${${sources_var}}")
    set(_matches 0)
    foreach(_file IN LISTS _sources)
        if(NOT _file MATCHES "/interp_bridge[.]c$")
            continue()
        endif()
        file(READ "${_file}" _text)
        set(_anchor "        /* Star Ocean battle $00D9 work-wait specialization. This does not skip")
        set(_batch [=[
        /* $80:8084 waits for NMI to clear the logo frame flag. Retain the
         * last complete pair for the original instruction/deadline path.
         * No IRQ, observer, opcode hook or APU-port wait may be batched. */
#if !defined(SNES_COSIM) && !defined(SNESRECOMP_INTERP_PROFILE) && \
    (!defined(SNESRECOMP_REVERSE_DEBUG) || !SNESRECOMP_REVERSE_DEBUG)
        if (lufia2_frame_wait_fastforward_enabled && auto_quiescent &&
            pc_before == 0x808084u && in.pc == 0x8084u &&
            !stop_on_rti && s_interp_bridge_depth == 1 &&
            !s_interp_bus_timing_active && g_snes && g_snes->cart &&
            g_snes->cart->type == CART_LOROM && cpu->ram &&
            !in.e && in.mf && in.dp == 0 && !in.waiting && !in.stopped &&
            !in.nmiWanted && !in.irqWanted && !g_snes->inNmi &&
            !g_snes->inIrq && g_snes->nmiEnabled &&
            !g_snes->hIrqEnabled && !g_snes->vIrqEnabled &&
            !trace && !dtrace && !wlog_state_sync && cpu->ram[0x6a]) {
            static int observers = -1;
            if (observers < 0)
                observers = getenv("SNESRECOMP_CYC_WATCH") != NULL ||
                    getenv("SNESRECOMP_IBRWATCH") != NULL ||
                    getenv("SNESRECOMP_INTERP_DTRACE") != NULL ||
                    getenv("SNESRECOMP_INTERP_MS_PROF") != NULL ||
                    getenv("SNESRECOMP_INTERP_RATE_LOG") != NULL ||
                    getenv("SNESRECOMP_WLOG_STATE") != NULL;
            bool hooked = false;
            for (int h = 0; h < s_pre_opcode_hook_count; ++h)
                if (s_pre_opcode_hooks[h].pc24 == 0x008084u ||
                    s_pre_opcode_hooks[h].pc24 == 0x008086u)
                    hooked = true;
            const Cart *cart = g_snes->cart;
            if (!observers && !hooked && cart->rom && cart->romSize >= 0x88u &&
                cart->rom[0x84] == 0xa5 && cart->rom[0x85] == 0x6a &&
                cart->rom[0x86] == 0xd0 && cart->rom[0x87] == 0xfc) {
                const uint64_t pair_master =
                    2ull * bridge_region_speed(0x808084u) +
                    bridge_region_speed(0x6au) +
                    2ull * bridge_region_speed(0x808086u) + 6ull;
                /* At most 1200 execution clocks: one refresh tax at most.
                 * Reserve another 80 clocks before the deadline so charging
                 * that tax cannot consume the reference crossing pair. */
                const uint64_t pairs = Lufia2LogoWaitPairs(
                    cpu->master_cycles, s_lle_master_deadline, pair_master,
                    step_cap > steps ? (uint64_t)(step_cap - steps) : 0u);
                if (pairs) {
                    (void)bridge_bus_read(cpu, 0x808084u);
                    (void)bridge_bus_read(cpu, 0x808085u);
                    const uint8_t value = bridge_bus_read(cpu, 0x6au);
                    (void)bridge_bus_read(cpu, 0x808086u);
                    (void)bridge_bus_read(cpu, 0x808087u);
                    s_interp_continuous_read_epoch += pairs - 1u;
                    Lufia2LogoWaitState(&in, value);
                    g_interp816_cur_pc = 0x808086u;
                    const uint64_t master = pairs * pair_master;
                    cpu->cycles += pairs * 6u;
                    cpu->master_cycles += master;
                    snes_refresh_charge();
                    cpu->coprocessor_master_cycles = cpu->master_cycles;
                    snes_sync_master_clock(g_snes, cpu->master_cycles);
                    cart_sync_coprocessors(g_snes->cart, cpu->master_cycles);
                    if (rtl_apu_extended_frame_timing() ||
                        !interp_bridge_use_absolute_apu_timeline(
                            rtl_apu_frame_timeline_active(),
                            cart_has_sa1(g_snes->cart), false))
                        s_apu_pending_master += master;
                    bridge_apu_flush(cpu);
                    s_lufia2_logo_wait_pairs += pairs;
                    steps += (long)(pairs * 2u - 1u);
                    continue;
                }
            }
        }
#endif

]=])
        string(REPLACE "${_anchor}" "" _removed "${_text}")
        string(LENGTH "${_text}" _before)
        string(LENGTH "${_removed}" _after)
        string(LENGTH "${_anchor}" _length)
        math(EXPR _delta "${_before} - ${_after}")
        if(NOT _delta EQUAL _length)
            message(FATAL_ERROR "Logo wait anchor must match exactly once")
        endif()
        if(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD AND LUFIA2_ENABLE_LOGO_WAIT_FASTFORWARD)
            string(REPLACE "${_anchor}" "${_batch}${_anchor}" _text "${_text}")
        endif()
        set(_counter "#include \"src/lufia2_logo_wait.h\"\nstatic uint64_t s_lufia2_logo_wait_pairs;\nuint64_t lufia2_logo_wait_pairs_count(void) { return s_lufia2_logo_wait_pairs; }\n")
        file(WRITE "${_file}" "${_counter}${_text}")
        math(EXPR _matches "${_matches} + 1")
    endforeach()
    if(NOT _matches EQUAL 1)
        message(FATAL_ERROR "Logo wait requires one generated interpreter bridge")
    endif()
    set(${sources_var} "${_sources}" PARENT_SCOPE)
endfunction()
