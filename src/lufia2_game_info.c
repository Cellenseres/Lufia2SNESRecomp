#include "lufia2_runtime.h"

const RtlGameInfo kLufia2GameInfo = {
    .title = "lufia2",
    .initialize = NULL,
    .run_frame = &Lufia2RunOneFrame,
    .draw_ppu_frame = &Lufia2DrawPpuFrame,
    .save_name_prefix = "lufia2",
    .state_save_extra = NULL,
    .state_load_extra = NULL,
    .on_state_loaded = NULL,
    .session_reset = NULL,
};
