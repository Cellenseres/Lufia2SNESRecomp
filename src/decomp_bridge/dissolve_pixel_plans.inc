static const FixedSpanOpcode dissolve_call[] = {
    {0xe85fu, 4u, 8u, 3u},
};

static const FixedSpanOpcode dissolve_random[] = {
    {0x82c7u, 1u, 3u, 1u},
    {0x82c8u, 1u, 3u, 1u},
    {0x82c9u, 1u, 4u, 1u},
    {0x82cau, 1u, 4u, 2u},
    {0x82cbu, 1u, 4u, 2u},
    {0x82ccu, 1u, 3u, 1u},
    {0x82cdu, 2u, 3u, 0u},
    {0x82cfu, 3u, 4u, 1u},
    {0x82d2u, 1u, 2u, 0u},
    {0x82d3u, 2u, 2u, 0u},
    {0x82d5u, 2u, 3u, 0u},
    {0x82dcu, 3u, 4u, 1u},
    {0x82dfu, 3u, 4u, 1u},
    {0x82e2u, 1u, 4u, 1u},
    {0x82e3u, 1u, 5u, 2u},
    {0x82e4u, 1u, 5u, 2u},
    {0x82e5u, 1u, 4u, 1u},
    {0x82e6u, 1u, 6u, 3u},
};

static const FixedSpanOpcode dissolve_initial[] = {
    {0xe863u, 2u, 3u, 1u},
    {0xe865u, 2u, 4u, 2u},
    {0xe867u, 1u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_row_zero_less[] = {
    {0xe868u, 1u, 2u, 0u},
    {0xe869u, 2u, 4u, 2u},
    {0xe86bu, 2u, 2u, 0u},
    {0xe86du, 1u, 2u, 0u},
    {0xe86eu, 2u, 5u, 3u},
    {0xe870u, 2u, 2u, 0u},
    {0xe872u, 2u, 4u, 2u},
    {0xe874u, 2u, 3u, 1u},
    {0xe876u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_row_end[] = {
    {0xe868u, 1u, 2u, 0u},
    {0xe869u, 2u, 4u, 2u},
    {0xe86bu, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_empty[] = {
    {0xe87cu, 2u, 2u, 0u},
    {0xe87eu, 2u, 2u, 0u},
    {0xe880u, 1u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_row_sum_equal[] = {
    {0xe868u, 1u, 2u, 0u},
    {0xe869u, 2u, 4u, 2u},
    {0xe86bu, 2u, 2u, 0u},
    {0xe86du, 1u, 2u, 0u},
    {0xe86eu, 2u, 5u, 3u},
    {0xe870u, 2u, 3u, 0u},
    {0xe874u, 2u, 3u, 1u},
    {0xe876u, 2u, 2u, 0u},
    {0xe878u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_row_sum_greater[] = {
    {0xe868u, 1u, 2u, 0u},
    {0xe869u, 2u, 4u, 2u},
    {0xe86bu, 2u, 2u, 0u},
    {0xe86du, 1u, 2u, 0u},
    {0xe86eu, 2u, 5u, 3u},
    {0xe870u, 2u, 3u, 0u},
    {0xe874u, 2u, 3u, 1u},
    {0xe876u, 2u, 2u, 0u},
    {0xe878u, 2u, 2u, 0u},
    {0xe87au, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_locate[] = {
    {0xe882u, 1u, 2u, 0u},
    {0xe883u, 2u, 3u, 1u},
    {0xe885u, 2u, 3u, 1u},
    {0xe887u, 2u, 5u, 3u},
    {0xe889u, 1u, 2u, 0u},
    {0xe88au, 2u, 7u, 3u},
    {0xe88cu, 1u, 3u, 1u},
    {0xe88du, 1u, 3u, 1u},
    {0xe88eu, 1u, 4u, 1u},
    {0xe88fu, 3u, 5u, 2u},
    {0xe892u, 2u, 3u, 1u},
    {0xe894u, 3u, 4u, 1u},
    {0xe897u, 1u, 4u, 2u},
    {0xe898u, 1u, 5u, 2u},
    {0xe899u, 1u, 2u, 0u},
    {0xe89au, 2u, 3u, 0u},
    {0xe89cu, 3u, 5u, 2u},
    {0xe89fu, 3u, 5u, 2u},
    {0xe8a2u, 1u, 2u, 0u},
    {0xe8a3u, 1u, 2u, 0u},
    {0xe8a4u, 1u, 2u, 0u},
    {0xe8a5u, 1u, 2u, 0u},
    {0xe8a6u, 2u, 4u, 2u},
    {0xe8a8u, 1u, 2u, 0u},
    {0xe8a9u, 1u, 2u, 0u},
    {0xe8aau, 3u, 3u, 0u},
    {0xe8adu, 1u, 2u, 0u},
    {0xe8aeu, 2u, 4u, 2u},
    {0xe8b0u, 2u, 4u, 2u},
    {0xe8b2u, 1u, 2u, 0u},
    {0xe8b3u, 2u, 3u, 0u},
    {0xe8b5u, 2u, 2u, 0u},
    {0xe8b7u, 3u, 4u, 1u},
    {0xe8bau, 2u, 3u, 1u},
    {0xe8bcu, 3u, 4u, 1u},
    {0xe8bfu, 2u, 3u, 0u},
    {0xe8c1u, 2u, 4u, 2u},
    {0xe8c3u, 1u, 2u, 0u},
    {0xe8c4u, 3u, 5u, 2u},
    {0xe8c7u, 1u, 2u, 0u},
    {0xe8c8u, 2u, 3u, 0u},
    {0xe8cau, 1u, 4u, 1u},
    {0xe8cbu, 2u, 5u, 3u},
    {0xe8cdu, 2u, 3u, 1u},
    {0xe8cfu, 2u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_bit_miss_more[] = {
    {0xe8d1u, 2u, 3u, 1u},
    {0xe8d3u, 2u, 3u, 0u},
    {0xe8d9u, 1u, 2u, 0u},
    {0xe8dau, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_bit_miss_last[] = {
    {0xe8d1u, 2u, 3u, 1u},
    {0xe8d3u, 2u, 3u, 0u},
    {0xe8d9u, 1u, 2u, 0u},
    {0xe8dau, 2u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_row_sum_less[] = {
    {0xe868u, 1u, 2u, 0u},
    {0xe869u, 2u, 4u, 2u},
    {0xe86bu, 2u, 2u, 0u},
    {0xe86du, 1u, 2u, 0u},
    {0xe86eu, 2u, 5u, 3u},
    {0xe870u, 2u, 3u, 0u},
    {0xe874u, 2u, 3u, 1u},
    {0xe876u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_bit_hit_chosen[] = {
    {0xe8d1u, 2u, 3u, 1u},
    {0xe8d3u, 2u, 2u, 0u},
    {0xe8d5u, 2u, 5u, 2u},
    {0xe8d7u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_write[] = {
    {0xe8ddu, 2u, 2u, 0u},
    {0xe8dfu, 2u, 5u, 3u},
    {0xe8e1u, 2u, 7u, 3u},
    {0xe8e3u, 1u, 2u, 0u},
    {0xe8e4u, 2u, 7u, 3u},
    {0xe8e6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_retry[] = {
    {0xe87cu, 2u, 2u, 0u},
    {0xe87eu, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_bit_hit_more[] = {
    {0xe8d1u, 2u, 3u, 1u},
    {0xe8d3u, 2u, 2u, 0u},
    {0xe8d5u, 2u, 5u, 2u},
    {0xe8d7u, 2u, 2u, 0u},
    {0xe8d9u, 1u, 2u, 0u},
    {0xe8dau, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_row_zero_equal[] = {
    {0xe868u, 1u, 2u, 0u},
    {0xe869u, 2u, 4u, 2u},
    {0xe86bu, 2u, 2u, 0u},
    {0xe86du, 1u, 2u, 0u},
    {0xe86eu, 2u, 5u, 3u},
    {0xe870u, 2u, 2u, 0u},
    {0xe872u, 2u, 4u, 2u},
    {0xe874u, 2u, 3u, 1u},
    {0xe876u, 2u, 2u, 0u},
    {0xe878u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_random_refill_begin[] = {
    {0x82c7u, 1u, 3u, 1u},
    {0x82c8u, 1u, 3u, 1u},
    {0x82c9u, 1u, 4u, 1u},
    {0x82cau, 1u, 4u, 2u},
    {0x82cbu, 1u, 4u, 2u},
    {0x82ccu, 1u, 3u, 1u},
    {0x82cdu, 2u, 3u, 0u},
    {0x82cfu, 3u, 4u, 1u},
    {0x82d2u, 1u, 2u, 0u},
    {0x82d3u, 2u, 2u, 0u},
    {0x82d5u, 2u, 2u, 0u},
    {0x82d7u, 3u, 6u, 2u},
    {0x832du, 2u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_random_first[] = {
    {0x832fu, 3u, 4u, 1u},
    {0x8332u, 3u, 4u, 1u},
    {0x8335u, 3u, 5u, 1u},
    {0x8338u, 1u, 2u, 0u},
    {0x8339u, 2u, 2u, 0u},
    {0x833bu, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_random_first_last[] = {
    {0x832fu, 3u, 4u, 1u},
    {0x8332u, 3u, 4u, 1u},
    {0x8335u, 3u, 5u, 1u},
    {0x8338u, 1u, 2u, 0u},
    {0x8339u, 2u, 2u, 0u},
    {0x833bu, 2u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_random_second[] = {
    {0x833du, 3u, 4u, 1u},
    {0x8340u, 3u, 4u, 1u},
    {0x8343u, 3u, 5u, 1u},
    {0x8346u, 1u, 2u, 0u},
    {0x8347u, 2u, 2u, 0u},
    {0x8349u, 2u, 3u, 0u},
};

static const FixedSpanOpcode dissolve_random_second_last[] = {
    {0x833du, 3u, 4u, 1u},
    {0x8340u, 3u, 4u, 1u},
    {0x8343u, 3u, 5u, 1u},
    {0x8346u, 1u, 2u, 0u},
    {0x8347u, 2u, 2u, 0u},
    {0x8349u, 2u, 2u, 0u},
};

static const FixedSpanOpcode dissolve_random_refill_end[] = {
    {0x834bu, 1u, 6u, 2u},
    {0x82dau, 2u, 2u, 0u},
    {0x82dcu, 3u, 4u, 1u},
    {0x82dfu, 3u, 4u, 1u},
    {0x82e2u, 1u, 4u, 1u},
    {0x82e3u, 1u, 5u, 2u},
    {0x82e4u, 1u, 5u, 2u},
    {0x82e5u, 1u, 4u, 1u},
    {0x82e6u, 1u, 6u, 3u},
};

static const FixedSpanOpcode dissolve_bit_hit_last[] = {
    {0xe8d1u, 2u, 3u, 1u},
    {0xe8d3u, 2u, 2u, 0u},
    {0xe8d5u, 2u, 5u, 2u},
    {0xe8d7u, 2u, 2u, 0u},
    {0xe8d9u, 1u, 2u, 0u},
    {0xe8dau, 2u, 2u, 0u},
};
