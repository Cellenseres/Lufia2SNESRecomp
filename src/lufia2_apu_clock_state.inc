#include "lufia2_apu_clock_state.h"

enum {
    LUFIA2_APU_CLOCK_MAGIC = 0x4c324143u,
    LUFIA2_APU_CLOCK_VERSION = 1u
};

typedef struct Lufia2ApuClockState {
    uint32_t magic;
    uint32_t version;
    uint32_t extended;
    uint32_t reserved;
    RtlApuFrameClock clock;
} Lufia2ApuClockState;

_Static_assert(sizeof(Lufia2ApuClockState) == 48u, "APU clock state layout changed");

static RtlApuFrameClock s_loaded_apu_clock;
static bool s_loaded_apu_clock_present;
static bool s_loaded_apu_timing_present;
static bool s_loaded_extended_frame_timing;

void Lufia2ApuClockSaveState(SaveLoadInfo *sli) {
    Lufia2ApuClockState state = {
        LUFIA2_APU_CLOCK_MAGIC, LUFIA2_APU_CLOCK_VERSION,
        g_extended_frame_timing, 0u, g_apu_frame_clock
    };
    sli->func(sli, &state, sizeof(state));
}

bool Lufia2ApuClockLoadState(SaveLoadInfo *sli) {
    uint32_t magic;
    Lufia2ApuClockState state;
    s_loaded_apu_clock_present = false;
    s_loaded_apu_timing_present = false;
    if (!sli->peek)
        return false;
    if (!sli->peek(sli, 0, &magic, sizeof(magic)) || magic != LUFIA2_APU_CLOCK_MAGIC) {
        /* Legacy desktop states used the fixed frame clock. */
        s_loaded_extended_frame_timing = false;
        s_loaded_apu_timing_present = true;
        return true;
    }
    if (!sli->peek(sli, 0, &state, sizeof(state)) ||
        state.version != LUFIA2_APU_CLOCK_VERSION || state.reserved ||
        state.extended > 1u)
        return false;
    sli->func(sli, &state, sizeof(state));
    s_loaded_apu_clock = state.clock;
    s_loaded_apu_clock_present = true;
    s_loaded_extended_frame_timing = state.extended != 0u;
    s_loaded_apu_timing_present = true;
    return true;
}

void Lufia2ApuClockApplyLoadedState(void) {
    if (s_loaded_apu_timing_present)
        g_extended_frame_timing = s_loaded_extended_frame_timing;
    if (s_loaded_apu_clock_present)
        g_apu_frame_clock = s_loaded_apu_clock;
    s_loaded_apu_clock_present = false;
    s_loaded_apu_timing_present = false;
}
