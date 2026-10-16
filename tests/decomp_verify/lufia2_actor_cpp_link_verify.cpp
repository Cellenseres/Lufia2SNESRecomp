#include "lufia2/actor.h"
typedef Lufia2ExecutionResult (*ActorFunction)(const Lufia2Memory *, Lufia2CpuState *);
static ActorFunction volatile functions[] = {
    Lufia2ActorStopSecondaryScript, Lufia2ActorMarkLeaderScriptStop,
    Lufia2ActorPrepareStepProbe, Lufia2ActorCommitTileStep};
int main(void) {
    for (unsigned i = 0; i < sizeof functions / sizeof functions[0]; ++i)
        if (!functions[i])
            return 1;
    return 0;
}
