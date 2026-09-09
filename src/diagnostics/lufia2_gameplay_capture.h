#ifndef LUFIA2_GAMEPLAY_CAPTURE_H
#define LUFIA2_GAMEPLAY_CAPTURE_H
#include <stdint.h>
#ifdef LUFIA2_ENABLE_GAMEPLAY_CAPTURE
#include "cpu_state.h"
#include "snes/interp816.h"
extern int lufia2_capture_active;
extern uint32_t lufia2_capture_pc;
void L2CaptureToggle(void);
void L2CaptureShutdown(void);
void L2CaptureFrameBegin(uint32_t input);
void L2CaptureFrameEnd(void);
void L2CaptureGuestEnd(void);
int L2CaptureBefore(const CpuState *, const Interp816 *, uint32_t pc, unsigned op);
void L2CaptureAfter(const CpuState *, const Interp816 *, uint32_t pc, unsigned op);
void L2CaptureCall(const CpuState *, const Interp816 *, uint32_t site,
                   uint32_t target, unsigned op, int body, int bounce);
void L2CaptureBus(uint32_t address, unsigned value, int write);
void L2CaptureAbort(void);
#define L2_CAPTURE_BEFORE(c,i,p,o) (lufia2_capture_active ? L2CaptureBefore(c,i,p,o) : 0)
#define L2_CAPTURE_PC(p) do { if(lufia2_capture_active) lufia2_capture_pc=(p); } while(0)
#define L2_CAPTURE_AFTER(c,i,p,o) do { if(lufia2_capture_active) L2CaptureAfter(c,i,p,o); } while(0)
#define L2_CAPTURE_CALL(c,i,p,t,o,b,e) do { if(lufia2_capture_active) L2CaptureCall(c,i,p,t,o,b,e); } while(0)
#define L2_CAPTURE_BUS(a,v,w) do { if(lufia2_capture_active) L2CaptureBus(a,v,w); } while(0)
#else
#define L2CaptureToggle() ((void)0)
#define L2CaptureShutdown() ((void)0)
#define L2CaptureFrameBegin(input) ((void)0)
#define L2CaptureFrameEnd() ((void)0)
#define L2CaptureGuestEnd() ((void)0)
#define L2CaptureAbort() ((void)0)
#define L2_CAPTURE_BEFORE(c,i,p,o) 0
#define L2_CAPTURE_PC(p) ((void)0)
#define L2_CAPTURE_AFTER(c,i,p,o) ((void)0)
#define L2_CAPTURE_CALL(c,i,p,t,o,b,e) ((void)0)
#define L2_CAPTURE_BUS(a,v,w) ((void)0)
#endif
#endif
