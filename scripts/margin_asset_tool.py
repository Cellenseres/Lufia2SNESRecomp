#!/usr/bin/env python3
"""Build sparse SRMP margin packs from full-width authoring TGAs."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import struct
import sys
import tempfile
from dataclasses import dataclass


MAGIC = b"SRMP"
VERSION = 1
HEADER = struct.Struct("<4sHHII")
ENTRY = struct.Struct("<IHHHHBBHIIII")
ENCODING_INDEXED8 = 1
ENCODING_RGBA8888 = 2
INTRO_ASSET_ID = 0x00000000
BATTLE_ASSET_BASE = 0x01000000
AUTHORING_SIZE = (342, 224)
INTRO_CENTER_WIDTH = 256
BATTLE_CENTER_WIDTH = 240


@dataclass(frozen=True)
class Image:
    width: int
    height: int
    rgba: bytes


@dataclass(frozen=True)
class PackedAsset:
    asset_id: int
    name: str
    width: int
    height: int
    center_x: int
    center_width: int
    encoding: int
    palette: bytes
    rows: bytes
    data: bytes
    opaque_pixels: int
    transparent_pixels: int
    source_size: int
    source_colors: int
    palette_colors: int
    remapped_pixels: int


def load_tga(path: Path) -> Image:
    raw = path.read_bytes()
    if len(raw) < 18:
        raise ValueError(f"{path}: truncated TGA header")
    (id_length, color_map, image_type, _cm_first, _cm_length, _cm_depth,
     _x, _y, width, height, depth, descriptor) = struct.unpack_from(
        "<BBBHHBHHHHBB", raw, 0)
    if color_map != 0 or image_type not in (2, 10) or depth not in (24, 32):
        raise ValueError(
            f"{path}: expected an uncompressed/RLE true-colour 24/32-bit TGA")
    if not width or not height or width > 65535 or height > 65535:
        raise ValueError(f"{path}: invalid dimensions {width}x{height}")

    bytes_per_pixel = depth // 8
    position = 18 + id_length
    count = width * height
    file_pixels: list[bytes] = []
    while len(file_pixels) < count:
        if image_type == 2:
            packet_count = count - len(file_pixels)
            run = False
        else:
            if position >= len(raw):
                raise ValueError(f"{path}: truncated TGA RLE packet")
            header = raw[position]
            position += 1
            packet_count = (header & 0x7F) + 1
            run = bool(header & 0x80)
        if packet_count > count - len(file_pixels):
            raise ValueError(f"{path}: TGA packet crosses the image boundary")
        if run:
            if position + bytes_per_pixel > len(raw):
                raise ValueError(f"{path}: truncated TGA RLE pixel")
            pixel = raw[position:position + bytes_per_pixel]
            position += bytes_per_pixel
            file_pixels.extend([pixel] * packet_count)
        else:
            need = packet_count * bytes_per_pixel
            if position + need > len(raw):
                raise ValueError(f"{path}: truncated TGA pixels")
            for offset in range(position, position + need, bytes_per_pixel):
                file_pixels.append(raw[offset:offset + bytes_per_pixel])
            position += need
        if image_type == 2:
            break
    if len(file_pixels) != count:
        raise ValueError(f"{path}: incomplete TGA image")

    top_origin = bool(descriptor & 0x20)
    right_origin = bool(descriptor & 0x10)
    rgba = bytearray(count * 4)
    for file_index, bgra in enumerate(file_pixels):
        file_y, file_x = divmod(file_index, width)
        x = width - 1 - file_x if right_origin else file_x
        y = file_y if top_origin else height - 1 - file_y
        out = (y * width + x) * 4
        rgba[out:out + 4] = bytes((
            bgra[2], bgra[1], bgra[0], bgra[3] if bytes_per_pixel == 4 else 255))
    return Image(width, height, bytes(rgba))


def color_distance(a: bytes, b: bytes) -> int:
    """Return squared RGBA distance."""
    return sum((int(left) - int(right)) ** 2
               for left, right in zip(a, b))


def build_palette(
        color_counts: dict[bytes, int],
        max_colors: int = 256) -> tuple[list[bytes], dict[bytes, int], int]:
    """Keep common colours and map overflow to the nearest entry."""
    if not color_counts:
        return [], {}, 0
    if max_colors <= 0 or max_colors > 256:
        raise ValueError(f"invalid palette size limit {max_colors}")

    source_colors = list(color_counts.keys())
    if len(source_colors) <= max_colors:
        palette = source_colors
    else:
        # Stable sorting keeps first-seen order for ties.
        palette = [
            color for color, _count in sorted(
                color_counts.items(), key=lambda item: -item[1])
        ][:max_colors]

    lookup = {color: index for index, color in enumerate(palette)}
    remapped_pixels = 0
    if len(source_colors) > len(palette):
        for color in source_colors:
            if color in lookup:
                continue
            best_index = min(
                range(len(palette)),
                key=lambda index: color_distance(color, palette[index]))
            lookup[color] = best_index
            remapped_pixels += color_counts[color]
    return palette, lookup, remapped_pixels


def validate_pack(packed: bytes) -> None:
    """Mirror the runtime's structural/palette checks before replacing output."""
    if len(packed) < HEADER.size:
        raise ValueError("generated margin pack is truncated")
    magic, version, count, reserved0, reserved1 = HEADER.unpack_from(packed, 0)
    if magic != MAGIC or version != VERSION or reserved0 or reserved1:
        raise ValueError("generated margin pack has an invalid header")
    index_end = HEADER.size + ENTRY.size * count
    if index_end > len(packed):
        raise ValueError("generated margin pack index is truncated")

    asset_ids: set[int] = set()
    previous_id = -1
    for asset_index in range(count):
        offset = HEADER.size + ENTRY.size * asset_index
        (asset_id, width, height, center_x, center_width, encoding, flags,
         palette_count, palette_offset, rows_offset,
         data_offset, data_size) = ENTRY.unpack_from(packed, offset)
        if asset_id in asset_ids or asset_id <= previous_id or flags:
            raise ValueError(
                f"generated margin asset {asset_index} has an invalid index")
        asset_ids.add(asset_id)
        previous_id = asset_id
        center_end = center_x + center_width
        if not width or not height or not center_width or center_end > width:
            raise ValueError(
                f"generated margin asset {asset_index} has invalid dimensions")

        if encoding == ENCODING_INDEXED8:
            if not (1 <= palette_count <= 256):
                raise ValueError(
                    f"generated margin asset {asset_index} has invalid palette")
            palette_bytes = palette_count * 4
            if (palette_offset < index_end or palette_offset > len(packed) or
                    palette_bytes > len(packed) - palette_offset):
                raise ValueError(
                    f"generated margin asset {asset_index} palette is truncated")
            pixel_bytes = 1
        elif encoding == ENCODING_RGBA8888:
            if palette_count != 0:
                raise ValueError(
                    f"generated margin asset {asset_index} has RGBA palette data")
            pixel_bytes = 4
        else:
            raise ValueError(
                f"generated margin asset {asset_index} has unknown encoding")

        rows_bytes = (height + 1) * 4
        if (rows_offset < index_end or rows_offset > len(packed) or
                rows_bytes > len(packed) - rows_offset):
            raise ValueError(
                f"generated margin asset {asset_index} row table is truncated")
        if (data_offset < index_end or data_offset > len(packed) or
                data_size > len(packed) - data_offset):
            raise ValueError(
                f"generated margin asset {asset_index} data is truncated")

        rows = struct.unpack_from(f"<{height + 1}I", packed, rows_offset)
        if rows[0] != 0 or rows[-1] != data_size:
            raise ValueError(
                f"generated margin asset {asset_index} has invalid row bounds")
        data = packed[data_offset:data_offset + data_size]
        for y in range(height):
            position, end = rows[y], rows[y + 1]
            if position > end or end > data_size:
                raise ValueError(
                    f"generated margin asset {asset_index} has invalid row offsets")
            while position < end:
                if end - position < 4:
                    raise ValueError(
                        f"generated margin asset {asset_index} has a short run")
                x, length = struct.unpack_from("<HH", data, position)
                position += 4
                if (not length or x + length > width or
                        not (x + length <= center_x or x >= center_end)):
                    raise ValueError(
                        f"generated margin asset {asset_index} has invalid run")
                byte_count = length * pixel_bytes
                if byte_count > end - position:
                    raise ValueError(
                        f"generated margin asset {asset_index} run is truncated")
                if encoding == ENCODING_INDEXED8:
                    for value in data[position:position + length]:
                        if value >= palette_count:
                            raise ValueError(
                                f"generated margin asset {asset_index} "
                                f"contains invalid palette index {value}")
                position += byte_count


def margin_ranges(width: int, center_x: int, center_end: int) -> tuple[range, range]:
    return range(center_x), range(center_end, width)


def pack_asset(
        path: Path, asset_id: int, name: str, center_width: int,
        expected_size: tuple[int, int] | None = None) -> PackedAsset:
    image = load_tga(path)
    if expected_size and (image.width, image.height) != expected_size:
        raise ValueError(
            f"{path}: expected {expected_size[0]}x{expected_size[1]}, "
            f"got {image.width}x{image.height}")
    if center_width <= 0 or center_width >= image.width:
        raise ValueError(
            f"{path}: centre width {center_width} does not fit {image.width}px")
    remainder = image.width - center_width
    if remainder & 1:
        raise ValueError(
            f"{path}: {image.width}px cannot centre a {center_width}px image")
    center_x = remainder // 2
    center_end = center_x + center_width

    color_counts: dict[bytes, int] = {}
    opaque_pixels = 0
    transparent_pixels = 0
    spans = margin_ranges(image.width, center_x, center_end)
    for y in range(image.height):
        for span in spans:
            for x in span:
                at = (y * image.width + x) * 4
                pixel = image.rgba[at:at + 4]
                if pixel[3]:
                    opaque_pixels += 1
                    color_counts[pixel] = color_counts.get(pixel, 0) + 1
                else:
                    transparent_pixels += 1

    palette_colors, palette_lookup, remapped_pixels = build_palette(color_counts)
    # Indexed8 requires at least one palette entry.
    indexed = bool(palette_colors)
    encoding = ENCODING_INDEXED8 if indexed else ENCODING_RGBA8888
    palette = b"".join(palette_colors) if indexed else b""
    data = bytearray()
    row_offsets = [0]
    for y in range(image.height):
        x = 0
        while x < image.width:
            if center_x <= x < center_end:
                x = center_end
                continue
            pixel_at = (y * image.width + x) * 4
            if image.rgba[pixel_at + 3] == 0:
                x += 1
                continue
            start = x
            while x < image.width and not (center_x <= x < center_end):
                at = (y * image.width + x) * 4
                if image.rgba[at + 3] == 0 or x - start == 65535:
                    break
                x += 1
            length = x - start
            data += struct.pack("<HH", start, length)
            for px in range(start, x):
                at = (y * image.width + px) * 4
                rgba = image.rgba[at:at + 4]
                if indexed:
                    data.append(palette_lookup[rgba])
                else:
                    data += rgba
        row_offsets.append(len(data))
    rows = struct.pack(f"<{len(row_offsets)}I", *row_offsets)
    return PackedAsset(
        asset_id, name, image.width, image.height, center_x, center_width,
        encoding, palette, rows, bytes(data), opaque_pixels,
        transparent_pixels, path.stat().st_size, len(color_counts),
        len(palette_colors), remapped_pixels)


def build_pack(assets: list[PackedAsset]) -> bytes:
    ordered = sorted(assets, key=lambda asset: asset.asset_id)
    if len({asset.asset_id for asset in ordered}) != len(ordered):
        raise ValueError("duplicate margin asset id")
    if len(ordered) > 65535:
        raise ValueError("too many margin assets")

    cursor = HEADER.size + ENTRY.size * len(ordered)
    payload = bytearray()
    entries = bytearray()
    for asset in ordered:
        palette_offset = cursor if asset.palette else 0
        payload += asset.palette
        cursor += len(asset.palette)
        rows_offset = cursor
        payload += asset.rows
        cursor += len(asset.rows)
        data_offset = cursor
        payload += asset.data
        cursor += len(asset.data)
        palette_count = len(asset.palette) // 4
        entries += ENTRY.pack(
            asset.asset_id, asset.width, asset.height,
            asset.center_x, asset.center_width,
            asset.encoding, 0, palette_count,
            palette_offset, rows_offset, data_offset, len(asset.data))
    return HEADER.pack(MAGIC, VERSION, len(ordered), 0, 0) + entries + payload


def replace_atomically(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent)
    os.close(descriptor)
    temporary = Path(temporary_name)
    try:
        temporary.write_bytes(data)
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def print_stats(asset: PackedAsset) -> None:
    margin_pixels = (asset.width - asset.center_width) * asset.height
    center_bytes = asset.center_width * asset.height * 4
    packed_size = ENTRY.size + len(asset.palette) + len(asset.rows) + len(asset.data)
    encoding = "Indexed8" if asset.encoding == ENCODING_INDEXED8 else "RGBA8888"
    print(asset.name)
    print(f"  Raw authoring file: {asset.source_size:,} bytes")
    print(f"  Center discarded:   {center_bytes:,} raw RGBA bytes")
    print(f"  Margin pixels:       {margin_pixels:,}")
    print(f"  Transparent pixels:  {asset.transparent_pixels:,}")
    print(f"  Stored pixels:       {asset.opaque_pixels:,}")
    print(f"  Source colours:      {asset.source_colors:,}")
    print(f"  Palette colours:     {asset.palette_colors:,}")
    if asset.remapped_pixels:
        print(f"  Palette repaired:    {asset.remapped_pixels:,} pixels -> nearest colour")
    elif not asset.opaque_pixels:
        print("  Palette:             empty margin (valid RGBA/no colour table)")
    print(f"  Encoding:            {encoding}")
    print(f"  Packed size:         {packed_size:,} bytes")


def parse_battle_id(path: Path) -> int | None:
    stem = path.stem.lower()
    if not stem.startswith("battle_") or len(stem) != len("battle_00"):
        return None
    try:
        value = int(stem[-2:], 16)
    except ValueError:
        return None
    return value if 0 <= value <= 0x18 else None


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Build sparse SRMP packs from full-width authoring TGAs")
    parser.add_argument("--output", type=Path,
                        default=Path("assets/widescreen/lufia2.l2mp"))
    parser.add_argument("--intro", type=Path,
                        default=Path("assets/img/lufia2_intro_margins.tga"))
    parser.add_argument("--battle-dir", type=Path,
                        default=Path("assets-src/battle_margins"))
    parser.add_argument(
        "--center-width", type=int, default=INTRO_CENTER_WIDTH,
        help="reference centre discarded for intro assets (default: 256)")
    parser.add_argument(
        "--battle-center-width", type=int, default=BATTLE_CENTER_WIDTH,
        help=("battle centre kept native at runtime; 240 stores an 8px "
              "authoring overlap per side to cover Lufia's black guard columns"))
    parser.add_argument("--no-intro", action="store_true")
    args = parser.parse_args(argv)

    try:
        assets: list[PackedAsset] = []
        if not args.no_intro:
            if not args.intro.is_file():
                raise ValueError(f"missing intro authoring image: {args.intro}")
            assets.append(pack_asset(
                args.intro, INTRO_ASSET_ID, "Intro", args.center_width,
                AUTHORING_SIZE))
        if args.battle_dir.is_dir():
            battle_count = 0
            for path in sorted(args.battle_dir.glob("battle_*.tga")):
                battle_id = parse_battle_id(path)
                if battle_id is None:
                    print(f"warning: ignoring unexpected authoring file {path}",
                          file=sys.stderr)
                    continue
                assets.append(pack_asset(
                    path, BATTLE_ASSET_BASE | battle_id,
                    f"Battle {battle_id:02X}", args.battle_center_width,
                    AUTHORING_SIZE))
                battle_count += 1
            print(f"Battle authoring images found: {battle_count}/25")
        else:
            print("Battle authoring images found: 0/25")
        packed = build_pack(assets)
        validate_pack(packed)
        replace_atomically(args.output, packed)
        for asset in assets:
            print_stats(asset)
        print(f"Pack: {args.output} ({len(packed):,} bytes, {len(assets)} assets)")
        print("Tile deduplication: not used (measured sparse/RLE size above)")
        return 0
    except (OSError, ValueError, struct.error) as error:
        print(f"margin_asset_tool: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
