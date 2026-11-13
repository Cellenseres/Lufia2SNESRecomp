enum {
    LUFIA2_DSP_PAYLOAD_SIZE = 33660u,
    LUFIA2_DSP_PADDING_SIZE = 4u,
    LUFIA2_DSP_WIRE_VERSION = 10u
};

_Static_assert(offsetof(Dsp, sampleRead) + sizeof(((Dsp *)0)->sampleRead) -
    offsetof(Dsp, ram) == LUFIA2_DSP_PAYLOAD_SIZE, "DSP snapshot fields changed");
_Static_assert(offsetof(Spc, cyclesUsed) - offsetof(Spc, a) == 15u,
    "SPC snapshot fields changed");

static void Lufia2RejectDspState(SaveLoadInfo *sli) {
    /* A failed load has already replaced other guest fields. */
    g_fail = true;
    if (sli->func == memory_sli_func)
        ((MemorySli *)sli)->error = true;
    else if (sli->func == file_sli_func)
        ((FileSli *)sli)->error = true;
}

static bool Lufia2DspStateHasPpuHeader(SaveLoadInfo *sli, size_t offset) {
    uint32_t header[2];
    return sli->peek(sli, offset, header, sizeof(header)) &&
        header[0] == 0x30555050u &&
        header[1] == PPU_SAVESTATE_REGS_SIZE + PPU_SAVESTATE_MEM_SIZE;
}

void Lufia2SaveLoadDspState(Dsp *dsp, SaveLoadInfo *sli) {
    size_t padding = LUFIA2_DSP_PADDING_SIZE;
    uint32_t reserved = 0u;
    if (sli->peek) {
        if (snes_saveload_get_version() >= LUFIA2_DSP_WIRE_VERSION) {
            if (!sli->peek(sli, LUFIA2_DSP_PAYLOAD_SIZE, &reserved, sizeof(reserved)) ||
                reserved) {
                Lufia2RejectDspState(sli);
                return;
            }
        } else {
            /* Legacy states identify their layout through the PPU header. */
            const size_t spc_size = offsetof(Spc, cyclesUsed) - offsetof(Spc, a);
            unsigned matches = 0u;
            for (size_t candidate = 0u; candidate <= LUFIA2_DSP_PADDING_SIZE;
                 candidate += LUFIA2_DSP_PADDING_SIZE) {
                const size_t after_spc = LUFIA2_DSP_PAYLOAD_SIZE + candidate + spc_size;
                if (Lufia2DspStateHasPpuHeader(sli, after_spc + 216u) ||
                    Lufia2DspStateHasPpuHeader(sli, after_spc + 222u)) {
                    padding = candidate;
                    ++matches;
                }
            }
            if (matches != 1u) {
                Lufia2RejectDspState(sli);
                return;
            }
        }
    }
    sli->func(sli, dsp->ram, LUFIA2_DSP_PAYLOAD_SIZE);
    if (padding)
        sli->func(sli, &reserved, LUFIA2_DSP_PADDING_SIZE);
}
