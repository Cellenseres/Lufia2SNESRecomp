enum { POST_RETURN_CYCLE_LIMIT = 59u };

_Static_assert(POST_RETURN_CYCLE_LIMIT * 8u + 40u <= MENU_LEAF_CLOCK_ROOM,
    "Post-return leaf clock reservation");

static const FixedSpanOpcode save_selection_backup[] = {
    {0xf46cu, 3u, 5u, 2u}, {0xf46fu, 1u, 2u, 0u},
    {0xf470u, 1u, 2u, 0u}, {0xf471u, 1u, 2u, 0u},
    {0xf472u, 1u, 2u, 0u}, {0xf473u, 1u, 2u, 0u}
};
static const FixedSpanOpcode backup_column[] = {
    {0xf474u, 3u, 4u, 1u}, {0xf477u, 3u, 4u, 1u}
};
static const FixedSpanOpcode backup_column_crossed[] = {
    {0xf474u, 3u, 5u, 1u}, {0xf477u, 3u, 4u, 1u}
};
static const FixedSpanOpcode backup_row[] = {
    {0xf47au, 3u, 4u, 1u}, {0xf47du, 3u, 4u, 1u}
};
static const FixedSpanOpcode backup_row_crossed[] = {
    {0xf47au, 3u, 5u, 1u}, {0xf47du, 3u, 4u, 1u}
};

typedef struct PostReturnLeafClock {
    FixedSpanClock span;
    uint16_t selection_index;
} PostReturnLeafClock;

static uint8_t BackupSelectionRead(void *context, uint32_t address) {
    PostReturnLeafClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (clock->span.timing == save_selection_backup)
        clock->span.timing = clock->selection_index > 0x26u
            ? backup_column_crossed : backup_column;
    else
        clock->span.timing = clock->selection_index > 0x14u
            ? backup_row_crossed : backup_row;
    clock->span.index = 0u;
    clock->span.count = 2u;
    return FixedSpanRead(&clock->span, address);
}

static const FixedSpanOpcode clear_base_stats[] = {
    {0xf6bcu, 2u, 4u, 2u}, {0xf6beu, 3u, 7u, 2u},
    {0xf6c1u, 3u, 7u, 2u}, {0xf6c4u, 3u, 7u, 2u},
    {0xf6c7u, 3u, 7u, 2u}, {0xf6cau, 3u, 7u, 2u},
    {0xf6cdu, 3u, 7u, 2u}, {0xf6d0u, 3u, 7u, 2u}
};
static const FixedSpanOpcode scratch_byte_match[] = {
    {0xeb5fu, 2u, 3u, 1u}, {0xeb61u, 3u, 3u, 0u},
    {0xeb64u, 2u, 3u, 1u}, {0xeb66u, 2u, 3u, 0u},
    {0xeb8du, 1u, 2u, 0u}
};

typedef struct PostReturnLeaf {
    uint32_t entry;
    uint32_t last;
    const FixedSpanOpcode *timing;
    unsigned count;
    Lufia2ExecutionResult (*run)(const Lufia2Memory *, Lufia2CpuState *);
    uint8_t frame_size;
    bool wide_accumulator;
} PostReturnLeaf;

static RecompReturn ActorBridgeTimedPostReturnLeaf(CpuState *cpu, unsigned kind) {
    static const PostReturnLeaf leaves[] = {
        {0x82f46cu, 0x82f480u, save_selection_backup,
            sizeof save_selection_backup / sizeof(FixedSpanOpcode),
            Lufia2SaveBackupMenuSelection, 2u, false},
        {0x82f6bcu, 0x82f6d3u, clear_base_stats,
            sizeof clear_base_stats / sizeof(FixedSpanOpcode),
            Lufia2PartyClearBaseStats, 2u, true},
        {0x85eb5fu, 0x85eb8eu, scratch_byte_match,
            sizeof scratch_byte_match / sizeof(FixedSpanOpcode),
            Lufia2BattleCheckScratchByte, 3u, false}
    };
    assert(kind < sizeof leaves / sizeof(PostReturnLeaf));
    const PostReturnLeaf *leaf = leaves + kind;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    const uint32_t fallback = cpu->PB == ((leaf->entry >> 16) ^ 0x80u)
        ? leaf->entry & 0x7fffffu : leaf->entry;
    if (cpu->DB != 0x7eu || (kind && cpu->D) ||
        cpu->m_flag != !leaf->wide_accumulator ||
        !QuietLeafClockSupported(cpu, leaf->entry,
            kind == 2u ? 0x85eb91u : leaf->last + 1u,
            leaf->frame_size, leaf->wide_accumulator))
        return ActorBridgeFallback(cpu, &frame, fallback);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, leaf->entry);
    PostReturnLeafClock clock = {0};
    clock.selection_index = (uint16_t)(cpu->X - 5u);
    clock.span.cpu = cpu;
    clock.span.bank = leaf->entry & 0xff0000u;
    clock.span.timing = leaf->timing;
    clock.span.count = leaf->count;
    const Lufia2Memory memory = {
        kind == 0u ? BackupSelectionRead : FixedSpanRead,
        FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = leaf->run(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == leaf->last);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return leaf->frame_size == 3u
        ? ReturnTimedLongCall(cpu, &frame, result.pc)
        : ReturnTimedMenuCall(cpu, &frame, result.pc);
}
