#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Host diagnostics; no guest state or semantic-library dependencies. */
enum {
    LUFIA2_PERF_APU, LUFIA2_PERF_DSP_SAMPLE, LUFIA2_PERF_SPC_SAMPLE,
    LUFIA2_PERF_INTERP_SAMPLE, LUFIA2_PERF_DECOMP,
    LUFIA2_PERF_PPU_LINE, LUFIA2_PERF_PPU_BG, LUFIA2_PERF_PPU_SPR,
    LUFIA2_PERF_PPU_EVAL, LUFIA2_PERF_HDMA,
    LUFIA2_PERF_MAP_PREPARE, LUFIA2_PERF_MAP_FINALIZE, LUFIA2_PERF_SHADOW,
    LUFIA2_PERF_INTRO, LUFIA2_PERF_UI,
    LUFIA2_PERF_NATIVE_SPC, LUFIA2_PERF_NATIVE_SPC_SIGNATURE,
    LUFIA2_PERF_NATIVE_FRAME_STEP, LUFIA2_PERF_NATIVE_SOUND_STEP,
    LUFIA2_PERF_RUNTIME_NMI, LUFIA2_PERF_RUNTIME_DISPATCH,
    LUFIA2_PERF_NATIVE_BUS_SAMPLE, LUFIA2_PERF_NATIVE_CLOCK_SAMPLE,
    LUFIA2_PERF_NATIVE_SPC_BODY_SAMPLE,
    LUFIA2_PERF_BATTLE_FREEZE, LUFIA2_PERF_BATTLE_JOB,
    LUFIA2_PERF_BATTLE_SCENE, LUFIA2_PERF_BATTLE_HUD,
    LUFIA2_PERF_BATTLE_OVERLAY, LUFIA2_PERF_BATTLE_ART,
    LUFIA2_PERF_BATTLE_POST, LUFIA2_PERF_PPU_RUN_LINE,
    LUFIA2_PERF_PPU_RECORD, LUFIA2_PERF_BATTLE_POLICY, LUFIA2_PERF_COUNT
};
enum {
    LUFIA2_PERF_PORT_READ, LUFIA2_PERF_PORT_WRITE,
    LUFIA2_PERF_FALLBACK, LUFIA2_PERF_HANDOFF, LUFIA2_PERF_SPC_WAIT_BASE,
    LUFIA2_PERF_COUNTER_COUNT = LUFIA2_PERF_SPC_WAIT_BASE + 8
};
typedef struct {
    uint64_t inclusive_ticks, exclusive_ticks;
    uint32_t calls, samples;
} Lufia2PerfStat;
typedef struct Lufia2PerfScope {
    uint64_t start, children;
    struct Lufia2PerfScope *parent;
    uint32_t id, pc;
    bool active;
} Lufia2PerfScope;
typedef struct {
    uint32_t pc;
    uint64_t calls, inclusive_ticks, exclusive_ticks;
} Lufia2PerfHotspot;
typedef uint64_t (*Lufia2PerfClock)(void);
extern bool lufia2_perf_enabled;
void Lufia2PerfInit(Lufia2PerfClock clock, bool enabled);
void Lufia2PerfBeginFrame(void);
void Lufia2PerfEnter(Lufia2PerfScope *scope, uint32_t id, uint32_t pc);
void Lufia2PerfEnterSample(Lufia2PerfScope *scope, uint32_t id);
void Lufia2PerfLeave(Lufia2PerfScope *scope);
void Lufia2PerfCount(uint32_t id);
const Lufia2PerfStat *Lufia2PerfStats(void);
const uint32_t *Lufia2PerfCounters(void);
const Lufia2PerfHotspot *Lufia2PerfHotspots(uint32_t *count);
uint32_t Lufia2PerfErrors(void);
const char *Lufia2PerfName(uint32_t id);
