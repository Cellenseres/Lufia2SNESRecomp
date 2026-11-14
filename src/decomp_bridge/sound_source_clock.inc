#include "lufia2/sound_source.h"

static const struct {
    uint32_t first;
    uint16_t second;
    uint32_t finish;
} sound_source_pairs[] = {
    {0x809953u, 0x995eu, 0x809969u},
    {0x809815u, 0x9823u, 0x809831u},
    {0x809848u, 0x9856u, 0x809864u}
};

static void ActorBridgeSoundSourcePair(CpuState *cpu, uint32_t entry);

static bool SoundSourceHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    if (hook != ActorBridgeSoundSourcePair)
        return false;
    for (unsigned i = 0u; i < sizeof sound_source_pairs / sizeof *sound_source_pairs; ++i)
        if (pc == sound_source_pairs[i].first)
            return true;
    return false;
}

static bool SoundSourceMemory(uint8_t bank, uint16_t source) {
    if (bank == 0x7eu || bank == 0x7fu)
        return true;
    if ((bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)) && source < 0x1fffu)
        return true;
    return (bank & 0x7fu) < 0x50u && source >= 0x8000u;
}

static void ActorBridgeSoundSourcePair(CpuState *cpu, uint32_t entry) {
    unsigned operation = 0u;
    while (operation < sizeof sound_source_pairs / sizeof *sound_source_pairs &&
           sound_source_pairs[operation].first != entry)
        ++operation;
    if (operation == sizeof sound_source_pairs / sizeof *sound_source_pairs ||
        cpu != &g_cpu || cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        cpu->Y > 0xfffdu || !SoundSourceMemory(cpu->DB, cpu->Y) ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - 256u ||
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1fffu, 256u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x1969u ||
        Lufia2DecompFrameOwnedSpanBudget(entry, entry + 1u,
            ActorBridgeSoundSourcePair) < 8u ||
        !Lufia2DecompFrameRangeAllows(entry, sound_source_pairs[operation].finish,
            SoundSourceHookOwned))
        return;
    const uint16_t first = (uint16_t)entry;
    const uint16_t second = sound_source_pairs[operation].second;
    const FixedSpanOpcode timing[] = {
        {first, 3u, (uint8_t)(4u + (cpu->Y >= 0x100u)), 1u},
        {first + 3u, 2u, 3u, 1u}, {first + 5u, 1u, 2u, 0u},
        {first + 6u, 2u, 3u, 0u},
        {second, 3u, (uint8_t)(4u + (cpu->Y + 1u >= 0x100u)), 1u},
        {second + 3u, 2u, 3u, 1u}, {second + 5u, 1u, 2u, 0u},
        {second + 6u, 2u, 3u, 0u}
    };
    FixedSpanClock clock = {.cpu = cpu, .bank = 0x800000u,
        .timing = timing, .count = 8u};
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2SoundSourcePair pair = Lufia2SoundReadSourcePair(
        &memory, state.data_bank, state.direct_page, state.y);
    BeginFixedSpanOpcode(&clock);
    assert(pair.complete && clock.index == 8u && !clock.remaining);
    state.accumulator = (uint16_t)((state.accumulator & 0xff00u) | pair.last_byte);
    state.y = pair.next;
    state.negative = (state.y & 0x8000u) != 0u;
    state.zero = 0u;
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(8u, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(sound_source_pairs[operation].finish);
}

static void ActorBridgeInstallSoundSourceHooks(void) {
    for (unsigned i = 0u; i < sizeof sound_source_pairs / sizeof *sound_source_pairs; ++i) {
        const uint32_t pc = sound_source_pairs[i].first;
        if (Lufia2DecompFrameRangeAllows(pc, pc + 1u, SoundSourceHookOwned))
            interp_bridge_set_pre_opcode_hook(pc, ActorBridgeSoundSourcePair);
    }
}
