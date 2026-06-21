#!/usr/bin/env python3
"""Annotate a BB2 function body with PERM_* directives matching a hint slug.

Bridge between .claude/rules/<slug>.md ("try maneuver X") and
decomp-permuter's manual-mutation macros. Reads src/<file>.c, locates the
named function body, emits an annotated copy to tmp/permuter_candidates/
<func>.c. Does NOT modify src/.

Hint catalog is intentionally small (proof-of-concept; extend over time):
  register-asm-pins        sweep candidate registers via PERM_VAR
  shared-end-label         PERM_GENERAL exit forms (return v / goto end)
  loop-rotation-two-shift  PERM_GENERAL opaque-one wrap for shift counts
  loop-counter-fills-load-delay   PERM_LINESWAP over a load/inc/use block

See .claude/rules/permuter-directives.md for the full convention.
Output forms are PROPOSALS — every closing form must clear the cheat-by-
any-spelling vetting (.claude/rules/no-new-park-categories.md) and the
mandatory layer-2 cheat-reviewer (.claude/rules/review-discipline-
before-commit.md) BEFORE commit.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = REPO_ROOT / "src"
OUT_DIR = REPO_ROOT / "tmp" / "permuter_candidates"

# slug -> (one-line description, annotator function)
HINTS: dict[str, tuple[str, str]] = {
    "register-asm-pins": (
        "Sweep candidate registers via PERM_VAR(reg, ...). DIAGNOSTIC ONLY "
        "— the closing form MUST drop the pin and rely on C structure "
        "(see .claude/rules/register-asm-pins.md).",
        "register_asm_pins",
    ),
    "shared-end-label": (
        "Sweep per-case `return v;` vs. `goto end;` exit forms with "
        "PERM_GENERAL (see .claude/rules/shared-end-label.md).",
        "shared_end_label",
    ),
    "loop-rotation-two-shift": (
        "Wrap shift counts with SOTN-sanctioned opaque-one arithmetic "
        "via PERM_GENERAL (see .claude/rules/loop-rotation-two-shift.md).",
        "loop_rotation_two_shift",
    ),
    "loop-counter-fills-load-delay": (
        "Sweep load/address-advance/counter-increment/use orderings inside "
        "a loop body with PERM_LINESWAP (see .claude/rules/codegen-"
        "technique-index.md § loop-counter-fills-load-delay).",
        "loop_lineswap",
    ),
}

# Permit lookup -> match the function's opening brace line. BB2 functions are
# generally `<ret> func_<hex>(<args>) {` on one line, possibly preceded by a
# storage class. This pattern is regex-greedy on the return type & args.
FUNC_RE_TEMPLATE = (
    r"^[A-Za-z_][\w\s\*]*\b{name}\s*\([^)]*\)\s*\{{"
)


def find_source_file(func: str) -> Path | None:
    """Grep src/ for the function definition; return the file path."""
    pattern = re.compile(FUNC_RE_TEMPLATE.format(name=re.escape(func)), re.MULTILINE)
    for path in SRC_DIR.glob("*.c"):
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if pattern.search(text):
            return path
    return None


def extract_function(text: str, func: str) -> tuple[int, int, str] | None:
    """Return (start_offset, end_offset, body_text) of the function definition.

    end_offset points one past the matching closing brace.
    """
    pattern = re.compile(FUNC_RE_TEMPLATE.format(name=re.escape(func)), re.MULTILINE)
    m = pattern.search(text)
    if not m:
        return None
    start = m.start()
    # Walk braces from the opening one in the match.
    depth = 0
    i = m.end() - 1  # position of the opening '{'
    while i < len(text):
        ch = text[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return start, i + 1, text[start : i + 1]
        i += 1
    return None


# -- annotators -------------------------------------------------------------

PIN_RE = re.compile(
    r'register\s+([A-Za-z_]\w*(?:\s*\*+)?)\s+(\w+)\s+asm\s*\(\s*"(\$[a-z0-9]+)"\s*\)'
)


def annotate_register_asm_pins(body: str) -> tuple[str, list[str]]:
    """Replace `register T x asm("$N")` with PERM_VAR-driven sweep.

    Emits (per pin):
        PERM_VAR(<x>_reg, "$N")
        register T x asm(PERM_GENERAL("\"$N\"", "\"$t0\"", "\"$t1\"", "\"$s0\""))
    The first alternative is the original; the rest are common sweep candidates.
    """
    notes: list[str] = []
    SWEEP = ['"$t0"', '"$t1"', '"$t2"', '"$s0"', '"$s1"', '"$a3"', '"$v1"']

    def repl(m: re.Match) -> str:
        ty, name, orig_reg = m.group(1), m.group(2), m.group(3)
        orig_q = f'"{orig_reg}"'
        alts = [orig_q] + [r for r in SWEEP if r != orig_q]
        general = "PERM_GENERAL(" + ", ".join(alts) + ")"
        notes.append(f"swept pin: register {ty} {name} asm({orig_q})")
        return f"register {ty} {name} asm({general})"

    new = PIN_RE.sub(repl, body)
    if not notes:
        notes.append(
            "no `register T x asm(\"$N\")` pin found; nothing to sweep. Add one "
            "(diagnostic) before re-running."
        )
    return new, notes


RETURN_VALUE_RE = re.compile(r"\breturn\s+([A-Za-z_]\w*)\s*;")


def annotate_shared_end_label(body: str) -> tuple[str, list[str]]:
    """Wrap each `return <var>;` with PERM_GENERAL switching between an inline
    return and a `goto end;`. The shared end label is appended just before the
    function's closing brace.
    """
    notes: list[str] = []
    return_vars: set[str] = set()

    def repl(m: re.Match) -> str:
        var = m.group(1)
        return_vars.add(var)
        return f"PERM_GENERAL(return {var};, goto end;)"

    new = RETURN_VALUE_RE.sub(repl, body)
    if return_vars:
        notes.append(
            f"swept return-form for variables: {', '.join(sorted(return_vars))}. "
            "Added `end:` label and shared return at function tail; ensure the "
            "return variable is in scope at the end label."
        )
        # Insert `end: return <first_var>;` before the closing brace.
        var = sorted(return_vars)[0]
        new = new.rstrip()
        assert new.endswith("}")
        new = new[:-1].rstrip() + f"\n    end:\n    return {var};\n}}"
    else:
        notes.append(
            "no `return <var>;` of an identifier found; shared-end-label hint "
            "needs a named return value to sweep against."
        )
    return new, notes


SHIFT_RE = re.compile(r"(\b\w+\b)\s*(<<|>>)\s*(\b\w+\b)")


def annotate_loop_rotation_two_shift(body: str) -> tuple[str, list[str]]:
    """Around each variable-count shift, propose an opaque-one wrap.

    `x << n` -> PERM_GENERAL(x << n, x << (n - 1 + one))
    where `one` is a `s32 one = 1;` declared at top of body. SOTN-sanctioned
    per the index; the permuter scores both forms.
    """
    notes: list[str] = []
    count = 0

    def repl(m: re.Match) -> str:
        nonlocal count
        lhs, op, rhs = m.group(1), m.group(2), m.group(3)
        # Skip shifts by literal constants — opaque-one only helps variable counts.
        if rhs.isdigit() or rhs.startswith("0x"):
            return m.group(0)
        count += 1
        return f"PERM_GENERAL({lhs} {op} {rhs}, {lhs} {op} ({rhs} - 1 + one))"

    new = SHIFT_RE.sub(repl, body)
    if count:
        # Inject `s32 one = 1;` after the opening `{`.
        new = new.replace("{", "{\n    s32 one = 1;  /* SOTN-sanctioned opaque */", 1)
        notes.append(
            f"wrapped {count} variable-count shift(s) with opaque-one PERM_GENERAL; "
            "`s32 one = 1;` declared at top of body."
        )
    else:
        notes.append(
            "no variable-count shift found; loop-rotation-two-shift hint not "
            "applicable to this body."
        )
    return new, notes


DO_WHILE_RE = re.compile(r"do\s*\{([^{}]*)\}\s*while\s*\([^)]+\)\s*;", re.DOTALL)


def annotate_loop_lineswap(body: str) -> tuple[str, list[str]]:
    """Wrap the first `do { ... } while (...);` body with PERM_LINESWAP."""
    notes: list[str] = []
    m = DO_WHILE_RE.search(body)
    if not m:
        notes.append(
            "no `do { ... } while (...);` loop found; loop-counter-fills-load-"
            "delay hint expects an explicit do-while. Hand-author the PERM_"
            "LINESWAP region around the load/inc/use block instead."
        )
        return body, notes
    inner = m.group(1).strip("\n")
    wrapped = f"do {{\nPERM_LINESWAP(\n{inner}\n)\n}} while " + m.group(0).split("while", 1)[1]
    new = body[: m.start()] + wrapped + body[m.end():]
    notes.append(
        "wrapped first do-while body with PERM_LINESWAP — make sure every line "
        "is a complete C statement (per README; otherwise use "
        "PERM_LINESWAP_TEXT)."
    )
    return new, notes


ANNOTATORS = {
    "register_asm_pins": annotate_register_asm_pins,
    "shared_end_label": annotate_shared_end_label,
    "loop_rotation_two_shift": annotate_loop_rotation_two_shift,
    "loop_lineswap": annotate_loop_lineswap,
}


# -- driver -----------------------------------------------------------------


def make_candidate(func: str, hint: str) -> Path:
    spec = HINTS.get(hint)
    if spec is None:
        sys.exit(
            f"unknown hint '{hint}'. --list-hints shows the catalog."
        )
    _description, annot_key = spec
    annotator = ANNOTATORS[annot_key]

    src_path = find_source_file(func)
    if src_path is None:
        sys.exit(f"function {func} not found under src/*.c")

    text = src_path.read_text(encoding="utf-8", errors="replace")
    extracted = extract_function(text, func)
    if extracted is None:
        sys.exit(f"function {func} matched but body not extractable from {src_path}")
    _start, _end, body = extracted

    new_body, notes = annotator(body)

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUT_DIR / f"{func}.c"
    preamble = [
        f"// Permuter candidate for {func} (hint: {hint}).",
        f"// Source: {src_path.relative_to(REPO_ROOT).as_posix()}",
        "// Generated by tools/permuter_annotate.py — DO NOT commit to src/.",
        "// See .claude/rules/permuter-directives.md for the conventions and the",
        "// MANDATORY post-permuter cheat-vetting (no-new-park-categories +",
        "// review-discipline-before-commit layer-2).",
        "//",
        "// Generator notes:",
    ]
    for note in notes:
        preamble.append(f"//   - {note}")
    preamble.append("")
    # Synthesise minimal PERM_* declarations so cpp parses if you preprocess
    # the file alone. The real permuter run uses its own prelude.inc.
    preamble += [
        "#ifndef PERM_GENERAL",
        "#  define PERM_GENERAL(a, ...) (a)",
        "#  define PERM_VAR(...)",
        "#  define PERM_RANDOMIZE(code) code",
        "#  define PERM_FORCE_SAMELINE(code) code",
        "#  define PERM_LINESWAP(...) __VA_ARGS__",
        "#  define PERM_INT(lo, hi) (lo)",
        "#  define PERM_IGNORE(code) code",
        "#  define PERM_PRETEND(code) code",
        "#  define PERM_ONCE(...) ",
        "#endif",
        "",
        '#include "common.h"',
        "",
    ]
    out_path.write_text("\n".join(preamble) + new_body + "\n", encoding="utf-8")
    return out_path


def print_invocation_hint(func: str, src_path: Path | None) -> None:
    src = src_path.relative_to(REPO_ROOT).as_posix() if src_path else f"src/<file>.c"
    print()
    print("Next steps (run inside WSL, repo root):")
    print(f"  source .venv/bin/activate")
    print(
        f"  python3 tools/decomp-permuter/import.py {src} asm/funcs/{func}.s"
    )
    print(
        f"  # Replace base.c with the candidate, then run permuter with -j N:"
    )
    print(
        f"  cp tmp/permuter_candidates/{func}.c "
        f"tools/decomp-permuter/<dir>/base.c"
    )
    print(
        f"  ./tools/decomp-permuter/permuter.py "
        f"tools/decomp-permuter/<dir> -j 8 --stop-on-zero"
    )
    print()
    print("Reminder: any closing form must clear the cheat-by-any-spelling")
    print("checklist (.claude/rules/no-new-park-categories.md) AND the layer-2")
    print("cheat-reviewer (.claude/rules/review-discipline-before-commit.md)")
    print("BEFORE you commit. Permuter is cheat-blind by design.")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--func", help="function name, e.g. func_80045294")
    ap.add_argument("--hint", help="hint slug; --list-hints to see catalog")
    ap.add_argument(
        "--list-hints",
        action="store_true",
        help="print supported hints and exit",
    )
    args = ap.parse_args()

    if args.list_hints:
        width = max(len(slug) for slug in HINTS)
        print("Supported hints:")
        for slug, (desc, _) in HINTS.items():
            print(f"  {slug.ljust(width)}  {desc}")
        return 0

    if not args.func or not args.hint:
        ap.error("--func and --hint are required (or pass --list-hints)")

    out_path = make_candidate(args.func, args.hint)
    src_path = find_source_file(args.func)
    print(f"wrote candidate: {out_path.relative_to(REPO_ROOT).as_posix()}")
    print_invocation_hint(args.func, src_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
