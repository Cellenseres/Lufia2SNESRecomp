#ifndef LUFIA2_PATCH_ACTOR_D508_EARLY_RETURN_H
#define LUFIA2_PATCH_ACTOR_D508_EARLY_RETURN_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cpu_state.h"
#include "snes/interp816.h"

enum {
    L2_ACTOR_D508_EARLY_RETURN_PC = 0xD508,
    L2_ACTOR_D508_EARLY_RETURN_NEXT_PC = 0xD599,
    L2_ACTOR_D508_EARLY_RETURN_OPCODES = 17,
    L2_ACTOR_D508_EARLY_RETURN_CYCLES = 50,
    /* Every possible 65816 cycle is conservatively priced as SlowROM. */
    L2_ACTOR_D508_EARLY_RETURN_MAX_MASTER = 400,
};

static inline int L2ActorD508EarlyReturnFitsStepCap(
        long steps, long step_cap) {
    return step_cap > steps &&
        (unsigned long long)(step_cap - steps) >=
            L2_ACTOR_D508_EARLY_RETURN_OPCODES;
}

/* The RTS at $D599 remains interpreter-owned, so a hook there is not retired. */
static inline int L2ActorD508EarlyReturnContainsOpcodePc(uint32_t pc24) {
    switch (pc24 & 0x7FFFFFu) {
        case 0x03D508u: case 0x03D50Au: case 0x03D50Eu:
        case 0x03D510u: case 0x03D539u: case 0x03D53Cu:
        case 0x03D53Eu: case 0x03D540u: case 0x03D543u:
        case 0x03D545u: case 0x03D547u: case 0x03D548u:
        case 0x03D54Au: case 0x03D54Eu: case 0x03D57Eu:
        case 0x03D581u: case 0x03D583u:
            return 1;
        default:
            return 0;
    }
}

/* Only bytes executed by the replacement are pinned. Skipped dispatcher code
 * remains outside this patch's ownership. */
static inline int L2ActorD508EarlyReturnBytesMatch(
        const uint8_t *rom, size_t rom_size) {
    static const uint8_t entry[] = {
        0xA6,0xA7, 0xBF,0x36,0x07,0x00, 0x89,0x80, 0xF0,0x27,
    };
    static const uint8_t middle[] = {
        0xBD,0x22,0x06, 0x29,0x80, 0xD0,0x5A,
        0xAD,0xA7,0x09, 0x89,0x01, 0xF0,0x37,
        0x8A, 0xD0,0x08, 0xAF,0xFE,0xD0,0x7F, 0xF0,0x2E,
    };
    static const uint8_t exit_path[] = {
        0xBD,0x36,0x07, 0x89,0x04, 0xF0,0x14,
    };
    enum {
        ENTRY_OFFSET = 0x1D508,
        MIDDLE_OFFSET = 0x1D539,
        EXIT_OFFSET = 0x1D57E,
    };
    return rom && rom_size >= EXIT_OFFSET + sizeof(exit_path) &&
        memcmp(rom + ENTRY_OFFSET, entry, sizeof(entry)) == 0 &&
        memcmp(rom + MIDDLE_OFFSET, middle, sizeof(middle)) == 0 &&
        memcmp(rom + EXIT_OFFSET, exit_path, sizeof(exit_path)) == 0;
}

/* This accepts the observed actor-zero idle path only. All dispatcher and
 * state-changing paths stay in interp816_runOpcode. cpu->ram is the runtime's
 * flat $7E0000-$7FFFFF WRAM image; $7F:D0FE therefore maps to 0x1D0FE. */
static inline int L2ActorD508EarlyReturnEligible(
        const CpuState *cpu, const Interp816 *in, int bridge_safe) {
    if (!bridge_safe || !cpu || !cpu->ram || !in || !in->read ||
        in->k != 0x83u || in->pc != L2_ACTOR_D508_EARLY_RETURN_PC ||
        in->db != 0x83u || in->dp != 0 || !in->mf || !in->xf || in->e ||
        in->waiting || in->stopped || in->nmiWanted || in->irqWanted)
        return 0;

    const uint8_t actor = cpu->ram[0x00A7u];
    if (actor != 0)
        return 0;
    const uint8_t actor_state = cpu->ram[0x0736u];
    return (actor_state & 0x84u) == 0 &&
        (cpu->ram[0x0622u] & 0x80u) == 0 &&
        (cpu->ram[0x09A7u] & 1u) != 0 &&
        cpu->ram[0x1D0FEu] == 0;
}

static inline uint8_t L2ActorD508ReadPc(Interp816 *in) {
    const uint16_t pc = in->pc++;
    return in->read(in->mem, ((uint32_t)in->k << 16) | pc);
}

static inline void L2ActorD508BeginOpcode(Interp816 *in) {
    extern uint32_t g_interp816_cur_pc;
    g_interp816_cur_pc = ((uint32_t)in->k << 16) | in->pc;
    (void)L2ActorD508ReadPc(in);
}

static inline void L2ActorD508SetZn8(Interp816 *in, uint8_t value) {
    in->z = value == 0;
    in->n = (value & 0x80u) != 0;
}

static inline void L2ActorD508LoadA8(Interp816 *in, uint8_t value) {
    in->a = (uint16_t)((in->a & 0xFF00u) | value);
    L2ActorD508SetZn8(in, value);
}

static inline uint32_t L2ActorD508ReadLongOperand(Interp816 *in) {
    uint32_t address = L2ActorD508ReadPc(in);
    address |= (uint32_t)L2ActorD508ReadPc(in) << 8;
    address |= (uint32_t)L2ActorD508ReadPc(in) << 16;
    return address;
}

/* Execute the accepted prefix with the original read callback and leave the
 * interpreter at $83:D599. The original RTS remains responsible for the
 * return-frame reads and final PC/SP transition. */
static inline unsigned L2ActorD508EarlyReturnStep(Interp816 *in) {
    uint8_t value, operand, high;
    uint32_t address;

    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);       /* LDX dp */
    value = in->read(in->mem, (uint16_t)(in->dp + operand));
    in->x = value; L2ActorD508SetZn8(in, value);

    L2ActorD508BeginOpcode(in);                                      /* LDA long,X */
    address = (L2ActorD508ReadLongOperand(in) + in->x) & 0xFFFFFFu;
    L2ActorD508LoadA8(in, in->read(in->mem, address));

    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* BIT #$80 */
    in->z = (((uint8_t)in->a & operand) == 0);
    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* BEQ $D539 */
    in->pc = (uint16_t)(in->pc + (int8_t)operand);

    L2ActorD508BeginOpcode(in);                                      /* LDA abs,X */
    operand = L2ActorD508ReadPc(in); high = L2ActorD508ReadPc(in);
    address = ((uint32_t)in->db << 16) |
        (uint16_t)((((uint16_t)high << 8) | operand) + in->x);
    L2ActorD508LoadA8(in, in->read(in->mem, address));

    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* AND #$80 */
    value = (uint8_t)in->a & operand;
    in->a = (uint16_t)((in->a & 0xFF00u) | value);
    L2ActorD508SetZn8(in, value);
    L2ActorD508BeginOpcode(in); (void)L2ActorD508ReadPc(in);          /* BNE not */

    L2ActorD508BeginOpcode(in);                                      /* LDA abs */
    operand = L2ActorD508ReadPc(in); high = L2ActorD508ReadPc(in);
    address = ((uint32_t)in->db << 16) | ((uint16_t)high << 8) | operand;
    L2ActorD508LoadA8(in, in->read(in->mem, address));
    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* BIT #$01 */
    in->z = (((uint8_t)in->a & operand) == 0);
    L2ActorD508BeginOpcode(in); (void)L2ActorD508ReadPc(in);          /* BEQ not */

    L2ActorD508BeginOpcode(in);                                      /* TXA */
    L2ActorD508LoadA8(in, (uint8_t)in->x);
    L2ActorD508BeginOpcode(in); (void)L2ActorD508ReadPc(in);          /* BNE not */

    L2ActorD508BeginOpcode(in);                                      /* LDA long */
    address = L2ActorD508ReadLongOperand(in);
    L2ActorD508LoadA8(in, in->read(in->mem, address));
    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* BEQ $D57E */
    in->pc = (uint16_t)(in->pc + (int8_t)operand);

    L2ActorD508BeginOpcode(in);                                      /* LDA abs,X */
    operand = L2ActorD508ReadPc(in); high = L2ActorD508ReadPc(in);
    address = ((uint32_t)in->db << 16) |
        (uint16_t)((((uint16_t)high << 8) | operand) + in->x);
    L2ActorD508LoadA8(in, in->read(in->mem, address));
    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* BIT #$04 */
    in->z = (((uint8_t)in->a & operand) == 0);
    L2ActorD508BeginOpcode(in); operand = L2ActorD508ReadPc(in);      /* BEQ $D599 */
    in->pc = (uint16_t)(in->pc + (int8_t)operand);

    in->cyclesUsed = 3;
    return L2_ACTOR_D508_EARLY_RETURN_CYCLES;
}

#endif
