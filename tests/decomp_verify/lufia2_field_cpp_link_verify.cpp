#include "lufia2/field.h"

typedef Lufia2ExecutionResult (*FieldFunction)(const Lufia2Memory *, Lufia2CpuState *);
static FieldFunction volatile functions[] = {
    Lufia2FieldMarkVisibleActorCells,
    Lufia2FieldMarkListedCells,
    Lufia2FieldSpawnActorsInArea,
    Lufia2FieldRandomAreaCoordinate
};

int main(void) {
    for (unsigned i = 0; i < sizeof functions / sizeof functions[0]; ++i)
        if (!functions[i])
            return 1;
    return 0;
}
