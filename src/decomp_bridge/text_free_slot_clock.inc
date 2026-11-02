#include "text_free_slot_plans.inc"

typedef enum TextFreeSlotPhase {
    TEXT_FREE_BEGIN, TEXT_FREE_QUERY, TEXT_FREE_BUSY, TEXT_FREE_FOUND,
    TEXT_FREE_CHILD, TEXT_FREE_RETURN, TEXT_FREE_DONE
} TextFreeSlotPhase;

typedef struct TextFreeSlotClock {
    FixedSpanClock span;
    TextFreeSlotPhase phase;
    unsigned slot;
    bool found;
} TextFreeSlotClock;

static void AdvanceTextFreeSlot(TextFreeSlotClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || clock->phase == TEXT_FREE_DONE)
            return;
#define TEXT_FREE_PLAN(name) do { \
    clock->span.timing = text_free_slot_##name; \
    clock->span.count = sizeof text_free_slot_##name / sizeof text_free_slot_##name[0]; \
} while (0)
        if (clock->phase == TEXT_FREE_BEGIN || clock->phase == TEXT_FREE_BUSY) {
            if (clock->phase == TEXT_FREE_BUSY && ++clock->slot == 40u) {
                clock->phase = TEXT_FREE_RETURN;
                TEXT_FREE_PLAN(full);
            } else {
                clock->phase = TEXT_FREE_QUERY;
                TEXT_FREE_PLAN(query_busy);
            }
        } else if (clock->phase == TEXT_FREE_QUERY) {
            if (clock->found) {
                clock->phase = TEXT_FREE_FOUND;
                TEXT_FREE_PLAN(found);
            } else {
                clock->phase = TEXT_FREE_BUSY;
                if (clock->slot == 39u) TEXT_FREE_PLAN(busy_last);
                else TEXT_FREE_PLAN(busy_repeat);
            }
        } else if (clock->phase == TEXT_FREE_FOUND) {
            clock->phase = TEXT_FREE_CHILD;
            clock->span.bank = 0x840000u;
            TEXT_FREE_PLAN(child);
        } else if (clock->phase == TEXT_FREE_CHILD) {
            clock->phase = TEXT_FREE_RETURN;
            clock->span.bank = 0x800000u;
            TEXT_FREE_PLAN(success);
        } else {
            clock->phase = TEXT_FREE_DONE;
            return;
        }
        clock->span.index = 0u;
#undef TEXT_FREE_PLAN
    }
}

static uint8_t TextFreeSlotRead(void *context, uint32_t address) {
    TextFreeSlotClock *clock = context;
    AdvanceTextFreeSlot(clock);
    const uint8_t value = FixedSpanRead(&clock->span, address);
    if (clock->phase == TEXT_FREE_QUERY) {
        clock->found = value == 0xffu;
        if (clock->found)
            clock->span.timing = text_free_slot_query_found;
    }
    return value;
}

static void TextFreeSlotWrite(void *context, uint32_t address, uint8_t value) {
    TextFreeSlotClock *clock = context;
    AdvanceTextFreeSlot(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool TextFreeSlotClockSupported(const CpuState *cpu) {
    const unsigned budget = 8192u;
    if (cpu->D || cpu->_flag_D || cpu->x_flag || cpu->host_return_valid != 2u ||
        cpu->S < 0x1f20u ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x80c1c3u,
            0x1ffcu, false, 1363u, budget) ||
        !Lufia2DecompFrameRangeQuiet(0x80c1c3u, 0x80c1dfu) ||
        !Lufia2DecompFrameRangeQuiet(0x8482d5u, 0x8482e7u))
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

static RecompReturn ActorBridgeTextFreeSlot(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (!TextFreeSlotClockSupported(cpu))
        return ActorBridgeFallback(cpu, &frame, 0x80c1c3u);
    TextFreeSlotClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x800000u;
    clock.span.timing = text_free_slot_begin;
    clock.span.count = sizeof text_free_slot_begin / sizeof text_free_slot_begin[0];
    const Lufia2Memory memory = {TextFreeSlotRead, TextFreeSlotWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2TextFindFreeActorSlot(&memory, &state);
    AdvanceTextFreeSlot(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && clock.phase == TEXT_FREE_DONE);
    assert(!clock.span.remaining && clock.span.index == clock.span.count);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
