typedef struct FieldRegionSdkContext {
    CpuState *cpu;
    long *steps;
    uint64_t *write_epoch;
    uint64_t *dynamic_epoch;
    long step_cap;
} FieldRegionSdkContext;

static FieldRegionSdkContext s_field_region_context;
