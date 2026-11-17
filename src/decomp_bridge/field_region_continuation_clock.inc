#include "lufia2/field_region_continuations.h"
#include "field_region_owner.h"

enum { FIELD_REGION_CONTINUATION_RESERVE = 512u };

typedef struct FieldRegionContinuationClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[10];
    ActorPushedCall child;
    uint32_t next_call;
    uint8_t frame_size;
} FieldRegionContinuationClock;

static bool FieldRegionContinuationContext(const CpuState *cpu, uint32_t entry) {
    const bool region_bank = entry >= 0x838265u && entry <= 0x83827du;
    return cpu == &g_cpu && cpu->ram && !cpu_interrupt_context_active() &&
        !cpu->emulation && !cpu->_flag_D && cpu->m_flag && !cpu->x_flag &&
        !cpu->D && cpu->PB == 0x83u && cpu->DB == (region_bank ? 0x7eu : 0x83u) &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu &&
        (!region_bank || cpu->ram[cpu->S + 1u] == 0x83u);
}

bool Lufia2DecompFieldRegionCanResume(CpuState *cpu, uint32_t entry) {
    if ((!Lufia2DecompFieldRegionEntry(entry) && entry != 0x83829eu) ||
        !FieldRegionContinuationContext(cpu, entry) || !g_snes ||
        !g_snes->apu || (!rtl_apu_frame_timeline_active() &&
            (!rtl_apu_extended_frame_timing() || !g_snes->apu->portTimeValid)) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1ffcu,
            FIELD_REGION_CONTINUATION_RESERVE) ||
        !Lufia2DecompFrameRangeAllows(0x838265u, 0x83829eu, FieldRegionHookOwned))
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    return g_snes->vPos < 262u && phase < 1364u && charged == cpu->master_cycles;
}

static void PrepareFieldRegionContinuation(
    FieldRegionContinuationClock *clock, uint32_t entry) {
    CpuState *cpu = clock->child.cpu;
    unsigned count = 0u;
    uint32_t call = entry;
    if (entry == 0x83826bu) {
        count = PlanFieldRegionClocks(cpu, entry, clock->timing);
        call = (cpu->ram[0x0622u] & 1u) || (cpu->ram[0x1d0a1u] & 4u)
            ? 0x83827du : 0x83827au;
    } else if (entry == 0x838285u) {
        count = PlanFieldRegionClocks(cpu, entry, clock->timing);
        const uint8_t scene = (uint8_t)cpu->A;
        call = scene == 0x0bu ? 0x838289u : scene == 0x0cu ? 0x838292u
            : scene == 0x0du ? 0x83829bu : 0x83829eu;
    }
    if (call == 0x83827du) {
        clock->timing[count++] = (FixedSpanOpcode){0x827du, 1u, 4u, 1u};
        call = 0x83827eu;
    }
    clock->frame_size = call == 0x838281u ? 3u : 2u;
    clock->next_call = call;
    if (call != 0x83829eu)
        clock->timing[count++] = (FixedSpanOpcode){(uint16_t)call,
            clock->frame_size + 1u, clock->frame_size == 3u ? 8u : 6u,
            clock->frame_size};
    assert(count <= sizeof clock->timing / sizeof clock->timing[0]);
    clock->span = (FixedSpanClock){.cpu = cpu, .bank = 0x830000u,
        .timing = clock->timing, .count = count};
}

static uint8_t FieldRegionContinuationChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    FieldRegionContinuationClock *clock = context;
    assert(site == clock->next_call && frame_size == clock->frame_size);
    assert(!clock->span.remaining && clock->span.index == clock->span.count);
    Lufia2DecompFieldRegionAccountPrefix(clock->child.cpu,
        clock->span.count, clock->span.progress_mask);
    if (!Lufia2DecompFieldRegionInvokeChild(clock->child.cpu, &clock->child,
            state, target, site, frame_size, ActorBridgePushedChild))
        return 0u;
    const uint32_t landing = site + frame_size + 1u;
    if (!Lufia2DecompFieldRegionHasBudget(clock->child.cpu, 10u) ||
        !Lufia2DecompFieldRegionCanResume(clock->child.cpu, landing)) {
        clock->child.unwound = interp_tier_dispatch_tail(clock->child.cpu,
            landing, site, clock->child.frame.entry_s, 0u);
        return 0u;
    }
    PrepareFieldRegionContinuation(clock, landing);
    return 1u;
}

RecompReturn Lufia2DecompFieldRegionResume(
    CpuState *cpu, uint32_t entry, uint32_t *next_pc) {
    assert(Lufia2DecompFieldRegionEntry(entry));
    FieldRegionContinuationClock clock = {0};
    const ActorBridgeFrame frame = {cpu->S, 0u, UINT32_MAX};
    clock.child = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    PrepareFieldRegionContinuation(&clock, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldContinueRegionProbe(
        &memory, &state, entry, FieldRegionContinuationChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND) {
        *next_pc = clock.next_call + clock.frame_size + 1u;
        return clock.child.unwound;
    }
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x83829eu);
    assert(!clock.span.remaining && clock.span.index == clock.span.count);
    Lufia2DecompFieldRegionAccountPrefix(cpu, clock.span.count, clock.span.progress_mask);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    *next_pc = result.pc;
    return RECOMP_RETURN_NORMAL;
}
