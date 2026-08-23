# Recompiler configuration

This directory contains the game-specific control-flow information used by
`snesrecomp`.

`bank00.cfg` seeds the native interrupt vectors from the ROM.

`bank82.cfg` describes the inline pointer dispatcher at `$82:8041`. The
dispatcher consumes its local JSR frame before transferring to the selected
handler, so the site uses `ptrtail_popcall`.

These files are maintained source files and belong in Git. Generated C under
`src/gen/` is derived from the ROM and is not committed.
