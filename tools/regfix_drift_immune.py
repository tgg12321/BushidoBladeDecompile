#!/usr/bin/env python3
"""Audit regfix.txt for `subst` rules where the LITERAL `.L<N>` label
in the pattern can safely be made drift-immune (`\\.L\\d+`).

Background: regfix `subst` rules apply at a specific instruction index
(@ N). The pattern matches a single instruction at that position. If
the pattern hardcodes a label like `.L1037` and any new C source
elsewhere in the same .c file adds new `.L<N>` labels, GCC's
file-wide counter shifts. The hardcoded pattern then fails to match,
producing wrong bytes silently.

**Safety constraint:** we ONLY rewrite when the rule's REPLACEMENT
label is CUSTOM (project-controlled — e.g., `.Lbody<func>` defined
elsewhere via `insert_label`). If the replacement is itself a
GCC-emitted `.L<digits>` label, both pattern AND replacement are
drift-sensitive, and changing the pattern alone exposes a pre-existing
soft-failure (the original `.L<N>` pattern silently no-op'd when the
label drifted; our drift-immune pattern would fire and write a
no-longer-existing replacement label, breaking the build).

The replacement is considered "custom" if it:
  - Contains a non-digit prefix on the label name (e.g., `.LbodyX`,
    `.Lc_foo`, `.Lret`)
  - OR is a non-`.L` mnemonic-style identifier

Pure `.L<digits>` replacements are LEFT ALONE — those need pair-shift
handling, not unilateral pattern relaxation.

By default, runs in --dry-run mode. Pass --apply to write changes.

Examples of what gets rewritten (safe):

  before:  func_X: subst "beq\\s+\\$2,\\$0,\\.L1037" "bne   $2,$0,.LbodyX" @ 17
  after:   func_X: subst "beq\\s+\\$2,\\$0,\\.L\\d+"  "bne   $2,$0,.LbodyX" @ 17

  Replacement `.LbodyX` is custom — drift-immune pattern is safe.

Examples of what is NOT rewritten (unsafe to drift-immunize):

  func_X: subst "\\.L293" ".L294" @ 185

  Both pattern (`.L293`) AND replacement (`.L294`) are GCC-emitted
  labels. The original works only if BOTH exist — and is silently
  skipped if `.L293` drifted. Making pattern `\\.L\\d+` would force
  the rewrite even when `.L294` doesn't exist, breaking the build.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REGFIX = ROOT / "regfix.txt"

# Match a subst rule line. Captures: func, pattern, replacement, idx
SUBST_RE = re.compile(
    r'^(\w+):\s+subst\s+"((?:\\.|[^"])*)"\s+"((?:\\.|[^"])*)"\s+@\s+(\d+)\s*$'
)

# Match a literal `.L<digits>` in a regex pattern. Should preserve the
# escape (`\.L`) when rewriting, and turn the digits into `\d+`.
# Catches both `\.L1037` and `.L1037` (some older rules may have the
# latter; we treat both as targets to drift-immunize).
LITERAL_LABEL_RE = re.compile(r"\\\.L\d+|\.L\d+")


def rewrite_pattern(pat: str) -> tuple[str, int]:
    """Replace literal `.L<N>` with `\\.L\\d+` in a regex pattern.

    Returns (new_pattern, num_replacements).
    """
    count = 0

    def _sub(_m: re.Match) -> str:
        nonlocal count
        count += 1
        return r"\.L\d+"

    new_pat = LITERAL_LABEL_RE.sub(_sub, pat)
    return new_pat, count


# A replacement is "drift-sensitive" if it contains a bare `.L<digits>`
# label name. That means the replacement target is a GCC-emitted label
# that itself can drift — drift-immunizing the pattern without also
# making the replacement drift-immune (or guaranteed-stable) breaks
# the build when both drift.
DRIFT_SENSITIVE_REPL_RE = re.compile(r"\.L\d+(?!\w)")


def replacement_is_safe(repl: str) -> bool:
    """Return True if the replacement label is "custom" (project-
    controlled), False if it's a drift-sensitive GCC label.

    We treat the replacement as safe when it has NO bare `.L<digits>`
    label reference — meaning all label references are either custom
    (e.g., `.LbodyX`, `.Lret`) or absent (e.g., the replacement is a
    register/immediate, not a branch target).
    """
    return not DRIFT_SENSITIVE_REPL_RE.search(repl)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--apply", action="store_true",
                    help="Write changes to regfix.txt. Default is dry-run.")
    ap.add_argument("--verbose", action="store_true",
                    help="Show before/after for each rewritten line.")
    args = ap.parse_args()

    if not REGFIX.exists():
        print(f"ERROR: {REGFIX} not found", file=sys.stderr)
        return 1

    text = REGFIX.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=False)
    new_lines: list[str] = []
    changes: list[tuple[int, str, str]] = []
    skipped: list[tuple[int, str, str]] = []

    for lineno, line in enumerate(lines, start=1):
        m = SUBST_RE.match(line)
        if not m:
            new_lines.append(line)
            continue
        func, pat, repl, idx = m.groups()
        new_pat, n_repl = rewrite_pattern(pat)
        if n_repl == 0 or new_pat == pat:
            new_lines.append(line)
            continue
        # Safety gate: don't rewrite if the replacement contains a
        # drift-sensitive bare `.L<digits>` label. See module docstring
        # for rationale (paired-shift, not unilateral rewrite).
        if not replacement_is_safe(repl):
            new_lines.append(line)
            skipped.append((lineno, func, repl))
            continue
        new_line = f'{func}: subst "{new_pat}" "{repl}" @ {idx}'
        new_lines.append(new_line)
        changes.append((lineno, line, new_line))

    if skipped:
        print(f"Skipped {len(skipped)} rule(s) with drift-sensitive replacements:")
        for lineno, func, repl in skipped[:5]:
            print(f"  line {lineno:>5}  {func:<32}  repl points at bare `.L<N>` ({repl[:50]})")
        if len(skipped) > 5:
            print(f"  ... and {len(skipped) - 5} more")
        print(f"  (these need pair-shift handling, not unilateral pattern rewrite)")
        print()

    if not changes:
        print(f"No safe-to-rewrite literal `.L<N>` patterns found.")
        return 0

    print(f"Found {len(changes)} subst rule(s) safe to drift-immunize:")
    for lineno, before, after in changes:
        if args.verbose:
            print()
            print(f"  line {lineno}:")
            print(f"    BEFORE: {before}")
            print(f"    AFTER:  {after}")
        else:
            # Show the function name and the literal label that was rewritten,
            # for quick scanning without flooding the terminal.
            m = SUBST_RE.match(before)
            func = m.group(1) if m else "?"
            literals = LITERAL_LABEL_RE.findall(m.group(2)) if m else []
            print(f"  line {lineno:>5}  {func:<32}  rewrites: {' '.join(literals)}")

    if not args.apply:
        print()
        print(f"(dry-run) Re-run with --apply to write changes to regfix.txt.")
        return 0

    REGFIX.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
    print()
    print(f"Wrote {len(changes)} rewrite(s) to regfix.txt.")
    print(f"Recommended: rebuild + `dc.sh verify --clean` to confirm nothing broke.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
