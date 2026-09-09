#ifndef LUFIA2_PATCH_FRAME_WAIT_H
#define LUFIA2_PATCH_FRAME_WAIT_H

#include "snes/interp816.h"

/* Plan a batch of complete CMP/BEQ-taken pairs without consuming the final
 * pair before the scheduler deadline. The caller has already established
 * deadline > current and supplies its remaining interpreter-step budget.
 * Keeping one reachable pair on the reference path preserves the original
 * deadline overshoot and resume-PC decision at an instruction boundary. */
static inline uint64_t L2FrameWaitBatchPairs(
        uint64_t current, uint64_t deadline, uint64_t pair_master,
        uint64_t remaining_steps) {
    if (deadline <= current || pair_master == 0 || remaining_steps <= 2)
        return 0;
    const uint64_t remaining_master = deadline - current;
    const uint64_t reachable_pairs =
        (remaining_master - 1u) / pair_master;
    uint64_t pairs = reachable_pairs > 1u ? reachable_pairs - 1u : 0u;
    const uint64_t step_pairs = (remaining_steps - 2u) / 2u;
    if (pairs > step_pairs)
        pairs = step_pairs;
    return pairs;
}

/* Architectural state after one or more equal CMP $40 / BEQ-taken pairs.
 * A, X/Y, stack, banks, D, I/D/V, widths and emulation mode are untouched. */
static inline void L2FrameWaitApplyEqualBatchState(
        Interp816 *in, uint16_t cmp_pc) {
    in->c = 1;
    in->z = 1;
    in->n = 0;
    in->pc = cmp_pc;
    in->cyclesUsed = 3;
}

/* A basic-block rewrite, not a new function root. The owning bridge still
 * retires exactly ONE guest instruction and performs all timing/scheduling.
 * Keep this payload independent of CpuState, PPU, audio and host platform. */
static inline bool L2FrameWaitEligible(const Interp816 *in, bool outer_scheduler) {
    return outer_scheduler && in->k == 0x83 &&
        (in->pc == 0x900e || in->pc == 0x9010) && in->mf && !in->e &&
        in->dp == 0 && !in->waiting && !in->stopped &&
        !in->nmiWanted && !in->irqWanted && in->read != NULL;
}

/* These reads intentionally use the SAME callback as interp816. This keeps
 * bus pricing, open bus, read epochs and access order in the existing core.
 * Eligibility and the immutable ROM signature must be checked before entry. */
static inline unsigned L2FrameWaitStep(Interp816 *in) {
    extern uint32_t g_interp816_cur_pc;
    uint16_t pc = in->pc;
    in->cyclesUsed = 0;
    in->pc++;
    (void)in->read(in->mem, ((uint32_t)in->k << 16) | pc);
    g_interp816_cur_pc = ((uint32_t)in->k << 16) | pc;
    in->cyclesUsed = pc == 0x900e ? 3 : 2;
    uint16_t operand_pc = in->pc++;
    uint8_t operand = in->read(in->mem, ((uint32_t)in->k << 16) | operand_pc);
    if (pc == 0x900e) {
        uint8_t value = in->read(in->mem, operand); /* D=0, M=1 */
        unsigned a = in->a & 0xffu;
        uint8_t difference = (uint8_t)(a - value);
        in->c = a >= value;
        in->z = difference == 0;
        in->n = (difference & 0x80u) != 0;
    } else if (in->z) {
        ++in->cyclesUsed;
        in->pc = (uint16_t)(in->pc + (int8_t)operand);
    }
    return in->cyclesUsed;
}

#endif
