#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)
#include "lufia2/nmi_tilemap_steps.h"
#endif

static void ActorBridgeNmiTilemapStep(CpuState *cpu, uint32_t entry);

static uint32_t NmiTilemapStepHookEnd(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeNmiTilemapStep)
        return 0u;
    switch (pc & 0x7fffffu) {
    case 0x0087fcu: return pc + 6u;
#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)
    case 0x008802u: case 0x008811u: case 0x008820u: return pc + 6u;
#endif
    case 0x00880du: case 0x00881cu: case 0x008829u: return pc + 4u;
    default: return 0u;
    }
}

static bool NmiTilemapStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return NmiTilemapStepHookEnd(pc, hook) != 0u;
}

static void ActorBridgeNmiTilemapStep(CpuState *cpu, uint32_t entry) {
    const uint32_t end = NmiTilemapStepHookEnd(entry, ActorBridgeNmiTilemapStep);
    if (!end || entry < 0x800000u || cpu != &g_cpu || cpu->m_flag != 1u ||
        cpu->x_flag || !NmiClockWindow(cpu, false, entry, end, 256u, 256u,
            true, NmiTilemapStepHookOwned))
        return;
    FixedSpanOpcode timing[LUFIA2_NMI_TILEMAP_HEADER_OPCODES];
    unsigned count = 0u;
#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)
    const bool prepare = entry == 0x808802u || entry == 0x808811u || entry == 0x808820u;
    if (prepare) {
        timing[count++] = (FixedSpanOpcode){(uint16_t)entry, 3u, 3u, 0u};
        timing[count++] = (FixedSpanOpcode){(uint16_t)(entry + 3u), 3u, 3u, 0u};
    } else
#endif
    if (entry == 0x808829u) {
        timing[count++] = (FixedSpanOpcode){0x8829u, 2u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){0x882bu, 2u, 5u, 2u};
    } else {
        const bool header = entry == 0x8087fcu;
        const uint8_t requests = header ? cpu->ram[0x74u] : (uint8_t)cpu->A;
        const uint8_t mask = header ? 0xc0u : entry == 0x80880du ? 0x30u : 0x0cu;
        const uint16_t bit_pc = (uint16_t)(entry + (header ? 2u : 0u));
        if (header)
            timing[count++] = (FixedSpanOpcode){(uint16_t)entry, 2u, 3u, 1u};
        timing[count++] = (FixedSpanOpcode){bit_pc, 2u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){bit_pc + 2u, 2u,
            (uint8_t)(requests & mask ? 2u : 3u), 0u};
    }
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = count};
    const Lufia2Memory memory = {NmiClockRead, NmiClockWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)
        prepare ? Lufia2NmiPrepareTilemapBlock(&state, entry) :
#endif
        Lufia2NmiStepTilemapRequests(&memory, &state, entry);
    NmiClockBeginOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
