#!/usr/bin/env python3
"""Re-score all DeepSeek drafts by compiling and diffing objdump output.

Doesn't rely on the permuter's C parser (which rejects C99-style mixed decls).
Instead:
  1. Compiles the draft via permuter_compile.sh
  2. Assembles the target .s
  3. Counts differing instruction words between the two .o files

Score = number of differing 32-bit instruction words (0 = exact match).

Runs from Git Bash (Windows) — uses WSL subprocess for compilation.

Usage:
    python tools/rescore_drafts.py
    python tools/rescore_drafts.py --limit 20
    python tools/rescore_drafts.py --func func_8003D330   # single function
"""
import argparse
import json
import os
import re
import subprocess
import sys

DRAFTS_DIR   = os.path.join("local_drafts", "bb2-deepseek")
ASM_DIR      = os.path.join("asm", "funcs")
PERMUTER_DIR = os.path.join("permuter")
WSL_ROOT     = '/mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"'
WSL_VENV     = "source .venv/bin/activate"

def make_wrapped(code, externs=""):
    parts = ['#include "common.h"', '#include "include_asm.h"', '']
    if externs:
        parts.append(externs)
        parts.append('')
    parts.append(code)
    parts.append('')
    return '\n'.join(parts)

OBJDUMP_CMD = "mipsel-linux-gnu-objdump -d --no-show-raw-insn"


def extract_symbols_from_asm(func_name):
    """Extract symbol references from the .s file for a function. Returns extern decl string."""
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        return ""

    symbols = set()
    try:
        with open(asm_path) as f:
            for line in f:
                # Extract %hi(SYMBOL) and %lo(SYMBOL) patterns
                for m in re.finditer(r'%(?:hi|lo|gp_rel)\(([A-Za-z_][A-Za-z0-9_]*)\)', line):
                    symbols.add(m.group(1))
    except Exception:
        pass

    # Build extern declarations for data symbols (those starting with D_)
    externs = []
    for sym in sorted(symbols):
        if sym.startswith('D_'):
            externs.append(f"extern u32 {sym};")
        elif sym.startswith('func_'):
            externs.append(f"extern void {sym}(void);")

    return "\n".join(externs)


def compile_draft(func_name, code):
    """Compile draft via permuter_compile.sh. Returns path to .o or None."""
    externs = extract_symbols_from_asm(func_name)
    wrapped = make_wrapped(code, externs)
    out_o = f"/tmp/bb2_score_{func_name}.o"
    # Write wrapped code to a local file, then compile from WSL
    local_src = f"/tmp/bb2_score_{func_name}.c"
    try:
        with open(local_src, "w") as f:
            f.write(wrapped)
    except Exception:
        return None

    # Convert Windows path to WSL path
    wsl_src = f"/mnt/c{local_src.replace(chr(92), '/')}"
    if local_src.startswith("C:") or local_src.startswith("c:"):
        wsl_src = f"/mnt/c/{local_src[3:].replace(chr(92), '/')}"

    wsl_cmd = (
        f'cd {WSL_ROOT} && '
        f'{WSL_VENV} && '
        f'bash tools/permuter_compile.sh "{wsl_src}" -o {out_o} 2>&1'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=30
        )
        # Check file was created
        check = subprocess.run(
            ["wsl", "bash", "-lc", f"test -f {out_o} && echo EXISTS"],
            capture_output=True, text=True, timeout=10
        )
        if "EXISTS" in check.stdout:
            return out_o
    except Exception:
        pass
    return None


def assemble_target(func_name):
    """Return path to target .o. Uses permuter's pre-built target.o if available."""
    # Fast path: permuter already assembled it
    perm_target = os.path.join(PERMUTER_DIR, func_name, "target.o")
    if os.path.exists(perm_target):
        return f"/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/{perm_target}".replace("\\", "/")

    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        return None

    # Read and clean the assembly (strip glabel/endlabel which are ASPSX-specific)
    try:
        with open(asm_path) as f:
            asm_content = f.read()
        # Strip glabel/endlabel directives (not understood by GNU as)
        asm_content = re.sub(r'^glabel\s+\w+\s*$', '', asm_content, flags=re.MULTILINE)
        asm_content = re.sub(r'^endlabel\s+\w+\s*$', '', asm_content, flags=re.MULTILINE)
    except Exception:
        return None

    # Write cleaned assembly to a temp file
    local_asm = f"/tmp/bb2_target_{func_name}.s"
    try:
        with open(local_asm, "w") as f:
            f.write(asm_content)
    except Exception:
        return None

    # /tmp in Git Bash is already a WSL-compatible path
    wsl_asm_path = local_asm

    out_o = f"/tmp/bb2_target_{func_name}.o"
    wsl_cmd = (
        f'mipsel-linux-gnu-as -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 '
        f'"{wsl_asm_path}" -o {out_o} 2>&1'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=30
        )
        check = subprocess.run(
            ["wsl", "bash", "-lc", f"test -f {out_o} && echo EXISTS"],
            capture_output=True, text=True, timeout=10
        )
        if "EXISTS" in check.stdout:
            return out_o
    except Exception:
        pass
    return None


def get_insn_hex(obj_path):
    """Return list of raw instruction hex strings from objdump -d."""
    wsl_cmd = f'mipsel-linux-gnu-objdump -d "{obj_path}" 2>/dev/null'
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=15
        )
        words = []
        for line in result.stdout.splitlines():
            # Format: "   0:  3c050a80   lui  a1,0xa80"
            m = re.match(r'^\s+[0-9a-f]+:\s+([0-9a-f]{8})\s', line)
            if m:
                words.append(m.group(1))
        return words
    except Exception:
        return []


def score_function(func_name, code):
    """Compile draft + target, return (score, ours_len, target_len) or None."""
    draft_o  = compile_draft(func_name, code)
    if not draft_o:
        return None, "compile_fail"

    target_o = assemble_target(func_name)
    if not target_o:
        return None, "no_target"

    ours   = get_insn_hex(draft_o)
    target = get_insn_hex(target_o)

    if not ours or not target:
        return None, "no_insns"

    # Count differing words (pad shorter with None)
    diffs = 0
    max_len = max(len(ours), len(target))
    for i in range(max_len):
        o = ours[i]   if i < len(ours)   else None
        t = target[i] if i < len(target) else None
        # Skip nops from delay slots / padding — 0x00000000 mismatches are noise
        if o != t:
            diffs += 1

    return diffs, f"{len(ours)}vs{len(target)}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--func", type=str, default="", help="Score a single function")
    args = parser.parse_args()

    if args.func:
        draft_path = os.path.join(DRAFTS_DIR, f"{args.func}.c")
        if not os.path.exists(draft_path):
            print(f"No draft for {args.func}")
            sys.exit(1)
        with open(draft_path) as f:
            code = f.read().strip()
        score, info = score_function(args.func, code)
        print(f"{args.func}: score={score} ({info})")
        return

    if not os.path.isdir(DRAFTS_DIR):
        print(f"ERROR: {DRAFTS_DIR} not found")
        sys.exit(1)

    drafts = sorted(f for f in os.listdir(DRAFTS_DIR) if f.endswith(".c"))
    if args.limit:
        drafts = drafts[:args.limit]

    print(f"Scoring {len(drafts)} drafts...\n")

    results     = []
    compile_failures = []
    other_failures   = []

    for i, fname in enumerate(drafts, 1):
        func_name = fname[:-2]
        with open(os.path.join(DRAFTS_DIR, fname)) as f:
            code = f.read().strip()

        print(f"[{i}/{len(drafts)}] {func_name}...", end=" ", flush=True)
        score, info = score_function(func_name, code)

        if score is None:
            print(f"FAIL ({info})")
            if info == "compile_fail":
                compile_failures.append(func_name)
            else:
                other_failures.append((func_name, info))
        elif score == 0:
            print(f"MATCH! score=0 ({info})")
            results.append((func_name, 0))
        else:
            print(f"score={score} ({info})")
            results.append((func_name, score))

    # Summary
    print("\n" + "="*60)
    print("RESULTS SUMMARY")
    print("="*60)
    print(f"  Scored:           {len(results)}")
    print(f"  Compile failures: {len(compile_failures)}")
    print(f"  Other failures:   {len(other_failures)}")

    if results:
        exact = [r for r in results if r[1] == 0]
        near  = sorted([r for r in results if 0 < r[1] <= 20], key=lambda x: x[1])
        mid   = sorted([r for r in results if 20 < r[1] <= 100], key=lambda x: x[1])
        far   = sorted([r for r in results if r[1] > 100], key=lambda x: x[1])

        print(f"\n  Exact matches (score=0): {len(exact)}")
        for fn, s in exact:
            print(f"    {fn}")

        print(f"\n  Near-misses (score 1-20): {len(near)}")
        for fn, s in near:
            print(f"    score={s:4d}  {fn}")

        print(f"\n  Medium (score 21-100): {len(mid)}")
        for fn, s in mid[:15]:
            print(f"    score={s:4d}  {fn}")
        if len(mid) > 15:
            print(f"    ... and {len(mid)-15} more")

        print(f"\n  Far off (score >100): {len(far)}")
        for fn, s in far[:10]:
            print(f"    score={s:4d}  {fn}")
        if len(far) > 10:
            print(f"    ... and {len(far)-10} more")

        scored_vals = [s for _, s in results]
        print(f"\n  Average score: {sum(scored_vals)/len(scored_vals):.1f}")
        print(f"  Best score:    {min(scored_vals)}")
        print(f"  Median score:  {sorted(scored_vals)[len(scored_vals)//2]}")


if __name__ == "__main__":
    main()
