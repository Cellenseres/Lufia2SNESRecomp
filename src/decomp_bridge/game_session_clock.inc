typedef struct GameSessionPhase {
    uint16_t pc;
    uint16_t branch;
    uint16_t return_pc;
    uint8_t byte_mode;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} GameSessionPhase;

static const FixedSpanOpcode game_session_a705[] = {
    {0xa705u, 1u, 3u, 1u},
    {0xa706u, 2u, 3u, 0u},
    {0xa708u, 2u, 2u, 0u},
    {0xa70au, 3u, 4u, 1u},
    {0xa70du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a710[] = {
    {0xa710u, 1u, 3u, 1u},
    {0xa711u, 2u, 2u, 0u},
    {0xa713u, 1u, 3u, 1u},
    {0xa714u, 1u, 4u, 1u},
    {0xa715u, 3u, 3u, 0u},
    {0xa718u, 3u, 5u, 2u},
    {0xa71bu, 2u, 3u, 0u},
    {0xa71du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a720[] = {
    {0xa720u, 3u, 5u, 2u},
    {0xa723u, 3u, 5u, 2u},
    {0xa726u, 3u, 5u, 2u},
    {0xa729u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a72c[] = {
    {0xa72cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a72f[] = {
    {0xa72fu, 2u, 3u, 0u},
    {0xa731u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a734[] = {
    {0xa734u, 2u, 2u, 0u},
    {0xa736u, 2u, 3u, 1u},
    {0xa738u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a73b[] = {
    {0xa73bu, 2u, 3u, 0u},
    {0xa73du, 2u, 4u, 2u},
    {0xa73fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a742[] = {
    {0xa742u, 2u, 7u, 4u},
    {0xa744u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a747[] = {
    {0xa747u, 2u, 7u, 4u},
    {0xa749u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a74c[] = {
    {0xa74cu, 2u, 7u, 4u},
    {0xa74eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a751[] = {
    {0xa751u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a754[] = {
    {0xa754u, 2u, 4u, 2u},
    {0xa756u, 3u, 3u, 0u},
    {0xa759u, 2u, 4u, 2u},
    {0xa75bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a75e[] = {
    {0xa75eu, 2u, 4u, 2u},
    {0xa760u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a765[] = {
    {0xa765u, 3u, 4u, 0u},
    {0xa88au, 3u, 3u, 0u},
    {0xa88du, 2u, 4u, 2u},
    {0xa88fu, 2u, 4u, 2u},
    {0xa891u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a768[] = {
    {0xa768u, 2u, 3u, 0u},
    {0xa76au, 2u, 2u, 0u},
    {0xa76cu, 2u, 3u, 1u},
    {0xa76eu, 3u, 3u, 0u},
    {0xa771u, 2u, 4u, 2u},
    {0xa773u, 2u, 3u, 0u},
    {0xa775u, 3u, 3u, 0u},
    {0xa778u, 3u, 5u, 2u},
    {0xa77bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a77e[] = {
    {0xa77eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a781[] = {
    {0xa781u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a786[] = {
    {0xa786u, 3u, 3u, 0u},
    {0xa789u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a78c[] = {
    {0xa78cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a78f[] = {
    {0xa78fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a792[] = {
    {0xa792u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a795[] = {
    {0xa795u, 3u, 5u, 2u},
    {0xa798u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a79d[] = {
    {0xa79du, 2u, 4u, 2u},
    {0xa79fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7a2[] = {
    {0xa7a2u, 3u, 3u, 0u},
    {0xa7a5u, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a7a9[] = {
    {0xa7a9u, 3u, 5u, 2u},
    {0xa7acu, 1u, 2u, 0u},
    {0xa7adu, 3u, 3u, 0u},
    {0xa7b0u, 3u, 5u, 2u},
    {0xa7b3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7b6[] = {
    {0xa7b6u, 2u, 3u, 0u},
    {0xa77eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7b8[] = {
    {0xa7b8u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a7bd[] = {
    {0xa7bdu, 3u, 3u, 0u},
    {0xa7c0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7c3[] = {
    {0xa7c3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7c6[] = {
    {0xa7c6u, 2u, 7u, 4u},
    {0xa7c8u, 2u, 4u, 2u},
    {0xa7cau, 2u, 4u, 2u},
    {0xa7ccu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7c8[] = {
    {0xa7c8u, 2u, 4u, 2u},
    {0xa7cau, 2u, 4u, 2u},
    {0xa7ccu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7cf[] = {
    {0xa7cfu, 2u, 4u, 2u},
    {0xa7d1u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a7d6[] = {
    {0xa7d6u, 2u, 4u, 2u},
    {0xa7d8u, 3u, 5u, 2u},
    {0xa7dbu, 3u, 3u, 0u},
    {0xa7deu, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a7e3[] = {
    {0xa7e3u, 3u, 3u, 0u},
    {0xa7e6u, 1u, 2u, 0u},
    {0xa7e7u, 2u, 4u, 2u},
    {0xa7e9u, 2u, 4u, 2u},
    {0xa7ebu, 3u, 3u, 0u},
    {0xa7eeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a7e6[] = {
    {0xa7e6u, 1u, 2u, 0u},
    {0xa7e7u, 2u, 4u, 2u},
    {0xa7e9u, 2u, 4u, 2u},
    {0xa7ebu, 3u, 3u, 0u},
    {0xa7eeu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a7f2[] = {
    {0xa7f2u, 2u, 3u, 0u},
    {0xa800u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a7f4[] = {
    {0xa7f4u, 3u, 3u, 0u},
    {0xa7f7u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_a7fb[] = {
    {0xa7fbu, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a800[] = {
    {0xa800u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a803[] = {
    {0xa803u, 2u, 4u, 2u},
    {0xa805u, 1u, 2u, 0u},
    {0xa806u, 1u, 2u, 0u},
    {0xa807u, 1u, 2u, 0u},
    {0xa808u, 3u, 5u, 2u},
    {0xa80bu, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a810[] = {
    {0xa810u, 2u, 7u, 4u},
    {0xa812u, 3u, 3u, 0u},
    {0xa815u, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a819[] = {
    {0xa819u, 2u, 4u, 2u},
    {0xa81bu, 2u, 4u, 2u},
    {0xa81du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a820[] = {
    {0xa820u, 2u, 4u, 2u},
    {0xa822u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a827[] = {
    {0xa827u, 3u, 3u, 0u},
    {0xa82au, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_a82e[] = {
    {0xa82eu, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a833[] = {
    {0xa833u, 3u, 3u, 0u},
    {0xa836u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a83b[] = {
    {0xa83bu, 2u, 3u, 0u},
    {0xa88au, 3u, 3u, 0u},
    {0xa88du, 2u, 4u, 2u},
    {0xa88fu, 2u, 4u, 2u},
    {0xa891u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a83d[] = {
    {0xa83du, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a842[] = {
    {0xa842u, 2u, 4u, 2u},
    {0xa844u, 3u, 5u, 2u},
    {0xa847u, 3u, 3u, 0u},
    {0xa84au, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a84f[] = {
    {0xa84fu, 3u, 3u, 0u},
    {0xa852u, 1u, 2u, 0u},
    {0xa853u, 2u, 4u, 2u},
    {0xa855u, 2u, 4u, 2u},
    {0xa857u, 3u, 3u, 0u},
    {0xa85au, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a852[] = {
    {0xa852u, 1u, 2u, 0u},
    {0xa853u, 2u, 4u, 2u},
    {0xa855u, 2u, 4u, 2u},
    {0xa857u, 3u, 3u, 0u},
    {0xa85au, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a85e[] = {
    {0xa85eu, 2u, 4u, 2u},
    {0xa860u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a865[] = {
    {0xa865u, 3u, 3u, 0u},
    {0xa868u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a86d[] = {
    {0xa86du, 2u, 7u, 4u},
    {0xa86fu, 2u, 7u, 4u},
    {0xa871u, 2u, 4u, 2u},
    {0xa873u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a878[] = {
    {0xa878u, 2u, 3u, 0u},
    {0xa842u, 2u, 4u, 2u},
    {0xa844u, 3u, 5u, 2u},
    {0xa847u, 3u, 3u, 0u},
    {0xa84au, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a87a[] = {
    {0xa87au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a87d[] = {
    {0xa87du, 2u, 4u, 2u},
    {0xa87fu, 1u, 2u, 0u},
    {0xa880u, 1u, 2u, 0u},
    {0xa881u, 1u, 2u, 0u},
    {0xa882u, 3u, 5u, 2u},
    {0xa885u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_a88a[] = {
    {0xa88au, 3u, 3u, 0u},
    {0xa88du, 2u, 4u, 2u},
    {0xa88fu, 2u, 4u, 2u},
    {0xa891u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a891[] = {
    {0xa891u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a894[] = {
    {0xa894u, 2u, 4u, 2u},
    {0xa896u, 1u, 2u, 0u},
    {0xa897u, 1u, 2u, 0u},
    {0xa898u, 1u, 2u, 0u},
    {0xa899u, 2u, 4u, 2u},
    {0xa89bu, 3u, 7u, 2u},
    {0xa89eu, 2u, 7u, 4u},
    {0xa8a0u, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_session_a8a4[] = {
    {0xa8a4u, 3u, 3u, 0u},
    {0xa8a7u, 2u, 4u, 2u},
    {0xa8a9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a8ac[] = {
    {0xa8acu, 2u, 4u, 2u},
    {0xa8aeu, 3u, 5u, 2u},
    {0xa8b1u, 3u, 3u, 0u},
    {0xa8b4u, 2u, 4u, 2u},
    {0xa8b6u, 2u, 4u, 2u},
    {0xa8b8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a8b8[] = {
    {0xa8b8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a8bb[] = {
    {0xa8bbu, 2u, 7u, 4u},
    {0xa8bdu, 2u, 4u, 2u},
    {0xa8bfu, 1u, 2u, 0u},
    {0xa8c0u, 1u, 2u, 0u},
    {0xa8c1u, 1u, 2u, 0u},
    {0xa8c2u, 2u, 4u, 2u},
    {0xa8c4u, 3u, 7u, 2u},
    {0xa8c7u, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_session_a8cb[] = {
    {0xa8cbu, 3u, 3u, 0u},
    {0xa8ceu, 3u, 3u, 0u},
    {0xa8d1u, 3u, 7u, 2u},
    {0xa8d4u, 1u, 2u, 0u},
    {0xa8d5u, 1u, 2u, 0u},
    {0xa8d6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_session_a8d1[] = {
    {0xa8d1u, 3u, 7u, 2u},
    {0xa8d4u, 1u, 2u, 0u},
    {0xa8d5u, 1u, 2u, 0u},
    {0xa8d6u, 1u, 2u, 0u},
};

static const FixedSpanOpcode game_session_a8d9[] = {
    {0xa8d9u, 3u, 5u, 2u},
    {0xa8dcu, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8e1[] = {
    {0xa8e1u, 3u, 8u, 4u},
    {0xa8e4u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8e4[] = {
    {0xa8e4u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8e9[] = {
    {0xa8e9u, 3u, 8u, 4u},
    {0xa8ecu, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8ec[] = {
    {0xa8ecu, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8f1[] = {
    {0xa8f1u, 3u, 8u, 4u},
    {0xa8f4u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8f4[] = {
    {0xa8f4u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_a8f9[] = {
    {0xa8f9u, 3u, 3u, 0u},
    {0xa8fcu, 2u, 4u, 2u},
    {0xa8feu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a901[] = {
    {0xa901u, 2u, 4u, 2u},
    {0xa903u, 3u, 5u, 2u},
    {0xa906u, 3u, 3u, 0u},
    {0xa909u, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a90d[] = {
    {0xa90du, 3u, 5u, 2u},
    {0xa910u, 3u, 3u, 0u},
    {0xa913u, 2u, 4u, 2u},
    {0xa915u, 3u, 3u, 0u},
    {0xa918u, 2u, 4u, 2u},
    {0xa91au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a910[] = {
    {0xa910u, 3u, 3u, 0u},
    {0xa913u, 2u, 4u, 2u},
    {0xa915u, 3u, 3u, 0u},
    {0xa918u, 2u, 4u, 2u},
    {0xa91au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a91a[] = {
    {0xa91au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a91d[] = {
    {0xa91du, 2u, 4u, 2u},
    {0xa91fu, 1u, 2u, 0u},
    {0xa920u, 1u, 2u, 0u},
    {0xa921u, 1u, 2u, 0u},
    {0xa922u, 3u, 3u, 0u},
    {0xa925u, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a929[] = {
    {0xa929u, 3u, 5u, 2u},
    {0xa92cu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a930[] = {
    {0xa930u, 3u, 10u, 4u},
    {0xa933u, 2u, 7u, 4u},
    {0xa935u, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_session_a933[] = {
    {0xa933u, 2u, 7u, 4u},
    {0xa935u, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_session_a939[] = {
    {0xa939u, 3u, 3u, 0u},
    {0xa93cu, 2u, 4u, 2u},
    {0xa93eu, 2u, 4u, 2u},
    {0xa940u, 1u, 2u, 0u},
    {0xa941u, 1u, 2u, 0u},
    {0xa942u, 2u, 4u, 2u},
    {0xa944u, 1u, 2u, 0u},
    {0xa945u, 4u, 6u, 2u},
    {0xa949u, 2u, 4u, 2u},
    {0xa94bu, 3u, 5u, 2u},
    {0xa94eu, 1u, 2u, 0u},
    {0xa94fu, 2u, 4u, 2u},
    {0xa951u, 1u, 2u, 0u},
    {0xa952u, 1u, 2u, 0u},
    {0xa953u, 1u, 2u, 0u},
    {0xa954u, 1u, 2u, 0u},
    {0xa955u, 1u, 2u, 0u},
    {0xa956u, 2u, 4u, 2u},
    {0xa958u, 1u, 2u, 0u},
    {0xa959u, 2u, 4u, 2u},
    {0xa95bu, 3u, 3u, 0u},
    {0xa95eu, 2u, 4u, 2u},
    {0xa960u, 3u, 5u, 2u},
    {0xa963u, 2u, 4u, 2u},
    {0xa965u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a93e[] = {
    {0xa93eu, 2u, 4u, 2u},
    {0xa940u, 1u, 2u, 0u},
    {0xa941u, 1u, 2u, 0u},
    {0xa942u, 2u, 4u, 2u},
    {0xa944u, 1u, 2u, 0u},
    {0xa945u, 4u, 6u, 2u},
    {0xa949u, 2u, 4u, 2u},
    {0xa94bu, 3u, 5u, 2u},
    {0xa94eu, 1u, 2u, 0u},
    {0xa94fu, 2u, 4u, 2u},
    {0xa951u, 1u, 2u, 0u},
    {0xa952u, 1u, 2u, 0u},
    {0xa953u, 1u, 2u, 0u},
    {0xa954u, 1u, 2u, 0u},
    {0xa955u, 1u, 2u, 0u},
    {0xa956u, 2u, 4u, 2u},
    {0xa958u, 1u, 2u, 0u},
    {0xa959u, 2u, 4u, 2u},
    {0xa95bu, 3u, 3u, 0u},
    {0xa95eu, 2u, 4u, 2u},
    {0xa960u, 3u, 5u, 2u},
    {0xa963u, 2u, 4u, 2u},
    {0xa965u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a95b[] = {
    {0xa95bu, 3u, 3u, 0u},
    {0xa95eu, 2u, 4u, 2u},
    {0xa960u, 3u, 5u, 2u},
    {0xa963u, 2u, 4u, 2u},
    {0xa965u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a968[] = {
    {0xa968u, 3u, 3u, 0u},
    {0xa96bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a96e[] = {
    {0xa96eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a971[] = {
    {0xa971u, 2u, 4u, 2u},
    {0xa973u, 1u, 2u, 0u},
    {0xa974u, 3u, 3u, 0u},
    {0xa977u, 2u, 4u, 2u},
    {0xa979u, 2u, 4u, 2u},
    {0xa97bu, 1u, 2u, 0u},
    {0xa97cu, 1u, 2u, 0u},
    {0xa97du, 2u, 4u, 2u},
    {0xa97fu, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_session_a983[] = {
    {0xa983u, 2u, 3u, 0u},
    {0xa985u, 2u, 2u, 0u},
    {0xa987u, 2u, 3u, 1u},
    {0xa989u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a98c[] = {
    {0xa98cu, 2u, 3u, 0u},
    {0xa98eu, 2u, 7u, 4u},
};

static const FixedSpanOpcode game_session_a992[] = {
    {0xa992u, 2u, 3u, 0u},
    {0xa994u, 2u, 2u, 0u},
    {0xa996u, 2u, 3u, 1u},
    {0xa998u, 3u, 3u, 0u},
    {0xa99bu, 2u, 4u, 2u},
    {0xa99du, 2u, 3u, 0u},
    {0xa99fu, 3u, 3u, 0u},
    {0xa9a2u, 2u, 4u, 2u},
    {0xa9a4u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a9a7[] = {
    {0xa9a7u, 2u, 4u, 2u},
    {0xa9a9u, 3u, 5u, 2u},
    {0xa9acu, 1u, 2u, 0u},
    {0xa9adu, 1u, 2u, 0u},
    {0xa9aeu, 2u, 4u, 2u},
    {0xa9b0u, 1u, 2u, 0u},
    {0xa9b1u, 2u, 4u, 2u},
    {0xa9b3u, 1u, 2u, 0u},
    {0xa9b4u, 3u, 3u, 0u},
    {0xa9b7u, 2u, 4u, 2u},
    {0xa9b9u, 3u, 5u, 2u},
    {0xa9bcu, 2u, 4u, 2u},
    {0xa9beu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a9c1[] = {
    {0xa9c1u, 3u, 3u, 0u},
    {0xa9c4u, 2u, 4u, 2u},
    {0xa9c6u, 2u, 4u, 2u},
    {0xa9c8u, 1u, 2u, 0u},
    {0xa9c9u, 1u, 2u, 0u},
    {0xa9cau, 1u, 2u, 0u},
    {0xa9cbu, 3u, 5u, 2u},
    {0xa9ceu, 2u, 4u, 2u},
    {0xa9d0u, 3u, 5u, 2u},
    {0xa9d3u, 1u, 2u, 0u},
    {0xa9d4u, 1u, 2u, 0u},
    {0xa9d5u, 2u, 4u, 2u},
    {0xa9d7u, 1u, 2u, 0u},
    {0xa9d8u, 2u, 4u, 2u},
    {0xa9dau, 1u, 2u, 0u},
    {0xa9dbu, 4u, 6u, 2u},
    {0xa9dfu, 2u, 4u, 2u},
    {0xa9e1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a9c6[] = {
    {0xa9c6u, 2u, 4u, 2u},
    {0xa9c8u, 1u, 2u, 0u},
    {0xa9c9u, 1u, 2u, 0u},
    {0xa9cau, 1u, 2u, 0u},
    {0xa9cbu, 3u, 5u, 2u},
    {0xa9ceu, 2u, 4u, 2u},
    {0xa9d0u, 3u, 5u, 2u},
    {0xa9d3u, 1u, 2u, 0u},
    {0xa9d4u, 1u, 2u, 0u},
    {0xa9d5u, 2u, 4u, 2u},
    {0xa9d7u, 1u, 2u, 0u},
    {0xa9d8u, 2u, 4u, 2u},
    {0xa9dau, 1u, 2u, 0u},
    {0xa9dbu, 4u, 6u, 2u},
    {0xa9dfu, 2u, 4u, 2u},
    {0xa9e1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a9e4[] = {
    {0xa9e4u, 2u, 7u, 4u},
    {0xa9e6u, 3u, 3u, 0u},
    {0xa9e9u, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_a9ed[] = {
    {0xa9edu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a9f0[] = {
    {0xa9f0u, 2u, 3u, 0u},
    {0xa9f2u, 2u, 2u, 0u},
    {0xa9f4u, 2u, 3u, 1u},
    {0xa9f6u, 2u, 2u, 0u},
    {0xa9f8u, 2u, 3u, 1u},
    {0xa9fau, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_a9fd[] = {
    {0xa9fdu, 2u, 3u, 0u},
    {0xa9ffu, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_aa04[] = {
    {0xaa04u, 3u, 3u, 0u},
    {0xaa07u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa0a[] = {
    {0xaa0au, 2u, 3u, 0u},
    {0xaa23u, 3u, 3u, 0u},
    {0xaa26u, 3u, 5u, 2u},
    {0xaa29u, 3u, 3u, 0u},
    {0xaa2cu, 3u, 5u, 2u},
    {0xaa2fu, 3u, 3u, 0u},
    {0xaa32u, 3u, 5u, 2u},
    {0xaa35u, 3u, 3u, 0u},
    {0xaa38u, 3u, 5u, 2u},
    {0xaa3bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa0c[] = {
    {0xaa0cu, 3u, 3u, 0u},
    {0xaa0fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa12[] = {
    {0xaa12u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa15[] = {
    {0xaa15u, 3u, 3u, 0u},
};

static const FixedSpanOpcode game_session_aa1a[] = {
    {0xaa1au, 3u, 3u, 0u},
    {0xaa1du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa20[] = {
    {0xaa20u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa23[] = {
    {0xaa23u, 3u, 3u, 0u},
    {0xaa26u, 3u, 5u, 2u},
    {0xaa29u, 3u, 3u, 0u},
    {0xaa2cu, 3u, 5u, 2u},
    {0xaa2fu, 3u, 3u, 0u},
    {0xaa32u, 3u, 5u, 2u},
    {0xaa35u, 3u, 3u, 0u},
    {0xaa38u, 3u, 5u, 2u},
    {0xaa3bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa3e[] = {
    {0xaa3eu, 3u, 3u, 0u},
    {0xaa41u, 3u, 3u, 0u},
    {0xaa44u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_aa48[] = {
    {0xaa48u, 3u, 3u, 0u},
    {0xaa4bu, 2u, 4u, 2u},
    {0xaa4du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa50[] = {
    {0xaa50u, 3u, 3u, 0u},
    {0xaa53u, 3u, 5u, 2u},
    {0xaa56u, 3u, 3u, 0u},
    {0xaa59u, 3u, 5u, 2u},
    {0xaa5cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa5f[] = {
    {0xaa5fu, 2u, 4u, 2u},
};

static const FixedSpanOpcode game_session_aa63[] = {
    {0xaa63u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa66[] = {
    {0xaa66u, 3u, 5u, 2u},
    {0xaa69u, 3u, 5u, 2u},
    {0xaa6cu, 2u, 3u, 0u},
};

static const FixedSpanOpcode game_session_aa70[] = {
    {0xaa70u, 2u, 2u, 0u},
    {0xaa72u, 3u, 4u, 1u},
    {0xaa75u, 2u, 2u, 0u},
    {0xaa77u, 3u, 4u, 1u},
    {0xaa7au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa7d[] = {
    {0xaa7du, 3u, 3u, 0u},
    {0xaa80u, 3u, 3u, 0u},
    {0xaa83u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_aa87[] = {
    {0xaa87u, 2u, 2u, 0u},
    {0xaa89u, 2u, 3u, 1u},
    {0xaa8bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa8e[] = {
    {0xaa8eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa91[] = {
    {0xaa91u, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_aa95[] = {
    {0xaa95u, 3u, 4u, 0u},
    {0xaa8eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa98[] = {
    {0xaa98u, 2u, 2u, 0u},
    {0xaa9au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa9d[] = {
    {0xaa9du, 2u, 3u, 0u},
    {0xaaafu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aa9f[] = {
    {0xaa9fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aaa2[] = {
    {0xaaa2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aaa5[] = {
    {0xaaa5u, 2u, 2u, 0u},
    {0xaaa7u, 2u, 3u, 1u},
    {0xaaa9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aaac[] = {
    {0xaaacu, 3u, 4u, 0u},
    {0xa71bu, 2u, 3u, 0u},
    {0xa71du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aaaf[] = {
    {0xaaafu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_aab2[] = {
    {0xaab2u, 1u, 4u, 1u},
    {0xaab3u, 1u, 4u, 1u},
};

static const FixedSpanOpcode game_session_c826[] = {
    {0xc826u, 2u, 3u, 0u},
    {0xc828u, 2u, 2u, 0u},
    {0xc82au, 3u, 4u, 1u},
    {0xc82du, 2u, 3u, 0u},
    {0xc82fu, 1u, 3u, 1u},
    {0xc830u, 3u, 3u, 0u},
    {0xc833u, 3u, 3u, 0u},
    {0xc836u, 3u, 3u, 0u},
    {0xc839u, 3u, 7u, 2u},
    {0xc83cu, 3u, 3u, 0u},
    {0xc83fu, 3u, 3u, 0u},
    {0xc842u, 3u, 3u, 0u},
    {0xc845u, 3u, 7u, 2u},
    {0xc848u, 1u, 4u, 1u},
    {0xc849u, 2u, 3u, 0u},
    {0xc84bu, 2u, 2u, 0u},
    {0xc84du, 2u, 3u, 1u},
    {0xc84fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c852[] = {
    {0xc852u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c855[] = {
    {0xc855u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c858[] = {
    {0xc858u, 2u, 3u, 0u},
    {0xc85au, 3u, 3u, 0u},
    {0xc85du, 2u, 4u, 2u},
    {0xc85fu, 3u, 3u, 0u},
    {0xc862u, 2u, 4u, 2u},
    {0xc864u, 3u, 3u, 0u},
    {0xc867u, 2u, 4u, 2u},
    {0xc869u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c86c[] = {
    {0xc86cu, 3u, 3u, 0u},
    {0xc86fu, 2u, 4u, 2u},
    {0xc871u, 3u, 3u, 0u},
    {0xc874u, 2u, 4u, 2u},
    {0xc876u, 3u, 3u, 0u},
    {0xc879u, 2u, 4u, 2u},
    {0xc87bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c87e[] = {
    {0xc87eu, 2u, 3u, 0u},
    {0xc880u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c883[] = {
    {0xc883u, 2u, 2u, 0u},
    {0xc885u, 3u, 4u, 1u},
    {0xc888u, 2u, 2u, 0u},
    {0xc88au, 2u, 3u, 1u},
    {0xc88cu, 3u, 3u, 0u},
    {0xc88fu, 3u, 3u, 0u},
    {0xc892u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_c896[] = {
    {0xc896u, 2u, 2u, 0u},
    {0xc898u, 2u, 3u, 1u},
    {0xc89au, 2u, 3u, 1u},
    {0xc89cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c89f[] = {
    {0xc89fu, 2u, 2u, 0u},
    {0xc8a1u, 2u, 3u, 1u},
    {0xc8a3u, 3u, 3u, 0u},
    {0xc8a6u, 3u, 5u, 2u},
    {0xc8a9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c8ac[] = {
    {0xc8acu, 1u, 3u, 1u},
    {0xc8adu, 2u, 2u, 0u},
    {0xc8afu, 1u, 3u, 1u},
    {0xc8b0u, 1u, 4u, 1u},
    {0xc8b1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c8b4[] = {
    {0xc8b4u, 3u, 3u, 0u},
    {0xc8b7u, 3u, 5u, 2u},
    {0xc8bau, 3u, 5u, 2u},
    {0xc8bdu, 3u, 3u, 0u},
    {0xc8c0u, 3u, 5u, 2u},
    {0xc8c3u, 3u, 3u, 0u},
    {0xc8c6u, 3u, 5u, 2u},
    {0xc8c9u, 3u, 5u, 2u},
    {0xc8ccu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c8cf[] = {
    {0xc8cfu, 2u, 3u, 0u},
    {0xc8d1u, 3u, 3u, 0u},
    {0xc8d4u, 3u, 5u, 2u},
    {0xc8d7u, 3u, 3u, 0u},
    {0xc8dau, 3u, 5u, 2u},
    {0xc8ddu, 3u, 3u, 0u},
    {0xc8e0u, 3u, 5u, 2u},
    {0xc8e3u, 3u, 3u, 0u},
    {0xc8e6u, 3u, 5u, 2u},
    {0xc8e9u, 3u, 3u, 0u},
    {0xc8ecu, 3u, 5u, 2u},
    {0xc8efu, 3u, 5u, 2u},
    {0xc8f2u, 3u, 5u, 2u},
    {0xc8f5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c8f8[] = {
    {0xc8f8u, 3u, 3u, 0u},
    {0xc8fbu, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_c8ff[] = {
    {0xc8ffu, 2u, 3u, 0u},
    {0xc901u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c904[] = {
    {0xc904u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c907[] = {
    {0xc907u, 2u, 2u, 0u},
    {0xc909u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c90c[] = {
    {0xc90cu, 2u, 2u, 0u},
    {0xc90eu, 2u, 3u, 1u},
    {0xc910u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c910[] = {
    {0xc910u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c913[] = {
    {0xc913u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c916[] = {
    {0xc916u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c919[] = {
    {0xc919u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c91c[] = {
    {0xc91cu, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_c920[] = {
    {0xc920u, 2u, 2u, 0u},
    {0xc922u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c925[] = {
    {0xc925u, 2u, 2u, 0u},
    {0xc927u, 2u, 3u, 1u},
    {0xc929u, 2u, 3u, 1u},
    {0xc92bu, 3u, 3u, 0u},
    {0xc92eu, 2u, 4u, 2u},
    {0xc930u, 2u, 2u, 0u},
    {0xc932u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c930[] = {
    {0xc930u, 2u, 2u, 0u},
    {0xc932u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c935[] = {
    {0xc935u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c938[] = {
    {0xc938u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c93b[] = {
    {0xc93bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c93e[] = {
    {0xc93eu, 2u, 5u, 2u},
};

static const FixedSpanOpcode game_session_c942[] = {
    {0xc942u, 2u, 3u, 1u},
    {0xc944u, 1u, 2u, 0u},
    {0xc945u, 2u, 2u, 0u},
    {0xc947u, 2u, 3u, 1u},
    {0xc949u, 2u, 3u, 1u},
    {0xc94bu, 2u, 5u, 2u},
};

static const FixedSpanOpcode game_session_c94f[] = {
    {0xc94fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c952[] = {
    {0xc952u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c955[] = {
    {0xc955u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c958[] = {
    {0xc958u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c95b[] = {
    {0xc95bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c95e[] = {
    {0xc95eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c961[] = {
    {0xc961u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c964[] = {
    {0xc964u, 2u, 2u, 0u},
    {0xc966u, 3u, 4u, 1u},
    {0xc969u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c96c[] = {
    {0xc96cu, 3u, 3u, 0u},
    {0xc96fu, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_c973[] = {
    {0xc973u, 2u, 2u, 0u},
    {0xc975u, 2u, 3u, 1u},
    {0xc977u, 3u, 4u, 1u},
};

static const FixedSpanOpcode game_session_c97c[] = {
    {0xc97cu, 3u, 4u, 0u},
    {0xc9f3u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c97f[] = {
    {0xc97fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c982[] = {
    {0xc982u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c985[] = {
    {0xc985u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c988[] = {
    {0xc988u, 2u, 3u, 0u},
    {0xc98au, 3u, 5u, 2u},
    {0xc98du, 1u, 2u, 0u},
    {0xc98eu, 3u, 5u, 2u},
    {0xc991u, 1u, 2u, 0u},
    {0xc992u, 3u, 5u, 2u},
    {0xc995u, 1u, 2u, 0u},
    {0xc996u, 2u, 3u, 0u},
};

static const FixedSpanOpcode game_session_c99a[] = {
    {0xc99au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c99d[] = {
    {0xc99du, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9a0[] = {
    {0xc9a0u, 2u, 3u, 1u},
};

static const FixedSpanOpcode game_session_c9a4[] = {
    {0xc9a4u, 2u, 2u, 0u},
    {0xc9a6u, 2u, 3u, 1u},
    {0xc9a8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9ab[] = {
    {0xc9abu, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_c9af[] = {
    {0xc9afu, 3u, 3u, 0u},
    {0xc9b2u, 3u, 5u, 2u},
    {0xc9b5u, 2u, 3u, 0u},
    {0xc99au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9b7[] = {
    {0xc9b7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9ba[] = {
    {0xc9bau, 2u, 3u, 0u},
    {0xc95bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9bc[] = {
    {0xc9bcu, 3u, 4u, 1u},
    {0xc9bfu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9c2[] = {
    {0xc9c2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9c5[] = {
    {0xc9c5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9c8[] = {
    {0xc9c8u, 2u, 3u, 1u},
};

static const FixedSpanOpcode game_session_c9cc[] = {
    {0xc9ccu, 2u, 2u, 0u},
    {0xc9ceu, 2u, 3u, 1u},
    {0xc9d0u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9d3[] = {
    {0xc9d3u, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_c9d7[] = {
    {0xc9d7u, 3u, 3u, 0u},
    {0xc9dau, 3u, 5u, 2u},
    {0xc9ddu, 1u, 3u, 0u},
    {0xc9deu, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_c9dd[] = {
    {0xc9ddu, 1u, 3u, 0u},
    {0xc9deu, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_c9e2[] = {
    {0xc9e2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9e5[] = {
    {0xc9e5u, 2u, 3u, 0u},
    {0xc9c2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9e7[] = {
    {0xc9e7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9ea[] = {
    {0xc9eau, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9ed[] = {
    {0xc9edu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9f0[] = {
    {0xc9f0u, 3u, 4u, 0u},
    {0xc95bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9f6[] = {
    {0xc9f6u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_c9f9[] = {
    {0xc9f9u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_c9fe[] = {
    {0xc9feu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca01[] = {
    {0xca01u, 2u, 2u, 0u},
    {0xca03u, 3u, 4u, 1u},
    {0xca06u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca09[] = {
    {0xca09u, 2u, 2u, 0u},
    {0xca0bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca0e[] = {
    {0xca0eu, 3u, 4u, 1u},
    {0xca11u, 2u, 3u, 1u},
    {0xca13u, 3u, 3u, 0u},
    {0xca16u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_ca1a[] = {
    {0xca1au, 2u, 2u, 0u},
    {0xca1cu, 2u, 3u, 1u},
    {0xca1eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca1e[] = {
    {0xca1eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca21[] = {
    {0xca21u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca24[] = {
    {0xca24u, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_ca28[] = {
    {0xca28u, 2u, 2u, 0u},
    {0xca2au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca2d[] = {
    {0xca2du, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_ca32[] = {
    {0xca32u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca35[] = {
    {0xca35u, 2u, 2u, 0u},
    {0xca37u, 3u, 4u, 1u},
    {0xca3au, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca3d[] = {
    {0xca3du, 3u, 3u, 0u},
    {0xca40u, 3u, 3u, 0u},
    {0xca43u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_ca47[] = {
    {0xca47u, 2u, 2u, 0u},
    {0xca49u, 2u, 3u, 1u},
    {0xca4bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca4e[] = {
    {0xca4eu, 2u, 3u, 1u},
};

static const FixedSpanOpcode game_session_ca52[] = {
    {0xca52u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca55[] = {
    {0xca55u, 3u, 5u, 2u},
};

static const FixedSpanOpcode game_session_ca5a[] = {
    {0xca5au, 2u, 3u, 0u},
    {0xca5cu, 1u, 2u, 0u},
    {0xca5du, 3u, 3u, 0u},
    {0xca60u, 3u, 5u, 2u},
    {0xca63u, 2u, 3u, 0u},
    {0xca65u, 2u, 3u, 0u},
    {0xca8eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca67[] = {
    {0xca67u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca6a[] = {
    {0xca6au, 2u, 2u, 0u},
    {0xca6cu, 3u, 4u, 1u},
    {0xca6fu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca72[] = {
    {0xca72u, 2u, 2u, 0u},
    {0xca74u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca77[] = {
    {0xca77u, 3u, 3u, 0u},
    {0xca7au, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_ca7e[] = {
    {0xca7eu, 2u, 2u, 0u},
    {0xca80u, 2u, 3u, 1u},
    {0xca82u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca82[] = {
    {0xca82u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca85[] = {
    {0xca85u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca88[] = {
    {0xca88u, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_ca8c[] = {
    {0xca8cu, 2u, 3u, 0u},
    {0xca93u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca8e[] = {
    {0xca8eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca91[] = {
    {0xca91u, 2u, 3u, 0u},
    {0xca82u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca96[] = {
    {0xca96u, 2u, 2u, 0u},
    {0xca98u, 3u, 4u, 1u},
    {0xca9bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_ca9e[] = {
    {0xca9eu, 3u, 3u, 0u},
    {0xcaa1u, 3u, 3u, 0u},
    {0xcaa4u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_caa8[] = {
    {0xcaa8u, 2u, 2u, 0u},
    {0xcaaau, 2u, 3u, 1u},
    {0xcaacu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_caaf[] = {
    {0xcaafu, 2u, 3u, 1u},
};

static const FixedSpanOpcode game_session_cab3[] = {
    {0xcab3u, 2u, 3u, 0u},
    {0xcab5u, 3u, 5u, 2u},
    {0xcab8u, 3u, 5u, 2u},
    {0xcabbu, 2u, 3u, 0u},
};

static const FixedSpanOpcode game_session_cabf[] = {
    {0xcabfu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_cac2[] = {
    {0xcac2u, 2u, 2u, 0u},
    {0xcac4u, 3u, 4u, 1u},
    {0xcac7u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_caca[] = {
    {0xcacau, 3u, 3u, 0u},
    {0xcacdu, 3u, 3u, 0u},
    {0xcad0u, 4u, 8u, 3u},
};

static const FixedSpanOpcode game_session_cad4[] = {
    {0xcad4u, 2u, 2u, 0u},
    {0xcad6u, 2u, 3u, 1u},
    {0xcad8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_cad8[] = {
    {0xcad8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_cadb[] = {
    {0xcadbu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_cade[] = {
    {0xcadeu, 2u, 2u, 0u},
};

static const FixedSpanOpcode game_session_cae2[] = {
    {0xcae2u, 2u, 2u, 0u},
    {0xcae4u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_cae7[] = {
    {0xcae7u, 2u, 3u, 0u},
    {0xcaefu, 1u, 4u, 1u},
};

static const FixedSpanOpcode game_session_cae9[] = {
    {0xcae9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_caec[] = {
    {0xcaecu, 3u, 4u, 0u},
    {0xc8ccu, 3u, 6u, 2u},
};

static const FixedSpanOpcode game_session_caef[] = {
    {0xcaefu, 1u, 4u, 1u},
};

static const GameSessionPhase game_session_phases[] = {
    {0xa705u, 0x0000u, 0x0000u, 1u, sizeof game_session_a705 / sizeof game_session_a705[0], game_session_a705},
    {0xa710u, 0x0000u, 0x0000u, 1u, sizeof game_session_a710 / sizeof game_session_a710[0], game_session_a710},
    {0xa720u, 0x0000u, 0x0000u, 0u, sizeof game_session_a720 / sizeof game_session_a720[0], game_session_a720},
    {0xa72cu, 0x0000u, 0x0000u, 0u, sizeof game_session_a72c / sizeof game_session_a72c[0], game_session_a72c},
    {0xa72fu, 0x0000u, 0x0000u, 0u, sizeof game_session_a72f / sizeof game_session_a72f[0], game_session_a72f},
    {0xa734u, 0x0000u, 0x0000u, 1u, sizeof game_session_a734 / sizeof game_session_a734[0], game_session_a734},
    {0xa73bu, 0x0000u, 0x0000u, 1u, sizeof game_session_a73b / sizeof game_session_a73b[0], game_session_a73b},
    {0xa742u, 0x0000u, 0x0000u, 0u, sizeof game_session_a742 / sizeof game_session_a742[0], game_session_a742},
    {0xa747u, 0x0000u, 0x0000u, 0u, sizeof game_session_a747 / sizeof game_session_a747[0], game_session_a747},
    {0xa74cu, 0x0000u, 0x0000u, 0u, sizeof game_session_a74c / sizeof game_session_a74c[0], game_session_a74c},
    {0xa751u, 0x0000u, 0x0000u, 0u, sizeof game_session_a751 / sizeof game_session_a751[0], game_session_a751},
    {0xa754u, 0x0000u, 0x0000u, 0u, sizeof game_session_a754 / sizeof game_session_a754[0], game_session_a754},
    {0xa75eu, 0xa763u, 0x0000u, 0u, sizeof game_session_a75e / sizeof game_session_a75e[0], game_session_a75e},
    {0xa765u, 0x0000u, 0x0000u, 0u, sizeof game_session_a765 / sizeof game_session_a765[0], game_session_a765},
    {0xa768u, 0x0000u, 0x0000u, 0u, sizeof game_session_a768 / sizeof game_session_a768[0], game_session_a768},
    {0xa77eu, 0x0000u, 0x0000u, 0u, sizeof game_session_a77e / sizeof game_session_a77e[0], game_session_a77e},
    {0xa781u, 0xa784u, 0x0000u, 0u, sizeof game_session_a781 / sizeof game_session_a781[0], game_session_a781},
    {0xa786u, 0x0000u, 0x0000u, 0u, sizeof game_session_a786 / sizeof game_session_a786[0], game_session_a786},
    {0xa78cu, 0x0000u, 0x0000u, 0u, sizeof game_session_a78c / sizeof game_session_a78c[0], game_session_a78c},
    {0xa78fu, 0x0000u, 0x0000u, 0u, sizeof game_session_a78f / sizeof game_session_a78f[0], game_session_a78f},
    {0xa792u, 0x0000u, 0x0000u, 0u, sizeof game_session_a792 / sizeof game_session_a792[0], game_session_a792},
    {0xa795u, 0xa79bu, 0x0000u, 0u, sizeof game_session_a795 / sizeof game_session_a795[0], game_session_a795},
    {0xa79du, 0x0000u, 0x0000u, 0u, sizeof game_session_a79d / sizeof game_session_a79d[0], game_session_a79d},
    {0xa7a2u, 0xa7a7u, 0x0000u, 0u, sizeof game_session_a7a2 / sizeof game_session_a7a2[0], game_session_a7a2},
    {0xa7a9u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7a9 / sizeof game_session_a7a9[0], game_session_a7a9},
    {0xa7b6u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7b6 / sizeof game_session_a7b6[0], game_session_a7b6},
    {0xa7b8u, 0xa7bbu, 0x0000u, 0u, sizeof game_session_a7b8 / sizeof game_session_a7b8[0], game_session_a7b8},
    {0xa7bdu, 0x0000u, 0x0000u, 0u, sizeof game_session_a7bd / sizeof game_session_a7bd[0], game_session_a7bd},
    {0xa7c3u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7c3 / sizeof game_session_a7c3[0], game_session_a7c3},
    {0xa7c6u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7c6 / sizeof game_session_a7c6[0], game_session_a7c6},
    {0xa7c8u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7c8 / sizeof game_session_a7c8[0], game_session_a7c8},
    {0xa7cfu, 0xa7d4u, 0x0000u, 0u, sizeof game_session_a7cf / sizeof game_session_a7cf[0], game_session_a7cf},
    {0xa7d6u, 0xa7e1u, 0x0000u, 0u, sizeof game_session_a7d6 / sizeof game_session_a7d6[0], game_session_a7d6},
    {0xa7e3u, 0xa7f0u, 0x0000u, 0u, sizeof game_session_a7e3 / sizeof game_session_a7e3[0], game_session_a7e3},
    {0xa7e6u, 0xa7f0u, 0x0000u, 0u, sizeof game_session_a7e6 / sizeof game_session_a7e6[0], game_session_a7e6},
    {0xa7f2u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7f2 / sizeof game_session_a7f2[0], game_session_a7f2},
    {0xa7f4u, 0x0000u, 0x0000u, 0u, sizeof game_session_a7f4 / sizeof game_session_a7f4[0], game_session_a7f4},
    {0xa7fbu, 0xa7feu, 0x0000u, 0u, sizeof game_session_a7fb / sizeof game_session_a7fb[0], game_session_a7fb},
    {0xa800u, 0x0000u, 0x0000u, 0u, sizeof game_session_a800 / sizeof game_session_a800[0], game_session_a800},
    {0xa803u, 0xa80eu, 0x0000u, 0u, sizeof game_session_a803 / sizeof game_session_a803[0], game_session_a803},
    {0xa810u, 0xa817u, 0x0000u, 0u, sizeof game_session_a810 / sizeof game_session_a810[0], game_session_a810},
    {0xa819u, 0x0000u, 0x0000u, 0u, sizeof game_session_a819 / sizeof game_session_a819[0], game_session_a819},
    {0xa820u, 0xa825u, 0x0000u, 0u, sizeof game_session_a820 / sizeof game_session_a820[0], game_session_a820},
    {0xa827u, 0x0000u, 0x0000u, 0u, sizeof game_session_a827 / sizeof game_session_a827[0], game_session_a827},
    {0xa82eu, 0xa831u, 0x0000u, 0u, sizeof game_session_a82e / sizeof game_session_a82e[0], game_session_a82e},
    {0xa833u, 0xa839u, 0x0000u, 0u, sizeof game_session_a833 / sizeof game_session_a833[0], game_session_a833},
    {0xa83bu, 0x0000u, 0x0000u, 0u, sizeof game_session_a83b / sizeof game_session_a83b[0], game_session_a83b},
    {0xa83du, 0xa840u, 0x0000u, 0u, sizeof game_session_a83d / sizeof game_session_a83d[0], game_session_a83d},
    {0xa842u, 0xa84du, 0x0000u, 0u, sizeof game_session_a842 / sizeof game_session_a842[0], game_session_a842},
    {0xa84fu, 0xa85cu, 0x0000u, 0u, sizeof game_session_a84f / sizeof game_session_a84f[0], game_session_a84f},
    {0xa852u, 0xa85cu, 0x0000u, 0u, sizeof game_session_a852 / sizeof game_session_a852[0], game_session_a852},
    {0xa85eu, 0xa863u, 0x0000u, 0u, sizeof game_session_a85e / sizeof game_session_a85e[0], game_session_a85e},
    {0xa865u, 0xa86bu, 0x0000u, 0u, sizeof game_session_a865 / sizeof game_session_a865[0], game_session_a865},
    {0xa86du, 0xa876u, 0x0000u, 0u, sizeof game_session_a86d / sizeof game_session_a86d[0], game_session_a86d},
    {0xa878u, 0xa84du, 0x0000u, 0u, sizeof game_session_a878 / sizeof game_session_a878[0], game_session_a878},
    {0xa87au, 0x0000u, 0x0000u, 0u, sizeof game_session_a87a / sizeof game_session_a87a[0], game_session_a87a},
    {0xa87du, 0xa888u, 0x0000u, 0u, sizeof game_session_a87d / sizeof game_session_a87d[0], game_session_a87d},
    {0xa88au, 0x0000u, 0x0000u, 0u, sizeof game_session_a88a / sizeof game_session_a88a[0], game_session_a88a},
    {0xa891u, 0x0000u, 0x0000u, 0u, sizeof game_session_a891 / sizeof game_session_a891[0], game_session_a891},
    {0xa894u, 0xa8a2u, 0x0000u, 0u, sizeof game_session_a894 / sizeof game_session_a894[0], game_session_a894},
    {0xa8a4u, 0x0000u, 0x0000u, 0u, sizeof game_session_a8a4 / sizeof game_session_a8a4[0], game_session_a8a4},
    {0xa8acu, 0x0000u, 0x0000u, 0u, sizeof game_session_a8ac / sizeof game_session_a8ac[0], game_session_a8ac},
    {0xa8b8u, 0x0000u, 0x0000u, 0u, sizeof game_session_a8b8 / sizeof game_session_a8b8[0], game_session_a8b8},
    {0xa8bbu, 0xa8c9u, 0x0000u, 0u, sizeof game_session_a8bb / sizeof game_session_a8bb[0], game_session_a8bb},
    {0xa8cbu, 0xa8d7u, 0x0000u, 0u, sizeof game_session_a8cb / sizeof game_session_a8cb[0], game_session_a8cb},
    {0xa8d1u, 0xa8d7u, 0x0000u, 0u, sizeof game_session_a8d1 / sizeof game_session_a8d1[0], game_session_a8d1},
    {0xa8d9u, 0xa8dfu, 0x0000u, 0u, sizeof game_session_a8d9 / sizeof game_session_a8d9[0], game_session_a8d9},
    {0xa8e1u, 0xa8e7u, 0x0000u, 0u, sizeof game_session_a8e1 / sizeof game_session_a8e1[0], game_session_a8e1},
    {0xa8e4u, 0xa8e7u, 0x0000u, 0u, sizeof game_session_a8e4 / sizeof game_session_a8e4[0], game_session_a8e4},
    {0xa8e9u, 0xa8efu, 0x0000u, 0u, sizeof game_session_a8e9 / sizeof game_session_a8e9[0], game_session_a8e9},
    {0xa8ecu, 0xa8efu, 0x0000u, 0u, sizeof game_session_a8ec / sizeof game_session_a8ec[0], game_session_a8ec},
    {0xa8f1u, 0xa8f7u, 0x0000u, 0u, sizeof game_session_a8f1 / sizeof game_session_a8f1[0], game_session_a8f1},
    {0xa8f4u, 0xa8f7u, 0x0000u, 0u, sizeof game_session_a8f4 / sizeof game_session_a8f4[0], game_session_a8f4},
    {0xa8f9u, 0x0000u, 0x0000u, 0u, sizeof game_session_a8f9 / sizeof game_session_a8f9[0], game_session_a8f9},
    {0xa901u, 0xa90bu, 0x0000u, 0u, sizeof game_session_a901 / sizeof game_session_a901[0], game_session_a901},
    {0xa90du, 0x0000u, 0x0000u, 0u, sizeof game_session_a90d / sizeof game_session_a90d[0], game_session_a90d},
    {0xa910u, 0x0000u, 0x0000u, 0u, sizeof game_session_a910 / sizeof game_session_a910[0], game_session_a910},
    {0xa91au, 0x0000u, 0x0000u, 0u, sizeof game_session_a91a / sizeof game_session_a91a[0], game_session_a91a},
    {0xa91du, 0xa927u, 0x0000u, 0u, sizeof game_session_a91d / sizeof game_session_a91d[0], game_session_a91d},
    {0xa929u, 0xa92eu, 0x0000u, 0u, sizeof game_session_a929 / sizeof game_session_a929[0], game_session_a929},
    {0xa930u, 0xa937u, 0x0000u, 0u, sizeof game_session_a930 / sizeof game_session_a930[0], game_session_a930},
    {0xa933u, 0xa937u, 0x0000u, 0u, sizeof game_session_a933 / sizeof game_session_a933[0], game_session_a933},
    {0xa939u, 0x0000u, 0x0000u, 0u, sizeof game_session_a939 / sizeof game_session_a939[0], game_session_a939},
    {0xa93eu, 0x0000u, 0x0000u, 0u, sizeof game_session_a93e / sizeof game_session_a93e[0], game_session_a93e},
    {0xa95bu, 0x0000u, 0x0000u, 0u, sizeof game_session_a95b / sizeof game_session_a95b[0], game_session_a95b},
    {0xa968u, 0x0000u, 0x0000u, 0u, sizeof game_session_a968 / sizeof game_session_a968[0], game_session_a968},
    {0xa96eu, 0x0000u, 0x0000u, 0u, sizeof game_session_a96e / sizeof game_session_a96e[0], game_session_a96e},
    {0xa971u, 0xa981u, 0x0000u, 0u, sizeof game_session_a971 / sizeof game_session_a971[0], game_session_a971},
    {0xa983u, 0x0000u, 0x0000u, 0u, sizeof game_session_a983 / sizeof game_session_a983[0], game_session_a983},
    {0xa98cu, 0xa990u, 0x0000u, 1u, sizeof game_session_a98c / sizeof game_session_a98c[0], game_session_a98c},
    {0xa992u, 0x0000u, 0x0000u, 0u, sizeof game_session_a992 / sizeof game_session_a992[0], game_session_a992},
    {0xa9a7u, 0x0000u, 0x0000u, 0u, sizeof game_session_a9a7 / sizeof game_session_a9a7[0], game_session_a9a7},
    {0xa9c1u, 0x0000u, 0x0000u, 0u, sizeof game_session_a9c1 / sizeof game_session_a9c1[0], game_session_a9c1},
    {0xa9c6u, 0x0000u, 0x0000u, 0u, sizeof game_session_a9c6 / sizeof game_session_a9c6[0], game_session_a9c6},
    {0xa9e4u, 0xa9ebu, 0x0000u, 0u, sizeof game_session_a9e4 / sizeof game_session_a9e4[0], game_session_a9e4},
    {0xa9edu, 0x0000u, 0x0000u, 0u, sizeof game_session_a9ed / sizeof game_session_a9ed[0], game_session_a9ed},
    {0xa9f0u, 0x0000u, 0x0000u, 0u, sizeof game_session_a9f0 / sizeof game_session_a9f0[0], game_session_a9f0},
    {0xa9fdu, 0xaa02u, 0x0000u, 1u, sizeof game_session_a9fd / sizeof game_session_a9fd[0], game_session_a9fd},
    {0xaa04u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa04 / sizeof game_session_aa04[0], game_session_aa04},
    {0xaa0au, 0x0000u, 0x0000u, 0u, sizeof game_session_aa0a / sizeof game_session_aa0a[0], game_session_aa0a},
    {0xaa0cu, 0x0000u, 0x0000u, 0u, sizeof game_session_aa0c / sizeof game_session_aa0c[0], game_session_aa0c},
    {0xaa12u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa12 / sizeof game_session_aa12[0], game_session_aa12},
    {0xaa15u, 0xaa18u, 0x0000u, 0u, sizeof game_session_aa15 / sizeof game_session_aa15[0], game_session_aa15},
    {0xaa1au, 0x0000u, 0x0000u, 0u, sizeof game_session_aa1a / sizeof game_session_aa1a[0], game_session_aa1a},
    {0xaa20u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa20 / sizeof game_session_aa20[0], game_session_aa20},
    {0xaa23u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa23 / sizeof game_session_aa23[0], game_session_aa23},
    {0xaa3eu, 0x0000u, 0x0000u, 0u, sizeof game_session_aa3e / sizeof game_session_aa3e[0], game_session_aa3e},
    {0xaa48u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa48 / sizeof game_session_aa48[0], game_session_aa48},
    {0xaa50u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa50 / sizeof game_session_aa50[0], game_session_aa50},
    {0xaa5fu, 0xaa61u, 0x0000u, 0u, sizeof game_session_aa5f / sizeof game_session_aa5f[0], game_session_aa5f},
    {0xaa63u, 0x0000u, 0x0000u, 0u, sizeof game_session_aa63 / sizeof game_session_aa63[0], game_session_aa63},
    {0xaa66u, 0xaa6eu, 0x0000u, 0u, sizeof game_session_aa66 / sizeof game_session_aa66[0], game_session_aa66},
    {0xaa70u, 0x0000u, 0x0000u, 1u, sizeof game_session_aa70 / sizeof game_session_aa70[0], game_session_aa70},
    {0xaa7du, 0x0000u, 0x0000u, 1u, sizeof game_session_aa7d / sizeof game_session_aa7d[0], game_session_aa7d},
    {0xaa87u, 0x0000u, 0x0000u, 1u, sizeof game_session_aa87 / sizeof game_session_aa87[0], game_session_aa87},
    {0xaa8eu, 0x0000u, 0x0000u, 1u, sizeof game_session_aa8e / sizeof game_session_aa8e[0], game_session_aa8e},
    {0xaa91u, 0xaa93u, 0x0000u, 1u, sizeof game_session_aa91 / sizeof game_session_aa91[0], game_session_aa91},
    {0xaa95u, 0x0000u, 0x0000u, 1u, sizeof game_session_aa95 / sizeof game_session_aa95[0], game_session_aa95},
    {0xaa98u, 0x0000u, 0x0000u, 1u, sizeof game_session_aa98 / sizeof game_session_aa98[0], game_session_aa98},
    {0xaa9du, 0x0000u, 0x0000u, 1u, sizeof game_session_aa9d / sizeof game_session_aa9d[0], game_session_aa9d},
    {0xaa9fu, 0x0000u, 0x0000u, 1u, sizeof game_session_aa9f / sizeof game_session_aa9f[0], game_session_aa9f},
    {0xaaa2u, 0x0000u, 0x0000u, 1u, sizeof game_session_aaa2 / sizeof game_session_aaa2[0], game_session_aaa2},
    {0xaaa5u, 0x0000u, 0x0000u, 1u, sizeof game_session_aaa5 / sizeof game_session_aaa5[0], game_session_aaa5},
    {0xaaacu, 0x0000u, 0x0000u, 1u, sizeof game_session_aaac / sizeof game_session_aaac[0], game_session_aaac},
    {0xaaafu, 0x0000u, 0x0000u, 1u, sizeof game_session_aaaf / sizeof game_session_aaaf[0], game_session_aaaf},
    {0xaab2u, 0x0000u, 0xaab4u, 1u, sizeof game_session_aab2 / sizeof game_session_aab2[0], game_session_aab2},
    {0xc826u, 0x0000u, 0x0000u, 1u, sizeof game_session_c826 / sizeof game_session_c826[0], game_session_c826},
    {0xc852u, 0x0000u, 0x0000u, 1u, sizeof game_session_c852 / sizeof game_session_c852[0], game_session_c852},
    {0xc855u, 0x0000u, 0x0000u, 1u, sizeof game_session_c855 / sizeof game_session_c855[0], game_session_c855},
    {0xc858u, 0x0000u, 0x0000u, 1u, sizeof game_session_c858 / sizeof game_session_c858[0], game_session_c858},
    {0xc86cu, 0x0000u, 0x0000u, 0u, sizeof game_session_c86c / sizeof game_session_c86c[0], game_session_c86c},
    {0xc87eu, 0x0000u, 0x0000u, 0u, sizeof game_session_c87e / sizeof game_session_c87e[0], game_session_c87e},
    {0xc883u, 0x0000u, 0x0000u, 1u, sizeof game_session_c883 / sizeof game_session_c883[0], game_session_c883},
    {0xc896u, 0x0000u, 0x0000u, 1u, sizeof game_session_c896 / sizeof game_session_c896[0], game_session_c896},
    {0xc89fu, 0x0000u, 0x0000u, 1u, sizeof game_session_c89f / sizeof game_session_c89f[0], game_session_c89f},
    {0xc8acu, 0x0000u, 0x0000u, 1u, sizeof game_session_c8ac / sizeof game_session_c8ac[0], game_session_c8ac},
    {0xc8b4u, 0x0000u, 0x0000u, 1u, sizeof game_session_c8b4 / sizeof game_session_c8b4[0], game_session_c8b4},
    {0xc8cfu, 0x0000u, 0x0000u, 1u, sizeof game_session_c8cf / sizeof game_session_c8cf[0], game_session_c8cf},
    {0xc8f8u, 0x0000u, 0x0000u, 0u, sizeof game_session_c8f8 / sizeof game_session_c8f8[0], game_session_c8f8},
    {0xc8ffu, 0x0000u, 0x0000u, 0u, sizeof game_session_c8ff / sizeof game_session_c8ff[0], game_session_c8ff},
    {0xc904u, 0x0000u, 0x0000u, 1u, sizeof game_session_c904 / sizeof game_session_c904[0], game_session_c904},
    {0xc907u, 0x0000u, 0x0000u, 1u, sizeof game_session_c907 / sizeof game_session_c907[0], game_session_c907},
    {0xc90cu, 0x0000u, 0x0000u, 1u, sizeof game_session_c90c / sizeof game_session_c90c[0], game_session_c90c},
    {0xc910u, 0x0000u, 0x0000u, 1u, sizeof game_session_c910 / sizeof game_session_c910[0], game_session_c910},
    {0xc913u, 0x0000u, 0x0000u, 1u, sizeof game_session_c913 / sizeof game_session_c913[0], game_session_c913},
    {0xc916u, 0x0000u, 0x0000u, 1u, sizeof game_session_c916 / sizeof game_session_c916[0], game_session_c916},
    {0xc919u, 0x0000u, 0x0000u, 1u, sizeof game_session_c919 / sizeof game_session_c919[0], game_session_c919},
    {0xc91cu, 0xc91eu, 0x0000u, 1u, sizeof game_session_c91c / sizeof game_session_c91c[0], game_session_c91c},
    {0xc920u, 0x0000u, 0x0000u, 1u, sizeof game_session_c920 / sizeof game_session_c920[0], game_session_c920},
    {0xc925u, 0x0000u, 0x0000u, 1u, sizeof game_session_c925 / sizeof game_session_c925[0], game_session_c925},
    {0xc930u, 0x0000u, 0x0000u, 1u, sizeof game_session_c930 / sizeof game_session_c930[0], game_session_c930},
    {0xc935u, 0x0000u, 0x0000u, 1u, sizeof game_session_c935 / sizeof game_session_c935[0], game_session_c935},
    {0xc938u, 0x0000u, 0x0000u, 1u, sizeof game_session_c938 / sizeof game_session_c938[0], game_session_c938},
    {0xc93bu, 0x0000u, 0x0000u, 1u, sizeof game_session_c93b / sizeof game_session_c93b[0], game_session_c93b},
    {0xc93eu, 0xc940u, 0x0000u, 1u, sizeof game_session_c93e / sizeof game_session_c93e[0], game_session_c93e},
    {0xc942u, 0xc94du, 0x0000u, 1u, sizeof game_session_c942 / sizeof game_session_c942[0], game_session_c942},
    {0xc94fu, 0x0000u, 0x0000u, 1u, sizeof game_session_c94f / sizeof game_session_c94f[0], game_session_c94f},
    {0xc952u, 0x0000u, 0x0000u, 1u, sizeof game_session_c952 / sizeof game_session_c952[0], game_session_c952},
    {0xc955u, 0x0000u, 0x0000u, 1u, sizeof game_session_c955 / sizeof game_session_c955[0], game_session_c955},
    {0xc958u, 0x0000u, 0x0000u, 1u, sizeof game_session_c958 / sizeof game_session_c958[0], game_session_c958},
    {0xc95bu, 0x0000u, 0x0000u, 1u, sizeof game_session_c95b / sizeof game_session_c95b[0], game_session_c95b},
    {0xc95eu, 0x0000u, 0x0000u, 1u, sizeof game_session_c95e / sizeof game_session_c95e[0], game_session_c95e},
    {0xc961u, 0x0000u, 0x0000u, 1u, sizeof game_session_c961 / sizeof game_session_c961[0], game_session_c961},
    {0xc964u, 0x0000u, 0x0000u, 1u, sizeof game_session_c964 / sizeof game_session_c964[0], game_session_c964},
    {0xc96cu, 0x0000u, 0x0000u, 1u, sizeof game_session_c96c / sizeof game_session_c96c[0], game_session_c96c},
    {0xc973u, 0xc97au, 0x0000u, 1u, sizeof game_session_c973 / sizeof game_session_c973[0], game_session_c973},
    {0xc97cu, 0x0000u, 0x0000u, 1u, sizeof game_session_c97c / sizeof game_session_c97c[0], game_session_c97c},
    {0xc97fu, 0x0000u, 0x0000u, 1u, sizeof game_session_c97f / sizeof game_session_c97f[0], game_session_c97f},
    {0xc982u, 0x0000u, 0x0000u, 1u, sizeof game_session_c982 / sizeof game_session_c982[0], game_session_c982},
    {0xc985u, 0x0000u, 0x0000u, 1u, sizeof game_session_c985 / sizeof game_session_c985[0], game_session_c985},
    {0xc988u, 0xc998u, 0x0000u, 1u, sizeof game_session_c988 / sizeof game_session_c988[0], game_session_c988},
    {0xc99au, 0x0000u, 0x0000u, 1u, sizeof game_session_c99a / sizeof game_session_c99a[0], game_session_c99a},
    {0xc99du, 0x0000u, 0x0000u, 1u, sizeof game_session_c99d / sizeof game_session_c99d[0], game_session_c99d},
    {0xc9a0u, 0xc9a2u, 0x0000u, 1u, sizeof game_session_c9a0 / sizeof game_session_c9a0[0], game_session_c9a0},
    {0xc9a4u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9a4 / sizeof game_session_c9a4[0], game_session_c9a4},
    {0xc9abu, 0xc9adu, 0x0000u, 1u, sizeof game_session_c9ab / sizeof game_session_c9ab[0], game_session_c9ab},
    {0xc9afu, 0x0000u, 0x0000u, 1u, sizeof game_session_c9af / sizeof game_session_c9af[0], game_session_c9af},
    {0xc9b7u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9b7 / sizeof game_session_c9b7[0], game_session_c9b7},
    {0xc9bau, 0x0000u, 0x0000u, 1u, sizeof game_session_c9ba / sizeof game_session_c9ba[0], game_session_c9ba},
    {0xc9bcu, 0x0000u, 0x0000u, 1u, sizeof game_session_c9bc / sizeof game_session_c9bc[0], game_session_c9bc},
    {0xc9c2u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9c2 / sizeof game_session_c9c2[0], game_session_c9c2},
    {0xc9c5u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9c5 / sizeof game_session_c9c5[0], game_session_c9c5},
    {0xc9c8u, 0xc9cau, 0x0000u, 1u, sizeof game_session_c9c8 / sizeof game_session_c9c8[0], game_session_c9c8},
    {0xc9ccu, 0x0000u, 0x0000u, 1u, sizeof game_session_c9cc / sizeof game_session_c9cc[0], game_session_c9cc},
    {0xc9d3u, 0xc9d5u, 0x0000u, 1u, sizeof game_session_c9d3 / sizeof game_session_c9d3[0], game_session_c9d3},
    {0xc9d7u, 0xc9e0u, 0x0000u, 1u, sizeof game_session_c9d7 / sizeof game_session_c9d7[0], game_session_c9d7},
    {0xc9ddu, 0xc9e0u, 0x0000u, 1u, sizeof game_session_c9dd / sizeof game_session_c9dd[0], game_session_c9dd},
    {0xc9e2u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9e2 / sizeof game_session_c9e2[0], game_session_c9e2},
    {0xc9e5u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9e5 / sizeof game_session_c9e5[0], game_session_c9e5},
    {0xc9e7u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9e7 / sizeof game_session_c9e7[0], game_session_c9e7},
    {0xc9eau, 0x0000u, 0x0000u, 1u, sizeof game_session_c9ea / sizeof game_session_c9ea[0], game_session_c9ea},
    {0xc9edu, 0x0000u, 0x0000u, 1u, sizeof game_session_c9ed / sizeof game_session_c9ed[0], game_session_c9ed},
    {0xc9f0u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9f0 / sizeof game_session_c9f0[0], game_session_c9f0},
    {0xc9f6u, 0x0000u, 0x0000u, 1u, sizeof game_session_c9f6 / sizeof game_session_c9f6[0], game_session_c9f6},
    {0xc9f9u, 0xc9fcu, 0x0000u, 1u, sizeof game_session_c9f9 / sizeof game_session_c9f9[0], game_session_c9f9},
    {0xc9feu, 0x0000u, 0x0000u, 1u, sizeof game_session_c9fe / sizeof game_session_c9fe[0], game_session_c9fe},
    {0xca01u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca01 / sizeof game_session_ca01[0], game_session_ca01},
    {0xca09u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca09 / sizeof game_session_ca09[0], game_session_ca09},
    {0xca0eu, 0x0000u, 0x0000u, 1u, sizeof game_session_ca0e / sizeof game_session_ca0e[0], game_session_ca0e},
    {0xca1au, 0x0000u, 0x0000u, 1u, sizeof game_session_ca1a / sizeof game_session_ca1a[0], game_session_ca1a},
    {0xca1eu, 0x0000u, 0x0000u, 1u, sizeof game_session_ca1e / sizeof game_session_ca1e[0], game_session_ca1e},
    {0xca21u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca21 / sizeof game_session_ca21[0], game_session_ca21},
    {0xca24u, 0xca26u, 0x0000u, 1u, sizeof game_session_ca24 / sizeof game_session_ca24[0], game_session_ca24},
    {0xca28u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca28 / sizeof game_session_ca28[0], game_session_ca28},
    {0xca2du, 0xca30u, 0x0000u, 1u, sizeof game_session_ca2d / sizeof game_session_ca2d[0], game_session_ca2d},
    {0xca32u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca32 / sizeof game_session_ca32[0], game_session_ca32},
    {0xca35u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca35 / sizeof game_session_ca35[0], game_session_ca35},
    {0xca3du, 0x0000u, 0x0000u, 1u, sizeof game_session_ca3d / sizeof game_session_ca3d[0], game_session_ca3d},
    {0xca47u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca47 / sizeof game_session_ca47[0], game_session_ca47},
    {0xca4eu, 0xca50u, 0x0000u, 1u, sizeof game_session_ca4e / sizeof game_session_ca4e[0], game_session_ca4e},
    {0xca52u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca52 / sizeof game_session_ca52[0], game_session_ca52},
    {0xca55u, 0xca58u, 0x0000u, 1u, sizeof game_session_ca55 / sizeof game_session_ca55[0], game_session_ca55},
    {0xca5au, 0x0000u, 0x0000u, 1u, sizeof game_session_ca5a / sizeof game_session_ca5a[0], game_session_ca5a},
    {0xca67u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca67 / sizeof game_session_ca67[0], game_session_ca67},
    {0xca6au, 0x0000u, 0x0000u, 1u, sizeof game_session_ca6a / sizeof game_session_ca6a[0], game_session_ca6a},
    {0xca72u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca72 / sizeof game_session_ca72[0], game_session_ca72},
    {0xca77u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca77 / sizeof game_session_ca77[0], game_session_ca77},
    {0xca7eu, 0x0000u, 0x0000u, 1u, sizeof game_session_ca7e / sizeof game_session_ca7e[0], game_session_ca7e},
    {0xca82u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca82 / sizeof game_session_ca82[0], game_session_ca82},
    {0xca85u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca85 / sizeof game_session_ca85[0], game_session_ca85},
    {0xca88u, 0xca8au, 0x0000u, 1u, sizeof game_session_ca88 / sizeof game_session_ca88[0], game_session_ca88},
    {0xca8cu, 0x0000u, 0x0000u, 1u, sizeof game_session_ca8c / sizeof game_session_ca8c[0], game_session_ca8c},
    {0xca8eu, 0x0000u, 0x0000u, 1u, sizeof game_session_ca8e / sizeof game_session_ca8e[0], game_session_ca8e},
    {0xca91u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca91 / sizeof game_session_ca91[0], game_session_ca91},
    {0xca96u, 0x0000u, 0x0000u, 1u, sizeof game_session_ca96 / sizeof game_session_ca96[0], game_session_ca96},
    {0xca9eu, 0x0000u, 0x0000u, 1u, sizeof game_session_ca9e / sizeof game_session_ca9e[0], game_session_ca9e},
    {0xcaa8u, 0x0000u, 0x0000u, 1u, sizeof game_session_caa8 / sizeof game_session_caa8[0], game_session_caa8},
    {0xcaafu, 0xcab1u, 0x0000u, 1u, sizeof game_session_caaf / sizeof game_session_caaf[0], game_session_caaf},
    {0xcab3u, 0xcabdu, 0x0000u, 1u, sizeof game_session_cab3 / sizeof game_session_cab3[0], game_session_cab3},
    {0xcabfu, 0x0000u, 0x0000u, 1u, sizeof game_session_cabf / sizeof game_session_cabf[0], game_session_cabf},
    {0xcac2u, 0x0000u, 0x0000u, 1u, sizeof game_session_cac2 / sizeof game_session_cac2[0], game_session_cac2},
    {0xcacau, 0x0000u, 0x0000u, 1u, sizeof game_session_caca / sizeof game_session_caca[0], game_session_caca},
    {0xcad4u, 0x0000u, 0x0000u, 1u, sizeof game_session_cad4 / sizeof game_session_cad4[0], game_session_cad4},
    {0xcad8u, 0x0000u, 0x0000u, 1u, sizeof game_session_cad8 / sizeof game_session_cad8[0], game_session_cad8},
    {0xcadbu, 0x0000u, 0x0000u, 1u, sizeof game_session_cadb / sizeof game_session_cadb[0], game_session_cadb},
    {0xcadeu, 0xcae0u, 0x0000u, 1u, sizeof game_session_cade / sizeof game_session_cade[0], game_session_cade},
    {0xcae2u, 0x0000u, 0x0000u, 1u, sizeof game_session_cae2 / sizeof game_session_cae2[0], game_session_cae2},
    {0xcae7u, 0x0000u, 0xcaf0u, 1u, sizeof game_session_cae7 / sizeof game_session_cae7[0], game_session_cae7},
    {0xcae9u, 0x0000u, 0x0000u, 1u, sizeof game_session_cae9 / sizeof game_session_cae9[0], game_session_cae9},
    {0xcaecu, 0x0000u, 0x0000u, 1u, sizeof game_session_caec / sizeof game_session_caec[0], game_session_caec},
    {0xcaefu, 0x0000u, 0xcaf0u, 1u, sizeof game_session_caef / sizeof game_session_caef[0], game_session_caef},
};

enum { GAME_SESSION_CLOCK_CAPACITY = 2048u };

typedef struct GameSessionClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[GAME_SESSION_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
    bool returning;
} GameSessionClock;

static bool GameSessionRegistersSupported(const CpuState *cpu) {
    return cpu->ram && cpu->PB == 0x84u && !cpu->emulation && cpu->m_flag <= 1u &&
        !cpu->x_flag && !cpu->D && !cpu->_flag_D &&
        (cpu->DB == 0x7eu || cpu->DB == 0u || cpu->DB == 0x80u) &&
        cpu->S >= 0x1f20u && cpu->S <= 0x1ffcu;
}

static void GameSessionOpcode(GameSessionClock *clock, unsigned pc,
                               unsigned fetched, unsigned cycles, unsigned data) {
    assert(clock->span.count < GAME_SESSION_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] =
        (FixedSpanOpcode){(uint16_t)pc, (uint8_t)fetched, (uint8_t)cycles, (uint8_t)data};
}

static const GameSessionPhase *FindGameSessionPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof game_session_phases / sizeof game_session_phases[0]; ++i)
        if (game_session_phases[i].pc == pc)
            return game_session_phases + i;
    return NULL;
}

static uint16_t GameSessionWord(const CpuState *cpu, unsigned address) {
    return cpu->ram[address] | (uint16_t)cpu->ram[address + 1u] << 8;
}

typedef struct GameSessionProjection {
    uint16_t hand;
    uint16_t scratch2;
    uint16_t scratch4;
    uint16_t scratchb;
    uint16_t scratch13;
    uint16_t deck;
    uint16_t candidate;
    uint16_t index;
    uint16_t position;
    uint16_t ranks[4];
} GameSessionProjection;

static bool GameSessionBranch(GameSessionClock *clock, uint16_t branch,
                              const GameSessionProjection *p,
                              bool *taken, uint16_t *target) {
    const CpuState *cpu = clock->call.cpu;
    const uint16_t score = GameSessionWord(cpu, 0u);
    const uint16_t deck_card = GameSessionWord(cpu, 0x7036u + p->deck) & 0xffu;
    switch (branch) {
    case 0xa763u:
        *taken = score != 21u; *target = *taken ? 0xa768u : 0xa765u; break;
    case 0xa784u:
        *taken = cpu->A != 1u; *target = *taken ? 0xa7b8u : 0xa786u; break;
    case 0xa79bu:
        *taken = GameSessionWord(cpu, 0x70c6u) == 5u;
        *target = *taken ? 0xa7c6u : 0xa79du; break;
    case 0xa7a7u:
        *taken = ((uint16_t)(21u - score) & 0x8000u) != 0u;
        *target = *taken ? 0xa7c6u : 0xa7a9u; break;
    case 0xa7bbu:
        *taken = cpu->A != 2u; *target = *taken ? 0xa77eu : 0xa7bdu; break;
    case 0xa7d4u:
        *taken = !((uint16_t)(score - 21u) & 0x8000u);
        *target = *taken ? 0xa810u : 0xa7d6u; break;
    case 0xa7e1u:
        *taken = ((uint16_t)(deck_card - 10u) & 0x8000u) != 0u;
        *target = *taken ? 0xa7e6u : 0xa7e3u; break;
    case 0xa7f0u:
        *taken = ((uint16_t)(21u - p->candidate) & 0x8000u) != 0u;
        *target = *taken ? 0xa7f4u : 0xa7f2u; break;
    case 0xa7feu:
        *taken = ((uint16_t)(cpu->A - 70u) & 0x8000u) != 0u;
        *target = *taken ? 0xa810u : 0xa800u; break;
    case 0xa80eu:
        *taken = GameSessionWord(cpu, 0x70c6u + (uint16_t)(p->hand * 2u)) != 5u;
        *target = *taken ? 0xa7c8u : 0xa810u; break;
    case 0xa817u:
        *taken = !((uint16_t)(2u - p->hand) & 0x8000u);
        *target = *taken ? 0xa7c8u : 0xa819u; break;
    case 0xa825u:
        *taken = ((uint16_t)(score - 17u) & 0x8000u) != 0u;
        *target = *taken ? 0xa83du : 0xa827u; break;
    case 0xa831u:
        *taken = cpu->A == 0u; *target = *taken ? 0xa842u : 0xa833u; break;
    case 0xa839u:
        *taken = false; *target = 0xa83bu; break;
    case 0xa840u:
        *taken = !((uint16_t)(score - 21u) & 0x8000u);
        *target = *taken ? 0xa88au : 0xa842u; break;
    case 0xa84du:
        *taken = ((uint16_t)(deck_card - 10u) & 0x8000u) != 0u;
        *target = *taken ? 0xa852u : 0xa84fu; break;
    case 0xa85cu:
        *taken = !((uint16_t)(21u - p->candidate) & 0x8000u);
        *target = *taken ? 0xa87au : 0xa85eu; break;
    case 0xa863u:
        *taken = !((uint16_t)(score - 17u) & 0x8000u);
        *target = *taken ? 0xa86du : 0xa865u; break;
    case 0xa86bu:
        *taken = false; *target = 0xa86du; break;
    case 0xa876u:
        *taken = !((uint16_t)(p->deck - 60u) & 0x8000u);
        *target = *taken ? 0xa88au : 0xa878u; break;
    case 0xa888u:
        *taken = GameSessionWord(cpu, 0x70c6u + (uint16_t)(p->hand * 2u)) != 5u;
        *target = *taken ? 0xa819u : 0xa88au; break;
    case 0xa8a2u:
        *taken = !(p->scratch4 & 0x8000u);
        *target = *taken ? 0xa891u : 0xa8a4u; break;
    case 0xa8c9u:
        *taken = !(p->scratchb & 0x8000u);
        *target = *taken ? 0xa8b8u : 0xa8cbu; break;
    case 0xa8d7u:
        *taken = !(p->position & 0x8000u);
        *target = *taken ? 0xa8d1u : 0xa8d9u; break;
    case 0xa8dfu:
        *taken = !((uint16_t)(p->ranks[0] - p->ranks[1]) & 0x8000u);
        *target = *taken ? 0xa8e4u : 0xa8e1u; break;
    case 0xa8e7u:
        *taken = !((uint16_t)(p->ranks[0] - p->ranks[2]) & 0x8000u);
        *target = *taken ? 0xa8ecu : 0xa8e9u; break;
    case 0xa8efu:
        *taken = !((uint16_t)(p->ranks[0] - p->ranks[3]) & 0x8000u);
        *target = *taken ? 0xa8f4u : 0xa8f1u; break;
    case 0xa8f7u:
        *taken = p->ranks[0] != 0u; *target = *taken ? 0xa939u : 0xa8f9u; break;
    case 0xa90bu:
        *taken = !((uint16_t)(21u - score) & 0x8000u);
        *target = *taken ? 0xa910u : 0xa90du; break;
    case 0xa927u:
        *taken = ((uint16_t)(21u - score) & 0x8000u) != 0u;
        *target = *taken ? 0xa933u : 0xa929u; break;
    case 0xa92eu:
        *taken = !((uint16_t)(p->ranks[0] - score) & 0x8000u);
        *target = *taken ? 0xa933u : 0xa930u; break;
    case 0xa937u:
        *taken = !(p->scratch4 & 0x8000u);
        *target = *taken ? 0xa91au : 0xa939u; break;
    case 0xa981u:
        *taken = !(p->scratch13 & 0x8000u);
        *target = *taken ? 0xa95bu : 0xa983u; break;
    case 0xa990u:
        *taken = p->scratch2 != 0u; *target = *taken ? 0xa93eu : 0xa992u; break;
    case 0xa9ebu:
        *taken = p->scratch4 != 3u; *target = *taken ? 0xa9c6u : 0xa9edu; break;
    case 0xaa02u:
        *taken = GameSessionWord(cpu, 0x7107u) != 0u;
        *target = *taken ? 0xaa0cu : 0xaa04u; break;
    case 0xaa18u:
        *taken = cpu->A != 1u; *target = *taken ? 0xaa12u : 0xaa1au; break;
    case 0xaa61u:
        *taken = score != 0u; *target = *taken ? 0xaaafu : 0xaa63u; break;
    case 0xaa6eu:
        *taken = (GameSessionWord(cpu, 0xb55u) | GameSessionWord(cpu, 0xb57u)) != 0u;
        *target = *taken ? 0xaa9fu : 0xaa70u; break;
    case 0xaa93u:
        *taken = (cpu->A & 0xffu) != 0u;
        *target = *taken ? 0xaa98u : 0xaa95u; break;
    case 0xc91eu:
        *taken = (cpu->A & 0xffu) != 1u;
        *target = *taken ? 0xc910u : 0xc920u; break;
    case 0xc940u:
        *taken = (uint8_t)p->scratch2 != 0u;
        *target = *taken ? 0xc93bu : 0xc942u; break;
    case 0xc94du:
        *taken = (uint8_t)p->scratch4 != 0u;
        *target = *taken ? 0xc930u : 0xc94fu; break;
    case 0xc97au:
        *taken = cpu->ram[0x71fau] != 0u;
        *target = *taken ? 0xc97fu : 0xc97cu; break;
    case 0xc998u: {
        const uint16_t location = GameSessionWord(cpu, 0x70eau + GameSessionWord(cpu, 0x71f8u));
        *taken = !((uint16_t)(GameSessionWord(cpu, 0x711cu + location) - 1u) & 0x8000u);
        *target = *taken ? 0xc9bcu : 0xc99au; break;
    }
    case 0xc9a2u:
        *taken = !cpu->ram[0x30u]; *target = *taken ? 0xc9b7u : 0xc9a4u; break;
    case 0xc9adu:
        *taken = (cpu->A & 0xffu) != 1u;
        *target = *taken ? 0xc99au : 0xc9afu; break;
    case 0xc9cau:
        *taken = !cpu->ram[0x30u]; *target = *taken ? 0xc9e7u : 0xc9ccu; break;
    case 0xc9d5u:
        *taken = (cpu->A & 0xffu) != 1u;
        *target = *taken ? 0xc9ddu : 0xc9d7u; break;
    case 0xc9e0u:
        *taken = !(cpu->A & 0xf00u);
        *target = *taken ? 0xc9c2u : 0xc9e2u; break;
    case 0xc9fcu:
        *taken = !GameSessionWord(cpu, 0x71cau);
        *target = *taken ? 0xca67u : 0xc9feu; break;
    case 0xca26u:
        *taken = !(cpu->A & 0xffu); *target = *taken ? 0xca1eu : 0xca28u; break;
    case 0xca30u:
        *taken = !GameSessionWord(cpu, 0x71ccu);
        *target = *taken ? 0xca8eu : 0xca32u; break;
    case 0xca50u:
        *taken = !cpu->ram[0u]; *target = *taken ? 0xca8eu : 0xca52u; break;
    case 0xca58u:
        *taken = !GameSessionWord(cpu, 0x71cau);
        *target = *taken ? 0xca67u : 0xca5au; break;
    case 0xca8au:
        *taken = !(cpu->A & 0xffu); *target = *taken ? 0xca82u : 0xca8cu; break;
    case 0xcab1u:
        *taken = !cpu->ram[0u]; *target = *taken ? 0xcaefu : 0xcab3u; break;
    case 0xcabdu:
        *taken = (GameSessionWord(cpu, 0xb55u) | GameSessionWord(cpu, 0xb57u)) != 0u;
        *target = *taken ? 0xcae9u : 0xcabfu; break;
    case 0xcae0u:
        *taken = !(cpu->A & 0xffu); *target = *taken ? 0xcad8u : 0xcae2u; break;
    default: return false;
    }
    return true;
}


static bool AppendGameSessionPhase(GameSessionClock *clock, uint16_t pc,
                                   GameSessionProjection projection) {
    const GameSessionPhase *phase = FindGameSessionPhase(pc);
    const CpuState *cpu = clock->call.cpu;
    if (!phase || clock->span.count + phase->count + 1u >= GAME_SESSION_CLOCK_CAPACITY)
        return false;
    for (unsigned i = 0u; i < phase->count; ++i) {
        const FixedSpanOpcode *op = phase->opcodes + i;
        switch (op->pc) {
        case 0xa73du:
        case 0xa754u: projection.hand = 0u; break;
        case 0xa742u:
        case 0xa747u:
        case 0xa74cu:
        case 0xa7c6u:
        case 0xa810u: ++projection.hand; break;
        case 0xa759u:
        case 0xa8a7u:
        case 0xa8fcu:
        case 0xa93cu: projection.scratch2 = 3u; break;
        case 0xa79du:
        case 0xa88fu:
        case 0xa8b6u:
        case 0xa913u: projection.scratch2 = 0u; break;
        case 0xa7cau:
        case 0xa81bu: projection.scratch2 = projection.hand; break;
        case 0xa7d6u:
        case 0xa842u: projection.position = projection.deck; break;
        case 0xa7e9u:
        case 0xa855u: {
            unsigned card = GameSessionWord(cpu, 0x7036u + projection.deck) & 0xffu;
            if (card >= 10u)
                card = 10u;
            projection.candidate = (uint16_t)(GameSessionWord(cpu, 0u) + card);
            projection.scratch2 = projection.candidate;
            break;
        }
        case 0xa807u:
        case 0xa881u: projection.index = (uint16_t)(projection.hand * 2u); break;
        case 0xa86du:
        case 0xa86fu: ++projection.deck; break;
        case 0xa88du: projection.scratch4 = 3u; break;
        case 0xa898u:
        case 0xa8c1u:
        case 0xa921u:
        case 0xa944u: projection.index = (uint16_t)(projection.scratch2 * 2u); break;
        case 0xa89eu:
        case 0xa8bbu:
        case 0xa933u: ++projection.scratch2; break;
        case 0xa8a0u:
        case 0xa935u: --projection.scratch4; break;
        case 0xa8aeu: projection.ranks[0] = GameSessionWord(cpu, 0u); break;
        case 0xa8b4u: projection.scratchb = 2u; break;
        case 0xa8c4u:
            if (projection.index > 6u || (projection.index & 1u))
                return false;
            projection.ranks[projection.index / 2u] = GameSessionWord(cpu, 0u);
            break;
        case 0xa8c7u: --projection.scratchb; break;
        case 0xa8cbu:
        case 0xa910u: projection.index = 0u; break;
        case 0xa8ceu:
        case 0xa915u: projection.position = 2u; break;
        case 0xa8d4u:
        case 0xa8d5u:
        case 0xa97bu:
        case 0xa97cu: ++projection.index; break;
        case 0xa8d6u: --projection.position; break;
        case 0xa903u: projection.ranks[0] = GameSessionWord(cpu, 0u); break;
        case 0xa90du: projection.ranks[0] = 0u; break;
        case 0xa918u: projection.scratch4 = projection.position; break;
        case 0xa94fu:
            projection.scratch13 = (uint16_t)(GameSessionWord(cpu, 0x70c6u + projection.index) - 1u);
            break;
        case 0xa958u: projection.index = (uint16_t)(projection.scratch2 * 10u); break;
        case 0xa979u: projection.index = GameSessionWord(cpu, 0x11u); break;
        case 0xa97fu: --projection.scratch13; break;
        case 0xa98eu: --projection.scratch2; break;
        case 0xa9c4u: projection.scratch4 = 0u; break;
        case 0xa9cau: projection.index = (uint16_t)(projection.scratch4 * 2u); break;
        case 0xa9e4u: ++projection.scratch4; break;
        case 0xc93eu:
            projection.scratch2 = (uint8_t)(projection.scratch2 - 1u);
            break;
        case 0xc94bu:
            projection.scratch4 = (uint8_t)(projection.scratch4 - 1u);
            break;
        case 0xc98du: projection.index = GameSessionWord(cpu, 0x71f8u); break;
        case 0xc991u: projection.index = GameSessionWord(cpu, 0x70eau + projection.index); break;
        default: break;
        }
        unsigned cycles = op->cycles;
        unsigned base = 0u;
        unsigned index = projection.index;
        switch (op->pc) {
        case 0xa7d8u:
        case 0xa844u: base = 0x7036u; index = projection.position; break;
        case 0xa808u:
        case 0xa882u:
        case 0xa94bu: base = 0x70c6u; break;
        case 0xa960u: base = 0x709eu; break;
        case 0xa9cbu: base = 0x70ceu; break;
        case 0xa9d0u: base = 0x70e9u; break;
        case 0xc98eu: base = 0x70eau; break;
        case 0xc992u: base = 0x711cu; break;
        default: break;
        }
        if (base && (base >> 8) != ((base + index) >> 8))
            ++cycles;
        const unsigned repetitions = op->pc == 0xc839u || op->pc == 0xc845u ? 256u : 1u;
        for (unsigned copy = 0u; copy < repetitions; ++copy) {
            if (clock->span.count + 1u >= GAME_SESSION_CLOCK_CAPACITY)
                return false;
            GameSessionOpcode(clock, op->pc, op->fetched, cycles, op->data_bytes);
        }
    }
    if (phase->return_pc) {
        clock->returning = true;
        clock->next = 0x840000u | phase->return_pc;
    }
    if (!phase->branch)
        return true;
    bool taken;
    uint16_t target;
    if (!GameSessionBranch(clock, phase->branch, &projection, &taken, &target))
        return false;
    GameSessionOpcode(clock, phase->branch, 2u, taken ? 3u : 2u, 0u);
    return AppendGameSessionPhase(clock, target, projection);
}

static bool GameSessionWindowSupported(const GameSessionClock *clock) {
    CpuState *cpu = clock->call.cpu;
    if (!GameSessionRegistersSupported(cpu) ||
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

static bool PrepareGameSessionClock(GameSessionClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->returning = false;
    clock->next = pc;
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x140000u ||
        (pc >> 16) != 0x84u || !GameSessionRegistersSupported(cpu))
        return false;
    const GameSessionPhase *phase = FindGameSessionPhase((uint16_t)pc);
    if (!phase || (pc != 0x84c826u && pc != 0x84aaafu && pc != 0x84aab2u &&
         cpu->m_flag != phase->byte_mode) ||
        (cpu->DB != 0x7eu && pc != 0x84a705u && pc != 0x84a710u &&
         (pc < 0x84c826u || pc > 0x84c8acu)))
        return false;
    const GameSessionProjection projection = {
        GameSessionWord(cpu, 0x17u), GameSessionWord(cpu, 2u),
        GameSessionWord(cpu, 4u), GameSessionWord(cpu, 0xbu),
        GameSessionWord(cpu, 0x13u), GameSessionWord(cpu, 0x15u),
        GameSessionWord(cpu, 2u), cpu->X, cpu->Y,
        {GameSessionWord(cpu, 0x70e7u), GameSessionWord(cpu, 0x70e9u),
         GameSessionWord(cpu, 0x70ebu), GameSessionWord(cpu, 0x70edu)}};
    return AppendGameSessionPhase(clock, (uint16_t)pc, projection) &&
        GameSessionWindowSupported(clock);
}

static uint8_t GameSessionChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    GameSessionClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareGameSessionClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeGameSession(CpuState *cpu, uint32_t entry,
                                         ActorSceneOwnersFunction run) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    GameSessionClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (cpu->host_return_valid != 2u || !PrepareGameSessionClock(&clock, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = run(&memory, &state, GameSessionChild, &clock);
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
