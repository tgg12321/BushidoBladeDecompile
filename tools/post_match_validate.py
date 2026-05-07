#!/usr/bin/env python3
"""Detect sibling regressions after a per-function match.

The `dc.sh verify <func>` says MATCH means the bytes of THAT function are
identical to target. It does not catch the case where ANOTHER function in
the same .c file regressed because adding/removing labels in <func> shifted
GCC's file-wide `.L<N>` numbering, breaking a sibling's regfix rule that
hardcodes a label literal in its replacement payload.

Run this after a match, before commit. It:

  1. Confirms `regfix_verify <func>` says MATCH.
  2. Runs SHA1 short-circuit on the whole binary. If match, success.
  3. If (1) passes but (2) fails: walks every function defined in the same
     .c file, runs per-function verify, and lists regressed siblings with
     remediation hints.

Usage:
    python3 tools/post_match_validate.py <func>

Exit codes:
    0  — function matches AND whole binary matches
    1  — function does not match (the immediate problem)
    2  — function matches but binary does not (sibling regression)
    3  — environment error (missing build, etc.)
"""
from __future__ import annotations

import argparse
import hashlib
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def find_source_file(func: str) -> Path | None:
    """Reuse fix_label_drift's logic: find the .c that defines `func`."""
    src_dir = ROOT / "src"
    glabel_re = re.compile(rf'\bglabel\s+{re.escape(func)}\b')
    include_asm_re = re.compile(rf'INCLUDE_ASM\([^)]*"{re.escape(func)}"\s*\)')
    cdef_re = re.compile(
        rf'^(?:[A-Za-z_][\w *]*\s+)?{re.escape(func)}\s*\(',
        re.MULTILINE,
    )
    for c in sorted(src_dir.glob("*.c")):
        try:
            text = c.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if glabel_re.search(text) or include_asm_re.search(text):
            return c
        for m in cdef_re.finditer(text):
            tail = text[m.end():m.end() + 600]
            brace = tail.find("{")
            semi = tail.find(";")
            if brace >= 0 and (semi < 0 or brace < semi):
                return c
    return None


def list_funcs_in_c_file(c_path: Path) -> list[str]:
    """Return functions defined (C body, not extern) in the .c file."""
    text = c_path.read_text(encoding="utf-8", errors="replace")
    funcs: list[str] = []
    seen: set[str] = set()
    # C definition: line opens with optional storage/type, then name(args) {.
    cdef_re = re.compile(
        r'^(?:static\s+)?(?:const\s+)?(?:[\w_]+\s*\*?\s+){1,4}'
        r'(\w+)\s*\([^;]*\)\s*\{',
        re.MULTILINE,
    )
    for m in cdef_re.finditer(text):
        name = m.group(1)
        if name in seen:
            continue
        # Filter out keywords that the regex picked up (e.g., "if", "while",
        # "switch", "do", "return") — these can't legally start a definition
        # but the regex is loose.
        if name in ('if', 'while', 'switch', 'do', 'return', 'for'):
            continue
        seen.add(name)
        funcs.append(name)
    return funcs


def verify_one(func: str) -> tuple[bool, str]:
    """Run regfix_verify.py for one function. Return (ok, output)."""
    res = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "regfix_verify.py"), func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    return res.returncode == 0, res.stdout + res.stderr


def whole_binary_match() -> bool | None:
    build = ROOT / "build" / "bb2.exe"
    orig = ROOT / "disc" / "SLUS_006.63"
    if not build.exists() or not orig.exists():
        return None
    a = hashlib.sha1(build.read_bytes()).hexdigest()
    b = hashlib.sha1(orig.read_bytes()).hexdigest()
    return a == b


def collect_label_literals_in_rules(func: str) -> list[tuple[str, str]]:
    """Return [(rule_text, label_literal)] for rules of `func` whose
    replacement payload hardcodes a `.L<N>` label."""
    p = ROOT / "regfix.txt"
    if not p.exists():
        return []
    out: list[tuple[str, str]] = []
    label_re = re.compile(r'\.L\d+')
    for line in p.read_text().splitlines():
        if not line.startswith(f"{func}:"):
            continue
        # Look for substs/inserts whose replacement contains a `.L<N>`.
        # subst format: func: subst "pattern" "replacement" @ idx
        m = re.match(r'^\w+:\s*(?:subst|insert|insert_after)\s+"[^"]+"\s+"([^"]*)"', line)
        if not m:
            continue
        repl = m.group(1)
        for lbl in label_re.findall(repl):
            out.append((line, lbl))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func", help="Function just matched")
    args = ap.parse_args()

    print(f"# Step 1: per-function verify {args.func}...")
    ok, out = verify_one(args.func)
    if not ok:
        print(f"post-match-validate: FAIL — {args.func} does not match.", file=sys.stderr)
        sys.stdout.write(out)
        return 1
    print(f"# {args.func} matches.")

    print("# Step 2: whole-binary SHA1 check...")
    bin_ok = whole_binary_match()
    if bin_ok is None:
        print("post-match-validate: ERROR: build/bb2.exe or disc/SLUS_006.63 missing. "
              "Run `make` first.", file=sys.stderr)
        return 3
    if bin_ok:
        print("# bb2.exe matches target. All clear.")
        return 0

    print(f"# bb2.exe does NOT match target — checking siblings.", file=sys.stderr)

    src = find_source_file(args.func)
    if src is None:
        print(f"post-match-validate: ERROR: could not locate source file for "
              f"{args.func}", file=sys.stderr)
        return 3

    print(f"# Source file: {src.name}", file=sys.stderr)
    sib_funcs = [f for f in list_funcs_in_c_file(src) if f != args.func]
    print(f"# Checking {len(sib_funcs)} sibling function(s) in {src.name}...",
          file=sys.stderr)

    regressed: list[tuple[str, str]] = []
    for sib in sib_funcs:
        ok, out = verify_one(sib)
        if not ok:
            regressed.append((sib, out))

    if not regressed:
        print(f"post-match-validate: bb2 mismatches but no sibling in {src.name} "
              f"shows a per-function diff. Regression may be in a different .c "
              f"file. Run `dc.sh verify --all --force` to scan all functions.",
              file=sys.stderr)
        return 2

    print()
    print(f"=== {len(regressed)} regressed sibling(s) in {src.name} ===")
    for sib, sib_out in regressed:
        # Extract one-line summary from regfix_verify output
        first_line = next(
            (l for l in sib_out.splitlines() if "differ" in l or "MISMATCH" in l),
            sib_out.splitlines()[0] if sib_out.splitlines() else "(no output)",
        )
        print(f"  {sib}:  {first_line.strip()}")
        # Find regfix rules for this sibling that hardcode `.L<N>` literals
        labels = collect_label_literals_in_rules(sib)
        if labels:
            print(f"    Suspicious rules (hardcoded `.L<N>` literals):")
            for rule, lbl in labels[:3]:
                rule_short = rule[:140] + ("..." if len(rule) > 140 else "")
                print(f"      {lbl} in: {rule_short}")
            if len(labels) > 3:
                print(f"      ... and {len(labels) - 3} more")
            print(f"    Try: `dc.sh fix-label-drift --apply` "
                  f"(reactive auto-fix from linker errors)")
        print(f"    Or:  `dc.sh diff {sib}` (manual investigation)")

    print()
    print(f"post-match-validate: {len(regressed)} sibling regression(s) — "
          f"resolve before commit.", file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
