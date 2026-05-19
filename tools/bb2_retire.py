#!/usr/bin/env python3
"""bb2_retire.py -- end-to-end safe retirement of a single regfix rule.

Runs the full workflow as one command:
  1. Locate the function's src/*.c
  2. Set up permuter/<func>/ if not already (via bb2_setup_backlog.sh)
  3. Triage: measure base score (25s) -- abort if base > MAX_BASE
  4. Run targeted permuter (time budget scales with base score)
  5. If score=0 found:
       - Apply match to src (via bb2_apply_match.py --verify)
       - Run make, check SHA1
       - If SHA1 mismatches: rollback and abort
       - If --commit given: write a conventional commit
  6. If no match: report best score reached

Usage:
  python3 tools/bb2_retire.py <func> [--src=<path>] [--time=<seconds>]
                              [--max-base=<score>] [--commit]

Safe to interrupt: changes are only persisted after SHA1 verification.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
import time
from pathlib import Path


def find_src_file(func: str) -> Path | None:
    """Locate src/<file>.c containing the function definition."""
    pat = re.compile(
        rf"^\s*(?:static\s+)?\w[\w\s\*]+?\s\*?{re.escape(func)}\s*\("
    )
    for sf in Path("src").glob("*.c"):
        try:
            text = sf.read_bytes().decode("utf-8", errors="ignore")
        except Exception:
            continue
        for line in text.split("\n"):
            if pat.match(line) and not line.rstrip().endswith(";"):
                return sf
    return None


def setup_permuter_dir(func: str, src: Path) -> bool:
    """Run bb2_setup_backlog.sh and check for success."""
    permdir = Path(f"permuter/{func}")
    if (permdir / "base.c").exists() and (permdir / "target.o").exists():
        print(f"[setup] permuter/{func}/ exists; skipping setup")
        return True
    print(f"[setup] preparing permuter/{func}/...")
    root = Path.cwd().resolve()
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    cmd = (
        f'cd "{wsl_root}" && bash tools/bb2_setup_backlog.sh "{func}" "{src.as_posix()}"'
    )
    r = subprocess.run(["wsl", "bash", "-c", cmd],
                       capture_output=True, text=True, timeout=120)
    out = (r.stdout or "") + (r.stderr or "")
    if "OK    " + func in out:
        return True
    print(f"[setup] FAILED: {out.strip()}")
    return False


def measure_base_score(func: str) -> int | None:
    """25s quick base-score measurement. Returns int or None on failure."""
    root = Path.cwd().resolve()
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'timeout 25 python3 tools/bb2_permuter.py permuter/{func} '
        f'--best-only -j 1 2>&1 | tr "\\r" "\\n"'
    )
    r = subprocess.run(["wsl", "bash", "-c", cmd],
                       capture_output=True, text=True, timeout=40)
    out = r.stdout
    m = re.search(r"base score = (\d+)", out)
    if m:
        return int(m.group(1))
    return None


def run_permuter(func: str, time_s: int) -> tuple[int | None, int]:
    """Run permuter with time budget; return (min_score, max_iters)."""
    root = Path.cwd().resolve()
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    # Clear prior outputs for a fresh search
    for d in Path(f"permuter/{func}").glob("output-*"):
        if d.is_dir():
            import shutil
            shutil.rmtree(d)
    log = f"/tmp/bb2_retire_{func}.log"
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'BB2_PERMUTER_HEAVY=1 timeout {time_s} python3 tools/bb2_permuter.py '
        f'permuter/{func} --stop-on-zero --best-only -j 6 > {log} 2>&1'
    )
    print(f"[permuter] running {time_s}s on permuter/{func}...")
    subprocess.run(["wsl", "bash", "-c", cmd], capture_output=True, timeout=time_s + 30)
    # Parse the log for scores
    parse_cmd = (
        f'tr "\\r" "\\n" < {log} | '
        f'grep -oE "score = [0-9]+" | awk \'{{print $NF}}\' | sort -n | head -1'
    )
    r = subprocess.run(["wsl", "bash", "-c", parse_cmd],
                       capture_output=True, text=True, timeout=30)
    min_score = int(r.stdout.strip()) if r.stdout.strip().isdigit() else None
    iter_cmd = (
        f'tr "\\r" "\\n" < {log} | '
        f'grep -oE "iteration [0-9]+" | awk \'{{print $NF}}\' | sort -n | tail -1'
    )
    r2 = subprocess.run(["wsl", "bash", "-c", iter_cmd],
                        capture_output=True, text=True, timeout=30)
    max_iter = int(r2.stdout.strip()) if r2.stdout.strip().isdigit() else 0
    return min_score, max_iter


def run_diag_diff(func: str) -> None:
    """Invoke bb2_diag_diff.py on the best output-* dir, print to our
    stdout. Used when the permuter plateaus (min_score > 0) so agents
    see what's still wrong instead of just a number.
    """
    root = Path.cwd().resolve()
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'python3 tools/bb2_diag_diff.py {func} 2>&1'
    )
    r = subprocess.run(["wsl", "bash", "-c", cmd],
                       capture_output=True, text=True, timeout=60)
    print(r.stdout or r.stderr)


def apply_match(func: str, src: Path) -> bool:
    """Apply via bb2_apply_match.py --verify (which auto-rebuilds + checks SHA1)."""
    root = Path.cwd().resolve()
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'python3 tools/bb2_apply_match.py {func} {src.as_posix()} '
        f'--retire-regfix 2>&1'
    )
    print(f"[apply] applying match (with verify+rollback)...")
    r = subprocess.run(["wsl", "bash", "-c", cmd],
                       capture_output=True, text=True, timeout=360)
    out = r.stdout
    print(out)
    return r.returncode == 0 and "SHA1 verified" in out


def make_commit(func: str, src: Path, base_score: int, best_score: int,
                iters: int) -> None:
    """Auto-commit the retirement."""
    msg = f"""Backlog: retire {func} regfix via targeted permuter

Match found by targeted permuter at iteration {iters} (base score {base_score}).
Build SHA1 still matches after applying.

Applied via bb2_retire.py (auto-verify + rollback safety).

Co-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>
"""
    cmd_add = ["git", "add", str(src), "regfix.txt"]
    cmd_commit = ["git", "commit", "-m", msg]
    subprocess.run(cmd_add, check=True)
    r = subprocess.run(cmd_commit, capture_output=True, text=True)
    if r.returncode != 0:
        print(f"[commit] FAILED: {r.stderr}")
        return
    print(f"[commit] OK")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("func")
    p.add_argument("--src", help="Override auto-detected src file")
    p.add_argument("--time", type=int, default=300, help="Permuter time budget (s)")
    p.add_argument("--max-base", type=int, default=300,
                   help="Abort if base score exceeds this (avoid burning cycles)")
    p.add_argument("--commit", action="store_true",
                   help="Auto-commit on successful retirement")
    args = p.parse_args()

    print(f"=== Retiring {args.func} ===")

    if args.src:
        src = Path(args.src)
    else:
        src = find_src_file(args.func)
        if src is None:
            sys.exit(f"Could not locate src file for {args.func}; use --src")
    print(f"[src] {src}")

    if not setup_permuter_dir(args.func, src):
        sys.exit("Setup failed; cannot proceed")

    base = measure_base_score(args.func)
    if base is None:
        sys.exit("Base score measurement failed (likely compile error)")
    print(f"[base] score = {base}")
    if base > args.max_base:
        sys.exit(f"Base score {base} > max {args.max_base}; aborting")
    if base == 0:
        print(f"[match] base already 0 -- function builds without regfix; "
              f"check whether regfix is dead code")

    if base > 0:
        # Scale time budget inversely with base score for triage efficiency
        time_budget = args.time if base <= 150 else max(args.time // 2, 120)
        min_score, max_iter = run_permuter(args.func, time_budget)
        print(f"[result] min_score={min_score} iters={max_iter}")
        if min_score != 0:
            # Auto-diag: show the agent what's left in the diff
            print()
            print(f"[diag] no match (plateau at {min_score}). Running asm-level diff "
                  f"on best output dir to show what remains:")
            run_diag_diff(args.func)
            print()
            print(f"[diag] hint: structure-of-remaining-diff above suggests the "
                  f"plateau cause. Common patterns:")
            print(f"   - uniform $regA <-> $regB swaps -> pin a variable to one of them")
            print(f"   - addressing-mode diffs (e.g., 'sh ra,8(rb)' vs 'sh ra,K(zero)') "
                  f"-> try alternate expression for the memory access")
            print(f"   - 1-2 instruction reorder -> try `do {{ ... }} while (0)` "
                  f"around adjacent statements")
            print(f"   - mfhi/division intermediate -> typically unfixable from C; "
                  f"keep the regfix rule")
            sys.exit(0)  # no match, that's fine; not an error

    # Match found
    if not apply_match(args.func, src):
        sys.exit("Apply failed (build mismatch or other); changes rolled back")

    if args.commit:
        make_commit(args.func, src, base, 0, max_iter)

    print(f"=== {args.func} retired successfully ===")


if __name__ == "__main__":
    main()
