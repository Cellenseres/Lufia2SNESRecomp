#ifndef LUFIA2_SCREEN_FADE_TIMING_INC
#define LUFIA2_SCREEN_FADE_TIMING_INC

static const FixedSpanOpcode screen_fade_common_timing[] = {
    {0x86c1u, 3u, 4u, 1u}, {0x86c4u, 2u, 2u, 0u},
    {0x86c6u, 1u, 3u, 0u}, {0x86c7u, 3u, 4u, 1u},
    {0x86cau, 2u, 2u, 0u}, {0x86ccu, 1u, 2u, 0u},
    {0x86cdu, 3u, 4u, 1u}, {0x86d0u, 3u, 4u, 1u},
    {0x86d3u, 1u, 2u, 0u}, {0x86d4u, 1u, 2u, 0u},
    {0x86d5u, 1u, 2u, 0u}, {0x86d6u, 1u, 3u, 0u},
    {0x86d7u, 2u, 2u, 0u}, {0x86d9u, 2u, 2u, 0u}
};
static const FixedSpanOpcode screen_fade_in_timing[] = {
    {0x86f2u, 1u, 3u, 0u}, {0x86f3u, 3u, 4u, 1u},
    {0x86f6u, 2u, 2u, 0u}, {0x86f8u, 2u, 2u, 0u},
    {0x86fau, 3u, 4u, 1u}, {0x86fdu, 2u, 2u, 0u},
    {0x86ffu, 3u, 4u, 1u}
};
static const FixedSpanOpcode screen_fade_out_timing[] = {
    {0x86dbu, 1u, 3u, 0u}, {0x86dcu, 1u, 2u, 0u},
    {0x86ddu, 2u, 2u, 0u}, {0x86dfu, 2u, 2u, 0u},
    {0x86e1u, 1u, 2u, 0u}, {0x86e2u, 3u, 4u, 1u},
    {0x86e5u, 1u, 2u, 0u}, {0x86e6u, 2u, 2u, 0u},
    {0x86e8u, 3u, 4u, 1u}, {0x86ebu, 2u, 2u, 0u},
    {0x86edu, 3u, 4u, 1u}, {0x86f0u, 2u, 3u, 0u}
};

#endif
