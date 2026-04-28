"""Shared m2c-output post-processing for the permuter sandbox.

Used by extract_inline_asm.py, gte_migrate.py, batch_permuter.py. Centralizes
the cleanups so improvements land everywhere.

Common failure modes addressed (ranked by frequency in the 33-function ERROR
sample from the 2026-04-27 batch):

  UNDECLARED_IDENT (~55%)
    -> Inject `GameObj`, `Vec3s*`, `SVECTOR`/`VECTOR`/`MATRIX`/`CVECTOR`,
       and standard m2c struct typedefs alongside the basic int typedefs.

  UNARY_STAR (~12%)
    -> Wrap arithmetic-pointer expressions inside `*(...)` with `(s32 *)`
       casts so GCC accepts the deref.

  M2C_PSEUDO_VAR (~9%) -- sp10, saved_reg_*, subroutine_arg*
    -> Auto-declare every referenced m2c pseudo at the top of the function
       body as `s32`.

  PARSE_BEFORE_COLON (~6%)
    -> Strip m2c warning lines that leaked into stdout
       (lines starting with `Warning:` outside a `/* */` block).

  M2C_ERROR(/* unset register ... */)
    -> Replace with a placeholder `0` so it parses; correctness lost but
       the model can fix it after the structure compiles.

The goal is to MINIMIZE compile failures so smart_match and permuter
actually get to do work. Even an imperfect compilable base.c is far more
useful than an exact one that won't parse.
"""
from __future__ import annotations

import re

TYPEDEFS = (
    "typedef unsigned char u8;\n"
    "typedef signed char s8;\n"
    "typedef unsigned short u16;\n"
    "typedef signed short s16;\n"
    "typedef unsigned int u32;\n"
    "typedef signed int s32;\n"
    "typedef unsigned long long u64;\n"
    "typedef signed long long s64;\n"
    "typedef volatile u8 vu8;\n"
    "typedef volatile s8 vs8;\n"
    "typedef volatile u16 vu16;\n"
    "typedef volatile s16 vs16;\n"
    "typedef volatile u32 vu32;\n"
    "typedef volatile s32 vs32;\n"
    "#define NULL ((void *)0)\n"
)

# Common m2c-context structs we want available in the sandbox. Layouts must
# match what m2c sees (so field accesses round-trip cleanly).
COMMON_STRUCTS = """
typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;
typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The
 * field layout is the union of all observed accesses; m2c picks the type
 * that best fits each access site. Mirroring smart_match.py's layout. */
typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;
"""


def strip_m2c_warnings(code: str) -> str:
    """m2c sometimes prints warnings to stdout that get mixed into the C
    body. Lines like:
        Warning: missing "jr $ra" in last block of func_X (initial).
    end up as parse errors. Drop any line that begins with `Warning:` AND
    isn't inside a /* */ comment."""
    out_lines: list[str] = []
    in_block_comment = False
    for line in code.splitlines():
        if in_block_comment:
            out_lines.append(line)
            if "*/" in line:
                in_block_comment = False
            continue
        # Quick heuristic: strip the obvious m2c stderr leak
        if re.match(r"^\s*Warning:\s+", line):
            continue
        if "/*" in line and "*/" not in line:
            in_block_comment = True
        out_lines.append(line)
    return "\n".join(out_lines)


def fix_m2c_macros(code: str) -> str:
    """Replace M2C_* macros with usable C."""
    code = re.sub(r"\bM2C_UNK\b", "s32", code)

    def fix_field(m):
        expr = m.group(1)
        typ = m.group(2).strip()
        offset = m.group(3).strip()
        return f"*({typ})((s32)({expr}) + {offset})"
    code = re.sub(r"M2C_FIELD\(([^,]+),\s*([^,]+),\s*([^)]+)\)", fix_field, code)

    code = re.sub(r"M2C_MEMSET\([^)]*\)", "/* M2C_MEMSET - TODO */ 0", code)

    # M2C_ERROR placeholders (e.g. unset register reads). Replace with `0` so
    # the function parses; semantically wrong but permuter can still operate.
    code = re.sub(
        r"M2C_ERROR\s*\(\s*/\*[^*]*\*/\s*\)",
        "/* M2C_ERROR */ 0",
        code,
    )
    return code


# Pseudo-variables m2c emits without declaring (they map to inferred stack/
# saved-reg slots, or to register names like `sp` for hand-written swaps).
# We insert declarations at the function-body opening.
PSEUDO_VAR_RE = re.compile(
    r"\b(sp[0-9A-Fa-f]+|saved_reg_\w+|subroutine_arg\d+)\b"
)
# A few register names m2c sometimes references directly when it can't infer
# a normal C representation. They become `s32` placeholders so the function
# at least parses; semantically wrong but lets permuter run.
BARE_REG_NAMES = {"sp", "fp", "gp"}


def declare_pseudo_vars(code: str) -> str:
    """Find every m2c pseudo-var and inject `s32 <name>;` declarations at
    the top of the function body, after the existing decls."""
    used = set(PSEUDO_VAR_RE.findall(code))
    # Drop ones already declared
    declared = set(re.findall(
        r"\b(?:s32|u32|s16|u16|s8|u8|void\s*\*)\s+"
        r"(sp[0-9A-Fa-f]+|saved_reg_\w+|subroutine_arg\d+)\b",
        code,
    ))
    missing = sorted(used - declared)

    # Bare register names referenced as C identifiers
    for reg in BARE_REG_NAMES:
        if re.search(rf"\b{reg}\b", code):
            # Skip if used inside an asm string literal -- only count
            # if it appears as a free identifier (no $ prefix, not in "..").
            free = re.findall(rf"(?<![\$\w]){reg}(?![\w])", code)
            # Also ensure it isn't part of a struct-field access pattern
            if free and reg not in declared:
                missing.append(reg)
                declared.add(reg)

    if not missing:
        return code

    # Insert after the first `{` that opens a function body.
    m = re.search(r"^[\w\s\*]+\b\w+\s*\([^)]*\)\s*\{\s*$", code, re.MULTILINE)
    if not m:
        return code
    insert_at = m.end()
    decls = "\n    " + "\n    ".join(f"s32 {v};" for v in missing) + "\n"
    return code[:insert_at] + decls + code[insert_at:]


def remove_project_includes(code: str) -> str:
    code = re.sub(r'#include\s+"common\.h"\s*\n?', "", code)
    code = re.sub(r'#include\s+"include_asm\.h"\s*\n?', "", code)
    code = re.sub(r'#include\s+"m2c_macros\.h"\s*\n?', "", code)
    code = re.sub(r'#include\s+"gte\.h"\s*\n?', "", code)
    return code


def add_externs(code: str, self_func: str) -> str:
    """Inject `extern` for every D_<addr> and func_<addr> referenced but
    not declared in the body."""
    used_globs = set(re.findall(r"\bD_[0-9A-Fa-f]{8}\b", code))
    decl_globs = set(re.findall(
        r"extern\s+[\w\s\*\[\]]+?(D_[0-9A-Fa-f]{8})", code))

    used_funcs = set(re.findall(r"\bfunc_[0-9A-Fa-f]{8}\b", code))
    decl_funcs = set(re.findall(
        r"(?:extern\s+\w+\s+|(?:void|s32|u32|s16|u16|s8|u8)\s+\*?)"
        r"(func_[0-9A-Fa-f]{8})\s*\(",
        code,
    ))

    extern_lines: list[str] = []
    for g in sorted(used_globs - decl_globs):
        extern_lines.append(f"extern s32 {g};")
    for fn in sorted(used_funcs - decl_funcs):
        if fn != self_func:
            extern_lines.append(f"extern s32 {fn}();")
    if not extern_lines:
        return code
    return "\n".join(extern_lines) + "\n\n" + code


def fix_unary_star(code: str) -> str:
    """No-op for now. The naive `*\\(...\\)` rewrite was producing
    `*((s32 *)(s32 *))` because lazy regex matches grabbed the cast prefix
    of an already-fixed M2C_FIELD expansion. A correct version needs a
    real expression parser; until then, leave m2c's unary-* output alone
    and let the model fix specific cases by hand. (See errror sample on
    func_8005C4C0 in tmp/retry_errors.progress.log for the failure mode.)"""
    return code


def make_compilable(code: str, self_func: str, *, include_gte: bool = False) -> str:
    """Run the full chain. The order matters:
       1. Strip m2c warning leak
       2. Drop project-only #includes
       3. Replace M2C_* macros with usable C
       4. Inject pseudo-var decls (BEFORE adding externs/typedefs prefix)
       5. Add externs for un-declared D_/func_ symbols
       6. Wrap arithmetic-pointer derefs
       7. Prepend typedefs + struct prelude (+ gte.h prelude if requested)"""
    code = strip_m2c_warnings(code)
    code = remove_project_includes(code)
    code = fix_m2c_macros(code)
    code = declare_pseudo_vars(code)
    code = add_externs(code, self_func)
    code = fix_unary_star(code)

    prefix = TYPEDEFS + COMMON_STRUCTS
    if include_gte:
        # Lazy import to avoid touching the filesystem at module import time
        from pathlib import Path
        gte_path = Path(__file__).resolve().parent.parent / "include" / "gte.h"
        if gte_path.exists():
            gte_h = gte_path.read_text(encoding="utf-8")
            gte_inline = re.sub(r'^\s*#ifndef.*\n#define.*\n', "", gte_h)
            gte_inline = re.sub(r'^\s*#endif\s*/\*\s*GTE_H\s*\*/\s*$',
                                "", gte_inline, flags=re.MULTILINE)
            gte_inline = re.sub(r'^\s*#include\s+"common\.h"\s*\n?', "",
                                gte_inline, flags=re.MULTILINE)
            # Strip duplicate struct defs (we already have them in prefix)
            gte_inline = re.sub(
                r"typedef struct (?:VECTOR|SVECTOR|CVECTOR|DVECTOR|MATRIX)[^;]*;",
                "", gte_inline,
            )
            prefix += "\n" + gte_inline + "\n"

    return prefix + code
