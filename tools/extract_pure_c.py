#!/usr/bin/env python3
"""Auto-extract a pure-C version of a tier-3 function from src/<file>.c
and write it to permuter/<func>/base.c (or stdout with --stdout).

Strips:
  - `register T x asm("$N");` declarations (the pin alone disappears;
    if combined with an init like `register T x asm("$N") = expr;`,
    the line becomes `T x = expr;`)
  - `__asm__` / `asm()` blocks (volatile or not, single or multi-line)
  - INLINE_MOVE_ALIASING: comment blocks (the whole /* ... */ block
    that begins with that marker)

Wraps with:
  - Standard BB2 typedefs (u8, s8, u16, s16, u32, s32)
  - Generic externs for every func_XXX called or referenced and every
    D_XXX symbol used (pre-ANSI signatures — `extern int foo();` —
    which are permissive enough for cc1 to accept)

Limitations (documented):
  - Doesn't preserve the original function's local-variable
    declarations if they had `register asm()` pins (the pin is removed,
    so the var is now a plain local — usually fine).
  - Doesn't handle multi-statement asm tricks like
    `__asm__ volatile("nop"); v = expr;` — the asm is stripped but the
    surrounding expression may now be malformed if it depended on side
    effects of the asm.
  - Doesn't infer correct extern types — uses generic `extern int`
    signatures. Some functions may need explicit typing to compile.
    When this happens, compile_failed shows up in the diagnostic and
    the function must be set up manually.

Usage:
    python3 tools/extract_pure_c.py <func> [--stdout]
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = ROOT / "src"
PERMUTER_DIR = ROOT / "permuter"

# Function-definition opening regex (same forms as classify_inline_asm).
FUNC_OPEN_SAMELINE_RE = re.compile(
    r'^\s*(?:static\s+)?(?:inline\s+)?'
    r'[a-zA-Z_][\w*]*\s+\**\s*({func})\s*\([^;{{]*\)\s*\{{'
)
FUNC_OPEN_KR_RE = re.compile(
    r'^\s*(?:static\s+)?(?:inline\s+)?'
    r'[a-zA-Z_][\w*]*\s+\**\s*({func})\s*\([^;{{]*\)\s*$'
)
BRACE_ALONE_RE = re.compile(r'^\s*\{\s*$')


def find_function(func: str, src_text: str) -> tuple[int, int] | None:
    """Find (start_line, end_line) of `func` in src_text (1-indexed,
    inclusive). Returns None if not found.
    Function STARTS at the signature line and ENDS at the matching `}`
    on its own line at column 0."""
    lines = src_text.splitlines()
    sameline = re.compile(FUNC_OPEN_SAMELINE_RE.pattern.format(func=re.escape(func)))
    kr = re.compile(FUNC_OPEN_KR_RE.pattern.format(func=re.escape(func)))

    start_idx = None
    body_start_idx = None
    for i, line in enumerate(lines):
        if sameline.match(line):
            start_idx = i
            body_start_idx = i
            break
        if kr.match(line):
            start_idx = i
            # Look for `{` on the next non-blank line.
            j = i + 1
            while j < len(lines) and not lines[j].strip():
                j += 1
            if j < len(lines) and BRACE_ALONE_RE.match(lines[j]):
                body_start_idx = j
                break
            else:
                # Looked like K&R but no `{` followed; not a definition.
                start_idx = None

    if start_idx is None or body_start_idx is None:
        return None

    # Walk from body_start_idx tracking brace depth.
    depth = 0
    for i in range(body_start_idx, len(lines)):
        line = lines[i]
        # Strip string literals + line comments to avoid counting braces inside.
        stripped = re.sub(r'"(?:[^"\\]|\\.)*"', '', line)
        stripped = re.sub(r"'(?:[^'\\]|\\.)*'", '', stripped)
        stripped = re.sub(r'//.*$', '', stripped)
        opens = stripped.count('{')
        closes = stripped.count('}')
        depth += opens - closes
        if depth == 0 and (opens > 0 or closes > 0):
            return (start_idx + 1, i + 1)  # 1-indexed inclusive
    return None


def strip_inline_move_aliasing_comments(body: str) -> str:
    """Remove /* INLINE_MOVE_ALIASING: ... */ comment blocks."""
    return re.sub(
        r'/\*\s*INLINE_MOVE_ALIASING:[\s\S]*?\*/\s*\n?',
        '', body,
    )


def strip_asm_blocks(body: str) -> str:
    """Remove `__asm__(...);` and `asm(...);` statements (NOT register-
    qualifier `asm("$N")` in declarations — those are handled separately).

    Matches __asm__ / __asm followed by optional volatile / __volatile__
    and a parenthesized arg list, terminated by `;` after the closing
    paren. Multi-line spans are handled.
    """
    # We need a parser, not just a regex, because of nested parens in
    # asm template constraints. Walk character-by-character.
    out = []
    i = 0
    n = len(body)
    while i < n:
        # Look for __asm__ / __asm keywords (NOT bare `asm` — those are
        # likely register qualifiers).
        m = re.match(r'(__asm__|__asm)(?:\s*(?:volatile|__volatile__))?\s*\(',
                     body[i:])
        if m:
            # Skip whitespace before the keyword we matched to (don't
            # leave a hanging space).
            # Find matching close paren.
            paren_start = i + m.end() - 1  # position of `(`
            depth = 1
            k = paren_start + 1
            while k < n and depth > 0:
                c = body[k]
                if c == '"' or c == "'":
                    # Skip string literal
                    quote = c
                    k += 1
                    while k < n and body[k] != quote:
                        if body[k] == '\\' and k + 1 < n:
                            k += 2
                        else:
                            k += 1
                    k += 1
                    continue
                if c == '(':
                    depth += 1
                elif c == ')':
                    depth -= 1
                k += 1
            # Now k is one past the matching `)`. Skip any trailing
            # whitespace and the `;` (if present).
            while k < n and body[k] in ' \t':
                k += 1
            if k < n and body[k] == ';':
                k += 1
            # Skip trailing newline if the asm was on its own line.
            if k < n and body[k] == '\n':
                k += 1
            i = k
            continue
        out.append(body[i])
        i += 1
    return ''.join(out)


def strip_register_pins(body: str) -> str:
    """Convert `register T x asm("$N");` to `T x;` (drop the pin but
    keep the declaration so later uses of x compile). Convert
    `register T x asm("$N") = expr;` to `T x = expr;`.
    """
    lines = body.splitlines(keepends=True)
    out = []
    for line in lines:
        # `register TYPE NAME asm("$N")(  = expr)?;` — convert to plain decl.
        m = re.match(
            r'^(\s*)register\s+([^=;]*)\basm\s*\(\s*"[^"]+"\s*\)\s*(?:=\s*([^;]+))?\s*;\s*(?://.*)?$',
            line.rstrip("\n"),
        )
        if m:
            indent, type_and_name, init_expr = m.group(1), m.group(2).strip(), m.group(3)
            if init_expr is None:
                # No init — declare the var without the register pin.
                out.append(f"{indent}{type_and_name};\n")
            else:
                # Has init — keep as `TYPE NAME = expr;`.
                out.append(f"{indent}{type_and_name} = {init_expr.strip()};\n")
            continue
        out.append(line)
    return ''.join(out)


# Generic pre-ANSI extern declarations for callees + symbol refs.
TYPEDEFS = """typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
"""


def find_externs_needed(body: str) -> tuple[set[str], set[str]]:
    """Scan body for func_XXX calls and D_XXX symbol refs. Returns
    (funcs, syms). Excludes the function being defined itself."""
    # Calls: `func_NAME(` (likely a function call) — could also be
    # function-pointer initialization but the regex catches both.
    funcs = set(re.findall(r'\bfunc_[0-9A-Fa-f]+\b', body))
    # Data symbols: D_XXXX
    syms = set(re.findall(r'\bD_[0-9A-Fa-f]+\b', body))
    return funcs, syms


def build_base_c(func: str, body: str, src_file: str) -> str:
    """Wrap a stripped function body in a standalone base.c."""
    funcs, syms = find_externs_needed(body)
    funcs.discard(func)  # don't extern ourselves
    funcs_decls = "\n".join(f"extern int {f}();" for f in sorted(funcs))
    syms_decls = "\n".join(f"extern int {s};" for s in sorted(syms))
    header = (
        f"/* Auto-extracted from {src_file} by tools/extract_pure_c.py.\n"
        f" * Tier-3 hints (register-asm pins, __asm__ blocks,\n"
        f" * INLINE_MOVE_ALIASING comments) have been stripped. */\n"
        f"\n"
        + TYPEDEFS
        + "\n"
        + (funcs_decls + "\n" if funcs_decls else "")
        + (syms_decls + "\n" if syms_decls else "")
        + "\n"
    )
    return header + body.lstrip("\n") + ("\n" if not body.endswith("\n") else "")


def extract(func: str) -> tuple[str, str] | None:
    """Find `func` in any src/*.c file. Returns (src_file_rel, base_c_text)."""
    for src_file in sorted(SRC_DIR.glob("*.c")):
        text = src_file.read_text(encoding="utf-8", errors="ignore")
        span = find_function(func, text)
        if span is None:
            continue
        lines = text.splitlines(keepends=True)
        body = "".join(lines[span[0] - 1 : span[1]])
        body = strip_inline_move_aliasing_comments(body)
        body = strip_asm_blocks(body)
        body = strip_register_pins(body)
        rel = str(src_file.relative_to(ROOT))
        return rel, build_base_c(func, body, rel)
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--stdout", action="store_true",
                    help="write to stdout instead of permuter/<func>/base.c")
    ap.add_argument("--force", action="store_true",
                    help="overwrite existing base.c")
    args = ap.parse_args()

    result = extract(args.func)
    if result is None:
        print(f"ERROR: function '{args.func}' not found in any src/*.c", file=sys.stderr)
        return 1
    src_file, base_c_text = result

    if args.stdout:
        sys.stdout.write(base_c_text)
        return 0

    target_dir = PERMUTER_DIR / args.func
    target_dir.mkdir(parents=True, exist_ok=True)
    base_c = target_dir / "base.c"
    if base_c.exists() and not args.force:
        print(f"NOTE: {base_c.relative_to(ROOT)} exists; pass --force to overwrite",
              file=sys.stderr)
        # Still need target.s/compile.sh symlinks — check.
        if (target_dir / "compile.sh").exists():
            return 0
        # Fallthrough — only base.c exists; we'll still need to run setup.
        print(f"WARN: compile.sh missing; run `dc.sh setup {args.func} <src>` separately",
              file=sys.stderr)
        return 0
    base_c.write_text(base_c_text, encoding="utf-8", newline="\n")
    print(f"wrote {base_c.relative_to(ROOT)} ({len(base_c_text)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
