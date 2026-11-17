typedef struct NativeCallOwnerContext {
    CpuState *cpu;
    long *steps;
    uint64_t *write_epoch;
    uint64_t *dynamic_epoch;
    long step_cap;
} NativeCallOwnerContext;

static NativeCallOwnerContext s_native_call_owner;
