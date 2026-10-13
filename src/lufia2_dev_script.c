#include "lufia2_dev_script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    DEV_MAX_COMMANDS = 1024,
    DEV_PATH = 512,
    DEV_NO_FRAME = 0xffffffffu,
};

typedef enum DevKind {
    DEV_PRESS,
    DEV_MASH,
    DEV_SHOT,
    DEV_HDSHOT,
    DEV_LAYERS,
    DEV_WRAM,
    DEV_PPU,
    DEV_POKE,
    DEV_QUIT,
} DevKind;

typedef struct DevCommand {
    DevKind kind;
    /* Offset from the battle anchor when `battle` is set. */
    uint32_t frame;
    bool battle;
    uint32_t buttons;
    uint32_t count;      /* press: frames; mash: period */
    uint32_t until;      /* mash: last frame, same base as `frame` */
    uint32_t address;
    uint8_t value;
    char path[DEV_PATH];
} DevCommand;

static DevCommand s_commands[DEV_MAX_COMMANDS];
static unsigned s_command_count;
static bool s_active;
static uint32_t s_battle_frame = DEV_NO_FRAME;
static bool s_battle_visible;

static uint32_t ParseButtons(const char *text) {
    static const struct { const char *name; uint32_t bit; } kButtons[] = {
        {"B", 0x001}, {"Y", 0x002}, {"SELECT", 0x004}, {"START", 0x008},
        {"UP", 0x010}, {"DOWN", 0x020}, {"LEFT", 0x040}, {"RIGHT", 0x080},
        {"A", 0x100}, {"X", 0x200}, {"L", 0x400}, {"R", 0x800},
    };
    uint32_t bits = 0;
    char copy[128];
    snprintf(copy, sizeof copy, "%s", text);
    for (char *token = strtok(copy, "+"); token; token = strtok(NULL, "+")) {
        bool found = false;
        for (size_t i = 0; i < sizeof kButtons / sizeof kButtons[0]; i++) {
            if (!strcmp(token, kButtons[i].name)) {
                bits |= kButtons[i].bit;
                found = true;
            }
        }
        if (!found && strcmp(token, "NONE"))
            fprintf(stderr, "[dev-script] unknown button '%s'\n", token);
    }
    return bits;
}

/* Resolved frame of a command, or DEV_NO_FRAME before its anchor exists. */
static uint32_t CommandFrame(const DevCommand *command, uint32_t offset) {
    if (!command->battle)
        return offset;
    return s_battle_frame == DEV_NO_FRAME ? DEV_NO_FRAME
                                          : s_battle_frame + offset;
}

bool Lufia2DevScriptInit(void) {
    const char *path = getenv("LUFIA2_DEV_SCRIPT");
    if (!path || !*path)
        return false;
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "[dev-script] cannot open %s\n", path);
        return false;
    }

    char line[1024];
    uint32_t previous = 0;
    bool previous_battle = false;
    unsigned line_number = 0;
    while (fgets(line, sizeof line, file)) {
        line_number++;
        char *hash = strchr(line, '#');
        if (hash) *hash = '\0';
        char when[64], verb[32], a[DEV_PATH], b[64], c[64];
        a[0] = b[0] = c[0] = '\0';
        const int fields =
            sscanf(line, "%63s %31s %511s %63s %63s", when, verb, a, b, c);
        if (fields < 2)
            continue;
        if (s_command_count == DEV_MAX_COMMANDS) {
            fprintf(stderr, "[dev-script] too many commands\n");
            break;
        }

        DevCommand *command = &s_commands[s_command_count];
        memset(command, 0, sizeof *command);
        if (!strncmp(when, "battle+", 7)) {
            command->battle = true;
            command->frame = (uint32_t)strtoul(when + 7, NULL, 10);
        } else if (when[0] == '+') {
            command->battle = previous_battle;
            command->frame = previous + (uint32_t)strtoul(when + 1, NULL, 10);
        } else {
            command->frame = (uint32_t)strtoul(when, NULL, 10);
        }
        previous = command->frame;
        previous_battle = command->battle;

        if (!strcmp(verb, "press")) {
            command->kind = DEV_PRESS;
            command->buttons = ParseButtons(a);
            command->count = b[0] ? (uint32_t)strtoul(b, NULL, 10) : 4u;
        } else if (!strcmp(verb, "mash")) {
            command->kind = DEV_MASH;
            command->buttons = ParseButtons(a);
            command->count = b[0] ? (uint32_t)strtoul(b, NULL, 10) : 8u;
            if (command->count < 4u) command->count = 4u;
            command->until = c[0] ? (uint32_t)strtoul(c, NULL, 10)
                                  : command->frame;
        } else if (!strcmp(verb, "shot") || !strcmp(verb, "hdshot") ||
                   !strcmp(verb, "layers") || !strcmp(verb, "wram") ||
                   !strcmp(verb, "ppu")) {
            command->kind = verb[0] == 's' ? DEV_SHOT
                          : verb[0] == 'h' ? DEV_HDSHOT
                          : verb[0] == 'l' ? DEV_LAYERS
                          : verb[0] == 'w' ? DEV_WRAM : DEV_PPU;
            snprintf(command->path, sizeof command->path, "%s", a);
        } else if (!strcmp(verb, "poke")) {
            command->kind = DEV_POKE;
            command->address = (uint32_t)strtoul(a, NULL, 16);
            command->value = (uint8_t)strtoul(b, NULL, 16);
        } else if (!strcmp(verb, "quit")) {
            command->kind = DEV_QUIT;
        } else {
            fprintf(stderr, "[dev-script] %s:%u: unknown command '%s'\n",
                    path, line_number, verb);
            continue;
        }
        s_command_count++;
    }
    fclose(file);
    s_active = s_command_count != 0;
    fprintf(stderr, "[dev-script] %u commands from %s\n",
            s_command_count, path);
    return s_active;
}

bool Lufia2DevScriptActive(void) { return s_active; }

void Lufia2DevScriptNoteBattle(uint32_t frame, bool visible) {
    if (visible && !s_battle_visible) {
        s_battle_frame = frame;
        if (s_active)
            fprintf(stderr, "[dev-script] battle visible at frame %u\n",
                    frame);
    }
    s_battle_visible = visible;
}

uint32_t Lufia2DevScriptInput(uint32_t frame, uint32_t host_input) {
    if (!s_active)
        return host_input;
    uint32_t input = host_input;
    for (unsigned i = 0; i < s_command_count; i++) {
        const DevCommand *command = &s_commands[i];
        const uint32_t start = CommandFrame(command, command->frame);
        if (start == DEV_NO_FRAME || frame < start)
            continue;
        if (command->kind == DEV_PRESS) {
            if (frame < start + command->count)
                input |= command->buttons;
        } else if (command->kind == DEV_MASH) {
            const uint32_t until = CommandFrame(command, command->until);
            if (frame <= until && (frame - start) % command->count < 2u)
                input |= command->buttons;
        }
    }
    return input;
}

void Lufia2DevScriptPoke(uint32_t frame, uint8_t *wram, size_t size) {
    if (!s_active || !wram)
        return;
    for (unsigned i = 0; i < s_command_count; i++) {
        const DevCommand *command = &s_commands[i];
        const uint32_t start = CommandFrame(command, command->frame);
        if (command->kind == DEV_POKE && start != DEV_NO_FRAME &&
            frame >= start && command->address < size)
            wram[command->address] = command->value;
    }
}

const char *Lufia2DevScriptRequest(uint32_t frame, const char *verb) {
    if (!s_active)
        return NULL;
    const DevKind kind = !strcmp(verb, "hdshot") ? DEV_HDSHOT
                       : !strcmp(verb, "layers") ? DEV_LAYERS
                       : !strcmp(verb, "wram") ? DEV_WRAM
                       : !strcmp(verb, "ppu") ? DEV_PPU : DEV_SHOT;
    for (unsigned i = 0; i < s_command_count; i++) {
        const DevCommand *command = &s_commands[i];
        if (command->kind == kind &&
            CommandFrame(command, command->frame) == frame)
            return command->path;
    }
    return NULL;
}

void Lufia2DevScriptFrame(uint32_t frame, const uint8_t *argb,
                         unsigned width, unsigned height) {
    const char *path = Lufia2DevScriptRequest(frame, "shot");
    if (!path || !argb)
        return;
    if (Lufia2DevWritePng(path, argb, width, height, (size_t)width * 4,
                         false))
        fprintf(stderr, "[dev-script] frame %u -> %s\n", frame, path);
    else
        fprintf(stderr, "[dev-script] could not write %s\n", path);
}

bool Lufia2DevScriptQuit(uint32_t frame) {
    if (!s_active)
        return false;
    for (unsigned i = 0; i < s_command_count; i++) {
        const DevCommand *command = &s_commands[i];
        const uint32_t start = CommandFrame(command, command->frame);
        if (command->kind == DEV_QUIT && start != DEV_NO_FRAME &&
            frame >= start)
            return true;
    }
    return false;
}

/* Stored (uncompressed) deflate keeps this free of a zlib dependency; these
 * are development captures, not shipped assets. */
static uint32_t Crc32(uint32_t crc, const uint8_t *data, size_t size) {
    static uint32_t table[256];
    if (!table[1]) {
        for (uint32_t n = 0; n < 256; n++) {
            uint32_t c = n;
            for (int k = 0; k < 8; k++)
                c = c & 1u ? 0xedb88320u ^ (c >> 1) : c >> 1;
            table[n] = c;
        }
    }
    crc = ~crc;
    for (size_t i = 0; i < size; i++)
        crc = table[(crc ^ data[i]) & 0xffu] ^ (crc >> 8);
    return ~crc;
}

static void Put32(uint8_t *out, uint32_t value) {
    out[0] = (uint8_t)(value >> 24);
    out[1] = (uint8_t)(value >> 16);
    out[2] = (uint8_t)(value >> 8);
    out[3] = (uint8_t)value;
}

static bool WriteChunk(FILE *file, const char *type, const uint8_t *data,
                       size_t size) {
    uint8_t header[8], trailer[4];
    Put32(header, (uint32_t)size);
    memcpy(header + 4, type, 4);
    uint32_t crc = Crc32(0, header + 4, 4);
    if (size)
        crc = Crc32(crc, data, size);
    Put32(trailer, crc);
    return fwrite(header, 1, 8, file) == 8 &&
           (!size || fwrite(data, 1, size, file) == size) &&
           fwrite(trailer, 1, 4, file) == 4;
}

bool Lufia2DevWritePng(const char *path, const uint8_t *argb,
                      unsigned width, unsigned height, size_t pitch,
                      bool keep_alpha) {
    if (!path || !argb || !width || !height || width > 16384u ||
        height > 16384u || pitch < (size_t)width * 4)
        return false;
    const unsigned channels = keep_alpha ? 4u : 3u;
    const size_t row = (size_t)width * channels + 1u;
    const size_t raw = row * height;
    /* zlib header, one stored block per 65535 bytes, adler32. */
    const size_t blocks = (raw + 65534u) / 65535u;
    const size_t packed = 2u + raw + blocks * 5u + 4u;
    uint8_t *data = (uint8_t *)malloc(packed);
    uint8_t *scan = (uint8_t *)malloc(raw);
    if (!data || !scan) {
        free(data);
        free(scan);
        return false;
    }

    for (unsigned y = 0; y < height; y++) {
        const uint8_t *src = argb + (size_t)y * pitch;
        uint8_t *dst = scan + (size_t)y * row;
        *dst++ = 0;
        for (unsigned x = 0; x < width; x++, src += 4) {
            *dst++ = src[2];
            *dst++ = src[1];
            *dst++ = src[0];
            if (keep_alpha) *dst++ = src[3];
        }
    }

    uint32_t s1 = 1, s2 = 0;
    for (size_t i = 0; i < raw; i++) {
        s1 = (s1 + scan[i]) % 65521u;
        s2 = (s2 + s1) % 65521u;
    }
    uint8_t *out = data;
    *out++ = 0x78;
    *out++ = 0x01;
    for (size_t offset = 0; offset < raw;) {
        const size_t chunk = raw - offset > 65535u ? 65535u : raw - offset;
        *out++ = offset + chunk == raw ? 1 : 0;
        *out++ = (uint8_t)chunk;
        *out++ = (uint8_t)(chunk >> 8);
        *out++ = (uint8_t)~chunk;
        *out++ = (uint8_t)(~chunk >> 8);
        memcpy(out, scan + offset, chunk);
        out += chunk;
        offset += chunk;
    }
    Put32(out, (s2 << 16) | s1);

    static const uint8_t signature[8] = {137, 'P', 'N', 'G', 13, 10, 26, 10};
    uint8_t header[13];
    Put32(header, width);
    Put32(header + 4, height);
    header[8] = 8;
    header[9] = keep_alpha ? 6 : 2;
    header[10] = header[11] = header[12] = 0;

    FILE *file = fopen(path, "wb");
    bool ok = file != NULL;
    ok = ok && fwrite(signature, 1, 8, file) == 8;
    ok = ok && WriteChunk(file, "IHDR", header, sizeof header);
    ok = ok && WriteChunk(file, "IDAT", data, packed);
    ok = ok && WriteChunk(file, "IEND", NULL, 0);
    if (file)
        ok = fclose(file) == 0 && ok;
    free(data);
    free(scan);
    return ok;
}
