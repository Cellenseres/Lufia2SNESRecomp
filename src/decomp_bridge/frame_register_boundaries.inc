#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
#include "lufia2/battle_target_control_steps.h"
#endif
#include "lufia2/frame_wait.h"
#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
#include "lufia2/nmi_control_steps.h"
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS)
#include "lufia2/battle_control_steps.h"
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS)
#include "lufia2/field_control_steps.h"
#endif
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS)
#include "lufia2/world_frame_control_steps.h"
#endif
#if defined(LUFIA2_HAVE_NATIVE_FRAME_EXIT_STEPS)
#include "lufia2/frame_exit_steps.h"
#endif

extern uint32_t g_interp816_cur_pc;

static int Lufia2DecompRunFrameRegisterBoundary(
    Interp816 *registers, uint32_t pc, uint8_t opcode) {
    if (registers->e || registers->k != pc >> 16 ||
        registers->pc != (uint16_t)pc || registers->waiting ||
        registers->stopped || registers->nmiWanted || registers->irqWanted)
        return 0;
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
    switch (opcode) {
#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
    case 0xa2u:
#endif
    case 0x48u: case 0x08u: case 0xe2u: case 0x28u: case 0x68u:
    case 0x5au: case 0xdau: case 0xc2u: case 0xfau: case 0x7au:
    case 0x6bu: case 0x20u: case 0x80u: case 0x60u: case 0x22u:
        break;
    default: return 0;
    }
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
    if (pc == 0x80876du && registers->xf)
        return 0;
    bool nmi_control = false;
#endif
    uint8_t expected_opcode;
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
    bool target_control = false;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS)
    bool battle_control = false;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS)
    bool field_control = false;
#endif
    switch (pc) {
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
    case 0x81ccacu:
    case 0x81ccd8u:
    case 0x81ccdfu:
    case 0x81d5afu:
    case 0x81d5b4u:
    case 0x81d618u:
    case 0x81d635u:
        target_control = true;
        expected_opcode = 0x20u;
        break;
    case 0x81ccdbu:
    case 0x81cce8u:
    case 0x81d581u:
    case 0x81d5e3u:
    case 0x81d640u:
    case 0x81d644u:
    case 0x81d64au:
        target_control = true;
        expected_opcode = 0x22u;
        break;
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
    case 0x808703u:
        nmi_control = true;
        expected_opcode = 0x08u;
        break;
    case 0x808704u: case 0x80876bu:
        nmi_control = true;
        expected_opcode = 0xc2u;
        break;
    case 0x80876du:
        nmi_control = true;
        expected_opcode = 0xa2u;
        break;
    case 0x8087a5u:
        nmi_control = true;
        expected_opcode = 0x28u;
        break;
    case 0x8087a6u:
        nmi_control = true;
        expected_opcode = 0x60u;
        break;
    case 0x808826u:
        nmi_control = true;
        expected_opcode = 0x20u;
        break;
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS)
    case 0x81de9eu:
        battle_control = true;
        expected_opcode = 0x22u;
        break;
    case 0x858e97u: case 0x859c7au: case 0x859cd6u:
        battle_control = true;
        expected_opcode = 0x6bu;
        break;
    case 0x858f49u: case 0x85aa7eu:
        battle_control = true;
        expected_opcode = 0x60u;
        break;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS)
    case 0x8381cbu: case 0x83c1feu: case 0x83c20fu: case 0x83c213u:
        field_control = true;
        expected_opcode = 0x22u;
        break;
    case 0x83c0f9u: case 0x83bc27u: case 0x83c245u:
        field_control = true;
        expected_opcode = 0x60u;
        break;
    case 0x83bc25u:
        field_control = true;
        expected_opcode = 0xe2u;
        break;
#endif
    case 0x8293c2u: case 0x868b48u: case 0x86974fu: case 0x84a5a8u:
        expected_opcode = 0x48u;
        break;
    case 0x8293c3u: case 0x868b49u: case 0x809378u: case 0x84a5a9u:
    case 0x868b57u:
        expected_opcode = 0x08u;
        break;
    case 0x8293c4u: case 0x868b4au: case 0x809379u: case 0x84a5aau:
    case 0x868b58u:
        expected_opcode = 0xe2u;
        break;
    case 0x8293ccu: case 0x868b52u: case 0x809381u: case 0x84a5b2u:
    case 0x868b6fu:
#if defined(LUFIA2_HAVE_NATIVE_FRAME_EXIT_STEPS)
    case 0x83829eu:
#endif
        expected_opcode = 0x28u;
        break;
    case 0x8293cdu: case 0x868b53u: case 0x869756u: case 0x84a5b3u:
        expected_opcode = 0x68u;
        break;
    case 0x868b55u: expected_opcode = 0x5au; break;
    case 0x868b56u: expected_opcode = 0xdau; break;
    case 0x868b5au: expected_opcode = 0xc2u; break;
    case 0x868b70u: expected_opcode = 0xfau; break;
    case 0x868b71u: expected_opcode = 0x7au; break;
#if defined(LUFIA2_HAVE_NATIVE_FRAME_EXIT_STEPS)
    case 0x85ecdau: case 0x85ecefu:
#endif
    case 0x868b72u: expected_opcode = 0x6bu; break;
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS)
    case 0x869282u: case 0x86929cu: expected_opcode = 0x20u; break;
    case 0x86929fu: expected_opcode = 0x80u; break;
#endif
    default: return 0;
    }
    if (opcode != expected_opcode)
        return 0;
    Lufia2CpuState state = {0};
    state.accumulator = registers->a;
    state.x = registers->x;
    state.y = registers->y;
    state.stack = registers->sp;
    state.direct_page = registers->dp;
    state.data_bank = registers->db;
    state.program_bank = registers->k;
    state.accumulator_is_8_bit = registers->mf;
    state.index_is_8_bit = registers->xf;
    state.carry = registers->c;
    state.zero = registers->z;
    state.negative = registers->n;
    state.overflow = registers->v;
    state.decimal = registers->d;
    state.irq_disable = registers->i;
    const Lufia2Memory memory = {
        registers->read, registers->write, registers->mem, NULL, NULL};
    (void)registers->read(registers->mem, pc);
    g_interp816_cur_pc = pc;
    const bool width_change = opcode == 0xe2u || opcode == 0xc2u;
    if (width_change)
        (void)registers->read(registers->mem, pc + 1u);
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS)
    else if (opcode == 0x20u) {
        (void)registers->read(registers->mem, pc + 1u);
        (void)registers->read(registers->mem, pc + 2u);
    } else if (opcode == 0x80u)
        (void)registers->read(registers->mem, pc + 1u);
#endif
#if (defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)) && !defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS)
    else if (opcode == 0x20u) {
        (void)registers->read(registers->mem, pc + 1u);
        (void)registers->read(registers->mem, pc + 2u);
    }
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
    if (opcode == 0xa2u) {
        (void)registers->read(registers->mem, pc + 1u);
        (void)registers->read(registers->mem, pc + 2u);
    }
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
    if (opcode == 0x22u) {
        (void)registers->read(registers->mem, pc + 1u);
        (void)registers->read(registers->mem, pc + 2u);
        (void)registers->read(registers->mem, pc + 3u);
    }
#endif
    unsigned cycles;
    switch (opcode) {
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
    case 0x22u: cycles = 8u; break;
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
    case 0x60u: cycles = 6u; break;
#endif
    case 0x48u: cycles = 3u + !registers->mf; break;
    case 0x68u: cycles = 4u + !registers->mf; break;
    case 0x5au: case 0xdau: cycles = 3u + !registers->xf; break;
    case 0x7au: case 0xfau: cycles = 4u + !registers->xf; break;
    case 0x28u: cycles = 4u; break;
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS) || defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
    case 0x20u:
#endif
    case 0x6bu: cycles = 6u; break;
    default: cycles = 3u; break;
    }
    const Lufia2ExecutionResult result =
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_TARGET_CONTROL_STEPS)
        target_control
        ? Lufia2BattleTargetControlStep(&memory, &state, pc) :
#endif
#if defined(LUFIA2_HAVE_NATIVE_NMI_CONTROL_STEPS)
        nmi_control
        ? Lufia2NmiControlStep(&memory, &state, pc) :
#endif
#if defined(LUFIA2_HAVE_NATIVE_BATTLE_CONTROL_STEPS)
        battle_control
        ? Lufia2BattleControlStep(&memory, &state, pc) :
#endif
#if defined(LUFIA2_HAVE_NATIVE_FIELD_CONTROL_STEPS)
        field_control
        ? Lufia2FieldControlStep(&memory, &state, pc) :
#endif
#if defined(LUFIA2_HAVE_NATIVE_WORLD_FRAME_CONTROL_STEPS)
        pc == 0x869282u || pc == 0x86929cu || pc == 0x86929fu
        ? Lufia2WorldFrameControlStep(&memory, &state, pc) :
#endif
#if defined(LUFIA2_HAVE_NATIVE_FRAME_EXIT_STEPS)
        pc == 0x83829eu || pc == 0x85ecdau || pc == 0x85ecefu
        ? Lufia2FrameExitStep(&memory, &state, pc) :
#endif
        pc >= 0x868b55u && pc <= 0x868b72u
        ? Lufia2SpriteFrameRegisterStep(&memory, &state, pc)
        : Lufia2FrameWaitRegisterStep(&memory, &state, pc);
    registers->a = state.accumulator;
    registers->x = state.x;
    registers->y = state.y;
    registers->sp = state.stack;
    registers->k = state.program_bank;
    registers->pc = (uint16_t)result.pc;
    registers->mf = state.accumulator_is_8_bit;
    registers->xf = state.index_is_8_bit;
    registers->c = state.carry;
    registers->z = state.zero;
    registers->n = state.negative;
    registers->v = state.overflow;
    registers->d = state.decimal;
    registers->i = state.irq_disable;
    registers->cyclesUsed = (uint8_t)cycles;
    return (int)cycles;
}
