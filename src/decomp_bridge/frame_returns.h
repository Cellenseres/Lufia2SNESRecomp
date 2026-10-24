#ifndef LUFIA2_FRAME_RETURNS_H
#define LUFIA2_FRAME_RETURNS_H

#include <stdint.h>
#include <stdbool.h>

struct CpuState;

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

#endif
