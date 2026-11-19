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
    case 0x3b77u:
    case 0x3b84u:
    case 0x3b92u:
    case 0x3dbdu:
    case 0x3edeu:
    case 0x3f94u:
    case 0x3fa0u:
    case 0x3facu:
        return SpcResumeAt(spc, 0x04u);
    case 0x0520u:
    case 0x05e8u:
        return SpcResumeAt(spc, 0x08u);
    case 0x3d01u:
    case 0x4064u:
        return SpcResumeAt(spc, 0x10u);
    case 0x0388u:
        return SpcResumeAt(spc, 0x13u);
    case 0x3d5eu:
        return SpcResumeAt(spc, 0x14u);
    case 0x3d8au:
        return SpcResumeAt(spc, 0x15u);
    case 0x03b3u:
    case 0x0497u:
    case 0x0498u:
    case 0x3a0eu:
    case 0x3d11u:
    case 0x3fc5u:
    case 0x3fc6u:
    case 0x3fc7u:
    case 0x400au:
    case 0x4010u:
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
    case 0x3b9bu:
    case 0x3d94u:
    case 0x3eccu:
    case 0x3ed4u:
    case 0x3f92u:
    case 0x3f9eu:
    case 0x3faau:
        return SpcResumeAt(spc, 0x24u);
    case 0x3ec3u:
        return SpcResumeAt(spc, 0x25u);
    case 0x05c5u:
    case 0x35ffu:
    case 0x360du:
    case 0x36f7u:
    case 0x3d62u:
    case 0x3dc6u:
    case 0x3deau:
    case 0x3e91u:
    case 0x3e98u:
    case 0x3ee9u:
    case 0x3ef0u:
    case 0x3efcu:
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
    case 0x3b59u:
    case 0x3d06u:
    case 0x41cau:
        return SpcResumeAt(spc, 0x2du);
    case 0x05ecu:
    case 0x36c5u:
    case 0x3d36u:
    case 0x4039u:
    case 0x405eu:
    case 0x4081u:
    case 0x40acu:
    case 0x4114u:
    case 0x4178u:
    case 0x419fu:
        return SpcResumeAt(spc, 0x2fu);
    case 0x369fu:
    case 0x3702u:
    case 0x4169u:
        return SpcResumeAt(spc, 0x30u);
    case 0x3a10u:
        return SpcResumeAt(spc, 0x3cu);
    case 0x3636u:
    case 0x3d31u:
    case 0x3e1bu:
    case 0x3ea9u:
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
    case 0x3611u:
    case 0x363bu:
    case 0x363eu:
    case 0x3641u:
    case 0x36a1u:
    case 0x36bdu:
    case 0x36c2u:
    case 0x3710u:
    case 0x3713u:
    case 0x3c3eu:
    case 0x3c41u:
    case 0x3c44u:
    case 0x3c47u:
    case 0x3c4au:
    case 0x3c4du:
    case 0x3c50u:
    case 0x3d07u:
    case 0x3d51u:
    case 0x3d66u:
    case 0x3d9eu:
    case 0x3db2u:
    case 0x3db5u:
    case 0x3db8u:
    case 0x3de5u:
    case 0x3e2cu:
    case 0x3e2fu:
    case 0x3ea3u:
    case 0x3ea6u:
    case 0x3eb7u:
    case 0x3ebau:
    case 0x42a9u:
    case 0x42aeu:
    case 0x42c0u:
        return SpcResumeAt(spc, 0x3fu);
    case 0x3b99u:
    case 0x3d88u:
    case 0x3d8du:
    case 0x4058u:
    case 0x407du:
        return SpcResumeAt(spc, 0x48u);
    case 0x3d98u:
    case 0x404eu:
    case 0x4193u:
    case 0x41bfu:
        return SpcResumeAt(spc, 0x4du);
    case 0x3c61u:
        return SpcResumeAt(spc, 0x53u);
    case 0x4085u:
    case 0x41d6u:
        return SpcResumeAt(spc, 0x5cu);
    case 0x03b4u:
    case 0x05d4u:
    case 0x3d57u:
    case 0x3d9du:
    case 0x3e85u:
    case 0x3fc8u:
    case 0x419eu:
        return SpcResumeAt(spc, 0x5du);
    case 0x049bu:
    case 0x04afu:
    case 0x0551u:
    case 0x05ccu:
    case 0x3c9au:
    case 0x3dc1u:
    case 0x3dcau:
    case 0x4020u:
    case 0x4029u:
    case 0x40bfu:
    case 0x40f7u:
    case 0x4100u:
    case 0x418cu:
        return SpcResumeAt(spc, 0x5fu);
    case 0x04f2u:
    case 0x050du:
    case 0x36a5u:
    case 0x3b1au:
    case 0x3d19u:
    case 0x3d74u:
    case 0x3d9au:
    case 0x3fd4u:
    case 0x405bu:
    case 0x4061u:
    case 0x415cu:
    case 0x416du:
    case 0x419bu:
    case 0x41b4u:
        return SpcResumeAt(spc, 0x60u);
    case 0x03aau:
    case 0x0518u:
    case 0x05bcu:
    case 0x05cfu:
    case 0x3e9au:
        return SpcResumeAt(spc, 0x64u);
    case 0x03aeu:
    case 0x3d3du:
    case 0x3ee2u:
    case 0x3fd8u:
    case 0x4070u:
    case 0x4106u:
        return SpcResumeAt(spc, 0x68u);
    case 0x3e20u:
    case 0x3eb2u:
        return SpcResumeAt(spc, 0x69u);
    case 0x41d7u:
        return SpcResumeAt(spc, 0x6bu);
    case 0x3d50u:
    case 0x3d6fu:
    case 0x41f4u:
        return SpcResumeAt(spc, 0x6du);
    case 0x0516u:
    case 0x0526u:
    case 0x0602u:
    case 0x3644u:
    case 0x3720u:
    case 0x3a17u:
    case 0x3b27u:
    case 0x3b73u:
    case 0x3b8eu:
    case 0x3b9fu:
    case 0x3c53u:
    case 0x3c60u:
    case 0x3c92u:
    case 0x3d03u:
    case 0x3e2bu:
    case 0x3e32u:
    case 0x3ebdu:
    case 0x3f07u:
    case 0x3f2fu:
    case 0x3f8fu:
    case 0x3f9bu:
    case 0x3fa7u:
    case 0x3fb3u:
    case 0x3fc1u:
    case 0x3fdcu:
    case 0x4017u:
    case 0x40eeu:
    case 0x41beu:
    case 0x420eu:
    case 0x42c3u:
        return SpcResumeAt(spc, 0x6fu);
    case 0x3c7cu:
    case 0x40cdu:
        return SpcResumeAt(spc, 0x73u);
    case 0x3d2cu:
        return SpcResumeAt(spc, 0x75u);
    case 0x0372u:
    case 0x0377u:
    case 0x0511u:
    case 0x05c7u:
        return SpcResumeAt(spc, 0x78u);
    case 0x0504u:
    case 0x41fbu:
    case 0x4207u:
        return SpcResumeAt(spc, 0x7au);
    case 0x05e7u:
    case 0x3d99u:
    case 0x40a7u:
    case 0x40aeu:
    case 0x419au:
        return SpcResumeAt(spc, 0x7du);
    case 0x40a8u:
    case 0x417cu:
        return SpcResumeAt(spc, 0x80u);
    case 0x04f3u:
    case 0x04feu:
    case 0x3b1bu:
    case 0x3d75u:
    case 0x3d7du:
    case 0x405cu:
    case 0x4062u:
    case 0x415du:
        return SpcResumeAt(spc, 0x84u);
    case 0x3b22u:
    case 0x3d1au:
    case 0x3d21u:
    case 0x3d9bu:
    case 0x3fd5u:
    case 0x4162u:
    case 0x419cu:
    case 0x41b5u:
        return SpcResumeAt(spc, 0x88u);
    case 0x039bu:
    case 0x3dabu:
    case 0x3eaeu:
        return SpcResumeAt(spc, 0x8bu);
    case 0x0365u:
    case 0x04bbu:
    case 0x05c2u:
    case 0x369bu:
    case 0x3d46u:
    case 0x3e7bu:
    case 0x3f11u:
    case 0x3f1fu:
    case 0x3fc9u:
    case 0x4191u:
    case 0x41c5u:
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
    case 0x3d0au:
    case 0x3d0du:
    case 0x3d25u:
    case 0x3f04u:
    case 0x3f0cu:
    case 0x3f19u:
    case 0x3f1cu:
    case 0x3f23u:
    case 0x3f96u:
    case 0x3fa2u:
    case 0x3faeu:
    case 0x3fb7u:
    case 0x3fbcu:
    case 0x40d3u:
    case 0x40e9u:
    case 0x4297u:
    case 0x429au:
        return SpcResumeAt(spc, 0x8fu);
    case 0x3d12u:
    case 0x4055u:
    case 0x40a5u:
        return SpcResumeAt(spc, 0x90u);
    case 0x40e3u:
        return SpcResumeAt(spc, 0x93u);
    case 0x416eu:
    case 0x4174u:
        return SpcResumeAt(spc, 0x94u);
    case 0x36a6u:
        return SpcResumeAt(spc, 0x95u);
    case 0x050eu:
        return SpcResumeAt(spc, 0x98u);
    case 0x3607u:
    case 0x3614u:
    case 0x3de1u:
        return SpcResumeAt(spc, 0x9bu);
    case 0x03b2u:
        return SpcResumeAt(spc, 0x9cu);
    case 0x41c9u:
        return SpcResumeAt(spc, 0x9eu);
    case 0x40abu:
    case 0x40afu:
        return SpcResumeAt(spc, 0x9fu);
    case 0x407bu:
    case 0x417du:
    case 0x4183u:
        return SpcResumeAt(spc, 0xa4u);
    case 0x40a9u:
        return SpcResumeAt(spc, 0xa8u);
    case 0x05e5u:
    case 0x40b5u:
    case 0x41adu:
        return SpcResumeAt(spc, 0xabu);
    case 0x41dau:
        return SpcResumeAt(spc, 0xadu);
    case 0x0525u:
    case 0x3b72u:
    case 0x3d10u:
    case 0x4206u:
        return SpcResumeAt(spc, 0xaeu);
    case 0x03b0u:
    case 0x3b7bu:
    case 0x3b88u:
    case 0x4002u:
    case 0x402eu:
        return SpcResumeAt(spc, 0xb0u);
    case 0x0502u:
    case 0x054du:
        return SpcResumeAt(spc, 0xbau);
    case 0x3d3au:
    case 0x3d4du:
    case 0x400bu:
    case 0x4011u:
    case 0x405au:
        return SpcResumeAt(spc, 0xbcu);
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
    case 0x0499u:
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
    case 0x3694u:
    case 0x3699u:
    case 0x3b7fu:
    case 0x3b8cu:
    case 0x3b94u:
    case 0x3b9du:
    case 0x3d1cu:
    case 0x3d23u:
    case 0x3d3bu:
    case 0x3d4bu:
    case 0x3d4eu:
    case 0x3d8fu:
    case 0x3d96u:
    case 0x3dbfu:
    case 0x3e7fu:
    case 0x3e8bu:
    case 0x3ec1u:
    case 0x3edcu:
    case 0x3ee0u:
    case 0x3f0fu:
    case 0x3f2bu:
    case 0x3f2du:
    case 0x3f99u:
    case 0x3fa5u:
    case 0x3fb1u:
    case 0x3fbau:
    case 0x3fbfu:
    case 0x3fd0u:
    case 0x403eu:
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
    case 0x41ceu:
    case 0x429fu:
        return SpcResumeAt(spc, 0xc4u);
    case 0x032au:
    case 0x032fu:
    case 0x0334u:
    case 0x0351u:
    case 0x035cu:
    case 0x3a11u:
    case 0x3a14u:
    case 0x3d43u:
    case 0x3ec6u:
    case 0x3eceu:
    case 0x3ed6u:
    case 0x3eebu:
    case 0x42a1u:
    case 0x42a6u:
    case 0x42b3u:
    case 0x42b8u:
    case 0x42bdu:
        return SpcResumeAt(spc, 0xc5u);
    case 0x3637u:
    case 0x3b79u:
    case 0x3b86u:
    case 0x3d32u:
    case 0x3e1cu:
    case 0x3eaau:
    case 0x4000u:
    case 0x402cu:
    case 0x40a3u:
    case 0x40bbu:
    case 0x4188u:
    case 0x41bau:
        return SpcResumeAt(spc, 0xc8u);
    case 0x3d83u:
    case 0x3fceu:
    case 0x4037u:
    case 0x4078u:
    case 0x4150u:
    case 0x4155u:
    case 0x41a7u:
        return SpcResumeAt(spc, 0xcbu);
    case 0x35fbu:
    case 0x3c93u:
    case 0x3d28u:
    case 0x3dd0u:
    case 0x3e8du:
    case 0x4018u:
    case 0x40efu:
    case 0x418fu:
    case 0x41c7u:
        return SpcResumeAt(spc, 0xcdu);
    case 0x3da9u:
    case 0x4054u:
    case 0x41b8u:
    case 0x420du:
        return SpcResumeAt(spc, 0xceu);
    case 0x3a0du:
    case 0x400cu:
    case 0x4012u:
    case 0x4036u:
    case 0x404du:
    case 0x4053u:
    case 0x4077u:
    case 0x40c8u:
    case 0x40deu:
    case 0x414fu:
    case 0x4154u:
    case 0x415bu:
    case 0x41ebu:
    case 0x41f3u:
    case 0x41fau:
    case 0x4204u:
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
    case 0x360fu:
    case 0x3616u:
    case 0x3639u:
    case 0x36f9u:
    case 0x3c98u:
    case 0x3d34u:
    case 0x3d3fu:
    case 0x3dc8u:
    case 0x3de3u:
    case 0x3decu:
    case 0x3e1eu:
    case 0x3e9cu:
    case 0x3eacu:
    case 0x3eb0u:
    case 0x3ee4u:
    case 0x3fdau:
    case 0x401eu:
    case 0x40f5u:
    case 0x4108u:
    case 0x41bcu:
    case 0x41dcu:
    case 0x4295u:
        return SpcResumeAt(spc, 0xd0u);
    case 0x3719u:
    case 0x371eu:
    case 0x3d5au:
    case 0x3d60u:
    case 0x3da3u:
    case 0x3da7u:
    case 0x3efeu:
    case 0x3f02u:
    case 0x3f5cu:
    case 0x410eu:
    case 0x4112u:
    case 0x4170u:
    case 0x4176u:
    case 0x417fu:
    case 0x4185u:
    case 0x41e0u:
    case 0x41e4u:
    case 0x4209u:
        return SpcResumeAt(spc, 0xd4u);
    case 0x04d6u:
    case 0x04dcu:
    case 0x04e2u:
    case 0x04e8u:
    case 0x04edu:
    case 0x04f9u:
    case 0x0506u:
    case 0x050au:
    case 0x36a9u:
    case 0x36aeu:
    case 0x36b4u:
    case 0x36bau:
    case 0x3b1du:
    case 0x3b24u:
    case 0x3b5du:
    case 0x3b63u:
    case 0x3b69u:
    case 0x3b6fu:
    case 0x3d6cu:
    case 0x3d77u:
    case 0x3d7fu:
    case 0x3ea0u:
    case 0x3f32u:
    case 0x3f35u:
    case 0x3f38u:
    case 0x3f3bu:
    case 0x3f3eu:
    case 0x3f41u:
    case 0x3f44u:
    case 0x3f47u:
    case 0x3f4au:
    case 0x3f4du:
    case 0x3f50u:
    case 0x3f53u:
    case 0x3f56u:
    case 0x3f59u:
    case 0x3f60u:
    case 0x3f65u:
    case 0x3f6au:
    case 0x3f6fu:
    case 0x3f74u:
    case 0x3f79u:
    case 0x3f7cu:
    case 0x3f7fu:
    case 0x3f82u:
    case 0x3f87u:
    case 0x3f8cu:
    case 0x4014u:
        return SpcResumeAt(spc, 0xd5u);
    case 0x05d7u:
    case 0x05e0u:
        return SpcResumeAt(spc, 0xd7u);
    case 0x41c0u:
        return SpcResumeAt(spc, 0xd8u);
    case 0x054fu:
    case 0x41ecu:
    case 0x41fdu:
        return SpcResumeAt(spc, 0xdau);
    case 0x420bu:
        return SpcResumeAt(spc, 0xdbu);
    case 0x0524u:
        return SpcResumeAt(spc, 0xdcu);
    case 0x0509u:
    case 0x0517u:
    case 0x051fu:
    case 0x05c4u:
    case 0x05d3u:
    case 0x3a0fu:
    case 0x3fd3u:
    case 0x4013u:
    case 0x4057u:
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
    case 0x0495u:
    case 0x04ebu:
    case 0x04f7u:
    case 0x051cu:
    case 0x05d5u:
    case 0x05deu:
    case 0x05f4u:
    case 0x05f8u:
    case 0x3b20u:
    case 0x3c54u:
    case 0x3d04u:
    case 0x3d2au:
    case 0x3d38u:
    case 0x3d41u:
    case 0x3d86u:
    case 0x3dc4u:
    case 0x3f08u:
    case 0x3f15u:
    case 0x3f90u:
    case 0x3f9cu:
    case 0x3fa8u:
    case 0x4083u:
    case 0x416bu:
    case 0x4172u:
    case 0x41e2u:
    case 0x4290u:
        return SpcResumeAt(spc, 0xe4u);
    case 0x3d69u:
    case 0x3ec9u:
    case 0x3ed1u:
    case 0x3ee6u:
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
    case 0x36c0u:
    case 0x3d58u:
    case 0x3d5cu:
    case 0x3da1u:
    case 0x3da5u:
    case 0x3dbbu:
    case 0x3e9eu:
    case 0x3f00u:
    case 0x3f29u:
    case 0x3f30u:
    case 0x3f5eu:
    case 0x3f63u:
    case 0x3f68u:
    case 0x3f6du:
    case 0x3f72u:
    case 0x3f77u:
    case 0x3f85u:
    case 0x3f8au:
    case 0x429du:
    case 0x42a4u:
    case 0x42acu:
    case 0x42b1u:
    case 0x42b6u:
    case 0x42bbu:
        return SpcResumeAt(spc, 0xe8u);
    case 0x0391u:
        return SpcResumeAt(spc, 0xe9u);
    case 0x3d70u:
    case 0x404bu:
    case 0x4074u:
    case 0x4157u:
        return SpcResumeAt(spc, 0xebu);
    case 0x3a0au:
    case 0x3ffdu:
    case 0x4033u:
    case 0x40c5u:
    case 0x40dbu:
        return SpcResumeAt(spc, 0xecu);
    case 0x3d54u:
    case 0x3d85u:
    case 0x41d3u:
        return SpcResumeAt(spc, 0xeeu);
    case 0x038du:
    case 0x0394u:
    case 0x03a1u:
    case 0x03a8u:
    case 0x3601u:
    case 0x3605u:
    case 0x36feu:
    case 0x3706u:
    case 0x3c56u:
    case 0x3d2fu:
    case 0x3d64u:
    case 0x3dadu:
    case 0x3dd4u:
    case 0x3dd8u:
    case 0x3e23u:
    case 0x3e89u:
    case 0x3e93u:
    case 0x3eb5u:
    case 0x3ef2u:
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
    case 0x41d4u:
    case 0x4292u:
        return SpcResumeAt(spc, 0xf0u);
    case 0x35fdu:
    case 0x3603u:
    case 0x360bu:
    case 0x36f5u:
    case 0x3700u:
    case 0x3704u:
    case 0x3dd2u:
    case 0x3dd6u:
    case 0x3de8u:
    case 0x3e8fu:
    case 0x3eeeu:
    case 0x3efau:
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
    case 0x417au:
    case 0x4181u:
    case 0x4194u:
    case 0x41a1u:
    case 0x41a9u:
    case 0x41afu:
        return SpcResumeAt(spc, 0xf4u);
    case 0x3691u:
    case 0x3696u:
    case 0x36fbu:
    case 0x3716u:
    case 0x371bu:
    case 0x3b5au:
    case 0x3b60u:
    case 0x3b66u:
    case 0x3b6cu:
    case 0x3b74u:
    case 0x3b81u:
    case 0x3b8fu:
    case 0x3b96u:
    case 0x3c95u:
    case 0x3d91u:
    case 0x3e86u:
    case 0x3e95u:
    case 0x3ebeu:
    case 0x3ed9u:
    case 0x3fcbu:
    case 0x4007u:
    case 0x400du:
    case 0x4030u:
    case 0x403bu:
    case 0x4040u:
    case 0x4045u:
    case 0x404fu:
    case 0x406du:
    case 0x4103u:
    case 0x4145u:
    case 0x414bu:
    case 0x4166u:
    case 0x41c2u:
    case 0x41e6u:
    case 0x41eeu:
    case 0x41f5u:
    case 0x41ffu:
        return SpcResumeAt(spc, 0xf5u);
    case 0x41cbu:
    case 0x41d0u:
        return SpcResumeAt(spc, 0xf6u);
    case 0x369du:
    case 0x36acu:
    case 0x36b2u:
    case 0x36b8u:
    case 0x3d17u:
    case 0x3d1fu:
    case 0x3d48u:
    case 0x3d55u:
    case 0x3d72u:
    case 0x3d7bu:
    case 0x3e7du:
    case 0x3e82u:
        return SpcResumeAt(spc, 0xf7u);
    case 0x04d1u:
    case 0x41deu:
        return SpcResumeAt(spc, 0xf8u);
    case 0x3dcdu:
    case 0x3e78u:
    case 0x3f26u:
    case 0x40b2u:
    case 0x40b7u:
        return SpcResumeAt(spc, 0xfau);
    case 0x41e9u:
    case 0x41f1u:
    case 0x41f8u:
    case 0x4202u:
        return SpcResumeAt(spc, 0xfbu);
    case 0x05d9u:
    case 0x05e2u:
    case 0x36a4u:
    case 0x36b1u:
    case 0x36b7u:
    case 0x3d1eu:
    case 0x3d4au:
    case 0x3d7au:
    case 0x3d82u:
    case 0x3daau:
    case 0x3e81u:
    case 0x3e84u:
    case 0x41d9u:
        return SpcResumeAt(spc, 0xfcu);
    case 0x3d16u:
    case 0x3fd7u:
    case 0x414au:
    case 0x41b7u:
    case 0x4205u:
        return SpcResumeAt(spc, 0xfdu);
    case 0x0367u:
    case 0x3f13u:
    case 0x3f21u:
        return SpcResumeAt(spc, 0xfeu);
    default:
        return 0u;
    }
}

#undef LUFIA2_SPC_INLINE
