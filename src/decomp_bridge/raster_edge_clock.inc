#include "lufia2/raster_edge.h"

enum { RASTER_DIVIDEND_LOW = 0x4204u, RASTER_QUOTIENT_LOW = 0x4214u };

static const FixedSpanOpcode raster_edge_begin[] = {
    {0xfec4u, 2u, 2u, 0u},
    {0xfec6u, 1u, 3u, 1u},
    {0xfec7u, 1u, 4u, 1u},
    {0xfec8u, 2u, 3u, 1u},
    {0xfecau, 1u, 2u, 0u},
    {0xfecbu, 2u, 5u, 2u},
};
static const FixedSpanOpcode raster_edge_difference[] = {
    {0xfecdu, 2u, 2u, 0u},
    {0xfecfu, 1u, 2u, 0u},
    {0xfed0u, 2u, 3u, 1u},
};
static const FixedSpanOpcode raster_edge_exact[] = {
    {0xfed2u, 2u, 2u, 0u},
    {0xfed4u, 2u, 3u, 1u},
    {0xfed6u, 1u, 2u, 0u},
    {0xfed7u, 2u, 3u, 1u},
    {0xfed9u, 2u, 3u, 1u},
    {0xfedbu, 2u, 3u, 1u},
    {0xfeddu, 2u, 3u, 0u},
    {0xff19u, 1u, 3u, 1u},
    {0xff1au, 2u, 3u, 0u},
    {0xff1cu, 1u, 2u, 0u},
    {0xff1du, 1u, 2u, 0u},
    {0xff1eu, 2u, 4u, 2u},
    {0xff20u, 1u, 2u, 0u},
    {0xff21u, 2u, 3u, 0u},
};
static const FixedSpanOpcode raster_edge_positive[] = {
    {0xfed2u, 2u, 3u, 0u},
    {0xfedfu, 2u, 3u, 0u},
    {0xff19u, 1u, 3u, 1u},
    {0xff1au, 2u, 3u, 0u},
    {0xff1cu, 1u, 2u, 0u},
    {0xff1du, 1u, 2u, 0u},
    {0xff1eu, 2u, 4u, 2u},
    {0xff20u, 1u, 2u, 0u},
    {0xff21u, 2u, 3u, 0u},
};
static const FixedSpanOpcode raster_edge_divide[] = {
    {0xfed2u, 2u, 3u, 0u},
    {0xfedfu, 2u, 2u, 0u},
    {0xfee1u, 2u, 2u, 0u},
    {0xfee3u, 1u, 2u, 0u},
    {0xfee4u, 3u, 4u, 1u},
    {0xfee7u, 3u, 4u, 1u},
    {0xfeeau, 2u, 3u, 1u},
    {0xfeecu, 3u, 4u, 1u},
    {0xfeefu, 2u, 3u, 0u},
    {0xfef1u, 1u, 2u, 0u},
    {0xfef2u, 1u, 2u, 0u},
    {0xfef3u, 2u, 4u, 2u},
    {0xfef5u, 1u, 2u, 0u},
    {0xfef6u, 2u, 3u, 0u},
    {0xfef8u, 2u, 3u, 1u},
};
static const FixedSpanOpcode raster_edge_forward[] = {
    {0xfefau, 2u, 2u, 0u},
    {0xfefcu, 3u, 4u, 1u},
    {0xfeffu, 1u, 2u, 0u},
    {0xff00u, 1u, 2u, 0u},
    {0xff01u, 2u, 3u, 1u},
    {0xff03u, 2u, 3u, 1u},
    {0xff05u, 2u, 3u, 0u},
    {0xff10u, 2u, 3u, 1u},
    {0xff12u, 1u, 2u, 0u},
    {0xff13u, 3u, 4u, 1u},
    {0xff16u, 1u, 3u, 1u},
    {0xff17u, 2u, 3u, 0u},
};
static const FixedSpanOpcode raster_edge_reverse[] = {
    {0xfefau, 2u, 3u, 0u},
    {0xff07u, 2u, 3u, 1u},
    {0xff09u, 1u, 2u, 0u},
    {0xff0au, 3u, 4u, 1u},
    {0xff0du, 1u, 2u, 0u},
    {0xff0eu, 2u, 3u, 1u},
    {0xff10u, 2u, 3u, 1u},
    {0xff12u, 1u, 2u, 0u},
    {0xff13u, 3u, 4u, 1u},
    {0xff16u, 1u, 3u, 1u},
    {0xff17u, 2u, 3u, 0u},
};
static const FixedSpanOpcode raster_edge_store[] = {
    {0xff23u, 2u, 3u, 1u},
    {0xff25u, 4u, 5u, 1u},
    {0xff29u, 1u, 4u, 1u},
};
static const FixedSpanOpcode raster_edge_repeat[] = {
    {0xff2au, 2u, 3u, 0u},
    {0xfecbu, 2u, 5u, 2u},
};
static const FixedSpanOpcode raster_edge_finish[] = {
    {0xfecdu, 2u, 3u, 0u},
    {0xff2cu, 2u, 2u, 0u},
    {0xff2eu, 1u, 3u, 1u},
    {0xff2fu, 1u, 4u, 1u},
};

enum { RASTER_EDGE_BASE_CYCLES = 37u, RASTER_EDGE_SAMPLE_CYCLES = 104u };

typedef enum RasterEdgePhase {
    EDGE_BEGIN, EDGE_DIFFERENCE, EDGE_EXACT, EDGE_POSITIVE, EDGE_DIVIDE,
    EDGE_FORWARD, EDGE_REVERSE, EDGE_STORE, EDGE_REPEAT, EDGE_FINISH
} RasterEdgePhase;

typedef struct RasterEdgeClock {
    FixedSpanClock span;
    RasterEdgePhase phase;
} RasterEdgeClock;

static void SetRasterEdgePhase(RasterEdgeClock *clock, RasterEdgePhase phase) {
    static const struct {
        const FixedSpanOpcode *timing;
        unsigned count;
    } phases[] = {
        {raster_edge_begin, sizeof raster_edge_begin / sizeof(FixedSpanOpcode)},
        {raster_edge_difference, sizeof raster_edge_difference / sizeof(FixedSpanOpcode)},
        {raster_edge_exact, sizeof raster_edge_exact / sizeof(FixedSpanOpcode)},
        {raster_edge_positive, sizeof raster_edge_positive / sizeof(FixedSpanOpcode)},
        {raster_edge_divide, sizeof raster_edge_divide / sizeof(FixedSpanOpcode)},
        {raster_edge_forward, sizeof raster_edge_forward / sizeof(FixedSpanOpcode)},
        {raster_edge_reverse, sizeof raster_edge_reverse / sizeof(FixedSpanOpcode)},
        {raster_edge_store, sizeof raster_edge_store / sizeof(FixedSpanOpcode)},
        {raster_edge_repeat, sizeof raster_edge_repeat / sizeof(FixedSpanOpcode)},
        {raster_edge_finish, sizeof raster_edge_finish / sizeof(FixedSpanOpcode)}
    };
    assert((unsigned)phase < sizeof phases / sizeof phases[0]);
    clock->phase = phase;
    clock->span.timing = phases[phase].timing;
    clock->span.count = phases[phase].count;
    clock->span.index = 0u;
}

static void PrepareRasterEdgeAccess(RasterEdgeClock *clock, uint32_t address) {
    BeginFixedSpanOpcode(&clock->span);
    if (clock->span.index != clock->span.count || clock->span.remaining)
        return;
    if (clock->phase == EDGE_BEGIN || clock->phase == EDGE_REPEAT) {
        SetRasterEdgePhase(clock, address == 4u ? EDGE_DIFFERENCE : EDGE_FINISH);
    } else if (clock->phase == EDGE_DIFFERENCE) {
        SetRasterEdgePhase(clock, address == 9u ? EDGE_EXACT
            : address == RASTER_DIVIDEND_LOW ? EDGE_DIVIDE : EDGE_POSITIVE);
    } else if (clock->phase == EDGE_DIVIDE) {
        SetRasterEdgePhase(clock,
            address == RASTER_QUOTIENT_LOW ? EDGE_FORWARD : EDGE_REVERSE);
    } else if (clock->phase == EDGE_STORE) {
        SetRasterEdgePhase(clock, EDGE_REPEAT);
    } else {
        assert(clock->phase == EDGE_EXACT || clock->phase == EDGE_POSITIVE ||
            clock->phase == EDGE_FORWARD || clock->phase == EDGE_REVERSE);
        SetRasterEdgePhase(clock, EDGE_STORE);
    }
}

static uint8_t RasterEdgeRead(void *context, uint32_t address) {
    RasterEdgeClock *clock = context;
    PrepareRasterEdgeAccess(clock, address);
    return FixedSpanRead(&clock->span, address);
}

static void RasterEdgeWrite(void *context, uint32_t address, uint8_t value) {
    RasterEdgeClock *clock = context;
    PrepareRasterEdgeAccess(clock, address);
    FixedSpanWrite(&clock->span, address, value);
}

static bool RasterEdgeOutputSupported(const CpuState *cpu, unsigned samples) {
    if (!samples)
        return true;
    const unsigned word = cpu->ram[0] | (unsigned)cpu->ram[1] << 8;
    const int32_t stride = word & 0x8000u
        ? (int32_t)word - 0x10000 : (int32_t)word;
    const int32_t end = (int32_t)cpu->X + stride * (int32_t)samples;
    return cpu->X <= 0x844cu && end >= 0 && end <= 0x844c;
}

static RecompReturn ActorBridgeTimedRasterEdge(CpuState *cpu) {
    const uint32_t entry = 0x83fec4u;
    ActorBridgeFrame frame = {cpu->S, cpu->host_return_valid, UINT32_MAX};
    unsigned samples = cpu->ram ? cpu->ram[0x22u] : 0u;
    if (samples > 128u)
        samples = 0u;
    const unsigned master = 8u *
        (RASTER_EDGE_BASE_CYCLES + samples * RASTER_EDGE_SAMPLE_CYCLES);
    const unsigned room = master + 40u * (master / 1324u + 2u);
    if (!cpu->ram || cpu->D || cpu->_flag_D || cpu->m_flag != 1u ||
        !RasterEdgeOutputSupported(cpu, samples) ||
        !QuietSpanClockSupported(cpu, entry, 0x83ff31u, 2u, false, room))
        return ActorBridgeFallback(cpu, &frame,
            cpu->PB == 3u ? 0x03fec4u : entry);
    if (frame.hrv)
        frame.host_return_pc24 = Lufia2DecompFramePairedReturnPc(cpu, entry);
    RasterEdgeClock clock = {0};
    clock.span.cpu = cpu;
    clock.span.bank = 0x830000u;
    SetRasterEdgePhase(&clock, EDGE_BEGIN);
    const Lufia2Memory memory = {
        RasterEdgeRead, RasterEdgeWrite, &clock, NULL, NULL};
    Lufia2CpuState state;
    ActorBridgeLoad(cpu, &state);
    const Lufia2ExecutionResult result = Lufia2RasterizeEdge(&memory, &state);
    BeginFixedSpanOpcode(&clock.span);
    assert(result.flow == LUFIA2_EXECUTION_RETURNED && result.pc == 0x83ff30u);
    assert(clock.phase == EDGE_FINISH && clock.span.index == clock.span.count &&
        !clock.span.remaining);
    ActorBridgeStore(cpu, &state);
    return ReturnTimedMenuCall(cpu, &frame, result.pc);
}

