typedef Lufia2ExecutionResult (*MainMenuService)(const Lufia2Memory *,
    Lufia2CpuState *, Lufia2PushedChildCall, void *);

typedef struct MainMenuCall {
    ActorPushedCall call;
    MainMenuClock clock;
} MainMenuCall;

static Lufia2ExecutionResult MainMenuIndicators(const Lufia2Memory *memory,
    Lufia2CpuState *cpu, Lufia2PushedChildCall child, void *context) {
    (void)child;
    (void)context;
    return Lufia2MenuSetOptionIndicators(memory, cpu);
}

static Lufia2ExecutionResult MainMenuScenarioItem(const Lufia2Memory *memory,
    Lufia2CpuState *cpu, Lufia2PushedChildCall child, void *context) {
    (void)child;
    (void)context;
    return Lufia2MenuFindScenarioItem(memory, cpu);
}

static MainMenuService MainMenuServiceAt(uint32_t entry) {
    typedef struct MainMenuEntry { uint32_t pc; MainMenuService service; } MainMenuEntry;
    static const MainMenuEntry entries[] = {
        {0x82a023u, Lufia2MenuBuildOptionsScreen},
        {0x82a07du, Lufia2MenuRefreshOptions},
        {0x82a0c7u, MainMenuIndicators},
        {0x82a0e3u, Lufia2MenuBuildCapsuleScreen},
        {0x82a1d8u, Lufia2MenuBuildCapsuleFeedingScreen},
        {0x82a233u, Lufia2MenuBuildCapsuleFeedingPanels},
        {0x82a88eu, Lufia2MenuRunScenarioItems},
        {0x82bf13u, Lufia2MenuRunOptionsScreen},
        {0x82bfc5u, Lufia2MenuRunCapsuleScreen},
        {0x82c044u, Lufia2MenuRunCapsuleAction},
        {0x82c0acu, Lufia2MenuRunCapsuleFeeding},
        {0x82c170u, Lufia2MenuChooseCapsule},
        {0x82d121u, Lufia2MenuDrawCapsuleExperience},
        {0x82fc3fu, MainMenuScenarioItem}
    };
    unsigned first = 0u, end = sizeof entries / sizeof entries[0];
    while (first < end) {
        const unsigned middle = first + (end - first) / 2u;
        if (entries[middle].pc < entry) first = middle + 1u;
        else if (entries[middle].pc > entry) end = middle;
        else return entries[middle].service;
    }
    return NULL;
}

static bool MainMenuChildMode(const CpuState *cpu, uint32_t site) {
    const bool wide = site == 0x82a03bu || site == 0x82a044u ||
        site == 0x82a149u || site == 0x82a155u || site == 0x82a8c7u ||
        site == 0x82a900u || site == 0x82a24cu || site == 0x82a255u ||
        site == 0x82a25eu || site == 0x82a267u || site == 0x82a273u ||
        site == 0x82c19au || site == 0x82c163u;
    const bool byte_index = site == 0x82a115u || site == 0x82a11du ||
        site == 0x82a176u;
    return cpu->m_flag == (wide ? 0u : 1u) &&
        cpu->x_flag == (byte_index ? 1u : 0u);
}

static uint8_t MainMenuBridgeChild(void *context, Lufia2CpuState *state,
    uint32_t target, uint32_t site, uint8_t frame_size) {
    MainMenuCall *screen = context;
    MainMenuClock *clock = &screen->clock;
    assert(clock->span.index == clock->span.count && !clock->span.remaining);
    const uint32_t landing = ActorBridgePushedReturnPc(site, frame_size);
    assert(landing == clock->next);
    if (!ActorBridgePushedChild(&screen->call, state, target, site, frame_size))
        return 0u;
    if (MainMenuChildMode(screen->call.cpu, site) &&
        PrepareMainMenuClock(clock, screen->call.cpu, landing))
        return 1u;
    screen->call.unwound = (RecompReturn)((int)interp_tier_dispatch_tail(
        screen->call.cpu, landing, landing, screen->call.frame.entry_s,
        screen->call.frame.hrv) + 1);
    return 0u;
}

static RecompReturn ActorBridgeMainMenu(CpuState *cpu, uint32_t entry) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    const MainMenuService service = MainMenuServiceAt(entry);
    MainMenuCall screen;
    screen.call = (ActorPushedCall){cpu, frame, RECOMP_RETURN_NORMAL};
    if (!service || cpu->host_return_valid != 2u ||
        !PrepareMainMenuClock(&screen.clock, cpu, entry))
        return ActorBridgeFallback(cpu, &frame, entry);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite,
        &screen.clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    state.resume_pc = entry;
    const Lufia2ExecutionResult result = service(&memory, &state,
        MainMenuBridgeChild, &screen);
    if (result.flow == LUFIA2_EXECUTION_CHILD_UNWOUND)
        return (RecompReturn)((int)screen.call.unwound - 1);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&screen.clock.span);
    assert(screen.clock.span.index == screen.clock.span.count &&
        !screen.clock.span.remaining && result.pc == screen.clock.next);
    if (result.flow != LUFIA2_EXECUTION_RETURNED)
        return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
            frame.entry_s, frame.hrv);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

static RecompReturn ActorBridgeTimedMenuDispatch(CpuState *cpu) {
    const ActorBridgeFrame frame = ActorBridgeEnter(cpu);
    MainMenuClock clock;
    if (Lufia2DecompMenuDispatchObserved())
        return ActorBridgeWholeX16(cpu, 0x828028u,
            Lufia2MenuDispatchInlineTable, 2u);
    if (!PrepareMainMenuClock(&clock, cpu, 0x828028u))
        return ActorBridgeFallback(cpu, &frame, 0x828028u);
    const Lufia2Memory memory = {FixedSpanRead, FixedSpanWrite,
        &clock.span, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2MenuDispatchInlineTable(&memory, &state);
    ActorBridgeStore(cpu, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(clock.span.index == clock.span.count && !clock.span.remaining);
    return interp_tier_dispatch_tail(cpu, result.pc, result.pc,
        frame.entry_s, frame.hrv);
}
