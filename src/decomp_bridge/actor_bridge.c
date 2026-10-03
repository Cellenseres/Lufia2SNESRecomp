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
