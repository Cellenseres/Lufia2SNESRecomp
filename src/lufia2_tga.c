/* Uncompressed and RLE 24/32-bit TGA into premultiplied ARGB8888. */

#include "lufia2_tga.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct TgaReader {
    FILE *file;
    int bytes_per_pixel;
    bool run_length_encoded;
    unsigned run_left;      /* pixels still to repeat from run_pixel */
    unsigned raw_left;      /* pixels still to read one by one */
    uint8_t run_pixel[4];
} TgaReader;

static uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static bool ReadRaw(TgaReader *reader, uint8_t *bgra) {
    bgra[3] = 255u;
    return fread(bgra, 1, (size_t)reader->bytes_per_pixel, reader->file) ==
           (size_t)reader->bytes_per_pixel;
}

static bool NextPixel(TgaReader *reader, uint8_t *bgra) {
    if (!reader->run_length_encoded)
        return ReadRaw(reader, bgra);

    if (reader->run_left) {
        reader->run_left--;
        for (int i = 0; i < 4; i++)
            bgra[i] = reader->run_pixel[i];
        return true;
    }
    if (reader->raw_left) {
        reader->raw_left--;
        return ReadRaw(reader, bgra);
    }

    int packet = fgetc(reader->file);
    if (packet == EOF)
        return false;
    /* Bit 7 marks a run, otherwise literal pixels follow. */
    if (packet & 0x80) {
        if (!ReadRaw(reader, reader->run_pixel))
            return false;
        reader->run_left = (unsigned)(packet & 0x7f);
        for (int i = 0; i < 4; i++)
            bgra[i] = reader->run_pixel[i];
        return true;
    }
    reader->raw_left = (unsigned)packet;
    return ReadRaw(reader, bgra);
}

bool Lufia2LoadTgaArgb(const char *path, uint32_t **out_pixels,
                       int *out_width, int *out_height) {
    uint8_t header[18];
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    bool ok = fread(header, 1, sizeof(header), file) == sizeof(header);
    int width = ok ? (int)read_u16_le(header + 12) : 0;
    int height = ok ? (int)read_u16_le(header + 14) : 0;
    int depth = ok ? header[16] : 0;
    const bool rle = ok && header[2] == 10;
    ok = ok && header[1] == 0 && (header[2] == 2 || header[2] == 10) &&
         (depth == 24 || depth == 32) && width > 0 && height > 0 &&
         width <= 4096 && height <= 4096 &&
         (size_t)width <= SIZE_MAX / (size_t)height;
    size_t count = ok ? (size_t)width * height : 0;
    ok = ok && count <= SIZE_MAX / sizeof(uint32_t) &&
         fseek(file, header[0], SEEK_CUR) == 0;
    uint32_t *pixels = ok ? (uint32_t *)malloc(count * sizeof(*pixels)) : NULL;
    ok = ok && pixels != NULL;
    bool top_origin = (header[17] & 0x20u) != 0;
    bool right_origin = (header[17] & 0x10u) != 0;
    TgaReader reader = {file, depth / 8, rle, 0, 0, {0, 0, 0, 255}};

    for (int file_y = 0; ok && file_y < height; file_y++) {
        for (int file_x = 0; file_x < width; file_x++) {
            uint8_t bgra[4] = {0, 0, 0, 255};
            if (!NextPixel(&reader, bgra)) {
                ok = false;
                break;
            }
            uint32_t a = bgra[3];
            uint32_t r = ((uint32_t)bgra[2] * a + 127u) / 255u;
            uint32_t g = ((uint32_t)bgra[1] * a + 127u) / 255u;
            uint32_t b = ((uint32_t)bgra[0] * a + 127u) / 255u;
            int x = right_origin ? width - 1 - file_x : file_x;
            int y = top_origin ? file_y : height - 1 - file_y;
            pixels[(size_t)y * width + x] =
                (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    fclose(file);
    if (!ok) {
        free(pixels);
        return false;
    }
    *out_pixels = pixels;
    *out_width = width;
    *out_height = height;
    return true;
}
