#include "buffer_span_bus.h"

static int Lufia2DecompRunSpriteOamRotateSpan(Interp816 *registers, CpuState *cpu,
                                 uint32_t pc, uint8_t opcode, bool auto_quiescent) {
    if (pc != 0x868cbfu || opcode != 0x2au || registers->e ||
        registers->mf || registers->xf || registers->dp || registers->d ||
        registers->waiting || registers->stopped || registers->nmiWanted ||
        registers->irqWanted || !cpu->ram)
        return 0;

    const unsigned count = cpu->ram[0x65u];
    if (!count || count > 16u || cpu->ram[0x66u] ||
        BufferSpanCycleBudget(cpu, s_lle_master_deadline) < 12u * count - 1u)
        return 0;

    unsigned offset;
    const Cart *cart = Lufia2DecompPlainRomOpcode(registers, pc, opcode, 5u, &offset);
    if (!cart || memcmp(cart->rom + offset + 1u, "\xc6\x65\xd0\xfb", 4u))
        return 0;

    unsigned cycles = 0;
    for (unsigned iteration = 0; iteration < count; ++iteration) {
        (void)registers->read(registers->mem, pc);
        g_interp816_cur_pc = pc;
        const uint16_t accumulator = registers->a;
        registers->a = (uint16_t)((accumulator << 1u) | registers->c);
        registers->c = (accumulator & 0x8000u) != 0u;
        registers->n = (registers->a & 0x8000u) != 0u;
        registers->z = registers->a == 0;
        Lufia2SpanAccountInstruction(cpu, 2u);

        Lufia2SpanProbeOpcode(registers, pc + 1u);
        (void)registers->read(registers->mem, pc + 1u);
        g_interp816_cur_pc = pc + 1u;
        (void)registers->read(registers->mem, pc + 2u);
        Lufia2DecompModifyWram(registers, 0xc6u, 0x65u, 0x66u, true);
        Lufia2SpanAccountInstruction(cpu, 7u);
        if (auto_quiescent)
            bridge_apu_flush(cpu);

        Lufia2SpanProbeOpcode(registers, pc + 3u);
        (void)registers->read(registers->mem, pc + 3u);
        g_interp816_cur_pc = pc + 3u;
        (void)registers->read(registers->mem, pc + 4u);
        registers->cyclesUsed = registers->z ? 2u : 3u;
        registers->pc = registers->z ? 0x8cc4u : 0x8cbfu;
        Lufia2SpanAccountInstruction(cpu, registers->cyclesUsed);
        cycles += 9u + registers->cyclesUsed;
        if (registers->z)
            break;
        if (iteration + 1u < count)
            Lufia2SpanProbeOpcode(registers, pc);
    }
    return (int)cycles;
}
