#include "lufia2_runtime.h"

const RtlGameInfo kLufia2GameInfo = {
    .title = "lufia2",
    .initialize = NULL,
    .run_frame = &Lufia2RunOneFrame,
    .draw_ppu_frame = &Lufia2DrawPpuFrame,
    .save_name_prefix = "lufia2",
    .state_save_extra = &Lufia2SaveExecutionState,
    .state_load_extra = &Lufia2LoadExecutionState,
    .on_state_loaded = &Lufia2ApplyExecutionState,
    .session_reset = NULL,
    .minimum_state_version = 9,
};
