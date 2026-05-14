#!/usr/bin/env python3
"""Check that permuter_compile.sh stays in sync with the Makefile build.

The permuter scores a candidate base.c by compiling it through
tools/permuter_compile.sh. If that script's cc1/maspsx flags drift from the
Makefile's, the permuter scores against DIFFERENT codegen than the real build
produces — a "match" in the permuter then may not match in the build, and a
real match may be scored as a miss.

This bit func_8007CE0C (2026-05-14): permuter_compile.sh was missing
--expand-lb / --expand-lb-funcs / --expand-dest-funcs that the Makefile passes.
Permuter runs on that function were quietly unreliable until the flags were
re-synced.

This tool extracts the maspsx flag *names*, the --aspsx-version value, and the
cc1 flag list from both files and reports any divergence. Path-valued flags
(--sdata-syms=..., etc.) legitimately differ (permuter uses absolute paths), so
only the flag *name* is compared for those.

Usage:
    python3 tools/check_permuter_parity.py

Exit code: 0 if in sync, 1 if drift detected (or on error).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MAKEFILE = ROOT / "Makefile"
PERMUTER = ROOT / "tools" / "permuter_compile.sh"

FLAG_RE = re.compile(r"--[A-Za-z0-9][A-Za-z0-9-]*(?:=\S*)?")


def flag_names(chunk: str) -> set[str]:
    """Set of --flag names (the part before '=') found in a text chunk."""
    return {tok.split("=", 1)[0] for tok in FLAG_RE.findall(chunk)}


def aspsx_version(chunk: str) -> str | None:
    m = re.search(r"--aspsx-version=(\S+)", chunk)
    return m.group(1) if m else None


def cc_flag_tokens(line: str) -> set[str]:
    """cc1 flag tokens from a flag string (e.g. '-O2 -G0 -mips1 ...')."""
    return {t for t in line.split() if t.startswith("-")}


def main() -> int:
    if not MAKEFILE.exists() or not PERMUTER.exists():
        print("ERROR: Makefile or tools/permuter_compile.sh missing.", file=sys.stderr)
        return 1

    mk = MAKEFILE.read_text(errors="ignore")
    pm = PERMUTER.read_text(errors="ignore")

    # --- Makefile side ---
    m = re.search(r"^MASPSX_FLAGS\s*:?=\s*(.+)$", mk, re.MULTILINE)
    if not m:
        print("ERROR: could not find MASPSX_FLAGS in Makefile.", file=sys.stderr)
        return 1
    mk_maspsx = m.group(1)
    m = re.search(r"^CC_FLAGS\s*:?=\s*(.+)$", mk, re.MULTILINE)
    if not m:
        print("ERROR: could not find CC_FLAGS in Makefile.", file=sys.stderr)
        return 1
    mk_cc = m.group(1)

    # --- permuter_compile.sh side ---
    # The maspsx invocation spans continuation lines between `"$MASPSX"` and
    # the `| mipsel-linux-gnu-as` pipe stage.
    start = pm.find('"$MASPSX"')
    end = pm.find("mipsel-linux-gnu-as", start) if start != -1 else -1
    if start == -1 or end == -1:
        print("ERROR: could not locate the maspsx invocation in permuter_compile.sh.",
              file=sys.stderr)
        return 1
    pm_maspsx = pm[start:end]
    m = re.search(r'^CC_FLAGS="([^"]*)"', pm, re.MULTILINE)
    if not m:
        print("ERROR: could not find CC_FLAGS in permuter_compile.sh.", file=sys.stderr)
        return 1
    pm_cc = m.group(1)

    problems: list[str] = []

    # maspsx flag-name parity
    mk_flags = flag_names(mk_maspsx)
    pm_flags = flag_names(pm_maspsx)
    missing_in_pm = mk_flags - pm_flags
    extra_in_pm = pm_flags - mk_flags
    if missing_in_pm:
        problems.append(
            "maspsx flags in Makefile but MISSING from permuter_compile.sh: "
            + " ".join(sorted(missing_in_pm))
            + "\n      => the permuter scores against different codegen than the build")
    if extra_in_pm:
        problems.append(
            "maspsx flags in permuter_compile.sh but NOT in the Makefile: "
            + " ".join(sorted(extra_in_pm)))

    # --aspsx-version value parity
    mk_ver, pm_ver = aspsx_version(mk_maspsx), aspsx_version(pm_maspsx)
    if mk_ver != pm_ver:
        problems.append(f"--aspsx-version mismatch: Makefile={mk_ver} permuter={pm_ver}")

    # cc1 flag parity
    mk_cc_set = cc_flag_tokens(mk_cc)
    pm_cc_set = cc_flag_tokens(pm_cc)
    cc_missing = mk_cc_set - pm_cc_set
    cc_extra = pm_cc_set - mk_cc_set
    if cc_missing:
        problems.append("cc1 flags in Makefile but MISSING from permuter_compile.sh: "
                        + " ".join(sorted(cc_missing)))
    if cc_extra:
        problems.append("cc1 flags in permuter_compile.sh but NOT in the Makefile: "
                        + " ".join(sorted(cc_extra)))

    if not problems:
        print("check-permuter-parity: OK — permuter_compile.sh matches the Makefile")
        print(f"  maspsx flags ({len(mk_flags)}): {' '.join(sorted(mk_flags))}")
        print(f"  --aspsx-version: {mk_ver}")
        return 0

    print("check-permuter-parity: DRIFT DETECTED")
    print()
    for p in problems:
        print(f"  - {p}")
    print()
    print("  Fix tools/permuter_compile.sh so its cc1 + maspsx flags match the")
    print("  Makefile's CC_FLAGS / MASPSX_FLAGS (path values may differ; flag")
    print("  names and --aspsx-version must not).")
    return 1


if __name__ == "__main__":
    sys.exit(main())
