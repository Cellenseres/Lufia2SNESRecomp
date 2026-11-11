#include "lufia2/battle_queue_steps.h"

enum { BATTLE_QUEUE_CLOCK_BUDGET = 250u, BATTLE_QUEUE_LENGTH = 0x1a8fu };

static void ActorBridgeBattleQueueStep(CpuState *cpu, uint32_t entry);

static bool BattleQueueHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return hook == ActorBridgeBattleQueueStep &&
        (pc == 0x85ecdbu || pc == 0x85ecdeu);
}

typedef struct BattleQueueClock {
    FixedSpanClock span;
    FixedSpanOpcode *timing;
    unsigned first_row_opcode;
    unsigned row;
    unsigned bytes_read;
    uint16_t length;
    bool known_free;
} BattleQueueClock;

static uint8_t BattleQueueClockRead(void *context, uint32_t address) {
    BattleQueueClock *clock = context;
    const uint8_t value = FixedSpanRead(&clock->span, address);
    assert(clock->bytes_read < 2u);
    clock->length |= (uint16_t)((uint16_t)value << (8u * clock->bytes_read));
    if (++clock->bytes_read == 2u) {
        if (clock->known_free) {
            assert(!clock->length);
            return value;
        }
        const unsigned branch = clock->first_row_opcode + 1u;
        if (!clock->length) {
            clock->timing[branch].cycles = 3u;
            clock->span.count = branch + 1u;
        } else if (clock->row == 0x5au) {
            clock->timing[clock->span.count - 1u].cycles = 2u;
        }
    }
    return value;
}

static bool BattleQueueClockSupported(const CpuState *cpu, uint32_t entry,
    bool *known_free) {
    *known_free = false;
    if ((entry != 0x85ecdbu && entry != 0x85ecdeu) ||
        cpu->m_flag || cpu->x_flag ||
        (cpu->DB != 0x97u && cpu->DB != 0x7eu) || cpu->D || cpu->_flag_D ||
        (entry == 0x85ecdeu && (cpu->Y > 0x5au || cpu->Y % 6u)) ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 3u) ||
        cpu->S < 0x1f10u || !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - BATTLE_QUEUE_CLOCK_BUDGET ||
        !cpu->ram || !g_snes || !g_snes->dma || g_snes->ram != cpu->ram)
        return false;
    const unsigned row = entry == 0x85ecdbu ? 0u : cpu->Y;
    *known_free = !g_snes->dma->hdmaPendingInit &&
        !(cpu->ram[BATTLE_QUEUE_LENGTH + row] |
          cpu->ram[BATTLE_QUEUE_LENGTH + row + 1u]);
    /* Two WRAM bytes and a taken branch. */
    const unsigned budget = *known_free
        ? (entry == 0x85ecdbu ? 8u : 5u) * cpu_region_speed(entry) + 22u + 80u
        : BATTLE_QUEUE_CLOCK_BUDGET;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
        ? 1024u : 1364u;
    if (!ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ffcu, true,
            fence - budget - 1u, budget))
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    return refresh_phase < 1364u &&
        (!refresh_clock || refresh_clock == cpu->master_cycles) &&
        Lufia2DecompFrameRangeAllows(entry, 0x85eceeu, BattleQueueHookOwned);
}

static void ActorBridgeBattleQueueStep(CpuState *cpu, uint32_t entry) {
    bool known_free;
    if (!BattleQueueClockSupported(cpu, entry, &known_free))
        return;
    static const FixedSpanOpcode row_timing[] = {
        {0xecdeu, 3u, 5u, 2u}, {0xece1u, 2u, 2u, 0u},
        {0xece3u, 1u, 2u, 0u}, {0xece4u, 1u, 2u, 0u},
        {0xece5u, 1u, 2u, 0u}, {0xece6u, 1u, 2u, 0u},
        {0xece7u, 1u, 2u, 0u}, {0xece8u, 1u, 2u, 0u},
        {0xece9u, 3u, 3u, 0u}, {0xececu, 2u, 3u, 0u}
    };
    const bool begin = entry == 0x85ecdbu;
    BattleQueueClock clock = {0};
    clock.first_row_opcode = begin ? 1u : 0u;
    clock.row = begin ? 0u : cpu->Y;
    FixedSpanOpcode timing[11];
    clock.known_free = known_free;
    clock.span.cpu = cpu;
    clock.span.bank = 0x850000u;
    if (known_free) {
        static const FixedSpanOpcode free_timing[] = {
            {0xecdbu, 3u, 3u, 0u}, {0xecdeu, 3u, 5u, 2u},
            {0xece1u, 2u, 3u, 0u}
        };
        clock.span.timing = free_timing + (begin ? 0u : 1u);
        clock.span.count = begin ? 3u : 2u;
    } else {
        clock.timing = timing;
        if (begin)
            timing[0] = (FixedSpanOpcode){0xecdbu, 3u, 3u, 0u};
        for (unsigned i = 0u; i < 10u; ++i)
            timing[clock.first_row_opcode + i] = row_timing[i];
        clock.span.timing = timing;
        clock.span.count = clock.first_row_opcode + 10u;
    }
    const Lufia2Memory memory = {BattleQueueClockRead, NULL, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = begin
        ? Lufia2BattleBeginVramQueueSearch(&memory, &state)
        : Lufia2BattleContinueVramQueueSearch(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
