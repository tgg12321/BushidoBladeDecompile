#!/usr/bin/env python3
"""Repair drifted `.L<N>` references in regfix.txt and regfix_stage2.txt.

GCC 2.7.2 numbers `.L<N>` labels per-function during compile; whenever a C
function in a translation unit changes shape (or a new function is added),
labels in *every following function in that file* shift. Existing regfix
rules that reference specific `.L<N>` numbers via `subst` then stop matching.

This tool reads the live `validate_regfix.py --live` output, finds every
PATTERN MISMATCH where the pattern references a `.L<N>` and the actual
instruction has a different `.L<M>`, and rewrites the rule's pattern (and
replacement, when the replacement also references a "real" label) with the
new numbers.

A "real" label is one < 900. Synthetic .L9xx labels (the fix-target convention
used by manual cleanups) are left alone -- they're chosen to avoid collisions
and never drift.

Usage:
    python3 tools/fix_label_drift.py                    # apply fixes
    python3 tools/fix_label_drift.py --dry-run          # preview only
    python3 tools/fix_label_drift.py --validate-after   # re-run validate at end

The tool is idempotent: re-running it after a successful fix produces no
changes.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

PATTERN_MISMATCH_RE = re.compile(
    r"Line\s+(\d+):\s+PATTERN MISMATCH at index\s+(\d+)\s*\n"
    r"\s*Pattern:\s+(.+?)\s*\n"
    r"\s*Instruction:\s+(.+?)\s*\n"
    r"\s*(\w+):\s+(subst\s+\"[^\"]+\"\s+\"[^\"]*\"\s+@\s+\d+)",
    re.MULTILINE,
)

LABEL_REF_RE = re.compile(r"\\\.L(\d+)|\.L(\d+)")


def run_validate() -> str:
    """Run validate_regfix.py --live and return combined stdout/stderr."""
    r = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "validate_regfix.py"), "--live"],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    return r.stdout + "\n" + r.stderr


def extract_drifts(validate_text: str) -> list[dict]:
    """Parse validate output into a list of drift records.

    Each record: {line, index, func, old_label, new_label,
                  pattern, replacement, raw_rule}"""
    drifts = []
    # The validator's PATTERN MISMATCH block format:
    #   Line N: PATTERN MISMATCH at index K
    #     Pattern:     <regex>
    #     Instruction: <text>
    #     <func>: <full rule>
    blocks = re.split(r"\n(?=  Line \d+:)", validate_text)
    for blk in blocks:
        m = re.search(
            r"^\s*Line\s+(\d+):\s+PATTERN MISMATCH at index\s+(\d+)",
            blk, re.MULTILINE)
        if not m:
            continue
        line_no = int(m.group(1))
        index = int(m.group(2))

        m_pat = re.search(r"Pattern:\s+(.+)", blk)
        m_ins = re.search(r"Instruction:\s+(.+)", blk)
        m_rule = re.search(r"^\s*(\w+):\s+(subst\b.+)$", blk, re.MULTILINE)
        if not (m_pat and m_ins and m_rule):
            continue

        pattern_re = m_pat.group(1).strip()
        instruction = m_ins.group(1).strip()
        func = m_rule.group(1).strip()
        rule_body = m_rule.group(2).strip()

        # Find a `.L<N>` in the pattern; find the corresponding `.L<M>` in
        # the actual instruction. Only handle the case where exactly one
        # label appears in each (the common drift signature).
        pat_labels = re.findall(r"\\?\.L(\d+)", pattern_re)
        ins_labels = re.findall(r"\.L(\d+)", instruction)
        if len(pat_labels) != 1 or len(ins_labels) != 1:
            continue
        old_label = int(pat_labels[0])
        new_label = int(ins_labels[0])
        if old_label == new_label:
            continue

        drifts.append({
            "line": line_no, "index": index, "func": func,
            "old_label": old_label, "new_label": new_label,
            "pattern_re": pattern_re,
            "instruction": instruction,
            "raw_rule_body": rule_body,
        })
    return drifts


def is_synthetic_label(n: int) -> bool:
    """Manual fix-target labels ≥ 900 are intentionally chosen to avoid
    collision and don't drift."""
    return n >= 900


def fix_rule_line(line: str, old_label: int, new_label: int) -> str:
    """Apply a uniform drift delta = (new_label - old_label) to all `.L<N>`
    references in a `subst` rule, on both pattern and replacement sides.
    Synthetic .L9xx labels (N >= 900) are never drifted -- they're hand-
    chosen targets for renames.

    Why uniform? Because GCC numbers `.L<N>` labels per-function in source
    order; when a function's body changes, all of its labels shift by the
    same delta. If our anchor label drifted by D, every other anchor in the
    same rule also drifted by D."""
    delta = new_label - old_label
    if delta == 0:
        return line

    m = re.match(
        r'^(\s*\w+\s*:\s*subst\s+)"((?:[^"\\]|\\.)*)"(\s+)"((?:[^"\\]|\\.)*)"(\s*@\s*\d+\s*)$',
        line.rstrip("\n"))
    if not m:
        return line

    head, pat, sep, repl, tail = m.groups()

    def shift_all(text: str) -> str:
        # Drift every `.L<N>` (escaped or plain) by delta, except synthetics.
        def repl_fn(mo):
            n = int(mo.group("n"))
            if is_synthetic_label(n):
                return mo.group(0)
            shifted = n + delta
            prefix = mo.group("prefix") or ""
            return f"{prefix}.L{shifted}"
        return re.sub(
            r"(?P<prefix>\\)?\.L(?P<n>\d+)\b", repl_fn, text)

    new_pat = shift_all(pat)
    new_repl = shift_all(repl)

    new_line = f'{head}"{new_pat}"{sep}"{new_repl}"{tail}\n'
    return new_line


def apply_fixes(drifts: list[dict], dry_run: bool) -> dict[str, int]:
    """Apply drift fixes to regfix.txt (and regfix_stage2.txt if needed).
    Returns a per-file change count."""
    counts = defaultdict(int)

    # Group drifts by file. For now everything goes in regfix.txt; we look
    # at line numbers reported by the validator -- regfix_stage2.txt has
    # its own validator pass, but in current state every error is from
    # regfix.txt. Detect by line-number overlap.
    target = ROOT / "regfix.txt"
    if not target.exists():
        return counts

    text = target.read_text(encoding="utf-8")
    lines = text.split("\n")

    # The validator's "Line N" is 1-based, so list index = N-1.
    # We collect per-line edits to avoid clobbering when multiple drifts
    # land on the same line (they shouldn't in practice).
    edits_by_line: dict[int, list[tuple[int, int]]] = defaultdict(list)
    for d in drifts:
        if d["line"] < 1 or d["line"] > len(lines):
            print(f"  WARN: line {d['line']} out of range "
                  f"(file has {len(lines)} lines)", file=sys.stderr)
            continue
        edits_by_line[d["line"]].append((d["old_label"], d["new_label"]))

    changed = 0
    preview_chunks: list[str] = []
    for line_no, edits in sorted(edits_by_line.items()):
        old_line = lines[line_no - 1]
        new_line = old_line + "\n"
        for old, new in edits:
            new_line = fix_rule_line(new_line, old, new)
        new_line = new_line.rstrip("\n")
        if new_line != old_line:
            preview_chunks.append(
                f"  L{line_no}: {old_line.strip()}\n"
                f"      -> {new_line.strip()}"
            )
            lines[line_no - 1] = new_line
            changed += 1

    if dry_run:
        print(f"\n(dry-run) would update {changed} line(s) in regfix.txt:\n")
        print("\n".join(preview_chunks))
        return counts

    if changed:
        target.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    counts[str(target.relative_to(ROOT))] = changed
    return counts


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--dry-run", action="store_true",
                    help="Print the proposed fixes without modifying files")
    ap.add_argument("--validate-after", action="store_true",
                    help="Re-run validate_regfix --live after applying fixes")
    ap.add_argument("--input", default=None,
                    help="Read validate output from a file instead of running it")
    args = ap.parse_args()

    if args.input:
        validate_text = Path(args.input).read_text(encoding="utf-8")
    else:
        print("Running validate_regfix --live...", file=sys.stderr)
        validate_text = run_validate()

    drifts = extract_drifts(validate_text)
    if not drifts:
        print("No label-drift PATTERN MISMATCH errors found. validate output:")
        # Show just the summary line
        for line in validate_text.splitlines():
            if "live error" in line.lower() or "All static" in line or "FOUND" in line:
                print(f"  {line.strip()}")
        return 0

    print(f"Found {len(drifts)} drifted label reference(s):")
    by_func = defaultdict(int)
    for d in drifts:
        by_func[d["func"]] += 1
    for func, n in sorted(by_func.items(), key=lambda kv: -kv[1]):
        print(f"  {func:50s} {n:>3d} drifted")

    counts = apply_fixes(drifts, args.dry_run)
    if not args.dry_run:
        for path, n in counts.items():
            print(f"\nUpdated {path}: {n} line(s)")
        if args.validate_after:
            print("\nRe-running validate...")
            r = subprocess.run(
                [sys.executable, str(ROOT / "tools" / "validate_regfix.py"), "--live"],
                cwd=str(ROOT),
            )
            return r.returncode
    return 0


if __name__ == "__main__":
    sys.exit(main())
