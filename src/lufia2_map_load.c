#include "lufia2_map_load.h"

#include <stdio.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"

enum {
    /* $83:B53B installs a regular map: it reads the map id from $05AC,
       calls the only map resource loader at $80:EAE7 and returns through
       $83:B580. An MVN keeps the routine on the interpreter tier, so these
       two sites are observable without replacing any guest code. */
    LUFIA2_MAP_LOAD_CALL = 0x83b548,
    LUFIA2_MAP_LOAD_RETURN = 0x83b580,

    LUFIA2_CURRENT_MAP = 0x05ac,

    /* $05AC is written before the load runs, so a mismatch normally means a
       load is imminent. A field that never reaches the loader belongs to a
       path these hooks do not cover; stop claiming a load rather than
       waiting for a commit that will not arrive. */
    LUFIA2_MAX_PENDING_FRAMES = 120,
};

extern uint8_t g_ram[0x20000];

static bool s_loading;
static bool s_committed;
static uint8_t s_pending_map;
static uint8_t s_committed_map;
static uint32_t s_generation;
static uint8_t s_pending_field;
static unsigned s_pending_frames;

static bool MapFieldIsPending(void) {
    /* $00 is not a regular map. The loader would resolve it to resource
       $3F, one below the first map, and the overworld renders from Mode 7
       instead of bank $7F - so it never reaches the loader and must not be
       reported as a load that is still on its way. */
    const uint8_t field = g_ram[LUFIA2_CURRENT_MAP];
    return s_committed && field != 0 && field != s_committed_map;
}

/* Entered with M=1 and the runtime map id in A. */
static void MapLoadBegin(CpuState *cpu, uint32_t pc24) {
    (void)pc24;
    const uint8_t map_id = (uint8_t)(cpu->A & 0xff);
    if (!s_loading || map_id != s_pending_map) {
        fprintf(stderr, "[map-load] begin runtime map $%02X\n",
            (unsigned)map_id);
    }
    s_loading = true;
    s_pending_map = map_id;
}

/* The same return carries the already-loaded skip path, so only an
   outstanding load commits. */
static void MapLoadCommit(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    (void)pc24;
    if (!s_loading)
        return;

    s_loading = false;
    s_committed = true;
    s_committed_map = s_pending_map;
    s_pending_frames = 0;
    s_generation++;
    fprintf(stderr, "[map-load] committed runtime map $%02X\n",
        (unsigned)s_committed_map);
}

void Lufia2MapLoadInstallHooks(void) {
    interp_bridge_set_pre_opcode_hook(LUFIA2_MAP_LOAD_CALL, MapLoadBegin);
    interp_bridge_set_pre_opcode_hook(LUFIA2_MAP_LOAD_RETURN, MapLoadCommit);
}

void Lufia2MapLoadFrame(void) {
    const uint8_t field = g_ram[LUFIA2_CURRENT_MAP];
    if (s_loading || !MapFieldIsPending()) {
        s_pending_field = field;
        s_pending_frames = 0;
        return;
    }

    /* Each selected map gets its own budget. Without this a map that
       never reaches the loader leaves the counter exhausted and the next
       one is never reported as loading. */
    if (field != s_pending_field) {
        s_pending_field = field;
        s_pending_frames = 0;
    }
    if (s_pending_frames > LUFIA2_MAX_PENDING_FRAMES)
        return;
    if (++s_pending_frames > LUFIA2_MAX_PENDING_FRAMES) {
        fprintf(stderr,
            "[map-load] runtime map $%02X never reached the loader; "
            "using structural map validation\n",
            (unsigned)field);
    }
}

bool Lufia2MapLoadInProgress(void) {
    return s_loading ||
        (MapFieldIsPending() && s_pending_frames <= LUFIA2_MAX_PENDING_FRAMES);
}

uint32_t Lufia2MapLoadGeneration(void) {
    return s_generation;
}
