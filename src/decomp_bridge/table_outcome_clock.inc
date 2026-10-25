enum { TABLE_TILEMAP_WORDS = 0x0400u };

typedef struct TableTilemapClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[6];
    unsigned phase;
    unsigned iteration;
} TableTilemapClock;

static void TableTilemapPrelude(TableTilemapClock *clock) {
    static const FixedSpanOpcode timing[] = {
        {0xf01au,1u,4u,2u},{0xf01bu,1u,4u,2u},{0xf01cu,1u,3u,1u},
        {0xf01du,2u,3u,0u},{0xf01fu,3u,3u,0u},{0xf022u,3u,3u,0u}
    };
    for (unsigned i=0u;i<6u;++i) clock->timing[i]=timing[i];
    clock->span.count=6u;
    clock->span.index=clock->span.remaining=0u;
}

static void TableTilemapIteration(TableTilemapClock *clock) {
    const unsigned offset=clock->iteration*2u;
    clock->timing[0]=(FixedSpanOpcode){0xf025u,3u,(uint8_t)(5u+(offset>=256u)),2u};
    clock->timing[1]=(FixedSpanOpcode){0xf028u,3u,7u,2u};
    clock->timing[2]=(FixedSpanOpcode){0xf02bu,1u,2u,0u};
    clock->timing[3]=(FixedSpanOpcode){0xf02cu,1u,2u,0u};
    clock->timing[4]=(FixedSpanOpcode){0xf02du,1u,2u,0u};
    clock->timing[5]=(FixedSpanOpcode){0xf02eu,2u,
        (uint8_t)(clock->iteration+1u==TABLE_TILEMAP_WORDS?2u:3u),0u};
    clock->span.count=6u;
    clock->span.index=clock->span.remaining=0u;
}

static void TableTilemapTail(TableTilemapClock *clock) {
    clock->timing[0]=(FixedSpanOpcode){0xf030u,1u,4u,1u};
    clock->timing[1]=(FixedSpanOpcode){0xf031u,1u,5u,2u};
    clock->timing[2]=(FixedSpanOpcode){0xf032u,1u,5u,2u};
    clock->span.count=3u;
    clock->span.index=clock->span.remaining=0u;
}

static void TableTilemapDataReady(TableTilemapClock *clock) {
    for (;;) {
        BeginFixedSpanOpcode(&clock->span);
        if (clock->span.remaining) return;
        assert(clock->span.index==clock->span.count);
        if (clock->phase==0u) {
            clock->phase=1u;
            TableTilemapIteration(clock);
        } else if (clock->phase==1u) {
            ++clock->iteration;
            if (clock->iteration<TABLE_TILEMAP_WORDS) TableTilemapIteration(clock);
            else {
                clock->phase=2u;
                TableTilemapTail(clock);
            }
        } else {
            assert(false);
            return;
        }
    }
}

static uint8_t TableTilemapRead(void *context,uint32_t address) {
    TableTilemapClock *clock=context;
    TableTilemapDataReady(clock);
    return FixedSpanRead(&clock->span,address);
}

static void TableTilemapWrite(void *context,uint32_t address,uint8_t value) {
    TableTilemapClock *clock=context;
    TableTilemapDataReady(clock);
    FixedSpanWrite(&clock->span,address,value);
}

static bool TableTilemapAdmitted(CpuState *cpu,const ActorBridgeFrame *frame) {
    const uint64_t budget=188000u;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type!=CART_LOROM ||
        g_snes->cart->romSize<0x28000u || cpu->PB!=0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag || cpu->x_flag ||
        cpu->host_return_valid!=2u || frame->hrv!=2u || cpu->S<0x1f20u ||
        cpu->S>0x1ff7u || (cpu->DB!=0x7eu && cpu->DB!=0x7fu) ||
        cpu->coprocessor_master_cycles!=cpu->master_cycles ||
        cpu->master_cycles>UINT64_MAX-budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos>=262u || g_snes->hPos>=1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu,0x84f01au,0x1ff7u,true,1363u,budget))
        return false;
    const unsigned lines=(g_snes->vPos<225u?225u:262u)-g_snes->vPos;
    if ((uint64_t)lines*1364u-g_snes->hPos<=budget) return false;
    uint64_t phase,current;
    snes_refresh_state_get(&phase,&current);
    return phase<1364u && (!current || current==cpu->master_cycles);
}

static RecompReturn ActorBridgeTableRestoreTilemap(CpuState *cpu) {
    const ActorBridgeFrame frame=ActorBridgeEnter(cpu);
    if (!TableTilemapAdmitted(cpu,&frame)) return ActorBridgeFallback(cpu,&frame,0x84f01au);
    TableTilemapClock clock;
    clock.span=(FixedSpanClock){0};
    clock.span.cpu=cpu;
    clock.span.bank=0x840000u;
    clock.span.timing=clock.timing;
    clock.phase=clock.iteration=0u;
    TableTilemapPrelude(&clock);
    const Lufia2Memory memory={TableTilemapRead,TableTilemapWrite,&clock,NULL,NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu,&state);
    const Lufia2ExecutionResult result=Lufia2CardTableRestoreTilemap(&memory,&state);
    ActorBridgeStore(cpu,&state);
    if (result.flow!=LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu,result.pc,result.pc,frame.entry_s,frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.pc==0x84f033u && clock.phase==2u &&
        clock.iteration==TABLE_TILEMAP_WORDS && clock.span.index==clock.span.count &&
        !clock.span.remaining);
    return ReturnTimedMenuCall(cpu,&frame,result.pc);
}

typedef struct TableOutcomeOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index;
    int8_t advance_x, advance_y;
} TableOutcomeOpcode;

enum {
    OUTCOME_PC_A60C = 0u,
    OUTCOME_PC_A610 = 1u,
    OUTCOME_PC_DEB9 = 2u,
    OUTCOME_PC_DEBC = 3u,
    OUTCOME_PC_DEBF = 4u,
    OUTCOME_PC_DEC0 = 5u,
    OUTCOME_PC_DEC2 = 6u,
    OUTCOME_PC_DEC5 = 7u,
    OUTCOME_PC_DEC8 = 8u,
    OUTCOME_PC_DECA = 9u,
    OUTCOME_PC_DECB = 10u,
    OUTCOME_PC_DECD = 11u,
    OUTCOME_PC_DED0 = 12u,
    OUTCOME_PC_DED3 = 13u,
    OUTCOME_PC_DED5 = 14u,
    OUTCOME_PC_DED8 = 15u,
    OUTCOME_PC_DEDB = 16u,
    OUTCOME_PC_DEDC = 17u,
    OUTCOME_PC_DEDF = 18u,
    OUTCOME_PC_DEE2 = 19u,
    OUTCOME_PC_DEE4 = 20u,
    OUTCOME_PC_DEE7 = 21u,
    OUTCOME_PC_DEEA = 22u,
    OUTCOME_PC_DEED = 23u,
    OUTCOME_PC_DEF0 = 24u,
    OUTCOME_PC_DEF3 = 25u,
    OUTCOME_PC_DEF6 = 26u,
    OUTCOME_PC_DEF8 = 27u,
    OUTCOME_PC_DEFB = 28u,
    OUTCOME_PC_DEFE = 29u,
    OUTCOME_PC_DF01 = 30u,
    OUTCOME_PC_DF04 = 31u,
    OUTCOME_PC_DF06 = 32u,
    OUTCOME_PC_DF09 = 33u,
    OUTCOME_PC_DF0C = 34u,
    OUTCOME_PC_DF0F = 35u,
    OUTCOME_PC_DF12 = 36u,
    OUTCOME_PC_DF14 = 37u,
    OUTCOME_PC_DF17 = 38u,
    OUTCOME_PC_DF1A = 39u,
    OUTCOME_PC_DF1C = 40u,
    OUTCOME_PC_DF1F = 41u,
    OUTCOME_PC_DF21 = 42u,
    OUTCOME_PC_DF24 = 43u,
    OUTCOME_PC_DF27 = 44u,
    OUTCOME_PC_DF29 = 45u,
    OUTCOME_PC_DF2C = 46u,
    OUTCOME_PC_DF30 = 47u,
    OUTCOME_PC_DF33 = 48u,
    OUTCOME_PC_DF35 = 49u,
    OUTCOME_PC_DF38 = 50u,
    OUTCOME_PC_DF3C = 51u,
    OUTCOME_PC_DF3F = 52u,
    OUTCOME_PC_DF41 = 53u,
    OUTCOME_PC_DF44 = 54u,
    OUTCOME_PC_DF46 = 55u,
    OUTCOME_PC_DF49 = 56u,
    OUTCOME_PC_DF4B = 57u,
    OUTCOME_PC_DF4D = 58u,
    OUTCOME_PC_DF50 = 59u,
    OUTCOME_PC_DF54 = 60u,
    OUTCOME_PC_DF57 = 61u,
    OUTCOME_PC_DF59 = 62u,
    OUTCOME_PC_DF5C = 63u,
    OUTCOME_PC_DF5E = 64u,
    OUTCOME_PC_DF61 = 65u,
    OUTCOME_PC_DF65 = 66u,
    OUTCOME_PC_DF68 = 67u,
    OUTCOME_PC_DF6A = 68u,
    OUTCOME_PC_DF6C = 69u,
    OUTCOME_PC_DF6F = 70u,
    OUTCOME_PC_DF71 = 71u,
    OUTCOME_PC_DF74 = 72u,
    OUTCOME_PC_DF76 = 73u,
    OUTCOME_PC_DF79 = 74u,
    OUTCOME_PC_DF7B = 75u,
    OUTCOME_PC_DF7E = 76u,
    OUTCOME_PC_DF81 = 77u,
    OUTCOME_PC_DF84 = 78u,
    OUTCOME_PC_DF87 = 79u,
    OUTCOME_PC_DF8A = 80u,
    OUTCOME_PC_DF8D = 81u,
    OUTCOME_PC_DF90 = 82u,
    OUTCOME_PC_DF93 = 83u,
    OUTCOME_PC_DF96 = 84u,
    OUTCOME_PC_DF99 = 85u,
    OUTCOME_PC_DF9A = 86u,
    OUTCOME_PC_DF9B = 87u,
    OUTCOME_PC_DF9C = 88u,
    OUTCOME_PC_DFA0 = 89u,
    OUTCOME_PC_DFA4 = 90u,
    OUTCOME_PC_DFA7 = 91u,
    OUTCOME_PC_DFA9 = 92u,
    OUTCOME_PC_DFAC = 93u,
    OUTCOME_PC_DFAD = 94u,
    OUTCOME_PC_DFB0 = 95u,
    OUTCOME_PC_DFB4 = 96u,
    OUTCOME_PC_DFB7 = 97u,
    OUTCOME_PC_DFB9 = 98u,
    OUTCOME_PC_DFBC = 99u,
    OUTCOME_PC_DFBF = 100u,
    OUTCOME_PC_DFC1 = 101u,
    OUTCOME_PC_DFC4 = 102u,
    OUTCOME_PC_DFC8 = 103u,
    OUTCOME_PC_DFCB = 104u,
    OUTCOME_PC_DFCD = 105u,
    OUTCOME_PC_DFD0 = 106u,
    OUTCOME_PC_DFD3 = 107u,
    OUTCOME_PC_DFD6 = 108u,
    OUTCOME_PC_DFD9 = 109u,
    OUTCOME_PC_DFDC = 110u,
    OUTCOME_PC_DFDF = 111u,
    OUTCOME_PC_DFE2 = 112u,
    OUTCOME_PC_DFE4 = 113u,
    OUTCOME_PC_DFE7 = 114u,
    OUTCOME_PC_DFEB = 115u,
    OUTCOME_PC_DFEE = 116u,
    OUTCOME_PC_DFF0 = 117u,
    OUTCOME_PC_DFF3 = 118u,
    OUTCOME_PC_DFF5 = 119u,
    OUTCOME_PC_DFF8 = 120u,
    OUTCOME_PC_DFFA = 121u,
    OUTCOME_PC_DFFD = 122u,
    OUTCOME_PC_E001 = 123u,
    OUTCOME_PC_E004 = 124u,
    OUTCOME_PC_E006 = 125u,
    OUTCOME_PC_E009 = 126u,
    OUTCOME_PC_E00B = 127u,
    OUTCOME_PC_E00E = 128u,
    OUTCOME_PC_E010 = 129u,
    OUTCOME_PC_E013 = 130u,
    OUTCOME_PC_E017 = 131u,
    OUTCOME_PC_E01A = 132u,
    OUTCOME_PC_E01C = 133u,
    OUTCOME_PC_E01F = 134u,
    OUTCOME_PC_E022 = 135u,
    OUTCOME_PC_E025 = 136u,
    OUTCOME_PC_E026 = 137u,
    OUTCOME_PC_E027 = 138u,
    OUTCOME_PC_E028 = 139u,
    OUTCOME_PC_E02C = 140u,
    OUTCOME_PC_E030 = 141u,
    OUTCOME_PC_E033 = 142u,
    OUTCOME_PC_E035 = 143u,
    OUTCOME_PC_E038 = 144u,
    OUTCOME_PC_E039 = 145u,
    OUTCOME_PC_E03C = 146u,
    OUTCOME_PC_E040 = 147u,
    OUTCOME_PC_E043 = 148u,
    OUTCOME_PC_E045 = 149u,
    OUTCOME_PC_E048 = 150u,
    OUTCOME_PC_E04B = 151u,
    OUTCOME_PC_E04D = 152u,
    OUTCOME_PC_E050 = 153u,
    OUTCOME_PC_E054 = 154u,
    OUTCOME_PC_E057 = 155u,
    OUTCOME_PC_E059 = 156u,
    OUTCOME_PC_E05C = 157u,
    OUTCOME_PC_E05F = 158u,
    OUTCOME_PC_E062 = 159u,
    OUTCOME_PC_E065 = 160u,
    OUTCOME_PC_E068 = 161u,
    OUTCOME_PC_E06B = 162u,
    OUTCOME_PC_E06E = 163u,
    OUTCOME_PC_E070 = 164u,
    OUTCOME_PC_E073 = 165u,
    OUTCOME_PC_E077 = 166u,
    OUTCOME_PC_E07A = 167u,
    OUTCOME_PC_E07C = 168u,
    OUTCOME_PC_E07F = 169u,
    OUTCOME_PC_E081 = 170u,
    OUTCOME_PC_E084 = 171u,
    OUTCOME_PC_E086 = 172u,
    OUTCOME_PC_E089 = 173u,
    OUTCOME_PC_E08D = 174u,
    OUTCOME_PC_E090 = 175u,
    OUTCOME_PC_E092 = 176u,
    OUTCOME_PC_E095 = 177u,
    OUTCOME_PC_E098 = 178u,
    OUTCOME_PC_E09B = 179u,
    OUTCOME_PC_E09E = 180u,
    OUTCOME_PC_E0A0 = 181u,
    OUTCOME_PC_E0A3 = 182u,
    OUTCOME_PC_E0A7 = 183u,
    OUTCOME_PC_E0AA = 184u,
    OUTCOME_PC_E0AC = 185u,
    OUTCOME_PC_E0AF = 186u,
    OUTCOME_PC_E0B0 = 187u,
    OUTCOME_PC_E0B3 = 188u,
    OUTCOME_PC_E0B7 = 189u,
    OUTCOME_PC_E0BA = 190u,
    OUTCOME_PC_E0BC = 191u,
    OUTCOME_PC_E0BF = 192u,
    OUTCOME_PC_E0C3 = 193u,
    OUTCOME_PC_E0C4 = 194u,
    OUTCOME_PC_E0C7 = 195u,
    OUTCOME_PC_E0CA = 196u,
    OUTCOME_PC_E0CC = 197u,
    OUTCOME_PC_E0CD = 198u,
    OUTCOME_PC_E0D0 = 199u,
    OUTCOME_PC_E0D3 = 200u,
    OUTCOME_PC_E0D6 = 201u,
    OUTCOME_PC_E0D9 = 202u,
    OUTCOME_PC_E0DC = 203u,
    OUTCOME_PC_E0DF = 204u,
    OUTCOME_PC_E0E2 = 205u,
    OUTCOME_PC_E0E4 = 206u,
    OUTCOME_PC_E0E7 = 207u,
    OUTCOME_PC_E0EB = 208u,
    OUTCOME_PC_E0EE = 209u,
    OUTCOME_PC_E0F0 = 210u,
    OUTCOME_PC_E0F3 = 211u,
    OUTCOME_PC_E0F4 = 212u,
    OUTCOME_PC_E0F7 = 213u,
    OUTCOME_PC_E0FB = 214u,
    OUTCOME_PC_E0FE = 215u,
    OUTCOME_PC_E100 = 216u,
    OUTCOME_PC_E103 = 217u,
    OUTCOME_PC_E106 = 218u,
    OUTCOME_PC_E109 = 219u,
    OUTCOME_PC_E10C = 220u,
    OUTCOME_PC_E110 = 221u,
    OUTCOME_PC_E113 = 222u,
    OUTCOME_PC_E115 = 223u,
    OUTCOME_PC_E118 = 224u,
    OUTCOME_PC_E11C = 225u,
    OUTCOME_PC_E11F = 226u,
    OUTCOME_PC_E121 = 227u,
    OUTCOME_PC_E124 = 228u,
    OUTCOME_PC_E128 = 229u,
    OUTCOME_PC_E129 = 230u,
    OUTCOME_PC_E12C = 231u,
    OUTCOME_PC_E12F = 232u,
    OUTCOME_PC_E132 = 233u,
    OUTCOME_PC_E136 = 234u,
    OUTCOME_PC_E139 = 235u,
    OUTCOME_PC_E13B = 236u,
    OUTCOME_PC_E13E = 237u,
    OUTCOME_PC_E141 = 238u,
    OUTCOME_PC_E143 = 239u,
    OUTCOME_PC_E146 = 240u,
    OUTCOME_PC_E14A = 241u,
    OUTCOME_PC_E14D = 242u,
    OUTCOME_PC_E14F = 243u,
    OUTCOME_PC_E152 = 244u,
    OUTCOME_PC_E156 = 245u,
    OUTCOME_PC_E157 = 246u,
    OUTCOME_PC_E15A = 247u,
    OUTCOME_PC_E15D = 248u,
    OUTCOME_PC_E160 = 249u,
    OUTCOME_PC_E162 = 250u,
    OUTCOME_PC_E165 = 251u,
    OUTCOME_PC_E168 = 252u,
    OUTCOME_PC_E16B = 253u,
    OUTCOME_PC_E16E = 254u,
    OUTCOME_PC_E171 = 255u,
    OUTCOME_PC_E174 = 256u,
    OUTCOME_PC_E175 = 257u,
    OUTCOME_PC_E178 = 258u,
    OUTCOME_PC_E17B = 259u,
    OUTCOME_PC_E17E = 260u,
    OUTCOME_PC_E181 = 261u,
    OUTCOME_PC_E184 = 262u,
    OUTCOME_PC_E187 = 263u,
    OUTCOME_PC_E18A = 264u,
    OUTCOME_PC_E18D = 265u,
    OUTCOME_PC_E190 = 266u,
    OUTCOME_PC_E193 = 267u,
    OUTCOME_PC_E334 = 268u,
    OUTCOME_PC_E337 = 269u,
    OUTCOME_PC_E33A = 270u,
    OUTCOME_PC_E33D = 271u,
    OUTCOME_PC_E33E = 272u,
    OUTCOME_PC_E33F = 273u,
    OUTCOME_PC_E340 = 274u,
    OUTCOME_PC_E342 = 275u,
    OUTCOME_PC_E345 = 276u,
    OUTCOME_PC_E348 = 277u,
    OUTCOME_PC_E34A = 278u,
    OUTCOME_PC_E34C = 279u,
    OUTCOME_PC_E34E = 280u,
    OUTCOME_PC_E350 = 281u,
    OUTCOME_PC_E352 = 282u,
    OUTCOME_PC_E355 = 283u,
    OUTCOME_PC_E357 = 284u,
    OUTCOME_PC_E35A = 285u,
    OUTCOME_PC_E35D = 286u,
    OUTCOME_PC_E360 = 287u,
    OUTCOME_PC_E362 = 288u,
    OUTCOME_PC_E365 = 289u,
    OUTCOME_PC_E368 = 290u,
    OUTCOME_PC_E36A = 291u,
    OUTCOME_PC_E36B = 292u,
    OUTCOME_PC_E36E = 293u,
    OUTCOME_PC_E371 = 294u,
    OUTCOME_PC_E374 = 295u,
    OUTCOME_PC_E377 = 296u,
    OUTCOME_PC_E37A = 297u,
    OUTCOME_PC_E37D = 298u,
    OUTCOME_PC_E380 = 299u,
    OUTCOME_PC_E383 = 300u,
    OUTCOME_PC_E386 = 301u,
    OUTCOME_PC_E389 = 302u,
    OUTCOME_PC_E38C = 303u,
    OUTCOME_PC_E38E = 304u,
    OUTCOME_PC_E391 = 305u,
    OUTCOME_PC_E393 = 306u,
    OUTCOME_PC_E395 = 307u,
    OUTCOME_PC_E398 = 308u,
    OUTCOME_PC_E39A = 309u,
    OUTCOME_PC_E39C = 310u,
    OUTCOME_PC_E39E = 311u,
    OUTCOME_PC_E3A1 = 312u,
    OUTCOME_PC_E3A4 = 313u,
    OUTCOME_PC_E3A8 = 314u,
    OUTCOME_PC_E3AA = 315u,
    OUTCOME_PC_E3AD = 316u,
    OUTCOME_PC_E3B0 = 317u,
    OUTCOME_PC_E3B3 = 318u,
    OUTCOME_PC_E3B6 = 319u,
    OUTCOME_PC_E3B9 = 320u,
    OUTCOME_PC_E3BC = 321u,
    OUTCOME_PC_E3BF = 322u,
    OUTCOME_PC_E3C2 = 323u,
    OUTCOME_PC_E3C5 = 324u,
    OUTCOME_PC_E3C7 = 325u,
    OUTCOME_PC_E3C9 = 326u,
    OUTCOME_PC_E3CB = 327u,
    OUTCOME_PC_E3CE = 328u,
    OUTCOME_PC_E3D0 = 329u,
    OUTCOME_PC_E3D2 = 330u,
    OUTCOME_PC_E3D4 = 331u,
    OUTCOME_PC_E3D6 = 332u,
    OUTCOME_PC_E3D8 = 333u,
    OUTCOME_PC_E3DB = 334u,
    OUTCOME_PC_E3DD = 335u,
    OUTCOME_PC_E3E0 = 336u,
    OUTCOME_PC_E3E3 = 337u,
    OUTCOME_PC_E3E6 = 338u,
    OUTCOME_PC_E3E9 = 339u,
    OUTCOME_PC_E3EC = 340u,
    OUTCOME_PC_E3EF = 341u,
    OUTCOME_PC_E3F2 = 342u,
    OUTCOME_PC_E3F5 = 343u,
    OUTCOME_PC_E3F6 = 344u,
    OUTCOME_PC_E3F9 = 345u,
    OUTCOME_PC_E3FA = 346u,
    OUTCOME_PC_E3FB = 347u,
    OUTCOME_PC_E3FC = 348u,
    OUTCOME_PC_E3FD = 349u,
    OUTCOME_PC_E400 = 350u,
    OUTCOME_PC_E403 = 351u,
    OUTCOME_PC_E406 = 352u,
    OUTCOME_PC_E408 = 353u,
    OUTCOME_PC_E40B = 354u,
    OUTCOME_PC_E40D = 355u,
    OUTCOME_PC_E410 = 356u,
    OUTCOME_PC_E411 = 357u,
    OUTCOME_PC_E412 = 358u,
    OUTCOME_PC_E413 = 359u,
    OUTCOME_PC_E415 = 360u,
    OUTCOME_PC_E416 = 361u,
    OUTCOME_PC_E418 = 362u,
    OUTCOME_PC_E41A = 363u,
    OUTCOME_PC_E41C = 364u,
    OUTCOME_PC_E41F = 365u,
    OUTCOME_PC_E421 = 366u,
    OUTCOME_PC_E424 = 367u,
    OUTCOME_PC_E426 = 368u,
    OUTCOME_PC_E427 = 369u,
    OUTCOME_PC_E42A = 370u,
    OUTCOME_PC_E42C = 371u,
    OUTCOME_PC_E42F = 372u,
    OUTCOME_PC_E432 = 373u,
    OUTCOME_PC_E434 = 374u,
    OUTCOME_PC_E437 = 375u,
    OUTCOME_PC_E439 = 376u,
    OUTCOME_PC_E43C = 377u,
    OUTCOME_PC_E43E = 378u,
    OUTCOME_PC_E441 = 379u,
    OUTCOME_PC_E444 = 380u,
    OUTCOME_PC_E447 = 381u,
    OUTCOME_PC_E44A = 382u,
    OUTCOME_PC_E44C = 383u,
    OUTCOME_PC_E44F = 384u,
    OUTCOME_PC_E451 = 385u,
    OUTCOME_PC_E454 = 386u,
    OUTCOME_PC_E457 = 387u,
    OUTCOME_PC_E459 = 388u,
    OUTCOME_PC_E45C = 389u,
    OUTCOME_PC_E45F = 390u,
    OUTCOME_PC_E461 = 391u,
    OUTCOME_PC_E464 = 392u,
    OUTCOME_PC_E466 = 393u,
    OUTCOME_PC_E469 = 394u,
    OUTCOME_PC_E46C = 395u,
    OUTCOME_PC_E46F = 396u,
    OUTCOME_PC_E472 = 397u,
    OUTCOME_PC_E475 = 398u,
    OUTCOME_PC_E478 = 399u,
    OUTCOME_PC_E47B = 400u,
    OUTCOME_PC_E47E = 401u,
    OUTCOME_PC_E481 = 402u,
    OUTCOME_PC_E484 = 403u,
    OUTCOME_PC_E486 = 404u,
    OUTCOME_PC_E489 = 405u,
    OUTCOME_PC_E48C = 406u,
    OUTCOME_PC_E48F = 407u,
    OUTCOME_PC_E490 = 408u,
    OUTCOME_PC_E493 = 409u,
    OUTCOME_PC_E495 = 410u,
    OUTCOME_PC_E498 = 411u,
    OUTCOME_PC_E49B = 412u,
    OUTCOME_PC_E49E = 413u,
    OUTCOME_PC_E4A0 = 414u,
    OUTCOME_PC_E4A1 = 415u,
    OUTCOME_PC_E4A2 = 416u,
    OUTCOME_PC_E4A3 = 417u,
    OUTCOME_PC_E4A5 = 418u,
    OUTCOME_PC_E4A7 = 419u,
    OUTCOME_PC_E4AA = 420u,
    OUTCOME_PC_E4AC = 421u,
    OUTCOME_PC_E4AF = 422u,
    OUTCOME_PC_E4B1 = 423u,
    OUTCOME_PC_E4B4 = 424u,
    OUTCOME_PC_E4B6 = 425u,
    OUTCOME_PC_E4B9 = 426u,
    OUTCOME_PC_E4BA = 427u,
    OUTCOME_PC_E4BB = 428u,
    OUTCOME_PC_E4BC = 429u,
    OUTCOME_PC_E4BD = 430u,
    OUTCOME_PC_E4C0 = 431u,
    OUTCOME_PC_E4C2 = 432u,
    OUTCOME_PC_E4C4 = 433u,
    OUTCOME_PC_E4C7 = 434u,
    OUTCOME_PC_E4C8 = 435u,
    OUTCOME_PC_E4C9 = 436u,
    OUTCOME_PC_E4CA = 437u,
    OUTCOME_PC_E4CB = 438u,
    OUTCOME_PC_E4CE = 439u,
    OUTCOME_PC_E4D1 = 440u,
    OUTCOME_PC_E4D3 = 441u,
    OUTCOME_PC_E4D6 = 442u,
    OUTCOME_PC_E4D8 = 443u,
    OUTCOME_PC_E4DB = 444u,
    OUTCOME_PC_E4DE = 445u,
    OUTCOME_PC_E4E0 = 446u,
    OUTCOME_PC_E4E2 = 447u,
    OUTCOME_PC_E4E4 = 448u,
    OUTCOME_PC_E4E7 = 449u,
    OUTCOME_PC_E4E9 = 450u,
    OUTCOME_PC_E4EC = 451u,
    OUTCOME_PC_E4ED = 452u,
    OUTCOME_PC_E4EF = 453u,
    OUTCOME_PC_E4F2 = 454u,
    OUTCOME_PC_E4F3 = 455u,
    OUTCOME_PC_E4F5 = 456u,
    OUTCOME_PC_E4F8 = 457u,
    OUTCOME_PC_E4FA = 458u,
    OUTCOME_PC_E4FB = 459u,
    OUTCOME_PC_E4FE = 460u,
    OUTCOME_PC_E500 = 461u,
    OUTCOME_PC_E502 = 462u,
    OUTCOME_PC_E503 = 463u,
    OUTCOME_PC_E504 = 464u,
    OUTCOME_PC_E505 = 465u,
    OUTCOME_PC_E507 = 466u,
    OUTCOME_PC_E50A = 467u,
    OUTCOME_PC_E50D = 468u,
    OUTCOME_PC_E50F = 469u,
    OUTCOME_PC_E512 = 470u,
    OUTCOME_PC_E514 = 471u,
    OUTCOME_PC_E517 = 472u,
    OUTCOME_PC_E51A = 473u,
    OUTCOME_PC_E51B = 474u,
    OUTCOME_PC_E51E = 475u,
    OUTCOME_PC_E521 = 476u,
    OUTCOME_PC_E522 = 477u,
    OUTCOME_PC_E525 = 478u,
    OUTCOME_PC_E528 = 479u,
    OUTCOME_PC_E52A = 480u,
    OUTCOME_PC_E52D = 481u,
    OUTCOME_PC_E530 = 482u,
    OUTCOME_PC_E533 = 483u,
    OUTCOME_PC_E534 = 484u,
    OUTCOME_PC_E535 = 485u,
    OUTCOME_PC_E539 = 486u,
    OUTCOME_PC_E53A = 487u,
    OUTCOME_PC_E53D = 488u,
    OUTCOME_PC_E540 = 489u,
    OUTCOME_PC_E543 = 490u,
    OUTCOME_PC_E546 = 491u,
    OUTCOME_PC_E549 = 492u,
    OUTCOME_PC_E54C = 493u,
    OUTCOME_PC_E54F = 494u,
    OUTCOME_PC_E552 = 495u,
    OUTCOME_PC_E555 = 496u,
    OUTCOME_PC_E558 = 497u,
    OUTCOME_PC_E55B = 498u,
    OUTCOME_PC_E55E = 499u,
    OUTCOME_PC_E561 = 500u,
    OUTCOME_PC_E564 = 501u,
    OUTCOME_PC_E565 = 502u,
    OUTCOME_PC_E567 = 503u,
    OUTCOME_PC_E569 = 504u,
    OUTCOME_PC_E56C = 505u,
    OUTCOME_PC_E56F = 506u,
    OUTCOME_PC_E572 = 507u,
    OUTCOME_PC_E574 = 508u,
    OUTCOME_PC_E577 = 509u,
    OUTCOME_PC_E57A = 510u,
    OUTCOME_PC_E57D = 511u,
    OUTCOME_PC_E57E = 512u,
    OUTCOME_PC_E581 = 513u,
    OUTCOME_PC_E584 = 514u,
    OUTCOME_PC_E587 = 515u,
    OUTCOME_PC_E58A = 516u,
    OUTCOME_PC_E58D = 517u,
    OUTCOME_PC_E590 = 518u,
    OUTCOME_PC_E591 = 519u,
    OUTCOME_PC_E592 = 520u,
    OUTCOME_PC_E594 = 521u,
    OUTCOME_PC_E597 = 522u,
    OUTCOME_PC_E59A = 523u,
    OUTCOME_PC_E59D = 524u,
    OUTCOME_PC_E5A0 = 525u,
    OUTCOME_PC_E5A3 = 526u,
    OUTCOME_PC_E5A6 = 527u,
    OUTCOME_PC_E5A9 = 528u,
    OUTCOME_PC_E5AC = 529u,
    OUTCOME_PC_E5AF = 530u,
    OUTCOME_PC_E5B2 = 531u,
    OUTCOME_PC_F01A = 532u,
    OUTCOME_PC_F01B = 533u,
    OUTCOME_PC_F01C = 534u,
    OUTCOME_PC_F01D = 535u,
    OUTCOME_PC_F01F = 536u,
    OUTCOME_PC_F022 = 537u,
    OUTCOME_PC_F025 = 538u,
    OUTCOME_PC_F028 = 539u,
    OUTCOME_PC_F02B = 540u,
    OUTCOME_PC_F02C = 541u,
    OUTCOME_PC_F02D = 542u,
    OUTCOME_PC_F02E = 543u,
    OUTCOME_PC_F030 = 544u,
    OUTCOME_PC_F031 = 545u,
    OUTCOME_PC_F032 = 546u,
    OUTCOME_PC_F033 = 547u,
    OUTCOME_PC_F4C6 = 548u,
    OUTCOME_PC_F4C9 = 549u,
    OUTCOME_PC_F4CC = 550u,
    OUTCOME_PC_F4CE = 551u,
    OUTCOME_PC_F4D1 = 552u,
    OUTCOME_PC_F4D2 = 553u,
    OUTCOME_PC_F4D3 = 554u,
    OUTCOME_PC_F4D4 = 555u,
    OUTCOME_PC_F4D7 = 556u,
    OUTCOME_PC_F4DB = 557u,
    OUTCOME_PC_F4DC = 558u,
    OUTCOME_PC_F4DF = 559u,
    OUTCOME_PC_F4E1 = 560u,
    OUTCOME_PC_F4E4 = 561u,
    OUTCOME_PC_F4E5 = 562u,
    OUTCOME_PC_F4E8 = 563u,
    OUTCOME_PC_F4EB = 564u,
    OUTCOME_PC_F4EE = 565u,
    OUTCOME_PC_F4F0 = 566u,
    OUTCOME_PC_F4F2 = 567u,
    OUTCOME_PC_F4F5 = 568u,
    OUTCOME_PC_F4F6 = 569u,
    OUTCOME_PC_F4F9 = 570u,
    OUTCOME_PC_F4FB = 571u,
    OUTCOME_PC_F4FE = 572u,
    OUTCOME_PC_F4FF = 573u,
    OUTCOME_PC_F502 = 574u,
    OUTCOME_PC_F505 = 575u,
    OUTCOME_PC_F507 = 576u,
    OUTCOME_PC_F50A = 577u,
    OUTCOME_PC_F50D = 578u,
    OUTCOME_PC_F50E = 579u,
    OUTCOME_PC_F511 = 580u,
    OUTCOME_PC_F514 = 581u,
    OUTCOME_PC_F516 = 582u,
    OUTCOME_PC_F519 = 583u,
    OUTCOME_PC_F51A = 584u,
    OUTCOME_PC_F51D = 585u,
    OUTCOME_PC_F51F = 586u,
    OUTCOME_PC_F522 = 587u,
    OUTCOME_PC_F525 = 588u,
    OUTCOME_PC_F528 = 589u,
    OUTCOME_PC_F52B = 590u,
    OUTCOME_PC_F52E = 591u,
    OUTCOME_PC_F531 = 592u,
    OUTCOME_PC_F534 = 593u,
    OUTCOME_PC_F537 = 594u,
    OUTCOME_PC_F539 = 595u,
    OUTCOME_PC_F53B = 596u,
    OUTCOME_PC_F53C = 597u,
    OUTCOME_PC_F53F = 598u,
    OUTCOME_PC_F542 = 599u,
    OUTCOME_PC_F545 = 600u,
    OUTCOME_PC_F546 = 601u,
    OUTCOME_PC_F547 = 602u,
    OUTCOME_PC_F548 = 603u,
    OUTCOME_PC_F54A = 604u,
    OUTCOME_PC_F54D = 605u,
    OUTCOME_PC_F550 = 606u,
    OUTCOME_PC_F552 = 607u,
    OUTCOME_PC_F554 = 608u,
    OUTCOME_PC_F557 = 609u,
    OUTCOME_PC_F55A = 610u,
    OUTCOME_PC_F55D = 611u,
    OUTCOME_PC_F560 = 612u,
    OUTCOME_PC_F562 = 613u,
    OUTCOME_PC_F564 = 614u,
    OUTCOME_PC_F567 = 615u,
    OUTCOME_PC_F569 = 616u,
    OUTCOME_PC_F56B = 617u,
    OUTCOME_PC_F56E = 618u,
    OUTCOME_PC_F571 = 619u,
    OUTCOME_PC_F575 = 620u,
    OUTCOME_PC_F577 = 621u,
    OUTCOME_PC_F579 = 622u,
    OUTCOME_PC_F57C = 623u,
    OUTCOME_PC_F57F = 624u,
    OUTCOME_PC_F581 = 625u,
    OUTCOME_PC_F583 = 626u,
    OUTCOME_PC_F585 = 627u,
    OUTCOME_PC_F588 = 628u,
    OUTCOME_PC_F589 = 629u,
    OUTCOME_PC_F58C = 630u,
    OUTCOME_PC_F58F = 631u,
    OUTCOME_PC_F592 = 632u,
    OUTCOME_PC_F595 = 633u,
    OUTCOME_PC_F597 = 634u,
    OUTCOME_PC_F59A = 635u,
    OUTCOME_PC_F59C = 636u,
    OUTCOME_PC_F59F = 637u,
    OUTCOME_PC_F5A0 = 638u,
    OUTCOME_PC_F5A1 = 639u,
    OUTCOME_PC_F5A2 = 640u,
    OUTCOME_PC_F5A4 = 641u,
    OUTCOME_PC_F5A5 = 642u,
    OUTCOME_PC_F5A7 = 643u,
    OUTCOME_PC_F5AA = 644u,
    OUTCOME_PC_F5AD = 645u,
    OUTCOME_PC_F5AF = 646u,
    OUTCOME_PC_F5B2 = 647u,
    OUTCOME_PC_F5B5 = 648u,
    OUTCOME_PC_F5B7 = 649u,
    OUTCOME_PC_F5B9 = 650u,
    OUTCOME_PC_F5BB = 651u,
    OUTCOME_PC_F5BE = 652u,
    OUTCOME_PC_F5BF = 653u,
    OUTCOME_PC_F5C0 = 654u,
    OUTCOME_PC_F5C1 = 655u,
    OUTCOME_PC_F5C2 = 656u,
    OUTCOME_PC_F5C5 = 657u,
    OUTCOME_PC_F5C7 = 658u,
    OUTCOME_PC_F5CA = 659u,
    OUTCOME_PC_F5CC = 660u,
    OUTCOME_PC_F5CE = 661u,
    OUTCOME_PC_F5D1 = 662u,
    OUTCOME_PC_F5D4 = 663u,
    OUTCOME_PC_F5D7 = 664u,
    OUTCOME_PC_F5D9 = 665u,
    OUTCOME_PC_F5DC = 666u,
    OUTCOME_PC_F5DE = 667u,
    OUTCOME_PC_F5E1 = 668u,
    OUTCOME_PC_F5E4 = 669u,
    OUTCOME_PC_F5E6 = 670u,
    OUTCOME_PC_F5E8 = 671u,
    OUTCOME_PC_F5EA = 672u,
    OUTCOME_PC_F5ED = 673u,
    OUTCOME_PC_F5EF = 674u,
    OUTCOME_PC_F5F2 = 675u,
    OUTCOME_PC_F5F3 = 676u,
    OUTCOME_PC_F5F5 = 677u,
    OUTCOME_PC_F5F8 = 678u,
    OUTCOME_PC_F5F9 = 679u,
    OUTCOME_PC_F5FB = 680u,
    OUTCOME_PC_F5FD = 681u,
    OUTCOME_PC_F600 = 682u,
    OUTCOME_PC_F603 = 683u,
    OUTCOME_PC_F606 = 684u,
    OUTCOME_PC_F609 = 685u,
    OUTCOME_PC_F60C = 686u,
    OUTCOME_PC_F60F = 687u,
    OUTCOME_PC_F610 = 688u,
    OUTCOME_PC_F612 = 689u,
    OUTCOME_PC_F615 = 690u,
    OUTCOME_PC_F618 = 691u,
    OUTCOME_PC_F61B = 692u,
    OUTCOME_PC_F61D = 693u,
    OUTCOME_PC_F61E = 694u,
    OUTCOME_PC_F620 = 695u,
    OUTCOME_PC_F623 = 696u,
    OUTCOME_PC_F624 = 697u,
    OUTCOME_PC_F626 = 698u,
    OUTCOME_PC_F628 = 699u,
    OUTCOME_PC_F62B = 700u,
    OUTCOME_PC_F62E = 701u,
    OUTCOME_PC_F630 = 702u,
    OUTCOME_PC_F632 = 703u,
    OUTCOME_PC_F635 = 704u,
    OUTCOME_PC_F637 = 705u,
    OUTCOME_PC_F639 = 706u,
    OUTCOME_PC_F63A = 707u,
    OUTCOME_PC_F63B = 708u,
    OUTCOME_PC_F63D = 709u,
    OUTCOME_PC_F63E = 710u,
    OUTCOME_PC_F640 = 711u,
    OUTCOME_PC_F642 = 712u,
    OUTCOME_PC_F643 = 713u,
    OUTCOME_PC_F644 = 714u,
    OUTCOME_PC_F645 = 715u,
    OUTCOME_PC_F647 = 716u,
    OUTCOME_PC_F648 = 717u,
    OUTCOME_PC_F64B = 718u,
    OUTCOME_PC_F64E = 719u,
    OUTCOME_PC_F651 = 720u,
    OUTCOME_PC_F654 = 721u,
    OUTCOME_PC_F657 = 722u,
    OUTCOME_PC_F65A = 723u,
    OUTCOME_PC_F65D = 724u,
    OUTCOME_PC_F660 = 725u,
    OUTCOME_PC_F663 = 726u,
    OUTCOME_PC_F666 = 727u,
    OUTCOME_PC_F668 = 728u,
    OUTCOME_PC_F66A = 729u,
    OUTCOME_PC_F66D = 730u,
    OUTCOME_PC_F66F = 731u,
    OUTCOME_PC_F672 = 732u,
    OUTCOME_PC_F675 = 733u,
    OUTCOME_PC_F678 = 734u,
    OUTCOME_PC_F67B = 735u,
    OUTCOME_PC_F67D = 736u,
    OUTCOME_PC_F680 = 737u,
    OUTCOME_PC_F683 = 738u,
    OUTCOME_PC_F686 = 739u,
    OUTCOME_PC_F689 = 740u,
    OUTCOME_PC_F68C = 741u,
    OUTCOME_PC_F68E = 742u,
    OUTCOME_PC_F691 = 743u,
    OUTCOME_PC_F693 = 744u,
    OUTCOME_PC_F696 = 745u,
    OUTCOME_PC_F698 = 746u,
    OUTCOME_PC_F69B = 747u,
    OUTCOME_PC_F69D = 748u,
    OUTCOME_PC_F6A0 = 749u,
    OUTCOME_PC_F6A2 = 750u,
    OUTCOME_PC_F6A4 = 751u,
    OUTCOME_PC_F6A6 = 752u,
    OUTCOME_PC_F6A9 = 753u,
    OUTCOME_PC_F6AB = 754u,
    OUTCOME_PC_F6AE = 755u,
    OUTCOME_PC_F6B1 = 756u,
    OUTCOME_PC_F6B3 = 757u,
    OUTCOME_PC_F6B6 = 758u,
    OUTCOME_PC_F6B8 = 759u,
    OUTCOME_PC_F6BB = 760u,
    OUTCOME_PC_F6BD = 761u,
    OUTCOME_PC_F6C0 = 762u,
    OUTCOME_PC_F6C2 = 763u,
    OUTCOME_PC_F6C5 = 764u,
    OUTCOME_PC_F6C7 = 765u,
    OUTCOME_PC_F6CA = 766u,
    OUTCOME_PC_F6CC = 767u,
    OUTCOME_PC_F6CF = 768u,
    OUTCOME_PC_F6D2 = 769u,
    OUTCOME_PC_F6D5 = 770u,
    OUTCOME_PC_F6D8 = 771u,
    OUTCOME_PC_F6DA = 772u,
    OUTCOME_PC_F6DD = 773u,
    OUTCOME_PC_F6DF = 774u,
    OUTCOME_PC_F6E2 = 775u,
    OUTCOME_PC_F6E5 = 776u,
    OUTCOME_PC_F6E8 = 777u,
    OUTCOME_PC_F6E9 = 778u,
    OUTCOME_PC_F6EB = 779u,
    OUTCOME_PC_F6EE = 780u,
    OUTCOME_PC_F6EF = 781u,
    OUTCOME_PC_F6F0 = 782u,
    OUTCOME_PC_F6F1 = 783u,
    OUTCOME_PC_F6F2 = 784u,
    OUTCOME_PC_F6F5 = 785u,
    OUTCOME_PC_F6F7 = 786u,
    OUTCOME_PC_F6FA = 787u,
    OUTCOME_PC_F6FC = 788u,
    OUTCOME_PC_F6FD = 789u,
    OUTCOME_PC_F6FE = 790u,
    OUTCOME_PC_F701 = 791u,
    OUTCOME_PC_F703 = 792u,
    OUTCOME_PC_F704 = 793u,
    OUTCOME_PC_F705 = 794u,
    OUTCOME_PC_F708 = 795u,
    OUTCOME_PC_F70A = 796u,
    OUTCOME_PC_F70D = 797u,
    OUTCOME_PC_F70F = 798u,
    OUTCOME_PC_F711 = 799u,
    OUTCOME_PC_F713 = 800u,
    OUTCOME_PC_F716 = 801u,
    OUTCOME_PC_F718 = 802u,
    OUTCOME_PC_F71B = 803u,
    OUTCOME_PC_F71E = 804u,
    OUTCOME_PC_F721 = 805u,
    OUTCOME_PC_F723 = 806u,
    OUTCOME_PC_F726 = 807u,
    OUTCOME_PC_F729 = 808u,
    OUTCOME_PC_F72C = 809u,
    OUTCOME_PC_F72E = 810u,
    OUTCOME_PC_F731 = 811u,
    OUTCOME_PC_F733 = 812u,
    OUTCOME_PC_F736 = 813u,
    OUTCOME_PC_F738 = 814u,
    OUTCOME_PC_F73B = 815u,
    OUTCOME_PC_F73D = 816u,
    OUTCOME_PC_F740 = 817u,
    OUTCOME_PC_F743 = 818u,
    OUTCOME_PC_F745 = 819u,
    OUTCOME_PC_F748 = 820u,
    OUTCOME_PC_F74A = 821u,
    OUTCOME_PC_F74D = 822u,
    OUTCOME_PC_F74E = 823u,
    OUTCOME_PC_F751 = 824u,
    OUTCOME_PC_F752 = 825u,
    OUTCOME_PC_F755 = 826u,
    OUTCOME_PC_F757 = 827u,
    OUTCOME_PC_F75A = 828u,
    OUTCOME_PC_F75B = 829u,
    OUTCOME_PC_F75E = 830u,
    OUTCOME_PC_F761 = 831u,
    OUTCOME_PC_F763 = 832u,
    OUTCOME_PC_F766 = 833u,
    OUTCOME_PC_F769 = 834u,
    OUTCOME_PC_F76A = 835u,
    OUTCOME_PC_F76D = 836u,
    OUTCOME_PC_F76E = 837u,
    OUTCOME_PC_F771 = 838u,
    OUTCOME_PC_F774 = 839u,
    OUTCOME_PC_F776 = 840u,
    OUTCOME_PC_F778 = 841u,
    OUTCOME_PC_F77B = 842u,
    OUTCOME_PC_F77D = 843u,
    OUTCOME_PC_F77F = 844u,
    OUTCOME_PC_F781 = 845u,
    OUTCOME_PC_F784 = 846u,
    OUTCOME_PC_F787 = 847u,
    OUTCOME_PC_F78B = 848u,
    OUTCOME_PC_F78E = 849u,
};

static const TableOutcomeOpcode table_outcome_opcodes[] = {
    {{0xa60cu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xa610u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdeb9u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdebcu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdebfu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdec0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdec2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdec5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdec8u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdecau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdecbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdecdu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xded0u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xded3u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xded5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xded8u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdedbu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdedcu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdedfu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdee2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdee4u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdee7u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdeeau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdeedu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdef0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdef3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdef6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdef8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdefbu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdefeu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf01u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf04u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf06u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf09u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf0cu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf0fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf12u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf14u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf17u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf1au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf1cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf1fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf21u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf24u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf27u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf29u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf2cu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdf30u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf33u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf35u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf38u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdf3cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf3fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf41u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf44u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf46u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf49u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf4bu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf4du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf50u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdf54u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf57u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf59u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf5cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf5eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf61u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdf65u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf68u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf6au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf6cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf6fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf71u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf74u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf76u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf79u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf7bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf7eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf81u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf84u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf87u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf8au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf8du, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf90u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf93u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf96u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdf99u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf9au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf9bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdf9cu, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfa0u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdfa4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfa7u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfa9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfacu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfadu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfb0u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdfb4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfb7u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfb9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfbcu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfbfu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfc1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfc4u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdfc8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfcbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfcdu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfd0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfd3u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfd6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfd9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfdcu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdfdfu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfe2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfe4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfe7u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdfebu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdfeeu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdff0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdff3u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdff5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdff8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdffau, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdffdu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe001u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe004u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe006u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe009u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe00bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe00eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe010u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe013u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe017u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe01au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe01cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe01fu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe022u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe025u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe026u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe027u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe028u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe02cu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe030u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe033u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe035u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe038u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe039u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe03cu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe040u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe043u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe045u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe048u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe04bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe04du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe050u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe054u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe057u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe059u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe05cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe05fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe062u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe065u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe068u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe06bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe06eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe070u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe073u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe077u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe07au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe07cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe07fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe081u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe084u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe086u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe089u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe08du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe090u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe092u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe095u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe098u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe09bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe09eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0a0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0a3u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe0a7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0aau, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0acu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0afu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0b0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe0b3u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe0b7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0bau, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0bcu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0bfu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe0c3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0c4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0c7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0cau, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0ccu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0cdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0d0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe0d3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0d6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe0d9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0dcu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe0dfu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe0e2u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0e4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0e7u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe0ebu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0eeu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0f0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0f3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0f4u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe0f7u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe0fbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe0feu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe100u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe103u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe106u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe109u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe10cu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe110u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe113u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe115u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe118u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe11cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe11fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe121u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe124u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe128u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe129u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe12cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe12fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe132u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe136u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe139u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe13bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe13eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe141u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe143u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe146u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe14au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe14du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe14fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe152u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe156u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe157u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe15au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe15du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe160u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe162u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe165u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe168u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe16bu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe16eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe171u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe174u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe175u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe178u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe17bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe17eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe181u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe184u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe187u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe18au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe18du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe190u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe193u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe334u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe337u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe33au, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe33du, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe33eu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe33fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe340u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe342u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe345u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe348u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe34au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe34cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe34eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe350u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe352u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe355u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe357u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe35au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe35du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe360u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe362u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe365u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe368u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe36au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe36bu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe36eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe371u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe374u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe377u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe37au, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe37du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe380u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe383u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe386u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe389u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe38cu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe38eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe391u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe393u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe395u, 3u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe398u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe39au, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe39cu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe39eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3a1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3a4u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe3a8u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3aau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3adu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3b0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3b3u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3b6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3b9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3bcu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3bfu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3c2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3c5u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3c7u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3c9u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe3cbu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3ceu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3d0u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe3d2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3d4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3d6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3d8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3dbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3ddu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3e0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3e3u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3e6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3e9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3ecu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3efu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3f2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3f5u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3f6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe3f9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3fau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3fbu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3fcu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe3fdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe400u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe403u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe406u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe408u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xe40bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe40du, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe410u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe411u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe412u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe413u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe415u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe416u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe418u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe41au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe41cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe41fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe421u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe424u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe426u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe427u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe42au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe42cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe42fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe432u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe434u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe437u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe439u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe43cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe43eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe441u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe444u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe447u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe44au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe44cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe44fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe451u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe454u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe457u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe459u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe45cu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe45fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe461u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe464u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe466u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe469u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe46cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe46fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe472u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe475u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe478u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe47bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe47eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe481u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe484u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe486u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe489u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe48cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe48fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe490u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe493u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe495u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe498u, 3u, 5u, 2u}, 0x7150u, 1u, 0, 0},
    {{0xe49bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe49eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4a0u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe4a1u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe4a2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe4a3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4a5u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4a7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4aau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe4acu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4afu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4b1u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4b4u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4b6u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4b9u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe4bau, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe4bbu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe4bcu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe4bdu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4c0u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4c2u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4c4u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe4c7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4c8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4c9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4cau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4cbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4ceu, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe4d1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4d3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4d6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4d8u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xe4dbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4deu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4e0u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4e2u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4e4u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe4e7u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4e9u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xe4ecu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4edu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4efu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4f2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4f3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4f5u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4f8u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe4fau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4fbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe4feu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe500u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe502u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe503u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe504u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe505u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe507u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe50au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe50du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe50fu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe512u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe514u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe517u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe51au, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe51bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe51eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe521u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe522u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe525u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe528u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe52au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe52du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe530u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe533u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe534u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe535u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe539u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe53au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe53du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe540u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe543u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe546u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe549u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe54cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe54fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe552u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe555u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe558u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe55bu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe55eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe561u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe564u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe565u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe567u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe569u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe56cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe56fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe572u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe574u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe577u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe57au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe57du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe57eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe581u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe584u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe587u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe58au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe58du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe590u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe591u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe592u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe594u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe597u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe59au, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe59du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5a0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5a3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5a6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5a9u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5acu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5afu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5b2u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf01au, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf01bu, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf01cu, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf01du, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf01fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf022u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf025u, 3u, 5u, 2u}, 0xa000u, 1u, 0, 0},
    {{0xf028u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf02bu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf02cu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf02du, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf02eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf030u, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf031u, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf032u, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf033u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4c6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf4c9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4ccu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf4ceu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf4d1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4d2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4d3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4d4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4d7u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xf4dbu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4dcu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4dfu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf4e1u, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xf4e4u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4e5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4e8u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf4ebu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf4eeu, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf4f0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4f2u, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xf4f5u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4f6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4f9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4fbu, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xf4feu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf4ffu, 3u, 5u, 2u}, 0x712au, 1u, 0, 0},
    {{0xf502u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf505u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf507u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf50au, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xf50du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf50eu, 3u, 5u, 2u}, 0x712au, 1u, 0, 0},
    {{0xf511u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf514u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf516u, 3u, 5u, 2u}, 0x7000u, 1u, 0, 0},
    {{0xf519u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf51au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf51du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf51fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf522u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf525u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf528u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf52bu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf52eu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf531u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf534u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf537u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf539u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf53bu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf53cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf53fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf542u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf545u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf546u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf547u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf548u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf54au, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf54du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf550u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf552u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf554u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf557u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf55au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf55du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf560u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf562u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf564u, 3u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf567u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf569u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf56bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf56eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf571u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xf575u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf577u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf579u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf57cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf57fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf581u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf583u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf585u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf588u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf589u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf58cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf58fu, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xf592u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf595u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf597u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xf59au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf59cu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf59fu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf5a0u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf5a1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf5a2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5a4u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5a5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf5a7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5aau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf5adu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf5afu, 3u, 5u, 2u}, 0x7150u, 2u, 0, 0},
    {{0xf5b2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5b5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5b7u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5b9u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5bbu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5beu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf5bfu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf5c0u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xf5c1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xf5c2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5c5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5c7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5cau, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5ccu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5ceu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf5d1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5d4u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xf5d7u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5d9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5dcu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5deu, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xf5e1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5e4u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5e6u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5e8u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5eau, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf5edu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5efu, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xf5f2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5f3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5f5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5f8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf5f9u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf5fbu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf5fdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf600u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf603u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf606u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf609u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf60cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf60fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf610u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf612u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf615u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf618u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf61bu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf61du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf61eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf620u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf623u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf624u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf626u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf628u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf62bu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf62eu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf630u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf632u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf635u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf637u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf639u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf63au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf63bu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf63du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf63eu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf640u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf642u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf643u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf644u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf645u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf647u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf648u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf64bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf64eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf651u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf654u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf657u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf65au, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf65du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf660u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf663u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf666u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf668u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf66au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf66du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf66fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf672u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf675u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf678u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf67bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf67du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf680u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf683u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf686u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf689u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf68cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf68eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf691u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf693u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf696u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf698u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf69bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf69du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6a0u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6a2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6a4u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf6a6u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6a9u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6abu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf6aeu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6b1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6b3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6b6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6b8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6bbu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6bdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6c0u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6c2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6c5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6c7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6cau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6ccu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6cfu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6d2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6d5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6d8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6dau, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6ddu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6dfu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6e2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6e5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6e8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6e9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6ebu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6eeu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6efu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6f0u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6f1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6f2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf6f5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6f7u, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xf6fau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf6fcu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf6fdu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf6feu, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xf701u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf703u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf704u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf705u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf708u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf70au, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf70du, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf70fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf711u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf713u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf716u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf718u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf71bu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf71eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf721u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf723u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf726u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf729u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf72cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf72eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf731u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf733u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf736u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf738u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf73bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf73du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf740u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf743u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf745u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf748u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf74au, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf74du, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf74eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf751u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf752u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf755u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf757u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf75au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf75bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf75eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf761u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf763u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf766u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf769u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf76au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf76du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf76eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf771u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf774u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf776u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf778u, 3u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf77bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf77du, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf77fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf781u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf784u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf787u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xf78bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf78eu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
};

enum { TABLE_OUTCOME_CAPACITY = 512u };
typedef struct TableOutcomeClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_OUTCOME_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint32_t budget;
    uint32_t next;
    bool returning;
} TableOutcomeClock;
static const uint32_t table_outcome_entries[]={0x84a60cu,0x84deb9u,0x84dedcu,0x84e334u,0x84e3f6u,0x84e416u,0x84e489u,0x84f01au,0x84f4c6u,0x84f53cu,0x84f589u,0x84f5a5u,0x84f648u,0x84f74eu};
static const uint32_t table_outcome_ends[]={0x84a610u,0x84dedbu,0x84e193u,0x84e3f5u,0x84e415u,0x84e51au,0x84e51au,0x84f033u,0x84f53bu,0x84f588u,0x84f5a4u,0x84f647u,0x84f74du,0x84f78eu};
static uint16_t TableOutcomeDirectWord(const CpuState *cpu, unsigned at) {
    return cpu->ram[at] | (uint16_t)cpu->ram[at + 1u] << 8;
}
static uint16_t TableOutcomeBankWord(const CpuState *cpu, uint16_t at) {
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    return cpu->ram[bank + at] | (uint16_t)cpu->ram[bank + (uint16_t)(at + 1u)] << 8;
}
static bool TableOutcomeAppendOpcode(TableOutcomeClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == TABLE_OUTCOME_CAPACITY) return false;
    clock->timing[clock->span.count++] = op;
    clock->budget += op.fetched * cpu_region_speed(0x840000u | op.pc) +
        op.data_bytes * 12u + (op.cycles - op.fetched - op.data_bytes) * 6u;
    return true;
}
static bool TableOutcomeRange(TableOutcomeClock *clock, unsigned first, unsigned last,
                         uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableOutcomeOpcode *record = table_outcome_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!TableOutcomeAppendOpcode(clock, op)) return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}
static bool TableOutcomeBranch(TableOutcomeClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_outcome_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return TableOutcomeAppendOpcode(clock, op);
}
#define LR(first,last,x,y) do { if (!TableOutcomeRange(clock, OUTCOME_PC_##first, OUTCOME_PC_##last, (uint16_t)(x), (uint16_t)(y))) return false; } while (0)
#define LB(pc,taken) do { if (!TableOutcomeBranch(clock, OUTCOME_PC_##pc, taken)) return false; } while (0)
#define LC(site) do { LR(site,site,0u,0u); clock->next = 0x840000u | (table_outcome_opcodes[OUTCOME_PC_##site].timing.pc + table_outcome_opcodes[OUTCOME_PC_##site].timing.fetched); clock->returning = false; return true; } while (0)


static bool PlanOutcomeSong(TableOutcomeClock *clock, uint16_t pc) {
    if (pc == 0xa60cu) { LC(A60C); }
    return pc == 0xa610u;
}

static bool PlanOutcomeSelectedRound(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xdeb9u) { LC(DEB9); }
    if (pc == 0xdebcU) {
        const uint16_t selection = TableOutcomeBankWord(cpu, 0x713eu);
        const uint16_t first = (uint16_t)(selection - 1u);
        LR(DEBC, DEBF, 0u, 0u);
        LB(DEC0, (first & 0x8000u) == 0u);
        if (first & 0x8000u) { LC(DEC2); }
        const uint16_t second = (uint16_t)(first - 1u);
        LR(DECA, DECA, 0u, 0u);
        LB(DECB, (second & 0x8000u) == 0u);
        if (second & 0x8000u) { LC(DECD); }
        LC(DED5);
    }
    if (pc == 0xdec5u) { LC(DEC5); }
    if (pc == 0xded0u) { LC(DED0); }
    if (pc == 0xdec8u) {
        LR(DEC8, DEC8, 0u, 0u);
        LC(DEB9);
    }
    if (pc == 0xded3u) {
        LR(DED3, DED3, 0u, 0u);
        LC(DEB9);
    }
    if (pc == 0xded8u) { LC(DED8); }
    return pc == 0xdedbu;
}

static bool PlanOutcomePairMarks(TableOutcomeClock *clock, uint16_t pc) {
    if (pc != 0xe3f6u) return false;
    const CpuState *cpu = clock->call.cpu;
    const unsigned selection = TableOutcomeBankWord(cpu, 0x7184u);
    if (selection < 1u || selection > 3u) return false;
    unsigned remaining = selection * 2u - 1u;
    unsigned offset = 0u;
    LR(E3F6, E3FD, 0u, remaining);
    do {
        const bool paired = TableOutcomeBankWord(cpu, (uint16_t)(0x7146u + offset)) == 2u;
        LR(E400, E403, offset, remaining);
        LB(E406, !paired);
        if (paired) {
            const bool occupied = TableOutcomeBankWord(cpu, (uint16_t)(0x7162u + offset)) != 0u;
            LR(E408, E408, offset, remaining);
            LB(E40B, !occupied);
            if (occupied) { LR(E40D, E40D, offset, remaining); }
        }
        LR(E410, E412, offset, remaining);
        offset += 2u;
        --remaining;
        LB(E413, remaining != 0u);
    } while (remaining);
    return true;
}

static bool PlanOutcomeAlternatePairMarks(TableOutcomeClock *clock, uint16_t pc) {
    if (pc != 0xf589u) return false;
    const CpuState *cpu = clock->call.cpu;
    unsigned remaining = TableOutcomeBankWord(cpu, 0x7184u);
    if (remaining < 1u || remaining > 5u) return false;
    unsigned offset = 0u;
    LR(F589, F58C, 0u, remaining);
    do {
        const bool paired = TableOutcomeBankWord(cpu, (uint16_t)(0x7146u + offset)) == 2u;
        LR(F58F, F592, offset, remaining);
        LB(F595, !paired);
        if (paired) {
            const bool occupied = TableOutcomeBankWord(cpu, (uint16_t)(0x7162u + offset)) != 0u;
            LR(F597, F597, offset, remaining);
            LB(F59A, !occupied);
            if (occupied) { LR(F59C, F59C, offset, remaining); }
        }
        LR(F59F, F5A1, offset, remaining);
        offset += 2u;
        --remaining;
        LB(F5A2, remaining != 0u);
    } while (remaining);
    return true;
}

static bool PlanOutcomePayStake(TableOutcomeClock *clock, uint16_t pc) {
    if (pc == 0xf78bu) {
        LR(F78B, F78B, 0u, 0u);
        return true;
    }
    if (pc != 0xf74eu) return false;
    const CpuState *cpu = clock->call.cpu;
    const bool borrow = TableOutcomeBankWord(cpu, 0x0b55u) <
        TableOutcomeBankWord(cpu, 0x71b6u);
    LR(F74E, F752, 0u, 0u);
    LB(F755, !borrow);
    if (borrow) {
        const bool exhausted = TableOutcomeBankWord(cpu, 0x0b57u) == 0u;
        LR(F757, F75E, 0u, 0u);
        LB(F761, !exhausted);
        if (exhausted) {
            LR(F763, F766, 0u, 0u);
            clock->next = 0x84f769u;
            return true;
        }
    }
    LR(F76A, F784, 0u, 0u);
    LC(F787);
}

static bool OutcomeAlternateChoicesFinish(TableOutcomeClock *clock) {
    LR(F723, F723, 0u, 0u);
    LC(F726);
}

static bool PlanOutcomeAlternateChoices(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xf648u) {
        LR(F648, F657, 0u, 0u);
        LC(F65A);
    }
    if (pc == 0xf65du) { LC(F65D); }
    if (pc == 0xf660u) { LC(F660); }
    if (pc == 0xf663u) {
        const uint16_t action = cpu->A;
        LR(F663, F663, 0u, 0u);
        LB(F666, action != 7u);
        if (action == 7u) {
            LR(F668, F668, 0u, 0u);
            LC(F65A);
        }
        LR(F66A, F66A, 0u, 0u);
        LB(F66D, action != 2u);
        if (action == 2u) {
            LR(F66F, F66F, 0u, 0u);
            LC(F672);
        }
        LR(F678, F678, 0u, 0u);
        LB(F67B, (action & 0x100u) == 0u);
        if (!(action & 0x100u)) { LC(F65A); }
        LR(F67D, F67D, 0u, 0u);
        LC(F680);
    }
    if (pc == 0xf675u) {
        LR(F675, F675, 0u, 0u);
        LR(F729, F73B, 0u, 0u);
        LC(F73D);
    }
    if (pc == 0xf683u) { LC(F683); }
    if (pc == 0xf686u) { LC(F686); }
    if (pc == 0xf689u) {
        LR(F689, F69B, 0u, 0u);
        LC(F69D);
    }
    if (pc == 0xf6a0u) {
        LR(F6A0, F6A4, 0u, 0u);
        LC(F6A6);
    }
    if (pc == 0xf6a9u) {
        const uint16_t variant = TableOutcomeBankWord(cpu, 0x7170u);
        LR(F6A9, F6AE, 0u, 0u);
        LB(F6B1, variant == 0u);
        if (variant) {
            LR(F6B3, F6B3, 0u, 0u);
            LB(F6B6, (variant & 9u) == 0u);
            if (!(variant & 9u)) return OutcomeAlternateChoicesFinish(clock);
        }
        LR(F6B8, F6CA, 0u, 0u);
        LC(F6CC);
    }
    if (pc == 0xf6cfu) { LC(F6CF); }
    if (pc == 0xf6d2u) { LC(F6D2); }
    if (pc == 0xf6d5u) {
        const uint16_t action = cpu->A;
        LR(F6D5, F6D5, 0u, 0u);
        LB(F6D8, action == 1u);
        if (action == 1u) return OutcomeAlternateChoicesFinish(clock);
        LR(F6DA, F6DA, 0u, 0u);
        LB(F6DD, (action & 0x100u) == 0u);
        if (!(action & 0x100u)) { LC(F6CF); }
        LR(F6DF, F6DF, 0u, 0u);
        LC(F6E2);
    }
    if (pc == 0xf6e5u) { LC(F6E5); }
    if (pc == 0xf6e8u) {
        const bool failed = ((uint16_t)(cpu->A - 1u) & 0x8000u) != 0u;
        LR(F6E8, F6E8, 0u, 0u);
        LB(F6E9, failed);
        if (failed) { LC(F6CF); }
        const unsigned selection = TableOutcomeBankWord(cpu, 0x7184u);
        if (selection > 4u) return false;
        LR(F6EB, F6F1, 0u, 0u);
        LR(F6F2, F708, selection * 4u, 0u);
        LC(F70A);
    }
    if (pc == 0xf70du) {
        LR(F70D, F711, 0u, 0u);
        LC(F713);
    }
    if (pc == 0xf716u) {
        const bool more = (uint16_t)(TableOutcomeBankWord(cpu, 0x7184u) + 1u) != 5u;
        LR(F716, F71E, 0u, 0u);
        LB(F721, more);
        if (more) { LC(F6CF); }
        return OutcomeAlternateChoicesFinish(clock);
    }
    if (pc == 0xf729u) {
        LR(F729, F73B, 0u, 0u);
        LC(F73D);
    }
    if (pc == 0xf740u) {
        LR(F740, F748, 0u, 0u);
        LC(F74A);
    }
    return pc == 0xf74du;
}

static bool PlanOutcomeAlternatePayout(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xf53cu) {
        LR(F53C, F53F, 0u, 5u);
        for (unsigned line = 0u; line < 5u; ++line) {
            LR(F542, F547, line * 2u, 5u - line);
            LB(F548, line != 4u);
        }
        LC(F54A);
    }
    if (pc == 0xf54du) { LC(F54D); }
    if (pc == 0xf550u) {
        const bool empty = TableOutcomeDirectWord(cpu, 0u) == 0u;
        LR(F550, F550, 0u, 0u);
        LB(F552, !empty);
        if (empty) {
            LR(F554, F554, 0u, 0u);
            return true;
        }
        LR(F557, F55A, 0u, 0u);
        LC(F55D);
    }
    if (pc == 0xf560u) {
        LR(F560, F56E, 0u, 0u);
        LC(F571);
    }
    if (pc == 0xf575u) {
        LR(F575, F577, 0u, 0u);
        LC(F579);
    }
    if (pc == 0xf57cu) { LC(F57C); }
    if (pc == 0xf57fu) {
        const bool more = (uint16_t)(TableOutcomeDirectWord(cpu, 0u) - 1u) != 0u;
        LR(F57F, F581, 0u, 0u);
        LB(F583, more);
        if (more) { LC(F55D); }
        LC(F585);
    }
    return pc == 0xf588u;
}

static bool OutcomeAlternateFinishColumn(TableOutcomeClock *clock, unsigned column) {
    const CpuState *cpu = clock->call.cpu;
    const unsigned offset = column * 2u;
    const uint16_t speed = (uint16_t)(TableOutcomeBankWord(cpu,
        (uint16_t)(0x7000u + offset)) - 1u);
    const bool ordinary = ((uint16_t)(speed - 5u) & 0x8000u) == 0u;
    LR(F516, F51A, offset, 0u);
    LB(F51D, ordinary);
    if (!ordinary) { LR(F51F, F51F, offset, 0u); }
    LR(F522, F52E, offset, 0u);
    LC(F531);
}

static bool OutcomeAlternateWaitForColumn(TableOutcomeClock *clock,
    unsigned column, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    const unsigned offset = column * 2u;
    const uint16_t position = (uint16_t)(TableOutcomeBankWord(cpu,
        (uint16_t)(0x7124u + offset)) + TableOutcomeBankWord(cpu,
        (uint16_t)(0x712au + offset)));
    const bool below = ((uint16_t)(position - 0x1c00u) & 0x8000u) != 0u;
    if (pc == 0xf4fbu) {
        LR(F4FB, F502, offset, 0u);
        LB(F505, !below);
    } else {
        LR(F50A, F511, offset, 0u);
        LB(F514, below);
    }
    if (below) { LC(F507); }
    return OutcomeAlternateFinishColumn(clock, column);
}

static bool PlanOutcomeStopAlternate(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xf4c6u) {
        LR(F4C6, F4D4, 0u, 0u);
        LC(F4D7);
    }
    const unsigned column = TableOutcomeBankWord(cpu, 0x713cu);
    if (column > 2u || cpu->X != column * 2u) return false;
    const unsigned offset = column * 2u;
    if (pc == 0xf4dbu) {
        LR(F4DB, F4E8, offset, 0u);
        LC(F4EB);
    }
    if (pc == 0xf4eeu) {
        const bool remaining = (uint16_t)(TableOutcomeDirectWord(cpu, 0u) - 1u) != 0u;
        LR(F4EE, F4EE, offset, 0u);
        LB(F4F0, remaining);
        if (remaining) { LC(F4EB); }
        const bool slowing = TableOutcomeBankWord(cpu,
            (uint16_t)(0x7124u + offset)) != 0x100u;
        LR(F4F2, F4F6, offset, 0u);
        LB(F4F9, slowing);
        if (slowing) {
            LR(F4D4, F4D4, offset, 0u);
            LC(F4D7);
        }
        return OutcomeAlternateWaitForColumn(clock, column, 0xf4fbu);
    }
    if (pc == 0xf50au)
        return OutcomeAlternateWaitForColumn(clock, column, pc);
    if (pc == 0xf534u) {
        const bool remaining = (uint16_t)(TableOutcomeDirectWord(cpu, 2u) - 1u) != 0u;
        LR(F534, F537, offset, 0u);
        LB(F539, remaining);
        if (remaining) {
            if (column == 2u) return false;
            LR(F4CE, F4D4, (column + 1u) * 2u, 0u);
            LC(F4D7);
        }
        return true;
    }
    return false;
}

static bool OutcomeOrdinaryMatchEnd(TableOutcomeClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    const bool ordinary = TableOutcomeBankWord(cpu, 0x7170u) == 1u;
    LR(E507, E50A, 0u, 0u);
    LB(E50D, !ordinary);
    if (!ordinary) return true;
    const bool more = (uint16_t)(TableOutcomeBankWord(cpu, 0x7176u) - 1u) != 0u;
    LR(E50F, E50F, 0u, 0u);
    LB(E512, more);
    if (!more) { LR(E514, E517, 0u, 0u); }
    return true;
}

static bool PlanOutcomeMatches(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe489u) {
        const unsigned selection = TableOutcomeBankWord(cpu, 0x7184u);
        if (selection < 1u || selection > 3u) return false;
        unsigned offset = 0u, remaining = selection == 1u ? 1u : 3u;
        LR(E489, E490, 0u, selection);
        LB(E493, selection == 1u);
        if (selection != 1u) { LR(E495, E495, 0u, remaining); }
        bool found = false;
        do {
            found = TableOutcomeBankWord(cpu, (uint16_t)(0x7150u + offset)) == 7u;
            LR(E498, E49B, offset, remaining);
            LB(E49E, found);
            if (found) break;
            LR(E4A0, E4A2, offset, remaining);
            offset += 2u;
            --remaining;
            LB(E4A3, remaining != 0u);
        } while (remaining);
        if (found) {
            LR(E4A7, E4AC, offset, remaining);
            LB(E4AF, offset != 0u);
            if (!offset) { LR(E4B1, E4B4, offset, remaining); }
            else { LR(E4B6, E4C2, offset, remaining); }
        } else { LR(E4A5, E4A5, offset, remaining); }
        remaining = selection * 2u - 1u;
        offset = 0u;
        LR(E4C4, E4CB, 0u, remaining);
        do {
            const uint16_t type = TableOutcomeBankWord(cpu,
                (uint16_t)(0x7146u + offset));
            LR(E4CE, E4CE, offset, remaining);
            LB(E4D1, type == 0u);
            if (type) {
                LR(E4D3, E4D3, offset, remaining);
                LB(E4D6, type != 1u);
                const uint16_t symbol = TableOutcomeBankWord(cpu,
                    (uint16_t)(0x7162u + offset));
                if (type == 1u) {
                    LR(E4D8, E4DB, offset, remaining);
                    LB(E4DE, symbol != 7u);
                    if (symbol == 7u) {
                        LR(E4E0, E4E7, offset, remaining);
                        return OutcomeOrdinaryMatchEnd(clock);
                    }
                } else {
                    LR(E4E9, E4EC, offset, remaining);
                    LB(E4ED, symbol != 1u);
                    if (symbol == 1u) {
                        LR(E4EF, E4EF, offset, remaining);
                        LR(E51B, E534, offset, remaining);
                        LC(E535);
                    }
                    LR(E4F2, E4F2, offset, remaining);
                    LB(E4F3, symbol != 2u);
                    if (symbol == 2u) {
                        LR(E4F5, E4F5, offset, remaining);
                        LR(E569, E597, offset, remaining);
                        LC(E59A);
                    }
                    LR(E4F8, E500, offset, remaining);
                    return OutcomeOrdinaryMatchEnd(clock);
                }
            }
            LR(E502, E504, offset, remaining);
            offset += 2u;
            --remaining;
            LB(E505, remaining != 0u);
        } while (remaining);
        return OutcomeOrdinaryMatchEnd(clock);
    }
    if (pc == 0xe539u) {
        LR(E539, E552, cpu->X, cpu->Y);
        LC(E555);
    }
    if (pc == 0xe558u) { LC(E558); }
    if (pc == 0xe55bu) { LC(E55B); }
    if (pc == 0xe55eu) {
        LR(E55E, E55E, cpu->X, cpu->Y);
        LC(E561);
    }
    if (pc == 0xe564u) {
        const bool more = (uint16_t)(cpu->A - 1u) != 0u;
        LR(E564, E564, cpu->X, cpu->Y);
        LB(E565, more);
        if (more) { LC(E561); }
        LR(E567, E567, cpu->X, cpu->Y);
        return true;
    }
    if (pc == 0xe59du) {
        LR(E59D, E5A6, cpu->X, cpu->Y);
        LC(E5A9);
    }
    if (pc == 0xe5acu) {
        LR(E5AC, E5B2, cpu->X, cpu->Y);
        return true;
    }
    return false;
}

static bool OutcomeRoundCounter(TableOutcomeClock *clock) {
    LR(E466, E46F, 0u, 0u);
    LC(E472);
}

static bool PlanOutcomeRoundOutcome(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe416u) {
        uint16_t mode = TableOutcomeDirectWord(cpu, 0x1fu);
        LR(E416, E418, 0u, 0u);
        LB(E41A, mode != 0u);
        if (!mode) {
            mode = TableOutcomeBankWord(cpu, 0x7170u);
            LR(E41C, E41C, 0u, 0u);
            LB(E41F, mode == 0u);
            if (!mode) return PlanOutcomeMatches(clock, 0xe489u);
            mode &= 9u;
            LR(E421, E421, 0u, 0u);
            LB(E424, mode != 0u);
            if (mode) return PlanOutcomeMatches(clock, 0xe489u);
        }
        const bool below = ((uint16_t)(mode - 7u) & 0x8000u) != 0u;
        LR(E426, E427, 0u, 0u);
        LB(E42A, below);
        if (below) {
            const bool paired = TableOutcomeBankWord(cpu, 0x7146u) == 2u;
            LR(E451, E454, 0u, 0u);
            LB(E457, paired);
            if (!paired) {
                LR(E459, E45F, 0u, 0u);
                LR(E486, E486, 0u, 0u);
                return true;
            }
        } else {
            LR(E42C, E434, 0u, 0u);
            const bool fixed = TableOutcomeDirectWord(cpu, 0x1fu) == 7u;
            LB(E437, fixed);
            if (!fixed) {
                LR(E439, E43C, 0u, 0u);
                LC(E43E);
            }
        }
        LR(E461, E464, 0u, 0u);
        return OutcomeRoundCounter(clock);
    }
    if (pc == 0xe441u) { LC(E441); }
    if (pc == 0xe444u) { LC(E444); }
    if (pc == 0xe447u) {
        LR(E447, E44A, 0u, 0u);
        LC(E44C);
    }
    if (pc == 0xe44fu) {
        LR(E44F, E44F, 0u, 0u);
        return OutcomeRoundCounter(clock);
    }
    if (pc == 0xe475u) {
        LR(E475, E47E, 0u, 0u);
        LC(E481);
    }
    if (pc == 0xe484u) {
        LR(E484, E486, 0u, 0u);
        return true;
    }
    return PlanOutcomeMatches(clock, pc);
}

static bool OutcomePayoutFinish(TableOutcomeClock *clock) {
    const bool quiet = TableOutcomeBankWord(clock->call.cpu, 0x71acu) == 0u;
    LR(E3D8, E3D8, 0u, 0u);
    LB(E3DB, quiet);
    if (!quiet) { LC(E3DD); }
    return true;
}

static bool OutcomePayoutStep(TableOutcomeClock *clock) {
    const bool full = TableOutcomeBankWord(clock->call.cpu, 0x71aeu) == 50u;
    LR(E362, E365, 0u, 0u);
    LB(E368, full);
    if (full) { LC(E38E); }
    LR(E36A, E377, 0u, 0u);
    LC(E37A);
}

static bool PlanOutcomePayout(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe334u) {
        LR(E334, E337, 0u, 5u);
        for (unsigned line = 0u; line < 5u; ++line) {
            LR(E33A, E33F, line * 2u, 5u - line);
            LB(E340, line != 4u);
        }
        LC(E342);
    }
    if (pc == 0xe345u) { LC(E345); }
    if (pc == 0xe348u) {
        const bool empty = TableOutcomeDirectWord(cpu, 0u) == 0u;
        LR(E348, E348, 0u, 0u);
        LB(E34A, !empty);
        if (empty) {
            LR(E34C, E34C, 0u, 0u);
            LB(E34E, TableOutcomeDirectWord(cpu, 0x1fu) != 0u);
            LR(E350, E352, 0u, 0u);
            return OutcomePayoutFinish(clock);
        }
        LR(E355, E360, 0u, 0u);
        return OutcomePayoutStep(clock);
    }
    if (pc == 0xe37du) { LC(E37D); }
    if (pc == 0xe380u) {
        LR(E380, E38C, 0u, 0u);
        LR(E3A8, E3B0, 0u, 0u);
        LC(E3B3);
    }
    if (pc == 0xe391u) {
        LR(E391, E3A1, 0u, 0u);
        LC(E3A4);
    }
    if (pc == 0xe3a8u) {
        LR(E3A8, E3B0, 0u, 0u);
        LC(E3B3);
    }
    if (pc == 0xe3b6u) {
        LR(E3B6, E3BF, 0u, 0u);
        LC(E3C2);
    }
    if (pc == 0xe3c5u) {
        LR(E3C5, E3C9, 0u, 0u);
        LC(E3CB);
    }
    if (pc == 0xe3ceu) {
        const bool more = (uint16_t)(TableOutcomeDirectWord(cpu, 2u) + 1u) !=
            TableOutcomeDirectWord(cpu, 0u);
        LR(E3CE, E3D4, 0u, 0u);
        LB(E3D6, more);
        if (more) return OutcomePayoutStep(clock);
        return OutcomePayoutFinish(clock);
    }
    if (pc == 0xe3e0u) {
        LR(E3E0, E3E0, 0u, 0u);
        LC(E3E3);
    }
    if (pc == 0xe3e6u) {
        LR(E3E6, E3EF, 0u, 0u);
        LC(E3F2);
    }
    return pc == 0xe3f5u;
}

static bool OutcomeAlternateLines(TableOutcomeClock *clock,
    unsigned offset, unsigned remaining) {
    const CpuState *cpu = clock->call.cpu;
    if ((offset & 1u) || remaining < 1u || remaining > 5u ||
        offset + remaining * 2u > 10u) return false;
    while (remaining) {
        const uint16_t type = TableOutcomeBankWord(cpu, (uint16_t)(0x7146u + offset));
        LR(F5D4, F5D4, offset, remaining);
        LB(F5D7, type == 0u);
        if (type) {
            LR(F5D9, F5D9, offset, remaining);
            LB(F5DC, type != 1u);
            const uint16_t symbol = TableOutcomeBankWord(cpu,
                (uint16_t)(0x7162u + offset));
            if (type == 1u) {
                LR(F5DE, F5E1, offset, remaining);
                LB(F5E4, symbol != 7u);
                if (symbol == 7u) { LR(F5E6, F5ED, offset, remaining); }
            } else {
                LR(F5EF, F5F2, offset, remaining);
                LB(F5F3, symbol != 1u);
                if (symbol == 1u) {
                    LR(F5F5, F5FD, offset, remaining);
                    LC(F600);
                }
                LR(F61D, F61D, offset, remaining);
                LB(F61E, symbol != 2u);
                if (symbol == 2u) {
                    LR(F620, F628, offset, remaining);
                    LC(F62B);
                }
                if (symbol < 3u || symbol > 8u) return false;
                LR(F630, F632, offset, remaining);
                unsigned exponent = symbol - 2u;
                do {
                    LR(F635, F635, offset, remaining);
                    --exponent;
                    LB(F637, exponent == 0u);
                    if (exponent) { LR(F639, F63B, offset, remaining); }
                } while (exponent);
                LR(F63D, F640, offset, remaining);
            }
        }
        LR(F642, F644, offset, remaining);
        offset += 2u;
        --remaining;
        LB(F645, remaining != 0u);
    }
    return true;
}

static bool OutcomeAlternateNextLine(TableOutcomeClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    if (cpu->X > 8u || (cpu->X & 1u) || cpu->Y < 1u || cpu->Y > 5u ||
        cpu->X + cpu->Y * 2u > 10u) return false;
    LR(F642, F644, cpu->X, cpu->Y);
    LB(F645, cpu->Y != 1u);
    if (cpu->Y == 1u) return true;
    return OutcomeAlternateLines(clock, cpu->X + 2u, cpu->Y - 1u);
}

static bool PlanOutcomeCalculateAlternate(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xf5a5u) {
        const unsigned selection = TableOutcomeBankWord(cpu, 0x7184u);
        if (selection < 1u || selection > 5u || cpu->Y != 0u) return false;
        unsigned offset = 0u, row = 0u;
        LR(F5A5, F5AD, 0u, 0u);
        for (unsigned line = 0u; line < selection; ++line) {
            const bool seven = TableOutcomeBankWord(cpu,
                (uint16_t)(0x7150u + row)) == 7u;
            LR(F5AF, F5B2, offset, row);
            LB(F5B5, !seven);
            if (seven) { LR(F5B7, F5BB, offset, row); }
            LR(F5BE, F5C2, offset, row);
            offset += 2u;
            row += 2u;
            LB(F5C5, offset != 6u);
            if (offset == 6u) {
                LR(F5C7, F5C7, offset, row);
                row = 2u;
            }
            LR(F5CA, F5CA, offset, row);
            LB(F5CC, line + 1u != selection);
        }
        LR(F5CE, F5D1, offset, row);
        return OutcomeAlternateLines(clock, 0u, selection);
    }
    if (pc == 0xf603u) { LC(F603); }
    if (pc == 0xf606u) { LC(F606); }
    if (pc == 0xf609u) {
        LR(F609, F609, 0u, 0u);
        LC(F60C);
    }
    if (pc == 0xf60fu) {
        const bool more = (uint16_t)(cpu->A - 1u) != 0u;
        LR(F60F, F60F, cpu->X, cpu->Y);
        LB(F610, more);
        if (more) { LC(F60C); }
        LR(F612, F612, cpu->X, cpu->Y);
        LC(F615);
    }
    if (pc == 0xf618u) { LC(F618); }
    if (pc == 0xf61bu) {
        LR(F61B, F61B, cpu->X, cpu->Y);
        return OutcomeAlternateNextLine(clock);
    }
    if (pc == 0xf62eu) {
        LR(F62E, F62E, cpu->X, cpu->Y);
        return OutcomeAlternateNextLine(clock);
    }
    return false;
}

static bool OutcomeModeFinish(TableOutcomeClock *clock) {
    const bool quiet = TableOutcomeBankWord(clock->call.cpu, 0x71e2u) == 0u;
    LR(E15D, E15D, 0u, 0u);
    LB(E160, quiet);
    if (quiet) return true;
    LR(E162, E16B, 0u, 0u);
    LC(E16E);
}

static bool OutcomeModePickSymbol(TableOutcomeClock *clock) {
    LR(E121, E121, 0u, 0u);
    LC(E124);
}

static bool OutcomeModeOrdinary(TableOutcomeClock *clock) {
    const bool active = (TableOutcomeBankWord(clock->call.cpu, 0x7170u) & 1u) != 0u;
    LR(E098, E09B, 0u, 0u);
    LB(E09E, active);
    if (active) {
        LR(E0D9, E0E2, 0u, 0u);
        return OutcomeModePickSymbol(clock);
    }
    LR(E0A0, E0A0, 0u, 0u);
    LC(E0A3);
}

static bool OutcomeModeSecondChance(TableOutcomeClock *clock) {
    LR(E022, E028, 0u, 0u);
    LC(E02C);
}

static bool OutcomeModeFirstChance(TableOutcomeClock *clock) {
    LR(DF96, DF9C, 0u, 0u);
    LC(DFA0);
}

static bool OutcomeModeFirstAward(TableOutcomeClock *clock, uint16_t pending) {
    LR(DFD0, DFDF, 0u, 0u);
    LB(DFE2, pending != 1u);
    if (pending == 1u) {
        LR(DFE4, DFE4, 0u, 0u);
        LC(DFE7);
    }
    LR(DFF5, DFF5, 0u, 0u);
    LB(DFF8, pending != 2u);
    if (pending == 2u) {
        LR(DFFA, DFFA, 0u, 0u);
        LC(DFFD);
    }
    LR(E00B, E00B, 0u, 0u);
    LB(E00E, pending != 4u);
    if (pending == 4u) {
        LR(E010, E010, 0u, 0u);
        LC(E013);
    }
    LR(E01F, E01F, 0u, 0u);
    return OutcomeModeFinish(clock);
}

static bool OutcomeModeSecondAward(TableOutcomeClock *clock, uint16_t pending) {
    LR(E05C, E06B, 0u, 0u);
    LB(E06E, pending != 3u);
    if (pending == 3u) {
        LR(E070, E070, 0u, 0u);
        LC(E073);
    }
    LR(E081, E081, 0u, 0u);
    LB(E084, pending != 2u);
    if (pending == 2u) {
        LR(E086, E086, 0u, 0u);
        LC(E089);
    }
    LR(E095, E095, 0u, 0u);
    return OutcomeModeFinish(clock);
}

static bool OutcomeModeStorePending(TableOutcomeClock *clock, uint16_t pc) {
    if (pc == 0xdf6cu) { LR(DF6C, DF6F, 0u, 0u); }
    else if (pc == 0xdf71u) { LR(DF71, DF74, 0u, 0u); }
    else if (pc == 0xdf76u) { LR(DF76, DF79, 0u, 0u); }
    else { LR(DF90, DF90, 0u, 0u); }
    LR(DF93, DF93, 0u, 0u);
    return OutcomeModeFirstChance(clock);
}

static bool PlanOutcomeMode(TableOutcomeClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    const uint16_t pending = TableOutcomeBankWord(cpu, 0x7172u);
    const uint16_t result = cpu->A;
    if (pc == 0xdedcu) {
        const uint16_t variant = TableOutcomeBankWord(cpu, 0x7170u);
        LR(DEDC, DEDF, 0u, 0u);
        LB(DEE2, (variant & 6u) == 0u);
        if (variant & 6u) {
            LR(DEE4, DEE4, 0u, 0u);
            return true;
        }
        const uint16_t flags = TableOutcomeBankWord(cpu, 0x716eu) & 0xff00u;
        LR(DEE7, DEF3, 0u, 0u);
        LB(DEF6, flags != 0x100u);
        if (flags == 0x100u) {
            LR(DEF8, DEFE, 0u, 0u);
            return OutcomeModeFinish(clock);
        }
        LR(DF01, DF01, 0u, 0u);
        LB(DF04, flags != 0x200u);
        if (flags == 0x200u) {
            LR(DF06, DF0C, 0u, 0u);
            return OutcomeModeFinish(clock);
        }
        LR(DF0F, DF0F, 0u, 0u);
        LB(DF12, variant == 0u);
        if (variant) {
            LR(DF14, DF14, 0u, 0u);
            return OutcomeModeOrdinary(clock);
        }
        LR(DF17, DF17, 0u, 0u);
        LB(DF1A, pending != 0u);
        if (pending) return OutcomeModeFirstChance(clock);
        const uint16_t deferred = TableOutcomeBankWord(cpu, 0x7174u);
        LR(DF1C, DF1C, 0u, 0u);
        LB(DF1F, deferred == 0u);
        if (!deferred) return OutcomeModeFirstChance(clock);
        LR(DF21, DF24, 0u, 0u);
        LB(DF27, deferred != 1u);
        if (deferred != 1u) {
            LR(DF4D, DF4D, 0u, 0u);
            LC(DF50);
        }
        LR(DF29, DF29, 0u, 0u);
        LC(DF2C);
    }
    if (pc == 0xdf30u) {
        LR(DF30, DF30, 0u, 0u);
        LB(DF33, result != 0u);
        if (result) return OutcomeModeFirstChance(clock);
        LR(DF35, DF35, 0u, 0u);
        LC(DF38);
    }
    if (pc == 0xdf3cu) {
        LR(DF3C, DF3C, 0u, 0u);
        LB(DF3F, result == 0u);
        if (!result) {
            LR(DF7B, DF8D, 0u, 0u);
            return OutcomeModeFinish(clock);
        }
        LR(DF41, DF41, 0u, 0u);
        const bool below3 = ((uint16_t)(result - 3u) & 0x8000u) != 0u;
        LB(DF44, below3);
        if (below3) return OutcomeModeStorePending(clock, 0xdf76u);
        LR(DF46, DF46, 0u, 0u);
        const bool below5 = ((uint16_t)(result - 5u) & 0x8000u) != 0u;
        LB(DF49, below5);
        if (below5) return OutcomeModeStorePending(clock, 0xdf71u);
        LR(DF4B, DF4B, 0u, 0u);
        return OutcomeModeStorePending(clock, 0xdf6cu);
    }
    if (pc == 0xdf54u) {
        LR(DF54, DF54, 0u, 0u);
        LB(DF57, result == 0u);
        if (!result) return OutcomeModeStorePending(clock, 0xdf90u);
        LR(DF59, DF59, 0u, 0u);
        const bool below = ((uint16_t)(result - 3u) & 0x8000u) != 0u;
        LB(DF5C, below);
        if (below) return OutcomeModeFirstChance(clock);
        LR(DF5E, DF5E, 0u, 0u);
        LC(DF61);
    }
    if (pc == 0xdf65u) {
        LR(DF65, DF65, 0u, 0u);
        const bool below = ((uint16_t)(result - 1u) & 0x8000u) != 0u;
        LB(DF68, below);
        if (below) return OutcomeModeStorePending(clock, 0xdf71u);
        LB(DF6A, result == 1u);
        return OutcomeModeStorePending(clock, result == 1u ? 0xdf76u : 0xdf6cu);
    }
    if (pc == 0xdfa4u) {
        LR(DFA4, DFA4, 0u, 0u);
        LB(DFA7, result != 0u);
        if (result) return OutcomeModeSecondChance(clock);
        LR(DFA9, DFAD, 0u, 0u);
        LC(DFB0);
    }
    if (pc == 0xdfb4u) {
        LR(DFB4, DFB4, 0u, 0u);
        LB(DFB7, result != 0u);
        if (result) return OutcomeModeSecondChance(clock);
        LR(DFB9, DFBC, 0u, 0u);
        LB(DFBF, pending != 5u);
        if (pending != 5u) return OutcomeModeFirstAward(clock, pending);
        LR(DFC1, DFC1, 0u, 0u);
        LC(DFC4);
    }
    if (pc == 0xdfc8u) {
        LR(DFC8, DFC8, 0u, 0u);
        LB(DFCB, result != 0u);
        if (result) return OutcomeModeSecondChance(clock);
        LR(DFCD, DFCD, 0u, 0u);
        return OutcomeModeFirstAward(clock, 0u);
    }
    if (pc == 0xdfebu || pc == 0xe001u || pc == 0xe017u) {
        const bool first = pc == 0xdfebu;
        const bool keep = first ? result == 0u :
            ((uint16_t)(result - 2u) & 0x8000u) != 0u;
        if (first) { LR(DFEB, DFEB, 0u, 0u); LB(DFEE, keep); }
        else if (pc == 0xe001u) { LR(E001, E001, 0u, 0u); LB(E004, keep); }
        else { LR(E017, E017, 0u, 0u); LB(E01A, keep); }
        if (!keep) {
            if (first) { LR(DFF0, DFF3, 0u, 0u); }
            else if (pc == 0xe001u) { LR(E006, E009, 0u, 0u); }
            else { LR(E01C, E01C, 0u, 0u); }
        }
        LR(E01F, E01F, 0u, 0u);
        return OutcomeModeFinish(clock);
    }
    if (pc == 0xe030u) {
        LR(E030, E030, 0u, 0u);
        LB(E033, result != 0u);
        if (result) return OutcomeModeOrdinary(clock);
        LR(E035, E039, 0u, 0u);
        LC(E03C);
    }
    if (pc == 0xe040u) {
        LR(E040, E040, 0u, 0u);
        LB(E043, result != 0u);
        if (result) return OutcomeModeOrdinary(clock);
        LR(E045, E048, 0u, 0u);
        LB(E04B, pending != 5u);
        if (pending != 5u) return OutcomeModeSecondAward(clock, pending);
        LR(E04D, E04D, 0u, 0u);
        LC(E050);
    }
    if (pc == 0xe054u) {
        LR(E054, E054, 0u, 0u);
        LB(E057, result != 0u);
        if (result) return OutcomeModeOrdinary(clock);
        LR(E059, E059, 0u, 0u);
        return OutcomeModeSecondAward(clock, 0u);
    }
    if (pc == 0xe077u || pc == 0xe08du) {
        const bool first = pc == 0xe077u;
        if (first) { LR(E077, E077, 0u, 0u); LB(E07A, result == 0u); }
        else { LR(E08D, E08D, 0u, 0u); LB(E090, result != 0u); }
        if (first ? result != 0u : result == 0u) {
            if (first) { LR(E07C, E07F, 0u, 0u); }
            else { LR(E092, E092, 0u, 0u); }
        }
        LR(E095, E095, 0u, 0u);
        return OutcomeModeFinish(clock);
    }
    if (pc == 0xe0a7u) {
        LR(E0A7, E0A7, 0u, 0u);
        LB(E0AA, result != 4u);
        if (result != 4u) { LR(E0E4, E0E4, 0u, 0u); LC(E0E7); }
        LR(E0AC, E0B0, 0u, 0u);
        LC(E0B3);
    }
    if (pc == 0xe0b7u) {
        LR(E0B7, E0B7, 0u, 0u);
        LB(E0BA, result != 0u);
        if (result) { LR(E0E4, E0E4, 0u, 0u); LC(E0E7); }
        LR(E0BC, E0BC, 0u, 0u);
        LC(E0BF);
    }
    if (pc == 0xe0c3u) {
        LR(E0C3, E0C7, result, 0u);
        LB(E0CA, result != 0u);
        if (!result) { LR(E0CC, E0CD, result, 0u); }
        LR(E0D0, E0E2, result, 0u);
        return OutcomeModePickSymbol(clock);
    }
    if (pc == 0xe0ebu) {
        LR(E0EB, E0EB, 0u, 0u);
        LB(E0EE, result != 4u);
        if (result != 4u) return OutcomeModeFinish(clock);
        LR(E0F0, E0F4, 0u, 0u);
        LC(E0F7);
    }
    if (pc == 0xe0fbu) {
        LR(E0FB, E0FB, 0u, 0u);
        LB(E0FE, result != 0u);
        if (result) return OutcomeModeFinish(clock);
        LR(E100, E109, 0u, 0u);
        LC(E10C);
    }
    if (pc == 0xe110u) {
        LR(E110, E110, 0u, 0u);
        LB(E113, result != 2u);
        if (result != 2u) return OutcomeModePickSymbol(clock);
        LR(E115, E115, 0u, 0u);
        LC(E118);
    }
    if (pc == 0xe11cu) {
        LR(E11C, E11C, 0u, 0u);
        LB(E11F, result == 100u);
        if (result != 100u) return OutcomeModePickSymbol(clock);
        LR(E0BC, E0BC, 0u, 0u);
        LC(E0BF);
    }
    if (pc == 0xe128u) {
        LR(E128, E12F, 0u, 0u);
        LC(E132);
    }
    if (pc == 0xe136u) {
        LR(E136, E136, 0u, 0u);
        LB(E139, result != 4u);
        if (result == 4u) {
            LR(E13B, E141, 0u, 0u);
            return OutcomeModeFinish(clock);
        }
        LR(E143, E143, 0u, 0u);
        LC(E146);
    }
    if (pc == 0xe14au) {
        LR(E14A, E14A, 0u, 0u);
        LB(E14D, result != 4u);
        if (result != 4u) return OutcomeModeFinish(clock);
        LR(E14F, E14F, 0u, 0u);
        LC(E152);
    }
    if (pc == 0xe156u) {
        LR(E156, E15A, 0u, 0u);
        return OutcomeModeFinish(clock);
    }
    if (pc == 0xe171u) {
        LR(E171, E17E, 0u, 0u);
        LC(E181);
    }
    if (pc == 0xe184u) {
        LR(E184, E18D, 0u, 0u);
        LC(E190);
    }
    return pc == 0xe193u;
}

#undef LR
#undef LB
#undef LC
static uint64_t TableOutcomeBudget(const TableOutcomeClock *clock) {
    const uint32_t budget=clock->budget;
    return budget+40u*(budget/1324u+1u);
}
static bool TableOutcomeByteLanding(uint32_t pc) {
    switch(pc) {
    case 0x84e3ceu:case 0x84f575u:case 0x84f57cu:case 0x84f57fu:
    case 0x84f6a9u:case 0x84f716u:return true;
    default:return false;
    }
}
static bool PrepareTableOutcomeClock(TableOutcomeClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = table_outcome_ends[clock->root];
    clock->returning = true;
    clock->budget=64u;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x28000u || cpu->PB != 0x84u || pc >> 16 != 0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        clock->call.frame.hrv != 2u || cpu->host_return_valid < 2u || cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const bool entry = pc == table_outcome_entries[clock->root];
    if ((entry && cpu->host_return_valid != 2u) || cpu->x_flag ||
        cpu->m_flag!=(unsigned)TableOutcomeByteLanding(pc)) return false;
    bool planned;
    switch (clock->root) {
    case 0u: planned=PlanOutcomeSong(clock,(uint16_t)pc); break;
    case 1u: planned=PlanOutcomeSelectedRound(clock,(uint16_t)pc); break;
    case 2u: planned=PlanOutcomeMode(clock,(uint16_t)pc); break;
    case 3u: planned=PlanOutcomePayout(clock,(uint16_t)pc); break;
    case 4u: planned=PlanOutcomePairMarks(clock,(uint16_t)pc); break;
    case 5u: planned=PlanOutcomeRoundOutcome(clock,(uint16_t)pc); break;
    case 6u: planned=PlanOutcomeMatches(clock,(uint16_t)pc); break;
    case 8u: planned=PlanOutcomeStopAlternate(clock,(uint16_t)pc); break;
    case 9u: planned=PlanOutcomeAlternatePayout(clock,(uint16_t)pc); break;
    case 10u: planned=PlanOutcomeAlternatePairMarks(clock,(uint16_t)pc); break;
    case 11u: planned=PlanOutcomeCalculateAlternate(clock,(uint16_t)pc); break;
    case 12u: planned=PlanOutcomeAlternateChoices(clock,(uint16_t)pc); break;
    case 13u: planned=PlanOutcomePayStake(clock,(uint16_t)pc); break;
    default:return false;
    }
    if (!planned) return false;
    const uint64_t budget = TableOutcomeBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, table_outcome_entries[clock->root],
            0x1ff7u, true, 1363u, budget)) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
static uint8_t TableOutcomeChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableOutcomeClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(!clock->returning && clock->next == site + frame_size + 1u);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableOutcomeClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}
static RecompReturn ActorBridgeTableOutcome(CpuState *cpu, unsigned root) {
    if (root==7u) return ActorBridgeTableRestoreTilemap(cpu);
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableOutcomeClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTableOutcomeClock(&clock, table_outcome_entries[root]))
        return ActorBridgeFallback(cpu, &frame, table_outcome_entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result=Lufia2SpecialMenuPlaySong(&memory, &state, TableOutcomeChild, &clock); break;
    case 1u: result=Lufia2CardTableRunSelectedRound(&memory, &state, TableOutcomeChild, &clock); break;
    case 2u: result=Lufia2CardTableChooseRoundMode(&memory, &state, TableOutcomeChild, &clock); break;
    case 3u: result=Lufia2CardTablePayRound(&memory, &state, TableOutcomeChild, &clock); break;
    case 4u: result=Lufia2CardTableMarkPairedLines(&memory, &state); break;
    case 5u: result=Lufia2CardTableUpdateRoundOutcome(&memory, &state, TableOutcomeChild, &clock); break;
    case 6u: result=Lufia2CardTableEvaluateMatches(&memory, &state, TableOutcomeChild, &clock); break;
    case 8u: result=Lufia2CardTableStopAlternateColumns(&memory, &state, TableOutcomeChild, &clock); break;
    case 9u: result=Lufia2CardTablePayAlternateRound(&memory, &state, TableOutcomeChild, &clock); break;
    case 10u: result=Lufia2CardTableMarkAlternatePairedLines(&memory, &state); break;
    case 11u: result=Lufia2CardTableCalculateAlternatePayout(&memory, &state, TableOutcomeChild, &clock); break;
    case 12u: result=Lufia2CardTableChooseAlternateLines(&memory, &state, TableOutcomeChild, &clock); break;
    case 13u: result=Lufia2CardTableTryPayStake(&memory, &state, TableOutcomeChild, &clock); break;
    default: return ActorBridgeFallback(cpu,&frame,table_outcome_entries[root]);
    }
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc, frame.entry_s, frame.hrv);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.returning && result.pc == clock.next && clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

