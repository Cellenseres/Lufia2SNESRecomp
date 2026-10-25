typedef struct TableServicePhase {
    uint16_t pc, branch, return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} TableServicePhase;

static const FixedSpanOpcode table_service_e9b1[] = {
    {0xe9b1u, 1u, 4u, 2u},
    {0xe9b2u, 1u, 4u, 2u},
    {0xe9b3u, 1u, 3u, 1u},
    {0xe9b4u, 2u, 3u, 0u},
    {0xe9b6u, 2u, 3u, 0u},
    {0xe9b8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode table_service_e9bb[] = {
    {0xe9bbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode table_service_e9be[] = {
    {0xe9beu, 2u, 2u, 0u},
    {0xe9c0u, 2u, 3u, 1u},
    {0xe9c2u, 2u, 3u, 1u},
    {0xe9c4u, 2u, 2u, 0u},
    {0xe9c6u, 2u, 3u, 1u},
    {0xe9c8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode table_service_e9cb[] = {
    {0xe9cbu, 2u, 2u, 0u},
    {0xe9cdu, 2u, 3u, 1u},
};

static const FixedSpanOpcode table_service_e9d1[] = {
    {0xe9d1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode table_service_e9d6[] = {
    {0xe9d6u, 2u, 2u, 0u},
    {0xe9d8u, 3u, 4u, 1u},
    {0xe9dbu, 2u, 2u, 0u},
    {0xe9ddu, 2u, 5u, 2u},
    {0xe9dfu, 1u, 4u, 1u},
    {0xe9e0u, 1u, 5u, 2u},
    {0xe9e1u, 1u, 5u, 2u},
};

static const FixedSpanOpcode table_service_e9df[] = {
    {0xe9dfu, 1u, 4u, 1u},
    {0xe9e0u, 1u, 5u, 2u},
    {0xe9e1u, 1u, 5u, 2u},
};

static const FixedSpanOpcode table_service_ec4f[] = {
    {0xec4fu, 1u, 3u, 1u},
    {0xec50u, 2u, 3u, 0u},
    {0xec52u, 3u, 3u, 0u},
    {0xec55u, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_ec59[] = {
    {0xec59u, 1u, 2u, 0u},
    {0xec5au, 3u, 3u, 0u},
    {0xec5du, 3u, 5u, 2u},
    {0xec60u, 3u, 3u, 0u},
    {0xec63u, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_ec67[] = {
    {0xec67u, 1u, 2u, 0u},
    {0xec68u, 3u, 3u, 0u},
    {0xec6bu, 3u, 5u, 2u},
    {0xec6eu, 3u, 3u, 0u},
    {0xec71u, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_ec75[] = {
    {0xec75u, 1u, 2u, 0u},
    {0xec76u, 3u, 3u, 0u},
    {0xec79u, 3u, 5u, 2u},
    {0xec7cu, 3u, 3u, 0u},
    {0xec7fu, 3u, 3u, 0u},
    {0xec82u, 3u, 3u, 0u},
    {0xec85u, 3u, 7u, 2u},
    {0xec88u, 1u, 2u, 0u},
    {0xec89u, 1u, 2u, 0u},
    {0xec8au, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ec85[] = {
    {0xec85u, 3u, 7u, 2u},
    {0xec88u, 1u, 2u, 0u},
    {0xec89u, 1u, 2u, 0u},
    {0xec8au, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ec8d[] = {
    {0xec8du, 3u, 3u, 0u},
    {0xec90u, 3u, 3u, 0u},
    {0xec93u, 3u, 7u, 2u},
    {0xec96u, 3u, 7u, 2u},
    {0xec99u, 1u, 2u, 0u},
    {0xec9au, 1u, 2u, 0u},
    {0xec9bu, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ec93[] = {
    {0xec93u, 3u, 7u, 2u},
    {0xec96u, 3u, 7u, 2u},
    {0xec99u, 1u, 2u, 0u},
    {0xec9au, 1u, 2u, 0u},
    {0xec9bu, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ec9e[] = {
    {0xec9eu, 3u, 3u, 0u},
    {0xeca1u, 1u, 2u, 0u},
    {0xeca2u, 3u, 3u, 0u},
    {0xeca5u, 2u, 4u, 2u},
    {0xeca7u, 2u, 3u, 0u},
    {0xeca9u, 4u, 5u, 1u},
    {0xecadu, 2u, 3u, 0u},
    {0xecafu, 3u, 3u, 0u},
    {0xecb2u, 3u, 7u, 2u},
    {0xecb5u, 1u, 2u, 0u},
    {0xecb6u, 1u, 2u, 0u},
    {0xecb7u, 1u, 2u, 0u},
    {0xecb8u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_eca7[] = {
    {0xeca7u, 2u, 3u, 0u},
    {0xeca9u, 4u, 5u, 1u},
    {0xecadu, 2u, 3u, 0u},
    {0xecafu, 3u, 3u, 0u},
    {0xecb2u, 3u, 7u, 2u},
    {0xecb5u, 1u, 2u, 0u},
    {0xecb6u, 1u, 2u, 0u},
    {0xecb7u, 1u, 2u, 0u},
    {0xecb8u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ecbc[] = {
    {0xecbcu, 3u, 3u, 0u},
    {0xecbfu, 2u, 4u, 2u},
    {0xecc1u, 4u, 6u, 2u},
    {0xecc5u, 3u, 7u, 2u},
    {0xecc8u, 1u, 2u, 0u},
    {0xecc9u, 1u, 2u, 0u},
    {0xeccau, 1u, 2u, 0u},
    {0xeccbu, 1u, 2u, 0u},
    {0xecccu, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ecc1[] = {
    {0xecc1u, 4u, 6u, 2u},
    {0xecc5u, 3u, 7u, 2u},
    {0xecc8u, 1u, 2u, 0u},
    {0xecc9u, 1u, 2u, 0u},
    {0xeccau, 1u, 2u, 0u},
    {0xeccbu, 1u, 2u, 0u},
    {0xecccu, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ecd0[] = {
    {0xecd0u, 3u, 3u, 0u},
    {0xecd3u, 3u, 3u, 0u},
    {0xecd6u, 3u, 3u, 0u},
    {0xecd9u, 2u, 4u, 2u},
    {0xecdbu, 3u, 3u, 0u},
    {0xecdeu, 2u, 4u, 2u},
    {0xece0u, 1u, 2u, 0u},
    {0xece1u, 3u, 7u, 2u},
    {0xece4u, 1u, 2u, 0u},
    {0xece5u, 1u, 2u, 0u},
    {0xece6u, 3u, 3u, 0u},
    {0xece9u, 3u, 7u, 2u},
    {0xececu, 1u, 2u, 0u},
    {0xecedu, 1u, 2u, 0u},
    {0xeceeu, 1u, 2u, 0u},
    {0xecefu, 1u, 2u, 0u},
    {0xecf0u, 3u, 3u, 0u},
    {0xecf3u, 3u, 7u, 2u},
    {0xecf6u, 1u, 2u, 0u},
    {0xecf7u, 1u, 2u, 0u},
    {0xecf8u, 3u, 3u, 0u},
    {0xecfbu, 3u, 7u, 2u},
    {0xecfeu, 1u, 2u, 0u},
    {0xecffu, 1u, 2u, 0u},
    {0xed00u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ecdb[] = {
    {0xecdbu, 3u, 3u, 0u},
    {0xecdeu, 2u, 4u, 2u},
    {0xece0u, 1u, 2u, 0u},
    {0xece1u, 3u, 7u, 2u},
    {0xece4u, 1u, 2u, 0u},
    {0xece5u, 1u, 2u, 0u},
    {0xece6u, 3u, 3u, 0u},
    {0xece9u, 3u, 7u, 2u},
    {0xececu, 1u, 2u, 0u},
    {0xecedu, 1u, 2u, 0u},
    {0xeceeu, 1u, 2u, 0u},
    {0xecefu, 1u, 2u, 0u},
    {0xecf0u, 3u, 3u, 0u},
    {0xecf3u, 3u, 7u, 2u},
    {0xecf6u, 1u, 2u, 0u},
    {0xecf7u, 1u, 2u, 0u},
    {0xecf8u, 3u, 3u, 0u},
    {0xecfbu, 3u, 7u, 2u},
    {0xecfeu, 1u, 2u, 0u},
    {0xecffu, 1u, 2u, 0u},
    {0xed00u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ece0[] = {
    {0xece0u, 1u, 2u, 0u},
    {0xece1u, 3u, 7u, 2u},
    {0xece4u, 1u, 2u, 0u},
    {0xece5u, 1u, 2u, 0u},
    {0xece6u, 3u, 3u, 0u},
    {0xece9u, 3u, 7u, 2u},
    {0xececu, 1u, 2u, 0u},
    {0xecedu, 1u, 2u, 0u},
    {0xeceeu, 1u, 2u, 0u},
    {0xecefu, 1u, 2u, 0u},
    {0xecf0u, 3u, 3u, 0u},
    {0xecf3u, 3u, 7u, 2u},
    {0xecf6u, 1u, 2u, 0u},
    {0xecf7u, 1u, 2u, 0u},
    {0xecf8u, 3u, 3u, 0u},
    {0xecfbu, 3u, 7u, 2u},
    {0xecfeu, 1u, 2u, 0u},
    {0xecffu, 1u, 2u, 0u},
    {0xed00u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ed04[] = {
    {0xed04u, 1u, 2u, 0u},
    {0xed05u, 1u, 2u, 0u},
    {0xed06u, 3u, 3u, 0u},
    {0xed09u, 1u, 2u, 0u},
    {0xed0au, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_ed0e[] = {
    {0xed0eu, 3u, 3u, 0u},
    {0xed11u, 3u, 3u, 0u},
    {0xed14u, 3u, 3u, 0u},
    {0xed17u, 3u, 7u, 2u},
    {0xed1au, 1u, 2u, 0u},
    {0xed1bu, 1u, 2u, 0u},
    {0xed1cu, 3u, 7u, 2u},
    {0xed1fu, 1u, 2u, 0u},
    {0xed20u, 1u, 2u, 0u},
    {0xed21u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ed14[] = {
    {0xed14u, 3u, 3u, 0u},
    {0xed17u, 3u, 7u, 2u},
    {0xed1au, 1u, 2u, 0u},
    {0xed1bu, 1u, 2u, 0u},
    {0xed1cu, 3u, 7u, 2u},
    {0xed1fu, 1u, 2u, 0u},
    {0xed20u, 1u, 2u, 0u},
    {0xed21u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ed24[] = {
    {0xed24u, 3u, 6u, 2u},
};

static const FixedSpanOpcode table_service_ed27[] = {
    {0xed27u, 3u, 5u, 2u},
    {0xed2au, 3u, 3u, 0u},
    {0xed2du, 3u, 3u, 0u},
    {0xed30u, 3u, 7u, 2u},
    {0xed33u, 1u, 2u, 0u},
    {0xed34u, 1u, 2u, 0u},
    {0xed35u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ed30[] = {
    {0xed30u, 3u, 7u, 2u},
    {0xed33u, 1u, 2u, 0u},
    {0xed34u, 1u, 2u, 0u},
    {0xed35u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ed38[] = {
    {0xed38u, 2u, 4u, 2u},
    {0xed3au, 3u, 3u, 0u},
};

static const FixedSpanOpcode table_service_ed3f[] = {
    {0xed3fu, 3u, 3u, 0u},
    {0xed42u, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_ed46[] = {
    {0xed46u, 3u, 3u, 0u},
};

static const FixedSpanOpcode table_service_ed4b[] = {
    {0xed4bu, 3u, 3u, 0u},
};

static const FixedSpanOpcode table_service_ed50[] = {
    {0xed50u, 3u, 3u, 0u},
    {0xed53u, 3u, 5u, 2u},
    {0xed56u, 3u, 3u, 0u},
    {0xed59u, 3u, 5u, 2u},
    {0xed5cu, 2u, 3u, 0u},
    {0xed72u, 3u, 5u, 2u},
    {0xed75u, 3u, 5u, 2u},
    {0xed78u, 3u, 5u, 2u},
    {0xed7bu, 3u, 5u, 2u},
    {0xed7eu, 3u, 3u, 0u},
    {0xed81u, 3u, 5u, 2u},
    {0xed84u, 3u, 5u, 2u},
    {0xed87u, 3u, 5u, 2u},
    {0xed8au, 3u, 5u, 2u},
    {0xed8du, 3u, 5u, 2u},
    {0xed90u, 3u, 5u, 2u},
    {0xed93u, 3u, 5u, 2u},
    {0xed96u, 3u, 3u, 0u},
    {0xed99u, 3u, 5u, 2u},
    {0xed9cu, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_ed5e[] = {
    {0xed5eu, 3u, 3u, 0u},
    {0xed61u, 3u, 5u, 2u},
    {0xed64u, 3u, 3u, 0u},
    {0xed67u, 3u, 5u, 2u},
    {0xed6au, 2u, 3u, 0u},
    {0xed72u, 3u, 5u, 2u},
    {0xed75u, 3u, 5u, 2u},
    {0xed78u, 3u, 5u, 2u},
    {0xed7bu, 3u, 5u, 2u},
    {0xed7eu, 3u, 3u, 0u},
    {0xed81u, 3u, 5u, 2u},
    {0xed84u, 3u, 5u, 2u},
    {0xed87u, 3u, 5u, 2u},
    {0xed8au, 3u, 5u, 2u},
    {0xed8du, 3u, 5u, 2u},
    {0xed90u, 3u, 5u, 2u},
    {0xed93u, 3u, 5u, 2u},
    {0xed96u, 3u, 3u, 0u},
    {0xed99u, 3u, 5u, 2u},
    {0xed9cu, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_ed6c[] = {
    {0xed6cu, 3u, 5u, 2u},
    {0xed6fu, 3u, 5u, 2u},
    {0xed72u, 3u, 5u, 2u},
    {0xed75u, 3u, 5u, 2u},
    {0xed78u, 3u, 5u, 2u},
    {0xed7bu, 3u, 5u, 2u},
    {0xed7eu, 3u, 3u, 0u},
    {0xed81u, 3u, 5u, 2u},
    {0xed84u, 3u, 5u, 2u},
    {0xed87u, 3u, 5u, 2u},
    {0xed8au, 3u, 5u, 2u},
    {0xed8du, 3u, 5u, 2u},
    {0xed90u, 3u, 5u, 2u},
    {0xed93u, 3u, 5u, 2u},
    {0xed96u, 3u, 3u, 0u},
    {0xed99u, 3u, 5u, 2u},
    {0xed9cu, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_edfa[] = {
    {0xedfau, 1u, 3u, 1u},
    {0xedfbu, 2u, 3u, 0u},
    {0xedfdu, 3u, 3u, 0u},
    {0xee00u, 2u, 4u, 2u},
    {0xee02u, 3u, 3u, 0u},
    {0xee05u, 2u, 4u, 2u},
    {0xee07u, 2u, 2u, 0u},
    {0xee09u, 2u, 3u, 1u},
    {0xee0bu, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_ee0f[] = {
    {0xee0fu, 2u, 3u, 0u},
    {0xee11u, 3u, 3u, 0u},
    {0xee14u, 3u, 3u, 0u},
    {0xee17u, 3u, 5u, 2u},
    {0xee1au, 3u, 7u, 2u},
    {0xee1du, 1u, 2u, 0u},
    {0xee1eu, 1u, 2u, 0u},
    {0xee1fu, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ee17[] = {
    {0xee17u, 3u, 5u, 2u},
    {0xee1au, 3u, 7u, 2u},
    {0xee1du, 1u, 2u, 0u},
    {0xee1eu, 1u, 2u, 0u},
    {0xee1fu, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ee22[] = {
    {0xee22u, 2u, 3u, 0u},
    {0xee24u, 3u, 3u, 0u},
    {0xee27u, 2u, 4u, 2u},
    {0xee29u, 3u, 3u, 0u},
    {0xee2cu, 2u, 4u, 2u},
    {0xee2eu, 2u, 2u, 0u},
    {0xee30u, 2u, 3u, 1u},
    {0xee32u, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_ee36[] = {
    {0xee36u, 2u, 3u, 0u},
    {0xee38u, 3u, 3u, 0u},
    {0xee3bu, 3u, 3u, 0u},
    {0xee3eu, 3u, 5u, 2u},
    {0xee41u, 3u, 7u, 2u},
    {0xee44u, 1u, 2u, 0u},
    {0xee45u, 1u, 2u, 0u},
    {0xee46u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ee3e[] = {
    {0xee3eu, 3u, 5u, 2u},
    {0xee41u, 3u, 7u, 2u},
    {0xee44u, 1u, 2u, 0u},
    {0xee45u, 1u, 2u, 0u},
    {0xee46u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_ee49[] = {
    {0xee49u, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_f12f[] = {
    {0xf12fu, 1u, 3u, 1u},
    {0xf130u, 1u, 4u, 2u},
    {0xf131u, 1u, 4u, 2u},
    {0xf132u, 1u, 3u, 1u},
    {0xf133u, 2u, 3u, 0u},
    {0xf135u, 2u, 3u, 0u},
    {0xf137u, 4u, 5u, 1u},
    {0xf13bu, 2u, 2u, 0u},
};

static const FixedSpanOpcode table_service_f13f[] = {
    {0xf13fu, 2u, 2u, 0u},
    {0xf141u, 4u, 5u, 1u},
    {0xf145u, 2u, 2u, 0u},
    {0xf147u, 3u, 4u, 1u},
    {0xf14au, 3u, 3u, 0u},
    {0xf14du, 3u, 5u, 2u},
    {0xf150u, 2u, 2u, 0u},
    {0xf152u, 3u, 4u, 1u},
    {0xf155u, 2u, 2u, 0u},
    {0xf157u, 3u, 4u, 1u},
    {0xf15au, 2u, 2u, 0u},
    {0xf15cu, 3u, 4u, 1u},
    {0xf15fu, 2u, 3u, 0u},
    {0xf16eu, 1u, 4u, 1u},
    {0xf16fu, 1u, 5u, 2u},
    {0xf170u, 1u, 5u, 2u},
    {0xf171u, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_f161[] = {
    {0xf161u, 2u, 2u, 0u},
};

static const FixedSpanOpcode table_service_f165[] = {
    {0xf165u, 2u, 2u, 0u},
    {0xf167u, 4u, 5u, 1u},
    {0xf16bu, 3u, 4u, 1u},
    {0xf16eu, 1u, 4u, 1u},
    {0xf16fu, 1u, 5u, 2u},
    {0xf170u, 1u, 5u, 2u},
    {0xf171u, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_f16e[] = {
    {0xf16eu, 1u, 4u, 1u},
    {0xf16fu, 1u, 5u, 2u},
    {0xf170u, 1u, 5u, 2u},
    {0xf171u, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_f173[] = {
    {0xf173u, 1u, 3u, 1u},
    {0xf174u, 2u, 3u, 0u},
    {0xf176u, 2u, 3u, 0u},
    {0xf178u, 2u, 2u, 0u},
    {0xf17au, 2u, 3u, 1u},
    {0xf17cu, 3u, 3u, 0u},
    {0xf17fu, 2u, 4u, 2u},
    {0xf181u, 2u, 3u, 0u},
    {0xf183u, 3u, 3u, 0u},
    {0xf186u, 3u, 3u, 0u},
    {0xf189u, 2u, 7u, 5u},
};

static const FixedSpanOpcode table_service_f18d[] = {
    {0xf18du, 4u, 6u, 2u},
    {0xf191u, 1u, 2u, 0u},
    {0xf192u, 1u, 2u, 0u},
    {0xf193u, 1u, 2u, 0u},
    {0xf194u, 1u, 2u, 0u},
    {0xf195u, 2u, 3u, 0u},
    {0xf189u, 2u, 7u, 5u},
};

static const FixedSpanOpcode table_service_f197[] = {
    {0xf197u, 4u, 6u, 2u},
    {0xf19bu, 1u, 4u, 1u},
};

static const FixedSpanOpcode table_service_f78f[] = {
    {0xf78fu, 2u, 3u, 0u},
    {0xf791u, 3u, 3u, 0u},
    {0xf794u, 1u, 2u, 0u},
    {0xf795u, 2u, 4u, 2u},
    {0xf797u, 3u, 3u, 0u},
};

static const FixedSpanOpcode table_service_f79c[] = {
    {0xf79cu, 3u, 3u, 0u},
    {0xf79fu, 3u, 3u, 0u},
    {0xf7a2u, 2u, 4u, 2u},
    {0xf7a4u, 2u, 3u, 0u},
    {0xf7a6u, 4u, 5u, 1u},
    {0xf7aau, 2u, 3u, 0u},
    {0xf7acu, 3u, 3u, 0u},
    {0xf7afu, 3u, 7u, 2u},
    {0xf7b2u, 1u, 2u, 0u},
    {0xf7b3u, 1u, 2u, 0u},
    {0xf7b4u, 1u, 2u, 0u},
    {0xf7b5u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_f79f[] = {
    {0xf79fu, 3u, 3u, 0u},
    {0xf7a2u, 2u, 4u, 2u},
    {0xf7a4u, 2u, 3u, 0u},
    {0xf7a6u, 4u, 5u, 1u},
    {0xf7aau, 2u, 3u, 0u},
    {0xf7acu, 3u, 3u, 0u},
    {0xf7afu, 3u, 7u, 2u},
    {0xf7b2u, 1u, 2u, 0u},
    {0xf7b3u, 1u, 2u, 0u},
    {0xf7b4u, 1u, 2u, 0u},
    {0xf7b5u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_f7a4[] = {
    {0xf7a4u, 2u, 3u, 0u},
    {0xf7a6u, 4u, 5u, 1u},
    {0xf7aau, 2u, 3u, 0u},
    {0xf7acu, 3u, 3u, 0u},
    {0xf7afu, 3u, 7u, 2u},
    {0xf7b2u, 1u, 2u, 0u},
    {0xf7b3u, 1u, 2u, 0u},
    {0xf7b4u, 1u, 2u, 0u},
    {0xf7b5u, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_f7b9[] = {
    {0xf7b9u, 3u, 3u, 0u},
    {0xf7bcu, 3u, 3u, 0u},
    {0xf7bfu, 2u, 4u, 2u},
    {0xf7c1u, 4u, 6u, 2u},
    {0xf7c5u, 3u, 7u, 2u},
    {0xf7c8u, 1u, 2u, 0u},
    {0xf7c9u, 1u, 2u, 0u},
    {0xf7cau, 1u, 2u, 0u},
    {0xf7cbu, 1u, 2u, 0u},
    {0xf7ccu, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_f7c1[] = {
    {0xf7c1u, 4u, 6u, 2u},
    {0xf7c5u, 3u, 7u, 2u},
    {0xf7c8u, 1u, 2u, 0u},
    {0xf7c9u, 1u, 2u, 0u},
    {0xf7cau, 1u, 2u, 0u},
    {0xf7cbu, 1u, 2u, 0u},
    {0xf7ccu, 2u, 7u, 4u},
};

static const FixedSpanOpcode table_service_f7d0[] = {
    {0xf7d0u, 2u, 3u, 0u},
};

static const FixedSpanOpcode table_service_f7d3[] = {
    {0xf7d3u, 1u, 3u, 1u},
    {0xf7d4u, 2u, 3u, 0u},
    {0xf7d6u, 3u, 3u, 0u},
    {0xf7d9u, 2u, 4u, 2u},
    {0xf7dbu, 3u, 3u, 0u},
    {0xf7deu, 2u, 4u, 2u},
    {0xf7e0u, 2u, 2u, 0u},
    {0xf7e2u, 2u, 3u, 1u},
    {0xf7e4u, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_f7e8[] = {
    {0xf7e8u, 2u, 3u, 0u},
    {0xf7eau, 3u, 3u, 0u},
    {0xf7edu, 3u, 3u, 0u},
    {0xf7f0u, 3u, 5u, 2u},
    {0xf7f3u, 3u, 7u, 2u},
    {0xf7f6u, 1u, 2u, 0u},
    {0xf7f7u, 1u, 2u, 0u},
    {0xf7f8u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_f7f0[] = {
    {0xf7f0u, 3u, 5u, 2u},
    {0xf7f3u, 3u, 7u, 2u},
    {0xf7f6u, 1u, 2u, 0u},
    {0xf7f7u, 1u, 2u, 0u},
    {0xf7f8u, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_f7fb[] = {
    {0xf7fbu, 2u, 3u, 0u},
    {0xf7fdu, 3u, 3u, 0u},
    {0xf800u, 2u, 4u, 2u},
    {0xf802u, 3u, 3u, 0u},
    {0xf805u, 2u, 4u, 2u},
    {0xf807u, 2u, 2u, 0u},
    {0xf809u, 2u, 3u, 1u},
    {0xf80bu, 4u, 8u, 3u},
};

static const FixedSpanOpcode table_service_f80f[] = {
    {0xf80fu, 2u, 3u, 0u},
    {0xf811u, 3u, 3u, 0u},
    {0xf814u, 3u, 3u, 0u},
    {0xf817u, 3u, 5u, 2u},
    {0xf81au, 3u, 7u, 2u},
    {0xf81du, 1u, 2u, 0u},
    {0xf81eu, 1u, 2u, 0u},
    {0xf81fu, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_f817[] = {
    {0xf817u, 3u, 5u, 2u},
    {0xf81au, 3u, 7u, 2u},
    {0xf81du, 1u, 2u, 0u},
    {0xf81eu, 1u, 2u, 0u},
    {0xf81fu, 1u, 2u, 0u},
};

static const FixedSpanOpcode table_service_f822[] = {
    {0xf822u, 1u, 4u, 1u},
};

static const TableServicePhase table_service_phases[] = {
    {0xe9b1u, 0x0000u, 0x0000u, 1u, sizeof table_service_e9b1 / sizeof table_service_e9b1[0], table_service_e9b1},
    {0xe9bbu, 0x0000u, 0x0000u, 1u, sizeof table_service_e9bb / sizeof table_service_e9bb[0], table_service_e9bb},
    {0xe9beu, 0x0000u, 0x0000u, 1u, sizeof table_service_e9be / sizeof table_service_e9be[0], table_service_e9be},
    {0xe9cbu, 0xe9cfu, 0x0000u, 1u, sizeof table_service_e9cb / sizeof table_service_e9cb[0], table_service_e9cb},
    {0xe9d1u, 0xe9d4u, 0x0000u, 1u, sizeof table_service_e9d1 / sizeof table_service_e9d1[0], table_service_e9d1},
    {0xe9d6u, 0x0000u, 0xe9e2u, 1u, sizeof table_service_e9d6 / sizeof table_service_e9d6[0], table_service_e9d6},
    {0xe9dfu, 0x0000u, 0xe9e2u, 1u, sizeof table_service_e9df / sizeof table_service_e9df[0], table_service_e9df},
    {0xec4fu, 0x0000u, 0x0000u, 1u, sizeof table_service_ec4f / sizeof table_service_ec4f[0], table_service_ec4f},
    {0xec59u, 0x0000u, 0x0000u, 0u, sizeof table_service_ec59 / sizeof table_service_ec59[0], table_service_ec59},
    {0xec67u, 0x0000u, 0x0000u, 0u, sizeof table_service_ec67 / sizeof table_service_ec67[0], table_service_ec67},
    {0xec75u, 0xec8bu, 0x0000u, 0u, sizeof table_service_ec75 / sizeof table_service_ec75[0], table_service_ec75},
    {0xec85u, 0xec8bu, 0x0000u, 0u, sizeof table_service_ec85 / sizeof table_service_ec85[0], table_service_ec85},
    {0xec8du, 0xec9cu, 0x0000u, 0u, sizeof table_service_ec8d / sizeof table_service_ec8d[0], table_service_ec8d},
    {0xec93u, 0xec9cu, 0x0000u, 0u, sizeof table_service_ec93 / sizeof table_service_ec93[0], table_service_ec93},
    {0xec9eu, 0xecbau, 0x0000u, 0u, sizeof table_service_ec9e / sizeof table_service_ec9e[0], table_service_ec9e},
    {0xeca7u, 0xecbau, 0x0000u, 0u, sizeof table_service_eca7 / sizeof table_service_eca7[0], table_service_eca7},
    {0xecbcu, 0xecceu, 0x0000u, 0u, sizeof table_service_ecbc / sizeof table_service_ecbc[0], table_service_ecbc},
    {0xecc1u, 0xecceu, 0x0000u, 0u, sizeof table_service_ecc1 / sizeof table_service_ecc1[0], table_service_ecc1},
    {0xecd0u, 0xed02u, 0x0000u, 0u, sizeof table_service_ecd0 / sizeof table_service_ecd0[0], table_service_ecd0},
    {0xecdbu, 0xed02u, 0x0000u, 0u, sizeof table_service_ecdb / sizeof table_service_ecdb[0], table_service_ecdb},
    {0xece0u, 0xed02u, 0x0000u, 0u, sizeof table_service_ece0 / sizeof table_service_ece0[0], table_service_ece0},
    {0xed04u, 0xed0cu, 0x0000u, 0u, sizeof table_service_ed04 / sizeof table_service_ed04[0], table_service_ed04},
    {0xed0eu, 0xed22u, 0x0000u, 0u, sizeof table_service_ed0e / sizeof table_service_ed0e[0], table_service_ed0e},
    {0xed14u, 0xed22u, 0x0000u, 0u, sizeof table_service_ed14 / sizeof table_service_ed14[0], table_service_ed14},
    {0xed24u, 0x0000u, 0x0000u, 0u, sizeof table_service_ed24 / sizeof table_service_ed24[0], table_service_ed24},
    {0xed27u, 0xed36u, 0x0000u, 0u, sizeof table_service_ed27 / sizeof table_service_ed27[0], table_service_ed27},
    {0xed30u, 0xed36u, 0x0000u, 0u, sizeof table_service_ed30 / sizeof table_service_ed30[0], table_service_ed30},
    {0xed38u, 0xed3du, 0x0000u, 0u, sizeof table_service_ed38 / sizeof table_service_ed38[0], table_service_ed38},
    {0xed3fu, 0x0000u, 0x0000u, 0u, sizeof table_service_ed3f / sizeof table_service_ed3f[0], table_service_ed3f},
    {0xed46u, 0xed49u, 0x0000u, 0u, sizeof table_service_ed46 / sizeof table_service_ed46[0], table_service_ed46},
    {0xed4bu, 0xed4eu, 0x0000u, 0u, sizeof table_service_ed4b / sizeof table_service_ed4b[0], table_service_ed4b},
    {0xed50u, 0x0000u, 0xed9du, 0u, sizeof table_service_ed50 / sizeof table_service_ed50[0], table_service_ed50},
    {0xed5eu, 0x0000u, 0xed9du, 0u, sizeof table_service_ed5e / sizeof table_service_ed5e[0], table_service_ed5e},
    {0xed6cu, 0x0000u, 0xed9du, 0u, sizeof table_service_ed6c / sizeof table_service_ed6c[0], table_service_ed6c},
    {0xedfau, 0x0000u, 0x0000u, 1u, sizeof table_service_edfa / sizeof table_service_edfa[0], table_service_edfa},
    {0xee0fu, 0xee20u, 0x0000u, 1u, sizeof table_service_ee0f / sizeof table_service_ee0f[0], table_service_ee0f},
    {0xee17u, 0xee20u, 0x0000u, 0u, sizeof table_service_ee17 / sizeof table_service_ee17[0], table_service_ee17},
    {0xee22u, 0x0000u, 0x0000u, 0u, sizeof table_service_ee22 / sizeof table_service_ee22[0], table_service_ee22},
    {0xee36u, 0xee47u, 0x0000u, 1u, sizeof table_service_ee36 / sizeof table_service_ee36[0], table_service_ee36},
    {0xee3eu, 0xee47u, 0x0000u, 0u, sizeof table_service_ee3e / sizeof table_service_ee3e[0], table_service_ee3e},
    {0xee49u, 0x0000u, 0xee4au, 0u, sizeof table_service_ee49 / sizeof table_service_ee49[0], table_service_ee49},
    {0xf12fu, 0xf13du, 0x0000u, 1u, sizeof table_service_f12f / sizeof table_service_f12f[0], table_service_f12f},
    {0xf13fu, 0x0000u, 0xf172u, 1u, sizeof table_service_f13f / sizeof table_service_f13f[0], table_service_f13f},
    {0xf161u, 0xf163u, 0x0000u, 1u, sizeof table_service_f161 / sizeof table_service_f161[0], table_service_f161},
    {0xf165u, 0x0000u, 0xf172u, 1u, sizeof table_service_f165 / sizeof table_service_f165[0], table_service_f165},
    {0xf16eu, 0x0000u, 0xf172u, 1u, sizeof table_service_f16e / sizeof table_service_f16e[0], table_service_f16e},
    {0xf173u, 0xf18bu, 0x0000u, 1u, sizeof table_service_f173 / sizeof table_service_f173[0], table_service_f173},
    {0xf18du, 0xf18bu, 0x0000u, 0u, sizeof table_service_f18d / sizeof table_service_f18d[0], table_service_f18d},
    {0xf197u, 0x0000u, 0xf19cu, 0u, sizeof table_service_f197 / sizeof table_service_f197[0], table_service_f197},
    {0xf78fu, 0xf79au, 0x0000u, 1u, sizeof table_service_f78f / sizeof table_service_f78f[0], table_service_f78f},
    {0xf79cu, 0xf7b7u, 0x0000u, 0u, sizeof table_service_f79c / sizeof table_service_f79c[0], table_service_f79c},
    {0xf79fu, 0xf7b7u, 0x0000u, 0u, sizeof table_service_f79f / sizeof table_service_f79f[0], table_service_f79f},
    {0xf7a4u, 0xf7b7u, 0x0000u, 0u, sizeof table_service_f7a4 / sizeof table_service_f7a4[0], table_service_f7a4},
    {0xf7b9u, 0xf7ceu, 0x0000u, 0u, sizeof table_service_f7b9 / sizeof table_service_f7b9[0], table_service_f7b9},
    {0xf7c1u, 0xf7ceu, 0x0000u, 0u, sizeof table_service_f7c1 / sizeof table_service_f7c1[0], table_service_f7c1},
    {0xf7d0u, 0x0000u, 0xf7d2u, 0u, sizeof table_service_f7d0 / sizeof table_service_f7d0[0], table_service_f7d0},
    {0xf7d3u, 0x0000u, 0x0000u, 1u, sizeof table_service_f7d3 / sizeof table_service_f7d3[0], table_service_f7d3},
    {0xf7e8u, 0xf7f9u, 0x0000u, 1u, sizeof table_service_f7e8 / sizeof table_service_f7e8[0], table_service_f7e8},
    {0xf7f0u, 0xf7f9u, 0x0000u, 0u, sizeof table_service_f7f0 / sizeof table_service_f7f0[0], table_service_f7f0},
    {0xf7fbu, 0x0000u, 0x0000u, 0u, sizeof table_service_f7fb / sizeof table_service_f7fb[0], table_service_f7fb},
    {0xf80fu, 0xf820u, 0x0000u, 1u, sizeof table_service_f80f / sizeof table_service_f80f[0], table_service_f80f},
    {0xf817u, 0xf820u, 0x0000u, 0u, sizeof table_service_f817 / sizeof table_service_f817[0], table_service_f817},
    {0xf822u, 0x0000u, 0xf823u, 0u, sizeof table_service_f822 / sizeof table_service_f822[0], table_service_f822},
};

static unsigned TableServiceReadPenalty(uint16_t pc, unsigned x, unsigned y) {
    (void)y;
    switch (pc) {
    case 0xee17u: return (0xa000u >> 8) != (((0xa000u + x) & 0xffffu) >> 8);
    case 0xee3eu: return (0x4000u >> 8) != (((0x4000u + x) & 0xffffu) >> 8);
    case 0xf7f0u: return (0xa000u >> 8) != (((0xa000u + x) & 0xffffu) >> 8);
    case 0xf817u: return (0x4000u >> 8) != (((0x4000u + x) & 0xffffu) >> 8);
    default: return 0u;
    }
}

typedef struct TableServiceProjection { uint16_t x, y, words, pairs; } TableServiceProjection;

static void ProjectTableServiceIndex(TableServiceProjection *state, uint16_t pc) {
    switch (pc) {
    case 0xec7cu: state->x = 0x000bu; break;
    case 0xec7fu: state->y = 0x0000u; break;
    case 0xec88u: ++state->y; break;
    case 0xec89u: ++state->y; break;
    case 0xec8au: --state->x; break;
    case 0xec8du: state->x = 0x0000u; break;
    case 0xec90u: state->y = 0x0003u; break;
    case 0xec99u: ++state->x; break;
    case 0xec9au: ++state->x; break;
    case 0xec9bu: --state->y; break;
    case 0xec9eu: state->x = 0x0000u; break;
    case 0xeca1u: state->y = state->x; break;
    case 0xecb5u: ++state->x; break;
    case 0xecb6u: ++state->y; break;
    case 0xecb7u: ++state->y; break;
    case 0xecc8u: ++state->x; break;
    case 0xecc9u: ++state->x; break;
    case 0xeccau: ++state->y; break;
    case 0xeccbu: ++state->y; break;
    case 0xecd0u: state->x = 0x0120u; break;
    case 0xecd3u: state->y = 0x4fd8u; break;
    case 0xece4u: ++state->x; break;
    case 0xece5u: ++state->x; break;
    case 0xececu: ++state->x; break;
    case 0xecedu: ++state->x; break;
    case 0xecf6u: ++state->x; break;
    case 0xecf7u: ++state->x; break;
    case 0xecfeu: ++state->x; break;
    case 0xecffu: ++state->x; break;
    case 0xed0eu: state->y = 0x0003u; break;
    case 0xed11u: state->x = 0x0000u; break;
    case 0xed1fu: ++state->x; break;
    case 0xed20u: ++state->x; break;
    case 0xed21u: --state->y; break;
    case 0xed2au: state->x = 0x0000u; break;
    case 0xed2du: state->y = 0x0005u; break;
    case 0xed33u: ++state->x; break;
    case 0xed34u: ++state->x; break;
    case 0xed35u: --state->y; break;
    case 0xedfdu: state->x = 0x02a1u; break;
    case 0xee02u: state->x = 0xa000u; break;
    case 0xee11u: state->x = 0x0000u; break;
    case 0xee14u: state->y = 0x0400u; break;
    case 0xee1du: ++state->x; break;
    case 0xee1eu: ++state->x; break;
    case 0xee1fu: --state->y; break;
    case 0xee24u: state->x = 0x02a2u; break;
    case 0xee29u: state->x = 0x4000u; break;
    case 0xee38u: state->x = 0x0000u; break;
    case 0xee3bu: state->y = 0x0400u; break;
    case 0xee44u: ++state->x; break;
    case 0xee45u: ++state->x; break;
    case 0xee46u: --state->y; break;
    case 0xf14au: state->x = 0x4000u; break;
    case 0xf17cu: state->x = 0xf19du; break;
    case 0xf183u: state->x = 0x0000u; break;
    case 0xf186u: state->y = 0x0000u; break;
    case 0xf191u: ++state->x; break;
    case 0xf192u: ++state->x; break;
    case 0xf193u: ++state->y; break;
    case 0xf194u: ++state->y; break;
    case 0xf791u: state->x = 0x0000u; break;
    case 0xf794u: state->y = state->x; break;
    case 0xf79cu: state->x = 0x0057u; break;
    case 0xf7b2u: ++state->x; break;
    case 0xf7b3u: ++state->y; break;
    case 0xf7b4u: ++state->y; break;
    case 0xf7b9u: state->x = 0x0000u; break;
    case 0xf7c8u: ++state->x; break;
    case 0xf7c9u: ++state->x; break;
    case 0xf7cau: ++state->y; break;
    case 0xf7cbu: ++state->y; break;
    case 0xf7d6u: state->x = 0x02a7u; break;
    case 0xf7dbu: state->x = 0xa000u; break;
    case 0xf7eau: state->x = 0x0000u; break;
    case 0xf7edu: state->y = 0x0400u; break;
    case 0xf7f6u: ++state->x; break;
    case 0xf7f7u: ++state->x; break;
    case 0xf7f8u: --state->y; break;
    case 0xf7fdu: state->x = 0x02a6u; break;
    case 0xf802u: state->x = 0x4000u; break;
    case 0xf811u: state->x = 0x0000u; break;
    case 0xf814u: state->y = 0x0400u; break;
    case 0xf81du: ++state->x; break;
    case 0xf81eu: ++state->x; break;
    case 0xf81fu: --state->y; break;
    case 0xed09u: state->y = (uint16_t)(state->y + 0x2000u); break;
    case 0xeca5u: state->words = 87u; break;
    case 0xecbfu: state->words = 56u; break;
    case 0xecd9u: state->words = 3u; break;
    case 0xf7a2u: state->words = 87u; break;
    case 0xf7bfu: state->words = 40u; break;
    case 0xecdeu: state->pairs = 2u; break;
    case 0xecb8u: --state->words; break;
    case 0xecccu: --state->words; break;
    case 0xed00u: --state->pairs; break;
    case 0xed0au: --state->words; break;
    case 0xf7b5u: --state->words; break;
    case 0xf7ccu: --state->words; break;
    default: break;
    }
}

enum { TABLE_SERVICE_CLOCK_CAPACITY = 9000u };

typedef struct TableServiceClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_SERVICE_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    unsigned root;
    uint8_t input_m, input_x, restore_x;
    bool returning;
} TableServiceClock;

static const uint32_t table_service_entries[] = {
    0x84f12fu, 0x84f173u, 0x84f78fu, 0x84ec4fu,
    0x84edfau, 0x84f7d3u, 0x84e9b1u
};

static uint16_t TableServiceWord(const CpuState *cpu, unsigned offset) {
    return cpu->ram[offset] | (uint16_t)cpu->ram[offset + 1u] << 8;
}

static const TableServicePhase *FindTableServicePhase(uint16_t pc) {
    unsigned first = 0u;
    unsigned last = sizeof table_service_phases / sizeof table_service_phases[0];
    while (first < last) {
        const unsigned middle = first + (last - first) / 2u;
        const TableServicePhase *phase = table_service_phases + middle;
        if (phase->pc == pc) return phase;
        if (phase->pc < pc) first = middle + 1u;
        else last = middle;
    }
    return NULL;
}

static bool TableServiceOpcode(TableServiceClock *clock, FixedSpanOpcode op,
                               const TableServiceProjection *projection) {
    if (clock->span.count == TABLE_SERVICE_CLOCK_CAPACITY) return false;
    switch (op.pc) {
    case 0xf12fu:
        op.cycles = (uint8_t)(4u - clock->input_m);
        op.data_bytes = (uint8_t)(2u - clock->input_m); break;
    case 0xf130u:
    case 0xf131u:
    case 0xe9b1u:
    case 0xe9b2u:
        op.cycles = (uint8_t)(4u - clock->input_x);
        op.data_bytes = (uint8_t)(2u - clock->input_x); break;
    case 0xf16fu:
    case 0xf170u:
    case 0xe9e0u:
    case 0xe9e1u:
        op.cycles = (uint8_t)(5u - clock->restore_x);
        op.data_bytes = (uint8_t)(2u - clock->restore_x); break;
    case 0xf171u:
        op.cycles = (uint8_t)(5u - clock->input_m);
        op.data_bytes = (uint8_t)(2u - clock->input_m); break;
    default: break;
    }
    op.cycles = (uint8_t)(op.cycles + TableServiceReadPenalty(op.pc, projection->x, projection->y));
    clock->timing[clock->span.count++] = op;
    return true;
}

static bool AppendTableServicePhase(TableServiceClock *clock, uint16_t pc,
                                    TableServiceProjection projection) {
    const CpuState *cpu = clock->call.cpu;
    for (unsigned pass = 0u; pass < TABLE_SERVICE_CLOCK_CAPACITY; ++pass) {
        const TableServicePhase *phase = FindTableServicePhase(pc);
        if (!phase) return false;
        for (unsigned i = 0u; i < phase->count; ++i) {
            const FixedSpanOpcode op = phase->opcodes[i];
            if (!TableServiceOpcode(clock, op, &projection)) return false;
            ProjectTableServiceIndex(&projection, op.pc);
        }
        if (phase->return_pc) {
            clock->next = 0x840000u | phase->return_pc;
            clock->returning = true;
        }
        if (!phase->branch) return true;
        bool taken;
        uint16_t target;
        switch (phase->branch) {
        case 0xf13du: taken = cpu->ram[0x6000u] != 1u; target = 0xf161u; break;
        case 0xf163u: taken = cpu->ram[0x6000u] != 2u; target = 0xf16eu; break;
        case 0xf18bu: {
            const uint32_t address = 0x84f19du + projection.y;
            const unsigned offset = ((address >> 16) & 127u) * 32768u + (address & 32767u);
            if (offset + 1u >= g_snes->cart->romSize) return false;
            const uint8_t *word = g_snes->cart->rom + offset;
            taken = !(word[0] | word[1]); target = 0xf197u; break;
        }
        case 0xf79au: taken = (cpu->ram[0x30u] & 1u) != 0u; target = 0xf79fu; break;
        case 0xf7b7u: taken = projection.words != 0u; target = 0xf7a4u; break;
        case 0xf7ceu: taken = projection.words != 0u; target = 0xf7c1u; break;
        case 0xec8bu: taken = projection.x != 0u; target = 0xec85u; break;
        case 0xec9cu: taken = projection.y != 0u; target = 0xec93u; break;
        case 0xecbau: taken = projection.words != 0u; target = 0xeca7u; break;
        case 0xecceu: taken = projection.words != 0u; target = 0xecc1u; break;
        case 0xed02u: taken = projection.pairs != 0u; target = 0xece0u; break;
        case 0xed0cu: taken = projection.words != 0u; target = 0xecdbu; break;
        case 0xed22u: taken = projection.y != 0u; target = 0xed14u; break;
        case 0xed36u: taken = projection.y != 0u; target = 0xed30u; break;
        case 0xed3du: taken = !cpu->ram[0x30u]; target = 0xed6cu; break;
        case 0xed49u: taken = !((uint16_t)(cpu->A - 2u) & 0x8000u); target = 0xed6cu; break;
        case 0xed4eu: taken = cpu->A == 1u; target = 0xed5eu; break;
        case 0xee20u: taken = projection.y != 0u; target = 0xee17u; break;
        case 0xee47u: taken = projection.y != 0u; target = 0xee3eu; break;
        case 0xf7f9u: taken = projection.y != 0u; target = 0xf7f0u; break;
        case 0xf820u: taken = projection.y != 0u; target = 0xf817u; break;
        case 0xe9cfu: taken = (cpu->ram[0x4au] & 0x10u) != 0u; target = 0xe9dfu; break;
        case 0xe9d4u: {
            const unsigned offset = (cpu->DB == 0x7fu ? 0x10000u : 0u) + 0x71beu;
            taken = !((uint8_t)(cpu->ram[offset] - 1u) & 0x80u); target = 0xe9dfu; break;
        }
        default: return false;
        }
        const FixedSpanOpcode branch = {phase->branch, 2u, (uint8_t)(taken ? 3u : 2u), 0u};
        if (!TableServiceOpcode(clock, branch, &projection)) return false;
        pc = taken ? target : (uint16_t)(phase->branch + 2u);
    }
    return false;
}

static uint64_t TableServiceBudget(const TableServiceClock *clock) {
    uint64_t budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    return budget + 40u * (budget / 1324u + 1u);
}

static bool TableServiceWindowSupported(const TableServiceClock *clock, bool nmi) {
    CpuState *cpu = clock->call.cpu;
    const uint64_t budget = TableServiceBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->dma ||
        g_snes->ram != cpu->ram || g_snes->beamMasterLast != cpu->master_cycles ||
        g_snes->hPos >= 1364u || g_snes->vPos >= 262u) return false;
    if (nmi) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_RESTORE_WIDTH)
        if (!Lufia2DecompNmiHasInterruptOwner() || !Lufia2DecompFrameReturnCpuReady() ||
            clock->root != 0u || !g_snes->inNmi || g_snes->inIrq ||
            g_snes->hIrqEnabled || g_snes->vIrqEnabled || g_snes->vPos < 225u ||
            cpu->DB != 0x80u || cpu->D || cpu->m_flag != 1u || cpu->x_flag ||
            cpu->_flag_D || cpu->ram[(uint16_t)(cpu->S + 1u)] != 0x6au ||
            cpu->ram[(uint16_t)(cpu->S + 2u)] != 0u ||
            cpu->ram[(uint16_t)(cpu->S + 3u)] != 0x80u ||
            cpu->ram[(uint16_t)(cpu->S + 4u)] != 0x52u ||
            cpu->ram[(uint16_t)(cpu->S + 5u)] != 0x86u ||
            cpu->ram[0x67u] != 0x22u || cpu->ram[0x68u] != 0x2fu ||
            cpu->ram[0x69u] != 0xf1u || cpu->ram[0x6au] != 0x84u || cpu->ram[0x6bu] != 0x60u ||
            g_snes->dma->dmaBusy || g_snes->dma->dmaTimer) return false;
        for (unsigned channel = 0u; channel < 8u; ++channel)
            if (g_snes->dma->channel[channel].dmaActive) return false;
#else
        return false;
#endif
    } else if (!ActorBridgeTimedStepSupportedWindow(cpu, table_service_entries[clock->root],
                    0x1ffcu, true, 1363u, budget) || !g_snes->hdmaBeamOff) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareTableServiceClock(TableServiceClock *clock, uint32_t pc, bool nmi) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = pc;
    clock->returning = false;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x28000u ||
        pc >> 16 != 0x84u || cpu->PB != 0x84u || cpu->emulation ||
        cpu->m_flag > 1u || cpu->x_flag > 1u || cpu->host_return_valid > 3u ||
        cpu->host_return_valid == 1u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (clock->root && cpu->D) || (clock->root == 3u && cpu->_flag_D)) return false;
    const TableServicePhase *phase = FindTableServicePhase((uint16_t)pc);
    if (!phase) return false;
    const bool entry = pc == table_service_entries[clock->root];
    if (entry) {
        if (clock->root >= 2u && clock->root != 6u && cpu->x_flag) return false;
        clock->input_m = cpu->m_flag;
        clock->input_x = cpu->x_flag;
        clock->restore_x = cpu->x_flag;
    } else {
        if (cpu->x_flag) return false;
        const bool normalizes = pc == 0x84ee0fu || pc == 0x84ee36u ||
            pc == 0x84f7e8u || pc == 0x84f80fu;
        if (!normalizes && cpu->m_flag != phase->byte_mode) return false;
        if (clock->root == 6u)
            clock->restore_x = (cpu->ram[(uint16_t)(cpu->S + 1u)] >> 4u) & 1u;
    }
    if (clock->root ? (cpu->DB != 0x7eu && cpu->DB != 0x7fu) :
        (cpu->DB != 0u && cpu->DB != 0x80u && cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const TableServiceProjection projection = {cpu->X, cpu->Y,
        TableServiceWord(cpu, 0u), TableServiceWord(cpu, 2u)};
    return AppendTableServicePhase(clock, (uint16_t)pc, projection) && TableServiceWindowSupported(clock, nmi);
}

static uint8_t TableServiceChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableServiceClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableServiceClock(clock, landing, false)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static Lufia2ExecutionResult RunTableServiceBody(TableServiceClock *clock) {
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock->span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(clock->call.cpu, &state);
    Lufia2ExecutionResult result;
    switch (clock->root) {
    case 0u: result = Lufia2CardTableUpdateHdma(&memory, &state); break;
    case 1u: result = Lufia2CardTableBuildHdma(&memory, &state); break;
    case 2u: result = Lufia2CardTableLoadAlternateTiles(&memory, &state); break;
    case 3u: result = Lufia2CardTableInitialize(&memory, &state, TableServiceChild, clock); break;
    case 4u: result = Lufia2CardTableLoadGraphics(&memory, &state, TableServiceChild, clock); break;
    case 5u: result = Lufia2CardTableLoadAlternateGraphics(&memory, &state, TableServiceChild, clock); break;
    default: result = Lufia2CardTableWaitFrame(&memory, &state, TableServiceChild, clock); break;
    }
    if (result.flow != LUFIA2_EXECUTION_CHILD_UNWOUND) ActorBridgeStore(clock->call.cpu, &state);
    return result;
}

static RecompReturn ReturnTableServiceLong(CpuState *cpu, const ActorBridgeFrame *frame, uint32_t source) {
    const uint16_t stack = cpu->S;
    bool progress = false;
    (void)Lufia2DecompFrameReturnReadByte(cpu, source, &progress);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const unsigned low = Lufia2DecompFrameReturnReadByte(cpu, cpu->S, &progress);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const unsigned high = Lufia2DecompFrameReturnReadByte(cpu, cpu->S, &progress);
    cpu->S = (uint16_t)(cpu->S + 1u);
    const uint8_t bank = Lufia2DecompFrameReturnReadByte(cpu, cpu->S, &progress);
    Lufia2DecompFrameReturnAccountOpcode(cpu, 6u, cpu_region_speed(source) + 24u + 12u, progress);
    const uint32_t target = (uint32_t)bank << 16 | (uint16_t)((high << 8 | low) + 1u);
    cpu->PB = bank;
    return ActorBridgeResolveReturn(cpu, frame, 3u, source, stack, target);
}

static RecompReturn ActorBridgeTableService(CpuState *cpu, unsigned root) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableServiceClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    const uint32_t entry = table_service_entries[root];
    if (frame.hrv != (root ? 2u : 3u) || !PrepareTableServiceClock(&clock, entry, false))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2ExecutionResult result = RunTableServiceBody(&clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc && clock.span.index == clock.span.count && !clock.span.remaining);
    return root ? ReturnTimedMenuCall(cpu, &frame, result.pc) : ReturnTableServiceLong(cpu, &frame, result.pc);
}

static void ActorBridgeCardTableHdmaHook(CpuState *cpu, uint32_t pc) {
    TableServiceClock clock;
    clock.root = 0u;
    clock.call = (ActorPushedCall){cpu, {0}, RECOMP_RETURN_NORMAL};
    if (pc != 0x84f12fu || !PrepareTableServiceClock(&clock, pc, true)) return;
    const Lufia2ExecutionResult result = RunTableServiceBody(&clock);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x84f172u &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    Lufia2DecompFrameReturnComplete(clock.span.count, clock.span.progress_mask);
    interp_bridge_pre_opcode_redirect(result.pc);
}
