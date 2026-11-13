enum { FIELD_STAIR_BODY_MAX_OPCODES = LUFIA2_FIELD_STAIR_OPCODE_BUDGET - 1u };

typedef struct FieldStairPlan {
    FixedSpanOpcode opcodes[FIELD_STAIR_BODY_MAX_OPCODES];
    unsigned count;
} FieldStairPlan;

static bool AppendFieldStairOpcode(FieldStairPlan *plan, uint16_t pc,
    uint8_t fetched, uint8_t cycles, uint8_t data) {
    if (plan->count == FIELD_STAIR_BODY_MAX_OPCODES)
        return false;
    plan->opcodes[plan->count++] = (FixedSpanOpcode){pc, fetched, cycles, data};
    return true;
}

#define STAIR_OPCODE(pc, fetched, cycles, data) \
    do { if (!AppendFieldStairOpcode(plan, pc, fetched, cycles, data)) return false; } while (0)

static bool PlanFieldStairSearch(const uint8_t *ram, FieldStairPlan *plan,
    unsigned header, unsigned stride, unsigned *rectangle, bool *hit) {
    STAIR_OPCODE(0xb882u, 2u, 3u, 0u);
    STAIR_OPCODE(0xb884u, 2u, 4u, 2u);
    STAIR_OPCODE(0xb886u, 4u, 6u, 2u);
    STAIR_OPCODE(0xb88au, 1u, 2u, 0u);
    unsigned index = ram[0xf000u + header] | (unsigned)ram[0xf001u + header] << 8;
    for (;;) {
        /* Keep geometry separate from scratch, stack and control writes. */
        if (index > 0x0ff0u)
            return false;
        const uint8_t *bounds = ram + 0xf000u + index;
        const bool end = bounds[0] == 0xffu;
        STAIR_OPCODE(0xb88bu, 2u, 3u, 0u);
        STAIR_OPCODE(0xb88du, 4u, 5u, 1u);
        STAIR_OPCODE(0xb891u, 2u, 2u, 0u);
        STAIR_OPCODE(0xb893u, 1u, 2u, 0u);
        STAIR_OPCODE(0xb894u, 2u, end ? 3u : 2u, 0u);
        if (end) {
            *hit = false;
            break;
        }
        STAIR_OPCODE(0xb896u, 2u, 3u, 1u);
        static const uint16_t compares[] = {0xb898u, 0xb89eu, 0xb8a6u, 0xb8acu};
        static const uint16_t branches[] = {0xb89cu, 0xb8a2u, 0xb8aau, 0xb8b0u};
        static const uint8_t edges[] = {1u, 3u, 2u, 4u};
        bool inside = true;
        for (unsigned edge = 0u; edge < 4u; ++edge) {
            if (edge == 2u)
                STAIR_OPCODE(0xb8a4u, 2u, 3u, 1u);
            const unsigned probe = ram[edge < 2u ? 0x8fu : 0x91u];
            const bool outside = edge & 1u ? probe >= bounds[edges[edge]] :
                probe < bounds[edges[edge]];
            STAIR_OPCODE(compares[edge], 4u, 5u, 1u);
            STAIR_OPCODE(branches[edge], 2u, outside ? 3u : 2u, 0u);
            if (outside) {
                inside = false;
                break;
            }
        }
        if (inside) {
            STAIR_OPCODE(0xb8b2u, 1u, 2u, 0u);
            STAIR_OPCODE(0xb8b3u, 2u, 3u, 0u);
            *hit = true;
            break;
        }
        STAIR_OPCODE(0xb8b5u, 2u, 3u, 0u);
        STAIR_OPCODE(0xb8b7u, 1u, 2u, 0u);
        STAIR_OPCODE(0xb8b8u, 1u, 2u, 0u);
        STAIR_OPCODE(0xb8b9u, 2u, 4u, 2u);
        STAIR_OPCODE(0xb8bbu, 1u, 2u, 0u);
        STAIR_OPCODE(0xb8bcu, 2u, 3u, 0u);
        index += stride;
    }
    *rectangle = index;
    STAIR_OPCODE(0xb8beu, 1u, 6u, 2u);
    return true;
}

static bool PlanFieldStairClocks(const uint8_t *ram, FieldStairPlan *plan) {
    unsigned rectangle;
    bool hit;
    plan->count = 0u;
    STAIR_OPCODE(0xb66eu, 3u, 3u, 0u);
    STAIR_OPCODE(0xb671u, 3u, 3u, 0u);
    STAIR_OPCODE(0xb674u, 3u, 6u, 2u);
    if (!PlanFieldStairSearch(ram, plan, 2u, 15u, &rectangle, &hit))
        return false;
    STAIR_OPCODE(0xb677u, 2u, hit ? 3u : 2u, 0u);
    if (hit) {
        const bool enabled = ram[0xf00eu + rectangle] != 0u;
        STAIR_OPCODE(0xb685u, 3u, 5u, 1u);
        STAIR_OPCODE(0xb688u, 2u, enabled ? 3u : 2u, 0u);
        if (!enabled) {
            STAIR_OPCODE(0xb68au, 3u, 4u, 0u);
            return true;
        }
    } else {
        STAIR_OPCODE(0xb679u, 3u, 3u, 0u);
        STAIR_OPCODE(0xb67cu, 3u, 3u, 0u);
        STAIR_OPCODE(0xb67fu, 3u, 6u, 2u);
        if (!PlanFieldStairSearch(ram, plan, 10u, 5u, &rectangle, &hit))
            return false;
        STAIR_OPCODE(0xb682u, 2u, hit ? 3u : 2u, 0u);
        if (!hit)
            return true;
    }
    STAIR_OPCODE(0xb68du, 3u, 5u, 1u);
    STAIR_OPCODE(0xb690u, 2u, 3u, 1u);
    STAIR_OPCODE(0xb692u, 4u, 5u, 1u);
    STAIR_OPCODE(0xb696u, 2u, 2u, 0u);
    STAIR_OPCODE(0xb698u, 2u, 3u, 1u);
    const bool same_rectangle = (ram[0x1d0bfu] & 0x7fu) == ram[0xf000u + rectangle];
    STAIR_OPCODE(0xb69au, 2u, same_rectangle ? 3u : 2u, 0u);
    if (!same_rectangle) {
        STAIR_OPCODE(0xb69cu, 2u, 2u, 0u);
        STAIR_OPCODE(0xb69eu, 4u, 5u, 1u);
    }
    const unsigned control = same_rectangle ? ram[0x1d0bfu] : 0xffu;
    STAIR_OPCODE(0xb6a2u, 3u, 5u, 1u);
    STAIR_OPCODE(0xb6a5u, 2u, 3u, 1u);
    const bool at_top = ram[0xf002u + rectangle] == ram[0x91u];
    STAIR_OPCODE(0xb6a7u, 2u, at_top ? 3u : 2u, 0u);
    bool publish;
    if (at_top) {
        STAIR_OPCODE(0xb6d3u, 4u, 5u, 1u);
        STAIR_OPCODE(0xb6d7u, 2u, 3u, 1u);
        STAIR_OPCODE(0xb6d9u, 2u, 2u, 0u);
        STAIR_OPCODE(0xb6dbu, 2u, control == 0xffu ? 2u : 3u, 0u);
        if (control == 0xffu)
            STAIR_OPCODE(0xb6ddu, 2u, 3u, 1u);
        STAIR_OPCODE(0xb6dfu, 4u, 5u, 1u);
        STAIR_OPCODE(0xb6e3u, 2u, 2u, 0u);
        STAIR_OPCODE(0xb6e5u, 4u, 5u, 1u);
        STAIR_OPCODE(0xb6e9u, 2u, 3u, 1u);
        publish = control != 0xffu && (control & 0x80u);
        STAIR_OPCODE(0xb6ebu, 2u, publish ? 2u : 3u, 0u);
        if (publish) {
            STAIR_OPCODE(0xb6edu, 3u, 5u, 1u);
            STAIR_OPCODE(0xb6f0u, 3u, 4u, 1u);
        }
    } else {
        STAIR_OPCODE(0xb6a9u, 3u, 5u, 1u);
        STAIR_OPCODE(0xb6acu, 1u, 2u, 0u);
        STAIR_OPCODE(0xb6adu, 2u, 3u, 1u);
        const bool at_bottom = (uint8_t)(ram[0xf004u + rectangle] - 1u) == ram[0x91u];
        STAIR_OPCODE(0xb6afu, 2u, at_bottom ? 2u : 3u, 0u);
        if (!at_bottom)
            return true;
        STAIR_OPCODE(0xb6b1u, 4u, 5u, 1u);
        STAIR_OPCODE(0xb6b5u, 2u, 3u, 1u);
        STAIR_OPCODE(0xb6b7u, 2u, 2u, 0u);
        STAIR_OPCODE(0xb6b9u, 2u, control == 0xffu ? 2u : 3u, 0u);
        if (control == 0xffu)
            STAIR_OPCODE(0xb6bbu, 2u, 3u, 1u);
        STAIR_OPCODE(0xb6bdu, 4u, 5u, 1u);
        STAIR_OPCODE(0xb6c1u, 2u, 2u, 0u);
        STAIR_OPCODE(0xb6c3u, 4u, 5u, 1u);
        STAIR_OPCODE(0xb6c7u, 2u, 3u, 1u);
        publish = !(control & 0x80u);
        STAIR_OPCODE(0xb6c9u, 2u, publish ? 2u : 3u, 0u);
        if (publish) {
            STAIR_OPCODE(0xb6cbu, 3u, 5u, 1u);
            STAIR_OPCODE(0xb6ceu, 3u, 4u, 1u);
            STAIR_OPCODE(0xb6d1u, 2u, 3u, 0u);
        }
    }
    if (publish) {
        STAIR_OPCODE(0xb6f3u, 2u, 2u, 0u);
        STAIR_OPCODE(0xb6f5u, 3u, 6u, 2u);
        STAIR_OPCODE(0xb6f8u, 3u, 5u, 1u);
        STAIR_OPCODE(0xb6fbu, 3u, 4u, 1u);
        STAIR_OPCODE(0xb6feu, 3u, 4u, 1u);
        STAIR_OPCODE(0xb701u, 3u, 4u, 1u);
    }
    STAIR_OPCODE(0xb704u, 4u, 5u, 1u);
    STAIR_OPCODE(0xb708u, 2u, 2u, 0u);
    STAIR_OPCODE(0xb70au, 2u, 3u, 1u);
    STAIR_OPCODE(0xb70cu, 4u, 5u, 1u);
    return true;
}
#undef STAIR_OPCODE

static unsigned FieldStairClockReserve(const FieldStairPlan *plan) {
    unsigned master = 42u;
    for (unsigned opcode = 0u; opcode < plan->count; ++opcode) {
        const FixedSpanOpcode *timing = plan->opcodes + opcode;
        const unsigned bus = timing->fetched + timing->data_bytes;
        master += 8u * bus + 6u * (timing->cycles - bus);
    }
    return master + 80u * (master / 1284u + 2u);
}

static bool FieldStairClockSupported(const CpuState *cpu, unsigned reserve) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != 0x7eu || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        g_interp_apu_driving || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x83b66eu) ||
        Lufia2DecompFramePairedReturnPc(cpu, 0x83b66eu) != 0x838268u ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) != 0x8267u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x83b66eu, 0x1ffdu,
            false, 1363u, reserve) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x1b8beu ||
        !Lufia2DecompFrameRangeQuiet(0x83b66eu, 0x83b711u) ||
        !Lufia2DecompFrameRangeQuiet(0x83b882u, 0x83b8bfu))
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedFieldStairs(CpuState *cpu) {
    FieldStairPlan plan;
    if (!FieldStairClockSupported(cpu, 0u) ||
        !PlanFieldStairClocks(cpu->ram, &plan) ||
        !FieldStairClockSupported(cpu, FieldStairClockReserve(&plan))) {
        const uint32_t entry = (uint32_t)cpu->PB << 16 | 0xb66eu;
        return interp_tier_dispatch_tail(cpu, entry, entry, cpu->S, cpu->host_return_valid);
    }
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0x838268u};
    const uint64_t start = cpu->master_cycles;
    PairedSpanClock clock = BeginPairedSpanClock(cpu, 0x83b66eu, plan.opcodes, plan.count);
    const Lufia2Memory memory = {PairedSpanRead, PairedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldStairRects(&memory, &state);
    PreparePairedSpan(&clock);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        (result.pc == 0x83b684u || result.pc == 0x83b710u));
    assert(clock.offset == plan.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    bool progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x83b66eu, 1u, progress);
    assert(cpu->master_cycles - start <= FieldStairClockReserve(&plan));
    return returned;
}
