#ifndef LUFIA2_MENU_UI_H
#define LUFIA2_MENU_UI_H

#include "snes/ppu.h"

enum { LUFIA2_MENU_UI_MAX_OBJECTS = 2048, LUFIA2_MENU_UI_RECORD = 32 };

typedef struct Lufia2MenuUiObject {
    uint32_t id, parent;
    int16_t x, y;
    uint16_t width, height;
    uint8_t kind, slot;
} Lufia2MenuUiObject;

void Lufia2MenuUiInit(const uint8_t *rom, size_t size,
                     const char *layout, const char *preview);
void Lufia2MenuUiPrepare(Ppu *ppu, bool enabled, unsigned width);
void Lufia2MenuUiLatchSprites(const Ppu *ppu);
void Lufia2MenuUiSpritePixel(int x, unsigned oam);
void Lufia2MenuUiPlane(Ppu *ppu, unsigned layer, unsigned line);
typedef void Lufia2MenuUiRenderer(Ppu *ppu, unsigned line);
void Lufia2MenuUiLine(Ppu *ppu, unsigned line, Lufia2MenuUiRenderer *draw);
void Lufia2MenuUiCompose(uint8_t *pixels, unsigned width,
                        unsigned height, bool authoritative);
void Lufia2MenuUiReset(Ppu *ppu);
bool Lufia2MenuUiLoad(const uint8_t *bytes, size_t size);
unsigned Lufia2MenuUiObjects(const Lufia2MenuUiObject **objects, uint32_t *scene);

#endif
