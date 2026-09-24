#include <stdint.h>

#include "cpu_state.h"
#include "lufia2/actor_frontend.h"

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
    const CpuState *cpu, Lufia2ActorFrontendCpu *out) {
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
    CpuState *cpu, const Lufia2ActorFrontendCpu *in) {
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
    const Lufia2ActorFrontendMemory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2ActorFrontendCpu state;

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
    const Lufia2ActorFrontendMemory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2ActorFrontendCpu state;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83f9d4u);
    ActorBridgeLoad(cpu, &state);
    Lufia2ActorResolveMapCellOffset(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 2, 0x83f9edu);
}

RecompReturn Lufia2DecompBridge_FB12(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2ActorFrontendMemory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2ActorFrontendCpu state;
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
    const Lufia2ActorFrontendMemory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2ActorFrontendCpu state;

    if (!ActorBridgeSupported(cpu, 0, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83fb71u);
    ActorBridgeLoad(cpu, &state);
    Lufia2ActorReadMapCellValue(&memory, &state);
    ActorBridgeStore(cpu, &state);
    return ActorBridgeReturn(cpu, &frame, 3, 0x83fb8au);
}

typedef Lufia2ActorPrimaryUpdateResult (*ActorWholeFunction)(
    const Lufia2ActorFrontendMemory *memory, Lufia2ActorFrontendCpu *cpu);

/* Whole JSR routine with exact LLE boundaries. */
static RecompReturn ActorBridgeWhole(
    CpuState *cpu, uint32_t entry_pc24, ActorWholeFunction run,
    uint8_t frame_size) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const Lufia2ActorFrontendMemory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    Lufia2ActorFrontendCpu state;
    Lufia2ActorPrimaryUpdateResult result;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, entry_pc24);
    ActorBridgeLoad(cpu, &state);
    result = run(&memory, &state);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_BOUNDARY) {
        /* Exact ROM state at result.pc; LLE finishes the RTS. */
        return interp_tier_dispatch_tail(
            cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    return ActorBridgeReturn(cpu, &frame, frame_size, result.pc);
}

RecompReturn Lufia2DecompBridge_C7F8(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83c7f8u, Lufia2ActorPrimaryUpdate, 2);
}

RecompReturn Lufia2DecompBridge_D508(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x83d508u, Lufia2ActorSecondaryUpdate, 2);
}

typedef struct ActorSlotsCall {
    CpuState *cpu;
    RecompReturn unwound;
} ActorSlotsCall;

/* BB93 child through the runtime dispatcher. */
static uint8_t ActorBridgeSlotChild(
    void *context,
    Lufia2ActorFrontendCpu *state,
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
    const Lufia2ActorFrontendMemory memory = {
        ActorBridgeRead, ActorBridgeWrite, cpu};
    ActorSlotsCall call;
    Lufia2ActorFrontendCpu state;
    Lufia2ActorPrimaryUpdateResult result;

    if (!ActorBridgeSupported(cpu, 1, 0))
        return ActorBridgeFallback(cpu, &frame, 0x83bb93u);
    call.cpu = cpu;
    call.unwound = RECOMP_RETURN_NORMAL;
    ActorBridgeLoad(cpu, &state);
    result = Lufia2UpdateActorSlots(
        &memory, &state, ActorBridgeSlotChild, &call);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_CHILD_UNWOUND)
        return (RecompReturn)((int)call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow == LUFIA2_ACTOR_PRIMARY_UPDATE_BOUNDARY)
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

RecompReturn Lufia2DecompBridge_8DC5(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x858dc5u, Lufia2BattleNmiUploads, 3);
}

RecompReturn Lufia2DecompBridge_CEF6(CpuState *cpu) {
    return ActorBridgeWhole(cpu, 0x86cef6u, Lufia2WorldMapNmiUploads, 3);
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
