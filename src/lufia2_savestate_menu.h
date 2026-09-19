#ifndef LUFIA2_SAVESTATE_MENU_H
#define LUFIA2_SAVESTATE_MENU_H

#include <stdbool.h>
#include <stdint.h>

#define LUFIA2_SAVESTATE_SLOTS 3

/* Four times the panel cell, so drawing is a clean box average. */
#define LUFIA2_SAVESTATE_THUMB_W 192
#define LUFIA2_SAVESTATE_THUMB_H 168

/* Read from sidecars, so the browser never loads a state to describe it. */
typedef struct Lufia2SavestateSlot {
    bool used;
    bool has_meta;
    /* "12 MIN AGO  14:24", or a full date past a day. */
    char when[24];
    unsigned map_id;
    /* NULL for the 21 ids the ROM leaves blank. */
    const char *map_name;
    const uint32_t *thumbnail;
} Lufia2SavestateSlot;

/* Hooks the game's save dispatches and clears pre-binding state files. */
void Lufia2SavestateMenuInstall(void);

/* The game save file states are bound to, or -1 before one is loaded. */
int Lufia2SavestateMenuGameSlot(void);

bool Lufia2SavestateMenuIsOpen(void);
/* False until a game save is loaded; the menu stays shut. */
bool Lufia2SavestateMenuOpen(void);
void Lufia2SavestateMenuClose(void);

/* Edge on Select + R, so either button may be held first. */
bool Lufia2SavestateMenuGesturePressed(uint32_t inputs);

void Lufia2SavestateMenuPollNav(uint32_t inputs, uint32_t ticks_ms);
void Lufia2SavestateMenuHandleKey(int key, int repeat);

/* Composed game pixels; ignored while the menu is open. */
void Lufia2SavestateMenuNoteFrame(
    const uint32_t *pixels, int width, int height);

int Lufia2SavestateMenuSelected(void);
const Lufia2SavestateSlot *Lufia2SavestateMenuSlot(int index);
const char *Lufia2SavestateMenuStatus(void);

void Lufia2SavestateMenuShutdown(void);

#endif
