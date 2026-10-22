/* The decompiled sound driver as the APU program.
 * Busy time defers later hardware accesses; state lives here. */

#include "lufia2_spc_native.h"

#include <stdlib.h>
#include <string.h>

#include "lufia2/spc_driver.h"
#include "snes/apu.h"
#include "snes/dsp.h"
#include "snes/saveload.h"
#include "snesrecomp_platform/apu_program.h"

enum {
    DRIVER_ENTRY = 0x0300,
    DRIVER_CODE = 0x0300,
    DRIVER_CODE_END = 0x0603,
    DRIVER_DATA = 0x09D2,
    DRIVER_DATA_END = 0x43FA,
    STATE_MAGIC = 0x4453324Cu, /* "L2SD" */
    STATE_VERSION = 1,
    QUEUE_LIMIT = 8192,
};

/* FNV-1a over the uploaded driver code and data ($90:C000 image). */
#define DRIVER_SIGNATURE 0x86d4aea6972c9d3aull

/* Hardware access the original makes later. */
typedef enum AccessKind {
    ACCESS_DSP,
    ACCESS_PORT,
    ACCESS_CONTROL,
    ACCESS_TIMER_TARGET,
} AccessKind;

typedef struct QueuedAccess {
    uint8_t kind;
    uint8_t target;
    uint8_t value;
    uint32_t due;
} QueuedAccess;

static Lufia2SpcDriver s_driver;
static Lufia2SpcHardware s_hardware;
static bool s_installed;
static bool s_active;
/* Port write or tick arrived while busy. */
static bool s_run_deferred;
/* Busy time reported so far in the current run. */
static uint32_t s_run_delay;
static QueuedAccess s_queue[QUEUE_LIMIT];
static unsigned s_queue_count;
/* Savestate load without chunk pending. */
static bool s_loaded_without_state;

static void Apply(Apu *apu, const QueuedAccess *access) {
    switch ((AccessKind)access->kind) {
    case ACCESS_DSP:
        apu->dspAdr = access->target;
        dsp_write(apu->dsp, access->target, access->value);
        break;
    case ACCESS_PORT:
        apu_cpuWrite(apu, (uint16_t)(0xF4 + access->target), access->value);
        break;
    case ACCESS_CONTROL:
        apu_cpuWrite(apu, 0xF1, access->value);
        break;
    case ACCESS_TIMER_TARGET:
        apu_cpuWrite(apu, (uint16_t)(0xFA + access->target), access->value);
        break;
    }
}

static void ApplyDue(Apu *apu) {
    unsigned applied = 0;

    while (applied < s_queue_count && (int32_t)(apu->cycles - s_queue[applied].due) >= 0)
        Apply(apu, &s_queue[applied++]);
    if (applied == 0)
        return;
    s_queue_count -= applied;
    memmove(s_queue, s_queue + applied, s_queue_count * sizeof(s_queue[0]));
}

/* Now, or after this run's busy time. */
static void Access(Apu *apu, AccessKind kind, uint8_t target, uint8_t value) {
    const QueuedAccess access = {(uint8_t)kind, target, value, apu->cycles + s_run_delay};

    if (s_run_delay == 0 && s_queue_count == 0) {
        Apply(apu, &access);
        return;
    }
    if (s_queue_count == QUEUE_LIMIT) {
        /* Unreachable with the game's data. */
        Apply(apu, &s_queue[0]);
        memmove(s_queue, s_queue + 1, --s_queue_count * sizeof(s_queue[0]));
    }
    s_queue[s_queue_count++] = access;
}

/* ---- driver hardware: the core's APU registers ---- */

static uint8_t ReadPort(void *context, unsigned port) {
    return apu_cpuRead((Apu *)context, (uint16_t)(0xF4 + port));
}

static void WritePort(void *context, unsigned port, uint8_t value) {
    Access((Apu *)context, ACCESS_PORT, (uint8_t)port, value);
}

/* A queued DSP write is what the original reads. */
static uint8_t ReadDsp(void *context, uint8_t reg) {
    Apu *apu = context;

    reg &= 0x7F;
    for (unsigned i = s_queue_count; i-- > 0;) {
        if (s_queue[i].kind == ACCESS_DSP && s_queue[i].target == reg)
            return s_queue[i].value;
    }
    return dsp_read(apu->dsp, reg);
}

static void WriteDsp(void *context, uint8_t reg, uint8_t value) {
    if (reg < 0x80)
        Access((Apu *)context, ACCESS_DSP, reg, value);
}

static void Delay(void *context, unsigned cycles) {
    (void)context;
    s_run_delay += cycles;
}

static void WriteControl(void *context, uint8_t value) {
    Access((Apu *)context, ACCESS_CONTROL, 0, value);
}

static void WriteTimerTarget(void *context, unsigned timer, uint8_t target) {
    Access((Apu *)context, ACCESS_TIMER_TARGET, (uint8_t)timer, target);
}

static uint8_t ReadTimer(void *context, unsigned timer) {
    return apu_cpuRead((Apu *)context, (uint16_t)(0xFD + timer));
}

static void Bind(Apu *apu) {
    s_hardware = (Lufia2SpcHardware){apu,      ReadPort,     WritePort,
                                     ReadDsp,  WriteDsp,     Delay,
                                     WriteControl, WriteTimerTarget, ReadTimer};
    s_driver.aram = apu->ram;
    s_driver.hardware = &s_hardware;
}

/* ---- the APU program ---- */

static uint64_t Fnv(const uint8_t *data, size_t size, uint64_t hash) {
    for (size_t i = 0; i < size; ++i)
        hash = (hash ^ data[i]) * 0x100000001b3ull;
    return hash;
}

static bool DriverUploaded(const Apu *apu) {
    uint64_t hash = Fnv(&apu->ram[DRIVER_CODE], DRIVER_CODE_END - DRIVER_CODE,
                        0xcbf29ce484222325ull);
    hash = Fnv(&apu->ram[DRIVER_DATA], DRIVER_DATA_END - DRIVER_DATA, hash);
    return hash == DRIVER_SIGNATURE;
}

static void RunDriver(Apu *apu) {
    Bind(apu);
    s_run_delay = 0;
    Lufia2SpcDriverRun(&s_driver);
    s_run_delay = 0;
}

static bool TakeOver(void *context, Apu *apu) {
    (void)context;
    if (apu->spc->pc != DRIVER_ENTRY || !DriverUploaded(apu))
        return false;
    s_queue_count = 0;
    s_run_deferred = false;
    s_run_delay = 0;
    Bind(apu);
    Lufia2SpcDriverStart(&s_driver, apu->ram, &s_hardware);
    s_active = true;
    return true;
}

/* Load without chunk: native if parked at entry; resume idle. */
static void SettleAfterLoad(Apu *apu) {
    s_loaded_without_state = false;
    s_queue_count = 0;
    s_run_deferred = false;
    s_active = apu->spc->pc == DRIVER_ENTRY && apu->cpuCyclesLeft != 0 &&
               DriverUploaded(apu);
    if (!s_active)
        return;
    Bind(apu);
    s_driver.wait = LUFIA2_SPC_WAIT_IDLE;
    s_driver.step_count = 0;
    s_driver.substep = 0;
    s_driver.repeat_command = 0;
}

static bool Active(void *context, Apu *apu) {
    (void)context;
    if (s_loaded_without_state)
        SettleAfterLoad(apu);
    return s_active;
}

static void Run(void *context, Apu *apu) {
    (void)context;
    if (s_queue_count != 0) {
        s_run_deferred = true;
        return;
    }
    RunDriver(apu);
}

static void Cycle(void *context, Apu *apu) {
    (void)context;
    if (s_queue_count == 0)
        return;
    ApplyDue(apu);
    if (s_queue_count == 0 && s_run_deferred) {
        s_run_deferred = false;
        RunDriver(apu);
    }
}

static void Loaded(void *context, Apu *apu) {
    (void)context;
    (void)apu;
    s_loaded_without_state = true;
}

static const SnesRecompApuProgram kProgram = {NULL, TakeOver, Active, Run, Cycle, Loaded};

bool Lufia2SpcNativeInstall(void) {
    const char *interpreted = getenv("LUFIA2_SPC_INTERPRETED");

    if (!s_installed && !(interpreted && interpreted[0] == '1')) {
        snesrecomp_apu_set_program(&kProgram);
        s_installed = true;
    }
    return s_installed;
}

const SnesRecompApuProgram *Lufia2SpcNativeProgram(void) { return &kProgram; }

bool Lufia2SpcNativeActive(void) { return s_installed && s_active; }

/* ---- savestate chunk ---- */

typedef struct NativeStateHeader {
    uint32_t magic;
    uint32_t version;
    uint8_t active;
    uint8_t run_deferred;
    uint8_t wait;
    uint8_t step_count;
    uint8_t steps[LUFIA2_SPC_MAX_STEPS];
    uint8_t substep;
    uint8_t counter;
    uint16_t cursor;
    uint16_t effect_pointer;
    uint8_t effect_id;
    uint8_t repeat_command;
    uint16_t repeat_arguments;
    uint32_t queue_count;
} NativeStateHeader;

/* Queue times relative to the APU clock. */
void Lufia2SpcNativeSaveState(SaveLoadInfo *sli, const Apu *apu) {
    const uint32_t now = apu->cycles;
    NativeStateHeader header;

    memset(&header, 0, sizeof(header));
    header.magic = STATE_MAGIC;
    header.version = STATE_VERSION;
    header.active = s_active;
    header.run_deferred = s_run_deferred;
    header.wait = (uint8_t)s_driver.wait;
    header.step_count = s_driver.step_count;
    memcpy(header.steps, s_driver.steps, sizeof(header.steps));
    header.substep = s_driver.substep;
    header.counter = s_driver.counter;
    header.cursor = s_driver.cursor;
    header.effect_pointer = s_driver.effect_pointer;
    header.effect_id = s_driver.effect_id;
    header.repeat_command = s_driver.repeat_command;
    header.repeat_arguments = s_driver.repeat_arguments;
    header.queue_count = s_queue_count;
    sli->func(sli, &header, sizeof(header));
    for (unsigned i = 0; i < s_queue_count; ++i) {
        QueuedAccess access = s_queue[i];
        access.due -= now;
        sli->func(sli, &access, sizeof(access));
    }
}

/* Older states end before the chunk; peek keeps them loadable. */
bool Lufia2SpcNativeLoadState(SaveLoadInfo *sli, Apu *apu) {
    NativeStateHeader header;
    uint32_t magic = 0;

    if (!sli->peek || !sli->peek(sli, 0, &magic, sizeof(magic)) || magic != STATE_MAGIC)
        return false;
    memset(&header, 0, sizeof(header));
    sli->func(sli, &header, sizeof(header));
    if (header.magic != STATE_MAGIC || header.version != STATE_VERSION ||
        header.queue_count > QUEUE_LIMIT || header.step_count > LUFIA2_SPC_MAX_STEPS)
        return false;
    const uint32_t now = apu->cycles;
    for (unsigned i = 0; i < header.queue_count; ++i) {
        sli->func(sli, &s_queue[i], sizeof(s_queue[i]));
        s_queue[i].due += now;
    }
    s_queue_count = header.queue_count;
    s_active = header.active != 0;
    s_run_deferred = header.run_deferred != 0;
    s_driver.wait = (Lufia2SpcWait)header.wait;
    s_driver.step_count = header.step_count;
    memcpy(s_driver.steps, header.steps, sizeof(s_driver.steps));
    s_driver.substep = header.substep;
    s_driver.counter = header.counter;
    s_driver.cursor = header.cursor;
    s_driver.effect_pointer = header.effect_pointer;
    s_driver.effect_id = header.effect_id;
    s_driver.repeat_command = header.repeat_command;
    s_driver.repeat_arguments = header.repeat_arguments;
    s_loaded_without_state = false;
    Bind(apu);
    return true;
}
