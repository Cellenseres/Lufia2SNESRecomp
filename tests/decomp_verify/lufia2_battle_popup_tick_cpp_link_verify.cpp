#include "lufia2/battle.h"
typedef Lufia2ExecutionResult (*PopupFunction)(const Lufia2Memory *, Lufia2CpuState *);
static PopupFunction volatile popups[] = {Lufia2BattleBuildSlotPopup,
                                          Lufia2BattleBuildPopupRecord};
static auto volatile tick = Lufia2BattleApplyTargetStatusTick;
int main() {
    for (auto f : popups)
        if (!f)
            return 1;
    return tick ? 0 : 1;
}
