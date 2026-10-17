#include "lufia2/menu.h"
int main() {
    auto volatile entry0 = &Lufia2SelectScreenFadeIn;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2SelectScreenFadeOut;
    if (!entry1)
        return 1;
    auto volatile entry2 = &Lufia2SelectScreenWaitForInput;
    if (!entry2)
        return 1;
    auto volatile entry3 = &Lufia2SelectScreenConfigureObject;
    if (!entry3)
        return 1;
    auto volatile entry4 = &Lufia2SelectScreenWaitForObjects;
    if (!entry4)
        return 1;
    return 0;
}
