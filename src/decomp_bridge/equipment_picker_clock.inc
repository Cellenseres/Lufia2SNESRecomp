static void EquipmentClockBeginPreview(EquipmentCallClock *clock) {
    EquipmentClockOpcode(clock, 0xb950u, 3u, 3u, 0u);
    EquipmentClockNearCall(clock, 0xb953u);
}

static void EquipmentClockReadInput(EquipmentCallClock *clock, bool resetting) {
    EquipmentClockOpcode(clock, resetting ? 0xb939u : 0xb93du, 2u, 2u, 0u);
    EquipmentClockOpcode(clock, resetting ? 0xb93bu : 0xb93fu, 2u, 3u, 0u);
    EquipmentClockNearCall(clock, 0xb916u);
}

static void EquipmentClockCancel(EquipmentCallClock *clock) {
    EquipmentClockOpcode(clock, 0xb9f9u, 2u, 2u, 0u);
    EquipmentClockOpcode(clock, 0xb9fbu, 3u, 3u, 0u);
    EquipmentClockOpcode(clock, 0xb9feu, 3u, 3u, 0u);
    EquipmentClockNearCall(clock, 0xba01u);
}

static void EquipmentClockConfirmSound(EquipmentCallClock *clock) {
    EquipmentClockOpcode(clock, 0xb9cbu, 2u, 2u, 0u);
    EquipmentClockLongCall(clock, 0xb9cdu);
}

static void EquipmentClockRejectChoice(EquipmentCallClock *clock) {
    EquipmentClockNearCall(clock, 0xb9f3u);
}

static void EquipmentClockPlanCommand(EquipmentCallClock *clock, unsigned command) {
    static const uint8_t commands[] = {1u, 2u, 3u, 4u, 6u, 7u, 10u, 11u};
    for (unsigned i = 0u; i < 8u; ++i) {
        const uint16_t pc = (uint16_t)(0xb919u + i * 4u);
        EquipmentClockOpcode(clock, pc, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, pc + 2u, 2u, command == commands[i] ? 3u : 2u, 0u);
        if (command == commands[i])
            break;
    }
    switch (command) {
    case 1u:
        EquipmentClockBeginPreview(clock);
        break;
    case 2u:
        EquipmentClockOpcode(clock, 0xb99cu, 2u, 3u, 0u);
        EquipmentClockOpcode(clock, 0xb99eu, 3u, 5u, 2u);
        EquipmentClockNearCall(clock, 0xb9a1u);
        break;
    case 3u:
        EquipmentClockJump(clock, 0xb941u);
        EquipmentClockCancel(clock);
        break;
    case 4u:
        EquipmentClockJump(clock, 0xb944u);
        EquipmentClockNearCall(clock, 0xba2cu);
        break;
    case 6u:
    case 7u:
        EquipmentClockJump(clock, 0xb947u);
        EquipmentClockOpcode(clock, 0xba4cu, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xba4eu, 1u, 2u, 0u);
        EquipmentClockNearCall(clock, 0xba4fu);
        break;
    case 10u:
        EquipmentClockJump(clock, 0xb94au);
        EquipmentClockNearCall(clock, 0xba1du);
        break;
    case 11u:
        EquipmentClockJump(clock, 0xb94du);
        EquipmentClockNearCall(clock, 0xba23u);
        break;
    default:
        EquipmentClockReadInput(clock, true);
        break;
    }
}

static bool EquipmentClockPackedReadSupported(const CpuState *cpu) {
    return cpu->DB == 0x7eu || cpu->X <= 0x1571u;
}

static bool PrepareEquipmentPickerClock(EquipmentCallClock *clock, uint16_t pc) {
    const CpuState *cpu = clock->call.cpu;
    switch (pc) {
    case 0xb907u:
        EquipmentClockOpcode(clock, 0xb907u, 2u, 2u, 0u);
        EquipmentClockLongCall(clock, 0xb909u);
        break;
    case 0xb90du:
    case 0xb910u:
    case 0xb956u:
    case 0xb9d1u:
    case 0xb9dau:
    case 0xb9ecu:
    case 0xb9efu:
    case 0xba04u:
    case 0xba5du:
        EquipmentClockNearCall(clock, pc);
        break;
    case 0xb913u:
    case 0xba29u:
        EquipmentClockJump(clock, pc);
        EquipmentClockBeginPreview(clock);
        break;
    case 0xb919u:
        EquipmentClockPlanCommand(clock, cpu->A & 0xffu);
        break;
    case 0xb959u:
        EquipmentClockOpcode(clock, pc, 2u, cpu->_flag_C ? 3u : 2u, 0u);
        if (cpu->_flag_C) {
            EquipmentClockOpcode(clock, 0xb985u, 2u, 2u, 0u);
            EquipmentClockOpcode(clock, 0xb987u, 3u, 4u, 1u);
            EquipmentClockOpcode(clock, 0xb98au, 2u, 2u, 0u);
            EquipmentClockOpcode(clock, 0xb98cu, 2u, 3u, 1u);
            EquipmentClockOpcode(clock, 0xb98eu, 3u, 3u, 0u);
            EquipmentClockLongCall(clock, 0xb991u);
        } else {
            if (!EquipmentClockPackedReadSupported(cpu))
                return false;
            EquipmentClockOpcode(clock, 0xb95bu, 1u, 2u, 0u);
            EquipmentClockOpcode(clock, 0xb95cu, 3u,
                5u + ((0x0a8du + cpu->X) >> 8u != 0x0au), 2u);
            EquipmentClockOpcode(clock, 0xb95fu, 3u, 5u, 2u);
            EquipmentClockOpcode(clock, 0xb962u, 3u, 4u, 1u);
            EquipmentClockNearCall(clock, 0xb965u);
        }
        break;
    case 0xb968u:
    case 0xb9d4u:
        EquipmentClockOpcode(clock, pc, 2u, 4u, 2u);
        EquipmentClockLongCall(clock, pc + 2u);
        break;
    case 0xb96eu:
        EquipmentClockOpcode(clock, 0xb96eu, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xb970u, 3u, 4u, 1u);
        EquipmentClockOpcode(clock, 0xb973u, 3u, 3u, 0u);
        EquipmentClockOpcode(clock, 0xb976u, 2u, 4u, 2u);
        EquipmentClockOpcode(clock, 0xb978u, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xb97au, 2u, 3u, 1u);
        EquipmentClockOpcode(clock, 0xb97cu, 3u, 3u, 0u);
        EquipmentClockLongCall(clock, 0xb97fu);
        break;
    case 0xb983u:
        EquipmentClockOpcode(clock, 0xb983u, 2u, 3u, 0u);
        EquipmentClockOpcode(clock, 0xb995u, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xb997u, 2u, 5u, 2u);
        EquipmentClockJump(clock, 0xb999u);
        EquipmentClockReadInput(clock, false);
        break;
    case 0xb995u:
        EquipmentClockOpcode(clock, 0xb995u, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xb997u, 2u, 5u, 2u);
        EquipmentClockJump(clock, 0xb999u);
        EquipmentClockReadInput(clock, false);
        break;
    case 0xb9a4u:
        EquipmentClockOpcode(clock, 0xb9a4u, 2u, 3u, 0u);
        EquipmentClockOpcode(clock, 0xb9a6u, 2u, cpu->_flag_C ? 3u : 2u, 0u);
        if (cpu->_flag_C)
            EquipmentClockRejectChoice(clock);
        else
            EquipmentClockNearCall(clock, 0xb9a8u);
        break;
    case 0xb9abu: {
        const unsigned status = cpu->A & 0xffu;
        EquipmentClockOpcode(clock, 0xb9abu, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xb9adu, 2u, status == 1u ? 3u : 2u, 0u);
        if (status == 1u) {
            EquipmentClockRejectChoice(clock);
        } else {
            EquipmentClockOpcode(clock, 0xb9afu, 2u, 2u, 0u);
            EquipmentClockOpcode(clock, 0xb9b1u, 2u, status == 2u ? 3u : 2u, 0u);
            EquipmentClockOpcode(clock, status == 2u ? 0xb9beu : 0xb9b3u, 2u, 3u, 0u);
            if (status == 2u) {
                EquipmentClockOpcode(clock, 0xb9c0u, 3u, 5u, 2u);
                EquipmentClockOpcode(clock, 0xb9c3u, 1u, 2u, 0u);
                EquipmentClockNearCall(clock, 0xb9c4u);
            } else {
                EquipmentClockNearCall(clock, 0xb9b5u);
            }
        }
        break;
    }
    case 0xb9b8u:
    case 0xb9c7u:
        EquipmentClockOpcode(clock, pc, 2u, 3u, 0u);
        EquipmentClockOpcode(clock, pc + 2u, 2u, cpu->_flag_C ? 3u : 2u, 0u);
        if (cpu->_flag_C) {
            EquipmentClockRejectChoice(clock);
        } else {
            if (pc == 0xb9b8u)
                EquipmentClockOpcode(clock, 0xb9bcu, 2u, 3u, 0u);
            EquipmentClockConfirmSound(clock);
        }
        break;
    case 0xb9ddu:
        EquipmentClockOpcode(clock, 0xb9ddu, 3u, 3u, 0u);
        EquipmentClockOpcode(clock, 0xb9e0u, 3u, 5u, 2u);
        EquipmentClockOpcode(clock, 0xb9e3u, 3u, 5u, 2u);
        EquipmentClockOpcode(clock, 0xb9e6u, 3u, 4u, 1u);
        EquipmentClockNearCall(clock, 0xb9e9u);
        break;
    case 0xb9f2u:
        clock->returning = true;
        break;
    case 0xb9f6u:
        EquipmentClockJump(clock, pc);
        EquipmentClockReadInput(clock, false);
        break;
    case 0xba07u:
        EquipmentClockOpcode(clock, 0xba07u, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xba09u, 2u, 5u, 2u);
        clock->next = 0x82ba0bu;
        clock->returning = true;
        break;
    case 0xba20u:
        EquipmentClockJump(clock, pc);
        EquipmentClockNearCall(clock, 0xba26u);
        break;
    case 0xba26u:
        EquipmentClockNearCall(clock, pc);
        break;
    case 0xba2fu:
        EquipmentClockOpcode(clock, 0xba2fu, 2u, 2u, 0u);
        EquipmentClockOpcode(clock, 0xba31u, 2u, (cpu->A & 0xffu) == 2u ? 3u : 2u, 0u);
        if ((cpu->A & 0xffu) == 2u) {
            EquipmentClockJump(clock, 0xba49u);
            EquipmentClockReadInput(clock, true);
        } else {
            if (!EquipmentClockPackedReadSupported(cpu))
                return false;
            EquipmentClockOpcode(clock, 0xba33u, 2u, 3u, 0u);
            EquipmentClockOpcode(clock, 0xba35u, 3u,
                5u + ((0x0a8du + cpu->X) >> 8u != 0x0au), 2u);
            EquipmentClockOpcode(clock, 0xba38u, 3u, 3u, 0u);
            EquipmentClockOpcode(clock, 0xba3bu, 3u, 5u, 2u);
            EquipmentClockOpcode(clock, 0xba3eu, 2u, 3u, 0u);
            EquipmentClockOpcode(clock, 0xba40u, 3u, 3u, 0u);
            EquipmentClockOpcode(clock, 0xba43u, 3u, 3u, 0u);
            EquipmentClockNearCall(clock, 0xba46u);
        }
        break;
    case 0xba49u:
        EquipmentClockJump(clock, pc);
        EquipmentClockReadInput(clock, true);
        break;
    case 0xba52u:
        EquipmentClockOpcode(clock, pc, 2u, 2u, 0u);
        EquipmentClockNearCall(clock, 0xba54u);
        break;
    case 0xba57u:
        EquipmentClockOpcode(clock, pc, 3u, 4u, 1u);
        EquipmentClockNearCall(clock, 0xba5au);
        break;
    case 0xba60u:
        EquipmentClockOpcode(clock, pc, 2u, 3u, 1u);
        EquipmentClockJump(clock, 0xba62u);
        EquipmentClockBeginPreview(clock);
        break;
    default:
        return false;
    }
    return true;
}
