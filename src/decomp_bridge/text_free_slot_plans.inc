static const FixedSpanOpcode text_free_slot_begin[] = {
    {0xc1c3u, 2u, 3u, 1u},
    {0xc1c5u, 3u, 3u, 0u},
};

static const FixedSpanOpcode text_free_slot_query_busy[] = {
    {0xc1c8u, 3u, 4u, 1u},
    {0xc1cbu, 2u, 2u, 0u},
    {0xc1cdu, 2u, 2u, 0u},
};

static const FixedSpanOpcode text_free_slot_query_found[] = {
    {0xc1c8u, 3u, 4u, 1u},
    {0xc1cbu, 2u, 2u, 0u},
    {0xc1cdu, 2u, 3u, 0u},
};

static const FixedSpanOpcode text_free_slot_busy_repeat[] = {
    {0xc1cfu, 1u, 2u, 0u},
    {0xc1d0u, 3u, 3u, 0u},
    {0xc1d3u, 2u, 3u, 0u},
};

static const FixedSpanOpcode text_free_slot_busy_last[] = {
    {0xc1cfu, 1u, 2u, 0u},
    {0xc1d0u, 3u, 3u, 0u},
    {0xc1d3u, 2u, 2u, 0u},
};

static const FixedSpanOpcode text_free_slot_found[] = {
    {0xc1d7u, 2u, 4u, 2u},
    {0xc1d9u, 4u, 8u, 3u},
};

static const FixedSpanOpcode text_free_slot_child[] = {
    {0x82d5u, 2u, 3u, 1u},
    {0x82d7u, 2u, 3u, 1u},
    {0x82d9u, 1u, 2u, 0u},
    {0x82dau, 2u, 3u, 1u},
    {0x82dcu, 2u, 3u, 1u},
    {0x82deu, 2u, 3u, 1u},
    {0x82e0u, 2u, 3u, 1u},
    {0x82e2u, 2u, 3u, 1u},
    {0x82e4u, 2u, 3u, 1u},
    {0x82e6u, 1u, 6u, 3u},
};

static const FixedSpanOpcode text_free_slot_success[] = {
    {0xc1ddu, 1u, 2u, 0u},
};

static const FixedSpanOpcode text_free_slot_full[] = {
    {0xc1d5u, 1u, 2u, 0u},
};
