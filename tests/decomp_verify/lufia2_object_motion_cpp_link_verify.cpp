#include "lufia2/actor.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {
    Lufia2ObjectAddDisplayOffsetX, Lufia2ObjectAddDisplayOffsetY,
    Lufia2ObjectAddFinePositionX,  Lufia2ObjectAddFinePositionY,
    Lufia2ObjectReadRandomOffset,  Lufia2ObjectReadSubcommand,
    Lufia2ObjectCopyAnimationState};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
