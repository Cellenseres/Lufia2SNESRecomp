#include "lufia2/battle_action_parameter_steps.h"

_Static_assert(LUFIA2_BATTLE_ACTION_PARAMETER_MAX_OPCODES <= 32u, "Action parameter progress mask");

static const FixedSpanOpcode action_attribute_advance[] = {
    {0xd3edu, 1u, 2u, 0u},
    {0xd3eeu, 1u, 2u, 0u},
    {0xd3efu, 1u, 2u, 0u},
    {0xd3f0u, 1u, 2u, 0u},
    {0xd3f1u, 1u, 2u, 0u},
    {0xd3f2u, 1u, 2u, 0u},
    {0xd3f3u, 1u, 2u, 0u},
    {0xd3f4u, 1u, 2u, 0u},
    {0xd3f5u, 3u, 3u, 0u},
    {0xd3f8u, 2u, 2u, 0u},
    {0xd3fau, 3u, 3u, 0u},

};

static const FixedSpanOpcode action_attribute_last[] = {
    {0xd3edu, 1u, 2u, 0u},
    {0xd3eeu, 1u, 2u, 0u},
    {0xd3efu, 1u, 2u, 0u},
    {0xd3f0u, 1u, 2u, 0u},
    {0xd3f1u, 1u, 2u, 0u},
    {0xd3f2u, 1u, 2u, 0u},
    {0xd3f3u, 1u, 2u, 0u},
    {0xd3f4u, 1u, 2u, 0u},
    {0xd3f5u, 3u, 3u, 0u},
    {0xd3f8u, 2u, 3u, 0u},
};

static const FixedSpanOpcode action_status_restore[] = {
    {0xd3fdu, 1u, 5u, 2u},
    {0xd3feu, 2u, 3u, 0u},
    {0xd400u, 2u, 2u, 0u},
    {0xd402u, 2u, 3u, 1u},
    {0xd404u, 3u, 3u, 0u},
    {0xd407u, 2u, 2u, 0u},
};

static const FixedSpanOpcode action_status_remove[] = {
    {0xd400u, 2u, 2u, 0u},
    {0xd402u, 2u, 3u, 1u},
    {0xd404u, 3u, 3u, 0u},
    {0xd407u, 2u, 2u, 0u},
};

static const FixedSpanOpcode action_status_add[] = {
    {0xd432u, 2u, 2u, 0u},
    {0xd434u, 2u, 3u, 1u},
    {0xd436u, 3u, 3u, 0u},
    {0xd439u, 2u, 2u, 0u},
};

static const FixedSpanOpcode action_attribute_apply[] = {
    {0xd63au, 2u, 7u, 5u},
    {0xd63cu, 1u, 2u, 0u},
    {0xd63du, 3u, 6u, 2u},
    {0xd640u, 3u, 7u, 2u},
    {0xd643u, 1u, 2u, 0u},
    {0xd644u, 1u, 2u, 0u},
    {0xd645u, 1u, 2u, 0u},
    {0xd646u, 1u, 2u, 0u},
    {0xd647u, 1u, 2u, 0u},
    {0xd648u, 1u, 2u, 0u},
    {0xd649u, 1u, 2u, 0u},
    {0xd64au, 1u, 2u, 0u},
    {0xd64bu, 3u, 3u, 0u},
    {0xd64eu, 2u, 3u, 0u},
};

static bool ActionParameterAddressIsRam(uint32_t address) {
    const unsigned bank = address >> 16;
    return bank == 0x7eu || bank == 0x7fu ||
        ((bank & 0x7fu) < 0x40u && (address & 0xffffu) < 0x2000u);
}

static void ActorBridgeActionAttributeProbe(CpuState *cpu) {
    const uint32_t entry = 0x85d33du;
    const uint32_t fence = 0x85d344u;
    const unsigned rom_offset = 0x2d33du;
    const uint8_t expected[] = {0xb7u, 0xb8u, 0xd0u, 0x03u, 0x4cu, 0xedu, 0xd3u};
    const Cart *cart = g_snes->cart;
    if (cart->romSize < rom_offset + sizeof expected ||
        memcmp(cart->rom + rom_offset, expected, sizeof expected) != 0 ||
        !Lufia2DecompFrameRangeAllows(entry, fence, ActionParameterStepHookOwned) ||
        Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeActionParameterStep) < 3u)
        return;

    const uint32_t pointer = cpu->ram[0xb8u] | (uint32_t)cpu->ram[0xb9u] << 8 |
        (uint32_t)cpu->ram[0xbau] << 16;
    const uint32_t low = (pointer + cpu->Y) & 0xffffffu;
    const uint32_t high = (low + 1u) & 0xffffffu;
    if (!ActionParameterAddressIsRam(low) || !ActionParameterAddressIsRam(high))
        return;

    FixedSpanOpcode timing[] = {
        {0xd33du, 2u, 7u, 5u}, {0xd33fu, 2u, 2u, 0u}, {0xd341u, 3u, 3u, 0u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u, .timing = timing, .count = 3u};
    for (uint32_t address = 0xb8u; address <= 0xbau; ++address)
        (void)FixedSpanRead(&clock, address);
    uint16_t value = FixedSpanRead(&clock, low);
    value |= (uint16_t)FixedSpanRead(&clock, high) << 8;

    if (value) {
        timing[1].cycles = 3u;
        clock.count = 2u;
    }
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    cpu->A = value;
    cpu->_flag_N = (value & 0x8000u) != 0u;
    cpu->_flag_Z = value == 0u;
    cpu_mirrors_to_p(cpu);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(value ? 0x85d344u : 0x85d3edu);
}

static void ActorBridgeActionParameterStep(CpuState *cpu, uint32_t entry) {
    const bool wide = entry == 0x85d33du || entry == 0x85d3edu || entry == 0x85d3fdu || entry == 0x85d63au;
    const unsigned limit = entry == 0x85d3fdu ? 0x1ffdu : 0x1fffu;
    const unsigned reserve = 512u;
    if (!ActionParameterStepHookOwned(entry, ActorBridgeActionParameterStep) ||
        cpu != &g_cpu || !cpu->ram || cpu->PB != 0x85u || cpu->D || cpu->_flag_D ||
        cpu->x_flag || cpu->m_flag != !wide ||
        !(cpu->DB < 0x40u || (cpu->DB >= 0x7eu && cpu->DB < 0xc0u)) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve)
        return;
    if (entry == 0x85d3edu &&
        (cpu->Y < 0x30u || cpu->Y > 0x54u || (cpu->Y - 0x30u) % 6u))
        return;
    if (entry == 0x85d63au) {
        if (cpu->Y < 0x2cu || cpu->Y > 0x50u || (cpu->Y - 0x2cu) % 6u || cpu->X < 0x200u)
            return;
        const uint32_t pointer = cpu->ram[0xb8u] | (uint32_t)cpu->ram[0xb9u] << 8 |
            (uint32_t)cpu->ram[0xbau] << 16;
        const uint32_t field = (pointer + cpu->Y) & 0xffffffu;
        const uint32_t actor = (((uint32_t)cpu->DB << 16) + cpu->X + 0x37u) & 0xffffffu;
        if (!ActionParameterAddressIsRam(field) ||
            !ActionParameterAddressIsRam((field + 1u) & 0xffffffu) ||
            !ActionParameterAddressIsRam(actor) ||
            !ActionParameterAddressIsRam((actor + 1u) & 0xffffffu))
            return;
    }
    // These CPU/WRAM spans cannot enable HDMA while its beam processing is off.
    if (!NativeOpcodeWindowWithDeferredHdmaSupported(cpu, entry, reserve, limit, true, true))
        return;
    if (entry == 0x85d33du) {
        ActorBridgeActionAttributeProbe(cpu);
        return;
    }
    const FixedSpanOpcode *timing;
    unsigned count;
    uint32_t next, fence;
#define ACTION_PLAN(rows, target, end) do { \
    timing = rows; count = sizeof rows / sizeof rows[0]; next = target; fence = end; \
} while (0)
    if (entry == 0x85d3edu) {
        if (cpu->Y == 0x54u) {
            ACTION_PLAN(action_attribute_last, 0x85d3fdu, 0x85d3fdu);
        } else {
            ACTION_PLAN(action_attribute_advance, 0x85d33du, 0x85d3fdu);
        }
    } else if (entry == 0x85d3fdu) {
        ACTION_PLAN(action_status_restore, 0x85d409u, 0x85d409u);
    } else if (entry == 0x85d400u) {
        ACTION_PLAN(action_status_remove, 0x85d409u, 0x85d409u);
    } else if (entry == 0x85d432u) {
        ACTION_PLAN(action_status_add, 0x85d43bu, 0x85d43bu);
    } else {
        ACTION_PLAN(action_attribute_apply,
            cpu->Y == 0x50u ? 0x85d650u : 0x85d63au, 0x85d650u);
    }
#undef ACTION_PLAN
    if (!Lufia2DecompFrameRangeAllows(entry, fence, ActionParameterStepHookOwned) ||
        Lufia2DecompFrameOwnedSpanBudget(entry, fence, ActorBridgeActionParameterStep) < count)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || charged != cpu->master_cycles)
        return;
    FixedSpanOpcode plan[LUFIA2_BATTLE_ACTION_PARAMETER_MAX_OPCODES];
    memcpy(plan, timing, count * sizeof plan[0]);
    if (entry == 0x85d63au && cpu->Y == 0x50u)
        plan[count - 1u].cycles = 2u;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned fetched = fence - entry < 32u ? fence - entry : 32u;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, fetched, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x850000u, .timing = plan,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleActionParameterStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == next);
    assert(clock.index == count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
