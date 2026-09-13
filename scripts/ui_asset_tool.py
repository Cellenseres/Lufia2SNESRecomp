#!/usr/bin/env python3
"""Crop pixel-art UI assets and preview their nine-slice behaviour."""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Sequence

try:
    from PIL import Image
except ImportError as exc:
    raise SystemExit(
        "Pillow is required. Install it with: python -m pip install Pillow"
    ) from exc


def positive(value: str) -> int:
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be greater than zero")
    return parsed


def validate_slices(size: tuple[int, int], slices: Sequence[int]) -> None:
    width, height = size
    left, top, right, bottom = slices
    if min(slices) < 0 or left + right >= width or top + bottom >= height:
        raise SystemExit(
            f"invalid slices {tuple(slices)} for a {width}x{height} texture"
        )


def fill_tiled(target: Image.Image, part: Image.Image,
               box: tuple[int, int, int, int]) -> None:
    left, top, right, bottom = box
    for y in range(top, bottom, part.height):
        for x in range(left, right, part.width):
            crop = part.crop((0, 0, min(part.width, right - x),
                              min(part.height, bottom - y)))
            target.paste(crop, (x, y), crop)


def nine_slice(source: Image.Image, size: tuple[int, int],
               slices: Sequence[int], tiled: bool = False) -> Image.Image:
    validate_slices(source.size, slices)
    width, height = size
    left, top, right, bottom = slices
    if left + right >= width or top + bottom >= height:
        raise SystemExit(f"preview size {width}x{height} is too small for the slices")
    sw, sh = source.size
    source_x = (0, left, sw - right, sw)
    source_y = (0, top, sh - bottom, sh)
    target_x = (0, left, width - right, width)
    target_y = (0, top, height - bottom, height)
    result = Image.new("RGBA", size)
    for row in range(3):
        for column in range(3):
            part = source.crop((source_x[column], source_y[row],
                                source_x[column + 1], source_y[row + 1]))
            target_size = (target_x[column + 1] - target_x[column],
                           target_y[row + 1] - target_y[row])
            box = (target_x[column], target_y[row],
                   target_x[column + 1], target_y[row + 1])
            if tiled and (row == 1 or column == 1):
                fill_tiled(result, part, box)
                continue
            if part.size != target_size:
                part = part.resize(target_size, Image.Resampling.NEAREST)
            result.paste(part, (target_x[column], target_y[row]), part)
    return result


def compact_tiled(source: Image.Image, slices: Sequence[int],
                  tile: tuple[int, int]) -> Image.Image:
    validate_slices(source.size, slices)
    left, top, right, bottom = slices
    tile_w, tile_h = tile
    if tile_w > source.width - left - right or \
            tile_h > source.height - top - bottom:
        raise SystemExit("tile is larger than the scalable source centre")
    xs = (0, left, left + tile_w, source.width - right, source.width)
    ys = (0, top, top + tile_h, source.height - bottom, source.height)
    output = Image.new("RGBA", (left + tile_w + right,
                                top + tile_h + bottom))
    dx = (0, left, left + tile_w, output.width)
    dy = (0, top, top + tile_h, output.height)
    for row, sy in enumerate((0, 1, 3)):
        for column, sx in enumerate((0, 1, 3)):
            part = source.crop((xs[sx], ys[sy], xs[sx + 1], ys[sy + 1]))
            output.paste(part, (dx[column], dy[row]), part)
    return output


def parse_size(value: str) -> tuple[int, int]:
    try:
        width, height = (int(part) for part in value.lower().split("x", 1))
    except (ValueError, TypeError) as exc:
        raise argparse.ArgumentTypeError("use WIDTHxHEIGHT, for example 512x150") from exc
    if width <= 0 or height <= 0:
        raise argparse.ArgumentTypeError("width and height must be positive")
    return width, height


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Crop a screenshot to PNG/BMP/TGA and create a 9-slice preview."
    )
    parser.add_argument("source", type=Path, help="PNG, BMP, TGA, or other Pillow input")
    parser.add_argument("output", type=Path, help="output .png, .bmp, or .tga")
    parser.add_argument("--crop", nargs=4, type=int, metavar=("X", "Y", "W", "H"),
                        help="crop rectangle in source-image pixels")
    parser.add_argument("--downscale", type=positive, default=1,
                        help="integer nearest-neighbour downscale (default: 1)")
    parser.add_argument("--slice", nargs=4, type=int,
                        metavar=("LEFT", "TOP", "RIGHT", "BOTTOM"),
                        help="write OUTPUT.9slice with these source-pixel margins")
    parser.add_argument("--fill-center", nargs=2, type=int, metavar=("X", "Y"),
                        help="erase menu text by filling the stretchable centre "
                             "with one sampled output pixel (requires --slice)")
    parser.add_argument("--tile", type=parse_size, metavar="WIDTHxHEIGHT",
                        help="tile scalable regions instead of stretching them")
    parser.add_argument("--compact", action="store_true",
                        help="reduce a tiled panel to corners plus one repeat "
                             "cell (requires --slice and --tile)")
    parser.add_argument("--preview", type=parse_size, metavar="WIDTHxHEIGHT",
                        help="also write a stretched PNG preview")
    args = parser.parse_args()

    image = Image.open(args.source).convert("RGBA")
    if args.crop:
        x, y, width, height = args.crop
        if width <= 0 or height <= 0 or x < 0 or y < 0 or \
                x + width > image.width or y + height > image.height:
            raise SystemExit(f"crop {tuple(args.crop)} is outside {image.size}")
        image = image.crop((x, y, x + width, y + height))
    if args.downscale != 1:
        if image.width % args.downscale or image.height % args.downscale:
            raise SystemExit("cropped width and height must be divisible by --downscale")
        image = image.resize((image.width // args.downscale,
                              image.height // args.downscale),
                             Image.Resampling.NEAREST)

    if args.fill_center:
        if not args.slice:
            raise SystemExit("--fill-center requires --slice")
        validate_slices(image.size, args.slice)
        sample_x, sample_y = args.fill_center
        if not (0 <= sample_x < image.width and 0 <= sample_y < image.height):
            raise SystemExit(f"sample {tuple(args.fill_center)} is outside {image.size}")
        left, top, right, bottom = args.slice
        colour = image.getpixel((sample_x, sample_y))
        image.paste(colour, (left, top, image.width - right, image.height - bottom))

    if args.compact:
        if not args.slice or not args.tile:
            raise SystemExit("--compact requires --slice and --tile")
        image = compact_tiled(image, args.slice, args.tile)

    suffix = args.output.suffix.lower()
    if suffix not in {".png", ".bmp", ".tga"}:
        raise SystemExit("output extension must be .png, .bmp, or .tga")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    image.save(args.output)
    print(f"wrote {args.output} ({image.width}x{image.height})")

    if args.slice:
        validate_slices(image.size, args.slice)
        sidecar = args.output.with_suffix(".9slice")
        suffix = " tile" if args.tile else ""
        sidecar.write_text(" ".join(str(value) for value in args.slice) + suffix + "\n",
                           encoding="ascii")
        print(f"wrote {sidecar}")
        if args.preview:
            preview = nine_slice(image, args.preview, args.slice,
                                 tiled=args.tile is not None)
            preview_path = args.output.with_name(args.output.stem + "-preview.png")
            preview.save(preview_path)
            print(f"wrote {preview_path} ({preview.width}x{preview.height})")
    elif args.preview:
        raise SystemExit("--preview requires --slice")


if __name__ == "__main__":
    main()
