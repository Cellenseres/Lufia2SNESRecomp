#ifndef LUFIA2_FIXED_SPAN_HOOKS
#define LUFIA2_FIXED_SPAN_HOOKS

static bool FixedSpanHooksQuiet(uint32_t bank,
    const FixedSpanOpcode *opcodes, unsigned count) {
    if (!count)
        return false;
    uint32_t first = bank | opcodes[0].pc;
    uint32_t end = first;
    for (unsigned i = 0u; i < count; ++i) {
        const uint32_t next = bank | opcodes[i].pc;
        if (!opcodes[i].fetched)
            return false;
        if (next != end) {
            if (!Lufia2DecompFrameRangeQuiet(first, end))
                return false;
            first = next;
        }
        end = next + opcodes[i].fetched;
    }
    return Lufia2DecompFrameRangeQuiet(first, end);
}

#endif
