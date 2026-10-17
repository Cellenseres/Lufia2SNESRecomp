#include "lufia2/ancient_cave.h"
#include "lufia2/battle.h"
int main() {
    auto volatile entry0 = &Lufia2AncientCaveAppendSavedItem;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2AncientCaveTestBlueItem;
    if (!entry1)
        return 1;
    auto volatile entry2 = &Lufia2BattleUploadVramQueue;
    if (!entry2)
        return 1;
    auto volatile entry3 = &Lufia2BattleClearActionWorkspace;
    if (!entry3)
        return 1;
    return 0;
}
