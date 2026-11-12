#include "lufia2/nmi_calls.h"

typedef struct NmiCallbackPlan {
    const FixedSpanOpcode *timing;
    unsigned count;
    uint32_t next;
    unsigned budget;
} NmiCallbackPlan;

static bool NmiCallbackHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeNmiResume &&
        (pc == 0x80864cu || pc == 0x808653u || pc == 0x80865au);
}

static NmiCallbackPlan NmiCallbacksPlan(const CpuState *cpu, uint32_t entry) {
    static const FixedSpanOpcode primary_present[] = {
        {0x864cu, 2u, 3u, 1u}, {0x864eu, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode secondary_present[] = {
        {0x864cu, 2u, 3u, 1u}, {0x864eu, 2u, 3u, 0u},
        {0x8653u, 2u, 3u, 1u}, {0x8655u, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode scroll_present[] = {
        {0x864cu, 2u, 3u, 1u}, {0x864eu, 2u, 3u, 0u},
        {0x8653u, 2u, 3u, 1u}, {0x8655u, 2u, 3u, 0u},
        {0x865au, 2u, 3u, 1u}, {0x865cu, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode callbacks_absent[] = {
        {0x864cu, 2u, 3u, 1u}, {0x864eu, 2u, 3u, 0u},
        {0x8653u, 2u, 3u, 1u}, {0x8655u, 2u, 3u, 0u},
        {0x865au, 2u, 3u, 1u}, {0x865cu, 2u, 3u, 0u}
    };
    const unsigned offset = entry == 0x80864cu ? 0u : entry == 0x808653u ? 2u : 4u;
    NmiCallbackPlan plan;
    if (entry == 0x80864cu && cpu->ram[0x6au]) {
        plan = (NmiCallbackPlan){primary_present, 2u, 0x808650u, 0u};
    } else if (entry != 0x80865au && cpu->ram[0x6fu]) {
        plan = (NmiCallbackPlan){secondary_present + offset, 4u - offset, 0x808657u, 0u};
    } else {
        const bool scroll = cpu->ram[0x71u] != 0u;
        plan = (NmiCallbackPlan){(scroll ? scroll_present : callbacks_absent) + offset,
            6u - offset, scroll ? 0x80865eu : 0x808661u, 0u};
    }
    const unsigned reads = plan.count / 2u;
    const unsigned taken = reads - (plan.next != 0x808661u);
    plan.budget = plan.count * 2u * cpu_region_speed(entry) + reads * 8u + taken * 6u + 80u;
    return plan;
}

static bool NmiCallbacksSupported(const CpuState *cpu, uint32_t entry,
    NmiCallbackPlan *plan) {
    if ((entry != 0x80864cu && entry != 0x808653u && entry != 0x80865au) ||
        (entry == 0x80864cu && cpu->x_flag != 1u) || cpu->m_flag != 1u ||
        !g_snes || !cpu->ram || cpu->ram != g_snes->ram)
        return false;
    *plan = NmiCallbacksPlan(cpu, entry);
    const unsigned reserve = entry == 0x80864cu ? 218u : 172u;
    return NmiClockWindow(cpu, cpu->x_flag != 0u, entry, plan->next,
        plan->budget, reserve, NmiCallbackHookOwned);
}

static bool RunNativeNmiCallbacks(CpuState *cpu, uint32_t entry) {
    NmiCallbackPlan plan;
    if (!NmiCallbacksSupported(cpu, entry, &plan))
        return false;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x800000u;
    clock.timing = plan.timing;
    clock.count = plan.count;
    const Lufia2Memory memory = {NmiClockRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2MainNmiResume(&memory, &state, entry, NULL, NULL);
    NmiClockBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == plan.next);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
    return true;
}
