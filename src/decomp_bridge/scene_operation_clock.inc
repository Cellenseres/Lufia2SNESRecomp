typedef struct SceneOperationPhase {
    uint16_t pc;
    uint16_t branch;
    uint32_t jump;
    uint16_t child;
    uint16_t return_pc;
    unsigned count;
    const FixedSpanOpcode *opcodes;
} SceneOperationPhase;

static const FixedSpanOpcode scene_operation_ba39[] = {
    {0xba39u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_ba3c[] = {
    {0xba3cu, 1u, 3u, 0u},
    {0xba3du, 2u, 2u, 0u},
    {0xba3fu, 1u, 3u, 0u},
    {0xba40u, 1u, 2u, 0u},
    {0xba41u, 1u, 2u, 0u},
    {0xba42u, 3u, 8u, 4u},
};

static const FixedSpanOpcode scene_operation_ba45[] = {
    {0xba45u, 3u, 4u, 1u},
    {0xba48u, 2u, 2u, 0u},
};

static const FixedSpanOpcode scene_operation_ba4c[] = {
    {0xba4cu, 3u, 4u, 0u},
};

static const FixedSpanOpcode scene_operation_ba4f[] = {
    {0xba4fu, 3u, 4u, 1u},
    {0xba52u, 3u, 4u, 0u},
};

static const FixedSpanOpcode scene_operation_ba69[] = {
    {0xba69u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_ba6c[] = {
    {0xba6cu, 1u, 3u, 1u},
    {0xba6du, 1u, 4u, 2u},
    {0xba6eu, 1u, 3u, 1u},
    {0xba6fu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba73[] = {
    {0xba73u, 2u, 3u, 1u},
    {0xba75u, 1u, 4u, 1u},
    {0xba76u, 2u, 3u, 1u},
    {0xba78u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba7c[] = {
    {0xba7cu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba80[] = {
    {0xba80u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba84[] = {
    {0xba84u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba88[] = {
    {0xba88u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba8c[] = {
    {0xba8cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_ba8f[] = {
    {0xba8fu, 2u, 2u, 0u},
    {0xba91u, 3u, 6u, 2u},
    {0xba94u, 1u, 5u, 2u},
    {0xba95u, 1u, 4u, 1u},
    {0xba96u, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_ba97[] = {
    {0xba97u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_ba9a[] = {
    {0xba9au, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_ba9e[] = {
    {0xba9eu, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_baa1[] = {
    {0xbaa1u, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_baa2[] = {
    {0xbaa2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_baa5[] = {
    {0xbaa5u, 2u, 2u, 0u},
    {0xbaa7u, 3u, 4u, 1u},
    {0xbaaau, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_baad[] = {
    {0xbaadu, 3u, 4u, 1u},
    {0xbab0u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bab4[] = {
    {0xbab4u, 2u, 2u, 0u},
    {0xbab6u, 2u, 3u, 1u},
    {0xbab8u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_babc[] = {
    {0xbabcu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bac0[] = {
    {0xbac0u, 2u, 2u, 0u},
    {0xbac2u, 3u, 4u, 1u},
    {0xbac5u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bac8[] = {
    {0xbac8u, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bac9[] = {
    {0xbac9u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bacc[] = {
    {0xbaccu, 1u, 4u, 2u},
    {0xbacdu, 1u, 3u, 1u},
    {0xbaceu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bad2[] = {
    {0xbad2u, 2u, 2u, 0u},
    {0xbad4u, 4u, 5u, 1u},
    {0xbad8u, 4u, 5u, 1u},
    {0xbadcu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bae0[] = {
    {0xbae0u, 2u, 2u, 0u},
    {0xbae2u, 3u, 6u, 2u},
    {0xbae5u, 1u, 4u, 1u},
    {0xbae6u, 1u, 5u, 2u},
    {0xbae7u, 2u, 2u, 0u},
    {0xbae9u, 3u, 6u, 2u},
    {0xbaecu, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_baed[] = {
    {0xbaedu, 4u, 4u, 0u},
};

static const FixedSpanOpcode scene_operation_baf1[] = {
    {0xbaf1u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_baf4[] = {
    {0xbaf4u, 2u, 2u, 0u},
};

static const FixedSpanOpcode scene_operation_baf8[] = {
    {0xbaf8u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bafb[] = {
    {0xbafbu, 3u, 4u, 1u},
    {0xbafeu, 3u, 3u, 0u},
    {0xbb01u, 3u, 5u, 2u},
    {0xbb04u, 2u, 2u, 0u},
    {0xbb06u, 3u, 4u, 1u},
    {0xbb09u, 3u, 4u, 1u},
    {0xbb0cu, 2u, 2u, 0u},
    {0xbb0eu, 3u, 4u, 1u},
    {0xbb11u, 2u, 2u, 0u},
    {0xbb13u, 3u, 4u, 1u},
    {0xbb16u, 2u, 2u, 0u},
    {0xbb18u, 3u, 4u, 1u},
    {0xbb1bu, 2u, 2u, 0u},
    {0xbb1du, 3u, 4u, 1u},
    {0xbb20u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bb24[] = {
    {0xbb24u, 3u, 3u, 0u},
    {0xbb27u, 3u, 5u, 2u},
    {0xbb2au, 2u, 2u, 0u},
    {0xbb2cu, 3u, 4u, 1u},
    {0xbb2fu, 2u, 2u, 0u},
    {0xbb31u, 3u, 4u, 1u},
    {0xbb34u, 2u, 2u, 0u},
    {0xbb36u, 3u, 4u, 1u},
    {0xbb39u, 2u, 2u, 0u},
    {0xbb3bu, 2u, 5u, 2u},
    {0xbb3du, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb40[] = {
    {0xbb40u, 3u, 3u, 0u},
    {0xbb43u, 3u, 5u, 2u},
    {0xbb46u, 2u, 2u, 0u},
    {0xbb48u, 3u, 4u, 1u},
    {0xbb4bu, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb4c[] = {
    {0xbb4cu, 2u, 2u, 0u},
    {0xbb4eu, 2u, 5u, 2u},
    {0xbb50u, 2u, 2u, 0u},
    {0xbb52u, 3u, 4u, 1u},
    {0xbb55u, 3u, 4u, 1u},
    {0xbb58u, 3u, 4u, 1u},
    {0xbb5bu, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb5e[] = {
    {0xbb5eu, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb70[] = {
    {0xbb70u, 2u, 2u, 0u},
    {0xbb72u, 4u, 5u, 1u},
    {0xbb76u, 3u, 4u, 1u},
    {0xbb79u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb7c[] = {
    {0xbb7cu, 2u, 2u, 0u},
};

static const FixedSpanOpcode scene_operation_bb80[] = {
    {0xbb80u, 2u, 2u, 0u},
    {0xbb82u, 3u, 4u, 1u},
    {0xbb85u, 2u, 2u, 0u},
    {0xbb87u, 2u, 3u, 0u},
};

static const FixedSpanOpcode scene_operation_bb89[] = {
    {0xbb89u, 2u, 2u, 0u},
    {0xbb8bu, 3u, 4u, 1u},
    {0xbb8eu, 2u, 2u, 0u},
    {0xbb90u, 3u, 6u, 2u},
    {0xbb93u, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb90[] = {
    {0xbb90u, 3u, 6u, 2u},
    {0xbb93u, 1u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb94[] = {
    {0xbb94u, 1u, 4u, 2u},
    {0xbb95u, 1u, 3u, 1u},
    {0xbb96u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bb9a[] = {
    {0xbb9au, 1u, 4u, 1u},
    {0xbb9bu, 1u, 5u, 2u},
    {0xbb9cu, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bb9f[] = {
    {0xbb9fu, 2u, 3u, 1u},
    {0xbba1u, 3u, 4u, 1u},
    {0xbba4u, 2u, 3u, 1u},
    {0xbba6u, 1u, 3u, 1u},
    {0xbba7u, 1u, 4u, 2u},
    {0xbba8u, 1u, 4u, 2u},
    {0xbba9u, 1u, 3u, 1u},
    {0xbbaau, 1u, 3u, 1u},
    {0xbbabu, 2u, 2u, 0u},
};

static const FixedSpanOpcode scene_operation_bbaf[] = {
    {0xbbafu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bbb3[] = {
    {0xbbb3u, 2u, 3u, 0u},
};

static const FixedSpanOpcode scene_operation_bbb5[] = {
    {0xbbb5u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bbbb[] = {
    {0xbbbbu, 2u, 2u, 0u},
    {0xbbbdu, 3u, 6u, 2u},
    {0xbbc0u, 1u, 3u, 1u},
    {0xbbc1u, 1u, 4u, 1u},
    {0xbbc2u, 2u, 3u, 1u},
    {0xbbc4u, 2u, 2u, 0u},
    {0xbbc6u, 4u, 5u, 1u},
    {0xbbcau, 4u, 5u, 1u},
    {0xbbceu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bbc0[] = {
    {0xbbc0u, 1u, 3u, 1u},
    {0xbbc1u, 1u, 4u, 1u},
    {0xbbc2u, 2u, 3u, 1u},
    {0xbbc4u, 2u, 2u, 0u},
    {0xbbc6u, 4u, 5u, 1u},
    {0xbbcau, 4u, 5u, 1u},
    {0xbbceu, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bbd2[] = {
    {0xbbd2u, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bbd5[] = {
    {0xbbd5u, 4u, 8u, 3u},
};

static const FixedSpanOpcode scene_operation_bbd9[] = {
    {0xbbd9u, 2u, 2u, 0u},
    {0xbbdbu, 2u, 3u, 1u},
    {0xbbddu, 3u, 6u, 2u},
};

static const FixedSpanOpcode scene_operation_bbe0[] = {
    {0xbbe0u, 1u, 4u, 1u},
    {0xbbe1u, 1u, 4u, 1u},
    {0xbbe2u, 1u, 5u, 2u},
    {0xbbe3u, 1u, 5u, 2u},
    {0xbbe4u, 1u, 4u, 1u},
    {0xbbe5u, 2u, 2u, 0u},
    {0xbbe7u, 3u, 4u, 1u},
    {0xbbeau, 1u, 6u, 2u},
};

static const SceneOperationPhase scene_operation_phases[] = {
    {0xba39u, 0x0000u, 0x000000u, 0xba39u, 0x0000u, 1u, scene_operation_ba39},
    {0xba3cu, 0x0000u, 0x000000u, 0xba42u, 0x0000u, 6u, scene_operation_ba3c},
    {0xba45u, 0xba4au, 0x000000u, 0x0000u, 0x0000u, 2u, scene_operation_ba45},
    {0xba4cu, 0x0000u, 0x009d00u, 0x0000u, 0x0000u, 1u, scene_operation_ba4c},
    {0xba4fu, 0x0000u, 0x009db0u, 0x0000u, 0x0000u, 2u, scene_operation_ba4f},
    {0xba67u, 0x0000u, 0x00ba67u, 0x0000u, 0x0000u, 0u, NULL},
    {0xba69u, 0x0000u, 0x000000u, 0xba69u, 0x0000u, 1u, scene_operation_ba69},
    {0xba6cu, 0x0000u, 0x000000u, 0xba6fu, 0x0000u, 4u, scene_operation_ba6c},
    {0xba73u, 0x0000u, 0x000000u, 0xba78u, 0x0000u, 4u, scene_operation_ba73},
    {0xba7cu, 0x0000u, 0x000000u, 0xba7cu, 0x0000u, 1u, scene_operation_ba7c},
    {0xba80u, 0x0000u, 0x000000u, 0xba80u, 0x0000u, 1u, scene_operation_ba80},
    {0xba84u, 0x0000u, 0x000000u, 0xba84u, 0x0000u, 1u, scene_operation_ba84},
    {0xba88u, 0x0000u, 0x000000u, 0xba88u, 0x0000u, 1u, scene_operation_ba88},
    {0xba8cu, 0x0000u, 0x000000u, 0xba8cu, 0x0000u, 1u, scene_operation_ba8c},
    {0xba8fu, 0x0000u, 0x000000u, 0x0000u, 0xba96u, 5u, scene_operation_ba8f},
    {0xba97u, 0x0000u, 0x000000u, 0xba97u, 0x0000u, 1u, scene_operation_ba97},
    {0xba9au, 0x0000u, 0x000000u, 0xba9au, 0x0000u, 1u, scene_operation_ba9a},
    {0xba9eu, 0x0000u, 0x000000u, 0xba9eu, 0x0000u, 1u, scene_operation_ba9e},
    {0xbaa1u, 0x0000u, 0x000000u, 0x0000u, 0xbaa1u, 1u, scene_operation_baa1},
    {0xbaa2u, 0x0000u, 0x000000u, 0xbaa2u, 0x0000u, 1u, scene_operation_baa2},
    {0xbaa5u, 0x0000u, 0x000000u, 0xbaaau, 0x0000u, 3u, scene_operation_baa5},
    {0xbaadu, 0x0000u, 0x000000u, 0xbab0u, 0x0000u, 2u, scene_operation_baad},
    {0xbab4u, 0x0000u, 0x000000u, 0xbab8u, 0x0000u, 3u, scene_operation_bab4},
    {0xbabcu, 0x0000u, 0x000000u, 0xbabcu, 0x0000u, 1u, scene_operation_babc},
    {0xbac0u, 0x0000u, 0x000000u, 0xbac5u, 0x0000u, 3u, scene_operation_bac0},
    {0xbac8u, 0x0000u, 0x000000u, 0x0000u, 0xbac8u, 1u, scene_operation_bac8},
    {0xbac9u, 0x0000u, 0x000000u, 0xbac9u, 0x0000u, 1u, scene_operation_bac9},
    {0xbaccu, 0x0000u, 0x000000u, 0xbaceu, 0x0000u, 3u, scene_operation_bacc},
    {0xbad2u, 0x0000u, 0x000000u, 0xbadcu, 0x0000u, 4u, scene_operation_bad2},
    {0xbae0u, 0x0000u, 0x000000u, 0x0000u, 0xbaecu, 7u, scene_operation_bae0},
    {0xbaedu, 0x0000u, 0x808000u, 0x0000u, 0x0000u, 1u, scene_operation_baed},
    {0xbaf1u, 0x0000u, 0x000000u, 0xbaf1u, 0x0000u, 1u, scene_operation_baf1},
    {0xbaf4u, 0xbaf6u, 0x000000u, 0x0000u, 0x0000u, 1u, scene_operation_baf4},
    {0xbaf8u, 0x0000u, 0x000000u, 0xbaf8u, 0x0000u, 1u, scene_operation_baf8},
    {0xbafbu, 0x0000u, 0x000000u, 0xbb20u, 0x0000u, 15u, scene_operation_bafb},
    {0xbb24u, 0x0000u, 0x000000u, 0xbb3du, 0x0000u, 11u, scene_operation_bb24},
    {0xbb40u, 0x0000u, 0x000000u, 0x0000u, 0xbb4bu, 5u, scene_operation_bb40},
    {0xbb4cu, 0x0000u, 0x000000u, 0xbb5bu, 0x0000u, 7u, scene_operation_bb4c},
    {0xbb5eu, 0x0000u, 0x000000u, 0x0000u, 0xbb5eu, 1u, scene_operation_bb5e},
    {0xbb70u, 0x0000u, 0x000000u, 0xbb79u, 0x0000u, 4u, scene_operation_bb70},
    {0xbb7cu, 0xbb7eu, 0x000000u, 0x0000u, 0x0000u, 1u, scene_operation_bb7c},
    {0xbb80u, 0x0000u, 0x00bb90u, 0x0000u, 0x0000u, 4u, scene_operation_bb80},
    {0xbb89u, 0x0000u, 0x000000u, 0x0000u, 0xbb93u, 5u, scene_operation_bb89},
    {0xbb90u, 0x0000u, 0x000000u, 0x0000u, 0xbb93u, 2u, scene_operation_bb90},
    {0xbb94u, 0x0000u, 0x000000u, 0xbb96u, 0x0000u, 3u, scene_operation_bb94},
    {0xbb9au, 0x0000u, 0x000000u, 0xbb9cu, 0x0000u, 3u, scene_operation_bb9a},
    {0xbb9fu, 0xbbadu, 0x000000u, 0x0000u, 0x0000u, 9u, scene_operation_bb9f},
    {0xbbafu, 0x0000u, 0x000000u, 0xbbafu, 0x0000u, 1u, scene_operation_bbaf},
    {0xbbb3u, 0x0000u, 0x00bbb9u, 0x0000u, 0x0000u, 1u, scene_operation_bbb3},
    {0xbbb5u, 0x0000u, 0x000000u, 0xbbb5u, 0x0000u, 1u, scene_operation_bbb5},
    {0xbbb9u, 0xbbb9u, 0x000000u, 0x0000u, 0x0000u, 0u, NULL},
    {0xbbbbu, 0x0000u, 0x000000u, 0xbbceu, 0x0000u, 9u, scene_operation_bbbb},
    {0xbbc0u, 0x0000u, 0x000000u, 0xbbceu, 0x0000u, 7u, scene_operation_bbc0},
    {0xbbd2u, 0x0000u, 0x000000u, 0xbbd2u, 0x0000u, 1u, scene_operation_bbd2},
    {0xbbd5u, 0x0000u, 0x000000u, 0xbbd5u, 0x0000u, 1u, scene_operation_bbd5},
    {0xbbd9u, 0x0000u, 0x000000u, 0xbbddu, 0x0000u, 3u, scene_operation_bbd9},
    {0xbbe0u, 0x0000u, 0x000000u, 0x0000u, 0xbbeau, 8u, scene_operation_bbe0},
};

enum { SCENE_OPERATION_CLOCK_CAPACITY = 256u };

typedef struct SceneOperationClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[SCENE_OPERATION_CLOCK_CAPACITY];
    ActorPushedCall call;
    uint32_t next;
} SceneOperationClock;

typedef struct SceneOperationProjection {
    uint16_t stack;
    uint8_t field_flags;
} SceneOperationProjection;

static bool SceneOperationBankSupported(unsigned bank) {
    return bank == 0x7eu || bank < 0x40u || (bank >= 0x80u && bank < 0xc0u);
}

static const SceneOperationPhase *FindSceneOperationPhase(uint16_t pc) {
    for (unsigned i = 0u; i < sizeof scene_operation_phases / sizeof scene_operation_phases[0]; ++i)
        if (scene_operation_phases[i].pc == pc)
            return scene_operation_phases + i;
    return NULL;
}

static void SceneOperationOpcode(SceneOperationClock *clock, FixedSpanOpcode op) {
    assert(clock->span.count < SCENE_OPERATION_CLOCK_CAPACITY);
    clock->timing[clock->span.count++] = op;
}

static bool SceneOperationProjectStack(SceneOperationClock *clock, uint16_t pc,
                                 SceneOperationProjection *projection) {
    const CpuState *cpu = clock->call.cpu;
    unsigned bank_offset = 0u;
    switch (pc) {
    case 0xba8fu:
        projection->stack = (uint16_t)(cpu->S + 3u);
        bank_offset = 3u;
        break;
    case 0xbae0u:
        projection->stack = (uint16_t)(cpu->S + 3u);
        projection->field_flags &= 0xbfu;
        bank_offset = 1u;
        break;
    case 0xbb9au:
        bank_offset = 1u;
        break;
    case 0xbbe0u:
        if ((cpu->ram[(uint16_t)(cpu->S + 1u)] & 0x38u) != 0x20u)
            return false;
        projection->stack = (uint16_t)(cpu->S + 7u);
        bank_offset = 2u;
        break;
    default:
        break;
    }
    if (projection->stack > 0x1ffdu)
        return false;
    return !bank_offset || SceneOperationBankSupported(cpu->ram[(uint16_t)(cpu->S + bank_offset)]);
}

static bool AppendSceneOperationPhase(SceneOperationClock *clock, uint16_t pc,
                                 SceneOperationProjection projection) {
    const CpuState *cpu = clock->call.cpu;
    for (unsigned phases = 0u; phases < 32u; ++phases) {
        const SceneOperationPhase *phase = FindSceneOperationPhase(pc);
        if (!phase || !SceneOperationProjectStack(clock, pc, &projection))
            return false;
        if (pc == 0xba3cu && (cpu->A & 0xffu) > 8u)
            return false;
        for (unsigned i = 0u; i < phase->count; ++i)
            SceneOperationOpcode(clock, phase->opcodes[i]);
        if (phase->child) {
            if (phase->child != 0xba42u)
                return true;
            const unsigned index = (cpu->A & 0xffu) * 2u;
            const uint8_t *table = g_snes->cart->rom + 0x3a55u + index;
            pc = (uint16_t)(table[0] | (uint16_t)table[1] << 8);
            continue;
        }
        if (phase->branch) {
            bool taken;
            uint16_t target;
            switch (phase->branch) {
            case 0xbb7eu: taken = !(cpu->A & 0xffu); target = 0xbb89u; break;
            case 0xba4au: taken = (projection.field_flags & 0x40u) != 0u; target = 0xba4fu; break;
            case 0xbaf6u: taken = !(cpu->A & 0xffu); target = 0xbb4cu; break;
            case 0xbbadu: taken = (cpu->A & 0xffu) != 2u; target = 0xbbb5u; break;
            case 0xbbb9u: taken = !cpu->_flag_C; target = 0xbbc0u; break;
            default: return false;
            }
            SceneOperationOpcode(clock, (FixedSpanOpcode){phase->branch, 2u, (uint8_t)(taken ? 3u : 2u), 0u});
            pc = taken ? target : (uint16_t)(phase->branch + 2u);
            continue;
        }
        if (phase->return_pc) {
            const uint16_t sp = projection.stack;
            const uint16_t back = (uint16_t)(cpu->ram[(uint16_t)(sp + 1u)] |
                (uint16_t)cpu->ram[(uint16_t)(sp + 2u)] << 8);
            if (back != 0xba44u) {
                clock->next = 0x800000u | (uint16_t)(back + 1u);
                return true;
            }
            pc = 0xba45u;
            continue;
        }
        if (phase->jump == 0xba67u || phase->jump > 0xffffu ||
            phase->jump == 0x9d00u || phase->jump == 0x9db0u) {
            clock->next = phase->jump > 0xffffu ? phase->jump : 0x800000u | phase->jump;
            return true;
        }
        pc = (uint16_t)phase->jump;
    }
    return false;
}

static bool SceneOperationWindowSupported(const SceneOperationClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    unsigned budget = 0u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(clock->span.bank | op->pc) +
            op->data_bytes * 8u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    budget += 40u * (budget / 1324u + 1u);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x80ba39u, 0x1ffcu, false, 1363u, budget) ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u)
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if (lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PrepareSceneOperationClock(SceneOperationClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x800000u;
    clock->span.timing = clock->timing;
    clock->next = 0u;
    if ((pc & 0xff0000u) != 0x800000u || !cpu->ram || !g_snes ||
        !g_snes->cart || !g_snes->cart->rom || g_snes->cart->romSize < 0x10000u ||
        g_snes->cart->type != CART_LOROM || cpu->PB != 0x80u || cpu->emulation ||
        cpu->m_flag != 1u || cpu->x_flag || cpu->D || cpu->_flag_D ||
        cpu->host_return_valid > 3u || cpu->host_return_valid == 1u ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu || !SceneOperationBankSupported(cpu->DB))
        return false;
    const SceneOperationProjection projection = {cpu->S, cpu->ram[0x05b5u]};
    return AppendSceneOperationPhase(clock, (uint16_t)pc, projection) && SceneOperationWindowSupported(clock);
}

static uint8_t SceneOperationChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    SceneOperationClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareSceneOperationClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeSceneOperation(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    SceneOperationClock clock = {0};
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareSceneOperationClock(&clock, 0x80ba39u))
        return ActorBridgeFallback(cpu, &frame, 0x80ba39u);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2TextRunSceneOperation(&memory, &state,
        SceneOperationChild, &clock);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining && clock.next == result.pc);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
}
