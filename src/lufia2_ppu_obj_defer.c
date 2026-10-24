#include "lufia2_ppu_obj_defer.h"
#include "lufia2_ppu_host.h"
#include "snesrecomp_platform/task.h"
#include <string.h>

enum {
    ROWS = 224,
    /* 128 sprites of eight columns without sprite limits. */
    LINE_SLIVERS = 1024,
    POOL_SLIVERS = 24576,
    SLIVER_FLIP = 1, SLIVER_CLIP = 2, SLIVER_WORDS = 4,
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
    bool has_sprites, row_only;
} Line;

bool lufia2_ppu_obj_defer_enabled;
Ppu *lufia2_ppu_obj_defer_ppu;

static Sliver s_pool[POOL_SLIVERS];
static Line s_lines[ROWS];
static Lufia2PpuObjDeferTarget s_target;
/* Same history as the PPU's objBuffer, outside the cleared span too. */
static PpuPixelPrioBufs s_row;
static uint32_t s_used;
static int s_open = -1;
static volatile uint32_t s_published;
static volatile bool s_closing;
static volatile uint32_t s_rows_done;
static uint64_t s_rows;

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
    for (unsigned i = l->clear_begin; i < l->clear_end; i += 4)
        memcpy(&s_row.data[i], &backdrop, sizeof backdrop);
    for (uint32_t i = 0; i < l->count; ++i) {
        const Sliver *s = &s_pool[l->first + i];
        const uint16_t *addr = l->vram + s->index;
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
        }
    }
}

/* Same conversion as Lufia2PpuOffloadRecordLine outside battles. */
static void Record(unsigned y, const Line *l) {
    uint16_t *out = s_target.obj_pixels + y * s_target.width;
    if (!l->has_sprites) {
        memset(out, 0, s_target.width * sizeof(uint16_t));
        return;
    }
    s_target.objects[y] = s_row;
    for (unsigned x = 0; x < s_target.width; ++x) {
        const uint16_t native = s_row.data[(int)x - (int)s_target.extra + kPpuExtraLeftRight];
        out[x] = (native & 255u) ?
            (uint16_t)((native & 255u) | (((native >> 12) - 2u) / 4u << 8) |
                       (((native >> 8) & 15u) == 4u ? 0x400u : 0u) | 0x800u) : 0;
    }
}

static void Worker(void *arg) {
    (void)arg;
    uint32_t done = 0;
    for (;;) {
        const uint32_t ready = s_published;
        __sync_synchronize();
        if (done < ready) {
            const Line *l = &s_lines[done];
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
}

bool Lufia2PpuObjDeferStart(Ppu *ppu, const Lufia2PpuObjDeferTarget *target) {
    if (!lufia2_ppu_obj_defer_enabled || !ppu || !target || lufia2_ppu_obj_defer_ppu ||
        !snesrecomp_task_worker_count()) return false;
    s_target = *target;
    s_row = ppu->objBuffer;
    s_used = 0;
    s_open = -1;
    s_published = 0;
    s_rows_done = 0;
    s_closing = false;
    __sync_synchronize();
    if (!snesrecomp_task_submit(0, Worker, NULL)) return false;
    lufia2_ppu_obj_defer_ppu = ppu;
    return true;
}

void Lufia2PpuObjDeferStop(void) {
    Ppu *ppu = lufia2_ppu_obj_defer_ppu;
    if (!ppu) return;
    lufia2_ppu_obj_defer_ppu = NULL;
    /* A begun row the capture never took still belongs to the PPU. */
    if (s_open >= 0) {
        s_lines[s_open].row_only = true;
        s_lines[s_open].count = s_used - s_lines[s_open].first;
        s_published = (uint32_t)s_open + 1u;
        s_open = -1;
    }
    s_closing = true;
    Signal();
    snesrecomp_task_wait();
    __sync_synchronize();
    ppu->objBuffer = s_row;
}

void Lufia2PpuObjDeferBeginLine(unsigned line, unsigned clear_begin, unsigned clear_end,
                                const uint16_t *vram) {
    Line *l = &s_lines[line];
    l->first = s_used;
    l->count = 0;
    l->clear_begin = (uint16_t)clear_begin;
    l->clear_end = (uint16_t)clear_end;
    l->vram = vram;
    l->has_sprites = false;
    l->row_only = false;
    s_open = (int)line;
}

void Lufia2PpuObjDeferSliver(int x, unsigned vram_index, uint16_t z, bool flip, bool clip) {
    Sliver *s = &s_pool[s_used++];
    s->x = (int16_t)x;
    s->index = (uint16_t)vram_index;
    s->z = z;
    s->flags = (uint8_t)((flip ? SLIVER_FLIP : 0) | (clip ? SLIVER_CLIP : 0));
    /* Words past VRAM are read now, as the original evaluator does. */
    if (vram_index > 0x7ff7u) {
        const uint16_t *addr = s_lines[s_open].vram + vram_index;
        s->low = addr[0];
        s->high = addr[8];
        s->flags |= SLIVER_WORDS;
    }
}

bool Lufia2PpuObjDeferLineFits(unsigned line) {
    return line < ROWS && line == s_published && POOL_SLIVERS - s_used >= LINE_SLIVERS;
}

bool Lufia2PpuObjDeferTakeLine(unsigned line) {
    return lufia2_ppu_obj_defer_ppu && s_open == (int)line;
}

void Lufia2PpuObjDeferPublish(unsigned y, bool has_sprites) {
    Line *l = &s_lines[y];
    l->has_sprites = has_sprites;
    l->count = s_used - l->first;
    s_open = -1;
    __sync_synchronize();
    s_published = y + 1u;
    Signal();
}

uint64_t Lufia2PpuObjDeferRows(void) { return s_rows; }

#if defined(LUFIA2_OBJ_DEFER_CHECK)
static uint64_t s_checked, s_mismatched;

void Lufia2PpuObjDeferCheck(unsigned y, bool has_sprites, const PpuPixelPrioBufs *expected) {
    s_lines[y].row_only = true;
    Lufia2PpuObjDeferPublish(y, has_sprites);
    while (s_rows_done <= y) __sync_synchronize();
    ++s_checked;
    if (memcmp(&s_row, expected, sizeof s_row)) ++s_mismatched;
}

void Lufia2PpuObjDeferCheckStats(uint64_t *checked, uint64_t *mismatched) {
    *checked = s_checked;
    *mismatched = s_mismatched;
}
#endif
