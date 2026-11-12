#include "common_rtl.h"
#include "snes/apu.h"

typedef struct BattleLeafCost {
    unsigned cycles;
    unsigned rom_bytes;
    unsigned ram_bytes;
} BattleLeafCost;

static const BattleLeafCost battle_oam_costs[4][11] = {
    {
        {14u, 7u, 3u},
        {158u, 88u, 31u},
        {336u, 190u, 66u},
        {538u, 304u, 107u},
        {233u, 123u, 52u},
        {386u, 210u, 81u},
        {564u, 312u, 116u},
        {766u, 426u, 157u},
        {461u, 245u, 102u},
        {614u, 332u, 131u},
        {792u, 434u, 166u},
    },
    {
        {14u, 7u, 3u},
        {182u, 100u, 37u},
        {384u, 214u, 78u},
        {610u, 340u, 125u},
        {771u, 436u, 155u},
        {956u, 544u, 191u},
        {1165u, 664u, 233u},
        {860u, 483u, 178u},
        {1013u, 570u, 207u},
        {1191u, 672u, 242u},
        {1393u, 786u, 283u},
    },
    {
        {14u, 7u, 3u},
        {206u, 112u, 43u},
        {432u, 238u, 90u},
        {586u, 328u, 119u},
        {771u, 436u, 155u},
        {980u, 556u, 197u},
        {675u, 375u, 142u},
        {828u, 462u, 171u},
        {1006u, 564u, 206u},
        {1208u, 678u, 247u},
        {903u, 497u, 192u},
    },
    {
        {14u, 7u, 3u},
        {230u, 124u, 49u},
        {384u, 214u, 78u},
        {562u, 316u, 113u},
        {771u, 436u, 155u},
        {466u, 255u, 100u},
        {619u, 342u, 129u},
        {797u, 444u, 164u},
        {999u, 558u, 205u},
        {694u, 377u, 150u},
        {847u, 464u, 179u},
    },
};

static BattleLeafCost BattleOamCost(unsigned count, unsigned alignment) {
    const unsigned base = count <= 10u ? count : 7u + (count - 7u) % 4u;
    const unsigned groups = (count - base) / 4u;
    BattleLeafCost cost = battle_oam_costs[alignment][base];
    cost.cycles += groups * 228u;
    cost.rom_bytes += groups * 122u;
    cost.ram_bytes += groups * 50u;
    return cost;
}

static unsigned BattleTileWraps(const uint8_t *ram, unsigned columns, unsigned rows) {
    const unsigned doubled = (ram[0] << 1u) & 0xffu;
    unsigned row_tile = (doubled + (doubled & 0xf0u) + (ram[0] >> 7u)) & 0xffu;
    unsigned wraps = 0u;
    for (unsigned row = 0u; row < rows; ++row) {
        unsigned word = (unsigned)ram[4] << 8u | row_tile;
        if (word + columns * 4u + 16u < 0xfff0u) {
            const unsigned crossings = word & 1u ? 0u : ((word & 15u) + columns * 2u) / 16u;
            wraps += crossings;
            word += columns * 2u + crossings * 16u;
        } else {
            for (unsigned column = 0u; column < columns; ++column) {
                const unsigned upper_carry = word + 0x10u > 0xffffu;
                const unsigned advanced = word + 2u + upper_carry;
                const unsigned carry = advanced > 0xffffu;
                word = advanced & 0xffffu;
                if (!(word & 15u)) {
                    ++wraps;
                    word = (word + 0x10u + carry) & 0xffffu;
                }
            }
        }
        row_tile = word & 0xffu;
    }
    return wraps;
}

static BattleLeafCost BattleTileCost(unsigned columns, unsigned rows, unsigned wraps) {
    const unsigned cells = columns * rows;
    const BattleLeafCost cost = {
        29u + rows * 48u + cells * 74u + wraps * 2u,
        11u + rows * 30u + cells * 41u + wraps * 3u,
        12u + rows * 11u + cells * 10u};
    return cost;
}

static uint64_t BattleLeafMaster(uint32_t entry, BattleLeafCost cost) {
    const unsigned internal = cost.cycles - cost.rom_bytes - cost.ram_bytes;
    return (uint64_t)cost.rom_bytes * cpu_region_speed(entry) +
        (uint64_t)cost.ram_bytes * 8u + (uint64_t)internal * 6u;
}

static unsigned BattleLeafClockRoom(uint32_t entry, BattleLeafCost cost) {
    const uint64_t master = BattleLeafMaster(entry, cost);
    return (unsigned)(master + 40u * (master / 1324u + 2u));
}

static bool BattleLeafClockWindowSupported(const CpuState *cpu, uint32_t entry,
                                          uint32_t end, unsigned room) {
    if (!g_snes || (g_snes->vPos < 225u && !g_snes->hdmaBeamOff))
        return false;
    return BattleClockAllows(cpu, entry, end, 2u, false, room, false, NULL);
}

static bool BattleLeafClockSupported(const CpuState *cpu, uint32_t entry,
                                      BattleLeafCost *cost) {
    if (cpu != &g_cpu || !cpu->ram || cpu->m_flag != 1u || cpu->x_flag ||
        cpu->D || cpu->_flag_D || cpu->S < 0x1f06u || !g_snes ||
        !g_snes->cart || g_snes->cart->type != CART_LOROM)
        return false;
    if (!interp_bridge_use_absolute_apu_timeline(
            rtl_apu_frame_timeline_active(), cart_has_sa1(g_snes->cart),
            rtl_apu_extended_frame_timing() && g_snes->apu &&
                g_snes->apu->portTimeValid))
        return false;
    unsigned columns = 0u, rows = 0u;
    if (entry == 0x81b705u) {
        const unsigned count = cpu->A & 0xffu;
        const unsigned used = cpu->ram[0x58u];
        if (cpu->DB != 0x7eu || (count &&
            (used + count > 128u || cpu->Y != used * 4u || cpu->X < 0x4000u ||
             cpu->X + count * 5u > 0x10000u)))
            return false;
        *cost = BattleOamCost(count, used & 3u);
    } else if (entry == 0x81be58u) {
        columns = cpu->ram[2] ? cpu->ram[2] : 256u;
        rows = cpu->ram[3] ? cpu->ram[3] : 256u;
        const unsigned destination = cpu->ram[8] | (unsigned)cpu->ram[9] << 8u;
        if (destination < 0x2000u ||
            destination + (rows - 1u) * 128u + (columns - 1u) * 4u + 0x43u >= 0x10000u)
            return false;
        *cost = BattleTileCost(columns, rows, 0u);
    } else {
        return false;
    }
    const uint32_t end = entry == 0x81b705u ? 0x81b7d9u : 0x81bebcu;
    if (!BattleLeafClockWindowSupported(cpu, entry, end,
                                   BattleLeafClockRoom(entry, *cost)))
        return false;
    if (columns) {
        *cost = BattleTileCost(columns, rows, BattleTileWraps(cpu->ram, columns, rows));
        return BattleLeafClockWindowSupported(cpu, entry, end,
                                         BattleLeafClockRoom(entry, *cost));
    }
    return true;
}

static void BattleLeafFetch(CpuState *cpu, uint32_t pc, unsigned bytes) {
    if (Lufia2DecompPollFetchOpcode(cpu, pc, bytes))
        return;
    for (unsigned i = 0u; i < bytes; ++i) {
        bool progress = false;
        (void)Lufia2DecompFrameReturnReadByte(cpu, pc + i, &progress);
    }
}

static RecompReturn ActorBridgeTimedBattleLeaf(CpuState *cpu, uint32_t entry) {
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    BattleLeafCost cost;
    if (!BattleLeafClockSupported(cpu, entry, &cost))
        return ActorBridgeFallback(cpu, &frame, entry);
    frame = ActorBridgeEnter(cpu);
    const unsigned count = cpu->A & 0xffu;
    if (entry == 0x81be58u)
        BattleLeafFetch(cpu, 0x81bebau, 1u);
    const Lufia2Memory memory = {ActorBridgeRead, ActorBridgeWrite, cpu, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = entry == 0x81b705u
        ? Lufia2BattleAppendOamSprites(&memory, &state)
        : Lufia2BattleTileBlock(&memory, &state);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED);
    ActorBridgeStore(cpu, &state);
    if (entry == 0x81b705u) {
        const uint32_t branch = !count ? 0x81b709u :
            result.pc == 0x81b77fu ? 0x81b77du : 0x81b7d3u;
        BattleLeafFetch(cpu, branch, 2u);
    }
    cost.cycles -= 6u;
    --cost.rom_bytes;
    cost.ram_bytes -= 2u;
    Lufia2DecompFrameReturnAccountOpcode(cpu, cost.cycles,
        BattleLeafMaster(entry, cost), true);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}
