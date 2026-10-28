#include "nmi_owner.h"
#include "host_events.h"
#include <stdint.h>

static int Lufia2DecompNmiHookHasOwner(uint32_t pc, int stop_on_rti,
                                      int auto_quiescent, long opcodes_left) {
    if (!stop_on_rti || auto_quiescent)
        return 0;
    switch (pc & 0x7fffffu) {
#if defined(LUFIA2_HAVE_NATIVE_TABLE_SERVICES)
    case 0x04f12fu:
        return opcodes_left >= 26u;
#endif
    case 0x008699u:
        return 1;
    case 0x05a8e7u:
        return opcodes_left >= LUFIA2_NMI_PRIMARY_WAVE_OPCODES;
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS)
    case 0x05aef5u:
        return opcodes_left >= LUFIA2_NMI_WAVE_FILL_WORD_OPCODES;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TIMER_CALLS)
    case 0x058f1bu:
    case 0x058f3cu:
        return opcodes_left >= LUFIA2_NMI_TIMER_CALL_OPCODES;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TIMER_STEPS)
    case 0x058f1fu:
        return opcodes_left >= LUFIA2_NMI_TIMER_HEADER_OPCODES;
    case 0x058f3eu:
        return opcodes_left >= LUFIA2_NMI_TIMER_ADVANCE_OPCODES;
#endif
#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_STEPS)
    case 0x05a74fu:
        return opcodes_left >= LUFIA2_NMI_RIPPLE_BYTE_OPCODES;
    case 0x05a760u:
        return opcodes_left >= LUFIA2_NMI_RIPPLE_FINISH_OPCODES;
#endif
    case 0x05a736u:
        return opcodes_left >= LUFIA2_NMI_RIPPLE_ROW_OPCODES;
    default:
        return 0;
    }
}

bool Lufia2DecompNmiHasInterruptOwner(void) {
    return s_native_hook_interrupt != 0 && !s_pre_opcode_redirect_valid;
}

void Lufia2DecompNmiInstallTickHook(void) {
    for (int i = 0; i < s_pre_opcode_hook_count; ++i)
        if (s_pre_opcode_hooks[i].pc24 == 0x008699u)
            return;
    interp_bridge_set_pre_opcode_hook(0x808699u, Lufia2DecompPlayTimeTick);
}
