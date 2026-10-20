# US shrine graphics

The US release's Submarine Shrine (map `CE`) and Ancient Cave final floor
(`F0`) use a different block layout from the shared US blockset. The latter
references blocks up to 780, but resource `147` contains only 719 entries.
The original loader faithfully reads the wrong definitions or adjacent bytes.

`data/shrine_graphics.bps` repairs four decoded resources:

- `10E`, `130`: the two map layers, with their cell flags preserved.
- `147`: the existing 719 blocks and properties, followed by 93 repaired blocks.
- `15C`: the existing 802 tiles, followed by 17 required tiles.

The two maps render identically to Frue Lufia 8.2. The five Daos Shrine maps
retain their original layouts, definitions, properties and pixels. Scripts,
actor records, palettes, executable code and non-resource data are unchanged.
The container is repacked in memory; its small overflow adds 1,224 bytes.

The host applies the embedded BPS once, after the original US SHA-1 check.
BPS checks the source, patch and result CRCs before replacing the buffer.
No patched ROM is written or included. The original semantic decompressor and
map loader remain unchanged. Reload the map when testing an old savestate.

The local map editor uses the same BPS. Its source-ROM identity remains the
original SHA-256. Existing paint for other maps remains valid.

## Sources

The graphics reference is the owner's `Frue_Lufia_v8-2_headerless.ips`.
Frue credits Relnqshd/Relinquished for the Ancient Cave/Submarine Shrine fix.
The repair also agrees with the scope of the older
[Fixxxer Deluxe](https://retrohackers.net/hack-details.php?id=229%2F) and the
[Ancient Cave patcher's Shrine Tileset option](https://rompatch.ancientcave.net/).
No translation, balancing or other Frue changes are included.

Decomp references: `Lufia2FieldLoadMapResources` (`80:EAE7`),
`SceneSelectTileset` and `Lufia2DecompressResource` (`80:8E9D`).
