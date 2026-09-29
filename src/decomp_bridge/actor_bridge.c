#include <stdint.h>

#include "cpu_state.h"
#include "lufia2/decomp.h"
#include "../lufia2_sprite_visibility.h"

typedef struct ActorBridgeFrame {
    uint16_t entry_s;
    uint8_t hrv;
    uint32_t host_return_pc24;
} ActorBridgeFrame;

static uint8_t ActorBridgeRead(void *context, uint32_t address) {
    return cpu_read8(
        (CpuState *)context, (uint8)(address >> 16), (uint16)address);
}

static void ActorBridgeWrite(void *context, uint32_t address, uint8_t value) {
    cpu_write8(
        (CpuState *)context, (uint8)(address >> 16), (uint16)address, value);
}

/* Same paired-return capture as a generated prologue. */
static ActorBridgeFrame ActorBridgeEnter(CpuState *cpu) {
    ActorBridgeFrame frame;

    frame.entry_s = cpu->S;
    frame.hrv = cpu->host_return_valid;
    frame.host_return_pc24 = 0xffffffffu;
    if (frame.hrv == 2 || frame.hrv == 3) {
        const uint16_t pcl =
            cpu_read8(cpu, 0x00, (uint16)(frame.entry_s + 1u));
        const uint16_t pch =
            cpu_read8(cpu, 0x00, (uint16)(frame.entry_s + 2u));
        const uint8_t pb = frame.hrv == 3
            ? cpu_read8(cpu, 0x00, (uint16)(frame.entry_s + 3u))
            : cpu->PB;
        frame.host_return_pc24 = ((uint32_t)pb << 16) |
            (uint16_t)((((pch << 8) | pcl) + 1u) & 0xffffu);
    }
    return frame;
}

/* Native only in M=1, native mode, binary arithmetic. */
static int ActorBridgeSupported(
    const CpuState *cpu, int allow_x8, int dp_page0) {
    return cpu->m_flag && !cpu->emulation && !cpu->_flag_D &&
           (allow_x8 || !cpu->x_flag) &&
           (!dp_page0 || !(cpu->D & 0xff00u));
}

static RecompReturn ActorBridgeFallback(
    CpuState *cpu, const ActorBridgeFrame *frame, uint32_t entry_pc24) {
    return interp_tier_dispatch_tail(
        cpu, entry_pc24, entry_pc24, frame->entry_s, frame->hrv);
}

static void ActorBridgeLoad(
    const CpuState *cpu, Lufia2CpuState *out) {
    out->accumulator = cpu->A;
    out->x = cpu->X;
    out->y = cpu->Y;
    out->stack = cpu->S;
    out->direct_page = cpu->D;
    out->data_bank = cpu->DB;
    out->program_bank = cpu->PB;
    out->carry = cpu->_flag_C;
    out->negative = cpu->_flag_N;
    out->zero = cpu->_flag_Z;
    out->accumulator_is_8_bit = cpu->m_flag;
    out->index_is_8_bit = cpu->x_flag;
    out->overflow = cpu->_flag_V;
    out->decimal = cpu->_flag_D;
    out->irq_disable = cpu->_flag_I;
}

static void ActorBridgeStore(
    CpuState *cpu, const Lufia2CpuState *in) {
    cpu->A = in->accumulator;
    cpu->X = in->x;
    cpu->Y = in->y;
    cpu->S = in->stack;
    cpu->D = in->direct_page;
    cpu->DB = in->data_bank;
    cpu->_flag_C = in->carry;
    cpu->_flag_N = in->negative;
    cpu->_flag_Z = in->zero;
    cpu->m_flag = in->accumulator_is_8_bit;
    cpu->x_flag = in->index_is_8_bit;
    cpu->_flag_V = in->overflow;
    cpu->_flag_D = in->decimal;
    cpu->_flag_I = in->irq_disable;
    cpu_mirrors_to_p(cpu);
}

/* Generated RTS/RTL tail for a balanced routine. */
static RecompReturn ActorBridgeReturn(
    CpuState *cpu,
    const ActorBridgeFrame *frame,
    uint8_t frame_size,
    uint32_t source_pc24) {
    const uint16_t ret_s = cpu->S;
    uint16_t pcl;
    uint16_t pch;
    uint8_t pb = cpu->PB;
    uint32_t rpc24;

    cpu->S = (uint16)(cpu->S + 1u);
    pcl = cpu_read8(cpu, 0x00, cpu->S);
    cpu->S = (uint16)(cpu->S + 1u);
    pch = cpu_read8(cpu, 0x00, cpu->S);
    if (frame_size == 3) {
        cpu->S = (uint16)(cpu->S + 1u);
        pb = cpu_read8(cpu, 0x00, cpu->S);
    }
    rpc24 = ((uint32_t)pb << 16) |
        (uint16_t)((((pch << 8) | pcl) + 1u) & 0xffffu);

    if (frame->hrv == frame_size && ret_s == frame->entry_s &&
        rpc24 != frame->host_return_pc24 &&
        !cpu_dispatch_has_entry(cpu, rpc24))
        return interp_tier_dispatch_rewritten_return(
            cpu, rpc24, source_pc24);
    if (frame->hrv == frame_size && ret_s == frame->entry_s &&
        rpc24 == frame->host_return_pc24)
        return RECOMP_RETURN_NORMAL;
    return cpu_dispatch_pc_from(
        cpu, rpc24, (uint16)(frame->entry_s + frame_size), source_pc24);
}

RecompReturn Lufia2DecompBridge_D350(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2CpuState state;

    /* TDC feeds DP high into the D370/D385 index. */
    if (!ActorBridgeSupported(cpu, 0, 1))
        return ActorBridgeFallback(cpu, &frame, 0x83d350u);
    ActorBridgeLoad(cpu, &state);
    /* D370/FB17 tables hold only known targets. */
    (void)Lufia2ActorPrimaryActionCore(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, 0x83d3aeu);
}

RecompReturn Lufia2DecompBridge_F9D4(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2CpuState state;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83f9d4u);
    ActorBridgeLoad(cpu, &state);
    Lufia2ActorResolveMapCellOffset(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, 0x83f9edu);
}

RecompReturn Lufia2DecompBridge_FB12(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2CpuState state;
    uint32_t rtl_pc24;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83fb12u);
    ActorBridgeLoad(cpu, &state);
    rtl_pc24 = Lufia2ActorMovementStep(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (rtl_pc24 == 0) {
        /* Unknown JMP ($FB1A,X) target: finish in LLE. */
        const uint16_t target = (uint16_t)(
            cpu_read8(cpu, cpu->PB, (uint16)(0xfb1au + cpu->X)) |
            ((uint16_t)cpu_read8(
                cpu, cpu->PB, (uint16)(0xfb1bu + cpu->X)) << 8));
        return interp_tier_dispatch_tail(
            cpu, ((uint32_t)cpu->PB << 16) | target, 0x83fb17u,
            frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 3, rtl_pc24);
}

RecompReturn Lufia2DecompBridge_FB71(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2CpuState state;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83fb71u);
    ActorBridgeLoad(cpu, &state);
    Lufia2ActorReadMapCellValue(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, 0x83fb8au);
}

typedef Lufia2ExecutionResult (*ActorWholeFunction)(
    const Lufia2Memory *memory, Lufia2CpuState *cpu);
typedef void (*ActorVoidFunction)(
    const Lufia2Memory *memory, Lufia2CpuState *cpu);

/* A complete leaf with its return instruction outside the portable API. */
static RecompReturn ActorBridgeRunVoid(
    CpuState *cpu, uint32_t entry_pc24, ActorVoidFunction run,
    uint8_t frame_size, uint32_t return_pc24, int any_width) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2CpuState state;

    if (cpu->emulation || (!any_width &&
        (cpu->_flag_D || !cpu->m_flag || cpu->x_flag)))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    run(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, frame_size, return_pc24);
}

/* Whole JSR routine with exact LLE boundaries. */
static RecompReturn ActorBridgeRunWhole(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size, int any_width) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (any_width == 4 ? !ActorBridgeSupported(cpu, 0, 0)
        : any_width == 3 ? cpu->emulation || cpu->_flag_D || cpu->x_flag
        : any_width == 2 ? cpu->emulation || cpu->_flag_D ||
                         cpu->m_flag || cpu->x_flag
        : any_width ? cpu->emulation || cpu->_flag_D
                    : !ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = run(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        /* Exact ROM state at result.pc; LLE finishes the RTS. */
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

static RecompReturn ActorBridgeWhole(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    return ActorBridgeRunWhole(cpu, entry_pc24, run, frame_size, 0);
}

/* Routine that sets its own widths (PHP first). */
static RecompReturn ActorBridgeWholeAnyWidth(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    return ActorBridgeRunWhole(cpu, entry_pc24, run, frame_size, 1);
}

/* Routine entered with M0X0 only. */
static RecompReturn ActorBridgeWholeM0X0(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    return ActorBridgeRunWhole(cpu, entry_pc24, run, frame_size, 2);
}

static RecompReturn ActorBridgeWholeM1X16(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    return ActorBridgeRunWhole(cpu, entry_pc24, run, frame_size, 4);
}

RecompReturn Lufia2DecompBridge_C7F8(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83c7f8u, Lufia2ActorPrimaryUpdate, 2);
}

RecompReturn Lufia2DecompBridge_D508(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83d508u, Lufia2ActorSecondaryUpdate, 2);
}

RecompReturn Lufia2DecompBridge_A21A(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83a21au, Lufia2PatchedFieldActorSprites, 3);
}

typedef struct ActorSlotsCall {
    CpuState *cpu;
    RecompReturn unwound;
} ActorSlotsCall;

/* BB93 child through the runtime dispatcher. */
static uint8_t ActorBridgeSlotChild(
    void *context,
    Lufia2CpuState *state,
    uint32_t target,
    uint32_t site) {
    ActorSlotsCall *call = (ActorSlotsCall *)context;
    RecompReturn result;

    ActorBridgeStore(call->cpu, state);
    result = cpu_dispatch_call_pc(call->cpu, target, site);
    if (result != RECOMP_RETURN_NORMAL) {
        call->unwound = result;
        return 0;
    }
    ActorBridgeLoad(call->cpu, state);
    return 1;
}

RecompReturn Lufia2DecompBridge_BB93(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorSlotsCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83bb93u);
    call.cpu = cpu;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2UpdateActorSlots(
        &memory, &state, ActorBridgeSlotChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_81C6(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8381c6u, Lufia2FieldTriggerUpdate, 2);
}

RecompReturn Lufia2DecompBridge_E03E(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83e03eu, Lufia2ObjectSlotsUpdate, 2);
}

RecompReturn Lufia2DecompBridge_9FA9(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x839fa9u, Lufia2FieldNmiUploads, 3);
}

RecompReturn Lufia2DecompBridge_BD77(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8ebd77u, Lufia2FieldScrollUpdate, 3);
}

RecompReturn Lufia2DecompBridge_80CD(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8380cdu, Lufia2FieldIdleTest, 2);
}

RecompReturn Lufia2DecompBridge_8682(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x838682u, Lufia2FieldAnimationTicks, 2);
}

RecompReturn Lufia2DecompBridge_AEB5(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83aeb5u, Lufia2FieldColourEffects, 2);
}

RecompReturn Lufia2DecompBridge_9C72(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x809c72u, Lufia2FieldEventTick, 3);
}

RecompReturn Lufia2DecompBridge_CBAE(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x80cbaeu, Lufia2FieldEventTimerTick, 3);
}

RecompReturn Lufia2DecompBridge_A9BA(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83a9bau, Lufia2ActorLoadSprite, 3);
}

RecompReturn Lufia2DecompBridge_8193(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x848193u, Lufia2SpriteGraphicsUpload, 3);
}

RecompReturn Lufia2DecompBridge_88BE(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8688beu, Lufia2TitleParticleSprites, 2);
}

RecompReturn Lufia2DecompBridge_838C(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x86838cu, Lufia2TitleObjects, 2);
}

RecompReturn Lufia2DecompBridge_86ED(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8686edu, Lufia2TitleLayers, 2);
}

RecompReturn Lufia2DecompBridge_8996(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x868996u, Lufia2TitlePaletteCycle, 2);
}

RecompReturn Lufia2DecompBridge_E8CE(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x86e8ceu, Lufia2WorldSpriteChain, 2);
}

RecompReturn Lufia2DecompBridge_F9E9(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81f9e9u, Lufia2PartyExperienceForLevel, 3);
}

RecompReturn Lufia2DecompBridge_ED9C(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x80ed9cu, Lufia2FieldBuildAttributes, 3);
}

RecompReturn Lufia2DecompBridge_8E9D(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x808e9du, Lufia2DecompressResource, 3);
}

RecompReturn Lufia2DecompBridge_8878(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x808878u, Lufia2MenuDrawString, 3);
}

RecompReturn Lufia2DecompBridge_F1C5(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x81f1c5u, Lufia2LoadItemRecord, 3);
}

RecompReturn Lufia2DecompBridge_F414(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81f414u, Lufia2LoadSpellRecord, 3);
}

RecompReturn Lufia2DecompBridge_C261(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x82c261u, Lufia2CapsuleLoadStats, 3);
}

RecompReturn Lufia2DecompBridge_F4D5(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x81f4d5u, Lufia2PartyDerivedStats, 3);
}

RecompReturn Lufia2DecompBridge_C515(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x82c515u, Lufia2CapsuleSetForms, 3);
}

RecompReturn Lufia2DecompBridge_C352(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x82c352u, Lufia2CapsuleSetAll, 3);
}

RecompReturn Lufia2DecompBridge_8B55(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x868b55u, Lufia2SpriteFrame, 3);
}

RecompReturn Lufia2DecompBridge_8B73(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x868b73u, Lufia2SpriteAnimateAll, 3);
}

RecompReturn Lufia2DecompBridge_8BCF(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x868bcfu, Lufia2SpriteClearOam, 3);
}

RecompReturn Lufia2DecompBridge_8BF5(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x868bf5u, Lufia2SpriteBuildOam, 3);
}

RecompReturn Lufia2DecompBridge_C35F(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x81c35fu, Lufia2BattlePopups, 3);
}

RecompReturn Lufia2DecompBridge_B264(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b264u, Lufia2BattleActiveMask, 2);
}

RecompReturn Lufia2DecompBridge_B2B5(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b2b5u, Lufia2BattleTargetRecord, 3);
}

RecompReturn Lufia2DecompBridge_B2DB(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b2dbu, Lufia2BattleTargetSlot, 3);
}

RecompReturn Lufia2DecompBridge_B505(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b505u, Lufia2BattleBlend, 2);
}

RecompReturn Lufia2DecompBridge_B54A(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x81b54au, Lufia2ColorToGray, 2);
}

RecompReturn Lufia2DecompBridge_B5A3(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b5a3u, Lufia2BattleHideOam, 2);
}

RecompReturn Lufia2DecompBridge_B974(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b974u, Lufia2BattleLoadPalette, 2);
}

RecompReturn Lufia2DecompBridge_B9AF(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81b9afu, Lufia2BattleCommitPalettes, 3);
}

RecompReturn Lufia2DecompBridge_C2C0(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2c0u, Lufia2BattleCopyC2C0, 2);
}

RecompReturn Lufia2DecompBridge_C2D0(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2d0u, Lufia2BattleClear2000, 2);
}

RecompReturn Lufia2DecompBridge_C2E3(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2e3u, Lufia2BattleFill2800, 2);
}

RecompReturn Lufia2DecompBridge_C2FB(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2fbu, Lufia2BattleClear3000, 2);
}

RecompReturn Lufia2DecompBridge_C30E(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c30eu, Lufia2BattleClear3800, 2);
}

RecompReturn Lufia2DecompBridge_C5CF(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c5cfu, Lufia2BattleTargetPointer, 2);
}

RecompReturn Lufia2DecompBridge_BE58(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81be58u, Lufia2BattleTileBlock, 2);
}

RecompReturn Lufia2DecompBridge_BD4B(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81bd4bu, Lufia2BattleSpriteBlock, 2);
}

RecompReturn Lufia2DecompBridge_E479(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x81e479u, Lufia2BattleFrameTop, 2);
}

RecompReturn Lufia2DecompBridge_E4AD(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x81e4adu, Lufia2BattleFrameSides, 2);
}

RecompReturn Lufia2DecompBridge_E542(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x81e542u, Lufia2BattleFrameRow, 2);
}

RecompReturn Lufia2DecompBridge_E570(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x81e570u, Lufia2BattleFrameEnds, 2);
}

RecompReturn Lufia2DecompBridge_E5C1(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x81e5c1u, Lufia2BattleGaugeBlock, 2);
}

RecompReturn Lufia2DecompBridge_E604(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x81e604u, Lufia2BattleGaugeColumn, 2);
}

RecompReturn Lufia2DecompBridge_F291(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x81f291u, Lufia2ItemTextPointer, 3);
}

RecompReturn Lufia2DecompBridge_F446(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f446u, Lufia2SpellTextPointer, 2);
}

RecompReturn Lufia2DecompBridge_F5ED(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f5edu, Lufia2PartyRestore11, 3);
}

RecompReturn Lufia2DecompBridge_F60B(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f60bu, Lufia2PartyRestore13, 3);
}

RecompReturn Lufia2DecompBridge_F7BD(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x81f7bdu, Lufia2BattleTable9EBA, 2, 3);
}

RecompReturn Lufia2DecompBridge_FB79(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81fb79u, Lufia2CharacterSpriteByte, 3);
}

RecompReturn Lufia2DecompBridge_EC41(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81ec41u, Lufia2BattleClearF000, 2);
}

RecompReturn Lufia2DecompBridge_F2A9(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f2a9u, Lufia2ItemNameTrimmed, 3);
}

RecompReturn Lufia2DecompBridge_F4ED(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f4edu, Lufia2PartyStatTotals, 2);
}

RecompReturn Lufia2DecompBridge_F78D(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f78du, Lufia2PartyPointers, 2);
}

RecompReturn Lufia2DecompBridge_FBA2(CpuState *cpu) {
    /* PHX precedes REP #$30; an X8 entry cannot restore its stack frame. */
    return ActorBridgeRunWhole(cpu, 0x81fba2u, Lufia2SpriteSizePacked, 3, 3);
}

RecompReturn Lufia2DecompBridge_FBDB(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81fbdbu, Lufia2CharacterSpriteBox, 3);
}

RecompReturn Lufia2DecompBridge_FCE2(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81fce2u, Lufia2CharacterSpritePointer, 3);
}

RecompReturn Lufia2DecompBridge_E7D2(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e7d2u, Lufia2BattleFillRect, 2);
}

RecompReturn Lufia2DecompBridge_E808(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e808u, Lufia2DecimalDigits3, 2);
}

RecompReturn Lufia2DecompBridge_EB34(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81eb34u, Lufia2BattlePaletteCopy, 2);
}

RecompReturn Lufia2DecompBridge_EB62(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81eb62u, Lufia2BattlePaletteSplit, 2);
}

RecompReturn Lufia2DecompBridge_F057(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f057u, Lufia2InventoryCount, 3);
}

RecompReturn Lufia2DecompBridge_E503(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e503u, Lufia2BattleWindow, 2);
}

RecompReturn Lufia2DecompBridge_E593(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e593u, Lufia2BattleGaugePanel, 2);
}

RecompReturn Lufia2DecompBridge_F4E9(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f4e9u, Lufia2PartyStatTotalsFar, 3);
}

RecompReturn Lufia2DecompBridge_F789(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f789u, Lufia2PartyPointersFar, 3);
}

RecompReturn Lufia2DecompBridge_BD47(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81bd47u, Lufia2BattleSpriteBlockFar, 3);
}

RecompReturn Lufia2DecompBridge_BE54(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81be54u, Lufia2BattleTileBlockFar, 3);
}

RecompReturn Lufia2DecompBridge_E3AE(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e3aeu, Lufia2BattleWindowE3AE, 3);
}

RecompReturn Lufia2DecompBridge_E3CD(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e3cdu, Lufia2BattleWindowE3CD, 3);
}

RecompReturn Lufia2DecompBridge_BAE8(CpuState *cpu) {
    if (cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);

        return ActorBridgeFallback(cpu, &frame, 0x81bae8u);
    }
    return ActorBridgeWholeM1X16(cpu, 0x81bae8u, Lufia2BattlePortraits, 3);
}

RecompReturn Lufia2DecompBridge_BAFB(CpuState *cpu) {
    if (cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);

        return ActorBridgeFallback(cpu, &frame, 0x81bafbu);
    }
    return ActorBridgeWholeM1X16(cpu, 0x81bafbu, Lufia2BattlePortrait, 2);
}

RecompReturn Lufia2DecompBridge_BB75(CpuState *cpu) {
    if (cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);

        return ActorBridgeFallback(cpu, &frame, 0x81bb75u);
    }
    return ActorBridgeWholeM1X16(cpu, 0x81bb75u, Lufia2BattlePortraitUpload, 2);
}

RecompReturn Lufia2DecompBridge_B48B(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81b48bu, Lufia2BattleFadeColor, 2);
}

RecompReturn Lufia2DecompBridge_B444(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81b444u, Lufia2BattlePaletteFade, 3);
}

RecompReturn Lufia2DecompBridge_E405(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e405u, Lufia2BattleTileFrame, 2);
}

RecompReturn Lufia2DecompBridge_E3EC(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e3ecu, Lufia2BattleTileWindow, 2);
}

RecompReturn Lufia2DecompBridge_F979(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f979u, Lufia2PartyLevelUpCheck, 2);
}

RecompReturn Lufia2DecompBridge_FC0B(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81fc0bu, Lufia2PartyNewRecord, 3);
}

RecompReturn Lufia2DecompBridge_8DC5(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x858dc5u, Lufia2BattleNmiUploads, 3);
}

RecompReturn Lufia2DecompBridge_CEF6(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x86cef6u, Lufia2WorldMapNmiUploads, 3);
}

RecompReturn Lufia2DecompBridge_ECDB(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x85ecdbu, Lufia2BattleVramQueueSlot, 3);
}

RecompReturn Lufia2DecompBridge_99BF(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8699bfu, Lufia2WorldMapStreamEdges, 2);
}

RecompReturn Lufia2DecompBridge_83A0(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8383a0u, Lufia2FieldMenuRequest, 2);
}

RecompReturn Lufia2DecompBridge_867B(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83867bu, Lufia2FieldTakeButtons, 2);
}

RecompReturn Lufia2DecompBridge_8103(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x838103u, Lufia2FieldStatusRequests, 2);
}

RecompReturn Lufia2DecompBridge_E746(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x82e746u, Lufia2TitleStateDispatch, 3);
}

RecompReturn Lufia2DecompBridge_85DC(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8385dcu, Lufia2FieldReloadSetup, 3);
}

RecompReturn Lufia2DecompBridge_939C(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x82939cu, Lufia2MenuNmi, 3);
}

RecompReturn Lufia2DecompBridge_81A9(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8681a9u, Lufia2SelectScreenNmi, 3);
}

RecompReturn Lufia2DecompBridge_92A4(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8092a4u, Lufia2IntroNmi, 3);
}

RecompReturn Lufia2DecompBridge_B452(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x85b452u, Lufia2BattleScript, 3);
}

RecompReturn Lufia2DecompBridge_8B4B(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x828b4bu, Lufia2MenuButtons, 3);
}

RecompReturn Lufia2DecompBridge_9313(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x829313u, Lufia2MenuWindowRequest, 2);
}

RecompReturn Lufia2DecompBridge_C627(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x82c627u, Lufia2MenuCursorBlink, 2);
}

RecompReturn Lufia2DecompBridge_B66E(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83b66eu, Lufia2FieldStairRects, 2);
}

RecompReturn Lufia2DecompBridge_B711(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83b711u, Lufia2FieldEventRects, 2);
}

RecompReturn Lufia2DecompBridge_B747(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83b747u, Lufia2FieldAreaRects, 2);
}

RecompReturn Lufia2DecompBridge_86C1(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8086c1u, Lufia2ScreenFade, 2);
}

RecompReturn Lufia2DecompBridge_9EDD(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x869eddu, Lufia2WorldMapRegionSearch, 2);
}

RecompReturn Lufia2DecompBridge_9CB8(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x809cb8u, Lufia2TextEngineStep, 3);
}

RecompReturn Lufia2DecompBridge_8A2F(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x858a2fu, Lufia2BattleSprites, 3);
}

RecompReturn Lufia2DecompBridge_ECF0(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x85ecf0u, Lufia2BattleFrameUpkeep, 3);
}

RecompReturn Lufia2DecompBridge_C1B4(CpuState *cpu) {
    return ActorBridgeWhole(
        cpu, 0x83c1b4u, Lufia2PlayerSlotStandardUpdate, 2);
}

/* S19 standalone contracts: M1X16, native mode, binary arithmetic. */
RecompReturn Lufia2DecompBridge_F0A2(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f0a2u, Lufia2InventoryAdd, 2);
}

RecompReturn Lufia2DecompBridge_E835(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81e835u, Lufia2BattleGlyph, 2);
}

/* Both RNG leaves save P before changing M/X and restore it before RTL. */
RecompReturn Lufia2DecompBridge_8299(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x808299u, Lufia2RandomScale, 3, 0x8082c6u, 1);
}

RecompReturn Lufia2DecompBridge_82C7(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x8082c7u, Lufia2RandomByte, 3, 0x8082e6u, 1);
}

RecompReturn Lufia2DecompBridge_A746(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83a746u, Lufia2ActorSyncFinePosition, 3, 0x83a76cu, 0);
}

RecompReturn Lufia2DecompBridge_C947(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83c947u, Lufia2ActorPrimaryReset, 2, 0x83c989u, 0);
}

RecompReturn Lufia2DecompBridge_CA68(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83ca68u, Lufia2ActorBlockedEvent, 3, 0x83ca92u, 0);
}

RecompReturn Lufia2DecompBridge_CB65(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83cb65u, Lufia2ActorClearSlotLinks, 2, 0x83cb70u, 0);
}

RecompReturn Lufia2DecompBridge_D416(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83d416u, Lufia2ActorLoadPrimaryScript, 3, 0x83d436u, 0);
}

RecompReturn Lufia2DecompBridge_FA3F(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83fa3fu, Lufia2ActorMarkMapOccupancy, 3, 0x83fa80u, 0);
}

RecompReturn Lufia2DecompBridge_FA81(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83fa81u, Lufia2ActorMoveFinePosition, 2, 0x83facau, 0);
}

RecompReturn Lufia2DecompBridge_FACB(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x83facbu, Lufia2ActorAddDisplayOffset, 2, 0x83faf3u, 0);
}

RecompReturn Lufia2DecompBridge_8766(CpuState *cpu) {
    return ActorBridgeRunVoid(
        cpu, 0x848766u, Lufia2QueueDeferredSound, 3, 0x848774u, 0);
}

RecompReturn Lufia2DecompBridge_8378(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x808378u, Lufia2Divide16, 3);
}

RecompReturn Lufia2DecompBridge_8CDA(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x868cdau, Lufia2SpriteSetTable, 3);
}

RecompReturn Lufia2DecompBridge_8CF5(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x868cf5u, Lufia2SpriteSetAnimation, 3);
}

RecompReturn Lufia2DecompBridge_F194(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f194u, Lufia2ItemRecordByte, 3);
}

RecompReturn Lufia2DecompBridge_F3F4(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f3f4u, Lufia2SpellRecordByteC, 3);
}

RecompReturn Lufia2DecompBridge_F404(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f404u, Lufia2SpellRecordByte8, 3);
}

RecompReturn Lufia2DecompBridge_F87F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f87fu, Lufia2PartyBaseStats, 3);
}

RecompReturn Lufia2DecompBridge_CE23(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x82ce23u, Lufia2CapsuleExperienceRange, 2);
}

RecompReturn Lufia2DecompBridge_810E(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82810eu, Lufia2MenuDrawWindow, 2);
}

RecompReturn Lufia2DecompBridge_C2FD(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82c2fdu, Lufia2CapsuleReset, 3);
}

RecompReturn Lufia2DecompBridge_CD83(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82cd83u, Lufia2CapsuleLevelUp, 3);
}

RecompReturn Lufia2DecompBridge_9F6F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x829f6fu, Lufia2MenuEquipCommands, 2);
}

RecompReturn Lufia2DecompBridge_D721(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82d721u, Lufia2MenuShopWindows, 2);
}

RecompReturn Lufia2DecompBridge_E49E(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82e49eu, Lufia2MenuShopTitle, 2);
}

RecompReturn Lufia2DecompBridge_EFC5(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82efc5u, Lufia2MenuSavedWindow, 2);
}

RecompReturn Lufia2DecompBridge_82F0A2(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82f0a2u, Lufia2MenuNameEntryWindows, 2);
}

RecompReturn Lufia2DecompBridge_D749(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82d749u, Lufia2MenuShopParty, 2);
}

RecompReturn Lufia2DecompBridge_A2E3(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82a2e3u, Lufia2MenuCapsuleScreen, 2);
}

RecompReturn Lufia2DecompBridge_950E(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82950eu, Lufia2MenuMemberStatus, 2);
}

RecompReturn Lufia2DecompBridge_D07B(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82d07bu, Lufia2MenuCapsuleStatus, 2);
}

RecompReturn Lufia2DecompBridge_E297(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82e297u, Lufia2MenuShopSetup, 2);
}

RecompReturn Lufia2DecompBridge_CD1F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82cd1fu, Lufia2CapsuleTryLearn, 3);
}

RecompReturn Lufia2DecompBridge_E5E1(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82e5e1u, Lufia2MenuShopCompare, 2);
}

RecompReturn Lufia2DecompBridge_DCF4(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82dcf4u, Lufia2MenuShopRow, 2);
}

RecompReturn Lufia2DecompBridge_DCC1(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82dcc1u, Lufia2MenuShopRows, 2);
}

RecompReturn Lufia2DecompBridge_B2C5(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82b2c5u, Lufia2MenuEquipUpgrade, 2);
}

RecompReturn Lufia2DecompBridge_9CB2(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x829cb2u, Lufia2MenuWarpList, 2);
}

RecompReturn Lufia2DecompBridge_A918(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82a918u, Lufia2MenuListCursor, 2);
}

RecompReturn Lufia2DecompBridge_ACDB(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82acdbu, Lufia2MenuListRow, 2);
}

RecompReturn Lufia2DecompBridge_ED8E(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81ed8eu, Lufia2PartyUnpackMember, 2);
}

RecompReturn Lufia2DecompBridge_EE94(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81ee94u, Lufia2PartyUnpackMemberBare, 2);
}

RecompReturn Lufia2DecompBridge_C129(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81c129u, Lufia2BattleIpSkills, 2);
}

RecompReturn Lufia2DecompBridge_DFA2(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81dfa2u, Lufia2BattleListRows, 2);
}

RecompReturn Lufia2DecompBridge_C652(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80c652u, Lufia2TextMeasure, 2);
}

RecompReturn Lufia2DecompBridge_C784(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80c784u, Lufia2TextClearGlyphBuffer, 3);
}

RecompReturn Lufia2DecompBridge_C56E(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80c56eu, Lufia2TextQueueWindowRow, 2);
}

RecompReturn Lufia2DecompBridge_C23D(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80c23du, Lufia2TextPrepareWindow, 2);
}

RecompReturn Lufia2DecompBridge_FB1F(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fb1fu, Lufia2ItemPossessionCount, 3);
}

RecompReturn Lufia2DecompBridge_C305(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80c305u, Lufia2TextBuildWindow, 2);
}

typedef struct ActorPushedCall {
    CpuState *cpu;
    ActorBridgeFrame frame;
    RecompReturn unwound;
} ActorPushedCall;

/* The cave function has already pushed the child's JSL frame. */
static uint8_t ActorBridgePushedChild(
    void *context, Lufia2CpuState *state, uint32_t target,
    uint32_t site, uint8_t frame_size) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    const uint16_t post_s = (uint16_t)(state->stack + frame_size);
    const uint32_t landing = (site & 0xff0000u) |
        (uint16_t)(site + (frame_size == 3u ? 4u : 3u));
    uint32_t return_pc24 = landing;
    RecompReturn result;

    ActorBridgeStore(call->cpu, state);
    result = cpu_dispatch_call_pc_pushed(
        call->cpu, target, site, frame_size, &return_pc24);
    if (result != RECOMP_RETURN_NORMAL) {
        call->unwound = result;
        return 0;
    }
    if (call->cpu->S != post_s || return_pc24 != landing) {
        call->unwound = interp_tier_dispatch_tail(
            call->cpu, return_pc24, site,
            call->frame.entry_s, call->frame.hrv);
        return 0;
    }
    ActorBridgeLoad(call->cpu, state);
    state->program_bank = call->cpu->PB;
    return 1;
}

RecompReturn Lufia2DecompBridge_9E31(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->D != 0)
        return ActorBridgeFallback(cpu, &frame, 0x839e31u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2AncientCaveGenerateFloor(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_83E0(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x8383e0u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2FieldEncounterHandoff(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_845B(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83845bu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2EncounterBattleSequence(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_8BC7(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x848bc7u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleVisualTransition(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_8821(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x818821u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleEntry(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_8000(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x818000u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleSetup(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_B9C7(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x81b9c7u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleBackgroundPrepare(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_876B(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x81876bu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleExit(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_B062(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83b062u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2FieldRestore(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}
