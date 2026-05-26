#!/usr/bin/env python3
"""Append a validated regfix rule to regfix.txt or regfix_stage2.txt.

Replaces the ~109 ad-hoc tmp/add_regfix*.py scripts. Every regfix addition
should go through this so format is consistent and indices are validated
against live pipeline output before commit.

Subcommands (one rule per invocation):

    swap <func> <regA> <regB> [--range N-M]
    subst <func> <pattern> <replacement> --idx N
    subst_multi <func> <pattern> <line1> <line2> [<line3>...] --idx N
    splice <func> <line1> [<line2>...] --range N-M
    delete <func> --idx N
    insert <func> <asm_text> --idx N
    insert_after <func> <asm_text> --idx N
    insert_label <func> <label> --idx N
    reorder <func> <indices_csv> --range N-M
    fill_delay <func> --jal-idx N --src-idx M
    drain_delay <func> --jal-idx N
    modify <func> --old "<rule line>" --new "<rule line>"

NOTE: the subcommand (op) comes FIRST, then <func>:
    add_regfix.py insert_label func_8001F938 ".LfuncF938_join:" --idx 80

subst_multi:
    1-to-N regex-driven substitution. Replaces the matched maspsx line
    at idx N with multiple output lines. Each replacement string becomes
    its own line; indentation is taken from the original line. Use when
    GCC emits 1 instruction where target emits 2+ (e.g., GCC's blez vs
    target's slt+beq for non-zero comparisons).

splice:
    K-to-N range replacement (no regex). Removes every instruction line
    in [start, end] inclusive and emits the listed replacement strings
    in their place. K is the number of instructions in the range; N is
    the number of replacement strings. K and N may differ. Use when a
    multi-instruction GCC idiom needs to collapse or expand to a
    different shape (e.g., mine's `li $tN,1; move $vM,$tN; move $vM,$tN`
    → target's `addiu $vM,$0,1`).

insert_label:
    Inserts a label (with no instruction index) right after the
    instruction at idx N. Use it to synthesize a stable, project-
    controlled branch target — e.g. `.LfuncXXXX_join:` — so a subst or
    insert rule can point at it instead of a file-wide GCC `.L<N>`
    auto-label. GCC's `.L<N>` numbers drift whenever another function in
    the same .c file is added or removed; a synthesized label cannot.
    idx is a POST-INSERT-phase index (regfix.py phase 9) — see the
    phase-ordering docstring in regfix.py.

modify:
    Replace one existing rule line for <func> in place — a scoped,
    validated find-and-replace. Both --old and --new must be complete
    rule lines beginning with `<func>:` (it cannot reassign a rule to a
    different function). --old must appear exactly once. The edit is
    live-validated and rolled back on failure. Use this to repair a
    drifted label reference (or any stale operand) in an existing rule
    without hand-editing regfix.txt.

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

# Patterns indicating asm_text was passed through a shell that ate `$N`
# positional-arg literals (e.g. `wsl bash -c '... "lui $7, ..."'` strips $7
# because no positional args bound). Symptoms in the surviving text:
#   - opcode followed immediately by a comma (`lui ,` instead of `lui $7,`)
#   - two adjacent commas (`,,` from `$7,$7` collapsing)
#   - operand starting with bare `,` after whitespace (` , `)
# Catch these BEFORE the rule lands so the agent fixes the quoting instead
# of debugging a silent build mismatch later.
_SHELL_STRIPPED_RE = re.compile(
    r"(?:\b[a-z][a-z0-9.]*\s+,)|(?:,,)|(?:,\s*,)|(?:\s,\s)",
    re.IGNORECASE,
)


def detect_shell_stripped_dollars(asm_text: str) -> str | None:
    """Return a hint message if asm_text looks like shell ate $N literals."""
    if _SHELL_STRIPPED_RE.search(asm_text):
        return (
            f"asm_text {asm_text!r} contains a stripped-register pattern "
            f"(e.g. `lui ,` or `,,`). This usually means the shell "
            f"interpreted `$N` literals as positional args and substituted "
            f"empty strings.\n"
            f"FIX: pass the rule via `--raw \"<full line>\"` instead, OR "
            f"build it through a Python tmp/inject_*.py script (Path.write_text "
            f"with newline='\\n') instead of a `wsl bash -c '...'` command line."
        )
    return None


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
        hint = detect_shell_stripped_dollars(args.replacement)
        if hint:
            raise ValueError(hint)
        return f'{f}: subst "{args.pattern}" "{args.replacement}" @ {args.idx}'

    if op == "subst_multi":
        if len(args.replacements) < 1:
            raise ValueError(
                "subst_multi needs at least one replacement line (with one, "
                "it's equivalent to subst; with two or more it expands 1-to-N)"
            )
        for r in args.replacements:
            hint = detect_shell_stripped_dollars(r)
            if hint:
                raise ValueError(hint)
        quoted = " ".join(f'"{r}"' for r in args.replacements)
        return f'{f}: subst_multi "{args.pattern}" {quoted} @ {args.idx}'

    if op == "splice":
        a, b = parse_range(args.range)
        if len(args.replacements) < 1:
            raise ValueError("splice needs at least one replacement line")
        for r in args.replacements:
            hint = detect_shell_stripped_dollars(r)
            if hint:
                raise ValueError(hint)
        quoted = " ".join(f'"{r}"' for r in args.replacements)
        return f"{f}: splice {a}..{b} {quoted}"

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
        hint = detect_shell_stripped_dollars(args.asm_text)
        if hint:
            raise ValueError(hint)
        return f'{f}: insert "{args.asm_text}" @ {args.idx}'

    if op == "insert_after":
        hint = detect_shell_stripped_dollars(args.asm_text)
        if hint:
            raise ValueError(hint)
        return f'{f}: insert_after "{args.asm_text}" @ {args.idx}'

    if op == "insert_label":
        label = args.label.strip()
        if '"' in label:
            raise ValueError("insert_label label must not contain a double-quote")
        if not label.endswith(":"):
            raise ValueError(
                f"insert_label label must end with ':' (got {label!r}); "
                f"regfix emits it as a label-definition line"
            )
        if not re.match(r"^[.A-Za-z_][\w.]*:$", label):
            raise ValueError(
                f"insert_label label {label!r} doesn't look like a valid asm "
                f"label (expected e.g. `.LfuncXXXX_join:`)"
            )
        return f'{f}: insert_label "{label}" @ {args.idx}'

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


def _diagnose_zero_mismatch(pattern: str, line: str) -> str | None:
    """If `pattern` fails on `line` but matching after canonicalizing
    `$zero` <-> `$0` succeeds, the failure is purely a `$0`/`$zero` token
    disagreement. Return a specific actionable hint; else None.

    maspsx is INCONSISTENT — it emits `$0` for some instructions (`subu
    $4,$0,$6`) and `$zero` for others (`addiu $4,$zero,19`) in the same
    function. A pattern written with one form silently no-ops on the other.
    This was the time-sink in the func_8002EA24 match (2026-05-13)."""
    norm_pattern = pattern.replace("$zero", "$0")
    norm_line = line.replace("$zero", "$0")
    try:
        if re.search(norm_pattern, norm_line) and not re.search(pattern, line):
            return (
                f"  >>> $0/$zero MISMATCH detected. Your pattern and the maspsx\n"
                f"      line disagree only on `$0` vs `$zero`. maspsx emits BOTH\n"
                f"      forms depending on the instruction -- don't assume either.\n"
                f"      pattern: {pattern}\n"
                f"      line   : {line}\n"
                f"      FIX: write `\\$(?:0|zero)` in the pattern to match either,\n"
                f"      or copy the exact `$0`/`$zero` token from the line above."
            )
    except re.error:
        return None
    return None


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
                base = (
                    f"  pre-validate FAILED: subst pattern {args.pattern!r} does not\n"
                    f"  match the line at idx {idx}:\n"
                    f"      {line}\n"
                )
                zero_hint = _diagnose_zero_mismatch(args.pattern, line)
                if zero_hint:
                    return False, base + zero_hint
                return False, base + (
                    f"  Common gotcha: maspsx writes `$zero` not `$0`. See\n"
                    f"  memory/feedback_regfix_reference.md."
                )
        except re.error as e:
            return False, f"  pre-validate FAILED: invalid regex {args.pattern!r}: {e}"
        return True, f"  pre-validate OK: idx {idx} = {line!r}"

    if op == "subst_multi":
        idx = args.idx
        if idx not in insn_map:
            return False, (
                f"  pre-validate FAILED: idx {idx} out of bounds (max={max_idx}).\n"
                f"  Nearby:\n{_show_context(insn_map, idx)}"
            )
        line = insn_map[idx]
        try:
            if not re.search(args.pattern, line):
                base = (
                    f"  pre-validate FAILED: subst_multi pattern {args.pattern!r}\n"
                    f"  does not match the line at idx {idx}:\n"
                    f"      {line}\n"
                )
                zero_hint = _diagnose_zero_mismatch(args.pattern, line)
                if zero_hint:
                    return False, base + zero_hint
                return False, base + (
                    f"  Common gotcha: maspsx writes `$zero` not `$0`. The\n"
                    f"  pattern must consume enough of the line for the new\n"
                    f"  multi-line replacement to land cleanly (typically\n"
                    f"  use `.*` to consume any trailing label/operand)."
                )
        except re.error as e:
            return False, f"  pre-validate FAILED: invalid regex {args.pattern!r}: {e}"
        n = len(args.replacements)
        return True, (
            f"  pre-validate OK: idx {idx} = {line!r}\n"
            f"  will emit {n} replacement line{'s' if n != 1 else ''}"
        )

    if op == "splice":
        a, b = parse_range(args.range)
        if a not in insn_map:
            return False, (
                f"  pre-validate FAILED: splice start idx {a} out of bounds "
                f"(max={max_idx}).\n"
                f"  Nearby:\n{_show_context(insn_map, a)}"
            )
        if b not in insn_map:
            return False, (
                f"  pre-validate FAILED: splice end idx {b} out of bounds "
                f"(max={max_idx}).\n"
                f"  Nearby:\n{_show_context(insn_map, b)}"
            )
        k = sum(1 for i in range(a, b + 1) if i in insn_map)
        n = len(args.replacements)
        return True, (
            f"  pre-validate OK: splice {a}..{b} replaces {k} insn(s) "
            f"with {n} line(s)\n"
            f"  Range content:\n"
            + "\n".join(f"     {i:>4}: {insn_map[i]}"
                       for i in range(a, b + 1) if i in insn_map)
        )

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

    if op == "insert_label":
        idx = args.idx
        # insert_label uses POST-INSERT-phase indices (regfix.py phase 9),
        # which can legitimately exceed the pre-regfix max when the
        # function's own regfix rules net-add instructions. Don't hard-fail
        # on bounds here — the live build is authoritative for placement.
        if idx in insn_map:
            return True, (
                f"  pre-validate: insert_label after pre-regfix idx {idx} "
                f"= {insn_map[idx]!r}\n"
                f"  NOTE: idx is POST-INSERT-phase; if this function has "
                f"insert/delete rules the live position may differ."
            )
        return True, (
            f"  pre-validate: insert_label idx {idx} not in pre-regfix map "
            f"(max={max_idx}) — expected if it's a post-insert-phase idx; "
            f"live check authoritative."
        )

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


def handle_modify(args) -> int:
    """Replace one existing rule line for args.func in place.

    Scoped find-and-replace: both --old and --new must be complete rule
    lines for the active function, --old must appear exactly once (matched
    on whole-line strip-equality, never as a substring), and the result is
    live-validated — rolled back on failure. Mirrors --raw's "trust the
    live build" stance: no static pre-validation of the new rule."""
    func = args.func
    old_line = args.old.strip()
    new_line = args.new.strip()
    enforce_scope(func, action="modify regfix rule for")

    for flag, line in (("--old", old_line), ("--new", new_line)):
        m = re.match(r"^(\w+)\s*:", line)
        if not m:
            print(f"ERROR: {flag} line must start with `<func>:` — got: {line!r}",
                  file=sys.stderr)
            return 1
        if m.group(1) != func:
            print(f"ERROR: {flag} line is for {m.group(1)!r}, not the active "
                  f"function {func!r}. modify cannot reassign a rule to another "
                  f"function.", file=sys.stderr)
            return 1
    if old_line == new_line:
        print("ERROR: --old and --new are identical; nothing to modify",
              file=sys.stderr)
        return 1

    target = regfix_path(args.stage2)
    if not target.exists():
        print(f"ERROR: {target} does not exist", file=sys.stderr)
        return 3
    original = target.read_bytes()
    file_lines = original.decode("utf-8").split("\n")
    hits = [i for i, l in enumerate(file_lines) if l.strip() == old_line]
    if not hits:
        print(f"ERROR: --old line not found in {target.name}:\n  {old_line}\n"
              f"  (matched on whole-line equality after strip — check exact text)",
              file=sys.stderr)
        return 1
    if len(hits) > 1:
        print(f"ERROR: --old line appears {len(hits)} times in {target.name}; "
              f"refusing ambiguous modify.\n  {old_line}", file=sys.stderr)
        return 1

    if args.dry_run:
        print(f"(dry-run) would modify {target.name} (line {hits[0] + 1}):")
        print(f"  - {old_line}")
        print(f"  + {new_line}")
        return 0

    file_lines[hits[0]] = new_line
    target.write_bytes("\n".join(file_lines).encode("utf-8"))

    if args.no_validate:
        print(f"Modified {target.name} (line {hits[0] + 1}):")
        print(f"  - {old_line}")
        print(f"  + {new_line}")
        return 0

    ok, out = validate_live(func)
    if ok:
        print(f"Modified {target.name} (line {hits[0] + 1}):")
        print(f"  - {old_line}")
        print(f"  + {new_line}")
        return 0
    target.write_bytes(original)
    print("VALIDATION FAILED — rolled back. validate_regfix output:",
          file=sys.stderr)
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

    p = sub.add_parser("subst_multi",
                       help="1-to-N regex substitution (replace 1 line with K lines)")
    p.add_argument("func")
    p.add_argument("pattern")
    p.add_argument("replacements", nargs="+",
                   help="One or more replacement lines (each becomes its own output line)")
    p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("splice",
                       help="K-to-N range replacement (no regex)")
    p.add_argument("func")
    p.add_argument("replacements", nargs="+",
                   help="One or more replacement lines (each becomes its own output line)")
    p.add_argument("--range", required=True, help="N-M (inclusive)")

    p = sub.add_parser("delete")
    p.add_argument("func"); p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("insert")
    p.add_argument("func"); p.add_argument("asm_text")
    p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("insert_after")
    p.add_argument("func"); p.add_argument("asm_text")
    p.add_argument("--idx", type=int, required=True)

    p = sub.add_parser("insert_label",
                       help="Insert a synthesized label (stable branch target)")
    p.add_argument("func")
    p.add_argument("label", help="Label text ending in ':' e.g. .LfuncXXXX_join:")
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

    p = sub.add_parser("modify",
                       help="Replace an existing rule line for <func> in place")
    p.add_argument("func")
    p.add_argument("--old", required=True,
                   help="Exact existing rule line to replace (must start with <func>:)")
    p.add_argument("--new", required=True,
                   help="Replacement rule line (must start with <func>:)")

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

    # `modify` is a find-and-replace on an existing rule, not a rule build —
    # intercept before build_rule (which constructs a NEW rule from op args).
    if args.op == "modify":
        return handle_modify(args)

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
