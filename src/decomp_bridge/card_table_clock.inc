typedef struct CardTablePhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} CardTablePhase;

static const FixedSpanOpcode card_table_da9b[] = {
    {0xda9bu, 1u, 3u, 1u},
    {0xda9cu, 2u, 3u, 0u},
    {0xda9eu, 2u, 3u, 0u},
    {0xdaa0u, 2u, 2u, 0u},
    {0xdaa2u, 3u, 4u, 1u},
    {0xdaa5u, 2u, 2u, 0u},
    {0xdaa7u, 3u, 4u, 1u},
    {0xdaaau, 2u, 2u, 0u},
    {0xdaacu, 3u, 4u, 1u},
    {0xdaafu, 2u, 3u, 0u},
    {0xdab1u, 1u, 3u, 1u},
    {0xdab2u, 3u, 3u, 0u},
    {0xdab5u, 3u, 3u, 0u},
    {0xdab8u, 3u, 3u, 0u},
    {0xdabbu, 3u, 7u, 2u},
    {0xdabeu, 3u, 3u, 0u},
    {0xdac1u, 3u, 3u, 0u},
    {0xdac4u, 3u, 3u, 0u},
    {0xdac7u, 3u, 7u, 2u},
    {0xdacau, 1u, 4u, 1u},
    {0xdacbu, 2u, 3u, 0u},
    {0xdacdu, 2u, 2u, 0u},
    {0xdacfu, 2u, 3u, 1u},
    {0xdad1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dad4[] = {
    {0xdad4u, 2u, 3u, 0u},
    {0xdad6u, 3u, 3u, 0u},
    {0xdad9u, 2u, 4u, 2u},
    {0xdadbu, 3u, 3u, 0u},
    {0xdadeu, 2u, 4u, 2u},
    {0xdae0u, 3u, 3u, 0u},
    {0xdae3u, 2u, 4u, 2u},
    {0xdae5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dae8[] = {
    {0xdae8u, 3u, 3u, 0u},
    {0xdaebu, 2u, 4u, 2u},
    {0xdaedu, 3u, 3u, 0u},
    {0xdaf0u, 2u, 4u, 2u},
    {0xdaf2u, 3u, 3u, 0u},
    {0xdaf5u, 2u, 4u, 2u},
    {0xdaf7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dafa[] = {
    {0xdafau, 2u, 3u, 0u},
    {0xdafcu, 2u, 2u, 0u},
    {0xdafeu, 4u, 5u, 1u},
    {0xdb02u, 2u, 2u, 0u},
    {0xdb04u, 4u, 5u, 1u},
    {0xdb08u, 2u, 2u, 0u},
    {0xdb0au, 4u, 5u, 1u},
    {0xdb0eu, 3u, 3u, 0u},
    {0xdb11u, 2u, 4u, 2u},
    {0xdb13u, 2u, 2u, 0u},
    {0xdb15u, 2u, 3u, 1u},
    {0xdb17u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db1a[] = {
    {0xdb1au, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db1d[] = {
    {0xdb1du, 1u, 3u, 1u},
    {0xdb1eu, 2u, 2u, 0u},
    {0xdb20u, 1u, 3u, 1u},
    {0xdb21u, 1u, 4u, 1u},
    {0xdb22u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db25[] = {
    {0xdb25u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db28[] = {
    {0xdb28u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db2b[] = {
    {0xdb2bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db2e[] = {
    {0xdb2eu, 2u, 2u, 0u},
    {0xdb30u, 2u, 3u, 1u},
    {0xdb32u, 2u, 2u, 0u},
    {0xdb34u, 2u, 3u, 1u},
    {0xdb36u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db39[] = {
    {0xdb39u, 2u, 2u, 0u},
    {0xdb3bu, 3u, 4u, 1u},
    {0xdb3eu, 2u, 2u, 0u},
    {0xdb40u, 2u, 3u, 1u},
    {0xdb42u, 3u, 3u, 0u},
    {0xdb45u, 3u, 3u, 0u},
    {0xdb48u, 4u, 8u, 3u},
};

static const FixedSpanOpcode card_table_db4c[] = {
    {0xdb4cu, 2u, 2u, 0u},
    {0xdb4eu, 2u, 3u, 1u},
    {0xdb50u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db53[] = {
    {0xdb53u, 2u, 2u, 0u},
    {0xdb55u, 4u, 5u, 1u},
    {0xdb59u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db5c[] = {
    {0xdb5cu, 2u, 3u, 0u},
    {0xdb5eu, 3u, 3u, 0u},
    {0xdb61u, 2u, 4u, 2u},
    {0xdb63u, 3u, 3u, 0u},
    {0xdb66u, 3u, 5u, 2u},
    {0xdb69u, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_db6e[] = {
    {0xdb6eu, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_db73[] = {
    {0xdb73u, 3u, 5u, 2u},
    {0xdb76u, 1u, 2u, 0u},
    {0xdb77u, 3u, 3u, 0u},
};

static const FixedSpanOpcode card_table_db7c[] = {
    {0xdb7cu, 3u, 4u, 0u},
    {0xdbcau, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_db7f[] = {
    {0xdb7fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db82[] = {
    {0xdb82u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db85[] = {
    {0xdb85u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db88[] = {
    {0xdb88u, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_db8d[] = {
    {0xdb8du, 3u, 4u, 0u},
    {0xdbcau, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_db90[] = {
    {0xdb90u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db93[] = {
    {0xdb93u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db96[] = {
    {0xdb96u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_db99[] = {
    {0xdb99u, 3u, 5u, 2u},
    {0xdb9cu, 3u, 3u, 0u},
};

static const FixedSpanOpcode card_table_dba1[] = {
    {0xdba1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dba4[] = {
    {0xdba4u, 1u, 2u, 0u},
};

static const FixedSpanOpcode card_table_dba7[] = {
    {0xdba7u, 3u, 4u, 0u},
    {0xdbadu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbaa[] = {
    {0xdbaau, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbad[] = {
    {0xdbadu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbb0[] = {
    {0xdbb0u, 3u, 3u, 0u},
    {0xdbb3u, 3u, 3u, 0u},
    {0xdbb6u, 3u, 5u, 2u},
    {0xdbb9u, 3u, 7u, 2u},
    {0xdbbcu, 1u, 2u, 0u},
    {0xdbbdu, 1u, 2u, 0u},
    {0xdbbeu, 1u, 2u, 0u},
};

static const FixedSpanOpcode card_table_dbb6[] = {
    {0xdbb6u, 3u, 5u, 2u},
    {0xdbb9u, 3u, 7u, 2u},
    {0xdbbcu, 1u, 2u, 0u},
    {0xdbbdu, 1u, 2u, 0u},
    {0xdbbeu, 1u, 2u, 0u},
};

static const FixedSpanOpcode card_table_dbc1[] = {
    {0xdbc1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbc4[] = {
    {0xdbc4u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbc7[] = {
    {0xdbc7u, 3u, 4u, 0u},
    {0xdb69u, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_dbcf[] = {
    {0xdbcfu, 3u, 8u, 4u},
    {0xdbd2u, 3u, 3u, 0u},
    {0xdbd5u, 3u, 5u, 2u},
    {0xdbd8u, 3u, 5u, 2u},
    {0xdbdbu, 3u, 5u, 2u},
    {0xdbdeu, 3u, 3u, 0u},
    {0xdbe1u, 3u, 5u, 2u},
    {0xdbe4u, 3u, 3u, 0u},
    {0xdbe7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbd8[] = {
    {0xdbd8u, 3u, 5u, 2u},
    {0xdbdbu, 3u, 5u, 2u},
    {0xdbdeu, 3u, 3u, 0u},
    {0xdbe1u, 3u, 5u, 2u},
    {0xdbe4u, 3u, 3u, 0u},
    {0xdbe7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbea[] = {
    {0xdbeau, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbed[] = {
    {0xdbedu, 3u, 5u, 2u},
    {0xdbf0u, 3u, 5u, 2u},
    {0xdbf3u, 3u, 3u, 0u},
    {0xdbf6u, 3u, 5u, 2u},
    {0xdbf9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbfc[] = {
    {0xdbfcu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dbff[] = {
    {0xdbffu, 2u, 3u, 0u},
    {0xdc01u, 2u, 2u, 0u},
    {0xdc03u, 3u, 4u, 1u},
    {0xdc06u, 2u, 2u, 0u},
    {0xdc08u, 2u, 3u, 1u},
    {0xdc0au, 2u, 3u, 0u},
    {0xdc0cu, 3u, 3u, 0u},
    {0xdc0fu, 3u, 3u, 0u},
    {0xdc12u, 4u, 8u, 3u},
};

static const FixedSpanOpcode card_table_dc16[] = {
    {0xdc16u, 2u, 3u, 0u},
    {0xdc18u, 2u, 2u, 0u},
    {0xdc1au, 2u, 3u, 1u},
    {0xdc1cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dc1f[] = {
    {0xdc1fu, 2u, 3u, 0u},
    {0xdc21u, 3u, 8u, 4u},
};

static const FixedSpanOpcode card_table_dc26[] = {
    {0xdc26u, 3u, 5u, 2u},
    {0xdc29u, 3u, 3u, 0u},
};

static const FixedSpanOpcode card_table_dc2e[] = {
    {0xdc2eu, 3u, 3u, 0u},
    {0xdc31u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dc34[] = {
    {0xdc34u, 2u, 3u, 0u},
    {0xdc36u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dc39[] = {
    {0xdc39u, 2u, 2u, 0u},
    {0xdc3bu, 4u, 5u, 1u},
    {0xdc3fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_dc42[] = {
    {0xdc42u, 2u, 3u, 1u},
    {0xdc44u, 2u, 2u, 0u},
    {0xdc46u, 4u, 5u, 1u},
    {0xdc4au, 4u, 5u, 1u},
    {0xdc4eu, 4u, 5u, 1u},
    {0xdc52u, 1u, 4u, 1u},
    {0xdc53u, 1u, 4u, 1u},
};

static const FixedSpanOpcode card_table_f38a[] = {
    {0xf38au, 1u, 3u, 1u},
    {0xf38bu, 2u, 3u, 0u},
    {0xf38du, 2u, 3u, 0u},
    {0xf38fu, 2u, 2u, 0u},
    {0xf391u, 3u, 4u, 1u},
    {0xf394u, 2u, 2u, 0u},
    {0xf396u, 3u, 4u, 1u},
    {0xf399u, 2u, 2u, 0u},
    {0xf39bu, 3u, 4u, 1u},
    {0xf39eu, 2u, 3u, 0u},
    {0xf3a0u, 1u, 3u, 1u},
    {0xf3a1u, 3u, 3u, 0u},
    {0xf3a4u, 3u, 3u, 0u},
    {0xf3a7u, 3u, 3u, 0u},
    {0xf3aau, 3u, 7u, 2u},
    {0xf3adu, 3u, 3u, 0u},
    {0xf3b0u, 3u, 3u, 0u},
    {0xf3b3u, 3u, 3u, 0u},
    {0xf3b6u, 3u, 7u, 2u},
    {0xf3b9u, 1u, 4u, 1u},
    {0xf3bau, 2u, 3u, 0u},
    {0xf3bcu, 2u, 2u, 0u},
    {0xf3beu, 2u, 3u, 1u},
    {0xf3c0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f3c3[] = {
    {0xf3c3u, 2u, 3u, 0u},
    {0xf3c5u, 3u, 3u, 0u},
    {0xf3c8u, 2u, 4u, 2u},
    {0xf3cau, 3u, 3u, 0u},
    {0xf3cdu, 2u, 4u, 2u},
    {0xf3cfu, 3u, 3u, 0u},
    {0xf3d2u, 2u, 4u, 2u},
    {0xf3d4u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f3d7[] = {
    {0xf3d7u, 3u, 3u, 0u},
    {0xf3dau, 2u, 4u, 2u},
    {0xf3dcu, 3u, 3u, 0u},
    {0xf3dfu, 2u, 4u, 2u},
    {0xf3e1u, 3u, 3u, 0u},
    {0xf3e4u, 2u, 4u, 2u},
    {0xf3e6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f3e9[] = {
    {0xf3e9u, 2u, 3u, 0u},
    {0xf3ebu, 2u, 2u, 0u},
    {0xf3edu, 4u, 5u, 1u},
    {0xf3f1u, 2u, 2u, 0u},
    {0xf3f3u, 4u, 5u, 1u},
    {0xf3f7u, 2u, 2u, 0u},
    {0xf3f9u, 4u, 5u, 1u},
    {0xf3fdu, 3u, 3u, 0u},
    {0xf400u, 2u, 4u, 2u},
    {0xf402u, 2u, 2u, 0u},
    {0xf404u, 2u, 3u, 1u},
    {0xf406u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f409[] = {
    {0xf409u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f40c[] = {
    {0xf40cu, 1u, 3u, 1u},
    {0xf40du, 2u, 2u, 0u},
    {0xf40fu, 1u, 3u, 1u},
    {0xf410u, 1u, 4u, 1u},
    {0xf411u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f414[] = {
    {0xf414u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f417[] = {
    {0xf417u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f41a[] = {
    {0xf41au, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f41d[] = {
    {0xf41du, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f420[] = {
    {0xf420u, 2u, 2u, 0u},
    {0xf422u, 2u, 3u, 1u},
    {0xf424u, 2u, 2u, 0u},
    {0xf426u, 2u, 3u, 1u},
    {0xf428u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f42b[] = {
    {0xf42bu, 2u, 2u, 0u},
    {0xf42du, 3u, 4u, 1u},
    {0xf430u, 2u, 2u, 0u},
    {0xf432u, 2u, 3u, 1u},
    {0xf434u, 3u, 3u, 0u},
    {0xf437u, 3u, 3u, 0u},
    {0xf43au, 4u, 8u, 3u},
};

static const FixedSpanOpcode card_table_f43e[] = {
    {0xf43eu, 2u, 2u, 0u},
    {0xf440u, 2u, 3u, 1u},
    {0xf442u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f445[] = {
    {0xf445u, 2u, 2u, 0u},
    {0xf447u, 4u, 5u, 1u},
    {0xf44bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f44e[] = {
    {0xf44eu, 2u, 3u, 0u},
    {0xf450u, 3u, 3u, 0u},
    {0xf453u, 2u, 4u, 2u},
    {0xf455u, 2u, 4u, 2u},
    {0xf457u, 3u, 3u, 0u},
    {0xf45au, 3u, 5u, 2u},
    {0xf45du, 3u, 5u, 2u},
    {0xf460u, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_f465[] = {
    {0xf465u, 3u, 5u, 2u},
    {0xf468u, 1u, 2u, 0u},
    {0xf469u, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_f46e[] = {
    {0xf46eu, 3u, 4u, 0u},
    {0xf4a5u, 2u, 3u, 0u},
    {0xf4a7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f471[] = {
    {0xf471u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f474[] = {
    {0xf474u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f477[] = {
    {0xf477u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f47a[] = {
    {0xf47au, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_f47f[] = {
    {0xf47fu, 3u, 4u, 0u},
    {0xf4a5u, 2u, 3u, 0u},
    {0xf4a7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f482[] = {
    {0xf482u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f485[] = {
    {0xf485u, 3u, 3u, 0u},
    {0xf488u, 4u, 8u, 3u},
};

static const FixedSpanOpcode card_table_f48c[] = {
    {0xf48cu, 1u, 2u, 0u},
    {0xf48du, 3u, 3u, 0u},
    {0xf490u, 1u, 2u, 0u},
    {0xf491u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f491[] = {
    {0xf491u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f494[] = {
    {0xf494u, 1u, 2u, 0u},
};

static const FixedSpanOpcode card_table_f497[] = {
    {0xf497u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f49a[] = {
    {0xf49au, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f49d[] = {
    {0xf49du, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f4a0[] = {
    {0xf4a0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f4a3[] = {
    {0xf4a3u, 2u, 3u, 0u},
    {0xf460u, 3u, 5u, 2u},
};

static const FixedSpanOpcode card_table_f4aa[] = {
    {0xf4aau, 2u, 2u, 0u},
    {0xf4acu, 4u, 5u, 1u},
    {0xf4b0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode card_table_f4b3[] = {
    {0xf4b3u, 2u, 3u, 1u},
    {0xf4b5u, 2u, 2u, 0u},
    {0xf4b7u, 4u, 5u, 1u},
    {0xf4bbu, 4u, 5u, 1u},
    {0xf4bfu, 4u, 5u, 1u},
    {0xf4c3u, 1u, 4u, 1u},
    {0xf4c4u, 1u, 4u, 1u},
};

static const CardTablePhase card_table_phases[] = {
    {0xda9bu, 0x0000u, 0x0000u, 1u, sizeof card_table_da9b / sizeof card_table_da9b[0], card_table_da9b},
    {0xdad4u, 0x0000u, 0x0000u, 1u, sizeof card_table_dad4 / sizeof card_table_dad4[0], card_table_dad4},
    {0xdae8u, 0x0000u, 0x0000u, 0u, sizeof card_table_dae8 / sizeof card_table_dae8[0], card_table_dae8},
    {0xdafau, 0x0000u, 0x0000u, 0u, sizeof card_table_dafa / sizeof card_table_dafa[0], card_table_dafa},
    {0xdb1au, 0x0000u, 0x0000u, 1u, sizeof card_table_db1a / sizeof card_table_db1a[0], card_table_db1a},
    {0xdb1du, 0x0000u, 0x0000u, 1u, sizeof card_table_db1d / sizeof card_table_db1d[0], card_table_db1d},
    {0xdb25u, 0x0000u, 0x0000u, 1u, sizeof card_table_db25 / sizeof card_table_db25[0], card_table_db25},
    {0xdb28u, 0x0000u, 0x0000u, 1u, sizeof card_table_db28 / sizeof card_table_db28[0], card_table_db28},
    {0xdb2bu, 0x0000u, 0x0000u, 1u, sizeof card_table_db2b / sizeof card_table_db2b[0], card_table_db2b},
    {0xdb2eu, 0x0000u, 0x0000u, 1u, sizeof card_table_db2e / sizeof card_table_db2e[0], card_table_db2e},
    {0xdb39u, 0x0000u, 0x0000u, 1u, sizeof card_table_db39 / sizeof card_table_db39[0], card_table_db39},
    {0xdb4cu, 0x0000u, 0x0000u, 1u, sizeof card_table_db4c / sizeof card_table_db4c[0], card_table_db4c},
    {0xdb53u, 0x0000u, 0x0000u, 1u, sizeof card_table_db53 / sizeof card_table_db53[0], card_table_db53},
    {0xdb5cu, 0xdb6cu, 0x0000u, 1u, sizeof card_table_db5c / sizeof card_table_db5c[0], card_table_db5c},
    {0xdb6eu, 0xdb71u, 0x0000u, 0u, sizeof card_table_db6e / sizeof card_table_db6e[0], card_table_db6e},
    {0xdb73u, 0xdb7au, 0x0000u, 0u, sizeof card_table_db73 / sizeof card_table_db73[0], card_table_db73},
    {0xdb7cu, 0xdbcdu, 0x0000u, 0u, sizeof card_table_db7c / sizeof card_table_db7c[0], card_table_db7c},
    {0xdb7fu, 0x0000u, 0x0000u, 0u, sizeof card_table_db7f / sizeof card_table_db7f[0], card_table_db7f},
    {0xdb82u, 0x0000u, 0x0000u, 0u, sizeof card_table_db82 / sizeof card_table_db82[0], card_table_db82},
    {0xdb85u, 0x0000u, 0x0000u, 0u, sizeof card_table_db85 / sizeof card_table_db85[0], card_table_db85},
    {0xdb88u, 0xdb8bu, 0x0000u, 0u, sizeof card_table_db88 / sizeof card_table_db88[0], card_table_db88},
    {0xdb8du, 0xdbcdu, 0x0000u, 0u, sizeof card_table_db8d / sizeof card_table_db8d[0], card_table_db8d},
    {0xdb90u, 0x0000u, 0x0000u, 0u, sizeof card_table_db90 / sizeof card_table_db90[0], card_table_db90},
    {0xdb93u, 0x0000u, 0x0000u, 0u, sizeof card_table_db93 / sizeof card_table_db93[0], card_table_db93},
    {0xdb96u, 0x0000u, 0x0000u, 0u, sizeof card_table_db96 / sizeof card_table_db96[0], card_table_db96},
    {0xdb99u, 0xdb9fu, 0x0000u, 0u, sizeof card_table_db99 / sizeof card_table_db99[0], card_table_db99},
    {0xdba1u, 0x0000u, 0x0000u, 0u, sizeof card_table_dba1 / sizeof card_table_dba1[0], card_table_dba1},
    {0xdba4u, 0xdba5u, 0x0000u, 0u, sizeof card_table_dba4 / sizeof card_table_dba4[0], card_table_dba4},
    {0xdba7u, 0x0000u, 0x0000u, 0u, sizeof card_table_dba7 / sizeof card_table_dba7[0], card_table_dba7},
    {0xdbaau, 0x0000u, 0x0000u, 0u, sizeof card_table_dbaa / sizeof card_table_dbaa[0], card_table_dbaa},
    {0xdbadu, 0x0000u, 0x0000u, 0u, sizeof card_table_dbad / sizeof card_table_dbad[0], card_table_dbad},
    {0xdbb0u, 0xdbbfu, 0x0000u, 0u, sizeof card_table_dbb0 / sizeof card_table_dbb0[0], card_table_dbb0},
    {0xdbb6u, 0xdbbfu, 0x0000u, 0u, sizeof card_table_dbb6 / sizeof card_table_dbb6[0], card_table_dbb6},
    {0xdbc1u, 0x0000u, 0x0000u, 0u, sizeof card_table_dbc1 / sizeof card_table_dbc1[0], card_table_dbc1},
    {0xdbc4u, 0x0000u, 0x0000u, 0u, sizeof card_table_dbc4 / sizeof card_table_dbc4[0], card_table_dbc4},
    {0xdbc7u, 0xdb6cu, 0x0000u, 0u, sizeof card_table_dbc7 / sizeof card_table_dbc7[0], card_table_dbc7},
    {0xdbcfu, 0x0000u, 0x0000u, 0u, sizeof card_table_dbcf / sizeof card_table_dbcf[0], card_table_dbcf},
    {0xdbd8u, 0x0000u, 0x0000u, 0u, sizeof card_table_dbd8 / sizeof card_table_dbd8[0], card_table_dbd8},
    {0xdbeau, 0x0000u, 0x0000u, 0u, sizeof card_table_dbea / sizeof card_table_dbea[0], card_table_dbea},
    {0xdbedu, 0x0000u, 0x0000u, 0u, sizeof card_table_dbed / sizeof card_table_dbed[0], card_table_dbed},
    {0xdbfcu, 0x0000u, 0x0000u, 0u, sizeof card_table_dbfc / sizeof card_table_dbfc[0], card_table_dbfc},
    {0xdbffu, 0x0000u, 0x0000u, 0u, sizeof card_table_dbff / sizeof card_table_dbff[0], card_table_dbff},
    {0xdc16u, 0x0000u, 0x0000u, 0u, sizeof card_table_dc16 / sizeof card_table_dc16[0], card_table_dc16},
    {0xdc1fu, 0xdc24u, 0x0000u, 1u, sizeof card_table_dc1f / sizeof card_table_dc1f[0], card_table_dc1f},
    {0xdc26u, 0xdc2cu, 0x0000u, 0u, sizeof card_table_dc26 / sizeof card_table_dc26[0], card_table_dc26},
    {0xdc2eu, 0x0000u, 0x0000u, 0u, sizeof card_table_dc2e / sizeof card_table_dc2e[0], card_table_dc2e},
    {0xdc34u, 0x0000u, 0x0000u, 0u, sizeof card_table_dc34 / sizeof card_table_dc34[0], card_table_dc34},
    {0xdc39u, 0x0000u, 0x0000u, 1u, sizeof card_table_dc39 / sizeof card_table_dc39[0], card_table_dc39},
    {0xdc42u, 0x0000u, 0xdc54u, 1u, sizeof card_table_dc42 / sizeof card_table_dc42[0], card_table_dc42},
    {0xf38au, 0x0000u, 0x0000u, 1u, sizeof card_table_f38a / sizeof card_table_f38a[0], card_table_f38a},
    {0xf3c3u, 0x0000u, 0x0000u, 1u, sizeof card_table_f3c3 / sizeof card_table_f3c3[0], card_table_f3c3},
    {0xf3d7u, 0x0000u, 0x0000u, 0u, sizeof card_table_f3d7 / sizeof card_table_f3d7[0], card_table_f3d7},
    {0xf3e9u, 0x0000u, 0x0000u, 0u, sizeof card_table_f3e9 / sizeof card_table_f3e9[0], card_table_f3e9},
    {0xf409u, 0x0000u, 0x0000u, 1u, sizeof card_table_f409 / sizeof card_table_f409[0], card_table_f409},
    {0xf40cu, 0x0000u, 0x0000u, 1u, sizeof card_table_f40c / sizeof card_table_f40c[0], card_table_f40c},
    {0xf414u, 0x0000u, 0x0000u, 1u, sizeof card_table_f414 / sizeof card_table_f414[0], card_table_f414},
    {0xf417u, 0x0000u, 0x0000u, 1u, sizeof card_table_f417 / sizeof card_table_f417[0], card_table_f417},
    {0xf41au, 0x0000u, 0x0000u, 1u, sizeof card_table_f41a / sizeof card_table_f41a[0], card_table_f41a},
    {0xf41du, 0x0000u, 0x0000u, 1u, sizeof card_table_f41d / sizeof card_table_f41d[0], card_table_f41d},
    {0xf420u, 0x0000u, 0x0000u, 1u, sizeof card_table_f420 / sizeof card_table_f420[0], card_table_f420},
    {0xf42bu, 0x0000u, 0x0000u, 1u, sizeof card_table_f42b / sizeof card_table_f42b[0], card_table_f42b},
    {0xf43eu, 0x0000u, 0x0000u, 1u, sizeof card_table_f43e / sizeof card_table_f43e[0], card_table_f43e},
    {0xf445u, 0x0000u, 0x0000u, 1u, sizeof card_table_f445 / sizeof card_table_f445[0], card_table_f445},
    {0xf44eu, 0xf463u, 0x0000u, 1u, sizeof card_table_f44e / sizeof card_table_f44e[0], card_table_f44e},
    {0xf465u, 0xf46cu, 0x0000u, 0u, sizeof card_table_f465 / sizeof card_table_f465[0], card_table_f465},
    {0xf46eu, 0x0000u, 0x0000u, 0u, sizeof card_table_f46e / sizeof card_table_f46e[0], card_table_f46e},
    {0xf471u, 0x0000u, 0x0000u, 0u, sizeof card_table_f471 / sizeof card_table_f471[0], card_table_f471},
    {0xf474u, 0x0000u, 0x0000u, 0u, sizeof card_table_f474 / sizeof card_table_f474[0], card_table_f474},
    {0xf477u, 0x0000u, 0x0000u, 0u, sizeof card_table_f477 / sizeof card_table_f477[0], card_table_f477},
    {0xf47au, 0xf47du, 0x0000u, 0u, sizeof card_table_f47a / sizeof card_table_f47a[0], card_table_f47a},
    {0xf47fu, 0x0000u, 0x0000u, 0u, sizeof card_table_f47f / sizeof card_table_f47f[0], card_table_f47f},
    {0xf482u, 0x0000u, 0x0000u, 0u, sizeof card_table_f482 / sizeof card_table_f482[0], card_table_f482},
    {0xf485u, 0x0000u, 0x0000u, 0u, sizeof card_table_f485 / sizeof card_table_f485[0], card_table_f485},
    {0xf48cu, 0x0000u, 0x0000u, 0u, sizeof card_table_f48c / sizeof card_table_f48c[0], card_table_f48c},
    {0xf491u, 0x0000u, 0x0000u, 0u, sizeof card_table_f491 / sizeof card_table_f491[0], card_table_f491},
    {0xf494u, 0xf495u, 0x0000u, 0u, sizeof card_table_f494 / sizeof card_table_f494[0], card_table_f494},
    {0xf497u, 0x0000u, 0x0000u, 0u, sizeof card_table_f497 / sizeof card_table_f497[0], card_table_f497},
    {0xf49au, 0x0000u, 0x0000u, 0u, sizeof card_table_f49a / sizeof card_table_f49a[0], card_table_f49a},
    {0xf49du, 0x0000u, 0x0000u, 0u, sizeof card_table_f49d / sizeof card_table_f49d[0], card_table_f49d},
    {0xf4a0u, 0x0000u, 0x0000u, 0u, sizeof card_table_f4a0 / sizeof card_table_f4a0[0], card_table_f4a0},
    {0xf4a3u, 0xf463u, 0x0000u, 0u, sizeof card_table_f4a3 / sizeof card_table_f4a3[0], card_table_f4a3},
    {0xf4aau, 0x0000u, 0x0000u, 1u, sizeof card_table_f4aa / sizeof card_table_f4aa[0], card_table_f4aa},
    {0xf4b3u, 0x0000u, 0xf4c5u, 1u, sizeof card_table_f4b3 / sizeof card_table_f4b3[0], card_table_f4b3},
};

enum { CARD_TABLE_CLOCK_CAPACITY = 768u };

typedef struct CardTableClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[CARD_TABLE_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} CardTableClock;

typedef struct CardTableProjection {
    uint16_t x;
    uint16_t y;
    uint16_t stake;
} CardTableProjection;

static uint16_t CardTableWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

static const CardTablePhase *FindCardTablePhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof card_table_phases / sizeof card_table_phases[0]; ++i)
        if (card_table_phases[i].pc == pc) return card_table_phases + i;
    return NULL;
}

static bool AppendCardTablePhase(CardTableClock *clock, uint16_t pc,
                                 CardTableProjection projection) {
    const CardTablePhase *phase = FindCardTablePhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase) return false;
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode op = phase->opcodes[i];
        const unsigned copies = op.pc == 0xdabbu || op.pc == 0xdac7u ||
            op.pc == 0xf3aau || op.pc == 0xf3b6u ? 256u : 1u;
        if (clock->span.count + copies >= CARD_TABLE_CLOCK_CAPACITY) return false;
        for (unsigned copy = 0u; copy < copies; ++copy)
            clock->timing[clock->span.count++] = op;
        switch (op.pc) {
        case 0xdbb0u: projection.x = 0u; break;
        case 0xdbb3u: projection.y = 3u; break;
        case 0xdbbcu:
        case 0xdbbdu: ++projection.x; break;
        case 0xdbbeu:
        case 0xf494u: --projection.y; break;
        case 0xf45au: projection.stake = cpu->ram[0x30u]; break;
        case 0xf490u: projection.y = (uint16_t)(cpu->A + 10u); break;
        default: break;
        }
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch) return true;
    bool taken;
    uint16_t target;
    switch (phase->branch) {
    case 0xdb6cu: taken = CardTableWord(cpu, 0x71aeu) != 0u; target = 0xdb7fu; break;
    case 0xdb71u: taken = CardTableWord(cpu, 0xb57u) != 0u; target = 0xdb7fu; break;
    case 0xdb7au: taken = CardTableWord(cpu, 0xb55u) >= 10u; target = 0xdb7fu; break;
    case 0xdb8bu: taken = CardTableWord(cpu, 0x7184u) != 0u; target = 0xdb90u; break;
    case 0xdb9fu: taken = !(CardTableWord(cpu, 0x7170u) & 6u); target = 0xdbaau; break;
    case 0xdba5u: taken = (uint16_t)(cpu->A - 1u) != 0u; target = 0xdbaau; break;
    case 0xdbbfu: taken = projection.y != 0u; target = 0xdbb6u; break;
    case 0xdbcdu: taken = !CardTableWord(cpu, 0x71aeu); target = 0xdc26u; break;
    case 0xdc24u:
        taken = !((uint16_t)(CardTableWord(cpu, 0x71aeu) - 1u) & 0x8000u);
        target = 0xdbd8u; break;
    case 0xdc2cu: taken = !(CardTableWord(cpu, 0x7170u) & 6u); target = 0xdc34u; break;
    case 0xf463u: taken = CardTableWord(cpu, 0xb57u) != 0u; target = 0xf471u; break;
    case 0xf46cu:
        taken = CardTableWord(cpu, 0xb55u) >= projection.stake;
        target = 0xf471u; break;
    case 0xf47du: taken = CardTableWord(cpu, 0x7184u) != 0u; target = 0xf482u; break;
    case 0xf495u: taken = projection.y != 0u; target = 0xf491u; break;
    default: return false;
    }
    if (clock->span.count == CARD_TABLE_CLOCK_CAPACITY) return false;
    clock->timing[clock->span.count++] = (FixedSpanOpcode){phase->branch, 2u,
        (uint8_t)(taken ? 3u : 2u), 0u};
    return AppendCardTablePhase(clock, taken ? target : (uint16_t)(phase->branch + 2u), projection);
}

static bool CardTableWindowSupported(const CardTableClock *clock) {
    CpuState *cpu = clock->call.cpu;
    unsigned budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x84da9bu, 0x1ffcu, true, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareCardTableClock(CardTableClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = pc;
    clock->returning = false;
    if (!cpu->ram || !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        pc >> 16 != 0x84u || cpu->PB != 0x84u || cpu->emulation ||
        cpu->m_flag > 1u || cpu->x_flag > 1u || cpu->host_return_valid < 2u ||
        cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        cpu->D || cpu->_flag_D) return false;
    const CardTablePhase *phase = FindCardTablePhase((uint16_t)pc);
    if (!phase) return false;
    const bool entry = pc == 0x84da9bu || pc == 0x84f38au;
    if (!entry) {
        if (cpu->x_flag) return false;
        const bool normalizes = pc == 0x84dad4u || pc == 0x84f3c3u ||
            pc == 0x84dafau || pc == 0x84f3e9u || pc == 0x84db5cu ||
            pc == 0x84f44eu || pc == 0x84dbffu || pc == 0x84dc16u ||
            pc == 0x84dc1fu || pc == 0x84dc34u;
        if (!normalizes && cpu->m_flag != phase->byte_mode) return false;
    }
    const bool gameplay = (pc >= 0x84db25u && pc < 0x84dc55u) ||
        (pc >= 0x84f414u && pc < 0x84f4c6u);
    if (gameplay ? cpu->DB != 0x7eu :
        (cpu->DB != 0u && cpu->DB != 0x80u && cpu->DB != 0x7eu && cpu->DB != 0x7fu))
        return false;
    const CardTableProjection projection = {cpu->X, cpu->Y, CardTableWord(cpu, 0x71b6u)};
    return AppendCardTablePhase(clock, (uint16_t)pc, projection) && CardTableWindowSupported(clock);
}

static uint8_t CardTableChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    CardTableClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareCardTableClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static void FinishCardTableBoundary(CardTableClock *clock, uint32_t pc) {
    unsigned end = clock->span.index;
    while (end < clock->span.count && (clock->span.bank | clock->timing[end].pc) != pc) {
        assert(!clock->timing[end].data_bytes);
        ++end;
    }
    assert(end < clock->span.count && !clock->span.remaining);
    clock->span.count = end;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == end && !clock->span.remaining);
}

static RecompReturn ActorBridgeCardTable(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CardTableClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareCardTableClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, CardTableChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED) {
        FinishCardTableBoundary(&clock, result.pc);
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    }
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && clock.next == result.pc &&
        clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
