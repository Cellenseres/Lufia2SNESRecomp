#include "field_diagnostic_plans.inc"

typedef struct FieldDiagnosticClock {
    FixedSpanClock span;
    unsigned operation;
    unsigned phase;
    unsigned cell;
    unsigned link;
    bool narrow;
    bool controller;
    bool waiting;
    ActorPushedCall call;
} FieldDiagnosticClock;

#define DIAGNOSTIC_SPAN(clock, name) \
    SetDiagnosticSpan(clock, diagnostic_##name, \
        (unsigned)(sizeof diagnostic_##name / sizeof diagnostic_##name[0]))

static void SetDiagnosticSpan(FieldDiagnosticClock *clock,
    const FixedSpanOpcode *timing, unsigned count) {
    clock->span.timing = timing;
    clock->span.count = count;
    clock->span.index = 0u;
}

static unsigned DiagnosticInputPath(const CpuState *cpu, unsigned held, unsigned mask) {
    if (!(cpu->ram[held] & mask))
        return 0u;
    return cpu->ram[held + 4u] & mask ? 2u : 1u;
}

static void SelectDiagnosticController(FieldDiagnosticClock *clock) {
    const CpuState *cpu = clock->span.cpu;
    if (clock->waiting) {
        clock->span.count = 0u;
        return;
    }
    unsigned path;
    switch (clock->phase++) {
    case 0u:
        path = DiagnosticInputPath(cpu, 0x49u, 0x20u);
        if (!path) DIAGNOSTIC_SPAN(clock, control_0_0);
        else if (path == 1u) DIAGNOSTIC_SPAN(clock, control_0_1);
        else DIAGNOSTIC_SPAN(clock, control_0_2);
        clock->waiting = path == 2u;
        break;
    case 1u:
        if (cpu->ram[(cpu->DB == 0x7fu ? 0x10000u : 0u) + 0x57cu]) {
            DIAGNOSTIC_SPAN(clock, control_1_1);
        } else {
            DIAGNOSTIC_SPAN(clock, control_1_0);
            clock->phase = 8u;
        }
        break;
    case 2u:
        path = DiagnosticInputPath(cpu, 0x47u, 0x10u);
        if (!path) DIAGNOSTIC_SPAN(clock, control_2_0);
        else if (path == 1u) DIAGNOSTIC_SPAN(clock, control_2_1);
        else DIAGNOSTIC_SPAN(clock, control_2_2);
        clock->waiting = path == 2u;
        if (!clock->waiting) clock->phase = 4u;
        break;
    case 3u:
        DIAGNOSTIC_SPAN(clock, control_3_0);
        clock->waiting = true;
        break;
    case 4u:
        path = DiagnosticInputPath(cpu, 0x49u, 0x40u);
        if (!path) DIAGNOSTIC_SPAN(clock, control_4_0);
        else if (path == 1u) DIAGNOSTIC_SPAN(clock, control_4_1);
        else DIAGNOSTIC_SPAN(clock, control_4_2);
        clock->waiting = path == 2u;
        break;
    case 5u:
        path = DiagnosticInputPath(cpu, 0x49u, 0x80u);
        if (!path) DIAGNOSTIC_SPAN(clock, control_5_0);
        else if (path == 1u) DIAGNOSTIC_SPAN(clock, control_5_1);
        else DIAGNOSTIC_SPAN(clock, control_5_2);
        clock->waiting = path == 2u;
        break;
    case 6u:
        path = DiagnosticInputPath(cpu, 0x48u, 0x80u);
        if (!path) DIAGNOSTIC_SPAN(clock, control_6_0);
        else if (path == 1u) DIAGNOSTIC_SPAN(clock, control_6_1);
        else DIAGNOSTIC_SPAN(clock, control_6_2);
        break;
    case 7u:
        if (cpu->ram[0x1e75au]) {
            DIAGNOSTIC_SPAN(clock, control_7_1);
            clock->phase = 11u;
        } else {
            DIAGNOSTIC_SPAN(clock, control_7_0);
        }
        break;
    case 8u:
        DIAGNOSTIC_SPAN(clock, control_8_0);
        clock->phase = 10u;
        break;
    case 9u:
        DIAGNOSTIC_SPAN(clock, control_map_return);
        clock->phase = 8u;
        break;
    case 11u:
        clock->operation = 6u;
        clock->phase = clock->cell = clock->link = 0u;
        DIAGNOSTIC_SPAN(clock, map_setup);
        break;
    default:
        clock->span.count = 0u;
        break;
    }
}

static void NextDiagnosticSpan(FieldDiagnosticClock *clock) {
    if (clock->operation == 8u) {
        SelectDiagnosticController(clock);
    } else if (clock->operation == 6u) {
        if (clock->phase == 0u) {
            const bool clear = clock->span.cpu->ram[0x1ea00u + clock->cell] == 0u;
            if (clock->cell == 255u) {
                if (clear) DIAGNOSTIC_SPAN(clock, map_cell_clear_last);
                else DIAGNOSTIC_SPAN(clock, map_cell_glyph_last);
                clock->phase = 1u;
            } else if ((clock->cell & 15u) == 15u) {
                if (clear) DIAGNOSTIC_SPAN(clock, map_cell_clear_row);
                else DIAGNOSTIC_SPAN(clock, map_cell_glyph_row);
            } else {
                if (clear) DIAGNOSTIC_SPAN(clock, map_cell_clear_next);
                else DIAGNOSTIC_SPAN(clock, map_cell_glyph_next);
            }
            ++clock->cell;
        } else if (clock->phase == 1u) {
            DIAGNOSTIC_SPAN(clock, map_values);
            clock->phase = 2u;
        } else if (clock->phase == 2u) {
            if (clock->span.cpu->ram[0x1e6f1u + clock->link] == 0xffu) {
                if (clock->link < 15u) DIAGNOSTIC_SPAN(clock, map_link_stop_page);
                else DIAGNOSTIC_SPAN(clock, map_link_stop_cross);
                clock->phase = 3u;
            } else if (clock->link == 63u) {
                DIAGNOSTIC_SPAN(clock, map_link_last_cross);
                clock->phase = 3u;
            } else if (clock->link < 15u) {
                DIAGNOSTIC_SPAN(clock, map_link_next_page);
            } else {
                DIAGNOSTIC_SPAN(clock, map_link_next_cross);
            }
            ++clock->link;
        } else if (clock->phase == 3u) {
            DIAGNOSTIC_SPAN(clock, map_finish);
            clock->phase = 4u;
        } else if (clock->controller) {
            clock->operation = 8u;
            clock->phase = 9u;
            SelectDiagnosticController(clock);
        } else {
            clock->span.count = 0u;
        }
    } else if (clock->operation == 7u) {
        if (clock->cell < 64u) {
            if (++clock->cell == 64u) DIAGNOSTIC_SPAN(clock, sprite_cell_last_cross);
            else DIAGNOSTIC_SPAN(clock, sprite_cell_next_cross);
        } else if (!clock->phase++) {
            if (clock->narrow) DIAGNOSTIC_SPAN(clock, sprite_finish_1);
            else DIAGNOSTIC_SPAN(clock, sprite_finish_0);
        } else {
            clock->span.count = 0u;
        }
    } else {
        clock->span.count = 0u;
    }
}

static void AdvanceDiagnosticClock(FieldDiagnosticClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.remaining || !clock->span.count)
            return;
        assert(clock->span.index == clock->span.count);
        NextDiagnosticSpan(clock);
    }
}

static uint8_t FieldDiagnosticRead(void *context, uint32_t address) {
    FieldDiagnosticClock *clock = context;
    AdvanceDiagnosticClock(clock);
    return FixedSpanRead(&clock->span, address);
}

static void FieldDiagnosticWrite(void *context, uint32_t address, uint8_t value) {
    FieldDiagnosticClock *clock = context;
    AdvanceDiagnosticClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool FieldDiagnosticWindow(const CpuState *cpu, uint32_t entry, unsigned operation) {
    unsigned budget = operation == 6u || operation == 8u ? 180000u : operation == 7u ? 64000u : 2048u;
    if (operation == 8u && cpu->ram && (cpu->DB == 0x7eu || (cpu->DB & 0x7fu) < 0x40u) &&
        !cpu->ram[0x57cu] && !(cpu->ram[0x49u] & cpu->ram[0x4du] & 0x20u))
        budget = 2048u;
    const unsigned frame = operation == 2u || operation == 3u || operation == 7u || operation == 8u ? 3u : 2u;
    if (cpu->D || cpu->_flag_D || cpu->S < 0x1f20u ||
        cpu->host_return_valid != frame ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, budget) ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        !Lufia2DecompFrameRangeQuiet(0x8eb0d4u, 0x8eb1d4u) ||
        !Lufia2DecompFrameRangeQuiet(0x8eb1d4u, 0x8eb288u) ||
        !Lufia2DecompFrameRangeQuiet(0x8584c6u, 0x85850eu))
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
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    return true;
}

static bool PrepareFieldDiagnostic(FieldDiagnosticClock *clock, CpuState *cpu,
    uint32_t entry, unsigned operation) {
    if (!FieldDiagnosticWindow(cpu, entry, operation))
        return false;
    *clock = (FieldDiagnosticClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = entry & 0xff0000u;
    clock->operation = operation;
    clock->narrow = cpu->x_flag != 0u;
    switch (operation) {
    case 0u:
        if (cpu->A & cpu->ram[0x49u] & 0xffu) DIAGNOSTIC_SPAN(clock, input_0_held);
        else DIAGNOSTIC_SPAN(clock, input_0_empty);
        break;
    case 1u:
        if (cpu->A & cpu->ram[0x48u] & 0xffu) DIAGNOSTIC_SPAN(clock, input_1_held);
        else DIAGNOSTIC_SPAN(clock, input_1_empty);
        break;
    case 2u:
        if (cpu->A & cpu->ram[0x46u] & 0xffu) DIAGNOSTIC_SPAN(clock, input_2_held);
        else DIAGNOSTIC_SPAN(clock, input_2_empty);
        break;
    case 3u:
        if (cpu->A & cpu->ram[0x47u] & 0xffu) DIAGNOSTIC_SPAN(clock, input_3_held);
        else DIAGNOSTIC_SPAN(clock, input_3_empty);
        break;
    case 4u: DIAGNOSTIC_SPAN(clock, value); break;
    case 5u: DIAGNOSTIC_SPAN(clock, divide); break;
    case 6u: DIAGNOSTIC_SPAN(clock, map_setup); break;
    case 7u:
        if (clock->narrow) DIAGNOSTIC_SPAN(clock, sprite_setup_1);
        else DIAGNOSTIC_SPAN(clock, sprite_setup_0);
        break;
    case 8u:
        clock->controller = true;
        SelectDiagnosticController(clock);
        break;
    default: return false;
    }
    return true;
}

static RecompReturn ActorBridgeFieldDiagnostic(CpuState *cpu, uint32_t entry,
    unsigned operation, ActorWholeFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    FieldDiagnosticClock clock;
    if (!PrepareFieldDiagnostic(&clock, cpu, entry, operation))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FieldDiagnosticRead, FieldDiagnosticWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state);
    AdvanceDiagnosticClock(&clock);
    assert(!clock.span.count && !clock.span.remaining && result.flow == LUFIA2_EXECUTION_RETURNED);
    ActorBridgeStore(cpu, &state);
    if (operation == 2u || operation == 3u || operation == 7u)
        return ReturnTimedLongCall(cpu, &frame, result.pc);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

static uint8_t FieldDiagnosticChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    FieldDiagnosticClock *clock = context;
    AdvanceDiagnosticClock(clock);
    assert(clock->waiting && !clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (FieldDiagnosticWindow(clock->call.cpu, landing, 8u)) {
        clock->waiting = false;
        SelectDiagnosticController(clock);
        return 1u;
    }
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeDeveloperControls(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    FieldDiagnosticClock clock;
    if (!PrepareFieldDiagnostic(&clock, cpu, 0x8eb0d4u, 8u))
        return ActorBridgeFallback(cpu, &frame, 0x8eb0d4u);
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    const Lufia2Memory memory = {FieldDiagnosticRead, FieldDiagnosticWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldRunDeveloperControls(
        &memory, &state, FieldDiagnosticChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    AdvanceDiagnosticClock(&clock);
    ActorBridgeStore(cpu, &state);
    assert(!clock.span.count && !clock.span.remaining);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ReturnTimedLongCall(cpu, &frame, result.pc);
}

#undef DIAGNOSTIC_SPAN
