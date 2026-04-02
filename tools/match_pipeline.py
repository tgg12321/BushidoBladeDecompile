#!/usr/bin/env python3
"""Mechanical matching pipeline — runs all free/programmatic steps for a function.

Chains: permuter setup → m2c → compile/score → regasm_explorer → permuter.
Outputs a structured report with everything a model agent needs to continue.

Usage (in WSL):
    source .venv/bin/activate
    python3 tools/match_pipeline.py func_80043C7C
    python3 tools/match_pipeline.py func_80043C7C --permuter-time 120
    python3 tools/match_pipeline.py func_80043C7C --skip-permuter
    python3 tools/match_pipeline.py func_80043C7C --json

Exit codes: 0 = matched, 1 = not matched (report on stdout), 2 = error
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_DIR = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
PERMUTER_DIR = ROOT / "permuter"
M2C_SCRIPT = ROOT / "tools" / "m2c" / "m2c.py"
M2C_CONTEXT = ROOT / "include" / "m2c_context.h"

TYPEDEFS = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
"""


def log(msg):
    print(msg, file=sys.stderr, flush=True)


def find_src_file(func_name):
    """Find which src/*.c file contains this function as an INCLUDE_ASM stub."""
    for src in sorted(SRC_DIR.glob("*.c")):
        with open(src) as f:
            if f'INCLUDE_ASM("asm/funcs", {func_name})' in f.read():
                return str(src.relative_to(ROOT))
    return None


def count_asm_instructions(func_name):
    """Count non-directive lines in the target asm."""
    asm_file = ASM_DIR / f"{func_name}.s"
    if not asm_file.exists():
        return 0
    count = 0
    with open(asm_file) as f:
        for line in f:
            s = line.strip()
            if s and not s.startswith(".") and not s.startswith("#") \
               and "glabel" not in s and "endlabel" not in s:
                count += 1
    return count


def is_leaf(func_name):
    """Check if function has no jal (function call) instructions."""
    asm_file = ASM_DIR / f"{func_name}.s"
    if not asm_file.exists():
        return False
    with open(asm_file) as f:
        return "jal" not in f.read()


def run_cmd(cmd, timeout=120):
    """Run a bash command, return (returncode, stdout, stderr)."""
    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True,
        timeout=timeout, cwd=str(ROOT),
    )
    return result.returncode, result.stdout, result.stderr


def setup_permuter(func_name, src_file):
    """Set up permuter directory. Returns True on success."""
    pdir = PERMUTER_DIR / func_name
    if (pdir / "target.o").exists() and (pdir / "compile.sh").exists():
        log(f"  Permuter dir already exists")
        return True

    log(f"  Running permuter_setup.sh...")
    rc, out, err = run_cmd(
        f"source .venv/bin/activate && "
        f"bash tools/permuter_setup.sh {func_name} {src_file} 2>&1",
        timeout=60,
    )
    ok = (pdir / "target.o").exists()
    if not ok:
        log(f"  FAILED: {out[-200:]}")
    return ok


def run_m2c(func_name):
    """Run m2c to generate initial C. Returns source string or None."""
    asm_file = ASM_DIR / f"{func_name}.s"
    if not M2C_SCRIPT.exists() or not asm_file.exists():
        return None

    cmd = f"python3 {M2C_SCRIPT} --target mipsel --valid-syntax"
    if M2C_CONTEXT.exists():
        cmd += f" --context {M2C_CONTEXT}"
    cmd += f" {asm_file}"

    rc, out, err = run_cmd(cmd, timeout=60)
    if rc == 0 and out.strip():
        return out.strip()
    return None


def ensure_base_c(func_name):
    """Make sure base.c exists with something compilable. Returns True if ready."""
    base_c = PERMUTER_DIR / func_name / "base.c"

    # Already has content?
    if base_c.exists() and base_c.stat().st_size > 100:
        log(f"  base.c exists ({base_c.stat().st_size} bytes)")
        return True

    # Try m2c
    log(f"  Running m2c for initial C...")
    m2c_out = run_m2c(func_name)
    if m2c_out:
        # m2c --valid-syntax includes its own typedefs, should compile standalone
        with open(base_c, "w", newline="\n") as f:
            f.write(m2c_out + "\n")
        log(f"  m2c generated {len(m2c_out)} chars")
        return True

    # Fallback: minimal stub
    log(f"  m2c failed, writing minimal stub")
    with open(base_c, "w", newline="\n") as f:
        f.write(TYPEDEFS + f"\nvoid {func_name}(void) {{\n}}\n")
    return True


def compile_and_score(func_name, source=None):
    """Compile and score via try_match.sh. Returns (score, full_output) or (None, error)."""
    args = func_name
    if source:
        args += f" {source}"
    rc, out, err = run_cmd(
        f"source .venv/bin/activate && bash tools/try_match.sh {args} 2>&1",
        timeout=60,
    )
    # Parse score from output
    m = re.search(r"Score:\s*(\d+)", out)
    if m:
        return int(m.group(1)), out
    return None, out + err


def run_regasm_explorer(func_name, jobs=4):
    """Run regasm_explorer. Returns (best_score, best_label, full_output)."""
    log(f"  Running regasm_explorer -j{jobs}...")
    rc, out, err = run_cmd(
        f"source .venv/bin/activate && "
        f"python3 tools/regasm_explorer.py {func_name} -j{jobs} --top 5 2>&1",
        timeout=600,
    )
    # Parse "Best: score N"
    m = re.search(r"Best:\s*score\s*(\d+)\s*.\s*(.*)", out)
    if m:
        return int(m.group(1)), m.group(2).strip(), out
    return None, "", out


def run_permuter(func_name, timeout=60, jobs=4):
    """Run permuter for N seconds. Returns best score found or None."""
    log(f"  Running permuter for {timeout}s with -j{jobs}...")
    rc, out, err = run_cmd(
        f"source .venv/bin/activate && "
        f"timeout {timeout} python3 tools/decomp-permuter/permuter.py "
        f"permuter/{func_name}/ -j{jobs} --stop-on-zero 2>&1 | tail -15",
        timeout=timeout + 30,
    )
    if "score 0" in out.lower() or "Stopped" in out:
        return 0
    # Parse best score
    best = None
    for line in out.splitlines():
        m = re.search(r"score\s+(\d+)", line, re.I)
        if m:
            s = int(m.group(1))
            if best is None or s < best:
                best = s
    return best


def has_regasm(func_name):
    """Check if base.c contains register asm declarations."""
    base_c = PERMUTER_DIR / func_name / "base.c"
    if not base_c.exists():
        return False
    with open(base_c) as f:
        return bool(re.search(r'register\s+.*asm\s*\(', f.read()))


def count_regasm(func_name):
    """Count register asm declarations in base.c."""
    base_c = PERMUTER_DIR / func_name / "base.c"
    if not base_c.exists():
        return 0
    with open(base_c) as f:
        return len(re.findall(r'register\s+.*asm\s*\(', f.read()))


def read_target_asm(func_name):
    """Read the target assembly."""
    asm_file = ASM_DIR / f"{func_name}.s"
    if not asm_file.exists():
        return ""
    with open(asm_file) as f:
        return f.read()


def read_base_c(func_name):
    """Read current base.c."""
    base_c = PERMUTER_DIR / func_name / "base.c"
    if not base_c.exists():
        return ""
    with open(base_c) as f:
        return f.read()


def main():
    parser = argparse.ArgumentParser(description="Mechanical matching pipeline")
    parser.add_argument("func_name", help="Function to match (e.g. func_80043C7C)")
    parser.add_argument("--skip-permuter", action="store_true", help="Skip permuter run")
    parser.add_argument("--skip-regasm", action="store_true", help="Skip regasm_explorer")
    parser.add_argument("--permuter-time", type=int, default=60, help="Permuter timeout (default: 60)")
    parser.add_argument("--src-file", help="Source file (auto-detected if omitted)")
    parser.add_argument("--json", action="store_true", help="Output JSON report to stdout")
    parser.add_argument("-j", "--jobs", type=int, default=4, help="Parallel jobs (default: 4)")
    args = parser.parse_args()

    func = args.func_name
    report = {
        "func": func,
        "steps": [],
        "score": None,
        "recommendation": None,
        "src_file": None,
        "insn_count": 0,
        "leaf": False,
    }

    # --- Step 1: Find source file ---
    src_file = args.src_file or find_src_file(func)
    if not src_file:
        log(f"ERROR: {func} not found as INCLUDE_ASM stub in any src/*.c file")
        sys.exit(2)
    report["src_file"] = src_file
    insn_count = count_asm_instructions(func)
    leaf = is_leaf(func)
    report["insn_count"] = insn_count
    report["leaf"] = leaf
    log(f"=== {func} === {src_file} | {insn_count} insns | leaf={leaf}")

    # --- Step 2: Setup permuter ---
    log("\n[1/5] Setup")
    if not setup_permuter(func, src_file):
        log("FATAL: permuter setup failed")
        sys.exit(2)
    report["steps"].append("setup")

    # --- Step 3: Ensure base.c ---
    log("\n[2/5] Base C")
    ensure_base_c(func)
    report["steps"].append("base_c")

    # --- Step 4: Initial score ---
    log("\n[3/5] Initial score")
    score, diff_output = compile_and_score(func)
    if score is None:
        log(f"  Compile failed — base.c needs fixes")
        report["score"] = None
        report["recommendation"] = "fix_compile"
        report["error"] = diff_output[-500:]
    else:
        log(f"  Score: {score}")
        report["score"] = score
        if score == 0:
            log("\n*** ALREADY MATCHED ***")
            report["recommendation"] = "matched"
            output_report(report, args.json)
            sys.exit(0)
    report["steps"].append(f"score:{score}")

    # --- Step 5: Regasm explorer ---
    if not args.skip_regasm and score is not None and score > 0:
        n_regasm = count_regasm(func)
        if n_regasm > 0 and n_regasm <= 12:  # 2^12 = 4096 max combos
            log(f"\n[4/5] Regasm explorer ({n_regasm} vars, {2**n_regasm} combos)")
            best_score, best_label, regasm_out = run_regasm_explorer(func, jobs=args.jobs)
            if best_score is not None:
                log(f"  Best: score {best_score} — {best_label}")
                report["steps"].append(f"regasm:{best_score}")
                if best_score == 0:
                    log("\n*** MATCHED via regasm_explorer! ***")
                    report["score"] = 0
                    report["recommendation"] = "matched_regasm"
                    output_report(report, args.json)
                    sys.exit(0)
                if best_score < score:
                    # Adopt the winner
                    winner = PERMUTER_DIR / func / "winner.c"
                    base_c = PERMUTER_DIR / func / "base.c"
                    if winner.exists():
                        shutil.copy(str(winner), str(base_c))
                        log(f"  Adopted regasm winner (score {score} → {best_score})")
                        score = best_score
                        report["score"] = score
        elif n_regasm > 12:
            log(f"\n[4/5] Regasm explorer — SKIPPED ({n_regasm} vars = {2**n_regasm} combos, too many)")
            report["steps"].append("regasm:skipped_too_many")
        else:
            log(f"\n[4/5] Regasm explorer — SKIPPED (no register asm in base.c)")
            report["steps"].append("regasm:skipped_none")
    else:
        log(f"\n[4/5] Regasm explorer — SKIPPED")
        report["steps"].append("regasm:skipped")

    # --- Step 6: Permuter ---
    if not args.skip_permuter and score is not None and score > 0:
        log(f"\n[5/5] Permuter ({args.permuter_time}s)")
        perm_score = run_permuter(func, timeout=args.permuter_time, jobs=args.jobs)
        if perm_score is not None:
            log(f"  Permuter best: {perm_score}")
            report["steps"].append(f"permuter:{perm_score}")
            if perm_score == 0:
                log("\n*** MATCHED via permuter! ***")
                report["score"] = 0
                report["recommendation"] = "matched_permuter"
                output_report(report, args.json)
                sys.exit(0)
            if perm_score < score:
                score = perm_score
                report["score"] = score
        else:
            log(f"  Permuter returned no score")
            report["steps"].append("permuter:no_score")
    else:
        log(f"\n[5/5] Permuter — SKIPPED")
        report["steps"].append("permuter:skipped")

    # --- Final: Re-score and recommend ---
    log("\n--- Final state ---")
    final_score, final_diff = compile_and_score(func)
    if final_score is not None:
        report["score"] = final_score
        score = final_score
    log(f"  Final score: {score}")

    if score is None:
        report["recommendation"] = "fix_compile"
        tier = "haiku"
    elif score == 0:
        report["recommendation"] = "matched"
        tier = "done"
    elif score <= 50:
        report["recommendation"] = "needs_haiku"
        tier = "haiku"
    elif score <= 300:
        report["recommendation"] = "needs_sonnet"
        tier = "sonnet"
    else:
        report["recommendation"] = "needs_opus"
        tier = "opus"

    report["tier"] = tier
    log(f"  Recommendation: {report['recommendation']} (tier: {tier})")

    output_report(report, args.json)
    sys.exit(0 if score == 0 else 1)


def output_report(report, as_json):
    """Print report to stdout."""
    if as_json:
        print(json.dumps(report, indent=2))
    else:
        print(f"\n{'='*60}")
        print(f"Function:       {report['func']}")
        print(f"Source:         {report.get('src_file', '?')}")
        print(f"Instructions:   {report.get('insn_count', '?')}")
        print(f"Leaf:           {report.get('leaf', '?')}")
        print(f"Score:          {report.get('score', 'N/A')}")
        print(f"Steps:          {' → '.join(report.get('steps', []))}")
        print(f"Recommendation: {report.get('recommendation', '?')}")
        print(f"Tier:           {report.get('tier', '?')}")
        print(f"{'='*60}")


if __name__ == "__main__":
    main()
