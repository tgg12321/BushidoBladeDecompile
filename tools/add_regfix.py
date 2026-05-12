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
sys.path.insert(0, str(ROOT / "tools"))
from active_func_scope import enforce_scope

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
        # Warn when this rule is paired with deletes: `insert @ N` uses
        # the CURRENT (post-prior-rule) index, which after deletes is
        # often NOT what the caller meant. The 8b1f6bf retro on
        # func_80078F60 surfaced this trap — they tried `insert @ 3`,
        # hit the renumbering issue, and had to switch to
        # `insert_after @ 0`. Print a hint here; don't block.
        sys.stderr.write(
            "HINT: `insert @ N` uses the index AFTER prior deletes have\n"
            "      already shifted things. If you have any `delete` rules\n"
            "      earlier in the recipe, prefer `insert_after @ M` with\n"
            "      the ORIGINAL pre-delete index of the anchor. See\n"
            "      memory/feedback_retirement_recipes.md gotcha #4.\n"
        )
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


_INSN_CACHE: dict[str, dict[int, str] | None] = {}


def _get_pipeline_instructions(func: str) -> dict[int, str] | None:
    """Run dump_text_indices for func; return {idx: text}. Cached per process.

    Returns None if the dump fails (silent — caller decides what to do)."""
    if func in _INSN_CACHE:
        return _INSN_CACHE[func]
    result = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "dump_text_indices.py"), func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    if result.returncode != 0 or not result.stdout.strip():
        _INSN_CACHE[func] = None
        return None
    insn_map: dict[int, str] = {}
    for line in result.stdout.splitlines():
        m = re.match(r"\s*(\d+):\s*(.*)", line)
        if m:
            insn_map[int(m.group(1))] = m.group(2).strip()
    _INSN_CACHE[func] = insn_map if insn_map else None
    return _INSN_CACHE[func]


def _show_context(insn_map: dict[int, str], idx: int, radius: int = 2) -> str:
    """Format insn_map[idx-r .. idx+r] as a multi-line string with arrow at idx."""
    out_lines = []
    for i in range(idx - radius, idx + radius + 1):
        if i in insn_map:
            arrow = " ->" if i == idx else "   "
            out_lines.append(f"  {arrow} {i:>4}: {insn_map[i]}")
    return "\n".join(out_lines)


def pre_validate_rule(args) -> tuple[bool, str]:
    """Cheap static check against the pre-regfix maspsx output for `args.func`.

    Catches:
      * Idx out of bounds (before a build round-trip).
      * Subst pattern that doesn't match the line at the indexed position
        (the silent-no-op trap — see memory/feedback_regfix_reference.md).

    Returns (ok, message). On ok=True, message is informational ("OK: idx N
    is `<text>`"); on ok=False, message explains what went wrong with
    surrounding context.

    Skipped (cannot be cheaply pre-validated without simulating the full
    phase pipeline):
      * `insert` / `insert_after` idx: indices shift after deletes in the
        same rule-set; live validation will catch real breakages.
      * `swap`, `reorder`, `fill_delay`, `drain_delay`: index semantics
        are too phase-dependent for a static check.

    Returns (ok=True, "skipped") for ops we don't know how to pre-validate.
    Pre-validation is best-effort; the live build is still the source of truth."""
    op = args.op
    func = args.func
    insn_map = _get_pipeline_instructions(func)
    if insn_map is None:
        return True, f"  pre-validate: skipped (no pipeline output for {func})"
    max_idx = max(insn_map.keys())

    if op == "subst":
        idx = args.idx
        if idx not in insn_map:
            return False, (
                f"  pre-validate FAILED: idx {idx} out of bounds (max={max_idx}).\n"
                f"  Nearby:\n{_show_context(insn_map, idx)}"
            )
        line = insn_map[idx]
        try:
            if not re.search(args.pattern, line):
                return False, (
                    f"  pre-validate FAILED: subst pattern {args.pattern!r} does not\n"
                    f"  match the line at idx {idx}:\n"
                    f"      {line}\n"
                    f"  Common gotcha: maspsx writes `$zero` not `$0`. See\n"
                    f"  memory/feedback_regfix_reference.md."
                )
        except re.error as e:
            return False, f"  pre-validate FAILED: invalid regex {args.pattern!r}: {e}"
        return True, f"  pre-validate OK: idx {idx} = {line!r}"

    if op == "delete":
        idx = args.idx
        if idx not in insn_map:
            ctx = _show_context(insn_map, min(idx, max_idx))
            return False, (
                f"  pre-validate FAILED: delete @ {idx} out of bounds (max={max_idx}).\n"
                f"  Last few instructions:\n{ctx}"
            )
        return True, f"  pre-validate OK: deleting idx {idx} = {insn_map[idx]!r}"

    if op in ("insert", "insert_after"):
        idx = args.idx
        if idx > max_idx:
            return False, (
                f"  pre-validate FAILED: {op} @ {idx} out of bounds (max={max_idx}).\n"
                f"  Nearby:\n{_show_context(insn_map, max(0, max_idx - 2))}"
            )
        if idx in insn_map:
            return True, f"  pre-validate OK: {op} relative to idx {idx} = {insn_map[idx]!r}"
        return True, f"  pre-validate: idx {idx} bounds OK (semantics vary by phase order; live check authoritative)"

    return True, "  pre-validate: skipped (op not pre-validatable)"


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
    ap.add_argument("--no-prevalidate", action="store_true",
                    help="Skip cheap static pre-check against dump_text_indices")
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
        enforce_scope(func, action="append regfix rule for")
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

    # Refuse to add rules for non-active functions. This is the guardrail that
    # would have prevented an earlier catastrophic regression of 79 rules.
    enforce_scope(args.func, action="append regfix rule for")

    try:
        rule = build_rule(args)
    except ValueError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    # Pre-validate: cheap static check against the live maspsx output. Catches
    # the most common errors (subst pattern doesn't match, idx out of bounds)
    # in seconds instead of after a full build round-trip.
    if not args.no_prevalidate:
        ok, msg = pre_validate_rule(args)
        print(msg, file=sys.stderr)
        if not ok:
            print("Use --no-prevalidate to bypass and rely on live validation.",
                  file=sys.stderr)
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
