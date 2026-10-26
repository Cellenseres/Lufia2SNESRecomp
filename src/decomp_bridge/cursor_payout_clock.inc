typedef struct CursorPayoutOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index, advance_y;
} CursorPayoutOpcode;

enum {
    CP_PC_A000 = 0u,
    CP_PC_A001 = 1u,
    CP_PC_A003 = 2u,
    CP_PC_A005 = 3u,
    CP_PC_A008 = 4u,
    CP_PC_A00B = 5u,
    CP_PC_A00D = 6u,
    CP_PC_A010 = 7u,
    CP_PC_A011 = 8u,
    CP_PC_A012 = 9u,
    CP_PC_A014 = 10u,
    CP_PC_A016 = 11u,
    CP_PC_A018 = 12u,
    CP_PC_A01B = 13u,
    CP_PC_A01C = 14u,
    CP_PC_A01D = 15u,
    CP_PC_A01E = 16u,
    CP_PC_A01F = 17u,
    CP_PC_A020 = 18u,
    CP_PC_A022 = 19u,
    CP_PC_A025 = 20u,
    CP_PC_A026 = 21u,
    CP_PC_A028 = 22u,
    CP_PC_A029 = 23u,
    CP_PC_A02A = 24u,
    CP_PC_A02B = 25u,
    CP_PC_A02D = 26u,
    CP_PC_A030 = 27u,
    CP_PC_A031 = 28u,
    CP_PC_A032 = 29u,
    CP_PC_A034 = 30u,
    CP_PC_A035 = 31u,
    CP_PC_A036 = 32u,
    CP_PC_A038 = 33u,
    CP_PC_A039 = 34u,
    CP_PC_A03C = 35u,
    CP_PC_A03F = 36u,
    CP_PC_A042 = 37u,
    CP_PC_A045 = 38u,
    CP_PC_A048 = 39u,
    CP_PC_A049 = 40u,
    CP_PC_A04A = 41u,
    CP_PC_A04C = 42u,
    CP_PC_A04F = 43u,
    CP_PC_A052 = 44u,
    CP_PC_A053 = 45u,
    CP_PC_A055 = 46u,
    CP_PC_A058 = 47u,
    CP_PC_A05B = 48u,
    CP_PC_A05C = 49u,
    CP_PC_A05D = 50u,
    CP_PC_A05F = 51u,
    CP_PC_A062 = 52u,
    CP_PC_A065 = 53u,
    CP_PC_A066 = 54u,
    CP_PC_A068 = 55u,
    CP_PC_A06B = 56u,
    CP_PC_A06E = 57u,
    CP_PC_A06F = 58u,
    CP_PC_A070 = 59u,
    CP_PC_A072 = 60u,
    CP_PC_A075 = 61u,
    CP_PC_A078 = 62u,
    CP_PC_A07B = 63u,
    CP_PC_A07E = 64u,
    CP_PC_A07F = 65u,
    CP_PC_A080 = 66u,
    CP_PC_A083 = 67u,
    CP_PC_A086 = 68u,
    CP_PC_A087 = 69u,
    CP_PC_A088 = 70u,
    CP_PC_A08B = 71u,
    CP_PC_A08E = 72u,
    CP_PC_A090 = 73u,
    CP_PC_A091 = 74u,
    CP_PC_A092 = 75u,
    CP_PC_A095 = 76u,
    CP_PC_A098 = 77u,
    CP_PC_A099 = 78u,
    CP_PC_A09A = 79u,
    CP_PC_A09B = 80u,
    CP_PC_A09C = 81u,
    CP_PC_A09D = 82u,
    CP_PC_A09E = 83u,
    CP_PC_A0A0 = 84u,
    CP_PC_A0A1 = 85u,
    CP_PC_A0A4 = 86u,
    CP_PC_A0A7 = 87u,
    CP_PC_A0A9 = 88u,
    CP_PC_A0AA = 89u,
    CP_PC_A0AB = 90u,
    CP_PC_A0AE = 91u,
    CP_PC_A0B1 = 92u,
    CP_PC_A0B2 = 93u,
    CP_PC_A0B4 = 94u,
    CP_PC_A0B7 = 95u,
    CP_PC_A0B8 = 96u,
    CP_PC_A0B9 = 97u,
    CP_PC_A0BA = 98u,
    CP_PC_A0BB = 99u,
    CP_PC_A0BE = 100u,
    CP_PC_A0C1 = 101u,
    CP_PC_A0C2 = 102u,
    CP_PC_A0C3 = 103u,
    CP_PC_A0C4 = 104u,
    CP_PC_A0C5 = 105u,
    CP_PC_A0C7 = 106u,
    CP_PC_A0C9 = 107u,
    CP_PC_A0CB = 108u,
    CP_PC_A0CE = 109u,
    CP_PC_A0D1 = 110u,
    CP_PC_A0D3 = 111u,
    CP_PC_A0D5 = 112u,
    CP_PC_A0D8 = 113u,
    CP_PC_A0D9 = 114u,
    CP_PC_A0DA = 115u,
    CP_PC_A0DB = 116u,
    CP_PC_A0DD = 117u,
    CP_PC_A0E0 = 118u,
    CP_PC_A0E3 = 119u,
    CP_PC_A0E4 = 120u,
    CP_PC_A0E5 = 121u,
    CP_PC_A0E7 = 122u,
    CP_PC_A0E8 = 123u,
    CP_PC_A0E9 = 124u,
    CP_PC_A0EB = 125u,
    CP_PC_A0EC = 126u,
    CP_PC_A0EF = 127u,
    CP_PC_A0F0 = 128u,
    CP_PC_A0F3 = 129u,
    CP_PC_A0F4 = 130u,
    CP_PC_A0F7 = 131u,
    CP_PC_A0FA = 132u,
    CP_PC_A0FB = 133u,
    CP_PC_A0FC = 134u,
    CP_PC_A0FF = 135u,
    CP_PC_A102 = 136u,
    CP_PC_A103 = 137u,
    CP_PC_A104 = 138u,
    CP_PC_A105 = 139u,
    CP_PC_A107 = 140u,
    CP_PC_A109 = 141u,
    CP_PC_A10B = 142u,
    CP_PC_A10D = 143u,
    CP_PC_A110 = 144u,
    CP_PC_A111 = 145u,
    CP_PC_C7A6 = 146u,
    CP_PC_C7A9 = 147u,
    CP_PC_C7AC = 148u,
    CP_PC_C7AF = 149u,
    CP_PC_C7B2 = 150u,
    CP_PC_C7B5 = 151u,
    CP_PC_C7B9 = 152u,
    CP_PC_C7BC = 153u,
    CP_PC_C7BF = 154u,
    CP_PC_C7C3 = 155u,
    CP_PC_C7C5 = 156u,
    CP_PC_C7C7 = 157u,
    CP_PC_C7C9 = 158u,
    CP_PC_C7CB = 159u,
    CP_PC_C7CE = 160u,
    CP_PC_C7D1 = 161u,
    CP_PC_C7D4 = 162u,
    CP_PC_C7D6 = 163u,
    CP_PC_C7D7 = 164u,
    CP_PC_C7DA = 165u,
    CP_PC_C7DC = 166u,
    CP_PC_C7DF = 167u,
    CP_PC_C7E2 = 168u,
    CP_PC_C7E4 = 169u,
    CP_PC_C7E7 = 170u,
    CP_PC_C7E9 = 171u,
    CP_PC_C7EB = 172u,
    CP_PC_C7EE = 173u,
    CP_PC_C7F1 = 174u,
    CP_PC_C7F4 = 175u,
    CP_PC_C7F8 = 176u,
    CP_PC_C7FB = 177u,
    CP_PC_C7FE = 178u,
    CP_PC_C802 = 179u,
    CP_PC_C804 = 180u,
    CP_PC_C806 = 181u,
    CP_PC_C808 = 182u,
    CP_PC_C80B = 183u,
    CP_PC_C80E = 184u,
    CP_PC_C810 = 185u,
    CP_PC_C812 = 186u,
    CP_PC_C815 = 187u,
    CP_PC_C818 = 188u,
};

static const CursorPayoutOpcode cursor_payout_opcodes[] = {
    {{0xa000u, 1u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xa001u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa003u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa005u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa008u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa00bu, 2u, 6u, 4u}, 0x0000u, 0u, 0u},
    {{0xa00du, 3u, 6u, 1u}, 0x0000u, 0u, 0u},
    {{0xa010u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa011u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa012u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa014u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa016u, 2u, 7u, 5u}, 0x0000u, 0u, 0u},
    {{0xa018u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa01bu, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa01cu, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa01du, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa01eu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa01fu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa020u, 2u, 7u, 5u}, 0x0000u, 0u, 0u},
    {{0xa022u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa025u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa026u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa028u, 1u, 4u, 1u}, 0x0000u, 0u, 0u},
    {{0xa029u, 1u, 6u, 3u}, 0x0000u, 0u, 0u},
    {{0xa02au, 1u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xa02bu, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa02du, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xa030u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa031u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa032u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa034u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa035u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa036u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa038u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa039u, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xa03cu, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa03fu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa042u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa045u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa048u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa049u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa04au, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa04cu, 3u, 4u, 1u}, 0x68d8u, 2u, 0u},
    {{0xa04fu, 3u, 6u, 1u}, 0x0000u, 0u, 0u},
    {{0xa052u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa053u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa055u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa058u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa05bu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa05cu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa05du, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa05fu, 3u, 4u, 1u}, 0x68d8u, 2u, 0u},
    {{0xa062u, 3u, 6u, 1u}, 0x0000u, 0u, 0u},
    {{0xa065u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa066u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa068u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa06bu, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa06eu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa06fu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa070u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa072u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa075u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa078u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa07bu, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa07eu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa07fu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa080u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa083u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa086u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa087u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa088u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa08bu, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa08eu, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa090u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa091u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa092u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa095u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa098u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa099u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa09au, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa09bu, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa09cu, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa09du, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa09eu, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0a0u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0a1u, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa0a4u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0a7u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0a9u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0aau, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0abu, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0aeu, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa0b1u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0b2u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0b4u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0b7u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0b8u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0b9u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0bau, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0bbu, 3u, 5u, 2u}, 0x68d8u, 2u, 0u},
    {{0xa0beu, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0c1u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0c2u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0c3u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0c4u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0c5u, 2u, 7u, 4u}, 0x0000u, 0u, 0u},
    {{0xa0c7u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0c9u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0cbu, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0ceu, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0d1u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0d3u, 2u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xa0d5u, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0d8u, 1u, 4u, 1u}, 0x0000u, 0u, 0u},
    {{0xa0d9u, 1u, 6u, 3u}, 0x0000u, 0u, 0u},
    {{0xa0dau, 1u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xa0dbu, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0ddu, 3u, 8u, 4u}, 0x0000u, 0u, 0u},
    {{0xa0e0u, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0e3u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0e4u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0e5u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0e7u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0e8u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0e9u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0ebu, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0ecu, 3u, 5u, 2u}, 0x6860u, 1u, 0u},
    {{0xa0efu, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0f0u, 3u, 5u, 2u}, 0x6862u, 1u, 0u},
    {{0xa0f3u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0f4u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0f7u, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa0fau, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0fbu, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa0fcu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa0ffu, 3u, 7u, 2u}, 0x0000u, 0u, 0u},
    {{0xa102u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa103u, 1u, 2u, 0u}, 0x0000u, 0u, 1u},
    {{0xa104u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa105u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa107u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xa109u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xa10bu, 2u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xa10du, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xa110u, 1u, 4u, 1u}, 0x0000u, 0u, 0u},
    {{0xa111u, 1u, 6u, 3u}, 0x0000u, 0u, 0u},
    {{0xc7a6u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7a9u, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7acu, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7afu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7b2u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7b5u, 4u, 8u, 3u}, 0x0000u, 0u, 0u},
    {{0xc7b9u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7bcu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7bfu, 4u, 8u, 3u}, 0x0000u, 0u, 0u},
    {{0xc7c3u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7c5u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7c7u, 2u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xc7c9u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7cbu, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7ceu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7d1u, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7d4u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7d6u, 1u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7d7u, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7dau, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7dcu, 3u, 5u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7dfu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7e2u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7e4u, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7e7u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7e9u, 2u, 4u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7ebu, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc7eeu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7f1u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7f4u, 4u, 8u, 3u}, 0x0000u, 0u, 0u},
    {{0xc7f8u, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7fbu, 3u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc7feu, 4u, 8u, 3u}, 0x0000u, 0u, 0u},
    {{0xc802u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc804u, 2u, 2u, 0u}, 0x0000u, 0u, 0u},
    {{0xc806u, 2u, 3u, 1u}, 0x0000u, 0u, 0u},
    {{0xc808u, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc80bu, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc80eu, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc810u, 2u, 3u, 0u}, 0x0000u, 0u, 0u},
    {{0xc812u, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc815u, 3u, 6u, 2u}, 0x0000u, 0u, 0u},
    {{0xc818u, 1u, 6u, 3u}, 0x0000u, 0u, 0u},
};

enum { CURSOR_PAYOUT_TIMING_CAPACITY = 512u };

typedef struct CursorPayoutClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[CURSOR_PAYOUT_TIMING_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint64_t master_bound;
    uint32_t next;
    bool returning;
} CursorPayoutClock;

static unsigned CursorPayoutWord(const CpuState *cpu, unsigned bank, unsigned at) {
    const unsigned offset = ((bank & 1u) << 16) | at;
    return cpu->ram[offset] | (unsigned)cpu->ram[offset + 1u] << 8;
}

static bool CursorPayoutOpcodeAdd(CursorPayoutClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == CURSOR_PAYOUT_TIMING_CAPACITY)
        return false;
    clock->timing[clock->span.count++] = op;
    clock->master_bound += op.fetched * cpu_region_speed(0x840000u | op.pc) +
        op.data_bytes * 12u + (op.cycles - op.fetched - op.data_bytes) * 6u;
    return true;
}

static bool CursorPayoutRange(CursorPayoutClock *clock, unsigned first,
    unsigned end, unsigned x, unsigned y) {
    for (unsigned index = first; index < end; ++index) {
        const CursorPayoutOpcode *record = cursor_payout_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const unsigned offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                (((unsigned)record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!CursorPayoutOpcodeAdd(clock, op))
            return false;
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}

static bool CursorPayoutBranch(CursorPayoutClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = cursor_payout_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return CursorPayoutOpcodeAdd(clock, op);
}

static bool CursorDefinitionCount(const CpuState *cpu, unsigned *count) {
    const uint32_t pointer = cpu->ram[8u] | (uint32_t)cpu->ram[9u] << 8 |
        (uint32_t)cpu->ram[10u] << 16;
    const unsigned bank = pointer >> 16;
    const unsigned at = pointer & 0xffffu;
    if (at > 0xffeeu)
        return false;
    if (bank == 0x7eu || bank == 0x7fu) {
        *count = CursorPayoutWord(cpu, bank, at + 16u);
        const unsigned destination = (unsigned)cpu->DB << 16 | 0x68d8u;
        if (pointer < destination + 114u && pointer + 114u > destination)
            return false;
    } else {
        if ((bank & 0x7fu) >= 0x40u || at < 0x8000u)
            return false;
        const unsigned offset = (bank & 0x7fu) * 0x8000u + (at & 0x7fffu) + 16u;
        if (offset + 1u >= g_snes->cart->romSize)
            return false;
        *count = g_snes->cart->rom[offset] | (unsigned)g_snes->cart->rom[offset + 1u] << 8;
    }
    return *count && *count <= 24u && at + 18u + *count * 4u <= 0x10000u;
}

static bool PlanCursorDefinition(CursorPayoutClock *clock) {
    unsigned count;
    if (!CursorDefinitionCount(clock->call.cpu, &count) ||
        !CursorPayoutRange(clock, CP_PC_A000, CP_PC_A00B, 0u, 0u))
        return false;
    for (unsigned byte = 0u; byte < 16u; ++byte) {
        if (!CursorPayoutRange(clock, CP_PC_A00B, CP_PC_A012, 0u, byte) ||
            !CursorPayoutBranch(clock, CP_PC_A012, byte != 15u))
            return false;
    }
    if (!CursorPayoutRange(clock, CP_PC_A014, CP_PC_A01E, 0u, 16u))
        return false;
    for (unsigned word = 0u; word < count * 2u; ++word) {
        if (!CursorPayoutRange(clock, CP_PC_A01E, CP_PC_A026, 0u, 16u + word * 2u) ||
            !CursorPayoutBranch(clock, CP_PC_A026, word + 1u < count * 2u))
            return false;
    }
    clock->next = 0x84a029u;
    clock->returning = true;
    return CursorPayoutRange(clock, CP_PC_A028, CP_PC_A029, 0u, 0u);
}

static bool PlanCursorSprites(CursorPayoutClock *clock, bool pop) {
    const CpuState *cpu = clock->call.cpu;
    unsigned depth = CursorPayoutWord(cpu, cpu->DB, 0x685eu);
    if (pop) {
        if (!depth || depth > 5u) return false;
        --depth;
    } else if (depth >= 5u) return false;
    const unsigned record = depth * 10u;
    const unsigned count = CursorPayoutWord(cpu, cpu->DB,
        pop ? 0x6862u + record : 0x68e8u);
    const unsigned oam = CursorPayoutWord(cpu, cpu->DB,
        pop ? 0x6860u + record : 0x69d8u);
    if (!count || count > 16u || oam > 0x200u - count * 4u || (oam & 3u))
        return false;
    if (pop) {
        if (!CursorPayoutRange(clock, CP_PC_A0DA, CP_PC_A0F4, record, 0u))
            return false;
        for (unsigned sprite = 0u; sprite < count; ++sprite) {
            if (!CursorPayoutRange(clock, CP_PC_A0F4, CP_PC_A105, 0u, oam + sprite * 4u) ||
                !CursorPayoutBranch(clock, CP_PC_A105, sprite + 1u < count))
                return false;
        }
        clock->next = 0x84a110u;
        return CursorPayoutRange(clock, CP_PC_A107, CP_PC_A110, 0u, 0u);
    }
    if (!CursorPayoutRange(clock, CP_PC_A02A, CP_PC_A03F, record, 0u) ||
        !CursorPayoutRange(clock, CP_PC_A03F, CP_PC_A070, record, 0u) ||
        !CursorPayoutRange(clock, CP_PC_A070, CP_PC_A09A, depth * 2u, 8u) ||
        !CursorPayoutRange(clock, CP_PC_A09A, CP_PC_A0AE, record, 16u))
        return false;
    for (unsigned sprite = 0u; sprite < count; ++sprite) {
        if (!CursorPayoutRange(clock, CP_PC_A0AE, CP_PC_A0C7, oam + sprite * 4u,
                18u + sprite * 4u) ||
            !CursorPayoutBranch(clock, CP_PC_A0C7, sprite + 1u < count))
            return false;
    }
    clock->next = 0x84a0d8u;
    return CursorPayoutRange(clock, CP_PC_A0C9, CP_PC_A0D8, 0u, 0u);
}

static bool PlanAwardStep(CursorPayoutClock *clock) {
    const CpuState *cpu = clock->call.cpu;
    const unsigned step = clock->span.count && clock->timing[0].pc == 0xc7ceu
        ? 10000u : CursorPayoutWord(cpu, cpu->DB, 0x69e4u);
    const bool negative = ((uint16_t)(CursorPayoutWord(cpu, 0u, 0u) - step) & 0x8000u) != 0u;
    if (!CursorPayoutRange(clock, CP_PC_C7D4, CP_PC_C7DA, 0u, 0u) ||
        !CursorPayoutBranch(clock, CP_PC_C7DA, !negative))
        return false;
    if (!negative) {
        clock->next = 0x84c7eeu;
        return CursorPayoutRange(clock, CP_PC_C7E9, CP_PC_C7EE, 0u, 0u);
    }
    if (!CursorPayoutRange(clock, CP_PC_C7DC, CP_PC_C7E2, 0u, 0u) ||
        !CursorPayoutBranch(clock, CP_PC_C7E2, step == 1u))
        return false;
    clock->next = step == 1u ? 0x84c815u : 0x84c7e7u;
    return step == 1u ? CursorPayoutRange(clock, CP_PC_C812, CP_PC_C815, 0u, 0u)
                     : CursorPayoutRange(clock, CP_PC_C7E4, CP_PC_C7E7, 0u, 0u);
}

static bool PlanAward(CursorPayoutClock *clock, uint32_t pc) {
    unsigned first, end;
    switch (pc) {
    case 0x84c7a6u: first = CP_PC_C7A6; end = CP_PC_C7AF; break;
    case 0x84c7afu: first = CP_PC_C7AF; end = CP_PC_C7B9; break;
    case 0x84c7b9u: first = CP_PC_C7B9; end = CP_PC_C7C3; break;
    case 0x84c7c3u: first = CP_PC_C7C3; end = CP_PC_C7CE; break;
    case 0x84c7ceu:
        return CursorPayoutRange(clock, CP_PC_C7CE, CP_PC_C7D4, 0u, 0u) && PlanAwardStep(clock);
    case 0x84c7d4u: return PlanAwardStep(clock);
    case 0x84c7e7u:
        return CursorPayoutRange(clock, CP_PC_C7E7, CP_PC_C7E9, 0u, 0u) && PlanAwardStep(clock);
    case 0x84c7eeu: first = CP_PC_C7EE; end = CP_PC_C7F8; break;
    case 0x84c7f8u: first = CP_PC_C7F8; end = CP_PC_C802; break;
    case 0x84c802u: first = CP_PC_C802; end = CP_PC_C80B; break;
    case 0x84c80bu: first = CP_PC_C80B; end = CP_PC_C80E; break;
    case 0x84c80eu:
        return CursorPayoutRange(clock, CP_PC_C80E, CP_PC_C812, 0u, 0u) && PlanAwardStep(clock);
    case 0x84c815u: first = CP_PC_C815; end = CP_PC_C818; break;
    case 0x84c818u: clock->returning = true; clock->next = pc; return true;
    default: return false;
    }
    clock->next = 0x840000u | cursor_payout_opcodes[end].timing.pc;
    return CursorPayoutRange(clock, first, end, 0u, 0u);
}

static bool PrepareCursorPayoutClock(CursorPayoutClock *clock, uint32_t pc) {
    static const uint32_t first[] = {0x84a000u, 0x84a02au, 0x84a0dau, 0x84c7a6u};
    static const uint32_t end[] = {0x84a02au, 0x84a0dau, 0x84a112u, 0x84c819u};
    CpuState *cpu = clock->call.cpu;
    if (!cpu->ram || cpu->PB != 0x84u || cpu->D || cpu->_flag_D || cpu->x_flag ||
        cpu->m_flag > 1u || (cpu->DB != 0x7eu && cpu->DB != 0x7fu) ||
        clock->call.frame.hrv != 2u || cpu->S < 0x1f20u || cpu->S > 0x1ff8u ||
        cpu->coprocessor_master_cycles != cpu->master_cycles ||
        !g_snes || !g_snes->cart ||
        g_snes->cart->type != CART_LOROM || !g_snes->cart->rom ||
        g_snes->cart->romSize < 0x280000u || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || !Lufia2DecompFrameRangeQuiet(first[clock->root], end[clock->root]))
        return false;
    if (clock->root == 3u && cpu->m_flag != (pc == 0x84c80bu || pc == 0x84c80eu))
        return false;
    if ((pc == 0x84a0d8u || pc == 0x84a110u) && !cpu->m_flag)
        return false;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->master_bound = 64u;
    clock->returning = false;
    bool planned;
    if (clock->root == 3u) planned = PlanAward(clock, pc);
    else if (pc == 0x84a000u) planned = PlanCursorDefinition(clock);
    else if (pc == 0x84a02au || pc == 0x84a0dau) planned = PlanCursorSprites(clock, pc == 0x84a0dau);
    else if (pc == 0x84a0d8u || pc == 0x84a110u) {
        clock->next = pc + 1u;
        clock->returning = true;
        planned = CursorPayoutRange(clock, pc == 0x84a0d8u ? CP_PC_A0D8 : CP_PC_A110,
            pc == 0x84a0d8u ? CP_PC_A0D9 : CP_PC_A111, 0u, 0u);
    } else return false;
    if (!planned) return false;
    const uint64_t budget = clock->master_bound + 40u * (clock->master_bound / 1324u + 1u);
    if (cpu->master_cycles > UINT64_MAX - budget ||
        !ActorBridgeTimedStepSupportedWindow(cpu, pc, 0x1ff8u, true, 1363u, budget))
        return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget)
        return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}

static uint8_t CursorPayoutChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame) {
    CursorPayoutClock *clock = context;
    BeginFixedSpanOpcode(&clock->span);
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame))
        return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame);
    if (PrepareCursorPayoutClock(clock, landing))
        return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeCursorPayout(CpuState *cpu, unsigned root) {
    static const uint32_t entries[] = {0x84a000u, 0x84a02au, 0x84a0dau, 0x84c7a6u};
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    CursorPayoutClock clock;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    clock.root = root;
    if (!PrepareCursorPayoutClock(&clock, entries[root]))
        return ActorBridgeFallback(cpu, &frame, entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result = Lufia2SpecialMenuLoadCursorDefinition(&memory, &state); break;
    case 1u: result = Lufia2SpecialMenuPushCursor(&memory, &state, CursorPayoutChild, &clock); break;
    case 2u: result = Lufia2SpecialMenuPopCursor(&memory, &state, CursorPayoutChild, &clock); break;
    default: result = Lufia2SpecialMenuPayAward(&memory, &state, CursorPayoutChild, &clock); break;
    }
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)clock.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && clock.returning &&
        clock.next == result.pc && clock.span.index == clock.span.count && !clock.span.remaining);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
