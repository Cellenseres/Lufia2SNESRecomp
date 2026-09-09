# Host-cost reductions from the dirty overlay. The single Lufia APU wait below
# advances the same guest/device time in bulk; it changes no AOT roots, unwind,
# register result, or APU synchronization policy.
function(lufia2_interp_replace source_var old new expected)
    set(_text "${${source_var}}")
    string(REPLACE "${old}" "" _removed "${_text}")
    string(LENGTH "${_text}" _n)
    string(LENGTH "${_removed}" _r)
    string(LENGTH "${old}" _o)
    math(EXPR _want "${expected} * ${_o}")
    math(EXPR _got "${_n} - ${_r}")
    if(NOT _got EQUAL _want)
        message(FATAL_ERROR "Pinned interpreter host-cost anchor mismatch: ${old}")
    endif()
    string(REPLACE "${old}" "${new}" _text "${_text}")
    set(${source_var} "${_text}" PARENT_SCOPE)
endfunction()

function(lufia2_optimize_interp_host_costs source_var)
    set(_source "${${source_var}}")
    # These remaining diagnostic presence switches are not cached by the
    # pinned core. Read them once on first use without changing empty/zero
    # presence semantics. The APU-port diagnostic is cached upstream now.
    set(_helpers [=[
#include "interp816.h"
static int lufia2_diag_wlog(void) {
    static int value = -1;
    if (value < 0) value = getenv("SNESRECOMP_WLOG_STATE") != NULL;
    return value;
}
static int lufia2_diag_yield(void) {
    static int value = -1;
    if (value < 0) value = getenv("SNESRECOMP_YIELD_DIAG") != NULL;
    return value;
}
static int lufia2_diag_stack(void) {
    static int value = -1;
    if (value < 0) value = getenv("SNESRECOMP_YIELD_STACK_DIAG") != NULL;
    return value;
}
static unsigned long long lufia2_poll_skipped;
unsigned long long lufia2_poll_skipped_count(void) {
    return lufia2_poll_skipped;
}
]=])
    # Apply before inserting helpers, so their initial lookups stay real getenv.
    lufia2_interp_replace(_source
        "const int wlog_state_sync = getenv(\"SNESRECOMP_WLOG_STATE\") != NULL;"
        "const int wlog_state_sync = lufia2_diag_wlog();" 1)
    lufia2_interp_replace(_source
        "getenv(\"SNESRECOMP_YIELD_DIAG\") &&"
        "lufia2_diag_yield() &&" 1)
    lufia2_interp_replace(_source
        "getenv(\"SNESRECOMP_YIELD_STACK_DIAG\") &&"
        "lufia2_diag_stack() &&" 1)
    lufia2_interp_replace(_source "#include \"interp816.h\"" "${_helpers}" 1)

    # Restricted form of the dirty bridge_fast_read path. Check the live cart
    # on every access; do not retain a ROM pointer or cache cartridge identity.
    # cpu_read8 and cart_getRomPtr in the pinned core resolve these ranges as
    # below. Out-of-bounds/mirrored ROM, SRAM, MMIO and coprocessors fall back.
    set(_bus_helper [=[
extern Snes *g_snes;
static uint8_t lufia2_bus_read8(CpuState *cpu, uint32_t adr) {
    const uint8_t bank = (uint8_t)(adr >> 16);
    const uint16_t addr = (uint16_t)adr;
    if (g_snes && g_snes->cart && g_snes->cart->type == CART_LOROM) {
        const int32_t woff = cpu_wram_offset(bank, addr);
        if (woff >= 0) {
            g_snes->cart->cpuBusAddress = adr & 0xFFFFFFu;
            const uint8_t value = cpu->ram[woff];
            cpu->open_bus = value;
            return value;
        }
        if (addr >= 0x8000u &&
            (bank <= 0x3fu || (bank >= 0x80u && bank <= 0xbfu))) {
            const uint32_t off = ((uint32_t)(bank & 0x7fu) << 15) |
                                 (addr & 0x7fffu);
            if (g_snes->cart->rom && off < g_snes->cart->romSize) {
                g_snes->cart->cpuBusAddress = adr & 0xFFFFFFu;
                const uint8_t value = g_snes->cart->rom[off];
                cpu->open_bus = value;
                return value;
            }
        }
    }
    return cpu_read8(cpu, bank, addr);
}
]=])
    lufia2_interp_replace(_source "extern Snes *g_snes;" "${_bus_helper}" 1)
    lufia2_interp_replace(_source
        "uint8_t value=cpu_read8(cpu,(uint8)((adr>>16)&0xff),(uint16)adr);"
        "uint8_t value=lufia2_bus_read8(cpu, adr);" 1)

    # Lufia's music-command handshake. This is intentionally PC- and byte-
    # exact instead of the dirty overlay's broad pattern matcher. It cannot
    # claim an unrelated game loop, and cached verification fails closed if
    # the pinned ROM/core context changes. Each skipped iteration advances the
    # normal SNES devices and charges the interpreter-derived 8 CPU cycles.
    set(_wait_old [=[
        /* Star Ocean battle $00D9 work-wait specialization. This does not skip
]=])
    set(_wait_new [=[
        /* Lufia II $80:9997: CMP long $00:2142; BNE $80:9997.
         * The loop has no visible intermediate state: it only spends guest
         * time until the SPC echoes A. Keep PC/flags untouched and let the
         * real interpreter execute the final, non-taken iteration. */
        if (auto_quiescent && pc_before == 0x809997u && in.mf && g_snes) {
            static int enabled = -1;
            static int bytes_ok = -1;
            if (enabled < 0) {
                const char *e = getenv("SNESRECOMP_POLL_FASTFWD");
                enabled = !(e && e[0] == '0');
            }
            if (bytes_ok < 0) {
                static const uint8_t expected[6] =
                    {0xcf, 0x42, 0x21, 0x00, 0xd0, 0xfa};
                bytes_ok = 1;
                for (unsigned i = 0; i < sizeof expected; i++)
                    if (bridge_bus_read(cpu, 0x809997u + i) != expected[i])
                        bytes_ok = 0;
            }
            if (enabled && bytes_ok) {
                const unsigned code_speed = bridge_region_speed(0x809997u);
                const unsigned data_speed = bridge_region_speed(0x002142u);
                const uint64_t compare_master =
                    4ull * code_speed + data_speed;
                const uint64_t branch_master = 2ull * code_speed + 6ull;
                const uint8_t accumulator = (uint8_t)in.a;
                unsigned long count = 0;
                for (;;) {
                    if (count >= 1000000ul || g_snes->inIrq ||
                        (s_lle_master_deadline &&
                         cpu->master_cycles >= s_lle_master_deadline))
                        break;
                    const uint8_t value = bridge_bus_read(cpu, 0x002142u);
                    if (value == accumulator)
                        break;
                    /* Execute CMP's architecturally visible state before its
                     * instruction boundary. An IRQ/deadline reached here must
                     * resume at BNE, just like the unaccelerated loop. */
                    const uint8_t result = (uint8_t)(accumulator - value);
                    in.c = accumulator >= value;
                    in.z = result == 0;
                    in.n = (result & 0x80u) != 0;
                    in.pc = 0x999bu;
                    cpu->cycles += 5u;
                    cpu->master_cycles += compare_master;
                    cpu->coprocessor_master_cycles = cpu->master_cycles;
                    snes_sync_master_clock(g_snes, cpu->master_cycles);
                    if (g_snes->inIrq ||
                        (s_lle_master_deadline &&
                         cpu->master_cycles >= s_lle_master_deadline))
                        break;
                    /* BNE is known taken because value != accumulator. */
                    in.pc = 0x9997u;
                    cpu->cycles += 3u;
                    cpu->master_cycles += branch_master;
                    cpu->coprocessor_master_cycles = cpu->master_cycles;
                    snes_sync_master_clock(g_snes, cpu->master_cycles);
                    count++;
                }
                if (count) {
                    lufia2_poll_skipped += count;
                    continue;
                }
                /* A CMP may have reached the deadline before its BNE. Its
                 * state is already in `in`; return through the normal top-of-
                 * loop deadline/IRQ handling without executing CMP twice. */
                if (in.pc == 0x999bu)
                    continue;
            }
        }

        /* Star Ocean battle $00D9 work-wait specialization. This does not skip
]=])
    lufia2_interp_replace(_source "${_wait_old}" "${_wait_new}" 1)
    # Fingerprints/refcounts filter impossible matches only. The original full
    # equality, slot order, repeat threshold, and the pinned 256-step window stay.
    set(_quiescent_struct_old [=[
        uint64_t write_epoch;
        uint64_t continuous_read_epoch;
        long step;
        unsigned repeats;
    } QuiescentState;
    QuiescentState qring[64];
    memset(qring, 0, sizeof qring);
]=])
    set(_quiescent_struct_new [=[
        uint64_t write_epoch;
        uint64_t continuous_read_epoch;
        long step;
        unsigned repeats;
    } QuiescentState;
    QuiescentState qring[64];
    memset(qring, 0, sizeof qring);
    /* The ring is rescanned on every interpreted opcode, so its scan cost is
     * paid per instruction. Fingerprints live in their own packed array: the
     * scan then walks 256 contiguous bytes instead of striding 64 structs,
     * and a struct is touched only once a fingerprint matches. The full
     * comparison still decides, in the same slot order, so the same entry
     * wins the match as before. */
    uint32_t qfp[64];
    memset(qfp, 0, sizeof qfp);
    /* Presence filter over the live fingerprints, so the common case -- the
     * interpreter making forward progress, no slot matching -- costs one byte
     * load instead of a 64-slot walk. Refcounted rather than a plain bitmap so
     * evicting one slot cannot clear a bucket another slot still occupies:
     * a zero here therefore *proves* no slot can match, which is what makes
     * skipping the scan behaviour-preserving. At most 64 entries are live, so
     * a byte per bucket cannot overflow. */
    uint8_t qcnt[1024];
    memset(qcnt, 0, sizeof qcnt);
]=])
    lufia2_interp_replace(_source "${_quiescent_struct_old}" "${_quiescent_struct_new}" 1)
    set(_quiescent_fill_old [=[
            now.write_epoch=g_interp_bridge_write_epoch; now.step=steps;
            now.continuous_read_epoch=s_interp_continuous_read_epoch;
            for (unsigned qi=0; qi<64; qi++) {
                QuiescentState *old=&qring[qi];
                if (old->step && steps-old->step<=256 &&
]=])
    set(_quiescent_fill_new [=[
            now.write_epoch=g_interp_bridge_write_epoch; now.step=steps;
            now.continuous_read_epoch=s_interp_continuous_read_epoch;
            uint32_t _fp;
            {
                _fp = now.pc * 2654435761u;
                _fp ^= (uint32_t)now.a * 40503u;
                _fp = _fp * 31u + (uint32_t)now.x;
                _fp = _fp * 31u + (uint32_t)now.y;
                _fp = _fp * 31u + (uint32_t)now.sp;
                _fp = _fp * 31u + (uint32_t)now.dp;
                _fp = _fp * 31u + (uint32_t)now.db;
                _fp = _fp * 31u + (uint32_t)now.k;
                _fp = _fp * 31u + (uint32_t)(
                    now.c | (now.z<<1) | (now.v<<2) | (now.n<<3) |
                    (now.i<<4) | (now.d<<5) | (now.mf<<6) | (now.xf<<7));
                _fp = _fp * 31u + (uint32_t)now.e;
                _fp ^= (uint32_t)now.write_epoch;
                _fp = _fp * 31u + (uint32_t)now.continuous_read_epoch;
                /* Never 0: a zeroed (never written) ring slot must not look
                 * like a candidate before the `old->step` test runs. */
                _fp |= 1u;
            }
            for (unsigned qi=0; qcnt[_fp & 1023u] && qi<64; qi++) {
                QuiescentState *old;
                if (qfp[qi]!=_fp) continue;
                old=&qring[qi];
                if (old->step && steps-old->step<=256 &&
]=])
    lufia2_interp_replace(_source "${_quiescent_fill_old}" "${_quiescent_fill_new}" 1)
    set(_quiescent_store_old [=[
            qring[steps & 63]=now;
]=])
    set(_quiescent_store_new [=[
            {
                const unsigned _slot=(unsigned)(steps & 63);
                if (qfp[_slot]) qcnt[qfp[_slot] & 1023u]--;
                qring[_slot]=now;
                qfp[_slot]=_fp;
                qcnt[_fp & 1023u]++;
            }
]=])
    lufia2_interp_replace(_source "${_quiescent_store_old}" "${_quiescent_store_new}" 1)
    set(${source_var} "${_source}" PARENT_SCOPE)
endfunction()

# Stack the game-specific interpreter optimizations on top of the portable
# snesrecomp-platform bridge overlay.  Keep this composition in the game tree:
# the platform layer must not know Lufia II PCs, ROM bytes, or wait loops.
function(lufia2_prepare_interp_host_costs sources_var snesrecomp_root)
    set(_platform_overlay
        "${CMAKE_BINARY_DIR}/generated/snesrecomp-platform/snes/interp_bridge.c")
    if(NOT EXISTS "${_platform_overlay}")
        message(FATAL_ERROR
            "Lufia2 interpreter host-cost overlay requires the prepared "
            "snesrecomp-platform bridge: ${_platform_overlay}")
    endif()

    file(READ "${_platform_overlay}" _patched)
    lufia2_optimize_interp_host_costs(_patched)

    set(_overlay_dir "${CMAKE_BINARY_DIR}/generated/lufia2/snes")
    set(_overlay "${_overlay_dir}/interp_bridge.c")
    file(MAKE_DIRECTORY "${_overlay_dir}")
    file(WRITE "${_overlay}" "${_patched}")
    set_source_files_properties("${_overlay}" PROPERTIES
        INCLUDE_DIRECTORIES
            "${snesrecomp_root}/runner/src/snes;${snesrecomp_root}/runner/src")

    set(_sources "${${sources_var}}")
    set(_platform_overlay_count 0)
    foreach(_source IN LISTS _sources)
        if(_source STREQUAL "${_platform_overlay}")
            math(EXPR _platform_overlay_count "${_platform_overlay_count} + 1")
        endif()
    endforeach()
    if(NOT _platform_overlay_count EQUAL 1)
        message(FATAL_ERROR
            "Expected exactly one prepared snesrecomp-platform bridge in "
            "${sources_var}; found ${_platform_overlay_count}")
    endif()
    list(REMOVE_ITEM _sources "${_platform_overlay}")
    list(APPEND _sources "${_overlay}")
    set(${sources_var} "${_sources}" PARENT_SCOPE)
endfunction()
