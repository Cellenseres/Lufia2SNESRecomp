#include "lufia2/item_record_steps.h"

static const FixedSpanOpcode item_read_begin[] = {
    {0xf194u, 1u, 3u, 1u},
    {0xf195u, 2u, 3u, 0u},
    {0xf197u, 2u, 3u, 0u},
};

static const FixedSpanOpcode item_read_finish[] = {
    {0xf19du, 3u, 5u, 2u},
    {0xf1a0u, 1u, 3u, 1u},
    {0xf1a1u, 2u, 2u, 0u},
    {0xf1a3u, 1u, 3u, 1u},
    {0xf1a4u, 1u, 4u, 1u},
    {0xf1a5u, 3u, 5u, 1u},
    {0xf1a8u, 1u, 4u, 1u},
    {0xf1a9u, 1u, 4u, 1u},
};

static bool ItemReadClockSupported(const CpuState *cpu, uint32_t entry) {
    if ((entry != 0x81f194u && entry != 0x81f19du) || cpu->S < 0x1f10u)
        return false;
    const bool begin = entry == 0x81f194u;
    const uint8_t bank = cpu->DB & 0x7fu;
    if (cpu->D || cpu->_flag_D || cpu->x_flag ||
        cpu->m_flag != (begin ? 0u : 1u) ||
        (bank >= 0x40u && cpu->DB != 0x7eu))
        return false;
    if (!begin && (!cpu->ram ||
        !(cpu->ram[0x0a0au] & 0x80u)))
        return false;
    const unsigned fetch = cpu_region_speed(entry);
    const unsigned master = begin ? 5u * fetch + 26u
        : 13u * fetch + cpu_region_speed(0x968000u) + 110u;
    const unsigned budget = master + 40u;
    if (!g_snes || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= 1364u || (refresh_clock && refresh_clock != cpu->master_cycles))
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u ? 1024u : 1364u;
    return budget < fence && ActorBridgeTimedStepSupportedWindow(
        cpu, entry, 0x1ffcu, true, fence - budget - 1u, budget);
}

static void ActorBridgeItemRecordStep(CpuState *cpu, uint32_t entry) {
    if (!ItemReadClockSupported(cpu, entry))
        return;
    const bool begin = entry == 0x81f194u;
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = entry & 0xff0000u;
    clock.timing = begin ? item_read_begin : item_read_finish;
    clock.count = begin ? 3u : 8u;
    const Lufia2Memory memory = {
        FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = begin
        ? Lufia2ItemBeginRecordRead(&memory, &state)
        : Lufia2ItemFinishRecordRead(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    BeginFixedSpanOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(clock.count, clock.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
