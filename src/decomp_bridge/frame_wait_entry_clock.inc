#include "lufia2/frame_wait.h"

typedef struct NativeFrameWait {
    uint32_t entry;
    uint32_t counter;
    bool saves_accumulator;
    bool saves_status;
} NativeFrameWait;

static const NativeFrameWait native_frame_waits[] = {
    {0x809378u, 0x80937bu, false, true},
    {0x80bf0bu, 0x80bf0bu, false, false},
    {0x8293c2u, 0x8293c6u, true, true},
    {0x83900cu, 0x83900cu, false, false},
    {0x848363u, 0x848363u, false, false},
    {0x848d4du, 0x848d4du, false, false},
    {0x84a5a8u, 0x84a5acu, true, true},
    {0x85851cu, 0x85851cu, false, false},
    {0x868b48u, 0x868b4cu, true, true},
    {0x86974fu, 0x869750u, true, false},
    {0x8eb288u, 0x8eb288u, false, false},
    {0x8eb550u, 0x8eb550u, false, false}
};

static bool NativeFrameWaitWindow(const CpuState *cpu, uint32_t pc) {
    const unsigned budget = 128u;
    if (cpu->D || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        (cpu->host_return_valid != 0u && cpu->host_return_valid != 2u) ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, pc, 0x1ffdu, true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const unsigned remaining = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (remaining * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, clock;
    snes_refresh_state_get(&phase, &clock);
    return phase < 1364u && (!clock || clock == cpu->master_cycles);
}

static FixedSpanOpcode NativeFrameWaitOpcode(const NativeFrameWait *wait,
                                             const CpuState *cpu, uint32_t pc) {
    const uint8_t accumulator = (uint8_t)(1u + !cpu->m_flag);
    FixedSpanOpcode opcode = {(uint16_t)pc, 1u, 0u, 0u};
    if (pc == wait->counter) {
        opcode.fetched = 2u;
        opcode.cycles = (uint8_t)(2u + accumulator);
        opcode.data_bytes = accumulator;
    } else if (pc < wait->counter) {
        const uint32_t status = wait->entry + wait->saves_accumulator;
        if (wait->saves_accumulator && pc == wait->entry) {
            opcode.cycles = (uint8_t)(2u + accumulator);
            opcode.data_bytes = accumulator;
        } else if (wait->saves_status && pc == status) {
            opcode.cycles = 3u;
            opcode.data_bytes = 1u;
        } else if (wait->saves_status && pc == status + 1u) {
            opcode.fetched = 2u;
            opcode.cycles = 3u;
        }
    }
    return opcode;
}

static RecompReturn ActorBridgeNativeFrameWait(CpuState *cpu, unsigned index) {
    const NativeFrameWait *wait = native_frame_waits + index;
    const ActorBridgeFrame initial = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    if (cpu->S < 0x1f20u || !NativeFrameWaitWindow(cpu, wait->entry))
        return ActorBridgeFallback(cpu, &initial, wait->entry);
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    uint32_t pc = wait->entry;
    for (unsigned steps = 0u; steps < 4u && pc <= wait->counter; ++steps) {
        if (!NativeFrameWaitWindow(cpu, pc))
            break;
        const FixedSpanOpcode opcode = NativeFrameWaitOpcode(wait, cpu, pc);
        assert(opcode.cycles);
        FixedSpanClock clock = {0};
        clock.cpu = cpu;
        clock.bank = pc & 0xff0000u;
        clock.timing = &opcode;
        clock.count = 1u;
        const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock, NULL, NULL};
        Lufia2CpuState state;
        ActorBridgeLoad(cpu, &state);
        const Lufia2ExecutionResult result = pc < wait->counter
            ? Lufia2FrameWaitRegisterStep(&memory, &state, pc)
            : Lufia2FrameWaitAdditionalStep(&memory, &state, pc);
        ActorBridgeStore(cpu, &state);
        BeginFixedSpanOpcode(&clock);
        assert(clock.index == 1u && !clock.remaining &&
            result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc > pc);
        pc = result.pc;
    }
    return interp_tier_dispatch_tail(cpu, pc, pc, frame.entry_s, frame.hrv);
}
