#include "target_record_clear_plans.inc"

typedef struct TargetRecordClearClock {
    FixedSpanClock span;
    unsigned remaining;
} TargetRecordClearClock;

static bool TargetRecordClearWindow(const CpuState *cpu) {
    const unsigned budget = 16384u;
    if (cpu->x_flag || cpu->host_return_valid != 2u || cpu->S < 0x1f30u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x81c586u,
            0x1ffcu, false, 1363u, budget) ||
        !Lufia2DecompFrameRangeQuiet(0x81c586u, 0x81c5cfu))
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

static uint8_t TargetRecordClearRead(void *context, uint32_t address) {
    return FixedSpanRead(&((TargetRecordClearClock *)context)->span, address);
}

static void TargetRecordClearWrite(void *context, uint32_t address, uint8_t value) {
    TargetRecordClearClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
    if (clock->remaining && clock->span.index == clock->span.count) {
        if (--clock->remaining) {
            clock->span.timing = target_record_clear_repeat;
            clock->span.count = sizeof target_record_clear_repeat /
                sizeof target_record_clear_repeat[0];
        } else {
            clock->span.timing = target_record_clear_finish;
            clock->span.count = sizeof target_record_clear_finish /
                sizeof target_record_clear_finish[0];
        }
        clock->span.index = 0u;
    }
}

static RecompReturn ActorBridgeClearTargetRecords(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (!TargetRecordClearWindow(cpu))
        return ActorBridgeFallback(cpu, &frame, 0x81c586u);
    TargetRecordClearClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x810000u;
    clock.span.timing = target_record_clear_start;
    clock.span.count = sizeof target_record_clear_start / sizeof target_record_clear_start[0];
    clock.remaining = 132u;
    const Lufia2Memory memory = {TargetRecordClearRead, TargetRecordClearWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleClearTargetRecords(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x81c5ceu);
    assert(!clock.remaining && !clock.span.remaining && clock.span.index == clock.span.count);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
