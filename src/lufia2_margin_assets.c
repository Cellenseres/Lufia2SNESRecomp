#include "lufia2_margin_assets.h"

#include <stdio.h>
#include <string.h>

#include "host_paths.h"
#include "lufia2_log.h"
#include "snesrecomp_platform/margin_pack.h"

enum {
    LUFIA2_MARGIN_INTRO_ID = 0x00000000u,
    LUFIA2_MARGIN_BATTLE_BASE = 0x01000000u,
};

static SnesRecompMarginPack s_pack;
static bool s_attempted;
static uint32_t s_last_failed_asset = UINT32_MAX;

static bool LoadOnce(void) {
    char path[1024];
    char error[256];
    const char *leaf = "assets/widescreen/lufia2.l2mp";
    s_attempted = true;
    if (!snesrecomp_exe_dir_path(leaf, path, sizeof path))
        snprintf(path, sizeof path, "%s", leaf);
    if (!snesrecomp_margin_pack_load_file(
            path, &s_pack, error, sizeof error)) {
        fprintf(stderr, "[margins] %s\n", error);
        return false;
    }
    LUFIA2_LOG("[margins] loaded %u sparse asset(s) from '%s'\n",
               (unsigned)s_pack.asset_count, path);
    return true;
}

static uint32_t AssetId(Lufia2MarginScene scene, uint8_t scene_id) {
    return scene == LUFIA2_MARGIN_SCENE_BATTLE
        ? LUFIA2_MARGIN_BATTLE_BASE | scene_id
        : LUFIA2_MARGIN_INTRO_ID;
}

static void ReportFailure(uint32_t asset_id, const char *reason) {
    if (asset_id == s_last_failed_asset)
        return;
    s_last_failed_asset = asset_id;
    fprintf(stderr, "[margins] asset $%08X %s\n",
            (unsigned)asset_id, reason);
}

bool Lufia2MarginAssetApply(
    Lufia2MarginScene scene, uint8_t scene_id,
    uint8_t *frame, size_t width, size_t height,
    unsigned brightness) {
    if (!frame || !width || !height || width > UINT16_MAX ||
        height > UINT16_MAX)
        return false;
    if (!s_attempted && !LoadOnce())
        return false;
    if (!s_pack.data)
        return false;

    const uint32_t asset_id = AssetId(scene, scene_id);
    SnesRecompMarginAsset asset;
    if (!snesrecomp_margin_asset_find(
            &s_pack, asset_id, &asset)) {
        ReportFailure(asset_id, "is missing");
        return false;
    }
    uint32_t backdrop;
    memcpy(&backdrop, frame, sizeof backdrop);
    const SnesRecompMarginComposite destination = {
        .pixels = frame,
        .pitch = width * sizeof(uint32_t),
        .width = (uint16_t)width,
        .height = (uint16_t)height,
        .brightness = (uint8_t)(brightness > 15u ? 15u : brightness),
        .protect_non_backdrop = scene != LUFIA2_MARGIN_SCENE_BATTLE,
        .backdrop_argb = backdrop,
    };
    if (!snesrecomp_margin_asset_composite_argb8888(
            &asset, &destination)) {
        ReportFailure(asset_id, "could not be composited");
        return false;
    }
    s_last_failed_asset = UINT32_MAX;
    return true;
}

void Lufia2MarginAssetsShutdown(void) {
    snesrecomp_margin_pack_close(&s_pack);
    s_attempted = false;
    s_last_failed_asset = UINT32_MAX;
}
