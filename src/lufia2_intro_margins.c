#include "lufia2_intro_margins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host_paths.h"
#include "lufia2_log.h"
#include "lufia2_tga.h"

static uint32_t *s_pixels;
static int s_width;
static int s_height;
static bool s_attempted;

static void LoadOnce(size_t width, size_t height) {
    char path[1024];
    const char *leaf = "assets/img/lufia2_intro_margins.tga";

    s_attempted = true;
    if (!snesrecomp_exe_dir_path(leaf, path, sizeof path))
        snprintf(path, sizeof path, "%s", leaf);
    if (!Lufia2LoadTgaArgb(path, &s_pixels, &s_width, &s_height)) {
        fprintf(stderr, "[intro-margins] no readable '%s'\n", path);
        return;
    }

    if ((size_t)s_width != width || (size_t)s_height != height) {
        fprintf(stderr,
            "[intro-margins] '%s' is %dx%d; the wide frame is %zux%zu\n",
            path, s_width, s_height, width, height);
        free(s_pixels);
        s_pixels = NULL;
        return;
    }
    LUFIA2_LOG("[intro-margins] using '%s'\n", path);
}

static uint32_t LoadPixel(const uint8_t *address) {
    uint32_t pixel;
    memcpy(&pixel, address, sizeof pixel);
    return pixel;
}

static void StorePixel(uint8_t *address, uint32_t pixel) {
    memcpy(address, &pixel, sizeof pixel);
}

/* Premultiplied, so a plain source-over. */
static uint32_t Over(uint32_t source, uint32_t destination) {
    const uint32_t alpha = source >> 24;
    if (alpha == 0xffu)
        return source;
    if (alpha == 0u)
        return destination;
    const uint32_t inverse = 255u - alpha;
    uint32_t out = 0xff000000u;
    for (unsigned shift = 0; shift < 24u; shift += 8u) {
        const uint32_t s = (source >> shift) & 0xffu;
        const uint32_t d = (destination >> shift) & 0xffu;
        out |= (((s + (d * inverse + 127u) / 255u) & 0xffu) << shift);
    }
    return out;
}

/* Follow the guest's fade, or the art pops in. */
static uint32_t Dim(uint32_t pixel, unsigned brightness) {
    if (brightness >= 15u)
        return pixel;
    uint32_t out = pixel & 0xff000000u;
    for (unsigned shift = 0; shift < 24u; shift += 8u) {
        const uint32_t c = (pixel >> shift) & 0xffu;
        out |= ((c * brightness / 15u) & 0xffu) << shift;
    }
    return out;
}

void Lufia2IntroMarginsApply(
    uint8_t *frame, size_t width, size_t height, size_t margin_width,
    unsigned brightness) {
    if (!frame || !margin_width || width <= margin_width * 2u || !height)
        return;
    if (!brightness)
        return;
    if (!s_attempted)
        LoadOnce(width, height);
    if (!s_pixels)
        return;

    const size_t pitch = width * sizeof(uint32_t);
    /* The top-left margin pixel is backdrop. */
    const uint32_t backdrop = LoadPixel(frame);

    for (size_t y = 0; y < height; y++) {
        for (size_t i = 0; i < margin_width; i++) {
            const size_t columns[2] = {i, width - 1u - i};
            for (unsigned side = 0; side < 2u; side++) {
                const size_t x = columns[side];
                uint8_t *const at = frame + y * pitch + x * sizeof(uint32_t);
                const uint32_t there = LoadPixel(at);
                /* Backdrop or cleared: the PPU drew nothing here. */
                if (there != backdrop && (there & 0x00ffffffu) != 0u)
                    continue;
                StorePixel(at, Over(
                    Dim(s_pixels[y * (size_t)s_width + x], brightness),
                    there));
            }
        }
    }
}

