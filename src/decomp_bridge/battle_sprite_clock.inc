typedef struct BattleSpriteClockGroup {
    uint16_t enabled, count, source;
    bool indirect;
} BattleSpriteClockGroup;

static const BattleSpriteClockGroup battle_sprite_clock_groups[] = {
    {0x15d7u, 0x15dau, 0x15d8u, true},
    {0x15dfu, 0x15e2u, 0x4c8au, false},
    {0x15cfu, 0x15d2u, 0x493du, false},
    {0x15cbu, 0x15ceu, 0x15ccu, true},
    {0x15c7u, 0x15cau, 0x48c0u, false},
    {0x15e3u, 0x15e6u, 0x4dcau, false},
    {0x15d3u, 0x15d6u, 0x4956u, false},
    {0x15dbu, 0x15deu, 0x4b4au, false},
    {0x15e7u, 0x15eau, 0x4b36u, false}
};

static void BattleSpriteAddCost(BattleLeafCost *total, BattleLeafCost part) {
    total->cycles += part.cycles;
    total->rom_bytes += part.rom_bytes;
    total->ram_bytes += part.ram_bytes;
}

static bool BattleSpriteBuildCost(const uint8_t *ram, BattleLeafCost *cost) {
    const unsigned overlays = ram[0x154eu];
    if (overlays > 6u)
        return false;
    *cost = (BattleLeafCost){2662u, 1352u, 312u};
    unsigned used = 0u;
    for (unsigned i = 0u; i < 9u; ++i) {
        const BattleSpriteClockGroup *group = &battle_sprite_clock_groups[i];
        if (!ram[group->enabled])
            continue;
        const unsigned count = ram[group->count];
        if (i == 6u && !count) {
            BattleSpriteAddCost(cost, (BattleLeafCost){15u, 12u, 3u});
            continue;
        }
        const BattleLeafCost setup = i == 6u ? (BattleLeafCost){20u, 15u, 5u}
            : group->indirect ? (BattleLeafCost){21u, 14u, 7u}
                             : (BattleLeafCost){19u, 14u, 5u};
        BattleSpriteAddCost(cost, setup);
        BattleSpriteAddCost(cost, BattleOamCost(count, used & 3u));
        used += count;
    }
    if (!overlays)
        return true;
    BattleSpriteAddCost(cost, (BattleLeafCost){20u + 42u * overlays,
        16u + 28u * overlays, 3u + 6u * overlays});
    for (unsigned i = 0u; i < overlays; ++i) {
        if (!ram[0x1534u + i])
            continue;
        const unsigned count = ram[0x157fu + i] ? ram[0x157fu + i] : 256u;
        BattleSpriteAddCost(cost, (BattleLeafCost){69u + 58u * count,
            43u + 33u * count, 17u + 8u * count});
    }
    return true;
}

static bool BattleSpriteSourcesSupported(const uint8_t *ram) {
    unsigned used = 0u;
    for (unsigned i = 0u; i < 9u; ++i) {
        const BattleSpriteClockGroup *group = &battle_sprite_clock_groups[i];
        if (!ram[group->enabled])
            continue;
        const unsigned count = ram[group->count];
        const unsigned source = group->indirect
            ? ram[group->source] | (unsigned)ram[group->source + 1u] << 8u
            : group->source;
        if (used + count > 128u ||
            (count && (source < 0x4000u || source + count * 5u > 0x10000u)))
            return false;
        used += count;
    }
    return ram[0x154eu] <= 6u;
}

static uint8_t BattleSpriteInlineChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    const Lufia2Memory *memory = context;
    assert(frame_size == 2u && (target == 0x81b5a3u || target == 0x81b705u));
    (void)frame_size;
    const Lufia2ExecutionResult result = target == 0x81b5a3u
        ? Lufia2BattleHideOam(memory, state)
        : Lufia2BattleAppendOamSprites(memory, state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    (void)result;
    const uint16_t low = memory->read_byte(memory->context, ++state->stack);
    const uint16_t high = memory->read_byte(memory->context, ++state->stack);
    assert((uint16_t)((high << 8 | low) + 1u) == (uint16_t)(site + 3u));
    (void)low;
    (void)high;
    state->program_bank = (uint8_t)(site >> 16);
    return 1u;
}

static bool BattleSpriteClockSupported(const CpuState *cpu, uint32_t entry,
                                      BattleLeafCost *cost) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || !cpu->m_flag || cpu->x_flag ||
        cpu->_flag_D || cpu->D || cpu->S < 0x1f10u || !g_snes || !g_snes->cart ||
        g_snes->cart->type != CART_LOROM ||
        !((cpu->DB & 0x7fu) < 0x40u || cpu->DB == 0x7eu) ||
        (g_snes->vPos < 225u && !g_snes->hdmaBeamOff) ||
        !interp_bridge_use_absolute_apu_timeline(rtl_apu_frame_timeline_active(),
            cart_has_sa1(g_snes->cart), rtl_apu_extended_frame_timing() &&
            g_snes->apu && g_snes->apu->portTimeValid))
        return false;
    const bool parent = entry == 0x81b5c4u;
    if (parent) {
        if (!BattleSpriteSourcesSupported(cpu->ram) || !BattleSpriteBuildCost(cpu->ram, cost) ||
            !Lufia2DecompFrameRangeQuiet(0x81b5a3u, 0x81b5c4u) ||
            !Lufia2DecompFrameRangeQuiet(0x81b6c4u, 0x81b705u) ||
            !Lufia2DecompFrameRangeQuiet(0x81b705u, 0x81b7d9u))
            return false;
    } else {
        assert(entry == 0x81b5a3u);
        *cost = (BattleLeafCost){2550u, 1281u, 290u};
    }
    return BattleClockAllows(cpu, entry, parent ? 0x81b6c4u : 0x81b5c4u,
        parent ? 3u : 2u, false, BattleLeafClockRoom(entry, *cost), false, NULL);
}

static RecompReturn ActorBridgeTimedBattleSprites(CpuState *cpu, bool parent) {
    const uint32_t entry = parent ? 0x81b5c4u : 0x81b5a3u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    BattleLeafCost cost;
    if (!BattleSpriteClockSupported(cpu, entry, &cost))
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = parent
        ? Lufia2BattleBuildSprites(&memory, &state, BattleSpriteInlineChild, &memory)
        : Lufia2BattleHideOam(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (parent ? 0x81b704u : 0x81b5c3u));
    ActorBridgeStore(cpu, &state);
    cost.cycles -= 6u;
    --cost.rom_bytes;
    cost.ram_bytes -= parent ? 3u : 2u;
    Lufia2DecompFrameReturnAccountOpcode(cpu, cost.cycles, BattleLeafMaster(entry, cost), true);
    return parent ? ReturnTimedLongCall(cpu, &frame, result.pc)
                  : ReturnTimedMenuCall(cpu, &frame, result.pc);
}
