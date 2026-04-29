#!/usr/bin/env python3
"""Check that build's symbol layout matches canonical addresses.

A silent failure mode: GCC compiles `lw v0, %gp_rel(D_800A3514)` but the
linker, given a build whose sdata ordering differs from the original,
places D_800A3514 at gp+1092 (= 0x800A3510). The build still produces a
binary, no error fires, but every load through that symbol now reads the
wrong word. This script catches that class of issue before it wastes
debugging time on individual functions.

What it checks:
  - For every D_<HEX> symbol in build/bb2.map, the actual address must
    match the address claimed by the name (D_800A3514 must be at
    0x800A3514).
  - For renamed symbols (g_str_overflow, jtbl_80010068, etc) listed in
    named_syms.txt or symbol_addrs.txt, the actual address must match
    the canonical.
  - Mismatches in the gp-rel reachable range [gp-0x8000, gp+0x7FFF] are
    reported as CRITICAL (silent miscompile risk). Mismatches outside
    are still reported but downgraded.

Usage:
    python3 tools/check_gp_layout.py
    python3 tools/check_gp_layout.py --gp-only      # just gp-rel range
    python3 tools/check_gp_layout.py --quiet        # exit code only
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MAP_FILE = ROOT / "build" / "bb2.map"
GP = 0x800A30CC
GP_REL_LO = GP - 0x8000
GP_REL_HI = GP + 0x7FFF

# Symbol address sources (read in order; later wins on conflict)
SYM_FILES = [
    ROOT / "named_syms.txt",
    ROOT / "symbol_addrs.txt",
]


def parse_canonical_syms() -> dict[str, int]:
    """Read named_syms.txt + symbol_addrs.txt for canonical addresses."""
    out: dict[str, int] = {}
    pat = re.compile(r'^\s*([A-Za-z_]\w*)\s*=\s*0[xX]([0-9A-Fa-f]+)\s*;')
    for f in SYM_FILES:
        if not f.exists():
            continue
        for line in f.read_text(encoding="utf-8", errors="replace").splitlines():
            # strip trailing comment
            line = line.split("//", 1)[0]
            m = pat.match(line)
            if m:
                out[m.group(1)] = int(m.group(2), 16)
    return out


def parse_map(map_path: Path) -> list[tuple[str, int]]:
    """Extract (symbol, actual_address) tuples from a GNU ld map file.

    Skips function symbols (func_*), keeps data symbols.
    """
    out: list[tuple[str, int]] = []
    # Allow dots in the name so `D_800A3514.NON_MATCHING` parses too.
    pat = re.compile(r'^\s+0x([0-9a-fA-F]+)\s+([A-Za-z_][\w.]*)\s*$')
    for line in map_path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = pat.match(line)
        if not m:
            continue
        name = m.group(2)
        # Skip function symbols and section anchors -- we only check data layout
        if name.startswith("func_"):
            continue
        if name in ("_gp", "_ftext", "_etext", "_edata", "_end", "_fbss"):
            continue
        out.append((name, int(m.group(1), 16)))
    return out


def canonical_for(name: str, named_syms: dict[str, int]) -> int | None:
    """Return the canonical address a name claims, or None if unknown."""
    # NON_MATCHING is a build-time placeholder for the same canonical sym
    base = name[:-len(".NON_MATCHING")] if name.endswith(".NON_MATCHING") else name

    # Direct hit in the named symbol files
    if base in named_syms:
        return named_syms[base]

    # D_<HEX> encodes its own canonical address
    m = re.match(r'^D_([0-9A-F]+)$', base)
    if m:
        return int(m.group(1), 16)

    # jtbl_<hex>, jpt_<hex>: the address is encoded in the name as well
    m = re.match(r'^(?:jtbl|jpt)_([0-9A-Fa-f]+)$', base)
    if m:
        return int(m.group(1), 16)

    return None


def in_gp_range(addr: int) -> bool:
    return GP_REL_LO <= addr <= GP_REL_HI


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--gp-only", action="store_true",
                        help="Only check symbols in the gp-rel reachable range")
    parser.add_argument("--quiet", action="store_true",
                        help="Suppress per-symbol output; exit code only")
    parser.add_argument("--map", default=str(MAP_FILE),
                        help="Path to ld map file (default: build/bb2.map)")
    args = parser.parse_args()

    map_path = Path(args.map)
    if not map_path.exists():
        print(f"ERROR: {map_path} not found. Run `make` first.", file=sys.stderr)
        return 2

    named = parse_canonical_syms()
    syms = parse_map(map_path)

    checked = 0
    skipped_unknown = 0
    critical_mismatches: list[tuple[str, int, int]] = []
    other_mismatches: list[tuple[str, int, int]] = []
    duplicates: dict[str, list[int]] = {}

    seen: dict[str, int] = {}
    for name, actual in syms:
        if args.gp_only and not in_gp_range(actual):
            continue

        # Catch the same canonical symbol bound to two different addresses --
        # NON_MATCHING split into a real def + placeholder is normal IF they
        # share an address; differing addresses signal a layout bug.
        base = name[:-len(".NON_MATCHING")] if name.endswith(".NON_MATCHING") else name
        if base in seen:
            if seen[base] != actual:
                duplicates.setdefault(base, [seen[base]]).append(actual)
        else:
            seen[base] = actual

        canonical = canonical_for(name, named)
        if canonical is None:
            skipped_unknown += 1
            continue
        checked += 1
        if canonical != actual:
            entry = (name, canonical, actual)
            if in_gp_range(canonical) or in_gp_range(actual):
                critical_mismatches.append(entry)
            else:
                other_mismatches.append(entry)

    total_issues = len(critical_mismatches) + len(other_mismatches) + len(duplicates)

    if not args.quiet:
        print(f"# gp-rel layout check  (gp = 0x{GP:08x}, "
              f"range [0x{GP_REL_LO:08x}, 0x{GP_REL_HI:08x}])")
        print(f"#   checked: {checked} symbols   "
              f"(skipped {skipped_unknown} unknown-canonical)")
        if not total_issues:
            print("# OK -- no layout drift detected.")
            return 0

        if critical_mismatches:
            print()
            print(f"## CRITICAL: {len(critical_mismatches)} gp-rel symbol(s) at wrong address")
            print("##   silently miscompiles any C using these symbols")
            print(f"#   {'symbol':30s} {'expected':>12s}  {'actual':>12s}  {'delta':>8s}  gp_off")
            for name, exp, act in critical_mismatches:
                gp_off = act - GP
                print(f"  {name:30s}  0x{exp:08x}   0x{act:08x}  "
                      f"{act - exp:+8d}  gp{gp_off:+d}")

        if other_mismatches and not args.gp_only:
            print()
            print(f"## NON-CRITICAL: {len(other_mismatches)} other symbol(s) at wrong address")
            print(f"#   {'symbol':30s} {'expected':>12s}  {'actual':>12s}  {'delta':>8s}")
            for name, exp, act in other_mismatches[:20]:
                print(f"  {name:30s}  0x{exp:08x}   0x{act:08x}  {act - exp:+8d}")
            if len(other_mismatches) > 20:
                print(f"  ... ({len(other_mismatches) - 20} more)")

        if duplicates:
            print()
            print(f"## DUPLICATE: {len(duplicates)} symbol(s) bound to multiple addresses")
            for base, addrs in list(duplicates.items())[:20]:
                addr_str = ", ".join(f"0x{a:08x}" for a in addrs)
                print(f"  {base:30s}  {addr_str}")

    # Critical mismatches and duplicates fail the check; non-critical mismatches
    # only warn (returns 0). gp-only mode treats both as failure.
    if critical_mismatches or duplicates:
        return 1
    if args.gp_only and other_mismatches:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
