#include "lufia2/field.h"
typedef Lufia2ExecutionResult (*FieldLeaf)(const Lufia2Memory *, Lufia2CpuState *);
static FieldLeaf volatile effects[] = {Lufia2FieldAdvancePaletteCycles,
                                       Lufia2FieldStepWaveTable};
static auto volatile selection = Lufia2FieldSelectSessionSetup;
int main() {
    for (auto f : effects)
        if (!f)
            return 1;
    return selection ? 0 : 1;
}
