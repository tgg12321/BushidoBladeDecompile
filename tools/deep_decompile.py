#!/usr/bin/env python3
"""Deep decompile: hammer a single function until it matches or we're truly stuck.

Unlike overnight_run.py (quick pass across many functions), this script focuses
all effort on ONE function, iterating permuter + DeepSeek in a tight loop until:
  - Score hits 0 (exact match) → apply to src, verify build, commit
  - No improvement for --patience rounds (default 20) → give up

Each round:
  1. Run permuter for --timeout seconds
  2. Score best permuter output (accurate objdump diff)
  3. If improved: update base.c, reset stale counter
  4. Ask DeepSeek to improve based on instruction diff
  5. If DeepSeek draft compiles and scores better: update base.c, reset stale counter
  6. If neither improved: increment stale counter

Usage (from project root, in WSL):
    python3 tools/deep_decompile.py func_80017FA0
    python3 tools/deep_decompile.py func_80017FA0 --patience 30 --timeout 120
    python3 tools/deep_decompile.py func_80017FA0 --no-deepseek   # permuter only
"""
import argparse
import io
import json
import os
import re
import subprocess
import sys
import time
import urllib.request
from datetime import datetime

# UTF-8 stdout handled by permuter_babysit import

# ---------------------------------------------------------------------------
# Imports from permuter_babysit
# ---------------------------------------------------------------------------
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from permuter_babysit import (
    ensure_permuter_dir,
    write_base_c,
    read_base_c_code,
    run_permuter,
    get_best_permuter_output,
    score_code,
    generate_improved,
    apply_and_verify,
    get_insn_diff,
    compile_code,
    strip_perm_headers,
    OLLAMA_URL,
)

DEEPSEEK_CUTOFF = 30  # auto-disable DeepSeek below this score when insn counts match

ASM_DIR    = os.path.join("asm", "funcs")
DRAFTS_DIR = os.path.join("local_drafts", "bb2-deepseek")
SRC_DIR    = "src"
VENV_ACTIVATE = "source .venv/bin/activate"

# ---------------------------------------------------------------------------
# Git
# ---------------------------------------------------------------------------

def git_commit(message):
    cmd = f'git add -A && git commit -m {json.dumps(message)}'
    result = subprocess.run(["bash", "-c", cmd],
                            capture_output=True, text=True, timeout=30)
    return result.returncode == 0


def git_add_progress(func_name, score, round_num):
    """Commit current progress (drafts + base.c)."""
    msg = f"deep: {func_name} score {score} (round {round_num})\n\nCo-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>"
    if git_commit(msg):
        print(f"  [committed: score {score}, round {round_num}]", flush=True)

# ---------------------------------------------------------------------------
# Main loop
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Deep decompile: hammer one function until match")
    parser.add_argument("func_name", help="Function to decompile (e.g. func_80017FA0)")
    parser.add_argument("--patience", type=int, default=20,
                        help="Give up after N rounds with no improvement (default 20)")
    parser.add_argument("--timeout", type=int, default=90,
                        help="Permuter seconds per round (default 90)")
    parser.add_argument("--jobs", "-j", type=int, default=4,
                        help="Permuter parallel jobs (default 4)")
    parser.add_argument("--no-deepseek", action="store_true",
                        help="Permuter only, no DeepSeek feedback")
    parser.add_argument("--commit-every", type=int, default=5,
                        help="Commit progress every N improvements (default 5)")
    args = parser.parse_args()

    func_name = args.func_name
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")

    if not os.path.exists(asm_path):
        print(f"ERROR: {asm_path} not found")
        sys.exit(1)

    asm_text = open(asm_path).read()

    # Preflight: check Ollama
    if not args.no_deepseek:
        print(f"Ollama URL: {OLLAMA_URL}")
        try:
            urllib.request.urlopen(OLLAMA_URL.replace("/api/generate", "/api/tags"), timeout=5).read()
            print(f"Ollama: OK")
        except Exception as e:
            print(f"ERROR: Ollama not reachable at {OLLAMA_URL}")
            print(f"  {e}")
            print(f"  Use --no-deepseek to run permuter only, or start Ollama.")
            sys.exit(1)

    # Ensure permuter dir
    if not ensure_permuter_dir(func_name):
        print(f"ERROR: could not set up permuter dir for {func_name}")
        sys.exit(1)

    # Get initial score
    base_code = strip_perm_headers(read_base_c_code(func_name))
    init_score, init_info = score_code(func_name, base_code)
    if init_score is None:
        print(f"ERROR: initial scoring failed ({init_info})")
        print(f"  Make sure permuter/{func_name}/base.c compiles and target.o exists")
        sys.exit(1)

    best_score = init_score
    best_code = base_code
    best_info = init_info
    stale_rounds = 0
    improvements = 0
    uncommitted_improvements = 0
    total_rounds = 0
    use_deepseek = not args.no_deepseek
    start_time = time.time()

    print(f"\n{'='*60}")
    print(f"DEEP DECOMPILE: {func_name}")
    print(f"{'='*60}")
    print(f"  Initial score: {best_score} ({init_info})")
    print(f"  Patience: {args.patience} rounds without improvement")
    print(f"  Permuter: {args.timeout}s × -j{args.jobs}")
    print(f"  DeepSeek: {'disabled' if args.no_deepseek else 'enabled'}")
    print(f"{'='*60}\n")

    try:
        while True:
            total_rounds += 1
            elapsed_total = time.time() - start_time
            print(f"--- Round {total_rounds} (best={best_score}, stale={stale_rounds}/{args.patience}, "
                  f"elapsed={elapsed_total/60:.0f}m) ---", flush=True)

            improved_this_round = False

            # --- Phase 1: Permuter search ---
            stopped_on_zero = run_permuter(func_name, timeout=args.timeout, jobs=args.jobs)

            perm_dir_score, perm_code = get_best_permuter_output(func_name)
            if perm_code:
                actual_score, info = score_code(func_name, perm_code)
                if actual_score is not None:
                    print(f"  [permuter] best: {actual_score} ({info})", flush=True)
                    if actual_score == 0 or stopped_on_zero:
                        print(f"\n  MATCH FOUND! Applying to source...", flush=True)
                        if apply_and_verify(func_name, perm_code):
                            msg = (f"deep: MATCH {func_name} (round {total_rounds})\n\n"
                                   f"Started at score {init_score}, matched after {total_rounds} rounds.\n"
                                   f"Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>")
                            git_commit(msg)
                            print(f"  COMMITTED! Score 0 after {total_rounds} rounds.")
                        else:
                            print(f"  Build verification failed — match not applied")
                        return
                    if actual_score < best_score:
                        print(f"  [permuter] IMPROVED: {best_score} → {actual_score}", flush=True)
                        best_score = actual_score
                        best_code = perm_code
                        best_info = info
                        write_base_c(func_name, perm_code)
                        improved_this_round = True

            # --- Auto-switch: disable DeepSeek when it can't help ---
            if use_deepseek and best_info:
                m = re.match(r'(\d+)vs(\d+)', best_info)
                if m and int(m.group(1)) == int(m.group(2)) and best_score <= DEEPSEEK_CUTOFF:
                    print(f"  [auto] instruction counts match ({best_info}) and score ≤ {DEEPSEEK_CUTOFF}"
                          f" — switching to permuter-only", flush=True)
                    use_deepseek = False

            # --- Phase 2: DeepSeek feedback ---
            if use_deepseek:
                print(f"  [deepseek] generating improvement...", end=" ", flush=True)
                t0 = time.time()
                diff_text = get_insn_diff(func_name, best_code)
                improved = generate_improved(
                    func_name, best_code, perm_dir_score, best_score, asm_text, diff_text
                )
                ds_elapsed = time.time() - t0

                if not improved:
                    print(f"FAILED ({ds_elapsed:.1f}s)", flush=True)
                else:
                    print(f"done ({ds_elapsed:.1f}s)", flush=True)
                    # Score it
                    print(f"  [deepseek] scoring...", end=" ", flush=True)
                    new_score, info = score_code(func_name, improved)
                    if new_score is None:
                        print(f"FAIL ({info})", flush=True)
                    else:
                        print(f"{new_score} ({info})", flush=True)
                        if new_score == 0:
                            print(f"\n  MATCH FOUND! Applying to source...", flush=True)
                            if apply_and_verify(func_name, improved):
                                msg = (f"deep: MATCH {func_name} (round {total_rounds})\n\n"
                                       f"Started at score {init_score}, matched after {total_rounds} rounds.\n"
                                       f"Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>")
                                git_commit(msg)
                                print(f"  COMMITTED! Score 0 after {total_rounds} rounds.")
                            else:
                                print(f"  Build verification failed — match not applied")
                            return
                        if new_score < best_score:
                            print(f"  [deepseek] IMPROVED: {best_score} → {new_score}", flush=True)
                            best_info = info
                            best_score = new_score
                            best_code = improved
                            write_base_c(func_name, improved)
                            improved_this_round = True

            # --- Track progress ---
            if improved_this_round:
                stale_rounds = 0
                improvements += 1
                uncommitted_improvements += 1
                # Save draft
                os.makedirs(DRAFTS_DIR, exist_ok=True)
                with open(os.path.join(DRAFTS_DIR, f"{func_name}.c"), "w", newline="\n") as f:
                    f.write(best_code + "\n")
                # Periodic commit
                if uncommitted_improvements >= args.commit_every:
                    git_add_progress(func_name, best_score, total_rounds)
                    uncommitted_improvements = 0
            else:
                stale_rounds += 1

            # --- Check patience ---
            if stale_rounds >= args.patience:
                print(f"\n  GIVING UP: {args.patience} rounds with no improvement.")
                break

    except KeyboardInterrupt:
        print(f"\n\nInterrupted by user.")

    # --- Final summary ---
    elapsed_total = time.time() - start_time

    # Commit any remaining progress
    if uncommitted_improvements > 0:
        git_add_progress(func_name, best_score, total_rounds)

    print(f"\n{'='*60}")
    print(f"DEEP DECOMPILE COMPLETE: {func_name}")
    print(f"{'='*60}")
    print(f"  Initial score:  {init_score}")
    print(f"  Final score:    {best_score}")
    print(f"  Improvement:    {init_score - best_score} ({init_score} → {best_score})")
    print(f"  Total rounds:   {total_rounds}")
    print(f"  Improvements:   {improvements}")
    print(f"  Elapsed:        {elapsed_total/60:.1f} minutes")
    if best_score > 0:
        print(f"\n  Not matched. Best draft saved to:")
        print(f"    {os.path.join(DRAFTS_DIR, func_name + '.c')}")
        print(f"    permuter/{func_name}/base.c")
    print()


if __name__ == "__main__":
    main()
