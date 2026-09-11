#include "lufia2_mode7_substep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The world map flies with more precision than the PPU can hold. $06:995B
 * keeps the 1/256 pixel remainder in $11EC/$11EF and publishes whole pixels
 * to M7X/M7Y, and $06:A894 reads only $1201 of the 8.8 angle in $1200, so the
 * matrix turns in steps of 1/256 turn -- about three pixels at the screen
 * edge. Those registers are integer, so the remainder can only be spent where
 * the host compiles the affine lines itself.
 *
 * The geometry lives in the platform. This file supplies what only the game
 * knows: where the remainders are in WRAM, and which reading belongs to the
 * frame being drawn. */

enum {
    CAMERA_FRAC_X = 0x11ec,
    CAMERA_FRAC_Y = 0x11ef,
    CAMERA_CENTER_X = 0x11f8, /* $06:A7AB, the only writer */
    CAMERA_CENTER_Y = 0x11fa,
    CAMERA_ANGLE = 0x1200,

    /* $06:A894 indexes the 256-entry sine table at $97:B226. */
    ANGLE_STEPS = 256u,

    RAW_M7X = 38,
    RAW_M7Y = 40,

    SAMPLE_RING = 8u,
    SAMPLE_LAG_DEFAULT = 1u,

    POSITION_WRAP = 4096u * 256u,
    ERROR_MARGIN = 128u,
    ANGLE_DELTA_MAX = 1024,

    DEBUG_EVERY = 60u,
    DEBUG_FAILURES = 40u,
    UNPAIRED_NOTICE = 16u,
    LAG_REPORTS = 4u
};

extern uint8_t g_ram[0x20000];

static Lufia2Mode7Sample s_ring[SAMPLE_RING];
static unsigned s_written;
static unsigned s_lag = SAMPLE_LAG_DEFAULT;
static uint32_t s_error[SAMPLE_RING];
static Lufia2Mode7Sample s_carry;
static bool s_have_carry;
static unsigned s_unpaired;
static unsigned s_consumed;
static unsigned s_debug_failures;
static unsigned s_reported;
static unsigned s_reported_lag = SAMPLE_RING;

static uint16_t ReadWord(unsigned wram) {
    return (uint16_t)(g_ram[wram] | ((uint16_t)g_ram[wram + 1u] << 8));
}

static uint16_t BandU16(const SnesPpuRasterBand *band, unsigned offset) {
    return (uint16_t)(band->regs[offset] |
                      ((uint16_t)band->regs[offset + 1u] << 8));
}

static int32_t BandCentre(const SnesPpuRasterBand *band, unsigned offset) {
    const uint16_t value = BandU16(band, offset) & 0x1fffu;
    return (value & 0x1000u) ? (int32_t)value - 0x2000 : (int32_t)value;
}

/* A pre-opcode hook on $06:A791 would catch every publish rather than one per
 * drawn frame, but $86:A791 runs as a compiled body, so it would never fire.
 * Reading here depends on no execution tier. */
static void TakeReading(void) {
    Lufia2Mode7Sample *sample = &s_ring[s_written % SAMPLE_RING];

    sample->center_x = ReadWord(CAMERA_CENTER_X);
    sample->center_y = ReadWord(CAMERA_CENTER_Y);
    sample->frac_x = g_ram[CAMERA_FRAC_X];
    sample->frac_y = g_ram[CAMERA_FRAC_Y];
    sample->angle = ReadWord(CAMERA_ANGLE);
    s_written++;
}

static bool DebugEnabled(void) {
    static bool s_resolved, s_enabled;
    if (!s_resolved) {
        const char *choice = getenv("LUFIA2_MODE7_SUBSTEP_DEBUG");
        s_resolved = true;
        s_enabled = choice && strcmp(choice, "0") != 0;
    }
    return s_enabled;
}

bool Lufia2Mode7SubstepEnabled(void) {
    static bool s_resolved, s_enabled = true;
    if (!s_resolved) {
        const char *choice = getenv("LUFIA2_MODE7_SUBSTEP");
        s_resolved = true;
        if (choice && (strcmp(choice, "off") == 0 ||
                       strcmp(choice, "0") == 0)) {
            s_enabled = false;
            fprintf(stderr,
                    "[video] Mode 7 sub-step camera disabled; "
                    "captured geometry is used as-is\n");
        }
    }
    return s_enabled;
}

static uint32_t WrapDistance(uint32_t a, uint32_t b, uint32_t modulus) {
    const uint32_t d = (a - b) % modulus;
    return d > modulus / 2u ? modulus - d : d;
}

/* How far a reading sits from the state the frame was drawn with, in 256ths
 * of a pixel and of a table step. */
static uint32_t ReadingError(const Lufia2Mode7Sample *reading,
                             int32_t center_x, int32_t center_y,
                             unsigned angle_index) {
    const uint32_t rx = (uint32_t)reading->center_x * 256u + reading->frac_x;
    const uint32_t ry = (uint32_t)reading->center_y * 256u + reading->frac_y;
    const uint32_t cx = (uint32_t)(center_x & 0xfff) * 256u;
    const uint32_t cy = (uint32_t)(center_y & 0xfff) * 256u;

    return WrapDistance(rx, cx, POSITION_WRAP) +
           WrapDistance(ry, cy, POSITION_WRAP) +
           WrapDistance(reading->angle, (uint32_t)angle_index << 8, 0x10000u);
}

/* Every band of one frame shares the angle and differs only in scale. */
static bool CaptureAngleIndex(const SnesPpuFrameCapture *capture,
                              unsigned *index) {
    for (unsigned i = 0; i < capture->band_count; i++) {
        if (snesrecomp_ppu_mode7_band_rotation_step(&capture->bands[i],
                                                    ANGLE_STEPS, index))
            return true;
    }
    return false;
}

bool Lufia2Mode7SubstepRefineWithSample(const SnesPpuFrameCapture *capture,
                                        const Lufia2Mode7Sample *sample,
                                        SnesRecompMode7Line *lines,
                                        unsigned line_count) {
    SnesRecompMode7Refinement refinement;
    unsigned angle_index;
    int32_t angle_delta;

    if (!capture || !sample || !lines || !capture->bands ||
        !capture->band_count)
        return false;
    if (!CaptureAngleIndex(capture, &angle_index))
        return false;

    /* The whole difference between the angle the guest held and the step its
     * matrix could express, not a remainder pinned to a foreign step. */
    angle_delta = (int32_t)(int16_t)((uint16_t)sample->angle -
                                     (uint16_t)(angle_index << 8));
    if (angle_delta > ANGLE_DELTA_MAX || angle_delta < -ANGLE_DELTA_MAX)
        return false;

    /* Likewise a whole position: a remainder belongs to the pixel it was
     * measured in, so bolting one onto the captured whole number costs a full
     * pixel every time it wraps out of step. */
    refinement.origin_x =
        ((uint32_t)sample->center_x * 256u + sample->frac_x) & 0xfffffu;
    refinement.origin_y =
        ((uint32_t)sample->center_y * 256u + sample->frac_y) & 0xfffffu;
    refinement.angle_delta = angle_delta;
    return snesrecomp_ppu_mode7_compile_lines_refined(capture, lines,
                                                      line_count, &refinement);
}

/* Which reading to spend is a question of consistency, not exactness: every
 * reading carries the same rate of change, so any fixed offset reconstructs a
 * smooth path, and only a moving one is visible. An exact match is not always
 * available either -- through part of the flyover the scene updates the
 * camera twice per displayed frame, leaving the uploaded state between two
 * readings. So score the offsets and hold the settled one. */
static const Lufia2Mode7Sample *PairSample(
    const SnesPpuFrameCapture *capture) {
    const int32_t center_x = BandCentre(&capture->bands[0], RAW_M7X);
    const int32_t center_y = BandCentre(&capture->bands[0], RAW_M7Y);
    const unsigned available =
        s_written < SAMPLE_RING ? s_written : SAMPLE_RING;
    unsigned angle_index;
    unsigned best = SAMPLE_RING;

    for (unsigned i = 1; i < capture->band_count; i++) {
        if (BandCentre(&capture->bands[i], RAW_M7X) != center_x ||
            BandCentre(&capture->bands[i], RAW_M7Y) != center_y)
            return NULL; /* Per-band centres are a different effect. */
    }
    if (!CaptureAngleIndex(capture, &angle_index))
        return NULL;

    for (unsigned lag = 0; lag < available; lag++) {
        const uint32_t error =
            ReadingError(&s_ring[(s_written - 1u - lag) % SAMPLE_RING],
                         center_x, center_y, angle_index);

        s_error[lag] = (s_error[lag] * 7u + error) / 8u;
        if (best == SAMPLE_RING || s_error[lag] < s_error[best])
            best = lag;
    }
    if (best == SAMPLE_RING)
        return NULL;
    /* Hold the settled offset unless clearly beaten, so a tie cannot
     * oscillate. */
    if (s_lag < available && s_error[s_lag] <= s_error[best] + ERROR_MARGIN)
        best = s_lag;
    s_lag = best;
    return &s_ring[(s_written - 1u - best) % SAMPLE_RING];
}

static void ReportFrame(const SnesPpuFrameCapture *capture,
                        const Lufia2Mode7Sample *sample) {
    const SnesPpuRasterBand *band = &capture->bands[0];
    unsigned step = 0;
    const bool named =
        snesrecomp_ppu_mode7_band_rotation_step(band, ANGLE_STEPS, &step);

    fprintf(stderr,
            "[m7-substep] centre %d,%d step %s%u bands=%u | "
            "wram %u,%u frac %u,%u angle $%04X | %s lag=%u err=%u\n",
            BandCentre(band, RAW_M7X), BandCentre(band, RAW_M7Y),
            named ? "" : "?", step, capture->band_count,
            (unsigned)ReadWord(CAMERA_CENTER_X),
            (unsigned)ReadWord(CAMERA_CENTER_Y),
            (unsigned)g_ram[CAMERA_FRAC_X], (unsigned)g_ram[CAMERA_FRAC_Y],
            (unsigned)ReadWord(CAMERA_ANGLE),
            sample ? "tracked" : "NO READING", s_lag,
            s_lag < SAMPLE_RING ? s_error[s_lag] : 0u);
}

bool Lufia2Mode7SubstepRefine(const SnesPpuFrameCapture *capture,
                              SnesRecompMode7Line *lines,
                              unsigned line_count) {
    const Lufia2Mode7Sample *sample;

    if (!Lufia2Mode7SubstepEnabled() || !capture || !capture->bands ||
        !capture->band_count || !lines ||
        line_count < capture->visible_height)
        return false;

    TakeReading();
    sample = PairSample(capture);
    /* Sample a tracked frame about once a second; report every frame with no
     * reading, up to a budget. */
    if (DebugEnabled() && (sample ? (s_consumed++ % DEBUG_EVERY) == 0u
                                  : s_debug_failures++ < DEBUG_FAILURES))
        ReportFrame(capture, sample);

    /* Holding the last remainder beats snapping back to the captured
     * geometry: alternating corrected and uncorrected frames is the flicker
     * this removes. */
    if (!sample) {
        if (++s_unpaired == UNPAIRED_NOTICE)
            fprintf(stderr,
                    "[video] Mode 7 sub-step camera: %u frames without a "
                    "camera reading; holding the last remainder\n",
                    (unsigned)UNPAIRED_NOTICE);
        if (!s_have_carry)
            return false;
        sample = &s_carry;
    } else {
        s_carry = *sample;
        s_have_carry = true;
    }
    if (s_lag != s_reported_lag && s_reported < LAG_REPORTS) {
        s_reported++;
        s_reported_lag = s_lag;
        fprintf(stderr,
                "[video] Mode 7 sub-step camera active, %u frame(s) of "
                "scene lead\n", s_lag);
    }
    return Lufia2Mode7SubstepRefineWithSample(capture, sample, lines,
                                              line_count);
}
