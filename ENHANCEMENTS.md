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

`assets/widescreen/lufia2.l2rooms` currently contains 76 authored maps,
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

## Widescreen room visibility

The margins follow the cells the game itself hides: a cell whose edge bits
are both set shows the map's first cell, as on the original screen. Actors in
the margins are hidden on such cells; the native view keeps all of them. This
also covers maps without room data, such as the Ancient Cave, Foomy Woods and
most maps from `0x4C`. On authored maps the room data still decides when the
margins open and what lies past the map's edges. When the game retraces its
hidden cells, the margins keep the old picture until the native screen
changes: a door's fade-out switches them at black.

`config.ini` selects the mode in `[Lufia2Features]` with
`WidescreenRoomVisibility = l2rooms`, `compare` or `game`. The
`LUFIA2_ROOM_VISIBILITY` environment variable overrides it, and the CMake cache
value `LUFIA2_ROOM_VISIBILITY_DEFAULT` sets the build default (`game`).
`l2rooms` restores the earlier behavior: room data on authored maps, the
inferred map elsewhere. `compare` draws authored maps from room data and
prints a `[room-compare]` line whenever the room or the game's hidden cells
change, counting the cells only one of them shows.

`LUFIA2_ROOM_VISIBILITY_DEBUG=1` draws hidden cells instead of hiding them and
tints them; cells whose edge is open get a second tint. Runtime-log builds
print `[room-vis]` lines with each map's hidden-cell counts and how well the
game's rule explains the native screen.
