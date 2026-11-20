/* Included by the generated PPU adapter so the maintained CPU window rules
 * remain authoritative. Resolve source entries only: no guest advancement,
 * sprite evaluation, palette decoding, or software framebuffer generation. */
bool Lufia2PpuResolveShadowLine(Ppu *ppu, unsigned line, unsigned layer,
                              uint8_t *map, uint8_t *known) {
    if (!ppu || !map || !known || !line || line > 224 || layer >= 2 ||
        PPU_mode(ppu) != 1 || PPU_bigTiles(ppu, layer) ||
        PPU_mosaicEnabled(ppu, layer) ||
        ((ppu->wsLayerRepeat | ppu->wsLayerMirror) & (1u << layer))) return false;
    if (PPU_forcedBlank(ppu)) return true;
    const unsigned sy = line + ppu->vScroll[layer];
    unsigned row = PPU_bgTilemapAdr(ppu, layer) + ((sy >> 3 & 31u) << 5);
    if ((sy & 256u) && PPU_bgTilemapHigher(ppu, layer))
        row += PPU_bgTilemapWider(ppu, layer) ? 2048u : 1024u;
    PpuWindows resolved;
    bool have_resolved = false;
    for (unsigned sub = 0; sub < 2; ++sub) {
        if (!IS_SCREEN_ENABLED(ppu, sub, layer)) continue;
        PpuWindows window;
        if (IS_SCREEN_WINDOWED(ppu, sub, layer))
            PpuWindows_Calc(&window, ppu, layer, (int)line);
        else PpuWindows_Clear(&window, ppu, layer, (int)line);
        /* The subscreen often shows the same spans; they select the same
         * entries, which the main pass has already written. */
        if (have_resolved && window.nr == resolved.nr && window.bits == resolved.bits &&
            !memcmp(window.edges, resolved.edges, (window.nr + 1u) * sizeof window.edges[0]))
            continue;
        resolved = window;
        have_resolved = true;
        for (unsigned span = 0; span < window.nr; ++span) {
            if (window.bits & (1u << span)) continue;
            for (int screen_x = window.edges[span]; screen_x < window.edges[span + 1];) {
                const unsigned sx = (unsigned)(screen_x + ppu->hScroll[layer]);
                const unsigned address = (row + (sx >> 3 & 31u) +
                    ((sx & 256u) && PPU_bgTilemapWider(ppu, layer) ? 1024u : 0u)) & 32767u;
                /* WsShadowTile returns the VRAM entry for any whole native chunk. */
                const uint16_t entry = screen_x >= 0 && screen_x + 8 <= 256
                    ? PpuRenderVram(ppu)[address]
                    : WsShadowTile((int)layer, screen_x, sy,
                        ppu->hScroll[layer], (uint16_t)address, PpuRenderVram(ppu)[address]);
                const unsigned cell = ((sy >> 3 & 63u) * 64u) + (sx >> 3 & 63u);
                uint8_t *packed = map + cell * 4u;
                /* A window, raster scroll or world phase can select different
                 * source entries for the same wrapped map cell. Such a frame
                 * cannot use a single immutable tilemap; fail before submit. */
                if (known[cell] && (uint16_t)(packed[0] | packed[1] << 8 |
                    (packed[2] & 7u) << 10 | (packed[3] & 3u) << 14 |
                    (packed[3] & 4u) << 11) != entry) return false;
                packed[0] = (uint8_t)entry;
                packed[1] = (uint8_t)((entry >> 8) & 3u);
                packed[2] = (uint8_t)((entry >> 10) & 7u);
                packed[3] = (uint8_t)(((entry >> 14) & 3u) | ((entry >> 11) & 4u));
                known[cell] = 1;
                screen_x += (int)(8u - (sx & 7u));
            }
        }
    }
    return true;
}

/* Field by field: runs on every recorded line, and the C library compares
 * these bytes one at a time. */
static bool ShadowRowKeyMatches(const Lufia2PpuShadowRowKey *a,
    const Lufia2PpuShadowRowKey *b, const uint8_t *regs) {
    if (a->native_row != b->native_row || a->history_row != b->history_row ||
        a->world_row != b->world_row || a->world_x != b->world_x ||
        a->world_y != b->world_y || a->scroll_y != b->scroll_y ||
        a->extra_left != b->extra_left || a->extra_right != b->extra_right ||
        a->margin_left != b->margin_left || a->margin_right != b->margin_right ||
        a->expand_layers != b->expand_layers || a->expand_windows != b->expand_windows)
        return false;
    for (unsigned i = 0; i < sizeof a->regs; i += 4u) {
        uint32_t x, y;
        memcpy(&x, a->regs + i, sizeof x);
        memcpy(&y, regs + i, sizeof y);
        if (x != y)
            return false;
    }
    return true;
}
bool Lufia2PpuResolveShadowLineCached(Ppu *ppu, unsigned line, unsigned layer,
    uint8_t *map, uint8_t *known, Lufia2PpuShadowMemo *memo) {
    if (!memo || !ppu || !map || !known || !line || line > 224 || layer >= 2)
        return Lufia2PpuResolveShadowLine(ppu, line, layer, map, known);
    if (PPU_mode(ppu) != 1 || PPU_bigTiles(ppu, layer) ||
        PPU_mosaicEnabled(ppu, layer) ||
        ((ppu->wsLayerRepeat | ppu->wsLayerMirror) & (1u << layer))) {
        memo->valid = false;
        return false;
    }
#if defined(SNESRECOMP_REVERSE_DEBUG) && SNESRECOMP_REVERSE_DEBUG
    /* Keep the debugger's per-line window observations on the reference path. */
    return Lufia2PpuResolveShadowLine(ppu, line, layer, map, known);
#else
    Lufia2PpuShadowRowKey key;
    const uint32_t sy = line + ppu->vScroll[layer];
    const uint32_t scroll_y = WsShadowScrollY((int)layer);
    const uint32_t relative = (sy - scroll_y) & 0x3ffu;
    key.native_row = sy >> 3;
    /* Both shadow-history variants are certified. Scrolls latched before an
     * IRQ need not agree with this line's PPU scroll; their row boundaries
     * therefore cannot be inferred from the native tilemap row alone. */
    key.history_row = (relative + (scroll_y & 7u)) >> 3;
    key.world_y = WsShadowWorldY((int)layer);
    key.world_row = (relative + key.world_y) >> 3;
    key.world_x = WsShadowWorldX((int)layer);
    key.scroll_y = scroll_y;
    key.extra_left = ppu->extraLeftCur;
    key.extra_right = ppu->extraRightCur;
    key.margin_left = PpuWidescreenLayerExtra(ppu, layer, (int)line, ppu->extraLeftCur);
    key.margin_right = PpuWidescreenLayerExtra(ppu, layer, (int)line, ppu->extraRightCur);
    key.expand_layers = ppu->wsWindowExpandLayers;
    key.expand_windows = ppu->wsWindowExpandWindows;
    if (memo->valid && ShadowRowKeyMatches(&memo->key, &key, (const uint8_t *)&ppu->inidisp)) {
        ++memo->reused;
        return true;
    }
    memcpy(key.regs, &ppu->inidisp, sizeof key.regs);
    const bool ok = Lufia2PpuResolveShadowLine(ppu, line, layer, map, known);
    memo->valid = ok;
    if (ok) { memo->key = key; ++memo->resolved; }
    return ok;
#endif
}
