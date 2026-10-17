#include "lufia2/field.h"
typedef Lufia2ExecutionResult (*ConditionFunction)(const Lufia2Memory *,
                                                   Lufia2CpuState *);
static ConditionFunction volatile functions[] = {
    Lufia2FieldStoreConditionResult, Lufia2FieldNegateConditionResult,
    Lufia2FieldTestLeaderPosition,   Lufia2FieldTestLeaderArea,
    Lufia2FieldFindObjectInBox,      Lufia2FieldFindObjectInArea,
    Lufia2FieldFindCellBitsInArea,   Lufia2FieldFindEventConditionList,
    Lufia2FieldTestConditionList};
int main() {
    for (auto f : functions)
        if (!f)
            return 1;
    return 0;
}
