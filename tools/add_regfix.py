#!/usr/bin/env python3
"""Append a validated regfix rule to regfix.txt or regfix_stage2.txt.

Replaces the ~109 ad-hoc tmp/add_regfix*.py scripts. Every regfix addition
should go through this so format is consistent and indices are validated
against live pipeline output before commit.

Subcommands (one rule per invocation):

    swap <func> <regA> <regB> [--range N-M]
    subst <func> <pattern> <replacement> --idx N
    delete <func> --idx N
    insert <func> <asm_text> --idx N
    insert_after <func> <asm_text> --idx N
    reorder <func> <indices_csv> --range N-M
    fill_delay <func> --jal-idx N --src-idx M
    drain_delay <func> --jal-idx N

Common flags (any subcommand):
    --stage2          Append to regfix_stage2.txt instead of regfix.txt
    --comment "..."   Add a leading `# <comment>` line above the rule
    --dry-run         Print the line that would be appended; do not modify
    --no-validate     Skip validate_regfix --live --func F (faster, lower safety)
    --raw "..."       Bypass parsing and append the literal line verbatim
                       (still passes through validate_regfix unless --no-validate)

Atomicity: rule is appended, then validate_regfix --live --func <func> runs.
If validation fails, the appended line is rolled back automatically.

Exit codes: 0 = appended + validated; 1 = parse/format error; 2 = validation
failed (rolled back); 3 = unrelated I/O error.
"""
from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

REG_RE = re.compile(r"^\$(?:\d+|zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|t[89]|k[01]|gp|sp|fp|ra)$")
RANGE_RE = re.compile(r"^(\d+)\s*-\s*(\d+)$")


def parse_range(s: str) -> tuple[int, int]:
    m = RANGE_RE.match(s)
    if not m:
        raise ValueError(f"bad range '{s}', expected N-M")
    a, b = int(m.group(1)), int(m.group(2))
    if a > b:
        raise ValueError(f"inverted range {a}-{b}")
    return a, b


def regfix_path(stage2: bool) -> Path:
    return ROOT / ("regfix_stage2.txt" if stage2 else "regfix.txt")


def build_rule(args) -> str:
    """Build the regfix line (without leading 'func: ' prefix)."""
    op = args.op
    f = args.func

    if op == "swap":
        if not REG_RE.match(args.reg_a) or not REG_RE.match(args.reg_b):
            raise ValueError(f"reg names must look like $N or $name; got {args.reg_a} / {args.reg_b}")
        body = f"{args.reg_a} <-> {args.reg_b}"
        if args.range:
            a, b = parse_range(args.range)
            body += f" @ {a}-{b}"
        return f"{f}: {body}"

    if op == "subst":
        # Defensive escaping — leave the user's pattern verbatim, but warn if
        # they used a bare $ that isn't escaped or trailing.
        return f'{f}: subst "{args.pattern}" "{args.replacement}" @ {args.idx}'

    if op == "delete":
        return f"{f}: delete @ {args.idx}"

    if op == "insert":
        return f'{f}: insert "{args.asm_text}" @ {args.idx}'

    if op == "insert_after":
        return f'{f}: insert_after "{args.asm_text}" @ {args.idx}'

    if op == "reorder":
        order = [s.strip() for s in args.indices_csv.split(",") if s.strip()]
        if not order or not all(s.isdigit() for s in order):
            raise ValueError("indices must be a comma-separated list of integers")
        a, b = parse_range(args.range)
        if sorted(int(s) for s in order) != list(range(a, b + 1)):
            raise ValueError(
                f"reorder indices {order} are not a permutation of range {a}-{b}"
            )
        return f"{f}: reorder {','.join(order)} @ {a}-{b}"

    if op == "fill_delay":
        return f"{f}: fill_delay @ {args.jal_idx} <- {args.src_idx}"

    if op == "drain_delay":
        return f"{f}: drain_delay @ {args.jal_idx}"

    raise ValueError(f"unknown op {op}")


def validate_live(func: str) -> tuple[bool, str]:
    """Run validate_regfix --live --func <func>. Return (ok, output)."""
    result = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "validate_regfix.py"),
         "--live", "--func", func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    return result.returncode == 0, (result.stdout + result.stderr).strip()


def append_with_rollback(target: Path, lines: list[str], func: str,
                         skip_validate: bool) -> int:
    """Append `lines` to target and run --live validation. Roll back on failure.

    Returns the exit code: 0 = ok, 2 = validation failed."""
    original = target.read_bytes() if target.exists() else b""
    text = original.decode("utf-8") if original else ""
    if text and not text.endswith("\n"):
        text += "\n"
    addition = "\n".join(lines) + "\n"
    text += addition
    target.write_bytes(text.encode("utf-8"))

    if skip_validate:
        return 0

    ok, out = validate_live(func)
    if ok:
        return 0
    # Roll back
    target.write_bytes(original)
    print("VALIDATION FAILED — rolled back. validate_regfix output:", file=sys.stderr)
    print(out, file=sys.stderr)
    return 2


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--stage2", action="store_true",
                    help="Append to regfix_stage2.txt instead of regfix.txt")
    ap.add_argument("--comment", default=None,
                    help="Add a `# <comment>` line above the rule")
    ap.add_argument("--dry-run", action="store_true",
                    help="Print line(s) that would be appended; do not modify")
    ap.add_argument("--no-validate", action="store_true",
                    help="Skip live pipeline validation after append")
    ap.add_argument("--raw", default=None,
                    help="Append the literal line verbatim instead of parsing args")

    sub = ap.add_subparsers(dest="op")

    p = sub.add_parser("swap")
    p.add_argument("func"); p.add_argument("reg_a"); p.add_argument("reg_b")
    p.add_argument("--range", default=None,
                   help="N-M (1-based inclusive). Omit for full-function swap.")

    p = sub.add_parser("subst")
    p.add_argument("func"); p.add_argument("pattern"); p.add_argument("replacement")
    p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("delete")
    p.add_argument("func"); p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("insert")
    p.add_argument("func"); p.add_argument("asm_text")
    p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("insert_after")
    p.add_argument("func"); p.add_argument("asm_text")
    p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("reorder")
    p.add_argument("func"); p.add_argument("indices_csv")
    p.add_argument("--range", required=True, help="N-M (1-based inclusive)")

    p = sub.add_parser("fill_delay")
    p.add_argument("func")
    p.add_argument("--jal-idx", dest="jal_idx", type=int, required=True)
    p.add_argument("--src-idx", dest="src_idx", type=int, required=True)

    p = sub.add_parser("drain_delay")
    p.add_argument("func")
    p.add_argument("--jal-idx", dest="jal_idx", type=int, required=True)

    args = ap.parse_args()

    # --raw mode: bypass parsing
    if args.raw is not None:
        line = args.raw.strip()
        m = re.match(r"^(\w+)\s*:", line)
        if not m:
            print("ERROR: --raw line must start with `<func>:`", file=sys.stderr)
            return 1
        func = m.group(1)
        lines = []
        if args.comment:
            lines.append(f"# {args.comment}")
        lines.append(line)
        target = regfix_path(args.stage2)
        if args.dry_run:
            print(f"(dry-run) would append to {target.name}:")
            for l in lines:
                print(f"  {l}")
            return 0
        rc = append_with_rollback(target, lines, func, args.no_validate)
        if rc == 0:
            print(f"Appended to {target.name}: {line}")
        return rc

    if args.op is None:
        ap.print_help()
        return 1

    try:
        rule = build_rule(args)
    except ValueError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    lines = []
    if args.comment:
        lines.append(f"# {args.comment}")
    lines.append(rule)

    target = regfix_path(args.stage2)
    if args.dry_run:
        print(f"(dry-run) would append to {target.name}:")
        for l in lines:
            print(f"  {l}")
        return 0

    rc = append_with_rollback(target, lines, args.func, args.no_validate)
    if rc == 0:
        print(f"Appended to {target.name}: {rule}")
    return rc


if __name__ == "__main__":
    sys.exit(main())
