#include "nmi_owner.h"
#include "host_events.h"

bool Lufia2DecompNmiHasInterruptOwner(void) {
    return s_native_hook_interrupt != 0 && !s_pre_opcode_redirect_valid;
}

void Lufia2DecompNmiInstallTickHook(void) {
    for (int i = 0; i < s_pre_opcode_hook_count; ++i)
        if (s_pre_opcode_hooks[i].pc24 == 0x008699u)
            return;
    interp_bridge_set_pre_opcode_hook(0x808699u, Lufia2DecompPlayTimeTick);
}
