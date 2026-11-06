#include "cpu_state.h"
#include "cpu_trace.h"
#include "common_cpu_infra.h"
#include "snes/interp_bridge.h"
#include "snes/snes.h"
#include "lufia2/actor.h"
#include "lufia2/world_map.h"
#include "patches/native_patches.h"
#include <stddef.h>

/* Original AOT entries own clocks, deadlines and returns. */
typedef struct LeafBus {
    CpuState *host;
    unsigned direct_page_cycle;
    unsigned operand_width;
    unsigned high_pending;
    uint8_t high_byte;
    const Lufia2CpuState *state;
    unsigned low_pending;
    uint32_t low_address;
    uint8_t low_byte;
    uint8_t load_status;
} LeafBus;

static int LeafTracingEnabled(void) {
#if SNESRECOMP_TRACE
    return 1;
#elif defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG
    return 1;
#else
    return Lufia2NativeObserversActive();
#endif
}

static uint8_t LeafRead(void *context, uint32_t address) {
    LeafBus *bus = context;
    if (bus->high_pending) {
        bus->high_pending = 0;
        return bus->high_byte;
    }
    if (bus->direct_page_cycle) {
        /* LDX reads before stack restoration. */
        bus->direct_page_cycle = 0;
        ++bus->host->cycles;
        bus->host->master_cycles += g_memsel ? 6u : 8u;
    }
    const unsigned width = bus->operand_width;
    bus->operand_width = 0;
    if (width == 2u) {
        /* Preserve the original word-access MMIO credit. */
        const uint16_t value = cpu_read16(
            bus->host, (uint8_t)(address >> 16), (uint16_t)address);
        bus->high_byte = (uint8_t)(value >> 8);
        bus->high_pending = 1;
        return (uint8_t)value;
    }
    return cpu_read8(bus->host, (uint8_t)(address >> 16), (uint16_t)address);
}

static void LeafWrite(void *context, uint32_t address, uint8_t value) {
    LeafBus *bus = context;
    cpu_write8(bus->host, (uint8_t)(address >> 16), (uint16_t)address, value);
}

static void LeafWriteWord(void *context, uint32_t address, uint8_t value) {
    LeafBus *bus = context;
    if (!bus->low_pending) {
        bus->low_pending = 1;
        bus->low_address = address;
        bus->low_byte = value;
        return;
    }
    bus->low_pending = 0;
    if ((address >> 16) == (bus->low_address >> 16)) {
        cpu_write16(bus->host, (uint8_t)(address >> 16),
            (uint16_t)bus->low_address,
            (uint16_t)(bus->low_byte | ((uint16_t)value << 8)));
    } else {
        /* Long stores carry across the bank boundary. */
        LeafWrite(context, bus->low_address, bus->low_byte);
        LeafWrite(context, address, value);
    }
}

static void ActorResetWrite(void *context, uint32_t address, uint8_t value) {
    const LeafBus *bus = context;
    if (bus->state->accumulator_is_8_bit || address < 0x010000u)
        LeafWrite(context, address, value);
    else
        LeafWriteWord(context, address, value);
}

static uint8_t BlockPointerRead(void *context, uint32_t address) {
    LeafBus *bus = context;
    if (!bus->high_pending)
        bus->operand_width = 2u;
    const uint8_t value = LeafRead(context, address);
    if (address == 0x5au) {
        const uint16_t word = (uint16_t)(value | ((uint16_t)bus->high_byte << 8));
        bus->load_status = (uint8_t)((word ? 0u : 2u) | (word & 0x8000u ? 0x80u : 0u));
    }
    return value;
}

static Lufia2CpuState LeafLoad(const CpuState *cpu) {
    Lufia2CpuState state = {0};
    state.accumulator = cpu->A;
    state.x = cpu->X;
    state.y = cpu->Y;
    state.stack = cpu->S;
    state.direct_page = cpu->D;
    state.data_bank = cpu->DB;
    state.program_bank = cpu->PB;
    state.carry = cpu->_flag_C;
    state.negative = cpu->_flag_N;
    state.zero = cpu->_flag_Z;
    state.accumulator_is_8_bit = cpu->m_flag;
    state.index_is_8_bit = cpu->x_flag;
    state.overflow = cpu->_flag_V;
    state.decimal = cpu->_flag_D;
    state.irq_disable = cpu->_flag_I;
    return state;
}

static void LeafStore(CpuState *cpu, const Lufia2CpuState *state) {
    cpu->A = state->accumulator;
    cpu->X = state->x;
    cpu->Y = state->y;
    cpu->S = state->stack;
    cpu->D = state->direct_page;
    cpu->DB = state->data_bank;
    cpu->_flag_C = state->carry;
    cpu->_flag_N = state->negative;
    cpu->_flag_Z = state->zero;
    cpu->m_flag = state->accumulator_is_8_bit;
    cpu->x_flag = state->index_is_8_bit;
    cpu->_flag_V = state->overflow;
    cpu->_flag_D = state->decimal;
    cpu->_flag_I = state->irq_disable;
    cpu_mirrors_to_p(cpu);
}

int Lufia2ActorResetAotBody(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->PB != 0x83u ||
        LeafTracingEnabled())
        return 0;
    const uint16_t operand = (uint16_t)(cpu->D + 0xa9u);
    if (operand > (cpu->x_flag ? 0x1fffu : 0x1ffeu) ||
        cpu->S < 0x1f00u || cpu->S > 0x1fffu)
        return 0;
    Lufia2CpuState state = LeafLoad(cpu);
    LeafBus bus = {0};
    bus.host = cpu;
    bus.state = &state;
    bus.direct_page_cycle = (cpu->D & 0xffu) != 0u;
    bus.operand_width = cpu->x_flag ? 1u : 2u;
    const Lufia2Memory memory = {LeafRead, ActorResetWrite, &bus, NULL, NULL};
    Lufia2ActorResetTransientState(&memory, &state);
    LeafStore(cpu, &state);
    return 1;
}

int Lufia2WorldIndexAotBody(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->PB != 0x86u ||
        LeafTracingEnabled())
        return 0;
    LeafBus bus = {0};
    bus.host = cpu;
    bus.operand_width = 2u;
    const Lufia2Memory memory = {LeafRead, LeafWrite, &bus, NULL, NULL};
    Lufia2CpuState state = LeafLoad(cpu);
    Lufia2WorldMapIndexObjectRecord(&memory, &state);
    LeafStore(cpu, &state);
    return 1;
}

int Lufia2WorldDisableDisplayAotBody(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || !cpu->m_flag || cpu->x_flag ||
        cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D || LeafTracingEnabled())
        return 0;
    LeafBus bus = {0};
    bus.host = cpu;
    const Lufia2Memory memory = {LeafRead, LeafWrite, &bus, NULL, NULL};
    Lufia2CpuState state = LeafLoad(cpu);
    Lufia2WorldMapDisableDisplay(&memory, &state);
    LeafStore(cpu, &state);
    return 1;
}

int Lufia2WorldBlockPointerAotBody(CpuState *cpu) {
    if (cpu->emulation || cpu->_flag_D || cpu->m_flag || cpu->D ||
        cpu->PB != 0x86u || LeafTracingEnabled())
        return 0;
    LeafBus bus = {0};
    bus.host = cpu;
    const Lufia2Memory memory = {BlockPointerRead, LeafWriteWord, &bus, NULL, NULL};
    const uint8_t entry_status = cpu->P;
    Lufia2CpuState state = LeafLoad(cpu);
    Lufia2WorldMapBuildBlockPointer(&memory, &state);
    LeafStore(cpu, &state);
    /* AOT materializes P only at the preceding load. */
    cpu->P = (uint8_t)((entry_status & ~0x82u) | bus.load_status);
    return 1;
}

#if defined(LUFIA2_WORLD_ARRIVAL_BOUNDARY_API) && LUFIA2_WORLD_ARRIVAL_BOUNDARY_API
typedef struct ArrivalBus {
    CpuState *host;
    uint32_t previous_block;
    uint8_t status;
} ArrivalBus;

static void ArrivalSync(ArrivalBus *bus, const Lufia2CpuState *state) {
    bus->host->A = state->accumulator;
    bus->host->_flag_C = state->carry;
    bus->host->_flag_N = state->negative;
    bus->host->_flag_Z = state->zero;
    bus->host->P = bus->status;
}

static uint8_t ArrivalRead(void *context, uint32_t address) {
    ArrivalBus *bus = context;
    const uint8_t value = cpu_read8(bus->host,
        (uint8_t)(address >> 16), (uint16_t)address);
    if (address == 0x8609e0u || address == 0x8609e1u ||
        address == 0x58u || address == 0x5au)
        bus->status = (uint8_t)((bus->status & ~0x82u) |
            (value ? 0u : 2u) | (value & 0x80u));
    return value;
}

static void ArrivalWrite(void *context, uint32_t address, uint8_t value) {
    ArrivalBus *bus = context;
    cpu_write8(bus->host, (uint8_t)(address >> 16), (uint16_t)address, value);
}

static uint8_t ArrivalBoundary(void *context, Lufia2CpuState *state, uint32_t pc) {
    ArrivalBus *bus = context;
    CpuState *cpu = bus->host;
    ArrivalSync(bus, state);
    if (pc == 0x869ad9u && bus->previous_block) {
        ++cpu->cycles;
        cpu->master_cycles += g_memsel ? 6u : 8u;
    }
    cpu_trace_block(cpu, pc);
    WatchdogCheck();
    if (interp_bridge_lle_master_deadline_reached(cpu))
        return 0;
    unsigned cycles;
    switch (pc) {
    case 0x869ab1u: cycles = 11u; break;
    case 0x869abbu: cycles = 2u; break;
    case 0x869ab8u:
    case 0x869abdu: cycles = 6u; break;
    case 0x869ac2u:
    case 0x869ac9u: cycles = 9u; break;
    case 0x869ad0u: cycles = 17u; break;
    case 0x869ad9u: cycles = 8u; break;
    default: return 0;
    }
    cpu->coprocessor_master_cycles = cpu->master_cycles;
    cpu->cycles += cycles;
    cpu->master_cycles += cycles * (g_memsel ? 6u : 8u);
    bus->previous_block = pc;
    return 1;
}

static uint32_t WorldArrivalAotBody(CpuState *cpu, uint32_t entry) {
    if (cpu != &g_cpu || cpu->emulation || cpu->_flag_D || !cpu->m_flag ||
        cpu->x_flag || cpu->PB != 0x86u || cpu->DB != 0x86u || cpu->D ||
        cpu->S < 0x1f20u || cpu->S > 0x1ffcu || !g_snes ||
        !cpu->ram || cpu->ram != g_snes->ram || LeafTracingEnabled())
        return 0;
    ArrivalBus bus = {cpu, 0u, cpu->P};
    const Lufia2Memory memory = {ArrivalRead, ArrivalWrite, &bus, NULL, NULL};
    Lufia2CpuState state = LeafLoad(cpu);
    const Lufia2ExecutionResult result = Lufia2WorldMapCheckArrivalWithBoundaries(
        &memory, &state, entry, ArrivalBoundary, &bus);
    if (result.flow == LUFIA2_EXECUTION_RETURNED) {
        if (result.pc == 0x869ad8u)
            bus.status = (uint8_t)((bus.status & ~0x83u) | 0x81u);
        else
            bus.status &= (uint8_t)~1u;
    }
    ArrivalSync(&bus, &state);
    return result.pc;
}

uint32_t Lufia2WorldMovingArrivalAotBody(CpuState *cpu) {
    return WorldArrivalAotBody(cpu, 0x869ab1u);
}

uint32_t Lufia2WorldStoppedArrivalAotBody(CpuState *cpu) {
    return WorldArrivalAotBody(cpu, 0x869ab8u);
}
#endif
