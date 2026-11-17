#include "nmi_owner.h"
#include "host_events.h"
#include <stdint.h>

static bool Lufia2DecompNmiHookHasOwner(uint32_t pc, int stop_on_rti,
                                      int admission_flags, long opcodes_left) {
    if (!stop_on_rti || (admission_flags & LUFIA2_NMI_OWNER_QUIESCENT))
        return 0;
    switch (pc & 0x7fffffu) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_PAD_STEPS)
    case 0x008770u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_PAD_STEP_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_TABLE_SERVICES)
    case 0x04f12fu:
        return opcodes_left >= 26u;
#endif
#if defined(LUFIA2_HAVE_NATIVE_SCREEN_FADE_REQUEST)
    case 0x0086c1u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_FADE_REQUEST_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_STEPS)
    case 0x0087fcu: case 0x00880du: case 0x00881cu: case 0x008829u:
#if defined(LUFIA2_HAVE_NATIVE_NMI_TILEMAP_LOADS)
    case 0x008802u: case 0x008811u: case 0x008820u:
#endif
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= ((pc & 0x7fffffu) == 0x0087fcu
                ? LUFIA2_NMI_TILEMAP_HEADER_OPCODES : LUFIA2_NMI_TILEMAP_STEP_OPCODES) &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_SCROLL_FINISH)
    case 0x0087f3u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_SCROLL_FINISH_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_SCROLL_FRONTS)
    case 0x0087a7u: case 0x0087c4u: case 0x0087cfu:
    case 0x0087dau: case 0x0087e5u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= ((pc & 0x7fffffu) == 0x0087a7u
                ? LUFIA2_NMI_SCROLL_SETUP_OPCODES : LUFIA2_NMI_DMA_SELECT_OPCODES) &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_CLOCK)
    case 0x00869bu:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_RESTORE_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_CALLBACK_CLOCK)
    case 0x00864cu:
    case 0x008653u:
    case 0x00865au:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= ((pc & 0x7fffffu) == 0x00864cu
                ? LUFIA2_NMI_PRIMARY_CALLBACK_OPCODES : (pc & 0x7fffffu) == 0x008653u
                ? LUFIA2_NMI_SECONDARY_OPCODES : LUFIA2_NMI_SCROLL_OPCODES) &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK)
    case 0x008664u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_DISPLAY_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_SCROLL_PAIRS) && defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_CLOCK) && defined(LUFIA2_HAVE_NATIVE_NMI_DISPLAY_CLOCK)
    case 0x0087b2u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_SCROLL_PAIR_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
    case 0x008699u:
        return 1;
    case 0x05a8e7u:
        return opcodes_left >= LUFIA2_NMI_PRIMARY_WAVE_OPCODES;
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_WAVE_FILL_STEPS)
    case 0x05aef5u:
        return opcodes_left >= LUFIA2_NMI_WAVE_FILL_WORD_OPCODES;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_HDMA_BUILDERS)
    case 0x05af5eu:
    case 0x05afe0u:
    case 0x05b058u:
    case 0x05b0c3u:
    case 0x05b13bu:
    case 0x05b1a4u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_HDMA_BUILDER_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_NMI_CALLBACKS)
    case 0x058e79u: case 0x058e7cu: case 0x058e8eu:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_BATTLE_CALLBACK_OPCODES &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
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
#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_WORD_PHASES)
    case 0x05aa3du:
    case 0x05aa74u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= ((pc & 0x7fffffu) == 0x05aa3du
                ? LUFIA2_NMI_RIPPLE_WORD_PREPARE_OPCODES
                : LUFIA2_NMI_RIPPLE_WORD_FINISH_OPCODES) &&
            !s_frame_return_interrupt_blocked && !s_interp_bus_timing_active &&
            !g_interp_bridge_pc_hook && !s_interp_pctrace;
#endif
#if defined(LUFIA2_HAVE_NATIVE_RIPPLE_WORD_STEPS)
    case 0x05aa61u:
        return !(admission_flags & LUFIA2_NMI_OWNER_OBSERVED) &&
            opcodes_left >= LUFIA2_NMI_RIPPLE_WORD_OPCODES &&
            !s_interp_bus_timing_active && !g_interp_bridge_pc_hook &&
            !s_interp_pctrace;
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
