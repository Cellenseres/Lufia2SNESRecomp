#include <stdbool.h>
#include <stdint.h>

#include "common_cpu_infra.h"
#include "cpu_state.h"
#include "funcs.h"
#include "lufia2_map_widescreen.h"
#include "widescreen.h"

enum {
    LUFIA2_MAP_ACTOR_COUNT = 72,
    LUFIA2_ACTOR_FLAGS = 0x0622,
    LUFIA2_ACTOR_SORT_KEYS = 0xe200,
    LUFIA2_ACTOR_SORT_IDS = 0xe300,
    LUFIA2_ACTOR_X = 0xddae,
    LUFIA2_ACTOR_Y = 0xde3e,
    LUFIA2_ACTOR_STATE = 0xe316,
    LUFIA2_NATIVE_ACTOR_LEFT = 0x20,
    LUFIA2_NATIVE_ACTOR_RIGHT = 0x130,
    LUFIA2_NATIVE_ACTOR_TOP = 0x10,
    LUFIA2_NATIVE_ACTOR_BOTTOM = 0x110,
};

static void WriteDirect16(CpuState *cpu, uint8_t offset, uint16_t value) {
    cpu_write16(cpu, 0, (uint16_t)(cpu->D + offset), value);
}

static bool IsBefore(uint16_t value, uint16_t boundary) {
    return (int16_t)(value - boundary) < 0;
}

static bool ActorIsInsideViewport(
    CpuState *cpu,
    uint8_t actor,
    uint16_t left,
    uint16_t right,
    uint16_t top,
    uint16_t bottom) {
    if ((cpu_read8(cpu, 0, LUFIA2_ACTOR_FLAGS + actor) & 0x04) != 0 ||
        (cpu_read8(cpu, 0x7f, LUFIA2_ACTOR_STATE + actor) & 0x80) != 0) {
        return false;
    }

    const uint16_t actor_offset = (uint16_t)actor * 2;
    const uint16_t world_x =
        cpu_read16(cpu, 0x7f, LUFIA2_ACTOR_X + actor_offset);
    const uint16_t world_y =
        cpu_read16(cpu, 0x7f, LUFIA2_ACTOR_Y + actor_offset);
    if (!Lufia2MapWidescreenWorldPointIsVisible(world_x, world_y))
        return false;

    const uint16_t x = (uint16_t)(world_x + 0x30);
    if (IsBefore(x, left) || !IsBefore(x, right))
        return false;

    const uint16_t y = (uint16_t)(world_y + 0x20);
    return !IsBefore(y, top) && IsBefore(y, bottom);
}

static uint16_t ActorSortKey(CpuState *cpu, uint8_t actor) {
    const uint16_t actor_offset = (uint16_t)actor * 2;
    uint16_t key = (uint16_t)(
        cpu_read16(cpu, 0x7f, LUFIA2_ACTOR_Y + actor_offset) + 0x20);
    key |= 0x1000;

    const uint8_t state =
        cpu_read8(cpu, 0x7f, LUFIA2_ACTOR_STATE + actor);
    if ((state & 0x01) != 0)
        key &= (uint16_t)~0x1000;
    else if ((state & 0x02) != 0)
        key |= 0x2000;
    return key;
}

static uint8_t CollectMapActors(
    CpuState *cpu,
    uint16_t left,
    uint16_t right,
    uint16_t top,
    uint16_t bottom) {
    uint8_t byte_count = 0;

    for (uint8_t actor = 0; actor < LUFIA2_MAP_ACTOR_COUNT; actor++) {
        if (!ActorIsInsideViewport(
                cpu, actor, left, right, top, bottom)) {
            continue;
        }

        const uint16_t key = ActorSortKey(cpu, actor);
        uint8_t insert = byte_count;
        while (insert != 0) {
            const uint8_t previous = (uint8_t)(insert - 2);
            const uint16_t previous_key =
                cpu_read16(cpu, 0x7e, LUFIA2_ACTOR_SORT_KEYS + previous);
            if (previous_key >= key)
                break;

            cpu_write16(
                cpu, 0x7e, LUFIA2_ACTOR_SORT_KEYS + insert,
                previous_key);
            cpu_write16(
                cpu, 0x7e, LUFIA2_ACTOR_SORT_IDS + insert,
                cpu_read16(
                    cpu, 0x7e, LUFIA2_ACTOR_SORT_IDS + previous));
            insert = previous;
        }

        cpu_write16(cpu, 0x7e, LUFIA2_ACTOR_SORT_KEYS + insert, key);
        cpu_write16(cpu, 0x7e, LUFIA2_ACTOR_SORT_IDS + insert, actor);
        byte_count = (uint8_t)(byte_count + 2);
    }

    return byte_count;
}

static void ResetPreviousSpriteOutput(CpuState *cpu) {
    const uint8_t sprite_count = cpu_read8(cpu, 0x7f, 0xd0b0);
    if (sprite_count == 0xff)
        return;

    for (int offset = sprite_count * 4; offset >= 0; offset -= 4)
        cpu_write16(cpu, 0, (uint16_t)(0x0140 + offset), 0xf000);
    for (uint16_t address = 0x0302; address <= 0x0310; address += 2)
        cpu_write16(cpu, 0, address, 0);
}

RecompReturn HleLufia2CollectMapActors(CpuState *cpu) {
    const uint16_t entry_s = cpu->S;
    const uint8_t return_frame = cpu->host_return_valid;

    cpu_write8(cpu, 0, cpu->S, cpu->DB);
    cpu->S = (uint16_t)(cpu->S - 1);
    cpu->DB = 0x7e;

    ResetPreviousSpriteOutput(cpu);

    uint16_t camera_x;
    uint16_t camera_y;
    if ((cpu_read16(cpu, 0, 0x1261) & 0x0040) != 0) {
        camera_x = cpu_read16(cpu, 0x7f, 0xd0ee);
        camera_y = cpu_read16(cpu, 0x7f, 0xd0f0);
    } else {
        camera_x = cpu_read16(cpu, 0, 0x1220);
        camera_y = cpu_read16(cpu, 0, 0x1228);
    }

    int margin = 0;
    if (g_ws_active && g_ws_extra > 0 &&
        Lufia2MapWidescreenIsActive()) {
        margin = g_ws_extra;
    }

    const uint16_t left = (uint16_t)(
        camera_x + LUFIA2_NATIVE_ACTOR_LEFT - margin);
    const uint16_t right = (uint16_t)(
        camera_x + LUFIA2_NATIVE_ACTOR_RIGHT + margin);
    const uint16_t top =
        (uint16_t)(camera_y + LUFIA2_NATIVE_ACTOR_TOP);
    const uint16_t bottom =
        (uint16_t)(camera_y + LUFIA2_NATIVE_ACTOR_BOTTOM);

    WriteDirect16(cpu, 0x9f, camera_x);
    WriteDirect16(cpu, 0xa1, camera_y);

    cpu->Y = CollectMapActors(cpu, left, right, top, bottom);
    cpu->X = LUFIA2_MAP_ACTOR_COUNT;
    cpu->m_flag = 1;
    cpu->x_flag = 1;
    cpu_mirrors_to_p(cpu);
    cpu->PB = 0x83;

    cpu_tailcall_inherit_return_context(entry_s, return_frame);
    return Lufia2MapActorContinuation_M1X1(cpu);
}
