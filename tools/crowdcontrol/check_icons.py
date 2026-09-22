#!/usr/bin/env python3
"""Validates icons against include/crowd/crowd_effects.h and packages them.

- Every CROWD_EFFECT code in the header must have a matching pack/icons/<code>.png.
- Every icon must be exactly 128x128 (required by Crowd Control).

Run standalone to just validate, or with --output to also write a zip of the icons.

Usage:
    python3 tools/crowdcontrol/check_icons.py
    python3 tools/crowdcontrol/check_icons.py --output icons.zip
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
EFFECTS_HEADER = REPO_ROOT / "include" / "crowd" / "crowd_effects.h"
ICONS_DIR = REPO_ROOT / "pack" / "icons"

REQUIRED_SIZE = (128, 128)

HEADER_ENTRY_RE = re.compile(r'CROWD_EFFECT\(\s*"([^"]+)"')


def parse_effect_codes(text: str) -> list[str]:
    return sorted(set(HEADER_ENTRY_RE.findall(text)))


def png_dimensions(path: Path) -> tuple[int, int] | None:
    with path.open("rb") as f:
        header = f.read(24)
    if len(header) < 24 or header[:8] != b"\x89PNG\r\n\x1a\n" or header[12:16] != b"IHDR":
        return None
    width, height = struct.unpack(">II", header[16:24])
    return (width, height)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--output", type=Path, help="path to write icons.zip to after validation succeeds")
    args = parser.parse_args()

    codes = parse_effect_codes(EFFECTS_HEADER.read_text(encoding="utf-8"))

    missing: list[str] = []
    wrong_size: list[tuple[str, tuple[int, int] | None]] = []

    for code in codes:
        icon_path = ICONS_DIR / f"{code}.png"
        if not icon_path.is_file():
            missing.append(code)
            continue
        dims = png_dimensions(icon_path)
        if dims != REQUIRED_SIZE:
            wrong_size.append((code, dims))

    if missing:
        print(f"Missing icon(s) ({len(missing)}):", file=sys.stderr)
        for code in missing:
            print(f"  {ICONS_DIR / f'{code}.png'}", file=sys.stderr)

    if wrong_size:
        print(f"Wrong-size icon(s) ({len(wrong_size)}), expected {REQUIRED_SIZE[0]}x{REQUIRED_SIZE[1]}:", file=sys.stderr)
        for code, dims in wrong_size:
            got = "unreadable/not a PNG" if dims is None else f"{dims[0]}x{dims[1]}"
            print(f"  {code}.png: {got}", file=sys.stderr)

    if missing or wrong_size:
        print("FAILED: pack/icons has drifted from include/crowd/crowd_effects.h.", file=sys.stderr)
        return 1

    print(f"OK: {len(codes)} effect(s), all icons present and {REQUIRED_SIZE[0]}x{REQUIRED_SIZE[1]}.")

    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(args.output, "w", zipfile.ZIP_DEFLATED) as zf:
            for code in codes:
                zf.write(ICONS_DIR / f"{code}.png", arcname=f"{code}.png")
        print(f"Wrote {args.output}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
