/* Lufia II save-state slot browser. */
#include "lufia2_savestate_menu.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "common_rtl.h"
#include "config.h"
#include "cpu_state.h"
#include "desktop/sdl_compat.h"
#include "lufia2_map_names.h"
#include "lufia2_overlay_ui.h"
#include "snes/interp_bridge.h"

extern uint8_t g_ram[0x20000];

enum {
    PAD_B      = 0x001,
    PAD_SELECT = 0x004,
    PAD_UP     = 0x010,
    PAD_DOWN   = 0x020,
    PAD_A      = 0x100,
    PAD_X      = 0x200,
    PAD_R      = 0x800,

    OPEN_GESTURE = PAD_SELECT | PAD_R,

    REPEAT_DELAY_MS = 350,
    REPEAT_RATE_MS  = 90,

    WRAM_CURRENT_MAP = 0x05ac,
    /* The save menu's chosen file; only bank $02 writes it. */
    WRAM_GAME_SAVE_FILE = 0x14b3,

    /* Save and load dispatches, all in interpreted banks. */
    DISPATCH_B05_SAVE = 0x0583ca,
    DISPATCH_B05_LOAD = 0x0583dd,
    DISPATCH_B02_LOAD = 0x02ead5,
    DISPATCH_B02_SAVE = 0x02eb61,
    /* The title screen reads a file here just to describe it. */
    DISPATCH_B02_PREVIEW = 0x02f001,

    /* $00:9040 checks four; the player is shown three. */
    GAME_SAVE_FILES = 4,

    /* Cleared once, so old unbound states cannot linger. */
    LEGACY_SLOT_SCAN = 20,
};

#define THUMB_W LUFIA2_SAVESTATE_THUMB_W
#define THUMB_H LUFIA2_SAVESTATE_THUMB_H
#define THUMB_PIXELS ((size_t)THUMB_W * THUMB_H)

/* Guest frames between thumbnail captures. */
#define THUMB_INTERVAL 6

#define THUMB_MAGIC 0x4854324Cu /* "L2TH" */
#define META_MAGIC  0x444D324Cu /* "L2MD" */
#define META_VERSION 1u

/* On-disk layout. Append only; a short read zero-fills the rest. */
typedef struct MetaBlob {
    uint32_t magic;
    uint32_t version;
    uint32_t map_id;
    uint32_t reserved[4];
} MetaBlob;

static Lufia2SavestateSlot s_slots[LUFIA2_SAVESTATE_SLOTS];
static uint32_t s_thumbs[LUFIA2_SAVESTATE_SLOTS][THUMB_PIXELS];
static bool s_open;
static bool s_scanned;
static int s_selected;
static char s_status[48];

static uint32_t s_live_thumb[THUMB_PIXELS];
static bool s_have_live_thumb;
static int s_thumb_countdown;

static uint32_t s_previous_inputs;
static uint32_t s_repeat_next;
static int s_repeat_dir;

static int s_game_file = -1;
/* Named by any transfer, unlocked only by a confirmed one. */
static bool s_bound;
static bool s_installed;

/* ── Slot files ─────────────────────────────────────────────────────────── */

/* One file per (game save file, state slot). */
static int slot_index(int slot) {
    const int file = s_game_file < 0 ? 0 : s_game_file;
    return file * LUFIA2_SAVESTATE_SLOTS + slot;
}

static void slot_path(int slot, const char *suffix, char *buf, size_t cap) {
    char sav[256];
    RtlSaveSlotPath(slot_index(slot), sav, sizeof(sav));
    snprintf(buf, cap, "%s%s", sav, suffix);
}

static bool file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

static bool read_thumb(int slot, uint32_t *out) {
    char path[288];
    uint32_t header[3];

    slot_path(slot, ".thumb", path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    const bool ok = fread(header, sizeof(uint32_t), 3, f) == 3 &&
        header[0] == THUMB_MAGIC &&
        header[1] == (uint32_t)THUMB_W &&
        header[2] == (uint32_t)THUMB_H &&
        fread(out, sizeof(uint32_t), THUMB_PIXELS, f) == THUMB_PIXELS;
    fclose(f);
    return ok;
}

static void write_thumb(int slot, const uint32_t *pixels) {
    char path[288];
    const uint32_t header[3] = {
        THUMB_MAGIC, (uint32_t)THUMB_W, (uint32_t)THUMB_H,
    };

    slot_path(slot, ".thumb", path, sizeof(path));
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(header, sizeof(uint32_t), 3, f);
    fwrite(pixels, sizeof(uint32_t), THUMB_PIXELS, f);
    fclose(f);
}

static bool read_meta(int slot, MetaBlob *out) {
    char path[288];

    slot_path(slot, ".meta", path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    memset(out, 0, sizeof(*out));
    const size_t got = fread(out, 1, sizeof(*out), f);
    fclose(f);
    return got >= sizeof(uint32_t) * 3 &&
        out->magic == META_MAGIC && out->version >= 1u;
}

static void write_meta(int slot) {
    char path[288];
    MetaBlob blob;

    memset(&blob, 0, sizeof(blob));
    blob.magic = META_MAGIC;
    blob.version = META_VERSION;
    blob.map_id = g_ram[WRAM_CURRENT_MAP];

    slot_path(slot, ".meta", path, sizeof(path));
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(&blob, 1, sizeof(blob), f);
    fclose(f);
}

/* Relative while it stays useful, absolute once it does not. */
static void format_when(const char *path, char *out, size_t cap) {
    struct stat st;
    struct tm broken;
    char clock[8];

    out[0] = '\0';
    if (stat(path, &st) != 0) return;
#ifdef _WIN32
    if (localtime_s(&broken, &st.st_mtime) != 0) return;
#else
    if (!localtime_r(&st.st_mtime, &broken)) return;
#endif

    double age = difftime(time(NULL), st.st_mtime);
    if (age < 0.0) age = 0.0;

    if (age >= 24.0 * 3600.0) {
        strftime(out, cap, "%Y-%m-%d %H:%M", &broken);
        return;
    }

    strftime(clock, sizeof(clock), "%H:%M", &broken);
    if (age < 60.0)
        snprintf(out, cap, "JUST NOW  %s", clock);
    else if (age < 3600.0)
        snprintf(out, cap, "%d MIN AGO  %s", (int)(age / 60.0), clock);
    else
        snprintf(out, cap, "%d H AGO  %s", (int)(age / 3600.0), clock);
}

static void rescan(void) {
    for (int i = 0; i < LUFIA2_SAVESTATE_SLOTS; i++) {
        Lufia2SavestateSlot *slot = &s_slots[i];
        char path[288];
        MetaBlob blob;

        memset(slot, 0, sizeof(*slot));
        slot_path(i, "", path, sizeof(path));
        slot->used = file_exists(path);
        if (!slot->used) continue;

        format_when(path, slot->when, sizeof(slot->when));
        if (read_thumb(i, s_thumbs[i]))
            slot->thumbnail = s_thumbs[i];
        if (read_meta(i, &blob)) {
            slot->has_meta = true;
            slot->map_id = blob.map_id;
            slot->map_name = Lufia2MapName(blob.map_id);
        }
    }
    s_scanned = true;
}

/* ── Selection ──────────────────────────────────────────────────────────── */

static void set_status(const char *format, int slot) {
    snprintf(s_status, sizeof(s_status), format, slot);
}

static void move_selection(int delta) {
    s_selected = (s_selected + delta + LUFIA2_SAVESTATE_SLOTS) %
                 LUFIA2_SAVESTATE_SLOTS;
    s_status[0] = '\0';
}

static void submit(bool save) {
    char path[288];

    RtlEnsureSaveDir();
    slot_path(s_selected, "", path, sizeof(path));

    if (save) {
        if (!RtlSaveSnapshot(path)) {
            set_status("SAVE FAILED: SLOT %02d", s_selected + 1);
            return;
        }
        if (s_have_live_thumb)
            write_thumb(s_selected, s_live_thumb);
        write_meta(s_selected);
        rescan();
        /* Stays open: the new row is the only proof it was written. */
        set_status("SAVED SLOT %02d", s_selected + 1);
        Lufia2OverlayUiPushSlotSaved(s_selected);
        return;
    }

    if (!s_slots[s_selected].used) {
        set_status("SLOT %02d IS EMPTY", s_selected + 1);
        Lufia2OverlayUiPushSlotEmpty(s_selected);
        return;
    }
    if (!RtlLoadSnapshot(path)) {
        set_status("LOAD FAILED: SLOT %02d", s_selected + 1);
        return;
    }
    Lufia2OverlayUiPushSlotLoaded(s_selected);
    Lufia2SavestateMenuClose();
}

/* ── Binding to the game's save file ────────────────────────────────────── */

/* Removes state files from before states were bound to a save file. */
static void clear_unbound_states(void) {
    static const char *const kSuffix[] = { "", ".thumb", ".meta" };
    char marker[288];
    char sav[256];
    int removed = 0;

    RtlSaveSlotPath(0, sav, sizeof(sav));
    snprintf(marker, sizeof(marker), "%s.bound", sav);
    if (file_exists(marker)) return;

    for (int i = 0; i < LEGACY_SLOT_SCAN; i++) {
        RtlSaveSlotPath(i, sav, sizeof(sav));
        for (size_t s = 0; s < sizeof kSuffix / sizeof kSuffix[0]; s++) {
            char path[288];
            snprintf(path, sizeof(path), "%s%s", sav, kSuffix[s]);
            if (file_exists(path) && remove(path) == 0) removed++;
        }
    }
    if (removed)
        fprintf(stderr, "[savestate] removed %d unbound state file(s).\n",
                removed);

    FILE *f = fopen(marker, "wb");
    if (f) fclose(f);
}

static void on_slot_transfer(CpuState *cpu, uint32_t pc24) {
    (void)cpu;
    int file;

    /* Bank $05 holds the file in direct page $03, whose D we cannot
     * assume; $05:83C0 has just mirrored it into the SRAM header. */
    if ((pc24 & 0x7F0000u) == 0x050000u) {
        if (!g_sram || g_sram_size < 1) return;
        file = g_sram[0] & (GAME_SAVE_FILES - 1);
    } else {
        file = g_ram[WRAM_GAME_SAVE_FILE] & (GAME_SAVE_FILES - 1);
    }

    /* A preview means the file select, not a game: it also unbinds. */
    s_bound = (pc24 & 0x7FFFFFu) != (uint32_t)DISPATCH_B02_PREVIEW;

    if (file == s_game_file) return;
    s_game_file = file;
    s_scanned = false;
}

void Lufia2SavestateMenuInstall(void) {
    static const uint32_t kDispatch[] = {
        DISPATCH_B05_SAVE, DISPATCH_B05_LOAD,
        DISPATCH_B02_LOAD, DISPATCH_B02_SAVE, DISPATCH_B02_PREVIEW,
    };
    if (s_installed) return;
    s_installed = true;

    for (size_t i = 0; i < sizeof kDispatch / sizeof kDispatch[0]; i++)
        interp_bridge_set_pre_opcode_hook(kDispatch[i], on_slot_transfer);

    RtlEnsureSaveDir();
    clear_unbound_states();
}

/* ── Host interface ─────────────────────────────────────────────────────── */

int Lufia2SavestateMenuGameSlot(void) {
    return s_game_file;
}

bool Lufia2SavestateMenuIsOpen(void) {
    return s_open;
}

bool Lufia2SavestateMenuOpen(void) {
    if (!s_bound || s_game_file < 0) return false;
    s_open = true;
    s_status[0] = '\0';
    s_repeat_dir = 0;
    rescan();
    return true;
}

void Lufia2SavestateMenuClose(void) {
    s_open = false;
    s_repeat_dir = 0;
    s_status[0] = '\0';
}

bool Lufia2SavestateMenuGesturePressed(uint32_t inputs) {
    const uint32_t previous = s_previous_inputs;
    s_previous_inputs = inputs;
    if (s_open) return false;
    if ((inputs & OPEN_GESTURE) != OPEN_GESTURE) return false;
    return (previous & OPEN_GESTURE) != OPEN_GESTURE;
}

void Lufia2SavestateMenuPollNav(uint32_t inputs, uint32_t ticks_ms) {
    const uint32_t previous = s_previous_inputs;
    s_previous_inputs = inputs;
    if (!s_open) return;

    const uint32_t pressed = inputs & ~previous;

    /* The open gesture is its own toggle. */
    if ((pressed & OPEN_GESTURE) &&
        (inputs & OPEN_GESTURE) == OPEN_GESTURE) {
        Lufia2SavestateMenuClose();
        return;
    }
    if (pressed & PAD_B) {
        Lufia2SavestateMenuClose();
        return;
    }
    if (pressed & PAD_X) {
        submit(true);
        return;
    }
    if (pressed & PAD_A) {
        submit(false);
        return;
    }

    const int direction = (inputs & PAD_UP) ? -1
        : (inputs & PAD_DOWN) ? +1 : 0;
    if (direction == 0) {
        s_repeat_dir = 0;
        return;
    }
    if (direction != s_repeat_dir) {
        move_selection(direction);
        s_repeat_dir = direction;
        s_repeat_next = ticks_ms + REPEAT_DELAY_MS;
    } else if ((int32_t)(ticks_ms - s_repeat_next) >= 0) {
        move_selection(direction);
        s_repeat_next = ticks_ms + REPEAT_RATE_MS;
    }
}

void Lufia2SavestateMenuHandleKey(int key, int repeat) {
    if (!s_open || repeat) return;
    /* No Up/Down: keyboard input also arrives as pad bits through
     * PollNav, and handling both moved the selection twice per press. */
    switch (key) {
    case SDLK_RETURN: submit(false); break;
    case SDLK_s:      submit(true); break;
    case SDLK_ESCAPE: Lufia2SavestateMenuClose(); break;
    default: break;
    }
}

void Lufia2SavestateMenuNoteFrame(
    const uint32_t *pixels, int width, int height) {
    if (s_open || !pixels || width <= 0 || height <= 0) return;

    /* A whole-frame average is not worth paying for every frame. */
    if (++s_thumb_countdown < THUMB_INTERVAL) return;
    s_thumb_countdown = 0;

    for (int y = 0; y < THUMB_H; y++) {
        const int y0 = y * height / THUMB_H;
        int y1 = (y + 1) * height / THUMB_H;
        if (y1 <= y0) y1 = y0 + 1;
        if (y1 > height) y1 = height;

        for (int x = 0; x < THUMB_W; x++) {
            const int x0 = x * width / THUMB_W;
            int x1 = (x + 1) * width / THUMB_W;
            if (x1 <= x0) x1 = x0 + 1;
            if (x1 > width) x1 = width;

            unsigned r = 0, g = 0, b = 0, n = 0;
            for (int sy = y0; sy < y1; sy++) {
                const uint32_t *row = pixels + (size_t)sy * width;
                for (int sx = x0; sx < x1; sx++) {
                    r += (row[sx] >> 16) & 0xFFu;
                    g += (row[sx] >> 8) & 0xFFu;
                    b += row[sx] & 0xFFu;
                    n++;
                }
            }
            if (!n) n = 1;
            /* The PPU leaves the top byte clear; force it opaque. */
            s_live_thumb[(size_t)y * THUMB_W + x] =
                0xFF000000u | ((r / n) << 16) | ((g / n) << 8) | (b / n);
        }
    }
    s_have_live_thumb = true;
}

int Lufia2SavestateMenuSelected(void) {
    return s_selected;
}

const Lufia2SavestateSlot *Lufia2SavestateMenuSlot(int index) {
    if (index < 0 || index >= LUFIA2_SAVESTATE_SLOTS) return NULL;
    if (!s_scanned) rescan();
    return &s_slots[index];
}

const char *Lufia2SavestateMenuStatus(void) {
    return s_status;
}

void Lufia2SavestateMenuShutdown(void) {
    s_open = false;
    s_scanned = false;
    s_have_live_thumb = false;
    s_status[0] = '\0';
    memset(s_slots, 0, sizeof(s_slots));
}
