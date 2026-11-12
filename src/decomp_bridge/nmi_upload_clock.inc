extern int g_interp_apu_driving;

typedef enum NmiUploadPhase {
    NMI_UPLOAD_STATUS,
    NMI_UPLOAD_OAM_FLAG,
    NMI_UPLOAD_OAM_BRANCH,
    NMI_UPLOAD_OAM_DATA,
    NMI_UPLOAD_PALETTE_FLAG,
    NMI_UPLOAD_PALETTE_BRANCH,
    NMI_UPLOAD_PALETTE_DATA,
    NMI_UPLOAD_BUSY_READ,
    NMI_UPLOAD_BUSY_BRANCH,
    NMI_UPLOAD_PAD_SETUP,
    NMI_UPLOAD_PAD_READ,
    NMI_UPLOAD_PAD_BRANCH,
    NMI_UPLOAD_PAD_CHANGED,
    NMI_UPLOAD_PAD_COUNTDOWN,
    NMI_UPLOAD_REPEAT_BRANCH,
    NMI_UPLOAD_PAD_REPEAT,
    NMI_UPLOAD_PAD_NEXT,
    NMI_UPLOAD_PAD_LOOP,
    NMI_UPLOAD_RESTORE,
    NMI_UPLOAD_RETURN,
    NMI_UPLOAD_DONE
} NmiUploadPhase;

typedef struct NmiUploadClock {
    FixedSpanClock bus;
    NmiUploadPhase phase;
    FixedSpanOpcode branch;
    unsigned pad;
    uint8_t request;
    bool busy;
    uint16_t buttons;
    uint16_t previous_buttons;
    uint16_t countdown;
    uint16_t word;
    uint32_t word_address;
    bool word_pending;
} NmiUploadClock;

static void NmiUploadSelectPhase(NmiUploadClock *clock, NmiUploadPhase phase) {
    static const FixedSpanOpcode status[] = {
        {0x8703u, 1u, 3u, 1u}, {0x8704u, 2u, 3u, 0u}};
    static const FixedSpanOpcode oam_flag[] = {
        {0x8706u, 2u, 3u, 1u}, {0x8708u, 2u, 2u, 0u}};
    static const FixedSpanOpcode oam_data[] = {
        {0x870cu, 2u, 2u, 0u}, {0x870eu, 2u, 5u, 2u},
        {0x8710u, 3u, 3u, 0u}, {0x8713u, 3u, 5u, 2u},
        {0x8716u, 2u, 2u, 0u}, {0x8718u, 3u, 4u, 1u},
        {0x871bu, 3u, 3u, 0u}, {0x871eu, 3u, 5u, 2u},
        {0x8721u, 2u, 2u, 0u}, {0x8723u, 3u, 4u, 1u},
        {0x8726u, 2u, 2u, 0u}, {0x8728u, 3u, 4u, 1u},
        {0x872bu, 3u, 3u, 0u}, {0x872eu, 3u, 5u, 2u},
        {0x8731u, 2u, 2u, 0u}, {0x8733u, 3u, 4u, 1u}};
    static const FixedSpanOpcode palette_flag[] = {
        {0x8736u, 2u, 3u, 1u}, {0x8738u, 2u, 2u, 0u}};
    static const FixedSpanOpcode palette_data[] = {
        {0x873cu, 2u, 2u, 0u}, {0x873eu, 2u, 5u, 2u},
        {0x8740u, 2u, 2u, 0u}, {0x8742u, 3u, 4u, 1u},
        {0x8745u, 2u, 2u, 0u}, {0x8747u, 3u, 4u, 1u},
        {0x874au, 3u, 3u, 0u}, {0x874du, 3u, 5u, 2u},
        {0x8750u, 2u, 2u, 0u}, {0x8752u, 3u, 4u, 1u},
        {0x8755u, 2u, 2u, 0u}, {0x8757u, 3u, 4u, 1u},
        {0x875au, 3u, 3u, 0u}, {0x875du, 3u, 5u, 2u},
        {0x8760u, 2u, 2u, 0u}, {0x8762u, 3u, 4u, 1u}};
    static const FixedSpanOpcode busy_read[] = {
        {0x8765u, 3u, 4u, 1u}, {0x8768u, 1u, 2u, 0u}};
    static const FixedSpanOpcode pad_setup[] = {
        {0x876bu, 2u, 3u, 0u}, {0x876du, 3u, 3u, 0u}};
    static const FixedSpanOpcode pad_read[] = {
        {0x8770u, 3u, 5u, 2u}, {0x8773u, 2u, 5u, 2u},
        {0x8775u, 3u, 5u, 2u}};
    static const FixedSpanOpcode pad_changed[] = {
        {0x8791u, 3u, 7u, 2u}, {0x8794u, 3u, 3u, 0u},
        {0x8797u, 2u, 5u, 2u}, {0x8799u, 2u, 5u, 2u},
        {0x879bu, 3u, 3u, 0u}, {0x879eu, 3u, 7u, 2u}};
    static const FixedSpanOpcode countdown[] = {{0x877au, 3u, 10u, 4u}};
    static const FixedSpanOpcode pad_repeat[] = {
        {0x877fu, 3u, 5u, 2u}, {0x8782u, 3u, 5u, 2u},
        {0x8785u, 2u, 5u, 2u}, {0x8787u, 2u, 5u, 2u},
        {0x8789u, 3u, 3u, 0u}, {0x878cu, 3u, 7u, 2u},
        {0x878fu, 2u, 3u, 0u}};
    static const FixedSpanOpcode pad_next[] = {
        {0x87a1u, 1u, 2u, 0u}, {0x87a2u, 1u, 2u, 0u}};
    static const FixedSpanOpcode restore[] = {{0x87a5u, 1u, 4u, 1u}};
    static const FixedSpanOpcode return_frame[] = {{0x87a6u, 1u, 6u, 2u}};
    const FixedSpanOpcode *timing = NULL;
    unsigned count = 0u;
#define NMI_UPLOAD_PLAN(name) timing = name; count = sizeof name / sizeof *name
    switch (phase) {
    case NMI_UPLOAD_STATUS: NMI_UPLOAD_PLAN(status); break;
    case NMI_UPLOAD_OAM_FLAG: NMI_UPLOAD_PLAN(oam_flag); break;
    case NMI_UPLOAD_OAM_DATA: NMI_UPLOAD_PLAN(oam_data); break;
    case NMI_UPLOAD_PALETTE_FLAG: NMI_UPLOAD_PLAN(palette_flag); break;
    case NMI_UPLOAD_PALETTE_DATA: NMI_UPLOAD_PLAN(palette_data); break;
    case NMI_UPLOAD_BUSY_READ: NMI_UPLOAD_PLAN(busy_read); break;
    case NMI_UPLOAD_PAD_SETUP: NMI_UPLOAD_PLAN(pad_setup); break;
    case NMI_UPLOAD_PAD_READ: NMI_UPLOAD_PLAN(pad_read); break;
    case NMI_UPLOAD_PAD_CHANGED: NMI_UPLOAD_PLAN(pad_changed); break;
    case NMI_UPLOAD_PAD_COUNTDOWN: NMI_UPLOAD_PLAN(countdown); break;
    case NMI_UPLOAD_PAD_REPEAT: NMI_UPLOAD_PLAN(pad_repeat); break;
    case NMI_UPLOAD_PAD_NEXT: NMI_UPLOAD_PLAN(pad_next); break;
    case NMI_UPLOAD_RESTORE: NMI_UPLOAD_PLAN(restore); break;
    case NMI_UPLOAD_RETURN: NMI_UPLOAD_PLAN(return_frame); break;
    case NMI_UPLOAD_DONE: break;
    default: {
        unsigned pc;
        bool taken;
        switch (phase) {
        case NMI_UPLOAD_OAM_BRANCH: pc = 0x870au; taken = !(clock->request & 0xc0u); break;
        case NMI_UPLOAD_PALETTE_BRANCH: pc = 0x873au; taken = !(clock->request & 0xc0u); break;
        case NMI_UPLOAD_BUSY_BRANCH: pc = 0x8769u; taken = clock->busy; break;
        case NMI_UPLOAD_PAD_BRANCH: pc = 0x8778u; taken = clock->buttons != clock->previous_buttons; break;
        case NMI_UPLOAD_REPEAT_BRANCH: pc = 0x877du; taken = clock->countdown != 1u; break;
        default: assert(phase == NMI_UPLOAD_PAD_LOOP); pc = 0x87a3u; taken = clock->pad != 0u; break;
        }
        clock->branch = (FixedSpanOpcode){(uint16_t)pc, 2u, (uint8_t)(2u + taken), 0u};
        timing = &clock->branch;
        count = 1u;
        break;
    }
    }
#undef NMI_UPLOAD_PLAN
    clock->phase = phase;
    clock->bus.timing = timing;
    clock->bus.count = count;
    clock->bus.index = 0u;
}

static void NmiUploadAdvancePhase(NmiUploadClock *clock) {
    NmiUploadPhase next;
    switch (clock->phase) {
    case NMI_UPLOAD_STATUS: next = NMI_UPLOAD_OAM_FLAG; break;
    case NMI_UPLOAD_OAM_FLAG: next = NMI_UPLOAD_OAM_BRANCH; break;
    case NMI_UPLOAD_OAM_BRANCH: next = clock->request & 0xc0u ? NMI_UPLOAD_OAM_DATA : NMI_UPLOAD_PALETTE_FLAG; break;
    case NMI_UPLOAD_OAM_DATA: next = NMI_UPLOAD_PALETTE_FLAG; break;
    case NMI_UPLOAD_PALETTE_FLAG: next = NMI_UPLOAD_PALETTE_BRANCH; break;
    case NMI_UPLOAD_PALETTE_BRANCH: next = clock->request & 0xc0u ? NMI_UPLOAD_PALETTE_DATA : NMI_UPLOAD_BUSY_READ; break;
    case NMI_UPLOAD_PALETTE_DATA: next = NMI_UPLOAD_BUSY_READ; break;
    case NMI_UPLOAD_BUSY_READ: next = NMI_UPLOAD_BUSY_BRANCH; break;
    case NMI_UPLOAD_BUSY_BRANCH: next = clock->busy ? NMI_UPLOAD_BUSY_READ : NMI_UPLOAD_PAD_SETUP; break;
    case NMI_UPLOAD_PAD_SETUP: clock->pad = 2u; next = NMI_UPLOAD_PAD_READ; break;
    case NMI_UPLOAD_PAD_READ: next = NMI_UPLOAD_PAD_BRANCH; break;
    case NMI_UPLOAD_PAD_BRANCH: next = clock->buttons != clock->previous_buttons ? NMI_UPLOAD_PAD_CHANGED : NMI_UPLOAD_PAD_COUNTDOWN; break;
    case NMI_UPLOAD_PAD_COUNTDOWN: next = NMI_UPLOAD_REPEAT_BRANCH; break;
    case NMI_UPLOAD_REPEAT_BRANCH: next = clock->countdown != 1u ? NMI_UPLOAD_PAD_NEXT : NMI_UPLOAD_PAD_REPEAT; break;
    case NMI_UPLOAD_PAD_CHANGED:
    case NMI_UPLOAD_PAD_REPEAT: next = NMI_UPLOAD_PAD_NEXT; break;
    case NMI_UPLOAD_PAD_NEXT: next = NMI_UPLOAD_PAD_LOOP; break;
    case NMI_UPLOAD_PAD_LOOP:
        next = clock->pad ? NMI_UPLOAD_PAD_READ : NMI_UPLOAD_RESTORE;
        clock->pad = 0u;
        break;
    case NMI_UPLOAD_RESTORE: next = NMI_UPLOAD_RETURN; break;
    default: assert(clock->phase == NMI_UPLOAD_RETURN); next = NMI_UPLOAD_DONE; break;
    }
    NmiUploadSelectPhase(clock, next);
}

static void NmiUploadBeginOpcode(NmiUploadClock *clock) {
    while (!clock->bus.remaining && clock->phase != NMI_UPLOAD_DONE) {
        if (clock->bus.index == clock->bus.count)
            NmiUploadAdvancePhase(clock);
        if (clock->phase != NMI_UPLOAD_DONE)
            NmiClockBeginOpcode(&clock->bus);
    }
}

static void NmiUploadCaptureByte(NmiUploadClock *clock, uint32_t address, uint8_t value) {
    const unsigned offset = address & 0xffffu;
    if (clock->phase == NMI_UPLOAD_OAM_FLAG || clock->phase == NMI_UPLOAD_PALETTE_FLAG)
        clock->request = value;
    else if (clock->phase == NMI_UPLOAD_BUSY_READ)
        clock->busy = (value & 1u) != 0u;
    else {
        uint16_t *word = NULL;
        unsigned low = 0u;
        if (clock->phase == NMI_UPLOAD_PAD_READ) {
            low = 0x4218u + clock->pad;
            if (offset == low || offset == low + 1u)
                word = &clock->buttons;
            else {
                low = 0x055eu + clock->pad;
                word = &clock->previous_buttons;
            }
        } else if (clock->phase == NMI_UPLOAD_PAD_COUNTDOWN) {
            low = 0x055au + clock->pad;
            word = &clock->countdown;
        }
        if (word)
            *word = offset == low ? value : (uint16_t)(*word | ((uint16_t)value << 8));
    }
}

static uint8_t NmiUploadRead(void *context, uint32_t address) {
    NmiUploadClock *clock = context;
    NmiUploadBeginOpcode(clock);
    FixedSpanClock *bus = &clock->bus;
    assert(bus->remaining);
    bool progress = false;
    uint8_t value;
    if (clock->word_pending) {
        assert(address == clock->word_address + 1u);
        value = (uint8_t)(clock->word >> 8);
        clock->word_pending = false;
    } else if (clock->phase == NMI_UPLOAD_PAD_READ &&
               bus->timing[bus->index].pc == 0x8770u) {
        clock->word = Lufia2DecompFrameReturnReadWord(bus->cpu, address, address + 1u, &progress);
        clock->word_address = address;
        clock->word_pending = true;
        value = (uint8_t)clock->word;
    } else if (clock->phase == NMI_UPLOAD_BUSY_READ) {
        value = Lufia2DecompNmiReadJoypadBusy(bus->cpu, &progress);
    } else {
        value = Lufia2DecompFrameReturnReadByte(bus->cpu, address, &progress);
    }
    NmiUploadCaptureByte(clock, address, value);
    bus->data_master += cpu_region_speed(address);
    bus->progress |= progress;
    if (!--bus->remaining)
        FinishFixedSpanOpcode(bus);
    return value;
}

static void NmiUploadWrite(void *context, uint32_t address, uint8_t value) {
    NmiUploadClock *clock = context;
    NmiUploadBeginOpcode(clock);
    FixedSpanClock *bus = &clock->bus;
    assert(bus->remaining);
    const FixedSpanOpcode *opcode = bus->timing + bus->index;
    const bool hardware_word = opcode->data_bytes == 2u &&
        (address & 0xffffu) >= 0x2100u && (address & 0xffffu) < 0x4380u;
    bool progress = false;
    if (hardware_word && bus->remaining == 2u) {
        clock->word = value;
        clock->word_address = address;
    } else if (hardware_word) {
        assert(address == clock->word_address + 1u);
        const uint16_t word = (uint16_t)(clock->word | ((uint16_t)value << 8));
        Lufia2DecompFrameReturnWriteWord(bus->cpu, clock->word_address, address, word, &progress);
    } else {
        Lufia2DecompFrameReturnWriteByte(bus->cpu, address, value, &progress);
    }
    bus->data_master += cpu_region_speed(address);
    bus->progress |= progress;
    if (!--bus->remaining)
        FinishFixedSpanOpcode(bus);
}

static bool NmiUploadSupported(const CpuState *cpu) {
    const unsigned reserve = 20000u;
    if (!g_snes || !g_snes->inNmi || g_snes->inIrq || !g_snes->dma || !g_snes->apu ||
        cpu != &g_cpu || !cpu->ram || cpu->ram != g_snes->ram ||
        cpu->emulation || cpu->PB != 0x80u ||
        cpu->DB != 0x80u || cpu->D || cpu->m_flag != 1u || cpu->x_flag > 1u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ff3u || cpu->host_return_valid != 2u ||
        !Lufia2DecompFramePairedCallReady(cpu, 0x808703u) || g_interp_apu_driving ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        g_snes->beamMasterLast != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        g_snes->hIrqEnabled || g_snes->vIrqEnabled || !g_snes->hdmaBeamOff ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer ||
        g_snes->dma->hdmaPendingInit || g_snes->autoJoyTimer > 4224u ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u || !g_snes->cart ||
        g_snes->cart->type != CART_LOROM || !g_snes->cart->rom ||
        g_snes->cart->romSize < 0x07a7u)
        return false;
    for (unsigned channel = 0u; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    if (deadline && (deadline <= cpu->master_cycles || deadline - cpu->master_cycles < reserve))
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles) &&
        Lufia2DecompFrameRangeQuiet(0x808703u, 0x8087a7u);
}

static RecompReturn RunNativeNmiUpload(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    NmiUploadClock clock = {0};
    clock.bus.cpu = cpu;
    clock.bus.bank = 0x800000u;
    NmiUploadSelectPhase(&clock, NMI_UPLOAD_STATUS);
    const Lufia2Memory memory = {NmiUploadRead, NmiUploadWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiSpritesPaletteAndPads(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x8087a6u);
    ActorBridgeStore(cpu, &state);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const uint16_t low = NmiUploadRead(&clock, cpu->S);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const uint16_t high = NmiUploadRead(&clock, cpu->S);
    NmiUploadBeginOpcode(&clock);
    assert(clock.phase == NMI_UPLOAD_DONE && !clock.word_pending);
    const uint16_t ret_s = (uint16_t)(cpu->S - 2u);
    const uint32_t target = ((uint32_t)cpu->PB << 16) | (uint16_t)((high << 8) + low + 1u);
    return ActorBridgeResolveReturn(cpu, &frame, 2u, result.pc, ret_s, target);
}
