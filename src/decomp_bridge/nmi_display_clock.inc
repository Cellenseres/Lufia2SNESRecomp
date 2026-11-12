#include "lufia2/nmi_finish.h"

enum { NMI_DISPLAY_MAX_CLOCKS = 632u, NMI_DISPLAY_MAX_OPCODES = 22u };

typedef struct NmiDisplayPlan {
    FixedSpanOpcode timing[NMI_DISPLAY_MAX_OPCODES];
    unsigned count;
    unsigned budget;
} NmiDisplayPlan;

static void NmiDisplaySchedule(const CpuState *cpu, NmiDisplayPlan *plan) {
    static const FixedSpanOpcode rollover[] = {
        {0x8664u, 3u, 4u, 1u}, {0x8667u, 3u, 4u, 1u},
        {0x866au, 3u, 4u, 1u}, {0x866du, 2u, 2u, 0u},
        {0x866fu, 1u, 2u, 0u}, {0x8670u, 3u, 4u, 1u},
        {0x8673u, 2u, 2u, 0u}, {0x8675u, 2u, 2u, 0u},
        {0x8677u, 3u, 4u, 1u}, {0x867au, 3u, 4u, 1u},
        {0x867du, 1u, 2u, 0u}, {0x867eu, 3u, 4u, 1u},
        {0x8681u, 2u, 2u, 0u}, {0x8683u, 2u, 2u, 0u},
        {0x8685u, 3u, 4u, 1u}, {0x8688u, 3u, 4u, 1u},
        {0x868bu, 1u, 2u, 0u}, {0x868cu, 3u, 4u, 1u},
        {0x868fu, 2u, 2u, 0u}, {0x8691u, 2u, 2u, 0u},
        {0x8693u, 3u, 4u, 1u}, {0x8696u, 3u, 6u, 2u}
    };
    unsigned count = NMI_DISPLAY_MAX_OPCODES;
    if (cpu->ram[0xb50u] & 0x80u) {
        count = 4u;
    } else {
        for (unsigned field = 0u; field < 3u; ++field) {
            if ((uint8_t)(cpu->ram[0xb50u - field] + 1u) < 60u) {
                count = 8u + field * 6u;
                break;
            }
        }
    }
    memcpy(plan->timing, rollover, count * sizeof rollover[0]);
    plan->count = count;
    if (count != NMI_DISPLAY_MAX_OPCODES)
        ++plan->timing[count - 1u].cycles;
    unsigned budget = 80u;
    const unsigned fetch_speed = cpu_region_speed(0x808664u);
    for (unsigned i = 0u; i < count; ++i) {
        const FixedSpanOpcode *step = plan->timing + i;
        budget += step->fetched * fetch_speed + step->data_bytes * 8u +
            (step->cycles - step->fetched - step->data_bytes) * 6u;
    }
    plan->budget = budget - 2u;
}

static bool NmiDisplayHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x808664u && hook == ActorBridgeNmiResume;
}

static bool NmiDisplaySupported(const CpuState *cpu, NmiDisplayPlan *plan) {
    if (!g_snes || !cpu->ram || cpu->ram != g_snes->ram || cpu->m_flag != 1u)
        return false;
    NmiDisplaySchedule(cpu, plan);
    return NmiClockWindow(cpu, 0x808664u, 0x808699u, plan->budget,
        NMI_DISPLAY_MAX_CLOCKS, NmiDisplayHookOwned);
}

static bool RunNativeNmiDisplay(CpuState *cpu) {
    NmiDisplayPlan plan;
    if (!NmiDisplaySupported(cpu, &plan))
        return false;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = 0x800000u;
    clock.timing = plan.timing;
    clock.count = plan.count;
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiDisplayAndClock(&memory, &state);
    NmiClockBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x808699u);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
    return true;
}
