#!/usr/bin/env python3
"""Audit callers of a function for argument-count consistency.

When you decompile an inline_asm stub to C, GCC's only signal about the
function's arity is the C declaration. If callers pass MORE arguments
than your declaration accepts, GCC silently dead-codes the extra-arg
computations in callers — re-compiling those callers to a shorter
sequence than the original. The change is invisible until you `verify
--all` and find dozens of unrelated functions diff-ing.

Usage:
    python3 tools/caller_audit.py <func_name>
    python3 tools/caller_audit.py <func_name> --json

Reports:
    Max arg count observed across all C-source callers (skips inline_asm
    `jal func` callers, which don't reveal arity). Then parses the
    function's *current* C declaration in src/ (if any) and warns if
    the declared arity is less than max-observed.

Exit codes:
    0 — no callers found, or declared arity >= max observed (safe)
    1 — declared arity < max observed (UNSAFE: signature is too narrow)
    2 — internal error (e.g., src/ not readable)

Limitations:
    * Skips function-pointer assignments (`p = func;`) — they don't
      reveal arity.
    * Skips inline-asm `jal` callers — they pass via $a0/$a1/$a2/$a3
      regardless.
    * Multi-line calls handled; nested parens, strings, comments
      tolerated.
    * Won't detect TYPE mismatches (signed vs unsigned widening). Just
      arity. The cascade-regression case is almost always arity, not
      type.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import List, Tuple

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"


def strip_comments_and_strings(text: str) -> str:
    """Remove /* */, //, and "..." string contents (replace with spaces, preserving line numbers and offsets)."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            # line comment
            j = text.find("\n", i)
            if j == -1:
                out.append(" " * (n - i))
                i = n
            else:
                out.append(" " * (j - i))
                i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            if j == -1:
                out.append(" " * (n - i))
                i = n
            else:
                out.append(" " * (j + 2 - i))
                i = j + 2
        elif c == '"':
            out.append('"')
            i += 1
            while i < n and text[i] != '"':
                if text[i] == "\\" and i + 1 < n:
                    out.append("  ")
                    i += 2
                else:
                    out.append(" ")
                    i += 1
            if i < n:
                out.append('"')
                i += 1
        elif c == "'":
            # char literal
            out.append("'")
            i += 1
            while i < n and text[i] != "'":
                if text[i] == "\\" and i + 1 < n:
                    out.append("  ")
                    i += 2
                else:
                    out.append(" ")
                    i += 1
            if i < n:
                out.append("'")
                i += 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def is_inside_inline_asm(text: str, pos: int) -> bool:
    """Return True if `pos` falls inside an `__asm__(...)` block."""
    # Search backward for the nearest __asm__( and check its closing ).
    asm_re = re.compile(r"__asm__\s*\(")
    last = None
    for m in asm_re.finditer(text, 0, pos):
        last = m
    if not last:
        return False
    # Walk forward from last.start() counting parens, tolerating string content.
    depth = 0
    in_string = False
    i = last.end() - 1  # at the open paren
    while i < len(text):
        ch = text[i]
        if not in_string:
            if ch == '"':
                in_string = True
            elif ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    return i >= pos
        else:
            if ch == "\\" and i + 1 < len(text):
                i += 2
                continue
            if ch == '"':
                in_string = False
        i += 1
    return True  # unterminated; treat as inside


def find_top_level_arg_count(text: str, open_paren_pos: int) -> int | None:
    """Given position of `(`, walk the matching `)` and count top-level commas + 1.

    Returns None if unmatched parens or empty paren returns 0."""
    if open_paren_pos >= len(text) or text[open_paren_pos] != "(":
        return None
    depth = 0
    commas = 0
    has_content = False
    i = open_paren_pos
    n = len(text)
    while i < n:
        ch = text[i]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return (commas + 1) if has_content else 0
        elif ch == "," and depth == 1:
            commas += 1
            has_content = True
        elif depth >= 1 and not ch.isspace():
            has_content = True
        i += 1
    return None  # unterminated


def _looks_like_declaration(stripped: str, name_start: int) -> bool:
    """Heuristic: is the `func_name(` at name_start a decl/def, not a call?

    A call is preceded by whitespace + (`;`, `{`, `}`, `(`, `,`, `=`, `&`,
    `*`, `!`, `?`, `:`, `+`, `-`) or start-of-file. A decl is preceded by
    whitespace + word char (the return-type token), e.g. `void f(`."""
    j = name_start - 1
    while j >= 0 and stripped[j] in " \t":
        j -= 1
    if j < 0:
        return False
    ch = stripped[j]
    # Newline → look further back to last non-space token.
    if ch == "\n":
        k = j - 1
        while k >= 0 and stripped[k] in " \t\n":
            k -= 1
        if k < 0:
            return False
        ch = stripped[k]
    if ch.isalnum() or ch == "_":
        return True  # preceded by an identifier/keyword → likely decl
    return False


def find_callers(func_name: str, src_dir: Path) -> List[dict]:
    """Find every C-source call site of `func_name`. Skip inline-asm contexts and decls."""
    call_re = re.compile(rf"\b{re.escape(func_name)}\s*\(")
    callers = []
    for c_file in sorted(src_dir.glob("*.c")):
        try:
            raw = c_file.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        stripped = strip_comments_and_strings(raw)
        for m in call_re.finditer(stripped):
            paren_pos = m.end() - 1
            if is_inside_inline_asm(raw, m.start()):
                continue
            if _looks_like_declaration(stripped, m.start()):
                continue
            arg_count = find_top_level_arg_count(stripped, paren_pos)
            if arg_count is None:
                continue
            line_no = raw[: m.start()].count("\n") + 1
            line_text = raw.splitlines()[line_no - 1].strip() if line_no - 1 < len(raw.splitlines()) else ""
            callers.append({
                "file": c_file.name,
                "line": line_no,
                "args": arg_count,
                "snippet": line_text[:120],
            })
    return callers


def find_declarations(func_name: str, src_dir: Path) -> List[dict]:
    """Find C declarations/definitions of `func_name`. Returns args count when parseable."""
    # Two-step: find every `func_name(` then walk back/forward to classify
    # as a declaration ("...func(args);"") or definition ("...func(args) {").
    name_re = re.compile(rf"\b{re.escape(func_name)}\s*\(")
    decls = []
    for c_file in sorted(src_dir.glob("*.c")):
        try:
            raw = c_file.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        stripped = strip_comments_and_strings(raw)
        for m in name_re.finditer(stripped):
            if is_inside_inline_asm(raw, m.start()):
                continue
            paren_open = m.end() - 1
            # Walk back to start of declaration: scan to the previous ; } or start-of-file.
            # If we hit one of those, the chunk between is a candidate declarator.
            start = paren_open
            while start > 0:
                ch = stripped[start - 1]
                if ch in ";}":
                    break
                start -= 1
            head = stripped[start:paren_open].strip()
            # head should look like: <ret-type-tokens> <name>
            # If head contains an `=` it's an assignment (function ptr) — skip.
            if "=" in head:
                continue
            # Must contain the name as the last token-ish thing.
            if not head.endswith(func_name):
                continue
            # The token before name should look like a type or '*'.
            # Lazy: just require head to have at least one non-name token.
            pre = head[: -len(func_name)].rstrip()
            if not pre or not re.search(r"[A-Za-z_*]", pre):
                continue
            # Now find matching ).
            depth = 0
            end = paren_open
            n = len(stripped)
            while end < n:
                ch = stripped[end]
                if ch == "(":
                    depth += 1
                elif ch == ")":
                    depth -= 1
                    if depth == 0:
                        break
                end += 1
            if end >= n:
                continue  # unterminated
            args_text = stripped[paren_open + 1 : end].strip()
            # Look at next non-space char after `)`: ; means declaration, { means definition.
            tail = end + 1
            while tail < n and stripped[tail].isspace():
                tail += 1
            if tail >= n or stripped[tail] not in ";{":
                # Not a top-level decl — could be a call or pointer declarator.
                continue
            is_def = stripped[tail] == "{"
            if not args_text or args_text == "void":
                arg_count = 0
            else:
                depth = 0
                commas = 0
                for ch in args_text:
                    if ch == "(":
                        depth += 1
                    elif ch == ")":
                        depth -= 1
                    elif ch == "," and depth == 0:
                        commas += 1
                arg_count = commas + 1
            line_no = raw[: m.start()].count("\n") + 1
            decls.append({
                "file": c_file.name,
                "line": line_no,
                "args": arg_count,
                "is_def": is_def,
            })
    return decls


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func_name")
    ap.add_argument("--json", action="store_true", help="Emit JSON instead of text")
    ap.add_argument("--quiet", action="store_true",
                    help="Print nothing on success (declared >= max-observed)")
    args = ap.parse_args()

    if not SRC.is_dir():
        print(f"ERROR: {SRC} not found", file=sys.stderr)
        return 2

    callers = find_callers(args.func_name, SRC)
    decls = find_declarations(args.func_name, SRC)

    max_observed = max((c["args"] for c in callers), default=0)
    declared = None
    declared_loc = None
    for d in decls:
        if d["is_def"]:
            declared = d["args"]
            declared_loc = f"{d['file']}:{d['line']}"
            break
    if declared is None and decls:
        declared = decls[0]["args"]
        declared_loc = f"{decls[0]['file']}:{decls[0]['line']}"

    mismatch = (declared is not None and declared < max_observed)

    if args.json:
        out = {
            "func": args.func_name,
            "max_observed": max_observed,
            "declared": declared,
            "declared_loc": declared_loc,
            "callers": callers,
            "decls": decls,
            "mismatch": mismatch,
        }
        json.dump(out, sys.stdout, indent=2)
        sys.stdout.write("\n")
        return 1 if mismatch else 0

    if args.quiet and not mismatch:
        return 0

    print(f"caller-audit: {args.func_name}")
    print(f"  callers in src/: {len(callers)}")
    if callers:
        # Histogram of arg counts
        hist: dict[int, int] = {}
        for c in callers:
            hist[c["args"]] = hist.get(c["args"], 0) + 1
        for n in sorted(hist):
            print(f"    {n}-arg: {hist[n]} call(s)")
        print(f"  max args observed: {max_observed}")
    if declared is not None:
        print(f"  declared arity: {declared} (at {declared_loc})")
    else:
        print(f"  declared arity: <not found in C; may be inline_asm or extern only>")

    if mismatch:
        print()
        print(f"  ERROR: declared arity {declared} < max observed {max_observed}")
        print(f"  GCC will dead-code the extra-arg computations in callers,")
        print(f"  causing a CASCADE of sibling-function regressions when the")
        print(f"  build links. Add unused params to bring the declaration up.")
        print()
        print(f"  Sample callers passing {max_observed} args:")
        for c in callers:
            if c["args"] == max_observed:
                print(f"    {c['file']}:{c['line']}  {c['snippet']}")
                break
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
