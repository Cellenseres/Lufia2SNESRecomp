#ifdef LUFIA2_ENABLE_QUIESCENCE_INDEX
#include "quiescence_index.h"
int lufia2_quiescence_index_enabled;
#endif
#ifdef LUFIA2_ENABLE_BRIDGE_AUDIT
#include "src/diagnostics/lufia2_bridge_audit.h"
#endif
#include "native_patches.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(LUFIA2_ENABLE_NATIVE_WAIT) || defined(LUFIA2_ENABLE_QUIESCENCE_INDEX)
#define L2_ENABLE_EXPERIMENT_AB 1
#endif

/* Runtime defaults for hosts that cannot pass an environment variable. The
 * Vita has no launcher script, so the compiled default is the only switch
 * there. 0 = reference path, 1 = experiment on, 2 = alternate automatically. */
#ifndef LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE
#define LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE 2
#endif
#ifndef LUFIA2_NATIVE_WAIT_DEFAULT_MODE
#ifdef LUFIA2_ENABLE_QUIESCENCE_INDEX
/* An index build pins the payload off, so the two never overlap. */
#define LUFIA2_NATIVE_WAIT_DEFAULT_MODE 0
#else
#define LUFIA2_NATIVE_WAIT_DEFAULT_MODE 2
#endif
#endif

int lufia2_native_wait_enabled;
int lufia2_frame_wait_fastforward_enabled;
int lufia2_actor_early_return_enabled;
int lufia2_actor_d508_early_return_enabled;
uint64_t lufia2_native_wait_steps[2];
uint64_t lufia2_actor_early_return_hits;
uint64_t lufia2_actor_d508_early_return_hits;
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
static uint64_t s_begin, s_sum, s_min, s_max;
#endif
#if defined(LUFIA2_ENABLE_NATIVE_WAIT) && defined(LUFIA2_ENABLE_PATCH_REPORTING)
static uint64_t s_previous[2];
#endif
#if defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) &&     defined(LUFIA2_ENABLE_PATCH_REPORTING)
static uint64_t s_previous_frame_wait_pairs[5];
#ifdef LUFIA2_ENABLE_DMA_HOST_FASTFORWARD
static uint64_t s_previous_dma_idle_ticks;
#endif
#endif
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
static unsigned s_frames;
#endif
#if defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN) &&     defined(LUFIA2_ENABLE_PATCH_REPORTING)
static uint64_t s_previous_actor_early_return_hits;
static uint64_t s_previous_actor_d508_early_return_hits;
#endif

#ifdef L2_ENABLE_EXPERIMENT_AB
/* Automatic on-device A/B for whichever experiment is set to `auto`. Both
 * arms then see the same scene under the same host load in one session,
 * which two separate manually played runs cannot give us. Neither experiment
 * changes guest state, so alternating them cannot drift the two arms apart:
 * the payload retires the same instruction the interpreter would, and the
 * index only preselects ring candidates for the unchanged equality test. */
enum { L2_AB_OFF, L2_AB_NATIVE_WAIT, L2_AB_QUIESCENCE_INDEX };
enum { L2_AB_WARMUP_WINDOWS = 2 };
typedef struct { uint64_t frames, elapsed_us, scheduler_us, windows; } L2AbTotals;
static int s_ab_target;
static int s_ab_host_windows;      /* a host report owns the window ends */
static unsigned s_ab_window;
static L2AbTotals s_ab_totals[2];  /* [0] reference arm, [1] experiment arm */

static const char *L2AbTag(void) {
    return s_ab_target == L2_AB_NATIVE_WAIT ? "native-wait-ab" : "quiescence-index-ab";
}
static const char *L2AbArm(int mode) {
    if (s_ab_target == L2_AB_NATIVE_WAIT) return mode ? "native" : "original";
    return mode ? "indexed" : "original";
}
static int L2AbMode(void) {
    if (s_ab_target == L2_AB_NATIVE_WAIT) return lufia2_native_wait_enabled != 0;
#ifdef LUFIA2_ENABLE_QUIESCENCE_INDEX
    if (s_ab_target == L2_AB_QUIESCENCE_INDEX) return lufia2_quiescence_index_enabled != 0;
#endif
    return 0;
}
static void L2AbSetMode(int on) {
    if (s_ab_target == L2_AB_NATIVE_WAIT) lufia2_native_wait_enabled = on;
#ifdef LUFIA2_ENABLE_QUIESCENCE_INDEX
    else if (s_ab_target == L2_AB_QUIESCENCE_INDEX) lufia2_quiescence_index_enabled = on;
#endif
}

void Lufia2NativePatchesSummary(void) {
    const L2AbTotals *reference = &s_ab_totals[0];
    const L2AbTotals *experiment = &s_ab_totals[1];
    if (!s_ab_target || !reference->frames || !experiment->frames) return;
    const double reference_ms =
        (double)reference->scheduler_us / 1000.0 / (double)reference->frames;
    const double experiment_ms =
        (double)experiment->scheduler_us / 1000.0 / (double)experiment->frames;
    fprintf(stderr,
        "[%s] summary windows=%llu/%llu frames=%llu/%llu "
        "scheduler_ms %s=%.4f %s=%.4f delta=%+.2f%%",
        L2AbTag(),
        (unsigned long long)reference->windows, (unsigned long long)experiment->windows,
        (unsigned long long)reference->frames, (unsigned long long)experiment->frames,
        L2AbArm(0), reference_ms, L2AbArm(1), experiment_ms,
        reference_ms > 0.0 ? (experiment_ms - reference_ms) * 100.0 / reference_ms : 0.0);
    if (reference->elapsed_us && experiment->elapsed_us) {
        const double reference_fps =
            (double)reference->frames * 1000000.0 / (double)reference->elapsed_us;
        const double experiment_fps =
            (double)experiment->frames * 1000000.0 / (double)experiment->elapsed_us;
        fprintf(stderr, " fps %s=%.2f %s=%.2f delta=%+.2f%%",
            L2AbArm(0), reference_fps, L2AbArm(1), experiment_fps,
            reference_fps > 0.0
                ? (experiment_fps - reference_fps) * 100.0 / reference_fps : 0.0);
    }
    fprintf(stderr, " (interleaved windows, manual play, no deterministic replay)\n");
}

static void L2AbWindowEnd(unsigned frames, uint64_t elapsed_us, uint64_t scheduler_us) {
    if (!s_ab_target) return;
    const int mode = L2AbMode();
    const unsigned window = s_ab_window++;
    const int counted = window >= L2_AB_WARMUP_WINDOWS && frames != 0;
    if (counted) {
        L2AbTotals *totals = &s_ab_totals[mode];
        totals->frames += frames;
        totals->elapsed_us += elapsed_us;
        totals->scheduler_us += scheduler_us;
        ++totals->windows;
    }
    fprintf(stderr, "[%s] window=%u arm=%s%s frames=%u scheduler_ms=%.4f",
        L2AbTag(), window, L2AbArm(mode), counted ? "" : " warmup",
        frames, frames ? (double)scheduler_us / 1000.0 / (double)frames : 0.0);
    if (elapsed_us)
        fprintf(stderr, " fps=%.2f", (double)frames * 1000000.0 / (double)elapsed_us);
    fputc('\n', stderr);
    L2AbSetMode(!mode);
    if (mode && counted) Lufia2NativePatchesSummary();
}

/* A host that already times whole frames owns the better window boundaries,
 * so its first call takes ownership from the boundary counter below. */
void Lufia2NativePatchesWindowEnd(unsigned frames, uint64_t elapsed_us,
                                  uint64_t scheduler_us) {
    s_ab_host_windows = 1;
    L2AbWindowEnd(frames, elapsed_us, scheduler_us);
}
#endif

/* Environment first, then the compiled default. Anything unrecognized picks
 * the reference path and says so rather than guessing. */
#ifdef L2_ENABLE_EXPERIMENT_AB
static int L2SelectMode(const char *name, int fallback, const char *on_word) {
    const char *value = getenv(name);
    if (!value) return fallback;
    if (!strcmp(value, "0")) return 0;
    if (!strcmp(value, "1")) return 1;
    if (!strcmp(value, "auto")) return 2;
    fprintf(stderr, "[native-patches] %s must be 0, 1 or auto; using the "
                    "reference path instead of %s\n", name, on_word);
    return 0;
}
#endif

#if defined(LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD) || defined(LUFIA2_ENABLE_ACTOR_EARLY_RETURN)
static int L2SelectBinaryMode(const char *name, int fallback) {
    const char *value = getenv(name);
    if (!value) return fallback;
    if (!strcmp(value, "0")) return 0;
    if (!strcmp(value, "1")) return 1;
    fprintf(stderr, "[native-patches] %s must be 0 or 1; using the "
                    "compiled default\n", name);
    return fallback;
}
#endif

int Lufia2NativeObserversActive(void) {
    static int active = -1;
    if (active < 0) {
        active = getenv("SNESRECOMP_CYC_WATCH") != NULL ||
            getenv("SNESRECOMP_IBRWATCH") != NULL ||
            getenv("SNESRECOMP_INTERP_DTRACE") != NULL ||
            getenv("SNESRECOMP_INTERP_MS_PROF") != NULL ||
            getenv("SNESRECOMP_INTERP_RATE_LOG") != NULL ||
            getenv("SNESRECOMP_WLOG_STATE") != NULL;
    }
    return active;
}

void Lufia2NativePatchesInit(void) {
#ifdef L2_ENABLE_EXPERIMENT_AB
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
    int wait_mode = L2SelectMode("LUFIA2_NATIVE_WAIT",
                                 LUFIA2_NATIVE_WAIT_DEFAULT_MODE, "the native payload");
#else
    int wait_mode = 0;
#endif
#ifdef LUFIA2_ENABLE_QUIESCENCE_INDEX
    const int index_mode = L2SelectMode("LUFIA2_QUIESCENCE_INDEX",
                                        LUFIA2_QUIESCENCE_INDEX_DEFAULT_MODE,
                                        "the indexed search");
    lufia2_quiescence_index_enabled = index_mode == 1;
    if (index_mode == 2) {
        s_ab_target = L2_AB_QUIESCENCE_INDEX;
        if (wait_mode == 2) {
            /* One experiment at a time, or neither arm means anything. */
            fprintf(stderr, "[native-patches] both experiments requested auto; "
                            "pinning the payload off and measuring the index\n");
            wait_mode = 0;
        }
    }
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    fprintf(stderr,
        "[quiescence-index] mode=%s predicate=original order=ascending-slot guest_timing=original\n",
        index_mode == 2 ? "auto-ab" : (index_mode ? "indexed" : "original"));
#endif
#endif
    if (wait_mode == 2 && !s_ab_target) s_ab_target = L2_AB_NATIVE_WAIT;
    lufia2_native_wait_enabled = wait_mode == 1;
#ifdef LUFIA2_ENABLE_NATIVE_WAIT
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    fprintf(stderr, "[native-wait] revision=step1 mode=%s pcs=83900E,839010 timing=original-bridge\n",
            wait_mode == 2 ? "auto-ab" : (lufia2_native_wait_enabled ? "native" : "original"));
#endif
#endif
#else
    lufia2_native_wait_enabled = 0;
#endif
#ifdef LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD
    lufia2_frame_wait_fastforward_enabled = L2SelectBinaryMode(
        "LUFIA2_FRAME_WAIT_FASTFORWARD", 1);
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    fprintf(stderr,
        "[frame-wait-native] revision=portable-step5 mode=%s "
        "pcs=83900E,868B4E,85EC96,848D4F,869752 "
        "deadline=reference-final-pair guest_timing=preserved\n",
        lufia2_frame_wait_fastforward_enabled ? "native" : "original");
#endif
#endif
#ifdef LUFIA2_ENABLE_ACTOR_EARLY_RETURN
    lufia2_actor_early_return_enabled = L2SelectBinaryMode(
        "LUFIA2_ACTOR_EARLY_RETURN", 1);
    lufia2_actor_d508_early_return_enabled = L2SelectBinaryMode(
        "LUFIA2_ACTOR_D508_EARLY_RETURN", 1);
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    fprintf(stderr,
        "[actor-fastpath] revision=c7f8-step2+d508-step1 "
        "c7f8=%s d508=%s path=write-free-early-return bus=preserved\n",
        lufia2_actor_early_return_enabled ? "native" : "original",
        lufia2_actor_d508_early_return_enabled ? "native" : "original");
#endif
#endif
}

void Lufia2NativeWaitBegin(void) {
#ifdef LUFIA2_ENABLE_BRIDGE_AUDIT
    L2BAFrameBegin();
#endif
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    s_begin = SDL_GetTicksNS();
#endif
}
void Lufia2NativeWaitEnd(void) {
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    uint64_t elapsed = SDL_GetTicksNS() - s_begin;
#endif
#ifdef LUFIA2_ENABLE_BRIDGE_AUDIT
    L2BAFrameEnd();
#endif
#ifdef LUFIA2_ENABLE_PATCH_REPORTING
    if (!s_frames || elapsed < s_min) s_min = elapsed;
    if (!s_frames || elapsed > s_max) s_max = elapsed;
    s_sum += elapsed;
    if (++s_frames == 120) {
#ifdef LUFIA2_ENABLE_FRAME_WAIT_FASTFORWARD
        const uint64_t frame_wait_pairs_83900e =
            lufia2_frame_wait_ff_site_pairs_count(0u);
        const uint64_t frame_wait_pairs_868b4e =
            lufia2_frame_wait_ff_site_pairs_count(1u);
        const uint64_t frame_wait_pairs_85ec96 =
            lufia2_frame_wait_ff_site_pairs_count(2u);
        const uint64_t frame_wait_pairs_848d4f =
            lufia2_frame_wait_ff_site_pairs_count(3u);
        const uint64_t frame_wait_pairs_869752 =
            lufia2_frame_wait_ff_site_pairs_count(4u);
        const uint64_t frame_wait_pairs =
            frame_wait_pairs_83900e + frame_wait_pairs_868b4e +
            frame_wait_pairs_85ec96 + frame_wait_pairs_848d4f +
            frame_wait_pairs_869752;
        fprintf(stderr,
            "[frame-wait-native] boundaries=120 scheduler_ms=%.4f "
            "min_ms=%.4f max_ms=%.4f skipped_pairs=%llu "
            "pairs_83900e=%llu pairs_868b4e=%llu "
            "pairs_85ec96=%llu pairs_848d4f=%llu "
            "pairs_869752=%llu\n",
            (double)s_sum / 120000000.0, (double)s_min / 1000000.0,
            (double)s_max / 1000000.0,
            (unsigned long long)(frame_wait_pairs -
                s_previous_frame_wait_pairs[0] -
                s_previous_frame_wait_pairs[1] -
                s_previous_frame_wait_pairs[2] -
                s_previous_frame_wait_pairs[3] -
                s_previous_frame_wait_pairs[4]),
            (unsigned long long)(frame_wait_pairs_83900e -
                s_previous_frame_wait_pairs[0]),
            (unsigned long long)(frame_wait_pairs_868b4e -
                s_previous_frame_wait_pairs[1]),
            (unsigned long long)(frame_wait_pairs_85ec96 -
                s_previous_frame_wait_pairs[2]),
            (unsigned long long)(frame_wait_pairs_848d4f -
                s_previous_frame_wait_pairs[3]),
            (unsigned long long)(frame_wait_pairs_869752 -
                s_previous_frame_wait_pairs[4]));
        s_previous_frame_wait_pairs[0] = frame_wait_pairs_83900e;
        s_previous_frame_wait_pairs[1] = frame_wait_pairs_868b4e;
        s_previous_frame_wait_pairs[2] = frame_wait_pairs_85ec96;
        s_previous_frame_wait_pairs[3] = frame_wait_pairs_848d4f;
        s_previous_frame_wait_pairs[4] = frame_wait_pairs_869752;
#ifdef LUFIA2_ENABLE_DMA_HOST_FASTFORWARD
        {
            const uint64_t dma_idle_ticks =
                Lufia2DmaHostFastForwardIdleTicks();
            fprintf(stderr,
                "[dma-host-ff] boundaries=120 idle_timer_ticks=%llu\n",
                (unsigned long long)(dma_idle_ticks -
                    s_previous_dma_idle_ticks));
            s_previous_dma_idle_ticks = dma_idle_ticks;
        }
#endif
#elif defined(LUFIA2_ENABLE_NATIVE_WAIT)
        fprintf(stderr,"[native-wait] boundaries=120 scheduler_ms=%.4f min_ms=%.4f max_ms=%.4f native_cmp=%llu native_beq=%llu\n",
            (double)s_sum / 120000000.0, (double)s_min / 1000000.0, (double)s_max / 1000000.0,
            (unsigned long long)(lufia2_native_wait_steps[0]-s_previous[0]),
            (unsigned long long)(lufia2_native_wait_steps[1]-s_previous[1]));
        s_previous[0] = lufia2_native_wait_steps[0];
        s_previous[1] = lufia2_native_wait_steps[1];
#endif
#ifdef LUFIA2_ENABLE_ACTOR_EARLY_RETURN
        fprintf(stderr,
            "[actor-fastpath] boundaries=120 c7f8_calls=%llu d508_calls=%llu\n",
            (unsigned long long)(lufia2_actor_early_return_hits -
                s_previous_actor_early_return_hits),
            (unsigned long long)(lufia2_actor_d508_early_return_hits -
                s_previous_actor_d508_early_return_hits));
        s_previous_actor_early_return_hits =
            lufia2_actor_early_return_hits;
        s_previous_actor_d508_early_return_hits =
            lufia2_actor_d508_early_return_hits;
#endif
        /* Fallback driver for a host without a frame-time report. Boundaries
         * are not frames, so this path never claims an fps figure. */
#ifdef L2_ENABLE_EXPERIMENT_AB
        if (!s_ab_host_windows) L2AbWindowEnd(120, 0, s_sum / 1000u);
#endif
        s_frames=0; s_sum=0; s_min=0; s_max=0;
    }
#endif
}
