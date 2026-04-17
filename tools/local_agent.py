#!/usr/bin/env python3
"""Local DeepSeek agent — iterative decomp matching with same protocol as Opus agents.

Runs entirely locally via Ollama. Follows the same rules:
  - Pre-screen for blockers
  - Generate initial C, score, iterate
  - Same score = escalate (no redundant C variants)
  - Permuter by attempt 3
  - Hard cap: 8 attempts
  - Logs to tmp/agent_audit/

Usage (from Git Bash, NOT WSL — Ollama only reachable from Windows):
    python tools/local_agent.py func_80089E30 src/main.c
    python tools/local_agent.py func_80089E30 src/main.c --dry-run
    python tools/local_agent.py func_80089E30 src/main.c --permuter-timeout 180
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

MODEL = "bb2-deepseek"  # overridable via --model
OLLAMA_URL = "http://localhost:11434/api/generate"
ASM_DIR = os.path.join("asm", "funcs")
PERMUTER_DIR = "permuter"
SYMBOL_ADDRS = "symbol_addrs.txt"
AUDIT_DIR = os.path.join("tmp", "agent_audit")
MAX_ATTEMPTS = 8
PERMUTER_TIMEOUT = 120
MAX_COMPILE_FIXES = 5

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def log(msg):
    ts = datetime.now().strftime("%H:%M:%S")
    print(f"[{ts}] {msg}")


def ollama_call(prompt, temperature=0.2):
    """Call DeepSeek via Ollama. Returns response text."""
    payload = json.dumps({
        "model": MODEL,
        "stream": False,
        "prompt": prompt,
        "options": {"temperature": temperature, "num_ctx": 16384},
    }).encode()
    req = urllib.request.Request(
        OLLAMA_URL, data=payload,
        headers={"Content-Type": "application/json"},
    )
    try:
        with urllib.request.urlopen(req, timeout=600) as resp:
            data = json.loads(resp.read().decode())
            return data.get("response", "").strip()
    except Exception as e:
        return f"/* ERROR: {e} */"


def wsl(cmd, timeout=120):
    """Run a command in WSL, return (returncode, stdout+stderr)."""
    full = f'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && {cmd}'
    result = subprocess.run(
        ["wsl", "bash", "-c", full],
        capture_output=True, text=True, timeout=timeout,
    )
    output = (result.stdout + result.stderr).strip()
    return result.returncode, output


def strip_markdown(code):
    """Remove markdown fences and bad includes from model output."""
    code = re.sub(r'^```[cC]?\s*\n?', '', code)
    code = re.sub(r'\n?```\s*$', '', code)
    # Remove any #include lines the model adds (base.c header handles this)
    code = re.sub(r'^\s*#include\s*[<"].*[>"]\s*\n?', '', code, flags=re.MULTILINE)
    # Remove duplicate blank lines
    code = re.sub(r'\n{3,}', '\n\n', code)
    return code.strip()


def fix_register_names(code):
    """Rename MIPS register names used as C variables to safe names."""
    # Map of register names to safe replacements
    reg_names = {
        'v0': 'val0', 'v1': 'val1',
        'a0': 'arg0', 'a1': 'arg1', 'a2': 'arg2', 'a3': 'arg3',
        'a4': 'arg4', 'a5': 'arg5', 'a6': 'arg6', 'a7': 'arg7',
        't0': 'tmp0', 't1': 'tmp1', 't2': 'tmp2', 't3': 'tmp3',
        't4': 'tmp4', 't5': 'tmp5', 't6': 'tmp6', 't7': 'tmp7',
        't8': 'tmp8', 't9': 'tmp9',
        's0': 'loc0', 's1': 'loc1', 's2': 'loc2', 's3': 'loc3',
        's4': 'loc4', 's5': 'loc5', 's6': 'loc6', 's7': 'loc7',
        's8': 'loc8',
        'sp': 'stkp', 'fp': 'frmp', 'ra': 'retaddr', 'gp': 'glbp', 'at': 'asmtmp',
    }
    for reg, safe in reg_names.items():
        # Only replace whole-word occurrences that look like variable uses
        # (not inside strings or comments, not part of longer identifiers)
        code = re.sub(r'\b' + reg + r'\b(?!_)', safe, code)
    return code


def fix_c89_declarations(code):
    """Fix C99-style mid-block declarations for GCC 2.7.2 compatibility.

    Strategy: find all 'type var = expr;' lines that appear after a non-declaration
    statement, split them into 'type var;' (hoisted to top) + 'var = expr;' (in place).
    Also declares any undeclared variables as s32 at function top.
    """
    type_pat = r'(?:(?:const\s+)?(?:extern\s+)?(?:u8|s8|u16|s16|u32|s32|void|vu8|vs16|vs32)\s*\**)'
    lines = code.split('\n')

    # Pass 1: Find the function opening brace
    func_open = -1
    for i, line in enumerate(lines):
        if '{' in line and (')' in line or i > 0):
            func_open = i
            break
    if func_open < 0:
        return code

    # Pass 2: Split initialized declarations that appear after statements
    new_lines = []
    hoisted_decls = []
    in_decl_section = True  # True until first non-decl statement

    for i, line in enumerate(lines):
        stripped = line.strip()

        if i <= func_open:
            new_lines.append(line)
            continue

        # Detect type-prefixed declarations with initializer
        m = re.match(r'^(\s*)(' + type_pat + r'\s+)(\*?\s*\w+)\s*=\s*(.+;)$', stripped)
        if m:
            indent = '    '
            type_part, var_part, init_part = m.group(2), m.group(3), m.group(4)
            if not in_decl_section:
                # Hoist declaration, keep assignment in place
                hoisted_decls.append(f"{indent}{type_part}{var_part};")
                new_lines.append(f"{indent}{var_part.strip()} = {init_part}")
            else:
                # Split even in decl section (move init to after decls)
                new_lines.append(line)
            continue

        # Pure declarations (no init) stay in place
        if re.match(r'^\s*' + type_pat, stripped) and '=' not in stripped and ';' in stripped:
            new_lines.append(line)
            continue

        # Check if we've left the declaration section
        if stripped and stripped != '{' and stripped != '}' and not stripped.startswith('//') and not stripped.startswith('extern'):
            if not re.match(r'^\s*' + type_pat, stripped):
                in_decl_section = False

        new_lines.append(line)

    # Inject hoisted declarations after function opening brace
    if hoisted_decls:
        result = []
        injected = False
        for i, line in enumerate(new_lines):
            result.append(line)
            if i == func_open and not injected:
                for decl in hoisted_decls:
                    result.append(decl)
                injected = True
        return '\n'.join(result)

    return '\n'.join(new_lines)

# ---------------------------------------------------------------------------
# Pre-screening
# ---------------------------------------------------------------------------

def pre_screen(func_name):
    """Check asm for known blockers. Returns (ok, reason)."""
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(asm_path):
        return False, f"asm file not found: {asm_path}"

    with open(asm_path) as f:
        asm = f.read()

    if re.search(r'lwl|lwr|swl|swr', asm):
        return False, "BLOCKED: lwl/lwr/swl/swr (needs fix_lwl.py)"
    if re.search(r'\badd\b|\baddi\b|\bsub\b|syscall|break ', asm):
        return False, "BLOCKED: likely handwritten asm"
    if re.search(r'\.word 0x800', asm):
        return False, "WARNING: may need rodata split"

    lines = len(asm.strip().split('\n'))
    return True, f"OK ({lines} lines)"

# ---------------------------------------------------------------------------
# Permuter integration
# ---------------------------------------------------------------------------

def setup_permuter(func_name, src_file):
    """Set up permuter directory."""
    rc, out = wsl(f"bash tools/dc.sh setup {func_name} {src_file}")
    pdir = os.path.join(PERMUTER_DIR, func_name)
    if not os.path.isdir(pdir):
        log(f"WARNING: permuter setup may have failed: {out}")
    return pdir


_original_headers = {}  # cache per pdir

def write_base_c(pdir, code):
    """Write code to permuter base.c. Always uses the ORIGINAL header (from setup)."""
    base_path = os.path.join(pdir, "base.c")

    # Cache the original header on first call
    if pdir not in _original_headers:
        header = '#include "common.h"\n\n'
        if os.path.exists(base_path):
            with open(base_path) as f:
                existing = f.read()
            header_lines = []
            for line in existing.split('\n'):
                if line.startswith('#') or line.strip() == '':
                    header_lines.append(line)
                else:
                    break
            if header_lines:
                header = '\n'.join(header_lines) + '\n\n'
        _original_headers[pdir] = header

    code = strip_markdown(code)
    with open(base_path, 'w', newline='\n') as f:
        f.write(_original_headers[pdir] + code + '\n')


def _wsl_path(p):
    """Convert Windows path separators to forward slashes for WSL."""
    return p.replace('\\', '/')


def get_score(pdir):
    """Get permuter score. Returns (score_int, raw_output) or (None, error).
    Uses compile first to get errors, then score if compile succeeds."""
    wp = _wsl_path(pdir)
    # Try compile first to capture errors
    rc, compile_out = wsl(f"bash tools/dc.sh compile {wp}")
    if rc != 0:
        return None, compile_out
    # Compile succeeded — get score
    rc, out = wsl(f"bash tools/dc.sh score {wp}")
    m = re.search(r'score\s*[=:]\s*(\d+)', out, re.IGNORECASE)
    if m:
        return int(m.group(1)), out
    return None, out


def get_debug_diff(pdir):
    """Get full permuter debug diff."""
    rc, out = wsl(f"bash tools/dc.sh debug {_wsl_path(pdir)}")
    return out


def run_permuter(pdir, timeout=120):
    """Run permuter. Returns best score or None."""
    log(f"Running permuter ({timeout}s max)...")
    wp = _wsl_path(pdir)
    rc, out = wsl(
        f"timeout {timeout} python3 tools/decomp-permuter/permuter.py {wp} -j4 --stop-on-zero 2>&1 | tail -30",
        timeout=timeout + 30,
    )
    if "score = 0" in out.lower() or "score=0" in out.lower():
        log("Permuter found score 0!")
        return 0
    scores = re.findall(r'score\s*[=:]\s*(\d+)', out, re.IGNORECASE)
    if scores:
        best = min(int(s) for s in scores)
        log(f"Permuter best: {best}")
        return best
    log(f"Permuter finished, no scores parsed")
    return None


def load_permuter_best(pdir):
    """Load the best permuter output's source code."""
    best_score, best_path = None, None
    for entry in os.listdir(pdir):
        m = re.match(r'output-(\d+)-\d+', entry)
        if m:
            score = int(m.group(1))
            sp = os.path.join(pdir, entry, "source.c")
            if os.path.exists(sp) and (best_score is None or score < best_score):
                best_score, best_path = score, sp
    if best_path:
        with open(best_path) as f:
            code = f.read()
        # Strip header lines (#include, extern, blank) to get just the function
        lines = code.split('\n')
        func_lines = []
        in_func = False
        for line in lines:
            if line.startswith('#'):
                continue
            if not in_func and (line.strip().startswith('extern') or line.strip() == ''):
                continue
            in_func = True
            func_lines.append(line)
        return '\n'.join(func_lines).strip()
    return None

# ---------------------------------------------------------------------------
# DeepSeek prompts
# ---------------------------------------------------------------------------

def generate_initial(func_name, asm):
    """Generate initial C decompilation."""
    prompt = (
        "Decompile this MIPS R3000A assembly to matching C for GCC 2.7.2 -O2.\n"
        "Output ONLY the C function. No explanation, no markdown fences.\n\n"
        "RULES:\n"
        "- C89 only. All variables declared at top of function/block.\n"
        "- Use types: s32, u8, s16, u16, u32, s8 (never int/short/char/unsigned).\n"
        "- Never use register asm() or inline assembly.\n"
        "- Prefer do {{ }} while() for loops.\n"
        "- Use goto for unconditional jumps to non-loop targets.\n"
        "- lui+lo16 globals: extern type D_XXXXXXXX; at function top.\n"
        "- $s0-$s7 saved in prologue = local variables.\n"
        "- Delay slot: instruction after jal/branch executes BEFORE it.\n"
        "- Never use MIPS register names (v0, a0, s0, etc.) as C variable names.\n\n"
        f"## Assembly ({func_name}):\n{asm}"
    )
    return ollama_call(prompt, temperature=0.2)


def ask_fix_compile(code, error):
    """Ask to fix compilation error."""
    prompt = (
        "Fix this compilation error for GCC 2.7.2 -O2.\n"
        "Output ONLY the corrected C function, nothing else.\n"
        "RULES: C89 only, types s32/u8/s16/u16/u32, no register asm.\n\n"
        f"## Code:\n{code}\n\n"
        f"## Error:\n{error}\n"
    )
    return ollama_call(prompt, temperature=0.1)


def ask_improve(code, diff, score, asm):
    """Ask to improve based on diff."""
    if len(diff) > 3000:
        diff = diff[:3000] + "\n... (truncated)"
    prompt = (
        f"This C doesn't match the target (score={score}, 0=exact).\n"
        "Lines with '<' = TARGET (wanted). Lines with '>' = YOUR output.\n"
        "Fix the C to reduce differences. Output ONLY the corrected function.\n\n"
        "HINTS:\n"
        "- Different registers = change variable declaration order or type\n"
        "- Missing/extra instructions = wrong control flow\n"
        "- Different instruction order = expression evaluation order\n"
        "- C89 only, project types (s32/u8/etc), no asm\n\n"
        f"## Code:\n{code}\n\n"
        f"## Diff:\n{diff}\n\n"
        f"## Target asm:\n{asm}\n"
    )
    return ollama_call(prompt, temperature=0.3)

# ---------------------------------------------------------------------------
# Audit log
# ---------------------------------------------------------------------------

class AuditLog:
    def __init__(self, func_name, agent_id, src_file):
        os.makedirs(AUDIT_DIR, exist_ok=True)
        base = os.path.splitext(os.path.basename(src_file))[0]
        self.path = os.path.join(AUDIT_DIR, f"local_{agent_id}_{base}.log")
        self.func = func_name
        self._w(f"=== [{func_name}] ===")
        self._w(f"MODEL: {MODEL}")
        self._w(f"START: {datetime.now().isoformat()}")

    def _w(self, line):
        with open(self.path, 'a', newline='\n') as f:
            f.write(line + '\n')
        log(line)

    def pre_screen(self, result):
        self._w(f"PRE-SCREEN: {result}")

    def attempt(self, n, score, change, hypothesis=""):
        if hypothesis:
            self._w(f"HYPOTHESIS: {hypothesis}")
        self._w(f"ATTEMPT {n}: score={score}, change={change}")

    def result(self, outcome, total, best=None):
        if best is not None:
            self._w(f"BEST_SCORE: {best}")
        self._w(f"RESULT: {outcome}")
        self._w(f"END: {datetime.now().isoformat()}")
        self._w(f"TOTAL_ATTEMPTS: {total}")

# ---------------------------------------------------------------------------
# Main agent loop
# ---------------------------------------------------------------------------

def run_agent(func_name, src_file, dry_run=False, permuter_timeout=120, agent_id="deepseek"):
    log(f"=== Local Agent: {func_name} ({src_file}) ===")
    audit = AuditLog(func_name, agent_id, src_file)

    # 1. Pre-screen
    ok, reason = pre_screen(func_name)
    audit.pre_screen(reason)
    if not ok:
        audit.result(f"TABLED ({reason})", 0)
        return False

    # 2. Read asm
    asm_path = os.path.join(ASM_DIR, f"{func_name}.s")
    with open(asm_path) as f:
        asm = f.read()
    audit._w(f"ASM_SIZE: {len(asm.strip().splitlines())}")

    # 3. Setup permuter
    pdir = setup_permuter(func_name, src_file)

    # 4. Generate initial C
    log(f"Generating initial C with {MODEL}...")
    code = strip_markdown(generate_initial(func_name, asm))
    if "ERROR" in code:
        audit.result(f"TABLED (generation error)", 0)
        return False

    # 5. Compile-fix loop
    compiled = False
    code = fix_register_names(code)
    code = fix_c89_declarations(code)
    for fix_i in range(MAX_COMPILE_FIXES):
        write_base_c(pdir, code)
        score, out = get_score(pdir)
        if score is not None:
            compiled = True
            break
        log(f"Compile error (fix {fix_i+1}/{MAX_COMPILE_FIXES}): {out[:150]}")
        new_code = strip_markdown(ask_fix_compile(code, out))
        if "ERROR" in new_code:
            break
        new_code = fix_register_names(new_code)
        new_code = fix_c89_declarations(new_code)
        code = new_code

    if not compiled:
        audit._w(f"LAST_ERROR: {out[:300]}")
        audit.result("TABLED (compile failed)", 0)
        return False

    # 6. Score check
    if score == 0:
        audit.attempt(1, 0, "initial generation — exact match!")
        audit.result("MATCHED", 1, 0)
        if not dry_run:
            integrate(func_name, src_file, code, pdir)
        return True

    audit.attempt(1, score, "initial DeepSeek generation")
    best_score = score
    best_code = code
    prev_score = score
    same_count = 0
    attempt = 1
    permuter_ran = False

    # 7. Iteration loop
    while attempt < MAX_ATTEMPTS:
        attempt += 1

        # Same score = escalate to permuter
        if same_count >= 1 and not permuter_ran:
            log(f"Same score {prev_score} twice — running permuter")
            perm_best = run_permuter(pdir, timeout=permuter_timeout)
            permuter_ran = True
            if perm_best == 0:
                audit.attempt(attempt, 0, "permuter found exact match")
                audit.result("MATCHED (permuter)", attempt, 0)
                if not dry_run:
                    integrate_permuter(func_name, src_file, pdir)
                return True
            if perm_best is not None and perm_best < best_score:
                best_score = perm_best
                pcode = load_permuter_best(pdir)
                if pcode:
                    best_code = pcode
                    code = pcode
                    write_base_c(pdir, code)
                audit.attempt(attempt, best_score, f"permuter improved to {best_score}")
            else:
                audit.attempt(attempt, best_score, f"permuter no improvement (best={perm_best})")
            same_count = 0
            continue

        # Permuter by attempt 3 if score > 200
        if attempt >= 3 and not permuter_ran and best_score > 200:
            log(f"Attempt {attempt}, score {best_score} > 200 — permuter")
            perm_best = run_permuter(pdir, timeout=permuter_timeout)
            permuter_ran = True
            if perm_best == 0:
                audit.attempt(attempt, 0, "permuter exact match")
                audit.result("MATCHED (permuter)", attempt, 0)
                if not dry_run:
                    integrate_permuter(func_name, src_file, pdir)
                return True
            if perm_best is not None and perm_best < best_score:
                best_score = perm_best
                pcode = load_permuter_best(pdir)
                if pcode:
                    best_code = pcode
                    code = pcode
                    write_base_c(pdir, code)
                audit.attempt(attempt, best_score, f"permuter improved to {best_score}")
            else:
                audit.attempt(attempt, best_score, f"permuter no improvement")
            same_count = 0
            continue

        # Ask DeepSeek to improve
        diff = get_debug_diff(pdir)
        hyp = f"reduce from {best_score} via diff analysis ({MODEL})"
        new_code = strip_markdown(ask_improve(best_code, diff, best_score, asm))

        if "ERROR" in new_code or not new_code.strip():
            audit.attempt(attempt, best_score, "model error/empty", hyp)
            continue

        write_base_c(pdir, new_code)
        new_score, out = get_score(pdir)

        if new_score is None:
            write_base_c(pdir, best_code)
            audit.attempt(attempt, "COMPILE_ERR", "reverted", hyp)
            continue

        audit.attempt(attempt, new_score, f"DeepSeek (was {best_score})", hyp)

        if new_score == 0:
            audit.result("MATCHED", attempt, 0)
            if not dry_run:
                integrate(func_name, src_file, new_code, pdir)
            return True

        if new_score > best_score:
            log(f"Regression {best_score} -> {new_score}, reverting")
            write_base_c(pdir, best_code)
            same_count = 0
        elif new_score == prev_score:
            same_count += 1
            best_code = new_code
        else:
            best_score = new_score
            best_code = new_code
            same_count = 0

        prev_score = new_score

        # Low score after permuter = regfix territory
        if permuter_ran and 0 < best_score <= 200:
            log(f"Score {best_score} after permuter — needs regfix (manual)")
            audit._w(f"NOTE: regfix territory (score={best_score})")
            break

    # Save best draft regardless
    draft_dir = os.path.join("local_drafts", MODEL)
    os.makedirs(draft_dir, exist_ok=True)
    draft_path = os.path.join(draft_dir, f"{func_name}.c")
    with open(draft_path, 'w', newline='\n') as f:
        f.write(best_code)
    log(f"Saved best draft to {draft_path} (score={best_score})")

    audit.result(f"TABLED (best={best_score})", attempt, best_score)
    return False


def integrate(func_name, src_file, code, pdir):
    """Integrate matched function into source."""
    log(f"Integrating {func_name} into {src_file}...")
    code = strip_markdown(code)
    # Write to tmp, use dc.sh replace
    tmp = "/tmp/local_agent_match.c"
    with open(tmp, 'w', newline='\n') as f:
        f.write(code)
    rc, out = wsl(f"bash tools/dc.sh replace {src_file} {func_name} {tmp}")
    log(out)
    rc, out = wsl("make 2>&1 | tail -5")
    log(out)
    if "OK: bb2 matches!" in out:
        log("BUILD VERIFIED!")
    else:
        log("WARNING: build mismatch after integration")


def integrate_permuter(func_name, src_file, pdir):
    """Integrate from permuter's best output."""
    code = load_permuter_best(pdir)
    if code:
        integrate(func_name, src_file, code, pdir)
    else:
        log("ERROR: no permuter output to integrate")


# ---------------------------------------------------------------------------
# Entry
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Local decomp agent (Ollama)")
    parser.add_argument("func", help="Function name")
    parser.add_argument("src", help="Source file (e.g. src/main.c)")
    parser.add_argument("--model", default=None,
                        help="Ollama model name (default: bb2-deepseek)")
    parser.add_argument("--dry-run", action="store_true",
                        help="Score only, don't integrate into source")
    parser.add_argument("--permuter-timeout", type=int, default=PERMUTER_TIMEOUT,
                        help=f"Permuter timeout seconds (default: {PERMUTER_TIMEOUT})")
    parser.add_argument("--agent-id", default=None,
                        help="Agent ID for audit log (default: model name)")
    args = parser.parse_args()

    if args.model:
        global MODEL
        MODEL = args.model
    if args.agent_id is None:
        args.agent_id = MODEL.replace("bb2-", "")

    success = run_agent(
        args.func, args.src,
        dry_run=args.dry_run,
        permuter_timeout=args.permuter_timeout,
        agent_id=args.agent_id,
    )
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
