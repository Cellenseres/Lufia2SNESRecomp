"""Reuse certified battle pixels; execute guest qualification and UI normally."""


def transform(text):
    anchors = [
        ('  // Default background is backdrop\n', '''  if (lufia2_ppu_host_costs_enabled && Lufia2BattleEffectsReadRow(ppu, y)) {
    Lufia2BattleEffectsHudLine(ppu, y, PpuDrawBattleMarginLine);
    Lufia2MenuUiLine(ppu, y, PpuDrawMenuOverlayLine);
    return;
  }

  // Default background is backdrop
'''),
        ('  Lufia2BattleEffectsHudLine(ppu, y, PpuDrawBattleMarginLine);\n  Lufia2MenuUiLine',
         '  Lufia2BattleEffectsStoreRow(ppu, y);\n  Lufia2BattleEffectsHudLine(ppu, y, PpuDrawBattleMarginLine);\n  Lufia2MenuUiLine'),
    ]
    for old, new in anchors:
        if text.count(old) != 1:
            raise ValueError('PPU retained-row anchor mismatch: ' + old)
        text = text.replace(old, new)
    return text
