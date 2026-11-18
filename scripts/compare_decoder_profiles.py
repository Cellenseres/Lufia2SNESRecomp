"""Compare interpreter activity from two matching replay captures."""

import argparse
import csv
from pathlib import Path


def read_profile(path):
    profile = {}
    with path.open(encoding="utf-8", newline="") as stream:
        rows = csv.reader(stream, strict=True)
        if next(rows, None) != ["pc", "mx", "count", "guest_cycles"]:
            raise ValueError(f"{path}: expected a decoded-opcodes.csv header")
        for row in rows:
            try:
                pc, mx, count, cycles = row
                key = int(pc, 16), int(mx)
                counters = int(count), int(cycles)
                if not (0 <= key[0] <= 0xFFFFFF and 0 <= key[1] <= 7):
                    raise ValueError("address or CPU mode out of range")
                if min(counters) < 0:
                    raise ValueError("negative counter")
                if key in profile:
                    raise ValueError("duplicate address and CPU mode")
            except ValueError as error:
                raise ValueError(f"{path}:{rows.line_num}: {error}") from error
            profile[key] = counters
    return profile


def print_report(before, after, limit):
    missing = (0, 0)
    changes = []
    for key in before.keys() | after.keys():
        old_count, old_cycles = before.get(key, missing)
        new_count, new_cycles = after.get(key, missing)
        if (old_count, old_cycles) != (new_count, new_cycles):
            changes.append((key, new_count - old_count, new_cycles - old_cycles))

    for column, label in enumerate(("Interpreter decodes", "Interpreter guest CPU cycles")):
        old = sum(counters[column] for counters in before.values())
        new = sum(counters[column] for counters in after.values())
        print(f"{label}: {old:,} -> {new:,} ({new - old:+,})")
    print(f"Changed address/mode rows: {len(changes):,}")

    if not limit:
        return
    for title, direction in (("Largest decreases", -1), ("Largest increases", 1)):
        ranked = sorted(
            (row for row in changes if direction * (row[1] or row[2]) > 0),
            key=lambda row: (-direction * row[1], -direction * row[2], row[0]),
        )
        if ranked:
            print(f"\n{title}")
            print(f"{'PC':>6} {'Mode':>4} {'Decodes':>12} {'Guest cycles':>14}")
            for (pc, mx), count, cycles in ranked[:limit]:
                print(f"{pc:06X} {mx:4} {count:+12,} {cycles:+14,}")

    remaining = sorted(
        ((key, count, cycles) for key, (count, cycles) in after.items() if count),
        key=lambda row: (-row[1], -row[2], row[0]),
    )
    if remaining:
        print("\nMost executed interpreter addresses in candidate")
        print(f"{'PC':>6} {'Mode':>4} {'Decodes':>12} {'Guest cycles':>14}")
        for (pc, mx), count, cycles in remaining[:limit]:
            print(f"{pc:06X} {mx:4} {count:12,} {cycles:14,}")


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog="Use the same input, starting state, and frame range for both captures.",
    )
    parser.add_argument("baseline", type=Path, help="baseline decoded-opcodes.csv")
    parser.add_argument("candidate", type=Path, help="candidate decoded-opcodes.csv")
    parser.add_argument("--limit", type=int, default=10, help="rows per table (0 for totals only)")
    args = parser.parse_args()
    if args.limit < 0:
        parser.error("--limit must be nonnegative")
    try:
        print_report(read_profile(args.baseline), read_profile(args.candidate), args.limit)
    except (OSError, UnicodeError, csv.Error, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
