/* The owning loop also retires the original JSR. */
enum { FIELD_RECTANGLE_BODY_MAX_OPCODES = LUFIA2_FIELD_RECTANGLE_OPCODE_BUDGET - 2u };

typedef struct FieldRectanglePlan {
    FixedSpanOpcode opcodes[FIELD_RECTANGLE_BODY_MAX_OPCODES];
    unsigned count;
} FieldRectanglePlan;

static bool AppendFieldRectangleOpcode(FieldRectanglePlan *plan, uint16_t pc,
    uint8_t fetched, uint8_t cycles, uint8_t data) {
    if (plan->count == FIELD_RECTANGLE_BODY_MAX_OPCODES)
        return false;
    plan->opcodes[plan->count++] = (FixedSpanOpcode){pc, fetched, cycles, data};
    return true;
}

#define RECTANGLE_OPCODE(pc, fetched, cycles, data) \
    do { if (!AppendFieldRectangleOpcode(plan, pc, fetched, cycles, data)) return false; } while (0)

static bool PlanFieldRectangleSearch(const uint8_t *ram, FieldRectanglePlan *plan,
    unsigned header, unsigned stride, unsigned *rectangle, bool *hit) {
    RECTANGLE_OPCODE(0xb882u, 2u, 3u, 0u);
    RECTANGLE_OPCODE(0xb884u, 2u, 4u, 2u);
    RECTANGLE_OPCODE(0xb886u, 4u, 6u, 2u);
    RECTANGLE_OPCODE(0xb88au, 1u, 2u, 0u);
    unsigned index = ram[0xf000u + header] | (unsigned)ram[0xf001u + header] << 8;
    for (;;) {
        /* Keep geometry separate from scratch, stack and control writes. */
        if (index > 0x0ff0u)
            return false;
        const uint8_t *bounds = ram + 0xf000u + index;
        const bool end = bounds[0] == 0xffu;
        RECTANGLE_OPCODE(0xb88bu, 2u, 3u, 0u);
        RECTANGLE_OPCODE(0xb88du, 4u, 5u, 1u);
        RECTANGLE_OPCODE(0xb891u, 2u, 2u, 0u);
        RECTANGLE_OPCODE(0xb893u, 1u, 2u, 0u);
        RECTANGLE_OPCODE(0xb894u, 2u, end ? 3u : 2u, 0u);
        if (end) {
            *hit = false;
            break;
        }
        RECTANGLE_OPCODE(0xb896u, 2u, 3u, 1u);
        static const uint16_t compares[] = {0xb898u, 0xb89eu, 0xb8a6u, 0xb8acu};
        static const uint16_t branches[] = {0xb89cu, 0xb8a2u, 0xb8aau, 0xb8b0u};
        static const uint8_t edges[] = {1u, 3u, 2u, 4u};
        bool inside = true;
        for (unsigned edge = 0u; edge < 4u; ++edge) {
            if (edge == 2u)
                RECTANGLE_OPCODE(0xb8a4u, 2u, 3u, 1u);
            const unsigned probe = ram[edge < 2u ? 0x8fu : 0x91u];
            const bool outside = edge & 1u ? probe >= bounds[edges[edge]] :
                probe < bounds[edges[edge]];
            RECTANGLE_OPCODE(compares[edge], 4u, 5u, 1u);
            RECTANGLE_OPCODE(branches[edge], 2u, outside ? 3u : 2u, 0u);
            if (outside) {
                inside = false;
                break;
            }
        }
        if (inside) {
            RECTANGLE_OPCODE(0xb8b2u, 1u, 2u, 0u);
            RECTANGLE_OPCODE(0xb8b3u, 2u, 3u, 0u);
            *hit = true;
            break;
        }
        RECTANGLE_OPCODE(0xb8b5u, 2u, 3u, 0u);
        RECTANGLE_OPCODE(0xb8b7u, 1u, 2u, 0u);
        RECTANGLE_OPCODE(0xb8b8u, 1u, 2u, 0u);
        RECTANGLE_OPCODE(0xb8b9u, 2u, 4u, 2u);
        RECTANGLE_OPCODE(0xb8bbu, 1u, 2u, 0u);
        RECTANGLE_OPCODE(0xb8bcu, 2u, 3u, 0u);
        index += stride;
    }
    *rectangle = index;
    RECTANGLE_OPCODE(0xb8beu, 1u, 6u, 2u);
    return true;
}

#if defined(LUFIA2_HAVE_NATIVE_FIELD_RECTANGLE_CLOCK)
static bool PlanFieldRectangleClocks(const CpuState *cpu, uint32_t entry,
    FieldRectanglePlan *plan) {
    unsigned rectangle;
    bool hit;
    plan->count = 0u;
    if (entry == 0x83b882u) {
        if (!PlanFieldRectangleSearch(cpu->ram, plan, cpu->X, cpu->Y, &rectangle, &hit))
            return false;
        --plan->count;
        return true;
    }
    if (entry == 0x83b711u) {
        RECTANGLE_OPCODE(0xb711u, 3u, 3u, 0u);
        RECTANGLE_OPCODE(0xb714u, 3u, 3u, 0u);
        RECTANGLE_OPCODE(0xb717u, 3u, 6u, 2u);
        if (!PlanFieldRectangleSearch(cpu->ram, plan, 12u, 5u, &rectangle, &hit))
            return false;
        RECTANGLE_OPCODE(0xb71au, 2u, hit ? 2u : 3u, 0u);
        if (hit) {
            RECTANGLE_OPCODE(0xb71cu, 3u, 5u, 1u);
            RECTANGLE_OPCODE(0xb71fu, 3u, 3u, 0u);
        }
        return true;
    }
    RECTANGLE_OPCODE(0xb747u, 2u, 3u, 0u);
    RECTANGLE_OPCODE(0xb749u, 3u, 3u, 0u);
    RECTANGLE_OPCODE(0xb74cu, 3u, 3u, 0u);
    RECTANGLE_OPCODE(0xb74fu, 3u, 6u, 2u);
    if (!PlanFieldRectangleSearch(cpu->ram, plan, 6u, 9u, &rectangle, &hit))
        return false;
    RECTANGLE_OPCODE(0xb752u, 2u, hit ? 2u : 3u, 0u);
    if (!hit)
        return true;
    RECTANGLE_OPCODE(0xb754u, 3u, 5u, 1u);
    RECTANGLE_OPCODE(0xb757u, 2u, 2u, 0u);
    RECTANGLE_OPCODE(0xb759u, 2u, 2u, 0u);
    const bool directional = (cpu->ram[0xf005u + rectangle] & 15u) == 2u;
    RECTANGLE_OPCODE(0xb75bu, 2u, directional ? 2u : 3u, 0u);
    if (directional) {
        RECTANGLE_OPCODE(0xb75du, 3u, 4u, 1u);
        RECTANGLE_OPCODE(0xb760u, 2u, 2u, 0u);
        const bool facing = cpu->ram[0x692u] == 4u;
        RECTANGLE_OPCODE(0xb762u, 2u, facing ? 2u : 3u, 0u);
        if (!facing)
            return true;
        RECTANGLE_OPCODE(0xb764u, 2u, 2u, 0u);
        RECTANGLE_OPCODE(0xb766u, 3u, 6u, 2u);
    }
    return true;
}

static unsigned FieldRectangleClockReserve(const FieldRectanglePlan *plan) {
    unsigned master = 42u;
    for (unsigned opcode = 0u; opcode < plan->count; ++opcode) {
        const FixedSpanOpcode *timing = plan->opcodes + opcode;
        const unsigned bus = timing->fetched + timing->data_bytes;
        master += 8u * bus + 6u * (timing->cycles - bus);
    }
    return master + 80u * (master / 1284u + 2u);
}

static bool FieldRectangleCallerSupported(const CpuState *cpu, uint32_t entry, uint32_t rpc) {
    if (entry == 0x83b711u)
        return rpc == 0x83827du;
    if (entry == 0x83b747u)
        return rpc == 0x83826bu;
    if (entry != 0x83b882u)
        return false;
    return (rpc == 0x83b677u && cpu->X == 2u && cpu->Y == 15u) ||
        (rpc == 0x83b682u && cpu->X == 10u && cpu->Y == 5u) ||
        (rpc == 0x83b71au && cpu->X == 12u && cpu->Y == 5u) ||
        (rpc == 0x83b752u && cpu->X == 6u && cpu->Y == 9u);
}

static bool FieldRectangleClockSupported(const CpuState *cpu, uint32_t entry, unsigned reserve) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != 0x7eu || cpu->m_flag != 1u ||
        cpu->x_flag > (entry == 0x83b747u ? 1u : 0u) ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f20u ||
        cpu->S > (entry == 0x83b882u ? 0x1ffcu : 0x1ffdu) ||
        g_interp_apu_driving || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !Lufia2DecompFramePairedCallReady(cpu, entry))
        return false;
    const uint32_t rpc = Lufia2DecompFramePairedReturnPc(cpu, entry);
    if (!FieldRectangleCallerSupported(cpu, entry, rpc) ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) !=
            (uint16_t)(rpc - 1u) ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry,
            entry == 0x83b882u ? 0x1ffcu : 0x1ffdu, false, 1363u, reserve) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x1b8beu ||
        !Lufia2DecompFrameRangeQuiet(0x83b882u, 0x83b8bfu))
        return false;
    if (entry == 0x83b711u &&
        (!Lufia2DecompFrameRangeQuiet(0x83b711u, 0x83b722u) ||
         !Lufia2DecompFrameRangeQuiet(0x83b726u, 0x83b727u)))
        return false;
    if (entry == 0x83b747u &&
        (!Lufia2DecompFrameRangeQuiet(0x83b747u, 0x83b769u) ||
         !Lufia2DecompFrameRangeQuiet(0x83b76du, 0x83b76eu)))
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedFieldRectangles(CpuState *cpu, uint32_t entry) {
    FieldRectanglePlan plan;
    if (!FieldRectangleClockSupported(cpu, entry, 0u) ||
        !PlanFieldRectangleClocks(cpu, entry, &plan) ||
        !FieldRectangleClockSupported(cpu, entry, FieldRectangleClockReserve(&plan))) {
        const uint32_t original = (uint32_t)cpu->PB << 16 | (entry & 0xffffu);
        return interp_tier_dispatch_tail(cpu, original, original, cpu->S, cpu->host_return_valid);
    }
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, entry)};
    PairedSpanClock clock = BeginPairedSpanClock(cpu, entry, plan.opcodes, plan.count);
    const Lufia2Memory memory = {PairedSpanRead, PairedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x83b711u
        ? Lufia2FieldEventRects(&memory, &state) : entry == 0x83b747u
        ? Lufia2FieldAreaRects(&memory, &state) : Lufia2FieldFindRectangleRecord(&memory, &state);
    PreparePairedSpan(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        assert(result.pc == (entry == 0x83b711u ? 0x83b722u : 0x83b769u));
        assert(cpu->master_cycles - start <= FieldRectangleClockReserve(&plan));
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (entry == 0x83b711u ? 0x83b726u :
            entry == 0x83b747u ? 0x83b76du : 0x83b8beu));
    bool progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, entry, 1u, progress);
    assert(cpu->master_cycles - start <= FieldRectangleClockReserve(&plan));
    return returned;
}
#endif
#undef RECTANGLE_OPCODE
