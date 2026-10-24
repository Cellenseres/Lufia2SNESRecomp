#include "lufia2_perf.h"
#include <string.h>

bool lufia2_perf_enabled;
static Lufia2PerfClock s_clock;
static Lufia2PerfScope *s_parent;
static Lufia2PerfStat s_stats[LUFIA2_PERF_COUNT];
static uint32_t s_serial[LUFIA2_PERF_COUNT];
static uint32_t s_counters[LUFIA2_PERF_COUNTER_COUNT];
static Lufia2PerfHotspot s_hotspots[4096];
static uint32_t s_errors;
static const char *const s_names[LUFIA2_PERF_COUNT] = {
    "apu_batch", "dsp_sample", "spc_sample", "interp_opcode", "decomp_bridge",
    "ppu_line", "ppu_background", "ppu_sprites", "ppu_sprite_eval", "hdma",
    "map_prepare", "map_finalize", "shadow", "intro_compose", "ui_compose",
    "native_spc", "native_spc_signature",
    "native_frame_step", "native_sound_step", "runtime_nmi", "runtime_dispatch",
    "native_bus_sample", "native_clock_sample", "native_spc_body_sample",
    "battle_freeze", "battle_job", "battle_scene", "battle_hud",
    "battle_overlay", "battle_art", "battle_post",
    "ppu_run_line", "ppu_record", "battle_policy"
};

void Lufia2PerfInit(Lufia2PerfClock clock, bool enabled) {
    s_clock = clock;
    lufia2_perf_enabled = enabled && clock != 0;
    s_parent = 0;
    s_errors = 0;
    memset(s_serial, 0, sizeof s_serial);
    memset(s_hotspots, 0, sizeof s_hotspots);
    Lufia2PerfBeginFrame();
}
void Lufia2PerfBeginFrame(void) {
    if (s_parent) ++s_errors;
    s_parent = 0;
    memset(s_stats, 0, sizeof s_stats);
    memset(s_counters, 0, sizeof s_counters);
}
static void Enter(Lufia2PerfScope *scope, uint32_t id, uint32_t pc) {
    scope->id = id;
    scope->pc = pc;
    scope->parent = s_parent;
    scope->children = 0;
    scope->active = true;
    s_parent = scope;
    ++s_stats[id].samples;
    scope->start = s_clock();
}
void Lufia2PerfEnter(Lufia2PerfScope *scope, uint32_t id, uint32_t pc) {
    scope->active = false;
    if (!lufia2_perf_enabled || id >= LUFIA2_PERF_COUNT) return;
    ++s_stats[id].calls;
    Enter(scope, id, pc);
}
void Lufia2PerfEnterSample(Lufia2PerfScope *scope, uint32_t id) {
    scope->active = false;
    if (!lufia2_perf_enabled || id >= LUFIA2_PERF_COUNT) return;
    ++s_stats[id].calls;
    /* Sampling phase continues across frames, including short frames. */
    if ((s_serial[id]++ & 63u) != 0) return;
    Enter(scope, id, 0);
}
void Lufia2PerfLeave(Lufia2PerfScope *scope) {
    uint64_t elapsed, exclusive;
    if (!scope->active) return;
    elapsed = s_clock() - scope->start;
    if (s_parent != scope) { ++s_errors; return; }
    exclusive = elapsed >= scope->children ? elapsed - scope->children : 0;
    s_stats[scope->id].inclusive_ticks += elapsed;
    s_stats[scope->id].exclusive_ticks += exclusive;
    s_parent = scope->parent;
    if (s_parent) s_parent->children += elapsed;
    if (scope->pc) {
        uint32_t slot = (scope->pc * 2654435761u) >> 20;
        uint32_t probe;
        for (probe = 0; probe < 4096; ++probe, slot = (slot + 1u) & 4095u) {
            Lufia2PerfHotspot *hotspot = &s_hotspots[slot];
            if (hotspot->pc && hotspot->pc != scope->pc) continue;
            hotspot->pc = scope->pc;
            ++hotspot->calls;
            hotspot->inclusive_ticks += elapsed;
            hotspot->exclusive_ticks += exclusive;
            break;
        }
        if (probe == 4096) ++s_errors;
    }
    scope->active = false;
}
void Lufia2PerfCount(uint32_t id) {
    if (lufia2_perf_enabled && id < LUFIA2_PERF_COUNTER_COUNT) ++s_counters[id];
}
const Lufia2PerfStat *Lufia2PerfStats(void) { return s_stats; }
const uint32_t *Lufia2PerfCounters(void) { return s_counters; }
const Lufia2PerfHotspot *Lufia2PerfHotspots(uint32_t *count) {
    *count = 4096;
    return s_hotspots;
}
uint32_t Lufia2PerfErrors(void) { return s_errors; }
const char *Lufia2PerfName(uint32_t id) {
    return id < LUFIA2_PERF_COUNT ? s_names[id] : "unknown";
}
