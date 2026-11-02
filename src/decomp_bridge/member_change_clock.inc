#include "snes/cart.h"
#include "member_change_plans.inc"

static uint16_t MemberChangeWord(const CpuState *cpu, unsigned offset) {
    if (cpu->DB == 0x7fu)
        offset += 0x10000u;
    return (uint16_t)(cpu->ram[offset] | (uint16_t)cpu->ram[offset + 1u] << 8);
}

static const MemberChangePlan *MemberChangePath(
    const CpuState *cpu, unsigned operation, unsigned record) {
    const uint16_t status = MemberChangeWord(cpu, record + 0x0fu) & 0xffu;
    if (operation < 4u) {
        const uint8_t mask = (uint8_t)(operation < 2u ? cpu->X : cpu->A);
        if (operation == 0u)
            return (status | mask) & 4u ? &MemberPlan0_disabled : &MemberPlan0_enabled;
        if (operation == 2u)
            return (status | mask) & 4u ? &MemberPlan2_disabled : &MemberPlan2_enabled;
        const bool disabled = (status & (uint8_t)~mask & 4u) != 0;
        const bool alive = MemberChangeWord(cpu, record + 0x11u) != 0;
        if (operation == 1u)
            return disabled ? &MemberPlan1_disabled : alive ? &MemberPlan1_alive : &MemberPlan1_revive;
        return disabled ? &MemberPlan3_disabled : alive ? &MemberPlan3_alive : &MemberPlan3_revive;
    }
    const unsigned current = operation < 6u ? 0x11u : 0x13u;
    const uint16_t value = MemberChangeWord(cpu, record + current);
    const uint16_t amount = (uint8_t)cpu->Y;
    if (operation == 4u || operation == 6u) {
        const uint16_t sum = (uint16_t)(amount + value);
        const bool floor = sum < MemberChangeWord(cpu, record + current + 0x14u);
        if (operation == 4u)
            return floor ? &MemberPlan4_floor : &MemberPlan4_sum;
        return floor ? &MemberPlan6_floor : &MemberPlan6_sum;
    }
    if (operation == 5u)
        return amount >= value ? &MemberPlan5_zero : &MemberPlan5_borrow;
    return amount >= value ? &MemberPlan7_zero : &MemberPlan7_borrow;
}

static const MemberChangePlan *PrepareMemberChange(
    CpuState *cpu, uint32_t entry, unsigned operation, FixedSpanClock *clock) {
    enum { CLOCK_ROOM = 2048u };
    if (!cpu->m_flag || cpu->x_flag || cpu->S < 0x1f20u ||
        (operation >= 4u && cpu->_flag_D) ||
        (cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - CLOCK_ROOM ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, false, 1363u, CLOCK_ROOM) ||
        !(cpu->DB == 0x7eu || cpu->DB == 0x7fu || (cpu->DB & 0x7fu) < 0x40u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x29ecau ||
        !Lufia2DecompFrameRangeQuiet(0x81f639u, 0x81f723u) ||
        !Lufia2DecompFrameRangeQuiet(0x81f7bdu, 0x81f7cau))
        return NULL;
    if (g_snes->vPos < 225u) {
        if (!g_snes->hdmaBeamOff ||
            (225u - g_snes->vPos) * 1364u - g_snes->hPos < CLOCK_ROOM)
            return NULL;
    } else if (g_snes->vPos > 257u) {
        return NULL;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return NULL;
    unsigned record = cpu->X;
    if (operation != 2u && operation != 3u) {
        const unsigned member = cpu->A & 0xffu;
        if (member >= 8u)
            return NULL;
        const uint8_t *table = g_snes->cart->rom + 0x29ebau + member * 2u;
        record = table[0] | (unsigned)table[1] << 8;
    }
    if (record < 0x100u || record > 0x1e00u)
        return NULL;
    const MemberChangePlan *plan = MemberChangePath(cpu, operation, record);
    *clock = (FixedSpanClock){0};
    clock->cpu = cpu;
    clock->bank = 0x810000u;
    clock->timing = plan->timing;
    clock->count = plan->count;
    return plan;
}

static RecompReturn ActorBridgeMemberChange(CpuState *cpu, uint32_t entry,
    unsigned operation, ActorWholeFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    FixedSpanClock clock;
    const MemberChangePlan *plan = PrepareMemberChange(cpu, entry, operation, &clock);
    if (!plan)
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (0x810000u | plan->returned));
    ActorBridgeStore(cpu, &state);
    if (plan->frame_size == 2u)
        return ReturnTimedMenuCall(cpu, &frame, result.pc);
    const uint16_t stack = cpu->S;
    bool progress = false;
    (void)Lufia2DecompFrameReturnReadByte(cpu, result.pc, &progress);
    const uint16_t low = Lufia2DecompFrameReturnReadByte(cpu, ++cpu->S, &progress);
    const uint16_t high = Lufia2DecompFrameReturnReadByte(cpu, ++cpu->S, &progress);
    const uint8_t bank = Lufia2DecompFrameReturnReadByte(cpu, ++cpu->S, &progress);
    Lufia2DecompFrameReturnAccountOpcode(cpu, 6u,
        cpu_region_speed(result.pc) + 3u * 8u + 2u * 6u, progress);
    const uint32_t target = (uint32_t)bank << 16 | (uint16_t)((high << 8 | low) + 1u);
    cpu->PB = bank;
    return ActorBridgeResolveReturn(cpu, &frame, 3u, result.pc, stack, target);
}
