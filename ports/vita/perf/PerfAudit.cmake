# Audit-only copies; fetched core and regular host stay unchanged.
function(lufia2_perf_replace source_var old new expected)
    string(REPLACE "${old}" "" _removed "${${source_var}}")
    string(LENGTH "${${source_var}}" _before)
    string(LENGTH "${_removed}" _after)
    string(LENGTH "${old}" _length)
    math(EXPR _actual "${_before} - ${_after}")
    math(EXPR _expected "${_length} * ${expected}")
    if(NOT _actual EQUAL _expected)
        message(FATAL_ERROR "Perf audit anchor mismatch: ${old}")
    endif()
    string(REPLACE "${old}" "${new}" _text "${${source_var}}")
    set(${source_var} "${_text}" PARENT_SCOPE)
endfunction()

function(lufia2_prepare_perf_sources sources_var core_root expected)
    set(_result)
    set(_matched 0)
    foreach(_input IN LISTS ${sources_var})
        get_filename_component(_name "${_input}" NAME)
        if(_name MATCHES "^(interp_bridge|common_rtl|common_cpu_infra|desktop_glue|apu|snes)\\.c$")
            if(NOT IS_ABSOLUTE "${_input}")
                set(_input "${CMAKE_SOURCE_DIR}/${_input}")
            endif()
            file(READ "${_input}" _text)
            string(REPLACE "\r\n" "\n" _text "${_text}")
            if(_name STREQUAL "interp_bridge.c")
                lufia2_perf_replace(_text [=[static void bridge_apu_flush(CpuState *cpu) {]=] [=[static void bridge_apu_flush(CpuState *cpu) {
    L2_SCOPE(audit_flush, L2_APU_FLUSH);]=] 1)
                lufia2_perf_replace(_text [=[        s_apu_pending_master = 0;
        return;]=] [=[        s_apu_pending_master = 0;
#ifdef SNESRECOMP_INTERP_PROFILE
        apu_prof_ms += 1000.0 * ((double)(clock() - _t0)) / CLOCKS_PER_SEC;
#endif
        return;]=] 1)
                lufia2_perf_replace(_text [=[static uint8_t bridge_bus_read(void *mem, uint32_t adr) {]=] [=[static uint8_t bridge_bus_read(void *mem, uint32_t adr) {
    L2_SAMPLE_SCOPE(audit_bus_read, L2_BUS, 257, 0, 0);]=] 1)
                lufia2_perf_replace(_text [=[static void bridge_bus_write(void *mem, uint32_t adr, uint8_t val) {]=] [=[static void bridge_bus_write(void *mem, uint32_t adr, uint8_t val) {
    L2_SAMPLE_SCOPE(audit_bus_write, L2_BUS, 257, 0, 0);]=] 1)
                lufia2_perf_replace(_text [=[static void sync_cpu_to_interp(const CpuState *c, Interp816 *in) {]=] [=[static void sync_cpu_to_interp(const CpuState *c, Interp816 *in) {
    L2_SAMPLE_SCOPE(audit_sync_in, L2_STATE_SYNC, 257, 0, 0);]=] 1)
                lufia2_perf_replace(_text [=[static void sync_interp_to_cpu(const Interp816 *in, CpuState *c) {]=] [=[static void sync_interp_to_cpu(const Interp816 *in, CpuState *c) {
    L2_SAMPLE_SCOPE(audit_sync_out, L2_STATE_SYNC, 257, 0, 0);]=] 1)
                lufia2_perf_replace(_text [=[static int _interp_run_core(CpuState *cpu, uint32_t entry_pc24,
                                 uint16_t s_exit, uint32_t *out_landing,
                                 uint32_t *out_return_pc,
                                 uint32_t yield_pc, uint16_t yield_flag_addr,
                                 uint8_t yield_flag_value,
                                 int reset_cap_on_bounce,
                                 const uint32_t *stop_pcs, int n_stop,
                                 int stop_on_rti) {]=] [=[static int _interp_run_core(CpuState *cpu, uint32_t entry_pc24,
                                 uint16_t s_exit, uint32_t *out_landing,
                                 uint32_t *out_return_pc,
                                 uint32_t yield_pc, uint16_t yield_flag_addr,
                                 uint8_t yield_flag_value,
                                 int reset_cap_on_bounce,
                                 const uint32_t *stop_pcs, int n_stop,
                                 int stop_on_rti) {
    L2_SCOPE(audit_interp, L2_INTERP);]=] 1)
                lufia2_perf_replace(_text [=[        const uint32_t pc_before = ((uint32_t)in.k << 16) | in.pc;]=] [=[        const uint32_t pc_before = ((uint32_t)in.k << 16) | in.pc;
        L2_SAMPLE_SCOPE(audit_pc, L2_PC, 1021, pc_before, (in.mf << 1) | in.xf);]=] 1)
                lufia2_perf_replace(_text [=[        if (auto_quiescent) {
            QuiescentState now;]=] [=[        if (auto_quiescent) {
            L2_SAMPLE_SCOPE(audit_quiescence, L2_QUIESCENCE, 257, 0, 0);
            QuiescentState now;]=] 1)
                lufia2_perf_replace(_text [=[            const int has_body  = cpu_dispatch_has_entry(cpu, target);]=] [=[            const int has_body  = cpu_dispatch_has_entry(cpu, target);
            L2PerfCall(pc_before, target, (in.mf << 1) | in.xf, op, has_body, bounce_ok);]=] 1)
                lufia2_perf_replace(_text [=[                RecompReturn _air = cpu_dispatch_pc_paired(cpu, target, _fs);]=] [=[                RecompReturn _air;
                {
                    L2_AOT_SCOPE(audit_aot, target, (in.mf << 1) | in.xf);
                    _air = cpu_dispatch_pc_paired(cpu, target, _fs);
                }
                L2PerfAotReturn(_air);]=] 1)
            endif()
            if(_name STREQUAL "common_rtl.c")
                lufia2_perf_replace(_text [=[void rtl_sync_apu_to_cpu_locked(void) {]=] [=[void rtl_sync_apu_to_cpu_locked(void) {
    L2_SCOPE(audit_apu_sync, L2_APU_SYNC);]=] 1)
            endif()
            if(_name STREQUAL "common_cpu_infra.c")
                lufia2_perf_replace(_text [=[    longjmp(g_watchdog_jmp, 1);]=] [=[    L2PerfAbort();
    longjmp(g_watchdog_jmp, 1);]=] 1)
            endif()
            if(_name STREQUAL "apu.c")
                lufia2_perf_replace(_text [=[bool apu_runToGuestCycle(Apu* apu, uint64_t guest_cycle,
                         uint32_t max_cycles) {]=] [=[bool apu_runToGuestCycle(Apu* apu, uint64_t guest_cycle,
                         uint32_t max_cycles) {
    L2_SCOPE(audit_apu_execute, L2_APU_EXEC);]=] 1)
            endif()
            if(_name STREQUAL "snes.c")
                lufia2_perf_replace(_text [=[void snes_catchupApu(Snes* snes) {]=] [=[void snes_catchupApu(Snes* snes) {
    L2_SCOPE(audit_apu_catchup, L2_APU_EXEC);]=] 1)
            endif()
            if(_name STREQUAL "desktop_glue.c")
                lufia2_perf_replace(_text [=[void RtlApuLock(void) {]=] [=[void RtlApuLock(void) {
    L2_SCOPE(audit_apu_lock, L2_APU_LOCK);]=] 1)
            endif()
            set(_text "#include \"lufia2_perf_audit.h\"\n${_text}")
            set(_output "${CMAKE_BINARY_DIR}/generated/perf-audit/${_name}")
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/perf-audit")
            file(WRITE "${_output}" "${_text}")
            get_filename_component(_parent "${_input}" DIRECTORY)
            # Preserve per-source production switches, especially common_rtl's
            # SNESRECOMP_FRAME_FINGERPRINTS=0. Copying only source text would
            # silently re-enable expensive diagnostic work in the audit build.
            foreach(_property IN ITEMS COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FLAGS
                    INCLUDE_DIRECTORIES LANGUAGE COMPILE_DEFINITIONS_RELEASE
                    COMPILE_DEFINITIONS_DEBUG COMPILE_DEFINITIONS_RELWITHDEBINFO
                    COMPILE_DEFINITIONS_MINSIZEREL)
                get_source_file_property(_value "${_input}" "${_property}")
                if(NOT _value STREQUAL "NOTFOUND")
                    set_property(SOURCE "${_output}" PROPERTY "${_property}" "${_value}")
                endif()
            endforeach()
            set_property(SOURCE "${_output}" APPEND PROPERTY INCLUDE_DIRECTORIES
                "${_parent};${core_root}/runner/src;${core_root}/runner/src/snes;${CMAKE_SOURCE_DIR}/src")
            list(APPEND _result "${_output}")
            math(EXPR _matched "${_matched} + 1")
        else()
            list(APPEND _result "${_input}")
        endif()
    endforeach()
    if(NOT _matched EQUAL expected)
        message(FATAL_ERROR "Perf audit translation unit count: ${_matched}, expected ${expected}")
    endif()
    set(${sources_var} "${_result}" PARENT_SCOPE)
endfunction()
