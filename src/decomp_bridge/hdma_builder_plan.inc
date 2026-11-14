typedef struct HdmaBuilderProfile {
    uint16_t entry, terminal, early_terminal, source;
    const FixedSpanOpcode *prefix, *loop, *suffix, *early;
    unsigned prefix_count, loop_count, suffix_count, early_count, iterations;
} HdmaBuilderProfile;

static const FixedSpanOpcode hdma_builder_0_prefix[] = {
    {0xaf5eu, 1u, 3u, 1u},
    {0xaf5fu, 2u, 2u, 0u},
    {0xaf61u, 1u, 3u, 1u},
    {0xaf62u, 1u, 4u, 1u},
    {0xaf63u, 3u, 5u, 2u},
    {0xaf66u, 1u, 4u, 2u},
    {0xaf67u, 3u, 3u, 0u},
};

static const FixedSpanOpcode hdma_builder_0_loop[] = {
    {0xaf6au, 2u, 3u, 0u},
    {0xaf6cu, 3u, 5u, 2u},
    {0xaf6fu, 1u, 2u, 0u},
    {0xaf70u, 3u, 5u, 2u},
    {0xaf73u, 4u, 6u, 2u},
    {0xaf77u, 2u, 3u, 0u},
    {0xaf79u, 1u, 2u, 0u},
    {0xaf7au, 1u, 2u, 0u},
    {0xaf7bu, 1u, 2u, 0u},
    {0xaf7cu, 1u, 2u, 0u},
    {0xaf7du, 3u, 3u, 0u},
    {0xaf80u, 2u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_0_suffix[] = {
    {0xaf82u, 1u, 5u, 2u},
    {0xaf83u, 3u, 5u, 2u},
    {0xaf86u, 1u, 2u, 0u},
    {0xaf87u, 1u, 2u, 0u},
    {0xaf88u, 3u, 3u, 0u},
    {0xaf8bu, 2u, 2u, 0u},
    {0xaf8du, 3u, 3u, 0u},
    {0xaf90u, 2u, 3u, 0u},
    {0xaf92u, 1u, 2u, 0u},
    {0xaf93u, 3u, 5u, 2u},
    {0xaf96u, 2u, 3u, 0u},
    {0xaf98u, 2u, 2u, 0u},
    {0xaf9au, 3u, 4u, 1u},
    {0xaf9du, 1u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_1_prefix[] = {
    {0xafe0u, 1u, 3u, 1u},
    {0xafe1u, 2u, 2u, 0u},
    {0xafe3u, 1u, 3u, 1u},
    {0xafe4u, 1u, 4u, 1u},
    {0xafe5u, 3u, 5u, 2u},
    {0xafe8u, 1u, 4u, 2u},
    {0xafe9u, 3u, 3u, 0u},
};

static const FixedSpanOpcode hdma_builder_1_loop[] = {
    {0xafecu, 2u, 3u, 0u},
    {0xafeeu, 3u, 5u, 2u},
    {0xaff1u, 1u, 2u, 0u},
    {0xaff2u, 3u, 5u, 2u},
    {0xaff5u, 4u, 6u, 2u},
    {0xaff9u, 2u, 3u, 0u},
    {0xaffbu, 1u, 2u, 0u},
    {0xaffcu, 1u, 2u, 0u},
    {0xaffdu, 1u, 2u, 0u},
    {0xaffeu, 1u, 2u, 0u},
    {0xafffu, 3u, 3u, 0u},
    {0xb002u, 2u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_1_suffix[] = {
    {0xb004u, 1u, 5u, 2u},
    {0xb005u, 3u, 5u, 2u},
    {0xb008u, 1u, 2u, 0u},
    {0xb009u, 1u, 2u, 0u},
    {0xb00au, 2u, 2u, 0u},
    {0xb00cu, 3u, 3u, 0u},
    {0xb00fu, 2u, 3u, 0u},
    {0xb011u, 1u, 2u, 0u},
    {0xb012u, 3u, 5u, 2u},
    {0xb015u, 2u, 3u, 0u},
    {0xb017u, 2u, 2u, 0u},
    {0xb019u, 3u, 4u, 1u},
    {0xb01cu, 1u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_2_prefix[] = {
    {0xb058u, 1u, 3u, 1u},
    {0xb059u, 2u, 2u, 0u},
    {0xb05bu, 1u, 3u, 1u},
    {0xb05cu, 1u, 4u, 1u},
    {0xb05du, 1u, 4u, 2u},
    {0xb05eu, 3u, 3u, 0u},
    {0xb061u, 1u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_2_loop[] = {
    {0xb062u, 2u, 3u, 0u},
    {0xb064u, 3u, 5u, 2u},
    {0xb067u, 1u, 2u, 0u},
    {0xb068u, 3u, 5u, 2u},
    {0xb06bu, 4u, 6u, 2u},
    {0xb06fu, 2u, 3u, 0u},
    {0xb071u, 1u, 2u, 0u},
    {0xb072u, 1u, 2u, 0u},
    {0xb073u, 1u, 2u, 0u},
    {0xb074u, 1u, 2u, 0u},
    {0xb075u, 3u, 3u, 0u},
    {0xb078u, 2u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_2_suffix[] = {
    {0xb07au, 1u, 5u, 2u},
    {0xb07bu, 2u, 2u, 0u},
    {0xb07du, 3u, 4u, 1u},
    {0xb080u, 1u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_3_prefix[] = {
    {0xb0c3u, 1u, 3u, 1u},
    {0xb0c4u, 2u, 2u, 0u},
    {0xb0c6u, 1u, 3u, 1u},
    {0xb0c7u, 1u, 4u, 1u},
    {0xb0c8u, 3u, 5u, 2u},
    {0xb0cbu, 3u, 3u, 0u},
    {0xb0ceu, 2u, 2u, 0u},
    {0xb0d0u, 1u, 4u, 2u},
    {0xb0d1u, 3u, 3u, 0u},
};

static const FixedSpanOpcode hdma_builder_3_loop[] = {
    {0xb0d4u, 2u, 3u, 0u},
    {0xb0d6u, 3u, 5u, 2u},
    {0xb0d9u, 4u, 6u, 2u},
    {0xb0ddu, 2u, 3u, 0u},
    {0xb0dfu, 1u, 2u, 0u},
    {0xb0e0u, 1u, 2u, 0u},
    {0xb0e1u, 1u, 2u, 0u},
    {0xb0e2u, 1u, 2u, 0u},
    {0xb0e3u, 3u, 3u, 0u},
    {0xb0e6u, 2u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_3_suffix[] = {
    {0xb0e8u, 1u, 5u, 2u},
    {0xb0e9u, 3u, 5u, 2u},
    {0xb0ecu, 2u, 2u, 0u},
    {0xb0eeu, 3u, 4u, 1u},
    {0xb0f1u, 1u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_3_early[] = {
    {0xb0f3u, 1u, 4u, 1u},
    {0xb0f4u, 2u, 2u, 0u},
    {0xb0f6u, 2u, 5u, 2u},
    {0xb0f8u, 3u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_4_prefix[] = {
    {0xb13bu, 1u, 3u, 1u},
    {0xb13cu, 2u, 2u, 0u},
    {0xb13eu, 1u, 3u, 1u},
    {0xb13fu, 1u, 4u, 1u},
    {0xb140u, 3u, 5u, 2u},
    {0xb143u, 2u, 2u, 0u},
    {0xb145u, 1u, 4u, 2u},
    {0xb146u, 3u, 3u, 0u},
};

static const FixedSpanOpcode hdma_builder_4_loop[] = {
    {0xb149u, 2u, 3u, 0u},
    {0xb14bu, 3u, 5u, 2u},
    {0xb14eu, 4u, 6u, 2u},
    {0xb152u, 2u, 3u, 0u},
    {0xb154u, 1u, 2u, 0u},
    {0xb155u, 1u, 2u, 0u},
    {0xb156u, 1u, 2u, 0u},
    {0xb157u, 1u, 2u, 0u},
    {0xb158u, 2u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_4_suffix[] = {
    {0xb15au, 1u, 5u, 2u},
    {0xb15bu, 3u, 5u, 2u},
    {0xb15eu, 2u, 2u, 0u},
    {0xb160u, 3u, 4u, 1u},
    {0xb163u, 1u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_4_early[] = {
    {0xb165u, 1u, 4u, 1u},
    {0xb166u, 2u, 2u, 0u},
    {0xb168u, 2u, 5u, 2u},
    {0xb16au, 3u, 4u, 1u},
};

static const FixedSpanOpcode hdma_builder_5_prefix[] = {
    {0xb1a4u, 1u, 3u, 1u},
    {0xb1a5u, 2u, 2u, 0u},
    {0xb1a7u, 1u, 3u, 1u},
    {0xb1a8u, 1u, 4u, 1u},
    {0xb1a9u, 1u, 4u, 2u},
    {0xb1aau, 3u, 3u, 0u},
    {0xb1adu, 1u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_5_loop[] = {
    {0xb1aeu, 2u, 3u, 0u},
    {0xb1b0u, 3u, 5u, 2u},
    {0xb1b3u, 1u, 2u, 0u},
    {0xb1b4u, 3u, 5u, 2u},
    {0xb1b7u, 4u, 6u, 2u},
    {0xb1bbu, 2u, 3u, 0u},
    {0xb1bdu, 1u, 2u, 0u},
    {0xb1beu, 1u, 2u, 0u},
    {0xb1bfu, 1u, 2u, 0u},
    {0xb1c0u, 1u, 2u, 0u},
    {0xb1c1u, 3u, 3u, 0u},
    {0xb1c4u, 2u, 2u, 0u},
};

static const FixedSpanOpcode hdma_builder_5_suffix[] = {
    {0xb1c6u, 1u, 5u, 2u},
    {0xb1c7u, 2u, 2u, 0u},
    {0xb1c9u, 3u, 4u, 1u},
    {0xb1ccu, 1u, 4u, 1u},
};

static const HdmaBuilderProfile hdma_builder_profiles[] = {
    {0xaf5eu, 0xaf9eu, 0x0000u, 0xa31cu, hdma_builder_0_prefix, hdma_builder_0_loop, hdma_builder_0_suffix, NULL, 7u, 12u, 14u, 0u, 16u},
    {0xafe0u, 0xb01du, 0x0000u, 0xa31cu, hdma_builder_1_prefix, hdma_builder_1_loop, hdma_builder_1_suffix, NULL, 7u, 12u, 13u, 0u, 16u},
    {0xb058u, 0xb081u, 0x0000u, 0xa31cu, hdma_builder_2_prefix, hdma_builder_2_loop, hdma_builder_2_suffix, NULL, 7u, 12u, 4u, 0u, 16u},
    {0xb0c3u, 0xb0f2u, 0xb0fbu, 0xa3b2u, hdma_builder_3_prefix, hdma_builder_3_loop, hdma_builder_3_suffix, hdma_builder_3_early, 9u, 10u, 5u, 4u, 32u},
    {0xb13bu, 0xb164u, 0xb16du, 0xa3b2u, hdma_builder_4_prefix, hdma_builder_4_loop, hdma_builder_4_suffix, hdma_builder_4_early, 8u, 9u, 5u, 4u, 32u},
    {0xb1a4u, 0xb1cdu, 0x0000u, 0xa532u, hdma_builder_5_prefix, hdma_builder_5_loop, hdma_builder_5_suffix, NULL, 7u, 12u, 4u, 0u, 32u},
};

typedef struct HdmaBuilderPlan {
    const HdmaBuilderProfile *profile;
    unsigned mode, phase, opcode_count;
    bool early, reset;
    uint32_t landing;
} HdmaBuilderPlan;

static bool PlanHdmaBuilder(uint32_t entry, unsigned phase, HdmaBuilderPlan *plan) {
    unsigned mode = 0u;
    while (mode < 6u && (0x850000u | hdma_builder_profiles[mode].entry) != entry)
        ++mode;
    if (mode == 6u)
        return false;
    const HdmaBuilderProfile *profile = hdma_builder_profiles + mode;
    const bool early = (mode == 3u && phase == 0x180u) ||
        (mode == 4u && (phase & 0x8000u));
    const bool reset = (mode == 0u && (uint16_t)(phase + 2u) == 0x20u) ||
        (mode == 1u && ((uint16_t)(phase - 2u) & 0x8000u));
    *plan = (HdmaBuilderPlan){profile, mode, phase,
        early ? profile->prefix_count - 2u + profile->early_count :
            profile->prefix_count + profile->loop_count * profile->iterations +
                profile->suffix_count - (mode < 2u && !reset),
        early, reset, 0x850000u | (early ? profile->early_terminal : profile->terminal)};
    return true;
}

static FixedSpanOpcode HdmaBuilderOpcode(const HdmaBuilderPlan *plan, unsigned index) {
    const HdmaBuilderProfile *profile = plan->profile;
    assert(index < plan->opcode_count);
    if (plan->early) {
        const unsigned prefix_count = profile->prefix_count - 2u;
        if (index >= prefix_count)
            return profile->early[index - prefix_count];
        FixedSpanOpcode opcode = profile->prefix[index];
        if (index + 1u == prefix_count)
            opcode.cycles = 3u;
        return opcode;
    }
    if (index < profile->prefix_count)
        return profile->prefix[index];
    index -= profile->prefix_count;
    const unsigned loop_opcodes = profile->loop_count * profile->iterations;
    if (index < loop_opcodes) {
        unsigned position, iteration;
        if (profile->loop_count == 12u) {
            position = index % 12u;
            iteration = index / 12u;
        } else if (profile->loop_count == 10u) {
            position = index % 10u;
            iteration = index / 10u;
        } else {
            assert(profile->loop_count == 9u);
            position = index % 9u;
            iteration = index / 9u;
        }
        FixedSpanOpcode opcode = profile->loop[position];
        if (position == 1u) {
            const unsigned initial = plan->mode == 2u || plan->mode == 5u ? 0u : plan->phase;
            const uint16_t source_index = plan->mode == 4u
                ? (uint16_t)(initial - 2u * iteration) : (uint16_t)(initial + 2u * iteration);
            opcode.cycles = 5u + ((profile->source & 0xffu) + source_index >= 0x100u);
        }
        if (position + 1u == profile->loop_count)
            opcode.cycles = iteration + 1u == profile->iterations ? 2u : 3u;
        return opcode;
    }
    index -= loop_opcodes;
    if (plan->mode < 2u) {
        const unsigned branch = plan->mode == 0u ? 5u : 4u;
        if (index > branch && !plan->reset)
            ++index;
        FixedSpanOpcode opcode = profile->suffix[index];
        if (index == branch)
            opcode.cycles = plan->reset ? 2u : 3u;
        return opcode;
    }
    return profile->suffix[index];
}
