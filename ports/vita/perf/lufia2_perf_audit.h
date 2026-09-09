#ifndef LUFIA2_PERF_AUDIT_H
#define LUFIA2_PERF_AUDIT_H
#include <stdint.h>

enum L2PerfCategory {
    L2_FRAME, L2_GUEST, L2_INTERP, L2_AOT, L2_APU_FLUSH, L2_APU_SYNC,
    L2_APU_LOCK, L2_APU_EXEC, L2_BUS, L2_QUIESCENCE, L2_STATE_SYNC, L2_PC,
    L2_VIDEO, L2_PPU, L2_CAPTURE, L2_GXM, L2_FALLBACK, L2_PRESENT,
    L2_EVENTS, L2_INPUT, L2_PACING, L2_MISC, L2_CATEGORY_COUNT
};

#ifdef LUFIA2_ENABLE_PERF_AUDIT
typedef struct L2PerfScope {
    struct L2PerfScope *parent;
    uint64_t start, children;
    int category, hot;
    unsigned active;
} L2PerfScope;
void L2PerfEnter(L2PerfScope *scope, int category, uint32_t pc, unsigned mx);
void L2PerfLeave(L2PerfScope *scope);
/* Only the bridge/game thread reads this flag through the sampling macro. */
extern int l2_perf_audit_active_main;
static inline int L2PerfSample(unsigned *counter, unsigned period) {
    if (!l2_perf_audit_active_main || --*counter != 0) return 0;
    *counter = period;
    return 1;
}
void L2PerfCall(uint32_t site, uint32_t target, unsigned mx, unsigned opcode,
                int has_body, int bounce_ok);
void L2PerfAotReturn(int result);
void L2PerfInit(void);
void L2PerfFrameBegin(unsigned frame);
void L2PerfFrameEnd(void);
void L2PerfFrameCancel(void);
void L2PerfAbort(void);

/* GCC cleanup runs on return/continue as well as normal block exit. These
 * macros are private to the Vita audit, never part of the platform ABI. */
#define L2_SCOPE(name, cat) \
    L2PerfScope name __attribute__((cleanup(L2PerfLeave))) = {0}; \
    L2PerfEnter(&name, cat, 0, 0)
#define L2_SAMPLE_SCOPE(name, cat, period, pc, mx) \
    static unsigned name##_counter = period; \
    L2PerfScope name __attribute__((cleanup(L2PerfLeave))) = {0}; \
    if (L2PerfSample(&name##_counter, period)) L2PerfEnter(&name, cat, pc, mx)
#define L2_AOT_SCOPE(name, pc, mx) \
    L2PerfScope name __attribute__((cleanup(L2PerfLeave))) = {0}; \
    L2PerfEnter(&name, L2_AOT, pc, mx)
#else
#define L2_SCOPE(name, cat) ((void)0)
#define L2_SAMPLE_SCOPE(name, cat, period, pc, mx) ((void)0)
#define L2_AOT_SCOPE(name, pc, mx) ((void)0)
#define L2PerfLeave(scope) ((void)0)
#define L2PerfCall(site, target, mx, opcode, body, bounce) ((void)0)
#define L2PerfAotReturn(result) ((void)0)
#define L2PerfInit() ((void)0)
#define L2PerfFrameBegin(frame) ((void)0)
#define L2PerfFrameEnd() ((void)0)
#define L2PerfFrameCancel() ((void)0)
#define L2PerfAbort() ((void)0)
#endif
#endif
