#ifndef LUFIA2_PPU_MEMORY_H
#define LUFIA2_PPU_MEMORY_H
#include "snes/ppu.h"
#include "snesrecomp_platform/snes_ppu_row_memory.h"

/* Qualified isolated BG consumers do not depend on other BG scroll, tile
 * addresses or windows. Unknown/OBJ consumers retain the complete snapshot. */
static inline void Lufia2PpuRowRegisters(uint8_t out[PPU_SAVESTATE_REGS_SIZE],
    const Ppu *ppu,unsigned layers) {
    memcpy(out,&ppu->inidisp,PPU_SAVESTATE_REGS_SIZE);
    if(layers==255u) return;
    for(unsigned bg=0;bg<4;++bg) if(!(layers&(1u<<bg))) {
        out[6u+bg]=0;
        out[10u+bg/2u]&=(uint8_t)~(15u<<((bg&1u)*4u));
        out[14u+bg*2u]=out[15u+bg*2u]=0;
        out[22u+bg*2u]=out[23u+bg*2u]=0;
        out[5]&=(uint8_t)~(16u<<bg);
        out[48u+bg/2u]&=(uint8_t)~(15u<<((bg&1u)*4u));
        out[56]&=(uint8_t)~(3u<<(bg*2u));
        for(unsigned at=58;at<62;++at) out[at]&=(uint8_t)~(1u<<bg);
    }
}

/* Conservative horizontal read set for an ordinary Mode-1 index row.
 * Host remapping changes X only. Unknown modes retain all memory pages. */
static inline void Lufia2PpuIndexRowMemory(const Ppu *ppu,unsigned line,
    unsigned layers,SnesRecompPpuMemoryMask *memory) {
    memset(memory,PPU_mode(ppu)==1?0:255,sizeof *memory);
    if(PPU_mode(ppu)!=1) return;
    for(unsigned bg=0;bg<3;++bg) if(layers&(1u<<bg)) {
        SnesPpuBgState state={0};
        state.tilemap_word_addr=(uint16_t)PPU_bgTilemapAdr(ppu,bg);
        state.char_word_addr=(uint16_t)PPU_bgTileAdr(ppu,bg);
        state.bpp=bg==2?2u:4u;
        state.big_tiles=PPU_bigTiles(ppu,bg)!=0;
        state.wide=PPU_bgTilemapWider(ppu,bg)!=0;
        state.tall=PPU_bgTilemapHigher(ppu,bg)!=0;
        unsigned y=(PPU_mosaicEnabled(ppu,bg)&&PPU_mosaicSize(ppu)>1?
                    ppu->mosaicModulo[line]:line)+ppu->vScroll[bg];
        snesrecomp_ppu_mode1_row_memory(ppu->vram,&state,y,memory);
    }
}
#endif
