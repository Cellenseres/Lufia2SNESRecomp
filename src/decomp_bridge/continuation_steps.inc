typedef enum ContinuationResult {
    CONTINUATION_STOP,
    CONTINUATION_NEXT,
    CONTINUATION_TAKEN
} ContinuationResult;

typedef struct Continuation {
    NativeSpanState span;
    Interp816 *registers;
    CpuState *cpu;
    const Cart *cart;
    const uint8_t *code;
    uint32_t bank;
    uint64_t dynamic_epoch;
    uint64_t write_epoch;
    bool batched;
} Continuation;

typedef struct ContinuationRange {
    uint32_t first;
    uint32_t last;
    void (*run)(Continuation *, uint16_t);
} ContinuationRange;

#if defined(_MSC_VER)
#define CONTINUATION_CORE static __forceinline
#define CONTINUATION_NOINLINE __declspec(noinline)
#else
#define CONTINUATION_CORE static inline __attribute__((always_inline))
#define CONTINUATION_NOINLINE __attribute__((noinline))
#endif

static bool ContinuationHookQuiet(uint32_t pc) {
    return !(s_pre_opcode_hook_pages[(pc & 0x7fffffu) >> 14] &
             (UINT64_C(1) << ((pc >> 8) & 63u))) ||
        Lufia2DecompFrameRangeQuiet(pc, pc + 1u);
}

CONTINUATION_CORE bool ContinuationReady(const Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand, unsigned length, unsigned cycles) {
    const uint8_t *code = run->code + (address & 0x7fffu);
    if (code[0] != opcode || (length > 1u && code[1] != (uint8_t)operand) ||
        (length > 2u && code[2] != (uint8_t)(operand >> 8)) ||
        (length > 3u && code[3] != (uint8_t)(operand >> 16)))
        return false;
    return NativeSpanCanStep(&run->span, run->cpu, cycles) &&
        (!run->span.instructions || ContinuationHookQuiet(run->bank | address));
}

CONTINUATION_CORE void ContinuationFetch(Continuation *run, uint16_t address, unsigned length) {
    Lufia2SpanFetch(run->registers, run->bank | address, length, run->span.instructions != 0u);
}

CONTINUATION_CORE ContinuationResult ContinuationFinish(Continuation *run, unsigned cycles,
    uint16_t next, ContinuationResult result) {
    NativeSpanStep(&run->span, run->registers, run->cpu, cycles, next);
    /* Outside a batch the bridge loop keeps its own progress checkpoint. */
    if (run->batched)
        NativeSpanSyncProgress(&run->span, run->cpu, &run->dynamic_epoch, &run->write_epoch);
    return result;
}

/* Only the first instruction may touch hardware; the loop then resamples its state. */
CONTINUATION_CORE bool ContinuationReadable(const Continuation *run, uint32_t low, bool wide) {
    return Lufia2DecompIsDecompressSourceByte(run->cart, low) &&
        (!wide || Lufia2DecompIsDecompressSourceByte(run->cart, (low + 1u) & 0xffffffu));
}

CONTINUATION_CORE bool ContinuationWritable(uint32_t low, bool wide) {
    return Lufia2DecompIsWramByte(low) && (!wide || Lufia2DecompIsWramByte((low + 1u) & 0xffffffu));
}

CONTINUATION_CORE ContinuationResult ContinuationImmediate(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand, bool wide) {
    Interp816 *registers = run->registers;
    const bool index = opcode == 0xa0u || opcode == 0xa2u || opcode == 0xc0u || opcode == 0xe0u;
    const unsigned length = wide ? 3u : 2u;
    if ((index ? registers->xf : registers->mf) == wide ||
        !ContinuationReady(run, address, opcode, operand, length, length))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, length);
    Lufia2DecompApplyOperand(registers, opcode, (uint16_t)operand, !wide);
    return ContinuationFinish(run, length, (uint16_t)(address + length), CONTINUATION_NEXT);
}

CONTINUATION_CORE ContinuationResult ContinuationStatus(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    if (!ContinuationReady(run, address, opcode, operand, 2u, 3u))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, 2u);
    const uint8_t status = interp816_getFlags(registers);
    interp816_setFlags(registers, (uint8_t)(opcode == 0xc2u ? status & ~operand : status | operand));
    return ContinuationFinish(run, 3u, (uint16_t)(address + 2u),
        operand & 4u ? CONTINUATION_STOP : CONTINUATION_NEXT);
}

CONTINUATION_CORE ContinuationResult ContinuationBranch(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    const Interp816 *registers = run->registers;
    const bool flag = opcode >> 6 == 0u ? registers->n : opcode >> 6 == 1u ? registers->v :
        opcode >> 6 == 2u ? registers->c : registers->z;
    const bool taken = flag == ((opcode & 0x20u) != 0u);
    const unsigned cycles = taken ? 3u : 2u;
    if (!ContinuationReady(run, address, opcode, operand, 2u, cycles))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, 2u);
    const uint16_t next = (uint16_t)(address + 2u);
    return ContinuationFinish(run, cycles, taken ? (uint16_t)(next + (int8_t)operand) : next,
        taken ? CONTINUATION_TAKEN : CONTINUATION_NEXT);
}

CONTINUATION_CORE ContinuationResult ContinuationJump(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    const unsigned length = opcode == 0x80u ? 2u : opcode == 0x5cu ? 4u : 3u;
    const unsigned cycles = opcode == 0x80u || opcode == 0x4cu ? 3u : 4u;
    if (!ContinuationReady(run, address, opcode, operand, length, cycles))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, length);
    uint16_t next = (uint16_t)operand;
    if (opcode == 0x80u)
        next = (uint16_t)(address + 2u + (int8_t)operand);
    else if (opcode == 0x82u)
        next = (uint16_t)(address + 3u + operand);
    else if (opcode == 0x5cu)
        run->registers->k = (uint8_t)(operand >> 16);
    return ContinuationFinish(run, cycles, next, CONTINUATION_TAKEN);
}

CONTINUATION_CORE bool ContinuationIndexesX(uint8_t opcode) {
    const uint8_t operation = Lufia2DecompMemoryReadOperation(opcode);
    const unsigned mode = opcode & 0x1fu;
    return opcode == 0xbdu || opcode == 0x9du || opcode == 0x9eu || opcode == 0xbfu ||
        opcode == 0x9fu || opcode == 0xb5u || opcode == 0x95u || opcode == 0x74u ||
        opcode == 0xb4u || opcode == 0x94u || opcode == 0xbcu || opcode == 0x34u || opcode == 0x3cu ||
        (operation != 0u && (mode == 0x15u || mode == 0x1du || mode == 0x1fu));
}

CONTINUATION_CORE bool ContinuationIndexesY(uint8_t opcode) {
    return opcode == 0xb9u || opcode == 0x99u || opcode == 0xb6u || opcode == 0x96u || opcode == 0xbeu ||
        (Lufia2DecompMemoryReadOperation(opcode) != 0u && (opcode & 0x1fu) == 0x19u);
}

CONTINUATION_CORE ContinuationResult ContinuationMemory(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    const uint8_t operation = Lufia2DecompMemoryReadOperation(opcode);
    const unsigned mode = opcode & 0x1fu;
    const bool direct = Lufia2DecompIsDirectPageOpcode(opcode) ||
        (operation != 0u && (mode == 0x05u || mode == 0x15u)) ||
        opcode == 0x24u || opcode == 0x34u || opcode == 0xc4u || opcode == 0xe4u;
    const bool far = opcode == 0xafu || opcode == 0xbfu || opcode == 0x8fu || opcode == 0x9fu ||
        (operation != 0u && (mode == 0x0fu || mode == 0x1fu));
    const bool index_x = ContinuationIndexesX(opcode);
    const bool indexed = index_x || ContinuationIndexesY(opcode);
    const bool load = operation != 0u || (opcode >= 0xa0u && opcode < 0xc0u);
    const bool data_x = opcode == 0xa6u || opcode == 0xaeu || opcode == 0x86u || opcode == 0x8eu ||
        opcode == 0xb6u || opcode == 0x96u || opcode == 0xbeu;
    const bool data_y = opcode == 0xa4u || opcode == 0xacu || opcode == 0x84u || opcode == 0x8cu ||
        opcode == 0xb4u || opcode == 0x94u || opcode == 0xbcu;
    const bool index_width = data_x || data_y || operation == 0xc0u || operation == 0xe0u;
    const unsigned length = direct ? 2u : far ? 4u : 3u;
    const unsigned index = !indexed ? 0u : index_x ? registers->x : registers->y;
    const bool wide = !(index_width ? registers->xf : registers->mf);
    const uint32_t low = Lufia2DecompMemoryOperandAddress(registers, operand, direct, far, index);
    const uint32_t high = (low + 1u) & (direct ? 0xffffu : 0xffffffu);
    const unsigned cycles = (direct ? 3u + indexed : far || (indexed && !load) ? 5u : 4u) + wide +
        (direct && (registers->dp & 0xffu) != 0u) +
        (indexed && !direct && !far &&
         ((operand >> 8) != ((operand + index) >> 8) || (!load && !registers->xf)));
    if (!ContinuationReady(run, address, opcode, operand, length, cycles))
        return CONTINUATION_STOP;
    const bool plain = load ? ContinuationReadable(run, low, false) &&
            (!wide || ContinuationReadable(run, high, false))
        : ContinuationWritable(low, false) && (!wide || ContinuationWritable(high, false));
    if (!plain && run->span.instructions)
        return CONTINUATION_STOP;

    ContinuationFetch(run, address, length);
    if (load) {
        const uint16_t value = Lufia2DecompReadData(registers, low, high, wide);
        if (operation == 0x89u) {
            const uint16_t sign = wide ? 0x8000u : 0x80u;
            registers->n = (value & sign) != 0u;
            registers->v = (value & (sign >> 1)) != 0u;
        }
        Lufia2DecompApplyOperand(registers,
            operation ? operation : data_x ? 0xa2u : data_y ? 0xa0u : 0xa9u, value, !wide);
    } else {
        const bool zero = opcode == 0x64u || opcode == 0x9cu || opcode == 0x9eu || opcode == 0x74u;
        Lufia2DecompWriteData(registers, low, high,
            zero ? 0u : data_x ? registers->x : data_y ? registers->y : registers->a, wide);
    }
    return ContinuationFinish(run, cycles, (uint16_t)(address + length),
        plain ? CONTINUATION_NEXT : CONTINUATION_STOP);
}

CONTINUATION_CORE ContinuationResult ContinuationModify(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    const bool direct = Lufia2DecompIsDirectPageOpcode(opcode);
    const bool indexed = opcode == 0xd6u || opcode == 0xf6u || opcode == 0xdeu || opcode == 0xfeu ||
        (opcode < 0x80u && (opcode & 0x17u) == 0x16u);
    const unsigned index = indexed ? registers->x : 0u;
    const unsigned length = direct ? 2u : 3u;
    const bool wide = !registers->mf;
    const uint32_t low = Lufia2DecompMemoryOperandAddress(registers, operand, direct, false, index);
    const unsigned cycles = (direct ? 5u : 6u) + indexed + 2u * wide +
        (direct && (registers->dp & 0xffu) != 0u) +
        (indexed && !direct && (!registers->xf || (operand >> 8) != ((operand + index) >> 8)));
    if (!ContinuationWritable(low, wide) ||
        !ContinuationReady(run, address, opcode, operand, length, cycles))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, length);
    Lufia2DecompModifyWram(registers, opcode, low, (low + 1u) & 0xffffffu, wide);
    return ContinuationFinish(run, cycles, (uint16_t)(address + length), CONTINUATION_NEXT);
}

CONTINUATION_CORE ContinuationResult ContinuationIndirect(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    const unsigned mode = opcode & 0x1fu;
    const bool far = mode == 0x07u || mode == 0x17u;
    const bool index_x = mode == 0x01u;
    const bool index_y = mode == 0x11u || mode == 0x17u;
    const bool stack_direct = mode == 0x03u;
    const bool stack_indirect = mode == 0x13u;
    const bool stack = stack_direct || stack_indirect;
    const unsigned operation = opcode & 0xe0u;
    const bool load = operation != 0x80u;
    const bool wide = !registers->mf;
    const uint16_t pointer = (uint16_t)((stack ? registers->sp :
        registers->dp + (index_x ? registers->x : 0u)) + operand);
    if (pointer > (stack_direct && !wide ? 0x1fffu : far ? 0x1ffdu : 0x1ffeu))
        return CONTINUATION_STOP;

    const uint8_t *ram = run->cpu->ram;
    const unsigned offset = stack_direct ? pointer : ram[pointer] | ((unsigned)ram[pointer + 1u] << 8);
    const uint32_t bank = stack_direct ? 0u : far ? ram[pointer + 2u] : registers->db;
    const unsigned index = index_y || stack_indirect ? registers->y : 0u;
    const uint32_t low = ((bank << 16) + offset + index) & 0xffffffu;
    const uint32_t high = (low + 1u) & 0xffffffu;
    const unsigned cycles = (stack_direct ? 4u : stack_indirect ? 7u :
        far || index_x || (index_y && !load) ? 6u : 5u) + wide +
        (!stack && (registers->dp & 0xffu) != 0u) +
        (index_y && !far && ((offset >> 8) != ((offset + index) >> 8) || (!load && !registers->xf)));
    if (!ContinuationReady(run, address, opcode, operand, 2u, cycles))
        return CONTINUATION_STOP;
    const bool plain = load ? ContinuationReadable(run, low, wide) : ContinuationWritable(low, wide);
    if (!plain && run->span.instructions)
        return CONTINUATION_STOP;

    ContinuationFetch(run, address, 2u);
    if (!stack_direct) {
        (void)Lufia2DecompReadData(registers, pointer, pointer + 1u, true);
        if (far)
            (void)registers->read(registers->mem, pointer + 2u);
    }
    if (load)
        Lufia2DecompApplyOperand(registers, (uint8_t)(operation | 0x09u),
            Lufia2DecompReadData(registers, low, high, wide), !wide);
    else
        Lufia2DecompWriteData(registers, low, high, registers->a, wide);
    return ContinuationFinish(run, cycles, (uint16_t)(address + 2u),
        plain ? CONTINUATION_NEXT : CONTINUATION_STOP);
}

CONTINUATION_CORE ContinuationResult ContinuationRegister(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    const unsigned cycles = opcode == 0xebu ? 3u : 2u;
    if (!ContinuationReady(run, address, opcode, operand, 1u, cycles))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, 1u);
    if (!Lufia2DecompRunTransferOrFlags(registers, opcode)) {
        const bool index_x = opcode == 0xe8u || opcode == 0xcau;
        const bool index_y = opcode == 0xc8u || opcode == 0x88u;
        const bool byte = index_x || index_y ? registers->xf : registers->mf;
        const uint16_t mask = byte ? 0xffu : 0xffffu;
        const uint16_t sign = byte ? 0x80u : 0x8000u;
        uint16_t *destination = index_x ? &registers->x : index_y ? &registers->y : &registers->a;
        const uint16_t value = *destination & mask;
        uint16_t result;
        if (opcode == 0x0au || opcode == 0x2au) {
            result = (uint16_t)((value << 1) | (opcode == 0x2au && registers->c));
            registers->c = (value & sign) != 0u;
        } else if (opcode == 0x4au || opcode == 0x6au) {
            result = (uint16_t)((value >> 1) | (opcode == 0x6au && registers->c ? sign : 0u));
            registers->c = (value & 1u) != 0u;
        } else {
            const bool increment = opcode == 0x1au || opcode == 0xe8u || opcode == 0xc8u;
            result = (uint16_t)(value + (increment ? 1 : -1));
        }
        result &= mask;
        *destination = result | (index_x || index_y ? 0u : *destination & ~mask);
        registers->n = (result & sign) != 0u;
        registers->z = result == 0u;
    }
    return ContinuationFinish(run, cycles, (uint16_t)(address + 1u),
        opcode == 0x58u || opcode == 0x78u ? CONTINUATION_STOP : CONTINUATION_NEXT);
}

CONTINUATION_CORE ContinuationResult ContinuationStack(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    const bool push_address = opcode == 0xf4u;
    const bool push = opcode == 0x08u || opcode == 0x0bu || opcode == 0x48u || opcode == 0x4bu ||
        opcode == 0x5au || opcode == 0x8bu || opcode == 0xdau || push_address;
    const bool index_x = opcode == 0xdau || opcode == 0xfau;
    const bool index_y = opcode == 0x5au || opcode == 0x7au;
    const bool accumulator = opcode == 0x48u || opcode == 0x68u;
    const bool wide = push_address || opcode == 0x0bu || opcode == 0x2bu ||
        ((index_x || index_y) && !registers->xf) || (accumulator && !registers->mf);
    const unsigned bytes = wide ? 2u : 1u;
    const unsigned length = push_address ? 3u : 1u;
    const unsigned cycles = push_address ? 5u : (push ? 3u : 4u) + wide;
    if (registers->sp < 0x1f00u || registers->sp > 0x1fffu ||
        (push ? registers->sp < 0x1f00u + bytes : registers->sp > 0x1fffu - bytes) ||
        !ContinuationReady(run, address, opcode, operand, length, cycles))
        return CONTINUATION_STOP;
    ContinuationFetch(run, address, length);
    if (push) {
        uint16_t value;
        switch (opcode) {
        case 0xf4u:
            value = (uint16_t)operand;
            /* Match the pinned SDK's compatibility behavior. */
            if (value == 0xc2aeu)
                registers->y = 0u;
            break;
        case 0x08u: value = interp816_getFlags(registers); break;
        case 0x0bu: value = registers->dp; break;
        case 0x4bu: value = registers->k; break;
        case 0x5au: value = registers->y; break;
        case 0x8bu: value = registers->db; break;
        case 0xdau: value = registers->x; break;
        default: value = registers->a; break;
        }
        if (wide) {
            registers->write(registers->mem, registers->sp, (uint8_t)(value >> 8));
            --registers->sp;
        }
        registers->write(registers->mem, registers->sp, (uint8_t)value);
        --registers->sp;
    } else {
        uint16_t value = registers->read(registers->mem, ++registers->sp);
        if (wide)
            value |= (uint16_t)registers->read(registers->mem, ++registers->sp) << 8;
        switch (opcode) {
        case 0x28u:
            interp816_setFlags(registers, (uint8_t)value);
            break;
        case 0x2bu:
            registers->dp = value;
            registers->n = (value & 0x8000u) != 0u;
            registers->z = value == 0u;
            break;
        case 0xabu:
            registers->db = (uint8_t)value;
            registers->n = (value & 0x80u) != 0u;
            registers->z = value == 0u;
            break;
        default:
            Lufia2DecompApplyOperand(registers, index_x ? 0xa2u : index_y ? 0xa0u : 0xa9u, value, !wide);
            break;
        }
    }
    return ContinuationFinish(run, cycles, (uint16_t)(address + length),
        opcode == 0x28u ? CONTINUATION_STOP : CONTINUATION_NEXT);
}

/* The bridge loop takes its bounce and return decisions from a span's first opcode. */
CONTINUATION_CORE ContinuationResult ContinuationControl(Continuation *run, uint16_t address,
    uint8_t opcode, uint32_t operand) {
    Interp816 *registers = run->registers;
    const uint32_t pc = run->bank | address;
    const bool far = opcode == 0x22u || opcode == 0x6bu;
    const bool call = opcode == 0x20u || opcode == 0x22u;
    const unsigned length = opcode == 0x60u || opcode == 0x6bu ? 1u : opcode == 0x22u ? 4u : 3u;
    if (run->span.instructions || !ContinuationReady(run, address, opcode, operand, length, 0u))
        return CONTINUATION_STOP;
    int cycles;
    if (call)
        cycles = Lufia2DecompRunDirectCallWithWidths(registers, pc, opcode,
            far ? operand : run->bank | operand, far, registers->mf, registers->xf);
    else if (opcode == 0x60u || opcode == 0x6bu)
        cycles = Lufia2DecompRunReturn(registers, pc, opcode, far, registers->mf, registers->xf);
    else if (opcode == 0x54u || opcode == 0x44u)
        cycles = Lufia2DecompRunBlockMove(registers, pc, opcode);
    else
        cycles = Lufia2DecompRunIndirectControlOpcode(registers, pc, opcode);
    if (!cycles)
        return CONTINUATION_STOP;
    return ContinuationFinish(run, (unsigned)cycles, registers->pc, CONTINUATION_STOP);
}

#define CONTINUATION_STEP(name, core, opcode) \
    static CONTINUATION_NOINLINE ContinuationResult name( \
        Continuation *run, uint16_t address, uint32_t operand) { \
        return core(run, address, opcode, operand); \
    }

#define CONTINUATION_IMMEDIATE(name, opcode, wide) \
    static CONTINUATION_NOINLINE ContinuationResult name( \
        Continuation *run, uint16_t address, uint32_t operand) { \
        return ContinuationImmediate(run, address, opcode, operand, wide); \
    }
