#ifndef LUFIA2_FRAME_RETURNS_H
#define LUFIA2_FRAME_RETURNS_H

#include <stdint.h>
#include <stdbool.h>

/* Host batching only; original opcode and device clocks remain exact. */
#define LUFIA2_FRAME_POLL_MAX_OPCODES 1024u
#define LUFIA2_FIELD_CALL_OPCODE_BUDGET 32u
#define LUFIA2_FIELD_TICK_OPCODES 8u
#define LUFIA2_FIELD_EFFECT_OPCODES 15u
#define LUFIA2_FIELD_TIMER_MAX_OPCODES 7u
#define LUFIA2_EVENT_SLOT_OPCODE_BUDGET 128u
#define LUFIA2_FIELD_IDLE_OPCODE_BUDGET 64u
#define LUFIA2_FIELD_STAIR_OPCODE_BUDGET 512u
#define LUFIA2_FIELD_RECTANGLE_OPCODE_BUDGET 512u
#define LUFIA2_FIELD_MAP_OPCODE_BUDGET 64u
#define LUFIA2_FIELD_VEHICLE_OPCODE_BUDGET 96u
#define LUFIA2_FIELD_ITEM_REGION_OPCODE_BUDGET 384u
#define LUFIA2_BATTLE_FIXED_OAM_OPCODE_BUDGET 840u
#define LUFIA2_BATTLE_FIXED_OAM_APPEND_OPCODE_BUDGET 768u
#define LUFIA2_BATTLE_FIXED_TILE_OPCODE_BUDGET 320u
#define LUFIA2_FIELD_TRIGGER_OPCODE_BUDGET 256u
#define LUFIA2_FIELD_TRIGGER_OWN_OPCODES 24u
#define LUFIA2_FIELD_REQUEST_OPCODE_BUDGET 32u

struct CpuState;

/* Exact NMI-counter pairs with verified semantic continuations. This
 * classifies an entry only: it grants no permission to bypass hooks or clocks.
 * Keep all native pair dispatch and timing gates on this same list. */
static inline bool Lufia2DecompFramePollIsCounterPair(uint32_t entry) {
    switch (entry) {
    case 0x8293c8u:
    case 0x83900eu:
    case 0x8eb552u:
    case 0x868b4eu:
    case 0x869752u:
    case 0x848d4fu:
    case 0x80bf0du:
    case 0x80937du:
    case 0x848365u:
    case 0x84a5aeu:
    case 0x85851eu:
    case 0x85ec96u:
    case 0x85ecb9u:
    case 0x8eb28au:
    case 0x8ec2c6u:
        return true;
    default:
        return false;
    }
}


void Lufia2DecompInstallFrameReturnHooks(void);
bool Lufia2DecompCommandTileHookOwned(
    uint32_t pc, void (*hook)(struct CpuState *, uint32_t));
bool Lufia2DecompFramePollHookIsNative(
    void (*hook)(struct CpuState *, uint32_t));
uint64_t Lufia2DecompFrameReturnDeadline(void);
bool Lufia2DecompFramePollBatchSupported(uint32_t entry);
bool Lufia2DecompFramePollTryIdle(struct CpuState *cpu, uint32_t entry);
uint32_t Lufia2DecompFramePollReferenceClock(uint32_t enabled);
uint64_t Lufia2DecompNativeIdleSteps(void);
uint32_t Lufia2DecompNativeIdleDiagnostics(uint64_t out[16],uint64_t contexts[16]);
void Lufia2DecompFramePollProbe(const struct CpuState *cpu,uint32_t entry);
bool Lufia2DecompPollFetchOpcode(struct CpuState *cpu, uint32_t pc, uint32_t fetched);
/* Local to one admitted eventless semantic span. It grants only immutable
 * LoROM instruction bytes and their ordinary fetch latches, not clocks/data
 * accesses. Discard before returning/rechecking the caller's device window;
 * no mapping/observer/lifecycle change may occur while the token is used. */
typedef struct Lufia2DecompRomOpcodeSpan {
    struct CpuState *cpu;
    void *cart;
    uint32_t base;
    uint8_t count, bytes[32];
} Lufia2DecompRomOpcodeSpan;
bool Lufia2DecompPrepareRomOpcodeSpan(struct CpuState *cpu,uint32_t base,
    unsigned count,Lufia2DecompRomOpcodeSpan *span);
bool Lufia2DecompFetchRomOpcodeSpan(struct CpuState *cpu,
    const Lufia2DecompRomOpcodeSpan *span,uint32_t pc,unsigned fetched);
/* Pure host bookkeeping for a short, observer-free plain-ROM span. The
 * caller guarantees no intervening CPU/device configuration changes. Each
 * guest clock and progress/APU boundary is retained. CanJoin additionally
 * certifies a pair of data-free/progress-free opcodes with no APU flush at
 * the first boundary, allowing only their host clock publications to join.
 * Unsupported/expired contexts use ordinary AccountOpcode immediately. */
typedef struct Lufia2DecompClockSpan {
    struct CpuState *cpu;
    void *system;
    uint64_t master, refresh_phase;
    uint32_t remaining;
} Lufia2DecompClockSpan;
bool Lufia2DecompPrepareClockSpan(struct CpuState *cpu,unsigned budget,
    Lufia2DecompClockSpan *span);
bool Lufia2DecompClockSpanCanJoin(struct CpuState *cpu,
    const Lufia2DecompClockSpan *span,uint64_t first,uint64_t total);
void Lufia2DecompAccountClockSpan(struct CpuState *cpu,Lufia2DecompClockSpan *span,
    unsigned cycles,uint64_t master,bool progress);
bool Lufia2DecompFrameReturnCpuReady(void);
bool Lufia2DecompFrameSchedulerBounces(void);
/* Original interrupt state; callers still validate timing and hooks. */
bool Lufia2DecompFramePairedCallReady(const struct CpuState *cpu, uint32_t entry);
void Lufia2DecompFramePairedAccountSpan(const struct CpuState *cpu, uint32_t entry,
    unsigned count, unsigned progress_mask);
int Lufia2DecompFramePairedFieldChild(struct CpuState *cpu, uint32_t target,
    uint32_t site, uint8_t frame_size, uint32_t *return_pc);
uint8_t Lufia2DecompNmiReadJoypadBusy(struct CpuState *cpu, bool *progress);
uint32_t Lufia2DecompFramePairedReturnPc(const struct CpuState *cpu, uint32_t entry);
bool Lufia2DecompFrameRangeQuiet(uint32_t first, uint32_t end);
bool Lufia2DecompFrameRangeAllows(uint32_t first, uint32_t end,
    bool (*owned)(uint32_t, void (*)(struct CpuState *, uint32_t)));

uint8_t Lufia2DecompFrameReturnReadByte(struct CpuState *cpu, uint32_t address,
                                        bool *progress);
void Lufia2DecompFrameReturnWriteByte(struct CpuState *cpu, uint32_t address,
                                      uint8_t value, bool *progress);
uint16_t Lufia2DecompFrameReturnReadWord(struct CpuState *cpu, uint32_t low,
                                        uint32_t high, bool *progress);
void Lufia2DecompFrameReturnWriteWord(struct CpuState *cpu, uint32_t low,
                                      uint32_t high, uint16_t value, bool *progress);

void Lufia2DecompFrameReturnAccountOpcode(struct CpuState *cpu, unsigned cycles,
                                          uint64_t master, bool progress);
void Lufia2DecompFrameReturnComplete(unsigned count, unsigned progress_mask);
unsigned Lufia2DecompFrameSpanBudget(uint32_t first, uint32_t end,
    void (*owner)(struct CpuState *, uint32_t));
unsigned Lufia2DecompFrameOwnedSpanBudget(uint32_t first, uint32_t end,
    void (*owner)(struct CpuState *, uint32_t));
/* Progress is one-based; zero means no observable progress. */
void Lufia2DecompFrameReturnCompleteSpan(unsigned count, unsigned last_progress);

#endif
