#!/usr/bin/env python3
"""bb2_extract_func_body.py -- pull a function body out of a src/*.c file
and wrap it in the standard permuter preamble (typedefs, externs).

Used by the backlog runner to populate permuter/<func>/base.c when the
function isn't an inline-asm cheat (where dc.sh inline-setup would
auto-fill from m2c).
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


STANDARD_PREAMBLE = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
#define NULL ((void *)0)

typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;
typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;
"""


def read_lf(path: Path) -> str:
    return path.read_bytes().decode("utf-8", errors="ignore")


def write_lf(path: Path, content: str) -> None:
    path.write_bytes(content.encode("utf-8"))


def extract_function(src_text: str, func_name: str) -> str | None:
    """Return the function definition `<rettype> <func_name>(args) { body }`
    by finding the matching close-brace via depth tracking. Returns None
    if not found.
    """
    lines = src_text.split("\n")
    pattern = re.compile(rf"^\s*(?:static\s+)?\w[\w\s\*]+\s\*?{re.escape(func_name)}\s*\(")
    start = None
    for i, line in enumerate(lines):
        if pattern.match(line):
            # Skip lines that end in ';' (declarations / extern)
            if line.rstrip().endswith(";"):
                continue
            start = i
            break
    if start is None:
        return None

    # Walk forward to opening brace
    brace_line = None
    for j in range(start, min(start + 10, len(lines))):
        if "{" in lines[j]:
            brace_line = j
            break
    if brace_line is None:
        return None

    # Track depth from brace_line onward
    depth = 0
    in_str = False
    in_chr = False
    in_block_comment = False
    end_line = None
    for j in range(brace_line, len(lines)):
        line = lines[j]
        k = 0
        while k < len(line):
            c = line[k]
            if in_block_comment:
                if c == "*" and k + 1 < len(line) and line[k + 1] == "/":
                    in_block_comment = False
                    k += 2
                    continue
            elif in_str:
                if c == "\\" and k + 1 < len(line):
                    k += 2
                    continue
                if c == '"':
                    in_str = False
            elif in_chr:
                if c == "\\" and k + 1 < len(line):
                    k += 2
                    continue
                if c == "'":
                    in_chr = False
            else:
                if c == "/" and k + 1 < len(line):
                    if line[k + 1] == "/":
                        break  # rest of line is comment
                    if line[k + 1] == "*":
                        in_block_comment = True
                        k += 2
                        continue
                elif c == '"':
                    in_str = True
                elif c == "'":
                    in_chr = True
                elif c == "{":
                    depth += 1
                elif c == "}":
                    depth -= 1
                    if depth == 0:
                        end_line = j
                        break
            k += 1
        if end_line is not None:
            break

    if end_line is None:
        return None

    return "\n".join(lines[start:end_line + 1])


def gather_externs(src_text: str, fn_body: str) -> list[str]:
    """Find function calls in fn_body and emit minimal extern decls for
    any that aren't defined in this src file or in fn_body itself.
    Returns a list of extern lines.
    """
    # Find every CALL: identifier followed by '('
    calls = set(re.findall(r"\b([A-Za-z_][A-Za-z_0-9]+)\s*\(", fn_body))
    # Drop known C keywords / control-flow / common builtins
    for kw in ("if", "while", "for", "switch", "return", "sizeof", "do",
               "void", "int", "char", "short", "long", "float", "double",
               "unsigned", "signed", "const", "volatile", "static", "register",
               "extern", "struct", "union", "enum", "typedef",
               "u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64",
               "vu8", "vs8", "vu16", "vs16", "vu32", "vs32",
               "__asm__", "asm"):
        calls.discard(kw)

    # Find extern decls already in src for these. We just emit "extern void <name>();"
    # for whatever isn't defined here AND isn't already extern'd in src.
    src_lines = src_text.split("\n")
    src_externs = {}
    for line in src_lines:
        m = re.match(r"\s*extern\s+([^;]+);", line)
        if m:
            decl = m.group(1).strip()
            # Get the identifier
            id_m = re.search(r"(\w+)\s*\(", decl)
            if id_m:
                src_externs[id_m.group(1)] = "extern " + decl + ";"

    # Find definitions in this src
    src_defs = set()
    for line in src_lines:
        m = re.match(r"^\s*(?:static\s+)?\w[\w\s\*]*\s\*?(\w+)\s*\(", line)
        if m and not line.rstrip().endswith(";") and not line.strip().startswith("extern"):
            src_defs.add(m.group(1))

    externs: list[str] = []
    for c in sorted(calls):
        if c == _CURRENT_FUNC:  # skip self
            continue
        if c in src_externs:
            externs.append(src_externs[c])
        elif c not in src_defs:
            # Don't know its signature, just emit a generic one
            externs.append(f"extern int {c}();")
    return externs


_CURRENT_FUNC = ""


def gather_data_refs(src_text: str, fn_body: str) -> list[str]:
    """Look for data symbol references (D_*, g_*) and emit `extern s32 X;`
    for any referenced. This is best-effort -- the permuter will fail
    compilation if types don't match, in which case the user can edit
    base.c by hand.
    """
    syms = set(re.findall(r"\b([DgG]_[A-Za-z_0-9]+|[a-z][a-z_0-9]*)\b", fn_body))
    # filter to ones starting with D_ or known g_/G_ prefixes
    data_syms = {s for s in syms if s.startswith(("D_", "g_", "G_"))}
    # Try to find their declarations in src. We only want lines that look
    # like top-level extern/decls -- skip lines that start with keywords
    # like "return", "if", "while", "for", "do", "goto", or whitespace
    # followed by a brace (function body).
    src_lines = src_text.split("\n")
    src_decls = {}
    CTRL_KEYWORDS = ("return", "if", "while", "for", "do", "goto", "switch",
                     "case", "break", "continue", "typedef", "static")
    for line in src_lines:
        stripped = line.lstrip()
        if any(stripped.startswith(kw + " ") or stripped.startswith(kw + "(") for kw in CTRL_KEYWORDS):
            continue
        # Must be at column 0 (top-level) or under 8 spaces leading
        if len(line) - len(stripped) > 4:
            continue
        m = re.match(r"\s*(?:extern\s+)?(\w[\w\s\*]+?)\s+(\w+)\s*(?:\[[^\]]*\])?\s*[;=]", line)
        if m:
            src_decls[m.group(2)] = m.group(0).rstrip(";").strip()

    decls: list[str] = []
    for sym in sorted(data_syms):
        if sym in src_decls:
            d = src_decls[sym]
            if not d.startswith("extern"):
                d = "extern " + d
            decls.append(d + ";")
        else:
            decls.append(f"extern s32 {sym};")
    return decls


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("func")
    p.add_argument("src_file")
    p.add_argument("--out", required=True)
    p.add_argument("--no-externs", action="store_true",
                   help="Don't try to gather externs (caller adds them)")
    args = p.parse_args()

    src_path = Path(args.src_file)
    if not src_path.exists():
        sys.exit(f"src file not found: {src_path}")

    src = read_lf(src_path)
    fn = extract_function(src, args.func)
    if fn is None:
        sys.exit(f"function {args.func} not found in {src_path}")
    global _CURRENT_FUNC
    _CURRENT_FUNC = args.func

    # Pull typedefs from src that come BEFORE the function definition. Any
    # struct/enum/typedef that the function might use should be in this
    # range. We greedy-match anything that looks like `typedef struct { ... } NAME;`
    # Skip names already in STANDARD_PREAMBLE.
    PREAMBLE_NAMES = {"Vec2s16", "Vec3s16", "Vec3s32", "Vec3", "VECTOR",
                      "SVECTOR", "CVECTOR", "DVECTOR", "MATRIX"}
    typedefs = []
    src_before_fn = src.split(fn)[0] if fn in src else src
    for m in re.finditer(
        r"typedef\s+(?:struct|union|enum)\s*(?:\w+)?\s*\{[^}]*\}\s*(\w+)\s*;",
        src_before_fn,
        re.DOTALL,
    ):
        if m.group(1) in PREAMBLE_NAMES:
            continue
        typedefs.append(m.group(0))

    parts = [STANDARD_PREAMBLE]
    if typedefs:
        parts.append("\n".join(typedefs))
    if not args.no_externs:
        externs = gather_externs(src, fn)
        data_refs = gather_data_refs(src, fn)
        if data_refs:
            parts.append("\n".join(data_refs))
        if externs:
            parts.append("\n".join(externs))
    parts.append(fn)

    out_path = Path(args.out)
    write_lf(out_path, "\n\n".join(parts) + "\n")
    print(f"wrote {out_path} ({len(fn.split(chr(10)))} body lines, "
          f"{len(externs) if not args.no_externs else 0} externs, "
          f"{len(data_refs) if not args.no_externs else 0} data refs)")


if __name__ == "__main__":
    main()
