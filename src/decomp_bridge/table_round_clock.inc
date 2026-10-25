typedef struct TableRoundOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index;
    int8_t advance_x, advance_y;
} TableRoundOpcode;

enum {
    ROUND_PC_9F1A = 0u,
    ROUND_PC_9F1B = 1u,
    ROUND_PC_9F1D = 2u,
    ROUND_PC_9F1F = 3u,
    ROUND_PC_9F21 = 4u,
    ROUND_PC_9F23 = 5u,
    ROUND_PC_9F25 = 6u,
    ROUND_PC_9F27 = 7u,
    ROUND_PC_9F29 = 8u,
    ROUND_PC_9F2B = 9u,
    ROUND_PC_9F2D = 10u,
    ROUND_PC_9F2F = 11u,
    ROUND_PC_9F32 = 12u,
    ROUND_PC_9F34 = 13u,
    ROUND_PC_9F36 = 14u,
    ROUND_PC_9F38 = 15u,
    ROUND_PC_9F3A = 16u,
    ROUND_PC_9F3C = 17u,
    ROUND_PC_9F3E = 18u,
    ROUND_PC_9F40 = 19u,
    ROUND_PC_9F42 = 20u,
    ROUND_PC_9F45 = 21u,
    ROUND_PC_9F47 = 22u,
    ROUND_PC_9F49 = 23u,
    ROUND_PC_9F4B = 24u,
    ROUND_PC_9F4D = 25u,
    ROUND_PC_9F4F = 26u,
    ROUND_PC_9F51 = 27u,
    ROUND_PC_9F53 = 28u,
    ROUND_PC_9F55 = 29u,
    ROUND_PC_9F58 = 30u,
    ROUND_PC_9F5A = 31u,
    ROUND_PC_9F5C = 32u,
    ROUND_PC_9F5E = 33u,
    ROUND_PC_9F60 = 34u,
    ROUND_PC_9F62 = 35u,
    ROUND_PC_9F64 = 36u,
    ROUND_PC_9F66 = 37u,
    ROUND_PC_9F68 = 38u,
    ROUND_PC_9F69 = 39u,
    ROUND_PC_9F6B = 40u,
    ROUND_PC_9F6D = 41u,
    ROUND_PC_9F6F = 42u,
    ROUND_PC_9F71 = 43u,
    ROUND_PC_9F73 = 44u,
    ROUND_PC_9F75 = 45u,
    ROUND_PC_9F77 = 46u,
    ROUND_PC_9F79 = 47u,
    ROUND_PC_9F7C = 48u,
    ROUND_PC_9F7E = 49u,
    ROUND_PC_9F80 = 50u,
    ROUND_PC_9F82 = 51u,
    ROUND_PC_9F84 = 52u,
    ROUND_PC_9F86 = 53u,
    ROUND_PC_9F88 = 54u,
    ROUND_PC_9F8A = 55u,
    ROUND_PC_9F8C = 56u,
    ROUND_PC_9F8F = 57u,
    ROUND_PC_9F91 = 58u,
    ROUND_PC_9F93 = 59u,
    ROUND_PC_9F95 = 60u,
    ROUND_PC_9F97 = 61u,
    ROUND_PC_9F99 = 62u,
    ROUND_PC_9F9B = 63u,
    ROUND_PC_9F9D = 64u,
    ROUND_PC_9F9F = 65u,
    ROUND_PC_9FA2 = 66u,
    ROUND_PC_9FA4 = 67u,
    ROUND_PC_9FA6 = 68u,
    ROUND_PC_9FA8 = 69u,
    ROUND_PC_9FAA = 70u,
    ROUND_PC_9FAC = 71u,
    ROUND_PC_9FAE = 72u,
    ROUND_PC_9FB0 = 73u,
    ROUND_PC_9FB2 = 74u,
    ROUND_PC_9FB5 = 75u,
    ROUND_PC_9FB7 = 76u,
    ROUND_PC_9FB9 = 77u,
    ROUND_PC_9FBB = 78u,
    ROUND_PC_9FBD = 79u,
    ROUND_PC_9FBF = 80u,
    ROUND_PC_9FC1 = 81u,
    ROUND_PC_9FC3 = 82u,
    ROUND_PC_9FC5 = 83u,
    ROUND_PC_9FC8 = 84u,
    ROUND_PC_9FCA = 85u,
    ROUND_PC_9FCC = 86u,
    ROUND_PC_9FCE = 87u,
    ROUND_PC_9FD0 = 88u,
    ROUND_PC_9FD2 = 89u,
    ROUND_PC_9FD4 = 90u,
    ROUND_PC_9FD6 = 91u,
    ROUND_PC_9FD8 = 92u,
    ROUND_PC_9FDB = 93u,
    ROUND_PC_9FDD = 94u,
    ROUND_PC_9FDF = 95u,
    ROUND_PC_9FE1 = 96u,
    ROUND_PC_9FE3 = 97u,
    ROUND_PC_9FE5 = 98u,
    ROUND_PC_9FE7 = 99u,
    ROUND_PC_9FE9 = 100u,
    ROUND_PC_9FEB = 101u,
    ROUND_PC_9FEE = 102u,
    ROUND_PC_9FF0 = 103u,
    ROUND_PC_9FF2 = 104u,
    ROUND_PC_9FF4 = 105u,
    ROUND_PC_9FF6 = 106u,
    ROUND_PC_9FF8 = 107u,
    ROUND_PC_9FFA = 108u,
    ROUND_PC_9FFC = 109u,
    ROUND_PC_9FFE = 110u,
    ROUND_PC_9FFF = 111u,
    ROUND_PC_DC60 = 112u,
    ROUND_PC_DC63 = 113u,
    ROUND_PC_DC66 = 114u,
    ROUND_PC_DC68 = 115u,
    ROUND_PC_DC6B = 116u,
    ROUND_PC_DC6E = 117u,
    ROUND_PC_DC71 = 118u,
    ROUND_PC_DC74 = 119u,
    ROUND_PC_DC77 = 120u,
    ROUND_PC_DC7A = 121u,
    ROUND_PC_DC7D = 122u,
    ROUND_PC_DC80 = 123u,
    ROUND_PC_DC83 = 124u,
    ROUND_PC_DC86 = 125u,
    ROUND_PC_DC87 = 126u,
    ROUND_PC_DC8A = 127u,
    ROUND_PC_DC8D = 128u,
    ROUND_PC_DC8F = 129u,
    ROUND_PC_DC92 = 130u,
    ROUND_PC_DC95 = 131u,
    ROUND_PC_DC98 = 132u,
    ROUND_PC_DC9A = 133u,
    ROUND_PC_DC9D = 134u,
    ROUND_PC_DC9F = 135u,
    ROUND_PC_DCA2 = 136u,
    ROUND_PC_DCA5 = 137u,
    ROUND_PC_DCA8 = 138u,
    ROUND_PC_DCAB = 139u,
    ROUND_PC_DCAD = 140u,
    ROUND_PC_DCB0 = 141u,
    ROUND_PC_DCB3 = 142u,
    ROUND_PC_DCB6 = 143u,
    ROUND_PC_DCB9 = 144u,
    ROUND_PC_DCBC = 145u,
    ROUND_PC_DCC0 = 146u,
    ROUND_PC_DCC1 = 147u,
    ROUND_PC_DCC4 = 148u,
    ROUND_PC_DCC7 = 149u,
    ROUND_PC_DCCA = 150u,
    ROUND_PC_DCCD = 151u,
    ROUND_PC_DCD0 = 152u,
    ROUND_PC_DCD2 = 153u,
    ROUND_PC_DCD4 = 154u,
    ROUND_PC_DCD7 = 155u,
    ROUND_PC_DCD9 = 156u,
    ROUND_PC_DCDC = 157u,
    ROUND_PC_DCDE = 158u,
    ROUND_PC_DCE1 = 159u,
    ROUND_PC_DCE2 = 160u,
    ROUND_PC_DCE3 = 161u,
    ROUND_PC_DCE4 = 162u,
    ROUND_PC_DCE6 = 163u,
    ROUND_PC_DCE7 = 164u,
    ROUND_PC_DCEA = 165u,
    ROUND_PC_DCEC = 166u,
    ROUND_PC_DCEF = 167u,
    ROUND_PC_DCF1 = 168u,
    ROUND_PC_DCF4 = 169u,
    ROUND_PC_DCF8 = 170u,
    ROUND_PC_DCFB = 171u,
    ROUND_PC_DCFD = 172u,
    ROUND_PC_DD00 = 173u,
    ROUND_PC_DD03 = 174u,
    ROUND_PC_DD05 = 175u,
    ROUND_PC_DD07 = 176u,
    ROUND_PC_DD0A = 177u,
    ROUND_PC_DD0C = 178u,
    ROUND_PC_DD0E = 179u,
    ROUND_PC_DD10 = 180u,
    ROUND_PC_DD13 = 181u,
    ROUND_PC_DD16 = 182u,
    ROUND_PC_DD18 = 183u,
    ROUND_PC_DD1A = 184u,
    ROUND_PC_DD1B = 185u,
    ROUND_PC_DD1C = 186u,
    ROUND_PC_DD1D = 187u,
    ROUND_PC_DD1F = 188u,
    ROUND_PC_DD22 = 189u,
    ROUND_PC_DD24 = 190u,
    ROUND_PC_DD26 = 191u,
    ROUND_PC_DD29 = 192u,
    ROUND_PC_DD2A = 193u,
    ROUND_PC_DD2D = 194u,
    ROUND_PC_DD2F = 195u,
    ROUND_PC_DD32 = 196u,
    ROUND_PC_DD35 = 197u,
    ROUND_PC_DD38 = 198u,
    ROUND_PC_DD3B = 199u,
    ROUND_PC_DD3E = 200u,
    ROUND_PC_DD41 = 201u,
    ROUND_PC_DD44 = 202u,
    ROUND_PC_DD47 = 203u,
    ROUND_PC_DD4A = 204u,
    ROUND_PC_DD4D = 205u,
    ROUND_PC_DD50 = 206u,
    ROUND_PC_DD52 = 207u,
    ROUND_PC_DD55 = 208u,
    ROUND_PC_DD58 = 209u,
    ROUND_PC_DD5B = 210u,
    ROUND_PC_DD5E = 211u,
    ROUND_PC_DD61 = 212u,
    ROUND_PC_DD63 = 213u,
    ROUND_PC_DD66 = 214u,
    ROUND_PC_DD69 = 215u,
    ROUND_PC_DD6C = 216u,
    ROUND_PC_DD6E = 217u,
    ROUND_PC_DD71 = 218u,
    ROUND_PC_DD73 = 219u,
    ROUND_PC_DD76 = 220u,
    ROUND_PC_DD79 = 221u,
    ROUND_PC_DD7C = 222u,
    ROUND_PC_DD7F = 223u,
    ROUND_PC_DD82 = 224u,
    ROUND_PC_DD85 = 225u,
    ROUND_PC_DD88 = 226u,
    ROUND_PC_DD8B = 227u,
    ROUND_PC_DD8E = 228u,
    ROUND_PC_DD90 = 229u,
    ROUND_PC_DD93 = 230u,
    ROUND_PC_DD96 = 231u,
    ROUND_PC_DD9A = 232u,
    ROUND_PC_DD9C = 233u,
    ROUND_PC_DD9E = 234u,
    ROUND_PC_DDA1 = 235u,
    ROUND_PC_DDA3 = 236u,
    ROUND_PC_DDA6 = 237u,
    ROUND_PC_DDA8 = 238u,
    ROUND_PC_DDAB = 239u,
    ROUND_PC_DDAE = 240u,
    ROUND_PC_DDB1 = 241u,
    ROUND_PC_DDB4 = 242u,
    ROUND_PC_DDB7 = 243u,
    ROUND_PC_DDB8 = 244u,
    ROUND_PC_DDB9 = 245u,
    ROUND_PC_DDBA = 246u,
    ROUND_PC_DDBC = 247u,
    ROUND_PC_DDBF = 248u,
    ROUND_PC_DDC1 = 249u,
    ROUND_PC_DDC4 = 250u,
    ROUND_PC_DDC6 = 251u,
    ROUND_PC_DDC9 = 252u,
    ROUND_PC_DDCC = 253u,
    ROUND_PC_DDCD = 254u,
    ROUND_PC_DDCE = 255u,
    ROUND_PC_DDCF = 256u,
    ROUND_PC_DDD0 = 257u,
    ROUND_PC_DDD3 = 258u,
    ROUND_PC_DDD6 = 259u,
    ROUND_PC_DDD8 = 260u,
    ROUND_PC_DDDA = 261u,
    ROUND_PC_DDDD = 262u,
    ROUND_PC_DDDF = 263u,
    ROUND_PC_DDE0 = 264u,
    ROUND_PC_DDE3 = 265u,
    ROUND_PC_DDE5 = 266u,
    ROUND_PC_DDE6 = 267u,
    ROUND_PC_DDE7 = 268u,
    ROUND_PC_DDE8 = 269u,
    ROUND_PC_DDEA = 270u,
    ROUND_PC_DDEC = 271u,
    ROUND_PC_DDEE = 272u,
    ROUND_PC_DDF1 = 273u,
    ROUND_PC_DDF3 = 274u,
    ROUND_PC_DDF5 = 275u,
    ROUND_PC_DDF8 = 276u,
    ROUND_PC_DDFB = 277u,
    ROUND_PC_DDFC = 278u,
    ROUND_PC_DDFD = 279u,
    ROUND_PC_DDFE = 280u,
    ROUND_PC_DE00 = 281u,
    ROUND_PC_DE03 = 282u,
    ROUND_PC_DE05 = 283u,
    ROUND_PC_DE08 = 284u,
    ROUND_PC_DE0A = 285u,
    ROUND_PC_DE0C = 286u,
    ROUND_PC_DE0E = 287u,
    ROUND_PC_DE11 = 288u,
    ROUND_PC_DE13 = 289u,
    ROUND_PC_DE16 = 290u,
    ROUND_PC_DE19 = 291u,
    ROUND_PC_DE1A = 292u,
    ROUND_PC_DE1B = 293u,
    ROUND_PC_DE1C = 294u,
    ROUND_PC_DE1D = 295u,
    ROUND_PC_DE20 = 296u,
    ROUND_PC_DE23 = 297u,
    ROUND_PC_DE26 = 298u,
    ROUND_PC_DE28 = 299u,
    ROUND_PC_DE2B = 300u,
    ROUND_PC_DE2E = 301u,
    ROUND_PC_DE30 = 302u,
    ROUND_PC_DE32 = 303u,
    ROUND_PC_DE34 = 304u,
    ROUND_PC_DE37 = 305u,
    ROUND_PC_DE39 = 306u,
    ROUND_PC_DE3A = 307u,
    ROUND_PC_DE3B = 308u,
    ROUND_PC_DE3C = 309u,
    ROUND_PC_DE3E = 310u,
    ROUND_PC_DE40 = 311u,
    ROUND_PC_DE43 = 312u,
    ROUND_PC_DE46 = 313u,
    ROUND_PC_DE47 = 314u,
    ROUND_PC_DE48 = 315u,
    ROUND_PC_DE49 = 316u,
    ROUND_PC_DE4A = 317u,
    ROUND_PC_DE4D = 318u,
    ROUND_PC_DE50 = 319u,
    ROUND_PC_DE53 = 320u,
    ROUND_PC_DE55 = 321u,
    ROUND_PC_DE58 = 322u,
    ROUND_PC_DE5B = 323u,
    ROUND_PC_DE5D = 324u,
    ROUND_PC_DE5F = 325u,
    ROUND_PC_DE61 = 326u,
    ROUND_PC_DE62 = 327u,
    ROUND_PC_DE63 = 328u,
    ROUND_PC_DE64 = 329u,
    ROUND_PC_DE66 = 330u,
    ROUND_PC_DE68 = 331u,
    ROUND_PC_DE6B = 332u,
    ROUND_PC_DE6E = 333u,
    ROUND_PC_DE71 = 334u,
    ROUND_PC_DE74 = 335u,
    ROUND_PC_DE77 = 336u,
    ROUND_PC_DE79 = 337u,
    ROUND_PC_DE7C = 338u,
    ROUND_PC_DE7E = 339u,
    ROUND_PC_DE81 = 340u,
    ROUND_PC_DE83 = 341u,
    ROUND_PC_DE86 = 342u,
    ROUND_PC_DE88 = 343u,
    ROUND_PC_DE8B = 344u,
    ROUND_PC_DE8D = 345u,
    ROUND_PC_DE90 = 346u,
    ROUND_PC_DE92 = 347u,
    ROUND_PC_DE95 = 348u,
    ROUND_PC_DE97 = 349u,
    ROUND_PC_DE9A = 350u,
    ROUND_PC_DE9C = 351u,
    ROUND_PC_DE9F = 352u,
    ROUND_PC_DEA1 = 353u,
    ROUND_PC_DEA4 = 354u,
    ROUND_PC_DEA6 = 355u,
    ROUND_PC_DEA9 = 356u,
    ROUND_PC_DEAC = 357u,
    ROUND_PC_DEAE = 358u,
    ROUND_PC_DEAF = 359u,
    ROUND_PC_DEB0 = 360u,
    ROUND_PC_DEB2 = 361u,
    ROUND_PC_DEB3 = 362u,
    ROUND_PC_DEB4 = 363u,
    ROUND_PC_DEB5 = 364u,
    ROUND_PC_DEB8 = 365u,
};

static const TableRoundOpcode table_round_opcodes[] = {
    {{0x9f1au, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f1bu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f1du, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f1fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f21u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f23u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f25u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f27u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f29u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f2bu, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f2du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f2fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f32u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f34u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f36u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f38u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f3au, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f3cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f3eu, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f40u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f42u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f45u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f47u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f49u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f4bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f4du, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f4fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f51u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f53u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f55u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f58u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f5au, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f5cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f5eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f60u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f62u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f64u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f66u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f68u, 1u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f69u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f6bu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f6du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f6fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f71u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f73u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f75u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f77u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f79u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f7cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f7eu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f80u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f82u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f84u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f86u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f88u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f8au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f8cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f8fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f91u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f93u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f95u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f97u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9f99u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f9bu, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9f9du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9f9fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fa2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fa4u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fa6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fa8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9faau, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9facu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9faeu, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9fb0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fb2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fb5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fb7u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fb9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fbbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fbdu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fbfu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fc1u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9fc3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fc5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fc8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fcau, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fccu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fceu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fd0u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fd2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fd4u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9fd6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fd8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fdbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fddu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fdfu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fe1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fe3u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fe5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9fe7u, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9fe9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9febu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9feeu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9ff0u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9ff2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9ff4u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9ff6u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9ff8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9ffau, 2u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0x9ffcu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0x9ffeu, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0x9fffu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc60u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc63u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc66u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc68u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc6bu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc6eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc71u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc74u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc77u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc7au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc7du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc80u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc83u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc86u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc87u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc8au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc8du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc8fu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc92u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdc95u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdc98u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc9au, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdc9du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc9fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdca2u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdca5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdca8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcabu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcadu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcb0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcb3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcb6u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdcb9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcbcu, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdcc0u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcc1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcc4u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcc7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdccau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdccdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcd0u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcd2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcd4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcd7u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcd9u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcdcu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcdeu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdce1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdce2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdce3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdce4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdce6u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdce7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdceau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdcecu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcefu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcf1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcf4u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdcf8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcfbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdcfdu, 3u, 5u, 2u}, 0x7006u, 2u, 0, 0},
    {{0xdd00u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd03u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd05u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd07u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd0au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd0cu, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdd0eu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd10u, 3u, 5u, 2u}, 0x7006u, 2u, 0, 0},
    {{0xdd13u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd16u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd18u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdd1au, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xdd1bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xdd1cu, 1u, 2u, 0u}, 0x0000u, 0u, -1, 0},
    {{0xdd1du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd1fu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd22u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdd24u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd26u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd29u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd2au, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdd2du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd2fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd32u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd35u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd38u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd3bu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd3eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd41u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd44u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd47u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd4au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd4du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd50u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd52u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd55u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd58u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd5bu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd5eu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdd61u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd63u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd66u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd69u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd6cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd6eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd71u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd73u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xdd76u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd79u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd7cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd7fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd82u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd85u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd88u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd8bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd8eu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd90u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd93u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdd96u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdd9au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd9cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdd9eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdda1u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdda3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdda6u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdda8u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddabu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddaeu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddb1u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddb4u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddb7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddb8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddb9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddbau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddbcu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddbfu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddc1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddc4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddc6u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddc9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddccu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddcdu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddceu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddcfu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddd0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddd3u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xddd6u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddd8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdddau, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xddddu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdddfu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdde0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdde3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdde5u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xdde6u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xdde7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xdde8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddeau, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xddecu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddeeu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xddf1u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xddf3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddf5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddf8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xddfbu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddfcu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddfdu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xddfeu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde00u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde03u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde05u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde08u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde0au, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xde0cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde0eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde11u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde13u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde16u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde19u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde1au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde1bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde1cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde1du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde20u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xde23u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde26u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde28u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xde2bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde2eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde30u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xde32u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde34u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xde37u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde39u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xde3au, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xde3bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xde3cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde3eu, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xde40u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde43u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde46u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde47u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde48u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde49u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde4au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde4du, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xde50u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde53u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde55u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xde58u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde5bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde5du, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xde5fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde61u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xde62u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xde63u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xde64u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde66u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde68u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde6bu, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde6eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde71u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde74u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde77u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde79u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde7cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde7eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde81u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde83u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde86u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde88u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde8bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde8du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde90u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde92u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde95u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde97u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xde9au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde9cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xde9fu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdea1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdea4u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdea6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdea9u, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xdeacu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdeaeu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xdeafu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xdeb0u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdeb2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdeb3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdeb4u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdeb5u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdeb8u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
};

enum { TABLE_ROUND_CAPACITY = 512u };
typedef struct TableRoundClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_ROUND_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint32_t next;
    bool returning;
} TableRoundClock;
static const uint32_t table_round_entries[]={0x849f1au,0x84dc60u,0x84dc87u,0x84de6eu};
static const uint32_t table_round_ends[]={0x849fffu,0x84dc86u,0x84dd29u,0x84deb8u};
static uint16_t TableRoundDirectWord(const CpuState *cpu, unsigned at) {
    return cpu->ram[at] | (uint16_t)cpu->ram[at + 1u] << 8;
}
static uint16_t TableRoundBankWord(const CpuState *cpu, uint16_t at) {
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    return cpu->ram[bank + at] | (uint16_t)cpu->ram[bank + (uint16_t)(at + 1u)] << 8;
}
static bool TableRoundAppendOpcode(TableRoundClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == TABLE_ROUND_CAPACITY) return false;
    clock->timing[clock->span.count++] = op;
    return true;
}
static bool TableRoundRange(TableRoundClock *clock, unsigned first, unsigned last,
                         uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableRoundOpcode *record = table_round_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!TableRoundAppendOpcode(clock, op)) return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}
static bool TableRoundBranch(TableRoundClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_round_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return TableRoundAppendOpcode(clock, op);
}
#define LR(first,last,x,y) do { if (!TableRoundRange(clock, ROUND_PC_##first, ROUND_PC_##last, (uint16_t)(x), (uint16_t)(y))) return false; } while (0)
#define LB(pc,taken) do { if (!TableRoundBranch(clock, ROUND_PC_##pc, taken)) return false; } while (0)
#define LC(site) do { LR(site,site,0u,0u); clock->next = 0x840000u | (table_round_opcodes[ROUND_PC_##site].timing.pc + table_round_opcodes[ROUND_PC_##site].timing.fetched); clock->returning = false; return true; } while (0)


typedef struct RoundKeyTiming {
    unsigned first, armed_branch, held_first, held_branch, consumed_first, consumed_last;
    uint8_t armed, held, mask;
} RoundKeyTiming;

static bool PlanRoundKeyGroup(TableRoundClock *clock,
    const RoundKeyTiming *keys, unsigned count) {
    const CpuState *cpu = clock->call.cpu;
    for (unsigned n = 0u; n < count; ++n) {
        const RoundKeyTiming *key = keys + n;
        const bool armed = (cpu->ram[key->armed] & key->mask) != 0u;
        if (!TableRoundRange(clock, key->first, key->armed_branch - 1u, 0u, 0u) ||
            !TableRoundBranch(clock, key->armed_branch, !armed)) return false;
        if (!armed) continue;
        const bool held = (cpu->ram[key->held] & key->mask) != 0u;
        if (!TableRoundRange(clock, key->held_first, key->held_branch - 1u, 0u, 0u) ||
            !TableRoundBranch(clock, key->held_branch, !held)) return false;
        if (!held) continue;
        return TableRoundRange(clock, key->consumed_first, key->consumed_last, 0u, 0u);
    }
    return true;
}

#define RK(first,armed,held,branch,used,last,latch,buttons,mask) \
    {ROUND_PC_##first,ROUND_PC_##armed,ROUND_PC_##held,ROUND_PC_##branch, \
     ROUND_PC_##used,ROUND_PC_##last,latch,buttons,mask}

static bool PlanRoundCommand(TableRoundClock *clock, uint16_t pc) {
    static const RoundKeyTiming directions[] = {
        RK(9F1F,9F23,9F25,9F29,9F2B,9F2F,0x4bu,0x47u,8u),
        RK(9F32,9F36,9F38,9F3C,9F3E,9F42,0x4bu,0x47u,4u),
        RK(9F45,9F49,9F4B,9F4F,9F51,9F55,0x4bu,0x47u,1u),
        RK(9F58,9F5C,9F5E,9F62,9F64,9F66,0x4bu,0x47u,2u)
    };
    static const RoundKeyTiming buttons[] = {
        RK(9F69,9F6D,9F6F,9F73,9F75,9F79,0x4au,0x46u,0x80u),
        RK(9F7C,9F80,9F82,9F86,9F88,9F8C,0x4bu,0x47u,0x80u),
        RK(9F8F,9F93,9F95,9F99,9F9B,9F9F,0x4bu,0x47u,0x40u),
        RK(9FA2,9FA6,9FA8,9FAC,9FAE,9FB2,0x4au,0x46u,0x40u),
        RK(9FB5,9FB9,9FBB,9FBF,9FC1,9FC5,0x4au,0x46u,0x20u),
        RK(9FC8,9FCC,9FCE,9FD2,9FD4,9FD8,0x4au,0x46u,0x10u),
        RK(9FDB,9FDF,9FE1,9FE5,9FE7,9FEB,0x4bu,0x47u,0x20u),
        RK(9FEE,9FF2,9FF4,9FF8,9FFA,9FFC,0x4bu,0x47u,0x10u)
    };
    if (pc != 0x9f1au) return false;
    LR(9F1A,9F1D,0u,0u);
    if (!PlanRoundKeyGroup(clock, directions, 4u)) return false;
    LR(9F68,9F68,0u,0u);
    if (!PlanRoundKeyGroup(clock, buttons, 8u)) return false;
    LR(9FFE,9FFE,0u,0u);
    return true;
}
#undef RK

static bool PlanRoundNumber(TableRoundClock *clock, uint16_t pc) {
    switch (pc) {
    case 0xdc60u:
        LR(DC60,DC63,0u,0u);
        LB(DC66,(TableRoundBankWord(clock->call.cpu,0x7170u)&8u)==0u);
        if (!(TableRoundBankWord(clock->call.cpu,0x7170u)&8u)) return true;
        LR(DC68,DC71,0u,0u); LC(DC74);
    case 0xdc77u: LR(DC77,DC80,0u,0u); LC(DC83);
    case 0xdc86u: return true;
    default: return false;
    }
}

static bool PlanRoundColumn(TableRoundClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xde6eu) { LC(DE6E); }
    if (pc == 0xde71u) { LC(DE71); }
    if (pc != 0xde74u) return false;
    const unsigned command = cpu->A;
    LR(DE74,DE74,0u,0u); LB(DE77,command!=1u);
    if (command == 1u) {
        LR(DE79,DE79,0u,0u); LB(DE7C,TableRoundBankWord(cpu,0x7128u)==0u);
        if (TableRoundBankWord(cpu,0x7128u)) { LR(DE7E,DE81,0u,0u); LR(DEB5,DEB5,0u,0u); return true; }
    } else {
        LR(DE83,DE83,0u,0u); LB(DE86,command!=4u);
        if (command == 4u) {
            LR(DE88,DE88,0u,0u); LB(DE8B,TableRoundBankWord(cpu,0x7126u)==0u);
            if (TableRoundBankWord(cpu,0x7126u)) { LR(DE8D,DE90,0u,0u); LR(DEB5,DEB5,0u,0u); return true; }
        } else {
            LR(DE92,DE92,0u,0u); LB(DE95,command!=3u);
            if (command == 3u) {
                LR(DE97,DE97,0u,0u); LB(DE9A,TableRoundBankWord(cpu,0x7124u)==0u);
                if (TableRoundBankWord(cpu,0x7124u)) { LR(DE9C,DE9F,0u,0u); LR(DEB5,DEB5,0u,0u); return true; }
            } else {
                LR(DEA1,DEA1,0u,0u); LB(DEA4,command!=6u);
                if (command != 6u) { LC(DE6E); }
            }
        }
    }
    LR(DEA6,DEA6,0u,0u);
    for (unsigned slot=0u;slot<3u;++slot) {
        const bool active=TableRoundBankWord(cpu,(uint16_t)(0x7124u+slot*2u))!=0u;
        LR(DEA9,DEA9,slot*2u,0u); LB(DEAC,active);
        if (active) { LR(DEB2,DEB5,slot*2u,0u); return true; }
        LR(DEAE,DEB0,slot*2u,0u);
    }
    return false;
}

static bool PlanRoundPassEnd(TableRoundClock *clock) {
    const unsigned passes=TableRoundDirectWord(clock->call.cpu,0x30u);
    if (!passes || passes>2u) return false;
    LR(DE05,DE0A,0u,0u); LB(DE0C,passes!=1u);
    if (passes!=1u) { LC(DDA8); }
    LR(DE0E,DE11,0u,0u); LC(DE13);
}

static bool PlanRoundOrdinaryScan(TableRoundClock *clock,uint16_t x,uint16_t y) {
    if (!x || x>12u) return false;
    do {
        const bool marked=(TableRoundBankWord(clock->call.cpu,(uint16_t)(0x7006u+y))&8u)!=0u;
        LR(DD10,DD13,x,y); LB(DD16,marked);
        if (marked) { LC(DD1F); }
        LR(DD18,DD1C,x,y); --x;y=(uint16_t)(y-2u); LB(DD1D,x!=0u);
    } while (x);
    LC(DD1F);
}

static bool PlanRoundOrdinaryDraw(TableRoundClock *clock) {
    const CpuState *cpu=clock->call.cpu;
    if (cpu->A>127u) return false;
    const uint16_t y=(uint16_t)((cpu->A+1u)*2u+TableRoundDirectWord(cpu,8u));
    LR(DCE1,DCEC,0u,y); LB(DCEF,TableRoundDirectWord(cpu,0x0eu)==7u);
    if (TableRoundDirectWord(cpu,0x0eu)==7u) return PlanRoundOrdinaryScan(clock,12u,y);
    LR(DCF1,DCF1,0u,0u); LC(DCF4);
}

static bool PlanRoundSpecialStart(TableRoundClock *clock) {
    const CpuState *cpu=clock->call.cpu;
    LR(DD2A,DD2A,0u,0u); LB(DD2D,TableRoundBankWord(cpu,0x7178u)!=1u);
    if (TableRoundBankWord(cpu,0x7178u)==1u) { LR(DD2F,DD35,0u,0u); LC(DD38); }
    LR(DD5E,DD5E,0u,0u); LB(DD61,TableRoundBankWord(cpu,0x7176u)!=1u);
    if (TableRoundBankWord(cpu,0x7176u)==1u) { LR(DD63,DD8E,0u,0u); return true; }
    LC(DD90);
}

static bool PlanRoundOrdinaryStart(TableRoundClock *clock) {
    const CpuState *cpu=clock->call.cpu;
    LR(DC92,DC95,0u,0u); LB(DC98,TableRoundBankWord(cpu,0x7176u)!=1u);
    if (TableRoundBankWord(cpu,0x7176u)!=1u) { LR(DCD4,DCD7,0u,0u); LC(DCD9); }
    LR(DC9A,DC9A,0u,0u); LB(DC9D,TableRoundBankWord(cpu,0x717cu)!=1u);
    if (TableRoundBankWord(cpu,0x717cu)!=1u) { LR(DCAD,DCB9,0u,0u); LC(DCBC); }
    LR(DC9F,DCAB,0u,0u); LR(DCCD,DCD7,0u,0u); LC(DCD9);
}

static bool PlanRoundMatchSearch(TableRoundClock *clock) {
    const CpuState *cpu=clock->call.cpu;
    const unsigned count=TableRoundBankWord(cpu,0x7184u),attempts=TableRoundDirectWord(cpu,0x1cu);
    if (!count || count>7u || !attempts || attempts>7u) return false;
    uint16_t x=0u,y=(uint16_t)(count*2u-1u);
    LR(DDC9,DDD0,0u,0u);
    do {
        const bool match=TableRoundBankWord(cpu,(uint16_t)(0x7146u+x))==TableRoundDirectWord(cpu,0x2au);
        LR(DDD3,DDD6,x,y); LB(DDD8,!match);
        if (match) {
            const bool enough=(int16_t)(TableRoundBankWord(cpu,(uint16_t)(0x7162u+x))-3u)>=0;
            LR(DDDA,DDE0,x,y); LB(DDE3,enough);
            if (enough) return PlanRoundPassEnd(clock);
        }
        LR(DDE5,DDE7,x,y); x=(uint16_t)(x+2u);--y; LB(DDE8,y!=0u);
    } while (y);
    LR(DDEA,DDF1,TableRoundDirectWord(cpu,0u),0u); LB(DDF3,attempts!=1u);
    if (attempts!=1u) { LC(DDC6); }
    LC(DDF5);
}

static bool PlanRoundFinalMatch(TableRoundClock *clock,bool exact) {
    const CpuState *cpu=clock->call.cpu;
    const unsigned count=TableRoundBankWord(cpu,0x7184u);
    if (!count || count>7u) return false;
    uint16_t x=0u,y=(uint16_t)(count*2u-1u);
    if (exact) { LR(DE16,DE1D,0u,0u); } else { LR(DE43,DE4A,0u,0u); }
    do {
        const bool type=TableRoundBankWord(cpu,(uint16_t)(0x7146u+x))==2u;
        if (exact) { LR(DE20,DE23,x,y); LB(DE26,!type); }
        else { LR(DE4D,DE50,x,y); LB(DE53,!type); }
        if (type) {
            const unsigned value=TableRoundBankWord(cpu,(uint16_t)(0x7162u+x));
            const bool match=exact?value==8u:(int16_t)(value-3u)<0;
            if (exact) { LR(DE28,DE2B,x,y); LB(DE2E,!match); }
            else { LR(DE55,DE58,x,y); LB(DE5B,!match); }
            if (match) {
                if (exact) { LR(DE30,DE37,0u,0u); }
                else { LR(DE5D,DE5F,0u,0u); }
                break;
            }
        }
        if (exact) { LR(DE39,DE3B,x,y); LB(DE3C,y!=1u); }
        else { LR(DE61,DE63,x,y); LB(DE64,y!=1u); }
        x=(uint16_t)(x+2u);--y;
    } while (y);
    if (exact) { LR(DE3E,DE3E,0u,0u); LC(DE40); }
    LR(DE66,DE66,0u,0u); LC(DE68);
}

static bool PlanRoundAdvance(TableRoundClock *clock,uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    switch (pc) {
    case 0xdc87u:
        LR(DC87,DC8A,0u,0u); LB(DC8D,(TableRoundBankWord(cpu,0x7170u)&8u)==0u);
        if (TableRoundBankWord(cpu,0x7170u)&8u) { LR(DC8F,DC8F,0u,0u); return PlanRoundSpecialStart(clock); }
        return PlanRoundOrdinaryStart(clock);
    case 0xdcc0u: LR(DCC0,DCCA,0u,0u); LR(DCCD,DCD7,0u,0u); LC(DCD9);
    case 0xdcdcu: LR(DCDC,DCDC,0u,0u); LC(DCDE);
    case 0xdce1u: return PlanRoundOrdinaryDraw(clock);
    case 0xdcf8u:
        LR(DCF8,DCF8,0u,0u); LB(DCFB,cpu->A!=0u);
        if (cpu->A) return PlanRoundOrdinaryScan(clock,cpu->X,cpu->Y);
        LR(DCFD,DD00,cpu->X,cpu->Y); LB(DD03,(TableRoundBankWord(cpu,(uint16_t)(0x7006u+cpu->Y))&8u)==0u);
        if (TableRoundBankWord(cpu,(uint16_t)(0x7006u+cpu->Y))&8u) {
            LR(DD05,DD07,0u,0u); LB(DD0A,TableRoundDirectWord(cpu,2u)==6u);
            if (TableRoundDirectWord(cpu,2u)!=6u) { LR(DD0C,DD0E,0u,0u); }
        }
        LC(DD1F);
    case 0xdd22u: {
        const unsigned passes=TableRoundDirectWord(cpu,0x30u);
        if (!passes || passes>3u) return false;
        LR(DD22,DD22,0u,0u); LB(DD24,passes!=1u);
        if (passes!=1u) { LC(DCD9); }
        LR(DD26,DD26,0u,0u); return true;
    }
    case 0xdd3bu: LR(DD3B,DD44,0u,0u); LC(DD47);
    case 0xdd4au: LR(DD4A,DD5B,0u,0u); return true;
    case 0xdd93u: LR(DD93,DD93,0u,0u); LC(DD96);
    case 0xdd9au: LR(DD9A,DDA6,0u,0u); LC(DDA8);
    case 0xddabu: LC(DDAB);
    case 0xddaeu: LC(DDAE);
    case 0xddb1u: LC(DDB1);
    case 0xddb4u: LR(DDB4,DDC4,0u,0u); LC(DDC6);
    case 0xddc9u: return PlanRoundMatchSearch(clock);
    case 0xddf8u: LR(DDF8,DE03,0u,0u); return PlanRoundPassEnd(clock);
    case 0xde16u: return PlanRoundFinalMatch(clock,true);
    case 0xde43u: return PlanRoundFinalMatch(clock,false);
    case 0xde6bu: LR(DE6B,DE6B,0u,0u); LR(DD26,DD26,0u,0u); return true;
    default: return false;
    }
}

#undef LR
#undef LB
#undef LC
static uint64_t TableRoundBudget(const TableRoundClock *clock) {
    uint64_t budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    return budget + 40u * (budget / 1324u + 1u);
}
static bool PrepareTableRoundClock(TableRoundClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = table_round_ends[clock->root];
    clock->returning = true;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x28000u || cpu->PB != 0x84u || pc >> 16 != 0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        clock->call.frame.hrv != 2u || cpu->host_return_valid < 2u || cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const bool entry = pc == table_round_entries[clock->root];
    if ((entry && cpu->host_return_valid != 2u) || cpu->x_flag || cpu->m_flag) return false;
    bool planned;
    switch (clock->root) {
    case 0u: planned = PlanRoundCommand(clock, (uint16_t)pc); break;
    case 1u: planned = PlanRoundNumber(clock, (uint16_t)pc); break;
    case 2u: planned = PlanRoundAdvance(clock, (uint16_t)pc); break;
    default: planned = PlanRoundColumn(clock, (uint16_t)pc); break;
    }
    if (!planned) return false;
    const uint64_t budget = TableRoundBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, table_round_entries[clock->root],
            0x1ff7u, true, 1363u, budget)) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
static uint8_t TableRoundChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableRoundClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(!clock->returning && clock->next == site + frame_size + 1u);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableRoundClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}
static RecompReturn ActorBridgeTableRound(CpuState *cpu, unsigned root) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableRoundClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTableRoundClock(&clock, table_round_entries[root]))
        return ActorBridgeFallback(cpu, &frame, table_round_entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result = Lufia2MenuConsumeCommand(&memory, &state); break;
    case 1u: result = Lufia2CardTableRefreshRoundNumber(&memory, &state, TableRoundChild, &clock); break;
    case 2u: result = Lufia2CardTableAdvanceRound(&memory, &state, TableRoundChild, &clock); break;
    default: result = Lufia2CardTableChooseRoundColumn(&memory, &state, TableRoundChild, &clock); break;
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
