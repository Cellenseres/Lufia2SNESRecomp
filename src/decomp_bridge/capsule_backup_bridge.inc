#include "lufia2/menu_capsule_selection.h"

typedef enum CapsuleBackupPhase {
    CAPSULE_BACKUP_BEGIN, CAPSULE_BACKUP_WORD,
    CAPSULE_BACKUP_RESTORE, CAPSULE_BACKUP_DONE
} CapsuleBackupPhase;

typedef struct CapsuleBackupClock {
    FixedSpanClock span;
    const struct CapsuleInventoryPlan *plan;
    CapsuleBackupPhase phase;
    unsigned word;
} CapsuleBackupClock;

static const FixedSpanOpcode capsule_backup_begin[] = {
    {0xfba7u, 1u, 3u, 1u}, {0xfba8u, 1u, 3u, 1u},
    {0xfba9u, 2u, 3u, 0u}, {0xfbabu, 2u, 3u, 0u},
    {0xfbadu, 2u, 2u, 0u}, {0xfbafu, 1u, 3u, 1u},
    {0xfbb0u, 1u, 4u, 1u}, {0xfbb1u, 2u, 3u, 0u},
    {0xfbb3u, 3u, 3u, 0u}
};
static const FixedSpanOpcode capsule_backup_word_0[] = {
    {0xfbb6u, 3u, 5u, 2u}, {0xfbb9u, 3u, 7u, 2u},
    {0xfbbcu, 1u, 2u, 0u}, {0xfbbdu, 1u, 2u, 0u},
    {0xfbbeu, 3u, 3u, 0u}, {0xfbc1u, 2u, 3u, 0u}
};
static const FixedSpanOpcode capsule_backup_word_1[] = {
    {0xfbb6u, 3u, 6u, 2u}, {0xfbb9u, 3u, 7u, 2u},
    {0xfbbcu, 1u, 2u, 0u}, {0xfbbdu, 1u, 2u, 0u},
    {0xfbbeu, 3u, 3u, 0u}, {0xfbc1u, 2u, 3u, 0u}
};
static const FixedSpanOpcode capsule_backup_word_2[] = {
    {0xfbb6u, 3u, 6u, 2u}, {0xfbb9u, 3u, 7u, 2u},
    {0xfbbcu, 1u, 2u, 0u}, {0xfbbdu, 1u, 2u, 0u},
    {0xfbbeu, 3u, 3u, 0u}, {0xfbc1u, 2u, 2u, 0u}
};
static const FixedSpanOpcode capsule_backup_return[] = {
    {0xfbc3u, 1u, 4u, 1u}, {0xfbc4u, 1u, 4u, 1u}
};
static const FixedSpanOpcode capsule_reload_begin[] = {
    {0xfbc6u, 1u, 3u, 1u}, {0xfbc7u, 1u, 3u, 1u},
    {0xfbc8u, 2u, 3u, 0u}, {0xfbcau, 2u, 3u, 0u},
    {0xfbccu, 2u, 2u, 0u}, {0xfbceu, 1u, 3u, 1u},
    {0xfbcfu, 1u, 4u, 1u}, {0xfbd0u, 2u, 3u, 0u},
    {0xfbd2u, 3u, 3u, 0u}
};
static const FixedSpanOpcode capsule_reload_word_0[] = {
    {0xfbd5u, 3u, 5u, 2u}, {0xfbd8u, 3u, 7u, 2u},
    {0xfbdbu, 1u, 2u, 0u}, {0xfbdcu, 1u, 2u, 0u},
    {0xfbddu, 3u, 3u, 0u}, {0xfbe0u, 2u, 3u, 0u}
};
static const FixedSpanOpcode capsule_reload_word_1[] = {
    {0xfbd5u, 3u, 5u, 2u}, {0xfbd8u, 3u, 7u, 2u},
    {0xfbdbu, 1u, 2u, 0u}, {0xfbdcu, 1u, 2u, 0u},
    {0xfbddu, 3u, 3u, 0u}, {0xfbe0u, 2u, 2u, 0u}
};
static const FixedSpanOpcode capsule_reload_return[] = {
    {0xfbe2u, 1u, 4u, 1u}, {0xfbe3u, 1u, 4u, 1u}
};

typedef struct CapsuleInventoryPlan {
    const FixedSpanOpcode *begin;
    const FixedSpanOpcode *word[3];
    const FixedSpanOpcode *finish;
    unsigned crossing_word;
} CapsuleInventoryPlan;

static const CapsuleInventoryPlan capsule_inventory_plans[] = {
    {capsule_backup_begin, {capsule_backup_word_0, capsule_backup_word_1,
        capsule_backup_word_2}, capsule_backup_return, 58u},
    {capsule_reload_begin, {capsule_reload_word_0, capsule_reload_word_0,
        capsule_reload_word_1}, capsule_reload_return, 96u}
};

static void AdvanceCapsuleBackup(CapsuleBackupClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count ||
            clock->phase == CAPSULE_BACKUP_DONE)
            return;
        if (clock->phase == CAPSULE_BACKUP_BEGIN) {
            clock->phase = CAPSULE_BACKUP_WORD;
        } else if (clock->phase == CAPSULE_BACKUP_WORD) {
            if (++clock->word == 96u)
                clock->phase = CAPSULE_BACKUP_RESTORE;
        } else {
            clock->phase = CAPSULE_BACKUP_DONE;
            return;
        }
        clock->span.index = 0u;
        if (clock->phase == CAPSULE_BACKUP_WORD) {
            const unsigned path = clock->word == 95u ? 2u :
                clock->word >= clock->plan->crossing_word ? 1u : 0u;
            clock->span.timing = clock->plan->word[path];
            clock->span.count = 6u;
        } else {
            clock->span.timing = clock->plan->finish;
            clock->span.count = 2u;
        }
    }
}

static uint8_t CapsuleBackupRead(void *context, uint32_t address) {
    CapsuleBackupClock *clock = context;
    AdvanceCapsuleBackup(clock);
    return FixedSpanRead(&clock->span, address);
}

static void CapsuleBackupWrite(void *context, uint32_t address, uint8_t value) {
    CapsuleBackupClock *clock = context;
    AdvanceCapsuleBackup(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static bool PrepareCapsuleBackup(CapsuleBackupClock *clock, CpuState *cpu,
                                   bool restore) {
    const uint32_t entry = restore ? 0x82fbc6u : 0x82fba7u;
    if ((restore && !Lufia2DecompFrameReturnCpuReady()) ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->host_return_valid != 2u ||
        cpu->S < 0x1f03u || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, false, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x280000u ||
        g_snes->vPos >= 262u ||
        !Lufia2DecompFrameRangeQuiet(entry, restore ? 0x82fbe5u : 0x82fbc6u))
        return false;
    /* Maximum original span, including return allowance. */
    const uint64_t work = 1265u * (uint64_t)cpu_region_speed(entry) + 6712u;
    const uint64_t bound = work + 40u * (work / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - bound ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, false, 1363u, bound))
        return false;
    if (!MenuWramSpanHdmaSafe())
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= bound)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    if (phase >= 1364u || (current && current != cpu->master_cycles))
        return false;
    *clock = (CapsuleBackupClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x820000u;
    clock->plan = &capsule_inventory_plans[restore ? 1u : 0u];
    clock->span.timing = clock->plan->begin;
    clock->span.count = 9u;
    return true;
}

static RecompReturn ActorBridgeCapsuleBackup(CpuState *cpu, bool restore) {
    const uint32_t entry = restore ? 0x82fbc6u : 0x82fba7u;
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CapsuleBackupClock clock;
    if (!PrepareCapsuleBackup(&clock, cpu, restore))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {CapsuleBackupRead, CapsuleBackupWrite,
        &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    state.resume_pc = entry;
    Lufia2ExecutionResult result;
#if defined(LUFIA2_MENU_INVENTORY_RESTORE_API)
    if (restore)
        result = Lufia2MenuRestoreInventory(&memory, &state);
    else
#endif
        result = Lufia2MenuBackupInventory(&memory, &state);
    ActorBridgeStore(cpu, &state);
    AdvanceCapsuleBackup(&clock);
    assert(clock.phase == CAPSULE_BACKUP_DONE && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED &&
        result.pc == (restore ? 0x82fbe4u : 0x82fbc5u));
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
