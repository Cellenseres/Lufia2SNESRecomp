#include "sound_clock_barrier.h"
#include "../lufia2_spc_native.h"
#include "snesrecomp_platform/apu_program.h"

#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
static bool SoundClockCapture(Lufia2SoundClockBarrier *clock) {
    CpuState *cpu = clock->cpu;
    Snes *system = clock->system;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    if (!clock->enabled || clock->active || system != g_snes ||
        system->apu != clock->apu || system->beamMasterLast != cpu->master_cycles ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !charged || charged != cpu->master_cycles || phase >= 1364u ||
        cpu->master_cycles >= clock->ceiling_master ||
        !(system->apuCatchupCycles >= 0.0 && system->apuCatchupCycles < 1.0) ||
        !Lufia2SpcNativeCpuSpanReady(clock->apu) ||
        snesrecomp_apu_cpu_span_open(clock->apu))
        return false;

    /* Timeline selection reads physical mapping fields before the span opens. */
    const bool extended = rtl_apu_extended_frame_timing();
    const bool timeline = rtl_apu_frame_timeline_active();
    if ((extended && !clock->apu->portTimeValid) ||
        !snesrecomp_beam_eventless_step(system->hPos, system->vPos,
            1u, true, false, system->autoJoyRead))
        return false;
    clock->relative_credit = extended ||
        !interp_bridge_use_absolute_apu_timeline(timeline, false, false);
    clock->absolute_flush = interp_bridge_use_absolute_apu_timeline(
        timeline, false, extended && clock->apu->portTimeValid);
    clock->flush_threshold = bridge_bounce_flush_thresh();
    clock->refresh_phase = phase;
    return true;
}

static bool SoundClockRestart(Lufia2SoundClockBarrier *clock) {
    /* The admitted group touches only pure memory and the four APU ports.
     * Its outer lock and event barriers retain the captured clock context. */
    clock->published_master = clock->cpu->master_cycles;
    clock->active = snesrecomp_apu_cpu_span_begin(clock->apu, 1024u);
    return clock->active;
}

static bool SoundClockFlushApu(Lufia2SoundClockBarrier *clock) {
    if (!s_apu_pending_master)
        return false;
    RtlApuLock();
    if (!clock->absolute_flush) {
        clock->system->apuCatchupCycles +=
            (double)s_apu_pending_master * kInterpApuPerMaster;
        g_apu_last_sync_master = clock->cpu->master_cycles;
        snes_catchupApu(clock->system);
    } else {
        g_apu_last_sync_master = clock->cpu->master_cycles;
    }
    rtl_sync_apu_to_cpu_locked();
    RtlApuUnlock();
    s_apu_pending_master = 0u;
    return true;
}
#endif

void Lufia2DecompSoundClockBarrier(Lufia2SoundClockBarrier *clock) {
    if (!clock || !clock->active)
        return;
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    CpuState *cpu = clock->cpu;
    snes_refresh_state_set(clock->refresh_phase, cpu->master_cycles);
    cpu->coprocessor_master_cycles = cpu->master_cycles;
    snes_sync_master_clock(clock->system, cpu->master_cycles);
    cart_sync_coprocessors(clock->system->cart, cpu->master_cycles);
    snesrecomp_apu_cpu_span_end(clock->apu);
#endif
    clock->active = false;
}

void Lufia2DecompSoundClockEnd(Lufia2SoundClockBarrier *clock) {
    if (!clock)
        return;
    Lufia2DecompSoundClockBarrier(clock);
    clock->enabled = false;
    if (clock->locked) {
        clock->locked = false;
        RtlApuUnlock();
    }
}

bool Lufia2DecompSoundClockBegin(CpuState *cpu,
    unsigned budget, Lufia2SoundClockBarrier *clock) {
    if (!clock)
        return false;
    *clock = (Lufia2SoundClockBarrier){.cpu = cpu};
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS) && \
    defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY) && !defined(SNES_COSIM) && \
    !defined(SNESRECOMP_INTERP_PROFILE) && \
    !(defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG)
    static int no_apu = -1;
    if (no_apu < 0)
        no_apu = getenv("SNESRECOMP_INTERP_NOAPU") != NULL;
    if (no_apu || cpu != &g_cpu || !budget || budget > 768u ||
        !s_frame_return_batch_allowed || !s_frame_return_auto_quiescent ||
        s_frame_return_interrupt_blocked || s_interp_bus_timing_active ||
        g_interp_bridge_pc_hook || Lufia2NativeObserversActive() ||
        !g_snes || !g_snes->apu || !g_snes->dma ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit ||
        g_snes->inNmi || g_snes->inIrq || g_snes->hIrqEnabled ||
        g_snes->vIrqEnabled || g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        g_snes->vPos == 224u || g_snes->vPos == 261u ||
        g_snes->ram != cpu->ram || g_snes->dma->dmaBusy ||
        g_snes->dma->dmaTimer || snesrecomp_beam_hold_active() ||
        cpu->master_cycles > UINT64_MAX - budget ||
        s_apu_pending_master > UINT64_MAX - budget ||
        Lufia2DecompFrameReturnDeadline() <= cpu->master_cycles ||
        Lufia2DecompFrameReturnDeadline() - cpu->master_cycles < budget)
        return false;
    const Cart *cart = g_snes->cart;
    if (!cart || cart->type != CART_LOROM || !cart->rom || cart->superfx ||
        cart->cx4 || cart->dsp1 || cart->sdd1 || cart->sa1)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    clock->system = g_snes;
    clock->apu = g_snes->apu;
    clock->ceiling_master = cpu->master_cycles + budget;
    clock->enabled = true;
    RtlApuLock();
    clock->locked = true;
    if (SoundClockCapture(clock) && SoundClockRestart(clock))
        return true;
    Lufia2DecompSoundClockEnd(clock);
#else
    (void)budget;
#endif
    return false;
}

void Lufia2DecompSoundClockAccount(Lufia2SoundClockBarrier *clock,
    unsigned cycles, uint64_t master, bool progress) {
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (!clock->enabled && clock->cpu->master_cycles < clock->ceiling_master) {
        const unsigned remaining = (unsigned)(clock->ceiling_master - clock->cpu->master_cycles);
        (void)Lufia2DecompSoundClockBegin(clock->cpu, remaining, clock);
    }
    if (clock->enabled && !clock->active && !SoundClockRestart(clock))
        Lufia2DecompSoundClockEnd(clock);
    if (clock->active) {
        CpuState *cpu = clock->cpu;
        const uint64_t phase = clock->refresh_phase + master;
        const uint64_t refresh = phase >= 1364u ? 40u : 0u;
        const uint64_t remaining = clock->ceiling_master - cpu->master_cycles;
        if (!master || master > remaining || refresh > remaining - master ||
            !snesrecomp_beam_eventless_step(clock->system->hPos,
                clock->system->vPos,
                cpu->master_cycles - clock->published_master + master + refresh,
                true, false, clock->system->autoJoyRead)) {
            Lufia2DecompSoundClockEnd(clock);
        } else {
            cpu->cycles += cycles;
            cpu->master_cycles += master + refresh;
            clock->refresh_phase = phase >= 1364u ? phase - 1364u : phase;
            bool synchronized = false;
            if (clock->relative_credit) {
                s_apu_pending_master += master;
                if (s_apu_pending_master >= clock->flush_threshold)
                    synchronized = SoundClockFlushApu(clock);
            }
            if (progress && SoundClockFlushApu(clock))
                synchronized = true;
            if (synchronized && !snesrecomp_apu_cpu_span_open(clock->apu))
                Lufia2DecompSoundClockEnd(clock);
            return;
        }
    }
#endif
    Lufia2DecompFrameReturnAccountOpcode(clock->cpu, cycles, master, progress);
}

uint8_t Lufia2DecompSoundClockRead(Lufia2SoundClockBarrier *clock,
    uint32_t address, bool *progress) {
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (clock->enabled) {
        const uint8_t bank = (uint8_t)(address >> 16);
        const uint16_t offset = (uint16_t)address;
        const int32_t wram = cpu_wram_offset(bank, offset);
        Cart *cart = clock->system->cart;
        uint8_t value;
        if (wram >= 0) {
            value = clock->cpu->ram[wram];
        } else if ((bank & 0x7fu) < 0x50u && offset >= 0x8000u && cart->romSize) {
            const uint32_t source = ((uint32_t)(bank & 0x7fu) << 15) |
                (offset & 0x7fffu);
            value = cart->rom[source % cart->romSize];
        } else {
            return Lufia2DecompFrameReturnReadByte(clock->cpu, address, progress);
        }
        clock->cpu->open_bus = value;
        cart->cpuBusAddress = address & 0xffffffu;
        *progress = false;
        if ((bank & 0x7fu) < 0x40u && offset < 0x2000u) {
            /* Low WRAM mirrors participate in the interpreter's wait ledger. */
            ++s_interp_continuous_read_epoch;
            BridgeDynamicValue *dynamic =
                &s_bridge_dynamic_values[(address ^ (address >> 8) ^ (address >> 16)) & 63u];
            if (!dynamic->valid || dynamic->address != address || dynamic->value != value) {
                dynamic->valid = 1;
                dynamic->address = address;
                dynamic->value = value;
                ++s_interp_dynamic_progress_epoch;
                *progress = true;
            }
        }
        return value;
    }
#endif
    return Lufia2DecompFrameReturnReadByte(clock->cpu, address, progress);
}

void Lufia2DecompSoundClockWrite(Lufia2SoundClockBarrier *clock,
    uint32_t address, uint8_t value, bool *progress) {
#if defined(LUFIA2_HAVE_SOUND_CLOCK_BARRIERS)
    if (clock->enabled && cpu_wram_offset((uint8_t)(address >> 16), (uint16_t)address) >= 0) {
        ++g_interp_bridge_write_epoch;
        /* Keep the CPU's write observers and title hooks on the original path. */
        cpu_write8(clock->cpu, (uint8_t)(address >> 16), (uint16_t)address, value);
        *progress = true;
        return;
    }
#endif
    Lufia2DecompFrameReturnWriteByte(clock->cpu, address, value, progress);
}
