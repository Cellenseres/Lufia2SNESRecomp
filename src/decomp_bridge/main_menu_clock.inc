enum { MAIN_MENU_CLOCK_CAPACITY = 64u };

typedef enum MainMenuTimingKind {
    MAIN_MENU_LINEAR, MAIN_MENU_CONDITION, MAIN_MENU_CHILD,
    MAIN_MENU_INLINE, MAIN_MENU_DISPATCH, MAIN_MENU_RETURN
} MainMenuTimingKind;

typedef struct MainMenuTimingBlock {
    uint16_t pc, offset, count, final_pc, next, taken;
    uint8_t modes, kind;
} MainMenuTimingBlock;

typedef struct MainMenuClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[MAIN_MENU_CLOCK_CAPACITY];
    uint64_t master_bound;
    uint32_t next;
    uint16_t dispatch_site, scenario_flags, scenario_remaining;
    uint8_t scenario_word, scenario_bits, capsule_type;
    bool returning;
} MainMenuClock;

#include "main_menu_timing.inc"

static unsigned MainMenuRamBase(const CpuState *cpu) {
    return cpu->DB == 0x7fu ? 0x10000u : 0u;
}

static unsigned MainMenuByte(const CpuState *cpu, unsigned address) {
    return cpu->ram[MainMenuRamBase(cpu) + address];
}

static const MainMenuTimingBlock *MainMenuBlock(uint16_t pc) {
    unsigned first = 0u, end = sizeof main_menu_blocks / sizeof main_menu_blocks[0];
    while (first < end) {
        const unsigned middle = first + (end - first) / 2u;
        const MainMenuTimingBlock *block = main_menu_blocks + middle;
        if (block->pc < pc) first = middle + 1u;
        else if (block->pc > pc) end = middle;
        else return block;
    }
    return NULL;
}

static bool MainMenuOpcode(MainMenuClock *clock, FixedSpanOpcode timing) {
    if (clock->span.count == MAIN_MENU_CLOCK_CAPACITY)
        return false;
    if (timing.pc == 0xc20eu || timing.pc == 0xc21cu || timing.pc == 0xc15au) {
        const CpuState *cpu = clock->span.cpu;
        const unsigned last = timing.pc == 0xc15au ? 0x1ffeu : 0x1fffu;
        if (cpu->DB == 0x83u && cpu->X > last)
            return false;
        timing.cycles = (uint8_t)((timing.pc == 0xc15au ? 5u : 4u) + (cpu->X > 0xffu));
    }
    clock->timing[clock->span.count++] = timing;
    clock->master_bound += timing.fetched *
        cpu_region_speed(0x820000u | timing.pc) + timing.data_bytes * 8u +
        (timing.cycles - timing.fetched - timing.data_bytes) * 6u;
    return true;
}

static bool MainMenuCondition(MainMenuClock *clock, uint16_t pc, bool *taken) {
    const CpuState *cpu = clock->span.cpu;
    const unsigned command = cpu->A & 0xffu;
    switch (pc) {
    case 0xbf2du: case 0xbfdfu: *taken = command == 1u; break;
    case 0xbf31u: case 0xbfe3u: *taken = command == 2u; break;
    case 0xbf35u: case 0xbfe7u: case 0xa8a1u: *taken = command == 3u; break;
    case 0xa8a5u: *taken = command == 4u; break;
    case 0xbfebu: case 0xa8a9u: *taken = command == 6u; break;
    case 0xbfefu: case 0xa8adu: *taken = command == 7u; break;
    case 0xa8b1u: *taken = command == 10u; break;
    case 0xa8b5u: *taken = command == 11u; break;
    case 0xbf44u: *taken = (MainMenuByte(cpu, 0x14acu) & 2u) != 0u; break;
    case 0xbf48u: *taken = (MainMenuByte(cpu, 0x14acu) & 1u) != 0u; break;
    case 0xbf4au: *taken = true; break;
    case 0xbf7cu: *taken = MainMenuByte(cpu, 0x0b52u) == 0u; break;
    case 0xbf8du: *taken = MainMenuByte(cpu, 0x0b52u) == 2u; break;
    case 0xa1a8u: case 0xc004u:
        *taken = MainMenuByte(cpu, 0x14dau) != 2u; break;
    case 0xc075u: *taken = MainMenuByte(cpu, 0x057cu) == 0u; break;
    case 0xc07cu: *taken = ((command ^ 1u) & 1u) != 0u; break;
    case 0xc086u:
        *taken = ((MainMenuByte(cpu, 0x10edu) + 1u) & 0xffu) != 100u; break;
    case 0xc092u: *taken = MainMenuByte(cpu, 0x10edu) != 1u; break;
    case 0xa8ddu: case 0xa903u: *taken = cpu->_flag_C != 0u; break;
    case 0xfc4au:
        clock->scenario_flags = (uint16_t)(MainMenuByte(cpu,
            0x091eu + clock->scenario_word) | MainMenuByte(cpu,
            0x091fu + clock->scenario_word) << 8);
        clock->scenario_bits = 16u;
        *taken = clock->scenario_flags == 0u;
        break;
    case 0xfc54u:
        *taken = !(clock->scenario_flags & 1u);
        clock->scenario_flags >>= 1;
        break;
    case 0xfc57u:
        --clock->scenario_remaining;
        *taken = clock->scenario_remaining == 0u;
        break;
    case 0xfc5au:
        --clock->scenario_bits;
        *taken = clock->scenario_bits != 0u;
        break;
    case 0xfc6du:
        clock->scenario_word = (uint8_t)(clock->scenario_word + 2u);
        *taken = clock->scenario_word != 8u;
        break;
    case 0xc0c3u: case 0xc17au: *taken = command == 1u; break;
    case 0xc0c7u: case 0xc17eu: *taken = command == 2u; break;
    case 0xc0cbu: case 0xc182u: *taken = command == 3u; break;
    case 0xc0cfu: *taken = command == 6u; break;
    case 0xc0d3u: *taken = command == 7u; break;
    case 0xc0d7u: *taken = command == 4u; break;
    case 0xc0e5u: *taken = !(cpu->ram[0x47u] & 4u); break;
    case 0xc0eau: *taken = MainMenuByte(cpu, 0x14b3u) == 0u; break;
    case 0xc0eeu: *taken = MainMenuByte(cpu, 0x14b3u) == 2u; break;
    case 0xc136u: case 0xc14du: *taken = cpu->_flag_C != 0u; break;
    case 0xc148u: *taken = MainMenuByte(cpu, 0x057cu) == 0u; break;
    case 0xc1bau:
        *taken = !(MainMenuByte(cpu, 0x14acu) & 3u);
        if (!*taken && clock->capsule_type >= 7u)
            return false;
        break;
    case 0xc1c6u: *taken = !(MainMenuByte(cpu, 0x14acu) & 2u); break;
    case 0xc1ceu:
        *taken = clock->capsule_type != 0u;
        clock->capsule_type = *taken ? (uint8_t)(clock->capsule_type - 1u) : 6u;
        break;
    case 0xc1dcu:
        clock->capsule_type = (uint8_t)(clock->capsule_type + 1u);
        *taken = clock->capsule_type != 7u;
        if (!*taken)
            clock->capsule_type = 0u;
        break;
    case 0xc1f2u:
        if (clock->capsule_type >= 7u)
            return false;
        *taken = MainMenuByte(cpu, 0x11bbu + clock->capsule_type) == 0u;
        break;
    case 0xc213u: case 0xc223u:
        if (cpu->DB == 0x83u && cpu->X >= 0x2000u)
            return false;
        if (pc == 0xc213u)
            *taken = !(MainMenuByte(cpu, cpu->X) & 8u);
        else
            *taken = (MainMenuByte(cpu, cpu->X) & 7u) >= cpu->ram[0u];
        break;
    case 0xc22du: *taken = !(MainMenuByte(cpu, 0x14acu) & 8u); break;
    default:
        return false;
    }
    return true;
}

static bool MainMenuDispatch(MainMenuClock *clock, uint16_t *target) {
    const CpuState *cpu = clock->span.cpu;
    unsigned selected;
    const uint16_t *targets;
    unsigned count;
    static const uint16_t indicators[] = {0xa0dau, 0xa0ddu, 0xa0e0u};
    static const uint16_t previous[] = {0xbf79u, 0xbf99u, 0xbfa3u};
    static const uint16_t next[] = {0xbf88u, 0xbf99u, 0xbfa3u, 0xbfadu};
    static const uint16_t actions[] = {0xc04du, 0xc051u, 0xc055u};
    static const uint16_t feeding[] = {0xc10du, 0xc115u, 0xc11au};
    switch (clock->dispatch_site) {
    case 0xa0d1u:
        selected = cpu->A & 0xffu;
        targets = indicators;
        count = sizeof indicators / sizeof indicators[0];
        break;
    case 0xbf4fu:
        selected = MainMenuByte(cpu, 0x14b3u);
        targets = previous;
        count = sizeof previous / sizeof previous[0];
        break;
    case 0xbf5bu:
        selected = MainMenuByte(cpu, 0x14b3u);
        targets = next;
        count = sizeof next / sizeof next[0];
        break;
    case 0xc044u:
        selected = cpu->A & 0xffu;
        targets = actions;
        count = sizeof actions / sizeof actions[0];
        break;
    case 0xc104u:
        selected = MainMenuByte(cpu, 0x14b3u);
        targets = feeding;
        count = sizeof feeding / sizeof feeding[0];
        break;
    default:
        return false;
    }
    if (selected >= count)
        return false;
    *target = targets[selected];
    return true;
}

static bool PlanMainMenu(MainMenuClock *clock, uint16_t entry) {
    const CpuState *cpu = clock->span.cpu;
    const MainMenuTimingBlock *initial = MainMenuBlock(entry);
    if (entry == 0x8028u) {
        if (cpu->x_flag || Lufia2DecompMenuDispatchObserved())
            return false;
        for (unsigned index = 0u; index < initial->count; ++index)
            if (!MainMenuOpcode(clock, main_menu_opcodes[initial->offset + index]))
                return false;
        return true;
    }
    if (!initial || !(initial->modes & (1u << (cpu->m_flag * 2u + cpu->x_flag))))
        return false;
    clock->scenario_remaining = (uint16_t)(cpu->X + 1u);
    if (entry >= 0xc170u && entry <= 0xc25eu)
        clock->capsule_type = (uint8_t)MainMenuByte(cpu, 0x14deu);
    uint16_t pc = entry;
    for (unsigned visited = 0u; visited < MAIN_MENU_CLOCK_CAPACITY; ++visited) {
        const MainMenuTimingBlock *block = MainMenuBlock(pc);
        if (!block)
            return false;
        bool taken = false;
        if (block->kind == MAIN_MENU_CONDITION &&
            !MainMenuCondition(clock, block->final_pc, &taken))
            return false;
        for (unsigned index = 0u; index < block->count; ++index) {
            FixedSpanOpcode timing = main_menu_opcodes[block->offset + index];
            if (block->kind == MAIN_MENU_CONDITION && index + 1u == block->count)
                timing.cycles = taken ? 3u : 2u;
            if (!MainMenuOpcode(clock, timing))
                return false;
        }
        if (block->kind == MAIN_MENU_CHILD || block->kind == MAIN_MENU_RETURN) {
            clock->next = 0x820000u | block->next;
            clock->returning = block->kind == MAIN_MENU_RETURN;
            return true;
        }
        if (block->kind == MAIN_MENU_INLINE) {
            if (Lufia2DecompMenuDispatchObserved())
                return false;
            clock->dispatch_site = block->final_pc;
        }
        pc = taken ? block->taken : block->next;
        if (block->kind == MAIN_MENU_DISPATCH && !MainMenuDispatch(clock, &pc))
            return false;
    }
    return false;
}

static bool PrepareMainMenuClock(MainMenuClock *clock, CpuState *cpu,
                                  uint32_t entry) {
    if (entry >> 16 != 0x82u || cpu->PB != 0x82u || cpu->D || cpu->_flag_D ||
        cpu->m_flag > 1u || cpu->x_flag > 1u ||
        (cpu->host_return_valid != 2u && cpu->host_return_valid != 3u) ||
        (cpu->DB != 0x83u && cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x280000u ||
        g_snes->vPos >= 262u)
        return false;
    *clock = (MainMenuClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x820000u;
    clock->span.timing = clock->timing;
    if (!PlanMainMenu(clock, (uint16_t)entry))
        return false;
    if (clock->returning &&
        !Lufia2DecompFrameRangeQuiet(clock->next, clock->next + 1u))
        return false;
    for (unsigned index = 0u; index < clock->span.count; ++index) {
        const FixedSpanOpcode *opcode = clock->timing + index;
        const uint32_t pc = 0x820000u | opcode->pc;
        if (!Lufia2DecompFrameRangeQuiet(pc, pc + opcode->fetched))
            return false;
        if (((opcode->pc >= 0xa125u && opcode->pc <= 0xa138u) ||
             opcode->pc == 0xa17eu || opcode->pc == 0xa181u) && cpu->X >= 48u)
            return false;
    }
    const uint64_t work = clock->master_bound + (clock->returning ? 64u : 0u);
    const uint64_t bound = work + 40u * (work / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - bound ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, bound))
        return false;
    if (!g_snes->hdmaBeamOff && g_snes->vPos < 225u) {
        const unsigned fence = g_snes->hPos < 1024u ? 1024u : 1364u;
        if (bound >= fence - g_snes->hPos &&
            !(entry == 0x82a1d8u && MenuWramSpanHdmaSafe()))
            return false;
    }
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= bound)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
