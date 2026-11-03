enum { SPELL_FIELD_CLOCK_ROOM = 768u, SPELL_FIELD_CYCLE_LIMIT = 88u };

_Static_assert(SPELL_FIELD_CYCLE_LIMIT * 8u + 40u <= SPELL_FIELD_CLOCK_ROOM,
    "Spell field clock reservation");

static const FixedSpanOpcode cave_lower_tile[] = {
    {0x9d57u, 3u, 3u, 0u}, {0x9d5au, 2u, 4u, 2u}
};
static const FixedSpanOpcode cave_lower_tile_finish[] = {
    {0x9d5cu, 3u, 5u, 2u}, {0x9d5fu, 3u, 3u, 0u},
    {0x9d62u, 2u, 4u, 2u}, {0x9d64u, 3u, 7u, 2u}
};
static const FixedSpanOpcode cave_lower_tile_crossed[] = {
    {0x9d5cu, 3u, 6u, 2u}, {0x9d5fu, 3u, 3u, 0u},
    {0x9d62u, 2u, 4u, 2u}, {0x9d64u, 3u, 7u, 2u}
};

static const FixedSpanOpcode spell_field_pointer[] = {
    {0xf3e4u, 3u, 6u, 2u},
    {0xf446u, 2u, 3u, 0u}, {0xf448u, 3u, 5u, 2u},
    {0xf44bu, 3u, 3u, 0u}, {0xf44eu, 1u, 2u, 0u},
    {0xf44fu, 1u, 2u, 0u}, {0xf450u, 4u, 6u, 2u},
    {0xf454u, 1u, 2u, 0u}, {0xf455u, 3u, 3u, 0u},
    {0xf458u, 3u, 5u, 2u}, {0xf45bu, 2u, 3u, 0u},
    {0xf45du, 1u, 6u, 2u}, {0xf3e7u, 3u, 5u, 2u},
    {0xf3eau, 1u, 3u, 1u}, {0xf3ebu, 2u, 2u, 0u},
    {0xf3edu, 1u, 3u, 1u}, {0xf3eeu, 1u, 4u, 1u}
};
static const FixedSpanOpcode spell_field_byte[] = {
    {0xf3efu, 3u, 5u, 1u}, {0xf3f2u, 1u, 4u, 1u}
};

typedef struct TileSpellClock {
    FixedSpanClock span;
    const FixedSpanOpcode *finish;
    unsigned finish_count;
} TileSpellClock;

static uint8_t TileSpellRead(void *context, uint32_t address) {
    TileSpellClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index == clock->span.count && !clock->span.remaining) {
        clock->span.timing = clock->finish;
        clock->span.index = 0u;
        clock->span.count = clock->finish_count;
    }
    return FixedSpanRead(&clock->span, address);
}

static bool SpellFieldSourceSupported(const CpuState *cpu) {
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x0b0000u || !cpu->ram)
        return false;
    const unsigned offset = 0x0afa5bu + cpu->ram[0x0a0bu] * 2u;
    const uint8_t *record = g_snes->cart->rom + offset;
    const unsigned pointer = (record[0] + ((unsigned)record[1] << 8) +
        0xfa5bu) & 0xffffu;
    return pointer >= 0x8000u && pointer <= 0xfff5u;
}

static RecompReturn ActorBridgeTimedTileSpell(CpuState *cpu, bool spell) {
    const uint32_t entry = spell ? 0x81f3e4u : 0x839d57u;
    const uint32_t last = spell ? 0x81f3f3u : 0x839d67u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const uint32_t fallback = cpu->PB == ((entry >> 16) ^ 0x80u)
        ? entry & 0x7fffffu : entry;
    TileSpellClock clock = {0};
    if (cpu->DB != 0x7eu || cpu->m_flag != spell ||
        (!spell && (cpu->D || cpu->Y > 0xfff4u)) ||
        (spell && (cpu->_flag_D || !SpellFieldSourceSupported(cpu) ||
            !Lufia2DecompFrameRangeQuiet(0x81f446u, 0x81f45eu))) ||
        !QuietSpanClockSupported(cpu, entry, last + 1u, spell ? 3u : 2u,
            !spell, spell ? SPELL_FIELD_CLOCK_ROOM : MENU_LEAF_CLOCK_ROOM))
        return ActorBridgeFallback(cpu, &frame, fallback);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    clock.span.cpu = cpu;
    clock.span.bank = entry & 0xff0000u;
    clock.span.timing = spell ? spell_field_pointer : cave_lower_tile;
    clock.span.count = spell
        ? sizeof spell_field_pointer / sizeof(FixedSpanOpcode)
        : sizeof cave_lower_tile / sizeof(FixedSpanOpcode);
    clock.finish = spell
        ? spell_field_byte
        : (cpu->Y > 0xf5u ? cave_lower_tile_crossed : cave_lower_tile_finish);
    clock.finish_count = spell ? 2u : 4u;
    const Lufia2Memory memory = {
        TileSpellRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = spell
        ? Lufia2SpellRecordByteA(&memory, &state)
        : Lufia2CaveReplaceLowerTileIndex(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == last);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return spell ? ReturnTimedLongCall(cpu, &frame, last)
        : ReturnTimedMenuCall(cpu, &frame, last);
}
