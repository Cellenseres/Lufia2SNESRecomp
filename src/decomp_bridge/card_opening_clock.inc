typedef struct CardOpeningPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} CardOpeningPhase;

static const FixedSpanOpcode card_opening_af4b[] = {
    {0xaf4bu, 2u, 3u, 0u},
    {0xaf4du, 2u, 2u, 0u},
    {0xaf4fu, 2u, 3u, 1u},
    {0xaf51u, 3u, 3u, 0u},
    {0xaf54u, 2u, 4u, 2u},
    {0xaf56u, 2u, 3u, 0u},
    {0xaf58u, 2u, 4u, 2u},
    {0xaf5au, 2u, 4u, 2u},
    {0xaf5cu, 2u, 4u, 2u},
    {0xaf5eu, 2u, 4u, 2u},
    {0xaf60u, 2u, 7u, 4u},
    {0xaf62u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af65[] = {
    {0xaf65u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af68[] = {
    {0xaf68u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af6b[] = {
    {0xaf6bu, 2u, 4u, 2u},
    {0xaf6du, 2u, 7u, 4u},
    {0xaf6fu, 3u, 3u, 0u},
    {0xaf72u, 2u, 4u, 2u},
    {0xaf74u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af77[] = {
    {0xaf77u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af7a[] = {
    {0xaf7au, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af7d[] = {
    {0xaf7du, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af80[] = {
    {0xaf80u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af83[] = {
    {0xaf83u, 2u, 4u, 2u},
    {0xaf85u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af88[] = {
    {0xaf88u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af8b[] = {
    {0xaf8bu, 3u, 3u, 0u},
    {0xaf8eu, 2u, 4u, 2u},
    {0xaf90u, 3u, 3u, 0u},
    {0xaf93u, 2u, 4u, 2u},
    {0xaf95u, 2u, 4u, 2u},
    {0xaf97u, 2u, 4u, 2u},
    {0xaf99u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_af9c[] = {
    {0xaf9cu, 2u, 7u, 4u},
    {0xaf9eu, 3u, 3u, 0u},
    {0xafa1u, 2u, 4u, 2u},
    {0xafa3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afa6[] = {
    {0xafa6u, 2u, 7u, 4u},
    {0xafa8u, 3u, 8u, 4u},
    {0xafabu, 2u, 4u, 2u},
    {0xafadu, 3u, 5u, 2u},
    {0xafb0u, 2u, 4u, 2u},
    {0xafb2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afb5[] = {
    {0xafb5u, 2u, 3u, 0u},
};

static const FixedSpanOpcode card_opening_afb8[] = {
    {0xafb8u, 2u, 4u, 2u},
    {0xafbau, 2u, 7u, 5u},
    {0xafbcu, 1u, 2u, 0u},
    {0xafbdu, 1u, 2u, 0u},
    {0xafbeu, 2u, 4u, 2u},
    {0xafc0u, 2u, 7u, 5u},
    {0xafc2u, 1u, 2u, 0u},
    {0xafc3u, 1u, 2u, 0u},
    {0xafc4u, 2u, 4u, 2u},
    {0xafc6u, 2u, 4u, 2u},
    {0xafc8u, 2u, 3u, 0u},
    {0xafcau, 2u, 4u, 2u},
    {0xafccu, 2u, 6u, 4u},
    {0xafceu, 1u, 2u, 0u},
    {0xafcfu, 2u, 4u, 2u},
    {0xafd1u, 2u, 3u, 1u},
    {0xafd3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afd6[] = {
    {0xafd6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afd9[] = {
    {0xafd9u, 2u, 2u, 0u},
    {0xafdbu, 2u, 3u, 1u},
    {0xafddu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afe0[] = {
    {0xafe0u, 2u, 4u, 2u},
    {0xafe2u, 2u, 4u, 2u},
    {0xafe4u, 2u, 2u, 0u},
    {0xafe6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afe9[] = {
    {0xafe9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afec[] = {
    {0xafecu, 2u, 2u, 0u},
    {0xafeeu, 2u, 3u, 1u},
    {0xaff0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_aff3[] = {
    {0xaff3u, 2u, 5u, 2u},
};

static const FixedSpanOpcode card_opening_aff7[] = {
    {0xaff7u, 2u, 5u, 2u},
    {0xaff9u, 2u, 3u, 0u},
};

static const FixedSpanOpcode card_opening_affc[] = {
    {0xaffcu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_afff[] = {
    {0xafffu, 2u, 4u, 2u},
    {0xb001u, 3u, 5u, 2u},
    {0xb004u, 1u, 2u, 0u},
    {0xb005u, 1u, 2u, 0u},
    {0xb006u, 2u, 4u, 2u},
    {0xb008u, 2u, 4u, 2u},
    {0xb00au, 2u, 4u, 2u},
    {0xb00cu, 2u, 7u, 4u},
    {0xb00eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b011[] = {
    {0xb011u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b014[] = {
    {0xb014u, 3u, 3u, 0u},
    {0xb017u, 2u, 4u, 2u},
    {0xb019u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b01c[] = {
    {0xb01cu, 2u, 4u, 2u},
    {0xb01eu, 1u, 2u, 0u},
    {0xb01fu, 3u, 3u, 0u},
    {0xb022u, 2u, 4u, 2u},
    {0xb024u, 2u, 7u, 4u},
};

static const FixedSpanOpcode card_opening_b18e[] = {
    {0xb18eu, 2u, 3u, 0u},
    {0xb190u, 2u, 2u, 0u},
    {0xb192u, 2u, 3u, 1u},
    {0xb194u, 3u, 3u, 0u},
    {0xb197u, 2u, 4u, 2u},
    {0xb199u, 2u, 3u, 0u},
    {0xb19bu, 2u, 4u, 2u},
    {0xb19du, 1u, 2u, 0u},
    {0xb19eu, 1u, 2u, 0u},
    {0xb19fu, 1u, 2u, 0u},
    {0xb1a0u, 2u, 4u, 2u},
    {0xb1a2u, 1u, 2u, 0u},
    {0xb1a3u, 2u, 4u, 2u},
    {0xb1a5u, 2u, 4u, 2u},
    {0xb1a7u, 3u, 5u, 2u},
    {0xb1aau, 1u, 2u, 0u},
    {0xb1abu, 3u, 3u, 0u},
    {0xb1aeu, 1u, 2u, 0u},
    {0xb1afu, 1u, 2u, 0u},
    {0xb1b0u, 1u, 2u, 0u},
    {0xb1b1u, 1u, 2u, 0u},
    {0xb1b2u, 2u, 4u, 2u},
    {0xb1b4u, 1u, 2u, 0u},
    {0xb1b5u, 2u, 4u, 2u},
    {0xb1b7u, 1u, 2u, 0u},
    {0xb1b8u, 2u, 4u, 2u},
    {0xb1bau, 1u, 2u, 0u},
    {0xb1bbu, 2u, 4u, 2u},
    {0xb1bdu, 2u, 7u, 5u},
    {0xb1bfu, 2u, 4u, 2u},
    {0xb1c1u, 1u, 2u, 0u},
    {0xb1c2u, 1u, 2u, 0u},
    {0xb1c3u, 2u, 7u, 5u},
    {0xb1c5u, 2u, 4u, 2u},
    {0xb1c7u, 1u, 2u, 0u},
    {0xb1c8u, 1u, 2u, 0u},
    {0xb1c9u, 2u, 7u, 5u},
    {0xb1cbu, 2u, 4u, 2u},
    {0xb1cdu, 2u, 4u, 2u},
    {0xb1cfu, 2u, 3u, 0u},
    {0xb1d1u, 2u, 2u, 0u},
    {0xb1d3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1d6[] = {
    {0xb1d6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1d9[] = {
    {0xb1d9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1dc[] = {
    {0xb1dcu, 2u, 2u, 0u},
    {0xb1deu, 2u, 3u, 1u},
    {0xb1e0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1e3[] = {
    {0xb1e3u, 2u, 4u, 2u},
    {0xb1e5u, 2u, 4u, 2u},
    {0xb1e7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1e7[] = {
    {0xb1e7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1ea[] = {
    {0xb1eau, 2u, 2u, 0u},
    {0xb1ecu, 2u, 3u, 1u},
    {0xb1eeu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1f1[] = {
    {0xb1f1u, 2u, 5u, 2u},
};

static const FixedSpanOpcode card_opening_b1f5[] = {
    {0xb1f5u, 2u, 3u, 0u},
    {0xb1f7u, 2u, 4u, 2u},
    {0xb1f9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b1fc[] = {
    {0xb1fcu, 2u, 4u, 2u},
    {0xb1feu, 2u, 4u, 2u},
    {0xb200u, 2u, 4u, 2u},
    {0xb202u, 1u, 2u, 0u},
};

static const FixedSpanOpcode card_opening_b205[] = {
    {0xb205u, 3u, 4u, 0u},
    {0xb20du, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b208[] = {
    {0xb208u, 2u, 7u, 4u},
    {0xb20au, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b20d[] = {
    {0xb20du, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b210[] = {
    {0xb210u, 2u, 4u, 2u},
    {0xb212u, 2u, 4u, 2u},
    {0xb214u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b217[] = {
    {0xb217u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b21a[] = {
    {0xb21au, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b21d[] = {
    {0xb21du, 2u, 3u, 0u},
    {0xb21fu, 2u, 2u, 0u},
    {0xb221u, 2u, 3u, 1u},
    {0xb223u, 2u, 3u, 1u},
    {0xb225u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_opening_b228[] = {
    {0xb228u, 2u, 3u, 0u},
};

static const CardOpeningPhase card_opening_phases[] = {
    {0xaf4bu, 0x0000u, 0x0000u, 0u, sizeof card_opening_af4b / sizeof card_opening_af4b[0], card_opening_af4b},
    {0xaf65u, 0x0000u, 0x0000u, 0u, sizeof card_opening_af65 / sizeof card_opening_af65[0], card_opening_af65},
    {0xaf68u, 0x0000u, 0x0000u, 0u, sizeof card_opening_af68 / sizeof card_opening_af68[0], card_opening_af68},
    {0xaf6bu, 0x0000u, 0x0000u, 0u, sizeof card_opening_af6b / sizeof card_opening_af6b[0], card_opening_af6b},
    {0xaf77u, 0x0000u, 0x0000u, 0u, sizeof card_opening_af77 / sizeof card_opening_af77[0], card_opening_af77},
    {0xaf7au, 0x0000u, 0x0000u, 0u, sizeof card_opening_af7a / sizeof card_opening_af7a[0], card_opening_af7a},
    {0xaf7du, 0x0000u, 0x0000u, 0u, sizeof card_opening_af7d / sizeof card_opening_af7d[0], card_opening_af7d},
    {0xaf80u, 0x0000u, 0x0000u, 0u, sizeof card_opening_af80 / sizeof card_opening_af80[0], card_opening_af80},
    {0xaf83u, 0x0000u, 0x0000u, 0u, sizeof card_opening_af83 / sizeof card_opening_af83[0], card_opening_af83},
    {0xaf88u, 0x0000u, 0x0000u, 0u, sizeof card_opening_af88 / sizeof card_opening_af88[0], card_opening_af88},
    {0xaf8bu, 0x0000u, 0x0000u, 0u, sizeof card_opening_af8b / sizeof card_opening_af8b[0], card_opening_af8b},
    {0xaf9cu, 0x0000u, 0x0000u, 0u, sizeof card_opening_af9c / sizeof card_opening_af9c[0], card_opening_af9c},
    {0xafa6u, 0x0000u, 0x0000u, 0u, sizeof card_opening_afa6 / sizeof card_opening_afa6[0], card_opening_afa6},
    {0xafb5u, 0x0000u, 0xafb7u, 0u, sizeof card_opening_afb5 / sizeof card_opening_afb5[0], card_opening_afb5},
    {0xafb8u, 0x0000u, 0x0000u, 0u, sizeof card_opening_afb8 / sizeof card_opening_afb8[0], card_opening_afb8},
    {0xafd6u, 0x0000u, 0x0000u, 1u, sizeof card_opening_afd6 / sizeof card_opening_afd6[0], card_opening_afd6},
    {0xafd9u, 0x0000u, 0x0000u, 1u, sizeof card_opening_afd9 / sizeof card_opening_afd9[0], card_opening_afd9},
    {0xafe0u, 0x0000u, 0x0000u, 1u, sizeof card_opening_afe0 / sizeof card_opening_afe0[0], card_opening_afe0},
    {0xafe9u, 0x0000u, 0x0000u, 1u, sizeof card_opening_afe9 / sizeof card_opening_afe9[0], card_opening_afe9},
    {0xafecu, 0x0000u, 0x0000u, 1u, sizeof card_opening_afec / sizeof card_opening_afec[0], card_opening_afec},
    {0xaff3u, 0xaff5u, 0x0000u, 1u, sizeof card_opening_aff3 / sizeof card_opening_aff3[0], card_opening_aff3},
    {0xaff7u, 0x0000u, 0xaffbu, 1u, sizeof card_opening_aff7 / sizeof card_opening_aff7[0], card_opening_aff7},
    {0xaffcu, 0x0000u, 0x0000u, 0u, sizeof card_opening_affc / sizeof card_opening_affc[0], card_opening_affc},
    {0xafffu, 0x0000u, 0x0000u, 0u, sizeof card_opening_afff / sizeof card_opening_afff[0], card_opening_afff},
    {0xb011u, 0x0000u, 0x0000u, 0u, sizeof card_opening_b011 / sizeof card_opening_b011[0], card_opening_b011},
    {0xb014u, 0x0000u, 0x0000u, 0u, sizeof card_opening_b014 / sizeof card_opening_b014[0], card_opening_b014},
    {0xb01cu, 0x0000u, 0xb026u, 0u, sizeof card_opening_b01c / sizeof card_opening_b01c[0], card_opening_b01c},
    {0xb18eu, 0x0000u, 0x0000u, 0u, sizeof card_opening_b18e / sizeof card_opening_b18e[0], card_opening_b18e},
    {0xb1d6u, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1d6 / sizeof card_opening_b1d6[0], card_opening_b1d6},
    {0xb1d9u, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1d9 / sizeof card_opening_b1d9[0], card_opening_b1d9},
    {0xb1dcu, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1dc / sizeof card_opening_b1dc[0], card_opening_b1dc},
    {0xb1e3u, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1e3 / sizeof card_opening_b1e3[0], card_opening_b1e3},
    {0xb1e7u, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1e7 / sizeof card_opening_b1e7[0], card_opening_b1e7},
    {0xb1eau, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1ea / sizeof card_opening_b1ea[0], card_opening_b1ea},
    {0xb1f1u, 0xb1f3u, 0x0000u, 1u, sizeof card_opening_b1f1 / sizeof card_opening_b1f1[0], card_opening_b1f1},
    {0xb1f5u, 0x0000u, 0x0000u, 1u, sizeof card_opening_b1f5 / sizeof card_opening_b1f5[0], card_opening_b1f5},
    {0xb1fcu, 0xb203u, 0x0000u, 0u, sizeof card_opening_b1fc / sizeof card_opening_b1fc[0], card_opening_b1fc},
    {0xb205u, 0x0000u, 0x0000u, 0u, sizeof card_opening_b205 / sizeof card_opening_b205[0], card_opening_b205},
    {0xb208u, 0x0000u, 0x0000u, 0u, sizeof card_opening_b208 / sizeof card_opening_b208[0], card_opening_b208},
    {0xb20du, 0x0000u, 0x0000u, 0u, sizeof card_opening_b20d / sizeof card_opening_b20d[0], card_opening_b20d},
    {0xb210u, 0x0000u, 0x0000u, 0u, sizeof card_opening_b210 / sizeof card_opening_b210[0], card_opening_b210},
    {0xb217u, 0x0000u, 0x0000u, 0u, sizeof card_opening_b217 / sizeof card_opening_b217[0], card_opening_b217},
    {0xb21au, 0x0000u, 0x0000u, 0u, sizeof card_opening_b21a / sizeof card_opening_b21a[0], card_opening_b21a},
    {0xb21du, 0x0000u, 0x0000u, 0u, sizeof card_opening_b21d / sizeof card_opening_b21d[0], card_opening_b21d},
    {0xb228u, 0x0000u, 0xb22au, 1u, sizeof card_opening_b228 / sizeof card_opening_b228[0], card_opening_b228},
};

static const CardOpeningPhase *FindCardOpeningPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof card_opening_phases / sizeof card_opening_phases[0]; ++i)
        if (card_opening_phases[i].pc == pc)
            return card_opening_phases + i;
    return NULL;
}

static void CardOpeningOpcode(CardScoreClock *clock, const FixedSpanOpcode *opcode) {
    const CpuState *cpu = clock->call.cpu;
    unsigned cycles = opcode->cycles;
    if (opcode->pc == 0xb001u || opcode->pc == 0xb1a7u) {
        const unsigned offset = opcode->pc == 0xb001u ? 0x15u : 0x17u;
        unsigned index = cpu->ram[offset] | (unsigned)cpu->ram[offset + 1u] << 8;
        const unsigned base = opcode->pc == 0xb001u ? 0x7036u : 0x70c6u;
        if (opcode->pc == 0xb1a7u)
            index = (uint16_t)(index * 2u);
        cycles += (base >> 8) != ((base + index) >> 8);
    }
    CardScoreOpcode(clock, opcode->pc, opcode->fetched, cycles, opcode->data_bytes);
}

static void AppendCardOpeningPhase(CardScoreClock *clock, uint16_t pc) {
    const CardOpeningPhase *phase = FindCardOpeningPhase(pc);
    assert(phase && !phase->branch);
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *opcode = phase->opcodes + i;
        CardOpeningOpcode(clock, opcode);
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
}

static bool PrepareCardOpening(CardScoreClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        (pc >> 16) != 0x84u || !CardScoreRegistersSupported(cpu))
        return false;
    const CardOpeningPhase *phase = FindCardOpeningPhase((uint16_t)pc);
    if (!phase || cpu->m_flag != phase->byte_mode)
        return false;
    if (!phase->branch) {
        AppendCardOpeningPhase(clock, (uint16_t)pc);
    } else {
        for (unsigned i = 0u; i < phase->count; ++i) {
            const FixedSpanOpcode *opcode = phase->opcodes + i;
            CardOpeningOpcode(clock, opcode);
        }
        if (phase->branch == 0xb203u) {
            const uint16_t hand = cpu->ram[0x17u] | (uint16_t)cpu->ram[0x18u] << 8;
            const bool negative = ((uint16_t)(hand - 1u) & 0x8000u) != 0u;
            CardScoreOpcode(clock, 0xb203u, 2u, negative ? 3u : 2u, 0u);
            AppendCardOpeningPhase(clock, negative ? 0xb208u : 0xb205u);
        } else {
            const bool opening = phase->branch == 0xaff5u;
            const bool again = cpu->ram[opening ? 6u : 4u] != 1u;
            CardScoreOpcode(clock, phase->branch, 2u, again ? 3u : 2u, 0u);
            AppendCardOpeningPhase(clock, opening ? (again ? 0xafe9u : 0xaff7u)
                                                  : (again ? 0xb1e7u : 0xb1f5u));
        }
    }
    return CardScoreWindowSupported(clock);
}

static uint8_t CardOpeningChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardScoreClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareCardOpening(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeCardOpening(CpuState *cpu, uint32_t entry,
                                           ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardScoreClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareCardOpening(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, CardOpeningChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
