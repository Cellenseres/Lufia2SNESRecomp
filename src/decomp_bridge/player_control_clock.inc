enum PlayerStepOpcode {
    PLAYER_CLOCK_C0EF, PLAYER_CLOCK_C0F2, PLAYER_CLOCK_C0F4, PLAYER_CLOCK_C0F7,
    PLAYER_CLOCK_C0F9, PLAYER_CLOCK_C0FA, PLAYER_CLOCK_C0FC, PLAYER_CLOCK_C0FF,
    PLAYER_CLOCK_C101, PLAYER_CLOCK_C103, PLAYER_CLOCK_C107, PLAYER_CLOCK_C1F2,
    PLAYER_CLOCK_C1F5, PLAYER_CLOCK_C1F8, PLAYER_CLOCK_C1FA, PLAYER_CLOCK_C1FC,
    PLAYER_CLOCK_C206, PLAYER_CLOCK_C208, PLAYER_CLOCK_C20C, PLAYER_CLOCK_C20D,
    PLAYER_CLOCK_C20F, PLAYER_CLOCK_C219, PLAYER_CLOCK_C21B, PLAYER_CLOCK_C21E,
    PLAYER_CLOCK_C221, PLAYER_CLOCK_C225, PLAYER_CLOCK_C228, PLAYER_CLOCK_C22C,
    PLAYER_CLOCK_C22E, PLAYER_CLOCK_C230, PLAYER_CLOCK_C232, PLAYER_CLOCK_C235,
    PLAYER_CLOCK_C237, PLAYER_CLOCK_C23B, PLAYER_CLOCK_C23E, PLAYER_CLOCK_C240,
    PLAYER_CLOCK_C243, PLAYER_CLOCK_C246, PLAYER_CLOCK_C247, PLAYER_CLOCK_C249,
    PLAYER_CLOCK_C24A, PLAYER_CLOCK_C24E, PLAYER_CLOCK_D350, PLAYER_CLOCK_D352,
    PLAYER_CLOCK_D354, PLAYER_CLOCK_D358, PLAYER_CLOCK_D35B, PLAYER_CLOCK_D35D,
    PLAYER_CLOCK_D360, PLAYER_CLOCK_D363, PLAYER_CLOCK_D365, PLAYER_CLOCK_D367,
    PLAYER_CLOCK_D368, PLAYER_CLOCK_D36A, PLAYER_CLOCK_D36C, PLAYER_CLOCK_D36E,
    PLAYER_CLOCK_D36F, PLAYER_CLOCK_D370, PLAYER_CLOCK_D373, PLAYER_CLOCK_D375,
    PLAYER_CLOCK_D377, PLAYER_CLOCK_D37A, PLAYER_CLOCK_D37C, PLAYER_CLOCK_D37F,
    PLAYER_CLOCK_D381, PLAYER_CLOCK_D382, PLAYER_CLOCK_D384, PLAYER_CLOCK_D385,
    PLAYER_CLOCK_D389, PLAYER_CLOCK_D39D, PLAYER_CLOCK_D39F, PLAYER_CLOCK_D3A1,
    PLAYER_CLOCK_D3A4, PLAYER_CLOCK_D3A6, PLAYER_CLOCK_D3A8, PLAYER_CLOCK_D3AB,
    PLAYER_CLOCK_D3AE, PLAYER_CLOCK_D3B7, PLAYER_CLOCK_D3B9, PLAYER_CLOCK_D3BC,
    PLAYER_CLOCK_D3BD, PLAYER_CLOCK_D3BF, PLAYER_CLOCK_D3C0, PLAYER_CLOCK_D3C4,
    PLAYER_CLOCK_D3C5, PLAYER_CLOCK_D3C7, PLAYER_CLOCK_D3CA, PLAYER_CLOCK_D3CB,
    PLAYER_CLOCK_D3CD, PLAYER_CLOCK_D3D1, PLAYER_CLOCK_D3D2, PLAYER_CLOCK_D3D3,
    PLAYER_CLOCK_D3D6, PLAYER_CLOCK_D3D7, PLAYER_CLOCK_D3D9, PLAYER_CLOCK_D3DC,
    PLAYER_CLOCK_D3DD, PLAYER_CLOCK_D3DF, PLAYER_CLOCK_D3E0, PLAYER_CLOCK_D3E4,
    PLAYER_CLOCK_D3E5, PLAYER_CLOCK_D3E7, PLAYER_CLOCK_D3EA, PLAYER_CLOCK_D3EB,
    PLAYER_CLOCK_D3ED, PLAYER_CLOCK_D3F1, PLAYER_CLOCK_D3F2, PLAYER_CLOCK_D3F3,
    PLAYER_CLOCK_D3F6, PLAYER_CLOCK_D3F7, PLAYER_CLOCK_D3F8, PLAYER_CLOCK_D3FA,
    PLAYER_CLOCK_D3FB, PLAYER_CLOCK_D3FD, PLAYER_CLOCK_D3FE, PLAYER_CLOCK_D3FF,
    PLAYER_CLOCK_D403, PLAYER_CLOCK_D405, PLAYER_CLOCK_D406, PLAYER_CLOCK_D409,
    PLAYER_CLOCK_D40D, PLAYER_CLOCK_D40F, PLAYER_CLOCK_D411, PLAYER_CLOCK_D415,
    PLAYER_CLOCK_F9AD, PLAYER_CLOCK_F9AF, PLAYER_CLOCK_F9B1, PLAYER_CLOCK_F9B3,
    PLAYER_CLOCK_F9B4, PLAYER_CLOCK_F9B6, PLAYER_CLOCK_F9BA, PLAYER_CLOCK_F9BE,
    PLAYER_CLOCK_F9C2, PLAYER_CLOCK_F9C4, PLAYER_CLOCK_F9C5, PLAYER_CLOCK_F9C7,
    PLAYER_CLOCK_F9C8, PLAYER_CLOCK_F9CC, PLAYER_CLOCK_F9CD, PLAYER_CLOCK_F9CF,
    PLAYER_CLOCK_FB12, PLAYER_CLOCK_FB13, PLAYER_CLOCK_FB15, PLAYER_CLOCK_FB16,
    PLAYER_CLOCK_FB17, PLAYER_CLOCK_FB22, PLAYER_CLOCK_FB24, PLAYER_CLOCK_FB25,
    PLAYER_CLOCK_FB27, PLAYER_CLOCK_FB28, PLAYER_CLOCK_FB2A, PLAYER_CLOCK_FB2B,
    PLAYER_CLOCK_FB2D, PLAYER_CLOCK_FBBD, PLAYER_CLOCK_FBBE, PLAYER_CLOCK_FBC0,
    PLAYER_CLOCK_FBC1, PLAYER_CLOCK_FBC2, PLAYER_CLOCK_FBC5, PLAYER_CLOCK_FBCE,
    PLAYER_CLOCK_FBD0, PLAYER_CLOCK_FBD3, PLAYER_CLOCK_FBD5, PLAYER_CLOCK_FBD7,
    PLAYER_CLOCK_FBDA, PLAYER_CLOCK_FBDC, PLAYER_CLOCK_FBDE, PLAYER_CLOCK_FBE1,
    PLAYER_CLOCK_FBE3, PLAYER_CLOCK_FBE5, PLAYER_CLOCK_FBE7, PLAYER_CLOCK_FBEA,
    PLAYER_CLOCK_FBEC, PLAYER_CLOCK_FBED, PLAYER_CLOCK_FBEF, PLAYER_CLOCK_FBF0,
    PLAYER_CLOCK_FBF1, PLAYER_CLOCK_FBF3, PLAYER_CLOCK_FBF4, PLAYER_CLOCK_FBF6,
    PLAYER_CLOCK_FBF9, PLAYER_CLOCK_FBFD, PLAYER_CLOCK_FC69, PLAYER_CLOCK_FC6C,
    PLAYER_CLOCK_FC6E, PLAYER_CLOCK_FC70, PLAYER_CLOCK_FC72, PLAYER_CLOCK_FC74,
    PLAYER_CLOCK_FC76, PLAYER_CLOCK_FC7A, PLAYER_CLOCK_FC7C, PLAYER_CLOCK_FC80,
    PLAYER_CLOCK_FC82, PLAYER_CLOCK_FC84, PLAYER_CLOCK_FC86, PLAYER_CLOCK_FC8A,
};

static const FixedSpanOpcode player_step_timing[] = {
    {0xc0efu, 3u, 4u, 1u}, {0xc0f2u, 2u, 3u, 1u},
    {0xc0f4u, 3u, 4u, 1u}, {0xc0f7u, 2u, 3u, 1u},
    {0xc0f9u, 1u, 6u, 2u}, {0xc0fau, 2u, 2u, 0u},
    {0xc0fcu, 3u, 6u, 2u}, {0xc0ffu, 2u, 2u, 0u},
    {0xc101u, 2u, 2u, 0u}, {0xc103u, 4u, 5u, 1u},
    {0xc107u, 1u, 6u, 2u}, {0xc1f2u, 3u, 6u, 2u},
    {0xc1f5u, 3u, 6u, 2u}, {0xc1f8u, 2u, 3u, 0u},
    {0xc1fau, 2u, 2u, 0u}, {0xc1fcu, 2u, 3u, 0u},
    {0xc206u, 2u, 3u, 1u}, {0xc208u, 4u, 5u, 1u},
    {0xc20cu, 1u, 2u, 0u}, {0xc20du, 2u, 2u, 0u},
    {0xc20fu, 4u, 8u, 3u}, {0xc219u, 2u, 3u, 0u},
    {0xc21bu, 3u, 6u, 2u}, {0xc21eu, 3u, 6u, 2u},
    {0xc221u, 4u, 8u, 3u}, {0xc225u, 3u, 6u, 2u},
    {0xc228u, 4u, 5u, 1u}, {0xc22cu, 2u, 3u, 0u},
    {0xc22eu, 2u, 2u, 0u}, {0xc230u, 2u, 2u, 0u},
    {0xc232u, 3u, 6u, 2u}, {0xc235u, 2u, 3u, 1u},
    {0xc237u, 4u, 8u, 3u}, {0xc23bu, 3u, 6u, 2u},
    {0xc23eu, 2u, 2u, 0u}, {0xc240u, 3u, 6u, 2u},
    {0xc243u, 2u, 3u, 0u}, {0xc246u, 1u, 2u, 0u},
    {0xc247u, 2u, 3u, 1u}, {0xc249u, 1u, 2u, 0u},
    {0xc24au, 4u, 5u, 1u}, {0xc24eu, 1u, 6u, 2u},
    {0xd350u, 2u, 3u, 1u}, {0xd352u, 2u, 3u, 1u},
    {0xd354u, 4u, 5u, 1u}, {0xd358u, 3u, 4u, 1u},
    {0xd35bu, 2u, 2u, 0u}, {0xd35du, 3u, 5u, 1u},
    {0xd360u, 3u, 4u, 1u}, {0xd363u, 2u, 2u, 0u},
    {0xd365u, 2u, 2u, 0u}, {0xd367u, 1u, 2u, 0u},
    {0xd368u, 2u, 3u, 1u}, {0xd36au, 2u, 2u, 0u},
    {0xd36cu, 2u, 2u, 0u}, {0xd36eu, 1u, 2u, 0u},
    {0xd36fu, 1u, 2u, 0u}, {0xd370u, 3u, 8u, 4u},
    {0xd373u, 2u, 2u, 0u}, {0xd375u, 2u, 3u, 1u},
    {0xd377u, 3u, 4u, 1u}, {0xd37au, 2u, 3u, 1u},
    {0xd37cu, 3u, 4u, 1u}, {0xd37fu, 2u, 3u, 1u},
    {0xd381u, 1u, 2u, 0u}, {0xd382u, 2u, 3u, 1u},
    {0xd384u, 1u, 2u, 0u}, {0xd385u, 4u, 5u, 1u},
    {0xd389u, 4u, 8u, 3u}, {0xd39du, 2u, 3u, 1u},
    {0xd39fu, 2u, 3u, 1u}, {0xd3a1u, 3u, 6u, 2u},
    {0xd3a4u, 2u, 3u, 1u}, {0xd3a6u, 2u, 2u, 0u},
    {0xd3a8u, 3u, 4u, 1u}, {0xd3abu, 3u, 5u, 1u},
    {0xd3aeu, 1u, 6u, 3u}, {0xd3b7u, 2u, 3u, 1u},
    {0xd3b9u, 3u, 4u, 1u}, {0xd3bcu, 1u, 2u, 0u},
    {0xd3bdu, 2u, 2u, 0u}, {0xd3bfu, 1u, 2u, 0u},
    {0xd3c0u, 4u, 5u, 1u}, {0xd3c4u, 1u, 6u, 2u},
    {0xd3c5u, 2u, 3u, 1u}, {0xd3c7u, 3u, 4u, 1u},
    {0xd3cau, 1u, 2u, 0u}, {0xd3cbu, 2u, 2u, 0u},
    {0xd3cdu, 4u, 5u, 1u}, {0xd3d1u, 1u, 2u, 0u},
    {0xd3d2u, 1u, 2u, 0u}, {0xd3d3u, 3u, 4u, 1u},
    {0xd3d6u, 1u, 6u, 2u}, {0xd3d7u, 2u, 3u, 1u},
    {0xd3d9u, 3u, 4u, 1u}, {0xd3dcu, 1u, 2u, 0u},
    {0xd3ddu, 2u, 2u, 0u}, {0xd3dfu, 1u, 2u, 0u},
    {0xd3e0u, 4u, 5u, 1u}, {0xd3e4u, 1u, 6u, 2u},
    {0xd3e5u, 2u, 3u, 1u}, {0xd3e7u, 3u, 4u, 1u},
    {0xd3eau, 1u, 2u, 0u}, {0xd3ebu, 2u, 2u, 0u},
    {0xd3edu, 4u, 5u, 1u}, {0xd3f1u, 1u, 2u, 0u},
    {0xd3f2u, 1u, 2u, 0u}, {0xd3f3u, 3u, 4u, 1u},
    {0xd3f6u, 1u, 6u, 2u}, {0xd3f7u, 1u, 3u, 0u},
    {0xd3f8u, 2u, 2u, 0u}, {0xd3fau, 1u, 3u, 0u},
    {0xd3fbu, 2u, 3u, 0u}, {0xd3fdu, 1u, 2u, 0u},
    {0xd3feu, 1u, 2u, 0u}, {0xd3ffu, 4u, 6u, 2u},
    {0xd403u, 2u, 3u, 1u}, {0xd405u, 1u, 2u, 0u},
    {0xd406u, 3u, 3u, 0u}, {0xd409u, 4u, 6u, 2u},
    {0xd40du, 2u, 3u, 0u}, {0xd40fu, 2u, 2u, 0u},
    {0xd411u, 4u, 5u, 1u}, {0xd415u, 1u, 6u, 2u},
    {0xf9adu, 2u, 3u, 1u}, {0xf9afu, 2u, 3u, 1u},
    {0xf9b1u, 2u, 3u, 1u}, {0xf9b3u, 1u, 3u, 0u},
    {0xf9b4u, 2u, 3u, 1u}, {0xf9b6u, 4u, 5u, 1u},
    {0xf9bau, 4u, 5u, 1u}, {0xf9beu, 4u, 5u, 1u},
    {0xf9c2u, 2u, 2u, 0u}, {0xf9c4u, 1u, 3u, 0u},
    {0xf9c5u, 2u, 3u, 0u}, {0xf9c7u, 1u, 2u, 0u},
    {0xf9c8u, 4u, 6u, 2u}, {0xf9ccu, 1u, 2u, 0u},
    {0xf9cdu, 2u, 3u, 0u}, {0xf9cfu, 1u, 6u, 2u},
    {0xfb12u, 1u, 3u, 0u}, {0xfb13u, 2u, 2u, 0u},
    {0xfb15u, 1u, 3u, 0u}, {0xfb16u, 1u, 2u, 0u},
    {0xfb17u, 3u, 6u, 2u}, {0xfb22u, 2u, 5u, 2u},
    {0xfb24u, 1u, 6u, 3u}, {0xfb25u, 2u, 5u, 2u},
    {0xfb27u, 1u, 6u, 3u}, {0xfb28u, 2u, 5u, 2u},
    {0xfb2au, 1u, 6u, 3u}, {0xfb2bu, 2u, 5u, 2u},
    {0xfb2du, 1u, 6u, 3u}, {0xfbbdu, 1u, 3u, 0u},
    {0xfbbeu, 2u, 2u, 0u}, {0xfbc0u, 1u, 3u, 0u},
    {0xfbc1u, 1u, 2u, 0u}, {0xfbc2u, 3u, 8u, 4u},
    {0xfbc5u, 1u, 6u, 2u}, {0xfbceu, 2u, 5u, 2u},
    {0xfbd0u, 3u, 6u, 2u}, {0xfbd3u, 2u, 2u, 0u},
    {0xfbd5u, 2u, 3u, 0u}, {0xfbd7u, 3u, 6u, 2u},
    {0xfbdau, 2u, 2u, 0u}, {0xfbdcu, 2u, 3u, 0u},
    {0xfbdeu, 3u, 6u, 2u}, {0xfbe1u, 2u, 2u, 0u},
    {0xfbe3u, 2u, 3u, 0u}, {0xfbe5u, 2u, 5u, 2u},
    {0xfbe7u, 3u, 6u, 2u}, {0xfbeau, 2u, 2u, 0u},
    {0xfbecu, 1u, 2u, 0u}, {0xfbedu, 2u, 2u, 0u},
    {0xfbefu, 1u, 2u, 0u}, {0xfbf0u, 1u, 6u, 2u},
    {0xfbf1u, 2u, 3u, 1u}, {0xfbf3u, 1u, 3u, 0u},
    {0xfbf4u, 2u, 3u, 1u}, {0xfbf6u, 3u, 6u, 2u},
    {0xfbf9u, 4u, 5u, 1u}, {0xfbfdu, 1u, 6u, 2u},
    {0xfc69u, 3u, 4u, 1u}, {0xfc6cu, 2u, 2u, 0u},
    {0xfc6eu, 2u, 2u, 0u}, {0xfc70u, 2u, 3u, 1u},
    {0xfc72u, 2u, 2u, 0u}, {0xfc74u, 2u, 2u, 0u},
    {0xfc76u, 4u, 5u, 1u}, {0xfc7au, 2u, 3u, 0u},
    {0xfc7cu, 4u, 5u, 1u}, {0xfc80u, 2u, 2u, 0u},
    {0xfc82u, 2u, 2u, 0u}, {0xfc84u, 2u, 2u, 0u},
    {0xfc86u, 4u, 5u, 1u}, {0xfc8au, 1u, 6u, 2u},
};
enum {
    PLAYER_STEP_MAX_OPCODES = 256u,
    PLAYER_STEP_DEBUG_INPUT = 0x057cu,
    PLAYER_STEP_INPUT = 0x47u,
    PLAYER_STEP_DIRECTION = 0x22u,
    PLAYER_STEP_PROBE_X = 0x8fu,
    PLAYER_STEP_PROBE_Y = 0x91u,
    PLAYER_STEP_SLOT = 0xa7u,
    PLAYER_STEP_RECORD_OFFSET = 0xabu,
    PLAYER_STEP_ACTOR_STATE = 0x0622u,
    PLAYER_STEP_TILE_X = 0x06bau,
    PLAYER_STEP_TILE_Y = 0x06e2u,
    PLAYER_STEP_MAP_WIDTH = 0x05b9u,
    PLAYER_STEP_MAP_ATTRIBUTES = 0x4000u,
    PLAYER_STEP_ACTION = 0x1e4deu,
    PLAYER_STEP_CONTROL_LATCH = 0x099cu,
    PLAYER_STEP_BOX_MIN_X = 0x1e5f6u,
    PLAYER_STEP_BOX_MIN_Y = 0x1e61eu,
    PLAYER_STEP_BOX_MAX_X = 0x1e646u,
    PLAYER_STEP_BOX_MAX_Y = 0x1e66eu,
    PLAYER_STEP_CODE_TABLE = 0x1c1b0u
};

typedef struct PlayerStepPlan {
    FixedSpanOpcode opcodes[PLAYER_STEP_MAX_OPCODES];
    unsigned count;
    uint32_t landing;
} PlayerStepPlan;

static bool AppendPlayerStepOps(PlayerStepPlan *plan, unsigned first, unsigned last) {
    const unsigned count = last - first + 1u;
    if (count > PLAYER_STEP_MAX_OPCODES - plan->count)
        return false;
    memcpy(plan->opcodes + plan->count, player_step_timing + first,
        count * sizeof *plan->opcodes);
    plan->count += count;
    return true;
}

static bool AppendPlayerStepBranch(PlayerStepPlan *plan, unsigned opcode, bool taken) {
    if (!AppendPlayerStepOps(plan, opcode, opcode))
        return false;
    plan->opcodes[plan->count - 1u].cycles = 2u + taken;
    return true;
}

#define PLAYER_OPS(first, last) \
    do { if (!AppendPlayerStepOps(plan, PLAYER_CLOCK_##first, PLAYER_CLOCK_##last)) return false; } while (0)
#define PLAYER_BRANCH(pc, taken) \
    do { if (!AppendPlayerStepBranch(plan, PLAYER_CLOCK_##pc, taken)) return false; } while (0)

static bool PlanPlayerStepMovement(PlayerStepPlan *plan, unsigned step) {
    PLAYER_OPS(FB12, FB17);
    if (step == 0u) PLAYER_OPS(FB22, FB24);
    else if (step == 2u) PLAYER_OPS(FB25, FB27);
    else if (step == 4u) PLAYER_OPS(FB28, FB2A);
    else PLAYER_OPS(FB2B, FB2D);
    return true;
}

static bool PlanPlayerStepSecondaryScript(PlayerStepPlan *plan) {
    PLAYER_OPS(D39D, D3A1);
    PLAYER_OPS(D3F7, D415);
    PLAYER_OPS(D3A4, D3AE);
    return true;
}

static bool PlanPlayerStepAction(PlayerStepPlan *plan, const uint8_t *ram,
    unsigned action, unsigned step) {
    PLAYER_OPS(D350, D363);
    const bool secondary = (ram[PLAYER_STEP_ACTOR_STATE] & 0x28u) != 0u;
    PLAYER_BRANCH(D365, secondary);
    if (secondary)
        return PlanPlayerStepSecondaryScript(plan);
    PLAYER_OPS(D367, D36A);
    PLAYER_BRANCH(D36C, action >= 4u);
    if (action >= 4u)
        return PlanPlayerStepSecondaryScript(plan);
    PLAYER_OPS(D36E, D370);
    const unsigned coordinate = ram[action < 2u ? PLAYER_STEP_TILE_Y : PLAYER_STEP_TILE_X];
    bool within_bounds = false;
    if (coordinate) {
        if (action == 0u || action == 2u) {
            const unsigned edge = ram[action == 0u ? PLAYER_STEP_BOX_MIN_Y : PLAYER_STEP_BOX_MIN_X];
            within_bounds = coordinate - 1u >= edge;
        } else {
            const uint8_t edge = (uint8_t)(ram[action == 1u ?
                PLAYER_STEP_BOX_MAX_Y : PLAYER_STEP_BOX_MAX_X] - 2u);
            within_bounds = edge >= coordinate;
        }
    }
    switch (action) {
    case 0u:
        PLAYER_OPS(D3B7, D3BC);
        PLAYER_BRANCH(D3BD, !coordinate);
        if (coordinate) PLAYER_OPS(D3BF, D3C0);
        PLAYER_OPS(D3C4, D3C4);
        break;
    case 1u:
        PLAYER_OPS(D3C5, D3CA);
        PLAYER_BRANCH(D3CB, !coordinate);
        if (coordinate) PLAYER_OPS(D3CD, D3D3);
        PLAYER_OPS(D3D6, D3D6);
        break;
    case 2u:
        PLAYER_OPS(D3D7, D3DC);
        PLAYER_BRANCH(D3DD, !coordinate);
        if (coordinate) PLAYER_OPS(D3DF, D3E0);
        PLAYER_OPS(D3E4, D3E4);
        break;
    default:
        PLAYER_OPS(D3E5, D3EA);
        PLAYER_BRANCH(D3EB, !coordinate);
        if (coordinate) PLAYER_OPS(D3ED, D3F3);
        PLAYER_OPS(D3F6, D3F6);
        break;
    }
    PLAYER_BRANCH(D373, !within_bounds);
    if (!within_bounds) {
        PLAYER_OPS(D3AE, D3AE);
        return true;
    }
    PLAYER_OPS(D375, D389);
    if (!PlanPlayerStepMovement(plan, step))
        return false;
    plan->landing = 0x83d38du;
    return true;
}

static bool PlanPlayerStep(const CpuState *cpu, const uint8_t *rom, size_t rom_size,
    PlayerStepPlan *plan) {
    plan->count = 0u;
    plan->landing = 0x83c245u;
    if (!cpu || !cpu->ram || !rom || rom_size <= PLAYER_STEP_CODE_TABLE + 3u)
        return false;
    const uint8_t *ram = cpu->ram;
    if (ram[PLAYER_STEP_SLOT] || ram[PLAYER_STEP_RECORD_OFFSET] ||
        ram[PLAYER_STEP_RECORD_OFFSET + 1u])
        return false;
    const unsigned direction = ram[PLAYER_STEP_DIRECTION];
    if (direction > 3u)
        return false;
    const unsigned step = rom[PLAYER_STEP_CODE_TABLE + direction];
    if (step > 6u || (step & 1u))
        return false;
    const unsigned width = ram[PLAYER_STEP_MAP_WIDTH];
    unsigned probe_x = ram[PLAYER_STEP_PROBE_X], probe_y = ram[PLAYER_STEP_PROBE_Y];
    if (step == 0u) probe_y = (probe_y + 1u) & 0xffu;
    else if (step == 6u) probe_x = (probe_x + 1u) & 0xffu;
    unsigned cell = probe_x + probe_y * width;
    const unsigned mask = (step == 0u || step == 4u) ? 0x10u : 0x20u;
    const bool blocked = (ram[PLAYER_STEP_MAP_ATTRIBUTES + cell] & mask) != 0u;
    PLAYER_OPS(C1F2, C1F2);
    PLAYER_OPS(C246, C24E);
    PLAYER_OPS(C1F5, C1F5);
    PLAYER_OPS(FBBD, FBC2);
    if (step == 0u) PLAYER_OPS(FBCE, FBD0);
    else if (step == 2u) PLAYER_OPS(FBD7, FBD7);
    else if (step == 4u) PLAYER_OPS(FBDE, FBDE);
    else PLAYER_OPS(FBE5, FBE7);
    PLAYER_OPS(FBF1, FBF6);
    PLAYER_OPS(F9B6, F9CF);
    PLAYER_OPS(FBF9, FBFD);
    if (step == 0u) PLAYER_OPS(FBD3, FBD5);
    else if (step == 2u) PLAYER_OPS(FBDA, FBDC);
    else if (step == 4u) PLAYER_OPS(FBE1, FBE3);
    else PLAYER_OPS(FBEA, FBEA);
    PLAYER_OPS(FBEC, FBEC);
    PLAYER_BRANCH(FBED, !blocked);
    if (blocked) PLAYER_OPS(FBEF, FBEF);
    PLAYER_OPS(FBF0, FBF0);
    PLAYER_OPS(FBC5, FBC5);
    PLAYER_OPS(C1F8, C1F8);
    PLAYER_BRANCH(C1FA, !blocked);
    if (blocked) {
        PLAYER_OPS(C1FC, C1FC);
        plan->landing = 0x83c1feu;
        return true;
    }
    unsigned x = ram[PLAYER_STEP_TILE_X], y = ram[PLAYER_STEP_TILE_Y];
    if (step == 0u) y = (y + 1u) & 0xffu;
    else if (step == 2u) x = (x - 1u) & 0xffu;
    else if (step == 4u) y = (y - 1u) & 0xffu;
    else x = (x + 1u) & 0xffu;
    cell = x + y * width;
    const bool turn = (ram[PLAYER_STEP_MAP_ATTRIBUTES + cell] & 1u) != 0u;
    PLAYER_OPS(C219, C21B);
    PLAYER_OPS(C0EF, C0F9);
    PLAYER_OPS(C21E, C21E);
    PLAYER_OPS(C246, C24E);
    PLAYER_OPS(C221, C221);
    if (!PlanPlayerStepMovement(plan, step))
        return false;
    PLAYER_OPS(C225, C225);
    PLAYER_OPS(F9AD, F9CF);
    PLAYER_OPS(C228, C22E);
    PLAYER_BRANCH(C230, turn);
    if (turn) {
        PLAYER_OPS(C206, C20F);
        if (!PlanPlayerStepAction(plan, ram, direction + 0x18u, step))
            return false;
        plan->landing = 0x83c213u;
        return true;
    }
    PLAYER_OPS(C232, C232);
    PLAYER_OPS(FC69, FC69);
    const bool debug = ram[PLAYER_STEP_DEBUG_INPUT] != 0u;
    const bool run_held = (ram[PLAYER_STEP_INPUT] & 0x80u) != 0u;
    PLAYER_BRANCH(FC6C, !debug);
    if (debug) {
        PLAYER_OPS(FC6E, FC70);
        PLAYER_BRANCH(FC72, !run_held);
        if (run_held) {
            PLAYER_OPS(FC74, FC7A);
        } else {
            PLAYER_OPS(FC7C, FC80);
            const bool keep_action = ram[PLAYER_STEP_ACTION] != 0x10u;
            PLAYER_BRANCH(FC82, keep_action);
            if (!keep_action)
                PLAYER_OPS(FC84, FC86);
        }
    }
    PLAYER_OPS(FC8A, FC8A);
    PLAYER_OPS(C235, C237);
    if (!PlanPlayerStepAction(plan, ram, direction, step))
        return false;
    if (plan->landing == 0x83d38du)
        return true;
    PLAYER_OPS(C23B, C23B);
    PLAYER_OPS(C0FA, C0FC);
    const bool acknowledge = (ram[PLAYER_STEP_CONTROL_LATCH] & 0x20u) != 0u;
    PLAYER_BRANCH(C0FF, !acknowledge);
    if (acknowledge)
        PLAYER_OPS(C101, C103);
    PLAYER_OPS(C107, C107);
    PLAYER_OPS(C23E, C243);
    return true;
}
#undef PLAYER_OPS
#undef PLAYER_BRANCH

typedef struct PlayerStepClock {
    FixedSpanClock span;
    const PlayerStepPlan *plan;
    unsigned offset;
    unsigned last_progress;
    uint16_t product;
} PlayerStepClock;

static void PreparePlayerStepClock(PlayerStepClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.index < clock->span.count || !clock->span.count)
            return;
        for (unsigned opcode = 0u, mask = clock->span.progress_mask;
             mask; ++opcode, mask >>= 1u)
            if (mask & 1u)
                clock->last_progress = clock->offset + opcode + 1u;
        clock->offset += clock->span.count;
        const unsigned remaining = clock->plan->count - clock->offset;
        clock->span = (FixedSpanClock){.cpu = clock->span.cpu, .bank = 0x830000u,
            .timing = clock->plan->opcodes + clock->offset,
            .count = remaining < 32u ? remaining : 32u};
    }
}

static uint8_t PlayerStepRead(void *context, uint32_t address) {
    PlayerStepClock *clock = context;
    PreparePlayerStepClock(clock);
    FixedSpanClock *span = &clock->span;
    assert(span->index < span->count);
    if (span->timing[span->index].pc != 0xf9c8u)
        return FixedSpanRead(span, address);
    bool progress = false;
    uint8_t value;
    /* The original multiplier result uses one paired hardware read. */
    if (span->remaining == 2u) {
        assert(address == 0x4216u);
        clock->product = Lufia2DecompFrameReturnReadWord(
            span->cpu, address, address + 1u, &progress);
        value = (uint8_t)clock->product;
    } else {
        assert(span->remaining == 1u && address == 0x4217u);
        value = (uint8_t)(clock->product >> 8);
    }
    span->data_master += cpu_region_speed(address);
    span->progress |= progress;
    if (!--span->remaining)
        FinishFixedSpanOpcode(span);
    return value;
}

static void PlayerStepWrite(void *context, uint32_t address, uint8_t value) {
    PlayerStepClock *clock = context;
    PreparePlayerStepClock(clock);
    FixedSpanWrite(&clock->span, address, value);
}

static unsigned PlayerStepReserve(const PlayerStepPlan *plan) {
    unsigned master = 0u;
    for (unsigned opcode = 0u; opcode < plan->count; ++opcode) {
        const FixedSpanOpcode *timing = plan->opcodes + opcode;
        const unsigned bus = timing->fetched + timing->data_bytes;
        master += 8u * bus + 6u * (timing->cycles - bus);
    }
    return master + 80u * (master / 1284u + 2u);
}

static void ActorBridgePlayerStep(CpuState *cpu, uint32_t entry);

static bool PlayerStepHookOwned(uint32_t pc, InterpPreOpcodeHook hook) {
    return pc == 0x83c1f2u && hook == ActorBridgePlayerStep;
}

static bool PlayerStepWindowSupported(const CpuState *cpu, unsigned reserve) {
    if (cpu != &g_cpu || !cpu->ram || cpu->emulation || cpu->_flag_D || cpu->D ||
        cpu->PB != 0x83u || cpu->DB != 0x83u || cpu->m_flag != 1u || cpu->x_flag != 0u ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f20u || cpu->S > 0x1ffcu ||
        !Lufia2DecompFrameReturnCpuReady() ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - reserve ||
        !ActorBridgeTimedStepSupportedWindow(cpu, 0x83c1f2u, 0x1ffcu, false, 1363u, reserve) ||
        !g_snes->apu || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize <= 0x88037u ||
        !g_snes->hdmaBeamOff || g_snes->dma->hdmaPendingInit || g_snes->vPos >= 262u)
        return false;
    const unsigned end_line = g_snes->vPos < 225u ? 225u : 262u;
    if ((end_line - g_snes->vPos) * 1364u - g_snes->hPos < reserve)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static bool PreparePlayerStep(const CpuState *cpu, PlayerStepPlan *plan,
    unsigned *reserve) {
    if (!PlayerStepWindowSupported(cpu, 0u))
        return false;
    const unsigned budget = Lufia2DecompFrameOwnedSpanBudget(
        0x83c1f2u, 0x83c245u, ActorBridgePlayerStep);
    static const uint32_t children[][2] = {
        {0x83c0efu, 0x83c108u}, {0x83c246u, 0x83c24fu},
        {0x83d350u, 0x83d416u}, {0x83f9adu, 0x83f9d0u},
        {0x83fb12u, 0x83fb2eu}, {0x83fbbdu, 0x83fbfeu},
        {0x83fc69u, 0x83fc8bu}
    };
    if (!budget)
        return false;
    for (unsigned child = 0u; child < sizeof children / sizeof *children; ++child)
        if (!Lufia2DecompFrameRangeQuiet(children[child][0], children[child][1]))
            return false;
    if (!PlanPlayerStep(cpu, g_snes->cart->rom, g_snes->cart->romSize, plan) ||
        plan->count > budget)
        return false;
    *reserve = PlayerStepReserve(plan);
    return PlayerStepWindowSupported(cpu, *reserve);
}

static void ActorBridgePlayerStep(CpuState *cpu, uint32_t entry) {
    if (entry != 0x83c1f2u)
        return;
    PlayerStepPlan plan;
    unsigned reserve;
    if (!PreparePlayerStep(cpu, &plan, &reserve))
        return;
    const uint64_t start = cpu->master_cycles;
    PlayerStepClock clock = {.span = {.cpu = cpu, .bank = 0x830000u,
        .timing = plan.opcodes, .count = plan.count < 32u ? plan.count : 32u},
        .plan = &plan};
    const Lufia2Memory memory = {PlayerStepRead, PlayerStepWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2PlayerSlotResumeStep(&memory, &state);
    PreparePlayerStepClock(&clock);
    assert(clock.offset == plan.count && !clock.span.remaining);
    assert(result.flow == (plan.landing == 0x83c245u ? LUFIA2_EXECUTION_RETURNED : LUFIA2_EXECUTION_BOUNDARY) && result.pc == plan.landing);
    ActorBridgeStore(cpu, &state);
    cpu->host_return_valid = 2u;
    assert(cpu->master_cycles - start <= reserve);
    Lufia2DecompFrameReturnCompleteSpan(plan.count, clock.last_progress);
    interp_bridge_pre_opcode_redirect(result.pc);
}
