#include <stdint.h>

#include "cpu_state.h"
#include "lufia2/player_update.h"

static void Lufia2BridgeSetIndexWidth(CpuState *cpu, uint8_t narrow) {
    cpu_mirrors_to_p(cpu);
    if (narrow) {
        cpu->P |= 0x10;
        cpu->X &= 0x00ff;
        cpu->Y &= 0x00ff;
    } else {
        cpu->P &= (uint8_t)~0x10;
    }
    cpu_p_to_mirrors(cpu);
}

static RecompReturn Lufia2BridgeRunChild(
    CpuState *cpu, uint32_t target, uint32_t call_site) {
    return cpu_dispatch_call_pc(cpu, target, call_site);
}

static uint8_t Lufia2BridgeReadByte(void *context, uint16_t address) {
    CpuState *cpu = (CpuState *)context;
    return cpu_read8(cpu, cpu->DB, address);
}

static uint32_t Lufia2BridgePeekRtsTarget(CpuState *cpu) {
    const uint16_t pcl_address = (uint16_t)(cpu->S + 1);
    const uint16_t pch_address = (uint16_t)(cpu->S + 2);
    const int32_t pcl_offset = cpu_wram_offset(0x00, pcl_address);
    const int32_t pch_offset = cpu_wram_offset(0x00, pch_address);
    uint16_t return_minus_one;

    if (pcl_offset >= 0 && pch_offset >= 0) {
        return_minus_one = (uint16_t)(
            cpu->ram[pcl_offset] | ((uint16_t)cpu->ram[pch_offset] << 8));
    } else {
        return_minus_one = (uint16_t)(
            cpu_read8(cpu, 0x00, pcl_address) |
            ((uint16_t)cpu_read8(cpu, 0x00, pch_address) << 8));
    }
    return ((uint32_t)cpu->PB << 16) |
           (uint16_t)(return_minus_one + 1);
}

static RecompReturn Lufia2BridgeReturn(
    CpuState *cpu,
    uint16_t entry_s,
    uint8_t entry_hrv,
    uint32_t paired_return_pc) {
    const uint16_t return_s = cpu->S;
    cpu->S = (uint16_t)(cpu->S + 1);
    const uint16_t pcl = cpu_read8(cpu, 0x00, cpu->S);
    cpu->S = (uint16_t)(cpu->S + 1);
    const uint16_t pch = cpu_read8(cpu, 0x00, cpu->S);
    const uint32_t return_pc =
        ((uint32_t)cpu->PB << 16) | ((((pch << 8) | pcl) + 1) & 0xffffu);

    if (entry_hrv == 2 && return_s == entry_s) {
        if (return_pc == paired_return_pc)
            return RECOMP_RETURN_NORMAL;
        return interp_tier_dispatch_rewritten_return(
            cpu, return_pc, 0x83bbf3u);
    }
    return cpu_dispatch_pc_from(
        cpu, return_pc, (uint16_t)(entry_s + 2), 0x83bbf3u);
}

RecompReturn Lufia2DecompBridge_BBF3(CpuState *cpu) {
    const Lufia2PlayerSlotSpecialMemory memory = {
        Lufia2BridgeReadByte,
        cpu,
    };
    const uint16_t entry_s = cpu->S;
    const uint8_t entry_hrv = cpu->host_return_valid;
    const uint32_t paired_return_pc =
        entry_hrv == 2 ? Lufia2BridgePeekRtsTarget(cpu) : 0xffffffffu;
    RecompReturn child_result = RECOMP_RETURN_NORMAL;

    Lufia2BridgeSetIndexWidth(cpu, 0);
    const Lufia2PlayerSlotSpecialResult result =
        Lufia2PlayerSlotSpecialUpdate(&memory);
    cpu->A = (uint16_t)((cpu->A & 0xff00u) | result.accumulator_low);
    cpu->_flag_N = result.negative;
    cpu->_flag_Z = result.zero;
    cpu_mirrors_to_p(cpu);
    switch (result.action) {
    case LUFIA2_PLAYER_SLOT_SPECIAL_CHILD:
        child_result = Lufia2BridgeRunChild(cpu, 0x83bc28u, 0x83bc1du);
        break;
    case LUFIA2_PLAYER_SLOT_STANDARD_CHILD:
        child_result = Lufia2BridgeRunChild(cpu, 0x83c1b4u, 0x83bc22u);
        break;
    case LUFIA2_PLAYER_SLOT_NO_CHILD:
        break;
    }
    if (child_result != RECOMP_RETURN_NORMAL)
        return (RecompReturn)((int)child_result - 1);

    Lufia2BridgeSetIndexWidth(cpu, 1);
    return Lufia2BridgeReturn(
        cpu, entry_s, entry_hrv, paired_return_pc);
}
