#include "lufia2/actor.h"
#include "lufia2/field.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {Lufia2ActorAllocateSprite,
                                   Lufia2ActorLoadSecondaryScript,
                                   Lufia2FieldResolveEventArea};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
