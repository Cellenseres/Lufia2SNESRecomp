#include "lufia2_build_identity.h"

#include <stdio.h>
#include <string.h>

#include "lufia2_build_identity_values.h"
#include "snes/tier2_capture.h"

/* Sidecar next to the tier2 manifest: <manifest>.build.json. */
static int SidecarPath(char *out, size_t size) {
    const char *manifest = tier2_capture_manifest_path("lufia2");
    size_t length = strlen(manifest);

    if (length >= 5 && strcmp(manifest + length - 5, ".json") == 0)
        length -= 5;
    if (length + sizeof ".build.json" > size)
        return 0;
    memcpy(out, manifest, length);
    memcpy(out + length, ".build.json", sizeof ".build.json");
    return 1;
}

void Lufia2ReportBuildIdentity(void) {
    char path[600];
    FILE *file;

    fprintf(stderr,
        "[build] lufia2 %s decomp %s %s %s profile=%d manifest %s\n",
        LUFIA2_BUILD_SUPERPROJECT, LUFIA2_BUILD_DECOMP,
        LUFIA2_BUILD_SNESRECOMP, LUFIA2_BUILD_CONFIG,
        LUFIA2_BUILD_INTERP_PROFILE, LUFIA2_BUILD_MANIFEST_SHA256);
    if (!tier2_capture_enabled() || !SidecarPath(path, sizeof path))
        return;
    file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "[build] cannot write %s\n", path);
        return;
    }
    fprintf(file,
        "{\n"
        "  \"schema\": \"lufia2 build identity v1\",\n"
        "  \"superproject\": \"%s\",\n"
        "  \"decomp\": \"%s\",\n"
        "  \"snesrecomp\": \"%s\",\n"
        "  \"config\": \"%s\",\n"
        "  \"interp_profile\": %d,\n"
        "  \"program_manifest_sha256\": \"%s\"\n"
        "}\n",
        LUFIA2_BUILD_SUPERPROJECT, LUFIA2_BUILD_DECOMP,
        LUFIA2_BUILD_SNESRECOMP, LUFIA2_BUILD_CONFIG,
        LUFIA2_BUILD_INTERP_PROFILE, LUFIA2_BUILD_MANIFEST_SHA256);
    fclose(file);
}
