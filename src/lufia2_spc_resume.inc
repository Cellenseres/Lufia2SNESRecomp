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
    case 0x0520u:
    case 0x05e8u:
        return SpcResumeAt(spc, 0x08u);
    case 0x3d01u:
    case 0x4064u:
        return SpcResumeAt(spc, 0x10u);
    case 0x0388u:
        return SpcResumeAt(spc, 0x13u);
    case 0x03b3u:
    case 0x3a0eu:
    case 0x3fc5u:
    case 0x3fc6u:
    case 0x3fc7u:
    case 0x404au:
    case 0x4052u:
    case 0x4076u:
    case 0x414eu:
        return SpcResumeAt(spc, 0x1cu);
    case 0x3d00u:
    case 0x4294u:
        return SpcResumeAt(spc, 0x1du);
    case 0x03b5u:
        return SpcResumeAt(spc, 0x1fu);
    case 0x05c5u:
    case 0x35ffu:
    case 0x3dc6u:
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
    case 0x051eu:
        return SpcResumeAt(spc, 0x2du);
    case 0x05ecu:
    case 0x4039u:
    case 0x4081u:
    case 0x4114u:
    case 0x4178u:
        return SpcResumeAt(spc, 0x2fu);
    case 0x4169u:
        return SpcResumeAt(spc, 0x30u);
    case 0x3a10u:
        return SpcResumeAt(spc, 0x3cu);
    case 0x3636u:
    case 0x3fd2u:
    case 0x40bau:
    case 0x4187u:
    case 0x41b9u:
        return SpcResumeAt(spc, 0x3du);
    case 0x0385u:
    case 0x0398u:
    case 0x03a3u:
    case 0x04a9u:
    case 0x04acu:
    case 0x04b8u:
    case 0x04bdu:
    case 0x04c2u:
    case 0x04c7u:
    case 0x04ccu:
    case 0x04d3u:
    case 0x04d9u:
    case 0x04dfu:
    case 0x04e5u:
    case 0x363bu:
    case 0x363eu:
    case 0x3641u:
    case 0x3c3eu:
    case 0x3c41u:
    case 0x3c44u:
    case 0x3c47u:
    case 0x3c4au:
    case 0x3c4du:
    case 0x3c50u:
    case 0x42a9u:
    case 0x42aeu:
    case 0x42c0u:
        return SpcResumeAt(spc, 0x3fu);
    case 0x407du:
        return SpcResumeAt(spc, 0x48u);
    case 0x404eu:
    case 0x4193u:
        return SpcResumeAt(spc, 0x4du);
    case 0x3c61u:
        return SpcResumeAt(spc, 0x53u);
    case 0x4085u:
        return SpcResumeAt(spc, 0x5cu);
    case 0x03b4u:
    case 0x05d4u:
    case 0x3fc8u:
        return SpcResumeAt(spc, 0x5du);
    case 0x04afu:
    case 0x05ccu:
    case 0x3c9au:
    case 0x3dcau:
    case 0x4020u:
    case 0x40bfu:
    case 0x40f7u:
    case 0x418cu:
        return SpcResumeAt(spc, 0x5fu);
    case 0x04f2u:
    case 0x050du:
    case 0x3fd4u:
    case 0x4061u:
    case 0x415cu:
    case 0x416du:
    case 0x41b4u:
        return SpcResumeAt(spc, 0x60u);
    case 0x03aau:
    case 0x0518u:
    case 0x05bcu:
    case 0x05cfu:
        return SpcResumeAt(spc, 0x64u);
    case 0x03aeu:
    case 0x3fd8u:
    case 0x4070u:
    case 0x4106u:
        return SpcResumeAt(spc, 0x68u);
    case 0x0516u:
    case 0x0526u:
    case 0x0602u:
    case 0x3644u:
    case 0x3a17u:
    case 0x3c53u:
    case 0x3c60u:
    case 0x3c92u:
    case 0x3d03u:
    case 0x3e2bu:
    case 0x3f2fu:
    case 0x3fc1u:
    case 0x3fdcu:
    case 0x40eeu:
    case 0x41beu:
    case 0x42c3u:
        return SpcResumeAt(spc, 0x6fu);
    case 0x3c7cu:
    case 0x40cdu:
        return SpcResumeAt(spc, 0x73u);
    case 0x0372u:
    case 0x0377u:
    case 0x0511u:
    case 0x05c7u:
        return SpcResumeAt(spc, 0x78u);
    case 0x0504u:
        return SpcResumeAt(spc, 0x7au);
    case 0x05e7u:
    case 0x40aeu:
        return SpcResumeAt(spc, 0x7du);
    case 0x04f3u:
    case 0x04feu:
    case 0x4062u:
    case 0x415du:
        return SpcResumeAt(spc, 0x84u);
    case 0x3fd5u:
    case 0x4162u:
    case 0x41b5u:
        return SpcResumeAt(spc, 0x88u);
    case 0x039bu:
        return SpcResumeAt(spc, 0x8bu);
    case 0x0365u:
    case 0x04bbu:
    case 0x05c2u:
    case 0x3fc9u:
    case 0x4191u:
        return SpcResumeAt(spc, 0x8du);
    case 0x035fu:
    case 0x0362u:
    case 0x0369u:
    case 0x036cu:
    case 0x036fu:
    case 0x037cu:
    case 0x037fu:
    case 0x0382u:
    case 0x05eeu:
    case 0x05f1u:
    case 0x3f23u:
    case 0x3fb7u:
    case 0x3fbcu:
    case 0x40d3u:
    case 0x40e9u:
    case 0x4297u:
    case 0x429au:
        return SpcResumeAt(spc, 0x8fu);
    case 0x4055u:
    case 0x40a5u:
        return SpcResumeAt(spc, 0x90u);
    case 0x40e3u:
        return SpcResumeAt(spc, 0x93u);
    case 0x416eu:
    case 0x4174u:
        return SpcResumeAt(spc, 0x94u);
    case 0x050eu:
        return SpcResumeAt(spc, 0x98u);
    case 0x3607u:
    case 0x3614u:
        return SpcResumeAt(spc, 0x9bu);
    case 0x03b2u:
        return SpcResumeAt(spc, 0x9cu);
    case 0x40afu:
        return SpcResumeAt(spc, 0x9fu);
    case 0x407bu:
        return SpcResumeAt(spc, 0xa4u);
    case 0x05e5u:
    case 0x40b5u:
    case 0x41adu:
        return SpcResumeAt(spc, 0xabu);
    case 0x0525u:
        return SpcResumeAt(spc, 0xaeu);
    case 0x03b0u:
    case 0x402eu:
        return SpcResumeAt(spc, 0xb0u);
    case 0x0502u:
        return SpcResumeAt(spc, 0xbau);
    case 0x0339u:
    case 0x033bu:
    case 0x033du:
    case 0x033fu:
    case 0x0341u:
    case 0x0343u:
    case 0x0345u:
    case 0x0347u:
    case 0x0349u:
    case 0x034bu:
    case 0x034du:
    case 0x034fu:
    case 0x0356u:
    case 0x0358u:
    case 0x038fu:
    case 0x04c0u:
    case 0x04c5u:
    case 0x04cau:
    case 0x04cfu:
    case 0x04f0u:
    case 0x04f5u:
    case 0x04fcu:
    case 0x0500u:
    case 0x0522u:
    case 0x05c0u:
    case 0x05eau:
    case 0x05feu:
    case 0x0600u:
    case 0x3f2bu:
    case 0x3f2du:
    case 0x3fbau:
    case 0x3fbfu:
    case 0x3fd0u:
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
    case 0x429fu:
        return SpcResumeAt(spc, 0xc4u);
    case 0x032au:
    case 0x032fu:
    case 0x0334u:
    case 0x0351u:
    case 0x035cu:
    case 0x3a11u:
    case 0x3a14u:
    case 0x42a1u:
    case 0x42a6u:
    case 0x42b3u:
    case 0x42b8u:
    case 0x42bdu:
        return SpcResumeAt(spc, 0xc5u);
    case 0x3637u:
    case 0x402cu:
    case 0x40a3u:
    case 0x40bbu:
    case 0x4188u:
    case 0x41bau:
        return SpcResumeAt(spc, 0xc8u);
    case 0x3fceu:
    case 0x4037u:
    case 0x4078u:
    case 0x4150u:
    case 0x4155u:
    case 0x41a7u:
        return SpcResumeAt(spc, 0xcbu);
    case 0x35fbu:
    case 0x3c93u:
    case 0x4018u:
    case 0x40efu:
    case 0x418fu:
        return SpcResumeAt(spc, 0xcdu);
    case 0x4054u:
    case 0x41b8u:
        return SpcResumeAt(spc, 0xceu);
    case 0x3a0du:
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
    case 0x0375u:
    case 0x037au:
    case 0x039du:
    case 0x03acu:
    case 0x0514u:
    case 0x051au:
    case 0x05beu:
    case 0x05cau:
    case 0x05d1u:
    case 0x05dau:
    case 0x05e3u:
    case 0x05f6u:
    case 0x05fau:
    case 0x3609u:
    case 0x3616u:
    case 0x3639u:
    case 0x3c98u:
    case 0x3dc8u:
    case 0x3fdau:
    case 0x401eu:
    case 0x40f5u:
    case 0x4108u:
    case 0x41bcu:
    case 0x4295u:
        return SpcResumeAt(spc, 0xd0u);
    case 0x410eu:
    case 0x4112u:
    case 0x4170u:
    case 0x4176u:
        return SpcResumeAt(spc, 0xd4u);
    case 0x04d6u:
    case 0x04dcu:
    case 0x04e2u:
    case 0x04e8u:
    case 0x04edu:
    case 0x04f9u:
    case 0x0506u:
    case 0x050au:
        return SpcResumeAt(spc, 0xd5u);
    case 0x05d7u:
    case 0x05e0u:
        return SpcResumeAt(spc, 0xd7u);
    case 0x0524u:
        return SpcResumeAt(spc, 0xdcu);
    case 0x0509u:
    case 0x0517u:
    case 0x051fu:
    case 0x05c4u:
    case 0x05d3u:
    case 0x3a0fu:
    case 0x3fd3u:
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
    case 0x04ebu:
    case 0x04f7u:
    case 0x051cu:
    case 0x05d5u:
    case 0x05deu:
    case 0x05f4u:
    case 0x05f8u:
    case 0x3c54u:
    case 0x3dc4u:
    case 0x3f08u:
    case 0x3f15u:
    case 0x4083u:
    case 0x416bu:
    case 0x4172u:
    case 0x4290u:
        return SpcResumeAt(spc, 0xe4u);
    case 0x3fb4u:
    case 0x3fc2u:
    case 0x40c2u:
    case 0x40d8u:
        return SpcResumeAt(spc, 0xe5u);
    case 0x0328u:
    case 0x032du:
    case 0x0332u:
    case 0x0337u:
    case 0x0354u:
    case 0x035au:
    case 0x05bau:
    case 0x05fcu:
    case 0x3f29u:
    case 0x429du:
    case 0x42a4u:
    case 0x42acu:
    case 0x42b1u:
    case 0x42b6u:
    case 0x42bbu:
        return SpcResumeAt(spc, 0xe8u);
    case 0x0391u:
        return SpcResumeAt(spc, 0xe9u);
    case 0x404bu:
    case 0x4074u:
    case 0x4157u:
        return SpcResumeAt(spc, 0xebu);
    case 0x3a0au:
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
    case 0x3c56u:
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
    case 0x4292u:
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
    case 0x3c95u:
    case 0x3fcbu:
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
    case 0x04d1u:
        return SpcResumeAt(spc, 0xf8u);
    case 0x3f26u:
    case 0x40b2u:
    case 0x40b7u:
        return SpcResumeAt(spc, 0xfau);
    case 0x05d9u:
    case 0x05e2u:
        return SpcResumeAt(spc, 0xfcu);
    case 0x3fd7u:
    case 0x414au:
    case 0x41b7u:
        return SpcResumeAt(spc, 0xfdu);
    case 0x0367u:
    case 0x3f21u:
        return SpcResumeAt(spc, 0xfeu);
    default:
        return 0u;
    }
}

#undef LUFIA2_SPC_INLINE
