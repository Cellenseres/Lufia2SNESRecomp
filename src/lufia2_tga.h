#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Caller owns *out_pixels and frees it. */
bool Lufia2LoadTgaArgb(const char *path, uint32_t **out_pixels,
                       int *out_width, int *out_height);
