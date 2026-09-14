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

`data/widescreen/lufia2_rooms.l2rooms` currently contains 76 authored maps,
224 rooms, and 373 directional room transitions. Story-sequence coverage
reaches North Dungeon B4 (`0x49`), with these authored location groups:

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
Coverage here means room metadata has been authored; it is not a substitute for
in-game verification of every camera position and transition.
