#include "decomp_bridge/frame_returns.h"

static unsigned s_frame_return_opcodes;
static unsigned s_frame_return_progress_mask;
static unsigned s_frame_return_last_progress;

bool Lufia2DecompFrameReturnCpuReady(void) {
    return !s_frame_return_interrupt_blocked;
}

bool Lufia2DecompFrameRangeQuiet(uint32_t first, uint32_t end) {
    if (g_interp_bridge_pc_hook || Lufia2NativeObserversActive())
        return false;
    first &= 0x7fffffu;
    end &= 0x7fffffu;
    if (first >= end || end - first > 256u ||
        (first >> 16) != ((end - 1u) >> 16))
        return false;
    for (unsigned page = first >> 8; page <= ((end - 1u) >> 8); ++page) {
        for (unsigned link = s_pre_opcode_hook_heads[page]; link;
             link = s_pre_opcode_hook_next[link - 1u]) {
            const uint32_t pc = s_pre_opcode_hooks[link - 1u].pc24;
            if (pc >= first && pc < end)
                return false;
        }
    }
    return true;
}

void Lufia2DecompFrameReturnWriteByte(CpuState *cpu, uint32_t address, uint8_t value,
                                      bool *progress) {
    const uint64_t before = g_interp_bridge_write_epoch;
    bridge_bus_write(cpu, address, value);
    *progress = before != g_interp_bridge_write_epoch;
}

uint8_t Lufia2DecompFrameReturnReadByte(CpuState *cpu, uint32_t address,
                                        bool *progress) {
    const uint64_t before = s_interp_dynamic_progress_epoch;
    const uint8_t value = bridge_bus_read(cpu, address);
    *progress = before != s_interp_dynamic_progress_epoch;
    return value;
}

uint16_t Lufia2DecompFrameReturnReadWord(CpuState *cpu, uint32_t low, uint32_t high,
                                         bool *progress) {
    const uint64_t before = s_interp_dynamic_progress_epoch;
    uint16_t value;
    if (!bridge_bus_read_word(cpu, low, high, &value)) {
        value = bridge_bus_read(cpu, low);
        value |= (uint16_t)bridge_bus_read(cpu, high) << 8;
    }
    *progress = before != s_interp_dynamic_progress_epoch;
    return value;
}

void Lufia2DecompFrameReturnWriteWord(CpuState *cpu, uint32_t low, uint32_t high,
                                      uint16_t value, bool *progress) {
    const uint64_t before = g_interp_bridge_write_epoch;
    if (!bridge_bus_write_word(cpu, low, high, value, false)) {
        bridge_bus_write(cpu, low, (uint8_t)value);
        bridge_bus_write(cpu, high, (uint8_t)(value >> 8));
    }
    *progress = before != g_interp_bridge_write_epoch;
}

void Lufia2DecompFrameReturnAccountOpcode(CpuState *cpu, unsigned cycles,
                                          uint64_t master, bool progress) {
    Lufia2DecompNmiAccountCycles(cpu, cycles, master);
    if (progress && s_frame_return_auto_quiescent)
        bridge_apu_flush(cpu);
}

void Lufia2DecompFrameReturnComplete(unsigned count, unsigned progress_mask) {
    s_frame_return_opcodes = count;
    s_frame_return_progress_mask = progress_mask;
    s_frame_return_last_progress = 0u;
}

void Lufia2DecompFrameReturnCompleteSpan(unsigned count, unsigned last_progress) {
    s_frame_return_opcodes = count;
    s_frame_return_progress_mask = 0u;
    s_frame_return_last_progress = last_progress;
}

unsigned Lufia2DecompFrameSpanBudget(uint32_t first, uint32_t end,
    void (*owner)(CpuState *, uint32_t)) {
    if (!s_frame_return_batch_allowed || s_interp_bus_timing_active ||
        !owner || !s_frame_return_poll_opcode_budget ||
        s_frame_return_poll_opcode_budget > LUFIA2_FRAME_POLL_MAX_OPCODES ||
        Lufia2NativeObserversActive() || s_interp_continuous_read_epoch >
            UINT64_MAX - 8u * s_frame_return_poll_opcode_budget)
        return 0u;
    first &= 0x7fffffu;
    end &= 0x7fffffu;
    if (first >= end || end - first > 256u)
        return 0u;
    bool found = false;
    for (unsigned page = first >> 8; page <= ((end - 1u) >> 8); ++page) {
        for (unsigned link = s_pre_opcode_hook_heads[page]; link;
             link = s_pre_opcode_hook_next[link - 1u]) {
            const unsigned slot = link - 1u;
            const uint32_t pc = s_pre_opcode_hooks[slot].pc24;
            if (pc < first || pc >= end)
                continue;
            if (pc != first || s_pre_opcode_hooks[slot].hook != owner)
                return 0u;
            found = true;
        }
    }
    return found ? s_frame_return_poll_opcode_budget : 0u;
}

static void Lufia2DecompFrameReturnContinue(long *steps, int auto_quiescent,
                                            uint64_t *write_epoch,
                                            uint64_t *dynamic_epoch) {
    if (!s_frame_return_opcodes)
        return;
    *steps += (long)s_frame_return_opcodes - 1;
    if (auto_quiescent) {
        if (s_frame_return_last_progress)
            *steps = (long)(s_frame_return_opcodes - s_frame_return_last_progress);
        for (unsigned opcode = 0, mask = s_frame_return_progress_mask;
             opcode < s_frame_return_opcodes && mask; ++opcode, mask >>= 1u)
            if (mask & 1u)
                *steps = (long)(s_frame_return_opcodes - opcode - 1u);
        *write_epoch = g_interp_bridge_write_epoch;
        *dynamic_epoch = s_interp_dynamic_progress_epoch;
    }
    s_frame_return_opcodes = 0u;
    s_frame_return_progress_mask = 0u;
    s_frame_return_last_progress = 0u;
}

/* No opcode hook/observer may be bypassed by a native pair. Every CMP
 * advances the continuous-read epoch, so omitted intermediate ring entries
 * cannot match later states. Admission excludes the epoch-wrap boundary. */
bool Lufia2DecompFramePollBatchSupported(uint32_t entry) {
    if (!s_frame_return_batch_allowed || s_interp_bus_timing_active ||
        s_interp_continuous_read_epoch > UINT64_MAX - LUFIA2_FRAME_POLL_MAX_OPCODES ||
        Lufia2NativeObserversActive()) return false;
    /* The setter replaces an existing normalized PC in place. Cached slots
     * remain valid only while BOTH current keys and callbacks still match;
     * clearing/rebuilding the table is therefore safe without live pointers. */
    typedef struct PairSlots { uint32_t key; int slot[2]; bool valid; } PairSlots;
    static PairSlots cache[4];
    const uint32_t normalized = entry & 0x7fffffu;
    PairSlots *const saved = cache + ((normalized ^ (normalized >> 8)) & 3u);
    if (saved->valid && saved->key == normalized) {
        bool current = true;
        for (unsigned part = 0; part < 2u; ++part) {
            const int slot = saved->slot[part];
            if (slot < 0 || slot >= s_pre_opcode_hook_count ||
                s_pre_opcode_hooks[slot].pc24 != ((normalized + part * 2u) & 0x7fffffu)) {
                current = false; break;
            }
        }
        if (current)
            return Lufia2DecompFramePollHookIsNative(s_pre_opcode_hooks[saved->slot[0]].hook) &&
                Lufia2DecompFramePollHookIsNative(s_pre_opcode_hooks[saved->slot[1]].hook);
    }
    saved->valid = false;
    saved->key = normalized;
    for (unsigned part = 0; part < 2u; ++part) {
        const uint32_t key = (entry + part * 2u) & 0x7fffffu;
        bool native = false;
        for (int slot = (int)s_pre_opcode_hook_heads[key >> 8] - 1; slot >= 0;
             slot = (int)s_pre_opcode_hook_next[slot] - 1) {
            if (s_pre_opcode_hooks[slot].pc24 == key) {
                native = Lufia2DecompFramePollHookIsNative(s_pre_opcode_hooks[slot].hook);
                saved->slot[part] = slot;
                break;
            }
        }
        if (!native) return false;
    }
    saved->valid = true;
    return true;
}

/* Only an observer-free native poll may fold its pure LoROM opcode bytes.
 * There is no intervening clock or device work;
 * Fetched bytes are discarded; the last supplies the final bus latches.
 * Data reads and every opcode's clock accounting stay on the original path.
 * No pointer is retained past this call, and mapper/allocation/timing refusal
 * leaves every latch untouched for the ordinary fetch callbacks. */
bool Lufia2DecompPollFetchOpcode(CpuState *cpu, uint32_t pc, uint32_t fetched) {
    if (fetched < 1u || fetched > 4u || (pc & 0xff000000u) ||
        !s_frame_return_batch_allowed || s_interp_bus_timing_active ||
        Lufia2NativeObserversActive() || !g_snes ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        (pc & 0xffffu) < 0x8000u || (pc & 0xffffu) > 0x10000u - fetched ||
        ((pc >> 16) & 0x7fu) >= 0x40u)
        return false;
    Cart *cart = g_snes->cart;
    const uint32_t offset = (((pc >> 16) & 0x7fu) << 15) | (pc & 0x7fffu);
    if (!cart->rom || offset >= cart->romSize || cart->romSize - offset < fetched)
        return false;
    cpu->open_bus = cart->rom[offset + fetched - 1u];
    cart->cpuBusAddress = (pc + fetched - 1u) & 0xffffffu;
    return true;
}

#include "snesrecomp_platform/beam_clock.h"
static uint64_t s_frame_return_idle_opcodes;
uint64_t Lufia2DecompNativeIdleSteps(void) { return s_frame_return_idle_opcodes; }

#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
static uint64_t s_native_idle_diagnostics[16], s_native_idle_contexts[16];
static bool s_native_idle_reference_clock;
#endif
uint32_t Lufia2DecompFramePollReferenceClock(uint32_t enabled) {
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
    s_native_idle_reference_clock = enabled != 0u;
    return 1u;
#else
    (void)enabled;
    return 0u;
#endif
}
static bool Lufia2NativeIdleDenied(unsigned reason) {
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
    ++s_native_idle_diagnostics[reason];
#else
    (void)reason;
#endif
    return false;
}
static void Lufia2NativeIdleAccepted(unsigned reason) {
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
    ++s_native_idle_diagnostics[reason];
#else
    (void)reason;
#endif
}
uint32_t Lufia2DecompNativeIdleDiagnostics(uint64_t out[16],uint64_t contexts[16]) {
    if (!out || !contexts) return 0;
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
    memcpy(out, s_native_idle_diagnostics, sizeof s_native_idle_diagnostics);
    memcpy(contexts, s_native_idle_contexts, sizeof s_native_idle_contexts);
    return 1;
#else
    memset(out, 0, 16u * sizeof *out);
    memset(contexts, 0, 16u * sizeof *contexts);
    return 0;
#endif
}
/* Only classifies a rejected top-level gate; it never advances guest state. */
static unsigned Lufia2NativeIdleContextReason(const CpuState *cpu,uint32_t entry) {
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS) && defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY)
    if (cpu != &g_cpu) return 0;
    if (!Lufia2DecompFramePollBatchSupported(entry)) return 1;
    if (!cpu->m_flag || cpu->x_flag > 1u || cpu->emulation || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1fffu || cpu->PB != entry >> 16 || !cpu->ram) return 0;
    if (!g_snes || g_snes->ram != cpu->ram || !g_snes->apu || !g_snes->dma) return 2;
    if (g_snes->inNmi || g_snes->inIrq) return 3;
    if (g_snes->hIrqEnabled || g_snes->vIrqEnabled) return 4;
    if (!g_snes->hdmaBeamOff || g_snes->beamMasterLast != cpu->master_cycles) return 5;
    if (g_snes->hPos >= 1364u || g_snes->vPos >= 262u) return 6;
    if (snesrecomp_beam_hold_active()) return 7;
    if (!g_snes->cart) return 8;
    return 14;
#else
    (void)cpu; (void)entry;
#endif
    return 0;
}


/* Diagnostic wrapper calls this only with DetailProfile enabled. */
void Lufia2DecompFramePollProbe(const CpuState *cpu,uint32_t entry) {
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
    if (Lufia2DecompFramePollIsCounterPair(entry))
        ++s_native_idle_contexts[Lufia2NativeIdleContextReason(cpu,entry)];
#else
    (void)cpu; (void)entry;
#endif
}

/* Fold only equal NMI-counter polls in a provably eventless interval. The
 * reference path retains every event edge, refresh stall and APU flush.
 * No sound-port read or handshake is eligible. The original step loop owns
 * the final deadline crossing; a span uses at most 128 remaining opcode slots. */
/* At most 1024 opcode slots, with at most 46 master clocks per pair.
 * Reject an out-of-span 64-bit distance before narrowing its division. */
static unsigned Lufia2DecompFramePollEdgeChunk(uint64_t need,
    uint32_t pair_master, uint32_t first_master, unsigned chunk) {
    const uint32_t limit = (chunk / 2u + 1u) * pair_master;
    if (need > limit) return chunk;
    const uint32_t pairs = need ? ((uint32_t)need - 1u) / pair_master : 0u;
    const uint32_t tail = (uint32_t)need - pairs * pair_master;
    const unsigned boundary = pairs * 2u + (tail <= first_master ? 1u : 2u);
    return boundary < chunk ? boundary : chunk;
}

static bool FramePollChannelsIdle(const Snes *snes) {
    const Dma *dma = snes->dma;
    if (!snes->hdmaBeamOff && dma->hdmaPendingInit)
        return false;
    for (unsigned index = 0u; index < 8u; ++index) {
        const DmaChannel *channel = dma->channel + index;
        if (channel->dmaActive || (!snes->hdmaBeamOff && channel->hdmaActive))
            return false;
    }
    return true;
}

static bool Lufia2DecompFramePollClockedEdges(CpuState *cpu, uint32_t entry) {
#if defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY) && !defined(SNES_COSIM) && \
    !defined(SNESRECOMP_INTERP_PROFILE) && \
    (!defined(SNESRECOMP_REVERSE_DEBUG) || !SNESRECOMP_REVERSE_DEBUG)
    if (cpu != &g_cpu || !Lufia2DecompFramePollIsCounterPair(entry) ||
        !Lufia2DecompFramePollBatchSupported(entry) ||
        !cpu->m_flag || cpu->x_flag > 1u || cpu->emulation || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1fffu ||
        cpu->PB != entry >> 16 || !cpu->ram || !g_snes ||
        g_snes->ram != cpu->ram || !g_snes->apu || !g_snes->dma ||
        g_snes->inNmi || g_snes->inIrq || g_snes->hIrqEnabled ||
        g_snes->vIrqEnabled ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        snesrecomp_beam_hold_active() || !g_snes->cart)
        return false;
    Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || cart->superfx || cart->cx4 || cart->dsp1 ||
        cart->sdd1 || cart->sa1 || g_snes->dma->dmaBusy || g_snes->dma->dmaTimer)
        return false;
    if (!FramePollChannelsIdle(g_snes))
        return false;
    const uint32_t offset = (((entry >> 16) & 0x7fu) << 15) | (entry & 0x7fffu);
    if (!cart->rom || offset >= cart->romSize || cart->romSize - offset < 4u ||
        cart->rom[offset] != 0xc5u || cart->rom[offset + 1u] != 0x40u ||
        cart->rom[offset + 2u] != 0xf0u || cart->rom[offset + 3u] != 0xfcu ||
        (uint8_t)cpu->A != cpu->ram[0x40u])
        return false;
    const BridgeDynamicValue *known = &s_bridge_dynamic_values[0];
    if (!known->valid || known->address != 0x40u || known->value != cpu->ram[0x40u])
        return false;
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    unsigned budget = s_frame_return_poll_opcode_budget;
    if (budget > LUFIA2_FRAME_POLL_MAX_OPCODES) budget = LUFIA2_FRAME_POLL_MAX_OPCODES;
    const uint32_t reserve = (budget + 127u) & ~127u;
    _Static_assert(LUFIA2_FRAME_POLL_MAX_OPCODES <= 1024u, "review native poll clock bounds");
    if (phase >= 1364u || charged != cpu->master_cycles ||
        deadline <= cpu->master_cycles || deadline - cpu->master_cycles <= reserve)
        return false;
    const uint32_t compare_master =
        2u * cpu_region_speed(entry) + cpu_region_speed(0x40u);
    const uint32_t branch_master = 2u * cpu_region_speed(entry) + 6u;
    const uint32_t pair_master = compare_master + branch_master;
    unsigned pairs = budget / 2u;
    const uint64_t room = deadline - cpu->master_cycles - reserve;
    if ((uint64_t)pairs * pair_master > room)
        pairs = (uint32_t)room / pair_master;
    if (!pairs) return false;
    cpu->_flag_C = cpu->_flag_Z = 1u;
    cpu->_flag_N = 0u;
    cpu_mirrors_to_p(cpu);
    unsigned opcode = 0u;
    const unsigned opcodes = pairs * 2u;
    while (opcode < opcodes) {
        unsigned chunk = opcodes - opcode;
        uint64_t refresh_phase, refresh_clock;
        snes_refresh_state_get(&refresh_phase, &refresh_clock);
        /* Each span ends at the same original opcode which crosses a device
         * edge. Real refresh/beam/APU handlers execute there, never later.
         * Reserve 128 clocks per 128 opcode slots. For at most 1024 slots,
         * raw clocks plus this reserve span fewer than 20 refresh periods;
         * their maximum 800 clocks fit within the 1024-clock reserve. */
        const uint64_t edges[] = {
            1364u - refresh_phase,
            (g_snes->vPos < 225u && g_snes->hPos < 1024u ?
                1024u : 1364u) - g_snes->hPos
        };
        const uint32_t first_master = opcode & 1u ?
            branch_master : compare_master;
        for (unsigned edge = 0u; edge < 2u; ++edge) {
            const uint64_t need = edges[edge];
            chunk = Lufia2DecompFramePollEdgeChunk(need, pair_master, first_master, chunk);
        }
        /* Auto-pad's start-of-vblank latch occurs before the first walk. */
        if (g_snes->autoJoyRead && g_snes->vPos == 225u && !g_snes->hPos)
            chunk = 1u;
        if (rtl_apu_extended_frame_timing() ||
            !interp_bridge_use_absolute_apu_timeline(
                rtl_apu_frame_timeline_active(), false, false)) {
            const uint64_t threshold = bridge_bounce_flush_thresh();
            const uint64_t need = threshold > s_apu_pending_master ?
                threshold - s_apu_pending_master : 0u;
            chunk = Lufia2DecompFramePollEdgeChunk(need, pair_master, first_master, chunk);
        }
        const unsigned compares = (chunk + ((opcode & 1u) ? 0u : 1u)) / 2u;
        const unsigned branches = chunk - compares;
        const bool last_compare = !((opcode + chunk - 1u) & 1u);
        cpu->open_bus = last_compare ? known->value : 0xfcu;
        cart->cpuBusAddress = last_compare ? 0x40u : entry + 3u;
        s_interp_continuous_read_epoch += compares;
        Lufia2DecompNmiAccountCycles(cpu, chunk * 3u,
            compares * compare_master + branches * branch_master);
        opcode += chunk;
    }
    s_frame_return_idle_opcodes += opcodes;
    Lufia2DecompFrameReturnComplete(opcodes, 0u);
    interp_bridge_pre_opcode_redirect(entry);
    Lufia2NativeIdleAccepted(15);
    return true;
#else
    (void)cpu; (void)entry;
    return false;
#endif
}

bool Lufia2DecompFramePollTryIdle(CpuState *cpu, uint32_t entry) {
    /* A native, read-only counter kernel may cross a device edge only by
     * running the real clock handlers at the same original opcode boundary.
     * IRQ/HDMA/observer ownership remains excluded by its full certificate.
     * The diagnostic reference retains the old eventless/per-opcode path. */
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
    if (!s_native_idle_reference_clock)
#endif
        if (Lufia2DecompFramePollClockedEdges(cpu, entry)) return true;
#if defined(SNESRECOMP_PLATFORM_BEAM_HOLD_QUERY) && !defined(SNES_COSIM) && \
    !defined(SNESRECOMP_INTERP_PROFILE) && \
    (!defined(SNESRECOMP_REVERSE_DEBUG) || !SNESRECOMP_REVERSE_DEBUG)
    if (cpu != &g_cpu || !Lufia2DecompFramePollIsCounterPair(entry) ||
        !Lufia2DecompFramePollBatchSupported(entry) ||
        !cpu->m_flag || cpu->x_flag > 1u || cpu->emulation || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1fffu ||
        cpu->PB != entry >> 16 || !cpu->ram || !g_snes ||
        g_snes->ram != cpu->ram || !g_snes->apu || !g_snes->dma ||
        g_snes->inNmi || g_snes->inIrq || g_snes->hIrqEnabled ||
        g_snes->vIrqEnabled ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u ||
        snesrecomp_beam_hold_active() || !g_snes->cart)
        return Lufia2NativeIdleDenied(Lufia2NativeIdleContextReason(cpu,entry));
    Cart *cart = g_snes->cart;
    if (cart->type != CART_LOROM || cart->superfx || cart->cx4 || cart->dsp1 ||
        cart->sdd1 || cart->sa1 || g_snes->dma->dmaBusy || g_snes->dma->dmaTimer)
        return Lufia2NativeIdleDenied(8);
    if (!FramePollChannelsIdle(g_snes))
        return Lufia2NativeIdleDenied(9);
    const uint32_t offset = (((entry >> 16) & 0x7fu) << 15) | (entry & 0x7fffu);
    if (!cart->rom || offset >= cart->romSize || cart->romSize - offset < 4u ||
        cart->rom[offset] != 0xc5u || cart->rom[offset + 1u] != 0x40u ||
        cart->rom[offset + 2u] != 0xf0u || cart->rom[offset + 3u] != 0xfcu ||
        (uint8_t)cpu->A != cpu->ram[0x40u])
        return Lufia2NativeIdleDenied(10);
    const BridgeDynamicValue *known = &s_bridge_dynamic_values[0];
    if (!known->valid || known->address != 0x40u || known->value != cpu->ram[0x40u])
        return Lufia2NativeIdleDenied(10);
    uint64_t phase, charged;
    snes_refresh_state_get(&phase, &charged);
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (phase >= 1364u || charged != cpu->master_cycles ||
        deadline <= cpu->master_cycles || deadline - cpu->master_cycles <= 128u)
        return Lufia2NativeIdleDenied(11);
    uint64_t room = 1364u - phase;
    const uint64_t beam_room = (g_snes->vPos < 225u && g_snes->hPos < 1024u
        ? 1024u : 1364u) - g_snes->hPos;
    if (beam_room < room) room = beam_room;
    const uint64_t deadline_room = deadline - cpu->master_cycles - 128u;
    if (deadline_room < room) room = deadline_room;
    uint64_t bulk_room = room;
    if (rtl_apu_extended_frame_timing() ||
        !interp_bridge_use_absolute_apu_timeline(rtl_apu_frame_timeline_active(),
                                                false, false)) {
        const uint64_t threshold = bridge_bounce_flush_thresh();
        bulk_room = s_apu_pending_master >= threshold ? 0u :
            threshold - s_apu_pending_master;
        if (bulk_room > room) bulk_room = room;
    }
    const uint32_t pair_master = 4u * cpu_region_speed(entry) +
        cpu_region_speed(0x40u) + 6u;
    if (room <= pair_master) return Lufia2NativeIdleDenied(12);
    unsigned pairs = ((uint32_t)room - 1u) / pair_master;
    /* The dispatcher guarantees at least16 remaining opcode slots. */
    if (pairs > 8u) pairs = 8u;
    const uint32_t master = pairs * pair_master;
    if (!snesrecomp_beam_eventless_step(g_snes->hPos, g_snes->vPos, master,
                                       true, false, g_snes->autoJoyRead))
        return Lufia2NativeIdleDenied(13);
    const unsigned bulk_pairs = bulk_room > pair_master ?
        (unsigned)((bulk_room - 1u) / pair_master) : 0u;
    if (pairs > bulk_pairs) {
        const uint32_t compare_master =
            2u * cpu_region_speed(entry) + cpu_region_speed(0x40u);
        const uint32_t branch_master = pair_master - compare_master;
        /* Preserve every original CMP/BEQ clock and APU-flush edge. The
         * observer-free, immutable LoROM/WRAM reads have only these final
         * latches at each opcode boundary. No SPC or DSP work is omitted. */
        cpu->_flag_C = cpu->_flag_Z = 1u;
        cpu->_flag_N = 0u;
        cpu_mirrors_to_p(cpu);
        unsigned opcode = 0u;
        const unsigned opcodes = pairs * 2u;
        while (opcode < opcodes) {
            unsigned chunk = opcodes - opcode;
            /* No beam/refresh edge is inside this certified interval.
             * End each aggregate at exactly the original opcode which
             * triggers an APU flush, with that opcode's bus/epoch latches.
             * Requery after a flush: transport backpressure may change the
             * host's flush threshold without changing guest execution. */
            if (rtl_apu_extended_frame_timing() ||
                !interp_bridge_use_absolute_apu_timeline(
                    rtl_apu_frame_timeline_active(), false, false)) {
                const uint64_t threshold = bridge_bounce_flush_thresh();
                const uint64_t need = threshold > s_apu_pending_master ?
                    threshold - s_apu_pending_master : 0u;
                const uint32_t first_master = opcode & 1u ?
                    branch_master : compare_master;
                const uint64_t full_pairs = need ? (need - 1u) / pair_master : 0u;
                if (full_pairs <= chunk / 2u) {
                    const uint64_t tail = need - full_pairs * pair_master;
                    const uint64_t boundary = full_pairs * 2u +
                        (tail <= first_master ? 1u : 2u);
                    if (boundary < chunk) chunk = (unsigned)boundary;
                }
            }
#if defined(LUFIA2_NATIVE_IDLE_DIAGNOSTICS)
            /* Diagnostic A/B: same jobs and binary, literal opcode clocks. */
            if (s_native_idle_reference_clock) chunk = 1u;
#endif
            const unsigned compares = (chunk + ((opcode & 1u) ? 0u : 1u)) / 2u;
            const unsigned branches = chunk - compares;
            const bool last_compare = !((opcode + chunk - 1u) & 1u);
            cpu->open_bus = last_compare ? known->value : 0xfcu;
            cart->cpuBusAddress = last_compare ? 0x40u : entry + 3u;
            s_interp_continuous_read_epoch += compares;
            Lufia2DecompNmiAccountCycles(cpu, chunk * 3u,
                compares * compare_master + branches * branch_master);
            opcode += chunk;
        }
        s_frame_return_idle_opcodes += pairs * 2u;
        Lufia2DecompFrameReturnComplete(pairs * 2u, 0u);
        interp_bridge_pre_opcode_redirect(entry);
        Lufia2NativeIdleAccepted(15);
        return true;
    }
    /* One original pair supplies the exact final bus latches. Further reads
     * are the same immutable byte; preserve their continuous-read epochs. */
    (void)bridge_bus_read(cpu, entry);
    (void)bridge_bus_read(cpu, entry + 1u);
    (void)bridge_bus_read(cpu, 0x40u);
    (void)bridge_bus_read(cpu, entry + 2u);
    (void)bridge_bus_read(cpu, entry + 3u);
    s_interp_continuous_read_epoch += pairs - 1u;
    cpu->_flag_C = cpu->_flag_Z = 1u;
    cpu->_flag_N = 0u;
    cpu_mirrors_to_p(cpu);
    Lufia2DecompNmiAccountCycles(cpu, pairs * 6u, master);
    s_frame_return_idle_opcodes += pairs * 2u;
    Lufia2DecompFrameReturnComplete(pairs * 2u, 0u);
    interp_bridge_pre_opcode_redirect(entry);
    Lufia2NativeIdleAccepted(14);
    return true;
#else
    (void)cpu; (void)entry;
    return false;
#endif
}
