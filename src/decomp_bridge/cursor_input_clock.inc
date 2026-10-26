extern uint64_t g_interp_bridge_write_epoch;

typedef struct CursorInputOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index, advance_y;
} CursorInputOpcode;

enum {
    CURSOR_PC_A112 = 0u,
    CURSOR_PC_A113 = 1u,
    CURSOR_PC_A115 = 2u,
    CURSOR_PC_A118 = 3u,
    CURSOR_PC_A11A = 4u,
    CURSOR_PC_A11C = 5u,
    CURSOR_PC_A11D = 6u,
    CURSOR_PC_A11E = 7u,
    CURSOR_PC_A11F = 8u,
    CURSOR_PC_A122 = 9u,
    CURSOR_PC_A124 = 10u,
    CURSOR_PC_A127 = 11u,
    CURSOR_PC_A129 = 12u,
    CURSOR_PC_A12A = 13u,
    CURSOR_PC_A12B = 14u,
    CURSOR_PC_A12D = 15u,
    CURSOR_PC_A130 = 16u,
    CURSOR_PC_A131 = 17u,
    CURSOR_PC_A133 = 18u,
    CURSOR_PC_A136 = 19u,
    CURSOR_PC_A138 = 20u,
    CURSOR_PC_A13B = 21u,
    CURSOR_PC_A13E = 22u,
    CURSOR_PC_A13F = 23u,
    CURSOR_PC_A142 = 24u,
    CURSOR_PC_A145 = 25u,
    CURSOR_PC_A148 = 26u,
    CURSOR_PC_A149 = 27u,
    CURSOR_PC_A14C = 28u,
    CURSOR_PC_A14F = 29u,
    CURSOR_PC_A151 = 30u,
    CURSOR_PC_A154 = 31u,
    CURSOR_PC_A157 = 32u,
    CURSOR_PC_A15A = 33u,
    CURSOR_PC_A15D = 34u,
    CURSOR_PC_A160 = 35u,
    CURSOR_PC_A161 = 36u,
    CURSOR_PC_A163 = 37u,
    CURSOR_PC_A166 = 38u,
    CURSOR_PC_A167 = 39u,
    CURSOR_PC_A16A = 40u,
    CURSOR_PC_A16C = 41u,
    CURSOR_PC_A16F = 42u,
    CURSOR_PC_A171 = 43u,
    CURSOR_PC_A174 = 44u,
    CURSOR_PC_A177 = 45u,
    CURSOR_PC_A17A = 46u,
    CURSOR_PC_A17D = 47u,
    CURSOR_PC_A17E = 48u,
    CURSOR_PC_A181 = 49u,
    CURSOR_PC_A183 = 50u,
    CURSOR_PC_A186 = 51u,
    CURSOR_PC_A189 = 52u,
    CURSOR_PC_A18C = 53u,
    CURSOR_PC_A18F = 54u,
    CURSOR_PC_A192 = 55u,
    CURSOR_PC_A193 = 56u,
    CURSOR_PC_A196 = 57u,
    CURSOR_PC_A199 = 58u,
    CURSOR_PC_A19A = 59u,
    CURSOR_PC_A19C = 60u,
    CURSOR_PC_A19F = 61u,
    CURSOR_PC_A1A0 = 62u,
    CURSOR_PC_A1A3 = 63u,
    CURSOR_PC_A1A5 = 64u,
    CURSOR_PC_A1A8 = 65u,
    CURSOR_PC_A1AA = 66u,
    CURSOR_PC_A1AD = 67u,
    CURSOR_PC_A1AF = 68u,
    CURSOR_PC_A1B2 = 69u,
    CURSOR_PC_A1B5 = 70u,
    CURSOR_PC_A1B6 = 71u,
    CURSOR_PC_A1B9 = 72u,
    CURSOR_PC_A1BB = 73u,
    CURSOR_PC_A1BE = 74u,
    CURSOR_PC_A1C1 = 75u,
    CURSOR_PC_A1C4 = 76u,
    CURSOR_PC_A1C7 = 77u,
    CURSOR_PC_A1CA = 78u,
    CURSOR_PC_A1CB = 79u,
    CURSOR_PC_A1CD = 80u,
    CURSOR_PC_A1CE = 81u,
    CURSOR_PC_A1D0 = 82u,
    CURSOR_PC_A1D3 = 83u,
    CURSOR_PC_A1D4 = 84u,
    CURSOR_PC_A1D6 = 85u,
    CURSOR_PC_A1D9 = 86u,
    CURSOR_PC_A1DB = 87u,
    CURSOR_PC_A1DE = 88u,
    CURSOR_PC_A1E1 = 89u,
    CURSOR_PC_A1E2 = 90u,
    CURSOR_PC_A1E4 = 91u,
    CURSOR_PC_A1E7 = 92u,
    CURSOR_PC_A1E8 = 93u,
    CURSOR_PC_A1EB = 94u,
    CURSOR_PC_A1EE = 95u,
    CURSOR_PC_A1F0 = 96u,
    CURSOR_PC_A1F3 = 97u,
    CURSOR_PC_A1F6 = 98u,
    CURSOR_PC_A1F9 = 99u,
    CURSOR_PC_A1FC = 100u,
    CURSOR_PC_A1FE = 101u,
    CURSOR_PC_A200 = 102u,
    CURSOR_PC_A203 = 103u,
    CURSOR_PC_A205 = 104u,
    CURSOR_PC_A208 = 105u,
    CURSOR_PC_A20B = 106u,
    CURSOR_PC_A20E = 107u,
    CURSOR_PC_A211 = 108u,
    CURSOR_PC_A214 = 109u,
    CURSOR_PC_A216 = 110u,
    CURSOR_PC_A219 = 111u,
    CURSOR_PC_A21C = 112u,
    CURSOR_PC_A21F = 113u,
    CURSOR_PC_A222 = 114u,
    CURSOR_PC_A224 = 115u,
    CURSOR_PC_A225 = 116u,
    CURSOR_PC_A226 = 117u,
    CURSOR_PC_A227 = 118u,
    CURSOR_PC_A229 = 119u,
    CURSOR_PC_A22A = 120u,
    CURSOR_PC_A22B = 121u,
    CURSOR_PC_A22D = 122u,
    CURSOR_PC_A22E = 123u,
    CURSOR_PC_A231 = 124u,
    CURSOR_PC_A234 = 125u,
    CURSOR_PC_A236 = 126u,
    CURSOR_PC_A238 = 127u,
    CURSOR_PC_A23B = 128u,
    CURSOR_PC_A23C = 129u,
    CURSOR_PC_A23E = 130u,
    CURSOR_PC_A241 = 131u,
    CURSOR_PC_A242 = 132u,
    CURSOR_PC_A245 = 133u,
    CURSOR_PC_A246 = 134u,
    CURSOR_PC_A248 = 135u,
    CURSOR_PC_A24B = 136u,
    CURSOR_PC_A24C = 137u,
    CURSOR_PC_A24D = 138u,
    CURSOR_PC_A24E = 139u,
    CURSOR_PC_A250 = 140u,
    CURSOR_PC_A252 = 141u,
    CURSOR_PC_A254 = 142u,
    CURSOR_PC_A257 = 143u,
    CURSOR_PC_A259 = 144u,
    CURSOR_PC_A25C = 145u,
    CURSOR_PC_A25D = 146u,
    CURSOR_PC_A25F = 147u,
    CURSOR_PC_A262 = 148u,
    CURSOR_PC_A264 = 149u,
    CURSOR_PC_A267 = 150u,
    CURSOR_PC_A26A = 151u,
    CURSOR_PC_A26D = 152u,
    CURSOR_PC_A26F = 153u,
    CURSOR_PC_A272 = 154u,
    CURSOR_PC_A274 = 155u,
    CURSOR_PC_A277 = 156u,
    CURSOR_PC_A27A = 157u,
    CURSOR_PC_A27C = 158u,
    CURSOR_PC_A27E = 159u,
    CURSOR_PC_A280 = 160u,
    CURSOR_PC_A282 = 161u,
    CURSOR_PC_A285 = 162u,
    CURSOR_PC_A288 = 163u,
    CURSOR_PC_A28B = 164u,
    CURSOR_PC_A28C = 165u,
    CURSOR_PC_A28D = 166u,
    CURSOR_PC_A28E = 167u,
    CURSOR_PC_A28F = 168u,
    CURSOR_PC_A291 = 169u,
    CURSOR_PC_A294 = 170u,
    CURSOR_PC_A295 = 171u,
    CURSOR_PC_A296 = 172u,
    CURSOR_PC_A297 = 173u,
    CURSOR_PC_A298 = 174u,
    CURSOR_PC_A299 = 175u,
    CURSOR_PC_A29A = 176u,
    CURSOR_PC_A29C = 177u,
    CURSOR_PC_A29E = 178u,
    CURSOR_PC_A29F = 179u,
    CURSOR_PC_A2A0 = 180u,
    CURSOR_PC_A2A3 = 181u,
    CURSOR_PC_A2A5 = 182u,
    CURSOR_PC_A2A7 = 183u,
    CURSOR_PC_A2A9 = 184u,
    CURSOR_PC_A2AC = 185u,
    CURSOR_PC_A2AE = 186u,
    CURSOR_PC_A2B1 = 187u,
    CURSOR_PC_A2B2 = 188u,
};
static const CursorInputOpcode cursor_input_opcodes[] = {
    {{0xa112u, 1u, 3u, 1u}, 0x0u, 0u, 0u},
    {{0xa113u, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa115u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa118u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa11au, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa11cu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa11du, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa11eu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa11fu, 3u, 5u, 2u}, 0x68ceu, 1u, 0u},
    {{0xa122u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa124u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa127u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa129u, 1u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa12au, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa12bu, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa12du, 3u, 5u, 2u}, 0x68a6u, 1u, 0u},
    {{0xa130u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa131u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa133u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa136u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa138u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa13bu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa13eu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa13fu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa142u, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa145u, 3u, 5u, 2u}, 0x68bau, 1u, 0u},
    {{0xa148u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa149u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa14cu, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa14fu, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa151u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa154u, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa157u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa15au, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa15du, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa160u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa161u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa163u, 3u, 5u, 2u}, 0x68a6u, 1u, 0u},
    {{0xa166u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa167u, 3u, 5u, 2u}, 0x68bau, 1u, 0u},
    {{0xa16au, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa16cu, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa16fu, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa171u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa174u, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa177u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa17au, 3u, 5u, 2u}, 0x68bau, 1u, 0u},
    {{0xa17du, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa17eu, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa181u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa183u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa186u, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa189u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa18cu, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa18fu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa192u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa193u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa196u, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa199u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa19au, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa19cu, 3u, 5u, 2u}, 0x689cu, 1u, 0u},
    {{0xa19fu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1a0u, 3u, 5u, 2u}, 0x68b0u, 1u, 0u},
    {{0xa1a3u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1a5u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa1a8u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa1aau, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1adu, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1afu, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa1b2u, 3u, 5u, 2u}, 0x68b0u, 1u, 0u},
    {{0xa1b5u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1b6u, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa1b9u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa1bbu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1beu, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa1c1u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa1c4u, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa1c7u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1cau, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1cbu, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1cdu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1ceu, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1d0u, 3u, 5u, 2u}, 0x689cu, 1u, 0u},
    {{0xa1d3u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1d4u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1d6u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa1d9u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa1dbu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1deu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1e1u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1e2u, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1e4u, 3u, 5u, 2u}, 0x68b0u, 1u, 0u},
    {{0xa1e7u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa1e8u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa1ebu, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa1eeu, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa1f0u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1f3u, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa1f6u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa1f9u, 3u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa1fcu, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa1feu, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa200u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa203u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa205u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa208u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa20bu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa20eu, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa211u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa214u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa216u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa219u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa21cu, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa21fu, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa222u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa224u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa225u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa226u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa227u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa229u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa22au, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa22bu, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa22du, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa22eu, 3u, 5u, 2u}, 0x6860u, 1u, 0u},
    {{0xa231u, 3u, 5u, 2u}, 0x6862u, 1u, 0u},
    {{0xa234u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa236u, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa238u, 3u, 4u, 1u}, 0x100u, 2u, 0u},
    {{0xa23bu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa23cu, 2u, 3u, 1u}, 0x0u, 0u, 0u},
    {{0xa23eu, 3u, 6u, 1u}, 0x0u, 0u, 0u},
    {{0xa241u, 1u, 2u, 0u}, 0x0u, 0u, 1u},
    {{0xa242u, 3u, 4u, 1u}, 0x100u, 2u, 0u},
    {{0xa245u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa246u, 2u, 3u, 1u}, 0x0u, 0u, 0u},
    {{0xa248u, 3u, 6u, 1u}, 0x0u, 0u, 0u},
    {{0xa24bu, 1u, 2u, 0u}, 0x0u, 0u, 1u},
    {{0xa24cu, 1u, 2u, 0u}, 0x0u, 0u, 1u},
    {{0xa24du, 1u, 2u, 0u}, 0x0u, 0u, 1u},
    {{0xa24eu, 2u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa250u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa252u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa254u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa257u, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa259u, 3u, 4u, 1u}, 0x686bu, 1u, 0u},
    {{0xa25cu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa25du, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa25fu, 3u, 8u, 2u}, 0x0u, 0u, 0u},
    {{0xa262u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa264u, 3u, 4u, 1u}, 0x686bu, 1u, 0u},
    {{0xa267u, 3u, 6u, 1u}, 0x0u, 0u, 0u},
    {{0xa26au, 3u, 4u, 1u}, 0x6866u, 1u, 0u},
    {{0xa26du, 2u, 3u, 1u}, 0x0u, 0u, 0u},
    {{0xa26fu, 3u, 5u, 2u}, 0x6864u, 1u, 0u},
    {{0xa272u, 2u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa274u, 3u, 5u, 2u}, 0x6867u, 1u, 0u},
    {{0xa277u, 3u, 4u, 1u}, 0x6862u, 1u, 0u},
    {{0xa27au, 2u, 3u, 1u}, 0x0u, 0u, 0u},
    {{0xa27cu, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa27eu, 2u, 7u, 5u}, 0x0u, 0u, 0u},
    {{0xa280u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa282u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa285u, 3u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa288u, 3u, 5u, 2u}, 0x6860u, 1u, 0u},
    {{0xa28bu, 1u, 4u, 2u}, 0x0u, 0u, 0u},
    {{0xa28cu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa28du, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa28eu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa28fu, 2u, 7u, 5u}, 0x0u, 0u, 0u},
    {{0xa291u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa294u, 1u, 2u, 0u}, 0x0u, 0u, 1u},
    {{0xa295u, 1u, 2u, 0u}, 0x0u, 0u, 1u},
    {{0xa296u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa297u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa298u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa299u, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa29au, 2u, 7u, 4u}, 0x0u, 0u, 0u},
    {{0xa29cu, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa29eu, 1u, 5u, 2u}, 0x0u, 0u, 0u},
    {{0xa29fu, 1u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa2a0u, 3u, 7u, 2u}, 0x0u, 0u, 0u},
    {{0xa2a3u, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa2a5u, 2u, 2u, 0u}, 0x0u, 0u, 0u},
    {{0xa2a7u, 2u, 3u, 1u}, 0x0u, 0u, 0u},
    {{0xa2a9u, 3u, 6u, 2u}, 0x0u, 0u, 0u},
    {{0xa2acu, 2u, 3u, 0u}, 0x0u, 0u, 0u},
    {{0xa2aeu, 3u, 4u, 0u}, 0x0u, 0u, 0u},
    {{0xa2b1u, 1u, 4u, 1u}, 0x0u, 0u, 0u},
    {{0xa2b2u, 1u, 6u, 2u}, 0x0u, 0u, 0u},
};


enum { CURSOR_INPUT_CLOCK_CAPACITY = 512u };

typedef struct CursorInputClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[CURSOR_INPUT_CLOCK_CAPACITY];
    uint64_t master_bound;
    uint32_t next;
    unsigned last_write;
} CursorInputClock;

static unsigned CursorInputWord(const CpuState *cpu, unsigned address) {
    const unsigned at = ((cpu->DB & 1u) << 16) | address;
    return cpu->ram[at] | (unsigned)cpu->ram[at + 1u] << 8;
}

static unsigned CursorInputByte(const CpuState *cpu, unsigned address) {
    return cpu->ram[((cpu->DB & 1u) << 16) | address];
}

static bool CursorInputAdd(CursorInputClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == CURSOR_INPUT_CLOCK_CAPACITY)
        return false;
    clock->timing[clock->span.count++] = op;
    clock->master_bound += op.fetched * cpu_region_speed(0x840000u | op.pc) +
        op.data_bytes * 12u + (op.cycles - op.fetched - op.data_bytes) * 6u;
    return true;
}

static bool CursorInputRange(CursorInputClock *clock, unsigned first,
    unsigned end, unsigned x, unsigned y) {
    for (unsigned index = first; index < end; ++index) {
        const CursorInputOpcode *record = cursor_input_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const unsigned offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                (((unsigned)record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!CursorInputAdd(clock, op))
            return false;
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}

static bool CursorInputBranch(CursorInputClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = cursor_input_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return CursorInputAdd(clock, op);
}

static bool PlanCursorPoll(CursorInputClock *clock, uint32_t entry) {
    const CpuState *cpu = clock->span.cpu;
    const unsigned selection = entry == 0x84a112u ? cpu->A & 255u :
        cpu->ram[6u] | (unsigned)cpu->ram[7u] << 8;
    if (selection >= 5u)
        return false;
    if (entry == 0x84a112u) {
        if (!CursorInputRange(clock, CURSOR_PC_A112, CURSOR_PC_A11A, 0u, 0u))
            return false;
    } else if (!CursorInputRange(clock, CURSOR_PC_A2AC, CURSOR_PC_A2B1, 0u, 0u)) {
        return false;
    }
    clock->next = 0x84a124u;
    return CursorInputRange(clock, CURSOR_PC_A11A, CURSOR_PC_A124,
        selection * 2u, 0u);
}

static bool PlanCursorDirection(CursorInputClock *clock, unsigned direction) {
    const CpuState *cpu = clock->span.cpu;
    const unsigned x = cpu->X;
    const bool vertical = direction < 2u;
    const bool backwards = direction == 0u || direction == 3u;
    const unsigned position = CursorInputWord(cpu, (vertical ? 0x68a6u : 0x689cu) + x);
    const unsigned count = CursorInputWord(cpu, (vertical ? 0x68bau : 0x68b0u) + x);
    const uint16_t changed = (uint16_t)(position + (backwards ? -1 : 1));
    const bool wrap = backwards ? (changed & 0x8000u) != 0u :
        ((uint16_t)(changed - count) & 0x8000u) == 0u;
    static const unsigned first[] = {
        CURSOR_PC_A12D, CURSOR_PC_A163, CURSOR_PC_A19C, CURSOR_PC_A1D0
    };
    static const unsigned branch[] = {
        CURSOR_PC_A131, CURSOR_PC_A16A, CURSOR_PC_A1A3, CURSOR_PC_A1D4
    };
    static const unsigned normal[] = {
        CURSOR_PC_A133, CURSOR_PC_A16C, CURSOR_PC_A1A5, CURSOR_PC_A1D6
    };
    static const unsigned wrapped[] = {
        CURSOR_PC_A145, CURSOR_PC_A177, CURSOR_PC_A1AF, CURSOR_PC_A1E4
    };
    static const unsigned child[] = {
        CURSOR_PC_A157, CURSOR_PC_A189, CURSOR_PC_A1C1, CURSOR_PC_A1F6
    };
    if (!CursorInputRange(clock, first[direction], branch[direction], x, 0u) ||
        !CursorInputBranch(clock, branch[direction], wrap))
        return false;
    clock->next = wrap ? 0x840000u | cursor_input_opcodes[child[direction]].timing.pc :
        0x84a222u;
    return CursorInputRange(clock, wrap ? wrapped[direction] : normal[direction],
        wrap ? child[direction] : wrapped[direction], x, 0u);
}

static bool PlanCursorCommand(CursorInputClock *clock) {
    const CpuState *cpu = clock->span.cpu;
    if (cpu->X > 8u || (cpu->X & 1u))
        return false;
    if (!CursorInputRange(clock, CURSOR_PC_A127, CURSOR_PC_A12B, cpu->X, 0u))
        return false;
    static const unsigned shifts[] = {
        CURSOR_PC_A12A, CURSOR_PC_A160, CURSOR_PC_A199, CURSOR_PC_A1CD
    };
    static const unsigned branches[] = {
        CURSOR_PC_A12B, CURSOR_PC_A161, CURSOR_PC_A19A, CURSOR_PC_A1CE
    };
    for (unsigned direction = 0u; direction < 4u; ++direction) {
        if (direction && !CursorInputRange(clock, shifts[direction],
                branches[direction], cpu->X, 0u))
            return false;
        const bool pressed = (cpu->A & (0x100u << direction)) != 0u;
        if (!CursorInputBranch(clock, branches[direction], !pressed))
            return false;
        if (pressed)
            return PlanCursorDirection(clock, direction);
    }
    if (!CursorInputRange(clock, CURSOR_PC_A1FE, CURSOR_PC_A203, cpu->X, 0u) ||
        !CursorInputBranch(clock, CURSOR_PC_A203, cpu->A != 1u))
        return false;
    if (cpu->A == 1u) {
        clock->next = 0x84a208u;
        return CursorInputRange(clock, CURSOR_PC_A205, CURSOR_PC_A208, 0u, 0u);
    }
    if (!CursorInputRange(clock, CURSOR_PC_A211, CURSOR_PC_A214, cpu->X, 0u) ||
        !CursorInputBranch(clock, CURSOR_PC_A214, cpu->A != 2u))
        return false;
    clock->next = cpu->A == 2u ? 0x84a219u : 0x84a257u;
    return cpu->A != 2u ||
        CursorInputRange(clock, CURSOR_PC_A216, CURSOR_PC_A219, 0u, 0u);
}

static bool PlanCursorMovement(CursorInputClock *clock) {
    const CpuState *cpu = clock->span.cpu;
    if (cpu->X > 8u || (cpu->X & 1u))
        return false;
    const unsigned record = cpu->X * 6u;
    const unsigned count = CursorInputByte(cpu, 0x6862u + record);
    const unsigned oam = CursorInputWord(cpu, 0x6860u + record);
    if (!count || count > 16u || (oam & 3u) || oam > 0x200u - count * 4u ||
        !CursorInputRange(clock, CURSOR_PC_A222, CURSOR_PC_A22E, cpu->X, 0u) ||
        !CursorInputRange(clock, CURSOR_PC_A22E, CURSOR_PC_A236, record, 0u))
        return false;
    for (unsigned sprite = 0u; sprite < count; ++sprite) {
        if (!CursorInputRange(clock, CURSOR_PC_A236, CURSOR_PC_A250,
                record, oam + sprite * 4u) ||
            !CursorInputBranch(clock, CURSOR_PC_A250, sprite + 1u < count))
            return false;
    }
    clock->next = 0x84a254u;
    return CursorInputRange(clock, CURSOR_PC_A252, CURSOR_PC_A254, record, 0u);
}

static bool CursorInputRomWord(uint32_t address, unsigned *value) {
    if (address > 0xfffffeu || (address & 0xffffu) < 0x8000u ||
        (address & 0xffffu) == 0xffffu || ((address >> 16) & 0x7fu) >= 0x40u)
        return false;
    const unsigned offset = ((address >> 16) & 0x7fu) * 0x8000u + (address & 0x7fffu);
    if (offset + 1u >= g_snes->cart->romSize)
        return false;
    *value = g_snes->cart->rom[offset] | (unsigned)g_snes->cart->rom[offset + 1u] << 8;
    return true;
}

static bool PlanCursorAnimation(CursorInputClock *clock) {
    const CpuState *cpu = clock->span.cpu;
    const unsigned record = cpu->X;
    if (record > 48u || (record & 1u))
        return false;
    const unsigned period = CursorInputByte(cpu, 0x686bu + record);
    const unsigned timer = CursorInputByte(cpu, 0x686au + record);
    const bool disabled = (((period - 1u) & 255u) & 0x80u) != 0u;
    if (!CursorInputRange(clock, CURSOR_PC_A257, CURSOR_PC_A25D, record, 0u) ||
        !CursorInputBranch(clock, CURSOR_PC_A25D, disabled))
        return false;
    if (!disabled) {
        const bool waiting = ((timer - 1u) & 255u) != 0u;
        if (!CursorInputRange(clock, CURSOR_PC_A25F, CURSOR_PC_A262, record, 0u) ||
            !CursorInputBranch(clock, CURSOR_PC_A262, waiting))
            return false;
        if (!waiting) {
            const unsigned count = CursorInputByte(cpu, 0x6862u + record) |
                (unsigned)cpu->ram[1u] << 8;
            const unsigned oam = CursorInputWord(cpu, 0x6860u + record);
            const uint32_t source = CursorInputWord(cpu, 0x6864u + record) |
                (uint32_t)CursorInputByte(cpu, 0x6866u + record) << 16;
            unsigned offset = CursorInputWord(cpu, 0x6867u + record), word;
            if (!count || count > 16u || (oam & 3u) || oam > 0x200u - count * 4u ||
                !CursorInputRomWord(source + offset, &word))
                return false;
            const bool continues = word != 0u;
            if (!continues)
                offset = 0u;
            if (offset + count * 2u > 0x10000u ||
                !CursorInputRomWord(source + offset + count * 2u - 2u, &word) ||
                !CursorInputRange(clock, CURSOR_PC_A264, CURSOR_PC_A280, record, 0u) ||
                !CursorInputBranch(clock, CURSOR_PC_A280, continues))
                return false;
            if (!continues &&
                !CursorInputRange(clock, CURSOR_PC_A282, CURSOR_PC_A288, record, 0u))
                return false;
            if (!CursorInputRange(clock, CURSOR_PC_A288, CURSOR_PC_A28F, record, 0u))
                return false;
            for (unsigned sprite = 0u; sprite < count; ++sprite) {
                if (!CursorInputRange(clock, CURSOR_PC_A28F, CURSOR_PC_A29C,
                        oam + 2u + sprite * 4u, offset + sprite * 2u) ||
                    !CursorInputBranch(clock, CURSOR_PC_A29C, sprite + 1u < count))
                    return false;
            }
            if (!CursorInputRange(clock, CURSOR_PC_A29E, CURSOR_PC_A2A3, record, 0u))
                return false;
        }
    }
    clock->next = 0x84a2a9u;
    return CursorInputRange(clock, CURSOR_PC_A2A3, CURSOR_PC_A2A9, record, 0u);
}

static bool PrepareCursorInputClock(CursorInputClock *clock, CpuState *cpu,
    uint32_t entry) {
    if (!cpu->ram || cpu->PB != 0x84u || cpu->D || cpu->_flag_D ||
        cpu->m_flag > 1u || cpu->x_flag > 1u ||
        (entry != 0x84a112u && cpu->x_flag) ||
        ((entry == 0x84a127u || entry == 0x84a222u) && cpu->m_flag) ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || !g_snes->cart || !g_snes->cart->rom ||
        g_snes->cart->type != CART_LOROM || g_snes->cart->romSize < 0x280000u ||
        !g_snes->hdmaBeamOff || g_snes->vPos >= 262u ||
        !Lufia2DecompFrameRangeQuiet(0x84a11au, 0x84a124u))
        return false;
    unsigned opcode_budget;
    if (entry == 0x84a112u) {
        if (cpu->host_return_valid != 2u ||
            !Lufia2DecompFrameRangeQuiet(entry, 0x84a124u))
            return false;
        opcode_budget = CURSOR_INPUT_CLOCK_CAPACITY;
    } else {
        uint32_t end;
        switch (entry) {
        case 0x84a127u: end = 0x84a222u; break;
        case 0x84a222u: end = 0x84a254u; break;
        case 0x84a257u: end = 0x84a2a9u; break;
        case 0x84a2acu: end = 0x84a2b1u; break;
        default: return false;
        }
        opcode_budget = Lufia2DecompFrameSpanBudget(entry, end, ActorBridgeCursorInputPhase);
        if (!opcode_budget)
            return false;
    }
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->master_bound = 64u;
    clock->last_write = 0u;
    bool planned;
    switch (entry) {
    case 0x84a112u: case 0x84a2acu: planned = PlanCursorPoll(clock, entry); break;
    case 0x84a127u: planned = PlanCursorCommand(clock); break;
    case 0x84a222u: planned = PlanCursorMovement(clock); break;
    default: planned = PlanCursorAnimation(clock); break;
    }
    if (!planned || clock->span.count > opcode_budget)
        return false;
    const uint64_t budget = clock->master_bound + 40u * (clock->master_bound / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, entry, 0x1ff7u, true, 1363u, budget))
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static uint8_t CursorInputRead(void *context, uint32_t address) {
    CursorInputClock *clock = context;
    return FixedSpanRead(&clock->span, address);
}

static void CursorInputWrite(void *context, uint32_t address, uint8_t value) {
    CursorInputClock *clock = context;
    const uint64_t before = g_interp_bridge_write_epoch;
    FixedSpanWrite(&clock->span, address, value);
    if (before != g_interp_bridge_write_epoch)
        clock->last_write = clock->span.index + (clock->span.remaining != 0u);
}

static uint32_t RunCursorInputPhase(CursorInputClock *clock, uint32_t entry) {
    const Lufia2Memory memory = {CursorInputRead, CursorInputWrite, clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(clock->span.cpu, &state);
    const Lufia2ExecutionResult result = Lufia2SpecialMenuCursorPhase(&memory, &state, entry);
    assert(result.flow == LUFIA2_EXECUTION_BOUNDARY && result.pc == clock->next);
    ActorBridgeStore(clock->span.cpu, &state);
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    return result.pc;
}

static void ActorBridgeCursorInputPhase(CpuState *cpu, uint32_t entry) {
    CursorInputClock clock;
    if (!PrepareCursorInputClock(&clock, cpu, entry))
        return;
    const uint32_t next = RunCursorInputPhase(&clock, entry);
    Lufia2DecompFrameReturnCompleteSpan(clock.span.count, clock.last_write);
    interp_bridge_pre_opcode_redirect(next);
}

static RecompReturn ActorBridgeCursorInput(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CursorInputClock clock;
    if (!PrepareCursorInputClock(&clock, cpu, 0x84a112u))
        return ActorBridgeFallback(cpu, &frame, 0x84a112u);
    const uint32_t next = RunCursorInputPhase(&clock, 0x84a112u);
    return interp_tier_dispatch_tail(cpu, next, next, frame.entry_s, frame.hrv);
}
