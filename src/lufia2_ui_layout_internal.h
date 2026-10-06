#ifndef LUFIA2_UI_LAYOUT_INTERNAL_H
#define LUFIA2_UI_LAYOUT_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

enum {
    LUFIA2_UI_NATIVE_WIDTH = 256,
    LUFIA2_UI_CANVAS_WIDTH = 342,
    LUFIA2_UI_HEIGHT = 224,
    LUFIA2_UI_LAYOUT_HEADER_BYTES = 24,
};

/* Both layout formats use little-endian records and CRC32 of their payload. */
static inline uint16_t Lufia2UiRead16(const uint8_t *data) {
    return data[0] | (uint16_t)data[1] << 8;
}

static inline uint32_t Lufia2UiRead32(const uint8_t *data) {
    return Lufia2UiRead16(data) | (uint32_t)Lufia2UiRead16(data + 2) << 16;
}

static inline void Lufia2UiWrite32(uint8_t *data, uint32_t value) {
    for (unsigned byte = 0; byte < 4; ++byte)
        data[byte] = (uint8_t)(value >> (byte * 8));
}

static inline uint32_t Lufia2UiChecksum(const uint8_t *data, size_t size) {
    uint32_t crc = UINT32_MAX;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

#endif
