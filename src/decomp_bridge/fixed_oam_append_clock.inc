enum { FIXED_OAM_APPEND_MAX_SPRITES = 32u, FIXED_OAM_APPEND_MAX_RUNS = 128u };

enum FixedOamAppendOpcodeIndex {
    OAM_APPEND_B705, OAM_APPEND_B707, OAM_APPEND_B709, OAM_APPEND_B70B,
    OAM_APPEND_B70D, OAM_APPEND_B70F, OAM_APPEND_B711, OAM_APPEND_B713,
    OAM_APPEND_B715, OAM_APPEND_B717, OAM_APPEND_B71A, OAM_APPEND_B71D,
    OAM_APPEND_B720, OAM_APPEND_B723, OAM_APPEND_B726, OAM_APPEND_B729,
    OAM_APPEND_B72C, OAM_APPEND_B72F, OAM_APPEND_B732, OAM_APPEND_B735,
    OAM_APPEND_B738, OAM_APPEND_B73B, OAM_APPEND_B73E, OAM_APPEND_B741,
    OAM_APPEND_B744, OAM_APPEND_B747, OAM_APPEND_B748, OAM_APPEND_B749,
    OAM_APPEND_B74C, OAM_APPEND_B74D, OAM_APPEND_B74E, OAM_APPEND_B74F,
    OAM_APPEND_B750, OAM_APPEND_B751, OAM_APPEND_B752, OAM_APPEND_B753,
    OAM_APPEND_B754, OAM_APPEND_B757, OAM_APPEND_B758, OAM_APPEND_B75A,
    OAM_APPEND_B75D, OAM_APPEND_B75E, OAM_APPEND_B75F, OAM_APPEND_B762,
    OAM_APPEND_B763, OAM_APPEND_B764, OAM_APPEND_B767, OAM_APPEND_B768,
    OAM_APPEND_B769, OAM_APPEND_B76C, OAM_APPEND_B76F, OAM_APPEND_B770,
    OAM_APPEND_B771, OAM_APPEND_B773, OAM_APPEND_B774, OAM_APPEND_B776,
    OAM_APPEND_B778, OAM_APPEND_B77A, OAM_APPEND_B77B, OAM_APPEND_B77D,
    OAM_APPEND_B780, OAM_APPEND_B782, OAM_APPEND_B785, OAM_APPEND_B788,
    OAM_APPEND_B78B, OAM_APPEND_B78E, OAM_APPEND_B78F, OAM_APPEND_B790,
    OAM_APPEND_B791, OAM_APPEND_B792, OAM_APPEND_B793, OAM_APPEND_B794,
    OAM_APPEND_B795, OAM_APPEND_B796, OAM_APPEND_B797, OAM_APPEND_B79A,
    OAM_APPEND_B79B, OAM_APPEND_B79D, OAM_APPEND_B7A0, OAM_APPEND_B7A2,
    OAM_APPEND_B7A4, OAM_APPEND_B7A6, OAM_APPEND_B7A8, OAM_APPEND_B7AA,
    OAM_APPEND_B7AC, OAM_APPEND_B7AE, OAM_APPEND_B7B0, OAM_APPEND_B7B2,
    OAM_APPEND_B7B3, OAM_APPEND_B7B4, OAM_APPEND_B7B6, OAM_APPEND_B7B8,
    OAM_APPEND_B7BA, OAM_APPEND_B7BD, OAM_APPEND_B7BF, OAM_APPEND_B7C2,
    OAM_APPEND_B7C3, OAM_APPEND_B7C4, OAM_APPEND_B7C6, OAM_APPEND_B7C7,
    OAM_APPEND_B7C8, OAM_APPEND_B7CB, OAM_APPEND_B7CC, OAM_APPEND_B7CE,
    OAM_APPEND_B7D0, OAM_APPEND_B7D2, OAM_APPEND_B7D3, OAM_APPEND_B7D5,
};

static const FixedSpanOpcode fixed_oam_append_timing[] = {
    {0xb705u, 2u, 3u, 1u}, {0xb707u, 2u, 2u, 0u},
    {0xb709u, 2u, 2u, 0u}, {0xb70bu, 2u, 2u, 0u},
    {0xb70du, 2u, 2u, 0u}, {0xb70fu, 2u, 3u, 1u},
    {0xb711u, 2u, 2u, 0u}, {0xb713u, 2u, 2u, 0u},
    {0xb715u, 2u, 3u, 0u}, {0xb717u, 3u, 6u, 2u},
    {0xb71au, 3u, 7u, 2u}, {0xb71du, 3u, 6u, 2u},
    {0xb720u, 3u, 7u, 2u}, {0xb723u, 3u, 6u, 2u},
    {0xb726u, 3u, 7u, 2u}, {0xb729u, 3u, 6u, 2u},
    {0xb72cu, 3u, 7u, 2u}, {0xb72fu, 3u, 6u, 2u},
    {0xb732u, 3u, 7u, 2u}, {0xb735u, 3u, 6u, 2u},
    {0xb738u, 3u, 7u, 2u}, {0xb73bu, 3u, 6u, 2u},
    {0xb73eu, 3u, 7u, 2u}, {0xb741u, 3u, 6u, 2u},
    {0xb744u, 3u, 7u, 2u}, {0xb747u, 1u, 2u, 0u},
    {0xb748u, 1u, 2u, 0u}, {0xb749u, 3u, 3u, 0u},
    {0xb74cu, 1u, 4u, 2u}, {0xb74du, 1u, 2u, 0u},
    {0xb74eu, 1u, 2u, 0u}, {0xb74fu, 1u, 2u, 0u},
    {0xb750u, 1u, 2u, 0u}, {0xb751u, 1u, 2u, 0u},
    {0xb752u, 1u, 2u, 0u}, {0xb753u, 1u, 2u, 0u},
    {0xb754u, 3u, 3u, 0u}, {0xb757u, 1u, 4u, 2u},
    {0xb758u, 2u, 3u, 0u}, {0xb75au, 3u, 5u, 1u},
    {0xb75du, 1u, 2u, 0u}, {0xb75eu, 1u, 2u, 0u},
    {0xb75fu, 3u, 5u, 1u}, {0xb762u, 1u, 2u, 0u},
    {0xb763u, 1u, 2u, 0u}, {0xb764u, 3u, 5u, 1u},
    {0xb767u, 1u, 2u, 0u}, {0xb768u, 1u, 2u, 0u},
    {0xb769u, 3u, 5u, 1u}, {0xb76cu, 3u, 6u, 1u},
    {0xb76fu, 1u, 5u, 2u}, {0xb770u, 1u, 5u, 2u},
    {0xb771u, 2u, 3u, 1u}, {0xb773u, 1u, 2u, 0u},
    {0xb774u, 2u, 2u, 0u}, {0xb776u, 2u, 3u, 1u},
    {0xb778u, 2u, 3u, 1u}, {0xb77au, 1u, 2u, 0u},
    {0xb77bu, 2u, 2u, 0u}, {0xb77du, 2u, 2u, 0u},
    {0xb780u, 2u, 3u, 0u}, {0xb782u, 3u, 6u, 2u},
    {0xb785u, 3u, 7u, 2u}, {0xb788u, 3u, 6u, 2u},
    {0xb78bu, 3u, 7u, 2u}, {0xb78eu, 1u, 2u, 0u},
    {0xb78fu, 1u, 4u, 2u}, {0xb790u, 1u, 2u, 0u},
    {0xb791u, 1u, 2u, 0u}, {0xb792u, 1u, 2u, 0u},
    {0xb793u, 1u, 2u, 0u}, {0xb794u, 1u, 2u, 0u},
    {0xb795u, 1u, 2u, 0u}, {0xb796u, 1u, 2u, 0u},
    {0xb797u, 3u, 3u, 0u}, {0xb79au, 1u, 4u, 2u},
    {0xb79bu, 2u, 3u, 0u}, {0xb79du, 3u, 5u, 1u},
    {0xb7a0u, 2u, 2u, 0u}, {0xb7a2u, 2u, 3u, 1u},
    {0xb7a4u, 2u, 3u, 1u}, {0xb7a6u, 2u, 2u, 0u},
    {0xb7a8u, 2u, 3u, 1u}, {0xb7aau, 2u, 2u, 0u},
    {0xb7acu, 2u, 3u, 0u}, {0xb7aeu, 2u, 5u, 2u},
    {0xb7b0u, 2u, 5u, 2u}, {0xb7b2u, 1u, 2u, 0u},
    {0xb7b3u, 1u, 2u, 0u}, {0xb7b4u, 2u, 2u, 0u},
    {0xb7b6u, 2u, 5u, 2u}, {0xb7b8u, 2u, 2u, 0u},
    {0xb7bau, 3u, 4u, 1u}, {0xb7bdu, 2u, 3u, 1u},
    {0xb7bfu, 3u, 6u, 1u}, {0xb7c2u, 1u, 5u, 2u},
    {0xb7c3u, 1u, 5u, 2u}, {0xb7c4u, 2u, 3u, 0u},
    {0xb7c6u, 1u, 2u, 0u}, {0xb7c7u, 1u, 2u, 0u},
    {0xb7c8u, 3u, 3u, 0u}, {0xb7cbu, 1u, 2u, 0u},
    {0xb7ccu, 2u, 3u, 0u}, {0xb7ceu, 2u, 5u, 2u},
    {0xb7d0u, 2u, 3u, 1u}, {0xb7d2u, 1u, 2u, 0u},
    {0xb7d3u, 2u, 2u, 0u}, {0xb7d5u, 3u, 3u, 0u},
};

typedef struct FixedOamAppendRun {
    uint8_t first, count, extra_cycles;
} FixedOamAppendRun;

typedef struct FixedOamAppendPlan {
    FixedOamAppendRun runs[FIXED_OAM_APPEND_MAX_RUNS];
    unsigned run_count;
    unsigned opcode_count;
    uint32_t landing;
} FixedOamAppendPlan;

static bool AppendFixedOamRun(FixedOamAppendPlan *plan, unsigned first,
    unsigned last, unsigned extra_cycles) {
    if (plan->run_count == FIXED_OAM_APPEND_MAX_RUNS)
        return false;
    const unsigned count = last - first + 1u;
    assert(first <= last && last < sizeof fixed_oam_append_timing / sizeof *fixed_oam_append_timing);
    plan->runs[plan->run_count++] = (FixedOamAppendRun){first, count, extra_cycles};
    plan->opcode_count += count;
    return true;
}

static bool PlanFixedOamAppend(unsigned count, unsigned used,
    FixedOamAppendPlan *plan) {
    *plan = (FixedOamAppendPlan){0};
    if (count > FIXED_OAM_APPEND_MAX_SPRITES || (count && used + count > 128u))
        return false;
#define OAM_RUN(first, last) \
    do { if (!AppendFixedOamRun(plan, OAM_APPEND_##first, OAM_APPEND_##last, 0u)) return false; } while (0)
#define OAM_BRANCH(pc, taken) \
    do { if (!AppendFixedOamRun(plan, OAM_APPEND_##pc, OAM_APPEND_##pc, (taken) ? 1u : 0u)) return false; } while (0)
    for (;;) {
        if (!count) {
            OAM_RUN(B705, B707);
            OAM_BRANCH(B709, true);
            plan->landing = 0x81b77fu;
            return true;
        }
        const unsigned slot = used & 3u;
        if (count < 4u) {
            OAM_RUN(B705, B70B);
            OAM_BRANCH(B70D, true);
        } else {
            OAM_RUN(B705, B711);
            OAM_BRANCH(B713, slot != 0u);
        }
        if (count >= 4u && slot == 0u) {
            OAM_RUN(B715, B77B);
            count -= 4u;
            used += 4u;
            OAM_BRANCH(B77D, count != 0u);
            if (!count) {
                plan->landing = 0x81b77fu;
                return true;
            }
        } else {
            OAM_RUN(B780, B7AC);
            for (unsigned shifted = 0u; shifted < slot; ++shifted) {
                OAM_RUN(B7B6, B7B6);
                OAM_BRANCH(B7B8, true);
                OAM_RUN(B7AE, B7B4);
            }
            OAM_RUN(B7B6, B7B6);
            OAM_BRANCH(B7B8, false);
            OAM_RUN(B7BA, B7D2);
            --count;
            ++used;
            OAM_BRANCH(B7D3, count == 0u);
            if (!count) {
                plan->landing = 0x81b7d8u;
                return true;
            }
            OAM_RUN(B7D5, B7D5);
        }
    }
#undef OAM_RUN
#undef OAM_BRANCH
}

_Static_assert(LUFIA2_BATTLE_FIXED_OAM_APPEND_OPCODE_BUDGET >= 719u + 2u,
    "reserve the original JSR and RTS");

typedef struct FixedOamAppendClock {
    FixedSpanClock span;
    const FixedOamAppendPlan *plan;
    FixedSpanOpcode timing[32];
    unsigned next_run, run_offset, completed;
} FixedOamAppendClock;

static void PrepareFixedOamAppendClock(FixedOamAppendClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count)
            return;
        if (clock->span.count)
            Lufia2DecompFramePairedAccountSpan(clock->span.cpu, 0x81b705u,
                clock->span.count, clock->span.progress_mask);
        clock->completed += clock->span.count;
        unsigned count = 0u;
        while (count < 32u && clock->next_run < clock->plan->run_count) {
            const FixedOamAppendRun *run = &clock->plan->runs[clock->next_run];
            FixedSpanOpcode opcode = fixed_oam_append_timing[run->first + clock->run_offset];
            opcode.cycles += run->extra_cycles;
            clock->timing[count++] = opcode;
            if (++clock->run_offset == run->count) {
                clock->run_offset = 0u;
                ++clock->next_run;
            }
        }
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x810000u,
            .timing = clock->timing, .count = count};
        if (!count)
            return;
    }
}

static uint8_t FixedOamAppendRead(void *context, uint32_t address) {
    FixedOamAppendClock *clock = context;
    PrepareFixedOamAppendClock(clock);
    return FixedSpanRead(&clock->span, address);
}

static void FixedOamAppendWrite(void *context, uint32_t address, uint8_t value) {
    FixedOamAppendClock *clock = context;
    PrepareFixedOamAppendClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool FixedOamAppendCallerSupported(uint32_t return_pc) {
    static const uint32_t return_pcs[] = {
        0x81b5e6u, 0x81b5f9u, 0x81b60cu, 0x81b61fu, 0x81b632u,
        0x81b645u, 0x81b659u, 0x81b66cu, 0x81b67fu
    };
    for (unsigned i = 0u; i < sizeof return_pcs / sizeof *return_pcs; ++i)
        if (return_pcs[i] == return_pc)
            return true;
    return false;
}

static RecompReturn ActorBridgeTimedFixedOamAppend(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, 0x81b705u)};
    if (rtl_apu_extended_frame_timing() || cpu != &g_cpu || !cpu->ram ||
        cpu->emulation || cpu->m_flag != 1u || cpu->x_flag || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x81u || cpu->DB != 0x7eu ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu || cpu->host_return_valid != 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x81b705u) ||
        !FixedOamAppendCallerSupported(frame.host_return_pc24) ||
        (cpu->ram[cpu->S + 1u] | (unsigned)cpu->ram[cpu->S + 2u] << 8) !=
            (uint16_t)(frame.host_return_pc24 - 1u) ||
        !g_snes || !g_snes->apu || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0xb7d8u ||
        (g_snes->vPos < 225u && !g_snes->hdmaBeamOff) || g_snes->dma->hdmaPendingInit)
        return ActorBridgeFallback(cpu, &frame, 0x81b705u);
    const unsigned count = cpu->A & 0xffu;
    const unsigned used = cpu->ram[0x58u];
    FixedOamAppendPlan plan;
    if ((count && (cpu->Y != used * 4u || cpu->X < 0x4000u ||
            (unsigned)cpu->X + count * 5u > 0x10000u)) ||
        !PlanFixedOamAppend(count, used, &plan) ||
        plan.opcode_count + 2u > LUFIA2_BATTLE_FIXED_OAM_APPEND_OPCODE_BUDGET ||
        !BattleClockAllows(cpu, 0x81b705u, 0x81b7d9u, 2u, false,
            BattleLeafClockRoom(0x81b705u, BattleOamCost(count, used & 3u)), false, NULL))
        return ActorBridgeFallback(cpu, &frame, 0x81b705u);
    FixedOamAppendClock clock = {.span = {.cpu = cpu}, .plan = &plan};
    const Lufia2Memory memory = {FixedOamAppendRead, FixedOamAppendWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleAppendOamSprites(&memory, &state);
    PrepareFixedOamAppendClock(&clock);
    assert(clock.completed == plan.opcode_count && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    bool progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(
        cpu, &frame, result.pc, &progress);
    Lufia2DecompFramePairedAccountSpan(cpu, 0x81b705u, 1u, progress);
    return returned;
}
