#ifndef LUFIA2_BATTLE_UI_H
#define LUFIA2_BATTLE_UI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    LUFIA2_BATTLE_UI_CARDS = 4,
    LUFIA2_BATTLE_UI_CARD_WIDTH = 64,
    LUFIA2_BATTLE_UI_CARD_HEIGHT = 48,
    LUFIA2_BATTLE_UI_LAYOUT_BYTES = 72,
    LUFIA2_BATTLE_UI_HEIGHT = 224,
};

typedef struct Lufia2BattleUiCardLayout {
    uint16_t anchor;
    int16_t offset;
} Lufia2BattleUiCardLayout;

typedef struct Lufia2BattleUiLayout {
    Lufia2BattleUiCardLayout cards[LUFIA2_BATTLE_UI_CARDS];
} Lufia2BattleUiLayout;

bool Lufia2BattleUiDecode(const uint8_t *data, size_t size,
                         Lufia2BattleUiLayout *layout);
int Lufia2BattleUiCardX(const Lufia2BattleUiLayout *layout,
                       unsigned card, unsigned width);
void Lufia2BattleUiDraw(const Lufia2BattleUiLayout *layout,
                       const uint32_t *cards, uint32_t *pixels,
                       unsigned width, unsigned height, unsigned top);
void Lufia2BattleUiInit(const char *layout_path, const char *preview_path);
void Lufia2BattleUiBegin(bool enabled, unsigned width, unsigned height);
bool Lufia2BattleUiActive(void);
bool Lufia2BattleUiCapturing(void);
void Lufia2BattleUiRecord(unsigned row, unsigned line,
                         const uint32_t cards[LUFIA2_BATTLE_UI_CARDS][LUFIA2_BATTLE_UI_CARD_WIDTH],
                         const uint32_t original[256],
                         const uint8_t original_mask[256]);
void Lufia2BattleUiFinish(uint8_t *pixels, size_t pitch);
void Lufia2BattleUiCompose(uint8_t *pixels, unsigned width,
                          unsigned height, bool authoritative);
void Lufia2BattleUiReset(void);

#endif
