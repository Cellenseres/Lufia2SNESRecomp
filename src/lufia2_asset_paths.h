#pragma once
#include "host_paths.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* A console may keep optional assets beside its external ROM/config instead
 * of the read-only application. Desktop retains its executable/cwd policy. */
static inline bool Lufia2AssetPath(const char *leaf, char *path, size_t capacity) {
    if (!leaf || !*leaf || !path || !capacity) return false;
    const char *root = getenv("LUFIA2_ASSET_ROOT");
    int count;
    if (root && *root) count = snprintf(path, capacity, "%s/%s", root, leaf);
    else {
        if (snesrecomp_exe_dir_path(leaf, path, capacity)) return true;
        count = snprintf(path, capacity, "%s", leaf);
    }
    return count >= 0 && (size_t)count < capacity;
}
