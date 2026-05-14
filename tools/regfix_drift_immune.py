#!/usr/bin/env python3
"""Audit regfix.txt for `subst` rules with hardcoded `.L<N>` labels and
rewrite them drift-immune where it is provably safe.

Background: regfix `subst` rules apply at a specific instruction index
(@ N). The pattern matches a single instruction at that position. If the
pattern hardcodes a label like `.L1037` and any new C source elsewhere in
the same .c file adds new `.L<N>` labels, GCC's file-wide label counter
shifts. The hardcoded pattern then fails to match, producing wrong bytes
silently — no build error, no linker error. (func_8007CE0C, 2026-05-14: 12
drift-broken rules in two sibling functions cost a full session to find.)

Two rewrite strategies, tried in this order per rule:

1. **Mnemonic-only swap** (best — fully drift-immune). When the pattern and
   the replacement are the SAME instruction except for the leading mnemonic,
   and any label is byte-identical on both sides:

     before:  func_X: subst "j\\s+\\.L289"  "b\\t.L289"  @ 19
     after:   func_X: subst "\\bj\\b"        "b"          @ 19

   The rewrite drops the label entirely — it touches only the mnemonic, so
   NO label reference survives in pattern OR replacement. This is the
   recurring j<->b / beq<->bne case (sign-splits, GTE wrappers). Provably
   safe: the @ idx pin is function-local and stable; only label *numbers*
   drift, never instruction indices.

2. **Pattern relaxation** (`.L<N>` -> `\\.L\\d+` in the pattern only).
   Applied only when the REPLACEMENT label is CUSTOM (project-controlled —
   e.g. `.LbodyX` defined via `insert_label`). If the replacement is itself
   a GCC-emitted `.L<digits>` label and it is NOT a mnemonic-only swap,
   both pattern AND replacement are drift-sensitive; relaxing the pattern
   alone would force a rewrite with a stale replacement label. Those are
   left alone and reported as needing pair-shift handling.

By default, runs in dry-run mode. Pass --apply to write changes.
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

# Match a literal `.L<digits>` in a regex pattern. Catches both `\.L1037`
# and `.L1037` (some older rules may have the latter).
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
# label. Drift-immunizing the pattern without also making the replacement
# stable breaks the build when both drift.
DRIFT_SENSITIVE_REPL_RE = re.compile(r"\.L\d+(?!\w)")


def replacement_is_safe(repl: str) -> bool:
    """True if the replacement label is custom (project-controlled), False
    if it's a drift-sensitive GCC `.L<digits>` label."""
    return not DRIFT_SENSITIVE_REPL_RE.search(repl)


def de_regex(pat: str) -> str | None:
    """Flatten a SIMPLE regfix pattern to the literal string it matches, or
    None if it uses regex constructs we can't safely flatten.

    Only handles the constructs that appear in branch-instruction patterns
    (`\\s+`, `\\s`, `\\t`, `\\n`, `\\.`, `\\$`, `\\,`). Anything with groups,
    classes, alternation, or leftover metacharacters/escapes bails — we never
    want to mis-flatten and emit a wrong rewrite.

    Note `\\t` / `\\n` in a regex pattern match a literal tab / newline, so
    they are flattened to whitespace just like `\\s+` (some regfix rules use
    `b\\t.L355` instead of `b\\s+.L355` for the inter-operand gap).
    """
    # Regex constructs that cannot appear in a literal instruction pattern.
    if any(c in pat for c in "()[]{}|*?^"):
        return None
    s = (pat.replace(r"\s+", " ").replace(r"\s*", " ").replace(r"\s", " ")
            .replace(r"\t", " ").replace(r"\n", " ")
            .replace(r"\.", ".").replace(r"\$", "$").replace(r"\,", ","))
    # A leftover backslash = an escape we don't handle; a leftover '+' = a
    # quantifier we didn't consume (the only expected '+' was inside `\s+`).
    # Note '$' is NOT checked here: after de-escaping `\$` it is a literal
    # register prefix ($2, $0), not a regex anchor.
    if "\\" in s or "+" in s:
        return None
    return s


def try_mnemonic_only_swap(pat: str, repl: str) -> tuple[str, str] | None:
    """If pat/repl are the same instruction modulo the leading mnemonic (and
    any operand, including a label, is byte-identical on both sides), return
    (new_pattern, new_replacement) for a fully drift-immune mnemonic-only
    rewrite. Otherwise return None.

      pat="j\\s+\\.L289"  repl="b\\t.L289"  ->  ("\\bj\\b", "b")
    """
    pat_lit = de_regex(pat)
    if pat_lit is None:
        return None
    # Normalize the replacement the way regfix.py loads it (\t -> tab etc.).
    repl_norm = repl.replace("\\t", "\t").replace("\\n", "\n")
    pm = re.match(r"^(\S+)\s+(.+)$", pat_lit)
    rm = re.match(r"^(\S+)\s+(.+)$", repl_norm)
    if not pm or not rm:
        return None
    m1, pat_rest = pm.group(1), pm.group(2)
    m2, repl_rest = rm.group(1), rm.group(2)
    # Mnemonics must be plain words so `\b<m>\b` is meaningful.
    if not re.fullmatch(r"\w+", m1) or not re.fullmatch(r"\w+", m2):
        return None
    if m1 == m2:
        return None  # not a swap
    if pat_rest.strip() != repl_rest.strip():
        return None  # operands differ -> not a pure mnemonic swap
    return (r"\b" + m1 + r"\b", m2)


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
    # (lineno, before, after, kind)
    changes: list[tuple[int, str, str, str]] = []
    skipped: list[tuple[int, str, str]] = []

    for lineno, line in enumerate(lines, start=1):
        m = SUBST_RE.match(line)
        if not m:
            new_lines.append(line)
            continue
        func, pat, repl, idx = m.groups()

        # Only rules with a literal `.L<N>` in the pattern can drift.
        if not LITERAL_LABEL_RE.search(pat):
            new_lines.append(line)
            continue

        # Strategy 1: mnemonic-only swap -> fully drift-immune rewrite.
        swap = try_mnemonic_only_swap(pat, repl)
        if swap is not None:
            new_pat, new_repl = swap
            new_line = f'{func}: subst "{new_pat}" "{new_repl}" @ {idx}'
            if new_line != line:
                new_lines.append(new_line)
                changes.append((lineno, line, new_line, "mnemonic-swap"))
                continue

        # Strategy 2: relax the literal label in the PATTERN to `\.L\d+`,
        # but only when the REPLACEMENT label is custom.
        new_pat, n_repl = rewrite_pattern(pat)
        if n_repl == 0 or new_pat == pat:
            new_lines.append(line)
            continue
        if not replacement_is_safe(repl):
            new_lines.append(line)
            skipped.append((lineno, func, repl))
            continue
        new_line = f'{func}: subst "{new_pat}" "{repl}" @ {idx}'
        new_lines.append(new_line)
        changes.append((lineno, line, new_line, "pattern-relax"))

    if skipped:
        print(f"Skipped {len(skipped)} rule(s) with drift-sensitive replacements "
              f"that are NOT mnemonic-only swaps:")
        for lineno, func, repl in skipped[:8]:
            print(f"  line {lineno:>5}  {func:<32}  repl points at bare `.L<N>` ({repl[:48]})")
        if len(skipped) > 8:
            print(f"  ... and {len(skipped) - 8} more")
        print(f"  (these need pair-shift handling — fix-label-drift / fix-asmfix-drift —")
        print(f"   not a unilateral pattern rewrite)")
        print()

    if not changes:
        print("No drift-fragile literal-`.L<N>` subst rules found that are safe to rewrite.")
        if skipped:
            return 1  # there ARE drift-fragile rules, just not auto-fixable
        return 0

    n_swap = sum(1 for *_, kind in changes if kind == "mnemonic-swap")
    n_relax = sum(1 for *_, kind in changes if kind == "pattern-relax")
    print(f"Found {len(changes)} subst rule(s) safe to drift-immunize "
          f"({n_swap} mnemonic-swap, {n_relax} pattern-relax):")
    for lineno, before, after, kind in changes:
        if args.verbose:
            print()
            print(f"  line {lineno} [{kind}]:")
            print(f"    BEFORE: {before}")
            print(f"    AFTER:  {after}")
        else:
            mm = SUBST_RE.match(before)
            func = mm.group(1) if mm else "?"
            literals = LITERAL_LABEL_RE.findall(mm.group(2)) if mm else []
            print(f"  line {lineno:>5}  {func:<32}  [{kind}]  {' '.join(literals)}")

    if not args.apply:
        print()
        print("(dry-run) Re-run with --apply to write changes to regfix.txt.")
        return 1

    REGFIX.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
    print()
    print(f"Wrote {len(changes)} rewrite(s) to regfix.txt.")
    print("Recommended: rebuild + `dc.sh verify --clean` to confirm nothing broke.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
