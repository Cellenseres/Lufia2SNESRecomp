typedef struct TableLineOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index;
    int8_t advance_x, advance_y;
} TableLineOpcode;

enum {
    LINE_PC_E194 = 0u,
    LINE_PC_E197 = 1u,
    LINE_PC_E19A = 2u,
    LINE_PC_E19D = 3u,
    LINE_PC_E1A0 = 4u,
    LINE_PC_E1A3 = 5u,
    LINE_PC_E1A6 = 6u,
    LINE_PC_E1A9 = 7u,
    LINE_PC_E1AC = 8u,
    LINE_PC_E1AF = 9u,
    LINE_PC_E1B2 = 10u,
    LINE_PC_E1B4 = 11u,
    LINE_PC_E1B7 = 12u,
    LINE_PC_E1B9 = 13u,
    LINE_PC_E1BC = 14u,
    LINE_PC_E1BE = 15u,
    LINE_PC_E1C1 = 16u,
    LINE_PC_E1C4 = 17u,
    LINE_PC_E1C7 = 18u,
    LINE_PC_E1CA = 19u,
    LINE_PC_E1CC = 20u,
    LINE_PC_E1CF = 21u,
    LINE_PC_E1D2 = 22u,
    LINE_PC_E1D5 = 23u,
    LINE_PC_E1D8 = 24u,
    LINE_PC_E1DB = 25u,
    LINE_PC_E1DD = 26u,
    LINE_PC_E1E0 = 27u,
    LINE_PC_E1E2 = 28u,
    LINE_PC_E1E5 = 29u,
    LINE_PC_E1E7 = 30u,
    LINE_PC_E1EA = 31u,
    LINE_PC_E1EC = 32u,
    LINE_PC_E1EF = 33u,
    LINE_PC_E1F1 = 34u,
    LINE_PC_E1F3 = 35u,
    LINE_PC_E1F5 = 36u,
    LINE_PC_E1F8 = 37u,
    LINE_PC_E1FA = 38u,
    LINE_PC_E1FD = 39u,
    LINE_PC_E200 = 40u,
    LINE_PC_E202 = 41u,
    LINE_PC_E205 = 42u,
    LINE_PC_E207 = 43u,
    LINE_PC_E20A = 44u,
    LINE_PC_E20C = 45u,
    LINE_PC_E20F = 46u,
    LINE_PC_E211 = 47u,
    LINE_PC_E214 = 48u,
    LINE_PC_E216 = 49u,
    LINE_PC_E219 = 50u,
    LINE_PC_E21B = 51u,
    LINE_PC_E21E = 52u,
    LINE_PC_E221 = 53u,
    LINE_PC_E224 = 54u,
    LINE_PC_E227 = 55u,
    LINE_PC_E229 = 56u,
    LINE_PC_E22C = 57u,
    LINE_PC_E22E = 58u,
    LINE_PC_E231 = 59u,
    LINE_PC_E234 = 60u,
    LINE_PC_E237 = 61u,
    LINE_PC_E238 = 62u,
    LINE_PC_E23A = 63u,
    LINE_PC_E23D = 64u,
    LINE_PC_E23E = 65u,
    LINE_PC_E23F = 66u,
    LINE_PC_E240 = 67u,
    LINE_PC_E241 = 68u,
    LINE_PC_E242 = 69u,
    LINE_PC_E245 = 70u,
    LINE_PC_E246 = 71u,
    LINE_PC_E249 = 72u,
    LINE_PC_E24C = 73u,
    LINE_PC_E24E = 74u,
    LINE_PC_E251 = 75u,
    LINE_PC_E253 = 76u,
    LINE_PC_E254 = 77u,
    LINE_PC_E255 = 78u,
    LINE_PC_E258 = 79u,
    LINE_PC_E25A = 80u,
    LINE_PC_E25B = 81u,
    LINE_PC_E25C = 82u,
    LINE_PC_E25F = 83u,
    LINE_PC_E261 = 84u,
    LINE_PC_E264 = 85u,
    LINE_PC_E265 = 86u,
    LINE_PC_E267 = 87u,
    LINE_PC_E269 = 88u,
    LINE_PC_E26B = 89u,
    LINE_PC_E26D = 90u,
    LINE_PC_E270 = 91u,
    LINE_PC_E272 = 92u,
    LINE_PC_E275 = 93u,
    LINE_PC_E278 = 94u,
    LINE_PC_E27B = 95u,
    LINE_PC_E27D = 96u,
    LINE_PC_E280 = 97u,
    LINE_PC_E283 = 98u,
    LINE_PC_E286 = 99u,
    LINE_PC_E288 = 100u,
    LINE_PC_E28B = 101u,
    LINE_PC_E28D = 102u,
    LINE_PC_E290 = 103u,
    LINE_PC_E292 = 104u,
    LINE_PC_E295 = 105u,
    LINE_PC_E297 = 106u,
    LINE_PC_E29A = 107u,
    LINE_PC_E29D = 108u,
    LINE_PC_E29F = 109u,
    LINE_PC_E2A2 = 110u,
    LINE_PC_E2A4 = 111u,
    LINE_PC_E2A7 = 112u,
    LINE_PC_E2A8 = 113u,
    LINE_PC_E2AB = 114u,
    LINE_PC_E2AD = 115u,
    LINE_PC_E2AE = 116u,
    LINE_PC_E2B1 = 117u,
    LINE_PC_E2B4 = 118u,
    LINE_PC_E2B7 = 119u,
    LINE_PC_E2BA = 120u,
    LINE_PC_E2BD = 121u,
    LINE_PC_E2C0 = 122u,
    LINE_PC_E2C3 = 123u,
    LINE_PC_E2C6 = 124u,
    LINE_PC_E2C9 = 125u,
    LINE_PC_E2CC = 126u,
    LINE_PC_E2CE = 127u,
    LINE_PC_E2D1 = 128u,
    LINE_PC_E2D2 = 129u,
    LINE_PC_E2D5 = 130u,
    LINE_PC_E2D7 = 131u,
    LINE_PC_E2DA = 132u,
    LINE_PC_E2DB = 133u,
    LINE_PC_E2DE = 134u,
    LINE_PC_E2E1 = 135u,
    LINE_PC_E2E3 = 136u,
    LINE_PC_E2E6 = 137u,
    LINE_PC_E2E9 = 138u,
    LINE_PC_E2EA = 139u,
    LINE_PC_E2ED = 140u,
    LINE_PC_E2EE = 141u,
    LINE_PC_E2F1 = 142u,
    LINE_PC_E2F4 = 143u,
    LINE_PC_E2F6 = 144u,
    LINE_PC_E2F8 = 145u,
    LINE_PC_E2FB = 146u,
    LINE_PC_E2FD = 147u,
    LINE_PC_E2FF = 148u,
    LINE_PC_E301 = 149u,
    LINE_PC_E304 = 150u,
    LINE_PC_E307 = 151u,
    LINE_PC_E30B = 152u,
    LINE_PC_E30E = 153u,
    LINE_PC_E30F = 154u,
    LINE_PC_E312 = 155u,
    LINE_PC_E315 = 156u,
    LINE_PC_E318 = 157u,
    LINE_PC_E31A = 158u,
    LINE_PC_E31D = 159u,
    LINE_PC_E31F = 160u,
    LINE_PC_E320 = 161u,
    LINE_PC_E321 = 162u,
    LINE_PC_E324 = 163u,
    LINE_PC_E326 = 164u,
    LINE_PC_E327 = 165u,
    LINE_PC_E328 = 166u,
    LINE_PC_E32B = 167u,
    LINE_PC_E32D = 168u,
    LINE_PC_E330 = 169u,
    LINE_PC_E331 = 170u,
    LINE_PC_E333 = 171u,
    LINE_PC_EB36 = 172u,
    LINE_PC_EB37 = 173u,
    LINE_PC_EB39 = 174u,
    LINE_PC_EB3C = 175u,
    LINE_PC_EB3F = 176u,
    LINE_PC_EB40 = 177u,
    LINE_PC_EB41 = 178u,
    LINE_PC_EB42 = 179u,
    LINE_PC_EB43 = 180u,
    LINE_PC_EB44 = 181u,
    LINE_PC_EB45 = 182u,
    LINE_PC_EB46 = 183u,
    LINE_PC_EB49 = 184u,
    LINE_PC_EB4C = 185u,
    LINE_PC_EB4D = 186u,
    LINE_PC_EB4E = 187u,
    LINE_PC_EB4F = 188u,
    LINE_PC_EB50 = 189u,
    LINE_PC_EB53 = 190u,
    LINE_PC_EB54 = 191u,
    LINE_PC_EB57 = 192u,
    LINE_PC_EB5A = 193u,
    LINE_PC_EB5D = 194u,
    LINE_PC_EB60 = 195u,
    LINE_PC_EB61 = 196u,
    LINE_PC_EB62 = 197u,
    LINE_PC_EB63 = 198u,
    LINE_PC_EB65 = 199u,
    LINE_PC_EB68 = 200u,
    LINE_PC_EB6B = 201u,
    LINE_PC_EB6E = 202u,
    LINE_PC_EB6F = 203u,
    LINE_PC_EB70 = 204u,
    LINE_PC_EB71 = 205u,
    LINE_PC_EB75 = 206u,
    LINE_PC_EB78 = 207u,
    LINE_PC_EB7B = 208u,
    LINE_PC_EB7D = 209u,
    LINE_PC_EB80 = 210u,
    LINE_PC_EB83 = 211u,
    LINE_PC_EB86 = 212u,
    LINE_PC_EB89 = 213u,
    LINE_PC_EB8C = 214u,
    LINE_PC_EB8F = 215u,
    LINE_PC_EB90 = 216u,
    LINE_PC_EB92 = 217u,
    LINE_PC_EB93 = 218u,
    LINE_PC_EB96 = 219u,
    LINE_PC_EB98 = 220u,
    LINE_PC_EB99 = 221u,
    LINE_PC_EB9A = 222u,
    LINE_PC_EB9B = 223u,
    LINE_PC_EB9F = 224u,
    LINE_PC_EBA2 = 225u,
    LINE_PC_EBA5 = 226u,
    LINE_PC_EBA7 = 227u,
    LINE_PC_EBAA = 228u,
    LINE_PC_EBAD = 229u,
    LINE_PC_EBAE = 230u,
    LINE_PC_EBB1 = 231u,
    LINE_PC_EBB4 = 232u,
    LINE_PC_EBB7 = 233u,
    LINE_PC_EBB8 = 234u,
    LINE_PC_EBBB = 235u,
    LINE_PC_EBBE = 236u,
    LINE_PC_EBBF = 237u,
    LINE_PC_EBC0 = 238u,
    LINE_PC_EBC4 = 239u,
    LINE_PC_EBC6 = 240u,
    LINE_PC_EBC8 = 241u,
    LINE_PC_EBCC = 242u,
    LINE_PC_EBCD = 243u,
    LINE_PC_EBCE = 244u,
    LINE_PC_EBCF = 245u,
    LINE_PC_EBD0 = 246u,
    LINE_PC_EBD1 = 247u,
    LINE_PC_EBD2 = 248u,
    LINE_PC_EBD3 = 249u,
    LINE_PC_EBD4 = 250u,
    LINE_PC_EBD6 = 251u,
    LINE_PC_EBDA = 252u,
    LINE_PC_EBDB = 253u,
    LINE_PC_EBDE = 254u,
    LINE_PC_EBE1 = 255u,
    LINE_PC_EBE3 = 256u,
    LINE_PC_EBE6 = 257u,
    LINE_PC_EBE9 = 258u,
    LINE_PC_EBEC = 259u,
    LINE_PC_EBEF = 260u,
    LINE_PC_EBF1 = 261u,
    LINE_PC_EBF4 = 262u,
    LINE_PC_EBF7 = 263u,
    LINE_PC_EBFA = 264u,
    LINE_PC_EBFD = 265u,
    LINE_PC_EBFF = 266u,
    LINE_PC_EC02 = 267u,
    LINE_PC_EC05 = 268u,
    LINE_PC_EC08 = 269u,
    LINE_PC_EC0B = 270u,
    LINE_PC_EC0C = 271u,
    LINE_PC_EC0E = 272u,
    LINE_PC_EC11 = 273u,
    LINE_PC_EC14 = 274u,
    LINE_PC_EC17 = 275u,
    LINE_PC_EC18 = 276u,
    LINE_PC_EC19 = 277u,
    LINE_PC_EC1A = 278u,
    LINE_PC_EC1D = 279u,
    LINE_PC_EC1F = 280u,
    LINE_PC_EC22 = 281u,
    LINE_PC_EC23 = 282u,
    LINE_PC_EC26 = 283u,
    LINE_PC_EC29 = 284u,
    LINE_PC_EC2C = 285u,
    LINE_PC_EC2D = 286u,
    LINE_PC_EC30 = 287u,
    LINE_PC_EC33 = 288u,
    LINE_PC_EC34 = 289u,
    LINE_PC_EC37 = 290u,
    LINE_PC_EC39 = 291u,
    LINE_PC_EC3C = 292u,
    LINE_PC_EC3D = 293u,
    LINE_PC_EC40 = 294u,
    LINE_PC_EC43 = 295u,
    LINE_PC_EC46 = 296u,
    LINE_PC_EC47 = 297u,
    LINE_PC_EC4A = 298u,
    LINE_PC_EC4D = 299u,
    LINE_PC_EC4E = 300u,
    LINE_PC_DC55 = 301u,
    LINE_PC_DC59 = 302u,
    LINE_PC_DC5C = 303u,
    LINE_PC_DC5F = 304u,
};

static const TableLineOpcode table_line_opcodes[] = {
    {{0xe194u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe197u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe19au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe19du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1a0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1a3u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1a6u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1a9u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1acu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1afu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1b2u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1b4u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1b7u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1b9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1bcu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1beu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1c1u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1c4u, 3u, 4u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1c7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1cau, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1ccu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1cfu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1d2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1d5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1d8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1dbu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1ddu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1e0u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1e2u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1e5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1e7u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1eau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1ecu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1efu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1f1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1f3u, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe1f5u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe1f8u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe1fau, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe1fdu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe200u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe202u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe205u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe207u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe20au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe20cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe20fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe211u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe214u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe216u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe219u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe21bu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe21eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe221u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe224u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe227u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe229u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe22cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe22eu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe231u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe234u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe237u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe238u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe23au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe23du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe23eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe23fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe240u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe241u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe242u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe245u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe246u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe249u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe24cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe24eu, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xe251u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe253u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe254u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe255u, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xe258u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe25au, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe25bu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe25cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe25fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe261u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe264u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe265u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe267u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe269u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe26bu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe26du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe270u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe272u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe275u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe278u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe27bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe27du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe280u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe283u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe286u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe288u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe28bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe28du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe290u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe292u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe295u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe297u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe29au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe29du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe29fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2a2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2a4u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2a7u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2a8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2abu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2adu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2aeu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2b1u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2b4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2b7u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2bau, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2bdu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2c0u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2c3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2c6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2c9u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2ccu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2ceu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2d1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2d2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2d5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2d7u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2dau, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2dbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2deu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2e1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2e3u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2e6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2e9u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2eau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2edu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2eeu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2f1u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe2f4u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2f6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2f8u, 3u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe2fbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe2fdu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe2ffu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe301u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe304u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe307u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe30bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe30eu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe30fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe312u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe315u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe318u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe31au, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xe31du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe31fu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe320u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe321u, 3u, 5u, 2u}, 0x7104u, 1u, 0, 0},
    {{0xe324u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe326u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe327u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe328u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe32bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe32du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe330u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe331u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe333u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb36u, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xeb37u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb39u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb3cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb3fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb40u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb41u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb42u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb43u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb44u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb45u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb46u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb49u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb4cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb4du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb4eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb4fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb50u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb53u, 1u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb54u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb57u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb5au, 3u, 5u, 2u}, 0x0100u, 1u, 0, 0},
    {{0xeb5du, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb60u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeb61u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xeb62u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xeb63u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb65u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb68u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb6bu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb6eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb6fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb70u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb71u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb75u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb78u, 3u, 5u, 2u}, 0x0102u, 1u, 0, 0},
    {{0xeb7bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb7du, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb80u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb83u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb86u, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xeb89u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb8cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb8fu, 1u, 2u, 0u}, 0x0000u, 0u, -1, 0},
    {{0xeb90u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb92u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb93u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb96u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb98u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb99u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb9au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeb9bu, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeb9fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xeba2u, 3u, 5u, 2u}, 0x0106u, 1u, 0, 0},
    {{0xeba5u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xeba7u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebaau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebadu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebaeu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebb1u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebb4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebb7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebb8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebbbu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebbeu, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xebbfu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebc0u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebc4u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebc6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebc8u, 4u, 5u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xebccu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebcdu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebceu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebcfu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebd0u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebd1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebd2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebd3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebd4u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebd6u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebdau, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebdbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebdeu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebe1u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebe3u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebe6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebe9u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebecu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebefu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebf1u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebf4u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebf7u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebfau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xebfdu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xebffu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec02u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec05u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec08u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec0bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xec0cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec0eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec11u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec14u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec17u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec18u, 1u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec19u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec1au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec1du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec1fu, 3u, 5u, 2u}, 0x0100u, 1u, 0, 0},
    {{0xec22u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec23u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec26u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec29u, 3u, 5u, 2u}, 0x0108u, 1u, 0, 0},
    {{0xec2cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec2du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec30u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec33u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec34u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec37u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec39u, 3u, 5u, 2u}, 0x0104u, 1u, 0, 0},
    {{0xec3cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec3du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec40u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec43u, 3u, 5u, 2u}, 0x010cu, 1u, 0, 0},
    {{0xec46u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec47u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xec4au, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec4du, 1u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xec4eu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xdc55u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xdc59u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc5cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xdc5fu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
};

enum { TABLE_LINE_CAPACITY = 512u };
typedef struct TableLineClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_LINE_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint32_t next;
    bool returning;
} TableLineClock;
static const uint32_t table_line_entries[] = {0x84e194u,0x84e2a8u,0x84e30fu,0x84eb36u,0x84ebc0u,0x84ebdbu,0x84ec18u,0x84dc55u};
static const uint32_t table_line_ends[] = {0x84e2a7u,0x84e30eu,0x84e333u,0x84ebbfu,0x84ebdau,0x84ec17u,0x84ec4eu,0x84dc5fu};
static uint16_t TableLineDirectWord(const CpuState *cpu, unsigned at) {
    return cpu->ram[at] | (uint16_t)cpu->ram[at + 1u] << 8;
}
static uint16_t TableLineBankWord(const CpuState *cpu, uint16_t at) {
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    return cpu->ram[bank + at] | (uint16_t)cpu->ram[bank + (uint16_t)(at + 1u)] << 8;
}
static bool TableLineAppendOpcode(TableLineClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == TABLE_LINE_CAPACITY) return false;
    clock->timing[clock->span.count++] = op;
    return true;
}
static bool TableLineRange(TableLineClock *clock, unsigned first, unsigned last,
                         uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableLineOpcode *record = table_line_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!TableLineAppendOpcode(clock, op)) return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}
static bool TableLineBranch(TableLineClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_line_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return TableLineAppendOpcode(clock, op);
}
#define LR(first,last,x,y) do { if (!TableLineRange(clock, LINE_PC_##first, LINE_PC_##last, (uint16_t)(x), (uint16_t)(y))) return false; } while (0)
#define LB(pc,taken) do { if (!TableLineBranch(clock, LINE_PC_##pc, taken)) return false; } while (0)
#define LC(site) do { LR(site,site,0u,0u); clock->next = 0x840000u | (table_line_opcodes[LINE_PC_##site].timing.pc + table_line_opcodes[LINE_PC_##site].timing.fetched); clock->returning = false; return true; } while (0)


static bool PlanTableChoicePoll(TableLineClock *clock, bool additional) {
    if (additional) { LC(E21E); }
    LC(E1A6);
}
static bool PlanTableChoiceFinish(TableLineClock *clock) {
    LR(E27D,E27D,0u,0u); LC(E280);
}
static bool PlanTableChoiceRestore(TableLineClock *clock) {
    LR(E283,E295,0u,0u); LC(E297);
}
static bool PlanTableLineChoice(TableLineClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    switch (pc) {
    case 0xe194u: LR(E194,E1A3,0u,0u); return PlanTableChoicePoll(clock, false);
    case 0xe1a9u: LC(E1A9);
    case 0xe1acu: LC(E1AC);
    case 0xe1afu:
        LR(E1AF,E1AF,0u,0u); LB(E1B2,cpu->A != 7u);
        if (cpu->A == 7u) { LC(E1B4); }
        LR(E1B9,E1B9,0u,0u); LB(E1BC,cpu->A != 2u);
        if (cpu->A == 2u) { LR(E1BE,E1BE,0u,0u); LC(E1C1); }
        LR(E1C7,E1C7,0u,0u); LB(E1CA,(cpu->A & 0x100u) == 0u);
        if (!(cpu->A & 0x100u)) return PlanTableChoicePoll(clock, false);
        LR(E1CC,E1CC,0u,0u); LC(E1CF);
    case 0xe1b7u: LR(E1B7,E1B7,0u,0u); return PlanTableChoicePoll(clock, false);
    case 0xe1c4u: LR(E1C4,E1C4,0u,0u); return PlanTableChoiceRestore(clock);
    case 0xe1d2u: LC(E1D2);
    case 0xe1d5u: LC(E1D5);
    case 0xe1d8u: LR(E1D8,E1EA,0u,0u); LC(E1EC);
    case 0xe1efu: LR(E1EF,E1F3,0u,0u); LC(E1F5);
    case 0xe1f8u: {
        LR(E1F8,E1FD,0u,0u);
        const unsigned variant = TableLineBankWord(cpu, 0x7170u);
        LB(E200,variant == 0u);
        if (variant) {
            LR(E202,E202,0u,0u); LB(E205,(variant & 9u) == 0u);
            if (!(variant & 9u)) return PlanTableChoiceFinish(clock);
        }
        LR(E207,E219,0u,0u); LC(E21B);
    }
    case 0xe21eu: LC(E21E);
    case 0xe221u: LC(E221);
    case 0xe224u:
        LR(E224,E224,0u,0u); LB(E227,cpu->A == 1u);
        if (cpu->A == 1u) return PlanTableChoiceFinish(clock);
        LR(E229,E229,0u,0u); LB(E22C,(cpu->A & 0x100u) == 0u);
        if (!(cpu->A & 0x100u)) return PlanTableChoicePoll(clock, true);
        LR(E22E,E22E,0u,0u); LC(E231);
    case 0xe234u: LC(E234);
    case 0xe237u: {
        LR(E237,E237,0u,0u); LB(E238,((uint16_t)(cpu->A - 1u) & 0x8000u) != 0u);
        if ((uint16_t)(cpu->A - 1u) & 0x8000u) return PlanTableChoicePoll(clock, true);
        const unsigned selected = TableLineBankWord(cpu, 0x7184u);
        if (!selected || selected > 2u) return false;
        LR(E23A,E24C,0u,0u);
        LR(E24E,E25F,(selected - 1u) * 8u + 4u,0u); LC(E261);
    }
    case 0xe264u:
        if (!cpu->Y || cpu->Y > 2u || cpu->X > 20u || (cpu->X & 3u)) return false;
        LR(E264,E264,0u,0u); LB(E265,cpu->Y != 1u);
        if (cpu->Y != 1u) { LR(E24E,E25F,cpu->X,0u); LC(E261); }
        LR(E267,E26B,0u,0u); LC(E26D);
    case 0xe270u: {
        const unsigned selected = TableLineBankWord(cpu, 0x7184u);
        LR(E270,E278,0u,0u); LB(E27B,(uint16_t)(selected + 1u) != 3u);
        return (uint16_t)(selected + 1u) == 3u ?
            PlanTableChoiceFinish(clock) : PlanTableChoicePoll(clock, true);
    }
    case 0xe283u: return PlanTableChoiceRestore(clock);
    case 0xe29au: LR(E29A,E2A2,0u,0u); LC(E2A4);
    case 0xe2a7u: return true;
    default: return false;
    }
}
static bool PlanTableCredit(TableLineClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe2ccu) { LR(E2CC,E2CC,0u,0u); LR(E30B,E30B,0u,0u); return true; }
    if (pc == 0xe30bu) { LR(E30B,E30B,0u,0u); return true; }
    if (pc == 0xe2bdu) { LR(E2BD,E2C6,0u,0u); LC(E2C9); }
    if (pc != 0xe2a8u) return false;
    LR(E2A8,E2A8,0u,0u);
    const bool currency = TableLineBankWord(cpu, 0x71aeu) == 0u;
    LB(E2AB,currency);
    if (!currency) { LR(E2AD,E2B7,0u,0u); LC(E2BA); }
    LR(E2CE,E2D2,0u,0u);
    const bool enough = TableLineBankWord(cpu, 0x0b55u) >= 10u;
    LB(E2D5,enough);
    if (!enough) {
        LR(E2D7,E2DE,0u,0u);
        const bool borrow = TableLineBankWord(cpu, 0x0b57u) != 0u;
        LB(E2E1,borrow);
        if (!borrow) {
            LR(E2E3,E2E6,0u,0u);
            clock->next = 0x84e2e9u;
            return true;
        }
    }
    LR(E2EA,E304,0u,0u); LC(E307);
}
static bool PlanTableExtraLines(TableLineClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe30fu) { LR(E30F,E318,0u,0u); LR(E31A,E32B,20u,0u); LC(E32D); }
    if (pc != 0xe330u || !cpu->Y || cpu->Y > 3u || cpu->X > 32u || (cpu->X & 3u)) return false;
    LR(E330,E330,0u,0u); LB(E331,cpu->Y != 1u);
    if (cpu->Y != 1u) { LR(E31A,E32B,cpu->X,0u); LC(E32D); }
    return true;
}
static uint16_t TableLineTile(unsigned digit) {
    const unsigned offset = 0x272dcu + digit * 2u;
    return g_snes->cart->rom[offset] | (uint16_t)g_snes->cart->rom[offset + 1u] << 8;
}
static bool PlanTableNumber(TableLineClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xeb36u) {
        const unsigned slot = TableLineBankWord(cpu, 0x7196u);
        if (slot > 2u || TableLineBankWord(cpu, 0x7194u) > 99u) return false;
        LR(EB36,EB57,0u,0u);
        for (unsigned word = 0u; word < 4u; ++word) {
            LR(EB5A,EB62,0x120u + slot * 16u + word * 2u,0u); LB(EB63,word != 3u);
        }
        LR(EB65,EB65,0u,0u); LC(EB68);
    }
    if (pc != 0xeb6bu) return false;
    const unsigned value = TableLineBankWord(cpu, 0x7194u), quotient = cpu->A;
    const unsigned offset = TableLineBankWord(cpu, 0x719au);
    if (value > 99u || quotient != value / 10u ||
        offset < 0x120u || offset > 0x140u || (offset & 15u)) return false;
    LR(EB6B,EB78,offset,0u);
    const bool same_tens = TableLineTile(quotient) == TableLineBankWord(cpu, (uint16_t)(0x102u + offset));
    LB(EB7B,same_tens);
    if (!same_tens) { LR(EB7D,EB86,offset,0u); }
    LR(EB89,EB8C,0u,0u);
    for (unsigned count = quotient;; --count) {
        LR(EB8F,EB8F,0u,0u); LB(EB90,count == 0u);
        if (!count) break;
        LR(EB92,EB96,0u,0u);
    }
    LR(EB98,EBA2,offset,0u);
    const bool same_units = TableLineTile(value - quotient * 10u) ==
        TableLineBankWord(cpu, (uint16_t)(0x106u + offset));
    LB(EBA5,same_units);
    if (!same_units) { LR(EBA7,EBBB,offset,0u); }
    LR(EBBE,EBBE,0u,0u);
    return true;
}
static bool PlanTableDivision(TableLineClock *clock, uint16_t pc) {
    if (pc != 0xebc0u) return false;
    LR(EBC0,EBD6,0u,0u);
    return true;
}
static bool PlanTablePendingNumbers(TableLineClock *clock, unsigned first) {
    const CpuState *cpu = clock->call.cpu;
    for (unsigned slot = first; slot < 3u; ++slot) {
        const bool pending = TableLineBankWord(cpu, (uint16_t)(0x71a0u + slot * 2u)) != 0u;
        switch (slot) {
        case 0u:
            LR(EBDE,EBDE,0u,0u); LB(EBE1,!pending);
            if (pending) { LR(EBE3,EBE6,0u,0u); LC(EBE9); }
            break;
        case 1u:
            LR(EBEC,EBEC,0u,0u); LB(EBEF,!pending);
            if (pending) { LR(EBF1,EBF4,0u,0u); LC(EBF7); }
            break;
        default:
            LR(EBFA,EBFA,0u,0u); LB(EBFD,!pending);
            if (pending) { LR(EBFF,EC02,0u,0u); LC(EC05); }
            break;
        }
    }
    LC(EC08);
}
static bool PlanTableNumberAnimation(TableLineClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    switch (pc) {
    case 0xebdbu: LR(EBDB,EBDB,0u,0u); return PlanTablePendingNumbers(clock, 0u);
    case 0xebecu: return PlanTablePendingNumbers(clock, 1u);
    case 0xebfau: return PlanTablePendingNumbers(clock, 2u);
    case 0xec08u: LC(EC08);
    case 0xec0bu:
        if (!cpu->Y || cpu->Y > 4u) return false;
        LR(EC0B,EC0B,0u,0u); LB(EC0C,cpu->Y != 1u);
        if (cpu->Y != 1u) return PlanTablePendingNumbers(clock, 0u);
        LR(EC0E,EC14,0u,0u);
        return true;
    default: return false;
    }
}
static bool PlanTableDigitMotion(TableLineClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc != 0xec18u || cpu->X < 0x120u || cpu->X > 0x140u || (cpu->X & 15u)) return false;
    LR(EC18,EC1A,0u,0u); LB(EC1D,(cpu->A & 1u) == 0u);
    if (cpu->A & 1u) { LR(EC1F,EC30,cpu->X,0u); }
    LR(EC33,EC34,0u,0u); LB(EC37,(cpu->A & 2u) == 0u);
    if (cpu->A & 2u) { LR(EC39,EC4A,cpu->X,0u); }
    LR(EC4D,EC4D,0u,0u);
    return true;
}

static bool PlanTableInputCommand(TableLineClock *clock, uint16_t pc) {
    switch (pc) {
    case 0xdc55u: LC(DC55);
    case 0xdc59u: LC(DC59);
    case 0xdc5cu: LC(DC5C);
    case 0xdc5fu: return true;
    default: return false;
    }
}

#undef LR
#undef LB
#undef LC
static uint64_t TableLineBudget(const TableLineClock *clock) {
    uint64_t budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    return budget + 40u * (budget / 1324u + 1u);
}
static bool PrepareTableLineClock(TableLineClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = table_line_ends[clock->root];
    clock->returning = true;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x28000u || cpu->PB != 0x84u || pc >> 16 != 0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        clock->call.frame.hrv != 2u || cpu->host_return_valid < 2u || cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const bool entry = pc == table_line_entries[clock->root];
    if ((entry && cpu->host_return_valid != 2u) || cpu->x_flag || (entry && cpu->m_flag && clock->root != 3u)) return false;
    if (!entry && cpu->m_flag && pc != 0x84e1f8u && pc != 0x84e270u) return false;
    bool planned;
    switch (clock->root) {
    case 0u: planned = PlanTableLineChoice(clock, (uint16_t)pc); break;
    case 1u: planned = PlanTableCredit(clock, (uint16_t)pc); break;
    case 2u: planned = PlanTableExtraLines(clock, (uint16_t)pc); break;
    case 3u: planned = PlanTableNumber(clock, (uint16_t)pc); break;
    case 4u: planned = PlanTableDivision(clock, (uint16_t)pc); break;
    case 5u: planned = PlanTableNumberAnimation(clock, (uint16_t)pc); break;
    case 6u: planned = PlanTableDigitMotion(clock, (uint16_t)pc); break;
    default: planned = PlanTableInputCommand(clock, (uint16_t)pc); break;
    }
    if (!planned) return false;
    const uint64_t budget = TableLineBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, table_line_entries[clock->root],
            0x1ff7u, true, 1363u, budget)) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
static uint8_t TableLineChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableLineClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(!clock->returning && clock->next == site + frame_size + 1u);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableLineClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}
static RecompReturn ActorBridgeTableLine(CpuState *cpu, unsigned root) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableLineClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTableLineClock(&clock, table_line_entries[root]))
        return ActorBridgeFallback(cpu, &frame, table_line_entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result = Lufia2CardTableChooseLines(&memory, &state, TableLineChild, &clock); break;
    case 1u: result = Lufia2CardTableSpendCredit(&memory, &state, TableLineChild, &clock); break;
    case 2u: result = Lufia2CardTableMarkExtraLines(&memory, &state, TableLineChild, &clock); break;
    case 3u: result = Lufia2CardTableDrawNumber(&memory, &state, TableLineChild, &clock); break;
    case 4u: result = Lufia2CardTableDivideNumberByTen(&memory, &state); break;
    case 5u: result = Lufia2CardTableAnimateNumbers(&memory, &state, TableLineChild, &clock); break;
    case 6u: result = Lufia2CardTableAdvanceNumberSprites(&memory, &state); break;
    default: result = Lufia2CardTableReadCommand(&memory, &state, TableLineChild, &clock); break;
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
