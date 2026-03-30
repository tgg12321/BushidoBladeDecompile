"""Overnight DeepSeek decompilation run.

For each remaining INCLUDE_ASM stub:
  1. Always regenerate with DeepSeek (prior drafts fed as context, not reused as-is)
  2. Standalone compile test
  3. If compiles + permuter dir exists: score it
  4. If score == 0 (exact match): apply to src, verify full build, commit immediately
  5. Save draft to local_drafts/bb2-deepseek/ regardless

Commits:
  - Immediate: after every score-0 match applied to src
  - Checkpoint: after every BATCH_SIZE functions, commits overnight_progress.md

Usage (from project root in Git Bash):
    python tools/overnight_run.py
    python tools/overnight_run.py --limit 50        # test run
    python tools/overnight_run.py --min-score 200   # also apply near-misses (risky)
    python tools/overnight_run.py --skip-compile    # draft only, no compile/score
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time
import urllib.request
from datetime import datetime

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

OLLAMA_URL    = "http://localhost:11434/api/generate"
MODEL         = "bb2-deepseek"
SRC_DIR       = "src"
ASM_DIR       = os.path.join("asm", "funcs")
DRAFTS_DIR    = os.path.join("local_drafts", "bb2-deepseek")
PERMUTER_DIR  = "permuter"
SYMBOL_ADDRS  = "symbol_addrs.txt"
PROGRESS_FILE = "overnight_progress.md"
BATCH_SIZE    = 10   # commit progress log every N functions

WSL_ROOT = '/mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"'
WSL_VENV = "source .venv/bin/activate"

# ---------------------------------------------------------------------------
# Symbol table
# ---------------------------------------------------------------------------

def load_symbol_table():
    globals_by_addr = {}
    funcs_by_addr = {}
    if not os.path.exists(SYMBOL_ADDRS):
        return globals_by_addr, funcs_by_addr
    with open(SYMBOL_ADDRS) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            m = re.match(r'(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;', line)
            if not m:
                continue
            name, addr = m.group(1), m.group(2).lower()
            if name.startswith("g_") or name.startswith("D_"):
                globals_by_addr[addr] = name
            else:
                funcs_by_addr[addr] = name
    return globals_by_addr, funcs_by_addr


def resolve_symbols(asm, globals_by_addr, funcs_by_addr):
    found = {}
    for m in re.finditer(r'\bD_([0-9A-Fa-f]{8})\b', asm):
        addr = "0x" + m.group(1).lower()
        if addr in globals_by_addr:
            found[m.group(0)] = globals_by_addr[addr]
    for m in re.finditer(r'\bfunc_([0-9A-Fa-f]{8})\b', asm):
        addr = "0x" + m.group(1).lower()
        if addr in funcs_by_addr:
            found[m.group(0)] = funcs_by_addr[addr]
    return found

# ---------------------------------------------------------------------------
# Context gathering
# ---------------------------------------------------------------------------

def get_context(func_name, asm, globals_by_addr, funcs_by_addr):
    parts = []

    known_syms = resolve_symbols(asm, globals_by_addr, funcs_by_addr)
    if known_syms:
        sym_lines = "\n".join(f"  {k} => {v}" for k, v in sorted(known_syms.items()))
        parts.append(
            "## Known symbol names (use these instead of D_XXXXXXXX):\n" + sym_lines
        )

    perm_dir = os.path.join(PERMUTER_DIR, func_name)
    if os.path.isdir(perm_dir):
        best_score, best_path = None, None
        for entry in os.listdir(perm_dir):
            m = re.match(r'output-(\d+)-\d+', entry)
            if m:
                score = int(m.group(1))
                sp = os.path.join(perm_dir, entry, "source.c")
                if os.path.exists(sp) and (best_score is None or score < best_score):
                    best_score, best_path = score, sp
        if best_path:
            code = open(best_path).read().strip()
            parts.append(
                f"## Best permuter result (score {best_score}, 0=exact match):\n"
                f"```c\n{code}\n```"
            )

    draft_path = os.path.join("local_drafts", f"{func_name}.c")
    if os.path.exists(draft_path):
        draft = open(draft_path).read().strip()
        if draft and "ERROR" not in draft:
            parts.append(
                "## Prior draft (reference, may have issues):\n"
                f"```c\n{draft}\n```"
            )

    return "\n\n".join(parts)

# ---------------------------------------------------------------------------
# Model query
# ---------------------------------------------------------------------------

def generate_draft(func_name, globals_by_addr, funcs_by_addr):
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        return None
    asm = open(asm_path).read()
    context = get_context(func_name, asm, globals_by_addr, funcs_by_addr)

    if context:
        prompt = (
            "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
            "Output ONLY the C function, no explanation, no markdown.\n\n"
            "I have prior work. Use it to produce a better result — fix issues, "
            "apply named symbols, resolve remaining differences.\n\n"
            f"{context}\n\n"
            "## Target assembly:\n" + asm
        )
    else:
        prompt = (
            "Decompile this MIPS assembly to matching C for GCC 2.7.2 -O2. "
            "Output ONLY the C function, no explanation, no markdown.\n\n" + asm
        )

    payload = json.dumps({"model": MODEL, "stream": False, "prompt": prompt}).encode()
    req = urllib.request.Request(OLLAMA_URL, data=payload,
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=600) as resp:
            data = json.loads(resp.read().decode())
            return data.get("response", "")
    except Exception as e:
        return f"/* ERROR: {e} */"


def clean_draft(text):
    text = text.strip()
    text = re.sub(r'^```[a-zA-Z]*\n', '', text)
    text = re.sub(r'\n```\s*$', '', text)
    # Strip any #include lines (GCC 2.7.2 needs none for standalone test)
    lines = [l for l in text.split('\n') if not l.strip().startswith('#include')]
    return '\n'.join(lines).strip()

# ---------------------------------------------------------------------------
# Compile test
# ---------------------------------------------------------------------------

COMPILE_WRAPPER = """\
#include "common.h"
#include "include_asm.h"

{code}
"""

def try_compile(func_name, code):
    """Try to compile the draft standalone. Returns (success, error_text)."""
    wrapped = COMPILE_WRAPPER.format(code=code)
    wsl_cmd = (
        f'cd {WSL_ROOT} && '
        f'{WSL_VENV} && '
        f'printf \'%s\' {json.dumps(wrapped)} > /tmp/bb2_overnight.c && '
        f'bash tools/permuter_compile.sh /tmp/bb2_overnight.c -o /tmp/bb2_overnight.o 2>&1 && '
        f'echo __OK__'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=30
        )
        output = result.stdout + result.stderr
        if "__OK__" in output:
            return True, ""
        # Extract useful error lines (strip noise, keep first 20 lines)
        error_lines = [l for l in output.splitlines()
                       if l.strip() and "__OK__" not in l][:20]
        return False, "\n".join(error_lines)
    except Exception as e:
        return False, str(e)


def fix_draft(func_name, asm, failed_code, error_text, attempt, globals_by_addr, funcs_by_addr):
    """Ask DeepSeek to fix a compile error. Returns cleaned code or None."""
    context = get_context(func_name, asm, globals_by_addr, funcs_by_addr)

    prompt = (
        "This C decompilation for GCC 2.7.2 -O2 failed to compile. "
        "Fix ALL compile errors and output ONLY the corrected C function — no explanation, no markdown.\n\n"
        "## Compile errors:\n"
        f"```\n{error_text}\n```\n\n"
        "## Failed code (attempt {attempt}):\n"
        f"```c\n{failed_code}\n```\n\n"
    )
    if context:
        prompt += f"## Reference context:\n{context}\n\n"
    prompt += "## Target assembly:\n" + asm

    payload = json.dumps({"model": MODEL, "stream": False, "prompt": prompt}).encode()
    req = urllib.request.Request(OLLAMA_URL, data=payload,
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=600) as resp:
            data = json.loads(resp.read().decode())
            raw = data.get("response", "")
            return clean_draft(raw) if raw else None
    except Exception:
        return None

# ---------------------------------------------------------------------------
# Permuter scoring
# ---------------------------------------------------------------------------

def get_permuter_score(func_name, code):
    """Write code to permuter base.c and get initial score. Returns int or None."""
    perm_dir = os.path.join(PERMUTER_DIR, func_name)
    if not os.path.isdir(perm_dir):
        return None

    base_c = os.path.join(perm_dir, "base.c")
    # Backup original
    orig = open(base_c).read() if os.path.exists(base_c) else ""

    # Wrap with headers (same as try_compile) so the permuter can compile it
    wrapped = COMPILE_WRAPPER.format(code=code)

    # Write new base via WSL
    wsl_cmd = (
        f'cd {WSL_ROOT} && '
        f'printf \'%s\' {json.dumps(wrapped)} > {json.dumps("permuter/" + func_name + "/base.c")} && '
        f'{WSL_VENV} && '
        f'python3 tools/decomp-permuter/permuter.py permuter/{func_name}/ --debug 2>&1 | head -20'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=60
        )
        output = result.stdout
        # Parse score — permuter outputs "score X" on its own line, e.g. "323" or "score: 323"
        # Skip error/syntax lines which may contain incidental numbers
        for line in output.split('\n'):
            if 'error' in line.lower() or 'syntax' in line.lower() or 'warning' in line.lower():
                continue
            m = re.match(r'^\s*(\d+)\s*$', line)
            if m:
                return int(m.group(1))
        # Fallback: look for "score X" pattern
        for line in output.split('\n'):
            m = re.search(r'\bscore[:\s]+(\d+)', line, re.IGNORECASE)
            if m:
                return int(m.group(1))
    except Exception:
        pass

    # Restore original on failure
    if orig:
        try:
            wsl_cmd2 = (
                f'cd {WSL_ROOT} && '
                f'printf \'%s\' {json.dumps(orig)} > {json.dumps("permuter/" + func_name + "/base.c")}'
            )
            subprocess.run(["wsl", "bash", "-lc", wsl_cmd2], timeout=10)
        except Exception:
            pass
    return None

# ---------------------------------------------------------------------------
# Apply match to src
# ---------------------------------------------------------------------------

def find_src_file(func_name):
    """Return the src file path that contains this stub, or None."""
    for fname in os.listdir(SRC_DIR):
        if not fname.endswith(".c"):
            continue
        path = os.path.join(SRC_DIR, fname)
        with open(path) as f:
            if f'INCLUDE_ASM("asm/funcs", {func_name})' in f.read():
                return path
    return None


def apply_to_src(func_name, code):
    """Replace INCLUDE_ASM stub with C code in the src file. Returns (success, src_path)."""
    src_path = find_src_file(func_name)
    if not src_path:
        return False, None

    with open(src_path) as f:
        content = f.read()

    stub_pattern = f'INCLUDE_ASM("asm/funcs", {func_name});'
    if stub_pattern not in content:
        stub_pattern = f'INCLUDE_ASM("asm/funcs", {func_name})'
    if stub_pattern not in content:
        return False, src_path

    new_content = content.replace(stub_pattern, code)

    # Write via WSL to preserve LF line endings
    wsl_path = src_path.replace("C:/", "/mnt/c/").replace("\\", "/")
    wsl_cmd = (
        f'printf \'%s\' {json.dumps(new_content)} > {json.dumps(wsl_path)}'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=30
        )
        return result.returncode == 0, src_path
    except Exception:
        return False, src_path


def revert_src(func_name, src_path, original_content):
    """Restore src file to original content if build fails."""
    wsl_path = src_path.replace("C:/", "/mnt/c/").replace("\\", "/")
    wsl_cmd = f'printf \'%s\' {json.dumps(original_content)} > {json.dumps(wsl_path)}'
    try:
        subprocess.run(["wsl", "bash", "-lc", wsl_cmd], timeout=30)
    except Exception:
        pass

# ---------------------------------------------------------------------------
# Build verification
# ---------------------------------------------------------------------------

def verify_build():
    """Run make and check for SHA1 match. Returns True on success."""
    wsl_cmd = (
        f'cd {WSL_ROOT} && '
        f'{WSL_VENV} && '
        f'make 2>&1 | tail -3'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=120
        )
        return "OK: bb2 matches" in result.stdout
    except Exception:
        return False

# ---------------------------------------------------------------------------
# Git
# ---------------------------------------------------------------------------

def git_commit(message):
    """Commit all staged changes via WSL."""
    wsl_cmd = (
        f'cd {WSL_ROOT} && '
        f'git add -u && '
        f'git add {PROGRESS_FILE} 2>/dev/null ; '
        f'git diff --cached --quiet || '
        f'git commit -m {json.dumps(message + chr(10) + chr(10) + "Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>")}'
    )
    try:
        result = subprocess.run(
            ["wsl", "bash", "-lc", wsl_cmd],
            capture_output=True, text=True, timeout=60
        )
        return result.returncode == 0
    except Exception:
        return False

# ---------------------------------------------------------------------------
# Stub discovery
# ---------------------------------------------------------------------------

def find_stubs():
    stubs = []
    for fname in sorted(os.listdir(SRC_DIR)):
        if not fname.endswith(".c"):
            continue
        with open(os.path.join(SRC_DIR, fname)) as f:
            for line in f:
                m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
                if m:
                    stubs.append((fname, m.group(1)))
    return stubs


def get_asm_size(func_name):
    path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(path):
        return 0
    with open(path) as f:
        return sum(1 for _ in f)

# ---------------------------------------------------------------------------
# Progress log
# ---------------------------------------------------------------------------

class ProgressLog:
    def __init__(self):
        self.entries = []
        self.matches = []
        self.start_time = datetime.now()

    def record(self, func_name, src_file, asm_lines, compiled, score, matched, note=""):
        self.entries.append({
            "func": func_name, "src": src_file, "lines": asm_lines,
            "compiled": compiled, "score": score, "matched": matched, "note": note,
            "time": datetime.now().strftime("%H:%M:%S"),
        })
        if matched:
            self.matches.append(func_name)

    def save(self):
        lines = [
            f"# Overnight Run — {self.start_time.strftime('%Y-%m-%d %H:%M')}",
            f"Updated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            "",
            f"**Progress:** {len(self.entries)} processed, "
            f"{sum(1 for e in self.entries if e['compiled'])} compiled, "
            f"{len(self.matches)} matched",
            "",
        ]
        if self.matches:
            lines += ["## Matches (applied to src)", ""]
            for m in self.matches:
                lines.append(f"- {m}")
            lines.append("")

        promising = [e for e in self.entries if e["score"] is not None and 0 < e["score"] <= 200]
        if promising:
            lines += ["## Near-misses (score 1-200, review manually)", ""]
            for e in promising:
                lines.append(f"- {e['func']} score={e['score']}  `local_drafts/bb2-deepseek/{e['func']}.c`")
            lines.append("")

        lines += ["## Full log", ""]
        lines.append(f"| Function | Lines | Compiled | Score | Result |")
        lines.append(f"|----------|-------|----------|-------|--------|")
        for e in self.entries:
            score_str = str(e["score"]) if e["score"] is not None else "—"
            result = "MATCH" if e["matched"] else ("near-miss" if e.get("score") and e["score"] <= 200 else ("compiled" if e["compiled"] else "no compile"))
            lines.append(f"| {e['func']} | {e['lines']} | {'Y' if e['compiled'] else 'N'} | {score_str} | {result} |")

        with open(PROGRESS_FILE, "w", newline="\n") as f:
            f.write("\n".join(lines) + "\n")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Overnight DeepSeek decompilation run")
    parser.add_argument("--limit", type=int, default=0, help="Max functions to process (0=all)")
    parser.add_argument("--min-lines", type=int, default=25)
    parser.add_argument("--max-lines", type=int, default=200)
    parser.add_argument("--skip-compile", action="store_true", help="Skip compile/score step")
    parser.add_argument("--min-score", type=int, default=0,
                        help="Auto-apply if score <= this (default 0 = exact match only)")
    args = parser.parse_args()

    os.makedirs(DRAFTS_DIR, exist_ok=True)

    print(f"Loading symbol table...")
    globals_by_addr, funcs_by_addr = load_symbol_table()
    print(f"  {len(globals_by_addr)} globals, {len(funcs_by_addr)} named funcs")

    print(f"Finding stubs...")
    all_stubs = find_stubs()
    stubs = []
    for src_file, func_name in all_stubs:
        size = get_asm_size(func_name)
        if size < args.min_lines or size > args.max_lines:
            continue
        stubs.append((src_file, func_name, size))

    if args.limit:
        stubs = stubs[:args.limit]

    print(f"  {len(stubs)} stubs to process (size {args.min_lines}-{args.max_lines} lines)")
    print(f"  Model: {MODEL}")
    print(f"  Auto-apply threshold: score <= {args.min_score}")
    print(f"  Compile testing: {'disabled' if args.skip_compile else 'enabled'}")
    print()

    log = ProgressLog()
    total = len(stubs)

    for i, (src_file, func_name, asm_lines) in enumerate(stubs):
        print(f"[{i+1}/{total}] {func_name} ({asm_lines} lines, {src_file})")

        # --- Generate draft ---
        # Always regenerate with DeepSeek. Existing drafts are fed as context only,
        # not used as the final answer (old Qwen drafts often don't compile).
        draft_path = os.path.join(DRAFTS_DIR, f"{func_name}.c")
        print(f"  draft: generating...", end=" ", flush=True)
        t0 = time.time()
        raw = generate_draft(func_name, globals_by_addr, funcs_by_addr)
        elapsed = time.time() - t0
        if not raw or raw.startswith("/* ERROR"):
            print(f"FAILED ({elapsed:.1f}s)")
            log.record(func_name, src_file, asm_lines, False, None, False, "generation failed")
            continue
        code = clean_draft(raw)
        with open(draft_path, "w", newline="\n") as f:
            f.write(code + "\n")
        print(f"done ({elapsed:.1f}s, {len(code)} chars)")

        if args.skip_compile:
            log.record(func_name, src_file, asm_lines, None, None, False, "compile skipped")
            continue

        # --- Compile + retry loop (up to MAX_RETRIES attempts) ---
        MAX_RETRIES = 10
        compiled = False
        asm_text = open(os.path.join(ASM_DIR, f"{func_name}.s")).read()

        for attempt in range(1, MAX_RETRIES + 1):
            print(f"  compile [{attempt}/{MAX_RETRIES}]: testing...", end=" ", flush=True)
            ok, error_text = try_compile(func_name, code)
            if ok:
                print("OK")
                compiled = True
                break
            print(f"FAIL")
            if attempt < MAX_RETRIES:
                # Feed error back to DeepSeek for a fix
                print(f"  fixing...", end=" ", flush=True)
                t0 = time.time()
                fixed = fix_draft(func_name, asm_text, code, error_text, attempt,
                                  globals_by_addr, funcs_by_addr)
                elapsed = time.time() - t0
                if fixed:
                    code = fixed
                    # Save each attempt so we have a trail
                    with open(draft_path, "w", newline="\n") as f:
                        f.write(code + "\n")
                    print(f"new draft ({elapsed:.1f}s)")
                else:
                    print(f"generation failed, stopping retries")
                    break

        if not compiled:
            log.record(func_name, src_file, asm_lines, False, None, False,
                       f"compile failed after {MAX_RETRIES} attempts")
            continue

        # --- Score ---
        score = None
        has_perm = os.path.isdir(os.path.join(PERMUTER_DIR, func_name))
        if has_perm:
            print(f"  score: checking...", end=" ", flush=True)
            score = get_permuter_score(func_name, code)
            print(f"{score}" if score is not None else "unknown")
        else:
            print(f"  score: no permuter dir, skipping")

        # --- Apply if match ---
        matched = False
        if score is not None and score <= args.min_score:
            print(f"  MATCH (score={score})! Applying to {src_file}...")

            src_path = find_src_file(func_name)
            orig_content = open(src_path).read() if src_path else ""

            applied, src_path = apply_to_src(func_name, code)
            if not applied:
                print(f"  apply: FAILED to write src")
                log.record(func_name, src_file, asm_lines, True, score, False, "apply failed")
                continue

            print(f"  build: verifying...", end=" ", flush=True)
            if verify_build():
                print("OK!")
                matched = True
                msg = f"overnight: match {func_name} (score 0)\n\nDeepSeek overnight run — exact byte match."
                if git_commit(msg):
                    print(f"  committed!")
                else:
                    print(f"  commit FAILED (saved on disk)")
            else:
                print("FAIL — reverting")
                revert_src(func_name, src_path, orig_content)

        log.record(func_name, src_file, asm_lines, True, score, matched)

        # --- Checkpoint commit every BATCH_SIZE ---
        if (i + 1) % BATCH_SIZE == 0:
            log.save()
            processed = i + 1
            matched_count = len(log.matches)
            compiled_count = sum(1 for e in log.entries if e["compiled"])
            msg = (
                f"overnight: checkpoint {processed}/{total} "
                f"({compiled_count} compiled, {matched_count} matched)"
            )
            if git_commit(msg):
                print(f"\n  [checkpoint {processed}/{total} committed]\n")
            else:
                print(f"\n  [checkpoint {processed}/{total} — nothing new to commit]\n")

    # Final save and commit
    log.save()
    matched_count = len(log.matches)
    compiled_count = sum(1 for e in log.entries if e["compiled"])
    msg = (
        f"overnight: final — {len(log.entries)} processed, "
        f"{compiled_count} compiled, {matched_count} matched"
    )
    git_commit(msg)

    # Summary
    print(f"\n{'='*60}")
    print(f"OVERNIGHT RUN COMPLETE")
    print(f"{'='*60}")
    print(f"  Processed:  {len(log.entries)}")
    print(f"  Compiled:   {compiled_count}")
    print(f"  Matched:    {matched_count}")
    if log.matches:
        print(f"\n  Matches applied:")
        for m in log.matches:
            print(f"    {m}")
    promising = [e for e in log.entries if e["score"] and 0 < e["score"] <= 200]
    if promising:
        print(f"\n  Near-misses (score 1-200): {len(promising)}")
        for e in promising[:10]:
            print(f"    {e['func']} score={e['score']}")
    print(f"\n  See {PROGRESS_FILE} for full results")


if __name__ == "__main__":
    main()
