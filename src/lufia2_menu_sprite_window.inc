/* Included beside the PPU window helpers, before menu colour resolution.
 * Both the retained renderer and its reference consume this same capture. */
static void PpuMaskMenuSpriteCapture(Ppu *ppu,unsigned line) {
  uint32_t *pixels=Lufia2MenuUiSpriteCaptureRow(ppu,line);
  if (!pixels) return;
  if (!IS_SCREEN_ENABLED(ppu,false,4)) {
    memset(pixels,0,256u*sizeof *pixels);
    return;
  }
  if (!IS_SCREEN_WINDOWED(ppu,false,4)) return;
  PpuWindows window;
  PpuWindows_Calc(&window,ppu,4,line);
  for (unsigned span=0;span<window.nr;++span) {
    if (!(window.bits&(1u<<span))) continue;
    const int left=IntMax(window.edges[span],0);
    const int right=IntMin(window.edges[span+1],256);
    if (left<right) memset(pixels+left,0,(unsigned)(right-left)*sizeof *pixels);
  }
}
