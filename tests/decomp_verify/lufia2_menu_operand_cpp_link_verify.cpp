#include "lufia2/menu.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {
    Lufia2MenuReadPointerOperand, Lufia2MenuReadOffsetPointerOperand,
    Lufia2MenuLoadNumberBytes, Lufia2MenuReadBranchOperands};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
