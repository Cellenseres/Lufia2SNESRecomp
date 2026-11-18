#include "lufia2/field_scene_steps.h"

static void ActorBridgeSceneObjectStep(CpuState *cpu, uint32_t entry);

static bool SceneObjectHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return (pc == 0x83a7cau || pc == 0x83a804u) && hook == ActorBridgeSceneObjectStep;
}

static void ActorBridgeSceneObjectStep(CpuState *cpu, uint32_t entry) {
    if (!SceneObjectHookOwned(entry, ActorBridgeSceneObjectStep) || cpu != &g_cpu ||
        cpu->PB != 0x83u || cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        cpu->X >= 32u || !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu) ||
        !Lufia2DecompOpcodeWindowReady() || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const bool copy = entry == 0x83a804u;
    const unsigned reserve = copy ? 608u : 320u;
    const uint32_t end = copy ? 0x83a82cu : 0x83a801u;
    if (cpu->master_cycles > UINT64_MAX - reserve ||
        !NativeOpcodeWindowSupported(cpu, entry, reserve, 0x1fffu, false) ||
        !Lufia2DecompFrameRangeAllows(entry, end, SceneObjectHookOwned))
        return;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (phase >= 1364u || (charged && charged != cpu->master_cycles))
        return;
    const bool active = (cpu->ram[0x064au + cpu->X] & 0x80u) != 0u;
    const bool selected = active && cpu->ram[(copy ? 0x1e3a6u : 0x1e1aeu) + cpu->X] != 0xffu;
    static const FixedSpanOpcode prepare[] = {
        {0xa7cau, 2u, 2u, 0u}, {0xa7ccu, 3u, 6u, 1u},
        {0xa7cfu, 3u, 4u, 1u}, {0xa7d2u, 2u, 2u, 0u},
        {0xa7d4u, 2u, 2u, 0u}, {0xa7d6u, 4u, 5u, 1u},
        {0xa7dau, 2u, 2u, 0u}, {0xa7dcu, 2u, 2u, 0u},
        {0xa7deu, 1u, 4u, 2u}, {0xa7dfu, 1u, 2u, 0u},
        {0xa7e0u, 2u, 3u, 1u}
    };
    static const FixedSpanOpcode sprites[] = {
        {0xa804u, 3u, 4u, 1u}, {0xa807u, 2u, 2u, 0u},
        {0xa809u, 2u, 2u, 0u}, {0xa80bu, 1u, 2u, 0u},
        {0xa80cu, 4u, 5u, 1u}, {0xa810u, 2u, 2u, 0u},
        {0xa812u, 2u, 2u, 0u}, {0xa814u, 1u, 4u, 2u},
        {0xa815u, 1u, 2u, 0u}, {0xa816u, 4u, 5u, 1u},
        {0xa81au, 1u, 3u, 0u}, {0xa81bu, 4u, 5u, 1u},
        {0xa81fu, 1u, 5u, 2u}, {0xa820u, 4u, 5u, 1u},
        {0xa824u, 1u, 3u, 0u}, {0xa825u, 4u, 5u, 1u}
    };
    unsigned count = copy ? selected ? 16u : active ? 7u : 3u
                          : selected ? 11u : active ? 8u : 5u;
    FixedSpanOpcode timing[LUFIA2_SCENE_OBJECT_MAX_OPCODES];
    const FixedSpanOpcode *plan = copy ? sprites : prepare;
    for (unsigned i = 0u; i < count; ++i)
        timing[i] = plan[i];
    if (!active || !selected)
        timing[count - 1u].cycles = 3u;
    if (copy || !selected) {
        timing[count++] = (FixedSpanOpcode){copy ? 0xa829u : 0xa7feu, 1u, 2u, 0u};
        timing[count++] = (FixedSpanOpcode){copy ? 0xa82au : 0xa7ffu, 2u,
            cpu->X ? 3u : 2u, 0u};
    }
    if (Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeSceneObjectStep) < count)
        return;
    Lufia2DecompRomOpcodeSpan fetch;
    const bool prepared = Lufia2DecompPrepareRomOpcodeSpan(cpu, entry, copy ? 32u : 24u, &fetch);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing,
        .count = count, .opcode_span = prepared ? &fetch : NULL};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const uint64_t start = cpu->master_cycles;
    const Lufia2ExecutionResult result = Lufia2FieldSceneObjectStep(&memory, &state, entry);
    BeginFixedSpanOpcode(&clock);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(result.pc == (!copy && selected ? 0x83a7e2u : cpu->X ? entry : end));
    assert(clock.index == clock.count && !clock.remaining);
    assert(cpu->master_cycles - start <= reserve);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
