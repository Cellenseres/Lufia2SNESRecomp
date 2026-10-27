extern uint64_t g_interp_bridge_write_epoch;

enum { PANEL_COPY_MAX_WORDS = 128u };

static void ActorBridgePanelCopySpan(CpuState *cpu, uint32_t entry);

typedef struct PanelCopyClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[9];
    unsigned words;
    unsigned remaining_words;
    unsigned completed_opcodes;
    unsigned last_write;
    uint16_t source_x;
    uint16_t remaining_y;
    uint16_t loop;
    uint32_t next;
} PanelCopyClock;

static void PlanPanelCopyWord(PanelCopyClock *clock, unsigned first) {
    const uint16_t loop = clock->loop;
    const unsigned read_cycles = 5u + (clock->source_x >= 0x100u);
    const unsigned branch_cycles = clock->remaining_y == 1u ? 2u : 3u;
    clock->timing[first] = (FixedSpanOpcode){loop, 3u, (uint8_t)read_cycles, 2u};
    clock->timing[first + 1u] = (FixedSpanOpcode){(uint16_t)(loop + 3u), 3u, 7u, 2u};
    clock->timing[first + 2u] = (FixedSpanOpcode){(uint16_t)(loop + 6u), 1u, 2u, 0u};
    clock->timing[first + 3u] = (FixedSpanOpcode){(uint16_t)(loop + 7u), 1u, 2u, 0u};
    clock->timing[first + 4u] = (FixedSpanOpcode){(uint16_t)(loop + 8u), 1u, 2u, 0u};
    clock->timing[first + 5u] = (FixedSpanOpcode){
        (uint16_t)(loop + 9u), 2u, (uint8_t)branch_cycles, 0u};
    clock->span.count = first + 6u;
}

static void BeginPanelCopyOpcode(PanelCopyClock *clock) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count || clock->span.remaining)
        return;
    clock->completed_opcodes += clock->span.count;
    if (!--clock->remaining_words)
        return;
    clock->source_x = (uint16_t)(clock->source_x + 2u);
    --clock->remaining_y;
    clock->span.index = 0u;
    clock->span.progress_mask = 0u;
    PlanPanelCopyWord(clock, 0u);
    BeginFixedSpanOpcode(&clock->span);
}

static uint8_t PanelCopyRead(void *context, uint32_t address) {
    PanelCopyClock *clock = context;
    BeginPanelCopyOpcode(clock);
    return FixedSpanRead(&clock->span, address);
}

static void PanelCopyWrite(void *context, uint32_t address, uint8_t value) {
    PanelCopyClock *clock = context;
    BeginPanelCopyOpcode(clock);
    const uint64_t before = g_interp_bridge_write_epoch;
    FixedSpanWrite(&clock->span, address, value);
    if (before != g_interp_bridge_write_epoch)
        clock->last_write = clock->completed_opcodes + clock->span.index +
            (clock->span.remaining != 0u);
}

static bool PreparePanelCopyClock(PanelCopyClock *clock, CpuState *cpu,
    uint32_t entry) {
    const bool initialize = entry == 0x84ef60u || entry == 0x84ef74u;
    const bool first = entry == 0x84ef60u || entry == 0x84ef68u;
    if ((!first && entry != 0x84ef74u && entry != 0x84ef7cu) ||
        cpu->PB != 0x84u || cpu->D || cpu->_flag_D || cpu->x_flag ||
        cpu->m_flag > 1u || (!initialize && cpu->m_flag) ||
        cpu->host_return_valid != 2u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x280000u ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const uint32_t loop = first ? 0x84ef68u : 0x84ef7cu;
    const uint32_t end = loop + 11u;
    if (initialize && !Lufia2DecompFrameRangeQuiet(entry, loop))
        return false;
    const unsigned opcode_budget =
        Lufia2DecompFrameSpanBudget(loop, end, ActorBridgePanelCopySpan);
    const unsigned setup_opcodes = initialize ? 3u : 0u;
    if (opcode_budget < setup_opcodes + 6u)
        return false;
    const unsigned x = initialize ? 0u : cpu->X;
    const unsigned y = initialize ? 1024u : cpu->Y;
    if (!y || y > 1024u || (x & 1u) || x > 0x7feu)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    uint64_t room = (uint64_t)lines * 1364u - g_snes->hPos - 1u;
    const uint64_t deadline_room = Lufia2DecompFrameReturnDeadline() - cpu->master_cycles;
    if (room > deadline_room)
        room = deadline_room;
    unsigned limit = (opcode_budget - setup_opcodes) / 6u;
    if (limit > PANEL_COPY_MAX_WORDS)
        limit = PANEL_COPY_MAX_WORDS;
    if (limit > y)
        limit = y;
    if (limit > (0x800u - x) / 2u)
        limit = (0x800u - x) / 2u;
    while (limit) {
        const uint64_t bound = 64u + (initialize ? 102u : 0u) + limit * 222u;
        const uint64_t reserved = bound + 40u * (bound / 1324u + 1u);
        if (reserved <= room && cpu->master_cycles <= UINT64_MAX - reserved)
            break;
        --limit;
    }
    if (!limit)
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->words = clock->remaining_words = limit;
    clock->completed_opcodes = clock->last_write = 0u;
    clock->source_x = (uint16_t)x;
    clock->remaining_y = (uint16_t)y;
    clock->loop = (uint16_t)loop;
    clock->next = limit == y ? end : loop;
    if (initialize) {
        clock->timing[0] = (FixedSpanOpcode){(uint16_t)entry, 2u, 3u, 0u};
        clock->timing[1] = (FixedSpanOpcode){(uint16_t)(entry + 2u), 3u, 3u, 0u};
        clock->timing[2] = (FixedSpanOpcode){(uint16_t)(entry + 5u), 3u, 3u, 0u};
    }
    PlanPanelCopyWord(clock, setup_opcodes);
    return true;
}

static uint32_t RunPanelCopySpan(PanelCopyClock *clock, uint32_t entry) {
    const Lufia2Memory memory = {PanelCopyRead, PanelCopyWrite, clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(clock->span.cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2CardTableCopyPanelSpan(&memory, &state, entry, clock->words);
    assert(result.pc == clock->next);
    ActorBridgeStore(clock->span.cpu, &state);
    BeginPanelCopyOpcode(clock);
    assert(!clock->remaining_words && !clock->span.remaining);
    return result.pc;
}

static void ActorBridgePanelCopySpan(CpuState *cpu, uint32_t entry) {
    PanelCopyClock clock;
    if (!PreparePanelCopyClock(&clock, cpu, entry))
        return;
    const uint32_t next = RunPanelCopySpan(&clock, entry);
    Lufia2DecompFrameReturnCompleteSpan(clock.completed_opcodes, clock.last_write);
    interp_bridge_pre_opcode_redirect(next);
}
