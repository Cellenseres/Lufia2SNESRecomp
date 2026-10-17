#include "lufia2/ancient_cave.h"
#include "lufia2/world_map.h"
typedef Lufia2ExecutionResult (*Entry)(const Lufia2Memory *, Lufia2CpuState *);
static Entry volatile entries[] = {
    Lufia2CaveDrawTileBlock, Lufia2WorldMapUploadTileQueue,
    Lufia2WorldMapUploadTileColumn, Lufia2WorldMapUploadTileBlock};
int main() {
    for (auto entry : entries)
        if (!entry)
            return 1;
    return 0;
}
