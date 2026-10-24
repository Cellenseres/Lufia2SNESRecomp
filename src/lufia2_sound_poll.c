#include "lufia2_sound_poll.h"
#include <stdlib.h>
#include "common_rtl.h"

static bool s_single_threaded;
bool lufia2_sound_poll_kernel_enabled;
uint64_t lufia2_sound_poll_kernel_steps;
bool lufia2_frame_poll_kernel_enabled;
uint64_t lufia2_frame_poll_kernel_steps;

void Lufia2HostSoundPollConfigure(bool single_threaded) {
    /* Reference mode is for local differential replay, never automatic. */
    s_single_threaded = single_threaded &&
        getenv("LUFIA2_SOUND_POLL_REFERENCE") == NULL;
    lufia2_sound_poll_kernel_enabled = s_single_threaded &&
        getenv("LUFIA2_SOUND_KERNEL_REFERENCE") == NULL;
    lufia2_frame_poll_kernel_enabled = s_single_threaded &&
        getenv("LUFIA2_FRAME_KERNEL_REFERENCE") == NULL;
}

uint32_t Lufia2HostSoundPollLimit(uint32_t original_limit) {
    /* On the frame timeline, raw semantic bus accesses do not advance
     * g_cpu.master_cycles. A synchronized port cannot change again until
     * the existing interpreter continuation advances guest time. Preserve
     * the first read and its flags, then use that same continuation. Reset
     * and IPL still need the original touch-paced, pre-frame handshake. */
    return s_single_threaded && rtl_apu_frame_timeline_active()
        ? 1u : original_limit;
}
