#include "lufia2/battle_command_steps.h"

static void ActorBridgeBattleCommandWidth(CpuState *cpu, uint32_t entry) {
    const bool narrow = entry == 0x818b3au;
    const uint32_t end = entry + 2u;
    const unsigned reserve = 128u;
    if (cpu->m_flag == narrow || cpu->x_flag || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, true) ||
        Lufia2DecompFrameOwnedSpanBudget(entry, end, ActorBridgeBattleCommandInputStep) < 1u)
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    const unsigned offset = (uint16_t)entry;
    const Cart *cart = g_snes->cart;
    if (cart->romSize < offset + 2u ||
        cart->rom[offset] != (narrow ? 0xe2u : 0xc2u) || cart->rom[offset + 1u] != 0x20u)
        return;

    const FixedSpanOpcode timing = {(uint16_t)entry, 2u, 3u, 0u};
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = &timing, .count = 1u};
    BeginFixedSpanOpcode(&clock);
    cpu->m_flag = narrow;
    cpu_mirrors_to_p(cpu);
    Lufia2DecompFrameReturnComplete(1u, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(end);
}

static void ActorBridgeBattleCommandInputStep(CpuState *cpu, uint32_t entry) {
    if (entry == 0x818b34u || entry == 0x818b3au) {
        ActorBridgeBattleCommandWidth(cpu, entry);
        return;
    }
    if (!BattleCommandInputHookOwned(entry, ActorBridgeBattleCommandInputStep) ||
        cpu != &g_cpu || cpu->PB != 0x81u || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->D || cpu->_flag_D || cpu->DB != 0x97u ||
        (cpu->host_return_valid && cpu->host_return_valid != 2u &&
         cpu->host_return_valid != 3u) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const unsigned reserve = entry == 0x81cbf1u ? 208u : entry == 0x81cc07u ? 160u
        : entry == 0x81cc1au ? 128u : 144u;
    const uint32_t end = entry == 0x81cbf1u ? 0x81cbffu : entry == 0x81cc07u ? 0x81cc22u
        : entry == 0x81cc1au ? 0x81cc1fu : 0x81cc2du;
    if (cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, end, BattleCommandInputHookOwned))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    static const FixedSpanOpcode preview_closed[] = {
        {0xcbf1u, 2u, 3u, 1u}, {0xcbf3u, 1u, 2u, 0u},
        {0xcbf4u, 2u, 3u, 1u}, {0xcbf6u, 2u, 2u, 0u},
        {0xcbf8u, 3u, 4u, 1u}, {0xcbfbu, 2u, 3u, 0u}
    };
    static const FixedSpanOpcode preview_ready[] = {
        {0xcbf1u, 2u, 3u, 1u}, {0xcbf3u, 1u, 2u, 0u},
        {0xcbf4u, 2u, 3u, 1u}, {0xcbf6u, 2u, 2u, 0u},
        {0xcbf8u, 3u, 4u, 1u}, {0xcbfbu, 2u, 2u, 0u},
        {0xcbfdu, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode primary[] = {
        {0xcc07u, 2u, 3u, 1u}, {0xcc09u, 2u, 2u, 0u},
        {0xcc0bu, 2u, 3u, 0u}, {0xcc14u, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode alternate[] = {
        {0xcc07u, 2u, 3u, 1u}, {0xcc09u, 2u, 2u, 0u},
        {0xcc0bu, 2u, 2u, 0u}, {0xcc0du, 2u, 2u, 0u},
        {0xcc0fu, 2u, 3u, 0u}, {0xcc20u, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode repeat[] = {
        {0xcc07u, 2u, 3u, 1u}, {0xcc09u, 2u, 2u, 0u},
        {0xcc0bu, 2u, 2u, 0u}, {0xcc0du, 2u, 2u, 0u},
        {0xcc0fu, 2u, 2u, 0u}, {0xcc11u, 3u, 3u, 0u}
    };
    static const FixedSpanOpcode primary_result[] = {
        {0xcc1au, 2u, 3u, 1u}, {0xcc1cu, 1u, 2u, 0u},
        {0xcc1du, 2u, 2u, 0u}
    };
    static const FixedSpanOpcode alternate_result[] = {
        {0xcc26u, 2u, 3u, 1u}, {0xcc28u, 1u, 2u, 0u},
        {0xcc29u, 2u, 2u, 0u}, {0xcc2bu, 2u, 2u, 0u}
    };
    const FixedSpanOpcode *plan;
    unsigned count;
    uint32_t stop;
    if (entry == 0x81cbf1u) {
        const bool ready = ((unsigned)cpu->ram[0x46u] << 1) & cpu->ram[0x47u] &
            0x20u & cpu->ram[0x057cu];
        plan = ready ? preview_ready : preview_closed;
        count = ready ? 7u : 6u;
        stop = ready ? 0x81cbffu : 0x81cc07u;
    } else if (entry == 0x81cc07u) {
        const uint8_t input = cpu->ram[0xddu];
        const bool first = (input & 0xa0u) != 0u;
        const bool second = !first && (input & 0x40u) != 0u;
        plan = first ? primary : second ? alternate : repeat;
        count = first ? 4u : 6u;
        stop = first ? 0x81cc16u : second ? 0x81cc22u : 0x81cb96u;
    } else {
        const bool first = entry == 0x81cc1au;
        plan = first ? primary_result : alternate_result;
        count = first ? 3u : 4u;
        stop = first ? 0x81cc1fu : 0x81cc2du;
    }
    if (Lufia2DecompFrameOwnedSpanBudget(entry, end,
            ActorBridgeBattleCommandInputStep) < count)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, end - entry, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x810000u, .timing = plan,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2BattleCommandInputStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == stop);
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
