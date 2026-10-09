#include <stdint.h>

#include "cpu_state.h"
#include "lufia2/decomp.h"
#include "decomp_bridge/host_events.h"
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

/* Inlined semantic children retain their caller bank between JSL/RTL frames. */
static void ActorBridgeExecutionCheckpoint(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    CpuState *cpu = (CpuState *)context;
    const uint8_t host_bank = cpu->PB;
    const uint8_t semantic_bank = state->program_bank;
    ActorBridgeStore(cpu, state);
    cpu->PB = (uint8_t)(pc >> 16);
    if (pc == 0x81f4e2u)
        Lufia2DecompPartyStatsDerived(cpu, pc);
    else if (pc == 0x81f576u)
        Lufia2DecompPartyStatTotals(cpu, pc);
    else if (pc == 0x81f099u)
        Lufia2DecompItemReceived(cpu, pc);
    else if (pc == 0x84890bu)
        Lufia2DecompCaveExitBegin(cpu, pc);
    else if (pc == 0x848a38u)
        Lufia2DecompCaveExitRestored(cpu, pc);
    else if (pc == 0x848b9cu)
        Lufia2DecompCaveDefeatBegin(cpu, pc);
    else if (pc == 0x848ba5u)
        Lufia2DecompCaveDefeatReset(cpu, pc);
    ActorBridgeLoad(cpu, state);
    cpu->PB = host_bank;
    state->program_bank = semantic_bank;
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (any_width == 5 ? cpu->emulation || cpu->_flag_D || cpu->m_flag
        : any_width == 4 ? !ActorBridgeSupported(cpu, 0, 0)
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
        cpu->PB = state.program_bank;
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

static RecompReturn ActorBridgeWholeM0(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    return ActorBridgeRunWhole(cpu, entry_pc24, run, frame_size, 5);
}

/* Routine that sets its accumulator width; X16 only. */
static RecompReturn ActorBridgeWholeX16(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    return ActorBridgeRunWhole(cpu, entry_pc24, run, frame_size, 3);
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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

static RecompReturn ActorBridgeAnimationSlots(CpuState *cpu);

RecompReturn Lufia2DecompBridge_8682(CpuState *cpu) {
    return ActorBridgeAnimationSlots(cpu);
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

static void ActorBridgeMenuNumber(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    CpuState *cpu = (CpuState *)context;
    ActorBridgeStore(cpu, state);
    Lufia2DecompMenuNumber(cpu, pc);
    ActorBridgeLoad(cpu, state);
}

static Lufia2ExecutionResult ActorBridgeDrawString(
    const Lufia2Memory *memory, Lufia2CpuState *state) {
    return Lufia2MenuDrawStringWithCheckpoint(
        memory, state, ActorBridgeMenuNumber, memory->context);
}

RecompReturn Lufia2DecompBridge_8878(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x808878u, ActorBridgeDrawString, 3);
}

RecompReturn Lufia2DecompBridge_F1C5(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x81f1c5u, Lufia2LoadItemRecord, 3);
}

RecompReturn Lufia2DecompBridge_F414(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81f414u, Lufia2LoadSpellRecord, 3);
}

RecompReturn Lufia2DecompBridge_C261(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82c261u, Lufia2CapsuleLoadStats, 3);
}

RecompReturn Lufia2DecompBridge_83F9F7(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83f9f7u, Lufia2MapCellOffset, 2u);
}

RecompReturn Lufia2DecompBridge_83F9D9(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x83f9d9u, Lufia2LayerCellOffset, 2u);
}

RecompReturn Lufia2DecompBridge_83F91F(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x83f91fu, Lufia2FieldCopyCellTile, 2u);
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
    return ActorBridgeWhole(cpu, 0x81c2c0u, Lufia2BattleLoadDisplayDefaults, 2);
}

RecompReturn Lufia2DecompBridge_C2D0(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2d0u, Lufia2BattleClearBackgroundTilemap, 2);
}

RecompReturn Lufia2DecompBridge_C2E3(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2e3u, Lufia2BattleResetPartyTilemap, 2);
}

RecompReturn Lufia2DecompBridge_C2FB(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c2fbu, Lufia2BattleClearWindowTilemap, 2);
}

RecompReturn Lufia2DecompBridge_C30E(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x81c30eu, Lufia2BattleClearTilemap3800, 2);
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
    return ActorBridgeWhole(cpu, 0x868cdau, Lufia2SpriteSetTable, 3);
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
    return ActorBridgeWholeX16(cpu, 0x81ed8eu, Lufia2PartyUnpackMember, 2);
}

RecompReturn Lufia2DecompBridge_EE94(CpuState *cpu) {
    return ActorBridgeWholeX16(cpu, 0x81ee94u, Lufia2PartyUnpackMemberBare, 2);
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

/* The portable caller has already pushed the child's call frame. */
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
    call->cpu->PB = (uint8_t)(target >> 16);
    /* Reload children can yield while NMI is disabled. A missing native
     * entry must continue in the owning interpreter, rather than start a
     * nested call interpreter that can consume the scheduler's deadline. */
    if (site >= 0x838637u && site <= 0x83866du &&
        !cpu_dispatch_has_entry(call->cpu, target)) {
        result = interp_tier_dispatch_tail(
            call->cpu, target, site,
            call->frame.entry_s, call->frame.hrv);
        /* The parent removes one child unwind level. This handoff already
         * represents the parent's continuation, so preserve its result. */
        call->unwound = (RecompReturn)((int)result + 1);
        return 0;
    }
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
    call->cpu->PB = (uint8_t)(landing >> 16);
    ActorBridgeLoad(call->cpu, state);
    state->program_bank = call->cpu->PB;
    return 1;
}

RecompReturn Lufia2DecompBridge_9E31(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
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

RecompReturn Lufia2DecompBridge_851E(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x81851eu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleDisplaySetup(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_886F(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x81886fu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleMainLoop(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_C240(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x81c240u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattlePrepareNextFrame(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

typedef Lufia2ExecutionResult (*BattleControlFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeBattleControl(
    CpuState *cpu, uint32_t entry, BattleControlFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->D != 0 || cpu->DB != 0x97u)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    result = run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_C739(CpuState *cpu) {
    return ActorBridgeBattleControl(cpu, 0x81c739u, Lufia2BattleCollectCommands);
}

RecompReturn Lufia2DecompBridge_890A(CpuState *cpu) {
    return ActorBridgeBattleControl(cpu, 0x81890au, Lufia2BattleExecuteTurns);
}


/* Battle entries share call-frame handling, with explicit entry contracts. */
typedef enum BattleBridgeWidth {
    BATTLE_BRIDGE_M0X0 = 0,
    BATTLE_BRIDGE_M1X0 = 1,
    BATTLE_BRIDGE_ANY_WIDTH = 2
} BattleBridgeWidth;

typedef enum BattleBridgeBank {
    BATTLE_BRIDGE_ANY_BANK = -1,
    BATTLE_BRIDGE_WRAM_BANK = -2,
    BATTLE_BRIDGE_REGISTER_BANK = -3
} BattleBridgeBank;

static int ActorBridgeBattleSupported(const CpuState *cpu, uint32_t entry,
                                      BattleBridgeWidth width, int any_dp, int db,
                                      int nonzero_mask) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != (uint8_t)(entry >> 16) ||
        (width != BATTLE_BRIDGE_ANY_WIDTH && (cpu->m_flag != width || cpu->x_flag)) ||
        (!any_dp && cpu->D != 0) || (nonzero_mask && !(cpu->A & 0xffu)))
        return 0;
    if (db >= 0)
        return cpu->DB == db;
    if (db == BATTLE_BRIDGE_ANY_BANK)
        return 1;
    /* $7E also maps low WRAM, but has no SNES register mirror. */
    return (cpu->DB & 0x7fu) < 0x40u ||
           (db == BATTLE_BRIDGE_WRAM_BANK && cpu->DB == 0x7eu);
}

static RecompReturn ActorBridgeBattleEntry(CpuState *cpu, uint32_t entry,
                                           ActorWholeFunction leaf,
                                           BattleControlFunction parent,
                                           unsigned frame_size, unsigned width,
                                           int any_dp, int db, int nonzero_mask) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeBattleSupported(cpu, entry, width, any_dp, db, nonzero_mask))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    result = parent ? parent(&memory, &state, ActorBridgePushedChild, &call)
                    : leaf(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s,
                                         frame.hrv);
    /* The sprite builder has a real asynchronous RTS alternative. */
    if (entry == 0x81b5c4u && result.pc == 0x81b69fu)
        frame_size = 2u;
    return ActorBridgeReturn(cpu, &frame, (uint8_t)frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_9236(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859236u, Lufia2BattlePartyStatusGate, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_96A2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8596a2u, Lufia2BattleSaveWorkArea, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_96B0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8596b0u, Lufia2BattleRestoreWorkArea, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_AB78(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85ab78u, Lufia2BattleStageTransfer, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_89E5(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8589e5u, Lufia2BattleClearSpriteOffsets, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9275(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859275u, Lufia2BattleQueuePartyTurns, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_C254(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81c254u, Lufia2BattleQueueEnemyTurns, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_C294(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81c294u, Lufia2BattleQueueCapsuleTurn, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_93B7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8593b7u, Lufia2BattleCheckOutcome, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_A79A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81a79au, 0, Lufia2BattlePrepareAction, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_C600(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81c600u, 0, Lufia2BattleStatusTick, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_CB77(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81cb77u, 0, Lufia2BattleChooseCommand, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_CC2E(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81cc2eu, 0, Lufia2BattleChoosePartyAction, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_D12F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d12fu, 0, Lufia2BattleActionSubmenuStart, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_D19A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d19au, 0, Lufia2BattleActionSubmenuResume,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_D4E0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d4e0u, 0, Lufia2BattleChooseTargets, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_B8B1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b8b1u, 0, Lufia2BattleTargetCoordinates, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_D920(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d920u, Lufia2BattleEnemyCursor, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_D92C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d92cu, Lufia2BattlePartyCursor, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_D938(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d938u, Lufia2BattleEnemyMarkedCursor, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_D948(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d948u, Lufia2BattlePartyMarkedCursor, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_D9D0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d9d0u, 0, Lufia2BattleCommandFrame, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_D975(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d975u, 0, Lufia2BattleConfirmCommand, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_BF3F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81bf3fu, 0, Lufia2BattleItemCommands, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_C031(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81c031u, 0, Lufia2BattleSpellCommands, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_BEBC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81bebcu, Lufia2BattleCommandTiles, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_BEED(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81beedu, Lufia2BattleActionMenuTiles, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_DEF4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81def4u, 0, Lufia2BattleActionWindow, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_DF0A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81df0au, 0, Lufia2BattlePartyWindows, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_E16F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e16fu, 0, Lufia2BattleClearActionWindow, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_E4D1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e4d1u, 0, Lufia2BattlePartyName, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9CD7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859cd7u, 0, Lufia2BattleQueueActionWindow, 3u,
                                  BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9CEE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859ceeu, 0, Lufia2BattleQueueListWindow, 3u,
                                  BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_DD7F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81dd7fu, 0, Lufia2BattleResultWindowPrepare,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x81, 0);
}

RecompReturn Lufia2DecompBridge_DDE7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81dde7u, 0, Lufia2BattleResultWindowLine, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x81, 0);
}

RecompReturn Lufia2DecompBridge_DE55(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81de55u, 0, Lufia2BattleResultWindowScroll, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x7e, 0);
}

RecompReturn Lufia2DecompBridge_DE9E(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81de9eu, 0, Lufia2BattleResultWindowWait, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x81, 0);
}

RecompReturn Lufia2DecompBridge_D9E1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81d9e1u, 0, Lufia2BattleResults, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x97, 0);
}

RecompReturn Lufia2DecompBridge_F7CA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f7cau, 0, Lufia2PartyApplyLevel, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_F7ED(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f7edu, 0, Lufia2PartyStatGrowth, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_F085(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f085u, 0, Lufia2InventoryReceive, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_DEE9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81dee9u, 0, Lufia2BattleRefreshTurnDisplay, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_E63A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e63au, 0, Lufia2BattlePartyStatusRows, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_E645(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e645u, 0, Lufia2BattlePartyStatusRow, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9DD4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859dd4u, 0, Lufia2BattleQueueTurnDisplay, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9150(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859150u, Lufia2BattleStatusName, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9173(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859173u, Lufia2BattleStatusPhrase, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_91A1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8591a1u, Lufia2BattleSyncStatusMarkers, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_91E0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8591e0u, Lufia2BattleClearStatusMarkers, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_D9C9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85d9c9u, Lufia2BattleEffectRecord, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 1);
}

RecompReturn Lufia2DecompBridge_DCA3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85dca3u, Lufia2BattleMultiply, 0, 3u,
                                  BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_REGISTER_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_DCEA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85dceau, Lufia2BattleRandomFraction, 0, 3u,
                                  BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8F67(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x858f67u, 0, Lufia2BattleRecoverStatuses, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9099(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859099u, 0, Lufia2BattleExpireStatuses, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9AAA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859aaau, Lufia2BattleSaveMessageState, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9ABC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859abcu, Lufia2BattleRestoreMessageState, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9671(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859671u, Lufia2BattleClearMessage, 0, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_WRAM_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_95FE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8595feu, 0, Lufia2BattleDisplayMessage, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_WRAM_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_9BDA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859bdau, 0, Lufia2BattleQueueStatusSprites, 3u,
                                  BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_834C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x80834cu, Lufia2Multiply16By8, 0, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0,
                                  BATTLE_BRIDGE_REGISTER_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9A7D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859a7du, Lufia2BattleMeasureMessage, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_E2AF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e2afu, Lufia2BattleStatusGauge, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x7e, 0);
}

RecompReturn Lufia2DecompBridge_E2C8(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e2c8u, Lufia2BattleStatusDigits, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, 0x7e, 0);
}

RecompReturn Lufia2DecompBridge_E73B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e73bu, 0, Lufia2BattleRenderMessage, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_REGISTER_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_E792(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81e792u, 0, Lufia2BattleLoadMessageGraphics,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_REGISTER_BANK, 0);
}

RecompReturn Lufia2DecompBridge_9ACE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x859aceu, Lufia2BattleNormalizeGlyph, 0, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_EA35(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81ea35u, 0, Lufia2BattleRenderGlyph, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_AADC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85aadcu, Lufia2BattleStartMessageEffect, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_REGISTER_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_AB28(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85ab28u, Lufia2BattleTickMessageEffect, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_AB5B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85ab5bu, Lufia2BattleQueueMessageCleanup, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8850(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x858850u, Lufia2BattleAnimateStatusIcons, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_919C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85919cu, 0, Lufia2BattleUpdateStatusIcons, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_EC81(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85ec81u, 0, Lufia2BattleFrameInputUpkeep, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_B705(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b705u, Lufia2BattleAppendOamSprites, 0, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_B5C4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b5c4u, 0, Lufia2BattleBuildSprites, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_CCCE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85ccceu, Lufia2BattleClearActionWork, 0, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_CCE3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cce3u, Lufia2BattleClearSavedActionWork, 0,
                                  3u, BATTLE_BRIDGE_ANY_WIDTH, 1,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_CCF8(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85ccf8u, Lufia2BattleClearActionRecords, 0, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_CD8C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cd8cu, Lufia2BattleSaveActionWork, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_CD9B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cd9bu, Lufia2BattleRestoreActionWork, 0, 3u,
                                  BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_CDFA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cdfau, Lufia2BattleActionRecordPointer, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_WRAM_BANK,
                                  1);
}

RecompReturn Lufia2DecompBridge_CDAA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cdaau, 0, Lufia2BattleLoadTurnRecord, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_CDD0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cdd0u, 0, Lufia2BattleLoadActionRecord, 3u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_B1A3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b1a3u, 0, Lufia2BattleRunConfiguredScript,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_B1C9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b1c9u, 0, Lufia2BattleRunBattlerScript, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_B1F7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b1f7u, 0, Lufia2BattleRunItemScript, 2u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_80F4FD(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x80f4fdu, Lufia2FieldStreamRightColumn, 3);
}

RecompReturn Lufia2DecompBridge_80F518(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x80f518u, Lufia2FieldStreamLeftColumn, 3);
}

RecompReturn Lufia2DecompBridge_80F589(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x80f589u, Lufia2FieldStreamTopRow, 3);
}

RecompReturn Lufia2DecompBridge_80F5A2(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x80f5a2u, Lufia2FieldStreamBottomRow, 3);
}

RecompReturn Lufia2DecompBridge_80F47A(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x80f47au, Lufia2FieldRedrawLayer, 3);
}

RecompReturn Lufia2DecompBridge_838E66(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x838e66u, Lufia2FieldRedrawAllLayers, 3);
}

RecompReturn Lufia2DecompBridge_83B5D3(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->D != 0 ||
        !(cpu->DB == 0x7eu || (cpu->DB & 0x7fu) < 0x40u))
        return ActorBridgeFallback(cpu, &frame, 0x83b5d3u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2FieldLoadMapHeader(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_80E844(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x80e844u, Lufia2FieldInitializeMapEvents, 3, 4);
}

RecompReturn Lufia2DecompBridge_80E722(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x80e722u, Lufia2FieldStartEvent, 3, 4);
}

RecompReturn Lufia2DecompBridge_8EB847(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x8eb847u, Lufia2CaveBuildMapHeader, 3, 3);
}

RecompReturn Lufia2DecompBridge_839B44(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x839b44u, Lufia2CaveRoomHeaderCoordinates, 3, 4);
}

RecompReturn Lufia2DecompBridge_83F611(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f611u, Lufia2FieldObjectLayer, 2, 4);
}

RecompReturn Lufia2DecompBridge_83D7A5(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83d7a5u, Lufia2ActorPositionToObjectProbe, 2, 4);
}

RecompReturn Lufia2DecompBridge_83F422(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f422u, Lufia2FieldSetObjectOrigin, 3, 4);
}

RecompReturn Lufia2DecompBridge_83F6B0(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f6b0u, Lufia2ActorResetObjectOffsets, 2, 3);
}

RecompReturn Lufia2DecompBridge_83F85A(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f85au, Lufia2FieldPendingTileOffsets, 2, 3);
}

RecompReturn Lufia2DecompBridge_83F784(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f784u, Lufia2FieldSetMapTileNumber, 2, 2);
}

RecompReturn Lufia2DecompBridge_83F7D4(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f7d4u, Lufia2FieldReleaseClaimedActors, 3, 0);
}

RecompReturn Lufia2DecompBridge_83F731(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f731u, Lufia2FieldCopyObjectPalette, 2, 2);
}

RecompReturn Lufia2DecompBridge_83F7F8(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f7f8u, Lufia2FieldPrepareObjectOrigin, 2, 4);
}

RecompReturn Lufia2DecompBridge_83F7DF(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f7dfu, Lufia2FieldStartObjectEvent, 2, 4);
}

RecompReturn Lufia2DecompBridge_83B007(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83b007u, Lufia2FieldUploadFixedGraphics, 3, 1);
}

RecompReturn Lufia2DecompBridge_83F6CA(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f6cau, Lufia2FieldSetupObjectActorSprite, 2, 4);
}

typedef Lufia2ExecutionResult (*FieldObjectTransition)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeObjectTransition(
    CpuState *cpu, uint32_t entry, FieldObjectTransition run,
    uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, entry);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_838B40(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x838b40u, Lufia2FieldLoadObjectRecord, 3u);
}

RecompReturn Lufia2DecompBridge_83F5EA(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f5eau, Lufia2FieldInitializeObjectActor, 3u);
}

RecompReturn Lufia2DecompBridge_83F5B9(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f5b9u, Lufia2FieldRefreshObjectActor, 2u);
}

RecompReturn Lufia2DecompBridge_83F7B1(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f7b1u, Lufia2FieldPlaceActorObject, 2u);
}

RecompReturn Lufia2DecompBridge_83F795(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f795u, Lufia2FieldRebuildActorObject, 2u);
}

RecompReturn Lufia2DecompBridge_83F620(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f620u, Lufia2FieldClaimPlacedObject, 3u);
}

RecompReturn Lufia2DecompBridge_80BFAA(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x80bfaau, Lufia2FieldFindHeaderRecord, 3u, 4);
}

RecompReturn Lufia2DecompBridge_83FB9F(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83fb9fu, Lufia2FieldFindPendingObject, 2u, 4);
}

RecompReturn Lufia2DecompBridge_83F9AD(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x83f9adu, Lufia2FieldObjectAttributeCell, 2u, 4);
}

RecompReturn Lufia2DecompBridge_83F80D(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f80du, Lufia2FieldMarkObjectAttributes, 2u);
}

RecompReturn Lufia2DecompBridge_83F86B(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f86bu, Lufia2FieldPlacePendingObject, 3u);
}

RecompReturn Lufia2DecompBridge_838A6F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838a6fu, Lufia2FieldClearObjectTileIds, 3u);
}

RecompReturn Lufia2DecompBridge_838E85(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838e85u, Lufia2FieldRenderRegion, 3u);
}

RecompReturn Lufia2DecompBridge_838E76(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838e76u, Lufia2FieldRenderLayerPair, 3u);
}

RecompReturn Lufia2DecompBridge_838C8A(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(
        cpu, 0x838c8au, Lufia2FieldRefreshObjectAttributes, 3u);
}

RecompReturn Lufia2DecompBridge_8389CE(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x8389ceu, Lufia2FieldClearObjectTileBit, 2u);
}

RecompReturn Lufia2DecompBridge_838A0A(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838a0au, Lufia2FieldRenderObjectLayers, 2u);
}

RecompReturn Lufia2DecompBridge_838B0E(CpuState *cpu) {
    return ActorBridgeWholeX16(
        cpu, 0x838b0eu, Lufia2FieldUpdatePlacedObject, 2u);
}

RecompReturn Lufia2DecompBridge_80C0B7(CpuState *cpu) {
    return ActorBridgeWholeX16(
        cpu, 0x80c0b7u, Lufia2SceneScriptReadOperand, 2u);
}

RecompReturn Lufia2DecompBridge_838AF7(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838af7u, Lufia2FieldObjectBitIndex, 2u);
}

RecompReturn Lufia2DecompBridge_838E01(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838e01u, Lufia2FieldLayerScaleMode, 2u);
}

RecompReturn Lufia2DecompBridge_838E2B(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838e2bu, Lufia2FieldPrepareCoordinateScale, 2u);
}

RecompReturn Lufia2DecompBridge_838E09(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838e09u, Lufia2FieldScaleCoordinateRight, 2u);
}

RecompReturn Lufia2DecompBridge_838E1A(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838e1au, Lufia2FieldScaleCoordinateLeft, 2u);
}

RecompReturn Lufia2DecompBridge_838DDA(CpuState *cpu) {
    return ActorBridgeWholeX16(
        cpu, 0x838ddau, Lufia2FieldPrepareLayerScrollX, 2u);
}

RecompReturn Lufia2DecompBridge_838DF0(CpuState *cpu) {
    return ActorBridgeWholeM0X0(
        cpu, 0x838df0u, Lufia2FieldPrepareLayerScrollY, 2u);
}

RecompReturn Lufia2DecompBridge_838AC9(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838ac9u, Lufia2FieldObjectBitTest, 3u);
}

RecompReturn Lufia2DecompBridge_838AD5(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838ad5u, Lufia2FieldObjectBitSet, 3u);
}

RecompReturn Lufia2DecompBridge_838AE5(CpuState *cpu) {
    return ActorBridgeWholeM1X16(
        cpu, 0x838ae5u, Lufia2FieldObjectBitClear, 3u);
}

RecompReturn Lufia2DecompBridge_839000(CpuState *cpu) {
    return ActorBridgeWholeM0(
        cpu, 0x839000u, Lufia2FieldPixelCellCeiling, 2u);
}

RecompReturn Lufia2DecompBridge_839004(CpuState *cpu) {
    return ActorBridgeWholeM0(
        cpu, 0x839004u, Lufia2FieldPixelCellFloor, 2u);
}

RecompReturn Lufia2DecompBridge_83F747(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f747u, Lufia2FieldRestoreObjectTiles, 3u);
}

RecompReturn Lufia2DecompBridge_83F933(CpuState *cpu) {
    return ActorBridgeObjectTransition(
        cpu, 0x83f933u, Lufia2FieldQueueObjectRedraw, 2u);
}

RecompReturn Lufia2DecompBridge_838B6A(CpuState *cpu) {
    return ActorBridgeRunWhole(cpu, 0x838b6au, Lufia2FieldCopyObjectTiles, 3u, 4);
}

static void ActorBridgeEquipmentListDraw(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    ActorPushedCall *call = (ActorPushedCall *)context;

    ActorBridgeStore(call->cpu, state);
    Lufia2DecompEquipmentListDraw(call->cpu, pc);
    ActorBridgeLoad(call->cpu, state);
}

RecompReturn Lufia2DecompBridge_82A318(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x82a318u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2MenuDrawStatus(
        &memory, &state, ActorBridgePushedChild,
        ActorBridgeEquipmentListDraw, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

static void ActorBridgeSpellPriceStored(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    ActorBridgeStore(call->cpu, state);
    Lufia2DecompSpellPriceStored(call->cpu, pc);
    ActorBridgeLoad(call->cpu, state);
}

RecompReturn Lufia2DecompBridge_82D905(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x82d905u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2MenuSpellShopSetup(
        &memory, &state, ActorBridgePushedChild,
        ActorBridgeSpellPriceStored, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_829918(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(
        cpu, 0x829918u, Lufia2AdjustPurchasePrice, 2u);
}

static void ActorBridgePlayTimeTick(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    ActorBridgeStore(call->cpu, state);
    Lufia2DecompPlayTimeTick(call->cpu, pc);
    ActorBridgeLoad(call->cpu, state);
}

RecompReturn Lufia2DecompBridge_808638(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->_flag_D)
        return ActorBridgeFallback(cpu, &frame, 0x808638u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2MainNmi(
        &memory, &state, ActorBridgePushedChild,
        ActorBridgePlayTimeTick, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return interp_tier_dispatch_tail(
        cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
}

static void ActorBridgeMapLoad(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    ActorBridgeStore(call->cpu, state);
    if (pc == 0x83b548u)
        Lufia2DecompMapLoadBegin(call->cpu, pc);
    else
        Lufia2DecompMapLoadCommitted(call->cpu, pc);
    ActorBridgeLoad(call->cpu, state);
}

RecompReturn Lufia2DecompBridge_83B53B(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83b53bu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2FieldInstallMap(
        &memory, &state, ActorBridgePushedChild, ActorBridgeMapLoad, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

RecompReturn Lufia2DecompBridge_80EAE7(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x80eae7u);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2FieldLoadMapResources(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

typedef Lufia2ExecutionResult (*ActorSaveFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall,
    Lufia2ExecutionCheckpoint, void *);

static void ActorBridgeGameFile(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    ActorBridgeStore(call->cpu, state);
    Lufia2DecompGameFile(call->cpu, pc);
    ActorBridgeLoad(call->cpu, state);
}

static RecompReturn ActorBridgeSave(
    CpuState *cpu, uint32_t entry, ActorSaveFunction run,
    uint8_t frame_size, int any_width) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (any_width ? cpu->emulation || cpu->_flag_D
                  : !ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    result = run(
        &memory, &state, ActorBridgePushedChild, ActorBridgeGameFile, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_809099(CpuState *cpu) {
    return ActorBridgeSave(cpu, 0x809099u, Lufia2LoadGameFile, 3u, 1);
}

RecompReturn Lufia2DecompBridge_8090C9(CpuState *cpu) {
    return ActorBridgeSave(cpu, 0x8090c9u, Lufia2SaveGameFile, 3u, 1);
}

RecompReturn Lufia2DecompBridge_80914B(CpuState *cpu) {
    return ActorBridgeSave(cpu, 0x80914bu, Lufia2ReadGameFile, 3u, 0);
}

static Lufia2ExecutionResult ActorBridgeWriteGameFile(
    const Lufia2Memory *memory, Lufia2CpuState *state,
    Lufia2PushedChildCall child, Lufia2ExecutionCheckpoint checkpoint,
    void *context) {
    (void)checkpoint;
    return Lufia2WriteGameFile(memory, state, child, context);
}

static Lufia2ExecutionResult ActorBridgeSaveChecksum(
    const Lufia2Memory *memory, Lufia2CpuState *state,
    Lufia2PushedChildCall child, Lufia2ExecutionCheckpoint checkpoint,
    void *context) {
    (void)checkpoint;
    return Lufia2SaveFileChecksum(memory, state, child, context);
}

RecompReturn Lufia2DecompBridge_809184(CpuState *cpu) {
    return ActorBridgeSave(cpu, 0x809184u, ActorBridgeWriteGameFile, 2u, 0);
}

RecompReturn Lufia2DecompBridge_8090FC(CpuState *cpu) {
    return ActorBridgeSave(cpu, 0x8090fcu, ActorBridgeSaveChecksum, 2u, 0);
}

RecompReturn Lufia2DecompBridge_8091D3(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8091d3u, Lufia2ResolveSaveFileAddress, 2u);
}

RecompReturn Lufia2DecompBridge_8082E7(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x8082e7u, Lufia2SeedRandom, 3u);
}

/* A song subscriber may call a guest command before the original STA $54.
 * Its JSL frame returns to the checkpoint itself, then the event runs again. */
static uint8_t ActorBridgeMusicCheckpoint(
    void *context, Lufia2CpuState *state, uint32_t pc) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    if (pc == 0x809692u) {
        ActorBridgeStore(call->cpu, state);
        Lufia2DecompMusicFadeOut(call->cpu, pc);
        ActorBridgeLoad(call->cpu, state);
        return 1;
    }
    for (;;) {
        uint32_t target;
        uint32_t landing = pc;
        uint16_t post_s;
        RecompReturn result;
        ActorBridgeStore(call->cpu, state);
        target = Lufia2DecompSongLoad(call->cpu, pc);
        ActorBridgeLoad(call->cpu, state);
        if (!target)
            return 1;
        post_s = state->stack;
        cpu_write8(call->cpu, 0u, state->stack, (uint8_t)(pc >> 16));
        state->stack = (uint16_t)(state->stack - 1u);
        cpu_write8(call->cpu, 0u, state->stack, (uint8_t)((pc - 1u) >> 8));
        state->stack = (uint16_t)(state->stack - 1u);
        cpu_write8(call->cpu, 0u, state->stack, (uint8_t)(pc - 1u));
        state->stack = (uint16_t)(state->stack - 1u);
        ActorBridgeStore(call->cpu, state);
        call->cpu->PB = (uint8_t)(target >> 16);
        result = cpu_dispatch_call_pc_pushed(
            call->cpu, target, pc, 3u, &landing);
        if (result != RECOMP_RETURN_NORMAL) {
            call->unwound = result;
            return 0;
        }
        if (call->cpu->S != post_s || landing != pc) {
            call->unwound = interp_tier_dispatch_tail(
                call->cpu, landing, pc,
                call->frame.entry_s, call->frame.hrv);
            return 0;
        }
        call->cpu->PB = (uint8_t)(pc >> 16);
        ActorBridgeLoad(call->cpu, state);
        state->program_bank = call->cpu->PB;
    }
}

typedef Lufia2ExecutionResult (*ActorMusicFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall,
    Lufia2MusicCheckpoint, void *);

static RecompReturn ActorBridgeMusic(
    CpuState *cpu, uint32_t entry, ActorMusicFunction run, uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    if (cpu->emulation || cpu->_flag_D)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    result = run(&memory, &state, ActorBridgePushedChild,
        ActorBridgeMusicCheckpoint, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

static Lufia2ExecutionResult ActorBridgePlaySong(
    const Lufia2Memory *memory, Lufia2CpuState *state,
    Lufia2PushedChildCall child, Lufia2MusicCheckpoint checkpoint,
    void *context) {
    (void)checkpoint;
    return Lufia2PlaySong(memory, state, child, context);
}

static Lufia2ExecutionResult ActorBridgeMusicVolume(
    const Lufia2Memory *memory, Lufia2CpuState *state,
    Lufia2PushedChildCall child, Lufia2MusicCheckpoint checkpoint,
    void *context) {
    (void)checkpoint;
    return Lufia2SetMusicVolume(memory, state, child, context);
}

RecompReturn Lufia2DecompBridge_80941A(CpuState *cpu) {
    return ActorBridgeMusic(cpu, 0x80941au, Lufia2LoadSong, 2u);
}

RecompReturn Lufia2DecompBridge_8093FE(CpuState *cpu) {
    return ActorBridgeMusic(cpu, 0x8093feu, ActorBridgePlaySong, 3u);
}

RecompReturn Lufia2DecompBridge_809692(CpuState *cpu) {
    return ActorBridgeMusic(cpu, 0x809692u, Lufia2FadeOutMusic, 3u);
}

RecompReturn Lufia2DecompBridge_809601(CpuState *cpu) {
    return ActorBridgeMusic(cpu, 0x809601u, ActorBridgeMusicVolume, 3u);
}

RecompReturn Lufia2DecompBridge_838D42(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(
        cpu, 0x838d42u, Lufia2FieldSetupLayerScroll, 3u);
}

RecompReturn Lufia2DecompBridge_848AF4(CpuState *cpu) {
    if (cpu->PB != 0x84u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0x8af4u;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeM0X0(
        cpu, 0x848af4u, Lufia2AncientCaveCarryBlueItem, 2u);
}

RecompReturn Lufia2DecompBridge_84890B(CpuState *cpu) {
    if (cpu->PB != 0x84u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0x890bu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeObjectTransition(
        cpu, 0x84890bu, Lufia2AncientCaveExit, 3u);
}

RecompReturn Lufia2DecompBridge_848888(CpuState *cpu) {
    if (cpu->PB != 0x84u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0x8888u;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeObjectTransition(
        cpu, 0x848888u, Lufia2AncientCaveResetParty, 2u);
}

RecompReturn Lufia2DecompBridge_848B9C(CpuState *cpu) {
    if (cpu->PB != 0x84u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0x8b9cu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeObjectTransition(
        cpu, 0x848b9cu, Lufia2AncientCaveDefeat, 3u);
}

RecompReturn Lufia2DecompBridge_83B76E(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xb76eu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeObjectTransition(
        cpu, 0x83b76eu, Lufia2FieldApplyAreaTransition, 3u);
}

RecompReturn Lufia2DecompBridge_8383EB(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0x83ebu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeObjectTransition(
        cpu, 0x8383ebu, Lufia2FieldBattleTransition, 3u);
}

static RecompReturn ActorBridgeFieldWidthSetup(
    CpuState *cpu, uint32_t entry, FieldObjectTransition run,
    uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->_flag_D)
        return ActorBridgeFallback(cpu, &frame, entry);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = run(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_8385DC(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0x85dcu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeFieldWidthSetup(
        cpu, 0x8385dcu, Lufia2FieldReloadMap, 3u);
}

RecompReturn Lufia2DecompBridge_83AB61(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xab61u;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(
        cpu, 0x83ab61u, Lufia2SpriteResetAllocations, 2u);
}

RecompReturn Lufia2DecompBridge_8EB09C(CpuState *cpu) {
    if (cpu->PB != 0x8eu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xb09cu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeFieldWidthSetup(
        cpu, 0x8eb09cu, Lufia2FieldPrepareCameraScroll, 3u);
}

RecompReturn Lufia2DecompBridge_80F35B(CpuState *cpu) {
    if (cpu->PB != 0x80u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xf35bu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeM0X0(
        cpu, 0x80f35bu, Lufia2FieldCopyMetatileGraphics, 2u);
}

RecompReturn Lufia2DecompBridge_80F3F1(CpuState *cpu) {
    if (cpu->PB != 0x80u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xf3f1u;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeM0(
        cpu, 0x80f3f1u, Lufia2FieldMirrorPlaneWord, 2u);
}

RecompReturn Lufia2DecompBridge_80F40A(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xf40au;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(
        cpu, 0x80f40au, Lufia2FieldMirrorPlaneByte, 2u);
}

RecompReturn Lufia2DecompBridge_80EF8E(CpuState *cpu) {
    if (cpu->PB != 0x80u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xef8eu;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeFieldWidthSetup(
        cpu, 0x80ef8eu, Lufia2FieldLoadSceneGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_80F2F3(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xf2f3u;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(
        cpu, 0x80f2f3u, Lufia2FieldSetSceneDisplay, 3u);
}

RecompReturn Lufia2DecompBridge_80F338(CpuState *cpu) {
    if (cpu->PB != 0x80u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xf338u;
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(
        cpu, 0x80f338u, Lufia2FieldCopyScenePalette, 3u);
}

/* Wave tables preserve decimal arithmetic and use a JSR frame. */
static RecompReturn ActorBridgeWaveTable(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S > (cpu->host_return_valid == 3u ? 0x1ffcu : 0x1ffdu))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_85ADE1(CpuState *cpu) {
    return ActorBridgeWaveTable(cpu, 0x85ade1u, Lufia2BattleWaveBackward);
}

RecompReturn Lufia2DecompBridge_85AE68(CpuState *cpu) {
    return ActorBridgeWaveTable(cpu, 0x85ae68u, Lufia2BattleWaveForward);
}

RecompReturn Lufia2DecompBridge_85AEEB(CpuState *cpu) {
    return ActorBridgeWaveTable(cpu, 0x85aeebu, Lufia2BattleWaveFill);
}

RecompReturn Lufia2DecompBridge_85AA3D(CpuState *cpu) {
    return ActorBridgeWaveTable(cpu, 0x85aa3du, Lufia2BattleRippleWords);
}

/* Menu palettes accept either accumulator width and use a JSL frame. */
static RecompReturn ActorBridgeMenuPalette(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->x_flag || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_8690C0(CpuState *cpu) {
    return ActorBridgeMenuPalette(cpu, 0x8690c0u, Lufia2MenuLoadPalette0);
}

RecompReturn Lufia2DecompBridge_8690D3(CpuState *cpu) {
    return ActorBridgeMenuPalette(cpu, 0x8690d3u, Lufia2MenuLoadPalette1);
}

RecompReturn Lufia2DecompBridge_8690E6(CpuState *cpu) {
    return ActorBridgeMenuPalette(cpu, 0x8690e6u, Lufia2MenuLoadPalette2);
}

RecompReturn Lufia2DecompBridge_8690F9(CpuState *cpu) {
    return ActorBridgeMenuPalette(cpu, 0x8690f9u, Lufia2MenuLoadPalette3);
}

RecompReturn Lufia2DecompBridge_86910C(CpuState *cpu) {
    return ActorBridgeMenuPalette(cpu, 0x86910cu, Lufia2MenuLoadPalette4);
}

/* World perspective tables use the original bank-$86 caller context. */
static RecompReturn ActorBridgeWorldPlane(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0 || cpu->DB != 0x86u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86A894(CpuState *cpu) {
    return ActorBridgeWorldPlane(cpu, 0x86a894u, Lufia2WorldMapPlane);
}

/* Menu multiply keeps P and X; any widths, JSL frame. */
static RecompReturn ActorBridgeMenuMultiply(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x82u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_828000(CpuState *cpu) {
    return ActorBridgeMenuMultiply(cpu, 0x828000u, Lufia2MenuMultiply);
}

/* World division keeps P; any widths and direct page. */
static RecompReturn ActorBridgeWorldDivide(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86A5A9(CpuState *cpu) {
    return ActorBridgeWorldDivide(cpu, 0x86a5a9u, Lufia2WorldMapDivide32);
}

/* Angle lookups keep P; any widths and direct page. */
static RecompReturn ActorBridgeBattleAngle(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_85DE2A(CpuState *cpu) {
    return ActorBridgeBattleAngle(cpu, 0x85de2au, Lufia2BattleSineOfAngle);
}

RecompReturn Lufia2DecompBridge_85DE1E(CpuState *cpu) {
    return ActorBridgeBattleAngle(cpu, 0x85de1eu, Lufia2BattleCosineOfAngle);
}

/* Sprite clear keeps P; any widths. */
static RecompReturn ActorBridgeWorldClearSprites(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E650(CpuState *cpu) {
    return ActorBridgeWorldClearSprites(cpu, 0x86e650u, Lufia2WorldMapClearSprites);
}

/* Slot flag clear needs M1X0. */
static RecompReturn ActorBridgeWorldClearSlots(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E640(CpuState *cpu) {
    return ActorBridgeWorldClearSlots(cpu, 0x86e640u, Lufia2WorldMapClearSlotFlags);
}

/* Menu item index needs the caller stack band. */
static RecompReturn ActorBridgeMenuItemIndex(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x82u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_8288A0(CpuState *cpu) {
    return ActorBridgeMenuItemIndex(cpu, 0x8288a0u, Lufia2MenuItemIndex);
}

/* Menu item position needs M1 and the caller stack band. */
static RecompReturn ActorBridgeMenuItemPosition(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_table) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x82u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u || !cpu->m_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_table(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_8288CB(CpuState *cpu) {
    return ActorBridgeMenuItemPosition(cpu, 0x8288cbu, Lufia2MenuItemPosition);
}

/* Scratch stat totals preserve the stat event and caller frame. */
static RecompReturn ActorBridgePartyStatCopy(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction calculate_totals) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u || cpu->D != 0u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = calculate_totals(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_81F481(CpuState *cpu) {
    return ActorBridgePartyStatCopy(cpu, 0x81f481u, Lufia2PartyStatTotalsOfCopy);
}

/* Step the five scene scripts, preserving their end-of-script carry. */
static int ActorBridgeSceneRamBank(uint8_t bank) {
    return bank < 0x40u || bank == 0x7eu || bank == 0x7fu ||
        (bank >= 0x80u && bank < 0xc0u);
}

static RecompReturn ActorBridgeSceneTracks(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction step_tracks) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->D != 0u ||
        !ActorBridgeSceneRamBank(cpu->DB) || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = step_tracks(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_8694D4(CpuState *cpu) {
    return ActorBridgeSceneTracks(cpu, 0x8694d4u, Lufia2SceneTrackStep);
}

/* Derive scene scroll and screen coordinates from the camera origin. */
static RecompReturn ActorBridgeSceneView(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction calculate_origin) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->D != 0u ||
        !ActorBridgeSceneRamBank(cpu->DB) ||
        !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = calculate_origin(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86A791(CpuState *cpu) {
    return ActorBridgeSceneView(cpu, 0x86a791u, Lufia2SceneViewOrigin);
}

/* Combine the signed angle components with the battle speed. */
static RecompReturn ActorBridgeBattleVelocity(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction calculate_velocity) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu || cpu->D != 0u ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = calculate_velocity(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_85DD63(CpuState *cpu) {
    return ActorBridgeBattleVelocity(cpu, 0x85dd63u, Lufia2BattleVelocityOfAngle);
}

/* Stage effect parameters around the original three-byte velocity call. */
static RecompReturn ActorBridgeEffectVelocity(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction calculate_velocity) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || cpu->S > 0x1ffcu || cpu->D != 0u ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f03u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = calculate_velocity(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_81A598(CpuState *cpu) {
    return ActorBridgeEffectVelocity(cpu, 0x81a598u, Lufia2BattleEffectVelocity);
}

/* Test world objects against the screen rectangle. */
static int ActorBridgeWorldRamBank(uint8_t bank) {
    return bank < 0x40u || bank == 0x7eu || bank == 0x7fu ||
        (bank >= 0x80u && bank < 0xc0u);
}

static RecompReturn ActorBridgeVisibleObject(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction test_visibility) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->D != 0u ||
        !ActorBridgeWorldRamBank(cpu->DB) ||
        cpu->m_flag || cpu->x_flag ||
        cpu->X > 0x1ff1u || cpu->Y > 0x1fd2u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = test_visibility(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E295(CpuState *cpu) {
    return ActorBridgeVisibleObject(cpu, 0x86e295u, Lufia2WorldMapTestObject);
}

/* Test world objects against the screen rectangle. */
static RecompReturn ActorBridgeVisibleList(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction test_visibility) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->D != 0u ||
        !ActorBridgeWorldRamBank(cpu->DB) ||
        cpu->m_flag || cpu->x_flag ||
        cpu->X != 0x1469u || cpu->Y != 0x124fu ||
        cpu->S < 0x1f00u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    /* The original updater starts at $1469/$124F with at most 21 objects.
     * Reject damaged counters before materializing a caller frame. */
    {
        const uint8_t low = ActorBridgeRead(cpu, 0x22u);
        const uint8_t high = ActorBridgeRead(cpu, 0x23u);
        const uint16_t count = (uint16_t)(low | ((uint16_t)high << 8));
        if (count == 0u || count > 21u)
            return ActorBridgeFallback(cpu, &frame, entry_pc24);
    }
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = test_visibility(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E287(CpuState *cpu) {
    return ActorBridgeVisibleList(cpu, 0x86e287u, Lufia2WorldMapTestObjects);
}

/* Build one perspective band with the original multiplier and write order. */
static RecompReturn ActorBridgeWorldRows(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_rows) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu ||
        cpu->D != 0u || cpu->DB != 0x86u ||
        (cpu->Y != 0x0382u && cpu->Y != 0x01c1u) ||
        cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    {
        const uint8_t low = ActorBridgeRead(cpu, 0x26u);
        const uint8_t high = ActorBridgeRead(cpu, 0x27u);
        const uint16_t count = (uint16_t)(low | ((uint16_t)high << 8));
        if (count == 0u || count > 112u)
            return ActorBridgeFallback(cpu, &frame, entry_pc24);
    }
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_rows(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86A9B0(CpuState *cpu) {
    return ActorBridgeWorldRows(cpu, 0x86a9b0u, Lufia2WorldPlaneRows0);
}

RecompReturn Lufia2DecompBridge_86AA5B(CpuState *cpu) {
    return ActorBridgeWorldRows(cpu, 0x86aa5bu, Lufia2WorldPlaneRows1);
}

RecompReturn Lufia2DecompBridge_86AB0E(CpuState *cpu) {
    return ActorBridgeWorldRows(cpu, 0x86ab0eu, Lufia2WorldPlaneRows2);
}

RecompReturn Lufia2DecompBridge_86ABC1(CpuState *cpu) {
    return ActorBridgeWorldRows(cpu, 0x86abc1u, Lufia2WorldPlaneRows3);
}

/* Preserve the original hardware product, scratch writes and arithmetic flags. */
static RecompReturn ActorBridgeWorldProduct(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction multiply) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = multiply(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86A583(CpuState *cpu) {
    return ActorBridgeWorldProduct(cpu, 0x86a583u, Lufia2WorldProduct16By8);
}

/* Build the 32 ripple bytes and preserve the original caller frame. */
static RecompReturn ActorBridgeBattleRippleRow(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_ripple) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag || (uint16_t)(cpu->D + 0x33u) >= 0x2000u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_ripple(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_85A736(CpuState *cpu) {
    return ActorBridgeBattleRippleRow(cpu, 0x85a736u, Lufia2BattleRippleRow);
}

/* Unpack four attributes per source byte with the original hardware product. */
static RecompReturn ActorBridgePackedAttributes(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction unpack_attributes) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x80u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u || cpu->D != 0u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = unpack_attributes(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    /* Supported output cannot reach the caller's 1Fxx return frame. */
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_80ED0E(CpuState *cpu) {
    return ActorBridgePackedAttributes(cpu, 0x80ed0eu, Lufia2FieldUnpackAttributes);
}

/* Copy both tile planes in their original forward byte order. */
static RecompReturn ActorBridgeBattleBlit(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction copy_rows) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->D != 0u || cpu->_flag_D ||
        !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u)))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = copy_rows(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    /* The supported outputs stay clear of the caller's return frame. */
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_81BCCC(CpuState *cpu) {
    return ActorBridgeBattleBlit(cpu, 0x81bcccu, Lufia2BattleBlitTileRows);
}

/* Preserve world motion arithmetic and original nested JSR frames. */
static RecompReturn ActorBridgeWorldMotion(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction move, uint16_t minimum_stack) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->S < minimum_stack ||
        cpu->D != 0u || !ActorBridgeWorldRamBank(cpu->DB) ||
        !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = move(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86A417(CpuState *cpu) {
    return ActorBridgeWorldMotion(cpu, 0x86a417u, Lufia2WorldStepOffsets, 0x1e00u);
}


RecompReturn Lufia2DecompBridge_86995B(CpuState *cpu) {
    return ActorBridgeWorldMotion(cpu, 0x86995bu, Lufia2WorldScrollAdvance, 0x1e02u);
}

/* Mark the fixed object-slot range without touching the caller frame. */
static RecompReturn ActorBridgeFieldObjectFlags(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction mark_slots) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x80u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = mark_slots(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_80C195(CpuState *cpu) {
    return ActorBridgeFieldObjectFlags(cpu, 0x80c195u, Lufia2FieldMarkObjectSlots);
}

/* Compute the cell address with the original nested call frames. */
static RecompReturn ActorBridgeFieldCellPointer(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction cell_pointer) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x83u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f04u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = cell_pointer(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_83F9D0(CpuState *cpu) {
    return ActorBridgeFieldCellPointer(cpu, 0x83f9d0u, Lufia2FieldCellPointer);
}

/* Clear the fixed sprite-slot flag range away from the caller frame. */
static RecompReturn ActorBridgeSpriteClearSlots(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction clear_slots) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = clear_slots(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_868E6B(CpuState *cpu) {
    return ActorBridgeSpriteClearSlots(cpu, 0x868e6bu, Lufia2SpriteClearSlots);
}

/* Shift the game random register in original word-write order. */
static RecompReturn ActorBridgeBattleRandomBit(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction shift_register) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u || !cpu->m_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = shift_register(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_858F4A(CpuState *cpu) {
    return ActorBridgeBattleRandomBit(cpu, 0x858f4au, Lufia2BattleRandomBit);
}

/* Build circle half widths with the original status frame. */
static RecompReturn ActorBridgeBattleCircleWidths(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_widths) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u || cpu->D != 0u || cpu->DB != 0x7eu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_widths(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_85B26D(CpuState *cpu) {
    return ActorBridgeBattleCircleWidths(cpu, 0x85b26du, Lufia2BattleCircleWidths);
}

/* Build the circle window from its verified half-width child. */
static RecompReturn ActorBridgeBattleCircleWindow(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_window) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f08u || cpu->D != 0u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_window(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_85B208(CpuState *cpu) {
    return ActorBridgeBattleCircleWindow(cpu, 0x85b208u, Lufia2BattleCircleWindow);
}

/* Copy an image buffer while keeping live code and return frames intact. */
static RecompReturn ActorBridgeMenuImageRow256(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction copy_buffer) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->S < 0x1f04u || cpu->x_flag || cpu->D != 0u || !cpu->m_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = copy_buffer(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_869009(CpuState *cpu) {
    return ActorBridgeMenuImageRow256(cpu, 0x869009u, Lufia2MenuCopyImageRow256);
}

/* Copy an image buffer while keeping live code and return frames intact. */
static RecompReturn ActorBridgeMenuImageRow128(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction copy_buffer) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->S < 0x1f04u || cpu->x_flag || cpu->D != 0u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = copy_buffer(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86906A(CpuState *cpu) {
    return ActorBridgeMenuImageRow128(cpu, 0x86906au, Lufia2MenuCopyImageRow128);
}

/* Copy an image buffer while keeping live code and return frames intact. */
static RecompReturn ActorBridgeMenuImageBlock(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction copy_buffer) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu || cpu->S < 0x1f08u || cpu->x_flag || cpu->D != 0u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = copy_buffer(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_868FF6(CpuState *cpu) {
    return ActorBridgeMenuImageBlock(cpu, 0x868ff6u, Lufia2MenuCopyImageBlock);
}

/* Copy an image buffer while keeping live code and return frames intact. */
static RecompReturn ActorBridgeRamBufferCopy(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction copy_buffer) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || (cpu->PB != 0x83u && cpu->PB != 0x86u) || cpu->S > 0x1ffcu || cpu->S < 0x1f00u || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = copy_buffer(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_00057D(CpuState *cpu) {
    return ActorBridgeRamBufferCopy(cpu, ((uint32_t)cpu->PB << 16) | 0x057du, Lufia2RamBlockMove);
}

/* Preserve ordered hardware accesses and the original saved frames. */
static RecompReturn ActorBridgeNmiScrollUploads(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction execute_uploads) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation ||
        cpu->PB != 0x80u ||
        cpu->S > 0x1ffcu ||
        cpu->S < 0x1f04u || cpu->D != 0u || !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u)))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = execute_uploads(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_8087A7(CpuState *cpu) {
    return ActorBridgeNmiScrollUploads(cpu, 0x8087a7u, Lufia2NmiScrollAndUploads);
}

/* Preserve ordered hardware accesses and the original saved frames. */
static RecompReturn ActorBridgeNmiTilemapUploads(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction execute_uploads) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation ||
        cpu->PB != 0x80u ||
        cpu->S > 0x1ffcu ||
        cpu->S < 0x1f02u || cpu->D != 0u || !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u)) || !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = execute_uploads(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_8087FC(CpuState *cpu) {
    return ActorBridgeNmiTilemapUploads(cpu, 0x8087fcu, Lufia2NmiTilemapUploads);
}

/* Preserve ordered hardware accesses and the original saved frames. */
static RecompReturn ActorBridgeBattleScaleColor(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction scale_color) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation ||
        cpu->PB != 0x81u ||
        cpu->S > 0x1ffcu ||
        cpu->S < 0x1f00u || cpu->D != 0u || !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u)) || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = scale_color(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_81B3F8(CpuState *cpu) {
    return ActorBridgeBattleScaleColor(cpu, 0x81b3f8u, Lufia2BattleScaleColor);
}

/* Preserve ordered hardware accesses and the original saved frames. */
static RecompReturn ActorBridgeBattlePaletteFade(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction fade_palette) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation ||
        cpu->PB != 0x81u ||
        cpu->S > 0x1ffcu ||
        cpu->S < 0x1f02u || cpu->D != 0u || !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u)) || !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    if (ActorBridgeRead(cpu, 0x11u) > 15u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = fade_palette(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_81B396(CpuState *cpu) {
    return ActorBridgeBattlePaletteFade(cpu, 0x81b396u, Lufia2BattlePaletteBrightness);
}

/* Fill the tile block with the original stride and return frame. */
static RecompReturn ActorBridgeMenuTileBlock(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction fill_block) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation ||
        cpu->PB != 0x82u ||
        cpu->S > 0x1ffcu ||
        cpu->S < 0x1f02u || cpu->D != 0u || cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = fill_block(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_8280A5(CpuState *cpu) {
    return ActorBridgeMenuTileBlock(cpu, 0x8280a5u, Lufia2MenuTileBlockFill);
}

/* Sort the bounded visible list in descending order, retaining stable object references. */
static RecompReturn ActorBridgeVisibleSort(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction sort_visible) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu ||
        cpu->S < 0x1f02u || cpu->m_flag || cpu->x_flag || cpu->D != 0u)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = sort_visible(&memory, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E686(CpuState *cpu) {
    return ActorBridgeVisibleSort(cpu, 0x86e686u, Lufia2WorldMapSortVisible);
}

/* Banks that map the CPU registers and low WRAM. */
static int ActorBridgeSystemBank(uint8_t bank) {
    return bank < 0x40u || (bank >= 0x80u && bank < 0xc0u);
}

/* Slide corrections: M8, DP0, Y <= $0800, low-WRAM DB. */
static RecompReturn ActorBridgeMenuSlideCorrection(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction correct_slide) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x82u || !cpu->m_flag || cpu->D != 0u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu || cpu->Y > 0x0800u ||
        !(ActorBridgeSystemBank(cpu->DB) || cpu->DB == 0x7eu))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = correct_slide(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_828AD8(CpuState *cpu) {
    return ActorBridgeMenuSlideCorrection(cpu, 0x828ad8u, Lufia2MenuSlideCorrectX);
}

RecompReturn Lufia2DecompBridge_828AE9(CpuState *cpu) {
    return ActorBridgeMenuSlideCorrection(cpu, 0x828ae9u, Lufia2MenuSlideCorrectY);
}

/* Colour tables through the WRAM port: M8/X16, JSL frame. */
static RecompReturn ActorBridgeBattleColors(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction load_colors,
    int needs_system_bank) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f02u || cpu->S > 0x1ffcu ||
        (needs_system_bank && !ActorBridgeSystemBank(cpu->DB)))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = load_colors(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_858AAF(CpuState *cpu) {
    return ActorBridgeBattleColors(cpu, 0x858aafu, Lufia2BattleColorsInit, 1);
}

RecompReturn Lufia2DecompBridge_858AF4(CpuState *cpu) {
    return ActorBridgeBattleColors(cpu, 0x858af4u, Lufia2BattleColorsParty, 0);
}

RecompReturn Lufia2DecompBridge_858B22(CpuState *cpu) {
    return ActorBridgeBattleColors(cpu, 0x858b22u, Lufia2BattleColorsMonster, 0);
}

/* Reads a WRAM word low byte first, as the guarded routine does. */
static uint16_t ActorBridgeReadWord(CpuState *cpu, uint32_t address) {
    const uint8_t low = ActorBridgeRead(cpu, address);

    return (uint16_t)(low | ((uint16_t)ActorBridgeRead(cpu, address + 1u) << 8));
}

/* World sprites: M16/X16, DP0, bounded counter and object record. */
static RecompReturn ActorBridgeWorldSprite(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_sprite,
    uint16_t lowest_stack, uint16_t counter_limit, int reads_object) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < lowest_stack || cpu->S > 0x1ffcu ||
        !(ActorBridgeSystemBank(cpu->DB) || cpu->DB == 0x7eu))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    if (ActorBridgeReadWord(cpu, ((uint32_t)cpu->DB << 16) | 0x1467u) > counter_limit)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    if (reads_object) {
        const uint16_t object = ActorBridgeReadWord(cpu, 0x02u);

        if (object < 0x1000u || object > 0x1e00u)
            return ActorBridgeFallback(cpu, &frame, entry_pc24);
    }
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_sprite(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E555(CpuState *cpu) {
    return ActorBridgeWorldSprite(
        cpu, 0x86e555u, Lufia2WorldMapDrawSpritePair, 0x1f04u, 126u, 1);
}

RecompReturn Lufia2DecompBridge_86E5BB(CpuState *cpu) {
    return ActorBridgeWorldSprite(
        cpu, 0x86e5bbu, Lufia2WorldMapStoreHighBits, 0x1f02u, 127u, 0);
}

RecompReturn Lufia2DecompBridge_86E479(CpuState *cpu) {
    return ActorBridgeWorldSprite(
        cpu, 0x86e479u, Lufia2WorldMapDrawSprite, 0x1f02u, 127u, 1);
}

RecompReturn Lufia2DecompBridge_86E4E7(CpuState *cpu) {
    return ActorBridgeWorldSprite(
        cpu, 0x86e4e7u, Lufia2WorldMapDrawSmallSprite, 0x1f02u, 127u, 1);
}

/* Effect stream opcodes run with M8/X16 from the dispatcher. */
static RecompReturn ActorBridgeEffectOpcode(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run_opcode) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = run_opcode(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_81A40B(CpuState *cpu) {
    return ActorBridgeEffectOpcode(cpu, 0x81a40bu, Lufia2BattleEffectAddToField);
}

RecompReturn Lufia2DecompBridge_81953F(CpuState *cpu) {
    return ActorBridgeEffectOpcode(cpu, 0x81953fu, Lufia2BattleEffectRepeat);
}

/* Video operands require DP0 and the battle stack. */
static RecompReturn ActorBridgeEffectVideo(
    CpuState *cpu, uint32_t entry, ActorWholeFunction command) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->D != 0u || cpu->S < 0x1f00u)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeEffectOpcode(cpu, entry, command);
}

RecompReturn Lufia2DecompBridge_81963A(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x81963au, Lufia2BattleEffectVideoRegister);
}

RecompReturn Lufia2DecompBridge_819653(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819653u, Lufia2BattleEffectBg3Map);
}

RecompReturn Lufia2DecompBridge_8199A5(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x8199a5u, Lufia2BattleEffectBackgroundRelease);
}

RecompReturn Lufia2DecompBridge_819999(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819999u, Lufia2BattleEffectBackgroundRequest);
}

RecompReturn Lufia2DecompBridge_8199B0(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x8199b0u, Lufia2BattleEffectBackgroundCopy);
}

RecompReturn Lufia2DecompBridge_819AB1(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819ab1u, Lufia2BattleEffectWindowBand);
}

/* Battle tile ids: rows need M16/X16, the grid X16. */
static RecompReturn ActorBridgeBattleTiles(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction fill_tiles,
    int needs_wide_accumulator, uint8_t frame_size) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || cpu->S > 0x1ffcu ||
        cpu->x_flag || (needs_wide_accumulator && cpu->m_flag))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = fill_tiles(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_859790(CpuState *cpu) {
    return ActorBridgeBattleTiles(cpu, 0x859790u, Lufia2BattleTileRow, 1, 2);
}

RecompReturn Lufia2DecompBridge_85972E(CpuState *cpu) {
    return ActorBridgeBattleTiles(cpu, 0x85972eu, Lufia2BattleTileGridEntry, 0, 3);
}

/* One battle actor sprite: M16/X16, JSR frame. */
static RecompReturn ActorBridgeBattleActorSprite(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction append_sprite) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || cpu->S > 0x1ffcu ||
        cpu->m_flag || cpu->x_flag)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = append_sprite(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_818EEA(CpuState *cpu) {
    return ActorBridgeBattleActorSprite(cpu, 0x818eeau, Lufia2BattleActorSprite);
}

/* World animations: M8/X16; stepping also needs DP0 and S $1F00..$1FFC. */
static RecompReturn ActorBridgeWorldAnimation(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction animate,
    int needs_caller_frame) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag ||
        (needs_caller_frame && (cpu->D != 0u || cpu->S < 0x1f00u)))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = animate(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E0B9(CpuState *cpu) {
    return ActorBridgeWorldAnimation(cpu, 0x86e0b9u, Lufia2WorldMapStartAnimation, 0);
}

RecompReturn Lufia2DecompBridge_86E11F(CpuState *cpu) {
    return ActorBridgeWorldAnimation(cpu, 0x86e11fu, Lufia2WorldMapStepAnimations, 1);
}

/* Battle actor lists; a rewritten child return continues in the original. */
static RecompReturn ActorBridgeBattleActorLists(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction build_lists) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || cpu->S > 0x1ffcu ||
        !cpu->m_flag || cpu->x_flag || cpu->_flag_D)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = build_lists(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_818E92(CpuState *cpu) {
    return ActorBridgeBattleActorLists(cpu, 0x818e92u, Lufia2BattleActorSprites);
}

/* Battle OAM groups: M8/X16, DP0, S $1F00..$1FFC, JSL frame. */
static RecompReturn ActorBridgeBattleOamGroup(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction draw_group) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = draw_group(&memory, &state);
    /* The OAM span check reads before any write. */
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_858B4B(CpuState *cpu) {
    return ActorBridgeBattleOamGroup(cpu, 0x858b4bu, Lufia2BattleSpriteRecordsEntry);
}

RecompReturn Lufia2DecompBridge_858BC0(CpuState *cpu) {
    return ActorBridgeBattleOamGroup(cpu, 0x858bc0u, Lufia2BattleSpriteSingleEntry);
}

RecompReturn Lufia2DecompBridge_858C27(CpuState *cpu) {
    return ActorBridgeBattleOamGroup(cpu, 0x858c27u, Lufia2BattleSpriteMarkersEntry);
}

/* Battle drift records: M8/X16, binary mode, S $1F00..$1FFC. */
static RecompReturn ActorBridgeBattleDrift(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction drift_records) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->_flag_D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = drift_records(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_85894A(CpuState *cpu) {
    return ActorBridgeBattleDrift(cpu, 0x85894au, Lufia2BattleDriftRecords);
}

/* Slot palettes: X16, DP0, S $1F00..$1FFC, count 1..7. */
static RecompReturn ActorBridgeMenuSlotPalettes(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction load_palettes) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = load_palettes(&memory, &state);
    /* The count check reads before any write. */
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_86911F(CpuState *cpu) {
    return ActorBridgeMenuSlotPalettes(cpu, 0x86911fu, Lufia2MenuLoadSlotPalettes);
}

/* Whole world-object passes; semantic guards reject before any write. */
static RecompReturn ActorBridgeWorldObjectParent(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction update_objects,
    uint16_t minimum_stack, int accumulator_8, int rom_tables) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x86u || cpu->x_flag ||
        cpu->m_flag != accumulator_8 || cpu->_flag_D || cpu->D != 0u ||
        !ActorBridgeWorldRamBank(cpu->DB) || cpu->S < minimum_stack ||
        cpu->S > 0x1ffcu ||
        (rom_tables && cpu->DB != 0x86u && cpu->DB != 0x06u))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = update_objects(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry_pc24)
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 2, result.pc);
}

RecompReturn Lufia2DecompBridge_86E430(CpuState *cpu) {
    return ActorBridgeWorldObjectParent(
        cpu, 0x86e430u, Lufia2WorldMapAssignSlot, 0x1f00u, 0, 0);
}

RecompReturn Lufia2DecompBridge_86E3D2(CpuState *cpu) {
    return ActorBridgeWorldObjectParent(
        cpu, 0x86e3d2u, Lufia2WorldMapDrawObjectByKind, 0x1f04u, 0, 0);
}

RecompReturn Lufia2DecompBridge_86E3AB(CpuState *cpu) {
    return ActorBridgeWorldObjectParent(
        cpu, 0x86e3abu, Lufia2WorldMapDrawObjects, 0x1f08u, 0, 0);
}

RecompReturn Lufia2DecompBridge_86E2D2(CpuState *cpu) {
    return ActorBridgeWorldObjectParent(
        cpu, 0x86e2d2u, Lufia2WorldMapProjectObjects, 0x1f00u, 0, 0);
}

RecompReturn Lufia2DecompBridge_86E1B9(CpuState *cpu) {
    return ActorBridgeWorldObjectParent(
        cpu, 0x86e1b9u, Lufia2WorldMapUpdateObjects, 0x1f10u, 1, 1);
}

/* Complete party render passes and their frame-setup parent. */
static RecompReturn ActorBridgeBattleRenderParent(
    CpuState *cpu, uint32_t entry, ActorWholeFunction render,
    uint16_t minimum_stack, int needs_mmio_bank) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    const uint8_t bank = cpu->DB;

    if (cpu->emulation || cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->_flag_D || cpu->D != 0u || cpu->S < minimum_stack ||
        cpu->S > 0x1ffcu || (needs_mmio_bank &&
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u))))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    result = render(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 3, result.pc);
}

RecompReturn Lufia2DecompBridge_858A39(CpuState *cpu) {
    return ActorBridgeBattleRenderParent(cpu, 0x858a39u,
        Lufia2BattleFrameSetup, 0x1f10u, 1);
}

RecompReturn Lufia2DecompBridge_858C98(CpuState *cpu) {
    return ActorBridgeBattleRenderParent(cpu, 0x858c98u,
        Lufia2BattleSpritePartyEntry, 0x1f00u, 0);
}

RecompReturn Lufia2DecompBridge_858D2E(CpuState *cpu) {
    return ActorBridgeBattleRenderParent(cpu, 0x858d2eu,
        Lufia2BattlePartyTilemapEntry, 0x1f00u, 0);
}

/* Native tile work with the original frame-wait child. */
typedef Lufia2ExecutionResult (*MenuWaitParent)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);


static RecompReturn ActorBridgeMenuWaitParent(
    CpuState *cpu, uint32_t entry, MenuWaitParent draw, int rectangle) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    ActorPushedCall call;

    if (cpu->emulation || cpu->PB != 0x82u || cpu->x_flag || cpu->_flag_D ||
        cpu->D != 0u || cpu->S < 0x1f04u || cpu->S > 0x1ffcu ||
        (rectangle && (cpu->m_flag || !(cpu->X & 0xffu) ||
        (cpu->X & 0xffu) > 32u || !(cpu->X >> 8) || (cpu->X >> 8) > 32u)))
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = draw(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_828069(CpuState *cpu) {
    return ActorBridgeMenuWaitParent(cpu, 0x828069u, Lufia2MenuTileGridFill, 0);
}

RecompReturn Lufia2DecompBridge_8280CA(CpuState *cpu) {
    return ActorBridgeMenuWaitParent(cpu, 0x8280cau, Lufia2MenuRecolorRect, 1);
}

RecompReturn Lufia2DecompBridge_82838F(CpuState *cpu) {
    return ActorBridgeMenuWaitParent(cpu, 0x82838fu, Lufia2MenuClearLayers, 0);
}

typedef Lufia2ExecutionResult (*MenuParentFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);


static RecompReturn ActorBridgeMenuParent(
    CpuState *cpu, uint32_t entry, MenuParentFunction parent,
    unsigned return_size, unsigned minimum_stack, int word_entry) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    ActorPushedCall call;

    if (cpu->emulation || cpu->PB != entry >> 16 || cpu->x_flag ||
        cpu->_flag_D || cpu->D != 0u || cpu->S < minimum_stack ||
        cpu->S > 0x1ffcu || (!word_entry && !cpu->m_flag))
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    call.cpu = cpu;
    call.frame = frame;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = parent(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, return_size, result.pc);
}

RecompReturn Lufia2DecompBridge_828044(CpuState *cpu) {
    return ActorBridgeMenuParent(cpu, 0x828044u, Lufia2MenuQueueVideoWrite, 3u, 0x1f04u, 0);
}

RecompReturn Lufia2DecompBridge_868DD7(CpuState *cpu) {
    return ActorBridgeMenuParent(cpu, 0x868dd7u, Lufia2MenuScreenSetup, 3u, 0x1f04u, 0);
}

RecompReturn Lufia2DecompBridge_869022(CpuState *cpu) {
    return ActorBridgeMenuParent(cpu, 0x869022u, Lufia2MenuLoadImageGrid, 3u, 0x1f10u, 1);
}

RecompReturn Lufia2DecompBridge_868F6F(CpuState *cpu) {
    return ActorBridgeMenuParent(cpu, 0x868f6fu, Lufia2MenuLoadImageSet, 3u, 0x1f10u, 0);
}

/* Slide count and slide: original sprite frame and JSR sites as children. */
RecompReturn Lufia2DecompBridge_828AFA(CpuState *cpu) {
    return ActorBridgeMenuParent(cpu, 0x828afau, Lufia2MenuSlideCount, 2u, 0x1f10u, 0);
}

RecompReturn Lufia2DecompBridge_8289FA(CpuState *cpu) {
    return ActorBridgeMenuParent(cpu, 0x8289fau, Lufia2MenuCursorSlide, 2u, 0x1f12u, 0);
}

extern int cpu_resolve_ancestor_skip(uint16_t);
extern int g_interp_apu_driving;

/* This opcode discards its own call frame before returning to the dispatcher. */
static RecompReturn ActorBridgeReturnPastCall(CpuState *cpu,
    const ActorBridgeFrame *frame, uint32_t source) {
    const uint16_t dispatch_stack = cpu->S;
    uint16_t low, high;
    uint32_t target;
    int ancestor;

    cpu->S = (uint16_t)(cpu->S + 1u);
    low = cpu_read8(cpu, 0, cpu->S);
    cpu->S = (uint16_t)(cpu->S + 1u);
    high = cpu_read8(cpu, 0, cpu->S);
    target = ((uint32_t)cpu->PB << 16) | (uint16_t)(((high << 8) | low) + 1u);
    ancestor = cpu_resolve_ancestor_skip(dispatch_stack);
    if (ancestor >= 0)
        return (RecompReturn)ancestor;
    if (interp_bridge_return_targets_owner(dispatch_stack, cpu->S))
        return interp_bridge_lle_yield_unwind(cpu, target);
    if (interp_bridge_has_direct_paired_bounce())
        return interp_tier_dispatch_rewritten_return(cpu, target, source);
    return cpu_dispatch_pc_from(cpu, target, (uint16_t)(frame->entry_s + 2u), source);
}

RecompReturn Lufia2DecompBridge_819169(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffau)
        return ActorBridgeFallback(cpu, &frame, 0x819169u);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleEffectYield(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturnPastCall(cpu, &frame, result.pc);
}

RecompReturn Lufia2DecompBridge_808703(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x80u || !cpu->m_flag || cpu->D != 0u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu || g_interp_apu_driving ||
        !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u)))
        return ActorBridgeFallback(cpu, &frame, 0x808703u);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = Lufia2NmiSpritesPaletteAndPads(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_828720(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};

    if (cpu->X > 0xffu ||
        !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u) ||
          cpu->DB == 0x7eu))
        return ActorBridgeFallback(cpu, &frame, 0x828720u);
    return ActorBridgeMenuParent(cpu, 0x828720u, Lufia2MenuCursor, 2u, 0x1f02u, 0);
}

/* Edge replay is read-only; unsafe tables return to the untouched entry. */
RecompReturn Lufia2DecompBridge_80F821(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x80u || cpu->D != 0u || cpu->_flag_D ||
        cpu->S < 0x1f09u || cpu->S > 0x1ffcu ||
        !(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u) ||
          cpu->DB == 0x7eu))
        return ActorBridgeFallback(cpu, &frame, 0x80f821u);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = Lufia2FieldTraceCellEdges(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == 0x80f821u)
        return ActorBridgeFallback(cpu, &frame, 0x80f821u);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

/* Original text and frame services retain their live guest frames. */
RecompReturn Lufia2DecompBridge_828B08(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (!(cpu->DB < 0x40u || (cpu->DB >= 0x80u && cpu->DB < 0xc0u) || cpu->DB == 0x7eu))
        return ActorBridgeFallback(cpu, &frame, 0x828b08u);
    return ActorBridgeMenuParent(cpu, 0x828b08u, Lufia2MenuInputLoop, 2u, 0x1f04u, 0);
}

typedef Lufia2ExecutionResult (*EffectGraphicsFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeEffectGraphics(
    CpuState *cpu, uint32_t entry, EffectGraphicsFunction command) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    ActorPushedCall call;

    if (cpu->emulation || cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f05u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    ActorBridgeLoad(cpu, &state);
    result = command(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_8199D0(CpuState *cpu) {
    return ActorBridgeEffectGraphics(cpu, 0x8199d0u, Lufia2BattleEffectGraphics);
}

RecompReturn Lufia2DecompBridge_819A12(CpuState *cpu) {
    return ActorBridgeEffectGraphics(
        cpu, 0x819a12u, Lufia2BattleEffectGraphicsAlternate);
}

RecompReturn Lufia2DecompBridge_81915B(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffau)
        return ActorBridgeFallback(cpu, &frame, 0x81915bu);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = Lufia2BattleEffectEnd(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturnPastCall(cpu, &frame, result.pc);
}

RecompReturn Lufia2DecompBridge_81917F(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x81917fu, Lufia2BattleEffectDelay);
}

RecompReturn Lufia2DecompBridge_81918F(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x81918fu, Lufia2BattleEffectJump);
}

RecompReturn Lufia2DecompBridge_819198(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819198u, Lufia2BattleEffectRepeatStart);
}

RecompReturn Lufia2DecompBridge_8191AD(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x8191adu, Lufia2BattleEffectRepeatJump);
}

RecompReturn Lufia2DecompBridge_8194EB(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x8194ebu, Lufia2BattleEffectLoopStart);
}

RecompReturn Lufia2DecompBridge_819500(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819500u, Lufia2BattleEffectLoopStart2);
}

RecompReturn Lufia2DecompBridge_819515(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819515u, Lufia2BattleEffectLoopStart3);
}

RecompReturn Lufia2DecompBridge_81952A(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x81952au, Lufia2BattleEffectLoopStart4);
}

RecompReturn Lufia2DecompBridge_819553(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819553u, Lufia2BattleEffectLoopNext2);
}

RecompReturn Lufia2DecompBridge_819567(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819567u, Lufia2BattleEffectLoopNext3);
}

RecompReturn Lufia2DecompBridge_81957B(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x81957bu, Lufia2BattleEffectLoopNext4);
}

static uint8_t ActorBridgeEffectEngineChild(
    void *context, Lufia2CpuState *state, uint32_t target,
    uint32_t site, uint8_t frame_size) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    const uint16_t post_s = (uint16_t)(state->stack + frame_size);
    const uint32_t landing = site + frame_size + 1u;
    uint32_t return_pc = landing;
    ActorBridgeStore(call->cpu, state);
    call->cpu->PB = (uint8_t)(target >> 16);
    if (!cpu_dispatch_has_entry(call->cpu, target)) {
        RecompReturn result = interp_tier_dispatch_tail(
            call->cpu, target, site, call->frame.entry_s, call->frame.hrv);
        call->unwound = (RecompReturn)((int)result + 1);
        return 0u;
    }
    RecompReturn result = cpu_dispatch_call_pc_pushed(
        call->cpu, target, site, frame_size, &return_pc);
    if (result != RECOMP_RETURN_NORMAL) {
        call->unwound = result;
        return 0u;
    }
    if (call->cpu->S != post_s || return_pc != landing) {
        result = interp_tier_dispatch_tail(
            call->cpu, return_pc, site, call->frame.entry_s, call->frame.hrv);
        call->unwound = (RecompReturn)((int)result + 1);
        return 0u;
    }
    call->cpu->PB = (uint8_t)(landing >> 16);
    ActorBridgeLoad(call->cpu, state);
    state->program_bank = call->cpu->PB;
    return 1u;
}

RecompReturn Lufia2DecompBridge_81895E(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x81u || cpu->D ||
        !cpu->m_flag || cpu->x_flag || cpu->S < 0x1f10u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, 0x81895eu);
    frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result = Lufia2BattlePlayEffect(
        &memory, &state, ActorBridgeEffectEngineChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

static uint8_t ActorBridgeSceneChild(
    void *context, Lufia2CpuState *state, uint32_t target,
    uint32_t site, uint8_t frame_size) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    const uint16_t post_s = (uint16_t)(state->stack + frame_size);
    const uint32_t landing = site + frame_size + 1u;
    uint32_t return_pc = landing;
    ActorBridgeStore(call->cpu, state);
    call->cpu->PB = (uint8_t)(target >> 16);
    if (!cpu_dispatch_has_entry(call->cpu, target)) {
        RecompReturn result = interp_tier_dispatch_tail(
            call->cpu, target, site,
            call->frame.entry_s, call->frame.hrv);
        call->unwound = (RecompReturn)((int)result + 1);
        return 0u;
    }
    RecompReturn result = cpu_dispatch_call_pc_pushed(
        call->cpu, target, site, frame_size, &return_pc);
    if (result != RECOMP_RETURN_NORMAL) {
        call->unwound = result;
        return 0u;
    }
    if (call->cpu->S != post_s || return_pc != landing) {
        result = interp_tier_dispatch_tail(
            call->cpu, return_pc, site,
            call->frame.entry_s, call->frame.hrv);
        call->unwound = (RecompReturn)((int)result + 1);
        return 0u;
    }
    call->cpu->PB = (uint8_t)(landing >> 16);
    ActorBridgeLoad(call->cpu, state);
    state->program_bank = call->cpu->PB;
    return 1u;
}

RecompReturn Lufia2DecompBridge_83A76D(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const uint32_t entry = ((uint32_t)cpu->PB << 16) | 0xa76du;
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u || cpu->D ||
        (cpu->DB != 0u && cpu->DB != 0x7eu && cpu->DB != 0x83u) ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result = Lufia2FieldRebuildSceneActors(
        &memory, &state, ActorBridgeSceneChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

static RecompReturn ActorBridgeFieldSession(
    CpuState *cpu, uint32_t entry, FieldObjectTransition run) {
    const ActorBridgeFrame frame = {
        cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u || cpu->D ||
        (cpu->DB != 0u && cpu->DB != 0x7eu && cpu->DB != 0x83u) ||
        cpu->S < 0x1f10u || cpu->S > 0x1fffu ||
        (entry == 0x83ad23u && (!cpu->m_flag || cpu->x_flag)))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(
        &memory, &state, ActorBridgeSceneChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    /* Session roots enter the field system without returning. */
    return interp_tier_dispatch_tail(
        cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
}

RecompReturn Lufia2DecompBridge_83ACB7(CpuState *cpu) {
    return ActorBridgeFieldSession(
        cpu, 0x83acb7u, Lufia2FieldBeginSessionSetup);
}

RecompReturn Lufia2DecompBridge_83AD23(CpuState *cpu) {
    return ActorBridgeFieldSession(
        cpu, 0x83ad23u, Lufia2FieldResumeSessionSetup);
}

static RecompReturn ActorBridgeEffectSlot(
    CpuState *cpu, uint32_t entry, ActorWholeFunction command, uint16_t minimum) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};

    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->PB != 0x81u ||
        cpu->D != 0u || cpu->S < minimum || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeWholeM1X16(cpu, entry, command, 2u);
}

RecompReturn Lufia2DecompBridge_818F91(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x818f91u, Lufia2BattleEffectFindActorSlot, 0x1f00u);
}
RecompReturn Lufia2DecompBridge_818FAB(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x818fabu, Lufia2BattleEffectFindScriptSlot, 0x1f00u);
}
RecompReturn Lufia2DecompBridge_818FC5(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x818fc5u, Lufia2BattleEffectSpawnActor, 0x1f02u);
}
RecompReturn Lufia2DecompBridge_81905B(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x81905bu, Lufia2BattleEffectSpawnMovingActor, 0x1f02u);
}
RecompReturn Lufia2DecompBridge_8190DF(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x8190dfu, Lufia2BattleEffectSpawnScript, 0x1f02u);
}
RecompReturn Lufia2DecompBridge_8191F2(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x8191f2u, Lufia2BattleEffectSpawnScriptAt, 0x1f02u);
}
RecompReturn Lufia2DecompBridge_81929E(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x81929eu, Lufia2BattleEffectSpawnActorAt, 0x1f02u);
}
RecompReturn Lufia2DecompBridge_81920F(CpuState *cpu) {
    return ActorBridgeEffectSlot(cpu, 0x81920fu, Lufia2BattleEffectSpawnScriptsForTargets, 0x1f04u);
}

RecompReturn Lufia2DecompBridge_8191B7(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x8191b7u, Lufia2BattleEffectBranchIfField);
}
RecompReturn Lufia2DecompBridge_8194CA(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x8194cau, Lufia2BattleEffectSelectPortraitStream);
}
RecompReturn Lufia2DecompBridge_819BA3(CpuState *cpu) {
    return ActorBridgeEffectVideo(cpu, 0x819ba3u, Lufia2BattleEffectSkipArgument);
}

static RecompReturn ActorBridgeSpriteLookup(
    CpuState *cpu, uint32_t entry, ActorWholeFunction lookup) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->x_flag || cpu->PB != 0x81u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = lookup(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

RecompReturn Lufia2DecompBridge_81FBC6(CpuState *cpu) {
    return ActorBridgeSpriteLookup(cpu, 0x81fbc6u, Lufia2CharacterSpriteWord);
}

RecompReturn Lufia2DecompBridge_81FB8E(CpuState *cpu) {
    return ActorBridgeSpriteLookup(cpu, 0x81fb8eu, Lufia2SpriteCoordinatesPacked);
}

static RecompReturn ActorBridgeTargetResolution(
    CpuState *cpu, uint32_t entry, ActorWholeFunction routine, uint16_t minimum) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};

    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->PB != 0x81u ||
        (cpu->DB != 0x97u && (entry != 0x81b80au || cpu->DB != 0x7eu)) ||
        cpu->D != 0u || cpu->S < minimum || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeWholeM1X16(cpu, entry, routine, 2u);
}

RecompReturn Lufia2DecompBridge_81B228(CpuState *cpu) {
    return ActorBridgeTargetResolution(cpu, 0x81b228u, Lufia2BattleResolveTargetMask, 0x1f00u);
}

RecompReturn Lufia2DecompBridge_81B7EF(CpuState *cpu) {
    return ActorBridgeTargetResolution(cpu, 0x81b7efu, Lufia2BattleTargetSpriteCoordinates, 0x1f03u);
}

RecompReturn Lufia2DecompBridge_81B80A(CpuState *cpu) {
    return ActorBridgeTargetResolution(cpu, 0x81b80au, Lufia2BattleEffectTargetCoordinates, 0x1f05u);
}

static RecompReturn ActorBridgeActionPhase(
    CpuState *cpu, uint32_t entry, BattleControlFunction run, uint8_t frame_size) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};

    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->PB != 0x81u ||
        cpu->DB != 0x97u || cpu->D != 0u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeBattleEntry(cpu, entry, NULL, run, frame_size,
        BATTLE_BRIDGE_M1X0, 0, 0x97u, 0);
}

RecompReturn Lufia2DecompBridge_81B139(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81b139u, Lufia2BattleRunActionEffects, 3u);
}

RecompReturn Lufia2DecompBridge_81B174(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81b174u, Lufia2BattleFinishActionEffects, 3u);
}

RecompReturn Lufia2DecompBridge_81AFC4(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81afc4u, Lufia2BattlePrepareActionEffect, 2u);
}

RecompReturn Lufia2DecompBridge_81B057(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81b057u, Lufia2BattleRunActorAction, 3u);
}

RecompReturn Lufia2DecompBridge_81B08A(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81b08au, Lufia2BattleRunActionPresentation, 3u);
}

RecompReturn Lufia2DecompBridge_81A8A7(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81a8a7u, Lufia2BattleSkipAction, 3u);
}

RecompReturn Lufia2DecompBridge_81A8A8(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81a8a8u, Lufia2BattleAttackAction, 3u);
}

RecompReturn Lufia2DecompBridge_81A968(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81a968u, Lufia2BattleBeginSpellAction, 3u);
}

RecompReturn Lufia2DecompBridge_81A977(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81a977u, Lufia2BattleSpellAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AA1B(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81aa1bu, Lufia2BattleBeginItemAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AA2E(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81aa2eu, Lufia2BattleItemAction, 3u);
}

static RecompReturn ActorBridgeActionRecord(
    CpuState *cpu, uint32_t entry, ActorWholeFunction lookup) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x81u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = lookup(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

RecompReturn Lufia2DecompBridge_81AAD0(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81aad0u, Lufia2BattleDefendAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AB07(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ab07u, Lufia2BattleContinueAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AF7A(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81af7au, Lufia2BattleBeginUncostedSpellAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AF89(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81af89u, Lufia2BattleUncostedSpellAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AF37(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81af37u, Lufia2BattleFollowupAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AD8D(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ad8du, Lufia2BattleWaitAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AEE1(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81aee1u, Lufia2BattleWaitLongAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AA73(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81aa73u, Lufia2BattleRepeatActionEffects, 3u);
}

RecompReturn Lufia2DecompBridge_81F45E(CpuState *cpu) {
    return ActorBridgeActionRecord(cpu, 0x81f45eu, Lufia2IpRecordPointer);
}

RecompReturn Lufia2DecompBridge_81F46B(CpuState *cpu) {
    return ActorBridgeActionRecord(cpu, 0x81f46bu, Lufia2IpNamePointer);
}

RecompReturn Lufia2DecompBridge_81F476(CpuState *cpu) {
    return ActorBridgeActionRecord(cpu, 0x81f476u, Lufia2CapsuleActionRecordPointer);
}

RecompReturn Lufia2DecompBridge_81AB0C(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ab0cu, Lufia2BattleIpAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AD3B(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ad3bu, Lufia2BattleCapsuleAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AE0F(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ae0fu, Lufia2BattleAlternateAttackAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AB94(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ab94u, Lufia2BattleCollectiveAction, 3u);
}

RecompReturn Lufia2DecompBridge_81AC72(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81ac72u, Lufia2BattleAction07, 3u);
}

RecompReturn Lufia2DecompBridge_81A832(CpuState *cpu) {
    return ActorBridgeActionPhase(cpu, 0x81a832u, Lufia2BattleDispatchAction, 3u);
}

static RecompReturn ActorBridgeCoordinateMath(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper,
    uint8_t return_size, int word_entry) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || (word_entry ? cpu->m_flag : !cpu->m_flag) ||
        cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = helper(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, return_size, result.pc);
}

RecompReturn Lufia2DecompBridge_83FC8B(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fc8bu,
        Lufia2ObjectFinePositionToProbe, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FCB4(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fcb4u,
        Lufia2ObjectScaleFinePosition, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83F988(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83f988u,
        Lufia2MapProbeTileHeight, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83F9F2(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83f9f2u,
        Lufia2MapProbeCellOffset, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83F9EE(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83f9eeu,
        Lufia2MapCellOffsetLong, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83F9B6(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83f9b6u,
        Lufia2MapPackedAttributeCell, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83F9A5(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83f9a5u,
        Lufia2FieldObjectAttributeCellLong, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83F9A9(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83f9a9u,
        Lufia2MapPackedAttributeCellLong, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83E60E(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83e60eu,
        Lufia2ObjectInterpolateCoordinate, 2u, 1);
}

RecompReturn Lufia2DecompBridge_83E6AA(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83e6aau,
        Lufia2ObjectApproachCoordinate, 2u, 1);
}

RecompReturn Lufia2DecompBridge_83FB2E(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fb2eu,
        Lufia2FieldProbeObjectAttributes, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83FB51(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fb51u,
        Lufia2FieldProbeObjectState, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83FB61(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fb61u,
        Lufia2FieldProbeObjectProperties, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FB8B(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fb8bu,
        Lufia2FieldPendingObjectState, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FB9B(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fb9bu,
        Lufia2FieldFindPendingObjectLong, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83FBF1(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fbf1u,
        Lufia2FieldReadProbeAttribute, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FBFE(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fbfeu,
        Lufia2FieldClearPendingOccupancy, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FC3C(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fc3cu,
        Lufia2FieldFindSpecialActor, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FC56(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fc56u,
        Lufia2FieldProbeInputAllowed, 2u, 0);
}

RecompReturn Lufia2DecompBridge_83FC69(CpuState *cpu) {
    return ActorBridgeCoordinateMath(cpu, 0x83fc69u,
        Lufia2FieldUpdateProbeAction, 2u, 0);
}

static RecompReturn ActorBridgeProbeDirection(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    const uint8_t direction = (uint8_t)cpu->A;

    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f04u || cpu->S > 0x1ffcu ||
        (entry == 0x83fbbdu && (direction > 6u || (direction & 1u))))
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = helper(&memory, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_83FBBD(CpuState *cpu) {
    return ActorBridgeProbeDirection(cpu, 0x83fbbdu,
        Lufia2FieldProbeDirectionBlocked);
}

RecompReturn Lufia2DecompBridge_83F49A(CpuState *cpu) {
    return ActorBridgeProbeDirection(cpu, 0x83f49au,
        Lufia2FieldSaveProbePosition);
}

RecompReturn Lufia2DecompBridge_83F4A7(CpuState *cpu) {
    return ActorBridgeProbeDirection(cpu, 0x83f4a7u,
        Lufia2FieldRestoreProbePosition);
}

RecompReturn Lufia2DecompBridge_83EC5F(CpuState *cpu) {
    return ActorBridgeProbeDirection(cpu, 0x83ec5fu,
        Lufia2ObjectProbeNextTile);
}


static RecompReturn ActorBridgeSpriteResource(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper, uint8_t return_size, uint8_t allow_x16) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || !cpu->m_flag || (!allow_x16 && !cpu->x_flag) || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f04u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = helper(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, return_size, result.pc);
}

RecompReturn Lufia2DecompBridge_83AB4F(CpuState *cpu) {
    return ActorBridgeSpriteResource(cpu, 0x83ab4fu,
        Lufia2ActorSetRecordOffsets, 3u, 1u);
}

RecompReturn Lufia2DecompBridge_83ABCC(CpuState *cpu) {
    return ActorBridgeSpriteResource(cpu, 0x83abccu,
        Lufia2SpriteReleaseAllocation, 3u, 0u);
}

RecompReturn Lufia2DecompBridge_83ABE9(CpuState *cpu) {
    return ActorBridgeSpriteResource(cpu, 0x83abe9u,
        Lufia2SpriteComputeVramBase, 3u, 0u);
}

RecompReturn Lufia2DecompBridge_83AB7C(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || !cpu->m_flag || !cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f04u || cpu->S > 0x1ffcu ||
        !(cpu->A & 0xffu))
        return ActorBridgeFallback(cpu, &frame, 0x83ab7cu);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = Lufia2SpriteReserveAllocation(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

static RecompReturn ActorBridgeEventMapState(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper, uint8_t return_size) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f04u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = helper(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, return_size, result.pc);
}
RecompReturn Lufia2DecompBridge_80D136(CpuState *cpu) {
    return ActorBridgeEventMapState(cpu, 0x80d136u,
        Lufia2FieldSaveObjectRegion, 2u);
}
RecompReturn Lufia2DecompBridge_80D15B(CpuState *cpu) {
    return ActorBridgeEventMapState(cpu, 0x80d15bu,
        Lufia2FieldRestoreObjectRegion, 2u);
}
RecompReturn Lufia2DecompBridge_80D18C(CpuState *cpu) {
    return ActorBridgeEventMapState(cpu, 0x80d18cu,
        Lufia2FieldNormalizeObjectOrigin, 2u);
}
RecompReturn Lufia2DecompBridge_80D227(CpuState *cpu) {
    return ActorBridgeEventMapState(cpu, 0x80d227u,
        Lufia2FieldMarkRegionObjects, 3u);
}

static RecompReturn ActorBridgeFieldRecordSearch(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper, uint8_t return_size) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu, ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f04u || cpu->S > 0x1ffcu || !cpu->Y)
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    ActorBridgeLoad(cpu, &state);
    result = helper(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, return_size, result.pc);
}
RecompReturn Lufia2DecompBridge_83B851(CpuState *cpu) {
    return ActorBridgeFieldRecordSearch(cpu, 0x83b851u,
        Lufia2FieldFindPointRecord, 3u);
}
RecompReturn Lufia2DecompBridge_83B882(CpuState *cpu) {
    return ActorBridgeFieldRecordSearch(cpu, 0x83b882u,
        Lufia2FieldFindRectangleRecord, 2u);
}

RecompReturn Lufia2DecompBridge_83F442(CpuState *cpu) {
    if (cpu->S < 0x1f04u || cpu->S > 0x1ffcu) {
        ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};

        return ActorBridgeFallback(cpu, &frame, 0x83f442u);
    }
    return ActorBridgeCoordinateMath(cpu, 0x83f442u,
        Lufia2FieldClearObjectAttributes, 3u, 0);
}

RecompReturn Lufia2DecompBridge_83F750(CpuState *cpu) {
    if (cpu->S < 0x1f04u || cpu->S > 0x1ffcu) {
        ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};

        return ActorBridgeFallback(cpu, &frame, 0x83f750u);
    }
    return ActorBridgeCoordinateMath(cpu, 0x83f750u,
        Lufia2FieldSetObjectTiles, 3u, 0);
}

static RecompReturn ActorBridgeEventTrigger(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f10u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, 3u, 4);
}
RecompReturn Lufia2DecompBridge_80E7FA(CpuState *cpu) {
    return ActorBridgeEventTrigger(cpu, 0x80e7fau, Lufia2FieldStartEventAtProbe);
}
RecompReturn Lufia2DecompBridge_80E7DF(CpuState *cpu) {
    return ActorBridgeEventTrigger(cpu, 0x80e7dfu, Lufia2FieldStartPositionEvent);
}

static RecompReturn ActorBridgeEventObjectRegion(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper, uint8_t return_size) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, return_size, 4);
}
RecompReturn Lufia2DecompBridge_80D0AC(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80d0acu, Lufia2FieldPrepareObjectRegion, 2u);
}
RecompReturn Lufia2DecompBridge_80D112(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80d112u, Lufia2FieldCopyEventObjectRegion, 2u);
}
RecompReturn Lufia2DecompBridge_80D1E1(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80d1e1u, Lufia2FieldRemoveRegionObjects, 3u);
}
RecompReturn Lufia2DecompBridge_80D19F(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80d19fu, Lufia2FieldRestoreRegionObjects, 3u);
}
RecompReturn Lufia2DecompBridge_80D0BA(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->S < 0x1f40u)
        return ActorBridgeFallback(cpu, &frame, 0x80d0bau);
    return ActorBridgeEventObjectRegion(cpu, 0x80d0bau, Lufia2FieldUpdateEventObjectRegion, 2u);
}
RecompReturn Lufia2DecompBridge_80D077(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80d077u, Lufia2FieldReadObjectRegionPosition, 2u);
}

RecompReturn Lufia2DecompBridge_80CE5C(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80ce5cu, Lufia2FieldReadObjectRegionDestination, 2u);
}

RecompReturn Lufia2DecompBridge_80CE7E(CpuState *cpu) {
    return ActorBridgeEventObjectRegion(cpu, 0x80ce7eu, Lufia2FieldReadObjectRegionArea, 2u);
}

static RecompReturn ActorBridgeActorEventHelper(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, 2u, 4);
}
RecompReturn Lufia2DecompBridge_83C0EF(CpuState *cpu) {
    return ActorBridgeActorEventHelper(cpu, 0x83c0efu, Lufia2FieldProbeLeaderPosition);
}
RecompReturn Lufia2DecompBridge_83C0FA(CpuState *cpu) {
    return ActorBridgeActorEventHelper(cpu, 0x83c0fau, Lufia2FieldAcknowledgeControlChange);
}
RecompReturn Lufia2DecompBridge_83F0BC(CpuState *cpu) {
    return ActorBridgeActorEventHelper(cpu, 0x83f0bcu, Lufia2FieldSetFollowingObjectDrawFlags);
}

static RecompReturn ActorBridgeObjectReleaseHelper(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper, uint8_t frame_size) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f04u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, frame_size, 4);
}
RecompReturn Lufia2DecompBridge_83EF6E(CpuState *cpu) {
    return ActorBridgeObjectReleaseHelper(cpu, 0x83ef6eu, Lufia2ObjectWakeMatchingPosition, 2u);
}
RecompReturn Lufia2DecompBridge_83F205(CpuState *cpu) {
    return ActorBridgeObjectReleaseHelper(cpu, 0x83f205u, Lufia2ObjectRemoveSlot, 3u);
}

static RecompReturn ActorBridgeSceneRecordHelper(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper,
    bool byte_input, uint8_t return_frame) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->m_flag != byte_input || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x80u || cpu->D ||
        cpu->S < (return_frame==3u?0x1f10u:0x1f00u) || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, return_frame, byte_input ? 4 : 2);
}
RecompReturn Lufia2DecompBridge_80C0D0(CpuState *cpu) {
    return ActorBridgeSceneRecordHelper(cpu, 0x80c0d0u, Lufia2SceneScriptReadWord, true, 2u);
}
RecompReturn Lufia2DecompBridge_80C102(CpuState *cpu) {
    return ActorBridgeSceneRecordHelper(cpu, 0x80c102u, Lufia2SceneScriptSeekRelative, false, 2u);
}
RecompReturn Lufia2DecompBridge_80C12E(CpuState *cpu) {
    return ActorBridgeSceneRecordHelper(cpu, 0x80c12eu, Lufia2SceneScriptFindRecord, true, 3u);
}

static RecompReturn ActorBridgeEventControlHelper(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper, uint8_t return_frame) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D ||
        cpu->S < (return_frame==3u?0x1f30u:0x1f00u) || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, return_frame, 4);
}
RecompReturn Lufia2DecompBridge_83BB76(CpuState *cpu) {
    return ActorBridgeEventControlHelper(cpu, 0x83bb76u, Lufia2FieldPrepareEventControl, 2u);
}
RecompReturn Lufia2DecompBridge_83B727(CpuState *cpu) {
    return ActorBridgeEventControlHelper(cpu, 0x83b727u, Lufia2FieldBeginEventControl, 3u);
}

static RecompReturn ActorBridgeObjectCollision(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper,
    uint8_t return_frame, uint16_t minimum_stack) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != (uint8_t)(entry >> 16) || cpu->D ||
        cpu->S < minimum_stack || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, return_frame, 4);
}

RecompReturn Lufia2DecompBridge_83BAC2(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83bac2u, Lufia2FieldFindActorAtProbe, 2u, 0x1f00u);
}

RecompReturn Lufia2DecompBridge_83DF87(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83df87u, Lufia2ActorSpawnFromId, 3u, 0x1f10u);
}

RecompReturn Lufia2DecompBridge_83F4B4(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83f4b4u, Lufia2FieldApplyObjectRecord, 2u, 0x1f40u);
}

RecompReturn Lufia2DecompBridge_80BFE7(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x80bfe7u, Lufia2SceneScriptSelectRecord, 3u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_83F435(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83f435u, Lufia2FieldSetPendingProbePosition, 2u, 0x1f00u);
}

RecompReturn Lufia2DecompBridge_83EC4F(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83ec4fu, Lufia2ObjectStartInteractionEvent, 2u, 0x1f30u);
}

RecompReturn Lufia2DecompBridge_83ECDE(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83ecdeu, Lufia2ObjectAllocateSpriteResources, 2u, 0x1f10u);
}

RecompReturn Lufia2DecompBridge_83C079(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83c079u,
        Lufia2FieldCanPushObject, 3u, 0x1f30u);
}

RecompReturn Lufia2DecompBridge_83C108(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83c108u,
        Lufia2FieldClaimActor, 3u, 0x1f10u);
}

RecompReturn Lufia2DecompBridge_80DCDA(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x80dcdau,
        Lufia2FieldPushPendingObject, 3u, 0x1f50u);
}

RecompReturn Lufia2DecompBridge_80EA47(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x80ea47u,
        Lufia2FieldSaveActorSlot, 2u, 0x1f00u);
}

RecompReturn Lufia2DecompBridge_80EA50(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x80ea50u,
        Lufia2FieldRestoreActorSlot, 2u, 0x1f10u);
}

RecompReturn Lufia2DecompBridge_83E033(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83e033u,
        Lufia2FieldSetObjectDrawFlags, 3u, 0x1f00u);
}

RecompReturn Lufia2DecompBridge_8099FD(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    if (cpu->emulation)
        return ActorBridgeFallback(cpu, &frame, 0x8099fdu);
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result = Lufia2WriteSoundDriverMode(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

static Lufia2ExecutionResult ActorBridgeSoundCommand(
    const Lufia2Memory *memory, Lufia2CpuState *state,
    Lufia2PushedChildCall child, Lufia2MusicCheckpoint checkpoint,
    void *context) {
    (void)checkpoint;
    return Lufia2SendSoundCommand(memory, state, child, context);
}

static Lufia2ExecutionResult ActorBridgeImmediateSound(
    const Lufia2Memory *memory, Lufia2CpuState *state,
    Lufia2PushedChildCall child, Lufia2MusicCheckpoint checkpoint,
    void *context) {
    (void)checkpoint;
    return Lufia2SendImmediateSound(memory, state, child, context);
}

RecompReturn Lufia2DecompBridge_80953B(CpuState *cpu) {
    return ActorBridgeMusic(cpu, 0x80953bu, ActorBridgeSoundCommand, 3u);
}

RecompReturn Lufia2DecompBridge_848775(CpuState *cpu) {
    return ActorBridgeMusic(cpu, 0x848775u, ActorBridgeImmediateSound, 3u);
}

RecompReturn Lufia2DecompBridge_8387A3(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x8387a3u,
        Lufia2FieldLoadObjectActionHeader, 3u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_838848(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x838848u,
        Lufia2FieldLoadObjectControlHeader, 2u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_838874(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x838874u,
        Lufia2FieldLoadObjectRegionHeader, 3u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_8EC338(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x8ec338u,
        Lufia2FieldSelectObjectCondition, 3u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_8EC34F(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x8ec34fu,
        Lufia2FieldResolveObjectCondition, 3u, 0x1f20u);
}

typedef Lufia2ExecutionResult (*ActorSoundDriverFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeSoundDriver(
    CpuState *cpu, uint32_t entry, ActorSoundDriverFunction run, uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;
    if (cpu->emulation || cpu->_flag_D)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    result = run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_809554(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809554u, Lufia2PlaySoundResource, 3u);
}

RecompReturn Lufia2DecompBridge_80956A(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x80956au, Lufia2SendUncheckedSoundCommand, 3u);
}

RecompReturn Lufia2DecompBridge_80957D(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x80957du, Lufia2LoadSoundResource, 2u);
}

RecompReturn Lufia2DecompBridge_8095D2(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8095d2u, Lufia2SoundDriverRequest05, 3u);
}

RecompReturn Lufia2DecompBridge_8095DF(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8095dfu, Lufia2SoundDriverRead1E, 3u);
}

RecompReturn Lufia2DecompBridge_8095F0(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8095f0u, Lufia2SoundDriverRead0F, 3u);
}

RecompReturn Lufia2DecompBridge_809612(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809612u, Lufia2SoundDriverWrite0C, 3u);
}

RecompReturn Lufia2DecompBridge_809623(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809623u, Lufia2SoundDriverWrite0D, 3u);
}

RecompReturn Lufia2DecompBridge_809634(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809634u, Lufia2SoundDriverRead18, 3u);
}

RecompReturn Lufia2DecompBridge_809644(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809644u, Lufia2SoundDriverWrite09, 3u);
}

RecompReturn Lufia2DecompBridge_809655(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809655u, Lufia2SoundDriverWrite1A, 3u);
}

RecompReturn Lufia2DecompBridge_80966F(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x80966fu, Lufia2SoundDriverWrite1B, 3u);
}

RecompReturn Lufia2DecompBridge_809685(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809685u, Lufia2SoundDriverRequest15, 3u);
}

RecompReturn Lufia2DecompBridge_80969F(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x80969fu, Lufia2PrepareSongResource, 3u);
}

RecompReturn Lufia2DecompBridge_8096CC(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8096ccu, Lufia2SoundDriverWrite0A, 3u);
}

RecompReturn Lufia2DecompBridge_8096DD(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8096ddu, Lufia2SoundDriverWriteComplement10, 3u);
}

RecompReturn Lufia2DecompBridge_8096F2(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8096f2u, Lufia2SoundDriverRead08, 3u);
}

RecompReturn Lufia2DecompBridge_8096B9(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8096b9u, Lufia2WaitSoundDriverFlagsClear, 3u);
}

RecompReturn Lufia2DecompBridge_809703(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809703u, Lufia2InitializeSoundResourceTable, 2u);
}

static RecompReturn ActorBridgeSoundLeaf(
    CpuState *cpu, uint32_t entry, ActorWholeFunction run, uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    if (cpu->emulation)
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_8099F4(CpuState *cpu) {
    return ActorBridgeSoundLeaf(cpu, 0x8099f4u, Lufia2WriteSoundTransferMarker, 2u);
}

RecompReturn Lufia2DecompBridge_8097DA(CpuState *cpu) {
    return ActorBridgeSoundLeaf(cpu, 0x8097dau, Lufia2AdvanceSoundSourceBank, 2u);
}

RecompReturn Lufia2DecompBridge_8099B2(CpuState *cpu) {
    return ActorBridgeSoundLeaf(cpu, 0x8099b2u, Lufia2CheckSoundDriverSignature, 3u);
}

RecompReturn Lufia2DecompBridge_8099CA(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8099cau, Lufia2SoundDriverWrite1F, 3u);
}

RecompReturn Lufia2DecompBridge_8099D8(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8099d8u, Lufia2SoundDriverWrite20, 3u);
}

RecompReturn Lufia2DecompBridge_8099E6(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8099e6u, Lufia2SoundDriverWrite21, 3u);
}


RecompReturn Lufia2DecompBridge_809747(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809747u, Lufia2BeginQueuedSoundResource, 2u);
}

RecompReturn Lufia2DecompBridge_809786(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809786u, Lufia2UpdateSoundResourceQueue, 3u);
}

RecompReturn Lufia2DecompBridge_809886(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809886u, Lufia2UploadSoundResourceSlot, 2u);
}

RecompReturn Lufia2DecompBridge_809528(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x809528u, Lufia2SoundDriverRequest03, 3u);
}


typedef Lufia2ExecutionResult (*ActorAnimationFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeObjectAnimation(
    CpuState *cpu, uint32_t entry, ActorAnimationFunction run) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (!ActorBridgeSupported(cpu, 0, 0) || cpu->PB != 0x83u || cpu->D ||
        cpu->S < 0x1f40u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    if ((entry == 0x83888cu || entry == 0x8388d9u) && cpu->DB != 0x7fu)
        return ActorBridgeFallback(cpu, &frame, entry);
    const ActorBridgeFrame active = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, active, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, active.entry_s, active.hrv);
    return ActorBridgeReturn(cpu, &active, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_838927(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x838927u, Lufia2FieldSaveAnimationRegion, 2u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_83894C(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83894cu, Lufia2FieldRestoreAnimationRegion, 2u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_838971(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x838971u, Lufia2FieldRedrawAnimatedRegion);
}

RecompReturn Lufia2DecompBridge_83881D(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x83881du, Lufia2FieldQueueObjectControlSound);
}

RecompReturn Lufia2DecompBridge_83888C(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x83888cu, Lufia2FieldCloseAnimatedRegion);
}

RecompReturn Lufia2DecompBridge_8388D9(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x8388d9u, Lufia2FieldOpenAnimatedRegion);
}

RecompReturn Lufia2DecompBridge_83873F(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x83873fu, Lufia2FieldApplyInitialObjectRegion);
}

RecompReturn Lufia2DecompBridge_838761(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x838761u, Lufia2FieldApplyAlternateObjectRegion);
}

RecompReturn Lufia2DecompBridge_838783(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x838783u, Lufia2FieldAnimateObjectAction);
}

RecompReturn Lufia2DecompBridge_8387CC(CpuState *cpu) {
    return ActorBridgeObjectAnimation(cpu, 0x8387ccu, Lufia2FieldAnimateObjectControl);
}

static RecompReturn ActorBridgeAnimationSlots(CpuState *cpu) {
    const ActorBridgeFrame fallback = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->PB != 0x83u ||
        cpu->D || cpu->S < 0x1f60u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &fallback, 0x838682u);
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2FieldAnimationTickSlots(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_83898E(CpuState *cpu) {
    return ActorBridgeObjectCollision(cpu, 0x83898eu,
        Lufia2FieldFlagAnimationRow, 2u, 0x1f20u);
}

RecompReturn Lufia2DecompBridge_809911(CpuState *cpu) {
    if (cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x809911u);
    }
    return ActorBridgeSoundDriver(cpu, 0x809911u, Lufia2UploadSoundPayload, 3u);
}

RecompReturn Lufia2DecompBridge_809A0A(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x809a0au, Lufia2WaitSoundDriverReply, 2u);
}

RecompReturn Lufia2DecompBridge_809945(CpuState *cpu) {
    if (!cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x809945u);
    }
    return ActorBridgeSoundDriver(cpu, 0x809945u, Lufia2SendSoundPayload, 2u);
}

RecompReturn Lufia2DecompBridge_8098A5(CpuState *cpu) {
    return ActorBridgeSoundDriver(cpu, 0x8098a5u, Lufia2SendSoundResourceHeader, 2u);
}

RecompReturn Lufia2DecompBridge_8097E5(CpuState *cpu) {
    if (!cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8097e5u);
    }
    return ActorBridgeSoundDriver(cpu, 0x8097e5u, Lufia2SendQueuedSoundChunk, 2u);
}

typedef Lufia2ExecutionResult (*ActorSceneBootstrapFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeSceneBootstrap(
    CpuState *cpu, uint32_t entry, ActorSceneBootstrapFunction run,
    uint8_t frame_size, bool byte_input) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u ||
        (byte_input && (!cpu->m_flag || cpu->x_flag)))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_83AC7A(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83ac7au);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83ac7au, Lufia2FieldResetSpriteBuffer, 3u);
}

RecompReturn Lufia2DecompBridge_83B512(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b512u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83b512u, Lufia2FieldResetObjectAnimation, 2u);
}

RecompReturn Lufia2DecompBridge_83B5AD(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b5adu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83b5adu, Lufia2FieldSelectSceneRecordBase, 2u);
}

RecompReturn Lufia2DecompBridge_80A368(CpuState *cpu) {
    if (cpu->PB != 0x80u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80a368u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80a368u, Lufia2SceneResetTextState, 2u);
}

RecompReturn Lufia2DecompBridge_80BE89(CpuState *cpu) {
    return ActorBridgeSceneBootstrap(cpu, 0x80be89u, Lufia2SceneRunStartRecord, 2u, true);
}

RecompReturn Lufia2DecompBridge_80BE4D(CpuState *cpu) {
    return ActorBridgeSceneBootstrap(cpu, 0x80be4du, Lufia2SceneRunInitialRecord, 3u, false);
}

RecompReturn Lufia2DecompBridge_80BE61(CpuState *cpu) {
    return ActorBridgeSceneBootstrap(cpu, 0x80be61u, Lufia2SceneRunResumeRecord, 3u, false);
}

RecompReturn Lufia2DecompBridge_80BE75(CpuState *cpu) {
    return ActorBridgeSceneBootstrap(cpu, 0x80be75u, Lufia2SceneRunTransitionRecord, 3u, false);
}

RecompReturn Lufia2DecompBridge_80BEAF(CpuState *cpu) {
    return ActorBridgeSceneBootstrap(cpu, 0x80beafu, Lufia2SceneRunMapText, 3u, false);
}

typedef Lufia2ExecutionResult (*ActorBootstrapFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeActorBootstrap(
    CpuState *cpu, uint32_t entry, ActorBootstrapFunction run,
    uint8_t frame_size, bool byte_input) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u ||
        (byte_input && (!cpu->m_flag || cpu->x_flag)))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_83A6DF(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a6dfu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83a6dfu, Lufia2ActorResetTransientState, 2u);
}

RecompReturn Lufia2DecompBridge_83A71C(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a71cu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83a71cu, Lufia2ActorSetFinePosition, 3u);
}

RecompReturn Lufia2DecompBridge_83A97E(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a97eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83a97eu, Lufia2ActorHasSpecialSceneSprite, 2u);
}

RecompReturn Lufia2DecompBridge_83A998(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a998u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83a998u, Lufia2ObjectResetSceneSprites, 2u);
}

RecompReturn Lufia2DecompBridge_83A9E5(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a9e5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83a9e5u, Lufia2ActorReadSpriteDescriptor, 3u);
}

RecompReturn Lufia2DecompBridge_83AA7D(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83aa7du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83aa7du, Lufia2ActorSelectSpriteTables, 3u);
}

RecompReturn Lufia2DecompBridge_83AA30(CpuState *cpu) {
    return ActorBridgeActorBootstrap(cpu, 0x83aa30u, Lufia2ActorSetSpriteHeightOffset, 3u, false);
}

RecompReturn Lufia2DecompBridge_83A9D0(CpuState *cpu) {
    return ActorBridgeActorBootstrap(cpu, 0x83a9d0u, Lufia2ActorRefreshSpriteDescriptor, 3u, false);
}

typedef Lufia2ExecutionResult (*ActorSceneRecordsFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeSceneRecords(
    CpuState *cpu, uint32_t entry, ActorSceneRecordsFunction run,
    uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    if (cpu->emulation || cpu->_flag_D || cpu->PB != (uint8_t)(entry >> 16))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_83A686(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a686u);
    }
    return ActorBridgeSceneRecords(cpu, 0x83a686u, Lufia2ActorResetSceneSlots, 2u);
}

RecompReturn Lufia2DecompBridge_80C01D(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c01du);
    }
    return ActorBridgeSceneRecords(cpu, 0x80c01du, Lufia2SceneReadActorAttributes, 3u);
}

RecompReturn Lufia2DecompBridge_80C05C(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c05cu);
    }
    return ActorBridgeSceneRecords(cpu, 0x80c05cu, Lufia2SceneReadActorCell, 3u);
}

RecompReturn Lufia2DecompBridge_80C093(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c093u);
    }
    return ActorBridgeSceneRecords(cpu, 0x80c093u, Lufia2SceneReadActorPair, 2u);
}

RecompReturn Lufia2DecompBridge_80C1A7(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c1a7u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80c1a7u, Lufia2SceneApplyActorCell, 3u);
}

RecompReturn Lufia2DecompBridge_83AAE5(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83aae5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83aae5u, Lufia2ActorQueueSceneSpriteUpload, 2u);
}

RecompReturn Lufia2DecompBridge_83FCD1(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83fcd1u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83fcd1u, Lufia2ObjectQueueSceneSpriteUpload, 2u);
}

typedef Lufia2ExecutionResult (*ActorSceneOwnersFunction)(
    const Lufia2Memory *, Lufia2CpuState *, Lufia2PushedChildCall, void *);

static RecompReturn ActorBridgeSceneOwners(
    CpuState *cpu, uint32_t entry, ActorSceneOwnersFunction run,
    uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    if (cpu->emulation || cpu->_flag_D || cpu->PB != (uint8_t)(entry >> 16))
        return ActorBridgeFallback(cpu, &frame, entry);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_83A82E(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->D != 0u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a82eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83a82eu, Lufia2ActorRebuildSceneState, 3u);
}

RecompReturn Lufia2DecompBridge_83ADCA(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83adcau);
    }
    return ActorBridgeSceneRecords(cpu, 0x83adcau, Lufia2FieldResetScene, 3u);
}

RecompReturn Lufia2DecompBridge_83ADDF(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83addfu);
    }
    return ActorBridgeSceneRecords(cpu, 0x83addfu, Lufia2FieldResetSavedScene, 2u);
}

RecompReturn Lufia2DecompBridge_83B503(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b503u);
    }
    return ActorBridgeSceneRecords(cpu, 0x83b503u, Lufia2FieldMarkCurrentMap, 2u);
}

RecompReturn Lufia2DecompBridge_83B52E(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b52eu);
    }
    return ActorBridgeSceneRecords(cpu, 0x83b52eu, Lufia2FieldResumeSceneSong, 2u);
}

RecompReturn Lufia2DecompBridge_80E898(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->_flag_D || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e898u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80e898u, Lufia2EventGetFlagMask, 3u);
}

RecompReturn Lufia2DecompBridge_808285(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->_flag_D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808285u);
    }
    return ActorBridgeSceneRecords(cpu, 0x808285u, Lufia2SceneUploadRequestedTilemaps, 3u);
}

RecompReturn Lufia2DecompBridge_848328(CpuState *cpu) {
    if (cpu->PB != 0x84u || cpu->_flag_D || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x848328u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x848328u, Lufia2TextClearWindowBuffer, 3u);
}

RecompReturn Lufia2DecompBridge_848204(CpuState *cpu) {
    if (cpu->PB != 0x84u || cpu->_flag_D || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x848204u);
    }
    return ActorBridgeSceneRecords(cpu, 0x848204u, Lufia2FieldRebuildPartyActors, 3u);
}

RecompReturn Lufia2DecompBridge_8482D5(CpuState *cpu) {
    if (cpu->PB != 0x84u || cpu->_flag_D || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8482d5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8482d5u, Lufia2ActorPreparePartyOffsets, 3u);
}

RecompReturn Lufia2DecompBridge_83AAAF(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83aaafu);
    }
    return ActorBridgeSceneRecords(cpu, 0x83aaafu, Lufia2ActorReleaseSceneSprite, 3u);
}

RecompReturn Lufia2DecompBridge_83FA12(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83fa12u);
    }
    return ActorBridgeSceneRecords(cpu, 0x83fa12u, Lufia2ActorClearSceneOccupancy, 3u);
}

RecompReturn Lufia2DecompBridge_83AAE1(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->_flag_D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83aae1u);
    }
    return ActorBridgeSceneRecords(cpu, 0x83aae1u, Lufia2ActorUploadSceneSprite, 3u);
}

RecompReturn Lufia2DecompBridge_859AF4(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859af4u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859af4u, Lufia2BattleQueueBackgroundTilemap, 3u);
}

RecompReturn Lufia2DecompBridge_859B0B(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b0bu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b0bu, Lufia2BattleQueueBackgroundTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859B22(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b22u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b22u, Lufia2BattleQueueExtraBackgroundTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859B39(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b39u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b39u, Lufia2BattleQueueSpriteTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859B50(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b50u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b50u, Lufia2BattleQueueSecondarySpriteTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859B67(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b67u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b67u, Lufia2BattleQueueWindowGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_859B7E(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b7eu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b7eu, Lufia2BattleQueueWindowTilemapHalf, 3u);
}

RecompReturn Lufia2DecompBridge_859B95(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859b95u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859b95u, Lufia2BattleQueueAuxiliaryTilemap, 3u);
}

RecompReturn Lufia2DecompBridge_859BAC(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859bacu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859bacu, Lufia2BattleQueueOverlayTilemap, 3u);
}

RecompReturn Lufia2DecompBridge_859BC3(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859bc3u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859bc3u, Lufia2BattleQueueWindowTilemaps, 3u);
}

RecompReturn Lufia2DecompBridge_859BF1(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859bf1u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859bf1u, Lufia2BattleQueueShortWindowGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_859C08(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c08u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c08u, Lufia2BattleQueueWindowTilemap, 3u);
}

RecompReturn Lufia2DecompBridge_859ADD(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859addu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859addu, Lufia2BattleQueueTilesAt3000, 3u);
}

RecompReturn Lufia2DecompBridge_859C1F(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c1fu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c1fu, Lufia2BattleQueueTilesAt4000, 3u);
}

RecompReturn Lufia2DecompBridge_859C36(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c36u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c36u, Lufia2BattleQueueTilesAt4800, 3u);
}

RecompReturn Lufia2DecompBridge_859C4D(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c4du);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c4du, Lufia2BattleQueueTilesAt1800, 3u);
}

RecompReturn Lufia2DecompBridge_859C64(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c64u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c64u, Lufia2BattleQueueTilePatchAt4400, 3u);
}

RecompReturn Lufia2DecompBridge_859C7B(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c7bu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c7bu, Lufia2BattleQueueShortAuxiliaryTilemap, 3u);
}

RecompReturn Lufia2DecompBridge_859C92(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859c92u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859c92u, Lufia2BattleQueueAlternateTilesAt4000, 3u);
}

RecompReturn Lufia2DecompBridge_859CA9(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859ca9u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859ca9u, Lufia2BattleQueueWindowTilemapPair, 3u);
}

RecompReturn Lufia2DecompBridge_859CC0(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859cc0u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859cc0u, Lufia2BattleQueueMinimalAuxiliaryTilemap, 3u);
}

RecompReturn Lufia2DecompBridge_859D05(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d05u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d05u, Lufia2BattleQueueSmallSpriteTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859D1C(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d1cu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d1cu, Lufia2BattleQueueAlternateSpriteTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859D33(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d33u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d33u, Lufia2BattleQueueMidSpriteTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859D4A(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d4au);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d4au, Lufia2BattleQueueUpperSpriteTiles, 3u);
}

RecompReturn Lufia2DecompBridge_859D61(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d61u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d61u, Lufia2BattleQueueTilesAt2000, 3u);
}

RecompReturn Lufia2DecompBridge_859D78(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d78u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d78u, Lufia2BattleQueueTilemapAt0E00, 3u);
}

RecompReturn Lufia2DecompBridge_859D8F(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859d8fu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859d8fu, Lufia2BattleQueueSmallTilesAt4000, 3u);
}

RecompReturn Lufia2DecompBridge_859DA6(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859da6u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859da6u, Lufia2BattleQueueTilesAt4400, 3u);
}

RecompReturn Lufia2DecompBridge_859DBD(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859dbdu);
    }
    return ActorBridgeSceneRecords(cpu, 0x859dbdu, Lufia2BattleQueueTilesAt4A00, 3u);
}

RecompReturn Lufia2DecompBridge_83AFCD(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83afcdu);
    }
    return ActorBridgeSceneRecords(cpu, 0x83afcdu, Lufia2FieldFadeIn, 3u);
}

RecompReturn Lufia2DecompBridge_83AFEA(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83afeau);
    }
    return ActorBridgeSceneRecords(cpu, 0x83afeau, Lufia2FieldFadeOut, 3u);
}

RecompReturn Lufia2DecompBridge_8482E7(CpuState *cpu) {
    if (cpu->PB != 0x84u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8482e7u);
    }
    return ActorBridgeSceneRecords(cpu, 0x8482e7u, Lufia2SceneUploadFixedGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_848311(CpuState *cpu) {
    if (cpu->PB != 0x84u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x848311u);
    }
    return ActorBridgeSceneRecords(cpu, 0x848311u, Lufia2FieldRestoreBackgroundLayers, 3u);
}

RecompReturn Lufia2DecompBridge_859906(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859906u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859906u, Lufia2BattleShowActionMessage, 3u);
}

RecompReturn Lufia2DecompBridge_859A71(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859a71u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x859a71u, Lufia2BattleClearMessageRow, 2u);
}

RecompReturn Lufia2DecompBridge_859510(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859510u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x859510u, Lufia2BattleCopyRecordName, 3u);
}

RecompReturn Lufia2DecompBridge_859578(CpuState *cpu) {
    if (cpu->PB != 0x85u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859578u);
    }
    return ActorBridgeSceneRecords(cpu, 0x859578u, Lufia2BattleLoadIpActionName, 3u);
}

RecompReturn Lufia2DecompBridge_859532(CpuState *cpu) {
    if (cpu->PB != 0x85u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859532u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x859532u, Lufia2BattleLoadStatusMessage, 3u);
}

RecompReturn Lufia2DecompBridge_85C4F1(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c4f1u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85c4f1u, Lufia2BattleExpandActionMessage, 3u);
}

RecompReturn Lufia2DecompBridge_86E5E2(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e5e2u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e5e2u, Lufia2WorldMapPackFirstSpriteHighBits, 2u);
}

RecompReturn Lufia2DecompBridge_86E5ED(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e5edu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e5edu, Lufia2WorldMapPackSecondSpriteHighBits, 2u);
}

RecompReturn Lufia2DecompBridge_86E5FA(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e5fau);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e5fau, Lufia2WorldMapPackThirdSpriteHighBits, 2u);
}

RecompReturn Lufia2DecompBridge_86E609(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e609u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e609u, Lufia2WorldMapPackFourthSpriteHighBits, 2u);
}

RecompReturn Lufia2DecompBridge_86E617(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e617u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e617u, Lufia2WorldMapInitializeSlots, 2u);
}

RecompReturn Lufia2DecompBridge_86E6C4(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e6c4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e6c4u, Lufia2WorldMapInitializeObjectRecords, 2u);
}

RecompReturn Lufia2DecompBridge_86E6F3(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e6f3u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e6f3u, Lufia2WorldMapSelectObjectRecord, 2u);
}

RecompReturn Lufia2DecompBridge_86E709(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e709u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86e709u, Lufia2WorldMapIndexObjectRecord, 2u);
}

RecompReturn Lufia2DecompBridge_80C9C0(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c9c0u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80c9c0u, Lufia2TextExpandSceneString, 3u);
}

RecompReturn Lufia2DecompBridge_80C7C2(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c7c2u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80c7c2u, Lufia2TextDrawSceneGlyph, 3u);
}

RecompReturn Lufia2DecompBridge_80C5DD(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c5ddu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80c5ddu, Lufia2TextWriteSceneWindowRow, 2u);
}

RecompReturn Lufia2DecompBridge_80C61D(CpuState *cpu) {
    if (cpu->PB != 0x80u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c61du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80c61du, Lufia2TextClearUploadRows, 2u);
}

RecompReturn Lufia2DecompBridge_8089AA(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8089aau);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8089aau, Lufia2MenuFormatNumberDigits, 2u);
}

RecompReturn Lufia2DecompBridge_8089D0(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8089d0u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8089d0u, Lufia2MenuAppendNumberDigit, 2u);
}

RecompReturn Lufia2DecompBridge_80C825(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->D) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c825u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80c825u, Lufia2TextUpdateSceneLabel, 3u);
}

RecompReturn Lufia2DecompBridge_80C8D5(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->D || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c8d5u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80c8d5u, Lufia2TextRenderSceneLabel, 3u);
}

RecompReturn Lufia2DecompBridge_868DBB(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868dbbu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x868dbbu, Lufia2MenuResetDisplayRequests, 3u);
}

RecompReturn Lufia2DecompBridge_868E79(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868e79u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x868e79u, Lufia2MenuInitializeDisplayState, 3u);
}

RecompReturn Lufia2DecompBridge_868EB0(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868eb0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868eb0u, Lufia2MenuLoadPrimaryGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_868EF1(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868ef1u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868ef1u, Lufia2MenuLoadAuxiliaryGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_868F1B(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868f1bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x868f1bu, Lufia2MenuLoadLargeGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_868F45(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868f45u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868f45u, Lufia2MenuLoadMediumGraphics, 3u);
}

RecompReturn Lufia2DecompBridge_8EBC99(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebc99u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8ebc99u, Lufia2FieldSelectMenuActor, 3u);
}

RecompReturn Lufia2DecompBridge_8289A4(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8289a4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8289a4u, Lufia2MenuInitializeAuxiliarySprites, 2u);
}

RecompReturn Lufia2DecompBridge_8EB000(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb000u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8eb000u, Lufia2FieldRunMenu, 3u);
}

RecompReturn Lufia2DecompBridge_83B82F(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b82fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83b82fu, Lufia2FieldRefreshMenuSelection, 3u);
}

RecompReturn Lufia2DecompBridge_829A4E(CpuState *cpu) {
    if (cpu->PB != 0x82u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829a4eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x829a4eu, Lufia2MenuRunMainScreen, 3u);
}

RecompReturn Lufia2DecompBridge_80B404(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->Y >= 5u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80b404u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80b404u, Lufia2FieldQueueMenuActorUpdates, 3u);
}

RecompReturn Lufia2DecompBridge_82A432(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82a432u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82a432u, Lufia2MenuRunMainInput, 2u);
}

RecompReturn Lufia2DecompBridge_82A582(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82a582u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82a582u, Lufia2MenuApplySpellList, 2u);
}

RecompReturn Lufia2DecompBridge_82A62D(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82a62du);
    }
    return ActorBridgeSceneOwners(cpu, 0x82a62du, Lufia2MenuRunAlternateSelection, 2u);
}

RecompReturn Lufia2DecompBridge_829AE0(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829ae0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x829ae0u, Lufia2MenuRefreshMainDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_8293CF(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8293cfu);
    }
    return ActorBridgeSceneOwners(cpu, 0x8293cfu, Lufia2MenuPlacePartyPortraits, 2u);
}

RecompReturn Lufia2DecompBridge_8299BE(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8299beu);
    }
    return ActorBridgeSceneOwners(cpu, 0x8299beu, Lufia2MenuInitializePartyPortrait, 2u);
}

RecompReturn Lufia2DecompBridge_82999B(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82999bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82999bu, Lufia2MenuInitializePartyPortraits, 2u);
}

RecompReturn Lufia2DecompBridge_829B10(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829b10u);
    }
    return ActorBridgeSceneOwners(cpu, 0x829b10u, Lufia2MenuBuildMainWindows, 2u);
}

RecompReturn Lufia2DecompBridge_828704(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828704u);
    }
    return ActorBridgeSceneOwners(cpu, 0x828704u, Lufia2MenuPresentMainDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_829EF2(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829ef2u);
    }
    return ActorBridgeSceneOwners(cpu, 0x829ef2u, Lufia2MenuBuildAlternateCursor, 2u);
}

RecompReturn Lufia2DecompBridge_829214(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829214u);
    }
    return ActorBridgeSceneOwners(cpu, 0x829214u, Lufia2MenuClearSpriteMask, 3u);
}

RecompReturn Lufia2DecompBridge_82891E(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82891eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82891eu, Lufia2MenuLoadCursorGrid, 2u);
}

RecompReturn Lufia2DecompBridge_82895B(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82895bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82895bu, Lufia2MenuSetCursorStyle, 2u);
}

RecompReturn Lufia2DecompBridge_82898D(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82898du);
    }
    return ActorBridgeSceneOwners(cpu, 0x82898du, Lufia2MenuValidatePartyCursor, 2u);
}

RecompReturn Lufia2DecompBridge_8289C4(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8289c4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8289c4u, Lufia2MenuBuildCursorPair, 2u);
}

RecompReturn Lufia2DecompBridge_8289EC(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8289ecu);
    }
    return ActorBridgeSceneOwners(cpu, 0x8289ecu, Lufia2MenuMoveCursorPair, 2u);
}

RecompReturn Lufia2DecompBridge_868B1C(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868b1cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x868b1cu, Lufia2MenuFadeDisplayIn, 3u);
}

RecompReturn Lufia2DecompBridge_868B32(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868b32u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868b32u, Lufia2MenuFadeDisplayOut, 3u);
}

RecompReturn Lufia2DecompBridge_8293F6(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8293f6u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8293f6u, Lufia2MenuPrepareDisplayText, 2u);
}

RecompReturn Lufia2DecompBridge_8289BC(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x8289bcu);
    }
    return ActorBridgeSceneOwners(cpu,0x8289bcu,Lufia2MenuReplaceCursor,2u);
}

RecompReturn Lufia2DecompBridge_829C10(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x829c10u);
    }
    return ActorBridgeSceneOwners(cpu,0x829c10u,Lufia2MenuInitializeListScroll,2u);
}

RecompReturn Lufia2DecompBridge_829B51(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x829b51u);
    }
    return ActorBridgeSceneOwners(cpu,0x829b51u,Lufia2MenuBuildListWindows,2u);
}

RecompReturn Lufia2DecompBridge_82A658(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82a658u);
    }
    return ActorBridgeSceneOwners(cpu,0x82a658u,Lufia2MenuRunListInput,2u);
}

RecompReturn Lufia2DecompBridge_82A711(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82a711u);
    }
    return ActorBridgeSceneOwners(cpu,0x82a711u,Lufia2MenuRunListSelection,2u);
}

RecompReturn Lufia2DecompBridge_80882E(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80882eu, Lufia2UploadTilemapBlock, 2u);
}

RecompReturn Lufia2DecompBridge_80884F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80884fu, Lufia2StartListedDma, 2u);
}

RecompReturn Lufia2DecompBridge_82FBE5(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82fbe5u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82fbe5u, Lufia2MenuSelectedListOffset, 2u);
}

RecompReturn Lufia2DecompBridge_828CE9(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828ce9u);
    }
    return ActorBridgeSceneOwners(cpu, 0x828ce9u, Lufia2MenuScrollListPage, 2u);
}

RecompReturn Lufia2DecompBridge_828CC1(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828cc1u);
    }
    return ActorBridgeSceneOwners(cpu, 0x828cc1u, Lufia2MenuInitializeScrollStep, 2u);
}

RecompReturn Lufia2DecompBridge_828C43(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828c43u);
    }
    return ActorBridgeSceneOwners(cpu, 0x828c43u, Lufia2MenuSetScrollPosition, 2u);
}

RecompReturn Lufia2DecompBridge_828C85(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828c85u);
    }
    return ActorBridgeSceneOwners(cpu, 0x828c85u, Lufia2MenuInitializeScrollRange, 2u);
}

RecompReturn Lufia2DecompBridge_828C9C(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828c9cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x828c9cu, Lufia2MenuInitializeScrollSprite, 2u);
}

RecompReturn Lufia2DecompBridge_868D47(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868d47u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868d47u, Lufia2MenuLoadListParameters, 3u);
}

RecompReturn Lufia2DecompBridge_829C52(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829c52u);
    }
    return ActorBridgeSceneOwners(cpu, 0x829c52u, Lufia2MenuInitializeListCursor, 2u);
}

RecompReturn Lufia2DecompBridge_82AC84(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ac84u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82ac84u, Lufia2MenuDrawListPage, 2u);
}

RecompReturn Lufia2DecompBridge_82ADA3(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ada3u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82ada3u, Lufia2MenuMoveListMarkerDown, 2u);
}

RecompReturn Lufia2DecompBridge_82AE53(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ae53u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82ae53u, Lufia2MenuMoveListMarkerUp, 2u);
}

RecompReturn Lufia2DecompBridge_82AD20(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ad20u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82ad20u, Lufia2MenuAnimateListUp, 2u);
}

RecompReturn Lufia2DecompBridge_82ADB4(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82adb4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82adb4u, Lufia2MenuAnimateListDown, 2u);
}

RecompReturn Lufia2DecompBridge_808DB3(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808db3u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x808db3u, Lufia2MenuWriteGlyph, 2u);
}

RecompReturn Lufia2DecompBridge_808DF9(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808df9u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x808df9u, Lufia2MenuAdvanceTextRow, 2u);
}

RecompReturn Lufia2DecompBridge_808E0F(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808e0fu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x808e0fu, Lufia2MenuResetTextWidth, 2u);
}

RecompReturn Lufia2DecompBridge_808D5D(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808d5du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x808d5du, Lufia2MenuSetTextPalette, 2u);
}

RecompReturn Lufia2DecompBridge_838192(CpuState *cpu) {
    if (cpu->PB != 0x83u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838192u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838192u, Lufia2FieldFrameServices, 3u);
}

RecompReturn Lufia2DecompBridge_83812E(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83812eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83812eu, Lufia2FieldPollHpRecovery, 2u);
}

RecompReturn Lufia2DecompBridge_838160(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838160u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838160u, Lufia2FieldPollMpRecovery, 2u);
}

RecompReturn Lufia2DecompBridge_83813B(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83813bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83813bu, Lufia2FieldStartHpRecovery, 2u);
}

RecompReturn Lufia2DecompBridge_83816D(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83816du);
    }
    return ActorBridgeSceneOwners(cpu, 0x83816du, Lufia2FieldStartMpRecovery, 2u);
}

RecompReturn Lufia2DecompBridge_8382A0(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8382a0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8382a0u, Lufia2FieldFindRecoveryObject, 2u);
}

RecompReturn Lufia2DecompBridge_83831E(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83831eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83831eu, Lufia2FieldScalePaletteGreen, 2u);
}

RecompReturn Lufia2DecompBridge_838323(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838323u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838323u, Lufia2FieldScalePaletteBlue, 2u);
}

RecompReturn Lufia2DecompBridge_8382B2(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8382b2u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8382b2u, Lufia2FieldPrepareRecoveryPalette, 2u);
}

RecompReturn Lufia2DecompBridge_838327(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838327u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838327u, Lufia2FieldLoadRecoveryGraphics, 2u);
}

RecompReturn Lufia2DecompBridge_83834A(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83834au);
    }
    return ActorBridgeSceneOwners(cpu, 0x83834au, Lufia2FieldConfigureRecoveryObjects, 2u);
}

RecompReturn Lufia2DecompBridge_838103(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838103u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838103u, Lufia2FieldProcessRequests, 2u);
}

RecompReturn Lufia2DecompBridge_8088DA(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8088dau);
    }
    return ActorBridgeSceneOwners(cpu, 0x8088dau, Lufia2MenuWriteCharacter, 2u);
}

RecompReturn Lufia2DecompBridge_8088C8(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8088c8u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8088c8u, Lufia2MenuWriteRawControl, 2u);
}

RecompReturn Lufia2DecompBridge_80832D(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || !cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80832du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80832du, Lufia2RefillRandomTable, 2u);
}

RecompReturn Lufia2DecompBridge_859337(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859337u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x859337u, Lufia2BattleInsertTurn, 3u);
}

RecompReturn Lufia2DecompBridge_8594E7(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8594e7u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8594e7u, Lufia2BattleCopyMessageName, 3u);
}

RecompReturn Lufia2DecompBridge_80F734(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80f734u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80f734u, Lufia2FieldLocateCell, 2u);
}

RecompReturn Lufia2DecompBridge_80F81C(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80f81cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80f81cu, Lufia2FieldDivisionDelay, 2u);
}

RecompReturn Lufia2DecompBridge_80F6AA(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80f6aau);
    }
    return ActorBridgeSceneOwners(cpu, 0x80f6aau, Lufia2FieldCellIndex, 2u);
}

RecompReturn Lufia2DecompBridge_80E8B9(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e8b9u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e8b9u, Lufia2FieldReadEventByte, 2u);
}

RecompReturn Lufia2DecompBridge_82C37B(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c37bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c37bu, Lufia2CapsuleClearFlags, 2u);
}

RecompReturn Lufia2DecompBridge_82C38B(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c38bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c38bu, Lufia2CapsuleResolveRecord, 2u);
}

RecompReturn Lufia2DecompBridge_82C3C4(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c3c4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c3c4u, Lufia2CapsuleFormIndex, 2u);
}

RecompReturn Lufia2DecompBridge_82C3D3(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c3d3u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c3d3u, Lufia2CapsuleSavedOffsets, 2u);
}

RecompReturn Lufia2DecompBridge_82C3F8(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c3f8u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c3f8u, Lufia2CapsuleLoadSavedStats, 2u);
}

RecompReturn Lufia2DecompBridge_85DD19(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85dd19u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85dd19u, Lufia2BattleRandomizeTurnPriority, 3u);
}

RecompReturn Lufia2DecompBridge_81BDCC(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81bdccu);
    }
    return ActorBridgeSceneOwners(cpu, 0x81bdccu, Lufia2BattleMirrorSpriteBlock, 2u);
}

RecompReturn Lufia2DecompBridge_81BDC8(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81bdc8u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81bdc8u, Lufia2BattleMirrorSpriteBlockFar, 3u);
}

RecompReturn Lufia2DecompBridge_80F5ED(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80f5edu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80f5edu, Lufia2FieldRenderMetatileColumn, 2u);
}

RecompReturn Lufia2DecompBridge_80F64E(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80f64eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80f64eu, Lufia2FieldRenderMetatileRow, 2u);
}

RecompReturn Lufia2DecompBridge_80E8D0(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e8d0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e8d0u, Lufia2FieldRewindEventByte, 2u);
}

RecompReturn Lufia2DecompBridge_80E8AD(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e8adu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e8adu, Lufia2FieldReadEventWord, 2u);
}

RecompReturn Lufia2DecompBridge_80E8F4(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e8f4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e8f4u, Lufia2FieldSetEventPointer, 2u);
}

RecompReturn Lufia2DecompBridge_80E9BC(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e9bcu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e9bcu, Lufia2FieldResolveEventVariable, 2u);
}

RecompReturn Lufia2DecompBridge_80E9ED(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e9edu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e9edu, Lufia2FieldResolveEventValue, 2u);
}

RecompReturn Lufia2DecompBridge_80E912(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e912u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80e912u, Lufia2FieldLookupEventActor, 2u);
}

RecompReturn Lufia2DecompBridge_80EA09(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ea09u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80ea09u, Lufia2FieldResolveEventPosition, 2u);
}

RecompReturn Lufia2DecompBridge_80D9F0(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->x_flag || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80d9f0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80d9f0u, Lufia2FieldReadEventVariableOperands, 2u);
}

RecompReturn Lufia2DecompBridge_80EBAA(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ebaau);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80ebaau, Lufia2FieldLoadSectionRecords, 3u);
}

RecompReturn Lufia2DecompBridge_80EC18(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ec18u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80ec18u, Lufia2FieldBuildPackedAttributes, 3u);
}

RecompReturn Lufia2DecompBridge_80EC78(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ec78u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80ec78u, Lufia2FieldPublishSectionSize, 3u);
}

RecompReturn Lufia2DecompBridge_80ECF2(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ecf2u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80ecf2u, Lufia2FieldAdvanceMapDestination, 2u);
}

RecompReturn Lufia2DecompBridge_80ECFE(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ecfeu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80ecfeu, Lufia2FieldResolveMapOffset, 2u);
}

RecompReturn Lufia2DecompBridge_80BF92(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80bf92u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80bf92u, Lufia2FieldSelectActorById, 3u);
}

RecompReturn Lufia2DecompBridge_85EDDB(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85eddbu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85eddbu, Lufia2BattleClearPartyRecordBytes, 3u);
}

RecompReturn Lufia2DecompBridge_81FAC9(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81fac9u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81fac9u, Lufia2BattleRunRelativeScript, 3u);
}

RecompReturn Lufia2DecompBridge_8EBB2E(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 12u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebb2eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x8ebb2eu, Lufia2FieldCycleSelectedSprite, 3u);
}

RecompReturn Lufia2DecompBridge_8EBBA8(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebba8u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8ebba8u, Lufia2FieldTakeSpriteSelectionButtons, 2u);
}

RecompReturn Lufia2DecompBridge_82CE52(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ce52u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82ce52u, Lufia2CapsuleBuildLevelExperience, 2u);
}

RecompReturn Lufia2DecompBridge_82CEAB(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ceabu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82ceabu, Lufia2CapsuleAdvanceExperienceStep, 2u);
}

RecompReturn Lufia2DecompBridge_80E8E2(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80e8e2u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80e8e2u, Lufia2FieldPublishEventPointer, 2u);
}

RecompReturn Lufia2DecompBridge_8691FE(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8691feu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8691feu, Lufia2WorldMapCellCenter, 2u);
}

RecompReturn Lufia2DecompBridge_86CDF5(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86cdf5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86cdf5u, Lufia2WorldMapBindResourcePointers, 2u);
}

RecompReturn Lufia2DecompBridge_82D270(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82d270u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82d270u, Lufia2CapsuleRebuildStatBlock, 2u);
}

RecompReturn Lufia2DecompBridge_82F6A4(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f6a4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f6a4u, Lufia2PartyClearSecondaryModifiers, 2u);
}

RecompReturn Lufia2DecompBridge_82F6D4(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f6d4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f6d4u, Lufia2PartyClearPrimaryModifiers, 2u);
}

RecompReturn Lufia2DecompBridge_83BA06(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83ba06u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83ba06u, Lufia2FieldProbeTalkTarget, 2u);
}

RecompReturn Lufia2DecompBridge_83BA5C(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83ba5cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83ba5cu, Lufia2FieldProbeTalkDown, 2u);
}

RecompReturn Lufia2DecompBridge_83BA80(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83ba80u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83ba80u, Lufia2FieldProbeTalkLeft, 2u);
}

RecompReturn Lufia2DecompBridge_83BA96(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83ba96u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83ba96u, Lufia2FieldProbeTalkUp, 2u);
}

RecompReturn Lufia2DecompBridge_83BAAC(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83baacu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83baacu, Lufia2FieldProbeTalkRight, 2u);
}

RecompReturn Lufia2DecompBridge_83BA76(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83ba76u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83ba76u, Lufia2FieldProbeTalkBlocked, 2u);
}

RecompReturn Lufia2DecompBridge_83B8BF(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b8bfu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83b8bfu, Lufia2FieldProbeActorContact, 2u);
}

RecompReturn Lufia2DecompBridge_83D927(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu || cpu->X > 6u || (cpu->X & 1u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d927u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83d927u, Lufia2FieldProbeContactEdge, 2u);
}

RecompReturn Lufia2DecompBridge_83D932(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d932u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83d932u, Lufia2FieldProbeContactEdgeDown, 2u);
}

RecompReturn Lufia2DecompBridge_83D93E(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d93eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x83d93eu, Lufia2FieldProbeContactEdgeLeft, 2u);
}

RecompReturn Lufia2DecompBridge_83D948(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d948u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83d948u, Lufia2FieldProbeContactEdgeUp, 2u);
}

RecompReturn Lufia2DecompBridge_83D952(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d952u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83d952u, Lufia2FieldProbeContactEdgeRight, 2u);
}

RecompReturn Lufia2DecompBridge_82D283(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82d283u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82d283u, Lufia2CapsuleBuildStatValues, 2u);
}
RecompReturn Lufia2DecompBridge_82D31C(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->m_flag || cpu->x_flag || cpu->D || cpu->S < 0x200u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82d31cu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82d31cu, Lufia2CapsuleAccumulateStatGrowth, 2u);
}

RecompReturn Lufia2DecompBridge_86D3A5(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d3a5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86d3a5u, Lufia2WorldMapBlankDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_86D2D6(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d2d6u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86d2d6u, Lufia2WorldMapResetSceneState, 2u);
}

RecompReturn Lufia2DecompBridge_86AE1E(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ae1eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86ae1eu, Lufia2WorldMapConfigureMode7, 2u);
}

RecompReturn Lufia2DecompBridge_86A7CE(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a7ceu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86a7ceu, Lufia2WorldMapUpdatePlane, 2u);
}

RecompReturn Lufia2DecompBridge_86CD41(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86cd41u);
    }
    return ActorBridgeSceneOwners(cpu, 0x86cd41u, Lufia2WorldMapInstallGraphics, 2u);
}

RecompReturn Lufia2DecompBridge_8692A1(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8692a1u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8692a1u, Lufia2WorldMapResetScene, 2u);
}

RecompReturn Lufia2DecompBridge_86AD82(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f00u || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ad82u);
    }
    return ActorBridgeSceneOwners(cpu, 0x86ad82u, Lufia2WorldMapUploadTilePlane, 2u);
}

RecompReturn Lufia2DecompBridge_8196FE(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8196feu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8196feu, Lufia2BattleEffectAddBg3Scroll, 2u);
}

RecompReturn Lufia2DecompBridge_819738(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819738u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819738u, Lufia2BattleEffectSetBg3Scroll, 2u);
}

RecompReturn Lufia2DecompBridge_81976B(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81976bu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81976bu, Lufia2BattleEffectAddBg1Scroll, 2u);
}

RecompReturn Lufia2DecompBridge_8197A5(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8197a5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8197a5u, Lufia2BattleEffectSetBg1Scroll, 2u);
}

RecompReturn Lufia2DecompBridge_86ADEE(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86adeeu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86adeeu, Lufia2WorldMapBuildCellOffset, 2u);
}

RecompReturn Lufia2DecompBridge_86AE05(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ae05u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86ae05u, Lufia2WorldMapBuildBlockPointer, 2u);
}

RecompReturn Lufia2DecompBridge_86AC6C(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ac6cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86ac6cu, Lufia2WorldMapStreamRow, 2u);
}

RecompReturn Lufia2DecompBridge_86ACFE(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86acfeu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86acfeu, Lufia2WorldMapStreamColumn, 2u);
}

RecompReturn Lufia2DecompBridge_80F6C6(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80f6c6u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80f6c6u, Lufia2FieldRenderRowBuffers, 2u);
}

RecompReturn Lufia2DecompBridge_86CD67(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86cd67u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86cd67u, Lufia2WorldMapUploadInitialTilemap, 2u);
}

RecompReturn Lufia2DecompBridge_86E356(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e356u);
    }
    return ActorBridgeSceneOwners(cpu, 0x86e356u, Lufia2WorldMapProjectDistance, 2u);
}

RecompReturn Lufia2DecompBridge_86A7F8(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a7f8u);
    }
    return ActorBridgeSceneOwners(cpu, 0x86a7f8u, Lufia2WorldMapBuildColorHdma, 2u);
}

RecompReturn Lufia2DecompBridge_86A4FA(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a4fau);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86a4fau, Lufia2WorldMapResolveAngleScale, 2u);
}

RecompReturn Lufia2DecompBridge_86A52F(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a52fu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86a52fu, Lufia2WorldMapMultiply16, 2u);
}

RecompReturn Lufia2DecompBridge_86CD91(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86cd91u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86cd91u, Lufia2WorldMapLoadResourceColors, 2u);
}

RecompReturn Lufia2DecompBridge_86CBF0(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86cbf0u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86cbf0u, Lufia2WorldMapBuildSkylineHdma, 2u);
}

RecompReturn Lufia2DecompBridge_86A913(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a913u);
    }
    return ActorBridgeSceneOwners(cpu, 0x86a913u, Lufia2WorldMapProjectTiltDistance, 2u);
}

RecompReturn Lufia2DecompBridge_86A956(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a956u);
    }
    return ActorBridgeSceneOwners(cpu, 0x86a956u, Lufia2WorldMapProjectTiltReciprocal, 2u);
}

RecompReturn Lufia2DecompBridge_80BE30(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80be30u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x80be30u, Lufia2SystemResolveEventFlagBit, 2u);
}

RecompReturn Lufia2DecompBridge_80BE1E(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->S < 4u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80be1eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80be1eu, Lufia2SystemTestEventFlag, 2u);
}

RecompReturn Lufia2DecompBridge_80BE1A(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->S < 6u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80be1au);
    }
    return ActorBridgeSceneOwners(cpu, 0x80be1au, Lufia2SystemTestEventFlagLong, 3u);
}

RecompReturn Lufia2DecompBridge_86AE4D(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ae4du);
    }
    return ActorBridgeSceneOwners(cpu, 0x86ae4du, Lufia2WorldMapCopyFlaggedBlocks, 2u);
}

RecompReturn Lufia2DecompBridge_86A0A2(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a0a2u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86a0a2u, Lufia2WorldMapQueuePaletteTransfer, 2u);
}

RecompReturn Lufia2DecompBridge_86A03B(CpuState *cpu) {
    if (cpu->PB != 0x86u || !cpu->m_flag || cpu->x_flag || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a03bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86a03bu, Lufia2WorldMapStepPaletteColors, 2u);
}

RecompReturn Lufia2DecompBridge_86CCFC(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D || !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ccfcu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86ccfcu, Lufia2WorldMapLoadResourceBlocks, 2u);
}

RecompReturn Lufia2DecompBridge_81E8EE(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81e8eeu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81e8eeu, Lufia2BattleMergeGlyphTile, 2u);
}

RecompReturn Lufia2DecompBridge_81E8E1(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81e8e1u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81e8e1u, Lufia2BattleEmitGlyphTile, 2u);
}

RecompReturn Lufia2DecompBridge_81E872(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81e872u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81e872u, Lufia2BattleBuildPartyNameTiles, 2u);
}

RecompReturn Lufia2DecompBridge_81E877(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81e877u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81e877u, Lufia2BattleBuildAlternateNameTiles, 2u);
}

RecompReturn Lufia2DecompBridge_819BBE(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819bbeu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819bbeu, Lufia2BattleEffectMoveTarget, 2u);
}

RecompReturn Lufia2DecompBridge_81958F(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->DB != 0x81u || cpu->D ||
        !cpu->m_flag || cpu->x_flag || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81958fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x81958fu, Lufia2BattleEffectChangeDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_8192E9(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->D || !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8192e9u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8192e9u, Lufia2BattleEffectSpawnSavedActorScript, 2u);
}

RecompReturn Lufia2DecompBridge_81B7D9(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->D || !cpu->m_flag || cpu->x_flag ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81b7d9u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81b7d9u, Lufia2BattleEffectTargetAdjustment, 2u);
}

RecompReturn Lufia2DecompBridge_85DB6D(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85db6du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85db6du, Lufia2SystemDivideVectorMagnitude, 3u);
}

RecompReturn Lufia2DecompBridge_85DAC7(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85dac7u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85dac7u, Lufia2SystemCalculateVectorAngle, 3u);
}

RecompReturn Lufia2DecompBridge_819393(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819393u);
    }
    return ActorBridgeSceneOwners(cpu, 0x819393u, Lufia2BattleEffectSpawnCurrentTargetScript, 2u);
}

RecompReturn Lufia2DecompBridge_85A701(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85a701u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85a701u, Lufia2BattleInitializeRipple, 3u);
}

RecompReturn Lufia2DecompBridge_85DC6F(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85dc6fu);
    }
    return ActorBridgeWholeM1X16(cpu, 0x85dc6fu, Lufia2SystemDivide24ByByte, 3u);
}

RecompReturn Lufia2DecompBridge_81940E(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81940Eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x81940Eu, Lufia2BattleEffectSpawnEnemyScripts, 2u);
}

RecompReturn Lufia2DecompBridge_82C4E4(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c4e4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82c4e4u, Lufia2CapsuleGetFlagMask, 2u);
}

RecompReturn Lufia2DecompBridge_82C4F4(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c4f4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82c4f4u, Lufia2CapsuleGetFormAddress, 2u);
}

RecompReturn Lufia2DecompBridge_82C504(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c504u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82c504u, Lufia2CapsuleGetItemAddress, 2u);
}

RecompReturn Lufia2DecompBridge_82C554(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82C554u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82C554u, Lufia2CapsuleChooseMenuItem, 2u);
}

RecompReturn Lufia2DecompBridge_82C482(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c482u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82c482u, Lufia2CapsuleGetStatusAddress, 2u);
}

RecompReturn Lufia2DecompBridge_8283EB(CpuState *cpu) {
    const unsigned columns = cpu->X >> 8;
    const unsigned rows = cpu->X & 0xffu;
    if (cpu->PB != 0x82u || cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu ||
        (cpu->A & 1u) || cpu->A >= 0x0800u || !columns || !rows ||
        ((cpu->A & 0x3fu) >> 1) + columns > 32u || (cpu->A >> 6) + rows > 32u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8283ebu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8283ebu, Lufia2MenuClearTileRectangle, 2u);
}

RecompReturn Lufia2DecompBridge_82CC3C(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82cc3cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82cc3cu, Lufia2CapsuleCheckMenuForm, 2u);
}

RecompReturn Lufia2DecompBridge_82C577(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c577u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c577u, Lufia2CapsuleEnsureMenuItem, 2u);
}

RecompReturn Lufia2DecompBridge_82C5F3(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c5f3u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c5f3u, Lufia2CapsuleUpdateItemCursor, 2u);
}

RecompReturn Lufia2DecompBridge_82C5AF(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c5afu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c5afu, Lufia2CapsuleRefreshMenuItem, 2u);
}

RecompReturn Lufia2DecompBridge_81A062(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a062u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a062u, Lufia2BattleEffectAimAtPoint, 2u);
}

RecompReturn Lufia2DecompBridge_81A094(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a094u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a094u, Lufia2BattleEffectAccelerateAtAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A0C4(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a0c4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a0c4u, Lufia2BattleEffectDecelerateAtAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A0F4(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a0f4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a0f4u, Lufia2BattleEffectDecelerateAtOffsetAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A12A(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a12au);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a12au, Lufia2BattleEffectSetPackedDrawState, 2u);
}

RecompReturn Lufia2DecompBridge_81A162(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a162u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a162u, Lufia2BattleEffectSetAlternatePackedDrawState, 2u);
}

RecompReturn Lufia2DecompBridge_81A19C(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a19cu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a19cu, Lufia2BattleEffectSetDrawState, 2u);
}

RecompReturn Lufia2DecompBridge_81A39D(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a39du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a39du, Lufia2BattleEffectCopyDrawValue, 2u);
}

RecompReturn Lufia2DecompBridge_81A3BF(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a3bfu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a3bfu, Lufia2BattleEffectSetDrawVariant, 2u);
}

RecompReturn Lufia2DecompBridge_81A3E9(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a3e9u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a3e9u, Lufia2BattleEffectSetParameterWord, 2u);
}

RecompReturn Lufia2DecompBridge_81A431(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a431u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a431u, Lufia2BattleEffectRandomizeParameterWord, 2u);
}

RecompReturn Lufia2DecompBridge_81A69A(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a69au);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a69au, Lufia2BattleEffectAccelerateByByte, 2u);
}

RecompReturn Lufia2DecompBridge_81A6C6(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a6c6u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a6c6u, Lufia2BattleEffectDecelerateByByte, 2u);
}

RecompReturn Lufia2DecompBridge_81A6F2(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a6f2u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a6f2u, Lufia2BattleEffectAccelerateOne, 2u);
}

RecompReturn Lufia2DecompBridge_81A71C(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a71cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a71cu, Lufia2BattleEffectAccelerateTwo, 2u);
}

RecompReturn Lufia2DecompBridge_81A746(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a746u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a746u, Lufia2BattleEffectAccelerateThree, 2u);
}

RecompReturn Lufia2DecompBridge_81A770(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a770u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a770u, Lufia2BattleEffectAccelerateFour, 2u);
}

RecompReturn Lufia2DecompBridge_81968A(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81968au);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81968au, Lufia2BattleEffectRestorePaletteRange, 2u);
}

RecompReturn Lufia2DecompBridge_81A264(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a264u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a264u, Lufia2BattleEffectProjectFromPosition, 2u);
}

RecompReturn Lufia2DecompBridge_81A31D(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a31du);
    }
    return ActorBridgeSceneOwners(cpu, 0x81a31du, Lufia2BattleEffectVectorFromParameters, 2u);
}

RecompReturn Lufia2DecompBridge_81A605(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a605u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a605u, Lufia2BattleEffectSetDelayOne, 2u);
}

RecompReturn Lufia2DecompBridge_81A60F(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a60fu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a60fu, Lufia2BattleEffectSetDelayTwo, 2u);
}

RecompReturn Lufia2DecompBridge_81A619(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a619u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a619u, Lufia2BattleEffectSetDelayThree, 2u);
}

RecompReturn Lufia2DecompBridge_81A623(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a623u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a623u, Lufia2BattleEffectSetDelayFour, 2u);
}

RecompReturn Lufia2DecompBridge_81A62D(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a62du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a62du, Lufia2BattleEffectSetDelayFive, 2u);
}

RecompReturn Lufia2DecompBridge_81A637(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a637u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a637u, Lufia2BattleEffectSetDelaySix, 2u);
}

RecompReturn Lufia2DecompBridge_81A641(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a641u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a641u, Lufia2BattleEffectSetDelayEight, 2u);
}

RecompReturn Lufia2DecompBridge_81A64B(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a64bu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a64bu, Lufia2BattleEffectSetDelayTen, 2u);
}

RecompReturn Lufia2DecompBridge_81A655(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a655u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a655u, Lufia2BattleEffectSetDelayTwenty, 2u);
}

RecompReturn Lufia2DecompBridge_81A03A(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a03au);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a03au, Lufia2BattleEffectOffsetBaseAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A04E(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a04eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a04eu, Lufia2BattleEffectRotateAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A295(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a295u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a295u, Lufia2BattleEffectSetVelocityWords, 2u);
}

RecompReturn Lufia2DecompBridge_81A35E(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a35eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a35eu, Lufia2BattleEffectSetDrawAttributes, 2u);
}

RecompReturn Lufia2DecompBridge_81A373(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a373u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a373u, Lufia2BattleEffectSetDrawValue, 2u);
}

RecompReturn Lufia2DecompBridge_81A388(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a388u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a388u, Lufia2BattleEffectSetDrawMode, 2u);
}

RecompReturn Lufia2DecompBridge_81A3D4(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a3d4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a3d4u, Lufia2BattleEffectSetDrawFlag, 2u);
}

RecompReturn Lufia2DecompBridge_819AF7(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819af7u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819af7u, Lufia2BattleEffectSetTargetMotionOffsets, 2u);
}

RecompReturn Lufia2DecompBridge_819D1F(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819d1fu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819d1fu, Lufia2BattleEffectMoveSpecialTarget, 2u);
}

RecompReturn Lufia2DecompBridge_819C76(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819c76u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819c76u, Lufia2BattleEffectSetTargetStatus, 2u);
}

RecompReturn Lufia2DecompBridge_8196C6(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8196c6u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8196c6u, Lufia2BattleEffectLoadPaletteRange, 2u);
}

RecompReturn Lufia2DecompBridge_85B1D6(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b1d6u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85b1d6u, Lufia2BattleEnableCircleWindow, 3u);
}

RecompReturn Lufia2DecompBridge_819A54(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819a54u);
    }
    return ActorBridgeSceneOwners(cpu, 0x819a54u, Lufia2BattleEffectSetCircleRadius, 2u);
}

RecompReturn Lufia2DecompBridge_85AB98(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ab98u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85ab98u, Lufia2BattleConfigureLayerColorDma, 3u);
}

RecompReturn Lufia2DecompBridge_81A455(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a455u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a455u, Lufia2BattleEffectSpawnStationaryActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A46E(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a46eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a46eu, Lufia2BattleEffectSpawnHalfTurnActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A489(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a489u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a489u, Lufia2BattleEffectSpawnActorWithAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A4A6(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a4a6u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a4a6u, Lufia2BattleEffectSpawnOffsetActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A4C3(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a4c3u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a4c3u, Lufia2BattleEffectSpawnUnshiftedMovingActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A4DC(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a4dcu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a4dcu, Lufia2BattleEffectSpawnHalfTurnMovingActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A4F7(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a4f7u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a4f7u, Lufia2BattleEffectSpawnRightwardActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A512(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a512u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a512u, Lufia2BattleEffectSpawnUpwardActor, 2u);
}

RecompReturn Lufia2DecompBridge_81A52D(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a52du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a52du, Lufia2BattleEffectSpawnMovingActorWithAngle, 2u);
}

RecompReturn Lufia2DecompBridge_81A54A(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a54au);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a54au, Lufia2BattleEffectSpawnActorWithVerticalSpeed, 2u);
}

RecompReturn Lufia2DecompBridge_81A1CC(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a1ccu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a1ccu, Lufia2BattleEffectSpawnMovingActorFromStream, 2u);
}

RecompReturn Lufia2DecompBridge_81A65F(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a65fu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a65fu, Lufia2BattleEffectSpawnZeroPositionScript, 2u);
}

RecompReturn Lufia2DecompBridge_81A676(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a676u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a676u, Lufia2BattleEffectSpawnCenteredScript, 2u);
}

RecompReturn Lufia2DecompBridge_81A1EC(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a1ecu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a1ecu, Lufia2BattleEffectClearDrawDirty, 2u);
}

RecompReturn Lufia2DecompBridge_81A1F5(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a1f5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a1f5u, Lufia2BattleEffectClearTarget, 2u);
}

RecompReturn Lufia2DecompBridge_81A567(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a567u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a567u, Lufia2BattleEffectSetDrawVariantThree, 2u);
}

RecompReturn Lufia2DecompBridge_81A588(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a588u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a588u, Lufia2BattleEffectUseThirdDrawParameter, 2u);
}

RecompReturn Lufia2DecompBridge_81A696(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a696u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a696u, Lufia2BattleEffectClearBackgroundUploadRequest, 2u);
}

RecompReturn Lufia2DecompBridge_81A5BD(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a5bdu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a5bdu, Lufia2BattleEffectLoadFirstPalettePreset, 2u);
}

RecompReturn Lufia2DecompBridge_81A5D5(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a5d5u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a5d5u, Lufia2BattleEffectLoadSecondPalettePreset, 2u);
}

RecompReturn Lufia2DecompBridge_81A5ED(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81a5edu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81a5edu, Lufia2BattleEffectLoadThirdPalettePreset, 2u);
}

RecompReturn Lufia2DecompBridge_8197D8(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8197d8u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8197d8u, Lufia2BattleEffectCopyTileRectangle, 2u);
}

RecompReturn Lufia2DecompBridge_819976(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819976u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819976u, Lufia2BattleEffectSpawnScriptWhenEnabled, 2u);
}

RecompReturn Lufia2DecompBridge_819853(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f06u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819853u);
    }
    return ActorBridgeSceneOwners(cpu, 0x819853u, Lufia2BattleEffectSetPaletteBrightness, 2u);
}

RecompReturn Lufia2DecompBridge_819935(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f06u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819935u);
    }
    return ActorBridgeSceneOwners(cpu, 0x819935u, Lufia2BattleEffectAdjustPaletteBrightness, 2u);
}

RecompReturn Lufia2DecompBridge_85920E(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85920eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85920eu, Lufia2BattleTogglePartyTargetState, 3u);
}

RecompReturn Lufia2DecompBridge_85922D(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85922du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85922du, Lufia2BattleToggleSpecialTargetState, 3u);
}

RecompReturn Lufia2DecompBridge_819BAC(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f06u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819bacu);
    }
    return ActorBridgeSceneOwners(cpu, 0x819bacu, Lufia2BattleEffectTogglePartyTargetState, 2u);
}

RecompReturn Lufia2DecompBridge_819D0B(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f06u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819d0bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x819d0bu, Lufia2BattleEffectToggleSpecialTargetState, 2u);
}

RecompReturn Lufia2DecompBridge_819664(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819664u);
    }
    return ActorBridgeSceneOwners(cpu,0x819664u,Lufia2BattleEffectLoadGraphicsResource,2u);
}

RecompReturn Lufia2DecompBridge_8196F6(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8196f6u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8196f6u, Lufia2BattleEffectClearWindowTiles, 2u);
}

RecompReturn Lufia2DecompBridge_8196FA(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8196fau);
    }
    return ActorBridgeSceneOwners(cpu, 0x8196fau, Lufia2BattleEffectResetPartyTiles, 2u);
}

RecompReturn Lufia2DecompBridge_819B56(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819b56u);
    }
    return ActorBridgeSceneOwners(cpu,0x819b56u,Lufia2BattleEffectSetPortraitPose,2u);
}

RecompReturn Lufia2DecompBridge_819DC9(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819dc9u);
    }
    return ActorBridgeSceneOwners(cpu,0x819dc9u,Lufia2BattleEffectRestorePortraitPose,2u);
}

RecompReturn Lufia2DecompBridge_819CCC(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819cccu);
    }
    return ActorBridgeSceneOwners(cpu,0x819cccu,Lufia2BattleEffectSetPortraitPoseIfStatusClear,2u);
}

RecompReturn Lufia2DecompBridge_819968(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819968u);
    }
    return ActorBridgeSceneOwners(cpu,0x819968u,Lufia2BattleEffectRefreshTurnDisplay,2u);
}

RecompReturn Lufia2DecompBridge_819DF6(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819df6u);
    }
    return ActorBridgeSceneOwners(cpu,0x819df6u,Lufia2BattleEffectRebuildSprites,2u);
}

RecompReturn Lufia2DecompBridge_819E13(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819e13u);
    }
    return ActorBridgeSceneOwners(cpu,0x819e13u,Lufia2BattleEffectSendSoundCommand,2u);
}

RecompReturn Lufia2DecompBridge_81A1FF(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81a1ffu);
    }
    return ActorBridgeSceneOwners(cpu,0x81a1ffu,Lufia2BattleEffectRandomizeCenteredParameter,2u);
}

RecompReturn Lufia2DecompBridge_81A2B0(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81a2b0u);
    }
    return ActorBridgeSceneOwners(cpu,0x81a2b0u,Lufia2BattleEffectSpawnVectorActor,2u);
}

RecompReturn Lufia2DecompBridge_819F8B(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f02u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819f8bu);
    }
    return ActorBridgeSceneOwners(cpu,0x819f8bu,Lufia2BattleEffectSendConditionalSound,2u);
}

RecompReturn Lufia2DecompBridge_819F39(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f04u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x819f39u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x819f39u, Lufia2BattleEffectSpawnPopupActor, 2u);
}

RecompReturn Lufia2DecompBridge_819E22(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f0au || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819e22u);
    }
    return ActorBridgeSceneOwners(cpu,0x819e22u,Lufia2BattleEffectSpawnTargetPopups,2u);
}

RecompReturn Lufia2DecompBridge_819B7E(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x819b7eu);
    }
    return ActorBridgeSceneOwners(cpu,0x819b7eu,Lufia2BattleEffectPrepareFrame,2u);
}

RecompReturn Lufia2DecompBridge_81986E(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81986eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81986eu, Lufia2BattleEffectCopyActorTiles, 2u);
}

RecompReturn Lufia2DecompBridge_81C321(CpuState *cpu) {
    if(cpu->PB!=0x81u || !cpu->m_flag || cpu->x_flag || cpu->D!=0u || cpu->S<0x1f0au || cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81c321u);
    }
    return ActorBridgeSceneOwners(cpu,0x81c321u,Lufia2BattleFadeOutWindows,2u);
}

RecompReturn Lufia2DecompBridge_81C339(CpuState *cpu) {
    if(cpu->PB!=0x81u || !cpu->m_flag || cpu->x_flag || cpu->D!=0u || cpu->S<0x1f0au || cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81c339u);
    }
    return ActorBridgeSceneOwners(cpu,0x81c339u,Lufia2BattleFadeInWindows,2u);
}

RecompReturn Lufia2DecompBridge_81BBE0(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f06u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81bbe0u);
    }
    return ActorBridgeSceneOwners(cpu,0x81bbe0u,Lufia2BattleUpdateSlotPortraitStatus,2u);
}

RecompReturn Lufia2DecompBridge_81BC55(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f0au || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81bc55u);
    }
    return ActorBridgeSceneOwners(cpu,0x81bc55u,Lufia2BattleLoadCapsuleGraphics,2u);
}
RecompReturn Lufia2DecompBridge_85DE9D(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f0au||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85de9du);
    }
    return ActorBridgeSceneOwners(cpu,0x85de9du,Lufia2BattleInitializeEnemySpriteRecords,3u);
}

RecompReturn Lufia2DecompBridge_85DEDC(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f06u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85dedcu);
    }
    return ActorBridgeSceneOwners(cpu,0x85dedcu,Lufia2BattleInitializeEnemySpriteRecord,2u);
}

RecompReturn Lufia2DecompBridge_85DF35(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f00u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85df35u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85df35u,Lufia2BattleFindEnemySpriteGroup,3u);
}
RecompReturn Lufia2DecompBridge_85EB91(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85eb91u);
    }
    return ActorBridgeSceneOwners(cpu,0x85eb91u,Lufia2BattleLoadEnemyGraphicsGroups,3u);
}

RecompReturn Lufia2DecompBridge_85EBE0(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f08u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85ebe0u);
    }
    return ActorBridgeSceneOwners(cpu,0x85ebe0u,Lufia2BattleAllocateEnemyTiles,2u);
}

RecompReturn Lufia2DecompBridge_85EC4D(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f06u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85ec4du);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85ec4du,Lufia2BattleAllocateEnemyPalette,2u);
}

RecompReturn Lufia2DecompBridge_85E7BC(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f12u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85e7bcu);
    }
    return ActorBridgeSceneOwners(cpu,0x85e7bcu,Lufia2BattlePlayTransition,3u);
}

RecompReturn Lufia2DecompBridge_85EAE4(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85eae4u);
    }
    return ActorBridgeSceneOwners(cpu,0x85eae4u,Lufia2BattleAdvanceTransitionFrame,2u);
}

RecompReturn Lufia2DecompBridge_85ABE4(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f00u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85abe4u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85abe4u,Lufia2BattlePrepareTransitionMask,3u);
}

RecompReturn Lufia2DecompBridge_85EDBB(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85edbbu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85edbbu, Lufia2BattleRetainPersistentStatuses, 3u);
}

RecompReturn Lufia2DecompBridge_85EDF1(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85edf1u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85edf1u, Lufia2BattleLoadSavedSubmenuChoices, 3u);
}

RecompReturn Lufia2DecompBridge_85EE3E(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f00u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ee3eu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85ee3eu, Lufia2BattleStoreSavedSubmenuChoices, 3u);
}

RecompReturn Lufia2DecompBridge_85EEA1(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f04u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85eea1u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85eea1u, Lufia2BattleStoreRemainingEncounter, 3u);
}
RecompReturn Lufia2DecompBridge_81839F(CpuState *cpu) {
    if(cpu->PB!=0x81u||!cpu->m_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x81839fu);
    }
    return ActorBridgeSceneOwners(cpu,0x81839fu,Lufia2BattleGenerateEncounterRecord,2u);
}

RecompReturn Lufia2DecompBridge_8184FC(CpuState *cpu) {
    if(cpu->PB!=0x81u||!cpu->m_flag || !cpu->x_flag||cpu->D!=0u||cpu->S<0x1f0au||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x8184fcu);
    }
    return ActorBridgeSceneOwners(cpu,0x8184fcu,Lufia2BattleAddEncounterEnemy,2u);
}

RecompReturn Lufia2DecompBridge_8181E6(CpuState *cpu) {
    if(cpu->PB!=0x81u||!cpu->m_flag||cpu->x_flag||cpu->D||cpu->S<0x1f16u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x8181e6u);
    }
    return ActorBridgeSceneOwners(cpu,0x8181e6u,Lufia2BattleLoadEncounter,2u);
}
RecompReturn Lufia2DecompBridge_85DE8E(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f00u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85de8eu);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85de8eu,Lufia2BattleClearEnemySpriteSlots,3u);
}
RecompReturn Lufia2DecompBridge_85DA71(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f02u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85da71u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85da71u,Lufia2BattleClearPartyModifiers,3u);
}
RecompReturn Lufia2DecompBridge_85DA9C(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f02u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85da9cu);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85da9cu,Lufia2BattleClearEnemyModifiers,3u);
}
RecompReturn Lufia2DecompBridge_85EE9B(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f00u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85ee9bu);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85ee9bu,Lufia2BattleClearSelectedPartyStatus,2u);
}
RecompReturn Lufia2DecompBridge_85EDB2(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f05u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85edb2u);
    }
    return ActorBridgeSceneOwners(cpu,0x85edb2u,Lufia2BattleClearAllModifiers,3u);
}
RecompReturn Lufia2DecompBridge_85EE82(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f02u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85ee82u);
    }
    return ActorBridgeSceneOwners(cpu,0x85ee82u,Lufia2BattleClearPartyStatuses,3u);
}
RecompReturn Lufia2DecompBridge_85ED51(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f00u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85ed51u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85ed51u,Lufia2BattleSnapshotStatuses,3u);
}
RecompReturn Lufia2DecompBridge_85DFB9(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f02u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x85dfb9u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x85dfb9u,Lufia2BattleReleaseDefeatedEnemies,3u);
}
RecompReturn Lufia2DecompBridge_858905(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f08u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x858905u);
    }
    return ActorBridgeSceneOwners(cpu,0x858905u,Lufia2BattleInitializeEnemySpriteSizes,3u);
}
RecompReturn Lufia2DecompBridge_858A03(CpuState *cpu) {
    if(cpu->PB!=0x85u||!cpu->m_flag||cpu->x_flag||cpu->S<0x1f00u||cpu->S>0x1ffbu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x858a03u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x858a03u,Lufia2BattleInitializePartySpriteDescriptors,3u);
}

RecompReturn Lufia2DecompBridge_85A972(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f04u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85a972u);
    }
    return ActorBridgeSceneOwners(
        cpu, 0x85a972u, Lufia2BattleConfigureResultHdma, 3u);
}

RecompReturn Lufia2DecompBridge_8EE751(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->Y >= 6u ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ee751u);
    }
    return ActorBridgeWholeAnyWidth(
        cpu, 0x8ee751u, Lufia2MenuRestoreSelectionState, 3u);
}

RecompReturn Lufia2DecompBridge_8EB993(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb993u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8eb993u,
        Lufia2RestoreSavedFieldState, 3u);
}

RecompReturn Lufia2DecompBridge_8EBA0D(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eba0du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8eba0du,
        Lufia2CaptureFieldSaveState, 3u);
}

RecompReturn Lufia2DecompBridge_85DE6C(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85de6cu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85de6cu,
        Lufia2SaveRecordChecksum, 3u);
}


RecompReturn Lufia2DecompBridge_85C8CF(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c8cfu);
    }
    return ActorBridgeWhole(cpu, 0x85c8cfu, Lufia2SaveUnpackCapsuleRecord, 2u);
}

RecompReturn Lufia2DecompBridge_85CB7B(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85cb7bu);
    }
    return ActorBridgeWhole(cpu, 0x85cb7bu, Lufia2SavePackCapsuleRecord, 2u);
}

RecompReturn Lufia2DecompBridge_85C932(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c932u);
    }
    return ActorBridgeWhole(cpu, 0x85c932u, Lufia2SaveFlagPartyLevelMismatch, 2u);
}


RecompReturn Lufia2DecompBridge_85C754(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c754u);
    }
    return ActorBridgeWhole(cpu, 0x85c754u, Lufia2SaveUnpackPartyRecord, 2u);
}

RecompReturn Lufia2DecompBridge_85CBD2(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85cbd2u);
    }
    return ActorBridgeWhole(cpu, 0x85cbd2u, Lufia2SavePackStatBits, 2u);
}


RecompReturn Lufia2DecompBridge_85CC69(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85cc69u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85cc69u, Lufia2SaveDerivePartyStatBonuses, 2u);
}

RecompReturn Lufia2DecompBridge_85CBDC(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85cbdcu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85cbdcu, Lufia2SaveRestorePartyBaseStats, 2u);
}

RecompReturn Lufia2DecompBridge_85CA22(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ca22u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85ca22u, Lufia2SavePackPartyRecord, 2u);
}


RecompReturn Lufia2DecompBridge_85C954(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c954u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85c954u, Lufia2SavePackState, 3u);
}

RecompReturn Lufia2DecompBridge_85C60E(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c60eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85c60eu, Lufia2SaveRestoreState, 3u);
}

RecompReturn Lufia2DecompBridge_82F703(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f703u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f703u, Lufia2PartySelectEquipmentSlot, 2u);
}

RecompReturn Lufia2DecompBridge_82F710(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f710u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f710u, Lufia2PartyLoadEquipmentItem, 2u);
}

RecompReturn Lufia2DecompBridge_82F722(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f722u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f722u, Lufia2PartyAccumulateEquipmentModifiers, 2u);
}

RecompReturn Lufia2DecompBridge_82F846(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f846u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f846u, Lufia2PartyRebuildEquipmentModifiers, 2u);
}

RecompReturn Lufia2DecompBridge_82994E(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82994eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82994eu, Lufia2PartyRebuildAllEquipmentStats, 3u);
}


RecompReturn Lufia2DecompBridge_81EC56(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ec56u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ec56u, Lufia2InitializeDefaultRecords, 3u);
}


RecompReturn Lufia2DecompBridge_81ED35(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ed35u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ed35u, Lufia2PartyResetDefaultRecords, 3u);
}

RecompReturn Lufia2DecompBridge_81EC35(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ec35u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ec35u, Lufia2StartDefaultRecords, 3u);
}

RecompReturn Lufia2DecompBridge_829971(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829971u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x829971u, Lufia2PartySelectActiveEquipmentMember, 2u);
}

RecompReturn Lufia2DecompBridge_82E893(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e893u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e893u, Lufia2PartyRefreshActiveEquipmentStats, 3u);
}

RecompReturn Lufia2DecompBridge_82E8DE(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e8deu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e8deu, Lufia2TitlePrepareSaveSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82E917(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e917u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e917u, Lufia2TitlePrepareSelectionMode, 2u);
}

RecompReturn Lufia2DecompBridge_82E998(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e998u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e998u, Lufia2TitleLoadSaveSelectionBits, 2u);
}

RecompReturn Lufia2DecompBridge_80905F(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80905fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80905fu, Lufia2SaveTestFileHighFlags, 3u);
}

RecompReturn Lufia2DecompBridge_86916C(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x7eu || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86916cu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86916cu, Lufia2MenuLoadSelectionPalettes, 3u);
}

RecompReturn Lufia2DecompBridge_809073(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->DB != 0x7eu || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x809073u);
    }
    return ActorBridgeSceneOwners(cpu, 0x809073u, Lufia2SaveCheckGameFile, 3u);
}
