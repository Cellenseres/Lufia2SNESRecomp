typedef struct TableSearchOpcode {
    FixedSpanOpcode timing;
    uint16_t read_base;
    uint8_t read_index;
    int8_t advance_x, advance_y;
} TableSearchOpcode;

enum {
    SEARCH_PC_E6C8 = 0u,
    SEARCH_PC_E6CB = 1u,
    SEARCH_PC_E6CD = 2u,
    SEARCH_PC_E6D0 = 3u,
    SEARCH_PC_E6D2 = 4u,
    SEARCH_PC_E6D5 = 5u,
    SEARCH_PC_E6D7 = 6u,
    SEARCH_PC_E6D9 = 7u,
    SEARCH_PC_E6DA = 8u,
    SEARCH_PC_E6DD = 9u,
    SEARCH_PC_E6E0 = 10u,
    SEARCH_PC_E6E2 = 11u,
    SEARCH_PC_E6E3 = 12u,
    SEARCH_PC_E6E4 = 13u,
    SEARCH_PC_E6E5 = 14u,
    SEARCH_PC_E6E7 = 15u,
    SEARCH_PC_E6E8 = 16u,
    SEARCH_PC_E6EB = 17u,
    SEARCH_PC_E6EE = 18u,
    SEARCH_PC_E6F0 = 19u,
    SEARCH_PC_E6F2 = 20u,
    SEARCH_PC_E6F5 = 21u,
    SEARCH_PC_E6F6 = 22u,
    SEARCH_PC_E6F9 = 23u,
    SEARCH_PC_E6FB = 24u,
    SEARCH_PC_E6FD = 25u,
    SEARCH_PC_E700 = 26u,
    SEARCH_PC_E703 = 27u,
    SEARCH_PC_E705 = 28u,
    SEARCH_PC_E708 = 29u,
    SEARCH_PC_E70B = 30u,
    SEARCH_PC_E70C = 31u,
    SEARCH_PC_E70E = 32u,
    SEARCH_PC_E711 = 33u,
    SEARCH_PC_E713 = 34u,
    SEARCH_PC_E714 = 35u,
    SEARCH_PC_E715 = 36u,
    SEARCH_PC_E716 = 37u,
    SEARCH_PC_E718 = 38u,
    SEARCH_PC_E71B = 39u,
    SEARCH_PC_E71D = 40u,
    SEARCH_PC_E720 = 41u,
    SEARCH_PC_E722 = 42u,
    SEARCH_PC_E725 = 43u,
    SEARCH_PC_E728 = 44u,
    SEARCH_PC_E72A = 45u,
    SEARCH_PC_E72C = 46u,
    SEARCH_PC_E72F = 47u,
    SEARCH_PC_E730 = 48u,
    SEARCH_PC_E732 = 49u,
    SEARCH_PC_E735 = 50u,
    SEARCH_PC_E737 = 51u,
    SEARCH_PC_E738 = 52u,
    SEARCH_PC_E739 = 53u,
    SEARCH_PC_E73B = 54u,
    SEARCH_PC_E73D = 55u,
    SEARCH_PC_E73F = 56u,
    SEARCH_PC_E740 = 57u,
    SEARCH_PC_E742 = 58u,
    SEARCH_PC_E745 = 59u,
    SEARCH_PC_E747 = 60u,
    SEARCH_PC_E74A = 61u,
    SEARCH_PC_E74C = 62u,
    SEARCH_PC_E74E = 63u,
    SEARCH_PC_E74F = 64u,
    SEARCH_PC_E752 = 65u,
    SEARCH_PC_E755 = 66u,
    SEARCH_PC_E757 = 67u,
    SEARCH_PC_E75A = 68u,
    SEARCH_PC_E75C = 69u,
    SEARCH_PC_E75F = 70u,
    SEARCH_PC_E761 = 71u,
    SEARCH_PC_E763 = 72u,
    SEARCH_PC_E766 = 73u,
    SEARCH_PC_E769 = 74u,
    SEARCH_PC_E76B = 75u,
    SEARCH_PC_E76D = 76u,
    SEARCH_PC_E770 = 77u,
    SEARCH_PC_E772 = 78u,
    SEARCH_PC_E773 = 79u,
    SEARCH_PC_E776 = 80u,
    SEARCH_PC_E779 = 81u,
    SEARCH_PC_E77B = 82u,
    SEARCH_PC_E77E = 83u,
    SEARCH_PC_E780 = 84u,
    SEARCH_PC_E783 = 85u,
    SEARCH_PC_E786 = 86u,
    SEARCH_PC_E789 = 87u,
    SEARCH_PC_E78B = 88u,
    SEARCH_PC_E78D = 89u,
    SEARCH_PC_E78F = 90u,
    SEARCH_PC_E792 = 91u,
    SEARCH_PC_E795 = 92u,
    SEARCH_PC_E798 = 93u,
    SEARCH_PC_E799 = 94u,
    SEARCH_PC_E79A = 95u,
    SEARCH_PC_E79B = 96u,
    SEARCH_PC_E79D = 97u,
    SEARCH_PC_E7A0 = 98u,
    SEARCH_PC_E7A2 = 99u,
    SEARCH_PC_E7A5 = 100u,
    SEARCH_PC_E7A7 = 101u,
    SEARCH_PC_E7AA = 102u,
    SEARCH_PC_E7AD = 103u,
    SEARCH_PC_E7B0 = 104u,
    SEARCH_PC_E7B3 = 105u,
    SEARCH_PC_E7B6 = 106u,
    SEARCH_PC_E7B8 = 107u,
    SEARCH_PC_E7B9 = 108u,
    SEARCH_PC_E7BA = 109u,
    SEARCH_PC_E7BB = 110u,
    SEARCH_PC_E7BD = 111u,
    SEARCH_PC_E7C0 = 112u,
    SEARCH_PC_E7C2 = 113u,
    SEARCH_PC_E7C4 = 114u,
    SEARCH_PC_E7C6 = 115u,
    SEARCH_PC_E7C9 = 116u,
    SEARCH_PC_E7CB = 117u,
    SEARCH_PC_E7CD = 118u,
    SEARCH_PC_E7CF = 119u,
    SEARCH_PC_E7D0 = 120u,
    SEARCH_PC_E7D3 = 121u,
    SEARCH_PC_E7D6 = 122u,
    SEARCH_PC_E7D9 = 123u,
    SEARCH_PC_E7DC = 124u,
    SEARCH_PC_E7DE = 125u,
    SEARCH_PC_E7E1 = 126u,
    SEARCH_PC_E7E4 = 127u,
    SEARCH_PC_E7E6 = 128u,
    SEARCH_PC_E7E8 = 129u,
    SEARCH_PC_E7E9 = 130u,
    SEARCH_PC_E7EA = 131u,
    SEARCH_PC_E7EB = 132u,
    SEARCH_PC_E7ED = 133u,
    SEARCH_PC_E7EF = 134u,
    SEARCH_PC_E7F1 = 135u,
    SEARCH_PC_E7F3 = 136u,
    SEARCH_PC_E7F6 = 137u,
    SEARCH_PC_E7F7 = 138u,
    SEARCH_PC_E7F8 = 139u,
    SEARCH_PC_E7F9 = 140u,
    SEARCH_PC_E7FA = 141u,
    SEARCH_PC_E7FC = 142u,
    SEARCH_PC_E7FD = 143u,
    SEARCH_PC_E800 = 144u,
    SEARCH_PC_E802 = 145u,
    SEARCH_PC_E805 = 146u,
    SEARCH_PC_E806 = 147u,
    SEARCH_PC_E809 = 148u,
    SEARCH_PC_E80B = 149u,
    SEARCH_PC_E80C = 150u,
    SEARCH_PC_E80D = 151u,
    SEARCH_PC_E80E = 152u,
    SEARCH_PC_E810 = 153u,
    SEARCH_PC_E812 = 154u,
    SEARCH_PC_E814 = 155u,
    SEARCH_PC_E816 = 156u,
};

static const TableSearchOpcode table_search_opcodes[] = {
    {{0xe6c8u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6cbu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6cdu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6d0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6d2u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6d5u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6d7u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6d9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6dau, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6ddu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6e0u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6e2u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6e3u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6e4u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6e5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6e7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6e8u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6ebu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6eeu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6f0u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe6f2u, 3u, 5u, 2u}, 0x7006u, 2u, 0, 0},
    {{0xe6f5u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6f6u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6f9u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe6fbu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe6fdu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe700u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe703u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe705u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe708u, 3u, 5u, 2u}, 0x7006u, 2u, 0, 0},
    {{0xe70bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe70cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe70eu, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe711u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe713u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe714u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe715u, 1u, 2u, 0u}, 0x0000u, 0u, -1, 0},
    {{0xe716u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe718u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe71bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe71du, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe720u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe722u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe725u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe728u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe72au, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe72cu, 3u, 5u, 2u}, 0x7006u, 2u, 0, 0},
    {{0xe72fu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe730u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe732u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe735u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe737u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe738u, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe739u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe73bu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe73du, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe73fu, 1u, 2u, 0u}, 0x0000u, 0u, -1, 0},
    {{0xe740u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe742u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe745u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe747u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe74au, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe74cu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe74eu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe74fu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe752u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe755u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe757u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe75au, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe75cu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe75fu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe761u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe763u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe766u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe769u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe76bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe76du, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe770u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe772u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe773u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe776u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe779u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe77bu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe77eu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe780u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe783u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe786u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe789u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe78bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe78du, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe78fu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe792u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe795u, 3u, 5u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe798u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe799u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe79au, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe79bu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe79du, 3u, 7u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7a0u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7a2u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7a5u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7a7u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7aau, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7adu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7b0u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe7b3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7b6u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7b8u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe7b9u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe7bau, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe7bbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7bdu, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7c0u, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7c2u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe7c4u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7c6u, 3u, 10u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe7c9u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe7cbu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7cdu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7cfu, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7d0u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7d3u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7d6u, 3u, 5u, 2u}, 0x7146u, 1u, 0, 0},
    {{0xe7d9u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7dcu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7deu, 3u, 5u, 2u}, 0x7162u, 1u, 0, 0},
    {{0xe7e1u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7e4u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7e6u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe7e8u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe7e9u, 1u, 2u, 0u}, 0x0000u, 0u, 1, 0},
    {{0xe7eau, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe7ebu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7edu, 2u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7efu, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7f1u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7f3u, 3u, 6u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7f6u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7f7u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7f8u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7f9u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7fau, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe7fcu, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe7fdu, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe800u, 2u, 7u, 4u}, 0x0000u, 0u, 0, 0},
    {{0xe802u, 3u, 5u, 2u}, 0x7006u, 2u, 0, 0},
    {{0xe805u, 1u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe806u, 3u, 3u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe809u, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe80bu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe80cu, 1u, 2u, 0u}, 0x0000u, 0u, 0, -1},
    {{0xe80du, 1u, 2u, 0u}, 0x0000u, 0u, -1, 0},
    {{0xe80eu, 2u, 2u, 0u}, 0x0000u, 0u, 0, 0},
    {{0xe810u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe812u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe814u, 2u, 4u, 2u}, 0x0000u, 0u, 0, 0},
    {{0xe816u, 1u, 6u, 0u}, 0x0000u, 0u, 0, 0},
};

enum { TABLE_SEARCH_CAPACITY = 512u };
typedef struct TableSearchClock {
    FixedSpanClock span;
    FixedSpanOpcode timing[TABLE_SEARCH_CAPACITY];
    ActorPushedCall call;
    unsigned root;
    uint32_t next;
    bool returning;
} TableSearchClock;
static const uint32_t table_search_entries[]={0x84e6c8u,0x84e74fu,0x84e773u,0x84e7efu};
static const uint32_t table_search_ends[]={0x84e74eu,0x84e772u,0x84e7cfu,0x84e816u};
static uint16_t TableSearchDirectWord(const CpuState *cpu, unsigned at) {
    return cpu->ram[at] | (uint16_t)cpu->ram[at + 1u] << 8;
}
static uint16_t TableSearchBankWord(const CpuState *cpu, uint16_t at) {
    const unsigned bank = cpu->DB == 0x7fu ? 0x10000u : 0u;
    return cpu->ram[bank + at] | (uint16_t)cpu->ram[bank + (uint16_t)(at + 1u)] << 8;
}
static bool TableSearchAppendOpcode(TableSearchClock *clock, FixedSpanOpcode op) {
    if (clock->span.count == TABLE_SEARCH_CAPACITY) return false;
    clock->timing[clock->span.count++] = op;
    return true;
}
static bool TableSearchRange(TableSearchClock *clock, unsigned first, unsigned last,
                         uint16_t x, uint16_t y) {
    for (unsigned index = first; index <= last; ++index) {
        const TableSearchOpcode *record = table_search_opcodes + index;
        FixedSpanOpcode op = record->timing;
        if (record->read_index) {
            const uint16_t offset = record->read_index == 1u ? x : y;
            op.cycles = (uint8_t)(op.cycles +
                ((record->read_base >> 8) != ((record->read_base + offset) >> 8)));
        }
        if (!TableSearchAppendOpcode(clock, op)) return false;
        x = (uint16_t)(x + record->advance_x);
        y = (uint16_t)(y + record->advance_y);
    }
    return true;
}
static bool TableSearchBranch(TableSearchClock *clock, unsigned index, bool taken) {
    FixedSpanOpcode op = table_search_opcodes[index].timing;
    op.cycles = taken ? 3u : 2u;
    return TableSearchAppendOpcode(clock, op);
}
#define LR(first,last,x,y) do { if (!TableSearchRange(clock, SEARCH_PC_##first, SEARCH_PC_##last, (uint16_t)(x), (uint16_t)(y))) return false; } while (0)
#define LB(pc,taken) do { if (!TableSearchBranch(clock, SEARCH_PC_##pc, taken)) return false; } while (0)
#define LC(site) do { LR(site,site,0u,0u); clock->next = 0x840000u | (table_search_opcodes[SEARCH_PC_##site].timing.pc + table_search_opcodes[SEARCH_PC_##site].timing.fetched); clock->returning = false; return true; } while (0)


static bool SearchSourceReady(const CpuState *cpu) {
    const uint16_t source=TableSearchDirectWord(cpu,8u);
    return source==0u || source==58u || source==116u;
}

static bool SearchSymbol(const CpuState *cpu,uint16_t y,uint16_t *symbol) {
    const uint32_t address=((uint32_t)cpu->DB<<16)+0x7006u+y;
    if (address<0x7e0000u || address>=0x7fffffu) return false;
    const uint32_t offset=address-0x7e0000u;
    *symbol=(uint16_t)((cpu->ram[offset]|(uint16_t)cpu->ram[offset+1u]<<8)+1u);
    return true;
}

static bool PlanSearchPreferredEnd(TableSearchClock *clock) {
    LR(E745,E747,0u,0u);
    const bool ordinary=(TableSearchDirectWord(clock->call.cpu,0x0eu)&8u)==0u;
    LB(E74A,ordinary);
    if (!ordinary) { LR(E74C,E74C,0u,0u); }
    return true;
}

static bool PlanSearchPreferred(TableSearchClock *clock,uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    if (pc==0xe6c8u) { LC(E6C8); }
    if (pc==0xe745u) return PlanSearchPreferredEnd(clock);
    if (pc==0xe6d5u) { LR(E6D5,E6D5,0u,0u); return PlanSearchPreferredEnd(clock); }
    if (pc!=0xe6cbu) return false;
    const uint16_t mode=TableSearchBankWord(cpu,0x716eu);
    LR(E6CB,E6CD,0u,0u); LB(E6D0,mode!=0u);
    if (!mode) { LC(E6D2); }
    const uint16_t cursor=TableSearchDirectWord(cpu,0u);
    const unsigned count=TableSearchBankWord(cpu,0x7184u);
    if (cursor>25u || count<1u || count>3u || !SearchSourceReady(cpu)) return false;
    LR(E6D7,E6DD,count,0u); LB(E6E0,count==1u);
    if (count!=1u) { LR(E6E2,E6E2,count,0u); }
    uint16_t y=(uint16_t)((cursor+1u+(count!=1u))*2u+TableSearchDirectWord(cpu,8u));
    LR(E6E3,E6EB,count,y); LB(E6EE,(mode&0xff00u)==0u);
    if (mode&0xff00u) {
        LR(E6F0,E6F6,count,y);
        uint16_t symbol; if (!SearchSymbol(cpu,y,&symbol)) return false;
        const bool seventh=symbol==7u;
        LB(E6F9,seventh);
        if (seventh) return PlanSearchPreferredEnd(clock);
        LR(E6FB,E6FB,count,y);
    }
    LR(E6FD,E700,count,y); LB(E703,count==1u);
    unsigned remaining=count==1u?1u:3u;
    if (count!=1u) { LR(E705,E705,remaining,y); }
    const uint16_t mask=TableSearchDirectWord(cpu,0x0eu);
    const uint16_t desired=TableSearchBankWord(cpu,0x716cu);
    do {
        LR(E708,E70E,remaining,y);
        uint16_t symbol; if (!SearchSymbol(cpu,y,&symbol)) return false;
        const bool matched=(symbol&mask)==desired;
        LB(E711,matched);
        if (matched) return PlanSearchPreferredEnd(clock);
        LR(E713,E715,remaining,y); y=(uint16_t)(y-2u); --remaining;
        LB(E716,remaining!=0u);
    } while (remaining);
    LR(E718,E71D,0u,y); LB(E720,(mask&8u)==0u);
    remaining=(mask&8u)?10u:5u;
    if (mask&8u) { LR(E722,E722,remaining,y); }
    LR(E725,E728,remaining,y);
    unsigned attempts=4u;
    do {
        LR(E72A,E732,remaining,y);
        uint16_t symbol; if (!SearchSymbol(cpu,y,&symbol)) return false;
        const bool matched=(symbol&mask)==desired;
        LB(E735,matched);
        if (matched) return PlanSearchPreferredEnd(clock);
        LR(E737,E739,remaining,y); y=(uint16_t)(y-2u);
        const bool nonnegative=attempts!=0u; LB(E73B,nonnegative);
        if (nonnegative) --attempts;
        else { LR(E73D,E73D,remaining,y); }
        LR(E73F,E73F,remaining,y); --remaining; LB(E740,remaining!=0u);
    } while (remaining);
    LC(E742);
}

static bool PlanSearchSingle(TableSearchClock *clock,uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    if (pc==0xe74fu) { LC(E74F); }
    if (pc==0xe770u) { LR(E770,E770,0u,0u); return true; }
    if (pc==0xe752u) {
        LR(E752,E752,0u,0u);
        const bool disabled=TableSearchBankWord(cpu,0x716eu)==0u;
        LB(E755,disabled);
        if (!disabled) { LR(E757,E75A,0u,0u); LC(E75C); }
    } else if (pc==0xe75fu) {
        LR(E75F,E75F,0u,0u);
        const bool found=TableSearchDirectWord(cpu,0x0bu)==0u;
        LB(E761,found);
        if (found) { LR(E770,E770,0u,0u); return true; }
        LR(E763,E766,0u,0u);
        const bool low_mode=(TableSearchBankWord(cpu,0x716eu)&0xff00u)==0u;
        LB(E769,low_mode);
        if (low_mode) { LR(E770,E770,0u,0u); return true; }
    } else return false;
    LR(E76B,E76B,0u,0u); LC(E76D);
}

static bool PlanSearchDouble(TableSearchClock *clock,uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    if (pc==0xe773u) { LC(E773); }
    if (pc==0xe776u) {
        LR(E776,E776,0u,0u);
        const bool disabled=TableSearchBankWord(cpu,0x716eu)==0u;
        LB(E779,disabled);
        if (disabled) { LC(E792); }
        LR(E77B,E77E,0u,0u); LC(E780);
    }
    if (pc==0xe783u) {
        LR(E783,E786,0u,0u);
        const bool low_mode=(TableSearchBankWord(cpu,0x716eu)&0xff00u)==0u;
        LB(E789,low_mode);
        if (low_mode) {
            LR(E7D0,E7D3,0u,0u);
            for (unsigned row=0u;row<5u;++row) {
                const unsigned x=row*2u;
                const bool pair=TableSearchBankWord(cpu,(uint16_t)(0x7146u+x))==2u;
                LR(E7D6,E7D9,x,5u-row); LB(E7DC,!pair);
                if (pair) {
                    const bool unit=TableSearchBankWord(cpu,(uint16_t)(0x7162u+x))==1u;
                    LR(E7DE,E7E1,x,5u-row); LB(E7E4,!unit);
                    if (unit) { LR(E7E6,E7E6,x,5u-row); }
                }
                LR(E7E8,E7EA,x,5u-row); LB(E7EB,row!=4u);
            }
            LR(E7ED,E7ED,0u,0u); LR(E7CD,E7CD,0u,0u); return true;
        }
        LR(E78B,E78B,0u,0u);
        const bool found=TableSearchDirectWord(cpu,0x0bu)==0u;
        LB(E78D,found);
        if (found) { LR(E7CD,E7CD,0u,0u); return true; }
        LC(E78F);
    }
    if (pc==0xe792u) { LC(E792); }
    if (pc==0xe795u) {
        const unsigned column=TableSearchBankWord(cpu,0x713cu);
        if (column>2u) return false;
        LR(E795,E7A5,column*2u,0u); LC(E7A7);
    }
    if (pc==0xe7c0u) { LR(E7C0,E7C0,0u,0u); LR(E7CD,E7CD,0u,0u); return true; }
    if (pc!=0xe7aau) return false;
    LR(E7AA,E7AD,0u,0u);
    for (unsigned row=0u;row<5u;++row) {
        const unsigned x=row*2u;
        const bool pair=TableSearchBankWord(cpu,(uint16_t)(0x7146u+x))==2u;
        LR(E7B0,E7B3,x,5u-row); LB(E7B6,pair);
        if (pair) {
            const uint16_t cursor=TableSearchDirectWord(cpu,0u);
            if (cursor>4u || cursor&1u) return false;
            LR(E7C2,E7C9,cursor,0u);
            const bool another=TableSearchDirectWord(cpu,4u)!=1u;
            LB(E7CB,another);
            if (another) { LC(E7A7); }
            LR(E7CD,E7CD,0u,0u); return true;
        }
        LR(E7B8,E7BA,x,5u-row); LB(E7BB,row!=4u);
    }
    LC(E7BD);
}

static bool PlanSearchSeventh(TableSearchClock *clock,uint16_t pc) {
    const CpuState *cpu=clock->call.cpu;
    if (pc==0xe7efu) { LR(E7EF,E7F1,0u,0u); LC(E7F3); }
    if (pc!=0xe7f6u || cpu->A>25u || !SearchSourceReady(cpu)) return false;
    uint16_t y=(uint16_t)((cpu->A+2u)*2u+TableSearchDirectWord(cpu,8u));
    LR(E7F6,E7FD,0u,y);
    for (unsigned left=3u;left>0u;--left) {
        LR(E800,E806,left,y); uint16_t symbol; if (!SearchSymbol(cpu,y,&symbol)) return false;
        const bool seventh=symbol==7u;
        LB(E809,seventh);
        if (seventh) { LR(E814,E814,left,y); return true; }
        LR(E80B,E80D,left,y); y=(uint16_t)(y-2u); LB(E80E,left!=1u);
    }
    LR(E810,E814,0u,y); return true;
}

#undef LR
#undef LB
#undef LC
static uint64_t TableSearchBudget(const TableSearchClock *clock) {
    uint64_t budget = 64u;
    for (unsigned i = 0u; i < clock->span.count; ++i) {
        const FixedSpanOpcode *op = clock->timing + i;
        budget += op->fetched * cpu_region_speed(0x840000u | op->pc) +
            op->data_bytes * 12u + (op->cycles - op->fetched - op->data_bytes) * 6u;
    }
    return budget + 40u * (budget / 1324u + 1u);
}
static bool PrepareTableSearchClock(TableSearchClock *clock, uint32_t pc) {
    const CpuState *cpu = clock->call.cpu;
    clock->span = (FixedSpanClock){0};
    clock->span.cpu = clock->call.cpu;
    clock->span.bank = 0x840000u;
    clock->span.timing = clock->timing;
    clock->next = table_search_ends[clock->root];
    clock->returning = true;
    if (!cpu->ram || !g_snes || !g_snes->dma || !g_snes->cart ||
        !g_snes->cart->rom || g_snes->cart->type != CART_LOROM ||
        g_snes->cart->romSize < 0x28000u || cpu->PB != 0x84u || pc >> 16 != 0x84u ||
        cpu->emulation || cpu->D || cpu->_flag_D || cpu->m_flag > 1u || cpu->x_flag > 1u ||
        clock->call.frame.hrv != 2u || cpu->host_return_valid < 2u || cpu->host_return_valid > 3u || cpu->S < 0x1f20u || cpu->S > 0x1ff7u ||
        (cpu->DB != 0x7eu && cpu->DB != 0x7fu)) return false;
    const bool entry = pc == table_search_entries[clock->root];
    if ((entry && cpu->host_return_valid != 2u) || cpu->x_flag || cpu->m_flag) return false;
    bool planned;
    switch (clock->root) {
    case 0u: planned=PlanSearchPreferred(clock,(uint16_t)pc); break;
    case 1u: planned=PlanSearchSingle(clock,(uint16_t)pc); break;
    case 2u: planned=PlanSearchDouble(clock,(uint16_t)pc); break;
    default: planned=PlanSearchSeventh(clock,(uint16_t)pc); break;
    }
    if (!planned) return false;
    const uint64_t budget = TableSearchBudget(clock);
    if (cpu->coprocessor_master_cycles != cpu->master_cycles ||
        cpu->master_cycles > UINT64_MAX - budget || !g_snes->hdmaBeamOff ||
        g_snes->vPos >= 262u || g_snes->hPos >= 1364u ||
        !ActorBridgeTimedStepSupportedWindow(cpu, table_search_entries[clock->root],
            0x1ff7u, true, 1363u, budget)) return false;
    const unsigned lines = (g_snes->vPos < 225u ? 225u : 262u) - g_snes->vPos;
    if ((uint64_t)lines * 1364u - g_snes->hPos <= budget) return false;
    uint64_t phase, current;
    snes_refresh_state_get(&phase, &current);
    return phase < 1364u && (!current || current == cpu->master_cycles);
}
static uint8_t TableSearchChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    TableSearchClock *clock = context;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    assert(!clock->returning && clock->next == site + frame_size + 1u);
    if (!ActorBridgePushedChild(&clock->call, state, target, site, frame_size)) return 0u;
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    if (PrepareTableSearchClock(clock, landing)) return 1u;
    clock->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        clock->call.cpu, landing, landing, clock->call.frame.entry_s,
        clock->call.frame.hrv) + 1);
    return 0u;
}
static RecompReturn ActorBridgeTableSearch(CpuState *cpu, unsigned root) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    TableSearchClock clock;
    clock.root = root;
    clock.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!PrepareTableSearchClock(&clock, table_search_entries[root]))
        return ActorBridgeFallback(cpu, &frame, table_search_entries[root]);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite, &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    Lufia2ExecutionResult result;
    switch (root) {
    case 0u: result = Lufia2CardTableChoosePreferredSymbol(&memory, &state, TableSearchChild, &clock); break;
    case 1u: result = Lufia2CardTableChooseSingleMatch(&memory, &state, TableSearchChild, &clock); break;
    case 2u: result = Lufia2CardTableChooseDoubleMatch(&memory, &state, TableSearchChild, &clock); break;
    default: result = Lufia2CardTableScanSeventhSymbol(&memory, &state, TableSearchChild, &clock); break;
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
