#ifndef LUFIA2_PATCH_ACTOR_EARLY_RETURN_H
#define LUFIA2_PATCH_ACTOR_EARLY_RETURN_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cpu_state.h"
#include "snes/interp816.h"

enum {
    L2_ACTOR_EARLY_RETURN_PC = 0xC7F8,
    L2_ACTOR_EARLY_RETURN_NEXT_PC = 0xC83B,
    L2_ACTOR_EARLY_RETURN_OPCODES = 19,
    L2_ACTOR_EARLY_RETURN_CYCLES = 52,
    /* $83:C801 LDA $1291,x is the only indexed read on this path whose page
     * can change: $1291 + $6F leaves page $12. The two $0622,x reads would
     * need actor >= $DE, which the eligibility guard rejects. Indexed reads
     * pay that crossing cycle, as hardware does. */
    L2_ACTOR_EARLY_RETURN_STATE_BASE = 0x1291,
    /* SlowROM upper bound: 50 bus cycles * 8 plus up to 3 internal cycles
     * * 6 -- two taken branches and the page crossing. The measured FastROM
     * path is 326 master clocks. */
    L2_ACTOR_EARLY_RETURN_MAX_MASTER = 418,
};

/* BB93 supplies slot indices 0..39. Eligible nonzero slots never cross the
 * $1291 page, so observed BB93 callers take 52 cycles here. Keep the helper
 * general so the cycle contract remains explicit. */
static inline unsigned L2ActorEarlyReturnCycles(unsigned actor) {
    const unsigned crossed =
        ((L2_ACTOR_EARLY_RETURN_STATE_BASE + actor) >> 8) !=
        (L2_ACTOR_EARLY_RETURN_STATE_BASE >> 8);
    return (unsigned)L2_ACTOR_EARLY_RETURN_CYCLES + crossed;
}

static inline int L2ActorEarlyReturnFitsStepCap(long steps, long step_cap) {
    return step_cap > steps &&
        (unsigned long long)(step_cap - steps) >=
            L2_ACTOR_EARLY_RETURN_OPCODES;
}

/* Pre-opcode hooks are stored with the LoROM mirror bit stripped. An
 * unrelated hook (for example the MSU driver in bank $00) must not disable
 * this block; a hook on any opcode we retire must retain ownership. */
static inline int L2ActorEarlyReturnContainsOpcodePc(uint32_t pc24) {
    switch (pc24 & 0x7FFFFFu) {
        case 0x03C7F8u: case 0x03C7FAu: case 0x03C7FDu:
        case 0x03C7FFu: case 0x03C801u: case 0x03C804u:
        case 0x03C806u: case 0x03C80Eu: case 0x03C811u:
        case 0x03C813u: case 0x03C815u: case 0x03C818u:
        case 0x03C81Au: case 0x03C81Cu: case 0x03C81Eu:
        case 0x03C821u: case 0x03C823u: case 0x03C825u:
        case 0x03C827u:
            return 1;
        default:
            return 0;
    }
}

/* $83:C7F8..$83:C828, ending with the taken BEQ $C83B.  The supported ROM
 * hash is checked by the build; this local signature prevents a future ROM
 * patch from silently reusing an instruction-level replacement. */
static inline int L2ActorEarlyReturnBytesMatch(
        const uint8_t *rom, size_t rom_size) {
    static const uint8_t expected[] = {
        0xA6,0xA7, 0xBD,0x22,0x06, 0x29,0x80, 0xD0,0x3A,
        0xBD,0x91,0x12, 0x89,0x07, 0xF0,0x06,
        0x3A, 0x9D,0x91,0x12, 0x80,0x2E,
        0xAD,0xA7,0x09, 0x89,0x01, 0xF0,0x19,
        0xAD,0x69,0x12, 0x10,0x14,
        0xA5,0xA7, 0xF0,0x1E,
        0xBD,0x22,0x06, 0x89,0x08, 0xD0,0x09,
        0x89,0x40, 0xF0,0x12,
    };
    const size_t offset = 0x1C7F8u;
    return rom && rom_size >= offset + sizeof(expected) &&
        memcmp(rom + offset, expected, sizeof(expected)) == 0;
}

/* This is deliberately narrower than every architectural path that reaches
 * $C83B. It accepts only the common, write-free actor-idle path proved by
 * the gameplay corpus. BB93 stores the current slot index (0..39) in $00A7;
 * slot zero takes a different branch at $C81C, so only slots 1..39 are
 * eligible. All other states remain authoritative interpreter work. */
static inline int L2ActorEarlyReturnEligible(
        const CpuState *cpu, const Interp816 *in, int bridge_safe) {
    if (!bridge_safe || !cpu || !cpu->ram || !in || !in->read ||
        in->k != 0x83u || in->pc != L2_ACTOR_EARLY_RETURN_PC ||
        in->db != 0x83u || in->dp != 0 || !in->mf || !in->xf || in->e ||
        in->waiting || in->stopped || in->nmiWanted || in->irqWanted)
        return 0;

    const uint8_t actor = cpu->ram[0x00A7u];
    if (actor == 0 || actor >= 0x28u)
        return 0;

    const uint8_t flags = cpu->ram[0x0622u + actor];
    const uint8_t state = cpu->ram[0x1291u + actor];
    const uint8_t global = cpu->ram[0x09A7u];
    const uint8_t direction = cpu->ram[0x1269u];
    return (flags & 0xC8u) == 0 && (state & 7u) == 0 &&
        (global & 1u) != 0 && (direction & 0x80u) != 0;
}

static inline uint8_t L2ActorReadPc(Interp816 *in) {
    const uint16_t pc = in->pc++;
    return in->read(in->mem, ((uint32_t)in->k << 16) | pc);
}

static inline void L2ActorBeginOpcode(Interp816 *in) {
    extern uint32_t g_interp816_cur_pc;
    g_interp816_cur_pc = ((uint32_t)in->k << 16) | in->pc;
    (void)L2ActorReadPc(in);
}

static inline void L2ActorSetZn8(Interp816 *in, uint8_t value) {
    in->z = value == 0;
    in->n = (value & 0x80u) != 0;
}

static inline void L2ActorLoadA8(Interp816 *in, uint8_t value) {
    in->a = (uint16_t)((in->a & 0xFF00u) | value);
    L2ActorSetZn8(in, value);
}

/* Execute the accepted 19-opcode prefix in native C. Every opcode, operand
 * and data read still uses the interpreter's current bus callback and occurs
 * in hardware order. The bridge therefore keeps open-bus state, read epochs,
 * region pricing and APU/device policy. It charges the returned aggregate
 * cycle count once, then executes the RTS at $83:C83B normally. */
static inline unsigned L2ActorEarlyReturnStep(Interp816 *in) {
    uint8_t actor, value, operand;

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* LDX dp */
    actor = in->read(in->mem, (uint16_t)(in->dp + operand));
    in->x = actor; L2ActorSetZn8(in, actor);

    L2ActorBeginOpcode(in);                                          /* LDA abs,X */
    operand = L2ActorReadPc(in); value = L2ActorReadPc(in);
    value = in->read(in->mem, ((uint32_t)in->db << 16) |
        (uint16_t)(((uint16_t)value << 8) | operand) + in->x);
    L2ActorLoadA8(in, value);

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* AND # */
    value = (uint8_t)in->a & operand;
    in->a = (uint16_t)((in->a & 0xFF00u) | value); L2ActorSetZn8(in, value);

    L2ActorBeginOpcode(in); (void)L2ActorReadPc(in);                  /* BNE not */

    L2ActorBeginOpcode(in);                                          /* LDA abs,X */
    operand = L2ActorReadPc(in); value = L2ActorReadPc(in);
    value = in->read(in->mem, ((uint32_t)in->db << 16) |
        (uint16_t)(((uint16_t)value << 8) | operand) + in->x);
    L2ActorLoadA8(in, value);

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* BIT # */
    in->z = (((uint8_t)in->a & operand) == 0);

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* BEQ taken */
    in->pc = (uint16_t)(in->pc + (int8_t)operand);

    L2ActorBeginOpcode(in);                                          /* LDA abs */
    operand = L2ActorReadPc(in); value = L2ActorReadPc(in);
    value = in->read(in->mem, ((uint32_t)in->db << 16) |
        (uint16_t)(((uint16_t)value << 8) | operand));
    L2ActorLoadA8(in, value);

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* BIT # */
    in->z = (((uint8_t)in->a & operand) == 0);
    L2ActorBeginOpcode(in); (void)L2ActorReadPc(in);                  /* BEQ not */

    L2ActorBeginOpcode(in);                                          /* LDA abs */
    operand = L2ActorReadPc(in); value = L2ActorReadPc(in);
    value = in->read(in->mem, ((uint32_t)in->db << 16) |
        (uint16_t)(((uint16_t)value << 8) | operand));
    L2ActorLoadA8(in, value);

    L2ActorBeginOpcode(in); (void)L2ActorReadPc(in);                  /* BPL not */

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* LDA dp */
    value = in->read(in->mem, (uint16_t)(in->dp + operand));
    L2ActorLoadA8(in, value);
    L2ActorBeginOpcode(in); (void)L2ActorReadPc(in);                  /* BEQ not */

    L2ActorBeginOpcode(in);                                          /* LDA abs,X */
    operand = L2ActorReadPc(in); value = L2ActorReadPc(in);
    value = in->read(in->mem, ((uint32_t)in->db << 16) |
        (uint16_t)(((uint16_t)value << 8) | operand) + in->x);
    L2ActorLoadA8(in, value);

    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* BIT #$08 */
    in->z = (((uint8_t)in->a & operand) == 0);
    L2ActorBeginOpcode(in); (void)L2ActorReadPc(in);                  /* BNE not */
    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* BIT #$40 */
    in->z = (((uint8_t)in->a & operand) == 0);
    L2ActorBeginOpcode(in); operand = L2ActorReadPc(in);              /* BEQ $C83B */
    in->pc = (uint16_t)(in->pc + (int8_t)operand);
    in->cyclesUsed = 3;
    return L2ActorEarlyReturnCycles(actor);
}

#endif
