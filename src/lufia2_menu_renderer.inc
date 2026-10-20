/* Included beside the native PPU's layer and window helpers. */
static uint32_t PpuMenuColour(const Ppu *ppu, uint16_t main, uint16_t sub,
                              bool clip, bool math) {
  unsigned colour = ppu->cgram[main & 255];
  unsigned r = clip ? 0 : colour & 31;
  unsigned g = clip ? 0 : (colour >> 5) & 31;
  unsigned b = clip ? 0 : (colour >> 10) & 31;
  const uint8_t *map = ppu->brightnessMult;
  if (math && (PPU_mathEnabled(ppu) & (1u << ((main >> 8) & 15)))) {
    unsigned other = ppu->fixedColor;
    if (!PPU_addSubscreen(ppu) || (sub & 255)) {
      if (PPU_addSubscreen(ppu)) other = ppu->cgram[sub & 255];
      if (PPU_halfColor(ppu)) map = ppu->brightnessMultHalf;
    }
    unsigned r2 = other & 31, g2 = (other >> 5) & 31, b2 = (other >> 10) & 31;
    if (PPU_subtractColor(ppu)) {
      r = r > r2 ? r - r2 : 0;
      g = g > g2 ? g - g2 : 0;
      b = b > b2 ? b - b2 : 0;
    } else {
      r += r2; g += g2; b += b2;
    }
  }
  return (uint32_t)map[r] << 16 | (uint32_t)map[g] << 8 | map[b];
}

static void PpuMenuSpriteWindow(Ppu *ppu, unsigned y, bool sub, uint8_t visible[256]) {
  memset(visible, 0, 256);
  if (!IS_SCREEN_ENABLED(ppu, sub, 4)) return;
  PpuWindows win;
  if (IS_SCREEN_WINDOWED(ppu, sub, 4)) PpuWindows_Calc(&win, ppu, 4, y);
  else PpuWindows_Clear(&win, ppu, 4, y);
  for (unsigned span = 0; span < win.nr; ++span) {
    int left = IntMax(win.edges[span], 0), right = IntMin(win.edges[span + 1], 256);
    if (!(win.bits & (1u << span)) && right > left) memset(visible + left, 1, right - left);
  }
}

static uint16_t PpuMenuSubpixel(Ppu *ppu, unsigned x, const uint8_t obj_visible[256]) {
  /* Captures remove these layers from both native screens. */
  unsigned i = x + kPpuExtraLeftRight;
  uint16_t pixel = ppu->bgBuffers[1].data[i];
  static const unsigned layers[] = {0, 2};
  for (unsigned layer = 0; layer < 2; ++layer) {
    unsigned source = layers[layer];
    if (IS_SCREEN_ENABLED(ppu, true, source) && ppu->overlayBuffers[source].data[i] > pixel)
      pixel = ppu->overlayBuffers[source].data[i];
  }
  if (obj_visible[x] && ppu->overlayBuffers[4].data[i] > pixel)
    pixel = ppu->overlayBuffers[4].data[i];
  return pixel;
}

static uint16_t PpuMenuHighlight(uint16_t pixel) {
  /* $82:9984: BG2 palette three becomes palette two. */
  return (pixel & 0x0ff0) == 0x0130 ? (pixel & ~0x70u) | 0x20u : pixel;
}

static void PpuDrawMenuOverlayLine(Ppu *ppu, unsigned y,
                                  const Lufia2MenuUiScanline *capture) {
  uint8_t main_obj[256], sub_obj[256];
  PpuMenuSpriteWindow(ppu, y, false, main_obj);
  PpuMenuSpriteWindow(ppu, y, true, sub_obj);
  bool has_sub = PPU_preventMathMode(ppu) != 3 && PPU_addSubscreen(ppu) &&
      PPU_mathEnabled(ppu) && ppu->screenEnabled[1];
  PpuPixelPrioBufs backdrop;
  if (capture->backdrop_vram) {
    const uint16_t *vram = ppu->renderVram;
    ppu->renderVram = capture->backdrop_vram;
    ClearBackdrop(ppu, &backdrop);
    PpuDrawBackground_4bpp_policy(ppu, &backdrop, y, false, 1, 0xb100, 0x7100,
        PPU_mosaicSize(ppu) > 1 && PPU_mosaicEnabled(ppu, 1));
    ppu->renderVram = vram;
  }
  PpuWindows win;
  PpuWindows_Calc(&win, ppu, 5, y);
  uint32_t *dst = (uint32_t *)(ppu->renderBuffer + (y - 1) * ppu->renderPitch);
  for (unsigned span = 0; span < win.nr; ++span) {
    bool inside = (win.bits & (1u << span)) != 0;
    bool clip = PpuHdWindowCondition(PPU_clipMode(ppu), inside);
    bool math = !PpuHdWindowCondition(PPU_preventMathMode(ppu), inside);
    for (int x = win.edges[span]; x < win.edges[span + 1]; ++x) {
      unsigned i = x + kPpuExtraLeftRight;
      uint16_t background = ppu->bgBuffers[0].data[i];
      uint16_t sub = has_sub ? ppu->bgBuffers[1].data[i] : 0x0500;
      if (x >= 0 && x < 256) {
        if (has_sub) sub = PpuMenuSubpixel(ppu, (unsigned)x, sub_obj);
        for (unsigned plane = 0; plane < 3; ++plane) {
          if (!capture->pixels[plane][x]) continue;
          uint16_t pixel = capture->priority[plane][x];
          if (plane == 2 && !main_obj[x]) pixel = 0;
          if (pixel < background) pixel = background;
          capture->pixels[plane][x] = PpuMenuColour(ppu, pixel, sub, clip, math) | 0xff000000u;
        }
      }
      uint16_t base = capture->backdrop_vram ? backdrop.data[i] : background;
      unsigned out = x + ppu->extraLeftRight;
      dst[out] = PpuMenuColour(ppu, base, sub, clip, math);
      if (capture->highlight)
        capture->highlight[out] = PpuMenuColour(ppu, PpuMenuHighlight(base), sub, clip, math);
    }
  }
}
