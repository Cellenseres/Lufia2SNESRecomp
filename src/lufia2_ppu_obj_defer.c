#include "lufia2_ppu_obj_defer.h"
#include "lufia2_ppu_host.h"
#include "lufia2_ppu_obj_pixels.h"
#include "lufia2_battle_effects.h"
#include "lufia2_ppu_timeline.h"
#include <stdlib.h>
#include "snesrecomp_platform/task.h"
#include <string.h>
#if defined(LUFIA2_PPU_JOB_TRACE)
#include <stdio.h>
#endif

enum {
    ROWS = 224,
    /* 128 sprites of eight columns without sprite limits. */
    LINE_SLIVERS = 1024,
    POOL_SLIVERS = 24576,
    /* Guest-thread slivers use the lower half, helper evaluation the upper.
     * With sprite limits a line has at most 34 slivers. */
    HELPER_POOL = POOL_SLIVERS / 2,
    /* Snapshots per frame: one, plus one after each timeline interrupt. */
    EVAL_FRAMES = 4,
    SLIVER_FLIP = 1, SLIVER_CLIP = 2, SLIVER_WORDS = 4, SLIVER_EFFECT = 8,
};

typedef struct Sliver {
    int16_t x;
    uint16_t index, z;
    uint8_t flags;
    uint16_t low, high;
} Sliver;

typedef struct Line {
    uint32_t first, count;
    uint16_t clear_begin, clear_end;
    const uint16_t *vram;
    bool has_sprites, row_only, evaluated, clear_effects;
    int16_t effect_left, effect_right;
    /* Evaluated on the helper from these per-line registers. */
    bool queued, margin;
    uint8_t frame, obsel, oamaddl, oamaddh, setini;
    uint16_t extra_left, extra_right;
} Line;

bool lufia2_ppu_obj_defer_enabled;
Ppu *lufia2_ppu_obj_defer_ppu;

static Sliver s_pool[POOL_SLIVERS];
static Line s_lines[ROWS];
static Lufia2PpuObjDeferTarget s_target;
/* Same history as the PPU's objBuffer, outside the cleared span too. */
static PpuPixelPrioBufs s_row;
/* [begin, end) of s_row known to hold the backdrop pattern. Most lines draw
 * no sprite, and clearing an already clear row is pure memory traffic. */
static bool s_row_backdrop;
static unsigned s_backdrop_begin, s_backdrop_end;
static bool s_mask[kPpuBufWidth];
/* Detached battle rows finish on the helper while the guest runs the next
 * frame, whose DMA may rewrite sprite characters. Those rows read this copy,
 * taken before the guest resumes; until then VRAM cannot change. Character
 * fetches near the end read eight words past VRAM, as from the live PPU. */
static uint16_t s_detached_vram[0x8000 + 8];
static const uint16_t *s_line_vram;
static const uint16_t *volatile s_row_vram;
/* Optional, persistent battle storage. Allocation refusal keeps the serial
 * path; workers never allocate or borrow live actor data. */
static PpuPixelPrioBufs *s_margins;
static uint32_t s_used;
static int s_open = -1;
static volatile uint32_t s_published;
static volatile bool s_closing;
static volatile uint32_t s_rows_done;
static uint64_t s_rows;
/* Detached: rows are closed but the helper may still run, followed by the
 * consumer task it was handed. The live row returns at the next join. */
static bool s_detached;
static Ppu *s_detached_ppu;
static void (*volatile s_then)(void *);
static void *volatile s_then_arg;
static Ppu *volatile s_then_snapshot;
/* Helper evaluation: frame snapshots written before their lines publish. */
static bool s_queueing;
static Lufia2PpuObjEval s_frames[EVAL_FRAMES];
static unsigned s_frame_count;
static uint32_t s_frame_epoch;
static Lufia2PpuObjEval s_work;
static unsigned s_work_frame;
/* Bumped whenever s_work is reloaded; ranges are only rebuilt from a new snapshot. */
static uint32_t s_work_serial, s_range_serial;
static uint32_t s_helper_used;
static Line *s_eval_line;
static volatile bool s_range_over, s_time_over;
static int s_open_queued = -1;

static int Max(int a, int b) { return a > b ? a : b; }
static int Min(int a, int b) { return a < b ? a : b; }

static void Signal(void) {
    __sync_synchronize();
#if defined(__3DS__)
    __asm__ volatile("sev");
#endif
}

static void Pause(void) {
#if defined(__3DS__)
    __asm__ volatile("wfe");
#endif
}

static void Rasterize(const Line *l) {
    static const uint64_t backdrop = 0x0500050005000500ull;
    const bool battle = s_target.battle_masks != NULL;
    if (battle && l->evaluated) memset(s_mask, 0, sizeof s_mask);
    const unsigned cleared_end = l->clear_end > l->clear_begin ?
        l->clear_begin + ((l->clear_end - l->clear_begin + 3u) & ~3u) : l->clear_begin;
    if (!s_row_backdrop || l->clear_begin < s_backdrop_begin || cleared_end > s_backdrop_end) {
        for (unsigned i = l->clear_begin; i < l->clear_end; i += 4)
            memcpy(&s_row.data[i], &backdrop, sizeof backdrop);
        s_row_backdrop = true;
        s_backdrop_begin = l->clear_begin;
        s_backdrop_end = cleared_end;
    }
    if (l->count || l->clear_effects) s_row_backdrop = false;
    const uint16_t *const copy = s_row_vram;
    const uint16_t *const vram = copy && l->vram == s_line_vram ? copy : l->vram;
    for (uint32_t i = 0; i < l->count; ++i) {
        const Sliver *s = &s_pool[l->first + i];
        const uint16_t *addr = vram + s->index;
        const uint32_t words = s->flags & SLIVER_WORDS ?
            (uint32_t)s->low | (uint32_t)s->high << 16 :
            (uint32_t)addr[0] | (uint32_t)addr[8] << 16;
        const uint32_t plane = Lufia2PpuDecode4bpp(words);
        const int px_left = Max(-(s->x + kPpuExtraLeftRight), 0);
        const int px_right = Min(256 + kPpuExtraLeftRight - s->x, 8);
        PpuZbufType *dst = s_row.data + s->x + px_left + kPpuExtraLeftRight;
        for (int px = px_left; px < px_right; px++, dst++) {
            const int shift = s->flags & SLIVER_FLIP ? px : 7 - px;
            const int pixel = (plane >> (4 * shift)) & 15u;
            if (pixel == 0) continue;
            if ((s->flags & SLIVER_CLIP) && s->x + px < 0) continue;
            dst[0] = (PpuZbufType)(s->z + pixel);
            if (battle) s_mask[s->x + px + kPpuExtraLeftRight] = (s->flags & SLIVER_EFFECT) != 0;
        }
    }
    if (l->clear_effects) {
        const PpuZbufType *margin = s_margins[l - s_lines].data;
        for (int x = l->effect_left; x < l->effect_right; ++x) {
            if (x >= 0 && x < 256) continue;
            const unsigned at = (unsigned)(x + kPpuExtraLeftRight);
            if (s_mask[at]) { s_row.data[at] = 0x0500; s_mask[at] = false; }
            if (margin[at]) { s_row.data[at] = margin[at]; s_mask[at] = true; }
        }
    }
}

/* Same conversion as Lufia2PpuOffloadRecordLine outside battles. */
static bool Record(unsigned y, const Line *l) {
    uint16_t *out = s_target.obj_pixels + y * s_target.width;
    const unsigned offset=kPpuExtraLeftRight-s_target.extra;
    const bool certified=s_target.valid_rows && s_target.has_rows;
    const bool reused=certified && Lufia2PpuObjRowMatches(
        s_target.valid_rows[y],s_target.has_rows[y],s_target.objects[y].data+offset,
        s_target.battle_masks?s_target.battle_masks[y]+offset:NULL,l->has_sprites,
        s_row.data+offset,s_target.battle_masks?s_mask+offset:NULL,s_target.width,s_target.extra);
    /* Native history and masks still belong to serial fallback, even when
     * only unused scratch outside the captured viewport changed. Its replay
     * reads just the active span; a matching row already holds the viewport. */
    if(s_target.battle_masks)memcpy(s_target.battle_masks[y],s_mask,sizeof s_mask);
    if(l->has_sprites) {
        PpuZbufType *const saved=s_target.objects[y].data;
        const unsigned begin=Min(l->clear_begin,offset),end=Max(l->clear_end,offset+s_target.width);
        if(reused) {
            memcpy(saved+begin,s_row.data+begin,(offset-begin)*sizeof *saved);
            memcpy(saved+offset+s_target.width,s_row.data+offset+s_target.width,
                (end-offset-s_target.width)*sizeof *saved);
        } else memcpy(saved+begin,s_row.data+begin,(end-begin)*sizeof *saved);
    }
    if(!reused) {
        Lufia2PpuObjPackRow(out,s_row.data+offset,l->has_sprites,
            s_target.battle_masks?s_mask+offset:NULL,s_target.width,s_target.extra);
        if(s_target.row_serials) s_target.row_serials[y]=s_target.serial;
    }
    if(certified) {
        s_target.has_rows[y]=l->has_sprites;
        s_target.valid_rows[y]=1u;
    }
    if(reused) {if(s_target.reused_rows)++*s_target.reused_rows;}
    else if(s_target.generated_rows)++*s_target.generated_rows;
    return reused;
}

static void Evaluate(Line *l, unsigned line) {
    if (s_work_frame != l->frame) {
        s_work = s_frames[l->frame];
        s_work_frame = l->frame;
        ++s_work_serial;
    }
    s_work.obsel = l->obsel; s_work.oamaddl = l->oamaddl;
    s_work.oamaddh = l->oamaddh; s_work.setini = l->setini;
    s_work.extraLeftCur = l->extra_left; s_work.extraRightCur = l->extra_right;
    s_work.marginSprites = l->margin;
    s_work.rangeOver = s_work.timeOver = false;
    s_eval_line = l;
    l->first = s_helper_used;
    l->has_sprites = Lufia2PpuObjEvalLine(&s_work, (int)line);
    l->count = s_helper_used - l->first;
    if (s_work.rangeOver) s_range_over = true;
    if (s_work.timeOver) s_time_over = true;
}

void Lufia2PpuObjEvalBeginSprites(void) { s_eval_line->evaluated = true; }

/* The cache compares all of OAM on every request. Between snapshot loads
 * the evaluator's OAM cannot change, so one full check per snapshot is
 * enough; the per-line size mode still goes through the full path. */
const uint32_t *Lufia2PpuObjEvalRangeRow(SnesRecompObjRangeCache *cache,
    const uint16_t oam[256], const uint8_t high_oam[32],
    unsigned size_mode, bool interlace, unsigned line) {
    if (oam == s_work.oam && cache->valid && s_range_serial == s_work_serial &&
        cache->size_mode == size_mode && cache->interlace == interlace)
        return cache->rows[line & 255u];
    const uint32_t *row = snesrecomp_obj_range_row(cache, oam, high_oam, size_mode, interlace, line);
    if (oam == s_work.oam) s_range_serial = s_work_serial;
    return row;
}

void Lufia2PpuObjEvalSliver(int x, unsigned vram_index, uint16_t z,
    bool flip, bool clip, bool effect) {
    Sliver *s = &s_pool[s_helper_used++];
    s->x = (int16_t)x;
    s->index = (uint16_t)vram_index;
    s->z = z;
    s->flags = (uint8_t)((flip ? SLIVER_FLIP : 0) | (clip ? SLIVER_CLIP : 0) |
        (effect ? SLIVER_EFFECT : 0));
    if (vram_index > 0x7ff7u) {
        const uint16_t *const copy = s_row_vram;
        const uint16_t *addr = (copy && s_eval_line->vram == s_line_vram ? copy :
            s_eval_line->vram) + vram_index;
        s->low = addr[0];
        s->high = addr[8];
        s->flags |= SLIVER_WORDS;
    }
}

static void Worker(void *arg) {
    (void)arg;
    uint32_t done = 0;
    for (;;) {
        const uint32_t ready = s_published;
        __sync_synchronize();
        if (done < ready) {
            Line *l = &s_lines[done];
            if (l->queued) Evaluate(l, done);
            Rasterize(l);
            if (!l->row_only) Record(done, l);
            ++done;
            ++s_rows;
            __sync_synchronize();
            s_rows_done = done;
            continue;
        }
        if (s_closing) {
            __sync_synchronize();
            if (done == s_published) break;
            continue;
        }
        Pause();
    }
    void (*then)(void *) = s_then;
    if (then) {
        if (s_then_snapshot) s_then_snapshot->objBuffer = s_row;
        then(s_then_arg);
    }
}

bool Lufia2PpuObjDeferStart(Ppu *ppu, const Lufia2PpuObjDeferTarget *target) {
    Lufia2PpuObjDeferJoin();
    if (!lufia2_ppu_obj_defer_enabled || !ppu || !target || lufia2_ppu_obj_defer_ppu ||
        !snesrecomp_task_worker_count()) return false;
    if (!target->objects || !target->obj_pixels || target->width < 256u ||
        target->width > 512u || target->extra > kPpuExtraLeftRight ||
        target->extra > target->width - 256u ||
        target->width - target->extra - 256u > kPpuExtraLeftRight) return false;
    if (target->battle_masks && !s_margins) {
        s_margins = calloc(ROWS, sizeof *s_margins);
        if (!s_margins) return false;
    }
    s_target = *target;
    s_row = ppu->objBuffer;
    s_row_backdrop = false;
    s_row_vram = NULL;
    s_line_vram = NULL;
    Lufia2BattleEffectsCaptureSpriteRow(s_mask);
    s_used = 0;
    s_open = -1;
    s_published = 0;
    s_rows_done = 0;
    s_closing = false;
    s_then = NULL;
    s_then_snapshot = NULL;
#if defined(LUFIA2_OBJ_DEFER_CHECK)
    s_queueing = false;
#else
    /* Field capture still waits for these rows, but evaluating them on the
     * helper overlaps the rest of the timeline (town: 5.3 -> 3.4 ms). */
    s_queueing = !(ppu->renderFlags & kPpuRenderFlags_NoSpriteLimits);
#endif
    s_frame_count = 0;
    s_work_frame = EVAL_FRAMES;
    s_helper_used = HELPER_POOL;
    s_range_over = s_time_over = false;
    s_open_queued = -1;
    __sync_synchronize();
    if (!snesrecomp_task_submit(0, Worker, NULL)) return false;
    lufia2_ppu_obj_defer_ppu = ppu;
    return true;
}

static Ppu *CloseRows(void) {
    Ppu *ppu = lufia2_ppu_obj_defer_ppu;
    if (!ppu) return NULL;
    lufia2_ppu_obj_defer_ppu = NULL;
    /* A begun row the capture never took still belongs to the PPU. */
    if (s_open >= 0) {
        s_lines[s_open].row_only = true;
        if (s_lines[s_open].queued) s_open_queued = s_open;
        else s_lines[s_open].count = s_used - s_lines[s_open].first;
        __sync_synchronize();
        s_published = (uint32_t)s_open + 1u;
        s_open = -1;
    }
    return ppu;
}
static void ReturnRow(Ppu *ppu) {
    ppu->objBuffer = s_row;
    /* The helper's evaluation results become visible with its row. */
    if (s_range_over) ppu->rangeOver = true;
    if (s_time_over) ppu->timeOver = true;
    if (s_open_queued >= 0) ppu->lineHasSprites = s_lines[s_open_queued].has_sprites;
    s_open_queued = -1;
    if (s_target.battle_masks) Lufia2BattleEffectsRestoreSpriteRow(s_mask);
}
bool Lufia2PpuObjDeferDetach(void) {
    Lufia2PpuObjDeferJoin();
    Ppu *ppu = CloseRows();
    if (!ppu) return false;
    if (s_line_vram) {
        memcpy(s_detached_vram, s_line_vram, sizeof s_detached_vram);
        __sync_synchronize();
        s_row_vram = s_detached_vram;
    }
    s_detached_ppu = ppu;
    s_detached = true;
    return true;
}
bool Lufia2PpuObjDeferThen(void (*fn)(void *), void *arg, Ppu *snapshot) {
    if (!s_detached || s_closing || !fn) return false;
    s_then = fn;
    s_then_arg = arg;
    s_then_snapshot = snapshot;
    __sync_synchronize();
    s_closing = true;
    Signal();
    return true;
}
void Lufia2PpuObjDeferJoin(void) {
    if (!s_detached) return;
    if (!s_closing) { s_closing = true; Signal(); }
    snesrecomp_task_wait();
    __sync_synchronize();
    s_detached = false;
    ReturnRow(s_detached_ppu);
}

void Lufia2PpuObjDeferStop(void) {
    Lufia2PpuObjDeferJoin();
#if defined(LUFIA2_PPU_JOB_TRACE)
    const unsigned done_before=s_rows_done;
#endif
    Ppu *ppu = CloseRows();
    if (!ppu) return;
    s_closing = true;
    Signal();
#if defined(LUFIA2_PPU_JOB_TRACE)
    const uint64_t wait_begin=snesrecomp_now_us();
#endif
    snesrecomp_task_wait();
    __sync_synchronize();
#if defined(LUFIA2_PPU_JOB_TRACE)
    const uint64_t wait_us=snesrecomp_now_us()-wait_begin;
    fprintf(stderr,"OBJ stop progress: done_before=%u published=%u done_after=%u slivers=%u wait_us=%llu\n",
        done_before,(unsigned)s_published,(unsigned)s_rows_done,(unsigned)s_used,
        (unsigned long long)wait_us);
#endif
    ReturnRow(ppu);
}

void Lufia2PpuObjDeferBeginLine(unsigned line, unsigned clear_begin, unsigned clear_end,
                                const uint16_t *vram) {
    Line *l = &s_lines[line];
    l->first = s_used;
    l->count = 0;
    l->clear_begin = (uint16_t)clear_begin;
    l->clear_end = (uint16_t)clear_end;
    l->vram = vram;
    s_line_vram = vram;
    l->has_sprites = false;
    l->row_only = false;
    l->evaluated = l->clear_effects = false;
    l->queued = false;
    s_open = (int)line;
}

void Lufia2PpuObjDeferBeginSprites(void) {
    s_lines[s_open].evaluated = true;
}

void Lufia2PpuObjDeferSliver(int x, unsigned vram_index, uint16_t z,
    bool flip, bool clip, bool effect) {
    Sliver *s = &s_pool[s_used++];
    s->x = (int16_t)x;
    s->index = (uint16_t)vram_index;
    s->z = z;
    s->flags = (uint8_t)((flip ? SLIVER_FLIP : 0) | (clip ? SLIVER_CLIP : 0) |
        (effect ? SLIVER_EFFECT : 0));
    /* Words past VRAM are read now, as the original evaluator does. */
    if (vram_index > 0x7ff7u) {
        const uint16_t *addr = s_lines[s_open].vram + vram_index;
        s->low = addr[0];
        s->high = addr[8];
        s->flags |= SLIVER_WORDS;
    }
}

static void ClearEffects(void *context, int left, int right) {
    Line *l = context;
    l->clear_effects = true;
    l->effect_left = (int16_t)left; l->effect_right = (int16_t)right;
    memset(&s_margins[l - s_lines], 0, sizeof *s_margins);
}
static void MarginPixel(void *context, int x, uint16_t native) {
    const Line *l = context;
    s_margins[l - s_lines].data[x + kPpuExtraLeftRight] = native;
}
bool Lufia2PpuObjDeferBattleMargins(Ppu *ppu, const uint16_t *vram, unsigned line) {
    const Lufia2BattleEffectsMarginWriter writer = {&s_lines[s_open], ClearEffects, MarginPixel};
    return Lufia2BattleEffectsSpriteMarginsWithWriter(ppu, vram, line,
        s_target.battle_masks ? &writer : NULL);
}

bool Lufia2PpuObjDeferLineFits(unsigned line) {
    return line < ROWS && line == s_published && HELPER_POOL - s_used >= LINE_SLIVERS;
}

bool Lufia2PpuObjDeferQueueing(void) {
    /* Outside the timeline's read-only intervals the snapshot is not stable. */
    return s_queueing && s_open >= 0 && Lufia2PpuTimelineReadOnlyEpoch() &&
        (s_frame_count < EVAL_FRAMES || s_frame_epoch == Lufia2PpuTimelineReadOnlyEpoch());
}

static void Snapshot(Lufia2PpuObjEval *e, const Ppu *ppu) {
    e->evenFrame = ppu->evenFrame;
    e->extraLeftRight = ppu->extraLeftRight;
    e->renderFlags = ppu->renderFlags;
    e->wsHudSplitHeight = ppu->wsHudSplitHeight;
    e->wsHudLeftEnd = ppu->wsHudLeftEnd;
    e->wsHudRightStart = ppu->wsHudRightStart;
    e->wsHudOamFirstSlot = ppu->wsHudOamFirstSlot;
    e->wsHudOamSlots = ppu->wsHudOamSlots;
    e->wsHudOamHeight = ppu->wsHudOamHeight;
    e->wsHudOamFirstSlot2 = ppu->wsHudOamFirstSlot2;
    e->wsHudOamSlots2 = ppu->wsHudOamSlots2;
    e->wsOamMotionGraceOn = ppu->wsOamMotionGraceOn;
    e->wsOamLeftHintStrict = ppu->wsOamLeftHintStrict;
    e->wsOamRightHintStrict = ppu->wsOamRightHintStrict;
    memcpy(e->wsOamLeftHint, ppu->wsOamLeftHint, sizeof e->wsOamLeftHint);
    memcpy(e->wsOamRightHint, ppu->wsOamRightHint, sizeof e->wsOamRightHint);
    memcpy(e->wsOamMotionGrace, ppu->wsOamMotionGrace, sizeof e->wsOamMotionGrace);
    memcpy(e->oam, ppu->oam, sizeof e->oam);
    memcpy(e->highOam, ppu->highOam, sizeof e->highOam);
    memset(e->effects, 0, sizeof e->effects);
    for (unsigned slot = 0; slot < 128; ++slot)
        if (Lufia2BattleEffectsSprite(ppu, slot)) e->effects[slot >> 3] |= (uint8_t)(1u << (slot & 7u));
}

void Lufia2PpuObjDeferQueue(const Ppu *ppu, unsigned line, bool margin_sprites) {
    Line *l = &s_lines[line];
    const uint32_t epoch = Lufia2PpuTimelineReadOnlyEpoch();
    if (!s_frame_count || s_frame_epoch != epoch) {
        Snapshot(&s_frames[s_frame_count++], ppu);
        s_frame_epoch = epoch;
    }
    l->queued = true;
    l->margin = margin_sprites;
    l->frame = (uint8_t)(s_frame_count - 1u);
    l->obsel = ppu->obsel; l->oamaddl = ppu->oamaddl;
    l->oamaddh = ppu->oamaddh; l->setini = ppu->setini;
    l->extra_left = ppu->extraLeftCur; l->extra_right = ppu->extraRightCur;
}

void Lufia2PpuObjDeferSyncFlags(Ppu *ppu) {
    if (s_detached) { Lufia2PpuObjDeferJoin(); return; }
    if (lufia2_ppu_obj_defer_ppu != ppu) return;
    while (s_rows_done != s_published) { Signal(); __sync_synchronize(); }
    if (s_range_over) ppu->rangeOver = true;
    if (s_time_over) ppu->timeOver = true;
}

bool Lufia2PpuObjDeferTakeLine(unsigned line) {
    return lufia2_ppu_obj_defer_ppu && s_open == (int)line;
}

void Lufia2PpuObjDeferPublish(unsigned y, bool has_sprites) {
    Line *l = &s_lines[y];
    if (!l->queued) {
        l->has_sprites = has_sprites;
        l->count = s_used - l->first;
    }
    s_open = -1;
    __sync_synchronize();
    s_published = y + 1u;
    Signal();
}

uint64_t Lufia2PpuObjDeferRows(void) { return s_rows; }

#if defined(LUFIA2_OBJ_DEFER_CHECK)
static uint64_t s_checked, s_mismatched;

void Lufia2PpuObjDeferCheck(unsigned y, bool has_sprites,
    const PpuPixelPrioBufs *expected, const bool *expected_mask) {
    s_lines[y].row_only = true;
    Lufia2PpuObjDeferPublish(y, has_sprites);
    while (s_rows_done <= y) __sync_synchronize();
    ++s_checked;
    if (memcmp(&s_row, expected, sizeof s_row) ||
        (expected_mask && memcmp(s_mask, expected_mask, sizeof s_mask))) ++s_mismatched;
}

void Lufia2PpuObjDeferCheckStats(uint64_t *checked, uint64_t *mismatched) {
    *checked = s_checked;
    *mismatched = s_mismatched;
}
#endif
