#include "lufia2/menu.h"
int main() {
    auto volatile entry0 = &Lufia2SelectScreenPlaySequence;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2SelectScreenPlayAlternateSequence;
    if (!entry1)
        return 1;
    return 0;
}
