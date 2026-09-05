#!/usr/bin/env python3
"""Diffs pack/CrashTeamRacingNativePack.cs against include/crowd/crowd_effects.h and reports drift.

Display names, categories, descriptions, and prices are pack-only and hand-authored. This script just checks effect codes and durations for validity.
Does not write the .cs file.

Usage:
    python3 tools/crowdcontrol/gen_pack.py          # report, always exits 0
    python3 tools/crowdcontrol/gen_pack.py --check  # report, exits 1 on drift (used by ctest)
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
EFFECTS_HEADER = REPO_ROOT / "include" / "crowd" / "crowd_effects.h"
PACK_FILE = REPO_ROOT / "pack" / "CrashTeamRacingNativePack.cs"

HEADER_ENTRY_RE = re.compile(
    r'CROWD_EFFECT\(\s*"([^"]+)"\s*,\s*(CROWD_EFFECT_\w+)\s*,\s*(\d+)\s*,\s*(CROWD_EFFECT_CATEGORY_\w+)\s*,\s*(\w+)\s*\)'
)

# Assumes one pack entry per line
PACK_ENTRY_RE = re.compile(
    r'new\(\s*"(?:[^"\\]|\\.)*"\s*,\s*"([^"\\]*(?:\\.[^"\\]*)*)"\s*\)\s*(\{(?P<body>[^{}]*)\})?'
)
DURATION_RE = re.compile(r"Duration\s*=\s*([0-9.]+)")


@dataclass
class HeaderEffect:
    code: str
    timed: bool
    duration_ms: int
    category: str
    handler: str


@dataclass
class PackEffect:
    code: str
    duration_s: float | None


def parse_header(text: str) -> dict[str, HeaderEffect]:
    effects: dict[str, HeaderEffect] = {}
    for m in HEADER_ENTRY_RE.finditer(text):
        code, kind, duration_ms, category, handler = m.groups()
        if category == "CROWD_EFFECT_CATEGORY_DEBUG":
            continue  # debug_test_timed is explicitly not part of the C# pack
        effects[code] = HeaderEffect(
            code=code,
            timed=(kind == "CROWD_EFFECT_TIMED"),
            duration_ms=int(duration_ms),
            category=category,
            handler=handler,
        )
    return effects


def parse_pack(text: str) -> dict[str, PackEffect]:
    effects: dict[str, PackEffect] = {}
    for m in PACK_ENTRY_RE.finditer(text):
        code = m.group(1)
        body = m.group("body") or ""
        duration_match = DURATION_RE.search(body)
        duration_s = float(duration_match.group(1)) if duration_match else None
        effects[code] = PackEffect(code=code, duration_s=duration_s)
    return effects


def humanize(handler: str) -> str:
    """CamelCase handler name -> "Camel Case" placeholder display text."""
    return re.sub(r"(?<!^)(?=[A-Z])", " ", handler)


def suggested_snippet(effect: HeaderEffect) -> str:
    name = humanize(effect.handler)
    if effect.timed:
        duration_s = effect.duration_ms // 1000
        return f'new("{name}", "{effect.code}") {{ Category = "TODO", Duration = {duration_s}, Price = 0, Description = "TODO" }},'
    return f'new("{name}", "{effect.code}") {{ Category = "TODO", Price = 0, Description = "TODO" }},'


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--check", action="store_true", help="exit non-zero if the two files have drifted (for ctest)")
    args = parser.parse_args()

    header_effects = parse_header(EFFECTS_HEADER.read_text(encoding="utf-8"))
    pack_effects = parse_pack(PACK_FILE.read_text(encoding="utf-8"))

    header_codes = set(header_effects)
    pack_codes = set(pack_effects)

    missing_from_pack = sorted(header_codes - pack_codes)
    extra_in_pack = sorted(pack_codes - header_codes)

    duration_mismatches: list[tuple[str, str]] = []
    for code in sorted(header_codes & pack_codes):
        header = header_effects[code]
        pack = pack_effects[code]
        if header.timed:
            expected_s = header.duration_ms / 1000.0
            if pack.duration_s is None:
                duration_mismatches.append((code, f"header declares {expected_s:g}s timed, pack has no Duration"))
            elif pack.duration_s != expected_s:
                duration_mismatches.append((code, f"header declares {expected_s:g}s, pack has Duration = {pack.duration_s:g}"))
        else:
            if pack.duration_s is not None:
                duration_mismatches.append((code, f"header declares this instant, pack has Duration = {pack.duration_s:g}"))

    problems = bool(missing_from_pack or extra_in_pack or duration_mismatches)

    print(f"crowd_effects.h: {len(header_effects)} effect(s)")
    print(f"{PACK_FILE.name}: {len(pack_effects)} effect(s)")
    print()

    if missing_from_pack:
        print(f"Missing from the pack ({len(missing_from_pack)}):")
        for code in missing_from_pack:
            print(f"  {code}")
            print(f"    {suggested_snippet(header_effects[code])}")
        print()

    if extra_in_pack:
        print(f"In the pack but not in crowd_effects.h ({len(extra_in_pack)}):")
        for code in extra_in_pack:
            print(f"  {code}")
        print()

    if duration_mismatches:
        print(f"Duration mismatches ({len(duration_mismatches)}):")
        for code, detail in duration_mismatches:
            print(f"  {code}: {detail}")
        print()

    if not problems:
        print("OK: every effect code, kind, and duration matches.")
        return 0

    if args.check:
        print("FAILED: pack/CrashTeamRacingNativePack.cs has drifted from include/crowd/crowd_effects.h.", file=sys.stderr)
        return 1

    print("(run with --check to make this a failing exit code, e.g. in CI)")
    return 0


if __name__ == "__main__":
    sys.exit(main())