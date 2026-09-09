#ifndef LUFIA2_BRIDGE_AUDIT_H
#define LUFIA2_BRIDGE_AUDIT_H
#include <stdint.h>

enum L2BAPhase {
    L2BA_CHECKS, L2BA_QUIESCENCE, L2BA_DISPATCH, L2BA_OPCODE,
    L2BA_BUS, L2BA_RETIRE, L2BA_SNES_SYNC, L2BA_CART_SYNC,
    L2BA_APU_OTHER, L2BA_APU_LOCK, L2BA_APU_SYNC, L2BA_STATE_SYNC,
    L2BA_AOT, L2BA_TAIL, L2BA_PHASE_COUNT
};
extern int l2ba_sample_active;
extern int l2ba_recording;
void L2BALoop(uint32_t pc, unsigned mx, int outer_scheduler);
void L2BAFinish(void);
void L2BAMark(int phase);
int L2BAEnter(int phase);
void L2BALeave(int previous);
void L2BAToggle(void);
void L2BAFrameBegin(void);
void L2BAFrameEnd(void);
int L2BASelfTest(void);

/* Scopes nest by restoring the previous phase. No clock is queried on an
 * unsampled instruction. These macros never evaluate CALL more than once. */
#define L2BA_CALL(PHASE, CALL) do { \
    if (l2ba_sample_active) { \
        int l2ba_previous = L2BAEnter(PHASE); \
        CALL; \
        L2BALeave(l2ba_previous); \
    } else { CALL; } \
} while (0)
#define L2BA_MARK(PHASE, OUTER) do { \
    if (l2ba_sample_active && (OUTER)) L2BAMark(PHASE); \
} while (0)
#endif
