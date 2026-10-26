#ifndef LUFIA2_FRAME_RETURNS_H
#define LUFIA2_FRAME_RETURNS_H

#include <stdint.h>
#include <stdbool.h>

/* Host batching only; original opcode and device clocks remain exact. */
#define LUFIA2_FRAME_POLL_MAX_OPCODES 1024u

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
bool Lufia2DecompFrameReturnCpuReady(void);
bool Lufia2DecompFrameRangeQuiet(uint32_t first, uint32_t end);

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
/* Progress is one-based; zero means no observable progress. */
void Lufia2DecompFrameReturnCompleteSpan(unsigned count, unsigned last_progress);

#endif
