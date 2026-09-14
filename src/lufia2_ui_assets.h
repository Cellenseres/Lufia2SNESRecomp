#ifndef LUFIA2_UI_ASSETS_H
#define LUFIA2_UI_ASSETS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Lufia2UiPanelAsset {
    uint32_t *pixels; /* Premultiplied ARGB8888; this asset is fully opaque. */
    int width;
    int height;
    int left;
    int top;
    int right;
    int bottom;
    bool tiled;
} Lufia2UiPanelAsset;

/* The caller supplies bytes that have already passed the supported US-ROM
 * SHA-1 check. This function repeats the exact-size and recipe checks before
 * allocating a self-contained canonical 48x48 menu panel. */
bool Lufia2UiAssetsExtract(const uint8_t *headerless_rom,
                           size_t rom_size,
                           Lufia2UiPanelAsset *out);

void Lufia2UiAssetsDestroy(Lufia2UiPanelAsset *asset);

#endif
