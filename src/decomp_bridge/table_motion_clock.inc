typedef struct TableMotionOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index;
    int8_t advance_x, advance_y;
} TableMotionOpcode;

enum {
    MOTION_PC_E9E3 = 0u,
    MOTION_PC_E9E4 = 1u,
    MOTION_PC_E9E6 = 2u,
    MOTION_PC_E9E9 = 3u,
    MOTION_PC_E9EB = 4u,
    MOTION_PC_E9EC = 5u,
    MOTION_PC_E9ED = 6u,
    MOTION_PC_E9EE = 7u,
    MOTION_PC_E9F1 = 8u,
    MOTION_PC_E9F3 = 9u,
    MOTION_PC_E9F5 = 10u,
    MOTION_PC_E9F6 = 11u,
    MOTION_PC_E9F9 = 12u,
    MOTION_PC_E9FB = 13u,
    MOTION_PC_E9FE = 14u,
    MOTION_PC_EA00 = 15u,
    MOTION_PC_EA03 = 16u,
    MOTION_PC_EA04 = 17u,
    MOTION_PC_EA07 = 18u,
    MOTION_PC_EA09 = 19u,
    MOTION_PC_EA0C = 20u,
    MOTION_PC_EA0F = 21u,
    MOTION_PC_EA12 = 22u,
    MOTION_PC_EA15 = 23u,
    MOTION_PC_EA17 = 24u,
    MOTION_PC_EA18 = 25u,
    MOTION_PC_EA1B = 26u,
    MOTION_PC_EA1D = 27u,
    MOTION_PC_EA20 = 28u,
    MOTION_PC_EA23 = 29u,
    MOTION_PC_EA24 = 30u,
    MOTION_PC_EA25 = 31u,
    MOTION_PC_EA26 = 32u,
    MOTION_PC_EA27 = 33u,
    MOTION_PC_EA28 = 34u,
    MOTION_PC_EA29 = 35u,
    MOTION_PC_EA2B = 36u,
    MOTION_PC_EA2C = 37u,
    MOTION_PC_EA2D = 38u,
    MOTION_PC_EA2F = 39u,
    MOTION_PC_EA30 = 40u,
    MOTION_PC_EA33 = 41u,
    MOTION_PC_EA34 = 42u,
    MOTION_PC_EA36 = 43u,
    MOTION_PC_EA39 = 44u,
    MOTION_PC_EA3A = 45u,
    MOTION_PC_EA3B = 46u,
    MOTION_PC_EA3C = 47u,
    MOTION_PC_EA3D = 48u,
    MOTION_PC_EA3E = 49u,
    MOTION_PC_EA40 = 50u,
    MOTION_PC_EA42 = 51u,
    MOTION_PC_EA44 = 52u,
    MOTION_PC_EA47 = 53u,
    MOTION_PC_EA49 = 54u,
    MOTION_PC_EA4A = 55u,
    MOTION_PC_EA4B = 56u,
    MOTION_PC_EA4C = 57u,
    MOTION_PC_EA4D = 58u,
    MOTION_PC_EA4E = 59u,
    MOTION_PC_EA50 = 60u,
    MOTION_PC_EA52 = 61u,
    MOTION_PC_EA54 = 62u,
    MOTION_PC_EA56 = 63u,
    MOTION_PC_EA59 = 64u,
    MOTION_PC_EA5B = 65u,
    MOTION_PC_EA5E = 66u,
    MOTION_PC_EA60 = 67u,
    MOTION_PC_EA63 = 68u,
    MOTION_PC_EA64 = 69u,
    MOTION_PC_EA65 = 70u,
    MOTION_PC_EA66 = 71u,
    MOTION_PC_EA69 = 72u,
    MOTION_PC_EA6C = 73u,
    MOTION_PC_EA6F = 74u,
    MOTION_PC_EA72 = 75u,
    MOTION_PC_EA74 = 76u,
    MOTION_PC_EA77 = 77u,
    MOTION_PC_EA79 = 78u,
    MOTION_PC_EA7C = 79u,
    MOTION_PC_EA7D = 80u,
    MOTION_PC_EA7E = 81u,
    MOTION_PC_EA7F = 82u,
    MOTION_PC_EA82 = 83u,
    MOTION_PC_EA85 = 84u,
    MOTION_PC_EA88 = 85u,
    MOTION_PC_EA8B = 86u,
    MOTION_PC_EA8D = 87u,
    MOTION_PC_EA90 = 88u,
    MOTION_PC_EA92 = 89u,
    MOTION_PC_EA95 = 90u,
    MOTION_PC_EA96 = 91u,
    MOTION_PC_EA97 = 92u,
    MOTION_PC_EA98 = 93u,
    MOTION_PC_EA9B = 94u,
    MOTION_PC_EA9E = 95u,
    MOTION_PC_EAA1 = 96u,
    MOTION_PC_EAA2 = 97u,
    MOTION_PC_EAA3 = 98u,
    MOTION_PC_EAA4 = 99u,
    MOTION_PC_EAA5 = 100u,
    MOTION_PC_EAA7 = 101u,
    MOTION_PC_EAA8 = 102u,
    MOTION_PC_EAA9 = 103u,
    MOTION_PC_EAAA = 104u,
    MOTION_PC_EAAB = 105u,
    MOTION_PC_EAAD = 106u,
    MOTION_PC_EAAF = 107u,
    MOTION_PC_EAB2 = 108u,
    MOTION_PC_EAB4 = 109u,
    MOTION_PC_EAB6 = 110u,
    MOTION_PC_EAB8 = 111u,
    MOTION_PC_EABB = 112u,
    MOTION_PC_EABD = 113u,
    MOTION_PC_EAC0 = 114u,
    MOTION_PC_EAC2 = 115u,
    MOTION_PC_EAC5 = 116u,
    MOTION_PC_EAC7 = 117u,
    MOTION_PC_EAC8 = 118u,
    MOTION_PC_EACB = 119u,
    MOTION_PC_EACC = 120u,
    MOTION_PC_EACD = 121u,
    MOTION_PC_EACE = 122u,
    MOTION_PC_EACF = 123u,
    MOTION_PC_EAD0 = 124u,
    MOTION_PC_EAD1 = 125u,
    MOTION_PC_EAD2 = 126u,
    MOTION_PC_EAD4 = 127u,
    MOTION_PC_EAD6 = 128u,
    MOTION_PC_EAD8 = 129u,
    MOTION_PC_EADB = 130u,
    MOTION_PC_EADC = 131u,
    MOTION_PC_EADD = 132u,
    MOTION_PC_EAE0 = 133u,
    MOTION_PC_EAE3 = 134u,
    MOTION_PC_EAE4 = 135u,
    MOTION_PC_EAE5 = 136u,
    MOTION_PC_EAE7 = 137u,
    MOTION_PC_EAE8 = 138u,
    MOTION_PC_EAEB = 139u,
    MOTION_PC_EAEE = 140u,
    MOTION_PC_EAEF = 141u,
    MOTION_PC_EAF0 = 142u,
    MOTION_PC_EAF1 = 143u,
    MOTION_PC_EAF2 = 144u,
    MOTION_PC_EAF5 = 145u,
    MOTION_PC_EAF8 = 146u,
    MOTION_PC_EAF9 = 147u,
    MOTION_PC_EAFA = 148u,
    MOTION_PC_EAFC = 149u,
    MOTION_PC_EAFD = 150u,
    MOTION_PC_EB00 = 151u,
    MOTION_PC_EB03 = 152u,
    MOTION_PC_EB04 = 153u,
    MOTION_PC_EB05 = 154u,
    MOTION_PC_EB06 = 155u,
    MOTION_PC_EB07 = 156u,
    MOTION_PC_EB0A = 157u,
    MOTION_PC_EB0D = 158u,
    MOTION_PC_EB0E = 159u,
    MOTION_PC_EB0F = 160u,
    MOTION_PC_EB11 = 161u,
    MOTION_PC_EB12 = 162u,
    MOTION_PC_EB15 = 163u,
    MOTION_PC_EB18 = 164u,
    MOTION_PC_EB19 = 165u,
    MOTION_PC_EB1A = 166u,
    MOTION_PC_EB1B = 167u,
    MOTION_PC_EB1C = 168u,
    MOTION_PC_EB1F = 169u,
    MOTION_PC_EB22 = 170u,
    MOTION_PC_EB23 = 171u,
    MOTION_PC_EB24 = 172u,
    MOTION_PC_EB26 = 173u,
    MOTION_PC_EB28 = 174u,
    MOTION_PC_EB29 = 175u,
    MOTION_PC_EB2C = 176u,
    MOTION_PC_EB2E = 177u,
    MOTION_PC_EB30 = 178u,
    MOTION_PC_EB32 = 179u,
    MOTION_PC_EB35 = 180u,
    MOTION_PC_ED9E = 181u,
    MOTION_PC_EDA1 = 182u,
    MOTION_PC_EDA4 = 183u,
    MOTION_PC_EDA7 = 184u,
    MOTION_PC_EDAA = 185u,
    MOTION_PC_EDAB = 186u,
    MOTION_PC_EDAC = 187u,
    MOTION_PC_EDAD = 188u,
    MOTION_PC_EDAF = 189u,
    MOTION_PC_EDB2 = 190u,
    MOTION_PC_EDB5 = 191u,
    MOTION_PC_EDB8 = 192u,
    MOTION_PC_EDB9 = 193u,
    MOTION_PC_EDBA = 194u,
    MOTION_PC_EDBB = 195u,
    MOTION_PC_EDBD = 196u,
    MOTION_PC_EDBF = 197u,
    MOTION_PC_EDC1 = 198u,
    MOTION_PC_EDC3 = 199u,
    MOTION_PC_EDC5 = 200u,
    MOTION_PC_EDC8 = 201u,
    MOTION_PC_EDCA = 202u,
    MOTION_PC_EDCB = 203u,
    MOTION_PC_EDCE = 204u,
    MOTION_PC_EDD0 = 205u,
    MOTION_PC_EDD3 = 206u,
    MOTION_PC_EDD5 = 207u,
    MOTION_PC_EDD8 = 208u,
    MOTION_PC_EDDB = 209u,
    MOTION_PC_EDDE = 210u,
    MOTION_PC_EDE0 = 211u,
    MOTION_PC_EDE3 = 212u,
    MOTION_PC_EDE5 = 213u,
    MOTION_PC_EDE6 = 214u,
    MOTION_PC_EDE7 = 215u,
    MOTION_PC_EDEA = 216u,
    MOTION_PC_EDEC = 217u,
    MOTION_PC_EDED = 218u,
    MOTION_PC_EDEE = 219u,
    MOTION_PC_EDF1 = 220u,
    MOTION_PC_EDF3 = 221u,
    MOTION_PC_EDF6 = 222u,
    MOTION_PC_EDF7 = 223u,
    MOTION_PC_EDF9 = 224u,
    MOTION_PC_F034 = 225u,
    MOTION_PC_F035 = 226u,
    MOTION_PC_F037 = 227u,
    MOTION_PC_F03A = 228u,
    MOTION_PC_F03C = 229u,
    MOTION_PC_F03F = 230u,
    MOTION_PC_F041 = 231u,
    MOTION_PC_F044 = 232u,
    MOTION_PC_F047 = 233u,
    MOTION_PC_F04A = 234u,
    MOTION_PC_F04D = 235u,
    MOTION_PC_F04F = 236u,
    MOTION_PC_F052 = 237u,
    MOTION_PC_F055 = 238u,
    MOTION_PC_F057 = 239u,
    MOTION_PC_F05A = 240u,
    MOTION_PC_F05D = 241u,
    MOTION_PC_F05F = 242u,
    MOTION_PC_F062 = 243u,
    MOTION_PC_F064 = 244u,
    MOTION_PC_F067 = 245u,
    MOTION_PC_F06A = 246u,
    MOTION_PC_F06C = 247u,
    MOTION_PC_F06F = 248u,
    MOTION_PC_F071 = 249u,
    MOTION_PC_F072 = 250u,
    MOTION_PC_F073 = 251u,
    MOTION_PC_F074 = 252u,
    MOTION_PC_F075 = 253u,
    MOTION_PC_F078 = 254u,
    MOTION_PC_F07A = 255u,
    MOTION_PC_F07D = 256u,
    MOTION_PC_F080 = 257u,
    MOTION_PC_F083 = 258u,
    MOTION_PC_F085 = 259u,
    MOTION_PC_F088 = 260u,
    MOTION_PC_F089 = 261u,
    MOTION_PC_F08A = 262u,
    MOTION_PC_F08B = 263u,
    MOTION_PC_F08D = 264u,
    MOTION_PC_F08F = 265u,
    MOTION_PC_F090 = 266u,
    MOTION_PC_F093 = 267u,
    MOTION_PC_F095 = 268u,
    MOTION_PC_F096 = 269u,
    MOTION_PC_F098 = 270u,
    MOTION_PC_F09A = 271u,
    MOTION_PC_F09B = 272u,
    MOTION_PC_F09C = 273u,
    MOTION_PC_F09F = 274u,
    MOTION_PC_F0A1 = 275u,
    MOTION_PC_F0A4 = 276u,
    MOTION_PC_F0A7 = 277u,
    MOTION_PC_F0A8 = 278u,
    MOTION_PC_F0AB = 279u,
    MOTION_PC_F0AD = 280u,
    MOTION_PC_F0AE = 281u,
    MOTION_PC_F0AF = 282u,
    MOTION_PC_F0B2 = 283u,
    MOTION_PC_F0B4 = 284u,
    MOTION_PC_F0B5 = 285u,
    MOTION_PC_F0B6 = 286u,
    MOTION_PC_F0B7 = 287u,
    MOTION_PC_F0B8 = 288u,
    MOTION_PC_F0B9 = 289u,
    MOTION_PC_F0BA = 290u,
    MOTION_PC_F0BB = 291u,
    MOTION_PC_F0BC = 292u,
    MOTION_PC_F0BF = 293u,
    MOTION_PC_F0C1 = 294u,
    MOTION_PC_F0C4 = 295u,
    MOTION_PC_F0C6 = 296u,
    MOTION_PC_F0C9 = 297u,
    MOTION_PC_F0CA = 298u,
    MOTION_PC_F0CB = 299u,
    MOTION_PC_F0CD = 300u,
    MOTION_PC_F0D0 = 301u,
    MOTION_PC_F0D3 = 302u,
    MOTION_PC_F0D4 = 303u,
    MOTION_PC_F0D7 = 304u,
    MOTION_PC_F0D9 = 305u,
    MOTION_PC_F0DC = 306u,
    MOTION_PC_F0DF = 307u,
    MOTION_PC_F0E2 = 308u,
    MOTION_PC_F0E4 = 309u,
    MOTION_PC_F0E7 = 310u,
    MOTION_PC_F0E9 = 311u,
    MOTION_PC_F0EC = 312u,
    MOTION_PC_F0EF = 313u,
    MOTION_PC_F0F1 = 314u,
    MOTION_PC_F0F4 = 315u,
    MOTION_PC_F0F6 = 316u,
    MOTION_PC_F0F9 = 317u,
    MOTION_PC_F0FB = 318u,
    MOTION_PC_F0FE = 319u,
    MOTION_PC_F101 = 320u,
    MOTION_PC_F104 = 321u,
    MOTION_PC_F105 = 322u,
    MOTION_PC_F106 = 323u,
    MOTION_PC_F107 = 324u,
    MOTION_PC_F108 = 325u,
    MOTION_PC_F10A = 326u,
    MOTION_PC_F10C = 327u,
    MOTION_PC_F10E = 328u,
    MOTION_PC_F111 = 329u,
    MOTION_PC_F114 = 330u,
    MOTION_PC_F116 = 331u,
    MOTION_PC_F119 = 332u,
    MOTION_PC_F11A = 333u,
    MOTION_PC_F11B = 334u,
    MOTION_PC_F11C = 335u,
    MOTION_PC_F11E = 336u,
    MOTION_PC_F120 = 337u,
    MOTION_PC_F121 = 338u,
    MOTION_PC_F124 = 339u,
    MOTION_PC_F126 = 340u,
    MOTION_PC_F127 = 341u,
    MOTION_PC_F129 = 342u,
    MOTION_PC_F12B = 343u,
    MOTION_PC_F12C = 344u,
    MOTION_PC_F12D = 345u,
    MOTION_PC_F12E = 346u,
};

static const TableMotionOpcode table_motion_opcodes[] = {
    {{0xe9e3u, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe9e4u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9e6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9e9u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe9ebu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9ecu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9edu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9eeu, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xe9f1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9f3u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe9f5u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9f6u, 3u, 5u, 2u}, 0x712au, 1u, 0, 0},
    {{0xe9f9u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe9fbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9feu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea00u, 3u, 5u, 2u}, 0x7000u, 1u, 0, 0},
    {{0xea03u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea04u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea07u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea09u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea0cu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea0fu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea12u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea15u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea17u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea18u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea1bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea1du, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea20u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea23u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea24u, 1u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea25u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea26u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea27u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea28u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea29u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea2bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea2cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea2du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea2fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea30u, 3u, 5u, 2u}, 0x0100u, 1u, 0, 0},
    {{0xea33u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea34u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea36u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea39u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xea3au, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xea3bu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xea3cu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xea3du, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xea3eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea40u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xea42u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea44u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea47u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea49u, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xea4au, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea4bu, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea4cu, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea4du, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xea4eu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea50u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea52u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xea54u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea56u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea59u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea5bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea5eu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea60u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea63u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea64u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea65u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea66u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea69u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea6cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea6fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea72u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea74u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea77u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea79u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea7cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea7du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea7eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea7fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea82u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea85u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea88u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea8bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea8du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea90u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea92u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xea95u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea96u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea97u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea98u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea9bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xea9eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaa1u, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xeaa2u, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaa3u, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaa4u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeaa5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaa7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xeaa8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xeaa9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xeaaau, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xeaabu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaadu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaafu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeab2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeab4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeab6u, 2u, 7u, 5u}, 0x0000u, 0u, 0, 0},
    {{0xeab8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeabbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeabdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeac0u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeac2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeac5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeac7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeac8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeacbu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeaccu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeacdu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeaceu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeacfu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xead0u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xead1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xead2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xead4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xead6u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xead8u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeadbu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeadcu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeaddu, 3u, 5u, 2u}, 0x70b4u, 1u, 0, 0},
    {{0xeae0u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeae3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeae4u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeae5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeae7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeae8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeaebu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaeeu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeaefu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeaf0u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeaf1u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeaf2u, 3u, 5u, 2u}, 0x70b4u, 1u, 0, 0},
    {{0xeaf5u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeaf8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeaf9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeafau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeafcu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeafdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb00u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb03u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb04u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb05u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeb06u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeb07u, 3u, 5u, 2u}, 0x70b4u, 1u, 0, 0},
    {{0xeb0au, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb0du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb0eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb0fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb11u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb12u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb15u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb18u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb19u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb1au, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeb1bu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeb1cu, 3u, 5u, 2u}, 0x70b4u, 1u, 0, 0},
    {{0xeb1fu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb22u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb23u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xeb24u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb26u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb28u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb29u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb2cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb2eu, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xeb30u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb32u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb35u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xed9eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeda1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeda4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeda7u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedaau, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xedabu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xedacu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedadu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedafu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedb2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedb5u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedb8u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xedb9u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xedbau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedbbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedbdu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedbfu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedc1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedc3u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xedc5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedc8u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedcau, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedcbu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedceu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedd0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedd3u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedd5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedd8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeddbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeddeu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xede0u, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xede3u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xede5u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xede6u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xede7u, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xedeau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedecu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xededu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xedeeu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedf1u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedf3u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xedf6u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xedf7u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xedf9u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf034u, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf035u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf037u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf03au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf03cu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf03fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf041u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf044u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf047u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf04au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf04du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf04fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf052u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf055u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf057u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf05au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf05du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf05fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf062u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf064u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf067u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf06au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf06cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf06fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf071u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf072u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf073u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf074u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf075u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf078u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf07au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf07du, 3u, 5u, 2u}, 0x2000u, 1u, 0, 0},
    {{0xf080u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf083u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf085u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf088u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf089u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf08au, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf08bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf08du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf08fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf090u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf093u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf095u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf096u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf098u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf09au, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf09bu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf09cu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf09fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0a1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0a4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0a7u, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0a8u, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xf0abu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0adu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf0aeu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf0afu, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xf0b2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0b4u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf0b5u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf0b6u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0b7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0b8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0b9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0bau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0bbu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0bcu, 3u, 5u, 2u}, 0x7188u, 2u, 0, 0},
    {{0xf0bfu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0c1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0c4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0c6u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0c9u, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0cau, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf0cbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0cdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0d0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0d3u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0d4u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf0d7u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0d9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0dcu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0dfu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0e2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0e4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0e7u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0e9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0ecu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0efu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0f1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0f4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0f6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf0f9u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0fbu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf0feu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf101u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf104u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf105u, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf106u, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf107u, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf108u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf10au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf10cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf10eu, 3u, 5u, 2u}, 0x2000u, 1u, 0, 0},
    {{0xf111u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf114u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf116u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf119u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf11au, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xf11bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xf11cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf11eu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf120u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf121u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf124u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf126u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf127u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xf129u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xf12bu, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xf12cu, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf12du, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xf12eu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
};

enum { TABLE_MOTION_CAPACITY = 4096u };
typedef struct TableMotionClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_MOTION_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint32_t next;
    bool returning;
} TableMotionClock;
static const uint32_t table_motion_entries[] = {
    0x84e9e3u,0x84ea4bu,0x84eaa5u,0x84ed9eu,0x84edcbu,
    0x84f034u,0x84f09cu,0x84f0d4u,0x84f105u
};
static const uint32_t table_motion_ends[] = {
    0x84ea4au,0x84eaa4u,0x84eb35u,0x84edcau,0x84edf9u,
    0x84f09bu,0x84f0d3u,0x84f104u,0x84f12eu
};
static uint16_t MotionDirectWord(const CpuState *cpu, unsigned at) {
    return cpu->ram[at] | (uint16_t)cpu->ram[at + 1u] << 8;
}
static uint16_t MotionBankWord(const CpuState *cpu, uint16_t at) {
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    return cpu->ram[bank + at] | (uint16_t)cpu->ram[bank + (uint16_t)(at + 1u)] << 8;
}
static bool MotionOpcode(TableMotionClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == TABLE_MOTION_CAPACITY) return false;
    clock->timing[clock->span.count++] = op;
    return true;
}
static bool MotionRange(TableMotionClock *clock, unsigned first, unsigned last,
                         uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableMotionOpcode *record = table_motion_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!MotionOpcode(clock, op)) return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}
static bool MotionBranch(TableMotionClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_motion_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return MotionOpcode(clock, op);
}
#define MR(first,last,x,y) do { if (!MotionRange(clock, MOTION_PC_##first, MOTION_PC_##last, (uint16_t)(x), (uint16_t)(y))) return false; } while (0)
#define MB(pc,taken) do { if (!MotionBranch(clock, MOTION_PC_##pc, taken)) return false; } while (0)
#define MC(site) do { MR(site,site,0u,0u); clock->next = 0x840000u | (table_motion_opcodes[MOTION_PC_##site].timing.pc + 3u); clock->returning = false; return true; } while (0)

static bool PlanMovingColumns(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    unsigned column = pc == 0xe9e3u ? 0u : MotionDirectWord(cpu, 0x11u);
    if (column >= 3u || (pc != 0xe9e3u && (pc != 0xea15u || cpu->X != column * 2u))) return false;
    if (pc == 0xe9e3u) { MR(E9E3,E9E9,0u,0u); }
    for (; column < 3u; ++column) {
        const uint16_t x = (uint16_t)(column * 2u);
        bool moving = true;
        if (pc == 0xea15u) { MR(EA15,EA1B,x,0u); }
        else {
            MR(E9EB,E9EE,x,0u);
            const uint16_t step = MotionBankWord(cpu, (uint16_t)(0x7124u + x));
            moving = step != 0u;
            MB(E9F1,!moving);
            if (moving) {
                MR(E9F3,E9FB,x,0u);
                const uint16_t position = (uint16_t)(step + MotionBankWord(cpu, (uint16_t)(0x712au + x)));
                const bool below = ((uint16_t)(position - 0x1c00u) & 0x8000u) != 0u;
                MB(E9FE,below);
                if (!below) {
                    MR(EA00,EA04,x,0u);
                    const uint16_t speed = (uint16_t)(MotionBankWord(cpu, (uint16_t)(0x7000u + x)) - 1u);
                    const bool keep = ((uint16_t)(speed - 5u) & 0x8000u) == 0u;
                    MB(EA07,keep);
                    if (!keep) { MR(EA09,EA09,x,0u); }
                    MR(EA0C,EA0F,x,0u);
                    MC(EA12);
                }
            }
        }
        if (moving) {
            MR(EA1D,EA2F,x,0u);
            for (unsigned sprite = 0u; sprite < 24u; ++sprite) {
                MR(EA30,EA3D,column * 96u + sprite * 4u,24u - sprite);
                MB(EA3E,sprite != 23u);
            }
        }
        MR(EA40,EA44,0u,0u);
        MB(EA47,column != 2u);
        pc = 0xe9ebu;
    }
    MR(EA49,EA49,0u,0u);
    return true;
}
static bool PlanRefreshColumns(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    unsigned column;
    switch (pc) {
    case 0xea4bu: column = 0u; MR(EA4B,EA54,0u,0u); break;
    case 0xea6fu: column = 1u; break;
    case 0xea88u: column = 2u; break;
    case 0xeaa1u: column = 3u; break;
    default: return false;
    }
    for (; column < 3u; ++column) {
        const bool moving = MotionBankWord(cpu, (uint16_t)(0x712au + column * 2u)) != 0u;
        switch (column) {
        case 0u:
            MR(EA56,EA56,0u,0u); MB(EA59,moving);
            if (!moving) { MR(EA5B,EA69,0u,0u); MC(EA6C); }
            break;
        case 1u:
            MR(EA6F,EA6F,0u,0u); MB(EA72,moving);
            if (!moving) { MR(EA74,EA82,0u,0u); MC(EA85); }
            break;
        default:
            MR(EA88,EA88,0u,0u); MB(EA8B,moving);
            if (!moving) { MR(EA8D,EA9B,0u,0u); MC(EA9E); }
            break;
        }
    }
    MR(EAA1,EAA3,0u,0u);
    return true;
}
static bool PlanDrawColumn(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    const uint16_t pointer = MotionDirectWord(cpu, 8u);
    if (pc != 0xeaa5u || cpu->X > 0x120u || cpu->Y < 4u || cpu->Y > 60u ||
        cpu->ram[0xau] != 0x7eu || (pointer != 0x7006u && pointer != 0x7040u && pointer != 0x707au)) return false;
    MR(EAA5,EAB2,0u,0u);
    for (unsigned row = 0u; row < 6u; ++row) {
        const uint16_t source = (uint16_t)(cpu->Y - 4u + row * 2u);
        uint16_t symbol = MotionDirectWord(cpu, pointer + source);
        MR(EAB4,EAB8,0u,source);
        MB(EABB,source != 48u);
        if (source == 48u) { MR(EABD,EAC0,0u,source); symbol = 9u; }
        else {
            MR(EAC2,EAC2,0u,source);
            const bool below = ((uint16_t)(symbol - 8u) & 0x8000u) != 0u;
            MB(EAC5,below);
            if (!below) { MR(EAC7,EAC8,0u,source); symbol = (uint16_t)(symbol - 3u); }
        }
        const uint16_t tile = (uint16_t)((symbol << 3) | (symbol >> 14));
        MR(EACB,EB2E,tile,cpu->X + row * 16u);
        MB(EB30,row == 5u);
        if (row != 5u) { MR(EB32,EB32,0u,0u); }
    }
    return true;
}
static bool PlanRoundReset(TableMotionClock *clock, uint16_t pc) {
    if (pc == 0xedc8u) { MR(EDC8,EDC8,0u,0u); return true; }
    if (pc != 0xed9eu) return false;
    MR(ED9E,EDA4,0u,0u);
    for (unsigned word = 0u; word < 6u; ++word) {
        MR(EDA7,EDAC,word * 2u,0u); MB(EDAD,word != 5u);
    }
    MR(EDAF,EDB2,0u,0u);
    for (unsigned word = 0u; word < 4u; ++word) {
        MR(EDB5,EDBA,word * 2u,0u); MB(EDBB,word != 3u);
    }
    MR(EDBD,EDC3,0u,0u);
    MC(EDC5);
}
static bool PlanRestoreLines(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    unsigned x = 0u;
    if (pc == 0xedcbu) {
        MR(EDCB,EDCB,0u,0u);
        const bool ordinary = MotionBankWord(cpu, 0x71b0u) == 0u;
        MB(EDCE,ordinary);
        if (ordinary) { MR(EDD5,EDD5,0u,0u); }
        else { MR(EDD0,EDD3,0u,0u); }
        MR(EDD8,EDDE,0u,0u);
    } else if (pc == 0xedf6u) {
        if (!cpu->Y || cpu->Y > 8u || cpu->X > 32u || (cpu->X & 3u)) return false;
        MR(EDF6,EDF6,0u,0u); MB(EDF7,cpu->Y != 1u);
        if (cpu->Y == 1u) return true;
        x = cpu->X;
    } else return false;
    MR(EDE0,EDF1,x,0u);
    MC(EDF3);
}
static bool PlanPanelAttributes(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc != 0xf034u) return false;
    MR(F034,F037,0u,0u);
    const bool disabled = MotionBankWord(cpu, 0x71b0u) == 0u;
    MB(F03A,disabled);
    if (!disabled) {
        MR(F03C,F03C,0u,0u);
        const bool waiting = MotionBankWord(cpu, 0x71b8u) != 1u;
        MB(F03F,waiting);
        if (!waiting) {
            MR(F041,F04A,0u,0u);
            const uint16_t flags = MotionBankWord(cpu, 0x71bcu);
            MB(F04D,flags == 0u);
            if (flags) {
                MR(F04F,F052,0u,0u); MB(F055,(flags & 8u) != 0u);
                if (!(flags & 8u)) {
                    MR(F057,F05A,0u,0u);
                    const bool different = MotionBankWord(cpu, 0x71bau) != 0x400u;
                    MB(F05D,different);
                    if (!different) { MR(F05F,F05F,0u,0u); }
                }
            }
            MR(F062,F078,0u,0u);
            for (unsigned row = 0u; row < 6u; ++row) {
                if (row) { MR(F07A,F07A,0u,0u); }
                const unsigned cells = row ? 6u : 4u;
                for (unsigned cell = 0u; cell < cells; ++cell) {
                    MR(F07D,F08A,(row ? 0xf4u + row * 0x40u : 0xf8u) + cell * 2u,0u);
                    MB(F08B,cell + 1u != cells);
                }
                MR(F08D,F096,0u,0u); MB(F098,row != 5u);
            }
        }
    }
    MR(F09A,F09A,0u,0u);
    return true;
}
static bool PlanActiveLines(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    unsigned remaining = 5u, x = 0u;
    if (pc == 0xf09cu) {
        MR(F09C,F09C,0u,0u);
        const bool waiting = MotionBankWord(cpu, 0x7192u) != 1u;
        MB(F09F,waiting);
        if (waiting) return true;
        MR(F0A1,F0A4,0u,0u);
    } else if (pc == 0xf0c9u) {
        remaining = MotionDirectWord(cpu, cpu->S + 1u);
        if (!remaining || remaining > 5u || cpu->X > 20u || (cpu->X & 3u)) return false;
        MR(F0C9,F0CA,0u,0u); MB(F0CB,remaining != 1u);
        --remaining;
        x = cpu->X;
    } else return false;
    while (remaining) {
        MR(F0A7,F0BC,x,x / 2u);
        const bool inactive = MotionBankWord(cpu, (uint16_t)(0x7188u + x / 2u)) == 0u;
        MB(F0BF,inactive);
        if (!inactive) { MR(F0C1,F0C4,0u,0u); MC(F0C6); }
        MR(F0C9,F0CA,0u,0u); MB(F0CB,remaining != 1u);
        --remaining; x += 4u;
    }
    MR(F0CD,F0D0,0u,0u);
    return true;
}
static bool PlanAccentAttributes(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xf0feu) { MR(F0FE,F101,0u,0u); return true; }
    if (pc != 0xf0d4u) return false;
    MR(F0D4,F0D4,0u,0u);
    const bool waiting = MotionBankWord(cpu, 0x71b4u) != 1u;
    MB(F0D7,waiting);
    if (waiting) return true;
    MR(F0D9,F0DF,0u,0u);
    const bool same = MotionBankWord(cpu, 0x71b2u) == 0x1800u;
    MB(F0E2,same);
    if (!same) { MR(F0E4,F0E4,0u,0u); }
    MR(F0E7,F0F9,0u,0u);
    MC(F0FB);
}
static bool PlanTileAttributes(TableMotionClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    const unsigned width = MotionDirectWord(cpu,0u), rows = MotionDirectWord(cpu,6u);
    const unsigned origin = MotionDirectWord(cpu,2u);
    if (pc != 0xf105u || !width || width > 16u || !rows || rows > 8u ||
        origin + (rows - 1u) * 0x40u + width * 2u > 0x2000u) return false;
    MR(F105,F10A,0u,0u);
    for (unsigned row = 0u; row < rows; ++row) {
        MR(F10C,F10C,0u,0u);
        for (unsigned cell = 0u; cell < width; ++cell) {
            MR(F10E,F11B,origin + row * 0x40u + cell * 2u,0u);
            MB(F11C,cell + 1u != width);
        }
        MR(F11E,F127,0u,0u); MB(F129,row + 1u != rows);
    }
    MR(F12B,F12D,0u,0u);
    return true;
}
#undef MR
#undef MB
#undef MC
static uint64_t MotionBudget(const TableMotionClock *clock) {
    uint64_t budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    return budget + 40u * (budget / 1324u + 1u);
}
static bool PrepareTableMotionClock(TableMotionClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = table_motion_ends[clock->root];
    clock->returning = true;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x28000u || cpu->PB != 0x84u || pc >> 16 != 0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        cpu->host_return_valid != 2u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const bool entry = pc == table_motion_entries[clock->root];
    if (entry) {
        if (clock->root && cpu->x_flag) return false;
        if (clock->root != 0u && clock->root != 1u && clock->root != 5u &&
            clock->root != 8u && cpu->m_flag) return false;
    } else if (cpu->x_flag || (pc != 0x84edc8u && cpu->m_flag)) return false;
    bool planned;
    switch (clock->root) {
    case 0u: planned = PlanMovingColumns(clock, (uint16_t)pc); break;
    case 1u: planned = PlanRefreshColumns(clock, (uint16_t)pc); break;
    case 2u: planned = PlanDrawColumn(clock, (uint16_t)pc); break;
    case 3u: planned = PlanRoundReset(clock, (uint16_t)pc); break;
    case 4u: planned = PlanRestoreLines(clock, (uint16_t)pc); break;
    case 5u: planned = PlanPanelAttributes(clock, (uint16_t)pc); break;
    case 6u: planned = PlanActiveLines(clock, (uint16_t)pc); break;
    case 7u: planned = PlanAccentAttributes(clock, (uint16_t)pc); break;
    default: planned = PlanTileAttributes(clock, (uint16_t)pc); break;
    }
    if (!planned) return false;
    const uint64_t budget = MotionBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, table_motion_entries[clock->root],
            0x1ff7u, true, 1363u, budget)) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
static uint8_t TableMotionChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableMotionClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(!clock->returning && clock->next == site + 3u);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableMotionClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}
static RecompReturn ActorBridgeTableMotion(CpuState *cpu, unsigned root) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableMotionClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTableMotionClock(&clock, table_motion_entries[root]))
        return ActorBridgeFallback(cpu, &frame, table_motion_entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result = Lufia2CardTableAdvanceColumns(&memory, &state, TableMotionChild, &clock); break;
    case 1u: result = Lufia2CardTableRefreshColumns(&memory, &state, TableMotionChild, &clock); break;
    case 2u: result = Lufia2CardTableDrawColumn(&memory, &state); break;
    case 3u: result = Lufia2CardTableResetRoundBuffers(&memory, &state, TableMotionChild, &clock); break;
    case 4u: result = Lufia2CardTableRestoreLineAttributes(&memory, &state, TableMotionChild, &clock); break;
    case 5u: result = Lufia2CardTableAnimatePanelAttributes(&memory, &state); break;
    case 6u: result = Lufia2CardTableAnimateLineAttributes(&memory, &state, TableMotionChild, &clock); break;
    case 7u: result = Lufia2CardTableAnimateAccentAttributes(&memory, &state, TableMotionChild, &clock); break;
    default: result = Lufia2CardTablePaintTileAttributes(&memory, &state); break;
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
