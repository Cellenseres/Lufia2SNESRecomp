#include "lufia2/ancient_cave.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {
    Lufia2CaveClearVisitedCells,     Lufia2CaveCountOccupiedCells,
    Lufia2CaveOffsetStairPosition,   Lufia2CaveResolveCellOrigin,
    Lufia2CaveLinkRoomCells,         Lufia2CaveMergeRoomCells,
    Lufia2CaveFillRoomRectangle,     Lufia2CaveResolveCellIndex,
    Lufia2CaveResolveBlockOffset,    Lufia2CaveResolveTileOffset,
    Lufia2CaveResolveWordTileOffset, Lufia2CavePickUnvisitedCell,
    Lufia2CaveFindUnvisitedCell,     Lufia2CaveAppendRoomLink,
    Lufia2CaveReplaceUpperTileIndex};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
