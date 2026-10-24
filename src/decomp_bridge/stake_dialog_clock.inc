typedef struct StakeDialogPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} StakeDialogPhase;

static const FixedSpanOpcode stake_dialog_ac66[] = {
    {0xac66u, 3u, 3u, 0u},
    {0xac69u, 3u, 5u, 2u},
    {0xac6cu, 3u, 5u, 2u},
    {0xac6fu, 3u, 5u, 2u},
    {0xac72u, 2u, 4u, 2u},
    {0xac74u, 2u, 4u, 2u},
    {0xac76u, 3u, 3u, 0u},
    {0xac79u, 3u, 3u, 0u},
    {0xac7cu, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_ac80[] = {
    {0xac80u, 3u, 3u, 0u},
    {0xac83u, 3u, 5u, 2u},
    {0xac86u, 3u, 3u, 0u},
    {0xac89u, 3u, 5u, 2u},
    {0xac8cu, 3u, 3u, 0u},
    {0xac8fu, 3u, 5u, 2u},
    {0xac92u, 3u, 3u, 0u},
    {0xac95u, 3u, 5u, 2u},
    {0xac98u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ac9b[] = {
    {0xac9bu, 2u, 3u, 0u},
    {0xac9du, 2u, 2u, 0u},
    {0xac9fu, 2u, 3u, 1u},
    {0xaca1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_aca4[] = {
    {0xaca4u, 2u, 3u, 0u},
    {0xaca6u, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_acaa[] = {
    {0xacaau, 2u, 7u, 4u},
    {0xacacu, 2u, 3u, 0u},
    {0xacaeu, 2u, 2u, 0u},
    {0xacb0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_acac[] = {
    {0xacacu, 2u, 3u, 0u},
    {0xacaeu, 2u, 2u, 0u},
    {0xacb0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_acb3[] = {
    {0xacb3u, 2u, 3u, 0u},
    {0xacb5u, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_acba[] = {
    {0xacbau, 3u, 5u, 2u},
    {0xacbdu, 1u, 2u, 0u},
    {0xacbeu, 3u, 5u, 2u},
    {0xacc1u, 1u, 2u, 0u},
    {0xacc2u, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_acc6[] = {
    {0xacc6u, 3u, 5u, 2u},
    {0xacc9u, 1u, 2u, 0u},
    {0xaccau, 3u, 5u, 2u},
    {0xaccdu, 3u, 5u, 2u},
    {0xacd0u, 2u, 4u, 2u},
    {0xacd2u, 3u, 5u, 2u},
    {0xacd5u, 2u, 4u, 2u},
    {0xacd7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_acd5[] = {
    {0xacd5u, 2u, 4u, 2u},
    {0xacd7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_acda[] = {
    {0xacdau, 2u, 4u, 2u},
    {0xacdcu, 1u, 2u, 0u},
    {0xacddu, 2u, 4u, 2u},
    {0xacdfu, 2u, 4u, 2u},
    {0xace1u, 2u, 4u, 2u},
    {0xace3u, 2u, 4u, 2u},
    {0xace5u, 3u, 3u, 0u},
    {0xace8u, 3u, 3u, 0u},
    {0xacebu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_acee[] = {
    {0xaceeu, 2u, 4u, 2u},
    {0xacf0u, 2u, 4u, 2u},
    {0xacf2u, 3u, 3u, 0u},
    {0xacf5u, 3u, 3u, 0u},
    {0xacf8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_acfb[] = {
    {0xacfbu, 3u, 3u, 0u},
    {0xacfeu, 3u, 3u, 0u},
    {0xad01u, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_ad05[] = {
    {0xad05u, 3u, 5u, 2u},
    {0xad08u, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad0c[] = {
    {0xad0cu, 1u, 3u, 1u},
    {0xad0du, 2u, 3u, 0u},
    {0xad0fu, 2u, 2u, 0u},
    {0xad11u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad14[] = {
    {0xad14u, 1u, 4u, 1u},
    {0xad15u, 3u, 5u, 2u},
    {0xad18u, 1u, 2u, 0u},
    {0xad19u, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad1d[] = {
    {0xad1du, 1u, 2u, 0u},
    {0xad1eu, 3u, 5u, 2u},
    {0xad21u, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad26[] = {
    {0xad26u, 3u, 8u, 4u},
    {0xad29u, 2u, 3u, 0u},
    {0xad41u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad2b[] = {
    {0xad2bu, 2u, 4u, 2u},
    {0xad2du, 1u, 2u, 0u},
    {0xad2eu, 3u, 5u, 2u},
    {0xad31u, 2u, 4u, 2u},
    {0xad33u, 3u, 5u, 2u},
    {0xad36u, 1u, 2u, 0u},
    {0xad37u, 2u, 4u, 2u},
    {0xad39u, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad3e[] = {
    {0xad3eu, 3u, 8u, 4u},
    {0xad41u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad41[] = {
    {0xad41u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad44[] = {
    {0xad44u, 2u, 3u, 0u},
    {0xad46u, 2u, 2u, 0u},
    {0xad48u, 2u, 3u, 1u},
    {0xad4au, 2u, 2u, 0u},
    {0xad4cu, 2u, 3u, 1u},
    {0xad4eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad51[] = {
    {0xad51u, 2u, 3u, 0u},
    {0xad53u, 2u, 4u, 2u},
    {0xad55u, 3u, 5u, 2u},
    {0xad58u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad5b[] = {
    {0xad5bu, 3u, 3u, 0u},
};

static const FixedSpanOpcode stake_dialog_ad60[] = {
    {0xad60u, 3u, 4u, 0u},
    {0xaca4u, 2u, 3u, 0u},
    {0xaca6u, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad63[] = {
    {0xad63u, 2u, 4u, 2u},
    {0xad65u, 3u, 5u, 2u},
    {0xad68u, 3u, 5u, 2u},
    {0xad6bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad6e[] = {
    {0xad6eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad71[] = {
    {0xad71u, 3u, 3u, 0u},
    {0xad74u, 3u, 3u, 0u},
    {0xad77u, 3u, 3u, 0u},
    {0xad7au, 3u, 7u, 2u},
    {0xad7du, 1u, 2u, 0u},
    {0xad7eu, 1u, 2u, 0u},
    {0xad7fu, 3u, 7u, 2u},
    {0xad82u, 1u, 2u, 0u},
    {0xad83u, 1u, 2u, 0u},
    {0xad84u, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_ad7a[] = {
    {0xad7au, 3u, 7u, 2u},
    {0xad7du, 1u, 2u, 0u},
    {0xad7eu, 1u, 2u, 0u},
    {0xad7fu, 3u, 7u, 2u},
    {0xad82u, 1u, 2u, 0u},
    {0xad83u, 1u, 2u, 0u},
    {0xad84u, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_ad87[] = {
    {0xad87u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad8a[] = {
    {0xad8au, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad8d[] = {
    {0xad8du, 2u, 3u, 0u},
    {0xad8fu, 2u, 2u, 0u},
    {0xad91u, 2u, 3u, 1u},
    {0xad93u, 2u, 2u, 0u},
    {0xad95u, 2u, 3u, 1u},
    {0xad97u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_ad9a[] = {
    {0xad9au, 2u, 3u, 0u},
};

static const FixedSpanOpcode stake_dialog_d1ef[] = {
    {0xd1efu, 1u, 3u, 1u},
    {0xd1f0u, 2u, 3u, 0u},
    {0xd1f2u, 3u, 5u, 2u},
    {0xd1f5u, 3u, 5u, 2u},
    {0xd1f8u, 2u, 4u, 2u},
    {0xd1fau, 2u, 4u, 2u},
    {0xd1fcu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d1ff[] = {
    {0xd1ffu, 3u, 3u, 0u},
    {0xd202u, 3u, 5u, 2u},
    {0xd205u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d208[] = {
    {0xd208u, 3u, 3u, 0u},
    {0xd20bu, 3u, 3u, 0u},
    {0xd20eu, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d212[] = {
    {0xd212u, 2u, 3u, 0u},
    {0xd214u, 2u, 2u, 0u},
    {0xd216u, 2u, 3u, 1u},
    {0xd218u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d21b[] = {
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d222[] = {
    {0xd222u, 3u, 5u, 2u},
    {0xd225u, 1u, 2u, 0u},
    {0xd226u, 4u, 6u, 2u},
    {0xd22au, 1u, 2u, 0u},
    {0xd22bu, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_d22f[] = {
    {0xd22fu, 3u, 5u, 2u},
    {0xd232u, 1u, 2u, 0u},
    {0xd233u, 4u, 6u, 2u},
    {0xd237u, 3u, 5u, 2u},
    {0xd23au, 2u, 4u, 2u},
    {0xd23cu, 3u, 5u, 2u},
    {0xd23fu, 3u, 3u, 0u},
    {0xd242u, 3u, 3u, 0u},
    {0xd245u, 2u, 4u, 2u},
    {0xd247u, 2u, 4u, 2u},
    {0xd249u, 1u, 2u, 0u},
    {0xd24au, 1u, 2u, 0u},
    {0xd24bu, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_d23f[] = {
    {0xd23fu, 3u, 3u, 0u},
    {0xd242u, 3u, 3u, 0u},
    {0xd245u, 2u, 4u, 2u},
    {0xd247u, 2u, 4u, 2u},
    {0xd249u, 1u, 2u, 0u},
    {0xd24au, 1u, 2u, 0u},
    {0xd24bu, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_d249[] = {
    {0xd249u, 1u, 2u, 0u},
    {0xd24au, 1u, 2u, 0u},
    {0xd24bu, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_d24f[] = {
    {0xd24fu, 2u, 4u, 2u},
    {0xd251u, 1u, 2u, 0u},
    {0xd252u, 1u, 2u, 0u},
    {0xd253u, 1u, 2u, 0u},
    {0xd254u, 2u, 4u, 2u},
    {0xd256u, 1u, 2u, 0u},
    {0xd257u, 1u, 2u, 0u},
    {0xd258u, 1u, 2u, 0u},
    {0xd259u, 2u, 4u, 2u},
    {0xd25bu, 2u, 4u, 2u},
    {0xd25du, 2u, 4u, 2u},
    {0xd25fu, 1u, 2u, 0u},
    {0xd260u, 2u, 4u, 2u},
    {0xd262u, 1u, 3u, 0u},
    {0xd263u, 2u, 4u, 2u},
    {0xd265u, 1u, 3u, 0u},
    {0xd266u, 2u, 4u, 2u},
    {0xd268u, 3u, 3u, 0u},
    {0xd26bu, 2u, 4u, 2u},
    {0xd26du, 2u, 4u, 2u},
    {0xd26fu, 3u, 3u, 0u},
    {0xd272u, 1u, 3u, 0u},
    {0xd273u, 1u, 2u, 0u},
    {0xd274u, 1u, 2u, 0u},
    {0xd275u, 1u, 2u, 0u},
    {0xd276u, 2u, 4u, 2u},
    {0xd278u, 1u, 2u, 0u},
    {0xd279u, 2u, 4u, 2u},
    {0xd27bu, 1u, 2u, 0u},
    {0xd27cu, 3u, 3u, 0u},
    {0xd27fu, 3u, 3u, 0u},
    {0xd282u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d285[] = {
    {0xd285u, 2u, 4u, 2u},
    {0xd287u, 3u, 3u, 0u},
    {0xd28au, 1u, 2u, 0u},
    {0xd28bu, 1u, 2u, 0u},
    {0xd28cu, 1u, 2u, 0u},
    {0xd28du, 2u, 4u, 2u},
    {0xd28fu, 1u, 2u, 0u},
    {0xd290u, 2u, 4u, 2u},
    {0xd292u, 1u, 2u, 0u},
    {0xd293u, 3u, 3u, 0u},
    {0xd296u, 3u, 3u, 0u},
    {0xd299u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d29c[] = {
    {0xd29cu, 3u, 3u, 0u},
    {0xd29fu, 3u, 3u, 0u},
    {0xd2a2u, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d2a6[] = {
    {0xd2a6u, 3u, 5u, 2u},
    {0xd2a9u, 1u, 2u, 0u},
    {0xd2aau, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2ae[] = {
    {0xd2aeu, 2u, 3u, 0u},
    {0xd2b0u, 2u, 2u, 0u},
    {0xd2b2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2b5[] = {
    {0xd2b5u, 2u, 3u, 0u},
    {0xd2b7u, 3u, 5u, 2u},
    {0xd2bau, 1u, 2u, 0u},
    {0xd2bbu, 2u, 4u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2bf[] = {
    {0xd2bfu, 1u, 2u, 0u},
    {0xd2c0u, 3u, 5u, 2u},
    {0xd2c3u, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2c8[] = {
    {0xd2c8u, 3u, 8u, 4u},
    {0xd2cbu, 2u, 3u, 0u},
    {0xd2e4u, 3u, 3u, 0u},
    {0xd2e7u, 3u, 3u, 0u},
    {0xd2eau, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d2cd[] = {
    {0xd2cdu, 2u, 4u, 2u},
    {0xd2cfu, 1u, 2u, 0u},
    {0xd2d0u, 4u, 6u, 2u},
    {0xd2d4u, 2u, 4u, 2u},
    {0xd2d6u, 3u, 5u, 2u},
    {0xd2d9u, 1u, 2u, 0u},
    {0xd2dau, 2u, 4u, 2u},
    {0xd2dcu, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2e1[] = {
    {0xd2e1u, 3u, 8u, 4u},
    {0xd2e4u, 3u, 3u, 0u},
    {0xd2e7u, 3u, 3u, 0u},
    {0xd2eau, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d2e4[] = {
    {0xd2e4u, 3u, 3u, 0u},
    {0xd2e7u, 3u, 3u, 0u},
    {0xd2eau, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d2ee[] = {
    {0xd2eeu, 2u, 3u, 0u},
    {0xd2f0u, 2u, 2u, 0u},
    {0xd2f2u, 2u, 3u, 1u},
    {0xd2f4u, 2u, 3u, 0u},
    {0xd2f6u, 2u, 4u, 2u},
    {0xd2f8u, 3u, 5u, 2u},
    {0xd2fbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2fb[] = {
    {0xd2fbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d2fe[] = {
    {0xd2feu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d301[] = {
    {0xd301u, 3u, 3u, 0u},
};

static const FixedSpanOpcode stake_dialog_d306[] = {
    {0xd306u, 1u, 2u, 0u},
    {0xd307u, 2u, 3u, 0u},
    {0xd309u, 2u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d30d[] = {
    {0xd30du, 2u, 2u, 0u},
    {0xd30fu, 2u, 3u, 0u},
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d311[] = {
    {0xd311u, 2u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d315[] = {
    {0xd315u, 2u, 2u, 0u},
    {0xd317u, 2u, 3u, 0u},
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d319[] = {
    {0xd319u, 1u, 3u, 0u},
    {0xd31au, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d31d[] = {
    {0xd31du, 2u, 3u, 1u},
    {0xd31fu, 1u, 2u, 0u},
    {0xd320u, 2u, 2u, 0u},
    {0xd322u, 2u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d326[] = {
    {0xd326u, 2u, 2u, 0u},
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d328[] = {
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d32d[] = {
    {0xd32du, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d330[] = {
    {0xd330u, 2u, 3u, 1u},
    {0xd332u, 1u, 2u, 0u},
    {0xd333u, 2u, 2u, 0u},
    {0xd335u, 2u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d339[] = {
    {0xd339u, 2u, 2u, 0u},
    {0xd33bu, 2u, 3u, 0u},
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d33d[] = {
    {0xd33du, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d340[] = {
    {0xd340u, 2u, 3u, 1u},
    {0xd342u, 1u, 2u, 0u},
    {0xd343u, 2u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d347[] = {
    {0xd347u, 2u, 2u, 0u},
    {0xd349u, 2u, 3u, 0u},
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d34b[] = {
    {0xd34bu, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d34e[] = {
    {0xd34eu, 2u, 3u, 1u},
    {0xd350u, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d353[] = {
    {0xd353u, 1u, 2u, 0u},
    {0xd354u, 2u, 3u, 0u},
    {0xd328u, 2u, 3u, 1u},
    {0xd32au, 3u, 4u, 0u},
    {0xd21bu, 2u, 3u, 0u},
    {0xd21du, 3u, 5u, 2u},
};

static const FixedSpanOpcode stake_dialog_d356[] = {
    {0xd356u, 2u, 3u, 0u},
    {0xd358u, 1u, 2u, 0u},
    {0xd359u, 3u, 3u, 0u},
};

static const FixedSpanOpcode stake_dialog_d35e[] = {
    {0xd35eu, 2u, 3u, 0u},
    {0xd360u, 2u, 2u, 0u},
    {0xd362u, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d365[] = {
    {0xd365u, 2u, 3u, 0u},
    {0xd367u, 2u, 4u, 2u},
    {0xd369u, 3u, 5u, 2u},
    {0xd36cu, 3u, 3u, 0u},
    {0xd36fu, 2u, 4u, 2u},
    {0xd371u, 3u, 3u, 0u},
    {0xd374u, 3u, 3u, 0u},
    {0xd377u, 3u, 5u, 2u},
    {0xd37au, 1u, 2u, 0u},
    {0xd37bu, 3u, 3u, 0u},
    {0xd37eu, 3u, 7u, 2u},
    {0xd381u, 1u, 2u, 0u},
    {0xd382u, 1u, 2u, 0u},
    {0xd383u, 1u, 2u, 0u},
    {0xd384u, 1u, 2u, 0u},
    {0xd385u, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d371[] = {
    {0xd371u, 3u, 3u, 0u},
    {0xd374u, 3u, 3u, 0u},
    {0xd377u, 3u, 5u, 2u},
    {0xd37au, 1u, 2u, 0u},
    {0xd37bu, 3u, 3u, 0u},
    {0xd37eu, 3u, 7u, 2u},
    {0xd381u, 1u, 2u, 0u},
    {0xd382u, 1u, 2u, 0u},
    {0xd383u, 1u, 2u, 0u},
    {0xd384u, 1u, 2u, 0u},
    {0xd385u, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d377[] = {
    {0xd377u, 3u, 5u, 2u},
    {0xd37au, 1u, 2u, 0u},
    {0xd37bu, 3u, 3u, 0u},
    {0xd37eu, 3u, 7u, 2u},
    {0xd381u, 1u, 2u, 0u},
    {0xd382u, 1u, 2u, 0u},
    {0xd383u, 1u, 2u, 0u},
    {0xd384u, 1u, 2u, 0u},
    {0xd385u, 1u, 2u, 0u},
};

static const FixedSpanOpcode stake_dialog_d388[] = {
    {0xd388u, 2u, 3u, 0u},
    {0xd38au, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d38d[] = {
    {0xd38du, 2u, 3u, 0u},
    {0xd38fu, 2u, 7u, 4u},
};

static const FixedSpanOpcode stake_dialog_d393[] = {
    {0xd393u, 3u, 3u, 0u},
    {0xd396u, 3u, 3u, 0u},
    {0xd399u, 2u, 4u, 2u},
    {0xd39bu, 3u, 3u, 0u},
    {0xd39eu, 2u, 4u, 2u},
    {0xd3a0u, 3u, 3u, 0u},
    {0xd3a3u, 2u, 4u, 2u},
    {0xd3a5u, 3u, 3u, 0u},
    {0xd3a8u, 2u, 4u, 2u},
    {0xd3aau, 3u, 3u, 0u},
    {0xd3adu, 3u, 7u, 2u},
    {0xd3b0u, 1u, 2u, 0u},
    {0xd3b1u, 1u, 2u, 0u},
    {0xd3b2u, 3u, 5u, 2u},
    {0xd3b5u, 3u, 3u, 0u},
    {0xd3b8u, 2u, 4u, 2u},
    {0xd3bau, 3u, 7u, 2u},
    {0xd3bdu, 1u, 2u, 0u},
    {0xd3beu, 1u, 2u, 0u},
    {0xd3bfu, 1u, 2u, 0u},
    {0xd3c0u, 1u, 2u, 0u},
    {0xd3c1u, 3u, 3u, 0u},
    {0xd3c4u, 3u, 7u, 2u},
    {0xd3c7u, 1u, 2u, 0u},
    {0xd3c8u, 1u, 2u, 0u},
    {0xd3c9u, 3u, 5u, 2u},
    {0xd3ccu, 3u, 3u, 0u},
    {0xd3cfu, 2u, 4u, 2u},
    {0xd3d1u, 3u, 7u, 2u},
    {0xd3d4u, 1u, 2u, 0u},
    {0xd3d5u, 1u, 2u, 0u},
    {0xd3d6u, 1u, 2u, 0u},
    {0xd3d7u, 1u, 2u, 0u},
    {0xd3d8u, 3u, 3u, 0u},
    {0xd3dbu, 1u, 2u, 0u},
    {0xd3dcu, 2u, 7u, 4u},
};

static const FixedSpanOpcode stake_dialog_d3a5[] = {
    {0xd3a5u, 3u, 3u, 0u},
    {0xd3a8u, 2u, 4u, 2u},
    {0xd3aau, 3u, 3u, 0u},
    {0xd3adu, 3u, 7u, 2u},
    {0xd3b0u, 1u, 2u, 0u},
    {0xd3b1u, 1u, 2u, 0u},
    {0xd3b2u, 3u, 5u, 2u},
    {0xd3b5u, 3u, 3u, 0u},
    {0xd3b8u, 2u, 4u, 2u},
    {0xd3bau, 3u, 7u, 2u},
    {0xd3bdu, 1u, 2u, 0u},
    {0xd3beu, 1u, 2u, 0u},
    {0xd3bfu, 1u, 2u, 0u},
    {0xd3c0u, 1u, 2u, 0u},
    {0xd3c1u, 3u, 3u, 0u},
    {0xd3c4u, 3u, 7u, 2u},
    {0xd3c7u, 1u, 2u, 0u},
    {0xd3c8u, 1u, 2u, 0u},
    {0xd3c9u, 3u, 5u, 2u},
    {0xd3ccu, 3u, 3u, 0u},
    {0xd3cfu, 2u, 4u, 2u},
    {0xd3d1u, 3u, 7u, 2u},
    {0xd3d4u, 1u, 2u, 0u},
    {0xd3d5u, 1u, 2u, 0u},
    {0xd3d6u, 1u, 2u, 0u},
    {0xd3d7u, 1u, 2u, 0u},
    {0xd3d8u, 3u, 3u, 0u},
    {0xd3dbu, 1u, 2u, 0u},
    {0xd3dcu, 2u, 7u, 4u},
};

static const FixedSpanOpcode stake_dialog_d3aa[] = {
    {0xd3aau, 3u, 3u, 0u},
    {0xd3adu, 3u, 7u, 2u},
    {0xd3b0u, 1u, 2u, 0u},
    {0xd3b1u, 1u, 2u, 0u},
    {0xd3b2u, 3u, 5u, 2u},
    {0xd3b5u, 3u, 3u, 0u},
    {0xd3b8u, 2u, 4u, 2u},
    {0xd3bau, 3u, 7u, 2u},
    {0xd3bdu, 1u, 2u, 0u},
    {0xd3beu, 1u, 2u, 0u},
    {0xd3bfu, 1u, 2u, 0u},
    {0xd3c0u, 1u, 2u, 0u},
    {0xd3c1u, 3u, 3u, 0u},
    {0xd3c4u, 3u, 7u, 2u},
    {0xd3c7u, 1u, 2u, 0u},
    {0xd3c8u, 1u, 2u, 0u},
    {0xd3c9u, 3u, 5u, 2u},
    {0xd3ccu, 3u, 3u, 0u},
    {0xd3cfu, 2u, 4u, 2u},
    {0xd3d1u, 3u, 7u, 2u},
    {0xd3d4u, 1u, 2u, 0u},
    {0xd3d5u, 1u, 2u, 0u},
    {0xd3d6u, 1u, 2u, 0u},
    {0xd3d7u, 1u, 2u, 0u},
    {0xd3d8u, 3u, 3u, 0u},
    {0xd3dbu, 1u, 2u, 0u},
    {0xd3dcu, 2u, 7u, 4u},
};

static const FixedSpanOpcode stake_dialog_d3e0[] = {
    {0xd3e0u, 2u, 4u, 2u},
    {0xd3e2u, 1u, 2u, 0u},
    {0xd3e3u, 3u, 3u, 0u},
    {0xd3e6u, 1u, 2u, 0u},
    {0xd3e7u, 3u, 3u, 0u},
    {0xd3eau, 2u, 4u, 2u},
    {0xd3ecu, 2u, 7u, 4u},
};

static const FixedSpanOpcode stake_dialog_d3f0[] = {
    {0xd3f0u, 3u, 3u, 0u},
    {0xd3f3u, 3u, 3u, 0u},
    {0xd3f6u, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d3fa[] = {
    {0xd3fau, 3u, 5u, 2u},
    {0xd3fdu, 2u, 4u, 2u},
    {0xd3ffu, 3u, 3u, 0u},
    {0xd402u, 3u, 3u, 0u},
    {0xd405u, 4u, 8u, 3u},
};

static const FixedSpanOpcode stake_dialog_d409[] = {
    {0xd409u, 2u, 3u, 0u},
    {0xd40bu, 2u, 2u, 0u},
    {0xd40du, 2u, 3u, 1u},
    {0xd40fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode stake_dialog_d412[] = {
    {0xd412u, 1u, 4u, 1u},
};

static const StakeDialogPhase stake_dialog_phases[] = {
    {0xac66u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ac66 / sizeof stake_dialog_ac66[0], stake_dialog_ac66},
    {0xac80u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ac80 / sizeof stake_dialog_ac80[0], stake_dialog_ac80},
    {0xac9bu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ac9b / sizeof stake_dialog_ac9b[0], stake_dialog_ac9b},
    {0xaca4u, 0xaca8u, 0x0000u, 1u, sizeof stake_dialog_aca4 / sizeof stake_dialog_aca4[0], stake_dialog_aca4},
    {0xacaau, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acaa / sizeof stake_dialog_acaa[0], stake_dialog_acaa},
    {0xacacu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acac / sizeof stake_dialog_acac[0], stake_dialog_acac},
    {0xacb3u, 0xacb8u, 0x0000u, 1u, sizeof stake_dialog_acb3 / sizeof stake_dialog_acb3[0], stake_dialog_acb3},
    {0xacbau, 0xacc4u, 0x0000u, 0u, sizeof stake_dialog_acba / sizeof stake_dialog_acba[0], stake_dialog_acba},
    {0xacc6u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acc6 / sizeof stake_dialog_acc6[0], stake_dialog_acc6},
    {0xacd5u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acd5 / sizeof stake_dialog_acd5[0], stake_dialog_acd5},
    {0xacdau, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acda / sizeof stake_dialog_acda[0], stake_dialog_acda},
    {0xaceeu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acee / sizeof stake_dialog_acee[0], stake_dialog_acee},
    {0xacfbu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_acfb / sizeof stake_dialog_acfb[0], stake_dialog_acfb},
    {0xad05u, 0xad0au, 0x0000u, 0u, sizeof stake_dialog_ad05 / sizeof stake_dialog_ad05[0], stake_dialog_ad05},
    {0xad0cu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad0c / sizeof stake_dialog_ad0c[0], stake_dialog_ad0c},
    {0xad14u, 0xad1bu, 0x0000u, 1u, sizeof stake_dialog_ad14 / sizeof stake_dialog_ad14[0], stake_dialog_ad14},
    {0xad1du, 0xad24u, 0x0000u, 0u, sizeof stake_dialog_ad1d / sizeof stake_dialog_ad1d[0], stake_dialog_ad1d},
    {0xad26u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad26 / sizeof stake_dialog_ad26[0], stake_dialog_ad26},
    {0xad2bu, 0xad3cu, 0x0000u, 0u, sizeof stake_dialog_ad2b / sizeof stake_dialog_ad2b[0], stake_dialog_ad2b},
    {0xad3eu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad3e / sizeof stake_dialog_ad3e[0], stake_dialog_ad3e},
    {0xad41u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad41 / sizeof stake_dialog_ad41[0], stake_dialog_ad41},
    {0xad44u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad44 / sizeof stake_dialog_ad44[0], stake_dialog_ad44},
    {0xad51u, 0x0000u, 0x0000u, 1u, sizeof stake_dialog_ad51 / sizeof stake_dialog_ad51[0], stake_dialog_ad51},
    {0xad5bu, 0xad5eu, 0x0000u, 0u, sizeof stake_dialog_ad5b / sizeof stake_dialog_ad5b[0], stake_dialog_ad5b},
    {0xad60u, 0xaca8u, 0x0000u, 0u, sizeof stake_dialog_ad60 / sizeof stake_dialog_ad60[0], stake_dialog_ad60},
    {0xad63u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad63 / sizeof stake_dialog_ad63[0], stake_dialog_ad63},
    {0xad6eu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad6e / sizeof stake_dialog_ad6e[0], stake_dialog_ad6e},
    {0xad71u, 0xad85u, 0x0000u, 0u, sizeof stake_dialog_ad71 / sizeof stake_dialog_ad71[0], stake_dialog_ad71},
    {0xad7au, 0xad85u, 0x0000u, 0u, sizeof stake_dialog_ad7a / sizeof stake_dialog_ad7a[0], stake_dialog_ad7a},
    {0xad87u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad87 / sizeof stake_dialog_ad87[0], stake_dialog_ad87},
    {0xad8au, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad8a / sizeof stake_dialog_ad8a[0], stake_dialog_ad8a},
    {0xad8du, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_ad8d / sizeof stake_dialog_ad8d[0], stake_dialog_ad8d},
    {0xad9au, 0x0000u, 0xad9cu, 1u, sizeof stake_dialog_ad9a / sizeof stake_dialog_ad9a[0], stake_dialog_ad9a},
    {0xd1efu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d1ef / sizeof stake_dialog_d1ef[0], stake_dialog_d1ef},
    {0xd1ffu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d1ff / sizeof stake_dialog_d1ff[0], stake_dialog_d1ff},
    {0xd208u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d208 / sizeof stake_dialog_d208[0], stake_dialog_d208},
    {0xd212u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d212 / sizeof stake_dialog_d212[0], stake_dialog_d212},
    {0xd21bu, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d21b / sizeof stake_dialog_d21b[0], stake_dialog_d21b},
    {0xd222u, 0xd22du, 0x0000u, 0u, sizeof stake_dialog_d222 / sizeof stake_dialog_d222[0], stake_dialog_d222},
    {0xd22fu, 0xd24du, 0x0000u, 0u, sizeof stake_dialog_d22f / sizeof stake_dialog_d22f[0], stake_dialog_d22f},
    {0xd23fu, 0xd24du, 0x0000u, 0u, sizeof stake_dialog_d23f / sizeof stake_dialog_d23f[0], stake_dialog_d23f},
    {0xd249u, 0xd24du, 0x0000u, 0u, sizeof stake_dialog_d249 / sizeof stake_dialog_d249[0], stake_dialog_d249},
    {0xd24fu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d24f / sizeof stake_dialog_d24f[0], stake_dialog_d24f},
    {0xd285u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d285 / sizeof stake_dialog_d285[0], stake_dialog_d285},
    {0xd29cu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d29c / sizeof stake_dialog_d29c[0], stake_dialog_d29c},
    {0xd2a6u, 0xd2acu, 0x0000u, 0u, sizeof stake_dialog_d2a6 / sizeof stake_dialog_d2a6[0], stake_dialog_d2a6},
    {0xd2aeu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2ae / sizeof stake_dialog_d2ae[0], stake_dialog_d2ae},
    {0xd2b5u, 0xd2bdu, 0x0000u, 1u, sizeof stake_dialog_d2b5 / sizeof stake_dialog_d2b5[0], stake_dialog_d2b5},
    {0xd2bfu, 0xd2c6u, 0x0000u, 0u, sizeof stake_dialog_d2bf / sizeof stake_dialog_d2bf[0], stake_dialog_d2bf},
    {0xd2c8u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2c8 / sizeof stake_dialog_d2c8[0], stake_dialog_d2c8},
    {0xd2cdu, 0xd2dfu, 0x0000u, 0u, sizeof stake_dialog_d2cd / sizeof stake_dialog_d2cd[0], stake_dialog_d2cd},
    {0xd2e1u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2e1 / sizeof stake_dialog_d2e1[0], stake_dialog_d2e1},
    {0xd2e4u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2e4 / sizeof stake_dialog_d2e4[0], stake_dialog_d2e4},
    {0xd2eeu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2ee / sizeof stake_dialog_d2ee[0], stake_dialog_d2ee},
    {0xd2fbu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2fb / sizeof stake_dialog_d2fb[0], stake_dialog_d2fb},
    {0xd2feu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d2fe / sizeof stake_dialog_d2fe[0], stake_dialog_d2fe},
    {0xd301u, 0xd304u, 0x0000u, 0u, sizeof stake_dialog_d301 / sizeof stake_dialog_d301[0], stake_dialog_d301},
    {0xd306u, 0xd30bu, 0x0000u, 0u, sizeof stake_dialog_d306 / sizeof stake_dialog_d306[0], stake_dialog_d306},
    {0xd30du, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d30d / sizeof stake_dialog_d30d[0], stake_dialog_d30d},
    {0xd311u, 0xd313u, 0x0000u, 1u, sizeof stake_dialog_d311 / sizeof stake_dialog_d311[0], stake_dialog_d311},
    {0xd315u, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d315 / sizeof stake_dialog_d315[0], stake_dialog_d315},
    {0xd319u, 0xd31bu, 0x0000u, 1u, sizeof stake_dialog_d319 / sizeof stake_dialog_d319[0], stake_dialog_d319},
    {0xd31du, 0xd324u, 0x0000u, 1u, sizeof stake_dialog_d31d / sizeof stake_dialog_d31d[0], stake_dialog_d31d},
    {0xd326u, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d326 / sizeof stake_dialog_d326[0], stake_dialog_d326},
    {0xd328u, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d328 / sizeof stake_dialog_d328[0], stake_dialog_d328},
    {0xd32du, 0xd32eu, 0x0000u, 1u, sizeof stake_dialog_d32d / sizeof stake_dialog_d32d[0], stake_dialog_d32d},
    {0xd330u, 0xd337u, 0x0000u, 1u, sizeof stake_dialog_d330 / sizeof stake_dialog_d330[0], stake_dialog_d330},
    {0xd339u, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d339 / sizeof stake_dialog_d339[0], stake_dialog_d339},
    {0xd33du, 0xd33eu, 0x0000u, 1u, sizeof stake_dialog_d33d / sizeof stake_dialog_d33d[0], stake_dialog_d33d},
    {0xd340u, 0xd345u, 0x0000u, 1u, sizeof stake_dialog_d340 / sizeof stake_dialog_d340[0], stake_dialog_d340},
    {0xd347u, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d347 / sizeof stake_dialog_d347[0], stake_dialog_d347},
    {0xd34bu, 0xd34cu, 0x0000u, 1u, sizeof stake_dialog_d34b / sizeof stake_dialog_d34b[0], stake_dialog_d34b},
    {0xd34eu, 0xd351u, 0x0000u, 1u, sizeof stake_dialog_d34e / sizeof stake_dialog_d34e[0], stake_dialog_d34e},
    {0xd353u, 0xd220u, 0x0000u, 1u, sizeof stake_dialog_d353 / sizeof stake_dialog_d353[0], stake_dialog_d353},
    {0xd356u, 0xd35cu, 0x0000u, 1u, sizeof stake_dialog_d356 / sizeof stake_dialog_d356[0], stake_dialog_d356},
    {0xd35eu, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d35e / sizeof stake_dialog_d35e[0], stake_dialog_d35e},
    {0xd365u, 0xd386u, 0x0000u, 1u, sizeof stake_dialog_d365 / sizeof stake_dialog_d365[0], stake_dialog_d365},
    {0xd371u, 0xd386u, 0x0000u, 0u, sizeof stake_dialog_d371 / sizeof stake_dialog_d371[0], stake_dialog_d371},
    {0xd377u, 0xd386u, 0x0000u, 0u, sizeof stake_dialog_d377 / sizeof stake_dialog_d377[0], stake_dialog_d377},
    {0xd388u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d388 / sizeof stake_dialog_d388[0], stake_dialog_d388},
    {0xd38du, 0xd391u, 0x0000u, 1u, sizeof stake_dialog_d38d / sizeof stake_dialog_d38d[0], stake_dialog_d38d},
    {0xd393u, 0xd3deu, 0x0000u, 0u, sizeof stake_dialog_d393 / sizeof stake_dialog_d393[0], stake_dialog_d393},
    {0xd3a5u, 0xd3deu, 0x0000u, 0u, sizeof stake_dialog_d3a5 / sizeof stake_dialog_d3a5[0], stake_dialog_d3a5},
    {0xd3aau, 0xd3deu, 0x0000u, 0u, sizeof stake_dialog_d3aa / sizeof stake_dialog_d3aa[0], stake_dialog_d3aa},
    {0xd3e0u, 0xd3eeu, 0x0000u, 0u, sizeof stake_dialog_d3e0 / sizeof stake_dialog_d3e0[0], stake_dialog_d3e0},
    {0xd3f0u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d3f0 / sizeof stake_dialog_d3f0[0], stake_dialog_d3f0},
    {0xd3fau, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d3fa / sizeof stake_dialog_d3fa[0], stake_dialog_d3fa},
    {0xd409u, 0x0000u, 0x0000u, 0u, sizeof stake_dialog_d409 / sizeof stake_dialog_d409[0], stake_dialog_d409},
    {0xd412u, 0x0000u, 0xd413u, 1u, sizeof stake_dialog_d412 / sizeof stake_dialog_d412[0], stake_dialog_d412},
};

enum { STAKE_DIALOG_CLOCK_CAPACITY = 2048u };

typedef struct StakeDialogClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[STAKE_DIALOG_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} StakeDialogClock;

typedef struct StakeDialogProjection {
    uint16_t index;
    uint16_t remaining;
    uint16_t selected;
    uint16_t reserved;
    uint16_t balance;
    uint16_t balance_high;
    uint16_t digits;
    uint16_t rows;
    uint16_t columns;
    uint16_t input;
    uint16_t changed;
} StakeDialogProjection;

static void StakeDialogOpcode(StakeDialogClock *clock, unsigned pc,
                              unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < STAKE_DIALOG_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static const StakeDialogPhase *FindStakeDialogPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof stake_dialog_phases / sizeof stake_dialog_phases[0]; ++i)
        if (stake_dialog_phases[i].pc == pc)
            return stake_dialog_phases + i;
    return NULL;
}

static uint16_t StakeDialogWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static bool AppendStakeDialogPhase(StakeDialogClock *clock, uint16_t pc,
                                   StakeDialogProjection projection) {
    const StakeDialogPhase *phase = FindStakeDialogPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= STAKE_DIALOG_CLOCK_CAPACITY)
        return false;
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        switch (op->pc) {
        case 0xac6fu:
        case 0xd1f5u: projection.reserved = 0u; break;
        case 0xac72u:
        case 0xd1f8u: projection.selected = StakeDialogWord(cpu, 0x69dcu); break;
        case 0xacaau: ++projection.selected; break;
        case 0xacd0u:
        case 0xd23au:
            projection.selected = (uint16_t)(projection.balance + projection.reserved);
            break;
        case 0xacd2u:
        case 0xd23cu: projection.reserved = 0u; break;
        case 0xad71u: projection.index = 0u; break;
        case 0xad74u: projection.remaining = 12u; break;
        case 0xad7du:
        case 0xad7eu:
        case 0xad82u:
        case 0xad83u: ++projection.index; break;
        case 0xad84u: --projection.remaining; break;
        case 0xd247u:
            if (projection.selected > 99u)
                return false;
            projection.digits = projection.selected;
            break;
        case 0xd24bu: projection.digits = (uint16_t)(projection.digits - 10u); break;
        case 0xd30du: projection.changed = 1u; break;
        case 0xd315u: projection.changed = 99u; break;
        case 0xd320u: projection.changed = (uint8_t)(projection.selected + 10u); break;
        case 0xd326u:
        case 0xd347u: projection.changed = 99u; break;
        case 0xd333u: projection.changed = (uint8_t)(projection.selected - 10u); break;
        case 0xd339u: projection.changed = 1u; break;
        case 0xd342u: projection.changed = (uint8_t)(projection.selected + 1u); break;
        case 0xd350u: projection.changed = (uint8_t)(projection.selected - 1u); break;
        case 0xd353u: ++projection.changed; break;
        case 0xd328u:
            projection.selected = (projection.selected & 0xff00u) | projection.changed;
            break;
        case 0xd36cu: projection.columns = 11u; break;
        case 0xd371u:
        case 0xd393u: projection.index = 0xccu; break;
        case 0xd374u: projection.remaining = 11u; break;
        case 0xd381u:
        case 0xd382u:
        case 0xd383u:
        case 0xd384u:
        case 0xd3b0u:
        case 0xd3b1u:
        case 0xd3bfu:
        case 0xd3c0u:
        case 0xd3c7u:
        case 0xd3c8u:
        case 0xd3d4u:
        case 0xd3d5u: ++projection.index; break;
        case 0xd385u: --projection.remaining; break;
        case 0xd38fu: --projection.columns; break;
        case 0xd3a0u: projection.rows = 1u; break;
        case 0xd3a5u: projection.columns = 2u; break;
        case 0xd3dcu: --projection.columns; break;
        case 0xd3ecu: --projection.rows; break;
        default: break;
        }
        unsigned cycles = op->cycles;
        switch (op->pc) {
        case 0xd377u:
        case 0xd3b2u:
        case 0xd3c9u:
            if ((0x100u >> 8) != ((0x100u + projection.index) >> 8))
                ++cycles;
            break;
        default: break;
        }
        StakeDialogOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch)
        return true;
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xaca8u:
        taken = projection.selected != 0u;
        target = taken ? 0xacacu : 0xacaau;
        break;
    case 0xacb8u:
        taken = projection.balance_high != 0u;
        target = taken ? 0xacd5u : 0xacbau;
        break;
    case 0xacc4u:
        taken = (uint16_t)(projection.balance + projection.reserved) >= projection.selected;
        target = taken ? 0xacd5u : 0xacc6u;
        break;
    case 0xad0au:
        taken = projection.reserved == projection.selected;
        target = taken ? 0xad41u : 0xad0cu;
        break;
    case 0xad1bu:
        taken = ((uint16_t)(projection.reserved - projection.selected) & 0x8000u) != 0u;
        target = taken ? 0xad2bu : 0xad1du;
        break;
    case 0xad24u:
        taken = projection.balance + (unsigned)(uint16_t)(projection.reserved - projection.selected) <= 0xffffu;
        target = taken ? 0xad41u : 0xad26u;
        break;
    case 0xad3cu:
        taken = projection.balance >= (uint16_t)(projection.selected - projection.reserved);
        target = taken ? 0xad41u : 0xad3eu;
        break;
    case 0xad5eu:
        taken = cpu->A == 1u;
        target = taken ? 0xad63u : 0xad60u;
        break;
    case 0xad85u:
        taken = projection.remaining != 0u;
        target = taken ? 0xad7au : 0xad87u;
        break;
    case 0xd220u:
        taken = projection.balance_high != 0u;
        target = taken ? 0xd23fu : 0xd222u;
        break;
    case 0xd22du:
        taken = (uint16_t)(projection.balance + projection.reserved) >= projection.selected;
        target = taken ? 0xd23fu : 0xd22fu;
        break;
    case 0xd24du:
        taken = !(projection.digits & 0x8000u);
        target = taken ? 0xd249u : 0xd24fu;
        break;
    case 0xd2acu:
        taken = projection.reserved == projection.selected;
        target = taken ? 0xd2e4u : 0xd2aeu;
        break;
    case 0xd2bdu:
        taken = ((uint16_t)(projection.reserved - projection.selected) & 0x8000u) != 0u;
        target = taken ? 0xd2cdu : 0xd2bfu;
        break;
    case 0xd2c6u:
        taken = projection.balance + (unsigned)(uint16_t)(projection.reserved - projection.selected) <= 0xffffu;
        target = taken ? 0xd2e4u : 0xd2c8u;
        break;
    case 0xd2dfu:
        taken = projection.balance >= (uint16_t)(projection.selected - projection.reserved);
        target = taken ? 0xd2e4u : 0xd2e1u;
        break;
    case 0xd304u:
        taken = cpu->A == 0u;
        target = taken ? 0xd2fbu : 0xd306u;
        break;
    case 0xd30bu:
        taken = (cpu->A & 0xffu) != 6u;
        target = taken ? 0xd311u : 0xd30du;
        break;
    case 0xd313u:
        taken = (cpu->A & 0xffu) != 5u;
        target = taken ? 0xd319u : 0xd315u;
        break;
    case 0xd31bu:
        taken = !(projection.input & 0x100u);
        target = taken ? 0xd32du : 0xd31du;
        break;
    case 0xd324u:
        taken = ((uint8_t)(projection.changed - 99u) & 0x80u) != 0u;
        target = taken ? 0xd328u : 0xd326u;
        break;
    case 0xd32eu:
        taken = !(projection.input & 0x200u);
        target = taken ? 0xd33du : 0xd330u;
        break;
    case 0xd337u:
        taken = !((uint8_t)(projection.changed - 1u) & 0x80u);
        target = taken ? 0xd328u : 0xd339u;
        break;
    case 0xd33eu:
        taken = !(projection.input & 0x400u);
        target = taken ? 0xd34bu : 0xd340u;
        break;
    case 0xd345u:
        taken = ((uint8_t)(projection.changed - 99u) & 0x80u) != 0u;
        target = taken ? 0xd328u : 0xd347u;
        break;
    case 0xd34cu:
        taken = !(projection.input & 0x800u);
        target = taken ? 0xd356u : 0xd34eu;
        break;
    case 0xd351u:
        taken = projection.changed != 0u;
        target = taken ? 0xd328u : 0xd353u;
        break;
    case 0xd35cu:
        taken = projection.input != 1u;
        target = taken ? 0xd2fbu : 0xd35eu;
        break;
    case 0xd386u:
        taken = !(projection.remaining & 0x8000u);
        target = taken ? 0xd377u : 0xd388u;
        break;
    case 0xd391u:
        taken = !(projection.columns & 0x8000u);
        target = taken ? 0xd371u : 0xd393u;
        break;
    case 0xd3deu:
        taken = !(projection.columns & 0x8000u);
        target = taken ? 0xd3aau : 0xd3e0u;
        break;
    case 0xd3eeu:
        taken = !(projection.rows & 0x8000u);
        target = taken ? 0xd3a5u : 0xd3f0u;
        break;
    default: return false;
    }
    StakeDialogOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendStakeDialogPhase(clock, target, projection);
}

static bool StakeDialogWindowSupported(const StakeDialogClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!CardScoreRegistersSupported(cpu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles)
        return false;
    unsigned budget = clock->returning
        ? cpu_region_speed(clock->next) + 2u * 8u + 3u * 6u : 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (!budget || cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x840000u, 0x1ffcu, true,
                                             1363u, budget))
        return false;
    if (g_snes->vPos < 225u) {
        const unsigned until_vblank =
            (225u - g_snes->vPos) * 1364u - g_snes->hPos;
        if (!g_snes->hdmaBeamOff || until_vblank <= budget)
            return false;
    } else if (g_snes->vPos >= 262u ||
               (262u - g_snes->vPos) * 1364u - g_snes->hPos <= budget) {
        return false;
    }
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareStakeDialogClock(StakeDialogClock *clock, uint32_t pc) {
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
    const StakeDialogPhase *phase = FindStakeDialogPhase((uint16_t)pc);
    if (!phase || (pc != 0x84d1efu && cpu->m_flag != phase->byte_mode))
        return false;
    const StakeDialogProjection projection = {cpu->X, cpu->Y,
        StakeDialogWord(cpu, 0x0bu), StakeDialogWord(cpu, 0x69e0u),
        StakeDialogWord(cpu, 0xb55u), StakeDialogWord(cpu, 0xb57u), 0u,
        StakeDialogWord(cpu, 2u), StakeDialogWord(cpu, 0u), cpu->A, 0u};
    return AppendStakeDialogPhase(clock, (uint16_t)pc, projection) &&
        StakeDialogWindowSupported(clock);
}

static uint8_t StakeDialogChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    StakeDialogClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareStakeDialogClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeStakeDialog(CpuState *cpu, uint32_t entry,
                                          ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    StakeDialogClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareStakeDialogClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, StakeDialogChild, &clock);
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
