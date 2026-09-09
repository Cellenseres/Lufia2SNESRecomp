#include "lufia2_perf_audit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "snesrecomp_platform/gpu_bg.h"

/* All mutable state belongs to the game thread. Audio callbacks return before
 * touching it. Owner is initialized before audio threads are created. No TLS,
 * allocations or I/O are needed in the instruction/lock hooks. */
static SDL_ThreadID s_owner;
int l2_perf_audit_active_main;
#define s_active l2_perf_audit_active_main
static unsigned s_frame, s_frames, s_windows, s_errors, s_dropped;
static L2PerfScope *s_top;
static L2PerfScope s_frame_scope;
enum { WINDOW = 600, HOT_CAP = 4096, EDGE_CAP = 4096, TOP = 16 };
typedef struct { uint64_t inclusive, exclusive, calls; } Metric;
static Metric s_metric[L2_CATEGORY_COUNT];
static uint64_t s_frame_us[WINDOW];
static uint64_t s_work_us[WINDOW], s_pacing_before;
static uint64_t s_aot_returns[2];
typedef struct {
    uint32_t pc;
    unsigned mx, category, used;
    uint64_t us, exclusive, samples;
} Hot;
static Hot s_hot[HOT_CAP];
typedef struct {
    uint32_t site, target;
    unsigned mx, opcode, used;
    uint64_t calls, missing, excluded;
} Edge;
static Edge s_edge[EDGE_CAP];
static const char *const names[L2_CATEGORY_COUNT] = {
    "frame_residual", "guest_host", "interpreter", "aot", "apu_flush",
    "apu_sync", "apu_lock_acquire", "apu_execute", "bus_sample", "quiescence_sample",
    "state_sync_sample", "pc_sample", "video_prepare", "ppu",
    "capture", "gxm_host", "cpu_fallback", "present", "events", "input",
    "pacing", "misc"
};
static uint64_t now_us(void) { return SDL_GetTicksNS() / 1000u; }
static int owned(void) { return SDL_GetCurrentThreadID() == s_owner; }

static int hot_slot(int category, uint32_t pc, unsigned mx) {
    unsigned h = (pc * 2654435761u + mx * 17u + (unsigned)category) & (HOT_CAP - 1);
    for (unsigned n = 0; n < HOT_CAP; ++n) {
        Hot *p = &s_hot[(h + n) & (HOT_CAP - 1)];
        if (!p->used) {
            p->used = 1; p->pc = pc; p->mx = mx; p->category = (unsigned)category;
            return (int)((h + n) & (HOT_CAP - 1));
        }
        if (p->pc == pc && p->mx == mx && p->category == (unsigned)category)
            return (int)((h + n) & (HOT_CAP - 1));
    }
    ++s_dropped;
    return -1;
}
void L2PerfEnter(L2PerfScope *p, int category, uint32_t pc, unsigned mx) {
    if (!owned() || !s_active) return;
    p->category = category; p->children = 0; p->parent = s_top;
    p->hot = (category == L2_PC || category == L2_AOT)
        ? hot_slot(category, pc, mx) : -1;
    p->start = now_us(); p->active = 1; s_top = p;
}
void L2PerfLeave(L2PerfScope *p) {
    if (!p->active) return;
    /* A watchdog longjmp bypasses cleanup. Never follow a stale stack pointer
     * afterwards: disable diagnostics and let the core handle the watchdog. */
    if (s_top != p) {
        ++s_errors; s_top = NULL; s_active = 0; p->active = 0; return;
    }
    const uint64_t elapsed = now_us() - p->start;
    const uint64_t exclusive = elapsed >= p->children ? elapsed - p->children : 0;
    Metric *m = &s_metric[p->category];
    m->inclusive += elapsed; m->exclusive += exclusive; ++m->calls;
    if (p->hot >= 0) {
        Hot *h = &s_hot[p->hot];
        h->us += elapsed; h->exclusive += exclusive; ++h->samples;
    }
    s_top = p->parent;
    if (s_top) s_top->children += elapsed;
    p->active = 0;
}
void L2PerfAbort(void) {
    if (!owned()) return;
    ++s_errors; s_active = 0; s_top = NULL;
}
void L2PerfCall(uint32_t site, uint32_t target, unsigned mx, unsigned opcode,
                int has_body, int bounce_ok) {
    if (!s_active) return;
    unsigned h = (site * 2654435761u ^ target * 97u ^ mx ^ opcode) & (EDGE_CAP - 1);
    for (unsigned n = 0; n < EDGE_CAP; ++n) {
        Edge *p = &s_edge[(h + n) & (EDGE_CAP - 1)];
        if (!p->used) {
            p->used = 1; p->site = site; p->target = target;
            p->mx = mx; p->opcode = opcode;
        }
        if (p->site == site && p->target == target && p->mx == mx && p->opcode == opcode) {
            ++p->calls; p->missing += !has_body; p->excluded += !bounce_ok;
            return;
        }
    }
    ++s_dropped;
}
void L2PerfAotReturn(int result) {
    if (s_active) ++s_aot_returns[result != 0];
}
void L2PerfInit(void) {
    s_owner = SDL_GetCurrentThreadID();
    fprintf(stderr, "[perf-audit] v1 enabled window=600 frames warmup=120 "
        "bus_q_sync_period=257 pc_period=1021 main_thread_only=1 "
        "inclusive_is_nested=1 max_windows=3\n");
}
void L2PerfFrameBegin(unsigned frame) {
    s_frame = frame;
    /* Three finite windows; no discovery file or environment input required. */
    if (s_errors) {
        static int reported;
        if (!reported) fprintf(stderr, "[perf-audit-invalid] scope stack interrupted; audit disabled\n");
        reported = 1; s_active = 0; return;
    }
    s_active = frame > 120 && s_windows < 3;
    if (s_active) {
        if (s_frames == 0) {
            SnesRecompGpuBgStages discard;
            (void)snesrecomp_gpu_bg_stages(&discard); /* exclude warmup */
        }
        s_pacing_before = s_metric[L2_PACING].inclusive;
        L2PerfEnter(&s_frame_scope, L2_FRAME, 0, 0);
    }
}
void L2PerfFrameCancel(void) {
    if (!s_active) return;
    L2PerfLeave(&s_frame_scope);
    s_active = 0;
    /* A paused/event-only frame must not enter a gameplay window. Discard
     * the partial window so its numerator and denominator stay consistent. */
    memset(s_metric, 0, sizeof(s_metric)); memset(s_hot, 0, sizeof(s_hot));
    memset(s_edge, 0, sizeof(s_edge)); memset(s_aot_returns, 0, sizeof(s_aot_returns));
    s_frames = 0;
}
static int compare_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}
static void report(void) {
    qsort(s_frame_us, s_frames, sizeof(s_frame_us[0]), compare_u64);
    qsort(s_work_us, s_frames, sizeof(s_work_us[0]), compare_u64);
    fprintf(stderr, "[perf-audit-window] first=%u last=%u frames=%u "
        "frame_p50_ms=%.3f frame_p95_ms=%.3f work_p95_ms=%.3f frame_avg_ms=%.4f errors=%u dropped=%u "
        "aot_return_normal=%llu aot_return_unwind=%llu\n",
        s_frame - s_frames + 1, s_frame, s_frames,
        s_frame_us[s_frames / 2] / 1000.0,
        s_frame_us[(s_frames * 95 - 1) / 100] / 1000.0,
        s_work_us[(s_frames * 95 - 1) / 100] / 1000.0,
        s_metric[L2_FRAME].inclusive / (1000.0 * s_frames), s_errors, s_dropped,
        (unsigned long long)s_aot_returns[0], (unsigned long long)s_aot_returns[1]);
    double divisor = 1000.0 * s_frames;
    for (int i = 0; i < L2_CATEGORY_COUNT; ++i) {
        const Metric *m = &s_metric[i];
        fprintf(stderr, "[perf-audit-stage] name=%s exclusive_ms=%.4f inclusive_ms=%.4f calls=%llu\n",
            names[i], m->exclusive / divisor, m->inclusive / divisor,
            (unsigned long long)m->calls);
    }
    /* Rank AOT by inclusive time and sampled PCs separately. Never add either
     * ranking to the exclusive stage partition; they describe the same work. */
    for (int cat = 0; cat < 2; ++cat) {
        unsigned selected[TOP], count = 0;
        for (unsigned rank = 0; rank < TOP; ++rank) {
            int best = -1;
            for (unsigned i = 0; i < HOT_CAP; ++i) {
                if (!s_hot[i].used || s_hot[i].category != (unsigned)(cat ? L2_PC : L2_AOT)) continue;
                int seen = 0;
                for (unsigned j = 0; j < count; ++j) if (selected[j] == i) seen = 1;
                if (!seen && (best < 0 || s_hot[i].us > s_hot[best].us)) best = (int)i;
            }
            if (best < 0) break;
            selected[count++] = (unsigned)best;
            const Hot *h = &s_hot[best];
            fprintf(stderr, "[perf-audit-hot] kind=%s pc=%06X mx=M%uX%u samples=%llu "
                "observed_inclusive_ms=%.5f observed_exclusive_ms=%.5f period=%u\n",
                cat ? "pc_sample" : "aot", h->pc, h->mx >> 1, h->mx & 1,
                (unsigned long long)h->samples, h->us / divisor, h->exclusive / divisor,
                cat ? 1021u : 1u);
        }
    }
    /* Complete bounded edge table, emitted only after a window. It is a text
     * diagnosis, not a build manifest or an instruction to promote roots. */
    for (unsigned i = 0; i < EDGE_CAP; ++i) {
        const Edge *e = &s_edge[i];
        if (e->used) fprintf(stderr, "[perf-audit-edge] site=%06X target=%06X mx=M%uX%u "
            "opcode=%02X calls=%llu missing=%llu excluded=%llu\n",
            e->site, e->target, e->mx >> 1, e->mx & 1, e->opcode,
            (unsigned long long)e->calls, (unsigned long long)e->missing,
            (unsigned long long)e->excluded);
    }
    SnesRecompGpuBgStages gpu;
    if (snesrecomp_gpu_bg_stages(&gpu)) fprintf(stderr,
        "[perf-audit-gxm-window] frames=%u upload_us=%u build_us=%u begin_scene_us=%u "
        "submit_us=%u end_scene_us=%u flip_queue_us=%u\n", gpu.frames,
        gpu.upload_us, gpu.build_us, gpu.begin_us, gpu.submit_us, gpu.end_us, gpu.flip_us);
}
void L2PerfFrameEnd(void) {
    if (!s_active) return;
    const uint64_t before = s_metric[L2_FRAME].inclusive;
    L2PerfLeave(&s_frame_scope);
    s_active = 0;
    if (s_top) { ++s_errors; return; }
    const uint64_t elapsed = s_metric[L2_FRAME].inclusive - before;
    const uint64_t pacing = s_metric[L2_PACING].inclusive - s_pacing_before;
    s_frame_us[s_frames] = elapsed;
    s_work_us[s_frames++] = elapsed >= pacing ? elapsed - pacing : 0;
    if (s_frames == WINDOW) {
        report();
        memset(s_metric, 0, sizeof(s_metric));
        memset(s_hot, 0, sizeof(s_hot)); memset(s_edge, 0, sizeof(s_edge));
        memset(s_aot_returns, 0, sizeof(s_aot_returns));
        s_frames = 0; ++s_windows;
        s_dropped = s_errors = 0;
    }
}
