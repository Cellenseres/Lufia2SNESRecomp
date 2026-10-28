static bool MenuWramSpanHdmaSafe(void) {
    if (g_snes->hdmaBeamOff || g_snes->vPos >= 225u)
        return true;
    if (g_snes->dma->hdmaPendingInit)
        return false;
    for (unsigned index = 0u; index < 8u; ++index) {
        const DmaChannel *channel = g_snes->dma->channel + index;
        if (!channel->hdmaActive || channel->terminated)
            continue;
        if (channel->fromB || channel->bAdr > 0x3cu ||
            (channel->aBank != 0x7eu && channel->aBank != 0x7fu) ||
            (channel->indirect && channel->indBank != 0x7eu && channel->indBank != 0x7fu))
            return false;
    }
    return true;
}
