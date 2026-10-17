#include "lufia2/item.h"
#include "lufia2/battle.h"
int main() {
    auto volatile entry0 = &Lufia2InventoryRemoveQuantity;
    if (!entry0)
        return 1;
    auto volatile entry1 = &Lufia2InventoryConsumeRequestedItem;
    if (!entry1)
        return 1;
    auto volatile entry2 = &Lufia2BattleResetTargetMotion;
    if (!entry2)
        return 1;
    auto volatile entry3 = &Lufia2BattleRefreshInactiveEnemySprites;
    if (!entry3)
        return 1;
    auto volatile entry4 = &Lufia2BattlePrepareAlternateAction;
    if (!entry4)
        return 1;
    auto volatile entry5 = &Lufia2BattleBuildActionAdjustments;
    if (!entry5)
        return 1;
    auto volatile entry6 = &Lufia2BattleCalculateActionBias;
    if (!entry6)
        return 1;
    auto volatile entry7 = &Lufia2BattleStoreActionResult;
    if (!entry7)
        return 1;
    auto volatile entry8 = &Lufia2BattleInvalidateTurnQueueActor;
    if (!entry8)
        return 1;
    auto volatile entry9 = &Lufia2BattleRemoveTurnQueueActor;
    if (!entry9)
        return 1;
    auto volatile entry10 = &Lufia2BattleAccumulateActionAdjustment;
    if (!entry10)
        return 1;
    auto volatile entry11 = &Lufia2BattleReadActorBiasFlag;
    if (!entry11)
        return 1;
    return 0;
}
