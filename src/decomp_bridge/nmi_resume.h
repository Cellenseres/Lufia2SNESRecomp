#ifndef LUFIA2_NMI_RESUME_H
#define LUFIA2_NMI_RESUME_H

#include <stdint.h>
#include "cpu_state.h"

void Lufia2DecompInstallNmiHooks(void);
void Lufia2DecompNmiAccountCycles(CpuState *cpu, unsigned cycles, uint64_t master);
uint8_t Lufia2DecompNmiReadByte(CpuState *cpu, uint32_t address);
void Lufia2DecompNmiWriteByte(CpuState *cpu, uint32_t address, uint8_t value);

#endif
