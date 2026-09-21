#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lufia2/player_update.h"
#include "snes_function_verify.h"

enum {
    LUFIA2_ROM_SIZE = 0x280000,
    BBF3_PC = 0x83bbf3,
    BBF3_CHILD_SPECIAL = 0x83bc28,
    BBF3_CHILD_STANDARD = 0x83c1b4,
    BBF3_RETURN_SENTINEL = 0x837fff,
    BBF3_STACK = 0x01f0,
    BBF3_RANDOM_DEFAULT = 65536,
    BBF3_GATE_COUNT = 6,
};

static const uint16_t kGateAddresses[BBF3_GATE_COUNT] = {
    0x09a8, 0x05b5, 0x05b7, 0x0622, 0x099b, 0x09a7,
};

typedef struct VerifyLog {
    FILE *file;
} VerifyLog;

typedef struct Bbf3Input {
    uint8_t gate[BBF3_GATE_COUNT];
    uint16_t a;
    uint8_t x;
    uint8_t y;
    uint16_t dp;
    uint8_t db;
    uint8_t c;
    uint8_t z;
    uint8_t v;
    uint8_t n;
    uint8_t d;
    uint8_t i;
} Bbf3Input;

typedef struct NativeReadContext {
    const Bbf3Input *input;
    uint16_t addresses[BBF3_GATE_COUNT];
    uint8_t values[BBF3_GATE_COUNT];
    size_t count;
    bool overflow;
} NativeReadContext;

typedef struct ReferenceResult {
    uint8_t action;
    uint8_t accumulator_low;
    uint8_t negative;
    uint8_t zero;
    uint16_t addresses[BBF3_GATE_COUNT];
    uint8_t values[BBF3_GATE_COUNT];
    size_t read_count;
    unsigned instructions;
} ReferenceResult;

static uint64_t s_rng = UINT64_C(0x9e3779b97f4a7c15);

static void Logf(VerifyLog *log, const char *format, ...) {
    va_list args;
    va_list copy;

    va_start(args, format);
    va_copy(copy, args);
    vprintf(format, args);
    if (log->file)
        vfprintf(log->file, format, copy);
    va_end(copy);
    va_end(args);
}

static uint32_t Random32(void) {
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 7;
    s_rng ^= s_rng << 17;
    return (uint32_t)(s_rng >> 32);
}

static int GateIndex(uint16_t address) {
    for (int i = 0; i < BBF3_GATE_COUNT; ++i) {
        if (kGateAddresses[i] == address)
            return i;
    }
    return -1;
}

static uint8_t NativeRead(void *opaque, uint16_t address) {
    NativeReadContext *context = (NativeReadContext *)opaque;
    const int index = GateIndex(address);
    const uint8_t value =
        index >= 0 ? context->input->gate[index] : 0xffu;

    if (context->count < BBF3_GATE_COUNT) {
        context->addresses[context->count] = address;
        context->values[context->count] = value;
        ++context->count;
    } else {
        context->overflow = true;
    }
    return value;
}

static bool PrepareReference(
    SnesVerifyBus *bus, Interp816 *cpu, const Bbf3Input *input) {
    for (int i = 0; i < BBF3_GATE_COUNT; ++i) {
        const uint32_t address =
            ((uint32_t)input->db << 16) | kGateAddresses[i];
        if (!SnesVerifyBusPoke(bus, address, input->gate[i]))
            return false;
    }

    if (!SnesVerifyBusPoke(
            bus, (uint32_t)(BBF3_STACK + 1), 0xfeu) ||
        !SnesVerifyBusPoke(
            bus, (uint32_t)(BBF3_STACK + 2), 0x7fu))
        return false;

    cpu->a = input->a;
    cpu->x = input->x;
    cpu->y = input->y;
    cpu->sp = BBF3_STACK;
    cpu->pc = (uint16_t)BBF3_PC;
    cpu->dp = input->dp;
    cpu->k = (uint8_t)(BBF3_PC >> 16);
    cpu->db = input->db;

    cpu->c = input->c != 0;
    cpu->z = input->z != 0;
    cpu->v = input->v != 0;
    cpu->n = input->n != 0;
    cpu->d = input->d != 0;
    cpu->i = input->i != 0;
    cpu->mf = true;
    cpu->xf = true;
    cpu->e = false;
    cpu->irqWanted = false;
    cpu->nmiWanted = false;
    cpu->waiting = false;
    cpu->stopped = false;
    interp816_set_brk_hook_enabled(cpu, false);

    SnesVerifyBusResetTrace(bus);
    return true;
}

static bool ReferenceBbf3(
    SnesVerifyBus *bus,
    Interp816 *cpu,
    const Bbf3Input *input,
    ReferenceResult *result) {
    const uint32_t stops[] = {
        BBF3_RETURN_SENTINEL,
        BBF3_CHILD_SPECIAL,
        BBF3_CHILD_STANDARD,
    };
    int stop;

    memset(result, 0, sizeof(*result));
    if (!PrepareReference(bus, cpu, input))
        return false;

    stop = SnesVerifyRunUntil(
        cpu, stops, sizeof(stops) / sizeof(stops[0]), 128,
        &result->instructions);
    if (stop < 0 || bus->event_overflow)
        return false;

    if (stop == 1)
        result->action = LUFIA2_PLAYER_SLOT_SPECIAL_CHILD;
    else if (stop == 2)
        result->action = LUFIA2_PLAYER_SLOT_STANDARD_CHILD;
    else
        result->action = LUFIA2_PLAYER_SLOT_NO_CHILD;

    result->accumulator_low = (uint8_t)cpu->a;
    result->negative = cpu->n ? 1u : 0u;
    result->zero = cpu->z ? 1u : 0u;

    for (size_t i = 0; i < bus->event_count; ++i) {
        const SnesVerifyBusEvent *event = &bus->events[i];
        const uint8_t bank = (uint8_t)(event->address >> 16);
        const uint16_t address = (uint16_t)event->address;
        if (event->write || bank != input->db || GateIndex(address) < 0)
            continue;
        if (result->read_count >= BBF3_GATE_COUNT)
            return false;
        result->addresses[result->read_count] = address;
        result->values[result->read_count] = event->value;
        ++result->read_count;
    }

    return true;
}

static void PrintInput(VerifyLog *log, const Bbf3Input *input) {
    Logf(log,
        "  input: 09A8=%02X 05B5=%02X 05B7=%02X 0622=%02X "
        "099B=%02X 09A7=%02X A=%04X DB=%02X D=%d\n",
        input->gate[0], input->gate[1], input->gate[2],
        input->gate[3], input->gate[4], input->gate[5],
        input->a, input->db, input->d);
}

static bool CompareCase(
    VerifyLog *log,
    SnesVerifyBus *bus,
    Interp816 *cpu,
    const Bbf3Input *input,
    unsigned case_index) {
    NativeReadContext native_context;
    Lufia2PlayerSlotSpecialMemory native_memory;
    Lufia2PlayerSlotSpecialResult native_result;
    ReferenceResult reference;

    memset(&native_context, 0, sizeof(native_context));
    native_context.input = input;
    native_memory.read_byte = NativeRead;
    native_memory.context = &native_context;

    native_result = Lufia2PlayerSlotSpecialUpdate(&native_memory);
    if (!ReferenceBbf3(bus, cpu, input, &reference)) {
        Logf(log, "FAIL $83:BBF3 case %u: reference execution failed\n",
            case_index);
        PrintInput(log, input);
        return false;
    }

    if (native_context.overflow ||
        native_result.action != reference.action ||
        native_result.accumulator_low != reference.accumulator_low ||
        native_result.negative != reference.negative ||
        native_result.zero != reference.zero ||
        native_context.count != reference.read_count) {
        Logf(log, "FAIL $83:BBF3 case %u: semantic mismatch\n", case_index);
        PrintInput(log, input);
        Logf(log,
            "  result native/ref: action=%u/%u A=%02X/%02X "
            "N=%u/%u Z=%u/%u reads=%u/%u\n",
            native_result.action, reference.action,
            native_result.accumulator_low, reference.accumulator_low,
            native_result.negative, reference.negative,
            native_result.zero, reference.zero,
            (unsigned)native_context.count, (unsigned)reference.read_count);
        return false;
    }

    for (size_t i = 0; i < native_context.count; ++i) {
        if (native_context.addresses[i] != reference.addresses[i] ||
            native_context.values[i] != reference.values[i]) {
            Logf(log,
                "FAIL $83:BBF3 case %u: read trace mismatch at %u "
                "native=%04X:%02X ref=%04X:%02X\n",
                case_index, (unsigned)i,
                native_context.addresses[i], native_context.values[i],
                reference.addresses[i], reference.values[i]);
            PrintInput(log, input);
            return false;
        }
    }

    return true;
}

static Bbf3Input MakeSystematic(unsigned index) {
    static const uint8_t banks[] = {0x00, 0x7e, 0x80, 0xbf};
    Bbf3Input input;
    memset(&input, 0, sizeof(input));

    input.gate[0] = (index & 1u) ? 0x08u : 0;
    input.gate[1] = (index & 2u) ? 0x02u : 0;
    input.gate[2] = (uint8_t)((index >> 2) & 0x07u);
    input.gate[3] = (index & 0x20u) ? 0x80u : 0;
    input.gate[4] = (index & 0x40u) ? 0x80u : 0;
    input.gate[5] = (index & 0x80u) ? 0x01u : 0;

    input.a = (uint16_t)(0xa500u | (index & 0xffu));
    input.x = (uint8_t)(index ^ 0x5au);
    input.y = (uint8_t)(index ^ 0xa5u);
    input.dp = (uint16_t)(index * 17u);
    input.db = banks[index & 3u];
    input.c = (index >> 0) & 1u;
    input.z = (index >> 1) & 1u;
    input.v = (index >> 2) & 1u;
    input.n = (index >> 3) & 1u;
    input.d = (index >> 4) & 1u;
    input.i = (index >> 5) & 1u;
    return input;
}

static Bbf3Input MakeRandom(void) {
    static const uint8_t banks[] = {0x00, 0x7e, 0x80, 0xbf};
    Bbf3Input input;

    for (int i = 0; i < BBF3_GATE_COUNT; ++i)
        input.gate[i] = (uint8_t)Random32();

    input.a = (uint16_t)Random32();
    input.x = (uint8_t)Random32();
    input.y = (uint8_t)Random32();
    input.dp = (uint16_t)Random32();
    input.db = banks[Random32() & 3u];
    input.c = Random32() & 1u;
    input.z = Random32() & 1u;
    input.v = Random32() & 1u;
    input.n = Random32() & 1u;
    input.d = Random32() & 1u;
    input.i = Random32() & 1u;
    return input;
}

int interp816_opcode_hook(uint32_t address) {
    (void)address;
    return 0;
}

int main(int argc, char **argv) {
    const char *rom_path;
    const char *report_path = NULL;
    unsigned random_cases = BBF3_RANDOM_DEFAULT;
    uint8_t *rom = NULL;
    size_t rom_size = 0;
    SnesVerifyBus bus;
    Interp816 *cpu = NULL;
    VerifyLog log = {0};
    unsigned passed = 0;
    unsigned failed = 0;
    unsigned case_index = 0;
    bool bus_initialized = false;

    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <lufia2.sfc> [--random count] [--report path]\n",
            argv[0]);
        return 2;
    }
    rom_path = argv[1];

    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--random") && i + 1 < argc) {
            random_cases = (unsigned)strtoul(argv[++i], NULL, 0);
        } else if (!strcmp(argv[i], "--report") && i + 1 < argc) {
            report_path = argv[++i];
        } else {
            fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 2;
        }
    }

    if (report_path) {
        log.file = fopen(report_path, "wb");
        if (!log.file) {
            fprintf(stderr, "cannot open report: %s\n", report_path);
            return 2;
        }
    }

    Logf(&log, "Lufia II decomp differential verifier\n");
    Logf(&log, "=====================================\n\n");
    Logf(&log, "Reference: original ROM bytes via snesrecomp interp816\n");
    Logf(&log, "Native:    Lufia2Decomp portable C semantics\n\n");

    if (!SnesVerifyLoadFile(rom_path, &rom, &rom_size)) {
        Logf(&log, "ERROR: could not read ROM: %s\n", rom_path);
        failed = 1;
        goto done;
    }
    if (rom_size != LUFIA2_ROM_SIZE) {
        Logf(&log,
            "ERROR: unsupported ROM size: %u bytes, expected %u\n",
            (unsigned)rom_size, (unsigned)LUFIA2_ROM_SIZE);
        failed = 1;
        goto done;
    }
    if (!SnesVerifyBusInit(&bus, rom, rom_size)) {
        Logf(&log, "ERROR: could not initialize reference bus\n");
        failed = 1;
        goto done;
    }
    bus_initialized = true;

    cpu = interp816_init(&bus, SnesVerifyBusRead, SnesVerifyBusWrite);
    if (!cpu) {
        Logf(&log, "ERROR: could not initialize interp816\n");
        failed = 1;
        goto done;
    }

    Logf(&log,
        "$83:BBF3 Lufia2PlayerSlotSpecialUpdate\n"
        "  systematic relevant-bit cases: 256\n"
        "  deterministic random cases:    %u\n",
        random_cases);

    for (unsigned i = 0; i < 256; ++i, ++case_index) {
        const Bbf3Input input = MakeSystematic(i);
        if (CompareCase(&log, &bus, cpu, &input, case_index))
            ++passed;
        else if (++failed >= 20)
            break;
    }

    for (unsigned i = 0; i < random_cases && failed < 20;
         ++i, ++case_index) {
        const Bbf3Input input = MakeRandom();
        if (CompareCase(&log, &bus, cpu, &input, case_index))
            ++passed;
        else
            ++failed;
    }

    Logf(&log, "\nSummary\n-------\n");
    Logf(&log, "cases passed: %u\n", passed);
    Logf(&log, "cases failed: %u\n", failed);
    if (!failed)
        Logf(&log, "RESULT: PASS - $83:BBF3 semantic behavior matched\n");
    else
        Logf(&log, "RESULT: FAIL - counterexample(s) found\n");

done:
    if (cpu)
        interp816_free(cpu);
    if (bus_initialized)
        SnesVerifyBusDestroy(&bus);
    free(rom);
    if (log.file)
        fclose(log.file);
    return failed ? 1 : 0;
}
