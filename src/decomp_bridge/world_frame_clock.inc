#include "lufia2/world_scene_returns.h"
#include "world_frame_owner.h"

typedef enum WorldClockReadKind {
    WORLD_READ_MOVEMENT,
    WORLD_READ_COLUMN,
    WORLD_READ_ROW,
    WORLD_READ_FLAGS,
    WORLD_READ_MORE_FLAGS,
    WORLD_READ_NONE
} WorldClockReadKind;

typedef struct WorldFrameClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[5];
    ActorPushedCall child;
    WorldClockReadKind read_kind;
    uint8_t flags;
    uint16_t next_call;
} WorldFrameClock;

static void WorldClockArm(WorldFrameClock *clock) {
    assert(!clock->span.remaining);
    assert(clock->span.index == clock->span.count);
    clock->span.timing = clock->timing;
    clock->span.count = 0u;
    clock->span.index = 0u;
}

static void WorldClockOpcode(WorldFrameClock *clock, uint16_t pc,
                              uint8_t fetched, uint8_t cycles, uint8_t data) {
    assert(clock->span.count < 5u);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){pc, fetched, cycles, data};
}

static void WorldClockLoad(WorldFrameClock *clock, uint16_t pc,
                            WorldClockReadKind kind) {
    WorldClockOpcode(clock, pc, 3u, 4u, 1u);
    clock->read_kind = kind;
}

static void WorldClockCall(WorldFrameClock *clock, uint16_t pc) {
    WorldClockOpcode(clock, pc, 3u, 6u, 2u);
    clock->read_kind = WORLD_READ_NONE;
    clock->next_call = pc;
}

static uint8_t WorldClockRead(void *context, uint32_t address) {
    WorldFrameClock *clock = context;
    const WorldClockReadKind kind = clock->read_kind;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    WorldClockArm(clock);
    switch (kind) {
    case WORLD_READ_MOVEMENT:
        WorldClockOpcode(clock, 0x925fu, 2u, value ? 2u : 3u, 0u);
        if (value)
            WorldClockCall(clock, 0x9261u);
        else
            WorldClockLoad(clock, 0x9264u, WORLD_READ_COLUMN);
        break;
    case WORLD_READ_COLUMN:
    case WORLD_READ_ROW: {
        const bool centered = (value & 15u) == 8u;
        const uint16_t first = kind == WORLD_READ_COLUMN ? 0x9267u : 0x9270u;
        WorldClockOpcode(clock, first, 2u, 2u, 0u);
        WorldClockOpcode(clock, (uint16_t)(first + 2u), 2u, 2u, 0u);
        WorldClockOpcode(clock, (uint16_t)(first + 4u), 2u, centered ? 2u : 3u, 0u);
        if (kind == WORLD_READ_COLUMN && centered)
            WorldClockLoad(clock, 0x926du, WORLD_READ_ROW);
        else
            WorldClockCall(clock, centered ? 0x9276u : 0x927fu);
        break;
    }
    case WORLD_READ_FLAGS:
        clock->flags = value;
        WorldClockLoad(clock, 0x928bu, WORLD_READ_MORE_FLAGS);
        break;
    case WORLD_READ_MORE_FLAGS: {
        const bool active = (clock->flags | value) != 0u;
        WorldClockOpcode(clock, 0x928eu, 2u, active ? 3u : 2u, 0u);
        WorldClockCall(clock, active ? 0x9293u : 0x9290u);
        break;
    }
    default:
        abort();
    }
    return value;
}

static void WorldClockWrite(void *context, uint32_t address, uint8_t value) {
    WorldFrameClock *clock = context;
    assert(clock->read_kind == WORLD_READ_NONE);
    FixedSpanWrite(&clock->span, address, value);
}

static void WorldClockAfterChild(WorldFrameClock *clock, uint16_t site) {
    WorldClockArm(clock);
    if (site == 0x9261u)
        WorldClockLoad(clock, 0x9264u, WORLD_READ_COLUMN);
    else if (site == 0x9285u)
        WorldClockLoad(clock, 0x9288u, WORLD_READ_FLAGS);
    else if (site == 0x929cu) {
        WorldClockOpcode(clock, 0x929fu, 2u, 3u, 0u);
        WorldClockLoad(clock, 0x925cu, WORLD_READ_MOVEMENT);
    } else
        WorldClockCall(clock, (uint16_t)(site + 3u));
}

static void WorldClockFinish(WorldFrameClock *clock, uint32_t landing) {
    if (landing == 0x86925cu && clock->span.count && clock->timing[0].pc == 0x929fu) {
        clock->span.count = 1u;
        BeginFixedSpanOpcode(&clock->span);
    }
}


/* Bound the prefix through its next JSR, including one refresh stall. */
static unsigned WorldClockPrefixBudget(uint32_t entry) {
    unsigned code_bytes = 3u, data_bytes = 2u, internal_cycles = 1u;
    switch (entry) {
    case 0x86925cu:
        code_bytes = 26u; data_bytes = 5u; internal_cycles = 3u;
        break;
    case 0x869264u:
        code_bytes = 21u; data_bytes = 4u; internal_cycles = 2u;
        break;
    case 0x869288u:
        code_bytes = 11u; data_bytes = 4u; internal_cycles = 2u;
        break;
    case 0x86929fu:
        code_bytes = 28u; data_bytes = 5u; internal_cycles = 4u;
        break;
    case 0x869276u: case 0x869279u: case 0x86927cu:
    case 0x86927fu: case 0x869282u: case 0x869285u:
    case 0x869290u: case 0x869293u: case 0x869296u:
    case 0x869299u: case 0x86929cu:
        break;
    default:
        return 0u;
    }
    return code_bytes * cpu_region_speed(entry) + data_bytes * 8u +
        internal_cycles * 6u + 40u;
}

static bool WorldClockWindow(CpuState *cpu, uint32_t entry, unsigned budget) {
    if (!g_snes || !Lufia2DecompFrameReturnCpuReady())
        return false;
    if (!budget ||
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1ffcu, budget) ||
#if defined(LUFIA2_HAVE_NATIVE_FRAME_DECISIONS)
        !Lufia2DecompFrameRangeAllows(0x86925cu, 0x8692a1u,
            Lufia2DecompFrameDecisionHookOwned))
#else
        !Lufia2DecompFrameRangeQuiet(0x86925cu, 0x8692a1u))
#endif
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return g_snes->vPos < 262u && phase < 1364u && charged == cpu->master_cycles;
}

static bool WorldClockContext(const CpuState *cpu) {
    return cpu == &g_cpu && cpu->ram && !cpu_interrupt_context_active() &&
        !cpu->_flag_D && cpu->m_flag && !cpu->x_flag &&
        !cpu->D && cpu->DB == 0x86u && cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu &&
        cpu->S == (uint16_t)(cpu->ram[0x9e8] | (uint16_t)cpu->ram[0x9e9] << 8);
}

static uint8_t WorldClockChild(void *context, Lufia2CpuState *state,
                                uint32_t target, uint32_t site, uint8_t frame) {
    WorldFrameClock *clock = context;
    assert((uint16_t)site == clock->next_call);
    assert(!clock->span.remaining && clock->span.index == clock->span.count);
    if (!ActorBridgePushedChild(&clock->child, state, target, site, frame))
        return 0u;
    const uint32_t landing = site + 3u;
    if (!WorldClockContext(clock->child.cpu) || !g_snes ||
        !WorldClockWindow(clock->child.cpu, landing, WorldClockPrefixBudget(landing))) {
        clock->child.unwound = interp_tier_dispatch_tail(clock->child.cpu,
            landing, site, clock->child.frame.entry_s, 0u);
        return 0u;
    }
    WorldClockAfterChild(clock, (uint16_t)site);
    return 1u;
}

bool Lufia2DecompWorldFrameCanResumeFrom(CpuState *cpu, uint32_t entry) {
    return Lufia2DecompWorldFrameEntry(entry) && WorldClockContext(cpu) && g_snes &&
        WorldClockWindow(cpu, entry, WorldClockPrefixBudget(entry));
}

bool Lufia2DecompWorldFrameCanResume(CpuState *cpu) {
    /* Preserve the partial header's existing handoff threshold. */
    return WorldClockContext(cpu) && g_snes && g_snes->hPos <= 800u &&
        WorldClockWindow(cpu, 0x86925cu, 384u);
}

static RecompReturn WorldFrameResumeAt(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc) {
    const ActorBridgeFrame frame = {cpu->S, 0u, UINT32_MAX};
    WorldFrameClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x860000u;
    clock.child = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    WorldClockArm(&clock);
    if (entry == 0x86925cu)
        WorldClockLoad(&clock, 0x925cu, WORLD_READ_MOVEMENT);
    else
        WorldClockCall(&clock, (uint16_t)entry);
    const Lufia2Memory memory = {WorldClockRead, WorldClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_TAIL)
    if (entry != 0x86925cu)
        result = Lufia2WorldMapContinueFrameTail(
            &memory, &state, entry, WorldClockChild, &clock);
    else
#endif
        result = Lufia2WorldMapContinueSession(
            &memory, &state, WorldClockChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        *next_pc = 0x860000u | (uint16_t)(clock.next_call + 3u);
        return clock.child.unwound;
    }
    WorldClockFinish(&clock, result.pc);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    *next_pc = result.pc;
    return RECOMP_RETURN_NORMAL;
}

RecompReturn Lufia2DecompWorldFrameResume(CpuState *cpu, uint32_t *next_pc) {
    return WorldFrameResumeAt(cpu, 0x86925cu, next_pc);
}

#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_TAIL)
RecompReturn Lufia2DecompWorldFrameResumeFrom(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc) {
    assert(Lufia2DecompWorldFrameEntry(entry));
    return WorldFrameResumeAt(cpu, entry, next_pc);
}
#endif
