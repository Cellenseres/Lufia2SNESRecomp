#include "lufia2/battle_target_marker_steps.h"

static const FixedSpanOpcode target_marker_unavailable[2][8] = {
    {
        {0xd60eu, 3u, 4u, 1u},
        {0xd611u, 2u, 3u, 0u},
        {0xd61bu, 1u, 2u, 0u},
        {0xd61cu, 1u, 2u, 0u},
        {0xd61du, 1u, 2u, 0u},
        {0xd61eu, 1u, 2u, 0u},
        {0xd61fu, 2u, 5u, 2u},
        {0xd621u, 2u, 3u, 0u}
    },
    {
        {0xd60eu, 3u, 4u, 1u},
        {0xd611u, 2u, 3u, 0u},
        {0xd61bu, 1u, 2u, 0u},
        {0xd61cu, 1u, 2u, 0u},
        {0xd61du, 1u, 2u, 0u},
        {0xd61eu, 1u, 2u, 0u},
        {0xd61fu, 2u, 5u, 2u},
        {0xd621u, 2u, 2u, 0u}
    },
};

static const FixedSpanOpcode target_marker_unselected[2][10] = {
    {
        {0xd60eu, 3u, 4u, 1u},
        {0xd611u, 2u, 2u, 0u},
        {0xd613u, 3u, 4u, 1u},
        {0xd616u, 2u, 3u, 0u},
        {0xd61bu, 1u, 2u, 0u},
        {0xd61cu, 1u, 2u, 0u},
        {0xd61du, 1u, 2u, 0u},
        {0xd61eu, 1u, 2u, 0u},
        {0xd61fu, 2u, 5u, 2u},
        {0xd621u, 2u, 3u, 0u}
    },
    {
        {0xd60eu, 3u, 4u, 1u},
        {0xd611u, 2u, 2u, 0u},
        {0xd613u, 3u, 4u, 1u},
        {0xd616u, 2u, 3u, 0u},
        {0xd61bu, 1u, 2u, 0u},
        {0xd61cu, 1u, 2u, 0u},
        {0xd61du, 1u, 2u, 0u},
        {0xd61eu, 1u, 2u, 0u},
        {0xd61fu, 2u, 5u, 2u},
        {0xd621u, 2u, 2u, 0u}
    },
};

static const FixedSpanOpcode target_marker_selected[4] = {
    {0xd60eu, 3u, 4u, 1u}, {0xd611u, 2u, 2u, 0u}, {0xd613u, 3u, 4u, 1u}, {0xd616u, 2u, 2u, 0u}
};

typedef struct TargetMarkerClock {
    FixedSpanClock span;
    unsigned cursor, counter, records_left, opcodes, progress_mask;
    bool selected;
} TargetMarkerClock;

static void SetTargetMarkerRecordClock(TargetMarkerClock *clock) {
    CpuState *cpu = clock->span.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x810000u;
    clock->selected = false;
    if (cpu->ram[0x1bb2u + clock->cursor]) {
        clock->span.timing = target_marker_unavailable[clock->counter == 1u];
        clock->span.count = 8u;
    } else if (!cpu->ram[0x1bb3u + clock->cursor]) {
        clock->span.timing = target_marker_unselected[clock->counter == 1u];
        clock->span.count = 10u;
    } else {
        clock->span.timing = target_marker_selected;
        clock->span.count = 4u;
        clock->selected = true;
    }
}

static void PrepareTargetMarkerRecord(TargetMarkerClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count)
        return;
    clock->progress_mask |= clock->span.progress_mask << clock->opcodes;
    clock->opcodes += clock->span.count;
    if (!clock->selected && --clock->counter && --clock->records_left) {
        clock->cursor += 4u;
        SetTargetMarkerRecordClock(clock);
    }
}

static uint8_t TargetMarkerRead(void *context, uint32_t address) {
    TargetMarkerClock *clock = context;
    PrepareTargetMarkerRecord(clock);
    return FixedSpanRead(&clock->span, address);
}

static void TargetMarkerWrite(void *context, uint32_t address, uint8_t value) {
    TargetMarkerClock *clock = context;
    PrepareTargetMarkerRecord(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool TargetMarkerHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x81d60eu && hook == ActorBridgeTargetMarkerRecords;
}

static unsigned TargetMarkerSpanSize(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x81d60eu || cpu->DB != 0x7eu || cpu->D ||
        cpu->m_flag != 1u || cpu->x_flag || !cpu->ram ||
        cpu->Y >= 48u || (cpu->Y & 3u) ||
        cpu->ram[0x25u] != 12u - cpu->Y / 4u ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, false,
            1363u, 1u) || g_snes->vPos >= 262u ||
        g_snes->dma->hdmaPendingInit ||
        !Lufia2DecompFrameRangeAllows(entry, 0x81d623u, TargetMarkerHookOwned))
        return 0u;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return 0u;
    unsigned records = cpu->ram[0x25u];
    if (records > 3u)
        records = 3u;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    while (records) {
        const unsigned room = 29u * records * 8u + 80u;
        if (g_snes->hPos < fence && room < fence - g_snes->hPos &&
            cpu->master_cycles <= UINT64_MAX - room &&
            deadline - cpu->master_cycles >= room)
            return records;
        --records;
    }
    return 0u;
}

static void ActorBridgeTargetMarkerRecords(CpuState *cpu, uint32_t entry) {
    const unsigned records = TargetMarkerSpanSize(cpu, entry);
    if (!records)
        return;
    TargetMarkerClock clock = {0};
    clock.span.cpu = cpu;
    clock.cursor = cpu->Y;
    clock.counter = cpu->ram[0x25u];
    clock.records_left = records;
    SetTargetMarkerRecordClock(&clock);
    const Lufia2Memory memory = {
        TargetMarkerRead, TargetMarkerWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BattleVisitTargetMarkerSpan(&memory, &state, entry, records);
    PrepareTargetMarkerRecord(&clock);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.opcodes, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
