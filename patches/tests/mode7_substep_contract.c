#include "lufia2_mode7_substep.h"

#include <stdio.h>
#include <string.h>

/* The geometry is the platform's and is covered by its own Mode 7 smoke test.
 * What is left here is the composition this game performs: a WRAM reading
 * carries whole values, and both of them have to reach the refinement whole.
 * Pinning a remainder to the captured whole number instead costs a full pixel
 * or a full table step at every wrap, which is what rubber-banded the flyover
 * wherever the reading did not sit in the captured pixel. */

enum {
    TEST_HEIGHT = 224u,
    TEST_CENTER_X = 0x0642u,
    TEST_CENTER_Y = 0x0317u,
    TEST_INDEX = 91u,
    TEST_SCALE = 448
};

static uint16_t s_vram[SNES_PPU_VRAM_WORDS];
static uint16_t s_cgram[SNES_PPU_CGRAM_ENTRIES];
static SnesRecompMode7Line s_lines[TEST_HEIGHT];
static int s_failures;

static void Put16(uint8_t *p, unsigned offset, int value) {
    p[offset] = (uint8_t)(unsigned)value;
    p[offset + 1u] = (uint8_t)((unsigned)value >> 8);
}

static int Expect(int condition, const char *name) {
    if (!condition) {
        s_failures++;
        fprintf(stderr, "mode7-substep case failed: %s\n", name);
    }
    return condition;
}

static void UnitCosSin(unsigned index, int *cos_out, int *sin_out) {
    const double turn = 6.283185307179586476925286766559;
    const double a = (double)(index & 0xffu) * (turn / 256.0);
    double c = 1.0, s = a, tc = 1.0, ts = a;
    unsigned n;

    for (n = 1; n <= 12u; n++) {
        tc *= -a * a / (double)((2u * n - 1u) * (2u * n));
        c += tc;
        ts *= -a * a / (double)((2u * n) * (2u * n + 1u));
        s += ts;
    }
    *cos_out = (int)(c * 256.0 + (c < 0.0 ? -0.5 : 0.5));
    *sin_out = (int)(s * 256.0 + (s < 0.0 ? -0.5 : 0.5));
}

/* One band shaped like the flyover: $06:A9B8 writes M7A and M7D as k*cos,
 * M7B as k*sin and M7C as its negation, and $06:A7A5 puts the camera in
 * M7X/M7Y with the BG1 scroll trailing it by the screen centre. */
static SnesPpuFrameCapture MakeCapture(SnesPpuRasterBand *band) {
    SnesPpuFrameCapture cap;
    int table_cos, table_sin;

    memset(&cap, 0, sizeof cap);
    memset(band, 0, sizeof *band);
    UnitCosSin(TEST_INDEX, &table_cos, &table_sin);

    band->y_end = TEST_HEIGHT;
    band->bg_mode = 7;
    band->main_enable = 1;
    band->brightness = 15;
    band->regs[0] = 15;
    band->regs[4] = 7;
    band->regs[58] = 1;
    Put16(band->regs, 30, table_cos * TEST_SCALE / 256);
    Put16(band->regs, 32, table_sin * TEST_SCALE / 256);
    Put16(band->regs, 34, -table_sin * TEST_SCALE / 256);
    Put16(band->regs, 36, table_cos * TEST_SCALE / 256);
    Put16(band->regs, 38, TEST_CENTER_X);
    Put16(band->regs, 40, TEST_CENTER_Y);
    Put16(band->regs, 42, TEST_CENTER_X - 0x80);
    Put16(band->regs, 44, TEST_CENTER_Y - 0x70);

    cap.vram = s_vram;
    cap.cgram = s_cgram;
    cap.bands = band;
    cap.band_count = 1;
    cap.native_width = 256;
    cap.canvas_width = 342;
    cap.canvas_extra = 43;
    cap.visible_height = TEST_HEIGHT;
    return cap;
}

/* Signed difference inside the 18-bit Mode 7 plane. */
static int32_t Delta(uint32_t a, uint32_t b) {
    int32_t d = (int32_t)((a - b) & SNESRECOMP_MODE7_COORD_MASK);
    return (d & 0x20000) ? d - 0x40000 : d;
}

/* Walking the reading across pixel boundaries has to move the picture by
 * exactly the distance walked, while the captured centre stays put. */
static void ReadingCarriesWholePosition(void) {
    SnesPpuRasterBand band;
    SnesPpuFrameCapture cap = MakeCapture(&band);
    int32_t previous = 0;
    int wrong = 0;
    unsigned tick;

    for (tick = 0; tick <= 1024u; tick++) {
        Lufia2Mode7Sample walk;

        walk.center_x = (uint16_t)(TEST_CENTER_X + (tick >> 8));
        walk.frac_x = (uint8_t)(tick & 0xffu);
        walk.center_y = TEST_CENTER_Y;
        walk.frac_y = 0;
        walk.angle = (uint16_t)(TEST_INDEX << 8);
        if (!Expect(Lufia2Mode7SubstepRefineWithSample(&cap, &walk, s_lines,
                                                       TEST_HEIGHT),
                    "the walk is refined"))
            return;
        if (tick && Delta(s_lines[0].start_x, (uint32_t)previous) != 1)
            wrong++;
        previous = (int32_t)s_lines[0].start_x;
    }
    Expect(wrong == 0, "no pixel wrap injects a spurious pixel");
}

/* And the angle: a reading may lead the matrix by more than one table step,
 * so the whole difference has to reach the refinement. */
static void ReadingCarriesWholeAngle(void) {
    SnesPpuRasterBand band;
    SnesPpuFrameCapture cap = MakeCapture(&band);
    int32_t previous = 0;
    int worst = 0;
    unsigned tick;

    for (tick = 0; tick <= 600u; tick++) {
        Lufia2Mode7Sample lead;

        lead.center_x = TEST_CENTER_X;
        lead.center_y = TEST_CENTER_Y;
        lead.frac_x = 0;
        lead.frac_y = 0;
        lead.angle = (uint16_t)((TEST_INDEX << 8) + tick);
        if (!Expect(Lufia2Mode7SubstepRefineWithSample(&cap, &lead, s_lines,
                                                       TEST_HEIGHT),
                    "the angle lead is refined"))
            return;
        if (tick) {
            const int32_t d = Delta(s_lines[0].start_x, (uint32_t)previous);
            const int32_t m = d < 0 ? -d : d;
            if (m > worst)
                worst = m;
        }
        previous = (int32_t)s_lines[0].start_x;
    }
    Expect(worst <= 16, "no angle wrap snaps back a table step");
}

/* A lead wider than the flyover ever produces means the reading and the
 * capture describe different scenes. */
static void ImplausibleLeadIsRefused(void) {
    SnesPpuRasterBand band;
    SnesPpuFrameCapture cap = MakeCapture(&band);
    Lufia2Mode7Sample far;

    far.center_x = TEST_CENTER_X;
    far.center_y = TEST_CENTER_Y;
    far.frac_x = 0;
    far.frac_y = 0;
    far.angle = (uint16_t)((TEST_INDEX << 8) + 4096u);
    Expect(!Lufia2Mode7SubstepRefineWithSample(&cap, &far, s_lines,
                                               TEST_HEIGHT),
           "an implausible angle lead is refused");
}

int Lufia2Mode7SubstepSelfTest(void) {
    s_failures = 0;
    ReadingCarriesWholePosition();
    ReadingCarriesWholeAngle();
    ImplausibleLeadIsRefused();
    if (s_failures) {
        fprintf(stderr, "mode7-substep selftest FAILED (%d)\n", s_failures);
        return 1;
    }
    printf("mode7-substep selftest PASS\n");
    return 0;
}
