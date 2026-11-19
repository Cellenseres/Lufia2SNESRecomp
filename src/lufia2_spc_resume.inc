static LUFIA2_SPC_INLINE unsigned SpcResumeAt(Spc *spc, uint8_t opcode) {
    if (spc->apu->ram[spc->pc] != opcode)
        return 0u;
    spc->cyclesUsed = 0u;
    (void)spc_readOpcode(spc);
    spc->cyclesUsed = cyclesPerOpcode[opcode];
    SpcResumeDoOpcode(spc, opcode);
    return spc->cyclesUsed;
}

unsigned Lufia2SpcResumeInstruction(Spc *spc) {
    switch (spc->pc) {
    case 0x406au:
    case 0x408au:
    case 0x40cau:
    case 0x40e0u:
        return SpcResumeAt(spc, 0x03u);
    case 0x4064u:
        return SpcResumeAt(spc, 0x10u);
    case 0x0388u:
        return SpcResumeAt(spc, 0x13u);
    case 0x404au:
    case 0x4052u:
    case 0x4076u:
    case 0x414eu:
        return SpcResumeAt(spc, 0x1cu);
    case 0x35ffu:
    case 0x401cu:
    case 0x4025u:
    case 0x408fu:
    case 0x409au:
    case 0x40f3u:
    case 0x40fcu:
    case 0x410cu:
    case 0x4196u:
    case 0x41a3u:
        return SpcResumeAt(spc, 0x28u);
    case 0x4039u:
    case 0x4081u:
    case 0x4114u:
    case 0x4178u:
        return SpcResumeAt(spc, 0x2fu);
    case 0x4169u:
        return SpcResumeAt(spc, 0x30u);
    case 0x3636u:
    case 0x40bau:
    case 0x4187u:
    case 0x41b9u:
        return SpcResumeAt(spc, 0x3du);
    case 0x0398u:
    case 0x363bu:
    case 0x363eu:
    case 0x3641u:
    case 0x3c50u:
        return SpcResumeAt(spc, 0x3fu);
    case 0x407du:
        return SpcResumeAt(spc, 0x48u);
    case 0x404eu:
    case 0x4193u:
        return SpcResumeAt(spc, 0x4du);
    case 0x4085u:
        return SpcResumeAt(spc, 0x5cu);
    case 0x4020u:
    case 0x40bfu:
    case 0x40f7u:
    case 0x418cu:
        return SpcResumeAt(spc, 0x5fu);
    case 0x4061u:
    case 0x415cu:
    case 0x416du:
    case 0x41b4u:
        return SpcResumeAt(spc, 0x60u);
    case 0x4070u:
    case 0x4106u:
        return SpcResumeAt(spc, 0x68u);
    case 0x3644u:
    case 0x3c53u:
    case 0x3f2fu:
    case 0x40eeu:
    case 0x41beu:
        return SpcResumeAt(spc, 0x6fu);
    case 0x40cdu:
        return SpcResumeAt(spc, 0x73u);
    case 0x40aeu:
        return SpcResumeAt(spc, 0x7du);
    case 0x4062u:
    case 0x415du:
        return SpcResumeAt(spc, 0x84u);
    case 0x4162u:
    case 0x41b5u:
        return SpcResumeAt(spc, 0x88u);
    case 0x039bu:
        return SpcResumeAt(spc, 0x8bu);
    case 0x4191u:
        return SpcResumeAt(spc, 0x8du);
    case 0x3f23u:
    case 0x40d3u:
    case 0x40e9u:
        return SpcResumeAt(spc, 0x8fu);
    case 0x4055u:
    case 0x40a5u:
        return SpcResumeAt(spc, 0x90u);
    case 0x40e3u:
        return SpcResumeAt(spc, 0x93u);
    case 0x416eu:
    case 0x4174u:
        return SpcResumeAt(spc, 0x94u);
    case 0x3607u:
    case 0x3614u:
        return SpcResumeAt(spc, 0x9bu);
    case 0x40afu:
        return SpcResumeAt(spc, 0x9fu);
    case 0x407bu:
        return SpcResumeAt(spc, 0xa4u);
    case 0x40b5u:
    case 0x41adu:
        return SpcResumeAt(spc, 0xabu);
    case 0x402eu:
        return SpcResumeAt(spc, 0xb0u);
    case 0x038fu:
    case 0x3f2bu:
    case 0x3f2du:
    case 0x4068u:
    case 0x407fu:
    case 0x4086u:
    case 0x4088u:
    case 0x40b0u:
    case 0x40d6u:
    case 0x40ecu:
    case 0x415fu:
    case 0x4164u:
    case 0x41abu:
    case 0x41b1u:
        return SpcResumeAt(spc, 0xc4u);
    case 0x3637u:
    case 0x402cu:
    case 0x40a3u:
    case 0x40bbu:
    case 0x4188u:
    case 0x41bau:
        return SpcResumeAt(spc, 0xc8u);
    case 0x4037u:
    case 0x4078u:
    case 0x4150u:
    case 0x4155u:
    case 0x41a7u:
        return SpcResumeAt(spc, 0xcbu);
    case 0x35fbu:
    case 0x4018u:
    case 0x40efu:
    case 0x418fu:
        return SpcResumeAt(spc, 0xcdu);
    case 0x4054u:
    case 0x41b8u:
        return SpcResumeAt(spc, 0xceu);
    case 0x4036u:
    case 0x404du:
    case 0x4053u:
    case 0x4077u:
    case 0x40c8u:
    case 0x40deu:
    case 0x414fu:
    case 0x4154u:
    case 0x415bu:
        return SpcResumeAt(spc, 0xcfu);
    case 0x039du:
    case 0x3609u:
    case 0x3616u:
    case 0x3639u:
    case 0x401eu:
    case 0x40f5u:
    case 0x4108u:
    case 0x41bcu:
        return SpcResumeAt(spc, 0xd0u);
    case 0x410eu:
    case 0x4112u:
    case 0x4170u:
    case 0x4176u:
        return SpcResumeAt(spc, 0xd4u);
    case 0x4060u:
    case 0x407au:
    case 0x40c9u:
    case 0x40dfu:
    case 0x4161u:
    case 0x41b3u:
        return SpcResumeAt(spc, 0xddu);
    case 0x038bu:
    case 0x039fu:
    case 0x03a6u:
    case 0x3f08u:
    case 0x3f15u:
    case 0x4083u:
    case 0x416bu:
    case 0x4172u:
        return SpcResumeAt(spc, 0xe4u);
    case 0x40c2u:
    case 0x40d8u:
        return SpcResumeAt(spc, 0xe5u);
    case 0x3f29u:
        return SpcResumeAt(spc, 0xe8u);
    case 0x0391u:
        return SpcResumeAt(spc, 0xe9u);
    case 0x404bu:
    case 0x4074u:
    case 0x4157u:
        return SpcResumeAt(spc, 0xebu);
    case 0x4033u:
    case 0x40c5u:
    case 0x40dbu:
        return SpcResumeAt(spc, 0xecu);
    case 0x038du:
    case 0x0394u:
    case 0x03a1u:
    case 0x03a8u:
    case 0x3601u:
    case 0x3605u:
    case 0x3f0au:
    case 0x3f17u:
    case 0x4027u:
    case 0x4043u:
    case 0x4048u:
    case 0x4072u:
    case 0x4091u:
    case 0x409cu:
    case 0x40bdu:
    case 0x40feu:
    case 0x4148u:
    case 0x418au:
    case 0x4198u:
    case 0x41a5u:
        return SpcResumeAt(spc, 0xf0u);
    case 0x35fdu:
    case 0x3603u:
    case 0x401au:
    case 0x4023u:
    case 0x408du:
    case 0x4098u:
    case 0x40f1u:
    case 0x40fau:
    case 0x410au:
    case 0x4110u:
    case 0x4152u:
    case 0x4159u:
    case 0x4194u:
    case 0x41a1u:
    case 0x41a9u:
    case 0x41afu:
        return SpcResumeAt(spc, 0xf4u);
    case 0x4030u:
    case 0x4040u:
    case 0x4045u:
    case 0x404fu:
    case 0x406du:
    case 0x4103u:
    case 0x4145u:
    case 0x414bu:
    case 0x4166u:
        return SpcResumeAt(spc, 0xf5u);
    case 0x3f26u:
    case 0x40b2u:
    case 0x40b7u:
        return SpcResumeAt(spc, 0xfau);
    case 0x414au:
    case 0x41b7u:
        return SpcResumeAt(spc, 0xfdu);
    case 0x3f21u:
        return SpcResumeAt(spc, 0xfeu);
    default:
        return 0u;
    }
}

#undef LUFIA2_SPC_INLINE
