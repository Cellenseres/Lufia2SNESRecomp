#include "lufia2/menu.h"
int main() {
    auto volatile entry0 = &Lufia2SelectScreenClearSpriteStatus;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2SelectScreenClearState;
    if (!entry1)
        return 1;
    auto volatile entry2 = &Lufia2SelectScreenConfigureHdmaSlot;
    if (!entry2)
        return 1;
    auto volatile entry3 = &Lufia2SelectScreenInitializeLayout;
    if (!entry3)
        return 1;
    auto volatile entry4 = &Lufia2SelectScreenControl;
    if (!entry4)
        return 1;
    return 0;
}
