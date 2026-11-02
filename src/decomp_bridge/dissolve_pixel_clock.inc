#include "dissolve_pixel_plans.inc"

typedef enum DissolvePixelPhase {
    DISSOLVE_IDLE, DISSOLVE_CALL, DISSOLVE_RANDOM,
    DISSOLVE_REFILL_BEGIN, DISSOLVE_REFILL_FIRST, DISSOLVE_REFILL_SECOND,
    DISSOLVE_REFILL_END, DISSOLVE_INITIAL, DISSOLVE_ROW,
    DISSOLVE_RETRY, DISSOLVE_LOCATE, DISSOLVE_BIT, DISSOLVE_WRITE
} DissolvePixelPhase;

typedef struct DissolvePixelClock {
    FixedSpanClock span;
    const Lufia2CpuState *state;
    DissolvePixelPhase phase;
    uint64_t reserved_until;
    uint32_t returned_pc;
    unsigned refill_slot;
    uint8_t row_limit_low;
    bool empty_sum;
    bool chosen;
    bool last_bit;
    bool indexed_page_crossed;
} DissolvePixelClock;

#define DISSOLVE_PLAN(clock, name) do { \
    (clock)->span.timing = dissolve_##name; \
    (clock)->span.count = sizeof dissolve_##name / sizeof dissolve_##name[0]; \
} while (0)

static void AdvanceDissolvePixel(DissolvePixelClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || clock->phase == DISSOLVE_IDLE)
            return;
        if (clock->phase == DISSOLVE_REFILL_BEGIN) {
            clock->refill_slot = 0u;
            clock->phase = DISSOLVE_REFILL_FIRST;
            DISSOLVE_PLAN(clock, random_first);
        } else if (clock->phase == DISSOLVE_REFILL_FIRST) {
            if (++clock->refill_slot == 24u) {
                clock->refill_slot = 0u;
                clock->phase = DISSOLVE_REFILL_SECOND;
                DISSOLVE_PLAN(clock, random_second);
            } else if (clock->refill_slot == 23u) {
                DISSOLVE_PLAN(clock, random_first_last);
            }
        } else if (clock->phase == DISSOLVE_REFILL_SECOND) {
            if (++clock->refill_slot == 31u) {
                clock->phase = DISSOLVE_REFILL_END;
                DISSOLVE_PLAN(clock, random_refill_end);
            } else if (clock->refill_slot == 30u) {
                DISSOLVE_PLAN(clock, random_second_last);
            }
        } else if (clock->phase == DISSOLVE_LOCATE) {
            clock->phase = DISSOLVE_BIT;
            clock->chosen = clock->last_bit = false;
            DISSOLVE_PLAN(clock, bit_miss_more);
        } else if (clock->phase == DISSOLVE_BIT && clock->chosen) {
            clock->phase = DISSOLVE_WRITE;
            DISSOLVE_PLAN(clock, write);
        } else if (clock->phase == DISSOLVE_BIT && !clock->last_bit) {
            DISSOLVE_PLAN(clock, bit_miss_more);
        } else {
            clock->phase = DISSOLVE_IDLE;
            return;
        }
        clock->span.index = 0u;
    }
}

static uint8_t ReadDissolveCrossedPage(DissolvePixelClock *clock,
    uint32_t address) {
    FixedSpanClock *span = &clock->span;
    const FixedSpanOpcode *timing = span->timing;
    const unsigned index = span->index;
    const unsigned progress_mask = span->progress_mask;
    FixedSpanOpcode adjusted = timing[index];
    ++adjusted.cycles;
    span->timing = &adjusted;
    span->index = span->progress_mask = 0u;
    const uint8_t value = FixedSpanRead(span, address);
    const bool progress = (span->progress_mask & 1u) != 0u;
    span->timing = timing;
    span->index = index + 1u;
    span->progress_mask = progress_mask |
        (progress && index < 32u ? 1u << index : 0u);
    return value;
}

static uint8_t DissolvePixelRead(void *context, uint32_t address) {
    DissolvePixelClock *clock = context;
    AdvanceDissolvePixel(clock);
    const uint16_t pc = clock->span.timing[clock->span.index].pc;
    const unsigned remaining = clock->span.remaining;
    const bool indexed = pc == 0xe86eu || pc == 0xe887u ||
        pc == 0xe8cbu || pc == 0xe8dfu;
    const uint8_t value = indexed && remaining == 1u && clock->indexed_page_crossed
        ? ReadDissolveCrossedPage(clock, address)
        : FixedSpanRead(&clock->span, address);
    if (indexed && remaining == 3u)
        clock->indexed_page_crossed = clock->state->y + value > 0xffu;
    if (pc == 0x82cfu && (uint8_t)(value + 1u) >= 55u) {
        clock->phase = DISSOLVE_REFILL_BEGIN;
        DISSOLVE_PLAN(clock, random_refill_begin);
    } else if (clock->phase == DISSOLVE_ROW) {
        if (pc == 0xe869u) {
            if (remaining == 2u) {
                clock->row_limit_low = value;
            } else if (clock->state->y ==
                    (uint16_t)(clock->row_limit_low | ((uint16_t)value << 8))) {
                DISSOLVE_PLAN(clock, row_end);
            }
        } else if (pc == 0xe86eu && remaining == 1u) {
            clock->empty_sum = (uint8_t)(clock->state->accumulator + value) == 0u;
            if (clock->empty_sum) DISSOLVE_PLAN(clock, row_zero_less);
            else DISSOLVE_PLAN(clock, row_sum_less);
        } else if (pc == 0xe874u) {
            const uint8_t sum = (uint8_t)clock->state->accumulator;
            if (clock->empty_sum) {
                if (sum == value) DISSOLVE_PLAN(clock, row_zero_equal);
                else DISSOLVE_PLAN(clock, row_zero_less);
            } else if (sum < value) DISSOLVE_PLAN(clock, row_sum_less);
            else if (sum == value) DISSOLVE_PLAN(clock, row_sum_equal);
            else DISSOLVE_PLAN(clock, row_sum_greater);
        }
    } else if (clock->phase == DISSOLVE_BIT && pc == 0xe8d1u) {
        const uint8_t mask = (uint8_t)clock->state->accumulator;
        clock->last_bit = mask == 0x80u;
        if (mask & value) {
            if (clock->last_bit) DISSOLVE_PLAN(clock, bit_hit_last);
            else DISSOLVE_PLAN(clock, bit_hit_more);
        } else if (clock->last_bit) DISSOLVE_PLAN(clock, bit_miss_last);
        else DISSOLVE_PLAN(clock, bit_miss_more);
    }
    return value;
}

static void DissolvePixelWrite(void *context, uint32_t address, uint8_t value) {
    DissolvePixelClock *clock = context;
    AdvanceDissolvePixel(clock);
    const uint16_t pc = clock->span.timing[clock->span.index].pc;
    FixedSpanWrite(&clock->span, address, value);
    if (clock->phase == DISSOLVE_BIT && pc == 0xe8d5u && !value) {
        clock->chosen = true;
        DISSOLVE_PLAN(clock, bit_hit_chosen);
    }
}

static bool DissolvePixelReserve(DissolvePixelClock *clock, uint64_t room) {
    CpuState *cpu = clock->span.cpu;
    if (cpu->master_cycles <= clock->reserved_until &&
        room <= clock->reserved_until - cpu->master_cycles)
        return true;
    if (cpu->master_cycles > UINT64_MAX - room ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x85e85fu,
            0x1ffcu, false, 1363u, room))
        return false;
    const unsigned line = g_snes->vPos;
    if (line < 225u) {
        if (!g_snes->hdmaBeamOff ||
            (225u - line) * 1364u - g_snes->hPos <= room)
            return false;
    } else if (line >= 262u ||
            (262u - line) * 1364u - g_snes->hPos <= room) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    clock->reserved_until = cpu->master_cycles + room;
    return true;
}

static uint8_t DissolvePixelBudget(void *context,
    const Lufia2CpuState *state, uint32_t pc) {
    DissolvePixelClock *clock = context;
    AdvanceDissolvePixel(clock);
    assert(clock->phase == DISSOLVE_IDLE && !clock->span.remaining);
    const uint64_t room = pc == 0x85e85fu ? 12288u :
        pc == 0x85e882u ? 4096u : 512u;
    if (!DissolvePixelReserve(clock, room))
        return 0u;
    clock->state = state;
    clock->span.bank = 0x850000u;
    clock->span.index = 0u;
    if (pc == 0x85e85fu) {
        clock->phase = DISSOLVE_CALL;
        DISSOLVE_PLAN(clock, call);
    } else if (pc == 0x85e868u) {
        clock->phase = DISSOLVE_ROW;
        DISSOLVE_PLAN(clock, row_sum_less);
    } else if (pc == 0x85e87cu) {
        clock->phase = DISSOLVE_RETRY;
        if ((uint8_t)state->accumulator) DISSOLVE_PLAN(clock, retry);
        else DISSOLVE_PLAN(clock, empty);
    } else {
        clock->phase = DISSOLVE_LOCATE;
        DISSOLVE_PLAN(clock, locate);
    }
    return 1u;
}

static uint8_t DissolvePixelRandom(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    DissolvePixelClock *clock = context;
    assert(target == 0x8082c7u && site == 0x85e85fu && frame_size == 3u);
    (void)target; (void)site; (void)frame_size;
    AdvanceDissolvePixel(clock);
    clock->phase = DISSOLVE_RANDOM;
    clock->span.bank = 0x800000u;
    clock->span.index = 0u;
    DISSOLVE_PLAN(clock, random);
    const Lufia2Memory memory =
        {DissolvePixelRead, DissolvePixelWrite, clock, NULL, NULL};
    state->program_bank = 0x80u;
    Lufia2RandomByte(&memory, state);
    const uint16_t low = (uint16_t)memory.read_byte(clock, ++state->stack);
    const uint16_t high = (uint16_t)memory.read_byte(clock, ++state->stack);
    state->program_bank = memory.read_byte(clock, ++state->stack);
    clock->returned_pc = ((uint32_t)state->program_bank << 16) |
        (uint16_t)(((high << 8) | low) + 1u);
    AdvanceDissolvePixel(clock);
    if (clock->returned_pc != 0x85e863u)
        return 0u;
    clock->phase = DISSOLVE_INITIAL;
    clock->span.bank = 0x850000u;
    clock->span.index = 0u;
    DISSOLVE_PLAN(clock, initial);
    return 1u;
}

static RecompReturn ActorBridgeDissolvePixel(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    if (cpu->D || cpu->_flag_D || cpu->x_flag || cpu->DB != 0x7eu ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f20u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !Lufia2DecompFrameRangeQuiet(0x85e85fu, 0x85e8e8u) ||
        !Lufia2DecompFrameRangeQuiet(0x8082c7u, 0x8082e7u) ||
        !Lufia2DecompFrameRangeQuiet(0x80832du, 0x80834cu))
        return ActorBridgeFallback(cpu, &frame, 0x85e85fu);
    DissolvePixelClock clock = {0};
    clock.span.cpu = cpu;
    if (!DissolvePixelReserve(&clock, 12288u))
        return ActorBridgeFallback(cpu, &frame, 0x85e85fu);
    const Lufia2Memory memory =
        {DissolvePixelRead, DissolvePixelWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleClearRandomMaskPixelWithBudget(&memory, &state,
            DissolvePixelRandom, &clock, DissolvePixelBudget, &clock);
    AdvanceDissolvePixel(&clock);
    assert(clock.phase == DISSOLVE_IDLE && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED) {
        const uint32_t pc = result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND
            ? clock.returned_pc : result.pc;
        return interp_tier_dispatch_tail(cpu, pc, pc, frame.entry_s, frame.hrv);
    }
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

#undef DISSOLVE_PLAN
