#!/usr/bin/env python3
"""One-line drift-surface summary for the session-start briefing.

Counts asmfix.txt + regfix.txt rules that use literal GCC auto-numbered
`.L<N>` labels — these are the rules that silently break when GCC's
file-wide label counter shifts (the cascade-drift class that
auto_drift_repair.py exists to fix).

Output: a single line suitable for the `dc.sh start` Cheats:/Queue:
column, plus a hint if the surface looks elevated. Exit code always 0.

This is a TREND-watching tool: the auto-repair flow handles cascade
when it fires, but tracking the underlying count helps decide if the
architectural Phase 3 fix (`rename @ <idx>` syntax) is worth building.

Usage:
    python3 tools/drift_surface_summary.py
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Address-derived labels (`.L80056FB4`): stable across GCC renumber.
_RE_ADDR_LABEL = re.compile(r"\.L[0-9A-Fa-f]{8}\b")
# GCC auto-numbered labels (`.L320`): drift-fragile.
_RE_AUTO_LABEL = re.compile(r"\.L\d{1,5}\b")


def count_fragile_asmfix() -> tuple[int, int]:
    """Return (n_fragile, n_total) for asmfix.txt rules.

    Fragile = `rename ".L<auto>" ...` rules (the source pattern pins on
    GCC's file-wide counter, which is exactly what shifts under cascade).
    delete_between/insert_before rules that anchor on `.L<addr>` are
    stable as long as the rename targets resolve (which auto-repair
    handles), so they're not counted here.
    """
    p = ROOT / "asmfix.txt"
    if not p.exists():
        return 0, 0
    n_fragile = 0
    n_total = 0
    for line in p.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        # All non-comment rule lines count toward total.
        if ":" in s:
            n_total += 1
        # Match `<func>: rename "<source>" "..."` where source is auto-numbered.
        m = re.match(r'^[\w]+:\s+rename\s+"([^"]+)"', s)
        if m and _RE_AUTO_LABEL.search(m.group(1)) and not _RE_ADDR_LABEL.search(m.group(1)):
            n_fragile += 1
    return n_fragile, n_total


def count_fragile_regfix() -> tuple[int, int]:
    """Return (n_fragile, n_total) for regfix.txt rules.

    Fragile = subst rules whose pattern OR replacement contains a literal
    GCC auto-numbered `.L<N>` (not the address-derived `.L<8hex>` form
    and not the drift-immune `\\.L\\d+` regex form).
    """
    p = ROOT / "regfix.txt"
    if not p.exists():
        return 0, 0
    n_fragile = 0
    n_total = 0
    for line in p.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        if ":" not in s:
            continue
        n_total += 1
        # Strip address-derived labels so they don't masquerade as auto.
        stripped = _RE_ADDR_LABEL.sub("", s)
        if _RE_AUTO_LABEL.search(stripped):
            n_fragile += 1
    return n_fragile, n_total


def main() -> int:
    af_fragile, af_total = count_fragile_asmfix()
    rf_fragile, rf_total = count_fragile_regfix()
    total_fragile = af_fragile + rf_fragile

    if total_fragile == 0:
        print("Drift:    0 fragile literal-`.LN` rules (asmfix + regfix)")
    else:
        print(f"Drift:    {total_fragile} fragile rules "
              f"({af_fragile} asmfix renames + {rf_fragile} regfix substs); "
              f"`dc.sh build-active` auto-repairs cascade")
    return 0


if __name__ == "__main__":
    sys.exit(main())
