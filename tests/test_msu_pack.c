/* Pack discovery, settings and fallback. Builds throwaway pack folders under
 * argv[1] and checks what snesrecomp_msu_resolve does with them. */
#include "snesrecomp_platform/msu_pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#  define test_mkdir(p) _mkdir(p)
#else
#  include <sys/types.h>
#  include <unistd.h>
#  define test_mkdir(p) mkdir((p), 0775)
#endif

static int failures;

static void ClearEnv(void) {
#ifdef _WIN32
    _putenv_s("SNESRECOMP_MSU1", "");
#else
    unsetenv("SNESRECOMP_MSU1");
#endif
}

static const char *Env(void) {
    const char *v = getenv("SNESRECOMP_MSU1");
    return (v && v[0]) ? v : NULL;
}

static void Check(const char *name, int ok) {
    printf("%-54s %s\n", name, ok ? "ok" : "FAIL");
    if (!ok) failures++;
}

/* scratch files */

static char s_root[512];

static void Path(char *out, size_t cap, const char *leaf) {
    snprintf(out, cap, "%s/%s", s_root, leaf);
}

static void MakeDir(const char *leaf) {
    char path[640];
    Path(path, sizeof(path), leaf);
    test_mkdir(path);
}

static void WriteBytes(const char *relative, const void *data, size_t len) {
    char path[640];
    Path(path, sizeof(path), relative);
    FILE *f = fopen(path, "wb");
    if (!f) return;
    if (len) fwrite(data, 1, len, f);
    fclose(f);
}

/* MSU1 header plus one silent frame. */
static void WriteTrack(const char *relative, unsigned loop_point) {
    unsigned char pcm[12] = { 'M', 'S', 'U', '1' };
    pcm[4] = (unsigned char)(loop_point);
    pcm[5] = (unsigned char)(loop_point >> 8);
    pcm[6] = (unsigned char)(loop_point >> 16);
    pcm[7] = (unsigned char)(loop_point >> 24);
    WriteBytes(relative, pcm, sizeof(pcm));
}

static void WriteText(const char *relative, const char *text) {
    WriteBytes(relative, text, strlen(text));
}

int main(int argc, char **argv) {
    const char *base_dir = argc > 1 ? argv[1] : ".";
    char scratch[512];
    snprintf(scratch, sizeof(scratch), "%s/msu_test", base_dir);
    test_mkdir(scratch);
    snprintf(s_root, sizeof(s_root), "%s", scratch);

    /* setting */
    Check("mode: empty is auto",
          snesrecomp_msu_parse_mode("") == SNESRECOMP_MSU_AUTO);
    Check("mode: NULL is auto",
          snesrecomp_msu_parse_mode(NULL) == SNESRECOMP_MSU_AUTO);
    Check("mode: off", snesrecomp_msu_parse_mode("off") == SNESRECOMP_MSU_OFF);
    Check("mode: 0", snesrecomp_msu_parse_mode("0") == SNESRECOMP_MSU_OFF);
    Check("mode: ON uppercase",
          snesrecomp_msu_parse_mode("ON") == SNESRECOMP_MSU_ON);
    Check("mode: junk falls back to auto",
          snesrecomp_msu_parse_mode("banana") == SNESRECOMP_MSU_AUTO);
    Check("mode: names round-trip",
          strcmp(snesrecomp_msu_mode_name(SNESRECOMP_MSU_OFF), "off") == 0 &&
          strcmp(snesrecomp_msu_mode_name(SNESRECOMP_MSU_AUTO), "auto") == 0 &&
          strcmp(snesrecomp_msu_mode_name(SNESRECOMP_MSU_ON), "on") == 0);

    /* default directory */
    char dir[SNESRECOMP_MSU_PATH_MAX];
    Check("default: no anchor, no roots -> relative msu",
          snesrecomp_msu_default_directory(NULL, dir, sizeof(dir)) &&
          strcmp(dir, "msu") == 0);
    Check("default: anchor gives <exe>/msu",
          snesrecomp_msu_default_directory("C:/Games/Lufia2", dir,
                                           sizeof(dir)) &&
          strcmp(dir, "C:/Games/Lufia2/msu") == 0);
    Check("default: anchor with a trailing slash does not double it",
          snesrecomp_msu_default_directory("C:/Games/Lufia2/", dir,
                                           sizeof(dir)) &&
          strcmp(dir, "C:/Games/Lufia2/msu") == 0);

    Check("default: a console data root works the same way",
          snesrecomp_msu_default_directory("ux0:data/Lufia2Recomp", dir,
                                           sizeof(dir)) &&
          strcmp(dir, "ux0:data/Lufia2Recomp/msu") == 0);

    Check("default: refuses a buffer that cannot hold it",
          !snesrecomp_msu_default_directory("C:/Games/Lufia2", dir, 4));

    /* pack scan */
    char pack_base[SNESRECOMP_MSU_PATH_MAX];
    int tracks = -1;
    char missing[640], empty[640], noise[640], pack[640];

    Path(missing, sizeof(missing), "not_here");
    Check("scan: missing folder",
          !snesrecomp_msu_scan_pack(missing, pack_base, sizeof(pack_base),
                                    &tracks) && tracks == 0);

    MakeDir("empty");
    Path(empty, sizeof(empty), "empty");
    Check("scan: empty folder",
          !snesrecomp_msu_scan_pack(empty, pack_base, sizeof(pack_base),
                                    &tracks));

    MakeDir("noise");
    WriteText("noise/readme.txt", "not a track\n");
    WriteText("noise/track.pcm", "no index in the name");
    Path(noise, sizeof(noise), "noise");
    Check("scan: nothing shaped like <name>-<N>.pcm",
          !snesrecomp_msu_scan_pack(noise, pack_base, sizeof(pack_base),
                                    &tracks));

    MakeDir("pack");
    WriteTrack("pack/lufia2_msu-0.pcm", 0);
    WriteTrack("pack/lufia2_msu-1.pcm", 44100);
    WriteTrack("pack/lufia2_msu-12.pcm", 0);
    WriteText("pack/lufia2_msu.msu", "");
    Path(pack, sizeof(pack), "pack");
    Check("scan: finds the base and counts tracks",
          snesrecomp_msu_scan_pack(pack, pack_base, sizeof(pack_base),
                                   &tracks) &&
          tracks == 3 && strstr(pack_base, "lufia2_msu") != NULL);
    Check("scan: base carries the folder",
          strstr(pack_base, "pack") != NULL);

    /* A base may contain '-'; only the final -<digits>.pcm is stripped. */
    MakeDir("dashes");
    WriteTrack("dashes/Lufia_II_-_Rise-1.pcm", 0);
    WriteTrack("dashes/Lufia_II_-_Rise-2.pcm", 0);
    char dashes[640];
    Path(dashes, sizeof(dashes), "dashes");
    Check("scan: keeps dashes inside the base",
          snesrecomp_msu_scan_pack(dashes, pack_base, sizeof(pack_base),
                                   &tracks) &&
          tracks == 2 && strstr(pack_base, "Lufia_II_-_Rise") != NULL);

    /* Two packs in one folder: the one with more tracks wins. */
    MakeDir("mixed");
    WriteTrack("mixed/small-0.pcm", 0);
    WriteTrack("mixed/big-0.pcm", 0);
    WriteTrack("mixed/big-1.pcm", 0);
    WriteTrack("mixed/big-2.pcm", 0);
    char mixed[640];
    Path(mixed, sizeof(mixed), "mixed");
    Check("scan: the bigger pack wins",
          snesrecomp_msu_scan_pack(mixed, pack_base, sizeof(pack_base),
                                   &tracks) &&
          tracks == 3 && strstr(pack_base, "big") != NULL);

    /* settings file */
    char ini[640], stray[640];
    WriteText("config.ini",
              "# comment\n"
              "[Graphics]\n"
              "WindowScale = 3\n"
              "\n"
              "[Sound]\n"
              "EnableAudio = 1\n"
              "Msu1 = off\n"
              "Msu1Dir = D:/packs/lufia2\n");
    Path(ini, sizeof(ini), "config.ini");
    SnesRecompMsuMode mode = SNESRECOMP_MSU_AUTO;
    char configured[SNESRECOMP_MSU_PATH_MAX];
    Check("ini: reads both keys",
          snesrecomp_msu_read_settings(ini, &mode, configured,
                                       sizeof(configured)) &&
          mode == SNESRECOMP_MSU_OFF &&
          strcmp(configured, "D:/packs/lufia2") == 0);

    WriteText("stray.ini", "[Graphics]\nMsu1 = off\n");
    Path(stray, sizeof(stray), "stray.ini");
    mode = SNESRECOMP_MSU_AUTO;
    Check("ini: ignores the key in another section",
          !snesrecomp_msu_read_settings(stray, &mode, configured,
                                        sizeof(configured)) &&
          mode == SNESRECOMP_MSU_AUTO);

    mode = SNESRECOMP_MSU_ON;
    Check("ini: a missing file leaves the mode alone",
          !snesrecomp_msu_read_settings(missing, &mode, configured,
                                        sizeof(configured)) &&
          mode == SNESRECOMP_MSU_ON);

    /* resolve: what players actually get */
    SnesRecompMsuRequest req;
    const SnesRecompMsuStatus *st;

    memset(&req, 0, sizeof(req));
    req.mode = SNESRECOMP_MSU_OFF;
    req.has_mode = true;
    req.directory = pack;
    req.driver_present = true;
    ClearEnv();
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: off never arms, even with a pack sitting there",
          st->mode == SNESRECOMP_MSU_OFF && !st->armed && !st->pack_found &&
          Env() == NULL);
    Check("resolve: off still reports where it would have looked",
          strcmp(st->directory, pack) == 0);

    memset(&req, 0, sizeof(req));
    req.mode = SNESRECOMP_MSU_AUTO;
    req.has_mode = true;
    req.directory = empty;
    req.driver_present = true;
    ClearEnv();
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: auto with no pack stays on the original music",
          !st->pack_found && !st->armed && st->directory_exists &&
          Env() == NULL);

    req.directory = missing;
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: a missing folder is created, not an error",
          st->directory_exists && !st->pack_found && !st->armed);

    memset(&req, 0, sizeof(req));
    req.mode = SNESRECOMP_MSU_ON;
    req.has_mode = true;
    req.directory = pack;
    req.driver_present = true;
    ClearEnv();
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: arms the runtime with the pack base",
          st->pack_found && st->track_count == 3 && st->armed &&
          Env() && strcmp(Env(), st->pack_base) == 0);

    req.driver_present = false;
    ClearEnv();
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: a pack without a driver keeps the original music",
          st->pack_found && !st->armed && Env() == NULL &&
          st->reason && strstr(st->reason, "driver") != NULL);

    /* precedence */
    WriteText("cfg_on.ini", "[Sound]\nMsu1 = on\n");
    char cfg_on[640];
    Path(cfg_on, sizeof(cfg_on), "cfg_on.ini");
    memset(&req, 0, sizeof(req));
    req.ini_path = cfg_on;
    req.directory = pack;
    req.driver_present = true;
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: config supplies the mode when nothing overrides",
          st->mode == SNESRECOMP_MSU_ON);

    req.mode = SNESRECOMP_MSU_OFF;
    req.has_mode = true;
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: an override beats the config",
          st->mode == SNESRECOMP_MSU_OFF);

    WriteText("cfg_dir.ini",
              "[Sound]\nMsu1 = auto\nMsu1Dir = Z:/from_config\n");
    char cfg_dir[640];
    Path(cfg_dir, sizeof(cfg_dir), "cfg_dir.ini");
    memset(&req, 0, sizeof(req));
    req.ini_path = cfg_dir;
    req.driver_present = true;
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: config folder beats the default",
          strcmp(st->directory, "Z:/from_config") == 0);

    req.directory = pack;
    st = snesrecomp_msu_resolve(&req);
    Check("resolve: an explicit folder beats the config",
          strcmp(st->directory, pack) == 0);

    Check("resolve: the status stays readable afterwards",
          snesrecomp_msu_status() == st);

    printf("\n%s (%d failure%s)\n", failures ? "FAIL" : "PASS", failures,
           failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
