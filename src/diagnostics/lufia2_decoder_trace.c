#define _GNU_SOURCE
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cpu_state.h"
#include "snes/interp816.h"
#include "snes/spc.h"

extern int snes_frame_counter;
int __real_interp816_runOpcode(Interp816 *cpu);
int __real_spc_runOpcode(Spc *spc);

static FILE *trace_file;

__attribute__((constructor))
static void OpenDecoderTrace(void) {
    char filename[] = "lufia2-interpreter-XXXXXX.csv";
    const int fd = mkstemps(filename, 4);
    if (fd < 0) {
        perror("Lufia II interpreter trace");
        return;
    }
    trace_file = fdopen(fd, "w");
    if (!trace_file) {
        perror("Lufia II interpreter trace");
        close(fd);
        unlink(filename);
        return;
    }
    setvbuf(trace_file, NULL, _IOLBF, 0);
    fputs("core,frame,master_cycle,apu_cycle,pc,a,x,y,sp,status,db,dp,emulation,waiting,stopped,irq_pending,nmi_pending\n",
          trace_file);
    fprintf(stderr, "Lufia II interpreter trace: %s\n", filename);
}

__attribute__((destructor))
static void CloseDecoderTrace(void) {
    if (!trace_file)
        return;
    const int write_failed = ferror(trace_file);
    if (fclose(trace_file) != 0 || write_failed)
        fputs("Lufia II interpreter trace: output incomplete\n", stderr);
}

int __wrap_interp816_runOpcode(Interp816 *cpu) {
    if (trace_file) {
        const uint64_t master = cpu->mem == &g_cpu ? g_cpu.master_cycles : 0;
        fprintf(trace_file, "65816,%d,%" PRIu64 ",,%02X%04X,%04X,%04X,%04X,%04X,%02X,%02X,%04X,%u,%u,%u,%u,%u\n",
                snes_frame_counter, master, cpu->k, cpu->pc, cpu->a, cpu->x, cpu->y,
                cpu->sp, interp816_getFlags(cpu), cpu->db, cpu->dp, cpu->e,
                cpu->waiting, cpu->stopped, cpu->irqWanted, cpu->nmiWanted);
    }
    return __real_interp816_runOpcode(cpu);
}

int __wrap_spc_runOpcode(Spc *spc) {
    if (trace_file) {
        const unsigned status = (spc->n << 7) | (spc->v << 6) | (spc->p << 5) |
                                (spc->b << 4) | (spc->h << 3) | (spc->i << 2) |
                                (spc->z << 1) | spc->c;
        fprintf(trace_file, "SPC700,,,%" PRIu64 ",%04X,%02X,%02X,%02X,%02X,%02X,,,,,%u,,\n",
                spc->apu->portClock, spc->pc, spc->a, spc->x, spc->y, spc->sp,
                status, spc->stopped);
    }
    return __real_spc_runOpcode(spc);
}
