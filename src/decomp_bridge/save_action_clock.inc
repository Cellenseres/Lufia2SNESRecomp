#include "lufia2/save_action_steps.h"

static void ActorBridgeSaveActionStep(CpuState *cpu, uint32_t entry);

static bool SaveActionHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x82e9e2u || pc == 0x82ea40u || pc == 0x82ea57u || pc == 0x82ea74u) &&
        hook == ActorBridgeSaveActionStep;
}

static void ActorBridgeSaveActionStep(CpuState *cpu, uint32_t entry) {
    if (!SaveActionHookOwned(entry, ActorBridgeSaveActionStep) || cpu != &g_cpu ||
        cpu->PB != 0x82u || cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const bool menu = entry == 0x82e9e2u;
    const unsigned reserve = menu ? 256u : entry == 0x82ea40u ? 192u
        : entry == 0x82ea57u ? 224u : 160u;
    const uint32_t end = menu ? 0x82ea29u : entry == 0x82ea40u ? 0x82ea99u
        : entry == 0x82ea57u ? 0x82ea7eu : 0x82ea7du;
    if (cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, end, SaveActionHookOwned))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    FixedSpanOpcode timing[LUFIA2_SAVE_ACTION_MAX_OPCODES];
    unsigned count = 0u;
    uint32_t stop;
    if (menu || entry == 0x82ea40u) {
        const uint8_t input = (uint8_t)cpu->A;
        const bool confirm = input == 8u || input == 2u;
        const unsigned comparisons = input == 8u ? 1u : input == 2u ? 2u : 3u;
        for (unsigned i = 0u; i < comparisons; ++i) {
            timing[count++] = (FixedSpanOpcode){(uint16_t)(entry + 4u * i), 2u, 2u, 0u};
            const bool taken = i + 1u == comparisons && (confirm || input == 1u);
            timing[count++] = (FixedSpanOpcode){(uint16_t)(entry + 4u * i + 2u),
                2u, taken ? 3u : 2u, 0u};
        }
        if (confirm) {
            timing[count++] = (FixedSpanOpcode){menu ? 0xe9f6u : 0xea97u, 2u, 2u, 0u};
            stop = menu ? 0x82e9f8u : 0x82ea99u;
        } else if (!menu && input == 1u) {
            stop = 0x82ea54u;
        } else {
            const bool back = menu && input == 1u && (cpu->ram[0x14acu] & 4u);
            if (menu && input == 1u) {
                timing[count++] = (FixedSpanOpcode){0xea22u, 3u, 4u, 1u};
                timing[count++] = (FixedSpanOpcode){0xea25u, 2u, 2u, 0u};
                timing[count++] = (FixedSpanOpcode){0xea27u, 2u, back ? 2u : 3u, 0u};
            }
            if (back) {
                stop = 0x82ea29u;
            } else {
                timing[count++] = (FixedSpanOpcode){menu ? 0xe9eeu : 0xea4cu, 2u, 2u, 0u};
                timing[count++] = (FixedSpanOpcode){menu ? 0xe9f0u : 0xea4eu, 2u, 3u, 0u};
                stop = menu ? 0x82e9dfu : 0x82ea3du;
            }
        }
    } else if (entry == 0x82ea57u) {
        const bool back = (cpu->ram[0x14acu] & 8u) != 0u;
        const bool change = back && cpu->ram[0x14edu] != 0u;
        static const FixedSpanOpcode plan[] = {
            {0xea57u, 3u, 4u, 1u}, {0xea5au, 2u, 2u, 0u},
            {0xea5cu, 2u, 2u, 0u}, {0xea5eu, 3u, 3u, 0u},
            {0xea61u, 3u, 4u, 1u}, {0xea64u, 2u, 2u, 0u},
            {0xea66u, 3u, 6u, 1u}
        };
        count = change ? 7u : back ? 6u : 3u;
        for (unsigned i = 0u; i < count; ++i)
            timing[i] = plan[i];
        if (!change)
            timing[count - 1u].cycles = 3u;
        stop = change ? 0x82ea69u : 0x82ea7eu;
    } else {
        const FixedSpanOpcode plan[] = {
            {0xea74u, 2u, 2u, 0u}, {0xea76u, 3u, 4u, 1u},
            {0xea79u, 3u, 4u, 1u}, {0xea7cu, 1u, 2u, 0u}
        };
        count = 4u;
        for (unsigned i = 0u; i < count; ++i)
            timing[i] = plan[i];
        stop = 0x82ea7du;
    }
    if (Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeSaveActionStep) < count)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const unsigned bytes = menu ? 16u : entry == 0x82ea40u ? 16u
        : entry == 0x82ea57u ? 18u : 9u;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, bytes, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x820000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2SaveActionStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == stop);
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
