#ifndef LUFIA2_FRAME_RETURNS_H
#define LUFIA2_FRAME_RETURNS_H

#include <stdint.h>
#include <stdbool.h>

struct CpuState;

void Lufia2DecompInstallFrameReturnHooks(void);
bool Lufia2DecompFramePollHookIsNative(
    void (*hook)(struct CpuState *, uint32_t));
uint64_t Lufia2DecompFrameReturnDeadline(void);

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
