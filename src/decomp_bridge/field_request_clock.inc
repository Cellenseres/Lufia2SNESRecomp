enum { FIELD_REQUEST_MAX_BODY_OPCODES = 13u, FIELD_REQUEST_CLOCK_RESERVE = 512u };

_Static_assert(FIELD_REQUEST_MAX_BODY_OPCODES + 2u <= LUFIA2_FIELD_REQUEST_OPCODE_BUDGET,
    "review request and original caller budget");

static unsigned PlanFieldMenuRequestClocks(const uint8_t *ram, FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define MENU_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    const bool queued = (ram[0x05b5u] & 0x40u) != 0u;
    MENU_OPCODE(0x83a0u, 2u, 2u, 0u);
    MENU_OPCODE(0x83a2u, 3u, 6u, 2u);
    MENU_OPCODE(0x83a5u, 2u, queued ? 3u : 2u, 0u);
    if (queued)
        return count;
    const bool window = (ram[0x09a7u] & 2u) != 0u;
    MENU_OPCODE(0x83a7u, 3u, 4u, 1u);
    MENU_OPCODE(0x83aau, 2u, 2u, 0u);
    MENU_OPCODE(0x83acu, 2u, window ? 2u : 3u, 0u);
    if (window) {
        const unsigned held = (ram[0x46u] | (unsigned)ram[0x47u] << 8) & 0x9080u;
        MENU_OPCODE(0x83aeu, 2u, 3u, 0u);
        MENU_OPCODE(0x83b0u, 3u, 3u, 0u);
        MENU_OPCODE(0x83b3u, 2u, 4u, 2u);
        MENU_OPCODE(0x83b5u, 2u, held ? 2u : 3u, 0u);
        if (held) {
            const bool pressed = ((ram[0x4au] | (unsigned)ram[0x4bu] << 8) & held) != 0u;
            MENU_OPCODE(0x83b7u, 2u, 7u, 4u);
            MENU_OPCODE(0x83b9u, 2u, pressed ? 2u : 3u, 0u);
            MENU_OPCODE(pressed ? 0x83bbu : 0x83ddu, 2u, 3u, 0u);
            return count;
        }
    }
    MENU_OPCODE(0x83ddu, 2u, 3u, 0u);
#undef MENU_OPCODE
    return count;
}

static unsigned PlanFieldStatusRequestClocks(const uint8_t *ram, FixedSpanOpcode *plan) {
    unsigned count = 0u;
#define STATUS_OPCODE(pc, fetched, cycles, data) \
    (plan[count++] = (FixedSpanOpcode){pc, fetched, cycles, data})
    const bool busy = (ram[0x09a8u] & 8u) != 0u;
    STATUS_OPCODE(0x8103u, 3u, 4u, 1u);
    STATUS_OPCODE(0x8106u, 2u, 2u, 0u);
    STATUS_OPCODE(0x8108u, 2u, busy ? 3u : 2u, 0u);
    if (!busy) {
        static const uint8_t masks[] = {2u, 4u, 1u};
        static const uint16_t tests[] = {0x810fu, 0x8119u, 0x8123u};
        static const uint16_t branches[] = {0x8111u, 0x811bu, 0x8125u};
        STATUS_OPCODE(0x810au, 2u, 3u, 0u);
        STATUS_OPCODE(0x810cu, 3u, 4u, 1u);
        for (unsigned request = 0u; request < 3u; ++request) {
            const bool active = (ram[0x05b7u] & masks[request]) != 0u;
            STATUS_OPCODE(tests[request], 2u, 2u, 0u);
            STATUS_OPCODE(branches[request], 2u, active ? 2u : 3u, 0u);
            if (active)
                return count;
        }
    }
    STATUS_OPCODE(0x812bu, 2u, 3u, 0u);
#undef STATUS_OPCODE
    return count;
}

static bool FieldRequestClockSupported(const CpuState *cpu, uint32_t entry) {
    if (entry != 0x8383a0u && entry != 0x838103u)
        return false;
    const uint32_t caller_return = entry == 0x8383a0u ? 0x83807au : 0x838092u;
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->D || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        g_interp_apu_driving || !Lufia2DecompFramePairedCallReady(cpu, entry) ||
        Lufia2DecompFramePairedReturnPc(cpu, entry) != caller_return ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - FIELD_REQUEST_CLOCK_RESERVE ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffdu,
            false, 1363u, FIELD_REQUEST_CLOCK_RESERVE) ||
        !g_snes->apu || !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->vPos >= 262u || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x18681u)
        return false;
    const uint32_t end = entry == 0x8383a0u ? 0x8383e0u : 0x83812eu;
    if (!Lufia2DecompFrameRangeQuiet(entry, end))
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < FIELD_REQUEST_CLOCK_RESERVE)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static RecompReturn ActorBridgeTimedFieldRequest(CpuState *cpu, uint32_t entry) {
    if (!FieldRequestClockSupported(cpu, entry)) {
        const uint32_t actual_entry = (uint32_t)cpu->PB << 16 | (entry & 0xffffu);
        return interp_tier_dispatch_tail(cpu, actual_entry, actual_entry,
            cpu->S, cpu->host_return_valid);
    }
    const uint64_t start = cpu->master_cycles;
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid,
        Lufia2DecompFramePairedReturnPc(cpu, entry)};
    FixedSpanOpcode plan[FIELD_REQUEST_MAX_BODY_OPCODES];
    const bool menu = entry == 0x8383a0u;
    const unsigned count = menu ? PlanFieldMenuRequestClocks(cpu->ram, plan)
        : PlanFieldStatusRequestClocks(cpu->ram, plan);
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x830000u, .timing = plan, .count = count};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = menu ? Lufia2FieldMenuRequest(&memory, &state)
        : Lufia2FieldStatusRequests(&memory, &state);
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    assert(cpu->master_cycles - start <= FIELD_REQUEST_CLOCK_RESERVE);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        assert((menu && result.pc == 0x8383bdu) ||
            (!menu && (result.pc == 0x838113u || result.pc == 0x83811du || result.pc == 0x838127u)));
        Lufia2DecompFramePairedAccountSpan(cpu, entry, count, clock.progress_mask);
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (menu ? 0x8383dfu : 0x83812du));
    bool return_progress = false;
    const RecompReturn returned = ReturnTimedMenuCallWithProgress(
        cpu, &frame, result.pc, &return_progress);
    Lufia2DecompFramePairedAccountSpan(cpu, entry, count + 1u,
        clock.progress_mask | (unsigned)return_progress << count);
    assert(cpu->master_cycles - start <= FIELD_REQUEST_CLOCK_RESERVE);
    return returned;
}
