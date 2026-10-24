#include <stdint.h>

#include "cpu_state.h"
#include "snes/interp_bridge.h"
#include "lufia2/decomp.h"
#include "lufia2/world_scene_timed_motion.h"
#include "lufia2/world_scene_timed_windows.h"
#include "lufia2/special_menu.h"
#include "lufia2/card_game.h"
#include "lufia2/poker.h"
#include "lufia2/board_game.h"
#include "lufia2/world_scene_particle_commands.h"
#include "lufia2/menu_party_actions.h"
#include "lufia2/menu_inventory_groups.h"
#include "lufia2/menu_equipment_commands.h"
#include "lufia2/object_script.h"
#include "lufia2/event_script.h"
#include "decomp_bridge/host_events.h"
#include "../lufia2_sprite_visibility.h"
#if defined(LUFIA2_HAVE_NATIVE_FRAME_RETURNS)
#include "snes/dma.h"
#include "common_cpu_infra.h"
#include "decomp_bridge/frame_returns.h"
#include "decomp_bridge/frame_return_timing.h"
#if defined(LUFIA2_HAVE_NATIVE_SELECT_RESUMPTIONS)
#include "decomp_bridge/select_resume_timing.h"
#endif
#endif


#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTINUATIONS)
#include "snes/snes.h"
#include "decomp_bridge/nmi_resume.h"
#include "decomp_bridge/nmi_timing.h"
#include "decomp_bridge/nmi_resume.h"
extern Snes *g_snes;
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
#include "lufia2/nmi_finish.h"
#include "decomp_bridge/nmi_owner.h"
#endif
#endif

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
    if (pc == 0x82803du)
        Lufia2DecompMenuDispatch(cpu, pc);
    else if (pc == 0x81f4e2u)
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
    if (!ActorBridgeSupported(cpu, 1, 1))
        return ActorBridgeFallback(cpu, &frame, 0x83d350u);
    ActorBridgeLoad(cpu, &state);
    /* D370/FB17 tables hold only known targets. */
    const Lufia2ActorPrimaryActionFlow flow =
        Lufia2ActorPrimaryActionCore(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (flow == LUFIA2_ACTOR_PRIMARY_ACTION_X8_BOUNDARY_D38D)
        return interp_tier_dispatch_tail(
            cpu, state.resume_pc, state.resume_pc, frame.entry_s, frame.hrv);
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
        return interp_tier_dispatch_tail(cpu, state.resume_pc, 0x83fb17u, frame.entry_s,
                                         frame.hrv);
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



RecompReturn Lufia2DecompBridge_A9BA(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83a9bau, Lufia2ActorLoadSprite, 3);
}

RecompReturn Lufia2DecompBridge_8193(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x848193u, Lufia2SpriteGraphicsUpload, 3);
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

RecompReturn Lufia2DecompBridge_81F5BD(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f5bdu, Lufia2PartyRestoreVitals, 3u);
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

RecompReturn Lufia2DecompBridge_F723(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f723u, Lufia2PartyAddMenuMember, 3u);
}

RecompReturn Lufia2DecompBridge_F74F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f74fu, Lufia2PartyRemoveMenuMember, 3u);
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
    if (cpu->m_flag)
        return ActorBridgeWhole(cpu, 0x85ecdbu, Lufia2BattleVramQueueSlot, 3);
    if (cpu->PB != 0x85u || (cpu->DB != 0x81u && cpu->DB != 0x85u) || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ecdbu);
    }
    return ActorBridgeWholeM0X0(cpu, 0x85ecdbu, Lufia2BattleVramQueueSlot, 3);
}

RecompReturn Lufia2DecompBridge_99BF(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x8699bfu, Lufia2WorldMapStreamEdges, 2);
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


RecompReturn Lufia2DecompBridge_8A2F(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x858a2fu, Lufia2BattleSprites, 3);
}

RecompReturn Lufia2DecompBridge_ECF0(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x85ecf0u, Lufia2BattleFrameUpkeep, 3);
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
    const int byte_index =
        cpu->m_flag && cpu->x_flag && !cpu->emulation && !cpu->_flag_D &&
        cpu->PB == 0x83u && cpu->DB == 0x87u && !cpu->D &&
        (cpu->host_return_valid == 0u || cpu->host_return_valid == 3u) &&
        cpu->S >= 0x1f10u && cpu->S <= 0x1ffcu;
    return ActorBridgeRunVoid(cpu, 0x83a746u, Lufia2ActorSyncFinePosition, 3, 0x83a76cu,
                              byte_index);
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

static int ActorBridgeRecordBankSupported(uint8_t bank);

RecompReturn Lufia2DecompBridge_D416(CpuState *cpu) {
    if (!cpu->emulation && !cpu->_flag_D && cpu->m_flag && cpu->x_flag &&
        cpu->PB == 0x83u && !cpu->D &&
        (cpu->host_return_valid == 0u || cpu->host_return_valid == 3u) &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu &&
        ActorBridgeRecordBankSupported(cpu->DB))
        return ActorBridgeRunVoid(cpu, 0x83d416u, Lufia2ActorLoadPrimaryScript, 3u,
                                  0x83d436u, 1);

    return ActorBridgeRunVoid(cpu, 0x83d416u, Lufia2ActorLoadPrimaryScript, 3,
                              0x83d436u, 0);
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
    if (!cpu->m_flag && (cpu->PB != 0x81u || cpu->DB != 0x7eu ||
            cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81f194u);
    }
    return ActorBridgeWholeX16(cpu, 0x81f194u, Lufia2ItemRecordByte, 3);
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

static uint32_t ActorBridgePushedReturnPc(uint32_t site, uint8_t frame_size) {
    if (frame_size == 2u && (site & 0x7fffffu) == 0x058f3bu)
        return (site & 0xff0000u) | 0x8f3cu;
    if (site == 0x8192e4u)
        return 0x8192e7u;
    return (site & 0xff0000u) |
        (uint16_t)(site + (frame_size == 3u ? 4u : 3u));
}

/* The portable caller has already pushed the child's call frame. */
static uint8_t ActorBridgePushedChild(
    void *context, Lufia2CpuState *state, uint32_t target,
    uint32_t site, uint8_t frame_size) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    const uint16_t post_s = (uint16_t)(state->stack + frame_size);
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
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

RecompReturn Lufia2DecompBridge_C7F8(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83c7f8u);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2ActorPrimaryUpdateWithChildren(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_D508(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83d508u);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2ActorSecondaryUpdateWithChildren(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
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

RecompReturn Lufia2DecompBridge_8596BE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8596beu, 0, Lufia2BattleRestoreSelectedItems,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85E174(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85e174u, Lufia2BattleReorderEnemies, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85E412(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85e412u, 0, Lufia2BattleInsertEnemy,
                                  3u, BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85E4E5(CpuState *cpu) {
    if (cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85e4e5u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85e4e5u, 0, Lufia2BattlePrepareEnemyInsertion,
                                  3u, BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F2CE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f2ceu, 0, Lufia2SaveDrawSlotPreviews,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F2FE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f2feu, 0, Lufia2SaveDrawSlotPreview,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F383(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f383u, 0, Lufia2SaveInitializePortraitTables,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F395(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f395u, 0, Lufia2SavePlaceSlotPortraits,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F3DD(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f3ddu, 0, Lufia2SaveUnhighlightPortraits,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F3FC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f3fcu, 0, Lufia2SaveHighlightSlotPortraits,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F427(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f427u, 0, Lufia2SaveDrawPortraitLevels,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829330(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829330u, 0, Lufia2MenuWaitFramesOrButton,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829353(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829353u, 0, Lufia2MenuWaitSpriteFrames,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82935D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82935du, 0, Lufia2MenuWaitButtonMasks,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829380(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829380u, 0, Lufia2MenuWaitSpriteMotionOrButton,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EE5EF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8ee5efu, 0, Lufia2MenuRunScenarioViewer,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85E5C2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85e5c2u, 0, Lufia2BattleFlashActionActor,
                                  3u, BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81B336(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b336u, Lufia2BattleGrayPaletteToBuffer, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81B367(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81b367u, Lufia2BattleGraySourcePalette, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
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

#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTINUATIONS)
static uint8_t ActorBridgeNmiRead(void *context, uint32_t address) {
    return Lufia2DecompNmiReadByte((CpuState *)context, address);
}

static void ActorBridgeNmiWrite(void *context, uint32_t address, uint8_t value) {
    Lufia2DecompNmiWriteByte((CpuState *)context, address, value);
}

static bool ActorBridgeNmiResumeSupported(const CpuState *cpu, uint32_t pc) {
    if (!g_snes || !g_snes->inNmi || cpu->emulation || !cpu->ram)
        return false;
#if defined(LUFIA2_HAVE_NATIVE_NMI_ENTRY)
    if (pc == 0x00862du || pc == 0x80862du)
        return cpu->PB == pc >> 16 && cpu->m_flag <= 1u && cpu->x_flag <= 1u &&
               cpu->S >= 0x1f08u && cpu->S <= 0x1ffbu;
#endif
    bool byte_mode;
    bool restore = false;
    switch (pc) {
    case 0x808638u:
        byte_mode = false;
        break;
    case 0x80864cu:
    case 0x808653u:
    case 0x80865au:
#if defined(LUFIA2_HAVE_NATIVE_NMI_FINISH)
    case 0x808664u:
#endif
        byte_mode = true;
        break;
#if defined(LUFIA2_HAVE_NATIVE_NMI_FINISH)
    case 0x80869bu:
        byte_mode = true;
        restore = true;
        break;
#endif
    default:
        return false;
    }
    return cpu->PB == 0x80u && cpu->m_flag == (byte_mode && !restore) &&
           cpu->x_flag == byte_mode && !cpu->_flag_D && !cpu->D && cpu->S >= 0x1f00u &&
           cpu->S <= 0x1ff3u && (!byte_mode || cpu->DB == 0x80u);
}

static Lufia2NmiSchedule ActorBridgeNmiSchedule(const CpuState *cpu, uint32_t pc) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_ENTRY)
    if (pc == 0x00862du || pc == 0x80862du)
        return Lufia2NmiEntrySchedule();
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_FINISH)
    if (pc == 0x808664u)
        return Lufia2NmiClockSchedule(cpu->ram);
    if (pc == 0x80869bu)
        return Lufia2NmiRestoreSchedule(cpu->ram);
#endif
    return pc == 0x808638u ? Lufia2NmiPrepareSchedule(cpu->ram)
                           : Lufia2NmiCallbacksSchedule(cpu->ram, pc);
}

static Lufia2ExecutionResult ActorBridgeNmiSlice(const Lufia2Memory *memory,
                                                 Lufia2CpuState *cpu, uint32_t pc) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_ENTRY)
    if (pc == 0x00862du || pc == 0x80862du)
        return Lufia2NmiSaveInterruptFrame(memory, cpu);
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_FINISH)
    if (pc == 0x808664u)
        return Lufia2NmiDisplayAndClock(memory, cpu);
    if (pc == 0x80869bu)
        return Lufia2NmiRestoreInterruptFrame(memory, cpu);
#endif
    return Lufia2MainNmiResume(memory, cpu, pc, NULL, NULL);
}

static void ActorBridgeNmiResume(CpuState *cpu, uint32_t pc) {
    if (!ActorBridgeNmiResumeSupported(cpu, pc))
        return;
    const Lufia2NmiSchedule schedule = ActorBridgeNmiSchedule(cpu, pc);
    const unsigned fetch_speed = cpu_region_speed(pc);
    const Lufia2Memory memory = {ActorBridgeNmiRead, ActorBridgeNmiWrite, cpu,
                                 ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = ActorBridgeNmiSlice(&memory, &state, pc);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == pc)
        return;
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    for (unsigned i = 0; i < schedule.count; ++i) {
        const Lufia2NmiCost cost = schedule.steps[i];
        Lufia2DecompNmiAccountCycles(cpu, Lufia2NmiCpuCycles(cost),
                                     Lufia2NmiMasterClocks(cost, fetch_speed));
    }
    interp_bridge_pre_opcode_redirect(result.pc);
}

#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
static void ActorBridgeNmiPrepareFrameRestore(CpuState *cpu, uint32_t pc) {
    if (pc != 0x808699u || !g_snes || !g_snes->inNmi || !cpu->ram || cpu->emulation ||
        cpu->PB != 0x80u || cpu->master_cycles > UINT64_MAX - 64u ||
        !Lufia2DecompNmiHasInterruptOwner())
        return;
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2NmiPrepareFrameRestore(&state);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc != 0x80869bu)
        return;
    (void)Lufia2DecompNmiReadByte(cpu, 0x808699u);
    (void)Lufia2DecompNmiReadByte(cpu, 0x80869au);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompNmiAccountCycles(cpu, 3u,
                                 2u * cpu_region_speed(0x808699u) + 6u);
    Lufia2DecompFrameReturnComplete(1u, 0u);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif

void Lufia2DecompInstallNmiHooks(void) {
    static const uint32_t entries[] = {
#if defined(LUFIA2_HAVE_NATIVE_NMI_ENTRY)
        0x00862du,
#endif
        0x808638u, 0x80864cu, 0x808653u, 0x80865au,
#if defined(LUFIA2_HAVE_NATIVE_NMI_FINISH)
        0x808664u, 0x80869bu,
#endif
    };
    for (unsigned i = 0; i < sizeof entries / sizeof entries[0]; ++i)
        interp_bridge_set_pre_opcode_hook(entries[i], ActorBridgeNmiResume);
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    Lufia2DecompSetPlayTimeTickContinuation(ActorBridgeNmiPrepareFrameRestore);
    Lufia2DecompNmiInstallTickHook();
#endif
}
#endif

#if defined(LUFIA2_HAVE_NATIVE_FRAME_RETURNS)
static bool ActorBridgeTimedStepSupportedWindow(const CpuState *cpu, uint32_t entry,
                                                unsigned stack_limit,
                                                bool wide_accumulator,
                                                unsigned last_position, uint64_t room) {
    if (cpu->emulation || !cpu->ram || cpu->PB != entry >> 16 || cpu->m_flag > 1u ||
        cpu->x_flag > 1u || cpu->S < 0x1f00u || cpu->S > stack_limit ||
        (!wide_accumulator && !cpu->m_flag))
        return false;
    if (!g_snes || !g_snes->dma || g_snes->ram != cpu->ram || g_snes->inNmi ||
        g_snes->inIrq || g_snes->hIrqEnabled || g_snes->vIrqEnabled ||
        g_snes->beamMasterLast != cpu->master_cycles || g_snes->hPos > last_position ||
        g_snes->dma->dmaBusy || g_snes->dma->dmaTimer)
        return false;
    for (unsigned channel = 0; channel < 8u; ++channel)
        if (g_snes->dma->channel[channel].dmaActive)
            return false;
    const uint64_t deadline = Lufia2DecompFrameReturnDeadline();
    return deadline > cpu->master_cycles && deadline - cpu->master_cycles >= room;
}

static bool ActorBridgeTimedStepSupported(const CpuState *cpu, uint32_t entry,
                                          unsigned stack_limit, bool wide_accumulator) {
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, stack_limit,
                                               wide_accumulator, 800u, 256u);
}
#include "fixed_span_clock.inc"
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_INPUT_STEPS)
#include "battle_input_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_STEPS)
#include "ripple_steps_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TIMER_STEPS)
#include "battle_timer_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS)
#include "wave_fill_word_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_STEPS)
#include "effect_slot_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_DISPATCH_STEPS)
#include "effect_dispatch_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_CLEANUP_STEPS)
#include "effect_cleanup_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
#include "nmi_timer_clock_guard.inc"
#include "primary_wave_clock.inc"
#include "ripple_clock.inc"
#endif
#include "inventory_search_clock.inc"
#include "actor_fine_position_clock.inc"
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TRANSITION_STEPS)
#include "swirl_rim_clock.inc"
#include "swirl_hdma_clock.inc"
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_SWIRL_PARTS)
#include "swirl_parts_clock.inc"
#endif
#endif
#if defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_STEPS)
#include "decompress_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_REFERENCE_STEPS)
#include "decompress_reference_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_BACKGROUND_STEPS)
#include "background_word_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_QUEUE_STEPS)
#include "battle_queue_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_MENU_FADE_CONTINUATIONS)
#include "menu_fade_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_EQUIPMENT_GROUP_STEPS)
#include "equipment_group_clock.inc"
#endif
#if defined(LUFIA2_HAVE_NATIVE_ITEM_RECORD_STEPS)
#include "item_record_clock.inc"
#endif
static bool ActorBridgeFrameReturnSupported(const CpuState *cpu, uint32_t entry) {
    unsigned stack_limit = 0x1ff8u;
#if defined(LUFIA2_HAVE_NATIVE_SELECT_RESUMPTIONS)
    const bool selection = entry == 0x868333u || entry == 0x868352u ||
                           entry == 0x8682fcu;
    if (selection && (cpu->DB != 0x7eu || cpu->D || cpu->_flag_D || cpu->x_flag))
        return false;
#else
    const bool selection = false;
#endif
    if (entry != 0x8293ccu && entry != 0x868b52u && entry != 0x869756u &&
        entry != 0x868b6fu && !selection)
        return false;
    if (entry == 0x868b6fu) {
        stack_limit = 0x1ff7u;
        if (cpu->x_flag)
            return false;
    }
    return ActorBridgeTimedStepSupported(cpu, entry, stack_limit,
                                         entry == 0x869756u);
}

typedef struct ActorBridgeFrameReturnBus {
    CpuState *cpu;
    Lufia2FrameReturnSchedule schedule;
    unsigned opcode;
    unsigned pulled;
    unsigned progress_mask;
} ActorBridgeFrameReturnBus;

static uint8_t ActorBridgeFrameReturnRead(void *context, uint32_t address) {
    ActorBridgeFrameReturnBus *bus = context;
    bool progress = false;
    const uint8_t value = Lufia2DecompFrameReturnReadByte(bus->cpu, address, &progress);
    if (progress)
        bus->progress_mask |= 1u << bus->opcode;
    if (++bus->pulled == bus->schedule.pulled_bytes[bus->opcode]) {
        bus->pulled = 0u;
        ++bus->opcode;
    }
    return value;
}

static void ActorBridgeFrameReturn(CpuState *cpu, uint32_t entry) {
    if (!ActorBridgeFrameReturnSupported(cpu, entry))
        return;
    const Lufia2FrameReturnSchedule schedule =
        Lufia2FrameReturnScheduleFor(entry, cpu->ram[cpu->S + 1u], cpu->m_flag);
    const unsigned fetch_speed = cpu_region_speed(entry);
    ActorBridgeFrameReturnBus bus = {cpu, schedule, 0u, 0u, 0u};
    const Lufia2Memory memory = {ActorBridgeFrameReturnRead, NULL, &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        entry == 0x868b6fu ? Lufia2SpriteFrameRestoreRegisters(&memory, &state)
                           : Lufia2FrameWaitRestoreRegisters(&memory, &state, entry);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    for (unsigned step = 0; step < schedule.count; ++step)
        Lufia2DecompFrameReturnAccountOpcode(
            cpu, Lufia2FrameReturnCpuCycles(schedule.pulled_bytes[step]),
            Lufia2FrameReturnMasterClocks(schedule.pulled_bytes[step], fetch_speed),
            (bus.progress_mask & (1u << step)) != 0u);
    Lufia2DecompFrameReturnComplete(schedule.count, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}

#if defined(LUFIA2_HAVE_NATIVE_SELECT_RESUMPTIONS)
typedef struct ActorBridgeSelectResumeBus {
    CpuState *cpu;
    unsigned progress_mask;
} ActorBridgeSelectResumeBus;

static uint8_t ActorBridgeSelectResumeRead(void *context, uint32_t address) {
    ActorBridgeSelectResumeBus *bus = context;
    bool progress = false;
    const uint8_t value = Lufia2DecompFrameReturnReadByte(bus->cpu, address, &progress);
    if (progress)
        bus->progress_mask |= 1u;
    return value;
}

static void ActorBridgeSelectResumeWrite(void *context, uint32_t address,
                                          uint8_t value) {
    ActorBridgeSelectResumeBus *bus = context;
    bool progress = false;
    Lufia2DecompFrameReturnWriteByte(bus->cpu, address, value, &progress);
    if (progress)
        bus->progress_mask |= 1u << 3u;
}

static void ActorBridgeSelectResume(CpuState *cpu, uint32_t entry) {
    if (!ActorBridgeFrameReturnSupported(cpu, entry))
        return;
    ActorBridgeSelectResumeBus bus = {cpu, 0u};
    const Lufia2Memory memory = {ActorBridgeSelectResumeRead,
                                 ActorBridgeSelectResumeWrite, &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2SelectScreenStepResumption(&memory, &state, entry);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    const Lufia2SelectResumeSchedule schedule =
        Lufia2SelectResumeScheduleFor(entry, result.pc);
    const unsigned fetch_speed = cpu_region_speed(entry);
    ActorBridgeStore(cpu, &state);
    for (unsigned step = 0; step < schedule.count; ++step) {
        const Lufia2SelectResumeOpcode opcode = schedule.opcodes[step];
        Lufia2DecompFrameReturnAccountOpcode(
            cpu, opcode.cycles, Lufia2SelectResumeMasterClocks(opcode, fetch_speed),
            (bus.progress_mask & (1u << step)) != 0u);
    }
    Lufia2DecompFrameReturnComplete(schedule.count, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif

#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLLS)
static const uint32_t sound_poll_entries[] = {
    0x80994bu, 0x809997u, 0x8098f7u, 0x80980du, 0x809868u,
    0x809a0fu, 0x809a17u, 0x809a26u, 0x809a2cu, 0x809399u,
    0x8093a1u, 0x809c18u, 0x809c30u, 0x809c40u, 0x809c69u
};

#if !defined(LUFIA2_HAVE_NATIVE_SOUND_POLL_RUNS)
typedef struct ActorBridgeSoundPollBus {
    CpuState *cpu;
    uint64_t data_master;
    unsigned data_bytes;
    unsigned progress_mask;
} ActorBridgeSoundPollBus;

static uint8_t ActorBridgeSoundPollRead(void *context, uint32_t address) {
    ActorBridgeSoundPollBus *bus = context;
    bool progress = false;
    bus->data_master += cpu_region_speed(address);
    ++bus->data_bytes;
    const uint8_t value =
        Lufia2DecompFrameReturnReadByte(bus->cpu, address, &progress);
    if (progress)
        bus->progress_mask = 1u;
    return value;
}

static void ActorBridgeSoundPoll(CpuState *cpu, uint32_t entry) {
    if (!g_snes || !g_snes->apu ||
        cpu->m_flag != (entry != 0x809c18u))
        return;
    if (!ActorBridgeTimedStepSupported(cpu, entry, 0x1ff8u, true))
        return;
    ActorBridgeSoundPollBus bus = {cpu, 0u, 0u, 0u};
    const Lufia2Memory memory = {ActorBridgeSoundPollRead, NULL, &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SoundPollStep(&memory, &state, entry);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    const unsigned fetched = result.pc - entry;
    const uint64_t master = fetched * cpu_region_speed(entry) + bus.data_master;
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnAccountOpcode(cpu, fetched + bus.data_bytes, master,
                                         bus.progress_mask != 0u);
    Lufia2DecompFrameReturnComplete(1u, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
#endif

#if defined(LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS)
#if defined(LUFIA2_HAVE_NATIVE_MENU_RECTANGLE_STEPS)
#include "lufia2/menu_rectangle.h"
#endif
#if defined(LUFIA2_HAVE_NATIVE_FRAME_SAVE_STEPS)
#include "lufia2/frame_wait.h"
#endif
#if defined(LUFIA2_HAVE_NATIVE_MENU_CURSOR_POLL)
#include "lufia2/menu_input_phase.h"
#endif
static const uint32_t sound_upload_entries[] = {0x809969u, 0x80997du, 0x809988u,
                                                0x80998bu, 0x80999du};

typedef struct ActorTimedOpcode {
    uint16_t pc;
    uint8_t fetched;
    uint8_t cycles;
    uint8_t direct;
    uint8_t indexed;
    uint8_t branch;
} ActorTimedOpcode;

static const ActorTimedOpcode sound_upload_opcodes[] = {
    {0x9969u, 2u, 3u, 1u, 0u, 0u}, {0x996bu, 4u, 5u, 0u, 0u, 0u},
    {0x996fu, 2u, 3u, 1u, 0u, 0u}, {0x9971u, 4u, 5u, 0u, 0u, 0u},
    {0x9975u, 2u, 3u, 1u, 0u, 0u}, {0x9977u, 2u, 2u, 0u, 0u, 0u},
    {0x9979u, 4u, 5u, 0u, 0u, 0u}, {0x997du, 3u, 4u, 0u, 1u, 0u},
    {0x9980u, 2u, 3u, 1u, 0u, 0u}, {0x9982u, 1u, 2u, 0u, 0u, 0u},
    {0x9983u, 2u, 2u, 0u, 0u, 1u}, {0x9988u, 3u, 4u, 0u, 1u, 0u},
    {0x998bu, 2u, 3u, 1u, 0u, 0u}, {0x998du, 1u, 2u, 0u, 0u, 0u},
    {0x998eu, 2u, 2u, 0u, 0u, 1u}, {0x9993u, 2u, 3u, 1u, 0u, 0u},
    {0x9995u, 2u, 2u, 0u, 0u, 0u}, {0x999du, 2u, 5u, 1u, 0u, 0u},
    {0x999fu, 2u, 5u, 1u, 0u, 0u}, {0x99a1u, 3u, 3u, 0u, 0u, 0u},
    {0x99a4u, 1u, 2u, 0u, 0u, 0u}, {0x99a5u, 1u, 2u, 0u, 0u, 0u},
    {0x99a6u, 2u, 2u, 0u, 0u, 1u}, {0x99a8u, 2u, 2u, 0u, 0u, 0u},
    {0x99aau, 4u, 5u, 0u, 0u, 0u}, {0x9c23u, 2u, 6u, 1u, 0u, 0u},
    {0x9c25u, 1u, 2u, 0u, 0u, 0u}, {0x9c26u, 1u, 3u, 0u, 0u, 0u},
    {0x9c27u, 2u, 2u, 0u, 0u, 0u}, {0x9c29u, 2u, 3u, 0u, 0u, 0u},
    {0x9c2bu, 1u, 3u, 0u, 0u, 0u}, {0x9c2cu, 2u, 6u, 1u, 0u, 0u},
    {0x9c2eu, 1u, 2u, 0u, 0u, 0u}, {0x9c2fu, 1u, 3u, 0u, 0u, 0u},
    {0x9c35u, 1u, 2u, 0u, 0u, 0u}, {0x9c36u, 2u, 3u, 0u, 0u, 0u},
    {0x9c38u, 3u, 5u, 0u, 0u, 0u}, {0x9c3bu, 2u, 3u, 0u, 0u, 0u},
    {0x9c3du, 1u, 2u, 0u, 0u, 0u}, {0x9c3eu, 2u, 2u, 0u, 0u, 1u},
    {0x9c49u, 1u, 3u, 0u, 0u, 0u}, {0x9c4au, 2u, 3u, 0u, 0u, 0u},
    {0x9c4cu, 2u, 7u, 1u, 0u, 0u}, {0x9c4eu, 1u, 2u, 0u, 0u, 0u},
    {0x9c4fu, 1u, 2u, 0u, 0u, 0u}, {0x9c50u, 1u, 2u, 0u, 0u, 0u},
    {0x9c51u, 2u, 7u, 1u, 0u, 0u}, {0x9c53u, 1u, 2u, 0u, 0u, 0u},
    {0x9c54u, 1u, 2u, 0u, 0u, 0u}, {0x9c55u, 3u, 5u, 0u, 0u, 0u},
    {0x9c58u, 2u, 3u, 0u, 0u, 0u}, {0x9c5au, 3u, 3u, 0u, 0u, 0u},
    {0x9c5du, 2u, 2u, 0u, 0u, 0u}, {0x9c5fu, 1u, 2u, 0u, 0u, 0u},
    {0x9c60u, 3u, 4u, 0u, 0u, 0u}, {0x9c63u, 2u, 2u, 0u, 0u, 0u},
    {0x9c65u, 1u, 4u, 0u, 0u, 0u}, {0x9c66u, 3u, 4u, 0u, 0u, 0u}};

typedef struct ActorTimedBus {
    CpuState *cpu;
    const ActorTimedOpcode *opcode;
    uint64_t master;
    unsigned transfers;
    unsigned cycles;
    unsigned count;
    unsigned progress_mask;
    uint16_t word_value;
    unsigned word_start;
    bool word_bank_wrap;
    bool progress;
} ActorTimedBus;

static void ActorBridgeTimedFinish(ActorTimedBus *bus, uint32_t next) {
    if (!bus->opcode)
        return;
    const ActorTimedOpcode *op = bus->opcode;
    const unsigned cycles =
        bus->cycles +
        (op->branch && (uint16_t)next != (uint16_t)(op->pc + op->fetched));
    const uint64_t master =
        bus->master + (cycles > bus->transfers ? (cycles - bus->transfers) * 6u : 0u);
    if (bus->progress)
        bus->progress_mask |= 1u << bus->count;
    Lufia2DecompFrameReturnAccountOpcode(bus->cpu, cycles, master, bus->progress);
    ++bus->count;
    bus->opcode = NULL;
}

static uint8_t ActorBridgeTimedRead(void *context, uint32_t address) {
    ActorTimedBus *bus = context;
    bool progress = false;
    bus->master += cpu_region_speed(address);
    const unsigned transfer = bus->transfers++;
    uint8_t value;
    if (bus->word_start && transfer == bus->word_start) {
        bus->word_value = Lufia2DecompFrameReturnReadWord(
            bus->cpu, address,
            (address + 1u) & (bus->word_bank_wrap ? 0xffffu : 0xffffffu), &progress);
        value = (uint8_t)bus->word_value;
    } else if (bus->word_start && transfer == bus->word_start + 1u) {
        value = (uint8_t)(bus->word_value >> 8);
    } else {
        value = Lufia2DecompFrameReturnReadByte(bus->cpu, address, &progress);
    }
    bus->progress |= progress;
    return value;
}

static void ActorBridgeTimedWrite(void *context, uint32_t address, uint8_t value) {
    ActorTimedBus *bus = context;
    bool progress = false;
    bus->master += cpu_region_speed(address);
    const unsigned transfer = bus->transfers++;
    const bool wide_port =
        bus->opcode && (bus->opcode->pc == 0x9c38u || bus->opcode->pc == 0x9c55u);
    if (wide_port && transfer == bus->opcode->fetched) {
        bus->word_value = value;
    } else if (wide_port) {
        const uint16_t word = bus->word_value | (uint16_t)value << 8;
        Lufia2DecompFrameReturnWriteWord(bus->cpu, address - 1u, address, word,
                                         &progress);
    } else {
        Lufia2DecompFrameReturnWriteByte(bus->cpu, address, value, &progress);
    }
    bus->progress |= progress;
}

static void ActorBridgeSoundUploadCheckpoint(void *context, Lufia2CpuState *state,
                                             uint32_t pc) {
    ActorTimedBus *bus = context;
    ActorBridgeTimedFinish(bus, pc);
    ActorBridgeStore(bus->cpu, state);
    const ActorTimedOpcode *op = NULL;
    for (unsigned i = 0u;
         i < sizeof sound_upload_opcodes / sizeof sound_upload_opcodes[0]; ++i)
        if (sound_upload_opcodes[i].pc == (uint16_t)pc) {
            op = sound_upload_opcodes + i;
            break;
        }
    if (!op)
        return;
    bus->opcode = op;
    bus->word_start = op->pc == 0x9c4cu || op->pc == 0x9c51u ? 5u : 0u;
    bus->word_bank_wrap = false;
    bus->master = 0u;
    bus->transfers = 0u;
    bus->progress = false;
    bus->cycles = op->cycles + (op->direct && (state->direct_page & 0xffu)) +
                  (op->indexed && state->y >= 0x100u);
    for (unsigned byte = 0u; byte < op->fetched; ++byte)
        (void)ActorBridgeTimedRead(bus, pc + byte);
}

enum {
    SOUND_TRANSFER_BUS_CLOCKS = 12,
    SOUND_TRANSFER_REFRESH_CLOCKS = 40,
    SOUND_TRANSFER_SCANLINE_CLOCKS = 1364,
    SOUND_TRANSFER_HDMA_POSITION = 1024
};

static unsigned ActorBridgeSoundTransferBudget(uint32_t entry) {
    unsigned cycles, fetched;
    switch (entry) {
    case 0x809969u:
        cycles = 26u;
        fetched = 20u;
        break;
    case 0x80997du:
        cycles = 31u;
        fetched = 20u;
        break;
    case 0x809988u:
        cycles = 18u;
        fetched = 12u;
        break;
    case 0x80998bu:
        cycles = 13u;
        fetched = 9u;
        break;
    case 0x80999du:
        cycles = 27u;
        fetched = 17u;
        break;
    case 0x809c23u:
        cycles = 16u;
        fetched = 8u;
        break;
    case 0x809c2bu:
        cycles = 14u;
        fetched = 5u;
        break;
    case 0x809c35u:
        cycles = 18u;
        fetched = 11u;
        break;
    case 0x809c36u:
        cycles = 16u;
        fetched = 10u;
        break;
    case 0x809c3bu:
        cycles = 8u;
        fetched = 5u;
        break;
    case 0x809c49u:
        cycles = 59u;
        fetched = 32u;
        break;
    default:
        return 0u;
    }
    return fetched * cpu_region_speed(entry) +
           (cycles - fetched) * SOUND_TRANSFER_BUS_CLOCKS +
           SOUND_TRANSFER_REFRESH_CLOCKS;
}

static bool ActorBridgeSoundWindowSupported(const CpuState *cpu, uint32_t entry,
                                            unsigned stack_limit, unsigned budget) {
    if (!g_snes || !g_snes->apu || !budget)
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= SOUND_TRANSFER_SCANLINE_CLOCKS ||
        (refresh_clock && refresh_clock != cpu->master_cycles))
        return false;
    const unsigned fence = !g_snes->hdmaBeamOff && g_snes->vPos < 225u
                               ? SOUND_TRANSFER_HDMA_POSITION
                               : SOUND_TRANSFER_SCANLINE_CLOCKS;
    if (budget >= fence)
        return false;
    return ActorBridgeTimedStepSupportedWindow(cpu, entry, stack_limit, true,
                                               fence - budget - 1u, budget);
}

static bool ActorBridgeSingleOpcodeSupported(const CpuState *cpu, uint32_t entry,
                                             unsigned budget) {
    if (ActorBridgeSoundWindowSupported(cpu, entry, 0x1fffu, budget))
        return true;
    if (!g_snes || !g_snes->apu || !g_snes->hdmaBeamOff ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget)
        return false;
    uint64_t refresh_phase, refresh_clock;
    snes_refresh_state_get(&refresh_phase, &refresh_clock);
    if (refresh_phase >= SOUND_TRANSFER_SCANLINE_CLOCKS ||
        (refresh_clock && refresh_clock != cpu->master_cycles))
        return false;
    return ActorBridgeTimedStepSupportedWindow(
        cpu, entry, 0x1fffu, true, SOUND_TRANSFER_SCANLINE_CLOCKS - 1u, 1u);
}

static bool ActorBridgeSoundTransferSupported(const CpuState *cpu, uint32_t entry) {
    return !cpu->x_flag && !cpu->D && !cpu->_flag_D &&
           ActorBridgeSoundWindowSupported(cpu, entry, 0x1ff8u,
                                           ActorBridgeSoundTransferBudget(entry));
}

static void ActorBridgeSoundUpload(CpuState *cpu, uint32_t entry) {
    if (cpu->m_flag != 1u || !ActorBridgeSoundTransferSupported(cpu, entry))
        return;
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    const Lufia2Memory memory = {ActorBridgeTimedRead, ActorBridgeTimedWrite, &bus,
                                 ActorBridgeSoundUploadCheckpoint, &bus};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SoundUploadStep(&memory, &state, entry);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(bus.count, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS)
static const uint32_t sound_branch_entries[] = {
    0x80994fu, 0x80999bu, 0x8098fau, 0x809811u, 0x80986cu, 0x809a13u,
    0x809a1bu, 0x809a2au, 0x809a30u, 0x80939du, 0x8093a5u, 0x809c1bu,
    0x809c33u, 0x809c43u, 0x809c6cu, 0x809c47u, 0x809c6eu};
static const uint32_t boot_sound_entries[] = {0x809c23u, 0x809c2bu, 0x809c35u,
                                              0x809c36u, 0x809c3bu, 0x809c49u};

static void ActorBridgeSoundBranch(CpuState *cpu, uint32_t entry) {
    const uint32_t next =
        Lufia2SoundPollBranchTarget(entry, cpu->_flag_Z, cpu->_flag_V);
    const unsigned budget =
        2u * cpu_region_speed(entry) + 6u + SOUND_TRANSFER_REFRESH_CLOCKS;
    if (next == entry || !ActorBridgeSingleOpcodeSupported(cpu, entry, budget))
        return;
    bool progress = false;
    (void)Lufia2DecompFrameReturnReadByte(cpu, entry, &progress);
    (void)Lufia2DecompFrameReturnReadByte(cpu, entry + 1u, &progress);
    const unsigned taken = next != entry + 2u;
    Lufia2DecompFrameReturnAccountOpcode(
        cpu, 2u + taken, 2u * cpu_region_speed(entry) + taken * 6u, false);
    Lufia2DecompFrameReturnComplete(1u, 0u);
    interp_bridge_pre_opcode_redirect(next);
}
static void ActorBridgeBootSound(CpuState *cpu, uint32_t entry) {
    if (cpu->m_flag != (entry != 0x809c3bu) ||
        !ActorBridgeSoundTransferSupported(cpu, entry))
        return;
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    const Lufia2Memory memory = {ActorBridgeTimedRead, ActorBridgeTimedWrite, &bus,
                                 ActorBridgeSoundUploadCheckpoint, &bus};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2BootSoundTransferStep(&memory, &state, entry);
    if (result.flow != LUFIA2_EXECUTION_BOUNDARY || result.pc == entry)
        return;
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(bus.count, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLL_RUNS) ||                                     \
    defined(LUFIA2_HAVE_NATIVE_FRAME_POLL_RUNS)
typedef struct ActorPollRun {
    ActorTimedBus bus;
    ActorTimedOpcode poll;
    ActorTimedOpcode branch;
    uint32_t entry;
    bool opcode_fetch;
} ActorPollRun;

static bool ActorBridgePollRunSupported(const CpuState *cpu, uint32_t entry) {
    if (!g_snes || !g_snes->apu ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1fffu, true, 1240u, 128u))
        return false;
    if (!g_snes->hdmaBeamOff && g_snes->vPos < 225u && g_snes->hPos >= 896u)
        return false;
    return true;
}

static void ActorBridgePollRunCheckpoint(void *context, Lufia2CpuState *state,
                                         uint32_t pc) {
    ActorPollRun *run = context;
    ActorTimedBus *bus = &run->bus;
    ActorBridgeTimedFinish(bus, pc);
    ActorBridgeStore(bus->cpu, state);
    const ActorTimedOpcode *op = pc == run->entry ? &run->poll : &run->branch;
    bus->opcode = op;
    bus->word_start =
        op == &run->poll && !state->accumulator_is_8_bit ? op->fetched : 0u;
    bus->word_bank_wrap = false;
    bus->master = 0u;
    bus->transfers = 0u;
    bus->progress = false;
    bus->cycles = op->cycles;
    if (run->opcode_fetch &&
        Lufia2DecompPollFetchOpcode(bus->cpu, pc, op->fetched)) {
        /* The admitted ROM span cannot cross its constant-speed bank window. */
        bus->master = (uint64_t)op->fetched * cpu_region_speed(pc);
        bus->transfers = op->fetched;
    } else {
        for (unsigned byte = 0u; byte < op->fetched; ++byte)
            (void)ActorBridgeTimedRead(bus, pc + byte);
    }
}

typedef Lufia2ExecutionResult (*ActorPollIteration)(const Lufia2Memory *,
                                                    Lufia2CpuState *, uint32_t);

static void ActorBridgePollRun(CpuState *cpu, uint32_t entry,
                               ActorPollIteration iteration, unsigned fetched, bool opcode_fetch) {
    if (!ActorBridgePollRunSupported(cpu, entry))
        return;
    ActorPollRun run = {0};
    run.entry = entry;
    run.opcode_fetch = opcode_fetch;
    run.bus.cpu = cpu;
    run.poll = (ActorTimedOpcode){(uint16_t)entry,
                                  (uint8_t)fetched,
                                  (uint8_t)(fetched + (cpu->m_flag ? 1u : 2u)),
                                  0u,
                                  0u,
                                  0u};
    run.branch = (ActorTimedOpcode){(uint16_t)(entry + fetched), 2u, 2u, 0u, 0u, 1u};
    const Lufia2Memory memory = {ActorBridgeTimedRead, NULL, &run.bus,
                                 ActorBridgePollRunCheckpoint, &run};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    uint32_t next = entry;
    do {
        const Lufia2ExecutionResult result = iteration(&memory, &state, entry);
        if (result.flow != LUFIA2_EXECUTION_BOUNDARY || !run.bus.opcode)
            break;
        ActorBridgeTimedFinish(&run.bus, result.pc);
        next = result.pc;
        ActorBridgeStore(cpu, &state);
    } while (next == entry && run.bus.count < 16u &&
             ActorBridgePollRunSupported(cpu, entry));
    if (!run.bus.count)
        return;
    Lufia2DecompFrameReturnComplete(run.bus.count, run.bus.progress_mask);
    interp_bridge_pre_opcode_redirect(next);
}
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLL_RUNS)
static void ActorBridgeSoundPollStep(CpuState *cpu, uint32_t entry, unsigned fetched) {
    unsigned index = 0u;
    while (index < sizeof sound_poll_entries / sizeof sound_poll_entries[0] &&
           sound_poll_entries[index] != entry)
        ++index;
    if (index == sizeof sound_poll_entries / sizeof sound_poll_entries[0])
        return;
    const unsigned data = cpu->m_flag ? 1u : 2u;
    const unsigned budget = fetched * cpu_region_speed(entry) +
                            data * SOUND_TRANSFER_BUS_CLOCKS +
                            SOUND_TRANSFER_REFRESH_CLOCKS;
    if (!ActorBridgeSingleOpcodeSupported(cpu, entry, budget))
        return;
    ActorTimedBus bus = {0};
    const ActorTimedOpcode opcode = {
        (uint16_t)entry, (uint8_t)fetched, (uint8_t)(fetched + data), 0u, 0u, 0u};
    bus.cpu = cpu;
    bus.opcode = &opcode;
    bus.cycles = opcode.cycles;
    bus.word_start = cpu->m_flag ? 0u : fetched;
    for (unsigned byte = 0u; byte < fetched; ++byte)
        (void)ActorBridgeTimedRead(&bus, entry + byte);
    const Lufia2Memory memory = {ActorBridgeTimedRead, NULL, &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SoundPollStep(&memory, &state, entry);
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(1u, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}

static void ActorBridgeSoundPollRun(CpuState *cpu, uint32_t entry) {
    if (cpu->m_flag != (entry != 0x809c18u))
        return;
    const bool absolute = entry == 0x8098f7u || entry == 0x809c18u ||
                          entry == 0x809c30u || entry == 0x809c40u ||
                          entry == 0x809c69u;
    if (ActorBridgePollRunSupported(cpu, entry))
        ActorBridgePollRun(cpu, entry, Lufia2SoundPollIteration, absolute ? 3u : 4u, true);
    else
        ActorBridgeSoundPollStep(cpu, entry, absolute ? 3u : 4u);
}
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_TRANSFER_OPCODES)
static void ActorBridgeSoundTransferOpcode(CpuState *cpu, uint32_t entry) {
    const ActorTimedOpcode *opcode = NULL;
    for (unsigned i = 0u;
         i < sizeof sound_upload_opcodes / sizeof sound_upload_opcodes[0]; ++i)
        if (sound_upload_opcodes[i].pc == (uint16_t)entry) {
            opcode = sound_upload_opcodes + i;
            break;
        }
    const bool wide = entry == 0x809c38u || entry == 0x809c3bu ||
                      (entry >= 0x809c4cu && entry <= 0x809c58u);
    if (!opcode || entry >> 16 != 0x80u || cpu->m_flag != !wide ||
        cpu->x_flag || cpu->D || cpu->_flag_D)
        return;
    if (ActorBridgeSoundTransferSupported(cpu, entry)) {
        if (entry >= 0x809c00u)
            ActorBridgeBootSound(cpu, entry);
        else
            ActorBridgeSoundUpload(cpu, entry);
        return;
    }
    if (!g_snes || !g_snes->hdmaBeamOff ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    const unsigned cycles = opcode->cycles + opcode->branch +
                            (opcode->indexed && cpu->Y >= 0x100u);
    const unsigned budget = cycles * SOUND_TRANSFER_BUS_CLOCKS +
                            SOUND_TRANSFER_REFRESH_CLOCKS;
    if (!ActorBridgeSingleOpcodeSupported(cpu, entry, budget))
        return;
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    const Lufia2Memory memory = {ActorBridgeTimedRead, ActorBridgeTimedWrite,
                                 &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    ActorBridgeSoundUploadCheckpoint(&bus, &state, entry);
    const Lufia2ExecutionResult result = entry >= 0x809c00u
        ? Lufia2BootSoundOpcodeStep(&memory, &state, entry)
        : Lufia2SoundUploadOpcodeStep(&memory, &state, entry);
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(bus.count, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
#if defined(LUFIA2_HAVE_NATIVE_MENU_CURSOR_POLL)
static const ActorTimedOpcode cursor_poll_opcodes[] = {
    {0x8b0cu, 3u, 5u, 0u, 0u, 0u},
    {0x8b0fu, 1u, 2u, 0u, 0u, 0u}, {0x8b10u, 1u, 2u, 0u, 0u, 0u},
    {0x8b11u, 1u, 2u, 0u, 0u, 0u}, {0x8b12u, 1u, 2u, 0u, 0u, 0u},
    {0x8b13u, 1u, 2u, 0u, 0u, 0u},
    {0x8b14u, 3u, 4u, 0u, 0u, 0u},
    {0x8b17u, 2u, 2u, 0u, 0u, 0u},
    {0x8b19u, 3u, 6u, 0u, 0u, 0u}
};

static void ActorBridgeCursorPollCheckpoint(void *context, Lufia2CpuState *state,
                                            uint32_t pc) {
    ActorTimedBus *bus = context;
    ActorBridgeTimedFinish(bus, pc);
    ActorBridgeStore(bus->cpu, state);
    const ActorTimedOpcode *opcode = NULL;
    for (unsigned i = 0; i < sizeof cursor_poll_opcodes / sizeof cursor_poll_opcodes[0];
         ++i)
        if (cursor_poll_opcodes[i].pc == (uint16_t)pc) {
            opcode = cursor_poll_opcodes + i;
            break;
        }
    if (!opcode)
        return;
    bus->opcode = opcode;
    bus->word_start = pc == 0x828b0cu ? opcode->fetched : 0u;
    bus->word_bank_wrap = false;
    bus->master = 0u;
    bus->transfers = 0u;
    bus->progress = false;
    bus->cycles = opcode->cycles + (pc == 0x828b14u && state->x >= 0x3fu);
    for (unsigned i = 0; i < opcode->fetched; ++i)
        (void)ActorBridgeTimedRead(bus, pc + i);
}

static void ActorBridgePrepareCursorPoll(CpuState *cpu, uint32_t entry) {
    if (entry != 0x828b0cu || cpu->m_flag != 1u || cpu->x_flag ||
        !(cpu->DB < 0x40u || cpu->DB == 0x7eu ||
          (cpu->DB >= 0x80u && cpu->DB < 0xc0u)) ||
        !ActorBridgeSoundWindowSupported(cpu, entry, 0x1fffu,
                                         28u * SOUND_TRANSFER_BUS_CLOCKS +
                                         SOUND_TRANSFER_REFRESH_CLOCKS))
        return;
    const uint16_t cursor = cpu->ram[0x14a9u] | (uint16_t)cpu->ram[0x14aau] << 8;
    if ((uint16_t)(cursor - 5u) > 0xffu)
        return;
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    const Lufia2Memory memory = {ActorBridgeTimedRead, ActorBridgeTimedWrite,
                                 &bus, ActorBridgeCursorPollCheckpoint, &bus};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2MenuPrepareCursorPoll(&memory, &state);
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(bus.count, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif

#if defined(LUFIA2_HAVE_NATIVE_FRAME_SAVE_STEPS)
typedef struct ActorFrameSaveEntry {
    uint32_t address;
    uint8_t fetched;
    bool saves_accumulator;
} ActorFrameSaveEntry;

static const ActorFrameSaveEntry frame_save_entries[] = {
    {0x8293c2u, 1u, true}, {0x8293c3u, 1u, false}, {0x8293c4u, 2u, false},
    {0x868b48u, 1u, true}, {0x868b49u, 1u, false}, {0x868b4au, 2u, false},
    {0x86974fu, 1u, true}, {0x809378u, 1u, false}, {0x809379u, 2u, false},
    {0x84a5a8u, 1u, true}, {0x84a5a9u, 1u, false}, {0x84a5aau, 2u, false}
};

static void ActorBridgeFrameSaveStep(CpuState *cpu, uint32_t entry) {
    const ActorFrameSaveEntry *selected = NULL;
    for (unsigned i = 0; i < sizeof frame_save_entries / sizeof frame_save_entries[0];
         ++i)
        if (frame_save_entries[i].address == entry) {
            selected = frame_save_entries + i;
            break;
        }
    if (!selected)
        return;
    const unsigned cycles = 3u + (selected->saves_accumulator && !cpu->m_flag);
    const unsigned budget = cycles * SOUND_TRANSFER_BUS_CLOCKS +
                            SOUND_TRANSFER_REFRESH_CLOCKS;
    if (!ActorBridgeSingleOpcodeSupported(cpu, entry, budget))
        return;
    const ActorTimedOpcode opcode = {
        (uint16_t)entry, selected->fetched, (uint8_t)cycles, 0u, 0u, 0u
    };
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    bus.opcode = &opcode;
    bus.cycles = cycles;
    for (unsigned i = 0; i < opcode.fetched; ++i)
        (void)ActorBridgeTimedRead(&bus, entry + i);
    const Lufia2Memory memory = {ActorBridgeTimedRead, ActorBridgeTimedWrite,
                                 &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2FrameWaitSaveRegistersStep(&memory, &state, entry);
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(1u, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif

#if defined(LUFIA2_HAVE_NATIVE_MENU_RECTANGLE_STEPS)
typedef struct ActorMenuRectangleOpcode {
    ActorTimedOpcode timing;
    uint8_t accumulator_8;
} ActorMenuRectangleOpcode;

static const ActorMenuRectangleOpcode menu_rectangle_opcodes[] = {
    {{0x83b5u, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83b7u, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83b9u, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83bbu, 1u, 2u, 0u, 0u, 0u}, 0u},
    {{0x83bcu, 2u, 3u, 0u, 0u, 0u}, 0u},
    {{0x83beu, 2u, 3u, 0u, 0u, 0u}, 1u},
    {{0x83c0u, 1u, 3u, 0u, 0u, 0u}, 1u},
    {{0x83c1u, 2u, 3u, 0u, 0u, 0u}, 1u},
    {{0x83c3u, 2u, 3u, 0u, 0u, 0u}, 1u},
    {{0x83c5u, 1u, 3u, 0u, 0u, 0u}, 0u},
    {{0x83c6u, 2u, 3u, 0u, 0u, 0u}, 0u},
    {{0x83c8u, 2u, 2u, 0u, 0u, 0u}, 1u},
    {{0x83cau, 1u, 3u, 0u, 0u, 0u}, 1u},
    {{0x83cbu, 1u, 4u, 0u, 0u, 0u}, 1u},
    {{0x83ccu, 2u, 3u, 0u, 0u, 0u}, 1u},
    {{0x83ceu, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83d0u, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83d2u, 3u, 7u, 0u, 0u, 0u}, 0u},
    {{0x83d5u, 3u, 7u, 0u, 0u, 0u}, 0u},
    {{0x83d8u, 1u, 2u, 0u, 0u, 0u}, 0u},
    {{0x83d9u, 1u, 2u, 0u, 0u, 0u}, 0u},
    {{0x83dau, 1u, 2u, 0u, 0u, 0u}, 0u},
    {{0x83dbu, 2u, 2u, 0u, 0u, 1u}, 0u},
    {{0x83ddu, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83dfu, 1u, 2u, 0u, 0u, 0u}, 0u},
    {{0x83e0u, 3u, 3u, 0u, 0u, 0u}, 0u},
    {{0x83e3u, 2u, 4u, 0u, 0u, 0u}, 0u},
    {{0x83e5u, 2u, 7u, 0u, 0u, 0u}, 0u},
    {{0x83e7u, 2u, 2u, 0u, 0u, 1u}, 0u},
    {{0x83e9u, 1u, 4u, 0u, 0u, 0u}, 0u},
};

static void ActorBridgeMenuRectangleStep(CpuState *cpu, uint32_t entry) {
    const ActorMenuRectangleOpcode *selected = NULL;
    for (unsigned i = 0;
         i < sizeof menu_rectangle_opcodes / sizeof menu_rectangle_opcodes[0]; ++i)
        if ((0x820000u | menu_rectangle_opcodes[i].timing.pc) == entry) {
            selected = menu_rectangle_opcodes + i;
            break;
        }
    if (!selected || cpu->m_flag != selected->accumulator_8 || cpu->x_flag ||
        cpu->D || cpu->_flag_D || !g_snes || !g_snes->hdmaBeamOff ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return;
    if ((entry == 0x8283d2u || entry == 0x8283d5u) &&
        (cpu->DB != 0x7eu || cpu->X >= 0x0800u || (cpu->X & 1u)))
        return;
    const ActorTimedOpcode *opcode = &selected->timing;
    const unsigned budget = (opcode->cycles + opcode->branch) *
                                SOUND_TRANSFER_BUS_CLOCKS +
                            SOUND_TRANSFER_REFRESH_CLOCKS;
    if (!ActorBridgeSingleOpcodeSupported(cpu, entry, budget))
        return;
    ActorTimedBus bus = {0};
    bus.cpu = cpu;
    bus.opcode = opcode;
    bus.cycles = opcode->cycles;
    if (entry == 0x8283ceu || entry == 0x8283d0u || entry == 0x8283ddu ||
        entry == 0x8283e5u)
        bus.word_start = opcode->fetched;
    bus.word_bank_wrap = true;
    for (unsigned i = 0; i < opcode->fetched; ++i)
        (void)ActorBridgeTimedRead(&bus, entry + i);
    const Lufia2Memory memory = {ActorBridgeTimedRead, ActorBridgeTimedWrite,
                                 &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2MenuClearLayerRectangleStep(&memory, &state, entry);
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(1u, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif

#if defined(LUFIA2_HAVE_NATIVE_FRAME_POLL_RUNS)
typedef struct ActorFramePollEntry {
    uint32_t address;
    bool branch;
} ActorFramePollEntry;

static const ActorFramePollEntry frame_poll_entries[] = {
    {0x8293c8u, false}, {0x8293cau, true},  {0x83900eu, false}, {0x839010u, true},
    {0x8eb552u, false}, {0x8eb554u, true},  {0x868b4eu, false}, {0x868b50u, true},
    {0x869752u, false}, {0x869754u, true},
#if defined(LUFIA2_HAVE_NATIVE_FRAME_COUNTER_STEPS)
    {0x8293c6u, false}, {0x83900cu, false}, {0x8eb550u, false}, {0x868b4cu, false},
    {0x869750u, false}, {0x848d4du, false}, {0x848d4fu, false}, {0x848d51u, true},
#endif
#if defined(LUFIA2_HAVE_NATIVE_TEXT_FRAME_WAIT)
    {0x80bf0bu, false}, {0x80bf0du, false}, {0x80bf0fu, true},
#endif
#if defined(LUFIA2_HAVE_NATIVE_ADDITIONAL_FRAME_WAITS)
    {0x80937bu, false}, {0x80937du, false}, {0x80937fu, true},  {0x848363u, false},
    {0x848365u, false}, {0x848367u, true},  {0x84a5acu, false}, {0x84a5aeu, false},
    {0x84a5b0u, true},  {0x85851cu, false}, {0x85851eu, false}, {0x858520u, true},
    {0x85ec94u, false}, {0x85ec96u, false}, {0x85ec98u, true},  {0x85ecb7u, false},
    {0x85ecb9u, false}, {0x85ecbbu, true},  {0x8eb288u, false}, {0x8eb28au, false},
    {0x8eb28cu, true},  {0x8ec2c4u, false}, {0x8ec2c6u, false}, {0x8ec2c8u, true},
#endif
};

static bool ActorBridgeFramePollSupported(const CpuState *cpu, uint32_t entry) {
    if (ActorBridgePollRunSupported(cpu, entry))
        return true;
    if (!g_snes || !g_snes->hdmaBeamOff ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    return ActorBridgeSingleOpcodeSupported(
        cpu, entry, 5u * SOUND_TRANSFER_BUS_CLOCKS + SOUND_TRANSFER_REFRESH_CLOCKS);
}

/* Retire the existing semantic CMP/BEQ pair through the same timed bus.
 * Batch host dispatch and qualified pure-ROM fetch pairs. Every data read,
 * opcode clock, refresh charge and APU boundary retains its original order. */
static Lufia2ExecutionResult ActorBridgeFramePollIteration(
    const Lufia2Memory *memory, Lufia2CpuState *state, uint32_t entry) {
    ActorPollIteration step = Lufia2FrameWaitPollStep;
#if defined(LUFIA2_HAVE_NATIVE_TEXT_FRAME_WAIT)
    if (entry == 0x80bf0du) step = Lufia2FrameWaitCounterStep;
#endif
#if defined(LUFIA2_HAVE_NATIVE_ADDITIONAL_FRAME_WAITS)
    if (entry == 0x85ec96u) step = Lufia2FrameWaitAdditionalStep;
#endif
    Lufia2ExecutionResult result = step(memory, state, entry);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == entry + 2u)
        result = step(memory, state, result.pc);
    return result;
}

static void ActorBridgeFramePollStep(CpuState *cpu, uint32_t entry) {
    /* Try the independently certified, eventless counter interval before the
     * generic timed-dispatch window. Its own guards stop strictly before each
     * beam, refresh and interrupt edge, and preserves every APU flush;
     * ordinary polls keep the
     * original dispatcher, including line-end events. */
    if ((entry == 0x8293c8u || entry == 0x869752u
#if defined(LUFIA2_HAVE_NATIVE_ADDITIONAL_FRAME_WAITS)
         || entry == 0x85ec96u
#endif
#if defined(LUFIA2_HAVE_NATIVE_TEXT_FRAME_WAIT)
         || entry == 0x80bf0du
#endif
        ) && Lufia2DecompFramePollTryIdle(cpu, entry))
        return;
    if (!ActorBridgeFramePollSupported(cpu, entry))
        return;
    unsigned index = 0u;
    while (index < sizeof frame_poll_entries / sizeof frame_poll_entries[0] &&
           frame_poll_entries[index].address != entry)
        ++index;
    if (index == sizeof frame_poll_entries / sizeof frame_poll_entries[0])
        return;
    const bool branch = frame_poll_entries[index].branch;
    if (cpu->m_flag && !cpu->D && g_snes->hdmaBeamOff &&
        (entry == 0x8293c8u || entry == 0x869752u
#if defined(LUFIA2_HAVE_NATIVE_ADDITIONAL_FRAME_WAITS)
         || entry == 0x85ec96u
#endif
#if defined(LUFIA2_HAVE_NATIVE_TEXT_FRAME_WAIT)
         || entry == 0x80bf0du
#endif
        ) && Lufia2DecompFramePollBatchSupported(entry) &&
        ActorBridgePollRunSupported(cpu, entry)) {
        ActorBridgePollRun(cpu, entry, ActorBridgeFramePollIteration, 2u, true);
        return;
    }
    const bool read_counter = !branch;
    ActorTimedBus bus = {0};
    const ActorTimedOpcode opcode = {
        (uint16_t)entry,
        2u,
        (uint8_t)(read_counter ? 3u + !cpu->m_flag + ((cpu->D & 0xffu) != 0u) : 2u),
        0u,
        0u,
        branch};
    bus.cpu = cpu;
    bus.opcode = &opcode;
    bus.cycles = opcode.cycles;
    bus.word_start = read_counter && !cpu->m_flag ? 2u : 0u;
    bus.word_bank_wrap = true;
    for (unsigned byte = 0u; byte < 2u; ++byte)
        (void)ActorBridgeTimedRead(&bus, entry + byte);
    const Lufia2Memory memory = {ActorBridgeTimedRead, NULL, &bus, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
#if defined(LUFIA2_HAVE_NATIVE_ADDITIONAL_FRAME_WAITS)
        Lufia2FrameWaitAdditionalStep(&memory, &state, entry);
#elif defined(LUFIA2_HAVE_NATIVE_FRAME_COUNTER_STEPS)
        Lufia2FrameWaitCounterStep(&memory, &state, entry);
#else
        Lufia2FrameWaitPollStep(&memory, &state, entry);
#endif
    ActorBridgeTimedFinish(&bus, result.pc);
    ActorBridgeStore(cpu, &state);
    Lufia2DecompFrameReturnComplete(1u, bus.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
#endif
#endif

bool Lufia2DecompFramePollHookIsNative(void (*hook)(CpuState *, uint32_t)) {
#if defined(LUFIA2_HAVE_NATIVE_FRAME_POLL_RUNS)
    return hook == ActorBridgeFramePollStep;
#else
    (void)hook;
    return false;
#endif
}

void Lufia2DecompInstallFrameReturnHooks(void) {
#if defined(LUFIA2_HAVE_NATIVE_MENU_FADE_CONTINUATIONS)
    interp_bridge_set_pre_opcode_hook(0x868b2bu, ActorBridgeMenuFadeContinuation);
    interp_bridge_set_pre_opcode_hook(0x868b41u, ActorBridgeMenuFadeContinuation);
#endif
#if defined(LUFIA2_HAVE_NATIVE_FRAME_RETURNS)
    interp_bridge_set_pre_opcode_hook(0x82fb94u, ActorBridgeInventorySearch);
    interp_bridge_set_pre_opcode_hook(0x83a746u, ActorBridgeActorFinePosition);
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TIMER_STEPS)
    interp_bridge_set_pre_opcode_hook(0x858f1fu, ActorBridgeBattleTimerStep);
    interp_bridge_set_pre_opcode_hook(0x858f3eu, ActorBridgeBattleTimerStep);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS)
    interp_bridge_set_pre_opcode_hook(0x85aef5u, ActorBridgeWaveFillWord);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_SLOT_STEPS)
    interp_bridge_set_pre_opcode_hook(0x818a7eu, ActorBridgeEffectSlotStep);
    interp_bridge_set_pre_opcode_hook(0x818ac1u, ActorBridgeEffectSlotStep);
    interp_bridge_set_pre_opcode_hook(0x818aaau, ActorBridgeEffectSlotStep);
    interp_bridge_set_pre_opcode_hook(0x818b1bu, ActorBridgeEffectSlotStep);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_DISPATCH_STEPS)
    interp_bridge_set_pre_opcode_hook(0x818a95u, ActorBridgeEffectDispatchPrepare);
    interp_bridge_set_pre_opcode_hook(0x818b08u, ActorBridgeEffectDispatchPrepare);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_EFFECT_CLEANUP_STEPS)
    interp_bridge_set_pre_opcode_hook(0x818bfbu, ActorBridgeEffectClearBytes);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TRANSITION_STEPS)
    interp_bridge_set_pre_opcode_hook(0x848c91u, ActorBridgeSwirlRimStep);
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_SWIRL_PARTS)
    interp_bridge_set_pre_opcode_hook(0x848cddu, ActorBridgeSwirlHdmaDispatch);
    interp_bridge_set_pre_opcode_hook(0x848ce7u, ActorBridgeSwirlPartClock);
    interp_bridge_set_pre_opcode_hook(0x848cf3u, ActorBridgeSwirlPartClock);
#else
    interp_bridge_set_pre_opcode_hook(0x848cddu, ActorBridgeSwirlHdmaRow);
#endif
#endif
#if defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_STEPS)
    interp_bridge_set_pre_opcode_hook(0x808eefu, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808f93u, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808efau, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808f9eu, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808f66u, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x80900au, ActorBridgeDecompressClock);
#if defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_FLAG_STEPS)
    interp_bridge_set_pre_opcode_hook(0x808f40u, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808fe4u, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808f14u, ActorBridgeDecompressClock);
    interp_bridge_set_pre_opcode_hook(0x808fb8u, ActorBridgeDecompressClock);
#endif
#endif
#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_STEPS)
#if !defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    interp_bridge_set_pre_opcode_hook(0x85a736u, ActorBridgeRippleStepClock);
#endif
    interp_bridge_set_pre_opcode_hook(0x85a74fu, ActorBridgeRippleStepClock);
    interp_bridge_set_pre_opcode_hook(0x85a760u, ActorBridgeRippleStepClock);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_INPUT_STEPS)
    interp_bridge_set_pre_opcode_hook(0x85ec9au, ActorBridgeBattleInputClock);
    interp_bridge_set_pre_opcode_hook(0x85ecbdu, ActorBridgeBattleInputClock);
#endif
#if defined(LUFIA2_HAVE_NATIVE_DECOMPRESS_REFERENCE_STEPS)
    interp_bridge_set_pre_opcode_hook(0x808f44u, ActorBridgeDecompressReferenceClock);
    interp_bridge_set_pre_opcode_hook(0x808fe8u, ActorBridgeDecompressReferenceClock);
    interp_bridge_set_pre_opcode_hook(0x808f6fu, ActorBridgeDecompressReferenceClock);
    interp_bridge_set_pre_opcode_hook(0x809013u, ActorBridgeDecompressReferenceClock);
    interp_bridge_set_pre_opcode_hook(0x808f5cu, ActorBridgeDecompressReferenceClock);
    interp_bridge_set_pre_opcode_hook(0x809000u, ActorBridgeDecompressReferenceClock);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_BACKGROUND_STEPS)
    interp_bridge_set_pre_opcode_hook(0x81ba59u, ActorBridgeBackgroundDecodeWord);
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
    interp_bridge_set_pre_opcode_hook(0x85a8e7u, ActorBridgePrimaryWaveClock);
    interp_bridge_set_pre_opcode_hook(0x85a736u, ActorBridgeRippleRowClock);
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_QUEUE_STEPS)
    interp_bridge_set_pre_opcode_hook(0x85ecdbu, ActorBridgeBattleQueueStep);
    interp_bridge_set_pre_opcode_hook(0x85ecdeu, ActorBridgeBattleQueueStep);
#endif
#endif
#if defined(LUFIA2_HAVE_NATIVE_EQUIPMENT_GROUP_STEPS)
    interp_bridge_set_pre_opcode_hook(0x82bd70u, ActorBridgeEquipmentInventoryStep);
    interp_bridge_set_pre_opcode_hook(0x82bd78u, ActorBridgeEquipmentInventoryStep);
    interp_bridge_set_pre_opcode_hook(0x82bd82u, ActorBridgeEquipmentInventoryStep);
    interp_bridge_set_pre_opcode_hook(0x82bd8eu, ActorBridgeEquipmentInventoryStep);
#endif
#if defined(LUFIA2_HAVE_NATIVE_ITEM_RECORD_STEPS)
    interp_bridge_set_pre_opcode_hook(0x81f194u, ActorBridgeItemRecordStep);
    interp_bridge_set_pre_opcode_hook(0x81f19du, ActorBridgeItemRecordStep);
#endif
#if defined(LUFIA2_HAVE_NATIVE_MENU_CURSOR_POLL)
    interp_bridge_set_pre_opcode_hook(0x828b0cu, ActorBridgePrepareCursorPoll);
#endif
#if defined(LUFIA2_HAVE_NATIVE_FRAME_SAVE_STEPS)
    for (unsigned i = 0; i < sizeof frame_save_entries / sizeof frame_save_entries[0];
         ++i)
        interp_bridge_set_pre_opcode_hook(frame_save_entries[i].address,
                                          ActorBridgeFrameSaveStep);
#endif
#if defined(LUFIA2_HAVE_NATIVE_MENU_RECTANGLE_STEPS)
    for (unsigned i = 0;
         i < sizeof menu_rectangle_opcodes / sizeof menu_rectangle_opcodes[0]; ++i)
        interp_bridge_set_pre_opcode_hook(0x820000u | menu_rectangle_opcodes[i].timing.pc,
                                          ActorBridgeMenuRectangleStep);
#endif
#if defined(LUFIA2_HAVE_NATIVE_FRAME_POLL_RUNS)
    for (unsigned i = 0u; i < sizeof frame_poll_entries / sizeof frame_poll_entries[0];
         ++i)
        interp_bridge_set_pre_opcode_hook(frame_poll_entries[i].address,
                                          ActorBridgeFramePollStep);
#endif
    static const uint32_t entries[] = {0x8293ccu, 0x868b52u, 0x869756u, 0x868b6fu};
    for (unsigned i = 0; i < sizeof entries / sizeof entries[0]; ++i)
        interp_bridge_set_pre_opcode_hook(entries[i], ActorBridgeFrameReturn);
#if defined(LUFIA2_HAVE_NATIVE_SELECT_RESUMPTIONS)
    static const uint32_t selection_entries[] = {0x868333u, 0x868352u, 0x8682fcu};
    for (unsigned i = 0; i < sizeof selection_entries / sizeof selection_entries[0];
         ++i)
        interp_bridge_set_pre_opcode_hook(selection_entries[i],
                                          ActorBridgeSelectResume);
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLLS)
    for (unsigned i = 0u; i < sizeof sound_poll_entries / sizeof sound_poll_entries[0];
         ++i)
        interp_bridge_set_pre_opcode_hook(sound_poll_entries[i],
#if defined(LUFIA2_HAVE_NATIVE_SOUND_POLL_RUNS)
                                          ActorBridgeSoundPollRun);
#else
                                          ActorBridgeSoundPoll);
#endif
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_UPLOAD_STEPS)
    for (unsigned i = 0u;
         i < sizeof sound_upload_entries / sizeof sound_upload_entries[0]; ++i)
        interp_bridge_set_pre_opcode_hook(sound_upload_entries[i],
                                          ActorBridgeSoundUpload);
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_HANDSHAKE_STEPS)
    for (unsigned i = 0u;
         i < sizeof sound_branch_entries / sizeof sound_branch_entries[0]; ++i)
        interp_bridge_set_pre_opcode_hook(sound_branch_entries[i],
                                          ActorBridgeSoundBranch);
    for (unsigned i = 0u; i < sizeof boot_sound_entries / sizeof boot_sound_entries[0];
         ++i)
        interp_bridge_set_pre_opcode_hook(boot_sound_entries[i], ActorBridgeBootSound);
#endif
#if defined(LUFIA2_HAVE_NATIVE_SOUND_TRANSFER_OPCODES)
    for (unsigned i = 0u;
         i < sizeof sound_upload_opcodes / sizeof sound_upload_opcodes[0]; ++i)
        interp_bridge_set_pre_opcode_hook(0x800000u | sound_upload_opcodes[i].pc,
                                          ActorBridgeSoundTransferOpcode);
#endif
}
#endif

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
    result = Lufia2MainNmi(&memory, &state, ActorBridgePushedChild,
                           ActorBridgePlayTimeTick, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s,
                                     frame.hrv);
}

static void ActorBridgeMapLoad(void *context, Lufia2CpuState *state, uint32_t pc) {
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

RecompReturn Lufia2DecompBridge_84878A(CpuState *cpu) {
    if (cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x84878au);
    }
    return ActorBridgeObjectTransition(cpu, 0x84878au, Lufia2AncientCaveEnter, 3u);
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

/* Animation stepping requires M8, DP0 and an isolated stack. */
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
        cpu->x_flag || (needs_caller_frame && !cpu->m_flag) ||
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

/* Return skips its own guest call frame. */
static RecompReturn ActorBridgeReturnPastFrame(CpuState *cpu,
    const ActorBridgeFrame *frame, uint8_t frame_size, uint32_t source) {
    const uint16_t dispatch_stack = cpu->S;
    uint16_t low, high;
    uint32_t target;
    int ancestor;

    cpu->S = (uint16_t)(cpu->S + 1u);
    low = cpu_read8(cpu, 0, cpu->S);
    cpu->S = (uint16_t)(cpu->S + 1u);
    high = cpu_read8(cpu, 0, cpu->S);
    if (frame_size == 3u) {
        cpu->S = (uint16_t)(cpu->S + 1u);
        cpu->PB = cpu_read8(cpu, 0, cpu->S);
    }
    target = ((uint32_t)cpu->PB << 16) | (uint16_t)(((high << 8) | low) + 1u);
    ancestor = cpu_resolve_ancestor_skip(dispatch_stack);
    if (ancestor >= 0)
        return (RecompReturn)ancestor;
    if (interp_bridge_return_targets_owner(dispatch_stack, cpu->S))
        return interp_bridge_lle_yield_unwind(cpu, target);
    if (frame_size == 3u || interp_bridge_has_direct_paired_bounce())
        return interp_tier_dispatch_rewritten_return(cpu, target, source);
    return cpu_dispatch_pc_from(cpu, target, (uint16_t)(frame->entry_s + 2u), source);
}

static RecompReturn ActorBridgeReturnPastCall(CpuState *cpu,
    const ActorBridgeFrame *frame, uint32_t source) {
    return ActorBridgeReturnPastFrame(cpu, frame, 2u, source);
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
        (entry != 0x83acefu && cpu->DB != 0u && cpu->DB != 0x7eu && cpu->DB != 0x83u) ||
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

static int ActorBridgeActionRecordSourceMapped(const CpuState *cpu, uint32_t entry) {
    uint16_t index = cpu->A;
    uint32_t table;
    if (entry == 0x81f45eu) {
        --index;
        if (index & 0x8000u)
            return 1;
        table = 0x848f10u;
    } else if (entry == 0x81f46bu)
        table = 0x95f1efu;
    else if (entry == 0x81f476u)
        table = 0x97f63bu;
    else
        return 0;
    const uint32_t address = table + (uint16_t)(index << 1);
    for (unsigned byte = 0; byte < 2u; ++byte) {
        const uint16_t offset = (uint16_t)(address + byte);
        if (offset >= 0x2000u && offset < 0x8000u)
            return 0;
    }
    return 1;
}

static RecompReturn ActorBridgeActionRecord(
    CpuState *cpu, uint32_t entry, ActorWholeFunction lookup) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->m_flag || cpu->x_flag || cpu->_flag_D ||
        cpu->PB != 0x81u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu ||
        !ActorBridgeActionRecordSourceMapped(cpu, entry))
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

static RecompReturn ActorBridgePlayerInput(
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper) {
    const ActorBridgeFrame frame = {
        cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f00u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeWholeAnyWidth(cpu, entry, helper, 2u);
}

RecompReturn Lufia2DecompBridge_83FC56(CpuState *cpu) {
    return ActorBridgePlayerInput(cpu, 0x83fc56u,
        Lufia2FieldProbeInputAllowed);
}

RecompReturn Lufia2DecompBridge_83FC69(CpuState *cpu) {
    return ActorBridgePlayerInput(cpu, 0x83fc69u,
        Lufia2FieldUpdateProbeAction);
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
    CpuState *cpu, uint32_t entry, ActorWholeFunction helper,
    int allow_x8, uint16_t maximum_stack) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || !cpu->m_flag || (!allow_x8 && cpu->x_flag) || cpu->_flag_D ||
        cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f00u || cpu->S > maximum_stack)
        return ActorBridgeFallback(cpu, &frame, entry);
    return ActorBridgeRunWhole(cpu, entry, helper, 2u, allow_x8 ? 0 : 4);
}
RecompReturn Lufia2DecompBridge_83C0EF(CpuState *cpu) {
    return ActorBridgeActorEventHelper(cpu, 0x83c0efu, Lufia2FieldProbeLeaderPosition, 1, 0x1ffdu);
}
RecompReturn Lufia2DecompBridge_83C0FA(CpuState *cpu) {
    return ActorBridgeActorEventHelper(cpu, 0x83c0fau, Lufia2FieldAcknowledgeControlChange, 1, 0x1ffdu);
}
RecompReturn Lufia2DecompBridge_83F0BC(CpuState *cpu) {
    return ActorBridgeActorEventHelper(cpu, 0x83f0bcu, Lufia2FieldSetFollowingObjectDrawFlags, 0, 0x1ffcu);
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
    return ActorBridgeSceneRecordHelper(
        cpu, 0x80c0d0u, Lufia2SceneScriptReadWord, cpu->m_flag != 0u, 2u);
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
    if (cpu->S == 0x1ffdu &&
        (cpu->host_return_valid == 0u || cpu->host_return_valid == 2u) &&
        cpu->PB == 0x83u && !cpu->D && ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeWholeM1X16(cpu, 0x83bb76u, Lufia2FieldPrepareEventControl,
                                     2u);
    return ActorBridgeEventControlHelper(cpu, 0x83bb76u, Lufia2FieldPrepareEventControl,
                                         2u);
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
        cpu->D || cpu->S < 0x1f60u || cpu->S > 0x1ffdu)
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

static RecompReturn ActorBridgeBattleSessionCalls(
    CpuState *cpu, ActorSceneOwnersFunction run);

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

RecompReturn Lufia2DecompBridge_82AE64(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ae64u, 0, Lufia2MenuAddEffectTarget,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829588(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829588u, 0, Lufia2MenuResetEffectSprites,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829E2F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829e2fu, 0, Lufia2MenuUpdateItemCapacityHint,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81F034(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f034u, 0, Lufia2InventoryConsumeMenuItem,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8295BF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8295bfu, 0, Lufia2MenuShowUseEffect,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8295FA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8295fau, 0, Lufia2MenuPositionEffectSprites,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82970B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82970bu, 0, Lufia2MenuLoadEffectDigitAnimation,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82975D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82975du, 0, Lufia2MenuRevealEffectLabel,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829765(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829765u, 0, Lufia2MenuRevealFirstEffectDigit,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829770(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829770u, 0, Lufia2MenuRevealSecondEffectDigit,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82977B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82977bu, 0, Lufia2MenuRevealThirdEffectDigit,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829786(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829786u, 0, Lufia2MenuRevealEffectSuffix,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AAFF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82aaffu, 0, Lufia2MenuDispatchListAction,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AB46(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ab46u, 0, Lufia2MenuClearListSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AB5B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ab5bu, 0, Lufia2MenuClearSelectionWithFeedback,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AB89(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ab89u, 0, Lufia2MenuSwapInventorySelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AB9A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ab9au, 0, Lufia2MenuDiscardInventorySelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82ABB1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82abb1u, 0, Lufia2MenuUseSelectedItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B42D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b42du, 0, Lufia2MenuSwapSpellSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B45B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b45bu, 0, Lufia2MenuUseSelectedSpell,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B564(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b564u, 0, Lufia2MenuCountSpellTargets,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B58A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b58au, 0, Lufia2MenuFillSpellTargets,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B59A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b59au, 0, Lufia2MenuPrepareSpellTargets,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B5CF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b5cfu, 0, Lufia2MenuReadSelectedSpell,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FBEE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fbeeu, 0, Lufia2InventoryReadSelectedItem,
                                  2u, BATTLE_BRIDGE_M0X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FC08(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fc08u, 0, Lufia2InventorySwapSelectedItems,
                                  2u, BATTLE_BRIDGE_M0X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FC1F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fc1fu, 0, Lufia2InventoryDiscardSelectedItem,
                                  2u, BATTLE_BRIDGE_M0X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82A9B5(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82a9b5u, 0, Lufia2MenuSortInventory,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B3AD(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b3adu, 0, Lufia2MenuSortSpells,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B5ED(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b5edu, 0, Lufia2MenuPrepareSpellMember,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B60A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b60au, 0, Lufia2MenuSelectAdjacentSpellMember,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81F1AB(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f1abu, 0, Lufia2ItemRecordWord7,
                                  3u, BATTLE_BRIDGE_ANY_WIDTH, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82922D(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82922du);
    }
    return ActorBridgeSceneOwners(cpu, 0x82922du, Lufia2MenuClearSecondarySpriteMask, 3u);
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

RecompReturn Lufia2DecompBridge_818351(CpuState *cpu) {
    if (cpu->S < 0x1f20u || cpu->S > 0x1ffcu || (uint8_t)cpu->A >= 32u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x818351u);
    }
    return ActorBridgeBattleEntry(cpu, 0x818351u, 0, Lufia2FieldGenerateEncounter,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
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

static int ActorBridgeRecordBankSupported(uint8_t bank) {
    return bank == 0x7eu || ActorBridgeSystemBank(bank);
}

RecompReturn Lufia2DecompBridge_82F703(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f703u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f703u, Lufia2PartySelectEquipmentSlot, 2u);
}

RecompReturn Lufia2DecompBridge_82F710(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f710u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f710u, Lufia2PartyLoadEquipmentItem, 2u);
}

RecompReturn Lufia2DecompBridge_82F722(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f722u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f722u,
                                    Lufia2PartyAccumulateEquipmentModifiers, 2u);
}

RecompReturn Lufia2DecompBridge_82F846(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f846u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f846u, Lufia2PartyRebuildEquipmentModifiers,
                                  2u);
}

RecompReturn Lufia2DecompBridge_82994E(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82994eu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82994eu, Lufia2PartyRebuildAllEquipmentStats,
                                  3u);
}


RecompReturn Lufia2DecompBridge_81EC56(CpuState *cpu) {
    if (cpu->PB != 0x81u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ec56u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ec56u, Lufia2InitializeDefaultRecords, 3u);
}


RecompReturn Lufia2DecompBridge_81ED35(CpuState *cpu) {
    if (cpu->PB != 0x81u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ed35u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ed35u, Lufia2PartyResetDefaultRecords, 3u);
}

RecompReturn Lufia2DecompBridge_81EC35(CpuState *cpu) {
    if (cpu->PB != 0x81u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ec35u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ec35u, Lufia2StartDefaultRecords, 3u);
}

RecompReturn Lufia2DecompBridge_829971(CpuState *cpu) {
    if (!cpu->emulation && !cpu->_flag_D && cpu->m_flag && !cpu->x_flag &&
        cpu->PB == 0x82u && !cpu->D &&
        (cpu->host_return_valid == 0u || cpu->host_return_valid == 2u) &&
        cpu->S >= 0x1f10u && cpu->S <= 0x1ffcu &&
        ActorBridgeRecordBankSupported(cpu->DB))
        return ActorBridgeWholeAnyWidth(cpu, 0x829971u,
                                        Lufia2PartySelectActiveEquipmentMember, 2u);

    if (cpu->PB != 0x82u || cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829971u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x829971u,
                                    Lufia2PartySelectActiveEquipmentMember, 2u);
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
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e8deu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e8deu, Lufia2TitlePrepareSaveSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82E917(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e917u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e917u, Lufia2TitlePrepareSelectionMode, 2u);
}

RecompReturn Lufia2DecompBridge_82E998(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e998u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e998u, Lufia2TitleLoadSaveSelectionBits, 2u);
}

RecompReturn Lufia2DecompBridge_80905F(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80905fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80905fu, Lufia2SaveTestFileHighFlags, 3u);
}

RecompReturn Lufia2DecompBridge_86916C(CpuState *cpu) {
    if (cpu->PB != 0x86u || !ActorBridgeRecordBankSupported(cpu->DB) || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86916cu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x86916cu, Lufia2MenuLoadSelectionPalettes,
                                    3u);
}

RecompReturn Lufia2DecompBridge_809073(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->DB != 0x7eu || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x809073u);
    }
    return ActorBridgeSceneOwners(cpu, 0x809073u, Lufia2SaveCheckGameFile, 3u);
}

RecompReturn Lufia2DecompBridge_82F20A(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f20au);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f20au, Lufia2SaveBuildMenuSlots, 2u);
}

RecompReturn Lufia2DecompBridge_82F22B(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f22bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f22bu, Lufia2SaveBuildMenuSlot, 2u);
}

RecompReturn Lufia2DecompBridge_82F481(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f481u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f481u, Lufia2SaveRestoreMenuSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82EEF7(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82eef7u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82eef7u, Lufia2SavePrepareMenuCursors, 2u);
}

RecompReturn Lufia2DecompBridge_82E941(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e941u);
    }
    return ActorBridgeSceneOwners(cpu,0x82e941u,Lufia2SaveOpenMenu,2u);
}

RecompReturn Lufia2DecompBridge_82E95D(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e95du);
    }
    return ActorBridgeSceneOwners(cpu,0x82e95du,Lufia2SaveRunAlternateMenu,2u);
}

RecompReturn Lufia2DecompBridge_82EF25(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ef25u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82ef25u, Lufia2SavePrepareSlotDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_82ECAF(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82ecafu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82ecafu, Lufia2SaveTestRecordList, 2u);
}

RecompReturn Lufia2DecompBridge_82EFB5(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82efb5u);
    }
    return ActorBridgeSceneOwners(cpu,0x82efb5u,Lufia2SaveResetSlotCursor,2u);
}

RecompReturn Lufia2DecompBridge_82ECF3(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82ecf3u);
    }
    return ActorBridgeSceneOwners(cpu,0x82ecf3u,Lufia2SaveSelectAlternatePartyMember,2u);
}

RecompReturn Lufia2DecompBridge_82F139(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f139u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f139u,Lufia2SavePrepareAlternateDisplay,2u);
}

RecompReturn Lufia2DecompBridge_82F190(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f190u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f190u,Lufia2SavePrepareAlternateActionDisplay,2u);
}

RecompReturn Lufia2DecompBridge_82E9D5(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e9d5u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e9d5u, Lufia2SaveRunMenuActions, 2u);
}

RecompReturn Lufia2DecompBridge_82EA3B(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82ea3bu);
    }
    return ActorBridgeSceneOwners(cpu,0x82ea3bu,Lufia2SaveRunSlotActions,2u);
}

RecompReturn Lufia2DecompBridge_82F0F6(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f0f6u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f0f6u, Lufia2MenuPrepareNameEntryUpperText,
                                  2u);
}

RecompReturn Lufia2DecompBridge_82F117(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f117u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f117u, Lufia2MenuPrepareNameEntryLowerText,
                                  2u);
}

RecompReturn Lufia2DecompBridge_82F4F4(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f4f4u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f4f4u,
                                    Lufia2MenuPrepareNameEntryTilePointers, 2u);
}

RecompReturn Lufia2DecompBridge_82F556(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f556u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82f556u, Lufia2MenuAppendNameEntryGlyph, 2u);
}

RecompReturn Lufia2DecompBridge_82F50A(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f50au);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f50au, Lufia2MenuDrawNameEntryRecord, 2u);
}

RecompReturn Lufia2DecompBridge_82F523(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f523u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f523u, Lufia2MenuInsertNameEntryCharacter,
                                  2u);
}

RecompReturn Lufia2DecompBridge_82F585(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f585u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f585u, Lufia2MenuRemoveNameEntryCharacter,
                                  2u);
}

RecompReturn Lufia2DecompBridge_82F5AF(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f5afu);
    }
    return ActorBridgeSceneOwners(cpu, 0x82f5afu, Lufia2MenuClearNameEntryRecord, 2u);
}

RecompReturn Lufia2DecompBridge_82F496(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f496u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f496u,Lufia2MenuPrepareAlternateGraphics,2u);
}

RecompReturn Lufia2DecompBridge_82F637(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f637u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f637u,Lufia2MenuDrawSelectionMessage,2u);
}

RecompReturn Lufia2DecompBridge_82F682(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f682u);
    }
    return ActorBridgeWholeAnyWidth(cpu,0x82f682u,Lufia2MenuRestoreSavedFieldBuffers,2u);
}

RecompReturn Lufia2DecompBridge_82F893(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f893u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f893u,Lufia2MenuTestEquipmentItemFlag,2u);
}

RecompReturn Lufia2DecompBridge_82EB9A(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82eb9au);
    }
    return ActorBridgeSceneOwners(cpu,0x82eb9au,Lufia2MenuConfirmExit,2u);
}

RecompReturn Lufia2DecompBridge_82ED50(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82ed50u);
    }
    return ActorBridgeSceneOwners(cpu,0x82ed50u,Lufia2MenuConfirmAlternateSelection,2u);
}

RecompReturn Lufia2DecompBridge_82EFF6(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82eff6u);
    }
    return ActorBridgeSceneOwners(cpu,0x82eff6u,Lufia2MenuRestoreAlternateSelection,2u);
}

RecompReturn Lufia2DecompBridge_82F5B5(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f5b5u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f5b5u,Lufia2MenuComputeSelectionPrice,2u);
}

RecompReturn Lufia2DecompBridge_829855(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x829855u);
    }
    return ActorBridgeWholeM1X16(cpu,0x829855u,Lufia2MenuCheckSelectionGold,2u);
}

RecompReturn Lufia2DecompBridge_82983D(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82983du);
    }
    return ActorBridgeWholeM1X16(cpu,0x82983du,Lufia2MenuSpendSelectionGold,2u);
}

RecompReturn Lufia2DecompBridge_82EDDF(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82eddfu);
    }
    return ActorBridgeSceneOwners(cpu,0x82eddfu,Lufia2MenuApplyAlternateSelection,2u);
}

RecompReturn Lufia2DecompBridge_8297E3(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x8297e3u);
    }
    return ActorBridgeSceneOwners(cpu,0x8297e3u,Lufia2MenuPlaySelectionFeedback,2u);
}

RecompReturn Lufia2DecompBridge_82F8C6(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f8c6u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f8c6u,Lufia2MenuAdvanceFlaggedEquipment,2u);
}

RecompReturn Lufia2DecompBridge_82F9E8(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f9e8u);
    }
    return ActorBridgeWholeM0X0(cpu,0x82f9e8u,Lufia2InventoryStorePackedSlot,2u);
}

RecompReturn Lufia2DecompBridge_82F9F2(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82f9f2u);
    }
    return ActorBridgeSceneOwners(cpu,0x82f9f2u,Lufia2InventoryAddMenuItem,2u);
}

RecompReturn Lufia2DecompBridge_82FA2E(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82fa2eu);
    }
    return ActorBridgeSceneOwners(cpu,0x82fa2eu,Lufia2InventoryRegisterPossession,2u);
}

RecompReturn Lufia2DecompBridge_82FAA0(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82faa0u);
    }
    return ActorBridgeWholeM0X0(cpu,0x82faa0u,Lufia2InventoryIncreaseSlotQuantity,2u);
}

RecompReturn Lufia2DecompBridge_82FAF4(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82faf4u);
    }
    return ActorBridgeWholeM0X0(cpu,0x82faf4u,Lufia2InventorySlotAtQuantityLimit,2u);
}

RecompReturn Lufia2DecompBridge_82FB94(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82fb94u);
    }
    return ActorBridgeWholeM0X0(cpu,0x82fb94u,Lufia2InventoryFindEmptySlot,2u);
}

RecompReturn Lufia2DecompBridge_82FB51(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82fb51u);
    }
    return ActorBridgeWholeM0X0(cpu,0x82fb51u,Lufia2ItemCheckPossessionBit,2u);
}

RecompReturn Lufia2DecompBridge_829246(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag ||
            cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u ||
            cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829246u);
    }
    return ActorBridgeSceneOwners(cpu,0x829246u,Lufia2MenuPollSelectionChoice,2u);
}

RecompReturn Lufia2DecompBridge_8292F9(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag ||
            cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u ||
            cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8292f9u);
    }
    return ActorBridgeSceneOwners(cpu,0x8292f9u,Lufia2MenuRestoreSelectionCursor,2u);
}

RecompReturn Lufia2DecompBridge_829AA3(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829aa3u);
    }
    return ActorBridgeSceneOwners(cpu, 0x829aa3u, Lufia2MenuResetSessionRecords, 3u);
}

RecompReturn Lufia2DecompBridge_82C2AE(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c2aeu);
    }
    return ActorBridgeWholeM1X16(cpu, 0x82c2aeu, Lufia2CapsuleInitializeSavedRecords,
                                 2u);
}

RecompReturn Lufia2DecompBridge_829808(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x7eu || !cpu->m_flag ||
            cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u ||
            cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x829808u);
    }
    return ActorBridgeWholeM1X16(cpu,0x829808u,Lufia2MenuCreditSelectionGold,2u);
}

RecompReturn Lufia2DecompBridge_82F660(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag ||
            cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u ||
            cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82f660u);
    }
    return ActorBridgeWholeM1X16(cpu,0x82f660u,Lufia2MenuSaveFieldBuffers,2u);
}

RecompReturn Lufia2DecompBridge_82C443(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82c443u);
    }
    return ActorBridgeSceneOwners(cpu,0x82c443u,Lufia2CapsuleStoreSavedStats,2u);
}

RecompReturn Lufia2DecompBridge_82C4A2(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82c4a2u);
    }
    return ActorBridgeWholeM1X16(cpu,0x82c4a2u,Lufia2CapsuleSelectAbilityFlags,2u);
}

RecompReturn Lufia2DecompBridge_82C4B3(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82c4b3u);
    }
    return ActorBridgeSceneOwners(cpu,0x82c4b3u,Lufia2CapsuleReadAbilityByte,2u);
}

RecompReturn Lufia2DecompBridge_82CCFE(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82ccfeu);
    }
    return ActorBridgeWholeM1X16(cpu,0x82ccfeu,Lufia2CapsuleBuildAbilityMask,2u);
}

RecompReturn Lufia2DecompBridge_82CD41(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82cd41u);
    }
    return ActorBridgeSceneOwners(cpu,0x82cd41u,Lufia2CapsuleUnlockAbility,2u);
}

RecompReturn Lufia2DecompBridge_82FD3D(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82fd3du);
    }
    return ActorBridgeSceneOwners(cpu,0x82fd3du,Lufia2PartyLearnSelectedSpell,3u);
}

RecompReturn Lufia2DecompBridge_82E75D(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e75du);
    }
    return ActorBridgeSceneOwners(cpu,0x82e75du,Lufia2TitleOpenSaveSelection,3u);
}

RecompReturn Lufia2DecompBridge_82E77C(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e77cu);
    }
    return ActorBridgeSceneOwners(cpu,0x82e77cu,Lufia2TitleRestoreSaveSelection,3u);
}

RecompReturn Lufia2DecompBridge_82E78D(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e78du);
    }
    return ActorBridgeSceneOwners(cpu,0x82e78du,Lufia2TitleOpenAlternateSaveSelection,3u);
}

RecompReturn Lufia2DecompBridge_82E861(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e861u);
    }
    return ActorBridgeSceneOwners(cpu,0x82e861u,Lufia2MenuOpenAlternateSaveSelection,3u);
}

RecompReturn Lufia2DecompBridge_82E87B(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e87bu);
    }
    return ActorBridgeSceneOwners(cpu,0x82e87bu,Lufia2MenuOpenClearedSaveSelection,3u);
}

RecompReturn Lufia2DecompBridge_82E8B2(CpuState *cpu) {
    if (cpu->PB != 0x82u || !ActorBridgeRecordBankSupported(cpu->DB) || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82e8b2u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82e8b2u, Lufia2MenuRunSaveSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82E7AA(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e7aau);
    }
    return ActorBridgeSceneOwners(cpu,0x82e7aau,Lufia2CapsuleOpenSelection,3u);
}

RecompReturn Lufia2DecompBridge_82E80C(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82e80cu);
    }
    return ActorBridgeSceneOwners(cpu,0x82e80cu,Lufia2MenuOpenItemSelection,3u);
}

RecompReturn Lufia2DecompBridge_82EBF2(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82ebf2u);
    }
    return ActorBridgeSceneOwners(cpu,0x82ebf2u,Lufia2MenuRunNameEntry,2u);
}

RecompReturn Lufia2DecompBridge_82B104(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82b104u);
    }
    return ActorBridgeSceneOwners(cpu,0x82b104u,Lufia2MenuRunItemSelection,2u);
}

RecompReturn Lufia2DecompBridge_82EEF2(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82eef2u);
    }
    return ActorBridgeSceneOwners(cpu,0x82eef2u,Lufia2MenuRunScenarioSelection,2u);
}

RecompReturn Lufia2DecompBridge_82EAF4(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82eaf4u);
    }
    return ActorBridgeSceneOwners(cpu,0x82eaf4u,Lufia2SaveRunSelection,2u);
}

RecompReturn Lufia2DecompBridge_82EE28(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82ee28u);
    }
    return ActorBridgeSceneOwners(cpu,0x82ee28u,Lufia2SaveRunEraseSelection,2u);
}

RecompReturn Lufia2DecompBridge_82EEC2(CpuState *cpu) {
    if(cpu->PB!=0x82u||cpu->DB!=0x7eu||!cpu->m_flag||cpu->x_flag||cpu->D!=0u||cpu->S<0x1f10u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,0x82eec2u);
    }
    return ActorBridgeSceneOwners(cpu,0x82eec2u,Lufia2SaveDrawErasePrompt,2u);
}

RecompReturn Lufia2DecompBridge_8EE710(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->Y >= 6u || cpu->S < 0x1f00u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ee710u);
    }
    return ActorBridgeWholeAnyWidth(
        cpu, 0x8ee710u, Lufia2MenuSaveSelectionState, 3u);
}

RecompReturn Lufia2DecompBridge_8386EA(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8386eau);
    }
    return ActorBridgeSceneOwners(cpu, 0x8386eau, Lufia2FieldApplyObjectConditions, 2u);
}

RecompReturn Lufia2DecompBridge_838728(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838728u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838728u, Lufia2FieldApplySavedObjectControls, 2u);
}

RecompReturn Lufia2DecompBridge_80EA5B(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ea5bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80ea5bu, Lufia2FieldInitializePendingObjects, 3u);
}

RecompReturn Lufia2DecompBridge_83B581(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83b581u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83b581u, Lufia2FieldResetMapEventState, 2u);
}

RecompReturn Lufia2DecompBridge_8EE6EA(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ee6eau);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8ee6eau, Lufia2MenuResetSaveSelectionRecords, 3u);
}

RecompReturn Lufia2DecompBridge_838386(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838386u);
    }
    return ActorBridgeSceneOwners(cpu, 0x838386u, Lufia2FieldRunSaveMenu, 2u);
}

RecompReturn Lufia2DecompBridge_8294C0(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8294c0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8294c0u, Lufia2MenuDrawCharacterSummary, 2u);
}

RecompReturn Lufia2DecompBridge_80CC26(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80cc26u);
    }
    return ActorBridgeWholeM1X16(cpu, 0x80cc26u, Lufia2FieldRequestTilemapUploads, 3u);
}

RecompReturn Lufia2DecompBridge_80EC98(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ec98u);
    }
    return ActorBridgeWholeM0X0(cpu, 0x80ec98u, Lufia2FieldDecompressMapData, 3u);
}

RecompReturn Lufia2DecompBridge_81DEA9(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81dea9u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81dea9u, Lufia2BattleDrawPartyStatusWindows, 2u);
}

RecompReturn Lufia2DecompBridge_81EB93(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffbu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81eb93u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81eb93u, Lufia2BattleRefreshPartyStatusSprites, 3u);
}

RecompReturn Lufia2DecompBridge_81E1B5(CpuState *cpu) {
    if (cpu->PB != 0x81u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81e1b5u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81e1b5u, Lufia2BattleDrawPartyStatusDetail, 2u);
}

RecompReturn Lufia2DecompBridge_81E308(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81e308u);
    }
    return ActorBridgeBattleEntry(cpu, 0x81e308u,
        Lufia2BattleFillStatusGauge, 0, 2u,
        BATTLE_BRIDGE_M1X0, 0, 0x7e, 0);
}

RecompReturn Lufia2DecompBridge_8281E6(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x82u || cpu->D != 0u ||
        cpu->Y > 0x12u || (cpu->Y & 1u) ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8281e6u);
    }
    return ActorBridgeWholeM0X0(
        cpu, 0x8281e6u, Lufia2MenuDrawWindowTopBottomEdge, 2u);
}

RecompReturn Lufia2DecompBridge_82820B(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x82u || cpu->D != 0u ||
        cpu->Y > 0x12u || (cpu->Y & 1u) ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82820bu);
    }
    return ActorBridgeWholeM0X0(
        cpu, 0x82820bu, Lufia2MenuDrawWindowCorner, 2u);
}

RecompReturn Lufia2DecompBridge_828230(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x82u || cpu->D != 0u ||
        cpu->Y > 0x12u || (cpu->Y & 1u) ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828230u);
    }
    return ActorBridgeWholeM0X0(
        cpu, 0x828230u, Lufia2MenuDrawWindowSideEdge, 2u);
}

RecompReturn Lufia2DecompBridge_828259(CpuState *cpu) {
    if (cpu->PB != 0x82u || cpu->DB != 0x82u || cpu->D != 0u ||
        cpu->Y > 0x12u || (cpu->Y & 1u) ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x828259u);
    }
    return ActorBridgeWholeM0X0(
        cpu, 0x828259u, Lufia2MenuDrawWindowThreeRowCorner, 2u);
}

RecompReturn Lufia2DecompBridge_85821C(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85821cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85821cu, Lufia2FieldDiagnosticsEditValue, 2u);
}

RecompReturn Lufia2DecompBridge_858267(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858267u);
    }
    return ActorBridgeSceneOwners(cpu, 0x858267u, Lufia2FieldDiagnosticsHideCursor, 2u);
}

RecompReturn Lufia2DecompBridge_858274(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858274u);
    }
    return ActorBridgeSceneOwners(cpu, 0x858274u, Lufia2FieldDiagnosticsShowCursor, 2u);
}

RecompReturn Lufia2DecompBridge_858281(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858281u);
    }
    return ActorBridgeWhole(cpu, 0x858281u, Lufia2FieldDiagnosticsCursorAddress, 2u);
}

RecompReturn Lufia2DecompBridge_85829D(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85829du);
    }
    return ActorBridgeSceneOwners(cpu, 0x85829du, Lufia2FieldDiagnosticsDrawValues, 2u);
}

RecompReturn Lufia2DecompBridge_858312(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858312u);
    }
    return ActorBridgeWhole(cpu, 0x858312u, Lufia2FieldDiagnosticsResolveFlag, 2u);
}

RecompReturn Lufia2DecompBridge_8584A9(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8584a9u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8584a9u, Lufia2FieldDiagnosticsClearTilemap, 2u);
}

RecompReturn Lufia2DecompBridge_85850E(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85850eu);
    }
    return ActorBridgeWhole(cpu, 0x85850eu, Lufia2FieldDiagnosticsConsumeHighButtons, 2u);
}

RecompReturn Lufia2DecompBridge_858515(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->DB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858515u);
    }
    return ActorBridgeWhole(cpu, 0x858515u, Lufia2FieldDiagnosticsConsumeLowButtons, 2u);
}

RecompReturn Lufia2DecompBridge_83C729(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->D != 0u ||
        cpu->S < 0x1f40u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c729u);
    }
    return ActorBridgeSceneOwners(cpu, 0x83c729u, Lufia2FieldSpawnRadialObjects, 3u);
}

RecompReturn Lufia2DecompBridge_83E200(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83e200u);
    }
    return ActorBridgeWhole(cpu, 0x83e200u, Lufia2ObjectSelectSpriteFrame, 2u);
}

RecompReturn Lufia2DecompBridge_83DFA5(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83dfa5u);
    }
    return ActorBridgeWhole(cpu, 0x83dfa5u, Lufia2ActorInitializeSpawnSlot, 2u);
}

RecompReturn Lufia2DecompBridge_83C33D(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c33du);
    }
    return ActorBridgeWhole(cpu, 0x83c33du, Lufia2FieldFindSecondaryAtProbe, 2u);
}

RecompReturn Lufia2DecompBridge_83F410(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83f410u);
    }
    return ActorBridgeWhole(cpu, 0x83f410u, Lufia2FieldPendingObjectType, 2u);
}

RecompReturn Lufia2DecompBridge_83C7C7(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->D != 0u ||
        cpu->S < 0x1f40u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c7c7u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x83c7c7u, Lufia2ActorMarkAllState40, 2u);
}

RecompReturn Lufia2DecompBridge_8595C6(CpuState *cpu) {
    if (cpu->PB != 0x85u || (cpu->A & 0xffu) >= 78u || cpu->D != 0u ||
        cpu->S < 0x1f40u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8595c6u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8595c6u, Lufia2BattleLoadIndexedMessage, 3u);
}

RecompReturn Lufia2DecompBridge_8EBA81(CpuState *cpu) {
    if (cpu->PB != 0x8eu || (cpu->DB != 0x83u && cpu->DB != 0x8eu) || cpu->D != 0u ||
        cpu->S < 0x1f40u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eba81u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8eba81u, Lufia2FieldResetPresentationState, 3u);
}

RecompReturn Lufia2DecompBridge_83C6AA(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c6aau);
    }
    return ActorBridgeWhole(cpu, 0x83c6aau, Lufia2FieldApplyObjectParameters, 3u);
}

RecompReturn Lufia2DecompBridge_859884(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859884u);
    }
    return ActorBridgeWhole(cpu, 0x859884u, Lufia2BattleBuildSpriteGrid, 3u);
}

RecompReturn Lufia2DecompBridge_8EB5FB(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->DB != 0x8eu || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb5fbu);
    }
    return ActorBridgeWhole(cpu, 0x8eb5fbu, Lufia2FieldBuildPrefixedName, 3u);
}

RecompReturn Lufia2DecompBridge_83A591(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a591u);
    }
    return ActorBridgeWhole(cpu, 0x83a591u, Lufia2FieldWriteOamQuad, 2u);
}

RecompReturn Lufia2DecompBridge_83A5C7(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a5c7u);
    }
    return ActorBridgeWhole(cpu, 0x83a5c7u, Lufia2FieldWriteOamQuadFlipX, 2u);
}

RecompReturn Lufia2DecompBridge_83A5FD(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a5fdu);
    }
    return ActorBridgeWhole(cpu, 0x83a5fdu, Lufia2FieldWriteOamQuadFlipY, 2u);
}

RecompReturn Lufia2DecompBridge_83A633(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a633u);
    }
    return ActorBridgeWhole(cpu, 0x83a633u, Lufia2FieldWriteOamQuadFlipXY, 2u);
}

RecompReturn Lufia2DecompBridge_83A669(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a669u);
    }
    return ActorBridgeWhole(cpu, 0x83a669u, Lufia2FieldSetOamSizeBit, 2u);
}

RecompReturn Lufia2DecompBridge_83A4A2(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a4a2u);
    }
    return ActorBridgeWhole(cpu, 0x83a4a2u, Lufia2FieldWriteOamSingle, 2u);
}

RecompReturn Lufia2DecompBridge_83A4BF(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a4bfu);
    }
    return ActorBridgeWhole(cpu, 0x83a4bfu, Lufia2FieldWriteOamVerticalPair, 2u);
}

RecompReturn Lufia2DecompBridge_83A501(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a501u);
    }
    return ActorBridgeWhole(cpu, 0x83a501u, Lufia2FieldWriteOamHorizontalPair, 2u);
}

RecompReturn Lufia2DecompBridge_83A534(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->DB != 0x83u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83a534u);
    }
    return ActorBridgeWhole(cpu, 0x83a534u, Lufia2FieldWriteOamSquare, 2u);
}

RecompReturn Lufia2DecompBridge_83AEB5(CpuState *cpu) {
    if (!ActorBridgeSystemBank(cpu->DB) || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83aeb5u);
    }
    return ActorBridgeBattleEntry(
        cpu, 0x83aeb5u, 0, Lufia2FieldColourEffectsWithServices, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_848D54(CpuState *cpu) {
    if (cpu->PB != 0x84u || (cpu->DB != 0x83u && cpu->DB != 0x84u) ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x848d54u);
    }
    return ActorBridgeBattleEntry(cpu, 0x848d54u,
        0, Lufia2FieldScaleScenePalette, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_848E07(CpuState *cpu) {
    if (cpu->PB != 0x84u || (cpu->DB != 0x83u && cpu->DB != 0x84u) ||
        !cpu->m_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x848e07u);
    }
    return ActorBridgeBattleEntry(cpu, 0x848e07u,
        Lufia2FieldFadeScenePalette, 0, 3u,
        BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_869470(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869470u);
    }
    return ActorBridgeWhole(cpu, 0x869470u, Lufia2WorldMapLoadMotionRecords, 2u);
}

RecompReturn Lufia2DecompBridge_86973E(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86973eu);
    }
    return ActorBridgeWhole(cpu, 0x86973eu, Lufia2WorldMapDisableDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_86994C(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86994cu);
    }
    return ActorBridgeWhole(cpu, 0x86994cu, Lufia2WorldMapSumMotionTerms, 2u);
}

RecompReturn Lufia2DecompBridge_869A44(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869a44u);
    }
    return ActorBridgeWhole(cpu, 0x869a44u, Lufia2WorldMapRememberTilePosition, 2u);
}

RecompReturn Lufia2DecompBridge_869AB1(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869ab1u);
    }
    return ActorBridgeWhole(cpu, 0x869ab1u, Lufia2WorldMapCheckMovingArrival, 2u);
}

RecompReturn Lufia2DecompBridge_869AB8(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869ab8u);
    }
    return ActorBridgeWhole(cpu, 0x869ab8u, Lufia2WorldMapCheckStoppedArrival, 2u);
}

RecompReturn Lufia2DecompBridge_86D37D(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d37du);
    }
    return ActorBridgeWhole(cpu, 0x86d37du, Lufia2WorldMapStartFadeOut, 2u);
}

RecompReturn Lufia2DecompBridge_86D381(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d381u);
    }
    return ActorBridgeWhole(cpu, 0x86d381u, Lufia2WorldMapStartFadeIn, 2u);
}

RecompReturn Lufia2DecompBridge_869F35(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869f35u);
    }
    return ActorBridgeWhole(cpu, 0x869f35u, Lufia2WorldMapResolveResourcePointer, 2u);
}

RecompReturn Lufia2DecompBridge_86A28B(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a28bu);
    }
    return ActorBridgeWhole(cpu, 0x86a28bu, Lufia2WorldMapCenterTargetTiles, 2u);
}

RecompReturn Lufia2DecompBridge_86A2C6(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a2c6u);
    }
    return ActorBridgeWhole(cpu, 0x86a2c6u, Lufia2WorldMapReadSecondObjectTarget, 2u);
}

RecompReturn Lufia2DecompBridge_86A376(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a376u);
    }
    return ActorBridgeWhole(cpu, 0x86a376u, Lufia2WorldMapConfigurePrimaryView, 2u);
}

RecompReturn Lufia2DecompBridge_86A38C(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a38cu);
    }
    return ActorBridgeWhole(cpu, 0x86a38cu, Lufia2WorldMapConfigureSecondaryView, 2u);
}

RecompReturn Lufia2DecompBridge_86D3AE(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d3aeu);
    }
    return ActorBridgeWhole(cpu, 0x86d3aeu, Lufia2WorldMapEnableDisplay, 2u);
}

RecompReturn Lufia2DecompBridge_86992B(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86992bu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86992bu,
        0, Lufia2WorldMapUpdateObjectHeading, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A357(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a357u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a357u,
        0, Lufia2WorldMapPositionAnimatedObject, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A338(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a338u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a338u,
        0, Lufia2WorldMapAnimatePairedObjects, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86D32D(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d32du);
    }
    return ActorBridgeBattleEntry(cpu, 0x86d32du,
        0, Lufia2WorldMapResetDisplayState, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86CBBF(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86cbbfu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86cbbfu,
        0, Lufia2WorldMapClearDisplayPalette, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869A71(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869a71u);
    }
    return ActorBridgeBattleEntry(cpu, 0x869a71u,
        0, Lufia2WorldMapReadTileClass, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8EB142(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb142u);
    }
    return ActorBridgeWhole(cpu, 0x8eb142u, Lufia2FieldConsumePressedLow, 3u);
}

RecompReturn Lufia2DecompBridge_8EB149(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb149u);
    }
    return ActorBridgeWhole(cpu, 0x8eb149u, Lufia2FieldConsumePressedHigh, 3u);
}

RecompReturn Lufia2DecompBridge_8EB7CD(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb7cdu);
    }
    return ActorBridgeWhole(cpu, 0x8eb7cdu, Lufia2FieldReadBankedStreamByte, 2u);
}

RecompReturn Lufia2DecompBridge_8EBAE7(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebae7u);
    }
    return ActorBridgeWhole(cpu, 0x8ebae7u, Lufia2FieldMarkEncounterActor, 3u);
}

RecompReturn Lufia2DecompBridge_8EC1C5(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ec1c5u);
    }
    return ActorBridgeWholeM0X0(cpu, 0x8ec1c5u, Lufia2FieldCountValue2B, 3u);
}

RecompReturn Lufia2DecompBridge_8EC36D(CpuState *cpu) {
    if (cpu->PB != 0x8eu || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ec36du);
    }
    return ActorBridgeWhole(cpu, 0x8ec36du, Lufia2FieldReadMapResourceFlags, 2u);
}

RecompReturn Lufia2DecompBridge_8192BE(CpuState *cpu) {
    if (cpu->PB != 0x81u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8192beu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8192beu,
        0, Lufia2BattleEffectDispatchHdmaCommand, 2u,
        BATTLE_BRIDGE_M1X0, 0, 0x81, 0);
}

RecompReturn Lufia2DecompBridge_85ADA8(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ada8u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85ada8u,
        0, Lufia2BattleEffectBeginBackwardWave, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AE27(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ae27u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85ae27u,
        0, Lufia2BattleEffectEndBackwardWave, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AE2F(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ae2fu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85ae2fu,
        0, Lufia2BattleEffectBeginForwardWave, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AEB1(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85aeb1u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85aeb1u,
        0, Lufia2BattleEffectEndForwardWave, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AEB9(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85aeb9u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85aeb9u,
        0, Lufia2BattleEffectBeginWaveFill, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AF1D(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85af1du);
    }
    return ActorBridgeBattleEntry(cpu, 0x85af1du,
        0, Lufia2BattleEffectEndWaveFill, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AF25(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85af25u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85af25u,
        0, Lufia2BattleEffectBeginHdmaMode5, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AF9F(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85af9fu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85af9fu,
        0, Lufia2BattleEffectEndHdmaMode5, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AFA7(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85afa7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85afa7u,
        0, Lufia2BattleEffectBeginHdmaMode6, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B01E(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b01eu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b01eu,
        0, Lufia2BattleEffectEndHdmaMode6, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B026(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b026u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b026u,
        0, Lufia2BattleEffectBeginHdmaMode7, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B082(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b082u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b082u,
        0, Lufia2BattleEffectEndHdmaMode7, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B08A(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b08au);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b08au,
        0, Lufia2BattleEffectBeginHdmaMode8, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B0FC(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b0fcu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b0fcu,
        0, Lufia2BattleEffectEndHdmaMode8, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B100(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b100u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b100u,
        0, Lufia2BattleEffectBeginHdmaMode9, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B16E(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b16eu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b16eu,
        0, Lufia2BattleEffectEndHdmaMode9, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B172(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b172u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b172u,
        0, Lufia2BattleEffectBeginHdmaMode10, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85B1CE(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b1ceu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85b1ceu,
        0, Lufia2BattleEffectEndHdmaMode10, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AF5E(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85af5eu);
    }
    return ActorBridgeWhole(cpu, 0x85af5eu, Lufia2BattleHdmaBuildMode5, 2u);
}

RecompReturn Lufia2DecompBridge_85AFE0(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85afe0u);
    }
    return ActorBridgeWhole(cpu, 0x85afe0u, Lufia2BattleHdmaBuildMode6, 2u);
}

RecompReturn Lufia2DecompBridge_85B058(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b058u);
    }
    return ActorBridgeWhole(cpu, 0x85b058u, Lufia2BattleHdmaBuildMode7, 2u);
}

RecompReturn Lufia2DecompBridge_85B0C3(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b0c3u);
    }
    return ActorBridgeWhole(cpu, 0x85b0c3u, Lufia2BattleHdmaBuildMode8, 2u);
}

RecompReturn Lufia2DecompBridge_85B13B(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b13bu);
    }
    return ActorBridgeWhole(cpu, 0x85b13bu, Lufia2BattleHdmaBuildMode9, 2u);
}

RecompReturn Lufia2DecompBridge_85B1A4(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b1a4u);
    }
    return ActorBridgeWhole(cpu, 0x85b1a4u, Lufia2BattleHdmaBuildMode10, 2u);
}

RecompReturn Lufia2DecompBridge_85B0F4(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b0f4u);
    }
    return ActorBridgeWhole(cpu, 0x85b0f4u, Lufia2BattleHdmaStopMode8, 2u);
}

RecompReturn Lufia2DecompBridge_85B166(CpuState *cpu) {
    if (cpu->PB != 0x85u || !cpu->m_flag || cpu->x_flag ||
        cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85b166u);
    }
    return ActorBridgeWhole(cpu, 0x85b166u, Lufia2BattleHdmaStopMode9, 2u);
}

RecompReturn Lufia2DecompBridge_85AC16(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ac16u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85ac16u,
        0, Lufia2BattleHdmaBeginTransitionMode13, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85ACDF(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85acdfu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85acdfu,
        0, Lufia2BattleHdmaBeginTransitionMode14, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AC79(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ac79u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85ac79u,
        0, Lufia2BattleHdmaBuildTransitionMode13, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85AD42(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ad42u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85ad42u,
        0, Lufia2BattleHdmaBuildTransitionMode14, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85A804(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85a804u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85a804u,
        0, Lufia2BattleBeginPrimaryWave, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85A8E7(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85a8e7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x85a8e7u,
        0, Lufia2BattleBuildPrimaryWaveTable, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83F559(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83f559u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83f559u,
        0, Lufia2FieldRequestObjectAnimation, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EB63B(CpuState *cpu) {
    if (!cpu->m_flag || cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb63bu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8eb63bu,
        0, Lufia2FieldTryItemRegion, 3u,
        BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EB760(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8eb760u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8eb760u,
        0, Lufia2FieldShowObjectItemNotice, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EC05F(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ec05fu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ec05fu,
        0, Lufia2FieldTryObjectReward, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EBCAD(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebcadu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebcadu,
        0, Lufia2FieldChooseCollectedObjects, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EBD5E(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebd5eu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebd5eu,
        Lufia2FieldFindAvailableChoice, 0, 2u,
        BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_869E3B(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869e3bu);
    }
    return ActorBridgeBattleEntry(cpu, 0x869e3bu,
        0, Lufia2WorldMapRandomScaledWord, 3u,
        BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_859255(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859255u);
    }
    return ActorBridgeBattleEntry(cpu, 0x859255u,
        Lufia2BattleCheckPartyRecord, 0, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_859326(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859326u);
    }
    return ActorBridgeBattleEntry(cpu, 0x859326u,
        Lufia2BattleSetPartyCommandBytes, 0, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85942D(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85942du);
    }
    return ActorBridgeBattleEntry(cpu, 0x85942du,
        Lufia2BattleSwapFirstTwoMembers, 0, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_859445(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859445u);
    }
    return ActorBridgeBattleEntry(cpu, 0x859445u,
        Lufia2BattleSwapFirstThirdMembers, 0, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_859460(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859460u);
    }
    return ActorBridgeBattleEntry(cpu, 0x859460u,
        Lufia2BattleReversePartyMembers, 0, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8597D1(CpuState *cpu) {
    if (cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8597d1u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8597d1u,
        0, Lufia2BattlePrepareSpecialPartyGrid, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8592CE(CpuState *cpu) {
    if (cpu->x_flag || cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8592ceu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8592ceu,
        0, Lufia2BattlePublishPartyAction, 3u,
        BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8592FF(CpuState *cpu) {
    if (cpu->x_flag || cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8592ffu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8592ffu,
        0, Lufia2BattleQueueStagedPartyAction, 3u,
        BATTLE_BRIDGE_ANY_WIDTH, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C161(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c161u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c161u,
        Lufia2FieldReadPlayerDirection, 0, 2u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C180(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c180u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c180u,
        Lufia2FieldClearSecondaryActionState, 0, 2u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C652(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c652u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c652u,
        Lufia2PartyListHasEntry, 0, 3u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C692(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c692u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c692u,
        Lufia2FieldReadSelectedItemByte, 0, 2u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C2FE(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c2feu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c2feu,
        0, Lufia2FieldSpawnActionActor, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C311(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c311u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c311u,
        0, Lufia2FieldSpawnFacingActionActor, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C32D(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c32du);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c32du,
        0, Lufia2FieldReadLeaderFacingAction, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D3A1(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d3a1u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d3a1u,
        0, Lufia2ActorStartSecondaryActionScript, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D3F7(CpuState *cpu) {
    if (!cpu->m_flag || cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d3f7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d3f7u,
        Lufia2ActorPrepareSecondaryScript, 0, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BE64(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83be64u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83be64u,
        0, Lufia2FieldChangeLeaderFacing, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BF33(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83bf33u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83bf33u,
        0, Lufia2FieldStepLeaderFacing, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C5FE(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c5feu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c5feu,
        0, Lufia2FieldSelectItemAnimation, 2u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C24F(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c24fu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c24fu,
        0, Lufia2FieldDispatchSelectedItemAction, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C3DC(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c3dcu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c3dcu,
        0, Lufia2FieldProbeFacingObject, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C40E(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c40eu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c40eu,
        0, Lufia2FieldCompareFacingTileHeight, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83FC1C(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83fc1cu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83fc1cu,
        0, Lufia2FieldSpawnLeaderSecondaryAction, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BE3B(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83be3bu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83be3bu,
        0, Lufia2FieldProbeDownRightHeight, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BE3F(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83be3fu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83be3fu,
        0, Lufia2FieldProbeDownLeftHeight, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BE43(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83be43u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83be43u,
        0, Lufia2FieldProbeDownThreeHeight, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D8AB(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d8abu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d8abu,
        0, Lufia2FieldProbeDownCollision, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D8C7(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d8c7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d8c7u,
        0, Lufia2FieldProbeLeftCollision, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D8DE(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d8deu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d8deu,
        0, Lufia2FieldProbeUpCollision, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D913(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d913u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d913u,
        0, Lufia2FieldProbeRightCollision, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BDFC(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83bdfcu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83bdfcu,
        0, Lufia2FieldTryLeaderLowerStep, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D87D(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83d87du);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d87du,
        0, Lufia2FieldProbeActorStepCollision, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EBBAF(CpuState *cpu) {
    if (cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8ebbafu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebbafu,
        0, Lufia2FieldTryVehicleTransition, 3u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C446(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c446u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c446u,
        0, Lufia2FieldSpawnHeaderActor, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_838674(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag ||
            cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x838674u);
    }
    return ActorBridgeBattleEntry(cpu, 0x838674u,
        Lufia2FieldTakeHighButtons, 0, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83C246(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag ||
            cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c246u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c246u,
        Lufia2FieldReadFacingStepCode, 0, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_80AD57(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ad57u);
    }
    return ActorBridgeBattleEntry(cpu, 0x80ad57u,
        0, Lufia2FieldFaceActorTowardActor, 2u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_80AD39(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80ad39u);
    }
    return ActorBridgeBattleEntry(cpu, 0x80ad39u,
        0, Lufia2FieldFaceActorGroupTowardActor, 3u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_80C1DF(CpuState *cpu) {
    if (cpu->PB != 0x80u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c1dfu);
    }
    return ActorBridgeBattleEntry(cpu, 0x80c1dfu,
        0, Lufia2FieldSelectInteractionVariant, 3u,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BB08(CpuState *cpu) {
    if (cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83bb08u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83bb08u,
        0, Lufia2FieldTryActorInteraction, 2u,
        BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_C1B4(CpuState *cpu) {
    if (cpu->PB != 0x83u || !cpu->m_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x83c1b4u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83c1b4u,
        0, Lufia2PlayerSlotStandardUpdateWithServices, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

static RecompReturn ActorBridgeSoundStartup(CpuState *cpu, uint32_t entry,
    ActorSoundDriverFunction run) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {
            cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeSoundDriver(cpu, entry, run, 3u);
}
RecompReturn Lufia2DecompBridge_809383(CpuState *cpu) {
    return ActorBridgeSoundStartup(cpu, 0x809383u, Lufia2InitializeBootSound);
}
RecompReturn Lufia2DecompBridge_8093D6(CpuState *cpu) {
    return ActorBridgeSoundStartup(cpu, 0x8093d6u, Lufia2ResetSoundResources);
}

static RecompReturn ActorBridgeFieldInteraction(
    CpuState *cpu, uint32_t entry, BattleControlFunction run, unsigned frame_size) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != (entry >> 16) || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeBattleEntry(cpu, entry, 0, run, frame_size,
        BATTLE_BRIDGE_M1X0, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83BE90(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83be90u, Lufia2FieldHandleLeaderAction, 2u);
}

RecompReturn Lufia2DecompBridge_80E817(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x80e817u, Lufia2FieldStartFacingPointEvent, 3u);
}

RecompReturn Lufia2DecompBridge_83BFB4(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83bfb4u, Lufia2FieldClaimFacingActor, 2u);
}

RecompReturn Lufia2DecompBridge_83C375(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83c375u, Lufia2FieldHandleFacingCellAction, 2u);
}

RecompReturn Lufia2DecompBridge_83BC28(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83bc28u, Lufia2FieldCoordinatePlayerAction, 2u);
}

RecompReturn Lufia2DecompBridge_83BCC8(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83bcc8u, Lufia2FieldHandlePlayerMovement, 2u);
}

RecompReturn Lufia2DecompBridge_83BF3E(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83bf3eu, Lufia2FieldReleaseClaimedActor, 2u);
}

RecompReturn Lufia2DecompBridge_83C460(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83c460u, Lufia2FieldOpenItemSelection, 2u);
}

RecompReturn Lufia2DecompBridge_83C01C(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x83c01cu, Lufia2FieldBeginObjectPush, 2u);
}

RecompReturn Lufia2DecompBridge_84836A(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x84836au, Lufia2FieldUpdateItemSelection, 3u);
}

RecompReturn Lufia2DecompBridge_84863E(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x84863eu, Lufia2FieldRotateSelectionMarker, 2u);
}

RecompReturn Lufia2DecompBridge_848685(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x848685u, Lufia2FieldSetSelectionMosaic, 2u);
}

RecompReturn Lufia2DecompBridge_84868C(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x84868cu, Lufia2FieldSetSelectionBlink, 2u);
}

RecompReturn Lufia2DecompBridge_84869E(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x84869eu, Lufia2FieldResetSelectionBlink, 2u);
}

RecompReturn Lufia2DecompBridge_8486C0(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x8486c0u, Lufia2FieldReadSelectedItem, 2u);
}

RecompReturn Lufia2DecompBridge_8486D7(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x8486d7u, Lufia2FieldDrawSelectedItemName, 2u);
}

RecompReturn Lufia2DecompBridge_848758(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x848758u, Lufia2FieldTakeSelectionHighButtons, 2u);
}

RecompReturn Lufia2DecompBridge_84875F(CpuState *cpu) {
    return ActorBridgeFieldInteraction(cpu, 0x84875fu, Lufia2FieldTakeSelectionLowButtons, 2u);
}

RecompReturn Lufia2DecompBridge_86D35D(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d35du);
    }
    return ActorBridgeBattleEntry(cpu, 0x86d35du,
        0, Lufia2WorldMapSetPlayerSong, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A1C5(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a1c5u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a1c5u,
        0, Lufia2WorldMapActivateVehicle, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A1EC(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a1ecu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a1ecu,
        0, Lufia2WorldMapRestorePlayerObject, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8698C6(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8698c6u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8698c6u,
        0, Lufia2WorldMapMovePlayerNorth, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8698CD(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8698cdu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8698cdu,
        0, Lufia2WorldMapMovePlayerWest, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8698D4(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8698d4u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8698d4u,
        0, Lufia2WorldMapMovePlayerSouth, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8698DB(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8698dbu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8698dbu,
        0, Lufia2WorldMapMovePlayerEast, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869F13(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869f13u);
    }
    return ActorBridgeBattleEntry(cpu, 0x869f13u,
        0, Lufia2WorldMapFindRegionPosition, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A2AE(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86a2aeu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a2aeu,
        0, Lufia2WorldMapConsumeRegionPosition, 2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_848000(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x84u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x848000u);
    }
    return ActorBridgeBattleEntry(cpu, 0x848000u,
        0, Lufia2FieldUpdateScreenEffects, 3u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8480C0(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x84u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8480c0u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8480c0u,
        0, Lufia2FieldLowerFixedColor, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8480D3(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x84u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8480d3u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8480d3u,
        0, Lufia2FieldRaiseFixedColor, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8480E6(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x84u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8480e6u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8480e6u,
        0, Lufia2FieldRaiseBrightness, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_848115(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x84u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x848115u);
    }
    return ActorBridgeBattleEntry(cpu, 0x848115u,
        0, Lufia2FieldLowerBrightness, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_848145(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x84u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x848145u);
    }
    return ActorBridgeBattleEntry(cpu, 0x848145u,
        0, Lufia2FieldUpdateScreenShake, 2u,
        BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83CA93(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83ca93u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83ca93u,
        Lufia2ActorChooseTargetDirection, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83CD6E(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83cd6eu);
    }
    return ActorBridgeBattleEntry(cpu, 0x83cd6eu,
        Lufia2ActorCompareLeaderTileY, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83CD77(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83cd77u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83cd77u,
        Lufia2ActorCompareActorTileX, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83CD80(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83cd80u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83cd80u,
        Lufia2ActorCompareActorTileY, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83CD89(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83cd89u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83cd89u,
        Lufia2ActorCompareLeaderTileX, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D3B7(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83d3b7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d3b7u,
        Lufia2ActorCheckNorthActionLimit, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D3C5(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83d3c5u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d3c5u,
        Lufia2ActorCheckSouthActionLimit, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D3D7(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83d3d7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d3d7u,
        Lufia2ActorCheckWestActionLimit, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_83D3E5(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->PB != 0x83u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x83d3e5u);
    }
    return ActorBridgeBattleEntry(cpu, 0x83d3e5u,
        Lufia2ActorCheckEastActionLimit, 0, 2u, BATTLE_BRIDGE_ANY_WIDTH,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EBE78(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebe78u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebe78u, Lufia2FieldFollowLayerScroll, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8EBF24(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebf24u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebf24u, Lufia2FieldRetainLayerScroll, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8EBF25(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebf25u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebf25u, 0, Lufia2FieldParallaxLayerScroll, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8EBF70(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebf70u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebf70u, Lufia2FieldShiftScrollRight, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8EBF88(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebf88u);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebf88u, Lufia2FieldReadScrollShiftCount, 0,
                                  2u, BATTLE_BRIDGE_ANY_WIDTH, 1,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EBF9B(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebf9bu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebf9bu, 0, Lufia2FieldScaleLayerScroll, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8EBFDE(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebfdeu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebfdeu, Lufia2FieldShiftScrollLeft, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8EBFEE(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8ebfeeu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8ebfeeu, Lufia2FieldWrapLayerScroll, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_85903F(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x85903fu);
    }
    return ActorBridgeBattleEntry(cpu, 0x85903fu, 0, Lufia2BattleShowStatusUpdates, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_859114(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x859114u);
    }
    return ActorBridgeBattleEntry(cpu, 0x859114u, 0, Lufia2BattleShowStatusMessage, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_8590BC(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8590bcu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8590bcu, 0, Lufia2BattleTickStatusMessage, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK,
                                  0);
}

RecompReturn Lufia2DecompBridge_869BCA(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x869bcau);
    }
    return ActorBridgeBattleEntry(cpu, 0x869bcau, 0, Lufia2WorldMapApplyEntryPosition, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869F55(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x869f55u);
    }
    return ActorBridgeBattleEntry(cpu, 0x869f55u, 0, Lufia2WorldMapRunScene03, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869F5C(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x869f5cu);
    }
    return ActorBridgeBattleEntry(cpu, 0x869f5cu, 0, Lufia2WorldMapRunScene06, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8696BC(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x8696bcu);
    }
    return ActorBridgeBattleEntry(cpu, 0x8696bcu, 0, Lufia2WorldMapHandleSceneInput, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A473(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a473u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a473u, Lufia2WorldMapPolarQuadrant0, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_86A496(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a496u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a496u, Lufia2WorldMapPolarQuadrant1, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_86A4AD(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a4adu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a4adu, Lufia2WorldMapPolarQuadrant2, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_86A4BC(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a4bcu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a4bcu, Lufia2WorldMapPolarQuadrant3, 0, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_86A0B9(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a0b9u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a0b9u, 0, Lufia2WorldMapRunVehicleScene03, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A0C6(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a0c6u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a0c6u, 0, Lufia2WorldMapRunVehicleScene07, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86D397(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86d397u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86d397u, 0, Lufia2WorldMapWaitFade, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A3A2(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a3a2u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a3a2u, 0, Lufia2WorldMapSelectSecondaryView, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A3AB(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a3abu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a3abu, 0, Lufia2WorldMapRunScene05, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A3E4(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a3e4u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a3e4u, 0, Lufia2WorldMapCallScene03, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A3E8(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a3e8u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a3e8u, 0, Lufia2WorldMapRunTileScene, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86A2E7(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86a2e7u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86a2e7u, 0, Lufia2WorldMapRunPairedObjectInput, 2u,
                                  BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869E4F(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call;
    Lufia2CpuState state;
    Lufia2ExecutionResult result;

    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, 0x869e4fu);
    frame = ActorBridgeEnter(cpu);
    call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    ActorBridgeLoad(cpu, &state);
    result = Lufia2WorldMapApplyRegionRecord(&memory, &state,
        ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    if (result.pc == 0x869ed8u)
        return ActorBridgeReturnPastFrame(cpu, &frame, 3u, result.pc);
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_86CC3C(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86cc3cu);
    }
    return ActorBridgeBattleEntry(cpu, 0x86cc3cu, NULL,
        Lufia2WorldMapBuildScene, 2u, BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869766(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x869766u);
    }
    return ActorBridgeBattleEntry(cpu, 0x869766u, NULL,
        Lufia2WorldMapReadPlayerInput, 2u, BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869758(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x869758u);
    }
    return ActorBridgeBattleEntry(cpu, 0x869758u, NULL,
        Lufia2WorldMapMaybeReadPlayerInput, 2u, BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86D34A(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x86d34au);
    }
    return ActorBridgeBattleEntry(cpu, 0x86d34au, NULL,
        Lufia2WorldMapRestoreEncounterScene, 2u, BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869DD6(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x869dd6u);
    }
    return ActorBridgeBattleEntry(cpu, 0x869dd6u, NULL,
        Lufia2WorldMapAnimateEncounterTransition, 2u, BATTLE_BRIDGE_ANY_WIDTH, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869C71(CpuState *cpu) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D != 0u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, 0x869c71u);

    frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result = Lufia2WorldMapCheckEncounter(&memory, &state,
        ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 2u, result.pc);
}

RecompReturn Lufia2DecompBridge_869B91(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x869b91u);
    }
    return ActorBridgeBattleEntry(cpu,0x869b91u,NULL,
        Lufia2WorldMapAnimateMenuReturn,2u,BATTLE_BRIDGE_ANY_WIDTH,0,0x86,0);
}

RecompReturn Lufia2DecompBridge_869BBE(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x869bbeu);
    }
    return ActorBridgeBattleEntry(cpu,0x869bbeu,NULL,
        Lufia2WorldMapClearPlayerScenePosition,2u,BATTLE_BRIDGE_ANY_WIDTH,0,0x86,0);
}

RecompReturn Lufia2DecompBridge_869B30(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x869b30u);
    }
    return ActorBridgeBattleEntry(cpu,0x869b30u,NULL,
        Lufia2WorldMapApplyMenuReturnTransition,2u,BATTLE_BRIDGE_ANY_WIDTH,0,0x86,0);
}

RecompReturn Lufia2DecompBridge_869ADB(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x869adbu);
    }
    return ActorBridgeBattleEntry(cpu,0x869adbu,NULL,
        Lufia2WorldMapMaybeOpenMenu,2u,BATTLE_BRIDGE_ANY_WIDTH,0,0x86,0);
}

RecompReturn Lufia2DecompBridge_86919E(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x86919eu);
    }
    return ActorBridgeBattleEntry(cpu,0x86919eu,NULL,
        Lufia2WorldMapRunSession,3u,BATTLE_BRIDGE_ANY_WIDTH,0,-1,0);
}

RecompReturn Lufia2DecompBridge_8384E1(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u ||
        cpu->DB != 0x83u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x8384e1u);
    }
    return ActorBridgeBattleEntry(cpu,0x8384e1u,NULL,
        Lufia2FieldRunMapTransition,2u,BATTLE_BRIDGE_ANY_WIDTH,0,0x83,0);
}

RecompReturn Lufia2DecompBridge_8EB28F(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->PB != 0x8eu ||
        cpu->DB != 0x83u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x8eb28fu);
    }
    return ActorBridgeBattleEntry(cpu,0x8eb28fu,NULL,
        Lufia2FieldSwitchDisplayScene,3u,BATTLE_BRIDGE_ANY_WIDTH,0,0x83,0);
}

RecompReturn Lufia2DecompBridge_838000(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1fffu)
        return ActorBridgeFallback(cpu,&frame,0x838000u);
    const Lufia2Memory memory = {
        ActorBridgeRead,ActorBridgeWrite,cpu,ActorBridgeExecutionCheckpoint,cpu};
    ActorPushedCall call = {cpu,frame,RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu,&state);
    const Lufia2ExecutionResult result = Lufia2FieldRunLoop(
        &memory,&state,ActorBridgeSceneChild,&call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound-1);
    ActorBridgeStore(cpu,&state);
    cpu->PB = state.program_bank;
    return interp_tier_dispatch_tail(
        cpu,result.pc,result.pc,frame.entry_s,frame.hrv);
}

RecompReturn Lufia2DecompBridge_8EC1DE(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x8eu || cpu->DB != 0x83u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
        return ActorBridgeFallback(cpu,&frame,0x8ec1deu);
    }
    return ActorBridgeBattleEntry(cpu,0x8ec1deu,NULL,
        Lufia2FieldApplyObjectReward,3u,BATTLE_BRIDGE_M1X0,0,0x83,0);
}

RecompReturn Lufia2DecompBridge_83FAFA(CpuState*cpu){
 const ActorBridgeFrame frame={cpu->S,cpu->host_return_valid,0xffffffffu};
 if(!ActorBridgeSupported(cpu,0,0)||cpu->PB!=0x83u||cpu->S<0x1f10u||cpu->S>0x1ffcu)
  return ActorBridgeFallback(cpu,&frame,0x83fafau);
 return ActorBridgeWholeM1X16(cpu,0x83fafau,Lufia2ActorExtendSignedByte,2u);
}

RecompReturn Lufia2DecompBridge_859419(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (!ActorBridgeSupported(cpu,0,0) || cpu->PB != 0x85u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        (cpu->DB > 0x3fu && (cpu->DB < 0x80u || cpu->DB > 0xbfu)))
        return ActorBridgeFallback(cpu,&frame,0x859419u);
    return ActorBridgeWholeM1X16(cpu,0x859419u,Lufia2BattleReversePartyOrder,3u);
}

RecompReturn Lufia2DecompBridge_828028(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->x_flag || cpu->PB != 0x82u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu,&frame,0x828028u);
    return ActorBridgeWholeX16(cpu,0x828028u,Lufia2MenuDispatchInlineTable,2u);
}

RecompReturn Lufia2DecompBridge_82E8CB(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->x_flag || cpu->PB != 0x82u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu,&frame,0x82e8cbu);
    return ActorBridgeWholeX16(cpu,0x82e8cbu,Lufia2MenuDispatchSaveMode,2u);
}

RecompReturn Lufia2DecompBridge_85EAEF(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (!ActorBridgeSupported(cpu,0,0) || cpu->PB != 0x85u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu,&frame,0x85eaefu);
    return ActorBridgeBattleEntry(cpu,0x85eaefu,NULL,
        Lufia2BattleFadeSpecialExit,3u,BATTLE_BRIDGE_M1X0,0,-1,0);
}

RecompReturn Lufia2DecompBridge_82A4BE(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->x_flag || cpu->PB != 0x82u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu,&frame,0x82a4beu);
    return ActorBridgeWholeX16(cpu,0x82a4beu,Lufia2MenuDispatchMainSelection,2u);
}

RecompReturn Lufia2DecompBridge_808117(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu)
        return ActorBridgeFallback(cpu,&frame,0x808117u);
    return ActorBridgeWholeAnyWidth(cpu,0x808117u,Lufia2SystemInitializeVideoMemory,2u);
}

RecompReturn Lufia2DecompBridge_8080E1(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu)
        return ActorBridgeFallback(cpu,&frame,0x8080e1u);
    return ActorBridgeWholeAnyWidth(cpu,0x8080e1u,Lufia2InitializeRuntimeDefaults,2u);
}

RecompReturn Lufia2DecompBridge_8091DF(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x80u || cpu->DB != 0x80u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu)
        return ActorBridgeFallback(cpu,&frame,0x8091dfu);
    return ActorBridgeSceneOwners(cpu,0x8091dfu,Lufia2InitializeStartupImage,2u);
}

RecompReturn Lufia2DecompBridge_849E1C(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x84u || cpu->DB != 0x7eu || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu)
        return ActorBridgeFallback(cpu,&frame,0x849e1cu);
    return ActorBridgeWholeM0X0(cpu,0x849e1cu,Lufia2FillWordBlock32,2u);
}

RecompReturn Lufia2DecompBridge_85DF44(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x85u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu,&frame,0x85df44u);
    return ActorBridgeSceneOwners(cpu,0x85df44u,Lufia2BattleBuildEnemyGraphicsGroups,3u);
}

RecompReturn Lufia2DecompBridge_858A96(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank=cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x85u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&frame,0x858a96u);
    return ActorBridgeSceneOwners(cpu,0x858a96u,Lufia2BattleRenderPartyPass,3u);
}

RecompReturn Lufia2DecompBridge_849D2C(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank=cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x84u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&frame,0x849d2cu);
    return ActorBridgeSceneOwners(cpu,0x849d2cu,Lufia2BattleBuildEnemyPreview,3u);
}

RecompReturn Lufia2DecompBridge_849CD4(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank=cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x84u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&frame,0x849cd4u);
    return ActorBridgeSceneOwners(cpu,0x849cd4u,Lufia2BattleUpdatePreviewBackground,3u);
}

RecompReturn Lufia2DecompBridge_849B3E(CpuState *cpu) {
    const ActorBridgeFrame fallback_frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank=cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x84u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&fallback_frame,0x849b3eu);
    if (bank == 0x97u)
        return ActorBridgeBattleSessionCalls(cpu, Lufia2BattleRunSpritePreviewWithExit);
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead,ActorBridgeWrite,cpu,
        ActorBridgeExecutionCheckpoint,cpu};
    ActorPushedCall call = {cpu,frame,RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu,&state);
    const Lufia2ExecutionResult result = Lufia2BattleRunSpritePreview(
        &memory,&state,ActorBridgePushedChild,&call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu,&state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu,result.pc,result.pc,frame.entry_s,frame.hrv);
    return ActorBridgeReturn(cpu,&frame,3u,result.pc);
}

RecompReturn Lufia2DecompBridge_80911C(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank=cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&frame,0x80911cu);
    return ActorBridgeSceneOwners(cpu,0x80911cu,Lufia2ClearGameFile,2u);
}

RecompReturn Lufia2DecompBridge_8090E5(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank = cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&frame,0x8090e5u);
    return ActorBridgeSceneOwners(cpu,0x8090e5u,Lufia2ValidateSaveFileChecksum,2u);
}

RecompReturn Lufia2DecompBridge_809037(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S,cpu->host_return_valid,0xffffffffu};
    const uint8_t bank = cpu->DB;
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || !cpu->x_flag ||
        cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !(bank < 0x40u || (bank >= 0x80u && bank < 0xc0u)))
        return ActorBridgeFallback(cpu,&frame,0x809037u);
    return ActorBridgeSceneOwners(cpu,0x809037u,Lufia2ValidateSaveFiles,3u);
}

RecompReturn Lufia2DecompBridge_8080D3(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->D || !cpu->m_flag ||
        !cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffdu)
        return ActorBridgeFallback(cpu, &frame, 0x8080d3u);
    return ActorBridgeWholeAnyWidth(cpu, 0x8080d3u, Lufia2ClearStartupWorkRam, 2u);
}

RecompReturn Lufia2DecompBridge_80FBCE(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->D || !cpu->m_flag ||
        cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, 0x80fbceu);
    return ActorBridgeSceneOwners(cpu, 0x80fbceu,
                                  Lufia2FieldClearCellFlagsAndPublish, 3u);
}
RecompReturn Lufia2DecompBridge_838E4C(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u || cpu->D || !cpu->m_flag ||
        cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return ActorBridgeFallback(cpu, &frame, 0x838e4cu);
    return ActorBridgeSceneOwners(cpu, 0x838e4cu, Lufia2FieldPublishRedrawnLayers,
                                  3u);
}
RecompReturn Lufia2DecompBridge_86D378(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x86u || cpu->D || !cpu->m_flag ||
        cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffdu)
        return ActorBridgeFallback(cpu, &frame, 0x86d378u);
    return ActorBridgeSceneOwners(cpu, 0x86d378u, Lufia2WorldMapSendSoundCommand,
                                  2u);
}

static uint8_t ActorBridgeOwnedChild(
    void *context, Lufia2CpuState *state, uint32_t target,
    uint32_t site, uint8_t frame_size) {
    ActorPushedCall *call = (ActorPushedCall *)context;
    ActorBridgeStore(call->cpu, state);
    call->cpu->PB = (uint8_t)(target >> 16);
    if (!cpu_dispatch_has_entry(call->cpu, target)) {
        const RecompReturn result = interp_tier_dispatch_tail(
            call->cpu, target, site, call->frame.entry_s, call->frame.hrv);
        call->unwound = (RecompReturn)((int)result + 1);
        return 0u;
    }
    return ActorBridgePushedChild(context, state, target, site, frame_size);
}

static RecompReturn ActorBridgeOwnedCalls(
    CpuState *cpu, ActorSceneOwnersFunction run, uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgeOwnedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_858045(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x85u || cpu->D ||
        !cpu->m_flag || !cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x858045u);
    }
    return ActorBridgeOwnedCalls(cpu, Lufia2FieldDiagnosticsRunMenu, 3u);
}

RecompReturn Lufia2DecompBridge_858415(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x85u || cpu->D ||
        !cpu->m_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x858415u);
    }
    return ActorBridgeOwnedCalls(cpu, Lufia2FieldDiagnosticsShowMessage, 3u);
}

RecompReturn Lufia2DecompBridge_858322(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x85u || cpu->D ||
        !cpu->m_flag || cpu->DB != 0x85u || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x858322u);
    }
    return ActorBridgeOwnedCalls(cpu, Lufia2FieldDiagnosticsSaveMenu, 2u);
}

static uint8_t ActorBridgeFieldServicesAvailable(
    const CpuState *cpu, uint32_t entry, uint8_t frame_size,
    uint32_t child, uint8_t child_index_8_bit) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->D ||
        cpu->PB != (uint8_t)(entry >> 16) || cpu->S < 0x1f20u ||
        cpu->S > (uint16_t)(0x1fffu - frame_size))
        return 0u;
    CpuState profile = *cpu;
    profile.m_flag = 1u;
    profile.x_flag = child_index_8_bit;
    profile.PB = (uint8_t)(child >> 16);
    cpu_mirrors_to_p(&profile);
    return (uint8_t)cpu_dispatch_has_entry(&profile, child);
}

RecompReturn Lufia2DecompBridge_CBAE(CpuState *cpu) {
    if (ActorBridgeFieldServicesAvailable(cpu, 0x80cbaeu, 3u, 0x80cc26u, 0u))
        return ActorBridgeOwnedCalls(cpu, Lufia2FieldEventTimerTickWithServices, 3u);
    return ActorBridgeWhole(cpu, 0x80cbaeu, Lufia2FieldEventTimerTick, 3u);
}

RecompReturn Lufia2DecompBridge_81C6(CpuState *cpu) {
    if (ActorBridgeFieldServicesAvailable(cpu, 0x8381c6u, 2u, 0x80cbaeu, 0u))
        return ActorBridgeOwnedCalls(cpu, Lufia2FieldTriggerUpdateWithServices, 2u);
    return ActorBridgeWhole(cpu, 0x8381c6u, Lufia2FieldTriggerUpdate, 2u);
}

RecompReturn Lufia2DecompBridge_83A0(CpuState *cpu) {
    if (cpu->x_flag &&
        ActorBridgeFieldServicesAvailable(cpu, 0x8383a0u, 2u, 0x83afeau, 1u))
        return ActorBridgeOwnedCalls(cpu, Lufia2FieldMenuRequestWithServices, 2u);
    return ActorBridgeWhole(cpu, 0x8383a0u, Lufia2FieldMenuRequest, 2u);
}

RecompReturn Lufia2DecompBridge_9C72(CpuState *cpu) {
    if (ActorBridgeFieldServicesAvailable(cpu, 0x809c72u, 3u, 0x848000u, cpu->x_flag))
        return ActorBridgeOwnedCalls(cpu, Lufia2FieldEventTickWithServices, 3u);
    return ActorBridgeWhole(cpu, 0x809c72u, Lufia2FieldEventTick, 3u);
}

RecompReturn Lufia2DecompBridge_80C21A(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->PB != 0x80u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x80c21au);
    }
    return ActorBridgeOwnedCalls(cpu, Lufia2FieldTextWindowTimer, 2u);
}

static RecompReturn ActorBridgeBattleSessionCalls(CpuState *cpu,
                                                  ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
                                 ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state = {0};
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        run(&memory, &state, ActorBridgeOwnedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s,
                                         frame.hrv);
    return ActorBridgeReturnPastFrame(cpu, &frame, 3u, result.pc);
}

RecompReturn Lufia2DecompBridge_868637(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x868637u);
    }
    return ActorBridgeWhole(cpu, 0x868637u, Lufia2TitleFindActiveSprite, 2u);
}

RecompReturn Lufia2DecompBridge_868655(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x868655u);
    }
    return ActorBridgeWhole(cpu, 0x868655u, Lufia2TitleCopyRecord, 2u);
}

RecompReturn Lufia2DecompBridge_868740(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x868740u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x868740u, Lufia2TitleCopyTileRows, 2u);
}

RecompReturn Lufia2DecompBridge_868513(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868513u);
    }
    return ActorBridgeWhole(cpu, 0x868513u, Lufia2TitleSelectObjectSprite, 2u);
}

RecompReturn Lufia2DecompBridge_868530(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868530u);
    }
    return ActorBridgeWhole(cpu, 0x868530u, Lufia2TitleSetObjectSpeed, 2u);
}

RecompReturn Lufia2DecompBridge_86866A(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86866au);
    }
    return ActorBridgeSceneOwners(cpu, 0x86866au, Lufia2TitleActivateObject, 2u);
}

RecompReturn Lufia2DecompBridge_868503(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868503u);
    }
    return ActorBridgeWholeM0X0(cpu, 0x868503u, Lufia2TitleApplyProductSign, 2u);
}

RecompReturn Lufia2DecompBridge_8685C2(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8685c2u);
    }
    return ActorBridgeWhole(cpu, 0x8685c2u, Lufia2TitleSpawnObjectParticle, 2u);
}

RecompReturn Lufia2DecompBridge_808450(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808450u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x808450u, Lufia2SineAngle, 3u);
}

RecompReturn Lufia2DecompBridge_808486(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x808486u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x808486u, Lufia2CosineAngle, 3u);
}


RecompReturn Lufia2DecompBridge_8683F2(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8683f2u);
    }
    return ActorBridgeWhole(cpu, 0x8683f2u, Lufia2TitleProjectObject, 2u);
}


RecompReturn Lufia2DecompBridge_86856F(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->D || cpu->S < 0x1f20u || cpu->S > 0x1ffdu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86856fu);
    }
    return ActorBridgeWhole(cpu, 0x86856fu, Lufia2TitleAnimateObjectParticles, 2u);
}

RecompReturn Lufia2DecompBridge_81F629(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x81f629u, Lufia2PartyClearStatuses, 3u);
}

RecompReturn Lufia2DecompBridge_82FD15(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fd15u, Lufia2PartyClearEquipment, 3u);
}

RecompReturn Lufia2DecompBridge_82FA57(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fa57u, Lufia2ItemRemoveRequested, 3u);
}

RecompReturn Lufia2DecompBridge_82FA77(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fa77u, Lufia2ItemClearRequestedPossession, 2u);
}

RecompReturn Lufia2DecompBridge_82FAD0(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fad0u, Lufia2ItemSubtractSlotQuantity, 2u);
}

RecompReturn Lufia2DecompBridge_82FCED(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fcedu, Lufia2ItemSelectPackedCount, 2u);
}

RecompReturn Lufia2DecompBridge_82FC97(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fc97u, Lufia2ItemIncrementPackedCount, 2u);
}

RecompReturn Lufia2DecompBridge_82FCC7(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fcc7u, Lufia2ItemDecrementPackedCount, 2u);
}

RecompReturn Lufia2DecompBridge_83C110(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x83c110u, Lufia2ActorPrepareAuxiliarySlot, 3u);
}

RecompReturn Lufia2DecompBridge_83F6C6(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x83f6c6u, Lufia2FieldSetupObjectActorSpriteFar, 3u);
}

RecompReturn Lufia2DecompBridge_80BF6F(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80bf6fu, Lufia2FieldFindWindowActor, 3u);
}

RecompReturn Lufia2DecompBridge_8EBAF6(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8ebaf6u, 0, Lufia2FieldRevealActor, 3u,
                                  BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EBB17(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x8ebb17u, Lufia2FieldHideActor, 3u);
}

RecompReturn Lufia2DecompBridge_80BF12(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80bf12u, Lufia2GoldAdd, 2u);
}

RecompReturn Lufia2DecompBridge_80BF43(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x80bf43u, Lufia2GoldSpend, 2u);
}

RecompReturn Lufia2DecompBridge_80A350(CpuState *cpu) {
    return ActorBridgeSceneBootstrap(cpu, 0x80a350u, Lufia2SceneReloadAndFadeIn, 2u, true);
}

RecompReturn Lufia2DecompBridge_9CB8(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2Memory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu,
        ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x809cb8u);
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2TextEngineStepWithServices(
        &memory, &state, ActorBridgePushedChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_EXECUTION_BOUNDARY) {
        cpu->PB = state.program_bank;
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, 3u, result.pc);
}

static RecompReturn ActorBridgeWorldSequence(CpuState *cpu, uint32_t entry,
    BattleControlFunction function, unsigned frame_size, bool save_bank) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeBattleEntry(cpu, entry, 0, function, frame_size,
        BATTLE_BRIDGE_M1X0, 0, save_bank ? BATTLE_BRIDGE_ANY_BANK : 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86AEB1(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86aeb1u,
        Lufia2WorldMapRunSceneSequence, 3u, true);
}

RecompReturn Lufia2DecompBridge_86D330(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86d330u,
        Lufia2WorldMapFinishDisplayReset, 2u, false);
}

RecompReturn Lufia2DecompBridge_869934(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x869934u,
        Lufia2WorldMapRefreshObjectPose, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AF3A(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86af3au,
        Lufia2WorldMapRunSceneTrack0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AF88(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86af88u,
        Lufia2WorldMapRunSceneTrack1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AF90(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86af90u,
        Lufia2WorldMapRunSceneTrack2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AF98(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86af98u,
        Lufia2WorldMapRunSceneTrack3, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AFA0(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86afa0u,
        Lufia2WorldMapRunSceneTrack4, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AFA8(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86afa8u,
        Lufia2WorldMapRunSceneTrack5, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AF71(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86af71u,
        Lufia2WorldMapUpdateSceneActor, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C1D2(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c1d2u,
        Lufia2WorldMapRunSceneScript, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C246(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c246u,
        Lufia2WorldMapStepSceneScript, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C284(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c284u,
        Lufia2WorldMapStartSceneActors, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C2AD(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c2adu,
        Lufia2WorldMapStartSceneActor, 2u, false);
}

RecompReturn Lufia2DecompBridge_86D385(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || !cpu->m_flag ||
        cpu->x_flag || cpu->D != 0u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86d385u);
    }
    return ActorBridgeWhole(cpu, 0x86d385u, Lufia2WorldMapApplyFadeIn, 2u);
}

static RecompReturn ActorBridgeWorldSceneLeaf(CpuState *cpu,uint32_t entry,
    Lufia2ExecutionResult (*function)(const Lufia2Memory*,Lufia2CpuState*)) {
    if(cpu->PB!=0x86u||cpu->DB!=0x86u||!cpu->m_flag||cpu->x_flag||cpu->D||
        cpu->S<0x1f20u||cpu->S>0x1ffcu) {
        const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu,&frame,entry);
    }
    return ActorBridgeWhole(cpu,entry,function,2u);
}

RecompReturn Lufia2DecompBridge_86AFB2(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86afb2u, Lufia2WorldMapRunGroupScene0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86AFE3(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86afe3u, Lufia2WorldMapRunGroupScene1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BF70(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bf70u, Lufia2WorldMapRunResourceScene0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BFF5(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bff5u, Lufia2WorldMapRunResourceScene1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C002(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c002u, Lufia2WorldMapRunResourceScene2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C00F(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c00fu, Lufia2WorldMapRunResourceScene3, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C041(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c041u, Lufia2WorldMapRunResourceScene4, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C2B3(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86c2b3u, Lufia2WorldMapPositionSceneGroup);
}

RecompReturn Lufia2DecompBridge_86C582(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86c582u, Lufia2WorldMapPrepareSceneWindowTables);
}

RecompReturn Lufia2DecompBridge_86966C(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86966cu, Lufia2WorldMapDarkenScenePalette);
}

RecompReturn Lufia2DecompBridge_86BB8C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bb8cu, Lufia2WorldMapInstallScenePalette, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BF99(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bf99u, Lufia2WorldMapUploadSceneGraphics, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BBE6(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bbe6u, Lufia2WorldMapLoadSceneBuffer, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C08C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c08cu, Lufia2WorldMapLoadAlternateSceneBuffer, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B15A(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b15au, Lufia2WorldMapRunGroupScene2, 2u,
                                    false);
}

RecompReturn Lufia2DecompBridge_86B59E(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b59eu, Lufia2WorldMapRunObjectScene, 2u,
                                    false);
}

RecompReturn Lufia2DecompBridge_86B63B(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b63bu,
                                    Lufia2WorldMapRunGroupTransitionScene, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BB5C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bb5cu, Lufia2WorldMapRunNearbyGroupScene,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86BE30(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86be30u, Lufia2WorldMapRunOffsetGroupScene,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86C04E(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c04eu, Lufia2WorldMapRunDualObjectScene,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86B19A(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b19au, Lufia2WorldMapLoadGroupSceneBuffer,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86BBC4(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bbc4u,
                                    Lufia2WorldMapPositionNearbySceneGroup, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BE72(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86be72u,
                                    Lufia2WorldMapStartSceneObjectAnimation, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BE7A(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86be7au,
                                    Lufia2WorldMapPositionOffsetSceneGroup, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BEC4(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86bec4u);
    }
    return ActorBridgeWholeM0X0(cpu, 0x86bec4u, Lufia2WorldMapOffsetSceneObject, 2u);
}

RecompReturn Lufia2DecompBridge_86C321(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86c321u,
                                     Lufia2WorldMapSelectSceneWindowRecord);
}

RecompReturn Lufia2DecompBridge_86C336(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c336u, Lufia2WorldMapLoadSceneWindowRecord,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86C2D9(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c2d9u,
                                    Lufia2WorldMapSubmitSceneWindowRecord, 2u, false);
}

static bool ActorBridgeCircleNeedsOriginalTiming(const CpuState *cpu, uint32_t entry) {
    if (!cpu->ram || cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu)
        return false;
    const unsigned stack = cpu->S;
    const uint16_t caller =
        (uint16_t)(cpu->ram[stack + 1u] | (uint16_t)cpu->ram[stack + 2u] << 8);
    unsigned far_frame;
    if (entry == 0x86c358u && cpu->DB == 0x86u) {
        far_frame = 1u;
    } else if (entry == 0x86c4a8u && caller == 0xc370u) {
        far_frame = 4u;
    } else if (entry == 0x86c40au && (caller == 0xc3c4u || caller == 0xc3cau)) {
        far_frame = caller == 0xc3c4u ? 5u : 4u;
    } else if (entry == 0x86c444u && (caller == 0xc3ebu || caller == 0xc3f1u)) {
        far_frame = caller == 0xc3ebu ? 6u : 5u;
    } else {
        return false;
    }
    if (stack + far_frame + 5u >= 0x2000u ||
        (far_frame > 1u && cpu->ram[stack + far_frame - 1u] != 0x86u))
        return false;
    const uint8_t *frame = cpu->ram + stack + far_frame;
    const uint16_t scene_return = (uint16_t)(frame[4] | (uint16_t)frame[5] << 8);
    /* Keep the transition's original VBlank phase. */
    return frame[0] == 0x14u && frame[1] == 0xc3u && frame[2] == 0x86u &&
           frame[3] == 0u && (scene_return == 0xb678u || scene_return == 0xb67fu);
}

static RecompReturn ActorBridgeCircleLeaf(CpuState *cpu, uint32_t entry,
                                          ActorWholeFunction function) {
    if (cpu->PB != 0x86u || cpu->DB != 0x7fu || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        ActorBridgeCircleNeedsOriginalTiming(cpu, entry)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWhole(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_86C358(CpuState *cpu) {
    if (ActorBridgeCircleNeedsOriginalTiming(cpu, 0x86c358u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86c358u);
    }
    return ActorBridgeWorldSequence(cpu, 0x86c358u, Lufia2WorldMapBuildCircularWindow,
                                    3u, true);
}

RecompReturn Lufia2DecompBridge_86C40A(CpuState *cpu) {
    return ActorBridgeCircleLeaf(cpu, 0x86c40au, Lufia2WorldMapAppendUpperCircleRows);
}

RecompReturn Lufia2DecompBridge_86C444(CpuState *cpu) {
    return ActorBridgeCircleLeaf(cpu, 0x86c444u, Lufia2WorldMapAppendLowerCircleRows);
}

RecompReturn Lufia2DecompBridge_86C4A8(CpuState *cpu) {
    return ActorBridgeCircleLeaf(cpu, 0x86c4a8u, Lufia2WorldMapBuildCircleExtentTable);
}

RecompReturn Lufia2DecompBridge_86E84F(CpuState *cpu) {
    if (cpu->S < 0x1f24u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e84fu);
    }
    return ActorBridgeWorldSequence(cpu, 0x86e84fu, Lufia2WorldMapPrepareSceneSprites,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86E8B5(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86e8b5u,
                                     Lufia2WorldMapClearSceneSpriteRecords);
}

static bool ActorBridgeSlopedWindowContext(const CpuState *cpu) {
    return cpu->PB == 0x86u && cpu->DB == 0x7fu && !cpu->D && cpu->S >= 0x1f20u &&
           cpu->S <= 0x1ffcu;
}

static RecompReturn ActorBridgeSlopedWindowLeaf(CpuState *cpu, uint32_t entry,
                                                ActorWholeFunction function,
                                                bool narrow) {
    if (!ActorBridgeSlopedWindowContext(cpu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeRunWhole(cpu, entry, function, 2u, narrow ? 0 : 2);
}

RecompReturn Lufia2DecompBridge_86C831(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86c831u,
                                       Lufia2WorldMapCountWindowEdgeRows, true);
}

RecompReturn Lufia2DecompBridge_86C843(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86c843u,
                                       Lufia2WorldMapResolveWindowEdgeEnd, true);
}

RecompReturn Lufia2DecompBridge_86C854(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86c854u,
                                       Lufia2WorldMapBuildLowerWindowBand, true);
}

RecompReturn Lufia2DecompBridge_86C928(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86c928u,
                                       Lufia2WorldMapBuildUpperWindowBand, true);
}

RecompReturn Lufia2DecompBridge_86CA06(CpuState *cpu) {
    if (!ActorBridgeSlopedWindowContext(cpu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86ca06u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86ca06u, 0, Lufia2WorldMapDrawSlopedWindowEdge,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x7f, 0);
}

RecompReturn Lufia2DecompBridge_86CA2F(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86ca2fu,
                                       Lufia2WorldMapDrawUpperRightWindowEdge, false);
}

RecompReturn Lufia2DecompBridge_86CA5E(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86ca5eu,
                                       Lufia2WorldMapDrawLowerRightWindowEdge, false);
}

RecompReturn Lufia2DecompBridge_86CA83(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86ca83u,
                                       Lufia2WorldMapDrawLowerLeftWindowEdge, false);
}

RecompReturn Lufia2DecompBridge_86CAB2(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86cab2u,
                                       Lufia2WorldMapDrawUpperLeftWindowEdge, false);
}

RecompReturn Lufia2DecompBridge_86CAD7(CpuState *cpu) {
    return ActorBridgeSlopedWindowLeaf(cpu, 0x86cad7u,
                                       Lufia2WorldMapInitializeWindowRows, true);
}

RecompReturn Lufia2DecompBridge_86E71E(CpuState *cpu) {
    if (cpu->S < 0x1f24u) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86e71eu);
    }
    return ActorBridgeWorldSequence(cpu, 0x86e71eu,
                                    Lufia2WorldMapAnimateSceneSpriteTrails, 2u, false);
}

RecompReturn Lufia2DecompBridge_86E7A2(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86e7a2u,
                                     Lufia2WorldMapSpawnSceneSpriteTrail);
}

RecompReturn Lufia2DecompBridge_86E7CE(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86e7ceu,
                                     Lufia2WorldMapAdvanceSceneSpriteTrail);
}

RecompReturn Lufia2DecompBridge_86E804(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86e804u,
                                     Lufia2WorldMapDrawSceneSpriteTrail);
}

static RecompReturn ActorBridgeWorldWindowClip(CpuState *cpu, uint32_t entry,
                                               BattleControlFunction function) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeBattleEntry(cpu, entry, 0, function, 2u, BATTLE_BRIDGE_M0X0, 0,
                                  0x86, 0);
}

RecompReturn Lufia2DecompBridge_86C536(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86c536u,
                                     Lufia2WorldMapSelectSlopedWindowRecord);
}

RecompReturn Lufia2DecompBridge_86C54B(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c54bu,
                                    Lufia2WorldMapLoadSlopedWindowRecord, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C4E0(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c4e0u,
                                    Lufia2WorldMapSubmitSlopedWindowRecord, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C5F8(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c5f8u, Lufia2WorldMapBuildSlopedWindow, 3u,
                                    false);
}

RecompReturn Lufia2DecompBridge_86CB21(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86cb21u, Lufia2WorldMapClipWindowEndpoint,
                                    2u, false);
}

RecompReturn Lufia2DecompBridge_86CB49(CpuState *cpu) {
    return ActorBridgeWorldWindowClip(cpu, 0x86cb49u,
                                      Lufia2WorldMapClipWindowLowerRight);
}

RecompReturn Lufia2DecompBridge_86CB9E(CpuState *cpu) {
    return ActorBridgeWorldWindowClip(cpu, 0x86cb9eu,
                                      Lufia2WorldMapResolveWindowRightIntersection);
}

RecompReturn Lufia2DecompBridge_86B039(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b039u,
                                    Lufia2WorldMapStepExpandingSceneWindows, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B1DE(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b1deu,
                                    Lufia2WorldMapStepSceneWindowAnimations, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B51C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b51cu, Lufia2WorldMapStepSceneSparkSprites, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B56D(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b56du, Lufia2WorldMapSpawnSceneSparkSprite, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BC0F(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bc0fu, Lufia2WorldMapStepBurstSceneFrame, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BCA6(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bca6u, Lufia2WorldMapAdvanceSceneOrbit, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BD9F(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bd9fu, Lufia2WorldMapStepSceneCirclePulse, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BDF6(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bdf6u, Lufia2WorldMapStartSceneCirclePulse, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B857(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b857u, Lufia2WorldMapStepPhasedSceneFrame, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B954(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86b954u, Lufia2WorldMapStopSceneParticleEmission);
}

RecompReturn Lufia2DecompBridge_86B95B(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b95bu, Lufia2WorldMapStartScenePhaseAnimation, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B969(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b969u, Lufia2WorldMapStartSceneFall, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B97E(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b97eu, Lufia2WorldMapStartSceneDeparture, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B9A3(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b9a3u, Lufia2WorldMapStepSceneFall, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B9E5(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86b9e5u, Lufia2WorldMapStepSceneRebound);
}

RecompReturn Lufia2DecompBridge_86BA16(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86ba16u, Lufia2WorldMapSplitSceneGroup, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BA2C(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86ba2cu, Lufia2WorldMapAlignSceneGroup);
}

RecompReturn Lufia2DecompBridge_86BA48(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86ba48u, Lufia2WorldMapStepSceneDeparture);
}

RecompReturn Lufia2DecompBridge_86BAA8(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86baa8u, Lufia2WorldMapResetSceneObjectAnimation, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BAAE(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86baaeu, Lufia2WorldMapSpawnSceneParticle, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BAEC(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86baecu, Lufia2WorldMapLoadScenePhaseWindow0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BAFA(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bafau, Lufia2WorldMapLoadScenePhaseWindow1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BB01(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bb01u, Lufia2WorldMapLoadScenePhaseWindow2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BB08(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bb08u, Lufia2WorldMapLoadScenePhaseWindow3, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BB0F(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bb0fu, Lufia2WorldMapLoadScenePhaseWindow4, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B454(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b454u, Lufia2WorldMapGrowSceneWindow, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B47E(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b47eu, Lufia2WorldMapRotateSceneWindow, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B4BE(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b4beu, Lufia2WorldMapContractSceneWindow, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B4F2(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b4f2u, Lufia2WorldMapSubmitPhasedSceneWindow, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B508(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b508u, Lufia2WorldMapContractSceneCircle, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B748(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b748u, Lufia2WorldMapStepLayeredSceneWindows, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B7CF(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86b7cfu, Lufia2WorldMapStopWindowSparkEmission);
}

RecompReturn Lufia2DecompBridge_86B7D3(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86b7d3u, Lufia2WorldMapBeginWindowColorPulse);
}

RecompReturn Lufia2DecompBridge_86B7D5(CpuState *cpu) {
    return ActorBridgeWorldSceneLeaf(cpu, 0x86b7d5u, Lufia2WorldMapRestartWindowColorPulse);
}

RecompReturn Lufia2DecompBridge_86B7DF(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b7dfu, Lufia2WorldMapLoadLayeredSceneWindow0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B7F7(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b7f7u, Lufia2WorldMapLoadLayeredSceneWindow1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B7FE(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b7feu, Lufia2WorldMapLoadLayeredSceneWindow2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B350(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b350u, Lufia2WorldMapCycleScenePaletteFade, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C1E3(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c1e3u,
        Lufia2WorldMapResumeSceneScript, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C237(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c237u,
        Lufia2WorldMapResumeSceneFrame, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B38E(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b38eu,
        Lufia2WorldMapResumePaletteWait0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B391(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b391u,
        Lufia2WorldMapResumePaletteWait1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B394(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b394u,
        Lufia2WorldMapResumePaletteWait2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B397(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b397u,
        Lufia2WorldMapResumePaletteWait3, 2u, false);
}

RecompReturn Lufia2DecompBridge_838007(CpuState *cpu) {
    const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x83u || cpu->DB != 0x83u ||
        !cpu->m_flag || cpu->D || cpu->S != 0x1fffu || cpu->host_return_valid != 0u)
        return ActorBridgeFallback(cpu, &frame, 0x838007u);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu,
                                 ActorBridgeExecutionCheckpoint, cpu};
    ActorPushedCall call = {cpu, frame, RECOMP_RETURN_NORMAL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result =
        Lufia2FieldResumeFrame(&memory, &state, ActorBridgeSceneChild, &call);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    cpu->PB = state.program_bank;
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s,
                                     frame.hrv);
}

static RecompReturn ActorBridgeFieldAttributeEntry(CpuState *cpu, uint32_t entry,
                                                   ActorWholeFunction run,
                                                   uint8_t frame_size) {
    if (cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame,
                                   ((uint32_t)cpu->PB << 16) | (uint16_t)entry);
    }
    return ActorBridgeWholeM1X16(cpu, entry, run, frame_size);
}

RecompReturn Lufia2DecompBridge_80EF2F(CpuState *cpu) {
    return ActorBridgeFieldAttributeEntry(cpu, 0x80ef2fu,
                                          Lufia2FieldMarkVisibleActorCells, 3u);
}

RecompReturn Lufia2DecompBridge_80EEEB(CpuState *cpu) {
    return ActorBridgeFieldAttributeEntry(cpu, 0x80eeebu, Lufia2FieldMarkListedCells,
                                          2u);
}

static RecompReturn ActorBridgeAreaSpawnEntry(CpuState *cpu, uint32_t entry,
                                              ActorWholeFunction run,
                                              uint8_t frame_size) {
    if (cpu->PB != 0x80u || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame,
                                   ((uint32_t)cpu->PB << 16) | (uint16_t)entry);
    }
    return ActorBridgeWholeM1X16(cpu, entry, run, frame_size);
}

RecompReturn Lufia2DecompBridge_80D533(CpuState *cpu) {
    return ActorBridgeAreaSpawnEntry(cpu, 0x80d533u, Lufia2FieldSpawnActorsInArea, 3u);
}

RecompReturn Lufia2DecompBridge_80D583(CpuState *cpu) {
    return ActorBridgeAreaSpawnEntry(cpu, 0x80d583u, Lufia2FieldRandomAreaCoordinate,
                                     2u);
}

static RecompReturn ActorBridgeSecondaryMovement(CpuState *cpu, uint32_t entry,
                                                 ActorWholeFunction run) {
    if (cpu->PB != 0x83u || cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame,
                                   ((uint32_t)cpu->PB << 16) | (uint16_t)entry);
    }
    return ActorBridgeWholeM1X16(cpu, entry, run, 2u);
}

RecompReturn Lufia2DecompBridge_83DA8A(CpuState *cpu) {
    return ActorBridgeSecondaryMovement(cpu, 0x830000u | 0xda8au,
                                        Lufia2ActorStopSecondaryScript);
}

RecompReturn Lufia2DecompBridge_83DA71(CpuState *cpu) {
    return ActorBridgeSecondaryMovement(cpu, 0x830000u | 0xda71u,
                                        Lufia2ActorMarkLeaderScriptStop);
}

RecompReturn Lufia2DecompBridge_83DA5B(CpuState *cpu) {
    return ActorBridgeSecondaryMovement(cpu, 0x830000u | 0xda5bu,
                                        Lufia2ActorPrepareStepProbe);
}

RecompReturn Lufia2DecompBridge_83D9C6(CpuState *cpu) {
    return ActorBridgeSecondaryMovement(cpu, 0x830000u | 0xd9c6u,
                                        Lufia2ActorCommitTileStep);
}

static RecompReturn ActorBridgePopupRecord(CpuState *cpu, uint32_t entry,
                                           ActorWholeFunction run) {
    if (cpu->PB != 0x81u || cpu->DB != 0x7eu || cpu->D || cpu->S < 0x1f00u ||
        cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame,
                                   ((uint32_t)cpu->PB << 16) | (uint16_t)entry);
    }
    return ActorBridgeWholeM1X16(cpu, entry, run, 2u);
}

RecompReturn Lufia2DecompBridge_81C3EE(CpuState *cpu) {
    return ActorBridgePopupRecord(cpu, 0x81c3eeu, Lufia2BattleBuildSlotPopup);
}

RecompReturn Lufia2DecompBridge_81C477(CpuState *cpu) {
    return ActorBridgePopupRecord(cpu, 0x81c477u, Lufia2BattleBuildPopupRecord);
}

RecompReturn Lufia2DecompBridge_81C652(CpuState *cpu) {
    if (cpu->PB != 0x81u || (cpu->DB & 0x7fu) >= 0x40u || cpu->D || cpu->S < 0x1f00u ||
        cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, ((uint32_t)cpu->PB << 16) | 0xc652u);
    }
    return ActorBridgeBattleEntry(
        cpu, 0x81c652u, NULL, Lufia2BattleApplyTargetStatusTick, 2u, BATTLE_BRIDGE_M1X0,
        0, BATTLE_BRIDGE_REGISTER_BANK, 0);
}

static RecompReturn ActorBridgeFieldConditionEntry(CpuState *cpu, uint32_t entry,
                                                   ActorWholeFunction run,
                                                   uint8_t frame_size) {
    if (cpu->PB != 0x80u || ((cpu->DB & 0x7fu) >= 0x40u && cpu->DB != 0x7eu) ||
        cpu->D || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame,
                                   ((uint32_t)cpu->PB << 16) | (uint16_t)entry);
    }
    return ActorBridgeRunWhole(cpu, entry, run, frame_size, 4u);
}

RecompReturn Lufia2DecompBridge_80E4C5(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e4c5u,
                                          Lufia2FieldStoreConditionResult, 2u);
}

RecompReturn Lufia2DecompBridge_80E55B(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e55bu,
                                          Lufia2FieldNegateConditionResult, 2u);
}

RecompReturn Lufia2DecompBridge_80E132(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e132u, Lufia2FieldTestLeaderPosition,
                                          2u);
}

RecompReturn Lufia2DecompBridge_80E15A(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e15au, Lufia2FieldTestLeaderArea,
                                          2u);
}

RecompReturn Lufia2DecompBridge_80CD8B(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80cd8bu, Lufia2FieldFindObjectInBox,
                                          2u);
}

RecompReturn Lufia2DecompBridge_80CD88(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80cd88u, Lufia2FieldFindObjectInArea,
                                          2u);
}

RecompReturn Lufia2DecompBridge_80E458(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e458u, Lufia2FieldFindCellBitsInArea,
                                          2u);
}

RecompReturn Lufia2DecompBridge_80E78D(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e78du,
                                          Lufia2FieldFindEventConditionList, 3u);
}

RecompReturn Lufia2DecompBridge_80E566(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e566u, Lufia2FieldTestConditionList,
                                          2u);
}

RecompReturn Lufia2DecompBridge_83ACEF(CpuState *cpu) {
    return ActorBridgeFieldSession(cpu, 0x83acefu, Lufia2FieldSelectSessionSetup);
}

static RecompReturn ActorBridgeFieldPaletteEntry(CpuState *cpu, uint32_t entry,
                                                 ActorWholeFunction function) {
    if (cpu->PB != 0x83u || cpu->DB != 0x7fu || cpu->D || cpu->S < 0x1f00u ||
        cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeRunWhole(cpu, entry, function, 2u, 4u);
}
RecompReturn Lufia2DecompBridge_83AEED(CpuState *cpu) {
    return ActorBridgeFieldPaletteEntry(cpu, 0x83aeedu,
                                        Lufia2FieldAdvancePaletteCycles);
}
RecompReturn Lufia2DecompBridge_83AF54(CpuState *cpu) {
    return ActorBridgeFieldPaletteEntry(cpu, 0x83af54u, Lufia2FieldStepWaveTable);
}

static RecompReturn ActorBridgeActorResourceEntry(CpuState *cpu, uint32_t entry,
                                                  ActorWholeFunction function,
                                                  uint8_t frame_size) {
    if (cpu->PB != 0x83u || cpu->D ||
        cpu->S < (entry == 0x83aa50u ? 0x1f20u : 0x1f00u) || cpu->S > 0x1ffcu ||
        (entry == 0x83aa50u && (cpu->DB & 0x7fu) >= 0x40u && cpu->DB != 0x7eu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, frame_size);
}
RecompReturn Lufia2DecompBridge_83AA50(CpuState *cpu) {
    return ActorBridgeActorResourceEntry(cpu, 0x83aa50u, Lufia2ActorAllocateSprite, 2u);
}
RecompReturn Lufia2DecompBridge_83DFFD(CpuState *cpu) {
    return ActorBridgeActorResourceEntry(cpu, 0x83dffdu, Lufia2ActorLoadSecondaryScript,
                                         3u);
}
RecompReturn Lufia2DecompBridge_80E92A(CpuState *cpu) {
    return ActorBridgeFieldConditionEntry(cpu, 0x80e92au, Lufia2FieldResolveEventArea,
                                          2u);
}

static RecompReturn ActorBridgeScalingEntry(CpuState *cpu, uint32_t entry,
                                            ActorWholeFunction function) {
    const int reads_operand = entry != 0x83fb05u;
    const int sets_accumulator_width = entry == 0x83faf4u;
    if (cpu->PB != 0x83u || cpu->D || cpu->emulation || cpu->_flag_D ||
        (!sets_accumulator_width && !cpu->m_flag) || cpu->S < 0x1f00u ||
        cpu->S > 0x1ffcu ||
        (reads_operand && (cpu->DB & 0x7fu) >= 0x40u && cpu->DB != 0x7eu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_83DE76(CpuState *cpu) {
    return ActorBridgeScalingEntry(cpu, 0x83de76u, Lufia2ActorBuildOrbitOffsets);
}

RecompReturn Lufia2DecompBridge_83DE9F(CpuState *cpu) {
    return ActorBridgeScalingEntry(cpu, 0x83de9fu, Lufia2ActorScaleRadius);
}

RecompReturn Lufia2DecompBridge_83FAF4(CpuState *cpu) {
    return ActorBridgeScalingEntry(cpu, 0x83faf4u, Lufia2ActorReadSignedOperand);
}

RecompReturn Lufia2DecompBridge_83FB05(CpuState *cpu) {
    return ActorBridgeScalingEntry(cpu, 0x83fb05u, Lufia2ObjectSignExtendNibble);
}

static RecompReturn ActorBridgeMenuOperandEntry(CpuState *cpu, uint32_t entry,
                                                ActorWholeFunction function) {
    const int needs_byte_mode = entry == 0x808c6cu;
    if (cpu->PB != 0x80u || cpu->D || cpu->emulation || cpu->_flag_D || cpu->x_flag ||
        (needs_byte_mode && !cpu->m_flag) || cpu->S < 0x1f00u || cpu->S > 0x1ffcu ||
        ((cpu->DB & 0x7fu) >= 0x40u && cpu->DB != 0x7eu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_808BA7(CpuState *cpu) {
    return ActorBridgeMenuOperandEntry(cpu, 0x808ba7u, Lufia2MenuReadPointerOperand);
}

RecompReturn Lufia2DecompBridge_808BB2(CpuState *cpu) {
    return ActorBridgeMenuOperandEntry(cpu, 0x808bb2u,
                                       Lufia2MenuReadOffsetPointerOperand);
}

RecompReturn Lufia2DecompBridge_808C6C(CpuState *cpu) {
    return ActorBridgeMenuOperandEntry(cpu, 0x808c6cu, Lufia2MenuLoadNumberBytes);
}

RecompReturn Lufia2DecompBridge_808C9E(CpuState *cpu) {
    return ActorBridgeMenuOperandEntry(cpu, 0x808c9eu, Lufia2MenuReadBranchOperands);
}

static RecompReturn ActorBridgeObjectHelper(CpuState *cpu, uint32_t entry,
                                            ActorWholeFunction function,
                                            unsigned position) {
    if (cpu->PB != 0x83u || cpu->D || cpu->emulation || cpu->_flag_D ||
        cpu->m_flag != !position || cpu->S < 0x1f00u || cpu->S > 0x1ffcu ||
        (!position && (cpu->DB & 0x7fu) >= 0x40u && cpu->DB != 0x7eu)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_83E1AD(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83e1adu, Lufia2ObjectAddDisplayOffsetX, 1u);
}

RecompReturn Lufia2DecompBridge_83E1B9(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83e1b9u, Lufia2ObjectAddDisplayOffsetY, 1u);
}

RecompReturn Lufia2DecompBridge_83EE9D(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83ee9du, Lufia2ObjectAddFinePositionX, 1u);
}

RecompReturn Lufia2DecompBridge_83EEA9(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83eea9u, Lufia2ObjectAddFinePositionY, 1u);
}

RecompReturn Lufia2DecompBridge_83EED8(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83eed8u, Lufia2ObjectReadRandomOffset, 0u);
}

RecompReturn Lufia2DecompBridge_83ED63(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83ed63u, Lufia2ObjectReadSubcommand, 0u);
}

RecompReturn Lufia2DecompBridge_83EE0E(CpuState *cpu) {
    return ActorBridgeObjectHelper(cpu, 0x83ee0eu, Lufia2ObjectCopyAnimationState, 0u);
}

static RecompReturn ActorBridgeCaveGridEntry(CpuState *cpu, uint32_t entry,
                                             ActorWholeFunction function,
                                             unsigned accumulator_mode,
                                             unsigned work_bank) {
    if (cpu->PB != 0x83u || cpu->D || cpu->emulation || cpu->_flag_D || cpu->x_flag ||
        (accumulator_mode != 2u && cpu->m_flag != accumulator_mode) ||
        (work_bank && cpu->DB != 0x7fu) || cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_839B18(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839b18u, Lufia2CaveClearVisitedCells, 1u,
                                    1u);
}

RecompReturn Lufia2DecompBridge_839B27(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839b27u, Lufia2CaveCountOccupiedCells, 1u,
                                    1u);
}

RecompReturn Lufia2DecompBridge_839B3C(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839b3cu, Lufia2CaveOffsetStairPosition, 1u,
                                    0u);
}

RecompReturn Lufia2DecompBridge_839B48(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839b48u, Lufia2CaveResolveCellOrigin, 1u,
                                    0u);
}

RecompReturn Lufia2DecompBridge_839B62(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839b62u, Lufia2CaveLinkRoomCells, 1u, 1u);
}

RecompReturn Lufia2DecompBridge_839BDD(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839bddu, Lufia2CaveMergeRoomCells, 1u, 1u);
}

RecompReturn Lufia2DecompBridge_839CA0(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839ca0u, Lufia2CaveFillRoomRectangle, 1u,
                                    0u);
}

RecompReturn Lufia2DecompBridge_839CCB(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839ccbu, Lufia2CaveResolveCellIndex, 1u, 0u);
}

RecompReturn Lufia2DecompBridge_839CD6(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839cd6u, Lufia2CaveResolveBlockOffset, 1u,
                                    0u);
}

RecompReturn Lufia2DecompBridge_839CF3(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839cf3u, Lufia2CaveResolveTileOffset, 1u,
                                    0u);
}

RecompReturn Lufia2DecompBridge_839D68(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839d68u, Lufia2CaveResolveWordTileOffset, 2u,
                                    0u);
}

RecompReturn Lufia2DecompBridge_839D11(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839d11u, Lufia2CavePickUnvisitedCell, 1u,
                                    1u);
}

RecompReturn Lufia2DecompBridge_839D23(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839d23u, Lufia2CaveFindUnvisitedCell, 1u,
                                    1u);
}

RecompReturn Lufia2DecompBridge_839D39(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839d39u, Lufia2CaveAppendRoomLink, 1u, 1u);
}

RecompReturn Lufia2DecompBridge_839D46(CpuState *cpu) {
    return ActorBridgeCaveGridEntry(cpu, 0x839d46u, Lufia2CaveReplaceUpperTileIndex, 0u,
                                    1u);
}

static RecompReturn ActorBridgeCavePlacementEntry(CpuState *cpu, uint32_t entry,
                                                  ActorWholeFunction function,
                                                  unsigned work_bank,
                                                  unsigned allow_byte_index) {
    if (cpu->PB != 0x83u || cpu->D || cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        (!allow_byte_index && cpu->x_flag) || (work_bank && cpu->DB != 0x7fu) ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_8399C8(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x8399c8u, Lufia2CaveResolveTilePosition,
                                         0u, 1u);
}

RecompReturn Lufia2DecompBridge_8399D3(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x8399d3u, Lufia2CaveProbeObjectPlacement,
                                         1u, 0u);
}

RecompReturn Lufia2DecompBridge_839A1C(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839a1cu, Lufia2CavePlaceRandomObject,
                                         1u, 0u);
}

RecompReturn Lufia2DecompBridge_839A6B(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839a6bu, Lufia2CaveProbeNearbyChest, 1u,
                                         0u);
}

RecompReturn Lufia2DecompBridge_839A9F(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839a9fu, Lufia2CaveProbeChestAtPosition,
                                         1u, 0u);
}

RecompReturn Lufia2DecompBridge_839ABC(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839abcu, Lufia2CaveTryPlaceRoomChest,
                                         1u, 0u);
}

RecompReturn Lufia2DecompBridge_839E1B(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839e1bu, Lufia2CaveDrawRandomBelow, 0u,
                                         1u);
}

RecompReturn Lufia2DecompBridge_839E11(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839e11u, Lufia2CaveDrawRandomIndex, 0u,
                                         1u);
}

RecompReturn Lufia2DecompBridge_839DE4(CpuState *cpu) {
    return ActorBridgeCavePlacementEntry(cpu, 0x839de4u, Lufia2CaveDrawRandomMean, 0u,
                                         1u);
}

static RecompReturn ActorBridgeTileUploadEntry(CpuState *cpu, uint32_t entry,
                                               ActorWholeFunction function,
                                               unsigned any_accumulator) {
    const uint8_t bank = cpu->DB;
    const uint8_t cave = entry == 0x839d88u;
    const uint8_t mapped_bank =
        cave ? bank == 0x7fu
             : bank < 0x40u || bank == 0x7eu || (bank >= 0x80u && bank < 0xc0u);
    if (cpu->PB != (cave ? 0x83u : 0x86u) || cpu->D || cpu->emulation || cpu->_flag_D ||
        cpu->x_flag || (!any_accumulator && !cpu->m_flag) || !mapped_bank ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeWholeAnyWidth(cpu, entry, function, 2u);
}

RecompReturn Lufia2DecompBridge_839D88(CpuState *cpu) {
    return ActorBridgeTileUploadEntry(cpu, 0x839d88u, Lufia2CaveDrawTileBlock, 0u);
}

RecompReturn Lufia2DecompBridge_86D1A1(CpuState *cpu) {
    return ActorBridgeTileUploadEntry(cpu, 0x86d1a1u, Lufia2WorldMapUploadTileQueue,
                                      0u);
}

RecompReturn Lufia2DecompBridge_86D1E8(CpuState *cpu) {
    return ActorBridgeTileUploadEntry(cpu, 0x86d1e8u, Lufia2WorldMapUploadTileColumn,
                                      1u);
}

RecompReturn Lufia2DecompBridge_86D271(CpuState *cpu) {
    return ActorBridgeTileUploadEntry(cpu, 0x86d271u, Lufia2WorldMapUploadTileBlock,
                                      1u);
}

static uint8_t ActorBridgeSelectSetupContext(const CpuState *cpu,
                                             unsigned any_accumulator) {
    return cpu->PB == 0x86u && !cpu->D && !cpu->emulation && !cpu->_flag_D &&
           !cpu->x_flag && (any_accumulator || cpu->m_flag) && cpu->S >= 0x1f00u &&
           cpu->S <= 0x1ffcu;
}

RecompReturn Lufia2DecompBridge_8680DB(CpuState *cpu) {
    if (!ActorBridgeSelectSetupContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8680dbu);
    }
    return ActorBridgeSceneOwners(cpu, 0x8680dbu, Lufia2SelectScreenLoadGraphics, 2u);
}

RecompReturn Lufia2DecompBridge_868157(CpuState *cpu) {
    if (!ActorBridgeSelectSetupContext(cpu, 1u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868157u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x868157u, Lufia2SelectScreenLoadPalette, 2u);
}

RecompReturn Lufia2DecompBridge_86817A(CpuState *cpu) {
    if (!ActorBridgeSelectSetupContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86817au);
    }
    return ActorBridgeSceneOwners(cpu, 0x86817au, Lufia2SelectScreenLoadMaps, 2u);
}

RecompReturn Lufia2DecompBridge_868033(CpuState *cpu) {
    if (!ActorBridgeSelectSetupContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868033u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868033u, Lufia2SelectScreenConfigureDisplay,
                                  2u);
}

RecompReturn Lufia2DecompBridge_868000(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->D || cpu->emulation || cpu->_flag_D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868000u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868000u, Lufia2SelectScreenRun, 3u);
}

static uint8_t ActorBridgeSelectControlContext(const CpuState *cpu,
                                               unsigned width_mode) {
    return cpu->PB == 0x86u && !cpu->D && !cpu->emulation && !cpu->_flag_D &&
           (width_mode == 2u || !cpu->x_flag) && (width_mode || cpu->m_flag) &&
           cpu->S >= 0x1f00u && cpu->S <= 0x1ffcu;
}

RecompReturn Lufia2DecompBridge_8689FD(CpuState *cpu) {
    if (!ActorBridgeSelectControlContext(cpu, 2u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8689fdu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8689fdu, Lufia2SelectScreenClearSpriteStatus,
                                    2u);
}

RecompReturn Lufia2DecompBridge_8687D3(CpuState *cpu) {
    if (!ActorBridgeSelectControlContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8687d3u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x8687d3u, Lufia2SelectScreenClearState, 2u);
}

RecompReturn Lufia2DecompBridge_868865(CpuState *cpu) {
    if (!ActorBridgeSelectControlContext(cpu, 1u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868865u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868865u, Lufia2SelectScreenConfigureHdmaSlot,
                                  2u);
}

RecompReturn Lufia2DecompBridge_868784(CpuState *cpu) {
    if (!ActorBridgeSelectControlContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868784u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868784u, Lufia2SelectScreenInitializeLayout,
                                  2u);
}

RecompReturn Lufia2DecompBridge_8681D0(CpuState *cpu) {
    if (!ActorBridgeSelectControlContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8681d0u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8681d0u, Lufia2SelectScreenControl, 2u);
}

static uint8_t ActorBridgeSelectTransitionContext(const CpuState *cpu,
                                                  unsigned either_m) {
    return cpu->PB == 0x86u && cpu->DB == 0x7eu && !cpu->D && !cpu->emulation &&
           !cpu->_flag_D && !cpu->x_flag && (either_m || cpu->m_flag) &&
           cpu->S >= 0x1f00u && cpu->S <= 0x1ffcu;
}

RecompReturn Lufia2DecompBridge_868305(CpuState *cpu) {
    if (!ActorBridgeSelectTransitionContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868305u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868305u, Lufia2SelectScreenFadeIn, 2u);
}

RecompReturn Lufia2DecompBridge_86830F(CpuState *cpu) {
    if (!ActorBridgeSelectTransitionContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86830fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86830fu, Lufia2SelectScreenFadeOut, 2u);
}

RecompReturn Lufia2DecompBridge_868339(CpuState *cpu) {
    if (!ActorBridgeSelectTransitionContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868339u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868339u, Lufia2SelectScreenWaitForInput, 2u);
}

RecompReturn Lufia2DecompBridge_86835B(CpuState *cpu) {
    if (!ActorBridgeSelectTransitionContext(cpu, 1u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86835bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x86835bu, Lufia2SelectScreenConfigureObject,
                                  2u);
}

RecompReturn Lufia2DecompBridge_8682C8(CpuState *cpu) {
    if (!ActorBridgeSelectTransitionContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8682c8u);
    }
    return ActorBridgeSceneOwners(cpu, 0x8682c8u, Lufia2SelectScreenWaitForObjects, 2u);
}

RecompReturn Lufia2DecompBridge_8681FB(CpuState *cpu) {
    if (cpu->DB != 0x7eu || !ActorBridgeSelectControlContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x8681fbu);
    }
    return ActorBridgeSceneOwners(cpu, 0x8681fbu, Lufia2SelectScreenPlaySequence, 2u);
}

RecompReturn Lufia2DecompBridge_868298(CpuState *cpu) {
    if (cpu->DB != 0x7eu || !ActorBridgeSelectControlContext(cpu, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x868298u);
    }
    return ActorBridgeSceneOwners(cpu, 0x868298u,
                                  Lufia2SelectScreenPlayAlternateSequence, 2u);
}

static uint8_t ActorBridgeActionWorkContext(const CpuState *cpu, uint8_t bank,
                                            unsigned mode) {
    return cpu->PB == bank && !cpu->D && !cpu->emulation && !cpu->_flag_D &&
           (mode == 2u || !cpu->x_flag) &&
           (mode == 1u || mode == 2u || cpu->m_flag == (mode == 3u ? 0u : 1u)) &&
           cpu->S >= 0x1f00u && cpu->S <= 0x1ffcu;
}

RecompReturn Lufia2DecompBridge_81F114(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x81u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81f114u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x81f114u, Lufia2InventoryRemoveQuantity, 3u);
}

RecompReturn Lufia2DecompBridge_81F15B(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x81u, 1u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81f15bu);
    }
    return ActorBridgeSceneOwners(cpu, 0x81f15bu, Lufia2InventoryConsumeRequestedItem,
                                  3u);
}

RecompReturn Lufia2DecompBridge_81EBF4(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x81u, 2u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81ebf4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x81ebf4u, Lufia2BattleResetTargetMotion, 3u);
}

RecompReturn Lufia2DecompBridge_85948C(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85948cu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85948cu,
                                  Lufia2BattleRefreshInactiveEnemySprites, 3u);
}

RecompReturn Lufia2DecompBridge_85CE21(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 2u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ce21u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85ce21u, Lufia2BattlePrepareAlternateAction,
                                  3u);
}

RecompReturn Lufia2DecompBridge_85CF68(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85cf68u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85cf68u, Lufia2BattleBuildActionAdjustments,
                                  2u);
}

RecompReturn Lufia2DecompBridge_85D025(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85d025u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85d025u, Lufia2BattleCalculateActionBias, 2u);
}

RecompReturn Lufia2DecompBridge_85D94F(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85d94fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85d94fu, Lufia2BattleStoreActionResult, 2u);
}

RecompReturn Lufia2DecompBridge_85D9F0(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85d9f0u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85d9f0u,
                                    Lufia2BattleInvalidateTurnQueueActor, 3u);
}

RecompReturn Lufia2DecompBridge_85937D(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85937du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85937du, Lufia2BattleRemoveTurnQueueActor,
                                    3u);
}

RecompReturn Lufia2DecompBridge_85CFAB(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 3u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85cfabu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85cfabu,
                                    Lufia2BattleAccumulateActionAdjustment, 2u);
}

RecompReturn Lufia2DecompBridge_85D080(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85d080u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85d080u, Lufia2BattleReadActorBiasFlag, 2u);
}

RecompReturn Lufia2DecompBridge_858F8A(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858f8au);
    }
    return ActorBridgeSceneOwners(cpu, 0x858f8au,
                                  Lufia2BattleResolveRecoverableStatuses, 2u);
}

RecompReturn Lufia2DecompBridge_85E0E4(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x85u, 2u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85e0e4u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85e0e4u,
                                  Lufia2BattleCheckEnemyGraphicsCapacity, 3u);
}

RecompReturn Lufia2DecompBridge_869093(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x86u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x869093u);
    }
    return ActorBridgeSceneOwners(cpu, 0x869093u, Lufia2SelectScreenQueueTilemapCopy,
                                  2u);
}

RecompReturn Lufia2DecompBridge_848862(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x84u, 3u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x848862u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x848862u, Lufia2AncientCaveAppendSavedItem,
                                    2u);
}

RecompReturn Lufia2DecompBridge_84886D(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x84u, 3u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x84886du);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x84886du, Lufia2AncientCaveTestBlueItem, 2u);
}

RecompReturn Lufia2DecompBridge_858E98(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x858e98u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x858e98u, Lufia2BattleUploadVramQueue, 2u);
}

RecompReturn Lufia2DecompBridge_859265(CpuState *cpu) {
    if (!ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x859265u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x859265u, Lufia2BattleClearActionWorkspace,
                                    3u);
}

static uint8_t ActorBridgeTargetBank(uint8_t bank) {
    return bank < 0x40u || (bank >= 0x7eu && bank < 0xc0u);
}
RecompReturn Lufia2DecompBridge_85CE6F(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeTargetBank(cpu->DB) ||
        !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85ce6fu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85ce6fu, Lufia2BattlePrepareTargetedAction,
                                  2u);
}
RecompReturn Lufia2DecompBridge_85E02D(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85e02du);
    }
    return ActorBridgeSceneOwners(cpu, 0x85e02du,
                                  Lufia2BattleFindEnemyGraphicsPlacement, 3u);
}

RecompReturn Lufia2DecompBridge_81FB1A(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x81u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x81fb1au);
    }
    return ActorBridgeSceneOwners(cpu, 0x81fb1au, Lufia2BattleRollEnemyItemDrop, 3u);
}
RecompReturn Lufia2DecompBridge_85C168(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeTargetBank(cpu->DB) ||
        !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85c168u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85c168u, Lufia2BattleResolveActorRecord, 2u);
}
RecompReturn Lufia2DecompBridge_85DA11(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85da11u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85da11u, Lufia2BattleRunEquipmentScripts, 3u);
}
RecompReturn Lufia2DecompBridge_85DA43(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85da43u);
    }
    return ActorBridgeSceneOwners(cpu, 0x85da43u, Lufia2BattleRunEquipmentItemScript,
                                  2u);
}
RecompReturn Lufia2DecompBridge_85DA5F(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85da5fu);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x85da5fu, Lufia2BattleCharacterFromActor, 3u);
}

RecompReturn Lufia2DecompBridge_85D0CD(CpuState *cpu) {
    if (cpu->S < 0x1f20u || !ActorBridgeTargetBank(cpu->DB) ||
        !ActorBridgeActionWorkContext(cpu, 0x85u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x85d0cdu);
    }
    return ActorBridgeSceneOwners(cpu, 0x85d0cdu, Lufia2BattleApplyTargetAction, 2u);
}

RecompReturn Lufia2DecompBridge_C23D(CpuState *cpu) {
    if (cpu->S < 0x1f10u || !ActorBridgeActionWorkContext(cpu, 0x80u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c23du);
    }
    return ActorBridgeSceneOwners(cpu, 0x80c23du, Lufia2TextPrepareWindowWithServices,
                                  2u);
}

RecompReturn Lufia2DecompBridge_80C2A1(CpuState *cpu) {
    if (cpu->S < 0x1f10u || !ActorBridgeActionWorkContext(cpu, 0x80u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c2a1u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80c2a1u, Lufia2TextBuildSceneWindowInterior,
                                  2u);
}

RecompReturn Lufia2DecompBridge_80C1FD(CpuState *cpu) {
    if (cpu->S < 0x1f10u || !ActorBridgeActionWorkContext(cpu, 0x80u, 0u)) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80c1fdu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80c1fdu, Lufia2TextCloseWindowWithServices,
                                  2u);
}

RecompReturn Lufia2DecompBridge_80C557(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x80c557u, Lufia2TextWindowTileOffset, 2u);
}

RecompReturn Lufia2DecompBridge_80C52C(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x80c52cu, Lufia2TextDrawWindowBorderRow, 2u);
}

RecompReturn Lufia2DecompBridge_83E018(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x83e018u, Lufia2FieldPlaceSpawnedActor, 3u);
}

RecompReturn Lufia2DecompBridge_80D2FB(CpuState *cpu) {
    if (cpu->S < 0x1f10u || !ActorBridgeActionWorkContext(cpu, 0x80u, 0u)) {
        const ActorBridgeFrame f = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &f, 0x80d2fbu);
    }
    return ActorBridgeSceneOwners(cpu, 0x80d2fbu, Lufia2FieldProbeEventCell, 2u);
}

RecompReturn Lufia2DecompBridge_81EF93(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81ef93u, 0, Lufia2ApplyMenuItem,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81EFDF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81efdfu, 0, Lufia2CheckMenuItemTargets,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81F2E7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f2e7u, 0, Lufia2ApplyMenuSpell,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81F34D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f34du, 0, Lufia2CheckMenuSpellTargets,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81F3BC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81f3bcu, 0, Lufia2ConsumeMenuSpellMp,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85CD10(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cd10u, 0, Lufia2BattleCollectMenuEffectValues,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85CD5D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85cd5du, 0, Lufia2BattleReadMenuEffectValue,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85D711(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85d711u, 0, Lufia2BattleCheckAnyMenuEffectTarget,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85D7CE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85d7ceu, 0, Lufia2BattleCheckMenuEffectTarget,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85D82C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85d82cu, 0, Lufia2BattleBuildRequestedEffectMasks,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85D8B9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85d8b9u, 0, Lufia2BattleBuildTargetEffectMasks,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_85D8A1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85d8a1u, 0, Lufia2BattleAddRequestedStatChange,
                                  2u, BATTLE_BRIDGE_M0X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829C7D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829c7du, 0, Lufia2MenuConfigureListCursor,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829DF3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829df3u, 0, Lufia2MenuConfigureItemUseCursor,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_81FAF5(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x81faf5u, 0, Lufia2MenuResolveActionActor,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829D55(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829d55u, 0, Lufia2MenuPrepareItemSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829DD3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829dd3u, 0, Lufia2MenuRefreshItemSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F9C9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f9c9u, 0, Lufia2MenuBuildEquipmentAttributes,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829984(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829984u, 0, Lufia2MenuHighlightPartySelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82942F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82942fu, 0, Lufia2MenuSubtractExperienceTotals,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829073(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829073u, 0, Lufia2MenuBuildGlyphBuffer,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8290F6(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8290f6u, 0, Lufia2MenuPadGlyphRow,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829113(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829113u, 0, Lufia2MenuUploadGlyphBuffer,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829133(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829133u, 0, Lufia2MenuPlaceGlyphRectangle,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_828345(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x828345u, 0, Lufia2MenuSaveTileLayers,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EB7E2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb7e2u, 0, Lufia2FieldCountGroupObjectConditions,
                                  3u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AE6F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ae6fu, 0, Lufia2MenuHandleSpecialItemEffect,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AF00(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82af00u, 0, Lufia2MenuHandleSpecialSpellEffect,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AF8E(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82af8eu, 0, Lufia2MenuSelectWarpDestination,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AEF3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82aef3u, 0, Lufia2MenuConfirmSpecialItemUse,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82AF78(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82af78u, 0, Lufia2MenuConfirmSpecialSpellUse,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B14B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b14bu, 0, Lufia2MenuDispatchItemAction,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829449(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829449u, 0, Lufia2MenuCollectWarpDestinations,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B03F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b03fu, 0, Lufia2MenuRestoreWarpScroll,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82836A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82836au, 0, Lufia2MenuRestoreTileLayers,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B183(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b183u, 0, Lufia2MenuRunPendingItemActions,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B1CE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b1ceu, 0, Lufia2MenuCloseItemSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B1FE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b1feu, 0, Lufia2MenuUsePendingItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829E07(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829e07u, 0, Lufia2MenuConfigurePendingItemTargetCursor,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829E1B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829e1bu, 0, Lufia2MenuConfigurePendingItemActionCursor,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FD81(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fd81u, 0, Lufia2MenuShowRecordDescription,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FEC8(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fec8u, 0, Lufia2MenuLoadDescriptionRecord,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FDEC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fdecu, 0, Lufia2MenuDispatchDescriptionLayout,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FDF6(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fdf6u, 0, Lufia2MenuDrawFramedDescription,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FE44(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fe44u, 0, Lufia2MenuDrawInlineDescription,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FE98(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fe98u, 0, Lufia2MenuConfigureDescriptionMask,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FEC0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fec0u, 0, Lufia2MenuClearDescriptionMask,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FEFE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fefeu, 0, Lufia2MenuLoadSecondaryDescription,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FF3A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ff3au, 0, Lufia2MenuDrawCenteredRecordName,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FF99(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ff99u, 0, Lufia2MenuDrawSecondaryDescriptionPrompt,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8282E2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8282e2u, 0, Lufia2MenuDrawWindowDivider,
                                  2u, BATTLE_BRIDGE_M0X0, 0,
                                  BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_868333(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x868333u, 0, Lufia2SelectScreenResumeFade,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_868352(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x868352u, 0, Lufia2SelectScreenResumeInputWait,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8682CE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8682ceu, 0, Lufia2SelectScreenResumeObjectWait,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8682FC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8682fcu, 0, Lufia2SelectScreenRepeatObjectWait,
                                  2u, BATTLE_BRIDGE_M1X0, 0,
                                  BATTLE_BRIDGE_WRAM_BANK, 0);
}

RecompReturn Lufia2DecompBridge_80BC98(CpuState *cpu) {
    if (cpu->PB != 0x80u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->_flag_D || cpu->S < 0x1f10u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x80bc98u);
    }
    return ActorBridgeSceneOwners(cpu, 0x80bc98u, Lufia2TextPlaceActorFromScript, 2u);
}

RecompReturn Lufia2DecompBridge_82C647(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c647u);
    }
    return ActorBridgeWholeAnyWidth(cpu, 0x82c647u, Lufia2CapsuleClassifyFood, 2u);
}

RecompReturn Lufia2DecompBridge_82C672(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82c672u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82c672u, Lufia2CapsuleDrawFoodMessage, 2u);
}

RecompReturn Lufia2DecompBridge_82CBD2(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82cbd2u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82cbd2u, Lufia2CapsuleDescribeFood, 2u);
}

RecompReturn Lufia2DecompBridge_82CC58(CpuState *cpu) {
    if (cpu->PB != 0x82u || !cpu->m_flag || cpu->x_flag || cpu->D ||
        cpu->S < 0x1f00u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x82cc58u);
    }
    return ActorBridgeSceneOwners(cpu, 0x82cc58u, Lufia2CapsuleApplySpecialFood, 2u);
}

RecompReturn Lufia2DecompBridge_82C492(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c492u, Lufia2CapsuleGetFoodMeterAddress, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C53A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c53au, Lufia2CapsuleRefreshSelectedForm, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C5A2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c5a2u, 0, Lufia2CapsuleResetFoodCursor,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C683(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c683u, 0, Lufia2CapsuleFeedItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C784(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c784u, 0, Lufia2CapsuleToggleFoodCache,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C78E(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c78eu, 0, Lufia2CapsuleAdvanceForm,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C818(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c818u, 0, Lufia2CapsuleRefreshFormScene,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C85A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c85au, Lufia2CapsuleLoadFormPanel, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C8B7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c8b7u, Lufia2CapsuleClearFormPanel, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C8ED(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c8edu, 0, Lufia2CapsuleSaveFormTiles,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C929(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c929u, Lufia2CapsuleCopyFormTiles, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C93C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c93cu, Lufia2CapsuleClearFormTiles, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82C958(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82c958u, 0, Lufia2CapsuleAnimateFormTiles,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CB10(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cb10u, 0, Lufia2CapsuleAnimateFood,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CB55(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cb55u, 0, Lufia2CapsuleSpawnFoodParticle,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CB8C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cb8cu, 0, Lufia2CapsuleStepFoodParticles,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CBAF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cbafu, 0, Lufia2CapsuleApplyFoodUnit,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CA18(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ca18u, Lufia2CapsuleApplyFormMask, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CA62(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ca62u, 0, Lufia2CapsuleBuildFormMasks,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CAB1(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cab1u, 0, Lufia2CapsuleChooseFormMaskBit,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CB00(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cb00u, Lufia2CapsuleMergeFormMask, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CF9D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cf9du, 0, Lufia2CapsuleLoadFormGraphics,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CFD4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cfd4u, Lufia2CapsuleQueueFormTiles, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CFFD(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cffdu, Lufia2CapsuleQueueSavedFormTiles, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D026(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d026u, Lufia2CapsuleArrangeFormTiles, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D13A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d13au, 0, Lufia2CapsuleRefreshFoodMeter,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D150(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d150u, 0, Lufia2CapsuleDrawFoodMeter,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D378(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d378u, 0, Lufia2CapsuleTickFormPalette,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D397(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d397u, Lufia2CapsuleRotateFormColors, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D3AC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d3acu, Lufia2CapsuleAdvanceFormColors, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D443(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d443u, Lufia2CapsuleFadeFormColors, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D4A8(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d4a8u, Lufia2CapsuleBeginFormBlend, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D4D0(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d4d0u, 0, Lufia2CapsuleEndFormBlend,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D4EE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d4eeu, 0, Lufia2CapsuleLoadFormEffects,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D577(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d577u, 0, Lufia2CapsuleRunFormEffects,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D5E9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d5e9u, Lufia2CapsuleStepFormEffectAngle, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D5FC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d5fcu, 0, Lufia2CapsuleStepFormEffectLife,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D62F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d62fu, 0, Lufia2CapsulePlaceFormEffect,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D68C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d68cu, Lufia2CapsuleSignFormProduct, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82CF72(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82cf72u, Lufia2CapsuleLoadFormPalette, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D248(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d248u, Lufia2CapsulePlaceSelectedSprite, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D69C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d69cu, 0, Lufia2MenuShopSession,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D702(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d702u, 0, Lufia2MenuShopInitializeSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DA5C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82da5cu, 0, Lufia2MenuShopSelectListWindow,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DAD8(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dad8u, 0, Lufia2MenuShopRefreshSelection,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DA8F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82da8fu, 0, Lufia2MenuRunShop,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DB15(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82db15u, 0, Lufia2MenuSelectShopItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E012(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e012u, Lufia2MenuShopSelectListMode, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DC69(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dc69u, 0, Lufia2MenuShopDrawListPanel,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DC8A(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dc8au, 0, Lufia2MenuShopDrawVisibleRows,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E327(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e327u, 0, Lufia2MenuShopBuildSelectedList,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DEA4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dea4u, 0, Lufia2MenuShopSellItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DF45(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82df45u, 0, Lufia2MenuShopChoosePurchaseQuantity,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E167(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e167u, 0, Lufia2MenuShopLearnSpell,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E1DE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e1deu, 0, Lufia2MenuShopStoreLearnedSpell,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E20C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e20cu, 0, Lufia2MenuShopComputeQuantityPrice,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E22C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e22cu, 0, Lufia2MenuShopAdjustQuantity,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DE53(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82de53u, 0, Lufia2MenuShopShowMessage,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DE75(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82de75u, 0, Lufia2MenuShopFinishTransaction,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E02C(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e02cu, 0, Lufia2MenuShopBuyItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829867(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829867u, Lufia2MenuCreditExchangeBalance, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82989D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82989du, Lufia2MenuSpendExchangeBalance, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8298B5(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8298b5u, Lufia2MenuCheckExchangeBalance, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E0A9(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e0a9u, 0, Lufia2MenuShopBuyExchangeUnits,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E0E2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e0e2u, Lufia2MenuShopExchangeUnitValue, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DFC4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dfc4u, 0, Lufia2MenuShopPreviewEquipment,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E0F8(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e0f8u, 0, Lufia2MenuShopEquipPurchase,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E140(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e140u, 0, Lufia2MenuShopReceiveItem,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F97D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f97du, 0, Lufia2InventoryReplaceEquipment,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F9B6(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f9b6u, Lufia2InventorySwapEquipmentSlot, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D7BC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d7bcu, 0, Lufia2MenuShopPrepareItemPurchase,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D970(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d970u, 0, Lufia2MenuShopPrepareItemSale,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DA13(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82da13u, 0, Lufia2MenuShopCountEquippedItems,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D8A5(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d8a5u, 0, Lufia2MenuShopPrepareEquipmentPreview,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D8D4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d8d4u, 0, Lufia2MenuShopPrepareSpellPurchase,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E6FC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e6fcu, Lufia2MenuShopSelectSpell, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E709(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e709u, Lufia2MenuShopSelectItem, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DD18(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dd18u, 0, Lufia2MenuShopScrollUp,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DDAE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ddaeu, 0, Lufia2MenuShopScrollDown,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82FB04(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82fb04u, Lufia2InventoryCountOccupiedSlots, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F6F2(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f6f2u, Lufia2ItemFindEquipmentKind, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_828F5E(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x828f5eu, Lufia2MenuShiftScrollTilesUp, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829009(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829009u, Lufia2MenuShiftScrollTilesDown, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E522(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e522u, 0, Lufia2MenuShopPrepareEquipmentMembers,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E569(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e569u, 0, Lufia2MenuShopMarkEquipmentMember,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E586(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e586u, 0, Lufia2MenuShopRefreshEquipmentMembers,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E5AC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e5acu, 0, Lufia2MenuShopLoadEquippedComparison,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E691(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e691u, 0, Lufia2MenuShopPrepareSpellMembers,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E6BE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e6beu, 0, Lufia2MenuShopSetSpellMemberPose,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DCDD(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dcddu, 0, Lufia2MenuShopDrawItemRow,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DD06(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dd06u, 0, Lufia2MenuShopDrawSpellRow,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_829302(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x829302u, Lufia2MenuPartyMemberMask, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F76B(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f76bu, Lufia2PartyPrepareEquipmentComparison, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F7B4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f7b4u, Lufia2PartyAddEquipmentComparison, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F7FD(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f7fdu, Lufia2PartySubtractEquipmentComparison, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F8AF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f8afu, 0, Lufia2InventoryRemoveFlaggedEquipment,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F8DC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f8dcu, 0, Lufia2InventoryUnequipItem,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82F925(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82f925u, 0, Lufia2InventoryEquipItem,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82ECC7(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82ecc7u, 0, Lufia2SaveSelectCharacter,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82D79D(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82d79du, 0, Lufia2MenuShopConfigureItemCursor,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8299F3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8299f3u, 0, Lufia2MenuResetPartyAnimations,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8EE700(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8ee700u, Lufia2MenuClearSelectionScratch, 0,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DC10(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dc10u, 0, Lufia2MenuRefreshShopPreview,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DCA3(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dca3u, 0, Lufia2MenuDrawShopList,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8692DF(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8692dfu, 0, Lufia2WorldMapRunAlternateScene,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_86944E(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86944eu, 0, Lufia2WorldMapCheckSceneCancellation,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869537(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x869537u, 0, Lufia2WorldMapPrepareSceneColorBands,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8695CA(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8695cau, 0, Lufia2WorldMapCycleAlternateScenePalette,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_869618(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x869618u, Lufia2WorldMapBrightenScenePalette, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86CE68(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86ce68u, 0, Lufia2WorldMapLoadAlternateSceneGraphics,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86E9D4(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86e9d4u, 0, Lufia2WorldMapPrepareAlternateSceneSprites,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86EA43(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86ea43u, 0, Lufia2WorldMapDrawSceneSpriteCommands,
                                  2u, BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86EA58(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86ea58u, 0, Lufia2WorldMapExpandSceneSpriteCommand,
                                  2u, BATTLE_BRIDGE_M0X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86EB11(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86eb11u, 0, Lufia2WorldMapEmitLargeSceneSprite,
                                  2u, BATTLE_BRIDGE_M0X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86EB52(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x86eb52u, 0, Lufia2WorldMapEmitSmallSceneSprite,
                                  2u, BATTLE_BRIDGE_M0X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_8EB4FC(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb4fcu, Lufia2AlternateDisplayMeasureText, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, -1, 0);
}

RecompReturn Lufia2DecompBridge_8EB50F(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb50fu, Lufia2AlternateDisplayDrawText, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, -1, 0);
}

RecompReturn Lufia2DecompBridge_8EB534(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb534u, Lufia2AlternateDisplayDrawTileStrip, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, -1, 0);
}

RecompReturn Lufia2DecompBridge_8EB557(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb557u, Lufia2AlternateDisplayUploadTilemap, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, -1, 0);
}

RecompReturn Lufia2DecompBridge_8EB578(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb578u, Lufia2AlternateDisplayAdvanceRow, 0,
                                  2u, BATTLE_BRIDGE_M1X0, 0, -1, 0);
}

RecompReturn Lufia2DecompBridge_8EB2EE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x8eb2eeu, 0, Lufia2RunAlternateDisplay,
                                  3u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_828636(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x828636u, 0, Lufia2MenuDrawSpellShopRows,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82E718(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82e718u, Lufia2MenuShopSelectInventoryItem, 0,
                                  2u, BATTLE_BRIDGE_M0X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82DC49(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82dc49u, 0, Lufia2MenuShopReadSelectedRecord,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B072(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b072u, 0, Lufia2MenuDrawWarpScrollRow,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B080(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b080u, 0, Lufia2MenuScrollWarpDestinationsUp,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_82B0BE(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x82b0beu, 0, Lufia2MenuScrollWarpDestinationsDown,
                                  2u, BATTLE_BRIDGE_M1X0, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_8284D5(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x8284d5u, Lufia2MenuDrawInventoryRows, 2u);
}

RecompReturn Lufia2DecompBridge_828526(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x828526u, Lufia2MenuDrawSpellRows, 2u);
}

RecompReturn Lufia2DecompBridge_8286C4(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x8286c4u, Lufia2MenuDrawScenarioRows, 2u);
}

RecompReturn Lufia2DecompBridge_828680(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x828680u, Lufia2MenuDrawPointerRows, 2u);
}

RecompReturn Lufia2DecompBridge_82E433(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82e433u, Lufia2MenuBuildOwnedItemList, 2u);
}

RecompReturn Lufia2DecompBridge_82E46B(CpuState *cpu) {
    return ActorBridgeWholeX16(cpu, 0x82e46bu, Lufia2MenuCountShopWordList, 2u);
}

RecompReturn Lufia2DecompBridge_82E484(CpuState *cpu) {
    return ActorBridgeWholeX16(cpu, 0x82e484u, Lufia2MenuCountShopByteList, 2u);
}

RecompReturn Lufia2DecompBridge_82FC7D(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82fc7du, Lufia2MenuCountOwnedItem, 2u);
}

RecompReturn Lufia2DecompBridge_8298C7(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x8298c7u, Lufia2MenuScaleShopPrice, 2u);
}

RecompReturn Lufia2DecompBridge_8291AB(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x8291abu, Lufia2MenuSplitSmallNumber, 2u);
}

RecompReturn Lufia2DecompBridge_829169(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x829169u, Lufia2MenuDrawSmallNumber, 2u);
}

RecompReturn Lufia2DecompBridge_829189(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x829189u, Lufia2MenuDrawSmallNumberFull, 2u);
}

RecompReturn Lufia2DecompBridge_82F87D(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82f87du, Lufia2MenuCountFlaggedEquipment, 2u);
}

RecompReturn Lufia2DecompBridge_82E4D5(CpuState *cpu) {
    return ActorBridgeWholeX16(cpu, 0x82e4d5u, Lufia2MenuDrawPartyStats, 2u);
}

RecompReturn Lufia2DecompBridge_82E504(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82e504u, Lufia2MenuDrawPartyStatValues, 2u);
}

RecompReturn Lufia2DecompBridge_82B67C(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82b67cu,
        Lufia2MenuReorderParty, 2u);
}

RecompReturn Lufia2DecompBridge_82B7B2(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82b7b2u,
        Lufia2MenuOpenEquipment, 2u);
}

RecompReturn Lufia2DecompBridge_829F0C(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x829f0cu,
        Lufia2MenuPrepareEquipmentPortrait, 2u);
}

RecompReturn Lufia2DecompBridge_829FB0(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x829fb0u,
        Lufia2MenuPresentEquipmentList, 2u);
}

RecompReturn Lufia2DecompBridge_82BD48(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bd48u,
        Lufia2MenuBuildEquipmentGroups, 2u);
}

RecompReturn Lufia2DecompBridge_82BD9B(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82bd9bu,
        Lufia2MenuGroupEquipmentOffset, 2u);
}

RecompReturn Lufia2DecompBridge_82BDDB(CpuState *cpu) {
    return ActorBridgeWholeAnyWidth(cpu, 0x82bddbu,
        Lufia2MenuFinishEquipmentGroups, 2u);
}


RecompReturn Lufia2DecompBridge_82BB6E(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82bb6eu,
        Lufia2MenuWeightEquipmentStats, 2u);
}

RecompReturn Lufia2DecompBridge_82BB91(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bb91u,
        Lufia2MenuUnequipSelectedSlot, 2u);
}

RecompReturn Lufia2DecompBridge_82BBE8(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bbe8u,
        Lufia2MenuUnequipAllSlots, 2u);
}

RecompReturn Lufia2DecompBridge_82BC46(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bc46u,
        Lufia2MenuDiscardSelectedSlot, 2u);
}


RecompReturn Lufia2DecompBridge_82BCB6(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bcb6u,
        Lufia2MenuDrawEquipmentRow, 2u);
}

RecompReturn Lufia2DecompBridge_828579(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x828579u,
        Lufia2MenuRenderEquipmentListRows, 2u);
}

RecompReturn Lufia2DecompBridge_82859B(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82859bu,
        Lufia2MenuRenderEquipmentListRow, 2u);
}

RecompReturn Lufia2DecompBridge_829A14(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x829a14u,
        Lufia2MenuCycleEquipmentMember, 2u);
}

RecompReturn Lufia2DecompBridge_82BE04(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82be04u,
        Lufia2MenuPrepareEquipmentList, 2u);
}

RecompReturn Lufia2DecompBridge_82BE71(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82be71u,
        Lufia2MenuLoadEquipmentSlotStats, 2u);
}

RecompReturn Lufia2DecompBridge_82BED9(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bed9u,
        Lufia2MenuClassifyEquipmentListSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82BEF0(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bef0u,
        Lufia2MenuResolveEquipmentListSelection, 2u);
}

RecompReturn Lufia2DecompBridge_829FFB(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x829ffbu,
        Lufia2MenuConfigureEquipmentPicker, 2u);
}

RecompReturn Lufia2DecompBridge_82F864(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82f864u,
        Lufia2MenuPrepareEquipmentPreview, 2u);
}

RecompReturn Lufia2DecompBridge_82B863(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82b863u,
        Lufia2MenuRunEquipmentSlots, 2u);
}

RecompReturn Lufia2DecompBridge_82831A(CpuState *cpu) {
    return ActorBridgeWholeM0X0(cpu, 0x82831au,
        Lufia2MenuDrawSequentialTileBlock, 2u);
}

RecompReturn Lufia2DecompBridge_82E319(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x82e319u,
        Lufia2MenuRegisterShopList, 2u);
}

RecompReturn Lufia2DecompBridge_82B052(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82b052u,
        Lufia2MenuRedrawWarpRows, 2u);
}

RecompReturn Lufia2DecompBridge_82DBC9(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82dbc9u,
        Lufia2MenuSelectShopOperation, 2u);
}

RecompReturn Lufia2DecompBridge_82DFA5(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82dfa5u,
        Lufia2MenuPurchaseShopSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82E290(CpuState *cpu) {
    return ActorBridgeWholeX16(cpu, 0x82e290u,
        Lufia2MenuPayShopSelection, 2u);
}

RecompReturn Lufia2DecompBridge_83E0FC(CpuState *cpu) {
    return ActorBridgeWholeM1X16(cpu, 0x83e0fcu,
        Lufia2ObjectRunScript, 2u);
}

RecompReturn Lufia2DecompBridge_80CC35(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->PB != 0x80u || cpu->D ||
        !cpu->m_flag || cpu->x_flag || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, 0xffffffffu};
        return ActorBridgeFallback(cpu, &frame, 0x80cc35u);
    }
    return ActorBridgeOwnedCalls(cpu, Lufia2FieldRunEventScript, 2u);
}

RecompReturn Lufia2DecompBridge_B452(CpuState *cpu) {
    return ActorBridgeBattleEntry(cpu, 0x85b452u, 0,
        Lufia2BattleScriptWithChildren, 3u, BATTLE_BRIDGE_M1X0,
        1, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_86B5DF(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b5dfu,
        Lufia2WorldMapTrackRisingSceneActor, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B637(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b637u,
        Lufia2WorldMapStopTimedSceneSound, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BD47(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bd47u,
        Lufia2WorldMapStartPairedScenePulse, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BD77(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bd77u,
        Lufia2WorldMapSetPairedSceneAnimation27, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BD7E(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bd7eu,
        Lufia2WorldMapSetPairedSceneAnimation26, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BD85(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bd85u,
        Lufia2WorldMapSetPairedSceneAnimation25, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BE2A(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86be2au,
        Lufia2WorldMapSetSceneSoundMode2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C1CC(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c1ccu,
        Lufia2WorldMapSetSceneSoundMode1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B60D(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b60du,
        Lufia2WorldMapStartRisingSceneActor, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BEE6(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bee6u,
        Lufia2WorldMapStepTrailingSceneParticles, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C0AC(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86c0acu,
        Lufia2WorldMapStepOrbitingSceneParticles, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B24C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b24cu,
        Lufia2WorldMapReduceSceneSparkEmission, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B250(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b250u,
        Lufia2WorldMapBeginSceneColorRise, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B259(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b259u,
        Lufia2WorldMapBeginSceneColorFall, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B2E7(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b2e7u,
        Lufia2WorldMapPrepareSceneWindowCollapse, 2u, false);
}

RecompReturn Lufia2DecompBridge_86BF3B(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86bf3bu,
        Lufia2WorldMapSpawnTrailingSceneParticle, 2u, false);
}

RecompReturn Lufia2DecompBridge_86C1B4(CpuState *cpu) {
    if (cpu->PB != 0x86u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, 0x86c1b4u);
    }
    return ActorBridgeBattleEntry(cpu, 0x86c1b4u,
        Lufia2WorldMapFindOrbitSceneParticleSlot, NULL, 2u,
        BATTLE_BRIDGE_M1X0, 0, 0x86, 0);
}

RecompReturn Lufia2DecompBridge_86B27C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b27cu,
        Lufia2WorldMapLoadSceneMirroredWindows0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B292(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b292u,
        Lufia2WorldMapLoadSceneMirroredWindows1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B297(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b297u,
        Lufia2WorldMapLoadSceneMirroredWindows2, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B29C(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b29cu,
        Lufia2WorldMapSubmitSceneSlopedWindow0, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B2B1(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b2b1u,
        Lufia2WorldMapSubmitSceneSlopedWindow1, 2u, false);
}

RecompReturn Lufia2DecompBridge_86B2B6(CpuState *cpu) {
    return ActorBridgeWorldSequence(cpu, 0x86b2b6u,
        Lufia2WorldMapSubmitSceneSlopedWindow2, 2u, false);
}

static RecompReturn ActorBridgeSpecialMenuLeaf(CpuState *cpu, uint32_t entry,
                                               ActorWholeFunction function,
                                               BattleBridgeWidth width) {
    if (cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeBattleEntry(cpu, entry, function, NULL, 2u,
        width, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_84A4DD(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84a4ddu,
        Lufia2SpecialMenuClearTilemaps, BATTLE_BRIDGE_M1X0);
}

RecompReturn Lufia2DecompBridge_84A4FD(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84a4fdu,
        Lufia2SpecialMenuHideSprites, BATTLE_BRIDGE_M1X0);
}

RecompReturn Lufia2DecompBridge_84A5DF(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84a5dfu,
        Lufia2SpecialMenuMultiplyWords, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B4A7(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b4a7u,
        Lufia2CardGameBuildDeck, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AF1C(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84af1cu,
        Lufia2CardGameDealCard, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AE5E(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84ae5eu,
        Lufia2CardGameScoreHand, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AB61(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84ab61u,
        Lufia2CardGameRankHand, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B16D(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b16du,
        Lufia2CardGameSwapSpriteBlocks, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B274(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b274u,
        Lufia2CardGameHideCardSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B295(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b295u,
        Lufia2CardGameLocateCardTilemap, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B2BC(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b2bcu,
        Lufia2CardGameBuildCardTiles, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B387(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b387u,
        Lufia2CardGameDrawCardTilemap, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B429(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b429u,
        Lufia2CardGameEmitCardSprite, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B45B(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b45bu,
        Lufia2CardGameMoveCardSprite, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B49B(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84b49bu,
        Lufia2CardGameRetileCardSprite, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AE30(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84ae30u,
        Lufia2CardGameDrawScoreDigit, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BA40(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84ba40u,
        Lufia2PokerSumStakes, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C3B4(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84c3b4u,
        Lufia2PokerPrepareHandRanks, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C55B(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84c55bu,
        Lufia2PokerCountCardMatches, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BB2F(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84bb2fu,
        Lufia2PokerClassifyHand, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C6E8(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84c6e8u,
        Lufia2PokerDrawSeatResult, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BBDA(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84bbdau,
        Lufia2PokerLocateResultText, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BD83(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84bd83u,
        Lufia2PokerAcceptOpponentStake, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BEE4(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84bee4u,
        Lufia2PokerLocateStakeSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C22C(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84c22cu,
        Lufia2CardGameClearStakeSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C299(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84c299u,
        Lufia2CardGameDrawStakeTiles, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C2CE(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84c2ceu,
        Lufia2CardGameDrawStakeSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84CBE2(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84cbe2u,
        Lufia2BoardGameCountCompletedLines, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84CD95(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84cd95u,
        Lufia2BoardGameDrawDigitTiles, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D09B(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84d09bu,
        Lufia2BoardGameLiftCellTiles, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D0CB(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84d0cbu,
        Lufia2BoardGamePlaceCellTiles, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D0EF(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84d0efu,
        Lufia2BoardGameRotateSelectedGroup, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D17A(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84d17au,
        Lufia2BoardGameDrawCells, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D414(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84d414u,
        Lufia2BoardGameDrawStakeDigitSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D538(CpuState *cpu) {
    return ActorBridgeSpecialMenuLeaf(cpu, 0x84d538u,
        Lufia2BoardGameRebuildCellLocations, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

static RecompReturn ActorBridgeSpecialMenuParent(CpuState *cpu, uint32_t entry,
                                                 BattleControlFunction function,
                                                 BattleBridgeWidth width) {
    if (cpu->S < 0x1f20u || cpu->S > 0x1ffcu) {
        const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
        return ActorBridgeFallback(cpu, &frame, entry);
    }
    return ActorBridgeBattleEntry(cpu, entry, NULL, function, 2u,
        width, 0, BATTLE_BRIDGE_ANY_BANK, 0);
}

RecompReturn Lufia2DecompBridge_84A455(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84a455u,
        Lufia2SpecialMenuInitializeVideo, BATTLE_BRIDGE_M1X0);
}

RecompReturn Lufia2DecompBridge_84B4D9(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84b4d9u,
        Lufia2CardGameShuffleDeck, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AAB5(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84aab5u,
        Lufia2CardGameShowHandResult, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AB01(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84ab01u,
        Lufia2CardGamePrepareResultSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AD9D(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84ad9du,
        Lufia2CardGamePreparePayout, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C350(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c350u,
        Lufia2CardGameReadStakeCommand, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B3C9(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84b3c9u,
        Lufia2CardGameEmitCardSprites, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B43B(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84b43bu,
        Lufia2CardGameMoveCardSprites, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B472(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84b472u,
        Lufia2CardGameRetileCardSprites, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84B523(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84b523u,
        Lufia2CardGameLoadSceneTilemap, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84AEE1(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84aee1u,
        Lufia2CardGameDrawRoundValues, BATTLE_BRIDGE_M1X0);
}

RecompReturn Lufia2DecompBridge_84AF11(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84af11u,
        Lufia2CardGameDrawBalance, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C1E8(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c1e8u,
        Lufia2CardGameRefundStake, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C20A(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c20au,
        Lufia2CardGameChargeStake, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C26F(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c26fu,
        Lufia2CardGameDrawAcceptedStake, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C578(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c578u,
        Lufia2PokerDrawStakes, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C69C(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c69cu,
        Lufia2PokerResetRound, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C819(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c819u,
        Lufia2PokerDrawCarryover, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84C42A(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84c42au,
        Lufia2PokerMeasureHand, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BBF1(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84bbf1u,
        Lufia2PokerRankHands, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BA1F(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84ba1fu,
        Lufia2PokerRefreshSeatSprites, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BAF1(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84baf1u,
        Lufia2PokerDrawHandResult, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BDC2(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84bdc2u,
        Lufia2PokerChooseOpponentAction, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84BD96(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84bd96u,
        Lufia2PokerDrawOpponentStake, BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D684(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84d684u,
        Lufia2BoardGameResetRound, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D70A(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84d70au,
        Lufia2BoardGameLoadTilemaps, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D765(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84d765u,
        Lufia2BoardGameInitialize, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D860(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84d860u,
        Lufia2BoardGameShuffleCells, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D892(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84d892u,
        Lufia2BoardGameShuffleDrawOrder, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84CCD3(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84ccd3u,
        Lufia2BoardGameDrawNextCell, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84CDA8(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84cda8u,
        Lufia2BoardGameUpdateDrawAnimation, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84CD5F(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84cd5fu,
        Lufia2BoardGameDrawRemainingCount, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84D558(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84d558u,
        Lufia2BoardGameAnimateParticles, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_84A421(CpuState *cpu) {
    return ActorBridgeSpecialMenuParent(cpu, 0x84a421u,
        Lufia2BoardGameCreditPayoutStep, cpu->m_flag ? BATTLE_BRIDGE_M1X0 : BATTLE_BRIDGE_M0X0);
}

RecompReturn Lufia2DecompBridge_82B7E5(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82b7e5u,
        Lufia2MenuRunEquipmentSelection, 2u);
}

RecompReturn Lufia2DecompBridge_82BA0C(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82ba0cu,
        Lufia2MenuRefreshEquipmentPicker, 2u);
}

RecompReturn Lufia2DecompBridge_82BA65(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82ba65u,
        Lufia2MenuEquipStrongestItems, 2u);
}

RecompReturn Lufia2DecompBridge_82BC96(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bc96u,
        Lufia2MenuDrawEquipmentRows, 2u);
}

RecompReturn Lufia2DecompBridge_82BCC4(CpuState *cpu) {
    return ActorBridgeSceneOwners(cpu, 0x82bcc4u,
        Lufia2MenuScrollEquipmentUp, 2u);
}
