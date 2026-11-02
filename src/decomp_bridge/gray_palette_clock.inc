#include "gray_palette_plans.inc"

typedef enum GrayPalettePhase {
    GRAY_BEGIN, GRAY_PREFIX, GRAY_COLOR, GRAY_AFTER, GRAY_FINISH, GRAY_DONE
} GrayPalettePhase;

typedef struct GrayPaletteClock {
    FixedSpanClock span;
    const GrayPalettePlan *plan;
    GrayPalettePhase phase;
    unsigned remaining;
} GrayPaletteClock;

static void SelectGrayPalettePhase(GrayPaletteClock *clock, GrayPalettePhasePlan plan) {
    clock->span.timing = plan.opcodes;
    clock->span.count = plan.count;
    clock->span.index = 0u;
}

static void AdvanceGrayPalette(GrayPaletteClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || clock->phase == GRAY_DONE)
            return;
        GrayPalettePhasePlan next;
        if (clock->phase == GRAY_BEGIN || clock->phase == GRAY_AFTER) {
            if (clock->phase == GRAY_AFTER && !--clock->remaining) {
                clock->phase = GRAY_FINISH;
                next = clock->plan->finish;
            } else {
                clock->phase = GRAY_PREFIX;
                next = clock->plan->prefix;
            }
        } else if (clock->phase == GRAY_PREFIX) {
            clock->phase = GRAY_COLOR;
            next = (GrayPalettePhasePlan){gray_palette_color,
                sizeof gray_palette_color / sizeof gray_palette_color[0]};
        } else if (clock->phase == GRAY_COLOR) {
            clock->phase = GRAY_AFTER;
            next = clock->remaining == 1u ? clock->plan->last : clock->plan->after;
        } else {
            clock->phase = GRAY_DONE;
            return;
        }
        SelectGrayPalettePhase(clock, next);
    }
}

static uint8_t GrayPaletteRead(void *context, uint32_t address) {
    GrayPaletteClock *clock = context;
    AdvanceGrayPalette(clock);
    return FixedSpanRead(&clock->span, address);
}

static void GrayPaletteWrite(void *context, uint32_t address, uint8_t value) {
    GrayPaletteClock *clock = context;
    AdvanceGrayPalette(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool GrayPaletteClockSupported(const CpuState *cpu, uint32_t entry,
    uint32_t end) {
    const unsigned budget = 32768u;
    if (cpu->D || cpu->_flag_D || cpu->x_flag || cpu->host_return_valid != 3u ||
        cpu->S < 0x1f20u || (cpu->DB & 0x7fu) >= 0x40u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, budget) ||
        cpu->ram[0x11] >= 16u || !Lufia2DecompFrameRangeQuiet(entry, end) ||
        !Lufia2DecompFrameRangeQuiet(0x81b54au, 0x81b5a3u))
        return false;
    const unsigned line = g_snes->vPos;
    if (line < 225u) {
        if (!g_snes->hdmaBeamOff || (225u - line) * 1364u - g_snes->hPos <= budget)
            return false;
    } else if (line >= 262u || (262u - line) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool TryTimedGrayPalette(CpuState *cpu, unsigned output, RecompReturn *returned) {
    static const uint32_t entries[] = {0x81b308u, 0x81b336u, 0x81b367u};
    static const uint32_t ends[] = {0x81b336u, 0x81b367u, 0x81b396u};
    if (!GrayPaletteClockSupported(cpu, entries[output], ends[output]))
        return false;
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    GrayPaletteClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.plan = &gray_palette_plans[output];
    clock.remaining = 15u;
    SelectGrayPalettePhase(&clock, clock.plan->begin);
    const Lufia2Memory memory = {GrayPaletteRead, GrayPaletteWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = output == 0u
        ? Lufia2BattleWriteGrayPaletteBuffer(&memory, &state)
        : output == 1u ? Lufia2BattleGrayPaletteToBuffer(&memory, &state)
                       : Lufia2BattleGraySourcePalette(&memory, &state);
    AdvanceGrayPalette(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && clock.phase == GRAY_DONE);
    assert(!clock.span.remaining && clock.span.index == clock.span.count);
    ActorBridgeStore(cpu, &state);
    *returned = ReturnTimedLongCall(cpu, &frame, result.pc);
    return true;
}
