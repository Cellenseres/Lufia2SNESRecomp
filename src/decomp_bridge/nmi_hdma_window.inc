static bool NmiSpanAvoidsHdma(const Snes *snes, uint64_t budget,
                             bool bounded_span) {
    if (snes->hdmaBeamOff || snes->vPos >= 225u)
        return true;
    if (!bounded_span)
        return false;
    const unsigned fence = snes->hPos < 1024u ? 1024u : 1364u;
    return budget < fence - snes->hPos;
}
