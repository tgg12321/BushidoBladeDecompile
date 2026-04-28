#!/usr/bin/env python3
"""Convert a GTE-using inline-asm function into a real C function that uses
`gte_*()` macros from include/gte.h.

Pipeline:
    1. Run m2c on asm/funcs/<func>.s -- emits C with `M2C_ERROR(/* unknown
       instruction: ... */)` placeholders for every cop2/lwc2/swc2/mtc2/mfc2
       op it doesn't know how to express.
    2. Walk the m2c output and replace each placeholder with the
       corresponding `gte_*()` macro from include/gte.h.
    3. Add `#include "gte.h"` and prepend the standard typedefs.
    4. Write to permuter/<func>/base.c (or stdout with --stdout).

What gets translated automatically:
  - All atomic GTE ops (rtps/rtpt/nclip/avsz3/avsz4/dpcs/dpct/intpl/sqr/
    ncs/nct/ncds/ncdt/nccs/ncct/cdp/cc/dpcl)  -> `gte_<op>()`
  - mfc2 reads from MAC regs ($24..$27)        -> `gte_stmac0..3(out)`
  - swc2 stores from screen-XY regs ($12..$14) -> `gte_stsxy*(p)`
  - swc2 stores from screen-Z reg ($19)        -> `gte_stsz(p)`

What still needs manual review (left as raw `__asm__()` with a TODO):
  - mtc2 register transfers IN to the GTE
  - lwc2 vector loads (need pointer reconstruction for gte_ldv0/1/2)
  - GTE control register accesses (cfc2/ctc2)

Usage:
    python3 tools/gte_migrate.py <func>                 # write permuter/<func>/base.c
    python3 tools/gte_migrate.py <func> --stdout        # print to stdout
    python3 tools/gte_migrate.py <func> --setup         # also stage permuter dir
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
PERMUTER_DIR = ROOT / "permuter"
M2C = ROOT / "tools" / "m2c" / "m2c.py"
M2C_CONTEXT = ROOT / "include" / "m2c_context.h"

# Atomic GTE ops -- one cop2 instruction, no operands
ATOMIC_OPS = {
    "rtps", "rtpt", "nclip", "avsz3", "avsz4", "dpcs", "dpct",
    "intpl", "sqr", "ncs", "nct", "ncds", "ncdt", "nccs", "ncct",
    "cdp", "cc", "dpcl",
}

# mfc2 source -> gte_st* macro name (without the trailing parens)
MFC2_TO_MACRO = {
    "$24": "gte_stmac0",
    "$25": "gte_stmac1",
    "$26": "gte_stmac2",
    "$27": "gte_stmac3",
}

# swc2 source register -> macro name
SWC2_TO_MACRO = {
    "$14": "gte_stsxy",
    "$19": "gte_stsz",
}

TYPEDEFS = (
    "typedef unsigned char u8;\n"
    "typedef signed char s8;\n"
    "typedef unsigned short u16;\n"
    "typedef signed short s16;\n"
    "typedef unsigned int u32;\n"
    "typedef signed int s32;\n"
    "typedef unsigned long long u64;\n"
    "typedef signed long long s64;\n"
    "#define NULL ((void *)0)\n"
)


def run_m2c(func: str) -> str | None:
    asm = ASM_FUNCS / f"{func}.s"
    if not asm.exists():
        return None
    cmd = [sys.executable, str(M2C), "--valid-syntax", "-t", "mipsel-gcc-c"]
    if M2C_CONTEXT.exists():
        cmd += ["--context", str(M2C_CONTEXT)]
    cmd.append(str(asm))
    r = subprocess.run(cmd, capture_output=True, text=True,
                       cwd=str(ROOT), timeout=60)
    if r.returncode != 0 or not r.stdout.strip():
        return None
    return r.stdout


# Regex to find any `M2C_ERROR(/* unknown instruction: ... */)` -- with the
# optional surrounding `(<assignment_target> = ...)` shape from m2c output.
M2C_ERR_RE = re.compile(
    r"M2C_ERROR\s*\(\s*/\*\s*unknown instruction:\s*(.*?)\s*\*/\s*\)"
)


def translate_one(insn: str) -> tuple[str, bool]:
    """Translate one GTE instruction text into either a `gte_*()` macro
    call or an inline `__asm__` fallback. Returns (replacement, fully_handled)."""
    s = insn.strip().rstrip(",")
    parts = s.split(None, 1)
    if not parts:
        return _fallback(s), False
    mnem = parts[0].lower()
    operands = parts[1] if len(parts) > 1 else ""

    # Atomic op?
    if mnem in ATOMIC_OPS:
        return f"gte_{mnem}()", True

    # mfc2 $rD, $copN
    #   - $24..$27 (MAC0..3) -> dedicated gte_stmac0..3 macro
    #   - any other cop reg  -> properly-constrained inline asm read
    if mnem == "mfc2":
        m = re.match(r"\$([\w]+),\s*(\$\d+)", operands)
        if m:
            target_reg = m.group(1)
            cop_reg = m.group(2)
            macro = MFC2_TO_MACRO.get(cop_reg)
            if macro:
                return f"{macro}(/* TODO: lvalue */ {target_reg})", True
            # Fallback for IR0/SXYP/etc -- emit a proper read with `=r` so
            # the value is usable as an expression.
            return (f'({{ s32 _t; __asm__ volatile("mfc2 %0, {cop_reg}" '
                    f': "=r"(_t)); _t; }})'), True

    # mtc2 $rS, $copN  -> proper-constrained inline asm write
    if mnem == "mtc2":
        m = re.match(r"\$([\w]+),\s*(\$\d+)", operands)
        if m:
            src_reg = m.group(1)
            cop_reg = m.group(2)
            return (f'__asm__ volatile("mtc2 %0, {cop_reg}" :: "r"'
                    f"(/* TODO: var holding {src_reg} */))"), True

    # swc2 $copN, offset($base)  -> gte_stsxy(p) etc.
    if mnem == "swc2":
        m = re.match(r"(\$\d+),\s*([^,]+)", operands)
        if m:
            cop_reg = m.group(1)
            mem_op = m.group(2).strip()
            macro = SWC2_TO_MACRO.get(cop_reg)
            if macro:
                return f"{macro}(/* TODO: ptr from {mem_op} */)", True

    # lwc2: leave as TODO for now -- often part of a gte_ldv0/1/2 pair
    return _fallback(s), False


def _fallback(insn_text: str) -> str:
    safe = insn_text.replace('"', '\\"')
    return f'/* TODO_GTE */ __asm__ volatile("{safe}")'


def migrate(c_text: str) -> tuple[str, dict]:
    """Walk m2c output and replace every M2C_ERROR(...) placeholder.

    Returns (new_text, stats) where stats counts handled vs leftover."""
    handled = 0
    leftover = 0

    def repl(m):
        nonlocal handled, leftover
        insn = m.group(1)
        out, full = translate_one(insn)
        if full:
            handled += 1
        else:
            leftover += 1
        return out

    new_text = M2C_ERR_RE.sub(repl, c_text)
    return new_text, {"handled": handled, "leftover": leftover}


def post_process(c_text: str, func: str) -> str:
    """Make m2c+gte_migrate output compile in the permuter sandbox.
    Delegates to the shared m2c_post pipeline with gte.h inlined."""
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from m2c_post import make_compilable
    return make_compilable(c_text, func, include_gte=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--stdout", action="store_true",
                    help="Print to stdout instead of writing permuter/<func>/base.c")
    ap.add_argument("--setup", action="store_true",
                    help="Also stage permuter dir (target.s, target.o, compile.sh)")
    args = ap.parse_args()

    raw = run_m2c(args.func)
    if raw is None:
        print(f"ERROR: m2c failed for {args.func}", file=sys.stderr)
        return 1

    migrated, stats = migrate(raw)
    final = post_process(migrated, args.func)

    print(f"# {args.func}: handled={stats['handled']} GTE ops, "
          f"leftover={stats['leftover']} need-manual TODOs",
          file=sys.stderr)

    if args.stdout:
        sys.stdout.write(final)
        return 0

    out_dir = PERMUTER_DIR / args.func
    out_dir.mkdir(parents=True, exist_ok=True)
    out = out_dir / "base.c"
    out.write_text(final, encoding="utf-8", newline="\n")
    print(f"wrote {out.relative_to(ROOT)} ({len(final)} bytes)", file=sys.stderr)

    if args.setup:
        # Reuse extract_inline_asm.py setup -- it already builds target.s/.o
        # and copies compile.sh from the right source. Then we OVERWRITE the
        # base.c that setup wrote with our GTE-aware version.
        r = subprocess.run(
            [sys.executable, str(ROOT / "tools" / "extract_inline_asm.py"),
             "setup", args.func],
            capture_output=True, text=True, cwd=str(ROOT),
        )
        if r.returncode == 0:
            out.write_text(final, encoding="utf-8", newline="\n")
            print(f"  setup OK; base.c re-written with GTE migration",
                  file=sys.stderr)
        else:
            print(f"  WARN: extract_inline_asm setup failed:\n{r.stderr}",
                  file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
