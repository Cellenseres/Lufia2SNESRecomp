#include "lufia2/battle_upkeep_steps.h"
#include "battle_upkeep_owner.h"

typedef struct BattleUpkeepClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[3];
    ActorPushedCall child;
    uint32_t next_call;
} BattleUpkeepClock;

static unsigned BattleUpkeepReserve(uint32_t entry) {
    switch (entry) {
    case 0x85ec81u: case 0x85ec8bu:
        return 4u * cpu_region_speed(entry) + 70u;
    case 0x85ec85u:
        return 10u * cpu_region_speed(entry) + 78u;
    case 0x85ec8fu:
        return 5u * cpu_region_speed(entry) + 54u;
    default:
        return 0u;
    }
}

bool Lufia2DecompBattleUpkeepCanResume(CpuState *cpu, uint32_t entry) {
    const unsigned reserve = BattleUpkeepReserve(entry);
    if (!reserve || cpu != &g_cpu || !cpu->ram || !g_snes || !g_snes->cart ||
        !g_snes->apu || g_snes->cart->type != CART_LOROM ||
        !ActorBridgeBattleSupported(cpu, 0x85ec81u, BATTLE_BRIDGE_M1X0,
            0, BATTLE_BRIDGE_WRAM_BANK, 0) ||
        cpu_interrupt_context_active() || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !interp_bridge_use_absolute_apu_timeline(rtl_apu_frame_timeline_active(),
            cart_has_sa1(g_snes->cart), rtl_apu_extended_frame_timing() &&
            g_snes->apu->portTimeValid) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1ffcu, reserve) ||
#if defined(LUFIA2_HAVE_NATIVE_FRAME_DECISIONS)
        !Lufia2DecompFrameRangeAllows(0x85ec81u, 0x85ec94u,
            Lufia2DecompFrameDecisionHookOwned))
#else
        !Lufia2DecompFrameRangeQuiet(0x85ec81u, 0x85ec94u))
#endif
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return g_snes->vPos < 262u && phase < 1364u && charged == cpu->master_cycles;
}

static void PrepareBattleUpkeep(BattleUpkeepClock *clock, uint32_t entry) {
    unsigned count = 0u;
    uint32_t call = entry;
    if (entry == 0x85ec85u) {
        const bool rebuild = clock->child.cpu->ram[0x12f3u] != 0u;
        clock->timing[count++] = (FixedSpanOpcode){0xec85u, 4u, 5u, 1u};
        clock->timing[count++] = (FixedSpanOpcode){0xec89u, 2u, rebuild ? 2u : 3u, 0u};
        call = rebuild ? 0x85ec8bu : 0x85ec94u;
    } else if (entry == 0x85ec8fu) {
        clock->timing[count++] = (FixedSpanOpcode){0xec8fu, 1u, 2u, 0u};
        clock->timing[count++] = (FixedSpanOpcode){0xec90u, 4u, 5u, 1u};
        call = 0x85ec94u;
    }
    if (call != 0x85ec94u)
        clock->timing[count++] = (FixedSpanOpcode){(uint16_t)call, 4u, 8u, 3u};
    clock->next_call = call;
    clock->span = (FixedSpanClock){.cpu = clock->child.cpu, .bank = 0x850000u,
        .timing = clock->timing, .count = count};
}

static uint8_t BattleUpkeepChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    BattleUpkeepClock *clock = context;
    assert(site == clock->next_call && frame_size == 3u);
    assert(!clock->span.remaining && clock->span.index == clock->span.count);
    Lufia2DecompCallOwnerAccountPrefix(clock->child.cpu,
        clock->span.count, clock->span.progress_mask);
    if (!Lufia2DecompCallOwnerInvokeChild(clock->child.cpu, &clock->child,
            state, target, site, frame_size, ActorBridgePushedChild))
        return 0u;
    const uint32_t landing = site + 4u;
    if (!Lufia2DecompCallOwnerHasBudget(clock->child.cpu, 4u) ||
        !Lufia2DecompBattleUpkeepCanResume(clock->child.cpu, landing)) {
        clock->child.unwound = interp_tier_dispatch_tail(clock->child.cpu,
            landing, site, clock->child.frame.entry_s, 0u);
        return 0u;
    }
    PrepareBattleUpkeep(clock, landing);
    return 1u;
}

RecompReturn Lufia2DecompBattleUpkeepResume(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc) {
    assert(Lufia2DecompBattleUpkeepEntry(entry));
    BattleUpkeepClock clock = {0};
    const ActorBridgeFrame frame = {cpu->S, 0u, UINT32_MAX};
    clock.child = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    PrepareBattleUpkeep(&clock, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2BattleContinueFrameUpkeep(
        &memory, &state, entry, BattleUpkeepChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        *next_pc = clock.next_call + 4u;
        return clock.child.unwound;
    }
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x85ec94u);
    assert(!clock.span.remaining && clock.span.index == clock.span.count);
    Lufia2DecompCallOwnerAccountPrefix(cpu, clock.span.count, clock.span.progress_mask);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    *next_pc = result.pc;
    return RECOMP_RETURN_NORMAL;
}
