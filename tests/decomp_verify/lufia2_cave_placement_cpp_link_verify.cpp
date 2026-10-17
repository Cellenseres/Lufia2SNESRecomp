#include "lufia2/ancient_cave.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {
    Lufia2CaveResolveTilePosition,  Lufia2CaveProbeObjectPlacement,
    Lufia2CavePlaceRandomObject,    Lufia2CaveProbeNearbyChest,
    Lufia2CaveProbeChestAtPosition, Lufia2CaveTryPlaceRoomChest,
    Lufia2CaveDrawRandomBelow,      Lufia2CaveDrawRandomIndex,
    Lufia2CaveDrawRandomMean};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
