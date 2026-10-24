"""Resolve menu colours from freshly completed indices, preserving fallback."""


def transform(text):
    anchor = 'static void PpuHdMarkWindow('
    if text.count(anchor) != 1:
        raise ValueError('Menu colour renderer anchor changed')
    helper = '''static bool PpuResolveMenuPrimitiveColours(Ppu *ppu,unsigned line,
    unsigned pass,uint32_t *output) {
  Lufia2MenuUiPrimitiveRows rows;
  if (!lufia2_ppu_host_costs_enabled ||
      pass>=3 || !output ||
      !Lufia2MenuUiPrimitiveRowsForLine(ppu,line,&rows)) return false;
  /* The primary screen removes UI layers inside their capture rectangle.
   * Restore the isolated clone's subscreen from already decoded sub-BG rows
   * and the qualified OBJ capture, without another tile or sprite walk. */
  const unsigned sub = PPU_preventMathMode(ppu)!=3 && PPU_addSubscreen(ppu) &&
      PPU_mathEnabled(ppu) ? ppu->screenEnabled[1] : 0;
  PpuWindows colour,objects,sub_objects;
  PpuWindows_Calc(&colour,ppu,5,line);
  IS_SCREEN_WINDOWED(ppu,0,4) ? PpuWindows_Calc(&objects,ppu,4,line) :
      PpuWindows_Clear(&objects,ppu,4,line);
  IS_SCREEN_WINDOWED(ppu,1,4) ? PpuWindows_Calc(&sub_objects,ppu,4,line) :
      PpuWindows_Clear(&sub_objects,ppu,4,line);
  memset(output,0,(256u+2u*ppu->extraLeftRight)*sizeof *output);
  static const unsigned layers[3]={0,2,4};
  const unsigned layer=layers[pass];
  unsigned colour_span=0,object_span=0,sub_span=0;
  for (unsigned x=0;x<256;++x) if(rows.pixels[pass][x]) {
    while(colour_span+1<colour.nr && (int)x>=colour.edges[colour_span+1]) ++colour_span;
    while(object_span+1<objects.nr && (int)x>=objects.edges[object_span+1]) ++object_span;
    while(sub_span+1<sub_objects.nr && (int)x>=sub_objects.edges[sub_span+1]) ++sub_span;
    const unsigned at=x+kPpuExtraLeftRight;
    uint16_t pixel=sub?ppu->bgBuffers[1].data[at]:0;
    for (unsigned layer=0;layer<=2;layer+=2) if(sub&(1u<<layer)) {
      const uint16_t candidate=ppu->overlayBuffers[layer].data[at];
      if(candidate>pixel) pixel=candidate;
    }
    if((sub&16u) && !(sub_objects.bits&(1u<<sub_span))) {
      const uint16_t candidate=ppu->overlayBuffers[kPpuOverlaySource_Obj].data[at];
      if(candidate>pixel) pixel=candidate;
    }
      uint16_t main=ppu->bgBuffers[0].data[at];
      uint16_t primitive=pass<2 ? rows.priority[pass][x] :
          !(objects.bits&(1u<<object_span)) ? ppu->overlayBuffers[kPpuOverlaySource_Obj].data[at] : 0;
      if (!(ppu->screenEnabled[0]&(1u<<layer))) primitive=0;
      if (primitive>main) main=primitive;
      output[x+ppu->extraLeftRight]=PpuHdColour(ppu,main,pixel,
          !!(colour.bits&(1u<<colour_span))) |
          0xff000000u;
  }
  return true;
}

'''
    text = text.replace(anchor, helper + anchor, 1)
    prototype = 'static void PpuDrawMenuOverlayLine(Ppu *ppu, unsigned y);'
    if text.count(prototype) != 1:
        raise ValueError('Menu callback prototype changed')
    text = text.replace(prototype, prototype +
        '\nstatic bool PpuResolveMenuPrimitiveColours(Ppu *ppu,unsigned line,unsigned pass,uint32_t *output);', 1)
    call = '''    PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);
    Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);'''
    if text.count(call) != 1:
        raise ValueError('Fresh primary menu indices anchor changed')
    return text.replace(call, '''    PpuWriteOverlayRenderLine(ppu, kPpuOverlaySource_Obj, y);
    Lufia2MenuUiPrimitiveRenderer(ppu,y,PpuResolveMenuPrimitiveColours);
    Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);''', 1)
