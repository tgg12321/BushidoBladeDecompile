#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Permuter babysitter: alternates DeepSeek semantic rewrites with permuter random search.

Workflow for each function:
  1. Ensure permuter dir + base.c is seeded from local_drafts/
  2. Run permuter for PERM_TIMEOUT seconds
  3. Find the best output-SCORE-* result the permuter discovered
  4. Score it accurately (objdump diff)
  5. Ask DeepSeek: "permuter found this (score X) — synthesize something better"
  6. Write new base.c, repeat for MAX_ROUNDS or until score=0

The two tools complement each other:
  - Permuter: fast random local mutations (can't restructure, can't reason)
  - DeepSeek: semantic reasoning about what the permuter found (can restructure whole functions)

Usage (Git Bash):
    python tools/permuter_babysit.py func_80043C7C func_80043D34
    python tools/permuter_babysit.py --top 10           # top N by rescore score
    python tools/permuter_babysit.py --min-score 100    # all drafts scoring <= 100
    python tools/permuter_babysit.py --rounds 8 func_8003D330
"""
import argparse
import io
import json
import os
import re
import subprocess
import sys
import tempfile
import time
from datetime import datetime

# Ensure UTF-8 output on Windows
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------
OLLAMA_URL   = "http://localhost:11434/api/generate"
MODEL        = "bb2-deepseek"
ASM_DIR      = os.path.join("asm", "funcs")
DRAFTS_DIR   = os.path.join("local_drafts", "bb2-deepseek")
PERMUTER_DIR = "permuter"
SRC_DIR      = "src"

WSL_ROOT = '/mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"'
WSL_VENV = "source .venv/bin/activate"

PERM_TIMEOUT = 90     # seconds per permuter run
PERM_JOBS    = 4      # permuter threads
MAX_ROUNDS   = 6      # DeepSeek+permuter cycles per function
SCORE_FLOOR  = 0      # only stop on exact match (score=0)

COMPILE_WRAPPER = '#include "common.h"\n#include "include_asm.h"\n\n{code}\n'

def win_to_wsl(path):
    """Convert a Windows absolute path (C:\\...) to WSL /mnt/c/... path."""
    path = path.replace("\\", "/")
    if len(path) >= 2 and path[1] == ":":
        return f"/mnt/{path[0].lower()}/{path[3:]}"
    return path


def write_temp_c(code, suffix=".c"):
    """Write code to a Windows temp file. Returns (win_path, wsl_path)."""
    tmp = tempfile.NamedTemporaryFile(delete=False, suffix=suffix, mode="w", newline="\n")
    tmp.write(code)
    tmp.close()
    return tmp.name, win_to_wsl(tmp.name)


# Typedef prefix lines the permuter adds to its output — strip these before wrapping
_PERM_TYPEDEF_RE = re.compile(
    r'^typedef\s+(unsigned\s+|signed\s+)?(char|short|int|long)\s+[su]\d+\s*;'
)


def strip_perm_headers(code):
    """Strip typedef lines (and leading blank lines) that permuter prepends to its outputs."""
    lines = code.splitlines()
    start = 0
    for i, line in enumerate(lines):
        if _PERM_TYPEDEF_RE.match(line.strip()):
            start = i + 1
        elif line.strip() == "" and start == i:
            start = i + 1
        else:
            break
    return "\n".join(lines[start:]).strip()

# ---------------------------------------------------------------------------
# Scoring (from rescore_drafts.py — the accurate objdump method)
# ---------------------------------------------------------------------------

def compile_code(func_name, code):
    """Compile code snippet via permuter_compile.sh. Returns WSL path to .o or None."""
    wrapped = COMPILE_WRAPPER.format(code=strip_perm_headers(code))
    _, wsl_src = write_temp_c(wrapped)
    wsl_out = f"/tmp/bb2_babysit_{func_name}.o"

    wsl_cmd = (
        f'cd {WSL_ROOT} && {WSL_VENV} && '
        f'bash tools/permuter_compile.sh "{wsl_src}" -o {wsl_out} 2>&1 && '
        f'test -f {wsl_out} && echo __OK__'
    )
    result = subprocess.run(["wsl", "bash", "-lc", wsl_cmd],
                            capture_output=True, text=True, timeout=90)
    if "__OK__" in result.stdout:
        return wsl_out
    return None


def get_insn_hex(wsl_obj_path):
    """Return list of raw instruction hex words via objdump."""
    cmd = f'mipsel-linux-gnu-objdump -d "{wsl_obj_path}" 2>/dev/null'
    result = subprocess.run(["wsl", "bash", "-lc", cmd],
                            capture_output=True, text=True, timeout=15)
    words = []
    for line in result.stdout.splitlines():
        m = re.match(r'^\s+[0-9a-f]+:\s+([0-9a-f]{8})\s', line)
        if m:
            words.append(m.group(1))
    return words


def get_target_o(func_name):
    """Return WSL path to target.o (pre-assembled from permuter setup)."""
    perm_target = os.path.join(PERMUTER_DIR, func_name, "target.o")
    if os.path.exists(perm_target):
        # Convert Windows path to WSL path
        abs_path = os.path.abspath(perm_target).replace("\\", "/")
        if abs_path[1] == ":":
            abs_path = f"/mnt/{abs_path[0].lower()}/{abs_path[3:]}"
        return abs_path
    return None


def score_code(func_name, code):
    """Accurate objdump-diff score. Returns (int, info_str) or (None, reason)."""
    draft_o = compile_code(func_name, code)
    if not draft_o:
        return None, "compile_fail"

    target_o = get_target_o(func_name)
    if not target_o:
        return None, "no_target"

    ours   = get_insn_hex(draft_o)
    target = get_insn_hex(target_o)
    if not ours or not target:
        return None, "no_insns"

    diffs = sum(1 for i in range(max(len(ours), len(target)))
                if (ours[i] if i < len(ours) else None) !=
                   (target[i] if i < len(target) else None))
    return diffs, f"{len(ours)}vs{len(target)}"


def get_insn_diff(func_name, our_code, max_lines=40):
    """Return a side-by-side instruction diff for DeepSeek context."""
    wrapped  = COMPILE_WRAPPER.format(code=strip_perm_headers(our_code))
    target_o = get_target_o(func_name)
    if not target_o:
        return ""

    _, wsl_src = write_temp_c(wrapped)
    wsl_out = f"/tmp/bb2_diff_{func_name}.o"

    wsl_cmd = (
        f'cd {WSL_ROOT} && {WSL_VENV} && '
        f'bash tools/permuter_compile.sh "{wsl_src}" -o {wsl_out} 2>/dev/null && '
        f'diff <(mipsel-linux-gnu-objdump -d --no-show-raw-insn "{wsl_out}" 2>/dev/null | grep -E "^\\s") '
        f'     <(mipsel-linux-gnu-objdump -d --no-show-raw-insn "{target_o}" 2>/dev/null | grep -E "^\\s") '
        f'| head -{max_lines}'
    )
    result = subprocess.run(["wsl", "bash", "-lc", wsl_cmd],
                            capture_output=True, text=True, timeout=60)
    return result.stdout.strip()

# ---------------------------------------------------------------------------
# Permuter runner
# ---------------------------------------------------------------------------

def run_permuter(func_name, timeout=None, jobs=None):
    timeout = timeout or PERM_TIMEOUT
    jobs    = jobs    or PERM_JOBS
    """Run permuter for `timeout` seconds in background. Returns when done or killed."""
    perm_path = f"permuter/{func_name}"
    wsl_cmd = (
        f'cd {WSL_ROOT} && {WSL_VENV} && '
        f'timeout {timeout} python3 tools/decomp-permuter/permuter.py {perm_path}/ '
        f'-j{jobs} --stop-on-zero 2>&1 | grep -v "^$" | tail -3'
    )
    print(f"  [permuter] running {timeout}s with -j{jobs}...", flush=True)
    t0 = time.time()
    result = subprocess.run(["wsl", "bash", "-lc", wsl_cmd],
                            capture_output=True, text=True,
                            timeout=timeout + 30)
    elapsed = time.time() - t0
    stopped_on_zero = "score 0" in result.stdout or "Stopped" in result.stdout
    print(f"  [permuter] done in {elapsed:.0f}s"
          + (" — MATCH FOUND!" if stopped_on_zero else ""), flush=True)
    return stopped_on_zero


def get_best_permuter_output(func_name):
    """Scan output-SCORE-N dirs, return (best_perm_score, source_code) or (None, None)."""
    perm_dir = os.path.join(PERMUTER_DIR, func_name)
    if not os.path.isdir(perm_dir):
        return None, None

    best_score, best_code = None, None
    for entry in os.listdir(perm_dir):
        m = re.match(r'output-(\d+)-\d+$', entry)
        if not m:
            continue
        score = int(m.group(1))
        src = os.path.join(perm_dir, entry, "source.c")
        if os.path.exists(src) and (best_score is None or score < best_score):
            best_score = score
            best_code  = open(src).read().strip()

    return best_score, best_code

# ---------------------------------------------------------------------------
# Permuter dir setup
# ---------------------------------------------------------------------------

def ensure_permuter_dir(func_name):
    """Ensure permuter dir exists with a seeded base.c from local_drafts/. Returns True if ready."""
    perm_dir = os.path.join(PERMUTER_DIR, func_name)
    draft_path = os.path.join(DRAFTS_DIR, f"{func_name}.c")
    asm_path   = os.path.join(ASM_DIR, f"{func_name}.s")

    if not os.path.exists(asm_path):
        print(f"  ERROR: no asm file for {func_name}")
        return False

    # Run permuter_setup.sh if no dir yet
    if not os.path.isdir(perm_dir):
        print(f"  [setup] running permuter_setup.sh...", flush=True)
        wsl_cmd = (
            f'cd {WSL_ROOT} && {WSL_VENV} && '
            f'bash tools/permuter_setup.sh {func_name} 2>&1 | tail -5'
        )
        result = subprocess.run(["wsl", "bash", "-lc", wsl_cmd],
                                capture_output=True, text=True, timeout=60)
        if not os.path.isdir(perm_dir):
            print(f"  ERROR: permuter_setup.sh failed: {result.stdout[:200]}")
            return False

    # Seed base.c from our DeepSeek draft if the draft is better than the current stub
    base_c = os.path.join(perm_dir, "base.c")
    base_is_stub = not os.path.exists(base_c) or (
        os.path.exists(base_c) and "TODO: decompile" in open(base_c).read()
    )
    if base_is_stub and os.path.exists(draft_path):
        print(f"  [setup] seeding base.c from DeepSeek draft", flush=True)
        draft_code = open(draft_path).read().strip()
        write_base_c(func_name, draft_code)

    return os.path.isdir(perm_dir)


PERM_TYPEDEFS = (
    "typedef unsigned char u8;\n"
    "typedef signed char s8;\n"
    "typedef unsigned short u16;\n"
    "typedef signed short s16;\n"
    "typedef unsigned int u32;\n"
    "typedef signed int s32;\n\n"
)


def read_base_c_code(func_name):
    """Read the full base.c content (typedefs + function). strip_perm_headers handles cleanup."""
    base_c = os.path.join(PERMUTER_DIR, func_name, "base.c")
    if not os.path.exists(base_c):
        return ""
    return open(base_c).read().strip()


def write_base_c(func_name, code):
    """Write new base.c with inline typedefs (permuter-compatible, LF endings)."""
    clean = strip_perm_headers(code)
    base_c = os.path.join(PERMUTER_DIR, func_name, "base.c")
    with open(base_c, "w", newline="\n") as f:
        f.write(PERM_TYPEDEFS + clean + "\n")

# ---------------------------------------------------------------------------
# DeepSeek synthesis
# ---------------------------------------------------------------------------

def generate_improved(func_name, best_perm_code, best_perm_score, our_score, asm_text, diff_text):
    """Ask DeepSeek to synthesize an improved C function given permuter output."""

    prompt_parts = [
        "You are improving a MIPS PS1 decompilation for GCC 2.7.2 -O2. "
        "The permuter found the code below as its best candidate. "
        "Analyze the instruction diff, reason about what structural changes are needed, "
        "and produce a BETTER C function. Output ONLY the C function — no explanation, no markdown.\n",
    ]

    if best_perm_code:
        prompt_parts.append(
            f"## Best permuter result (permuter score {best_perm_score}, our diff score {our_score}):\n"
            f"```c\n{best_perm_code}\n```\n"
        )

    if diff_text:
        prompt_parts.append(
            f"## Instruction diff (< = compiled, > = target):\n"
            f"```\n{diff_text}\n```\n"
            "Focus on fixing these specific mismatches. "
            "Consider: variable types, signed/unsigned, operation ordering, loop structure, "
            "register reuse patterns.\n"
        )

    prompt_parts.append(
        f"## Target assembly:\n{asm_text}"
    )

    prompt = "\n".join(prompt_parts)
    payload = json.dumps({"model": MODEL, "stream": False, "prompt": prompt}).encode()
    req = __import__("urllib.request", fromlist=["Request", "urlopen"])
    request_obj = req.Request(OLLAMA_URL, data=payload,
                              headers={"Content-Type": "application/json"})
    try:
        with req.urlopen(request_obj, timeout=600) as resp:
            data = json.loads(resp.read().decode())
            raw = data.get("response", "")
    except Exception as e:
        print(f"  ERROR: DeepSeek request failed: {e}")
        return None

    # DeepSeek sometimes outputs literal \n (0x5C 0x6E) instead of real newlines (0x0A).
    # Unconditionally replace — harmless if there are none.
    raw = raw.replace('\x5c\x6e', '\x0a')

    # Clean up code fences
    raw = raw.strip()
    raw = re.sub(r'^```[a-zA-Z]*\n', '', raw)
    raw = re.sub(r'\n```\s*$', '', raw)
    lines = [l for l in raw.split('\n') if not l.strip().startswith('#include')]
    return '\n'.join(lines).strip()

# ---------------------------------------------------------------------------
# Apply match to src
# ---------------------------------------------------------------------------

def find_src_file(func_name):
    for fname in os.listdir(SRC_DIR):
        if not fname.endswith(".c"):
            continue
        path = os.path.join(SRC_DIR, fname)
        with open(path) as f:
            if f'INCLUDE_ASM("asm/funcs", {func_name})' in f.read():
                return path
    return None


def apply_and_verify(func_name, code):
    """Replace INCLUDE_ASM stub in src, build, verify SHA1. Returns True on success."""
    src_path = find_src_file(func_name)
    if not src_path:
        print(f"  ERROR: can't find src file for {func_name}")
        return False

    with open(src_path) as f:
        orig = f.read()

    stub = f'INCLUDE_ASM("asm/funcs", {func_name});'
    if stub not in orig:
        stub = f'INCLUDE_ASM("asm/funcs", {func_name})'
    if stub not in orig:
        print(f"  ERROR: stub pattern not found in {src_path}")
        return False

    new_content = orig.replace(stub, code)
    wsl_path = os.path.abspath(src_path).replace("\\", "/")
    if wsl_path[1] == ":":
        wsl_path = f"/mnt/{wsl_path[0].lower()}/{wsl_path[3:]}"

    wsl_cmd = f'printf \'%s\' {json.dumps(new_content)} > "{wsl_path}"'
    subprocess.run(["wsl", "bash", "-lc", wsl_cmd], capture_output=True, timeout=30)

    # Verify build
    build_cmd = (
        f'cd {WSL_ROOT} && {WSL_VENV} && '
        f'make 2>&1 | tail -3'
    )
    result = subprocess.run(["wsl", "bash", "-lc", build_cmd],
                            capture_output=True, text=True, timeout=120)
    if "OK: bb2 matches" in result.stdout:
        return True

    # Revert on failure
    revert_cmd = f'printf \'%s\' {json.dumps(orig)} > "{wsl_path}"'
    subprocess.run(["wsl", "bash", "-lc", revert_cmd], capture_output=True, timeout=30)
    print(f"  build FAILED — reverted")
    return False


def git_commit_match(func_name):
    msg = (
        f"match {func_name} via permuter+deepseek babysitter\n\n"
        f"Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
    )
    wsl_cmd = (
        f'cd {WSL_ROOT} && git add -u && '
        f'git diff --cached --quiet || git commit -m {json.dumps(msg)}'
    )
    subprocess.run(["wsl", "bash", "-lc", wsl_cmd], capture_output=True, timeout=60)

# ---------------------------------------------------------------------------
# Per-function babysit loop
# ---------------------------------------------------------------------------

def babysit_function(func_name, max_rounds=MAX_ROUNDS, perm_timeout=None, perm_jobs=None):
    print(f"\n{'='*60}")
    print(f"BABYSITTING: {func_name}")
    print(f"{'='*60}")

    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        print(f"  ERROR: {asm_path} not found — skipping")
        return False

    asm_text = open(asm_path).read()

    if not ensure_permuter_dir(func_name):
        return False

    # Score the current base.c before any rounds to establish baseline
    print(f"  [init] scoring current base.c...", end=" ", flush=True)
    init_code = read_base_c_code(func_name)
    init_score, init_info = score_code(func_name, init_code) if init_code else (None, "empty")
    if init_score is not None:
        best_score_ever = init_score
        print(f"{init_score} ({init_info})")
    else:
        best_score_ever = None
        print(f"FAIL ({init_info}) - will accept any compiling DeepSeek output <= 100")

    for rnd in range(1, max_rounds + 1):
        print(f"\n  --- Round {rnd}/{max_rounds} ---")

        # Run permuter
        stopped_on_zero = run_permuter(func_name, timeout=perm_timeout, jobs=perm_jobs)

        # Get best permuter output
        best_perm_score, best_perm_code = get_best_permuter_output(func_name)
        if best_perm_code:
            print(f"  [permuter] best output dir score: {best_perm_score}")
        else:
            print(f"  [permuter] no output dirs found yet")

        # Score accurately with objdump
        code_to_score = best_perm_code or read_base_c_code(func_name)
        if code_to_score:
            print(f"  [score] measuring...", end=" ", flush=True)
            our_score, info = score_code(func_name, code_to_score)
            print(f"{our_score} ({info})" if our_score is not None else f"FAIL ({info})")
        else:
            our_score = None

        if our_score == 0 or stopped_on_zero:
            print(f"  EXACT MATCH!")
            if apply_and_verify(func_name, code_to_score):
                git_commit_match(func_name)
                print(f"  Committed match for {func_name}!")
                return True
            else:
                print(f"  WARNING: score=0 but full build failed — continuing")

        if our_score is not None:
            if best_score_ever is None or our_score < best_score_ever:
                best_score_ever = our_score
                print(f"  New best: {our_score}")

            if our_score <= SCORE_FLOOR:
                print(f"  Score {our_score} <= floor {SCORE_FLOOR} — close enough, stopping")
                print(f"  Draft is at permuter/{func_name}/output-*/source.c — inspect manually")
                break

        if rnd == max_rounds:
            print(f"  Max rounds reached. Best score: {best_score_ever}")
            break

        # Ask DeepSeek to synthesize a better version
        print(f"  [deepseek] generating improved base.c...", end=" ", flush=True)
        t0 = time.time()

        diff_text = ""
        if best_perm_code:
            diff_text = get_insn_diff(func_name, best_perm_code)

        improved = generate_improved(
            func_name, best_perm_code, best_perm_score, our_score, asm_text, diff_text
        )
        elapsed = time.time() - t0

        if not improved:
            print(f"FAILED ({elapsed:.1f}s) — keeping current base.c")
            continue

        print(f"done ({elapsed:.1f}s, {len(improved)} chars)")

        # Quick compile check on the new code
        print(f"  [compile] checking new draft...", end=" ", flush=True)
        new_score, info = score_code(func_name, improved)
        if new_score is None:
            print(f"FAIL ({info}) — keeping previous base.c")
            continue
        print(f"score={new_score} ({info})")

        # Only update base.c if the new version is better (or close)
        accept_threshold = (best_score_ever + 10) if best_score_ever is not None else 100
        if new_score <= accept_threshold:
            write_base_c(func_name, improved)
            if best_score_ever is None or new_score < best_score_ever:
                best_score_ever = new_score
            print(f"  Updated base.c (score {new_score})")
            # Save to drafts dir too
            draft_path = os.path.join(DRAFTS_DIR, f"{func_name}.c")
            os.makedirs(DRAFTS_DIR, exist_ok=True)
            with open(draft_path, "w", newline="\n") as f:
                f.write(improved + "\n")
        else:
            print(f"  New draft worse ({new_score} vs {best_score_ever}) — keeping old base.c")

    print(f"\n  Final best score for {func_name}: {best_score_ever}")
    return False

# ---------------------------------------------------------------------------
# Candidate loading
# ---------------------------------------------------------------------------

def load_top_candidates(min_score=None, top_n=None):
    """Load function names from rescore_output.log sorted by score."""
    log_path = "rescore_output.log"
    if not os.path.exists(log_path):
        return []

    entries = []
    for line in open(log_path, errors="replace"):
        m = re.search(r'\] (func_[0-9A-Fa-f]+)\.\.\. score=(\d+)', line)
        if m:
            entries.append((int(m.group(2)), m.group(1)))

    entries.sort()
    if min_score is not None:
        entries = [(s, f) for s, f in entries if s <= min_score]
    if top_n:
        entries = entries[:top_n]
    return [f for _, f in entries]

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="DeepSeek+permuter babysitter for BB2 decompilation"
    )
    parser.add_argument("funcs", nargs="*", help="Function names to babysit")
    parser.add_argument("--top", type=int, default=0,
                        help="Process top N functions from rescore_output.log")
    parser.add_argument("--min-score", type=int, default=0,
                        help="Process all drafts with rescore score <= N")
    parser.add_argument("--rounds", type=int, default=MAX_ROUNDS,
                        help=f"Max rounds per function (default {MAX_ROUNDS})")
    parser.add_argument("--timeout", type=int, default=PERM_TIMEOUT,
                        help=f"Permuter timeout per round in seconds (default {PERM_TIMEOUT})")
    parser.add_argument("--jobs", type=int, default=PERM_JOBS,
                        help=f"Permuter parallel jobs (default {PERM_JOBS})")
    args = parser.parse_args()

    perm_timeout = args.timeout
    perm_jobs    = args.jobs

    funcs = list(args.funcs)

    if args.top:
        funcs += load_top_candidates(top_n=args.top)
    elif args.min_score:
        funcs += load_top_candidates(min_score=args.min_score)

    if not funcs:
        parser.print_help()
        print("\nNo functions specified. Try:")
        print("  python tools/permuter_babysit.py --top 5")
        print("  python tools/permuter_babysit.py func_80043C7C func_80043D34")
        sys.exit(1)

    # Deduplicate while preserving order
    seen = set()
    funcs = [f for f in funcs if not (f in seen or seen.add(f))]

    print(f"Babysitting {len(funcs)} functions:")
    for f in funcs:
        print(f"  {f}")
    print(f"\nConfig: {args.rounds} rounds, {perm_timeout}s/round, -j{perm_jobs}")
    print()

    matched = []
    for func_name in funcs:
        success = babysit_function(func_name, max_rounds=args.rounds,
                                   perm_timeout=perm_timeout, perm_jobs=perm_jobs)
        if success:
            matched.append(func_name)

    print(f"\n{'='*60}")
    print(f"BABYSIT RUN COMPLETE")
    print(f"{'='*60}")
    print(f"  Processed: {len(funcs)}")
    print(f"  Matched:   {len(matched)}")
    if matched:
        for f in matched:
            print(f"    {f}")
    if not matched:
        print(f"  No exact matches — check permuter/FUNC/output-*/ for near-misses")


if __name__ == "__main__":
    main()
