# Recompiler configuration

This directory contains the game-specific control-flow information used by
`snesrecomp`.

`bank00.cfg` seeds the native interrupt vectors from the ROM.

`bank82.cfg` describes the inline pointer dispatcher at `$82:8041`. The
dispatcher consumes its local JSR frame before transferring to the selected
handler, so the site uses `ptrtail_popcall`.

There is deliberately no `bank81.cfg`. `$81:895E` is not promotable: the
dispatch frame it reaches through `$81:92BE` has no statically provable exit.

`bank00.cfg` also keeps `$05:8F1B` on LLE. That routine constructs a dynamic
PEA/PHA/RTS call frame and is unsafe to split across AOT and LLE.

These files are maintained source files and belong in Git. Generated C under
`src/gen/` is derived from the ROM and is not committed.
