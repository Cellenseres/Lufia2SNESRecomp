#ifndef LUFIA2_SOUND_CLOCK_BARRIER_H
#define LUFIA2_SOUND_CLOCK_BARRIER_H

#include <stdbool.h>
#include <stdint.h>

struct CpuState;
struct Snes;
struct Apu;

typedef struct Lufia2SoundClockBarrier {
    struct CpuState *cpu;
    struct Snes *system;
    struct Apu *apu;
    uint64_t refresh_phase;
    uint64_t published_master;
    uint64_t ceiling_master;
    uint64_t flush_threshold;
    bool relative_credit;
    bool absolute_flush;
    bool enabled;
    bool active;
    bool locked;
} Lufia2SoundClockBarrier;

#ifdef __cplusplus
extern "C" {
#endif

/* The caller retains the sound group's original hook and bus admission. */
bool Lufia2DecompSoundClockBegin(struct CpuState *cpu,
    unsigned budget, Lufia2SoundClockBarrier *clock);
void Lufia2DecompSoundClockAccount(Lufia2SoundClockBarrier *clock,
    unsigned cycles, uint64_t master, bool progress);
void Lufia2DecompSoundClockBarrier(Lufia2SoundClockBarrier *clock);
void Lufia2DecompSoundClockEnd(Lufia2SoundClockBarrier *clock);

#ifdef __cplusplus
}
#endif

#endif
