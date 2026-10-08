#include "lufia2_sound_menu.h"

#include <string.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"

enum {
    DISPATCH_TARGET_STORE = 0x82803d,
    CONFIG_LEFT_TABLE_RETURN = 0xbf51,
    MUSIC_TABLE_OFFSET = 7,
    WRONG_MUSIC_TARGET = 0xb3ad,
    MUSIC_TARGET = 0xbfad,
    TABLE_POINTER = 0x5d,
    TABLE_BANK = 0x5f,
};

static bool s_fix_enabled = true;
static bool s_supported_rom;

bool Lufia2SoundMenuFixEnabled(void) {
    return s_fix_enabled;
}

void Lufia2SoundMenuSetFixEnabled(bool enabled) {
    s_fix_enabled = enabled;
}

static void FixMusicTarget(CpuState *cpu, uint32_t pc24) {
    if (!s_fix_enabled || !s_supported_rom || !cpu->ram ||
        (pc24 & 0x7fffffu) != (DISPATCH_TARGET_STORE & 0x7fffffu) ||
        (cpu->PB & 0x7fu) != 2u || cpu->emulation ||
        cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->Y != MUSIC_TABLE_OFFSET || cpu->A != WRONG_MUSIC_TARGET ||
        cpu->ram[TABLE_POINTER] != (CONFIG_LEFT_TABLE_RETURN & 0xffu) ||
        cpu->ram[TABLE_POINTER + 1u] != (CONFIG_LEFT_TABLE_RETURN >> 8) ||
        cpu->ram[TABLE_BANK] != cpu->PB)
        return;

    /* Both targets have identical N/Z flags. */
    cpu->A = MUSIC_TARGET;
}

bool Lufia2SoundMenuInstall(const uint8_t *rom, size_t size) {
    static const uint8_t left_table[] = {
        0xad, 0xb3, 0x14, 0x20, 0x28, 0x80,
        0x79, 0xbf, 0x99, 0xbf, 0xa3, 0xbf, 0xad, 0xb3, 0x14,
    };
    static const uint8_t dispatch[] = {
        0xc2, 0x20, 0x29, 0xff, 0x00, 0x0a, 0x1a, 0xa8,
        0x68, 0x85, 0x5d, 0xe2, 0x20, 0x4b, 0x68, 0x85,
        0x5f, 0xc2, 0x20, 0xb7, 0x5d, 0x85, 0x60, 0xe2,
        0x20, 0x6c, 0x60, 0x00,
    };
    s_supported_rom = rom && size == 2621440u &&
        memcmp(rom + 0x13f4c, left_table, sizeof(left_table)) == 0 &&
        memcmp(rom + 0x10028, dispatch, sizeof(dispatch)) == 0;
    if (s_supported_rom)
        interp_bridge_set_pre_opcode_hook(DISPATCH_TARGET_STORE, FixMusicTarget);
    return s_supported_rom;
}
