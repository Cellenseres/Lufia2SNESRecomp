include_guard(GLOBAL)

# Invoked only for the explicit desktop audit build, after NativeWait's
# guarded seam. Every replacement is checked against the owner-built input.
function(lufia2_apply_bridge_audit text_var)
    set(_audit_text "${${text_var}}")
    macro(l2ba_replace old new expected)
        string(REPLACE "${old}" "" _removed "${_audit_text}")
        string(LENGTH "${_audit_text}" _before)
        string(LENGTH "${_removed}" _after)
        string(LENGTH "${old}" _length)
        math(EXPR _required "${_length} * ${expected}")
        math(EXPR _actual "${_before} - ${_after}")
        if(NOT _actual EQUAL _required)
            message(FATAL_ERROR "BridgeAudit anchor mismatch; inspect maintained overlay before building")
        endif()
        string(REPLACE "${old}" "${new}" _audit_text "${_audit_text}")
    endmacro()
    l2ba_replace([==[        const uint32_t pc_before = ((uint32_t)in.k << 16) | in.pc;]==] [==[        const uint32_t pc_before = ((uint32_t)in.k << 16) | in.pc;
        if (l2ba_recording || l2ba_sample_active)
            L2BALoop(pc_before, (unsigned)in.mf | ((unsigned)in.xf << 1) | ((unsigned)in.e << 2),
                     s_interp_bridge_depth == 1 && auto_quiescent && !stop_on_rti);]==] 1)
    l2ba_replace([==[        if (auto_quiescent) {
            QuiescentState now;]==] [==[        L2BA_MARK(L2BA_QUIESCENCE, s_interp_bridge_depth == 1);
        if (auto_quiescent) {
            QuiescentState now;]==] 1)
    l2ba_replace([==[        if (s_pre_opcode_hook_count > 0) {]==] [==[        L2BA_MARK(L2BA_DISPATCH, s_interp_bridge_depth == 1);
        if (s_pre_opcode_hook_count > 0) {]==] 1)
    l2ba_replace([==[        int _cyc = (int)Lufia2NativeWaitTryStep(&in,]==] [==[        L2BA_MARK(L2BA_OPCODE, s_interp_bridge_depth == 1);
        int _cyc = (int)Lufia2NativeWaitTryStep(&in,]==] 1)
    l2ba_replace([==[        s_interp_bus_timing_active=0;]==] [==[        L2BA_MARK(L2BA_RETIRE, s_interp_bridge_depth == 1);
        s_interp_bus_timing_active=0;]==] 1)
    l2ba_replace([==[        static int s_cycw = -1;]==] [==[        L2BA_MARK(L2BA_TAIL, s_interp_bridge_depth == 1);
        static int s_cycw = -1;]==] 1)
    l2ba_replace([==[    s_interp_bridge_depth--;]==] [==[    if (s_interp_bridge_depth == 1) L2BAFinish();
    s_interp_bridge_depth--;]==] 1)
    l2ba_replace([==[static void bridge_apu_flush(CpuState *cpu) {]==] [==[static void bridge_apu_flush_l2ba_impl(CpuState *cpu);
static void bridge_apu_flush(CpuState *cpu) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_APU_OTHER) : -1;
    bridge_apu_flush_l2ba_impl(cpu);
    if (previous >= 0) L2BALeave(previous);
}
static void bridge_apu_flush_l2ba_impl(CpuState *cpu) {]==] 1)
    l2ba_replace([==[static uint8_t bridge_bus_read(void *mem, uint32_t adr) {]==] [==[static uint8_t bridge_bus_read_l2ba_impl(void *mem, uint32_t adr);
static uint8_t bridge_bus_read(void *mem, uint32_t adr) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_BUS) : -1;
    uint8_t result = bridge_bus_read_l2ba_impl(mem, adr);
    if (previous >= 0) L2BALeave(previous);
    return result;
}
static uint8_t bridge_bus_read_l2ba_impl(void *mem, uint32_t adr) {]==] 1)
    l2ba_replace([==[static void bridge_bus_write(void *mem, uint32_t adr, uint8_t val) {]==] [==[static void bridge_bus_write_l2ba_impl(void *mem, uint32_t adr, uint8_t val);
static void bridge_bus_write(void *mem, uint32_t adr, uint8_t val) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_BUS) : -1;
    bridge_bus_write_l2ba_impl(mem, adr, val);
    if (previous >= 0) L2BALeave(previous);
}
static void bridge_bus_write_l2ba_impl(void *mem, uint32_t adr, uint8_t val) {]==] 1)
    l2ba_replace([==[static bool bridge_bus_read_word(void *mem, uint32_t adrl, uint32_t adrh,
                                 uint16_t *out) {]==] [==[static bool bridge_bus_read_word_l2ba_impl(void *mem, uint32_t adrl, uint32_t adrh,
                                 uint16_t *out);
static bool bridge_bus_read_word(void *mem, uint32_t adrl, uint32_t adrh,
                                 uint16_t *out) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_BUS) : -1;
    bool result = bridge_bus_read_word_l2ba_impl(mem, adrl, adrh, out);
    if (previous >= 0) L2BALeave(previous);
    return result;
}
static bool bridge_bus_read_word_l2ba_impl(void *mem, uint32_t adrl, uint32_t adrh,
                                 uint16_t *out) {]==] 1)
    l2ba_replace([==[static bool bridge_bus_write_word(void *mem, uint32_t adrl, uint32_t adrh,
                                  uint16_t val, bool reversed) {]==] [==[static bool bridge_bus_write_word_l2ba_impl(void *mem, uint32_t adrl, uint32_t adrh,
                                  uint16_t val, bool reversed);
static bool bridge_bus_write_word(void *mem, uint32_t adrl, uint32_t adrh,
                                  uint16_t val, bool reversed) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_BUS) : -1;
    bool result = bridge_bus_write_word_l2ba_impl(mem, adrl, adrh, val, reversed);
    if (previous >= 0) L2BALeave(previous);
    return result;
}
static bool bridge_bus_write_word_l2ba_impl(void *mem, uint32_t adrl, uint32_t adrh,
                                  uint16_t val, bool reversed) {]==] 1)
    l2ba_replace([==[static void sync_cpu_to_interp(const CpuState *c, Interp816 *in) {]==] [==[static void sync_cpu_to_interp_l2ba_impl(const CpuState *c, Interp816 *in);
static void sync_cpu_to_interp(const CpuState *c, Interp816 *in) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_STATE_SYNC) : -1;
    sync_cpu_to_interp_l2ba_impl(c, in);
    if (previous >= 0) L2BALeave(previous);
}
static void sync_cpu_to_interp_l2ba_impl(const CpuState *c, Interp816 *in) {]==] 1)
    l2ba_replace([==[static void sync_interp_to_cpu(const Interp816 *in, CpuState *c) {]==] [==[static void sync_interp_to_cpu_l2ba_impl(const Interp816 *in, CpuState *c);
static void sync_interp_to_cpu(const Interp816 *in, CpuState *c) {
    int previous = l2ba_sample_active ? L2BAEnter(L2BA_STATE_SYNC) : -1;
    sync_interp_to_cpu_l2ba_impl(in, c);
    if (previous >= 0) L2BALeave(previous);
}
static void sync_interp_to_cpu_l2ba_impl(const Interp816 *in, CpuState *c) {]==] 1)
    l2ba_replace([==[RtlApuLock();]==] [==[L2BA_CALL(L2BA_APU_LOCK, RtlApuLock());]==] 4)
    l2ba_replace([==[rtl_sync_apu_to_cpu_locked();]==] [==[L2BA_CALL(L2BA_APU_SYNC, rtl_sync_apu_to_cpu_locked());]==] 4)
    l2ba_replace([==[snes_catchupApu(g_snes);]==] [==[L2BA_CALL(L2BA_APU_SYNC, snes_catchupApu(g_snes));]==] 1)
    l2ba_replace([==[snes_sync_master_clock(g_snes, cpu->master_cycles);]==] [==[L2BA_CALL(L2BA_SNES_SYNC, snes_sync_master_clock(g_snes, cpu->master_cycles));]==] 5)
    l2ba_replace([==[snes_sync_master_clock(g_snes, target);]==] [==[L2BA_CALL(L2BA_SNES_SYNC, snes_sync_master_clock(g_snes, target));]==] 1)
    l2ba_replace([==[cart_sync_coprocessors(g_snes->cart, cpu->master_cycles);]==] [==[L2BA_CALL(L2BA_CART_SYNC, cart_sync_coprocessors(g_snes->cart, cpu->master_cycles));]==] 5)
    l2ba_replace([==[                RecompReturn _air = cpu_dispatch_pc_paired(cpu, target, _fs);]==] [==[                int _l2ba_aot = l2ba_sample_active ? L2BAEnter(L2BA_AOT) : -1;
                RecompReturn _air = cpu_dispatch_pc_paired(cpu, target, _fs);
                if (_l2ba_aot >= 0) L2BALeave(_l2ba_aot);]==] 1)
    l2ba_replace([==[cpu_dispatch_pc_paired(cpu, pc_before, 0);]==] [==[L2BA_CALL(L2BA_AOT, cpu_dispatch_pc_paired(cpu, pc_before, 0));]==] 1)
    set(${text_var} "#include \"src/diagnostics/lufia2_bridge_audit.h\"\n${_audit_text}" PARENT_SCOPE)
endfunction()
