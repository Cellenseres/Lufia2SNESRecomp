#include "lufia2_room_inflate.h"

#include <string.h>

typedef struct RoomBits {
    const uint8_t *data;
    size_t size;
    size_t at;
    uint32_t bits;
    unsigned count;
    bool failed;
} RoomBits;

typedef struct RoomHuffman {
    uint16_t count[16];
    uint16_t symbols[288];
} RoomHuffman;

static unsigned TakeBits(RoomBits *reader, unsigned count) {
    while (reader->count < count) {
        if (reader->at == reader->size) {
            reader->failed = true;
            return 0;
        }
        reader->bits |= (uint32_t)reader->data[reader->at++] << reader->count;
        reader->count += 8;
    }
    const unsigned value = reader->bits & ((1u << count) - 1u);
    reader->bits >>= count;
    reader->count -= count;
    return value;
}

/* Incomplete trees are valid only for an empty distance alphabet or one code. */
static bool BuildHuffman(RoomHuffman *tree, const uint8_t *lengths,
                         unsigned count, bool code_lengths) {
    uint16_t offsets[16];
    memset(tree->count, 0, sizeof(tree->count));
    for (unsigned i = 0; i < count; i++) {
        if (lengths[i] > 15)
            return false;
        tree->count[lengths[i]]++;
    }
    int available = 1;
    for (unsigned length = 1; length <= 15; length++) {
        available = available * 2 - tree->count[length];
        if (available < 0)
            return false;
    }
    if (available && (code_lengths || count - tree->count[0] > 1 ||
                      (count != tree->count[0] && tree->count[1] != 1)))
        return false;
    offsets[1] = 0;
    for (unsigned length = 1; length < 15; length++)
        offsets[length + 1] = offsets[length] + tree->count[length];
    for (unsigned i = 0; i < count; i++) {
        if (lengths[i])
            tree->symbols[offsets[lengths[i]]++] = (uint16_t)i;
    }
    return true;
}

static int ReadSymbol(RoomBits *reader, const RoomHuffman *tree) {
    unsigned code = 0, first = 0, index = 0;
    for (unsigned length = 1; length <= 15; length++) {
        code = (code << 1) | TakeBits(reader, 1);
        const unsigned count = tree->count[length];
        if (reader->failed)
            return -1;
        if (code >= first && code - first < count)
            return tree->symbols[index + code - first];
        index += count;
        first = (first + count) << 1;
    }
    reader->failed = true;
    return -1;
}

static bool ReadTrees(RoomBits *reader, unsigned type,
                      RoomHuffman *literal, RoomHuffman *distance) {
    uint8_t lengths[320];
    if (type == 1) {
        for (unsigned i = 0; i < 288; i++)
            lengths[i] = (uint8_t)(i < 144 ? 8 : i < 256 ? 9 : i < 280 ? 7 : 8);
        if (!BuildHuffman(literal, lengths, 288, false))
            return false;
        memset(lengths, 5, 32);
        return BuildHuffman(distance, lengths, 32, false);
    }
    static const uint8_t order[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };
    const unsigned literals = TakeBits(reader, 5) + 257;
    const unsigned distances = TakeBits(reader, 5) + 1;
    const unsigned codes = TakeBits(reader, 4) + 4;
    if (literals > 286 || reader->failed)
        return false;
    uint8_t code_lengths[19] = {0};
    for (unsigned i = 0; i < codes; i++)
        code_lengths[order[i]] = (uint8_t)TakeBits(reader, 3);
    RoomHuffman alphabet;
    if (reader->failed || !BuildHuffman(&alphabet, code_lengths, 19, true))
        return false;
    unsigned at = 0;
    while (at < literals + distances) {
        const int symbol = ReadSymbol(reader, &alphabet);
        if (symbol < 0)
            return false;
        if (symbol < 16) {
            lengths[at++] = (uint8_t)symbol;
            continue;
        }
        if (symbol == 16 && at == 0)
            return false;
        const unsigned repeat = symbol == 16 ? TakeBits(reader, 2) + 3 :
            symbol == 17 ? TakeBits(reader, 3) + 3 : TakeBits(reader, 7) + 11;
        const uint8_t value = symbol == 16 ? lengths[at - 1] : 0;
        if (reader->failed || repeat > literals + distances - at)
            return false;
        memset(lengths + at, value, repeat);
        at += repeat;
    }
    return lengths[256] && BuildHuffman(literal, lengths, literals, false) &&
        BuildHuffman(distance, lengths + literals, distances, false);
}

bool Lufia2RoomInflate(const uint8_t *input, size_t input_size,
                      uint8_t *output, size_t output_size) {
    if (!input || (!output && output_size) || input_size < 6)
        return false;
    const unsigned header = ((unsigned)input[0] << 8) | input[1];
    if ((input[0] & 15u) != 8 || (input[0] >> 4) > 7 ||
        header % 31 || (input[1] & 32u))
        return false;
    const unsigned window = 1u << ((input[0] >> 4) + 8);
    RoomBits reader = {input + 2, input_size - 6, 0, 0, 0, false};
    size_t written = 0;
    bool final;
    do {
        final = TakeBits(&reader, 1) != 0;
        const unsigned type = TakeBits(&reader, 2);
        if (reader.failed || type == 3)
            return false;
        if (type == 0) {
            reader.bits = 0;
            reader.count = 0;
            const unsigned count = TakeBits(&reader, 16);
            const unsigned inverse = TakeBits(&reader, 16);
            if (reader.failed || (count ^ inverse) != 0xffffu ||
                count > output_size - written || count > reader.size - reader.at)
                return false;
            if (count)
                memcpy(output + written, reader.data + reader.at, count);
            written += count;
            reader.at += count;
            continue;
        }
        RoomHuffman literal, distance;
        if (!ReadTrees(&reader, type, &literal, &distance))
            return false;
        for (;;) {
            const int symbol = ReadSymbol(&reader, &literal);
            if (symbol < 0)
                return false;
            if (symbol < 256) {
                if (written == output_size)
                    return false;
                output[written++] = (uint8_t)symbol;
                continue;
            }
            if (symbol == 256)
                break;
            if (symbol > 285)
                return false;
            static const uint16_t length_base[29] = {
                3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,
                51,59,67,83,99,115,131,163,195,227,258
            };
            static const uint8_t length_bits[29] = {
                0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
            };
            static const uint16_t distance_base[30] = {
                1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,
                385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577
            };
            static const uint8_t distance_bits[30] = {
                0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
            };
            const unsigned length = length_base[symbol - 257] +
                TakeBits(&reader, length_bits[symbol - 257]);
            const int code = ReadSymbol(&reader, &distance);
            if (code < 0 || code >= 30)
                return false;
            const unsigned back = distance_base[code] + TakeBits(&reader, distance_bits[code]);
            if (reader.failed || back > window || back > written ||
                length > output_size - written)
                return false;
            for (unsigned i = 0; i < length; i++) {
                output[written] = output[written - back];
                written++;
            }
        }
    } while (!final);
    if (reader.failed || reader.at != reader.size || written != output_size)
        return false;
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < output_size; i++) {
        a = (a + output[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    const uint8_t *tail = input + input_size - 4;
    const uint32_t checksum = ((uint32_t)tail[0] << 24) | ((uint32_t)tail[1] << 16) |
        ((uint32_t)tail[2] << 8) | tail[3];
    return checksum == ((b << 16) | a);
}
