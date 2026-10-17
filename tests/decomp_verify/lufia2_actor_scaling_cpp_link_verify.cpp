#include "lufia2/actor.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {Lufia2ActorBuildOrbitOffsets, Lufia2ActorScaleRadius,
                                   Lufia2ActorReadSignedOperand,
                                   Lufia2ObjectSignExtendNibble};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
