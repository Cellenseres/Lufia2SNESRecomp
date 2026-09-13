# Overlay skin assets

The desktop overlay looks for these optional files beside the executable:

```text
assets/img/lufia2_menu_panel.tga
assets/img/lufia2_menu_panel.9slice
```

`lufia2_menu_panel.tga` must be an uncompressed 24-bit or 32-bit true-colour
TGA. Alpha is supported. Keep pixel-art assets at their native SNES resolution.
The default Lufia theme uses frame-space coordinates, so one asset pixel gets
exactly the same final transform as one game pixel (including aspect correction,
window resizing, fullscreen, and letterboxing). The overlay is still composited
separately after the game and shaders; it is never written into the guest frame.

Extracted game artwork is intentionally ignored by Git. This keeps proprietary
Lufia artwork out of the repository while still allowing the local runtime to
load it from this directory.

The `.9slice` file contains four source-pixel margins in this order:

```text
left top right bottom [tile]
```

For example:

```text
16 16 16 16 tile
```

The optional `tile` word repeats all scalable regions instead of stretching
them. This is the faithful choice for SNES artwork: Lufia's edge motifs repeat
on a 16-pixel cadence and must not be resampled. Without `tile`, the old
nearest-neighbour stretch behaviour remains available. Put every rounded
corner, highlight, shadow, and border pixel inside its fixed margin.

```text
+--------+----------------+--------+
| corner |   top edge     | corner |
+--------+----------------+--------+
|  left  | repeat/stretch | right  |
+--------+----------------+--------+
| corner |  bottom edge   | corner |
+--------+----------------+--------+
```

The supplied 120x88 clean panel is a 15x11-tile construction. It compacts
losslessly to 48x48: 16 pixels of fixed corner, one 16x16 repeat cell, and
16 pixels of the opposite corner. Recreate it from a native frame crop with:

```powershell
python scripts/ui_asset_tool.py panel.png `
  assets/img/lufia2_menu_panel.tga `
  --slice 16 16 16 16 --tile 16x16 --compact --preview 240x88
```

The extraction tool can also remove text already present in a captured panel
by filling the stretchable centre from a clean sampled pixel. Coordinates for
`--fill-center` refer to the cropped and downscaled output image.

If either file is absent or invalid, the built-in navy/gold panel is used. A
missing `.9slice` file alone falls back to one quarter of the texture size on
each side.

The local Lufia panel also selects the game's flat dark-brown text colour
(`#422921`). Text shadows are disabled for the default skin; a custom theme can
enable `text_shadow` explicitly. Further colour, bitmap-font, scale, and spacing
changes live in `src/lufia2_overlay_ui.c`; the shared presenter deliberately has
no game-specific styling.
