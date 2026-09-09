#ifndef LUFIA2_NATIVE_PATCHES_H
#define LUFIA2_NATIVE_PATCHES_H
#include "actor_early_return.h"
#include "actor_d508_early_return.h"
#include "frame_wait.h"

extern int lufia2_native_wait_enabled;
extern int lufia2_frame_wait_fastforward_enabled;
extern int lufia2_actor_early_return_enabled;
extern int lufia2_actor_d508_early_return_enabled;
extern uint64_t lufia2_native_wait_steps[2];
extern uint64_t lufia2_actor_early_return_hits;
extern uint64_t lufia2_actor_d508_early_return_hits;
uint64_t lufia2_frame_wait_ff_pairs_count(void);
uint64_t lufia2_frame_wait_ff_site_pairs_count(unsigned site);
extern const uint8_t *g_rom;
void Lufia2NativePatchesInit(void);
void Lufia2NativeWaitBegin(void);
void Lufia2NativeWaitEnd(void);
int Lufia2NativeWaitSelfTest(void);
int Lufia2FrameWaitFastForwardSelfTest(void);
int Lufia2ActorEarlyReturnSelfTest(void);
int Lufia2ActorD508EarlyReturnSelfTest(void);
int Lufia2NativeObserversActive(void);
#ifdef LUFIA2_ENABLE_DMA_HOST_FASTFORWARD
uint64_t Lufia2DmaHostFastForwardIdleTicks(void);
int Lufia2DmaHostFastForwardSelfTest(void);
#endif

/* Automatic A/B for whichever experiment is set to `auto`. A host that already
 * times whole frames ends each measurement window here and the arm flips for
 * the next one. Diagnostics only; neither call touches guest state. */
void Lufia2NativePatchesWindowEnd(unsigned frames, uint64_t elapsed_us,
                                  uint64_t scheduler_us);
void Lufia2NativePatchesSummary(void);

/* Return zero without touching the interpreter or guest bus on fallback.
 * The supported, hash-verified USA ROM is immutable in this project. Check
 * the four local bytes too, so a future ROM edit cannot silently reuse this
 * payload. No new hooks are registered and no existing hooks are replaced. */
static inline unsigned Lufia2NativeWaitTryStep(Interp816 *in, bool outer_scheduler) {
#if defined(SNES_COSIM) || (defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    (void)in; (void)outer_scheduler;
    return 0;
#else
    if (!lufia2_native_wait_enabled || !L2FrameWaitEligible(in, outer_scheduler) || !g_rom)
        return 0;
    const uint8_t *bytes = g_rom + 0x1900e;
    if (bytes[0] != 0xc5 || bytes[1] != 0x40 || bytes[2] != 0xf0 || bytes[3] != 0xfc)
        return 0;
    unsigned which = in->pc == 0x900e ? 0 : 1;
    unsigned cycles = L2FrameWaitStep(in);
    ++lufia2_native_wait_steps[which];
    return cycles;
#endif
}
#endif
