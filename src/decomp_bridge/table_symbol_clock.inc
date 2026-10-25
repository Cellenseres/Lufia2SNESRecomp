typedef struct TableSymbolOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index;
    int8_t advance_x, advance_y;
} TableSymbolOpcode;

enum {
    SYMBOL_PC_E5B5 = 0u,
    SYMBOL_PC_E5B8 = 1u,
    SYMBOL_PC_E5BB = 2u,
    SYMBOL_PC_E5BE = 3u,
    SYMBOL_PC_E5C1 = 4u,
    SYMBOL_PC_E5C3 = 5u,
    SYMBOL_PC_E5C7 = 6u,
    SYMBOL_PC_E5C8 = 7u,
    SYMBOL_PC_E5CA = 8u,
    SYMBOL_PC_E5CC = 9u,
    SYMBOL_PC_E5CF = 10u,
    SYMBOL_PC_E5D0 = 11u,
    SYMBOL_PC_E5D1 = 12u,
    SYMBOL_PC_E5D2 = 13u,
    SYMBOL_PC_E5D5 = 14u,
    SYMBOL_PC_E5D6 = 15u,
    SYMBOL_PC_E5D9 = 16u,
    SYMBOL_PC_E5DC = 17u,
    SYMBOL_PC_E5DE = 18u,
    SYMBOL_PC_E5E1 = 19u,
    SYMBOL_PC_E5E4 = 20u,
    SYMBOL_PC_E5E5 = 21u,
    SYMBOL_PC_E5E8 = 22u,
    SYMBOL_PC_E5EB = 23u,
    SYMBOL_PC_E5ED = 24u,
    SYMBOL_PC_E5EF = 25u,
    SYMBOL_PC_E5F1 = 26u,
    SYMBOL_PC_E5F4 = 27u,
    SYMBOL_PC_E5F5 = 28u,
    SYMBOL_PC_E5F8 = 29u,
    SYMBOL_PC_E5FA = 30u,
    SYMBOL_PC_E5FD = 31u,
    SYMBOL_PC_E600 = 32u,
    SYMBOL_PC_E603 = 33u,
    SYMBOL_PC_E606 = 34u,
    SYMBOL_PC_E609 = 35u,
    SYMBOL_PC_E60C = 36u,
    SYMBOL_PC_E60F = 37u,
    SYMBOL_PC_E612 = 38u,
    SYMBOL_PC_E613 = 39u,
    SYMBOL_PC_E616 = 40u,
    SYMBOL_PC_E618 = 41u,
    SYMBOL_PC_E61B = 42u,
    SYMBOL_PC_E61D = 43u,
    SYMBOL_PC_E620 = 44u,
    SYMBOL_PC_E622 = 45u,
    SYMBOL_PC_E625 = 46u,
    SYMBOL_PC_E627 = 47u,
    SYMBOL_PC_E628 = 48u,
    SYMBOL_PC_E629 = 49u,
    SYMBOL_PC_E62C = 50u,
    SYMBOL_PC_E62E = 51u,
    SYMBOL_PC_E62F = 52u,
    SYMBOL_PC_E630 = 53u,
    SYMBOL_PC_E633 = 54u,
    SYMBOL_PC_E635 = 55u,
    SYMBOL_PC_E638 = 56u,
    SYMBOL_PC_E639 = 57u,
    SYMBOL_PC_E63A = 58u,
    SYMBOL_PC_E63D = 59u,
    SYMBOL_PC_E63F = 60u,
    SYMBOL_PC_E640 = 61u,
    SYMBOL_PC_E641 = 62u,
    SYMBOL_PC_E643 = 63u,
    SYMBOL_PC_E645 = 64u,
    SYMBOL_PC_E648 = 65u,
    SYMBOL_PC_E64B = 66u,
    SYMBOL_PC_E64F = 67u,
    SYMBOL_PC_E650 = 68u,
    SYMBOL_PC_E651 = 69u,
    SYMBOL_PC_E652 = 70u,
    SYMBOL_PC_E653 = 71u,
    SYMBOL_PC_E657 = 72u,
    SYMBOL_PC_E659 = 73u,
    SYMBOL_PC_E65C = 74u,
    SYMBOL_PC_E65D = 75u,
    SYMBOL_PC_E65E = 76u,
    SYMBOL_PC_E65F = 77u,
    SYMBOL_PC_E661 = 78u,
    SYMBOL_PC_E664 = 79u,
    SYMBOL_PC_E665 = 80u,
    SYMBOL_PC_E668 = 81u,
    SYMBOL_PC_E66A = 82u,
    SYMBOL_PC_E66D = 83u,
    SYMBOL_PC_E66F = 84u,
    SYMBOL_PC_E671 = 85u,
    SYMBOL_PC_E672 = 86u,
    SYMBOL_PC_E673 = 87u,
    SYMBOL_PC_E676 = 88u,
    SYMBOL_PC_E679 = 89u,
    SYMBOL_PC_E67A = 90u,
    SYMBOL_PC_E67B = 91u,
    SYMBOL_PC_E67C = 92u,
    SYMBOL_PC_E67E = 93u,
    SYMBOL_PC_E681 = 94u,
    SYMBOL_PC_E683 = 95u,
    SYMBOL_PC_E686 = 96u,
    SYMBOL_PC_E688 = 97u,
    SYMBOL_PC_E68B = 98u,
    SYMBOL_PC_E68D = 99u,
    SYMBOL_PC_E690 = 100u,
    SYMBOL_PC_E692 = 101u,
    SYMBOL_PC_E695 = 102u,
    SYMBOL_PC_E698 = 103u,
    SYMBOL_PC_E699 = 104u,
    SYMBOL_PC_E69A = 105u,
    SYMBOL_PC_E69B = 106u,
    SYMBOL_PC_E69C = 107u,
    SYMBOL_PC_E69F = 108u,
    SYMBOL_PC_E6A2 = 109u,
    SYMBOL_PC_E6A4 = 110u,
    SYMBOL_PC_E6A6 = 111u,
    SYMBOL_PC_E6A8 = 112u,
    SYMBOL_PC_E6AB = 113u,
    SYMBOL_PC_E6AE = 114u,
    SYMBOL_PC_E6B0 = 115u,
    SYMBOL_PC_E6B1 = 116u,
    SYMBOL_PC_E6B2 = 117u,
    SYMBOL_PC_E6B3 = 118u,
    SYMBOL_PC_E6B5 = 119u,
    SYMBOL_PC_E6B7 = 120u,
    SYMBOL_PC_E6B9 = 121u,
    SYMBOL_PC_E6BC = 122u,
    SYMBOL_PC_E6BE = 123u,
    SYMBOL_PC_E6C0 = 124u,
    SYMBOL_PC_E6C3 = 125u,
    SYMBOL_PC_E6C5 = 126u,
    SYMBOL_PC_E6C7 = 127u,
    SYMBOL_PC_E817 = 128u,
    SYMBOL_PC_E81A = 129u,
    SYMBOL_PC_E81D = 130u,
    SYMBOL_PC_E820 = 131u,
    SYMBOL_PC_E823 = 132u,
    SYMBOL_PC_E824 = 133u,
    SYMBOL_PC_E825 = 134u,
    SYMBOL_PC_E826 = 135u,
    SYMBOL_PC_E828 = 136u,
    SYMBOL_PC_E829 = 137u,
    SYMBOL_PC_E82A = 138u,
    SYMBOL_PC_E82C = 139u,
    SYMBOL_PC_E82E = 140u,
    SYMBOL_PC_E830 = 141u,
    SYMBOL_PC_E832 = 142u,
    SYMBOL_PC_E835 = 143u,
    SYMBOL_PC_E837 = 144u,
    SYMBOL_PC_E83A = 145u,
    SYMBOL_PC_E83D = 146u,
    SYMBOL_PC_E83E = 147u,
    SYMBOL_PC_E83F = 148u,
    SYMBOL_PC_E840 = 149u,
    SYMBOL_PC_E842 = 150u,
    SYMBOL_PC_E845 = 151u,
    SYMBOL_PC_E847 = 152u,
    SYMBOL_PC_E84A = 153u,
    SYMBOL_PC_E84C = 154u,
    SYMBOL_PC_E84F = 155u,
    SYMBOL_PC_E852 = 156u,
    SYMBOL_PC_E854 = 157u,
    SYMBOL_PC_E857 = 158u,
    SYMBOL_PC_E859 = 159u,
    SYMBOL_PC_E85C = 160u,
    SYMBOL_PC_E85E = 161u,
    SYMBOL_PC_E861 = 162u,
    SYMBOL_PC_E864 = 163u,
    SYMBOL_PC_E866 = 164u,
    SYMBOL_PC_E869 = 165u,
    SYMBOL_PC_E86B = 166u,
    SYMBOL_PC_E86E = 167u,
    SYMBOL_PC_E870 = 168u,
    SYMBOL_PC_E873 = 169u,
    SYMBOL_PC_E876 = 170u,
    SYMBOL_PC_E879 = 171u,
    SYMBOL_PC_E87C = 172u,
    SYMBOL_PC_E87F = 173u,
    SYMBOL_PC_E881 = 174u,
    SYMBOL_PC_E884 = 175u,
    SYMBOL_PC_E887 = 176u,
    SYMBOL_PC_E88A = 177u,
    SYMBOL_PC_E88C = 178u,
    SYMBOL_PC_E88F = 179u,
    SYMBOL_PC_E892 = 180u,
    SYMBOL_PC_E895 = 181u,
    SYMBOL_PC_E897 = 182u,
    SYMBOL_PC_E89A = 183u,
    SYMBOL_PC_E89D = 184u,
    SYMBOL_PC_E89F = 185u,
    SYMBOL_PC_E8A2 = 186u,
    SYMBOL_PC_E8A5 = 187u,
    SYMBOL_PC_E8A6 = 188u,
    SYMBOL_PC_E8A7 = 189u,
    SYMBOL_PC_E8A8 = 190u,
    SYMBOL_PC_E8AA = 191u,
    SYMBOL_PC_E8AD = 192u,
    SYMBOL_PC_E8B0 = 193u,
    SYMBOL_PC_E8B2 = 194u,
    SYMBOL_PC_E8B5 = 195u,
    SYMBOL_PC_E8B8 = 196u,
    SYMBOL_PC_E8BB = 197u,
    SYMBOL_PC_E8BD = 198u,
    SYMBOL_PC_E8C0 = 199u,
    SYMBOL_PC_E8C3 = 200u,
    SYMBOL_PC_E8C6 = 201u,
    SYMBOL_PC_E8C8 = 202u,
    SYMBOL_PC_E8CB = 203u,
    SYMBOL_PC_E8CE = 204u,
    SYMBOL_PC_E8D0 = 205u,
    SYMBOL_PC_E8D3 = 206u,
    SYMBOL_PC_E8D6 = 207u,
    SYMBOL_PC_E8D7 = 208u,
    SYMBOL_PC_E8D8 = 209u,
    SYMBOL_PC_E8DB = 210u,
    SYMBOL_PC_E8DE = 211u,
    SYMBOL_PC_E8E0 = 212u,
    SYMBOL_PC_E8E3 = 213u,
    SYMBOL_PC_E8E6 = 214u,
    SYMBOL_PC_E8E9 = 215u,
    SYMBOL_PC_E8EB = 216u,
    SYMBOL_PC_E8EE = 217u,
    SYMBOL_PC_E8F1 = 218u,
    SYMBOL_PC_E8F4 = 219u,
    SYMBOL_PC_E8F6 = 220u,
    SYMBOL_PC_E8F9 = 221u,
    SYMBOL_PC_E8FC = 222u,
    SYMBOL_PC_E8FE = 223u,
    SYMBOL_PC_E901 = 224u,
    SYMBOL_PC_E904 = 225u,
    SYMBOL_PC_E905 = 226u,
    SYMBOL_PC_E906 = 227u,
    SYMBOL_PC_E907 = 228u,
    SYMBOL_PC_E908 = 229u,
    SYMBOL_PC_E909 = 230u,
    SYMBOL_PC_E90A = 231u,
    SYMBOL_PC_E90B = 232u,
    SYMBOL_PC_E90E = 233u,
    SYMBOL_PC_E90F = 234u,
    SYMBOL_PC_E910 = 235u,
    SYMBOL_PC_E913 = 236u,
    SYMBOL_PC_E914 = 237u,
    SYMBOL_PC_E915 = 238u,
    SYMBOL_PC_E916 = 239u,
    SYMBOL_PC_E917 = 240u,
    SYMBOL_PC_E91A = 241u,
    SYMBOL_PC_E91B = 242u,
    SYMBOL_PC_E91D = 243u,
    SYMBOL_PC_E91F = 244u,
    SYMBOL_PC_E920 = 245u,
    SYMBOL_PC_E922 = 246u,
    SYMBOL_PC_E925 = 247u,
    SYMBOL_PC_E926 = 248u,
    SYMBOL_PC_E927 = 249u,
    SYMBOL_PC_E929 = 250u,
    SYMBOL_PC_E92A = 251u,
    SYMBOL_PC_E92D = 252u,
    SYMBOL_PC_E930 = 253u,
    SYMBOL_PC_E933 = 254u,
    SYMBOL_PC_E936 = 255u,
    SYMBOL_PC_E939 = 256u,
    SYMBOL_PC_E93C = 257u,
    SYMBOL_PC_E93F = 258u,
    SYMBOL_PC_E942 = 259u,
    SYMBOL_PC_E945 = 260u,
    SYMBOL_PC_E947 = 261u,
    SYMBOL_PC_E94A = 262u,
    SYMBOL_PC_E94D = 263u,
    SYMBOL_PC_E950 = 264u,
    SYMBOL_PC_E953 = 265u,
    SYMBOL_PC_E956 = 266u,
    SYMBOL_PC_E959 = 267u,
    SYMBOL_PC_E95C = 268u,
    SYMBOL_PC_E95F = 269u,
    SYMBOL_PC_E962 = 270u,
    SYMBOL_PC_E965 = 271u,
    SYMBOL_PC_E967 = 272u,
    SYMBOL_PC_E969 = 273u,
    SYMBOL_PC_E96A = 274u,
    SYMBOL_PC_E96B = 275u,
    SYMBOL_PC_E96C = 276u,
    SYMBOL_PC_E96F = 277u,
    SYMBOL_PC_E970 = 278u,
    SYMBOL_PC_E973 = 279u,
    SYMBOL_PC_E975 = 280u,
    SYMBOL_PC_E978 = 281u,
    SYMBOL_PC_E979 = 282u,
    SYMBOL_PC_E97D = 283u,
    SYMBOL_PC_E97F = 284u,
    SYMBOL_PC_E982 = 285u,
    SYMBOL_PC_E983 = 286u,
    SYMBOL_PC_E984 = 287u,
    SYMBOL_PC_E988 = 288u,
    SYMBOL_PC_E98B = 289u,
    SYMBOL_PC_E98C = 290u,
    SYMBOL_PC_E98D = 291u,
    SYMBOL_PC_E98E = 292u,
    SYMBOL_PC_E991 = 293u,
    SYMBOL_PC_E993 = 294u,
    SYMBOL_PC_E996 = 295u,
    SYMBOL_PC_E999 = 296u,
    SYMBOL_PC_E99B = 297u,
    SYMBOL_PC_E99D = 298u,
    SYMBOL_PC_E9A0 = 299u,
    SYMBOL_PC_E9A2 = 300u,
    SYMBOL_PC_E9A4 = 301u,
    SYMBOL_PC_E9A6 = 302u,
    SYMBOL_PC_E9A9 = 303u,
    SYMBOL_PC_E9AB = 304u,
    SYMBOL_PC_E9AE = 305u,
    SYMBOL_PC_E9B0 = 306u,
};

static const TableSymbolOpcode table_symbol_opcodes[] = {
    {{0xe5b5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5b8u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5bbu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5beu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5c1u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5c3u, 4u, 8u, 3u}, 0x0000u, 0u, 0, 0},
    {{0xe5c7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5c8u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5cau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5ccu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5cfu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5d0u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5d1u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5d2u, 3u, 5u, 2u}, 0x712au, 1u, 0, 0},
    {{0xe5d5u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5d6u, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xe5d9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5dcu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5deu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe5e1u, 3u, 5u, 2u}, 0x712au, 1u, 0, 0},
    {{0xe5e4u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5e5u, 3u, 5u, 2u}, 0x7124u, 1u, 0, 0},
    {{0xe5e8u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5ebu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5edu, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe5efu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5f1u, 3u, 5u, 2u}, 0x7000u, 1u, 0, 0},
    {{0xe5f4u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5f5u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5f8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5fau, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe5fdu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe600u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe603u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe606u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe609u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe60cu, 3u, 8u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe60fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe612u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe613u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe616u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe618u, 3u, 5u, 2u}, 0x7140u, 2u, 0, 0},
    {{0xe61bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe61du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe620u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe622u, 3u, 5u, 2u}, 0x7118u, 1u, 0, 0},
    {{0xe625u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe627u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe628u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe629u, 3u, 5u, 2u}, 0x7118u, 1u, 0, 0},
    {{0xe62cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe62eu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe62fu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe630u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe633u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe635u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe638u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe639u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe63au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe63du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe63fu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe640u, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe641u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe643u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe645u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe648u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe64bu, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe64fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe650u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe651u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe652u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe653u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe657u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe659u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe65cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe65du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe65eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe65fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe661u, 3u, 5u, 2u}, 0x7000u, 1u, 0, 0},
    {{0xe664u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe665u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe668u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe66au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe66du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe66fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe671u, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe672u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe673u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe676u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe679u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe67au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe67bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe67cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe67eu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe681u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe683u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe686u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe688u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe68bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe68du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe690u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe692u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe695u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe698u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe699u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe69au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe69bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe69cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe69fu, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe6a2u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6a4u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6a6u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6a8u, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xe6abu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6aeu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6b0u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe6b1u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe6b2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe6b3u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6b5u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe6b7u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6b9u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe6bcu, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe6beu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6c0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6c3u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6c5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6c7u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe817u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe81au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe81du, 3u, 5u, 2u}, 0x7140u, 1u, 0, 0},
    {{0xe820u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe823u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe824u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe825u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe826u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe828u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe829u, 1u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe82au, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe82cu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe82eu, 2u, 3u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe830u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe832u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe835u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe837u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe83au, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe83du, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe83eu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe83fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe840u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe842u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe845u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe847u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe84au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe84cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe84fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe852u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe854u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe857u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe859u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe85cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe85eu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe861u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe864u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe866u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe869u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe86bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe86eu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe870u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe873u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe876u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe879u, 3u, 5u, 2u}, 0x7150u, 1u, 0, 0},
    {{0xe87cu, 3u, 5u, 2u}, 0x7156u, 1u, 0, 0},
    {{0xe87fu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe881u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe884u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe887u, 3u, 5u, 2u}, 0x715cu, 1u, 0, 0},
    {{0xe88au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe88cu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe88fu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe892u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe895u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe897u, 3u, 5u, 2u}, 0x7156u, 1u, 0, 0},
    {{0xe89au, 3u, 5u, 2u}, 0x715cu, 1u, 0, 0},
    {{0xe89du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe89fu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe8a2u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8a5u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe8a6u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe8a7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe8a8u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8aau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8adu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8b0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8b2u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe8b5u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8b8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8bbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8bdu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe8c0u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8c3u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe8c6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8c8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8cbu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8ceu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8d0u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe8d3u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8d6u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe8d7u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe8d8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8dbu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8deu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8e0u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe8e3u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8e6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8e9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8ebu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe8eeu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8f1u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe8f4u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8f6u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8f9u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe8fcu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe8feu, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe901u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe904u, 1u, 4u, 1u}, 0x0000u, 0u, 0, 0},
    {{0xe905u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe906u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe907u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe908u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe909u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe90au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe90bu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe90eu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe90fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe910u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe913u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe914u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe915u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe916u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 1},
    {{0xe917u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe91au, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe91bu, 2u, 7u, 5u}, 0x0000u, 0u, 0, 0},
    {{0xe91du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe91fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe920u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe922u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe925u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe926u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe927u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe929u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe92au, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe92du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe930u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe933u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe936u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe939u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe93cu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe93fu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe942u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe945u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe947u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe94au, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe94du, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe950u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe953u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe956u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe959u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe95cu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe95fu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe962u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe965u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe967u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe969u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe96au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe96bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe96cu, 3u, 5u, 2u}, 0x7130u, 2u, 0, 0},
    {{0xe96fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe970u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe973u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe975u, 3u, 5u, 2u}, 0x7136u, 2u, 0, 0},
    {{0xe978u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe979u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe97du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe97fu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe982u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe983u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe984u, 4u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe988u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe98bu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe98cu, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe98du, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe98eu, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe991u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe993u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe996u, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe999u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe99bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe99du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9a0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9a2u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe9a4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe9a6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9a9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9abu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe9aeu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe9b0u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
};

enum { TABLE_SYMBOL_CAPACITY = 512u };
typedef struct TableSymbolClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_SYMBOL_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint32_t next;
    bool returning;
} TableSymbolClock;
static const uint32_t table_symbol_entries[]={0x84e5b5u,0x84e640u,0x84e673u,0x84e817u,0x84e829u,0x84e906u,0x84e91bu,0x84e92au};
static const uint32_t table_symbol_ends[]={0x84e63fu,0x84e672u,0x84e6c7u,0x84e828u,0x84e905u,0x84e91au,0x84e929u,0x84e9b0u};
static uint16_t TableSymbolDirectWord(const CpuState *cpu, unsigned at) {
    return cpu->ram[at] | (uint16_t)cpu->ram[at + 1u] << 8;
}
static uint16_t TableSymbolBankWord(const CpuState *cpu, uint16_t at) {
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    return cpu->ram[bank + at] | (uint16_t)cpu->ram[bank + (uint16_t)(at + 1u)] << 8;
}
static bool TableSymbolAppendOpcode(TableSymbolClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == TABLE_SYMBOL_CAPACITY) return false;
    clock->timing[clock->span.count++] = op;
    return true;
}
static bool TableSymbolRange(TableSymbolClock *clock, unsigned first, unsigned last,
                         uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableSymbolOpcode *record = table_symbol_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!TableSymbolAppendOpcode(clock, op)) return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}
static bool TableSymbolBranch(TableSymbolClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_symbol_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return TableSymbolAppendOpcode(clock, op);
}
#define LR(first,last,x,y) do { if (!TableSymbolRange(clock, SYMBOL_PC_##first, SYMBOL_PC_##last, (uint16_t)(x), (uint16_t)(y))) return false; } while (0)
#define LB(pc,taken) do { if (!TableSymbolBranch(clock, SYMBOL_PC_##pc, taken)) return false; } while (0)
#define LC(site) do { LR(site,site,0u,0u); clock->next = 0x840000u | (table_symbol_opcodes[SYMBOL_PC_##site].timing.pc + table_symbol_opcodes[SYMBOL_PC_##site].timing.fetched); clock->returning = false; return true; } while (0)


static bool PlanSymbolPrepare(TableSymbolClock *clock, uint16_t pc) {
    if (pc != 0xe640u) return false;
    const CpuState *cpu = clock->call.cpu;
    const uint16_t column = TableSymbolBankWord(cpu, 0x713cu);
    if (column > 2u) return false;
    const uint16_t x = (uint16_t)(column * 2u);
    const uint16_t previous = (uint16_t)(TableSymbolBankWord(cpu,
        (uint16_t)(0x7000u + x)) - 1u);
    const bool clamped = ((uint16_t)(previous - 5u) & 0x8000u) != 0u;
    LR(E640,E665,x,0u); LB(E668,!clamped);
    if (clamped) { LR(E66A,E66A,x,0u); }
    LR(E66D,E671,x,0u);
    return true;
}

static bool PlanSymbolRemember(TableSymbolClock *clock, uint16_t pc) {
    if (pc != 0xe817u) return false;
    LR(E817,E81A,0u,0u);
    for (unsigned row = 0u; row < 3u; ++row) {
        LR(E81D,E825,row * 2u,3u - row); LB(E826,row != 2u);
    }
    return true;
}

static bool PlanSymbolAppend(TableSymbolClock *clock, uint16_t pc) {
    if (pc != 0xe91bu) return false;
    LR(E91B,E927,TableSymbolDirectWord(clock->call.cpu,0x22u),
        clock->call.cpu->Y);
    return true;
}

static bool PlanSymbolReadColumn(TableSymbolClock *clock, uint16_t pc) {
    switch (pc) {
    case 0xe906u: LR(E906,E90A,0u,0u); LC(E90B);
    case 0xe90eu: LR(E90E,E90F,0u,0u); LC(E910);
    case 0xe913u: LR(E913,E916,0u,0u); LC(E917);
    case 0xe91au: return true;
    default: return false;
    }
}

static bool PlanSymbolFind(TableSymbolClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe673u) { LC(E673); }
    if (pc == 0xe676u) {
        const uint16_t column = TableSymbolBankWord(cpu,0x713cu);
        const bool ordinary = (TableSymbolDirectWord(cpu,0x0eu) & 8u) == 0u;
        LR(E676,E688,column * 2u,0u); LB(E68B,ordinary);
        if (!ordinary) { LR(E68D,E68D,0u,0u); }
        LR(E690,E690,0u,0u); LC(E692);
    }
    if (pc != 0xe695u) return false;
    const unsigned count = TableSymbolBankWord(cpu,0x7184u);
    if (count == 0u || count > 7u) return false;
    unsigned remaining = count * 2u - 1u;
    unsigned x = 0u;
    LR(E695,E69C,0u,0u);
    do {
        const bool type_matches = TableSymbolBankWord(cpu,(uint16_t)(0x7146u+x)) ==
            TableSymbolDirectWord(cpu,6u);
        LR(E69F,E6A2,x,remaining); LB(E6A4,!type_matches);
        if (type_matches) {
            LR(E6A6,E6AB,x,remaining);
            const bool value_matches = TableSymbolBankWord(cpu,(uint16_t)(0x7162u+x)) ==
                TableSymbolBankWord(cpu,0x716cu);
            LB(E6AE,value_matches);
            if (value_matches) return true;
        }
        LR(E6B0,E6B2,x,remaining);
        x += 2u; --remaining; LB(E6B3,remaining != 0u);
    } while (remaining);
    const uint16_t cursor = TableSymbolDirectWord(cpu,0u);
    const uint16_t attempts = TableSymbolDirectWord(cpu,4u);
    LR(E6B5,E6BC,cursor,0u); LB(E6BE,attempts != 1u);
    if (attempts != 1u) { LC(E692); }
    LR(E6C0,E6C5,cursor,0u);
    return true;
}

static bool PlanSymbolLabels(TableSymbolClock *clock, uint16_t x, uint16_t y) {
    LR(E613,E618,x,y);
    const bool stopped = TableSymbolBankWord(clock->call.cpu,
        (uint16_t)(0x7140u+y)) == 0u;
    LB(E61B,stopped);
    if (!stopped) { LR(E61D,E620,x,y); }
    LR(E622,E633,x,y); LC(E635);
}

static bool PlanSymbolStop(TableSymbolClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    if (pc == 0xe5b5u) { LR(E5B5,E5B5,0u,0u); LC(E5B8); }
    if (pc == 0xe5bbu) { LR(E5BB,E5C1,0u,0u); LC(E5C3); }
    if (pc == 0xe60cu) {
        LR(E60C,E612,0u,0u); return PlanSymbolLabels(clock,0u,0u);
    }
    if (pc == 0xe638u) {
        LR(E638,E63A,cpu->X,cpu->Y);
        const uint16_t y = (uint16_t)(cpu->Y+2u);
        LB(E63D,y!=6u);
        return y==6u || PlanSymbolLabels(clock,cpu->X,y);
    }
    if (pc != 0xe5c7u && pc != 0xe5e1u) return false;
    if (pc==0xe5c7u && TableSymbolBankWord(cpu,0x713cu)>2u) return false;
    const uint16_t x = pc==0xe5c7u ?
        (uint16_t)(TableSymbolBankWord(cpu,0x713cu)*2u) : cpu->X;
    const uint16_t candidate = pc==0xe5c7u ?
        (uint16_t)(cpu->A+TableSymbolDirectWord(cpu,2u)) :
        TableSymbolDirectWord(cpu,2u);
    if (x>4u || (x&1u)) return false;
    const uint16_t next_position = (uint16_t)(
        TableSymbolBankWord(cpu,(uint16_t)(0x712au+x)) +
        TableSymbolBankWord(cpu,(uint16_t)(0x7124u+x)));
    const bool crossing = ((uint16_t)(next_position-0x1c00u)&0x8000u)==0u;
    if (pc==0xe5c7u) {
        LR(E5C7,E5D9,x,0u); LB(E5DC,crossing);
    } else {
        LR(E5E1,E5E8,x,0u); LB(E5EB,!crossing);
    }
    if (!crossing) { LC(E5DE); }
    LR(E5ED,E5ED,x,0u);
    const bool another = ((uint16_t)(candidate-1u)&0x8000u)==0u;
    LB(E5EF,another);
    if (another) { LC(E5DE); }
    const uint16_t speed = (uint16_t)(TableSymbolBankWord(cpu,
        (uint16_t)(0x7000u+x))-1u);
    const bool clamped = ((uint16_t)(speed-5u)&0x8000u)!=0u;
    LR(E5F1,E5F5,x,0u); LB(E5F8,!clamped);
    if (clamped) { LR(E5FA,E5FA,x,0u); }
    LR(E5FD,E606,x,0u); LC(E609);
}

static bool PlanSymbolMatchRows(TableSymbolClock *clock, bool cleared) {
    const CpuState *cpu = clock->call.cpu;
    LR(E873,E876,0u,0u);
    for (unsigned row=0u;row<3u;++row) {
        const unsigned x=row*2u;
        const uint16_t first=cleared?0u:TableSymbolBankWord(cpu,(uint16_t)(0x7150u+x));
        const uint16_t second=cleared?0u:TableSymbolBankWord(cpu,(uint16_t)(0x7156u+x));
        const uint16_t third=cleared?0u:TableSymbolBankWord(cpu,(uint16_t)(0x715cu+x));
        const bool pair=first==second || first==third;
        LR(E879,E87C,x,3u-row); LB(E87F,first!=second);
        if (first==second) { LR(E881,E884,x,3u-row); }
        LR(E887,E887,x,3u-row); LB(E88A,first!=third);
        if (first==third) { LR(E88C,E88F,x,3u-row); }
        LR(E892,E892,x,3u-row); LB(E895,pair);
        if (!pair) {
            LR(E897,E89A,x,3u-row); LB(E89D,second!=third);
            if (second==third) { LR(E89F,E8A2,x,3u-row); }
        }
        LR(E8A5,E8A7,x,3u-row); LB(E8A8,row!=2u);
    }
    const uint16_t center=cleared?0u:TableSymbolBankWord(cpu,0x7156u);
    const uint16_t lower_left=cleared?0u:TableSymbolBankWord(cpu,0x7152u);
    const uint16_t upper_right=cleared?0u:TableSymbolBankWord(cpu,0x7160u);
    const bool forward_pair=lower_left==center || lower_left==upper_right;
    LR(E8AA,E8AD,6u,0u); LB(E8B0,lower_left!=center);
    if (lower_left==center) { LR(E8B2,E8B5,6u,0u); }
    LR(E8B8,E8B8,6u,0u); LB(E8BB,lower_left!=upper_right);
    if (lower_left==upper_right) { LR(E8BD,E8C0,6u,0u); }
    LR(E8C3,E8C3,6u,0u); LB(E8C6,forward_pair);
    if (!forward_pair) {
        LR(E8C8,E8CB,6u,0u); LB(E8CE,center!=upper_right);
        if (center==upper_right) { LR(E8D0,E8D3,6u,0u); }
    }
    LR(E8D6,E8DB,6u,0u);
    const uint16_t upper_left=cleared?0u:TableSymbolBankWord(cpu,0x715eu);
    const uint16_t lower_right=cleared?0u:TableSymbolBankWord(cpu,0x7154u);
    const bool backward_pair=upper_left==center || upper_left==lower_right;
    LB(E8DE,upper_left!=center);
    if (upper_left==center) { LR(E8E0,E8E3,8u,0u); }
    LR(E8E6,E8E6,8u,0u); LB(E8E9,upper_left!=lower_right);
    if (upper_left==lower_right) { LR(E8EB,E8EE,8u,0u); }
    LR(E8F1,E8F1,8u,0u); LB(E8F4,backward_pair);
    if (!backward_pair) {
        LR(E8F6,E8F9,8u,0u); LB(E8FC,center!=lower_right);
        if (center==lower_right) { LR(E8FE,E901,8u,0u); }
    }
    LR(E904,E904,8u,0u);
    return true;
}

static bool PlanSymbolMatches(TableSymbolClock *clock, uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    const bool cleared=pc==0xe829u;
    if (cleared) {
        LR(E829,E837,0u,0u);
        for (unsigned n=0u;n<19u;++n) {
            LR(E83A,E83F,n*2u,0u); LB(E840,n!=18u);
        }
        LR(E842,E842,0u,0u);
        const bool empty=TableSymbolBankWord(cpu,0x717eu)==0u;
        LB(E845,empty);
        if (!empty) { LR(E847,E84A,0u,0u); LC(E84C); }
    } else if (pc!=0xe84fu && pc!=0xe861u && pc!=0xe873u) return false;
    if (cleared || pc==0xe84fu) {
        LR(E84F,E854,0u,0u);
        const bool empty=TableSymbolBankWord(cpu,0x7180u)==0u;
        LB(E857,empty);
        if (!empty) { LR(E859,E85C,0u,0u); LC(E85E); }
    }
    if (cleared || pc==0xe84fu || pc==0xe861u) {
        LR(E861,E866,0u,0u);
        const bool empty=TableSymbolBankWord(cpu,0x7182u)==0u;
        LB(E869,empty);
        if (!empty) { LR(E86B,E86E,0u,0u); LC(E870); }
    }
    return PlanSymbolMatchRows(clock,cleared);
}

static bool PlanSymbolStart(TableSymbolClock *clock, uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    if (pc==0xe92au) { LR(E92A,E930,0u,0u); LC(E933); }
    if (pc==0xe936u) { LR(E936,E93F,0u,0u); LC(E942); }
    const bool initial=pc==0xe945u;
    if (!initial && pc!=0xe9aeu) return false;
    unsigned finished=initial?0u:TableSymbolDirectWord(cpu,0u);
    if (finished>=3u) return false;
    if (initial) { LR(E945,E962,0u,0u); }
    else { LR(E9AE,E9AE,0u,0u); }
    LR(E965,E967,0u,0u);
    for (unsigned column=finished;column<3u;++column) {
        const unsigned y=column*2u;
        const uint16_t timer=initial?8u:TableSymbolBankWord(cpu,(uint16_t)(0x7130u+y));
        LR(E969,E970,0u,y); LB(E973,timer!=1u);
        if (timer==1u) {
            const uint16_t x=TableSymbolBankWord(cpu,(uint16_t)(0x7136u+y));
            if (x>28u || (x&3u)) return false;
            const unsigned rom_at=0x271f5u+x;
            if (rom_at+3u>=g_snes->cart->romSize) return false;
            const uint8_t *table=g_snes->cart->rom+rom_at;
            const uint16_t duration=table[0]|(uint16_t)table[1]<<8;
            LR(E975,E979,x,y); LB(E97D,duration==0u);
            if (duration) { LR(E97F,E991,x,y); }
            else {
                LR(E993,E99D,x,y); ++finished; LB(E9A0,finished==3u);
                if (finished==3u) return true;
            }
        }
        LR(E9A2,E9A6,0u,y); LB(E9A9,column!=2u);
    }
    LC(E9AB);
}

#undef LR
#undef LB
#undef LC
static uint64_t TableSymbolBudget(const TableSymbolClock *clock) {
    uint64_t budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    return budget + 40u * (budget / 1324u + 1u);
}
static bool PrepareTableSymbolClock(TableSymbolClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = table_symbol_ends[clock->root];
    clock->returning = true;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x28000u || cpu->PB != 0x84u || pc >> 16 != 0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        clock->call.frame.hrv != 2u || cpu->host_return_valid < 2u || cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const bool entry = pc == table_symbol_entries[clock->root];
    if ((entry && cpu->host_return_valid != 2u) || cpu->x_flag || cpu->m_flag) return false;
    bool planned;
    switch (clock->root) {
    case 0u: planned = PlanSymbolStop(clock, (uint16_t)pc); break;
    case 1u: planned = PlanSymbolPrepare(clock, (uint16_t)pc); break;
    case 2u: planned = PlanSymbolFind(clock, (uint16_t)pc); break;
    case 3u: planned = PlanSymbolRemember(clock, (uint16_t)pc); break;
    case 4u: planned = PlanSymbolMatches(clock, (uint16_t)pc); break;
    case 5u: planned = PlanSymbolReadColumn(clock, (uint16_t)pc); break;
    case 6u: planned = PlanSymbolAppend(clock, (uint16_t)pc); break;
    default: planned = PlanSymbolStart(clock, (uint16_t)pc); break;
    }
    if (!planned) return false;
    const uint64_t budget = TableSymbolBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, table_symbol_entries[clock->root],
            0x1ff7u, true, 1363u, budget)) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
static uint8_t TableSymbolChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableSymbolClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(!clock->returning && clock->next == site + frame_size + 1u);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableSymbolClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}
static RecompReturn ActorBridgeTableSymbol(CpuState *cpu, unsigned root) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableSymbolClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTableSymbolClock(&clock, table_symbol_entries[root]))
        return ActorBridgeFallback(cpu, &frame, table_symbol_entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result = Lufia2CardTableStopColumn(&memory, &state, TableSymbolChild, &clock); break;
    case 1u: result = Lufia2CardTablePrepareColumnSymbols(&memory, &state); break;
    case 2u: result = Lufia2CardTableFindMatchingSymbol(&memory, &state, TableSymbolChild, &clock); break;
    case 3u: result = Lufia2CardTableRememberRoundSymbol(&memory, &state); break;
    case 4u: result = Lufia2CardTableBuildColumnMatches(&memory, &state, TableSymbolChild, &clock); break;
    case 5u: result = Lufia2CardTableReadMatchColumn(&memory, &state, TableSymbolChild, &clock); break;
    case 6u: result = Lufia2CardTableAppendMatchSymbol(&memory, &state); break;
    default: result = Lufia2CardTableStartColumns(&memory, &state, TableSymbolChild, &clock); break;
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
