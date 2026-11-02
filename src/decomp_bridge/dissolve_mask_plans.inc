static const FixedSpanOpcode dissolve_mask_begin[] = {
    {0xe7c5u, 1u, 3u, 1u},
    {0xe7c6u, 2u, 2u, 0u},
    {0xe7c8u, 1u, 3u, 1u},
    {0xe7c9u, 1u, 4u, 1u},
    {0xe7cau, 3u, 3u, 0u},
    {0xe7cdu, 3u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_mask_tile[] = {
    {0xe7d0u, 2u, 4u, 2u},
    {0xe7d2u, 2u, 4u, 2u},
    {0xe7d4u, 2u, 2u, 0u},
    {0xe7d6u, 2u, 3u, 1u},
};

static const FixedSpanOpcode dissolve_mask_row[] = {
    {0xe7d8u, 3u, 4u, 1u},
    {0xe7dbu, 3u, 4u, 1u},
    {0xe7deu, 3u, 4u, 1u},
    {0xe7e1u, 3u, 4u, 1u},
    {0xe7e4u, 3u, 6u, 1u},
    {0xe7e7u, 3u, 6u, 1u},
    {0xe7eau, 1u, 2u, 0u},
    {0xe7ebu, 1u, 2u, 0u},
    {0xe7ecu, 1u, 2u, 0u},
    {0xe7edu, 1u, 2u, 0u},
    {0xe7eeu, 2u, 5u, 2u},
    {0xe7f0u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_mask_advance[] = {
    {0xe7f2u, 2u, 3u, 0u},
    {0xe7f4u, 2u, 4u, 2u},
    {0xe7f6u, 1u, 2u, 0u},
    {0xe7f7u, 3u, 3u, 0u},
    {0xe7fau, 1u, 2u, 0u},
    {0xe7fbu, 2u, 4u, 2u},
    {0xe7fdu, 1u, 2u, 0u},
    {0xe7feu, 3u, 3u, 0u},
    {0xe801u, 1u, 2u, 0u},
    {0xe802u, 2u, 3u, 0u},
    {0xe804u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_mask_row_end[] = {
    {0xe7d8u, 3u, 4u, 1u},
    {0xe7dbu, 3u, 4u, 1u},
    {0xe7deu, 3u, 4u, 1u},
    {0xe7e1u, 3u, 4u, 1u},
    {0xe7e4u, 3u, 6u, 1u},
    {0xe7e7u, 3u, 6u, 1u},
    {0xe7eau, 1u, 2u, 0u},
    {0xe7ebu, 1u, 2u, 0u},
    {0xe7ecu, 1u, 2u, 0u},
    {0xe7edu, 1u, 2u, 0u},
    {0xe7eeu, 2u, 5u, 2u},
    {0xe7f0u, 2u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_mask_advance_end[] = {
    {0xe7f2u, 2u, 3u, 0u},
    {0xe7f4u, 2u, 4u, 2u},
    {0xe7f6u, 1u, 2u, 0u},
    {0xe7f7u, 3u, 3u, 0u},
    {0xe7fau, 1u, 2u, 0u},
    {0xe7fbu, 2u, 4u, 2u},
    {0xe7fdu, 1u, 2u, 0u},
    {0xe7feu, 3u, 3u, 0u},
    {0xe801u, 1u, 2u, 0u},
    {0xe802u, 2u, 3u, 0u},
    {0xe804u, 2u, 2u, 0u},
};
