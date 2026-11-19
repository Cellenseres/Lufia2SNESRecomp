static void SpcBootstrapSetNz(Spc *spc, uint8_t value) {
    spc->n = (value & 0x80u) != 0u;
    spc->z = value == 0u;
}

static void SpcBootstrapCompare(Spc *spc, uint8_t left, uint8_t right) {
    spc->c = left >= right;
    SpcBootstrapSetNz(spc, (uint8_t)(left - right));
}

static unsigned SpcBootstrapBranch(Spc *spc, bool take, uint16_t target, uint16_t next) {
    spc->pc = take ? target : next;
    return take ? 4u : 2u;
}

static void SpcBootstrapStore(Apu *apu, uint16_t address, uint8_t value) {
    (void)apu_cpuRead(apu, address);
    apu_cpuWrite(apu, address, value);
}

static uint16_t SpcBootstrapReadWord(Apu *apu, uint16_t address) {
    const uint8_t low = apu_cpuRead(apu, address);
    const uint8_t high = apu_cpuRead(apu, (uint16_t)(address + 1u));
    return low | ((uint16_t)high << 8);
}

static void SpcBootstrapLoadPorts(Apu *apu, uint16_t address) {
    Spc *spc = apu->spc;
    const uint16_t value = SpcBootstrapReadWord(apu, address);
    spc->a = (uint8_t)value;
    spc->y = (uint8_t)(value >> 8);
    spc->n = (value & 0x8000u) != 0u;
    spc->z = value == 0u;
}

static unsigned SpcBootstrapStep(Apu *apu) {
    Spc *spc = apu->spc;
    switch (spc->pc) {
    case 0xffc0u:
        spc->pc = 0xffc2u;
        spc->x = 0xefu;
        SpcBootstrapSetNz(spc, spc->x);
        return 2u;
    case 0xffc2u:
        spc->pc = 0xffc3u;
        spc->sp = spc->x;
        return 2u;
    case 0xffc3u:
        spc->pc = 0xffc5u;
        spc->a = 0u;
        SpcBootstrapSetNz(spc, spc->a);
        return 2u;
    case 0xffc5u:
        spc->pc = 0xffc6u;
        SpcBootstrapStore(apu, spc->x, spc->a);
        return 4u;
    case 0xffc6u:
        spc->pc = 0xffc7u;
        SpcBootstrapSetNz(spc, --spc->x);
        return 2u;
    case 0xffc7u:
        return SpcBootstrapBranch(spc, !spc->z, 0xffc5u, 0xffc9u);
    case 0xffc9u:
        spc->pc = 0xffccu;
        SpcBootstrapStore(apu, 0xf4u, 0xaau);
        return 5u;
    case 0xffccu:
        spc->pc = 0xffcfu;
        SpcBootstrapStore(apu, 0xf5u, 0xbbu);
        return 5u;
    case 0xffcfu:
        spc->pc = 0xffd2u;
        SpcBootstrapCompare(spc, apu_cpuRead(apu, 0xf4u), 0xccu);
        return 5u;
    case 0xffd2u:
        return SpcBootstrapBranch(spc, !spc->z, 0xffcfu, 0xffd4u);
    case 0xffd4u:
        spc->pc = 0xffefu;
        return 4u;
    case 0xffd6u:
        spc->pc = 0xffd8u;
        spc->y = apu_cpuRead(apu, 0xf4u);
        SpcBootstrapSetNz(spc, spc->y);
        return 3u;
    case 0xffd8u:
        return SpcBootstrapBranch(spc, !spc->z, 0xffd6u, 0xffdau);
    case 0xffdau:
        spc->pc = 0xffdcu;
        SpcBootstrapCompare(spc, spc->y, apu_cpuRead(apu, 0xf4u));
        return 3u;
    case 0xffdcu:
        return SpcBootstrapBranch(spc, !spc->z, 0xffe9u, 0xffdeu);
    case 0xffdeu:
        spc->pc = 0xffe0u;
        spc->a = apu_cpuRead(apu, 0xf5u);
        SpcBootstrapSetNz(spc, spc->a);
        return 3u;
    case 0xffe0u:
        spc->pc = 0xffe2u;
        SpcBootstrapStore(apu, 0xf4u, spc->y);
        return 4u;
    case 0xffe2u: {
        spc->pc = 0xffe4u;
        const uint16_t address = (uint16_t)(SpcBootstrapReadWord(apu, 0u) + spc->y);
        SpcBootstrapStore(apu, address, spc->a);
        return 7u;
    }
    case 0xffe4u:
        spc->pc = 0xffe5u;
        SpcBootstrapSetNz(spc, ++spc->y);
        return 2u;
    case 0xffe5u:
        return SpcBootstrapBranch(spc, !spc->z, 0xffdau, 0xffe7u);
    case 0xffe7u: {
        spc->pc = 0xffe9u;
        const uint8_t high = (uint8_t)(apu_cpuRead(apu, 1u) + 1u);
        apu_cpuWrite(apu, 1u, high);
        SpcBootstrapSetNz(spc, high);
        return 4u;
    }
    case 0xffe9u:
        return SpcBootstrapBranch(spc, !spc->n, 0xffdau, 0xffebu);
    case 0xffebu:
        spc->pc = 0xffedu;
        SpcBootstrapCompare(spc, spc->y, apu_cpuRead(apu, 0xf4u));
        return 3u;
    case 0xffedu:
        return SpcBootstrapBranch(spc, !spc->n, 0xffdau, 0xffefu);
    case 0xffefu:
        spc->pc = 0xfff1u;
        SpcBootstrapLoadPorts(apu, 0xf6u);
        return 5u;
    case 0xfff1u:
        spc->pc = 0xfff3u;
        SpcBootstrapStore(apu, 0u, spc->a);
        apu_cpuWrite(apu, 1u, spc->y);
        return 5u;
    case 0xfff3u:
        spc->pc = 0xfff5u;
        SpcBootstrapLoadPorts(apu, 0xf4u);
        return 5u;
    case 0xfff5u:
        spc->pc = 0xfff7u;
        SpcBootstrapStore(apu, 0xf4u, spc->a);
        return 4u;
    case 0xfff7u:
        spc->pc = 0xfff8u;
        spc->a = spc->y;
        SpcBootstrapSetNz(spc, spc->a);
        return 2u;
    case 0xfff8u:
        spc->pc = 0xfff9u;
        spc->x = spc->a;
        SpcBootstrapSetNz(spc, spc->x);
        return 2u;
    case 0xfff9u:
        return SpcBootstrapBranch(spc, !spc->z, 0xffd6u, 0xfffbu);
    case 0xfffbu:
        spc->pc = 0xfffeu;
        spc->pc = SpcBootstrapReadWord(apu, spc->x);
        return 6u;
    default:
        return 0u;
    }
}

static unsigned RunSpcBootstrap(Apu *apu) {
    Spc *spc = apu->spc;
    if (SNESRECOMP_SPC_DIAGNOSTICS || SNESRECOMP_TRACE || !s_apu_program ||
        !apu->romReadable || spc->pc < 0xffc0u || spc->p || spc->stopped)
        return 0u;

    static int trace_requested = -1;
    if (trace_requested < 0)
        trace_requested = getenv("SNESRECOMP_SPC_TRACE") != NULL;
    if (trace_requested)
        return 0u;

    const unsigned cycles = SpcBootstrapStep(apu);
    if (cycles)
        spc->cyclesUsed = cycles;
    return cycles;
}
