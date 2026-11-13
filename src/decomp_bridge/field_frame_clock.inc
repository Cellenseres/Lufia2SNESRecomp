enum { FIELD_FRAME_MAX_OPCODES = 24u, FIELD_FRAME_CLOCK_RESERVE = 672u };

static void ActorBridgePrepareFieldFrame(CpuState *cpu, uint32_t entry);

static bool FieldFrameHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x838007u && hook == ActorBridgePrepareFieldFrame;
}

static unsigned PlanFieldFrameClocks(const uint8_t *ram, FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define FRAME_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    const bool second_pad = ram[0x057cu] != 0u;
    FRAME_OPCODE(0x8007u, 2u, 2u, 0u);
    FRAME_OPCODE(0x8009u, 3u, 4u, 1u);
    FRAME_OPCODE(0x800cu, 3u, 4u, 1u);
    FRAME_OPCODE(0x800fu, 2u, second_pad ? 2u : 3u, 0u);
    if (second_pad) {
        const bool wait = (ram[0x49u] & 0x10u) != 0u;
        FRAME_OPCODE(0x8011u, 2u, 2u, 0u);
        FRAME_OPCODE(0x8013u, 2u, 3u, 1u);
        FRAME_OPCODE(0x8015u, 2u, wait ? 3u : 2u, 0u);
        if (wait)
            return count;
    }
    const bool blocked = (ram[0x0622u] & 0x80u) != 0u;
    FRAME_OPCODE(0x8017u, 3u, 4u, 1u);
    FRAME_OPCODE(0x801au, 2u, 2u, 0u);
    FRAME_OPCODE(0x801cu, 2u, blocked ? 3u : 2u, 0u);
    if (!blocked) {
        const bool requested = ram[0x05b7u] != 0u;
        FRAME_OPCODE(0x801eu, 3u, 4u, 1u);
        FRAME_OPCODE(0x8021u, 2u, requested ? 3u : 2u, 0u);
        if (!requested) {
            const bool reload = (ram[0x05b5u] & 0x80u) != 0u;
            FRAME_OPCODE(0x8023u, 2u, 2u, 0u);
            FRAME_OPCODE(0x8025u, 3u, 6u, 2u);
            FRAME_OPCODE(0x8028u, 2u, reload ? 2u : 3u, 0u);
            if (reload) {
                const bool transition = (ram[0x05b3u] & 8u) != 0u;
                FRAME_OPCODE(0x802au, 3u, 4u, 1u);
                FRAME_OPCODE(0x802du, 2u, 2u, 0u);
                FRAME_OPCODE(0x802fu, 2u, transition ? 2u : 3u, 0u);
                if (transition)
                    return count;
                const bool map = (ram[0x05b5u] & 8u) != 0u;
                FRAME_OPCODE(0x8036u, 2u, 2u, 0u);
                FRAME_OPCODE(0x8038u, 3u, 6u, 2u);
                FRAME_OPCODE(0x803bu, 2u, map ? 2u : 3u, 0u);
                if (map)
                    return count;
            }
        }
    }
    const bool scene = (ram[0x05b5u] & 0x20u) != 0u;
    FRAME_OPCODE(0x8062u, 2u, 2u, 0u);
    FRAME_OPCODE(0x8064u, 3u, 6u, 2u);
    FRAME_OPCODE(0x8067u, 2u, scene ? 2u : 3u, 0u);
#undef FRAME_OPCODE
    return count;
}

static bool FieldFrameClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x838007u || cpu != &g_cpu || !cpu->ram || cpu->emulation ||
        cpu->D || cpu->_flag_D || cpu->PB != 0x83u || cpu->DB != 0x83u ||
        cpu->m_flag != 1u || cpu->x_flag > 1u || cpu->S != 0x1fffu ||
        cpu->host_return_valid || g_interp_bridge_pc_hook ||
        !g_snes || !g_snes->dma || !g_snes->apu || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize <= 0x18070u || g_snes->vPos >= 262u ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIELD_FRAME_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu,
            false, 1363u, FIELD_FRAME_CLOCK_RESERVE) ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < FIELD_FRAME_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static void ActorBridgePrepareFieldFrame(CpuState *cpu, uint32_t entry) {
    if (!FieldFrameClockSupported(cpu, entry))
        return;
    FixedSpanOpcode timing[FIELD_FRAME_MAX_OPCODES];
    const unsigned count = PlanFieldFrameClocks(cpu->ram, timing);
    if (Lufia2DecompFrameSpanBudget(entry, 0x838070u, ActorBridgePrepareFieldFrame) < count)
        return;
    const uint64_t start = cpu->master_cycles;
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = timing, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldPrepareFrame(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= FIELD_FRAME_CLOCK_RESERVE);
    Lufia2DecompFrameReturnComplete(count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
