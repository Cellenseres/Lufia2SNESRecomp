#include "lufia2/nmi_calls.h"
#include "decomp_bridge/nmi_resume.h"

static void FinishInterruptOpcode(FixedSpanClock *clock) {
    const FixedSpanOpcode *timing = clock->timing + clock->index;
    const unsigned internal = timing->cycles - timing->fetched - timing->data_bytes;
    Lufia2DecompNmiAccountCycles(clock->cpu, timing->cycles,
        clock->fetch_master + clock->data_master + internal * 6u);
    ++clock->index;
    clock->fetch_master = clock->data_master = 0u;
}

static void BeginInterruptOpcode(FixedSpanClock *clock) {
    while (!clock->remaining && clock->index < clock->count) {
        const FixedSpanOpcode *timing = clock->timing + clock->index;
        for (unsigned byte = 0u; byte < timing->fetched; ++byte) {
            const uint32_t address = clock->bank | (uint16_t)(timing->pc + byte);
            (void)Lufia2DecompNmiReadByte(clock->cpu, address);
            clock->fetch_master += cpu_region_speed(address);
        }
        clock->remaining = timing->data_bytes;
        if (!clock->remaining)
            FinishInterruptOpcode(clock);
    }
}

static uint8_t InterruptEntryRead(void *context, uint32_t address) {
    FixedSpanClock *clock = context;
    BeginInterruptOpcode(clock);
    assert(clock->remaining);
    const uint8_t value = Lufia2DecompNmiReadByte(clock->cpu, address);
    clock->data_master += cpu_region_speed(address);
    if (!--clock->remaining)
        FinishInterruptOpcode(clock);
    return value;
}

static void InterruptEntryWrite(void *context, uint32_t address, uint8_t value) {
    FixedSpanClock *clock = context;
    BeginInterruptOpcode(clock);
    assert(clock->remaining);
    Lufia2DecompNmiWriteByte(clock->cpu, address, value);
    clock->data_master += cpu_region_speed(address);
    if (!--clock->remaining)
        FinishInterruptOpcode(clock);
}

static bool InterruptEntryWindow(const CpuState *cpu, uint16_t address) {
    if (!cpu_interrupt_context_active() || cpu->emulation ||
        (cpu->PB != 0u && cpu->PB != 0x80u) ||
        cpu->m_flag > 1u || cpu->x_flag > 1u ||
        (cpu->x_flag && (cpu->X > 0xffu || cpu->Y > 0xffu)) ||
        !g_snes || !g_snes->dma || !cpu->ram || g_snes->ram != cpu->ram ||
        !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x86c1u - 0x8000u ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        g_snes->dma->hdmaPendingInit ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->hPos > 1000u || cpu->master_cycles > UINT64_MAX - 256u)
        return false;
    if (address == 0x862du) {
        if (!g_snes->inNmi || cpu->S < 0x1f08u || cpu->S > 0x1ffbu)
            return false;
    } else if (address == 0x86c0u) {
        if ((!g_snes->inNmi && !g_snes->inIrq) ||
            cpu->S < 0x1f00u || cpu->S > 0x1ffbu)
            return false;
    } else {
        return false;
    }
    for (unsigned i = 0u; i < 8u; ++i)
        if (g_snes->dma->channel[i].dmaActive || g_snes->dma->channel[i].hdmaActive)
            return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool RunNativeInterruptEntry(CpuState *cpu, uint16_t address) {
    if (!InterruptEntryWindow(cpu, address))
        return false;
    static const FixedSpanOpcode save_frame[] = {
        {0x862du, 1u, 3u, 1u}, {0x862eu, 1u, 3u, 1u},
        {0x862fu, 2u, 3u, 0u}, {0x8631u, 1u, 4u, 2u},
        {0x8632u, 1u, 4u, 2u}, {0x8633u, 1u, 4u, 2u},
        {0x8634u, 4u, 4u, 0u}
    };
    static const FixedSpanOpcode return_frame[] = {{0x86c0u, 1u, 7u, 4u}};
    FixedSpanClock clock = {0};
    clock.cpu = cpu;
    clock.bank = (uint32_t)cpu->PB << 16;
    clock.timing = address == 0x862du ? save_frame : return_frame;
    clock.count = address == 0x862du ? sizeof save_frame / sizeof save_frame[0] : 1u;
    const Lufia2Memory memory = {InterruptEntryRead, InterruptEntryWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = address == 0x862du
        ? Lufia2NmiSaveInterruptFrame(&memory, &state)
        : Lufia2NmiReturnEmptyInterrupt(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY);
    assert(address != 0x862du || result.pc == 0x808638u);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    BeginInterruptOpcode(&clock);
    assert(clock.index == clock.count && !clock.remaining);
    return true;
}

static RecompReturn ActorBridgeInterruptEntry(CpuState *cpu, uint16_t address) {
    const bool owns_interrupt = !cpu_interrupt_context_active();
    if (owns_interrupt)
        cpu_interrupt_context_enter();
    RecompReturn result;
    if (RunNativeInterruptEntry(cpu, address)) {
        result = address == 0x862du
            ? interp_tier_dispatch_interrupt(cpu, 0x808638u)
            : RECOMP_RETURN_NORMAL;
    } else {
        result = interp_tier_dispatch_interrupt(
            cpu, ((uint32_t)cpu->PB << 16) | address);
    }
    if (owns_interrupt)
        cpu_interrupt_context_leave();
    return result;
}
