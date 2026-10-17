#include "lufia2/menu.h"
int main() {
    auto volatile entry0 = &Lufia2SelectScreenLoadGraphics;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2SelectScreenLoadPalette;
    if (!entry1)
        return 1;
    auto volatile entry2 = &Lufia2SelectScreenLoadMaps;
    if (!entry2)
        return 1;
    auto volatile entry3 = &Lufia2SelectScreenConfigureDisplay;
    if (!entry3)
        return 1;
    auto volatile entry4 = &Lufia2SelectScreenRun;
    if (!entry4)
        return 1;
    return 0;
}
