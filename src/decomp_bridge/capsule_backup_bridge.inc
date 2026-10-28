#include "lufia2/menu_capsule_selection.h"

typedef enum CapsuleBackupPhase {
    CAPSULE_BACKUP_BEGIN, CAPSULE_BACKUP_WORD,
    CAPSULE_BACKUP_RESTORE, CAPSULE_BACKUP_DONE
} CapsuleBackupPhase;

typedef struct CapsuleBackupClock {
    FixedSpanClock span;
    FixedSpanOpcode word_timing[6];
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
static const FixedSpanOpcode capsule_backup_word[] = {
    {0xfbb6u, 3u, 6u, 2u}, {0xfbb9u, 3u, 7u, 2u},
    {0xfbbcu, 1u, 2u, 0u}, {0xfbbdu, 1u, 2u, 0u},
    {0xfbbeu, 3u, 3u, 0u}, {0xfbc1u, 2u, 3u, 0u}
};
static const FixedSpanOpcode capsule_backup_restore[] = {
    {0xfbc3u, 1u, 4u, 1u}, {0xfbc4u, 1u, 4u, 1u}
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
            clock->word_timing[0].cycles =
                (uint8_t)(5u + (0x8du + 2u * clock->word > 0xffu));
            clock->word_timing[5].cycles = clock->word == 95u ? 2u : 3u;
            clock->span.timing = clock->word_timing;
            clock->span.count = 6u;
        } else {
            clock->span.timing = capsule_backup_restore;
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

static uint64_t CapsuleBackupBound(const FixedSpanOpcode *timing,
                                    unsigned count, unsigned fetch_speed) {
    uint64_t master = 0u;
    for (unsigned index = 0u; index < count; ++index) {
        const FixedSpanOpcode *opcode = timing + index;
        master += opcode->fetched * fetch_speed + opcode->data_bytes * 8u +
            (opcode->cycles - opcode->fetched - opcode->data_bytes) * 6u;
    }
    return master;
}

static bool PrepareCapsuleBackup(CapsuleBackupClock *clock, CpuState *cpu) {
    if (cpu->m_flag != 1u || cpu->x_flag || cpu->host_return_valid != 2u ||
        cpu->S < 0x1f03u || cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x82fba7u, 0x1ff7u, false, 1363u, 64u) ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM ||
        !g_snes->cart->rom || g_snes->cart->romSize < 0x280000u ||
        g_snes->vPos >= 262u || !Lufia2DecompFrameRangeQuiet(0x82fba7u, 0x82fbc6u))
        return false;
    const unsigned speed = cpu_region_speed(0x82fba7u);
    const uint64_t work = CapsuleBackupBound(capsule_backup_begin, 9u, speed) +
        96u * CapsuleBackupBound(capsule_backup_word, 6u, speed) +
        CapsuleBackupBound(capsule_backup_restore, 2u, speed) + 64u;
    const uint64_t bound = work + 40u * (work / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - bound ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x82fba7u, 0x1ff7u, false, 1363u, bound))
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
    clock->span.timing = capsule_backup_begin;
    clock->span.count = 9u;
    for (unsigned index = 0u; index < 6u; ++index)
        clock->word_timing[index] = capsule_backup_word[index];
    return true;
}

static RecompReturn ActorBridgeCapsuleBackup(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CapsuleBackupClock clock;
    if (!PrepareCapsuleBackup(&clock, cpu))
        return ActorBridgeFallback(cpu, &frame, 0x82fba7u);
    const Lufia2Memory memory = {CapsuleBackupRead, CapsuleBackupWrite,
        &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    state.resume_pc = 0x82fba7u;
    const Lufia2ExecutionResult result = Lufia2MenuBackupInventory(&memory, &state);
    ActorBridgeStore(cpu, &state);
    AdvanceCapsuleBackup(&clock);
    assert(clock.phase == CAPSULE_BACKUP_DONE && !clock.span.remaining &&
        result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x82fbc5u);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
