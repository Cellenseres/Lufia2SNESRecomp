enum { COLUMN_SPAN_CAPACITY = 512u };

typedef struct ColumnSpanClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[COLUMN_SPAN_CAPACITY];
    uint64_t master_bound;
    unsigned units;
    unsigned last_write;
    uint32_t next;
} ColumnSpanClock;

static bool ColumnSpanOpcode(ColumnSpanClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == COLUMN_SPAN_CAPACITY)
        return false;
    clock->timing[clock->span.count++] = op;
    clock->master_bound += op.fetched * cpu_region_speed(0x840000u | op.pc) +
        op.data_bytes * 12u + (op.cycles - op.fetched - op.data_bytes) * 6u;
    return true;
}

static bool ColumnSpanRange(ColumnSpanClock *clock, unsigned first,
    unsigned last, uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableMotionOpcode *record = table_motion_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!ColumnSpanOpcode(clock, op))
            return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}

static bool ColumnSpanBranch(ColumnSpanClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_motion_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return ColumnSpanOpcode(clock, op);
}

static bool ColumnSpanDrawRow(ColumnSpanClock *clock, unsigned row,
    unsigned remaining) {
    const CpuState *cpu = clock->span.cpu;
    const uint16_t pointer = MotionDirectWord(cpu, 8u);
    const uint16_t source = (uint16_t)(MotionDirectWord(cpu, 0x24u) + row * 2u);
    uint16_t symbol = MotionDirectWord(cpu, pointer + source);
    if (!ColumnSpanRange(clock, MOTION_PC_EAB4, MOTION_PC_EAB8, 0u, source) ||
        !ColumnSpanBranch(clock, MOTION_PC_EABB, source != 48u))
        return false;
    if (source == 48u) {
        if (!ColumnSpanRange(clock, MOTION_PC_EABD, MOTION_PC_EAC0, 0u, source))
            return false;
        symbol = 9u;
    } else {
        const bool below = ((uint16_t)(symbol - 8u) & 0x8000u) != 0u;
        if (!ColumnSpanRange(clock, MOTION_PC_EAC2, MOTION_PC_EAC2, 0u, source) ||
            !ColumnSpanBranch(clock, MOTION_PC_EAC5, below))
            return false;
        if (!below) {
            if (!ColumnSpanRange(clock, MOTION_PC_EAC7, MOTION_PC_EAC8, 0u, source))
                return false;
            symbol = (uint16_t)(symbol - 3u);
        }
    }
    if (symbol > 255u)
        return false;
    const uint16_t tile = (uint16_t)(symbol << 3);
    const uint16_t output = (uint16_t)(MotionDirectWord(cpu, 0x26u) + row * 16u);
    if (!ColumnSpanRange(clock, MOTION_PC_EACB, MOTION_PC_EB2E, tile, output) ||
        !ColumnSpanBranch(clock, MOTION_PC_EB30, row + 1u == remaining))
        return false;
    return row + 1u == remaining ||
        ColumnSpanRange(clock, MOTION_PC_EB32, MOTION_PC_EB32, 0u, 0u);
}

static bool PrepareColumnSpan(ColumnSpanClock *clock, CpuState *cpu, uint32_t pc) {
    const bool drawing = pc == 0x84eab4u;
    const uint32_t end = drawing ? 0x84eb35u : 0x84ea40u;
    if ((pc != 0x84ea30u && !drawing) || cpu->PB != 0x84u ||
        cpu->D || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->host_return_valid != 2u || (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, pc, 0x1ff7u, true, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x28000u ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const unsigned opcode_budget =
        Lufia2DecompFrameSpanBudget(pc, end, ActorBridgeColumnSpan);
    if (!opcode_budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    unsigned remaining;
    if (drawing) {
        const uint16_t pointer = MotionDirectWord(cpu, 8u);
        remaining = MotionDirectWord(cpu, 0x28u);
        const uint16_t source = MotionDirectWord(cpu, 0x24u);
        if (!remaining || remaining > 6u || source > 66u || (source & 1u) ||
            cpu->ram[10u] != 0x7eu || MotionDirectWord(cpu, 0x26u) > 0x180u ||
            (pointer != 0x7006u && pointer != 0x7040u && pointer != 0x707au))
            return false;
    } else {
        remaining = cpu->Y;
        if (!remaining || remaining > 24u || cpu->X > 0x120u)
            return false;
    }
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    uint64_t room = (uint64_t)lines * 1364u - g_snes->hPos - 1u;
    const uint64_t deadline_room = Lufia2DecompFrameReturnDeadline() - cpu->master_cycles;
    if (room > deadline_room)
        room = deadline_room;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->master_bound = 64u;
    clock->units = 0u;
    clock->last_write = 0u;
    clock->next = pc;
    for (unsigned unit = 0u; unit < remaining; ++unit) {
        const unsigned old_count = clock->span.count;
        const uint64_t old_bound = clock->master_bound;
        const bool planned = drawing ? ColumnSpanDrawRow(clock, unit, remaining) :
            (ColumnSpanRange(clock, MOTION_PC_EA30, MOTION_PC_EA3D,
                (uint16_t)(cpu->X + unit * 4u), (uint16_t)(remaining - unit)) &&
             ColumnSpanBranch(clock, MOTION_PC_EA3E, unit + 1u != remaining));
        const uint64_t bound = clock->master_bound +
            40u * (clock->master_bound / 1324u + 1u);
        if (!planned || bound > room || clock->span.count > opcode_budget) {
            clock->span.count = old_count;
            clock->master_bound = old_bound;
            break;
        }
        ++clock->units;
    }
    if (clock->units == remaining)
        clock->next = end;
    return clock->units != 0u;
}

static uint8_t ColumnSpanRead(void *context, uint32_t address) {
    ColumnSpanClock *clock = context;
    return FixedSpanRead(&clock->span, address);
}

static void ColumnSpanWrite(void *context, uint32_t address, uint8_t value) {
    ColumnSpanClock *clock = context;
    FixedSpanWrite(&clock->span, address, value);
    clock->last_write = clock->span.index + (clock->span.remaining != 0u);
}

static void ActorBridgeColumnSpan(CpuState *cpu, uint32_t pc) {
    ColumnSpanClock clock;
    if (!PrepareColumnSpan(&clock, cpu, pc))
        return;
    const Lufia2Memory memory = {ColumnSpanRead, ColumnSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = pc == 0x84ea30u ?
        Lufia2CardTableContinueSpriteStrip(&memory, &state, clock.units) :
        Lufia2CardTableContinueColumnDrawing(&memory, &state, clock.units);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == clock.next);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    Lufia2DecompFrameReturnCompleteSpan(clock.span.count, clock.last_write);
    interp_bridge_pre_opcode_redirect(result.pc);
}
