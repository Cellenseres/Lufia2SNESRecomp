# Enhancements

SDL and OpenGL presentation now use the reusable `snesrecomp-platform` layer.
The launcher can select the renderer and OpenGL supports shader presets.

Next on the list:

- SDL GPU for wider platform support;
- widescreen with game-aware camera and HUD handling;
- optional MSU-1 music;
- save states and a safe soft reset.

Generated files under `src/gen/` are never edited by hand.

## Widescreen room-data coverage

`assets/widescreen/lufia2.l2rooms` (format version 5) currently contains 76
authored maps with 6,676 painted cells and a fill per map. It was converted
from the version 4 file, which also held 224 rooms and 373 room transitions;
those are gone, as the game's own hidden cells now decide what shows.
Story-sequence coverage reaches North Dungeon B4 (`0x49`), with these authored
location groups:

- Arek Daos Shrine; Elcid; Secret Skills Cave; Cave to Sundletan; Sundletan;
  Lake Cave; and the Shrine to Alunze Kingdom;
- Alunze Castle and Kingdom; Alunze North Shrine; Alunze Northwest Cave;
  Tanbel; Tanbel Southeast Tower; Clamento; and Ruby Cave;
- the route to Parcelyte; Parcelyte and its castle; Treasure Sword Shrine; the
  route to Gordovan; Gordovan and Gordovan West Tower;
- Merix Village; Cave Bridge; Bound Kingdom; and North Dungeon B1-B4.

The data also includes the ROM's unnamed maps `0x1D`, `0x36`, and `0x4A`, plus
Small Shrine to Aleyn (`0x55`) and both Port Town of Aleyn maps (`0x5D`-`0x5E`).
Foomy Woods (`0x0D`) is the sole missing map inside the `0x01`-`0x4A` range.
Coverage here means widescreen data has been authored; it is not a substitute for
in-game verification of every camera position and transition.

## Widescreen visibility

The margins follow the game's hidden cells, including on maps without authored
data such as the Ancient Cave and Foomy Woods. Painted cells override the margins
inside and outside map bounds; fills apply only outside. The native 256 columns
keep the game's picture. Room changes wait for the native redraw or fade.

The v5 `.l2rooms` file stores paint and fills per map. Versions 1-4 still load:
room paint is combined in room-ID order, and the most common fill wins.

The old `l2rooms` and `compare` settings are retired. Visibility always follows
the game. `LUFIA2_ROOM_VISIBILITY_DEBUG=1` tints hidden and open cells;
runtime-log builds report `[room-vis]` diagnostics.
