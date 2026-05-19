#!/usr/bin/env python3
"""cc1psx diagnostic: for a list of tier-3 functions, compile each from a
pure-C permuter base.c with BOTH decompals/mips-gcc-2.7.2 AND Sony's
cc1psx (via dosemu2 wrapper), score both against target bytes.

Answers the question: "is the tier-3 cost primarily compiler-divergence
(cc1psx matches but decompals doesn't) or pure C source weakness (neither
matches)?" The answer determines whether a compiler-patch workstream
would actually retire tier-3 functions or not.

Each function needs a permuter/<func>/base.c that contains the pure-C
candidate (no INLINE_MOVE_ALIASING, no register-asm pins, no scheduling
barriers). The base.c is compiled through the project's normal pipeline
(cpp | cc1 | prologue_fix | maspsx | as) with CC1 swapped between
decompals and cc1psx_wrapper.

For each function, output:
    func           decompals_match  cc1psx_match  category
    func_800483DC  N                Y             COMPILER-FIXABLE
    func_XXX       N                N             C-SOURCE-ISSUE
    func_YYY       Y                Y             ALREADY-MATCHING (shouldn't be tier-3)

A high count of COMPILER-FIXABLE means patching decompals could close
much of the tier-3 gap. A high count of C-SOURCE-ISSUE means most
tier-3 functions need pure-C retry regardless of compiler.

Usage:
    python3 tools/cc1psx_diagnostic.py [func1 func2 ...]
        If no funcs given, auto-pick top N tier-3 functions from the
        classifier output that ALSO have a permuter base.c ready.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PERMUTER_DIR = ROOT / "permuter"

CC1_DECOMPALS = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
CC1_PSX_WRAPPER = ROOT / "tools" / "cc1psx_wrapper.sh"

PROLOGUE_FIX = ROOT / "tools" / "prologue_fix.py"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w".split()
CPP_FLAGS = [
    "-I", str(ROOT / "include"), "-undef", "-Wall", "-lang-c", "-fno-builtin",
    "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips",
    "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ", "-D__EXTENSIONS__",
    "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C", "-DPERMUTER",
]
MASPSX_FLAGS = [
    "--expand-div", "--aspsx-version=2.34",
    "--sdata-syms=sdata_syms.txt",
    "--sdata-funcs=sdata_funcs.txt",
    "--sdata-exclude=sdata_exclude.txt",
    "--expand-lb",
    "--expand-lb-funcs=expand_lb_funcs.txt",
    "--multu-funcs=multu_funcs.txt",
    "--expand-dest-funcs=expand_dest_funcs.txt",
]
AS_FLAGS = ["-I", str(ROOT / "include"), "-march=r3000", "-mtune=r3000",
            "-no-pad-sections", "-O1", "-G0"]


def _shq(s) -> str:
    """Shell-quote a single argument."""
    import shlex
    return shlex.quote(str(s))


def compile_with(cc1_cmd_str: str, base_c: Path, out_o: Path) -> tuple[bool, str]:
    """Compile base.c through full pipeline using cc1_cmd_str as the CC1
    invocation (shell-quoted by caller). Returns (success, error_text).
    Uses a single shell pipeline because Popen-chains with multi-process
    pipes were hitting deadlocks under WSL's mounted filesystem.

    Critical: pass base_c as a path RELATIVE to ROOT (cwd of the bash -c
    invocation). cc1 emits the path into the .file directive verbatim,
    and maspsx parses .file lines via space-split — absolute paths with
    spaces (BB2's "Bushido Blade 2 Decompile" parent dir) make it raise
    `too many values to unpack (expected 3)`."""
    cpp_flags_q = " ".join(_shq(f) for f in CPP_FLAGS)
    cc_flags_q = " ".join(_shq(f) for f in CC_FLAGS)
    maspsx_flags_q = " ".join(_shq(f) for f in MASPSX_FLAGS)
    as_flags_q = " ".join(_shq(f) for f in AS_FLAGS)
    # Relativize base_c against ROOT so the .file directive in cc1's
    # output is space-free.
    try:
        rel_base_c = base_c.resolve().relative_to(ROOT.resolve())
    except ValueError:
        # base_c is outside ROOT — fall back to absolute (will fail in
        # maspsx for paths with spaces, but at least we tried).
        rel_base_c = base_c
    pipeline = (
        f"mipsel-linux-gnu-cpp {cpp_flags_q} {_shq(rel_base_c)} "
        f"| {cc1_cmd_str} {cc_flags_q} "
        f"| python3 {_shq(PROLOGUE_FIX)} "
        f"| python3 {_shq(MASPSX)} {maspsx_flags_q} "
        f"| mipsel-linux-gnu-as {as_flags_q} -o {_shq(out_o)}"
    )
    try:
        cp = subprocess.run(
            ["bash", "-c", "set -o pipefail; " + pipeline],
            capture_output=True, timeout=90, cwd=str(ROOT),
        )
    except subprocess.TimeoutExpired:
        return False, "timeout"
    if cp.returncode != 0:
        err = cp.stderr.decode("utf-8", errors="ignore")[-500:]
        return False, f"rc={cp.returncode}: {err}"
    if not out_o.exists() or out_o.stat().st_size == 0:
        return False, "empty .o"
    return True, ""


def objdump_bytes(o_file: Path, func: str) -> list[int] | None:
    """Extract instruction bytes for `func` from the .o file."""
    try:
        cp = subprocess.run(
            ["mipsel-linux-gnu-objdump", "-d", str(o_file)],
            capture_output=True, text=True, timeout=10)
    except Exception:
        return None
    if cp.returncode != 0:
        return None
    lines = cp.stdout.splitlines()
    in_func = False
    words: list[int] = []
    for line in lines:
        if f"<{func}>:" in line:
            in_func = True
            continue
        if in_func:
            if line.startswith("Disassembly") or (line and not line[0].isspace() and ">:" in line):
                break
            # objdump line: "  abc:   12345678   addu $a0,$a1,$a2"
            parts = line.strip().split(None, 2)
            if len(parts) >= 2 and parts[0].endswith(":"):
                try:
                    words.append(int(parts[1], 16))
                except ValueError:
                    pass
    return words if words else None


def target_bytes(func: str) -> list[int] | None:
    """Extract target instruction bytes for `func` directly from
    disc/SLUS_006.63 using the file offset in asm/funcs/<func>.s.
    Format of first line: `glabel func_NAME` then per-insn lines:
        /* FILE_OFFSET VADDR HEX_BYTES */ instr
    We grab FILE_OFFSET from the first instruction line."""
    p = ROOT / "asm" / "funcs" / f"{func}.s"
    if not p.exists():
        return None
    # Count instructions; grab first file offset.
    import re as _re
    first_offset = None
    insns = 0
    for line in p.read_text(encoding="utf-8", errors="ignore").splitlines():
        m = _re.match(r'\s*/\*\s*([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s*\*/', line)
        if m:
            insns += 1
            if first_offset is None:
                first_offset = int(m.group(1), 16)
    if first_offset is None or insns == 0:
        return None
    disc = ROOT / "disc" / "SLUS_006.63"
    if not disc.exists():
        return None
    nbytes = insns * 4
    raw = disc.read_bytes()[first_offset:first_offset + nbytes]
    if len(raw) != nbytes:
        return None
    # PSX is little-endian, but we want raw 32-bit words for comparison.
    words = []
    for i in range(0, nbytes, 4):
        words.append(int.from_bytes(raw[i:i+4], "little"))
    return words


def diagnose(func: str) -> dict:
    """Run the diagnostic for one function."""
    pdir = PERMUTER_DIR / func
    base_c = pdir / "base.c"
    if not base_c.exists():
        return {"func": func, "status": "no_permuter_base"}

    target = target_bytes(func)
    if not target:
        return {"func": func, "status": "no_target_bytes"}

    with tempfile.TemporaryDirectory() as td:
        td_path = Path(td)
        # decompals
        decompals_o = td_path / "decompals.o"
        d_ok, d_err = compile_with(_shq(CC1_DECOMPALS), base_c, decompals_o)
        decompals_bytes = objdump_bytes(decompals_o, func) if d_ok else None

        # cc1psx
        cc1psx_o = td_path / "cc1psx.o"
        p_ok, p_err = compile_with(f"bash {_shq(CC1_PSX_WRAPPER)}", base_c, cc1psx_o)
        cc1psx_bytes = objdump_bytes(cc1psx_o, func) if p_ok else None

    def score(mine: list[int] | None, tgt: list[int]) -> str:
        if mine is None:
            return "compile_failed"
        if len(mine) != len(tgt):
            return f"len_mismatch ({len(mine)} vs {len(tgt)})"
        diff = sum(1 for a, b in zip(mine, tgt) if a != b)
        if diff == 0:
            return "MATCH"
        return f"diff:{diff}/{len(tgt)}"

    d_score = score(decompals_bytes, target)
    p_score = score(cc1psx_bytes, target)

    # Category
    if d_score == "MATCH" and p_score == "MATCH":
        cat = "BOTH_MATCH"
    elif d_score != "MATCH" and p_score == "MATCH":
        cat = "COMPILER_FIXABLE"
    elif d_score == "MATCH" and p_score != "MATCH":
        cat = "DECOMPALS_ONLY"
    else:
        cat = "C_SOURCE_ISSUE"

    return {
        "func": func, "status": "ok",
        "target_insns": len(target),
        "decompals_score": d_score,
        "cc1psx_score": p_score,
        "decompals_err": d_err if not d_ok else "",
        "cc1psx_err": p_err if not p_ok else "",
        "category": cat,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("funcs", nargs="*", help="function name(s); auto-pick if empty")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    funcs = args.funcs
    if not funcs:
        # Auto-pick: find permuter dirs with base.c and pick those that
        # are also in the tier-3 list (from classify_inline_asm).
        candidates = sorted(p.name for p in PERMUTER_DIR.iterdir()
                            if p.is_dir() and (p / "base.c").exists())
        funcs = candidates[:5]
        print(f"No funcs given — auto-picked first 5 with permuter base.c:", file=sys.stderr)
        for f in funcs:
            print(f"  {f}", file=sys.stderr)
        if not funcs:
            print("ERROR: no permuter dirs with base.c found", file=sys.stderr)
            return 1

    results = []
    for func in funcs:
        print(f"--- {func} ---", file=sys.stderr)
        r = diagnose(func)
        results.append(r)
        if r["status"] != "ok":
            print(f"  SKIP: {r['status']}", file=sys.stderr)
        else:
            print(f"  decompals: {r['decompals_score']:30s}  cc1psx: {r['cc1psx_score']:30s}  -> {r['category']}",
                  file=sys.stderr)

    if args.json:
        print(json.dumps(results, indent=2))
        return 0

    # Summary
    cats = {"COMPILER_FIXABLE": 0, "C_SOURCE_ISSUE": 0, "BOTH_MATCH": 0,
            "DECOMPALS_ONLY": 0, "SKIPPED": 0}
    for r in results:
        if r["status"] != "ok":
            cats["SKIPPED"] += 1
        else:
            cats[r["category"]] += 1

    print()
    print("=== summary ===")
    print(f"  COMPILER_FIXABLE (cc1psx matches, decompals doesn't): {cats['COMPILER_FIXABLE']}")
    print(f"  C_SOURCE_ISSUE   (neither compiler matches):          {cats['C_SOURCE_ISSUE']}")
    print(f"  BOTH_MATCH       (already matching, not tier-3):      {cats['BOTH_MATCH']}")
    print(f"  DECOMPALS_ONLY   (decompals matches but cc1psx doesn't): {cats['DECOMPALS_ONLY']}")
    print(f"  SKIPPED          (no permuter base / no target):      {cats['SKIPPED']}")
    print()
    ok_count = sum(1 for r in results if r["status"] == "ok")
    if ok_count > 0:
        compiler_fix_pct = 100 * cats["COMPILER_FIXABLE"] / ok_count
        print(f"Compiler-patch potential: {compiler_fix_pct:.0f}% of analyzed functions "
              f"could be retired by a perfect decompals→cc1psx calibration patch.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
