#include "lufia2/menu.h"
#include "lufia2/battle.h"
int main() {
    auto volatile entry0 = &Lufia2BattleResolveRecoverableStatuses;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2BattleCheckEnemyGraphicsCapacity;
    if (!entry1)
        return 1;
    auto volatile entry2 = &Lufia2SelectScreenQueueTilemapCopy;
    if (!entry2)
        return 1;
    return 0;
}
