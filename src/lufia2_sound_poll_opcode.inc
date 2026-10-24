/* Reuse the original addressing, flags and branch implementations. This
 * removes the large opcode switch from the bounded poll kernels, while the
 * original runOpcode still owns fetch, interrupt entry and all tallies.
 */
static bool Lufia2SoundPollOpcode(Interp816 *cpu, uint8_t opcode, uint32_t pc) {
    if (!cpu->mf || cpu->e) return false;
    if (lufia2_sound_poll_kernel_enabled &&
        (pc == 0x809997u || pc == 0x809a0fu || pc == 0x809a17u) && opcode == 0xcfu) {
        uint32_t low = 0;
        const uint32_t high = interp816_adrAbl(cpu, &low);
        interp816_cmp(cpu, low, high);
        return true;
    }
    if (lufia2_sound_poll_kernel_enabled &&
        (pc == 0x80999bu || pc == 0x809a13u || pc == 0x809a1bu ||
         pc == 0x809a2au || pc == 0x809a30u) && opcode == 0xd0u) {
        interp816_doBranch(cpu, interp816_readOpcode(cpu), !cpu->z);
        return true;
    }
    if (lufia2_sound_poll_kernel_enabled &&
        (pc == 0x809a26u || pc == 0x809a2cu) && opcode == 0xafu) {
        uint32_t low = 0;
        const uint32_t high = interp816_adrAbl(cpu, &low);
        interp816_lda(cpu, low, high);
        return true;
    }
    if (lufia2_frame_poll_kernel_enabled && cpu->dp == 0 && pc == 0x8293c8u && opcode == 0xc5u) {
        uint32_t low = 0;
        const uint32_t high = interp816_adrDp(cpu, &low);
        interp816_cmp(cpu, low, high);
        return true;
    }
    if (lufia2_frame_poll_kernel_enabled && pc == 0x8293cau && opcode == 0xf0u) {
        interp816_doBranch(cpu, interp816_readOpcode(cpu), cpu->z);
        return true;
    }
    return false;
}
